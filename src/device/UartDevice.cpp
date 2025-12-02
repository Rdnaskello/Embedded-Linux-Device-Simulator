#include "elsim/device/UartDevice.hpp"

#include <cstdint>

namespace elsim {

std::uint8_t UartDevice::read(std::uint32_t /*offset*/) {
    // TODO: Реальна логіка UART буде додана пізніше.
    // Поки що просто повертаємо 0.
    return 0;
}

void UartDevice::write(std::uint32_t /*offset*/, std::uint8_t /*value*/) {
    // TODO: Реальна логіка UART буде додана пізніше.
    // Поки що нічого не робимо.
}

void UartDevice::tick() {
    // TODO: Реалізувати таймінг/буфери UART.
    // Поки що no-op.
}

}  // namespace elsim
