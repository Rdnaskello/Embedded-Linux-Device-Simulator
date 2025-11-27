#pragma once

namespace elsim::device {

/**
 * @brief Абстракція над колекцією пристроїв, яку можна "тикнути" за один крок симуляції.
 *
 * Реалізація цього інтерфейсу повинна викликати tick() у всіх зареєстрованих IDevice.
 */
class IDevicesTickable {
   public:
    virtual ~IDevicesTickable() = default;

    /**
     * @brief Виконує один логічний крок для всіх пристроїв.
     */
    virtual void tickAll() = 0;
};

}  // namespace elsim::device
