#include "RunCommand.hpp"

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

namespace elsim::cli {

namespace {

using elsim::core::Logger;
using elsim::core::LogLevel;

constexpr int kExitSuccess = 0;
constexpr int kExitUsageError = 1;
constexpr int kExitRuntimeError = 2;

std::optional<LogLevel> parseLogLevel(std::string_view value) {
    if (value == "trace") {
        return LogLevel::Debug;  // alias
    }
    if (value == "debug") {
        return LogLevel::Debug;
    }
    if (value == "info") {
        return LogLevel::Info;
    }
    if (value == "warn") {
        return LogLevel::Warn;
    }
    if (value == "error") {
        return LogLevel::Error;
    }
    if (value == "off") {
        return LogLevel::Off;
    }
    return std::nullopt;
}

// Load & validate board config, throws on failure.
elsim::core::BoardDescription loadBoardConfig(const fs::path& configPath) {
    auto& logger = Logger::instance();
    logger.info("CLI", "[elsim] Using config: " + configPath.string());

    auto board = elsim::core::BoardConfigParser::loadFromFile(configPath.string());
    logger.info("CLI", "[elsim] Board config loaded successfully. Name: " + board.name);
    return board;
}

void printUsage() {
    std::cerr << "Usage:\n";
    std::cerr << "  elsim run --config <path> [--program <path>] [--log-level <trace|debug|info|warn|error|off>] "
                 "[--dry-run]\n";
    std::cerr << "  elsim --config <path> [--program <path>] [--log-level <trace|debug|info|warn|error|off>] "
                 "[--dry-run]   (backward-compatible)\n";
}

}  // namespace

void RunCommand::printHelp() {
    std::cout << "elsim run\n\n";
    std::cout << "Starts the simulator (same as the legacy elsim CLI).\n\n";
    std::cout << "Usage:\n";
    std::cout << "  elsim run --config <path> [--program <path>] [--log-level <trace|debug|info|warn|error|off>] "
                 "[--dry-run]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --config <path>            Required. Path to board YAML config.\n";
    std::cout << "  --program <path>           Optional. Path to .elsim-bin program.\n";
    std::cout
        << "  --log-level <level>        Optional. trace|debug|info|warn|error|off (default: info). trace==debug.\n";
    std::cout << "  --dry-run                  Optional. Validate config/program and construct simulator, but do not "
                 "start.\n";
}

int RunCommand::execute(const std::vector<std::string>& args) {
    fs::path configPath;
    bool hasConfig = false;

    LogLevel logLevel = LogLevel::Info;
    bool dryRun = false;

    fs::path programPath;
    bool hasProgram = false;

    // Allow: "elsim run --help"
    for (std::size_t i = 0; i < args.size(); ++i) {
        std::string_view arg = args[i];
        if (arg == "--help" || arg == "-h") {
            printHelp();
            return kExitSuccess;
        }
    }

    // Parse args
    for (std::size_t i = 0; i < args.size(); ++i) {
        std::string_view arg = args[i];

        if (arg == "--config") {
            if (i + 1 >= args.size()) {
                std::cerr << "Missing value for --config\n";
                printUsage();
                return kExitUsageError;
            }
            configPath = fs::path{args[++i]};
            hasConfig = true;
        } else if (arg == "--log-level") {
            if (i + 1 >= args.size()) {
                std::cerr << "Missing value for --log-level\n";
                printUsage();
                return kExitUsageError;
            }
            std::string_view levelStr = args[++i];
            auto lvl = parseLogLevel(levelStr);
            if (!lvl) {
                std::cerr << "Unknown log level: " << levelStr << " (expected: trace|debug|info|warn|error|off)\n";
                return kExitUsageError;
            }
            logLevel = *lvl;
        } else if (arg == "--dry-run") {
            dryRun = true;
        } else if (arg == "--program") {
            if (i + 1 >= args.size()) {
                std::cerr << "Missing value for --program\n";
                printUsage();
                return kExitUsageError;
            }
            programPath = fs::path{args[++i]};
            hasProgram = true;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            printUsage();
            return kExitUsageError;
        }
    }

    if (!hasConfig) {
        std::cerr << "Missing required --config argument\n";
        printUsage();
        return kExitUsageError;
    }

    // Set log level (only for run; help/list-boards stay clean)
    Logger::instance().set_level(logLevel);
    Logger::instance().info("CLI", "Logger initialized");

    if (!fs::exists(configPath)) {
        Logger::instance().error("CLI", "Config file not found: " + configPath.string());
        std::cerr << "Config file not found: " << configPath << "\n";
        return kExitRuntimeError;
    }

    try {
        // 1) Load + validate config
        auto board = loadBoardConfig(configPath);

        if (dryRun) {
            Logger::instance().info(
                "CLI", "[elsim] Dry-run mode: configuration file will be validated. Simulator will NOT be started.");
        } else {
            Logger::instance().info("CLI", "[elsim] Starting simulation...");
        }

        // 2) Construct simulator and load board
        elsim::core::Simulator sim(std::cout);
        sim.loadBoard(board);

        // 3) Optionally load program
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

            cpu->setPc(entryPoint);
            Logger::instance().info(
                "CLI", "[elsim] Program loaded successfully. Entry point set to " + std::to_string(entryPoint));
        } else {
            Logger::instance().warn("CLI",
                                    "[elsim] No program specified via --program. CPU will start from its reset PC.");
        }

        // 4) Dry-run ends here
        if (dryRun) {
            if (hasProgram) {
                Logger::instance().info(
                    "CLI",
                    "[elsim] Dry-run successful: configuration is valid, Simulator constructed and program loaded.");
            } else {
                Logger::instance().info(
                    "CLI", "[elsim] Dry-run successful: configuration is valid and Simulator constructed.");
            }
            return kExitSuccess;
        }

        // 5) Start simulation
        sim.start();

        Logger::instance().info("CLI",
                                "[elsim] Simulation finished. Total cycles: " + std::to_string(sim.cycleCount()));
        return kExitSuccess;

    } catch (const YAML::Exception& ex) {
        Logger::instance().error(
            "CLI", std::string("[elsim] Failed to parse YAML config '") + configPath.string() + "': " + ex.what());
        std::cerr << "Failed to parse YAML config '" << configPath << "': " << ex.what() << "\n";
        return kExitRuntimeError;
    } catch (const elsim::core::BoardConfigException& ex) {
        Logger::instance().error(
            "CLI", std::string("[elsim] Invalid board configuration in '") + configPath.string() + "': " + ex.what());
        std::cerr << "Invalid board configuration in '" << configPath << "': " << ex.what() << "\n";
        return kExitRuntimeError;
    } catch (const std::exception& ex) {
        Logger::instance().error("CLI", std::string("[elsim] Simulation failed: ") + ex.what());
        std::cerr << "Simulation failed: " << ex.what() << "\n";
        return kExitRuntimeError;
    } catch (...) {
        Logger::instance().error("CLI", "[elsim] Simulation failed: unknown error.");
        std::cerr << "Simulation failed: unknown error\n";
        return kExitRuntimeError;
    }
}

}  // namespace elsim::cli
