#include "elsim/core/Simulator.hpp"

#include <algorithm>
#include <cctype>
#include <limits>

#include "elsim/core/BoardDescription.hpp"
#include "elsim/core/DeviceMemoryAdapter.hpp"
#include "elsim/core/FakeCpu.hpp"
#include "elsim/core/MemoryBusAdapter.hpp"
#include "elsim/device/DeviceFactory.hpp"  // знадобиться пізніше в loadBoard
#include "elsim/device/VirtualLedDevice.hpp"

namespace {
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}
}  // namespace

namespace elsim::core {

Simulator::Simulator(std::ostream& log)
    : log_(log), running_(false), cycleCount_(0), memoryBus_(nullptr), cpu_(nullptr) {
    log_ << "[Simulator] Created (empty state)\n";
}

Simulator::~Simulator() = default;

void Simulator::loadBoard(const BoardDescription& board) {
    // Скидаємо стан симулятора
    running_ = false;
    cycleCount_ = 0;

    cpu_.reset();
    devices_.clear();
    memoryBus_.reset();
    gpio_.reset();

    log_ << "[Simulator] Loading board: " << board.name << "\n";
    log_ << "[Simulator] Description: " << board.description << "\n";

    // --- CPU info + створення CPU ---
    if (board.cpu.type.empty()) {
        throw std::runtime_error("BoardDescription.cpu.type must not be empty");
    }

    log_ << "[Simulator] CPU type: " << board.cpu.type << ", freq: " << board.cpu.frequencyHz << " Hz"
         << ", endianness: " << board.cpu.endianness << "\n";

    // Обираємо реалізацію CPU за типом.
    // Поки що підтримуємо лише "test-cpu" -> FakeCpu.
    if (board.cpu.type == "test-cpu") {
        cpu_ = std::make_unique<FakeCpu>();
        log_ << "[Simulator] Created FakeCpu for type 'test-cpu'\n";
    } else {
        throw std::runtime_error("Unsupported CPU type: '" + board.cpu.type +
                                 "'. Only 'test-cpu' is supported at the moment.");
    }

    // --- Обчислюємо розмір RAM і створюємо MemoryBus ---
    std::size_t ramSize = 0;
    bool hasRam = false;

    for (const auto& region : board.memory) {
        if (region.type == MemoryType::Ram) {
            if (hasRam) {
                throw std::runtime_error("Multiple RAM regions are not supported yet");
            }

            if (region.baseAddress != 0) {
                throw std::runtime_error("RAM region '" + region.name +
                                         "' must start at baseAddress 0 in current implementation");
            }

            if (region.sizeBytes == 0) {
                throw std::runtime_error("RAM region '" + region.name + "' must have non-zero size");
            }

            if (region.sizeBytes > std::numeric_limits<std::size_t>::max()) {
                throw std::runtime_error("RAM region '" + region.name + "' size does not fit into size_t");
            }

            ramSize = static_cast<std::size_t>(region.sizeBytes);
            hasRam = true;
        }
    }

    if (!hasRam) {
        throw std::runtime_error("BoardDescription must define at least one RAM memory region");
    }

    memoryBus_ = std::make_unique<MemoryBus>(ramSize);
    log_ << "[Simulator] Created MemoryBus with RAM size " << ramSize << " bytes\n";

    // --- Підключаємо MemoryBus до CPU через адаптер ---
    if (!cpu_) {
        throw std::runtime_error("Simulator::loadBoard: CPU is not initialized");
    }

    // Створюємо адаптер, який реалізує IMemoryBus для CPU.
    auto busAdapter = std::make_shared<MemoryBusAdapter>(memoryBus_.get());
    cpu_->setMemoryBus(busAdapter);
    log_ << "[Simulator] Connected CPU to MemoryBus via MemoryBusAdapter\n";

    // --- Логування пам'яті (для дебагу карти) ---
    log_ << "[Simulator] Memory regions: " << board.memory.size() << "\n";
    for (const auto& region : board.memory) {
        const char* typeStr = "?";
        switch (region.type) {
            case MemoryType::Ram:
                typeStr = "RAM";
                break;
            case MemoryType::Rom:
                typeStr = "ROM";
                break;
            case MemoryType::Mmio:
                typeStr = "MMIO";
                break;
        }

        log_ << "  - [" << typeStr << "] " << region.name << " @ 0x" << std::hex << region.baseAddress << " size "
             << std::dec << region.sizeBytes << " bytes\n";
    }

    // За потреби можна розширити API MemoryBus для складніших MMIO-пристроїв і регіонів.

    // --- Пристрої: створюємо через DeviceFactory ---
    log_ << "[Simulator] Devices: " << board.devices.size() << "\n";

    // v0.3: Determine GPIO pin count "from the board" (first gpio device param pin_count, else default 32).
    std::uint32_t pinCount = 32;

    for (const auto& devDesc : board.devices) {
        const std::string typeLower = toLower(devDesc.type);
        if (typeLower != "gpio") {
            continue;
        }

        auto it = devDesc.params.find("pin_count");
        if (it == devDesc.params.end()) {
            break;  // gpio present but no pin_count -> keep default 32
        }

        unsigned long v = std::stoul(it->second, nullptr, 0);  // base 0 supports "32" and "0x20"
        if (v == 0 || v > 32) {
            throw std::runtime_error("Simulator: GPIO pin_count must be in range 1..32");
        }

        pinCount = static_cast<std::uint32_t>(v);
        break;
    }

    ::elsim::DeviceFactory::BoardServices services{};
    gpio_ = std::make_shared<::elsim::core::GpioController>(pinCount);
    services.gpio = gpio_;

    for (const auto& devDesc : board.devices) {
        log_ << "  - Creating device '" << devDesc.name << "' of type '" << devDesc.type << "' @ 0x" << std::hex
             << devDesc.baseAddress << std::dec << "\n";

        const std::string typeLower = toLower(devDesc.type);

        ::elsim::IDevice* raw = nullptr;

        if (typeLower == "gpio" || typeLower == "led" || typeLower == "virtual-led" || typeLower == "button" ||
            typeLower == "virtual-button") {
            raw = ::elsim::DeviceFactory::createDevice(devDesc, services);
        } else {
            raw = ::elsim::DeviceFactory::createDevice(devDesc);
        }

        devices_.emplace_back(raw);

        // --- Пошук MMIO-регіону для цього девайса ---
        std::uint32_t mmioSize = 0;
        for (const auto& region : board.memory) {
            if (region.type == MemoryType::Mmio && region.baseAddress == devDesc.baseAddress) {
                if (region.sizeBytes == 0) {
                    throw std::runtime_error("MMIO region '" + region.name + "' for device '" + devDesc.name +
                                             "' has zero size");
                }
                if (region.sizeBytes > std::numeric_limits<std::uint32_t>::max()) {
                    throw std::runtime_error("MMIO region '" + region.name +
                                             "' size does not fit into 32-bit for device '" + devDesc.name + "'");
                }

                mmioSize = static_cast<std::uint32_t>(region.sizeBytes);
                break;
            }
        }
        if (mmioSize == 0) {
            continue;
        }

        // --- Мапимо девайс у MemoryBus через DeviceMemoryAdapter ---
        auto* devicePtr = devices_.back().get();
        auto mmioAdapter = std::make_shared<DeviceMemoryAdapter>(devicePtr);
        memoryBus_->mapDevice(static_cast<std::uint32_t>(devDesc.baseAddress), mmioSize, mmioAdapter);
        log_ << "[Simulator] Mapped device '" << devDesc.name << "' to MMIO region @ 0x" << std::hex
             << devDesc.baseAddress << std::dec << " size " << mmioSize << " bytes\n";
    }

    log_ << "[Simulator] Board loaded successfully " << "(CPU created, MemoryBus created, devices instantiated; "
         << "memory wiring & MMIO will be implemented in next steps).\n";
}

void Simulator::start(std::uint64_t maxCycles) {
    if (!cpu_) {
        log_ << "[Simulator] ERROR: Cannot start — CPU not initialized!\n";
        return;
    }

    running_ = true;
    cycleCount_ = 0;

    log_ << "[Simulator] Starting simulation...\n";

    while (running_) {
        runOneTick();

        if (maxCycles != 0 && cycleCount_ >= maxCycles) {
            log_ << "[Simulator] Max cycles reached.\n";
            break;
        }
    }

    log_ << "[Simulator] Simulation finished.\n";
}

void Simulator::stop() { running_ = false; }

void Simulator::runOneTick() {
    if (!cpu_) {
        log_ << "[Simulator] ERROR: runOneTick() but CPU is null.\n";
        running_ = false;
        return;
    }

    // 1. Дати CPU виконати один крок
    cpu_->step();

    // 2. Перевірити HALT
    if (cpu_->isHalted()) {
        log_ << "[Simulator] CPU entered HALT state. Stopping simulation.\n";
        running_ = false;
        return;  // Важливо: не оновлюємо девайси і не збільшуємо лічильник циклів
    }

    // 3. Оновити всі пристрої
    for (auto& dev : devices_) {
        if (dev) {
            dev->tick();
        }
    }

    // 4. Збільшити кількість циклів
    ++cycleCount_;
}

bool Simulator::isRunning() const noexcept { return running_; }

std::uint64_t Simulator::cycleCount() const noexcept { return cycleCount_; }

ICpu* Simulator::cpu() noexcept { return cpu_.get(); }

const ICpu* Simulator::cpu() const noexcept { return cpu_.get(); }

MemoryBus* Simulator::memoryBus() noexcept { return memoryBus_.get(); }

const MemoryBus* Simulator::memoryBus() const noexcept { return memoryBus_.get(); }

std::shared_ptr<const elsim::core::GpioController> Simulator::gpioController() const noexcept { return gpio_; }

std::vector<const ::elsim::VirtualLedDevice*> Simulator::ledDevices() const {
    std::vector<const ::elsim::VirtualLedDevice*> out;
    for (const auto& d : devices_) {
        if (!d) continue;
        if (auto* led = dynamic_cast<const ::elsim::VirtualLedDevice*>(d.get())) {
            out.push_back(led);
        }
    }
    return out;
}

}  // namespace elsim::core
