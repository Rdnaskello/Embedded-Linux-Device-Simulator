#include "MonitorCommand.hpp"

#include <iostream>
#include <string_view>

namespace elsim::cli {

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
    for (std::size_t i = 0; i < args.size(); ++i) {
        std::string_view a = args[i];
        if (a == "--help" || a == "-h") {
            printHelp();
            return 0;
        }
    }

    std::cerr << "monitor: not implemented yet\n";
    std::cerr << "Run: elsim help monitor\n";
    return 2;
}

}  // namespace elsim::cli
