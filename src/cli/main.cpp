#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

#include "elsim/core/BoardConfigParser.hpp"
#include "elsim/core/BoardDescription.hpp"
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

namespace {

// Завантажуємо та валідовуємо board-конфіг, або завершуємо процес з помилкою.
elsim::core::BoardDescription loadBoardConfigOrExit(const fs::path& configPath) {
    auto& logger = Logger::instance();

    logger.info("CLI", "[elsim] Using config: " + configPath.string());

    try {
        auto board = elsim::core::BoardConfigParser::loadFromFile(configPath.string());
        logger.info("CLI", "[elsim] Board config loaded successfully. Name: " + board.name);
        return board;
    } catch (const YAML::Exception& ex) {
        logger.error("CLI",
                     std::string("[elsim] Failed to parse YAML config '") + configPath.string() + "': " + ex.what());
        std::cerr << "Failed to parse YAML config '" << configPath << "': " << ex.what() << "\n";
        std::exit(1);
    } catch (const elsim::core::BoardConfigException& ex) {
        logger.error("CLI",
                     std::string("[elsim] Invalid board configuration in '") + configPath.string() + "': " + ex.what());
        std::cerr << "Invalid board configuration in '" << configPath << "': " << ex.what() << "\n";
        std::exit(1);
    } catch (const std::exception& ex) {
        logger.error("CLI", std::string("[elsim] Unexpected error while loading config '") + configPath.string() +
                                "': " + ex.what());
        std::cerr << "Unexpected error while loading config '" << configPath << "': " << ex.what() << "\n";
        std::exit(1);
    }
}

}  // namespace

int main(int argc, char** argv) {
    fs::path configPath;
    bool hasConfig = false;

    LogLevel logLevel = LogLevel::Info;  // дефолтний рівень
    bool dryRun = false;                 // режим dry-run

    // Простий ручний парсинг аргументів:
    // очікуємо:
    //   --config <path> [--log-level <debug|info|error|off>] [--dry-run]
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
        } else if (arg == "--dry-run") {
            dryRun = true;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            std::cerr << "Usage: " << argv[0] << " --config <path-to-config.yaml>"
                      << " [--log-level <debug|info|error|off>]" << " [--dry-run]\n";
            return 1;
        }
    }

    if (!hasConfig) {
        std::cerr << "Missing required --config argument\n";
        std::cerr << "Usage: " << argv[0] << " --config <path-to-config.yaml>"
                  << " [--log-level <debug|info|error|off>]" << " [--dry-run]\n";
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

    // NEW: dry-run режим — тепер реально парсимо YAML і перевіряємо BoardDescription
    if (dryRun) {
        Logger::instance().info("CLI",
                                "[elsim] Dry-run mode: configuration file will be validated. "
                                "Simulator will NOT be started.");

        // 1. Завантажити і провалідувати конфіг
        auto board = loadBoardConfigOrExit(configPath);

        // (Опційно) мінімальний smoke-тест, що Simulator взагалі може створитись.
        try {
            elsim::cli::TestCpu cpu;
            elsim::cli::TestDevices dev;

            elsim::core::Simulator sim(cpu, dev);

            Logger::instance().debug("CLI", "[elsim] Board has " + std::to_string(board.memory.size()) +
                                                " memory regions and " + std::to_string(board.devices.size()) +
                                                " devices.");

            Logger::instance().info("CLI",
                                    "[elsim] Dry-run successful: configuration file is valid. "
                                    "Simulator constructed correctly.");
            return 0;
        } catch (const std::exception& ex) {
            Logger::instance().error("CLI",
                                     std::string("[elsim] Dry-run failed while constructing Simulator: ") + ex.what());
            return 1;
        } catch (...) {
            Logger::instance().error("CLI", "[elsim] Dry-run failed: unknown error during Simulator construction.");
            return 1;
        }
    }

    // Поки що, коли не dry-run, просто запускаємо smoke-тест симулятора.
    // Наступні таски під'єднають сюди реальну ініціалізацію з BoardDescription.
    return elsim::cli::run_default_simulation();
}
