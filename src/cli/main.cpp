#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

#include "elsim/core/ICpu.hpp"
#include "elsim/core/Logger.hpp"
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

// ---- Глобальні using'и для логера ----
using elsim::core::Logger;
using elsim::core::LogLevel;

// ---- Парсер рівня логування ----
std::optional<LogLevel> parse_log_level(std::string_view value) {
    if (value == "debug") {
        return LogLevel::Debug;
    }
    if (value == "info") {
        return LogLevel::Info;
    }
    if (value == "error") {
        return LogLevel::Error;
    }
    if (value == "off") {
        return LogLevel::Off;
    }
    return std::nullopt;
}

int main(int argc, char** argv) {
    fs::path configPath;
    bool hasConfig = false;

    LogLevel logLevel = LogLevel::Info;  // дефолтний рівень

    // Простий ручний парсинг аргументів:
    // очікуємо:
    //   --config <path> [--log-level <debug|info|error|off>]
    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];

        if (arg == "--config") {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for --config\n";
                return 1;
            }
            configPath = fs::path{argv[++i]};
            hasConfig = true;
        } else if (arg == "--log-level") {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for --log-level\n";
                return 1;
            }
            std::string_view levelStr = argv[++i];
            auto lvl = parse_log_level(levelStr);
            if (!lvl) {
                std::cerr << "Unknown log level: " << levelStr << " (expected: debug|info|error|off)\n";
                return 1;
            }
            logLevel = *lvl;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            std::cerr << "Usage: " << argv[0]
                      << " --config <path-to-config.yaml> [--log-level <debug|info|error|off>]\n";
            return 1;
        }
    }

    if (!hasConfig) {
        std::cerr << "Missing required --config argument\n";
        std::cerr << "Usage: " << argv[0] << " --config <path-to-config.yaml> [--log-level <debug|info|error|off>]\n";
        return 1;
    }

    // Встановлюємо рівень логування
    Logger::instance().set_level(logLevel);
    Logger::instance().info("CLI", "Logger initialized");

    if (!fs::exists(configPath)) {
        Logger::instance().error("CLI", "Config file not found: " + configPath.string());
        std::cerr << "Config file not found: " << configPath << "\n";
        return 1;
    }

    std::ifstream in(configPath);
    if (!in) {
        Logger::instance().error("CLI", "Failed to open config file: " + configPath.string());
        std::cerr << "Failed to open config file: " << configPath << "\n";
        return 1;
    }

    std::string configContent((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    Logger::instance().info("CLI", "[elsim] Using config: " + configPath.string());
    Logger::instance().debug("CLI", "[elsim] Config size: " + std::to_string(configContent.size()) + " bytes");

    // Тут поки просто викликаємо заглушку.
    return elsim::cli::run_default_simulation();
}
