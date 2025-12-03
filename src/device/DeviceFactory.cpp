#include "elsim/device/DeviceFactory.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <string>
#include <string_view>

#include "elsim/core/Logger.hpp"
#include "elsim/device/TimerDevice.hpp"
#include "elsim/device/UartDevice.hpp"

namespace elsim {

namespace {

// Simple helper to normalize type string to lower-case.
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
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
    } else if (normalizedType == "timer") {
        logger.info(COMPONENT, "Creating TIMER device: " + name);

        char buf[96];
        std::snprintf(buf, sizeof(buf), "Created TIMER device '%s' at base=0x%08X", name.c_str(), baseAddress);
        logger.debug(COMPONENT, buf);
        return new TimerDevice(baseAddress);
    }

    // ERROR log for unknown type.
    logger.error(
        COMPONENT, "Unknown device type '" + type + "' for device '" + name + "', base_addr=0x" + [&] {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%X", baseAddress);
            return std::string(buf);
        }());

    return nullptr;
}

}  // namespace elsim
