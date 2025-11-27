#pragma once

#include <cstdint>
#include <iostream>
#include <ostream>

// Forward declarations, щоб не тягнути зайві залежності в заголовок.
namespace elsim::core {
class ICpu;
}

namespace elsim::device {
class IDevicesTickable;
}

namespace elsim::core {

/**
 * @brief Головний цикл симуляції: виконує крок CPU та оновлює всі пристрої.
 *
 * Відповідає за:
 *  - координацію cpu.step() + devices.tickAll()
 *  - логіку start/stop
 *  - підрахунок кількості виконаних тактів
 *  - базове логування етапів симуляції
 */
class Simulator {
   public:
    /**
     * @brief Створює симулятор.
     *
     * @param cpu      Посилання на CPU, який реалізує інтерфейс ICpu.
     * @param devices  Колекція пристроїв, яка вміє виконувати tickAll().
     * @param log      Потік для логування (за замовчуванням std::cout).
     */
    Simulator(ICpu& cpu, device::IDevicesTickable& devices, std::ostream& log = std::cout);

    /**
     * @brief Запускає цикл симуляції.
     *
     * @param maxCycles Максимальна кількість тактів симуляції.
     *                  Якщо 0 — симуляція триває, поки не буде викликано stop().
     */
    void start(std::uint64_t maxCycles = 0);

    /**
     * @brief Просить зупинити симуляцію.
     *
     * Реальна зупинка відбудеться після завершення поточного такту в start().
     */
    void stop();

    /**
     * @brief Виконує один логічний такт симуляції:
     *        cpu.step() + devices.tickAll().
     */
    void runOneTick();

    /**
     * @brief Чи зараз симуляція в стані "running".
     */
    [[nodiscard]] bool isRunning() const noexcept;

    /**
     * @brief Повертає кількість виконаних тактів симуляції.
     */
    [[nodiscard]] std::uint64_t cycleCount() const noexcept;

   private:
    ICpu& cpu_;
    device::IDevicesTickable& devices_;
    std::ostream& log_;

    bool running_{false};
    std::uint64_t cycleCount_{0};
};

}  // namespace elsim::core
