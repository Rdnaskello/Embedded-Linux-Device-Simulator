#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

#include "elsim/core/ICpu.hpp"
#include "elsim/core/Simulator.hpp"
#include "elsim/device/IDevicesTickable.hpp"

namespace fs = std::filesystem;

namespace elsim::cli {

// ---------------------------
// Фейковий CPU, щоб тестувати Simulator
// ---------------------------
class TestCpu : public core::ICpu {
   public:
    int stepCount = 0;

    void step() override { ++stepCount; }

    void reset() override { stepCount = 0; }

    bool loadImage(const std::string& /*path*/) override {
        // Для цього smoke-тесту нам все одно, просто повернемо true
        return true;
    }

    void setMemoryBus(std::shared_ptr<core::IMemoryBus> /*bus*/) override {
        // У цьому тесті шину не використовуємо
    }
};

// ---------------------------
// Фейковий DeviceManager
// ---------------------------
class TestDevices : public device::IDevicesTickable {
   public:
    int tickCount = 0;

    void tickAll() override { ++tickCount; }
};

int run_default_simulation() {
    TestCpu cpu;
    TestDevices dev;

    core::Simulator sim(cpu, dev);

    sim.start(5);  // запускаємо рівно 5 тiкiв

    std::cout << "CPU stepCount = " << cpu.stepCount << "\n";
    std::cout << "DEV tickCount = " << dev.tickCount << "\n";
    std::cout << "sim.cycleCount() = " << sim.cycleCount() << "\n";

    if (cpu.stepCount == 5 && dev.tickCount == 5 && sim.cycleCount() == 5) {
        std::cout << "Simulator smoke test PASSED\n";
        return 0;
    } else {
        std::cout << "Simulator smoke test FAILED\n";
        return 1;
    }
}

}  // namespace elsim::cli

int main(int argc, char** argv) {
    if (argc != 3 || std::string_view(argv[1]) != "--config") {
        std::cerr << "Usage: " << argv[0] << " --config <path-to-config.yaml>\n";
        return 1;
    }

    fs::path configPath{argv[2]};

    if (!fs::exists(configPath)) {
        std::cerr << "Config file not found: " << configPath << "\n";
        return 1;
    }

    std::ifstream in(configPath);
    if (!in) {
        std::cerr << "Failed to open config file: " << configPath << "\n";
        return 1;
    }

    std::string configContent((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    std::cout << "[elsim] Using config: " << configPath << "\n";
    std::cout << "[elsim] Config size: " << configContent.size() << " bytes\n";

    // Тут поки просто викликаємо заглушку.
    return elsim::cli::run_default_simulation();
}
