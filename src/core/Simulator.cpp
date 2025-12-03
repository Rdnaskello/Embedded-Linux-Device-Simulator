#include "elsim/core/Simulator.hpp"

#include <limits>

#include "elsim/core/BoardDescription.hpp"
#include "elsim/core/FakeCpu.hpp"
#include "elsim/device/DeviceFactory.hpp"  // знадобиться пізніше в loadBoard
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

    // TODO: у майбутньому тут будемо реєструвати ROM та MMIO-регіони
    //       через mapDevice(...) і підключати їх до MemoryBus.
    // TODO: коли з’явиться IMemoryBus, можна буде передати шину в CPU
    //       (cpu_->setMemoryBus(...)).

    // --- Пристрої: створюємо через DeviceFactory ---
    log_ << "[Simulator] Devices: " << board.devices.size() << "\n";

    for (const auto& devDesc : board.devices) {
        log_ << "  - Creating device '" << devDesc.name << "' of type '" << devDesc.type << "' @ 0x" << std::hex
             << devDesc.baseAddress << std::dec << "\n";

        if (devDesc.baseAddress > std::numeric_limits<std::uint32_t>::max()) {
            throw std::runtime_error("Device baseAddress out of 32-bit range for device '" + devDesc.name + "'");
        }

        // Використовуємо фабрику з elsim::DeviceFactory (не elsim::device!)
        ::elsim::IDevice* raw = ::elsim::DeviceFactory::createDevice(devDesc.type, devDesc.name,
                                                                     static_cast<std::uint32_t>(devDesc.baseAddress));

        if (!raw) {
            throw std::runtime_error("DeviceFactory could not create device of type '" + devDesc.type + "' (name: '" +
                                     devDesc.name + "')");
        }

        devices_.emplace_back(raw);

        // TODO: коли буде готовий MemoryBus API для MMIO,
        //       тут зареєструємо пристрій як MMIO-регіон через mapDevice().
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

    // Поки тільки CPU — тіку пристроїв додамо окремою таскою.
    cpu_->step();

    ++cycleCount_;
}

bool Simulator::isRunning() const noexcept { return running_; }

std::uint64_t Simulator::cycleCount() const noexcept { return cycleCount_; }

}  // namespace elsim::core
