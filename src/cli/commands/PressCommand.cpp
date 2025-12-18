#include "PressCommand.hpp"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string_view>
#include <thread>

#include "elsim/core/BoardConfigParser.hpp"
#include "elsim/core/BoardDescription.hpp"
#include "elsim/core/Logger.hpp"
#include "elsim/core/ProgramLoader.hpp"
#include "elsim/core/Simulator.hpp"
#include "elsim/device/VirtualButtonDevice.hpp"

namespace fs = std::filesystem;

namespace elsim::cli {

namespace {

constexpr int kExitSuccess = 0;
constexpr int kExitUsageError = 1;
constexpr int kExitRuntimeError = 2;

void printUsage() {
    std::cerr << "Usage:\n";
    std::cerr << "  elsim press --config <path> --button <name> [--program <path>] [--hold-ms <N>] [--steps <K>] "
                 "[--repeat <R>]\n";
}

bool parseU64(std::string_view s, std::uint64_t& out) {
    try {
        std::size_t pos = 0;
        unsigned long long v = std::stoull(std::string{s}, &pos, 0);
        if (pos != s.size()) {
            return false;
        }
        out = static_cast<std::uint64_t>(v);
        return true;
    } catch (...) {
        return false;
    }
}

void runSteps(elsim::core::Simulator& sim, std::uint64_t steps) {
    if (steps == 0) {
        return;
    }
    for (std::uint64_t i = 0; i < steps; ++i) {
        sim.runOneTick();
        auto* cpu = sim.cpu();
        if (cpu && cpu->isHalted()) {
            break;
        }
    }
}

// RAII: temporarily set logger level, then restore.
class ScopedLogLevel final {
   public:
    explicit ScopedLogLevel(elsim::core::LogLevel level)
        : logger_(elsim::core::Logger::instance()), prev_(logger_.level()) {
        logger_.set_level(level);
    }

    ~ScopedLogLevel() { logger_.set_level(prev_); }

    ScopedLogLevel(const ScopedLogLevel&) = delete;
    ScopedLogLevel& operator=(const ScopedLogLevel&) = delete;

   private:
    elsim::core::Logger& logger_;
    elsim::core::LogLevel prev_;
};

elsim::VirtualButtonDevice* findButtonByName(elsim::core::Simulator& sim, std::string_view name) {
    auto buttons = sim.buttonDevices();
    for (auto* b : buttons) {
        if (b && b->name() == name) {
            return b;
        }
    }
    return nullptr;
}

void printAvailableButtons(elsim::core::Simulator& sim) {
    auto buttons = sim.buttonDevices();
    if (buttons.empty()) {
        std::cerr << "Available buttons: (none)\n";
        return;
    }
    std::cerr << "Available buttons:\n";
    for (const auto* b : buttons) {
        if (!b) continue;
        std::cerr << "  - " << b->name() << "\n";
    }
}

}  // namespace

void PressCommand::printHelp() {
    std::cout << "elsim press\n\n";
    std::cout << "Presses a virtual button from board.yaml during simulation.\n\n";
    std::cout << "Usage:\n";
    std::cout << "  elsim press --config <path> --button <name> [--program <path>] [--hold-ms <N>] [--steps <K>] "
                 "[--repeat <R>]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --config <path>       Required. Path to board YAML config.\n";
    std::cout << "  --button <name>       Required. Button name (devices[].name).\n";
    std::cout << "  --program <path>      Optional. Path to .elsim-bin program.\n";
    std::cout << "  --hold-ms <N>         Optional. Hold duration in ms (default: 100).\n";
    std::cout << "  --steps <K>           Optional. CPU steps after press and after release (default: 100).\n";
    std::cout << "  --repeat <R>          Optional. Repeat press R times (default: 1).\n";
    std::cout << "  --help                Show this help.\n";
}

int PressCommand::execute(const std::vector<std::string>& args) {
    // Allow: "elsim press --help"
    for (std::size_t i = 0; i < args.size(); ++i) {
        std::string_view a = args[i];
        if (a == "--help" || a == "-h") {
            printHelp();
            return kExitSuccess;
        }
    }

    fs::path configPath;
    bool hasConfig = false;

    fs::path programPath;
    bool hasProgram = false;

    std::string buttonName;
    bool hasButton = false;

    std::uint64_t holdMs = 100;
    std::uint64_t steps = 100;
    std::uint64_t repeat = 1;

    // Parse args
    for (std::size_t i = 0; i < args.size(); ++i) {
        std::string_view a = args[i];

        if (a == "--config") {
            if (i + 1 >= args.size()) {
                std::cerr << "Missing value for --config\n";
                printUsage();
                return kExitUsageError;
            }
            configPath = fs::path{args[++i]};
            hasConfig = true;
        } else if (a == "--program") {
            if (i + 1 >= args.size()) {
                std::cerr << "Missing value for --program\n";
                printUsage();
                return kExitUsageError;
            }
            programPath = fs::path{args[++i]};
            hasProgram = true;
        } else if (a == "--button") {
            if (i + 1 >= args.size()) {
                std::cerr << "Missing value for --button\n";
                printUsage();
                return kExitUsageError;
            }
            buttonName = args[++i];
            hasButton = true;
        } else if (a == "--hold-ms") {
            if (i + 1 >= args.size()) {
                std::cerr << "Missing value for --hold-ms\n";
                printUsage();
                return kExitUsageError;
            }
            std::uint64_t v = 0;
            if (!parseU64(args[++i], v)) {
                std::cerr << "Invalid value for --hold-ms (expected integer)\n";
                return kExitUsageError;
            }
            holdMs = v;
        } else if (a == "--steps") {
            if (i + 1 >= args.size()) {
                std::cerr << "Missing value for --steps\n";
                printUsage();
                return kExitUsageError;
            }
            std::uint64_t v = 0;
            if (!parseU64(args[++i], v)) {
                std::cerr << "Invalid value for --steps (expected integer)\n";
                return kExitUsageError;
            }
            steps = v;
        } else if (a == "--repeat") {
            if (i + 1 >= args.size()) {
                std::cerr << "Missing value for --repeat\n";
                printUsage();
                return kExitUsageError;
            }
            std::uint64_t v = 0;
            if (!parseU64(args[++i], v) || v == 0) {
                std::cerr << "Invalid value for --repeat (expected integer >= 1)\n";
                return kExitUsageError;
            }
            repeat = v;
        } else {
            std::cerr << "Unknown argument: " << a << "\n";
            printUsage();
            return kExitUsageError;
        }
    }

    if (!hasConfig) {
        std::cerr << "Missing required --config argument\n";
        printUsage();
        return kExitUsageError;
    }
    if (!hasButton) {
        std::cerr << "Missing required --button argument\n";
        printUsage();
        return kExitUsageError;
    }

    if (!fs::exists(configPath)) {
        std::cerr << "Config file not found: " << configPath << "\n";
        return kExitRuntimeError;
    }

    if (hasProgram && !fs::exists(programPath)) {
        std::cerr << "Program file not found: " << programPath << "\n";
        return kExitRuntimeError;
    }

    try {
        auto board = elsim::core::BoardConfigParser::loadFromFile(configPath.string());

        // Keep output clean in press mode.
        ScopedLogLevel _logOff(elsim::core::LogLevel::Off);

        std::ostringstream simLog;
        elsim::core::Simulator sim(simLog);
        sim.loadBoard(board);

        if (hasProgram) {
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
        }

        auto* btn = findButtonByName(sim, buttonName);
        if (!btn) {
            std::cerr << "Button not found: '" << buttonName << "'\n";
            printAvailableButtons(sim);
            return kExitRuntimeError;
        }

        std::cout << "Pressed button '" << btn->name() << "' (hold " << holdMs << "ms, steps " << steps << ", repeat "
                  << repeat << ")\n";

        for (std::uint64_t i = 0; i < repeat; ++i) {
            btn->press();
            runSteps(sim, steps);

            if (holdMs > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(holdMs));
            }

            btn->release();
            runSteps(sim, steps);

            if (auto* cpu = sim.cpu(); cpu && cpu->isHalted()) {
                break;
            }
        }

        return kExitSuccess;

    } catch (const elsim::core::BoardConfigException& ex) {
        std::cerr << "Invalid board configuration in '" << configPath << "': " << ex.what() << "\n";
        return kExitRuntimeError;
    } catch (const std::exception& ex) {
        std::cerr << "press failed: " << ex.what() << "\n";
        return kExitRuntimeError;
    }
}

}  // namespace elsim::cli
