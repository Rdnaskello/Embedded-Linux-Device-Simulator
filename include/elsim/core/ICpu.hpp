#pragma once

#include <cstdint>
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

    // Підключити шину пам'яті до CPU
    virtual void setMemoryBus(std::shared_ptr<IMemoryBus> bus) = 0;

    // Чи знаходиться CPU у стані HALT
    virtual bool isHalted() const noexcept = 0;

    // ===== Program Counter (PC) — канонічний API =====

    // Отримати поточне значення PC
    virtual std::uint32_t getPc() const noexcept = 0;

    // Встановити PC
    virtual void setPc(std::uint32_t value) noexcept = 0;

    // ===== Legacy compatibility (тимчасово) =====
    // Потрібно для сумісності зі старим кодом (CLI, приклади)
    // Може бути видалено у наступних релізах

    void setPC(std::uint32_t value) noexcept { setPc(value); }
    std::uint32_t getPC() const noexcept { return getPc(); }
};

}  // namespace elsim::core
