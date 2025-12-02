#include "elsim/device/TimerDevice.hpp"

#include <cstdint>

namespace elsim {

std::uint8_t TimerDevice::read(std::uint32_t /*offset*/) {
    // TODO: Реальна логіка таймера буде додана пізніше.
    // Поки що просто повертаємо 0.
    return 0;
}

void TimerDevice::write(std::uint32_t /*offset*/, std::uint8_t /*value*/) {
    // TODO: Налаштування таймера (період, режим тощо) — потім.
    // Поки що нічого не робимо.
}

void TimerDevice::tick() {
    // TODO: Збільшувати лічильник, генерувати інтеррапти тощо.
    // Поки що no-op.
}

}  // namespace elsim
