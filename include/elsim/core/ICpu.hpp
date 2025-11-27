#pragma once

#include <memory>
#include <string>

namespace elsim::core {

class IMemoryBus;

class ICpu {
   public:
    virtual ~ICpu() = default;

    // Виконати один крок симуляції CPU
    virtual void step() = 0;

    // Скинути стан CPU до початкового
    virtual void reset() = 0;

    // Завантажити образ програми / прошивки з файлу.
    // Повертає true, якщо завантаження пройшло успішно.
    virtual bool loadImage(const std::string& path) = 0;

    // Підключити шину пам'яті до CPU.
    virtual void setMemoryBus(std::shared_ptr<IMemoryBus> bus) = 0;
};
}  // namespace elsim::core