#pragma once

#include <cstdint>

#include "BaseDevice.hpp"
#include "elsim/core/Logger.hpp"

namespace elsim {

class TimerDevice : public BaseDevice {
   public:
    static constexpr std::uint32_t RegisterSize = 8;  // COUNTER(4) + CONTROL(4)

    // Адреси регістрів
    enum Registers : std::uint32_t {
        REG_COUNTER = 0x00,  // поточне значення лічильника (4 байти, R)
        REG_CONTROL = 0x04   // керування / reset (W)
    };

    TimerDevice(std::uint32_t baseAddress, std::uint32_t logPeriod = 1000);

    std::uint8_t read(std::uint32_t offset) override;
    void write(std::uint32_t offset, std::uint8_t value) override;
    void tick() override;

   private:
    std::uint32_t m_counter = 0;    // внутрішній лічильник тікiв
    std::uint32_t m_logPeriod = 0;  // період логування, 0 = вимкнено
};

}  // namespace elsim
