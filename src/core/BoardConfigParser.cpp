#include "elsim/core/BoardConfigParser.hpp"

#include <yaml-cpp/yaml.h>

#include <sstream>
#include <utility>

namespace elsim::core {

// ===== BoardConfigException =====

BoardConfigException::BoardConfigException(BoardConfigErrorCode code, const std::string& message)
    : std::runtime_error(message), code_(code) {}

namespace {

// Допоміжна функція для формування шляху до поля (для кращих повідомлень про помилки).
std::string makePath(const std::string& base, const std::string& key) {
    if (base.empty()) {
        return key;
    }
    return base + "." + key;
}

[[noreturn]] void throwMissingField(const std::string& path) {
    throw BoardConfigException(BoardConfigErrorCode::MissingField, "Missing required field '" + path + "'");
}

[[noreturn]] void throwInvalidType(const std::string& path, const std::string& expected) {
    throw BoardConfigException(BoardConfigErrorCode::InvalidType,
                               "Field '" + path + "' has invalid type. Expected " + expected + ".");
}

[[noreturn]] void throwInvalidValue(const std::string& path, const std::string& reason) {
    throw BoardConfigException(BoardConfigErrorCode::InvalidValue, "Field '" + path + "' has invalid value: " + reason);
}

// Взяти обов'язкове поле з мапи YAML.
YAML::Node requireField(const YAML::Node& parent, const std::string& key, const std::string& parentPath) {
    auto node = parent[key];
    if (!node) {
        throwMissingField(makePath(parentPath, key));
    }
    return node;
}

// Прочитати скаляр певного типу з перевіркою.
template <typename T>
T readScalar(const YAML::Node& node, const std::string& path) {
    if (!node.IsScalar()) {
        throwInvalidType(path, "scalar");
    }

    try {
        return node.as<T>();
    } catch (const YAML::TypedBadConversion<T>&) {
        throwInvalidType(path, "convertible scalar");
    }
}

// Парсинг MemoryType з рядка.
MemoryType parseMemoryType(const YAML::Node& node, const std::string& path) {
    auto value = readScalar<std::string>(node, path);

    if (value == "ram" || value == "RAM") {
        return MemoryType::Ram;
    }
    if (value == "rom" || value == "ROM" || value == "flash") {
        return MemoryType::Rom;
    }
    if (value == "mmio" || value == "MMIO") {
        return MemoryType::Mmio;
    }

    throwInvalidValue(path, "unsupported memory type '" + value + "'");
}

// ===== Парсинг секцій =====

CpuDescription parseCpu(const YAML::Node& root) {
    const std::string sectionPath = "cpu";

    auto cpuNode = requireField(root, "cpu", "");
    if (!cpuNode.IsMap()) {
        throwInvalidType(sectionPath, "map");
    }

    CpuDescription cpu{};

    // cpu.type
    {
        auto typeNode = requireField(cpuNode, "type", sectionPath);
        cpu.type = readScalar<std::string>(typeNode, makePath(sectionPath, "type"));
        if (cpu.type.empty()) {
            throwInvalidValue(makePath(sectionPath, "type"), "must not be empty");
        }
    }

    // cpu.frequency_hz
    {
        auto freqNode = requireField(cpuNode, "frequency_hz", sectionPath);
        cpu.frequencyHz = readScalar<std::uint64_t>(freqNode, makePath(sectionPath, "frequency_hz"));
        if (cpu.frequencyHz == 0) {
            throwInvalidValue(makePath(sectionPath, "frequency_hz"), "must be > 0");
        }
    }

    // cpu.endianness (опційне, за замовчуванням "little")
    {
        auto endianNode = cpuNode["endianness"];
        if (endianNode) {
            cpu.endianness = readScalar<std::string>(endianNode, makePath(sectionPath, "endianness"));
        } else {
            cpu.endianness = "little";
        }
    }

    return cpu;
}

std::vector<MemoryRegion> parseMemory(const YAML::Node& root) {
    const std::string sectionPath = "memory";

    auto memNode = requireField(root, "memory", "");
    if (!memNode.IsSequence()) {
        throwInvalidType(sectionPath, "sequence");
    }

    std::vector<MemoryRegion> regions;
    regions.reserve(memNode.size());

    for (std::size_t i = 0; i < memNode.size(); ++i) {
        const auto& item = memNode[i];
        std::ostringstream path;
        path << sectionPath << "[" << i << "]";

        if (!item.IsMap()) {
            throwInvalidType(path.str(), "map");
        }

        MemoryRegion region{};

        // name
        {
            auto nameNode = requireField(item, "name", path.str());
            region.name = readScalar<std::string>(nameNode, makePath(path.str(), "name"));
            if (region.name.empty()) {
                throwInvalidValue(makePath(path.str(), "name"), "must not be empty");
            }
        }

        // type
        {
            auto typeNode = requireField(item, "type", path.str());
            region.type = parseMemoryType(typeNode, makePath(path.str(), "type"));
        }

        // base
        {
            auto baseNode = requireField(item, "base", path.str());
            region.baseAddress = readScalar<std::uint64_t>(baseNode, makePath(path.str(), "base"));
        }

        // size
        {
            auto sizeNode = requireField(item, "size", path.str());
            region.sizeBytes = readScalar<std::uint64_t>(sizeNode, makePath(path.str(), "size"));
            if (region.sizeBytes == 0) {
                throwInvalidValue(makePath(path.str(), "size"), "must be > 0");
            }
        }

        regions.push_back(region);
    }

    if (regions.empty()) {
        throwInvalidValue(sectionPath, "at least one memory region is required");
    }

    return regions;
}

std::vector<DeviceDescription> parseDevices(const YAML::Node& root) {
    const std::string sectionPath = "devices";

    auto devNode = root["devices"];
    if (!devNode) {
        // devices може бути відсутній → просто порожній список
        return {};
    }

    if (!devNode.IsSequence()) {
        throwInvalidType(sectionPath, "sequence");
    }

    std::vector<DeviceDescription> devices;
    devices.reserve(devNode.size());

    std::set<std::string> deviceNames;

    for (std::size_t i = 0; i < devNode.size(); ++i) {
        const auto& item = devNode[i];
        std::ostringstream path;
        path << sectionPath << "[" << i << "]";

        if (!item.IsMap()) {
            throwInvalidType(path.str(), "map");
        }

        DeviceDescription dev{};

        // type
        {
            auto typeNode = requireField(item, "type", path.str());
            dev.type = readScalar<std::string>(typeNode, makePath(path.str(), "type"));
            if (dev.type.empty()) {
                throwInvalidValue(makePath(path.str(), "type"), "must not be empty");
            }
        }

        // name
        {
            auto nameNode = requireField(item, "name", path.str());
            dev.name = readScalar<std::string>(nameNode, makePath(path.str(), "name"));
            if (dev.name.empty()) {
                throwInvalidValue(makePath(path.str(), "name"), "must not be empty");
            }

            if (!deviceNames.insert(dev.name).second) {
                throw BoardConfigException(BoardConfigErrorCode::DuplicateName,
                                           "Duplicate device name '" + dev.name + "' in devices section");
            }
        }

        // base
        {
            auto baseNode = requireField(item, "base", path.str());
            dev.baseAddress = readScalar<std::uint64_t>(baseNode, makePath(path.str(), "base"));
        }

        // params (опційно, мапа)
        {
            auto paramsNode = item["params"];
            if (paramsNode) {
                if (!paramsNode.IsMap()) {
                    throwInvalidType(makePath(path.str(), "params"), "map");
                }

                for (auto it = paramsNode.begin(); it != paramsNode.end(); ++it) {
                    const auto& keyNode = it->first;
                    const auto& valueNode = it->second;

                    auto keyPath = makePath(path.str() + ".params", keyNode.Scalar());
                    auto key = readScalar<std::string>(keyNode, keyPath);
                    auto value = readScalar<std::string>(valueNode, keyPath);

                    dev.params.emplace(std::move(key), std::move(value));
                }
            }
        }

        devices.push_back(std::move(dev));
    }

    return devices;
}

BoardDescription parseBoard(const YAML::Node& root) {
    BoardDescription board{};

    // name
    {
        auto nameNode = requireField(root, "name", "");
        board.name = readScalar<std::string>(nameNode, "name");
        if (board.name.empty()) {
            throwInvalidValue("name", "must not be empty");
        }
    }

    // description (опційне)
    {
        auto descNode = root["description"];
        if (descNode) {
            board.description = readScalar<std::string>(descNode, "description");
        } else {
            board.description.clear();
        }
    }

    // version (опційне, на майбутнє)
    {
        auto versionNode = root["version"];
        if (versionNode) {
            auto version = readScalar<int>(versionNode, "version");
            if (version != 1) {
                throw BoardConfigException(BoardConfigErrorCode::UnsupportedVersion,
                                           "Unsupported board configuration version: " + std::to_string(version));
            }
        }
        // якщо версії немає – вважаємо, що це версія 1
    }

    board.cpu = parseCpu(root);
    board.memory = parseMemory(root);
    board.devices = parseDevices(root);

    return board;
}

}  // namespace

// ===== Публічний інтерфейс =====

BoardDescription BoardConfigParser::loadFromFile(const std::string& path) {
    // 1. Завантажити YAML-файл
    YAML::Node root = YAML::LoadFile(path);

    // Дозволяємо два варіанти:
    //  - кореневий об'єкт одразу містить поля name/cpu/memory/...
    //  - або вкладений під ключем "board"
    YAML::Node boardNode;
    if (root["board"]) {
        boardNode = root["board"];
    } else {
        boardNode = root;
    }

    if (!boardNode.IsMap()) {
        throwInvalidType("board", "map");
    }

    // 2. Розпарсити та провалідувати структуру
    return parseBoard(boardNode);
}

}  // namespace elsim::core
