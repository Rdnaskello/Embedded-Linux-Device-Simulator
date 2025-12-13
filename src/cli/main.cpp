#include <yaml-cpp/yaml.h>

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

#include "elsim/core/BoardConfigParser.hpp"
#include "elsim/core/BoardDescription.hpp"
#include "elsim/core/Logger.hpp"
#include "elsim/core/ProgramLoader.hpp"
#include "elsim/core/Simulator.hpp"

namespace fs = std::filesystem;

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

    fs::path programPath;
    bool hasProgram = false;

    // Простий ручний парсинг аргументів:
    // очікуємо:
    //   --config <path> [--log-level <debug|info|error|off>] [--dry-run] [--program <path>]
    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];

        if (arg == "--config") {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for --config\n";
                std::cerr << "Usage: " << argv[0] << " --config <path-to-config.yaml>"
                          << " [--log-level <debug|info|error|off>]" << " [--dry-run]"
                          << " [--program <path-to-program.elsim-bin>]\n";
                return 1;
            }
            configPath = fs::path{argv[++i]};
            hasConfig = true;
        } else if (arg == "--log-level") {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for --log-level\n";
                std::cerr << "Usage: " << argv[0] << " --config <path-to-config.yaml>"
                          << " [--log-level <debug|info|error|off>]" << " [--dry-run]"
                          << " [--program <path-to-program.elsim-bin>]\n";
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
        } else if (arg == "--program") {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for --program\n";
                std::cerr << "Usage: " << argv[0] << " --config <path-to-config.yaml>"
                          << " [--log-level <debug|info|error|off>]" << " [--dry-run]"
                          << " [--program <path-to-program.elsim-bin>]\n";
                return 1;
            }
            programPath = fs::path{argv[++i]};
            hasProgram = true;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            std::cerr << "Usage: " << argv[0] << " --config <path-to-config.yaml>"
                      << " [--log-level <debug|info|error|off>]" << " [--dry-run]"
                      << " [--program <path-to-program.elsim-bin>]\n";
            return 1;
        }
    }

    if (!hasConfig) {
        std::cerr << "Missing required --config argument\n";
        std::cerr << "Usage: " << argv[0] << " --config <path-to-config.yaml>"
                  << " [--log-level <debug|info|error|off>]" << " [--dry-run]"
                  << " [--program <path-to-program.elsim-bin>]\n";
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

    // 1. Завантажити і провалідувати конфіг (спільно для dry-run та normal mode)
    auto board = loadBoardConfigOrExit(configPath);

    try {
        if (dryRun) {
            Logger::instance().info(
                "CLI", "[elsim] Dry-run mode: configuration file will be validated. Simulator will NOT be started.");
        } else {
            Logger::instance().info("CLI", "[elsim] Starting simulation...");
        }

        // Створюємо симулятор і завантажуємо плату
        elsim::core::Simulator sim(std::cout);
        sim.loadBoard(board);

        // --- NEW: якщо задана програма, завантажуємо її в пам'ять і виставляємо PC ---
        if (hasProgram) {
            Logger::instance().info("CLI", "[elsim] Loading program from '" + programPath.string() + "'");

            auto* bus = sim.memoryBus();
            if (!bus) {
                throw std::runtime_error("Simulator has no MemoryBus initialized.");
            }

            elsim::core::ProgramLoader loader;
            std::uint32_t entryPoint = 0;
            loader.loadBinary(programPath.string(), *bus, entryPoint);

            auto* cpu = sim.cpu();
            if (!cpu) {
                throw std::runtime_error("Simulator has no CPU initialized.");
            }

            cpu->setPC(entryPoint);

            Logger::instance().info(
                "CLI", "[elsim] Program loaded successfully. Entry point set to " + std::to_string(entryPoint));
        } else {
            Logger::instance().warn("CLI",
                                    "[elsim] No program specified via --program. CPU will start from its reset PC.");
        }

        // --- Якщо dry-run: перевірка завершена, симуляцію не запускаємо ---
        if (dryRun) {
            if (hasProgram) {
                Logger::instance().info(
                    "CLI",
                    "[elsim] Dry-run successful: configuration is valid, Simulator constructed and program loaded.");
            } else {
                Logger::instance().info(
                    "CLI", "[elsim] Dry-run successful: configuration is valid and Simulator constructed.");
            }
            return 0;
        }

        // --- Звичайний режим: запускаємо симуляцію ---
        sim.start();

        Logger::instance().info("CLI",
                                "[elsim] Simulation finished. Total cycles: " + std::to_string(sim.cycleCount()));
        return 0;

    } catch (const std::exception& ex) {
        Logger::instance().error("CLI", std::string("[elsim] Simulation failed: ") + ex.what());
        std::cerr << "Simulation failed: " << ex.what() << "\n";
        return 1;
    } catch (...) {
        Logger::instance().error("CLI", "[elsim] Simulation failed: unknown error.");
        std::cerr << "Simulation failed: unknown error\n";
        return 1;
    }
}
