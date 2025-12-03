#include <cstdint>
#include <iostream>

#include "elsim/core/Logger.hpp"
#include "elsim/device/TimerDevice.hpp"

using namespace elsim;

int main() {
    auto& logger = core::Logger::instance();
    // Наразі Logger не має API для зміни рівня, тож просто використовуємо дефолтний.

    TimerDevice timer(0x20000000u, 5u);  // логувати кожні 5 тікiв

    std::cout << "Starting timer smoke test...\n";

    for (int i = 0; i < 20; ++i) {
        timer.tick();

        // Читаємо 32-бітне значення лічильника по 4 байти (little-endian)
        std::uint32_t value = 0;
        value |= static_cast<std::uint32_t>(timer.read(0)) << 0;
        value |= static_cast<std::uint32_t>(timer.read(1)) << 8;
        value |= static_cast<std::uint32_t>(timer.read(2)) << 16;
        value |= static_cast<std::uint32_t>(timer.read(3)) << 24;

        std::cout << "Tick " << (i + 1) << " -> counter=" << value << '\n';
    }

    // Тест reset через CONTROL-регістр (offset 4)
    timer.write(4, 1);

    std::uint32_t afterReset = 0;
    afterReset |= static_cast<std::uint32_t>(timer.read(0)) << 0;
    afterReset |= static_cast<std::uint32_t>(timer.read(1)) << 8;
    afterReset |= static_cast<std::uint32_t>(timer.read(2)) << 16;
    afterReset |= static_cast<std::uint32_t>(timer.read(3)) << 24;

    std::cout << "After reset -> counter=" << afterReset << '\n';

    return 0;
}
