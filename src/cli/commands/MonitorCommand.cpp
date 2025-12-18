#include "MonitorCommand.hpp"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string_view>
#include <thread>

#include "../monitor/MonitorRenderers.hpp"
#include "../monitor/MonitorSnapshot.hpp"
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

enum class MonitorFormat { Text, Json };

void printUsage() {
    std::cerr << "Usage:\n";
    std::cerr << "  elsim monitor --config <path> [--program <path>] [--interval-ms <N>] [--steps <K>] [--once] "
                 "[--format <text|json>]\n";
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

bool parseFormat(std::string_view s, MonitorFormat& out) {
    if (s == "text") {
        out = MonitorFormat::Text;
        return true;
    }
    if (s == "json") {
        out = MonitorFormat::Json;
        return true;
    }
    return false;
}

elsim::cli::monitor::MonitorSnapshot makeSnapshot(const elsim::core::BoardDescription& board,
                                                  const elsim::core::Simulator& sim) {
    elsim::cli::monitor::MonitorSnapshot s{};
    s.board_name = board.name;

    if (auto gpio = sim.gpioController()) {
        s.gpio_dir = static_cast<std::uint32_t>(gpio->getDirectionMask());
        s.gpio_in = static_cast<std::uint32_t>(gpio->getInputMask());
        s.gpio_out = static_cast<std::uint32_t>(gpio->getOutputMask());
    } else {
        s.gpio_dir = 0;
        s.gpio_in = 0;
        s.gpio_out = 0;
    }

    auto leds = sim.ledDevices();
    s.leds.reserve(leds.size());
    for (const auto* led : leds) {
        elsim::cli::monitor::LedSnapshot ls{};
        ls.name = led->name();
        ls.pin = static_cast<std::uint32_t>(led->pin());
        ls.active_high = led->activeHigh();
        ls.is_on = led->isOn();
        s.leds.push_back(std::move(ls));
    }

    return s;
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
    std::cout << "  elsim monitor --config <path> [--program <path>] [--interval-ms <N>] [--steps <K>] [--once] "
                 "[--format <text|json>]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --config <path>         Required. Path to board YAML config.\n";
    std::cout << "  --program <path>        Optional. Path to .elsim-bin program.\n";
    std::cout << "  --once                  Optional. Print one snapshot and exit.\n";
    std::cout << "  --interval-ms <N>       Optional. Refresh interval in ms (default: 200).\n";
    std::cout << "  --steps <K>             Optional. CPU steps between refreshes (default: 0).\n";
    std::cout << "  --format <text|json>    Optional. Output format (default: text).\n";
    std::cout << "  --help                  Show this help.\n";
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
    MonitorFormat format = MonitorFormat::Text;

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
        } else if (a == "--format") {
            if (i + 1 >= args.size()) {
                std::cerr << "Missing value for --format\n";
                printUsage();
                return kExitUsageError;
            }
            MonitorFormat f{};
            if (!parseFormat(args[++i], f)) {
                std::cerr << "Invalid value for --format (expected: text|json)\n";
                return kExitUsageError;
            }
            format = f;
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

        auto renderOnce = [&](const elsim::core::BoardDescription& b, const elsim::core::Simulator& s) {
            const auto snap = makeSnapshot(b, s);

            if (format == MonitorFormat::Json) {
                if (once) {
                    // Human-friendly pretty JSON for one-shot mode.
                    std::cout << elsim::cli::monitor::RenderMonitorJson(snap);
                } else {
                    // Stream-friendly NDJSON (single line per snapshot).
                    std::cout << elsim::cli::monitor::RenderMonitorNdjson(snap) << "\n";
                }
            } else {
                std::cout << elsim::cli::monitor::RenderMonitorText(snap);
            }
        };

        // One-shot mode.
        if (once) {
            runSteps(sim, steps);
            renderOnce(board, sim);
            return kExitSuccess;
        }

        // Live mode.
        while (true) {
            runSteps(sim, steps);
            renderOnce(board, sim);

            // Ensure streaming consumers see data immediately.
            std::cout.flush();

            // Stop if CPU halted (prevents infinite loop on HALT programs).
            if (auto* cpu = sim.cpu(); cpu && cpu->isHalted()) {
                return kExitSuccess;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));

            // Stable separator between frames only in text mode (keeps JSON stream clean).
            if (format == MonitorFormat::Text) {
                std::cout << "\n";
                std::cout.flush();
            }
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
