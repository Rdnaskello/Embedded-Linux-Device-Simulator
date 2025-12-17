#include "CliApp.hpp"

#include <iostream>
#include <string_view>

#include "commands/HelpCommand.hpp"
#include "commands/ListBoardsCommand.hpp"
#include "commands/MonitorCommand.hpp"
#include "commands/RunCommand.hpp"

namespace elsim::cli {

namespace {

bool isFlagLike(std::string_view s) { return !s.empty() && s[0] == '-'; }

}  // namespace

int CliApp::run(const std::vector<std::string>& args) {
    // args[0] = program name
    if (args.size() <= 1) {
        // No command/args -> help (success)
        HelpCommand help;
        return help.execute({});
    }

    std::string_view first = args[1];

    // Aliases for help
    if (first == "--help" || first == "-h") {
        HelpCommand help;
        return help.execute({});
    }

    // Backward-compat: "elsim --config ..." treated as "elsim run --config ..."
    if (isFlagLike(first)) {
        RunCommand cmd;
        std::vector<std::string> subargs(args.begin() + 1, args.end());
        return cmd.execute(subargs);
    }

    // Normal subcommands
    if (first == "run") {
        RunCommand cmd;
        std::vector<std::string> subargs(args.begin() + 2, args.end());
        return cmd.execute(subargs);
    }

    if (first == "list-boards") {
        ListBoardsCommand cmd;
        std::vector<std::string> subargs(args.begin() + 2, args.end());
        return cmd.execute(subargs);
    }

    if (first == "help") {
        HelpCommand cmd;
        std::vector<std::string> subargs(args.begin() + 2, args.end());  // may contain command name
        return cmd.execute(subargs);
    }

    if (first == "monitor") {
        MonitorCommand cmd;
        std::vector<std::string> subargs(args.begin() + 2, args.end());
        return cmd.execute(subargs);
    }

    // Unknown command
    std::cerr << "Unknown command: " << first << "\n";
    std::cerr << "Run: elsim help\n";
    return 1;
}

int CliApp::runHelp(const std::vector<std::string>& /*args*/) {
    HelpCommand help;
    return help.execute({});
}

}  // namespace elsim::cli
