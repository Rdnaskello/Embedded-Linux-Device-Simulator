#include "elsim/core/BoardConfigParser.hpp"

#include <yaml-cpp/yaml.h>

#include <cstdint>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

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

[[noreturn]] void throwDuplicateName(const std::string& name, const std::string& where) {
    throw BoardConfigException(BoardConfigErrorCode::DuplicateName,
                               "Duplicate name '" + name + "' in " + where + " section");
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

bool readBoolScalarDefault(const YAML::Node& node, const std::string& path, bool defaultValue) {
    if (!node) {
        return defaultValue;
    }
    auto v = readScalar<std::string>(node, path);
    // Accept a few friendly forms, but keep messages clear.
    if (v == "true" || v == "True" || v == "1" || v == "yes" || v == "on") {
        return true;
    }
    if (v == "false" || v == "False" || v == "0" || v == "no" || v == "off") {
        return false;
    }
    throwInvalidValue(path, "must be boolean (true/false/1/0)");
}

std::uint32_t readU32ScalarDefault(const YAML::Node& node, const std::string& path, std::uint32_t defaultValue) {
    if (!node) {
        return defaultValue;
    }
    auto v64 = readScalar<std::uint64_t>(node, path);
    if (v64 > 0xFFFF'FFFFull) {
        throwInvalidValue(path, "must fit into 32-bit unsigned integer");
    }
    return static_cast<std::uint32_t>(v64);
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

bool hasMmioRegionAtBase(const std::vector<MemoryRegion>& memory, std::uint64_t base) {
    for (const auto& r : memory) {
        if (r.type == MemoryType::Mmio && r.baseAddress == base) {
            return true;
        }
    }
    return false;
}

bool devicesContainGpio(const std::vector<DeviceDescription>& devices) {
    for (const auto& d : devices) {
        if (d.type == "gpio" || d.type == "GPIO") {
            return true;
        }
    }
    return false;
}

std::uint32_t extractPinCountFromGpioDevice(const std::vector<DeviceDescription>& devices) {
    for (const auto& d : devices) {
        if (d.type != "gpio" && d.type != "GPIO") {
            continue;
        }
        auto it = d.params.find("pin_count");
        if (it == d.params.end()) {
            return 32;
        }
        // We keep this parsing simple here; the actual device also validates.
        try {
            unsigned long v = std::stoul(it->second, nullptr, 0);
            if (v == 0 || v > 32) {
                return 32;  // will be rejected elsewhere if needed; parser will validate when using gpio section
            }
            return static_cast<std::uint32_t>(v);
        } catch (...) {
            return 32;
        }
    }
    return 32;
}

DeviceDescription parseGpioSectionOrThrow(const YAML::Node& root, const std::vector<MemoryRegion>& memory,
                                          const std::vector<DeviceDescription>& existingDevices,
                                          std::unordered_set<std::string>& usedNames) {
    const std::string sectionPath = "gpio";

    auto gpioNode = root["gpio"];
    if (!gpioNode) {
        // Not present -> return empty marker (caller will not use)
        return DeviceDescription{};
    }

    if (!gpioNode.IsMap()) {
        throwInvalidType(sectionPath, "map");
    }

    // Do not allow mixing: gpio section + devices gpio (two sources of truth)
    if (devicesContainGpio(existingDevices)) {
        throwInvalidValue(sectionPath, "cannot be used together with devices[].type == 'gpio' (choose one format)");
    }

    // gpio.mmio_base required
    auto baseNode = requireField(gpioNode, "mmio_base", sectionPath);
    const std::uint64_t mmioBase = readScalar<std::uint64_t>(baseNode, makePath(sectionPath, "mmio_base"));

    if (!hasMmioRegionAtBase(memory, mmioBase)) {
        throwInvalidValue(makePath(sectionPath, "mmio_base"),
                          "no matching MMIO memory region found at this base address");
    }

    // gpio.pin_count optional default 32
    const std::uint32_t pinCount = readU32ScalarDefault(gpioNode["pin_count"], makePath(sectionPath, "pin_count"), 32);

    if (pinCount == 0 || pinCount > 32) {
        throwInvalidValue(makePath(sectionPath, "pin_count"), "must be in range 1..32");
    }

    DeviceDescription dev{};
    dev.type = "gpio";
    dev.name = "gpio0";
    dev.baseAddress = mmioBase;
    dev.params.emplace("pin_count", std::to_string(pinCount));

    // global name uniqueness
    if (!usedNames.insert(dev.name).second) {
        throwDuplicateName(dev.name, "board");
    }

    return dev;
}

std::vector<DeviceDescription> parseLedsSection(const YAML::Node& root, std::uint32_t pinCount,
                                                std::unordered_set<std::string>& usedNames,
                                                std::unordered_set<std::uint32_t>& usedPins) {
    const std::string sectionPath = "leds";

    auto ledsNode = root["leds"];
    if (!ledsNode) {
        return {};
    }

    if (!ledsNode.IsSequence()) {
        throwInvalidType(sectionPath, "sequence");
    }

    std::vector<DeviceDescription> out;
    out.reserve(ledsNode.size());

    for (std::size_t i = 0; i < ledsNode.size(); ++i) {
        const auto& item = ledsNode[i];
        std::ostringstream path;
        path << sectionPath << "[" << i << "]";

        if (!item.IsMap()) {
            throwInvalidType(path.str(), "map");
        }

        DeviceDescription dev{};
        dev.type = "virtual-led";
        dev.baseAddress = 0;  // not MMIO-mapped

        // name
        {
            auto nameNode = requireField(item, "name", path.str());
            dev.name = readScalar<std::string>(nameNode, makePath(path.str(), "name"));
            if (dev.name.empty()) {
                throwInvalidValue(makePath(path.str(), "name"), "must not be empty");
            }
            if (!usedNames.insert(dev.name).second) {
                throwDuplicateName(dev.name, "board");
            }
        }

        // pin
        std::uint32_t pin = 0;
        {
            auto pinNode = requireField(item, "pin", path.str());
            pin = readU32ScalarDefault(pinNode, makePath(path.str(), "pin"), 0xFFFF'FFFFu);
            if (pin == 0xFFFF'FFFFu) {
                throwInvalidType(makePath(path.str(), "pin"), "uint32");
            }
            if (pin >= pinCount) {
                throwInvalidValue(makePath(path.str(), "pin"), "must be in range 0.." + std::to_string(pinCount - 1));
            }
            if (!usedPins.insert(pin).second) {
                throwInvalidValue(makePath(path.str(), "pin"), "pin is already used by another LED/Button");
            }
        }

        // active_high optional default true
        const bool activeHigh = readBoolScalarDefault(item["active_high"], makePath(path.str(), "active_high"), true);

        dev.params.emplace("pin", std::to_string(pin));
        dev.params.emplace("active_high", activeHigh ? "true" : "false");

        out.push_back(std::move(dev));
    }

    return out;
}

std::vector<DeviceDescription> parseButtonsSection(const YAML::Node& root, std::uint32_t pinCount,
                                                   std::unordered_set<std::string>& usedNames,
                                                   std::unordered_set<std::uint32_t>& usedPins) {
    const std::string sectionPath = "buttons";

    auto btnNode = root["buttons"];
    if (!btnNode) {
        return {};
    }

    if (!btnNode.IsSequence()) {
        throwInvalidType(sectionPath, "sequence");
    }

    std::vector<DeviceDescription> out;
    out.reserve(btnNode.size());

    for (std::size_t i = 0; i < btnNode.size(); ++i) {
        const auto& item = btnNode[i];
        std::ostringstream path;
        path << sectionPath << "[" << i << "]";

        if (!item.IsMap()) {
            throwInvalidType(path.str(), "map");
        }

        DeviceDescription dev{};
        dev.type = "virtual-button";
        dev.baseAddress = 0;  // not MMIO-mapped

        // name
        {
            auto nameNode = requireField(item, "name", path.str());
            dev.name = readScalar<std::string>(nameNode, makePath(path.str(), "name"));
            if (dev.name.empty()) {
                throwInvalidValue(makePath(path.str(), "name"), "must not be empty");
            }
            if (!usedNames.insert(dev.name).second) {
                throwDuplicateName(dev.name, "board");
            }
        }

        // pin
        std::uint32_t pin = 0;
        {
            auto pinNode = requireField(item, "pin", path.str());
            pin = readU32ScalarDefault(pinNode, makePath(path.str(), "pin"), 0xFFFF'FFFFu);
            if (pin == 0xFFFF'FFFFu) {
                throwInvalidType(makePath(path.str(), "pin"), "uint32");
            }
            if (pin >= pinCount) {
                throwInvalidValue(makePath(path.str(), "pin"), "must be in range 0.." + std::to_string(pinCount - 1));
            }
            if (!usedPins.insert(pin).second) {
                throwInvalidValue(makePath(path.str(), "pin"), "pin is already used by another LED/Button");
            }
        }

        // active_high optional default true
        const bool activeHigh = readBoolScalarDefault(item["active_high"], makePath(path.str(), "active_high"), true);

        // mode optional default momentary
        std::string mode = "momentary";
        if (item["mode"]) {
            mode = readScalar<std::string>(item["mode"], makePath(path.str(), "mode"));
            if (mode.empty()) {
                throwInvalidValue(makePath(path.str(), "mode"), "must not be empty");
            }
        }
        if (mode != "momentary") {
            throwInvalidValue(makePath(path.str(), "mode"), "unsupported mode '" + mode + "' (supported: momentary)");
        }

        dev.params.emplace("pin", std::to_string(pin));
        dev.params.emplace("active_high", activeHigh ? "true" : "false");
        dev.params.emplace("mode", mode);

        out.push_back(std::move(dev));
    }

    return out;
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

    // --- Collect used names from existing devices (global uniqueness across board) ---
    std::unordered_set<std::string> usedNames;
    usedNames.reserve(board.devices.size() + 8);

    for (const auto& d : board.devices) {
        if (!usedNames.insert(d.name).second) {
            // parseDevices already checks duplicates within devices,
            // but this keeps global uniqueness logic consistent.
            throwDuplicateName(d.name, "board");
        }
    }

    // --- GPIO / LEDs / Buttons: new "v0.3" sections (syntactic sugar -> devices[]) ---
    // If leds/buttons are used, gpio section is required.
    const bool hasLedsSection = static_cast<bool>(root["leds"]);
    const bool hasButtonsSection = static_cast<bool>(root["buttons"]);
    const bool hasGpioSection = static_cast<bool>(root["gpio"]);

    if ((hasLedsSection || hasButtonsSection) && !hasGpioSection) {
        throwMissingField("gpio");
    }

    // Determine pin_count for validation:
    // - if gpio section exists: use it
    // - else (legacy): allow existing gpio device in devices and try to derive pin_count
    std::uint32_t pinCount = 32;
    if (hasGpioSection) {
        // parsed in parseGpioSectionOrThrow (also validates pin_count)
        // but we need pinCount here too; read it directly with same defaults for validation.
        auto gpioNode = root["gpio"];
        pinCount = readU32ScalarDefault(gpioNode["pin_count"], "gpio.pin_count", 32);
        if (pinCount == 0 || pinCount > 32) {
            throwInvalidValue("gpio.pin_count", "must be in range 1..32");
        }
    } else if (devicesContainGpio(board.devices)) {
        pinCount = extractPinCountFromGpioDevice(board.devices);
        if (pinCount == 0 || pinCount > 32) {
            pinCount = 32;
        }
    }

    std::unordered_set<std::uint32_t> usedPins;
    usedPins.reserve(32);

    // Add gpio device from section (if present)
    if (hasGpioSection) {
        auto gpioDev = parseGpioSectionOrThrow(root, board.memory, board.devices, usedNames);
        if (!gpioDev.type.empty()) {
            board.devices.push_back(std::move(gpioDev));
        }
    }

    // Parse LEDs + Buttons
    {
        auto ledDevs = parseLedsSection(root, pinCount, usedNames, usedPins);
        for (auto& d : ledDevs) {
            board.devices.push_back(std::move(d));
        }
    }
    {
        auto btnDevs = parseButtonsSection(root, pinCount, usedNames, usedPins);
        for (auto& d : btnDevs) {
            board.devices.push_back(std::move(d));
        }
    }

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
