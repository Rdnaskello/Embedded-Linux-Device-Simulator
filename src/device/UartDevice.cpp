#include "elsim/device/UartDevice.hpp"

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>

#include "elsim/core/Logger.hpp"

namespace elsim {

namespace {
constexpr std::string_view COMPONENT = "UART";
}  // namespace

std::uint8_t UartDevice::read(std::uint32_t offset) {
    auto& logger = core::Logger::instance();

    // Поки що підтримуємо тільки offset 0 як "умовний RX-регістр",
    // який завжди повертає 0.
    if (offset != 0U) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "READ from unsupported offset 0x%X -> 0x00", offset);
        logger.warn(COMPONENT, buf);
        return 0;
    }

    logger.debug(COMPONENT, "READ offset=0x0 -> value=0x00");
    return 0;
}

void UartDevice::write(std::uint32_t offset, std::uint8_t value) {
    auto& logger = core::Logger::instance();

    // Minimal UART model:
    // - Only offset 0 is treated as a TX register.
    // - Writing a byte to offset 0 prints it to the host stdout and logs the event.

    if (offset != 0U) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "WRITE to unsupported offset 0x%X value=0x%02X (ignored)", offset,
                      static_cast<unsigned int>(value));
        logger.warn(COMPONENT, buf);
        return;
    }

    // 1) Print the character to host stdout.
    unsigned char ch = value;
    std::putchar(ch);

    // 2) Log the transmitted byte at DEBUG level.
    char hexBuf[8];
    std::snprintf(hexBuf, sizeof(hexBuf), "%02X", static_cast<unsigned int>(value));

    std::string message = "TX 0x";
    message += hexBuf;

    if (std::isprint(ch)) {
        message += " ('";
        message += static_cast<char>(ch);
        message += "')";
    }

    message += " at offset 0x0";

    logger.debug(COMPONENT, message);
}

void UartDevice::tick() {
    // Minimal UART model:
    // - No internal buffering or timing is simulated.
    // - All transmission happens synchronously in write().
    // Therefore, tick() is a no-op for now.
}

}  // namespace elsim
