#include "ListBoardsCommand.hpp"

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

namespace elsim::cli {

namespace {

constexpr int kExitSuccess = 0;
constexpr int kExitUsageError = 1;
constexpr int kExitRuntimeError = 2;

struct BoardEntry {
    std::string file;
    std::string name;
    std::string description;
};

bool hasYamlExtension(const fs::path& p) {
    auto ext = p.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext == ".yaml" || ext == ".yml";
}

std::optional<std::string> tryGetScalar(const YAML::Node& node) {
    if (!node || !node.IsScalar()) {
        return std::nullopt;
    }
    return node.as<std::string>();
}

std::optional<BoardEntry> readBoardEntry(const fs::path& yamlPath) {
    YAML::Node root = YAML::LoadFile(yamlPath.string());
    YAML::Node b = root["board"];
    if (!b) {
        return std::nullopt;
    }

    auto name = tryGetScalar(b["name"]);
    auto desc = tryGetScalar(b["description"]);

    BoardEntry e;
    e.file = yamlPath.filename().string();
    e.name = name.value_or("-");
    e.description = desc.value_or("-");
    return e;
}

void printUsage() {
    std::cerr << "Usage:\n";
    std::cerr << "  elsim list-boards [--path <dir>] [--recursive] [--all]\n";
}

bool isTestLikeFilename(const std::string& filename) {
    return filename.rfind("invalid-", 0) == 0 || filename == "device-factory-test.yaml";
}

}  // namespace

void ListBoardsCommand::printHelp() {
    std::cout << "elsim list-boards\n\n";
    std::cout << "Lists board YAML files in a directory.\n\n";
    std::cout << "Usage:\n";
    std::cout << "  elsim list-boards [--path <dir>] [--recursive] [--all]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --path <dir>     Optional. Directory to scan (default: examples/board-examples).\n";
    std::cout << "  --recursive      Optional. Scan subdirectories recursively.\n";
    std::cout << "  --all            Optional. Include test/invalid YAML files too.\n";
    std::cout << "  --help           Show this help.\n";
}

int ListBoardsCommand::execute(const std::vector<std::string>& args) {
    fs::path dir = fs::path{"examples/board-examples"};
    bool recursive = false;
    bool showAll = false;

    for (std::size_t i = 0; i < args.size(); ++i) {
        std::string_view a = args[i];

        if (a == "--help" || a == "-h") {
            printHelp();
            return kExitSuccess;
        } else if (a == "--path") {
            if (i + 1 >= args.size()) {
                std::cerr << "Missing value for --path\n";
                printUsage();
                return kExitUsageError;
            }
            dir = fs::path{args[++i]};
        } else if (a == "--recursive") {
            recursive = true;
        } else if (a == "--all") {
            showAll = true;
        } else {
            std::cerr << "Unknown argument: " << a << "\n";
            printUsage();
            return kExitUsageError;
        }
    }

    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        std::cerr << "Directory not found: " << dir.string() << "\n";
        return kExitRuntimeError;
    }

    std::vector<BoardEntry> entries;

    try {
        if (recursive) {
            for (const auto& it : fs::recursive_directory_iterator(dir)) {
                if (!it.is_regular_file()) {
                    continue;
                }
                const auto& p = it.path();
                if (!hasYamlExtension(p)) {
                    continue;
                }

                auto e = readBoardEntry(p);
                if (!e) {
                    if (showAll) {
                        entries.push_back({p.filename().string(), "-", "-"});
                    }
                    continue;
                }

                // NEW: hide test/invalid YAML by default
                if (!showAll && isTestLikeFilename(e->file)) {
                    continue;
                }

                // Default filter: only "examples" that have both name+description
                if (!showAll) {
                    if (e->name == "-" || e->description == "-") {
                        continue;
                    }
                }

                entries.push_back(*e);
            }
        } else {
            for (const auto& it : fs::directory_iterator(dir)) {
                if (!it.is_regular_file()) {
                    continue;
                }
                const auto& p = it.path();
                if (!hasYamlExtension(p)) {
                    continue;
                }

                auto e = readBoardEntry(p);
                if (!e) {
                    if (showAll) {
                        entries.push_back({p.filename().string(), "-", "-"});
                    }
                    continue;
                }

                // NEW: hide test/invalid YAML by default
                if (!showAll && isTestLikeFilename(e->file)) {
                    continue;
                }

                if (!showAll) {
                    if (e->name == "-" || e->description == "-") {
                        continue;
                    }
                }

                entries.push_back(*e);
            }
        }
    } catch (const YAML::Exception& ex) {
        std::cerr << "Failed to parse YAML while listing boards: " << ex.what() << "\n";
        return kExitRuntimeError;
    } catch (const std::exception& ex) {
        std::cerr << "Failed to list boards: " << ex.what() << "\n";
        return kExitRuntimeError;
    }

    std::sort(entries.begin(), entries.end(), [](const BoardEntry& a, const BoardEntry& b) { return a.file < b.file; });

    if (entries.empty()) {
        std::cout << "(no boards found)\n";
        return kExitSuccess;
    }

    for (const auto& e : entries) {
        std::cout << e.file << " | " << e.name << " | " << e.description << "\n";
    }

    return kExitSuccess;
}

}  // namespace elsim::cli
