#pragma once

#include <cstdint>

#include "BaseDevice.hpp"

namespace elsim {

class TimerDevice : public BaseDevice {
   public:
    TimerDevice(std::uint32_t baseAddress) : BaseDevice("Timer", baseAddress, RegisterSize) {}

    // Карта регістрів таймера
    enum Registers : std::uint32_t {
        REG_CURRENT = 0x00,  // поточне значення лічильника
        REG_RELOAD = 0x04,   // період
        REG_CONTROL = 0x08,  // біти: enable, irq_enable, mode
        REG_STATUS = 0x0C,   // переповнення / ready
    };

    static constexpr std::uint32_t RegisterSize = 0x10;

    // Перевизначення методів
    std::uint8_t read(std::uint32_t offset) override;
    void write(std::uint32_t offset, std::uint8_t value) override;
    void tick() override;

   private:
    // Пізніше тут буде внутрішня логіка:
    //  - лічильник
    //  - reload value
    //  - control flags
};

}  // namespace elsim
