#include <cstdint>
#include <iostream>

#include "elsim/core/Logger.hpp"
#include "elsim/device/TimerDevice.hpp"
#include "elsim/device/UartDevice.hpp"

using elsim::UartDevice;
using elsim::core::Logger;
using elsim::core::LogLevel;
using namespace elsim;  // як у timer_smoke_test для TimerDevice

int main() {
    // 1. Налаштовуємо логер: хочемо бачити debug-повідомлення від UART/Timer.
    Logger::instance().set_level(LogLevel::Debug);

    std::cout << "Starting UART+Timer demo...\n";

    // 2. Створюємо UART і Timer з якимись базовими адресами.
    //    Адреси можна взяти ті самі, що й у board.yaml, але тут вони поки що "логічні".
    UartDevice uart(0x40000000u);
    TimerDevice timer(0x40000100u, 5u);  // логувати кожні 5 тікiв (див. timer_smoke_test)

    // 3. Надсилаємо повідомлення через UART.
    //
    //    Логіка така сама, як у uart_smoke_test:
    //    uart.write(0, <байт>);  // offset 0 — TX-регістр.
    //    У консолі має зʼявитися текст + debug-логи UART. :contentReference[oaicite:3]{index=3}
    const char* msg = "Hi from UART+Timer demo!\n";
    for (const char* p = msg; *p != '\0'; ++p) {
        uart.write(0, static_cast<std::uint8_t>(*p));
    }

    // 4. Крутимо таймер кілька разів і читаємо лічильник (копіпаста патерну з timer_smoke_test).
    // :contentReference[oaicite:4]{index=4}
    for (int i = 0; i < 10; ++i) {
        timer.tick();

        std::uint32_t value = 0;
        value |= static_cast<std::uint32_t>(timer.read(0)) << 0;
        value |= static_cast<std::uint32_t>(timer.read(1)) << 8;
        value |= static_cast<std::uint32_t>(timer.read(2)) << 16;
        value |= static_cast<std::uint32_t>(timer.read(3)) << 24;

        std::cout << "Tick " << (i + 1) << " -> counter=" << value << '\n';
    }

    // 5. Перевіряємо reset таймера (як у timer_smoke_test).
    timer.write(4, 1);  // CONTROL-регістр: reset

    std::uint32_t afterReset = 0;
    afterReset |= static_cast<std::uint32_t>(timer.read(0)) << 0;
    afterReset |= static_cast<std::uint32_t>(timer.read(1)) << 8;
    afterReset |= static_cast<std::uint32_t>(timer.read(2)) << 16;
    afterReset |= static_cast<std::uint32_t>(timer.read(3)) << 24;

    std::cout << "After reset -> counter=" << afterReset << '\n';

    std::cout << "UART+Timer demo finished.\n";
    return 0;
}
