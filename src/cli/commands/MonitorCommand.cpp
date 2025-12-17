#include "MonitorCommand.hpp"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>
#include <thread>

#include "elsim/core/BoardConfigParser.hpp"
#include "elsim/core/BoardDescription.hpp"
#include "elsim/core/Logger.hpp"
#include "elsim/core/ProgramLoader.hpp"
#include "elsim/core/Simulator.hpp"
#include "elsim/device/VirtualLedDevice.hpp"

namespace fs = std::filesystem;

namespace elsim::cli {

namespace {

constexpr int kExitSuccess = 0;
constexpr int kExitUsageError = 1;
constexpr int kExitRuntimeError = 2;

void printUsage() {
    std::cerr << "Usage:\n";
    std::cerr << "  elsim monitor --config <path> [--program <path>] [--interval-ms <N>] [--steps <K>] [--once]\n";
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

void dumpHex32(std::string_view label, std::uint32_t value) {
    std::cout << "  " << label << " = 0x" << std::hex << std::setw(8) << std::setfill('0') << value << std::dec << "\n";
}

void dumpState(const elsim::core::BoardDescription& board, const elsim::core::Simulator& sim) {
    std::cout << "[MONITOR] Board: " << board.name << "\n";

    auto gpio = sim.gpioController();
    if (gpio) {
        std::cout << "GPIO:\n";
        const std::uint32_t dir = static_cast<std::uint32_t>(gpio->getDirectionMask());
        const std::uint32_t in = static_cast<std::uint32_t>(gpio->getInputMask());
        const std::uint32_t out = static_cast<std::uint32_t>(gpio->getOutputMask());

        dumpHex32("DIR     ", dir);
        dumpHex32("DATA_IN ", in);
        dumpHex32("DATA_OUT", out);
    } else {
        std::cout << "GPIO:\n";
        std::cout << "  (not present)\n";
    }

    auto leds = sim.ledDevices();
    std::cout << "\nLEDs:\n";
    if (leds.empty()) {
        std::cout << "  (none)\n";
        return;
    }

    for (const auto* led : leds) {
        std::cout << "  " << led->name() << " = " << (led->isOn() ? "ON" : "OFF") << "  (pin=" << led->pin()
                  << ", active_high=" << (led->activeHigh() ? "true" : "false") << ")\n";
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

}  // namespace

void MonitorCommand::printHelp() {
    std::cout << "elsim monitor\n\n";
    std::cout << "Shows GPIO/LED state (one-shot or periodically).\n\n";
    std::cout << "Usage:\n";
    std::cout << "  elsim monitor --config <path> [--program <path>] [--interval-ms <N>] [--steps <K>] [--once]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --config <path>       Required. Path to board YAML config.\n";
    std::cout << "  --program <path>      Optional. Path to .elsim-bin program.\n";
    std::cout << "  --once                Optional. Print one snapshot and exit.\n";
    std::cout << "  --interval-ms <N>     Optional. Refresh interval in ms (default: 200).\n";
    std::cout << "  --steps <K>           Optional. CPU steps between refreshes (default: 0).\n";
    std::cout << "  --help                Show this help.\n";
}

int MonitorCommand::execute(const std::vector<std::string>& args) {
    // Allow: "elsim monitor --help"
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

    bool once = false;
    std::uint64_t intervalMs = 200;
    std::uint64_t steps = 0;

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
        } else if (a == "--once") {
            once = true;
        } else if (a == "--interval-ms") {
            if (i + 1 >= args.size()) {
                std::cerr << "Missing value for --interval-ms\n";
                printUsage();
                return kExitUsageError;
            }
            std::uint64_t v = 0;
            if (!parseU64(args[++i], v)) {
                std::cerr << "Invalid value for --interval-ms (expected integer)\n";
                return kExitUsageError;
            }
            intervalMs = v;
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

    if (!fs::exists(configPath)) {
        std::cerr << "Config file not found: " << configPath << "\n";
        return kExitRuntimeError;
    }

    if (hasProgram && !fs::exists(programPath)) {
        std::cerr << "Program file not found: " << programPath << "\n";
        return kExitRuntimeError;
    }

    // Guard against busy-looping in live mode.
    if (!once && intervalMs == 0) {
        std::cerr << "--interval-ms must be > 0 in live mode\n";
        return kExitUsageError;
    }

    try {
        auto board = elsim::core::BoardConfigParser::loadFromFile(configPath.string());

        // Keep output clean in monitor mode (also suppress LED toggle logs).
        ScopedLogLevel _logOff(elsim::core::LogLevel::Off);

        // Keep Simulator output clean as well.
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

        // One-shot mode.
        if (once) {
            runSteps(sim, steps);
            dumpState(board, sim);
            return kExitSuccess;
        }

        // Live mode.
        while (true) {
            runSteps(sim, steps);
            dumpState(board, sim);

            // Stop if CPU halted (prevents infinite loop on HALT programs).
            if (auto* cpu = sim.cpu(); cpu && cpu->isHalted()) {
                return kExitSuccess;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
            std::cout << "\n";
        }

    } catch (const elsim::core::BoardConfigException& ex) {
        std::cerr << "Invalid board configuration in '" << configPath << "': " << ex.what() << "\n";
        return kExitRuntimeError;
    } catch (const std::exception& ex) {
        std::cerr << "monitor failed: " << ex.what() << "\n";
        return kExitRuntimeError;
    }
}

}  // namespace elsim::cli
