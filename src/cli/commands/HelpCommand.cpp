#include "HelpCommand.hpp"

#include <iostream>
#include <string_view>

namespace elsim::cli {

namespace {

void printGeneralHelp() {
    std::cout << "elsim\n\n";
    std::cout << "Usage:\n";
    std::cout << "  elsim <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  run         Start simulator (legacy mode).\n";
    std::cout << "  list-boards List available example board YAML files.\n";
    std::cout << "  help        Show help (general or per-command).\n\n";
    std::cout << "Aliases:\n";
    std::cout << "  elsim --help            == elsim help\n";
    std::cout << "  elsim --config ...      == elsim run --config ...   (backward-compatible)\n\n";
    std::cout << "Examples:\n";
    std::cout << "  elsim help\n";
    std::cout << "  elsim help run\n";
    std::cout << "  elsim run --config examples/board-examples/minimal-board.yaml --dry-run\n";
    std::cout << "  elsim list-boards --path examples/board-examples\n";
    std::cout << "  press       Press a virtual button (inject GPIO input).\n";
    std::cout << "  monitor     Watch GPIO/LED state (one-shot or periodic).\n";
}

void printRunHelp() {
    // keep in sync with RunCommand::printHelp()
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

void printListBoardsHelp() {
    std::cout << "elsim list-boards\n\n";
    std::cout << "Lists board YAML files in a directory.\n\n";
    std::cout << "Usage:\n";
    std::cout << "  elsim list-boards [--path <dir>] [--recursive]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --path <dir>     Optional. Directory to scan (default: examples/board-examples).\n";
    std::cout << "  --recursive      Optional. Scan subdirectories recursively.\n";
    std::cout << "  --help           Show this help.\n";
    std::cout << "  elsim list-boards [--path <dir>] [--recursive] [--all]\n\n";
    std::cout << "  --all            Optional. Include test/invalid YAML files too.\n";
}

void printHelpHelp() {
    std::cout << "elsim help\n\n";
    std::cout << "Usage:\n";
    std::cout << "  elsim help\n";
    std::cout << "  elsim help <command>\n\n";
    std::cout << "Commands:\n";
    std::cout << "  run\n";
    std::cout << "  list-boards\n";
    std::cout << "  help\n";
    std::cout << "  monitor\n";
    std::cout << "  press\n";
}

}  // namespace

int HelpCommand::execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        printGeneralHelp();
        return 0;
    }

    std::string_view cmd = args[0];

    if (cmd == "run") {
        printRunHelp();
        return 0;
    }
    if (cmd == "list-boards") {
        printListBoardsHelp();
        return 0;
    }
    if (cmd == "help") {
        printHelpHelp();
        return 0;
    }
    if (cmd == "monitor") {
        std::cout << "elsim monitor\n\n";
        std::cout << "Shows GPIO/LED state (one-shot or periodically).\n\n";
        std::cout << "Usage:\n";
        std::cout << "  elsim monitor --config <path> [--program <path>] [--interval-ms <N>] [--steps <K>] [--once] "
                     "[--format <text|json>]\n\n";
        std::cout << "Run: elsim monitor --help\n";
        return 0;
    }
    if (cmd == "press") {
        std::cout << "elsim press\n\n";
        std::cout << "Presses a virtual button from board.yaml during simulation.\n\n";
        std::cout << "Usage:\n";
        std::cout << "  elsim press --config <path> --button <name> [--program <path>] [--hold-ms <N>] [--steps <K>] "
                     "[--repeat <R>]\n\n";
        std::cout << "Run: elsim press --help\n";
        return 0;
    }

    std::cerr << "Unknown command for help: " << cmd << "\n";
    std::cerr << "Run: elsim help\n";
    return 1;
}

}  // namespace elsim::cli
