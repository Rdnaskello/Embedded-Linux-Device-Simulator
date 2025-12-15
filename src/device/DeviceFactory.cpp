#include "elsim/device/DeviceFactory.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>

#include "elsim/core/Logger.hpp"
#include "elsim/device/GpioDevice.hpp"
#include "elsim/device/TimerDevice.hpp"
#include "elsim/device/UartDevice.hpp"
#include "elsim/device/VirtualLedDevice.hpp"

namespace elsim {

namespace {

// Simple helper to normalize type string to lower-case.
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

std::uint32_t parseU32Param(const std::map<std::string, std::string>& params, const std::string& key,
                            std::uint32_t defaultValue) {
    auto it = params.find(key);
    if (it == params.end()) {
        return defaultValue;
    }
    unsigned long v = std::stoul(it->second, nullptr, 0);  // base 0 supports "32" and "0x20"
    if (v > 0xFFFF'FFFFul) {
        throw std::runtime_error("DeviceFactory: param '" + key + "' is too large");
    }
    return static_cast<std::uint32_t>(v);
}

bool parseBoolParam(const std::map<std::string, std::string>& params, const std::string& key, bool defaultValue) {
    auto it = params.find(key);
    if (it == params.end()) {
        return defaultValue;
    }

    std::string v = toLower(it->second);
    if (v == "1" || v == "true" || v == "yes" || v == "on") {
        return true;
    }
    if (v == "0" || v == "false" || v == "no" || v == "off") {
        return false;
    }

    throw std::runtime_error("DeviceFactory: param '" + key + "' must be boolean (true/false/1/0)");
}

std::uint32_t checkedBaseAddressU32(std::uint64_t baseAddress, const std::string& devName) {
    if (baseAddress > 0xFFFF'FFFFull) {
        throw std::runtime_error("DeviceFactory: device '" + devName + "' baseAddress exceeds 32-bit range");
    }
    return static_cast<std::uint32_t>(baseAddress);
}

constexpr std::string_view COMPONENT = "DeviceFactory";

}  // namespace

IDevice* DeviceFactory::createDevice(const std::string& type, const std::string& name, std::uint32_t baseAddress) {
    const auto normalizedType = toLower(type);
    auto& logger = core::Logger::instance();

    if (normalizedType == "uart") {
        logger.info(COMPONENT, "Creating UART device: " + name);
        char buf[96];
        std::snprintf(buf, sizeof(buf), "Created UART device '%s' at base=0x%08X", name.c_str(), baseAddress);
        logger.debug(COMPONENT, buf);
        return new UartDevice(baseAddress);
    }

    if (normalizedType == "timer") {
        logger.info(COMPONENT, "Creating TIMER device: " + name);
        char buf[96];
        std::snprintf(buf, sizeof(buf), "Created TIMER device '%s' at base=0x%08X", name.c_str(), baseAddress);
        logger.debug(COMPONENT, buf);
        return new TimerDevice(baseAddress);
    }

    logger.error(
        COMPONENT, "Unknown device type '" + type + "' for device '" + name + "', base_addr=0x" + [&] {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%X", baseAddress);
            return std::string(buf);
        }());

    return nullptr;
}

IDevice* DeviceFactory::createDevice(const elsim::core::DeviceDescription& desc) {
    const auto normalizedType = toLower(desc.type);

    // IMPORTANT: without services, GPIO/LED would create a private controller and break board-level wiring.
    if (normalizedType == "gpio" || normalizedType == "led" || normalizedType == "virtual-led") {
        throw std::runtime_error(
            "DeviceFactory::createDevice(desc): device type '" + desc.type +
            "' requires BoardServices (shared GPIO). Use createDevice(desc, services) from Simulator.");
    }

    const std::uint32_t base32 = checkedBaseAddressU32(desc.baseAddress, desc.name);
    return createDevice(desc.type, desc.name, base32);
}

IDevice* DeviceFactory::createDevice(const elsim::core::DeviceDescription& desc, const BoardServices& services) {
    const auto normalizedType = toLower(desc.type);
    auto& logger = core::Logger::instance();

    const std::uint32_t base32 = checkedBaseAddressU32(desc.baseAddress, desc.name);

    if (normalizedType == "gpio") {
        const std::uint32_t pinCount = parseU32Param(desc.params, "pin_count", 32);

        if (pinCount == 0 || pinCount > 32) {
            logger.error(COMPONENT, "Invalid pin_count for GPIO '" + desc.name + "' (allowed: 1..32)");
            throw std::runtime_error("DeviceFactory: GPIO pin_count must be in range 1..32");
        }

        if (!services.gpio) {
            throw std::runtime_error("DeviceFactory: GPIO device '" + desc.name +
                                     "' requires BoardServices.gpio (shared controller), but it is null");
        }

        // v0.3 invariant: exactly one source of truth for GPIO wiring per board.
        // Simulator creates BoardServices.gpio with the board pin_count, so they must match.
        if (services.gpio->pinCount() != static_cast<std::size_t>(pinCount)) {
            throw std::runtime_error("DeviceFactory: GPIO device '" + desc.name +
                                     "' pin_count mismatch with BoardServices.gpio");
        }

        logger.info(COMPONENT, "Creating GPIO device: " + desc.name);

        char buf[128];
        std::snprintf(buf, sizeof(buf), "Created GPIO device '%s' at base=0x%08X pin_count=%u", desc.name.c_str(),
                      base32, pinCount);
        logger.debug(COMPONENT, buf);

        return new GpioDevice(desc.name, base32, pinCount, services.gpio);
    }

    if (normalizedType == "led" || normalizedType == "virtual-led") {
        if (!services.gpio) {
            throw std::runtime_error("DeviceFactory: LED device '" + desc.name +
                                     "' requires BoardServices.gpio (shared controller), but it is null");
        }

        const std::uint32_t pin = parseU32Param(desc.params, "pin", 0xFFFF'FFFFu);
        if (pin == 0xFFFF'FFFFu) {
            throw std::runtime_error("DeviceFactory: LED device '" + desc.name + "' requires param 'pin'");
        }

        const bool activeHigh = parseBoolParam(desc.params, "active_high", true);

        logger.info(COMPONENT, "Creating Virtual LED device: " + desc.name);

        char buf[160];
        std::snprintf(buf, sizeof(buf), "Created LED device '%s' pin=%u active_high=%s", desc.name.c_str(), pin,
                      activeHigh ? "true" : "false");
        logger.debug(COMPONENT, buf);

        return new VirtualLedDevice(desc.name, services.gpio, static_cast<std::size_t>(pin), activeHigh);
    }

    // fallback to existing path for uart/timer (and unknowns)
    return createDevice(desc.type, desc.name, base32);
}

}  // namespace elsim
