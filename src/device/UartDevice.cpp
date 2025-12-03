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

std::uint8_t UartDevice::read(std::uint32_t /*offset*/) {
    // Minimal UART model:
    // - No receive buffer or status registers are implemented yet.
    // - Reads always return 0.
    // Future tasks may add RX data handling and status flags.
    return 0;
}

void UartDevice::write(std::uint32_t offset, std::uint8_t value) {
    // Minimal UART model:
    // - Only offset 0 is treated as a TX register.
    // - Writing a byte to offset 0 prints it to the host stdout and logs the event.

    if (offset != 0U) {
        // For now, ignore writes to any other offsets.
        // In future we might add status/configuration registers here.
        return;
    }

    // 1) Print the character to host stdout.
    unsigned char ch = value;
    std::putchar(ch);
    // We rely on stdio buffering; no explicit fflush() here to avoid overhead.

    // 2) Log the transmitted byte at DEBUG level.
    auto& logger = core::Logger::instance();

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
