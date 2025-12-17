#pragma once

#include <cstdint>
#include <iosfwd>
#include <iostream>
#include <memory>
#include <vector>

#include "elsim/core/GpioController.hpp"
#include "elsim/core/ICpu.hpp"
#include "elsim/core/MemoryBus.hpp"
#include "elsim/device/IDevice.hpp"

namespace elsim {
class VirtualLedDevice;
}

namespace elsim::core {

class BoardDescription;

/**
 * @brief Головний цикл симуляції: виконує крок CPU та оновлює всі пристрої.
 *
 * Simulator сам створює CPU/RAM/пристрої
 * на основі BoardDescription у методі loadBoard().
 */
class Simulator {
   public:
    explicit Simulator(std::ostream& log = std::cout);
    ~Simulator();

    /// Ініціалізує плату на основі опису: CPU, RAM, пристрої, MemoryBus (MMIO).
    void loadBoard(const BoardDescription& board);

    void start(std::uint64_t maxCycles = 0);
    void stop();
    void runOneTick();

    // Доступ до компонентів симулятора
    ICpu* cpu() noexcept;
    const ICpu* cpu() const noexcept;
    MemoryBus* memoryBus() noexcept;
    const MemoryBus* memoryBus() const noexcept;

    [[nodiscard]] bool isRunning() const noexcept;
    [[nodiscard]] std::uint64_t cycleCount() const noexcept;

    std::shared_ptr<const elsim::core::GpioController> gpioController() const noexcept;
    std::vector<const elsim::VirtualLedDevice*> ledDevices() const;

   private:
    // Логування
    std::ostream& log_;

    // Стан симуляції
    bool running_{false};
    std::uint64_t cycleCount_{0};

    // "Залізо" плати
    std::unique_ptr<MemoryBus> memoryBus_;
    std::unique_ptr<ICpu> cpu_;
    std::vector<std::unique_ptr<elsim::IDevice>> devices_;
    std::shared_ptr<elsim::core::GpioController> gpio_;
};

}  // namespace elsim::core
