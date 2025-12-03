#include "elsim/device/TimerDevice.hpp"

#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>

#include "elsim/core/Logger.hpp"

namespace elsim {

namespace {
constexpr std::string_view COMPONENT = "TIMER";
}  // namespace

TimerDevice::TimerDevice(std::uint32_t baseAddress, std::uint32_t logPeriod)
    : BaseDevice("Timer", baseAddress, RegisterSize), m_counter(0), m_logPeriod(logPeriod) {}

// ------------------------------------------------------------
// READ
// ------------------------------------------------------------
std::uint8_t TimerDevice::read(std::uint32_t offset) {
    auto& logger = core::Logger::instance();

    switch (offset) {
        // COUNTER — 32-бітне значення, розбите на 4 байти (little-endian)
        case REG_COUNTER:
            return static_cast<std::uint8_t>((m_counter >> 0) & 0xFF);
        case REG_COUNTER + 1:
            return static_cast<std::uint8_t>((m_counter >> 8) & 0xFF);
        case REG_COUNTER + 2:
            return static_cast<std::uint8_t>((m_counter >> 16) & 0xFF);
        case REG_COUNTER + 3:
            return static_cast<std::uint8_t>((m_counter >> 24) & 0xFF);

        default: {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "Invalid read offset 0x%X", offset);
            logger.debug(COMPONENT, buf);
            return 0xFF;
        }
    }
}

// ------------------------------------------------------------
// WRITE
// ------------------------------------------------------------
void TimerDevice::write(std::uint32_t offset, std::uint8_t value) {
    auto& logger = core::Logger::instance();

    switch (offset) {
        case REG_CONTROL: {
            // Мінімальна семантика:
            // - запис значення 1 у CONTROL скидає лічильник
            if (value == 1U) {
                m_counter = 0;

                logger.debug(COMPONENT, "Counter reset via CONTROL register");
            }
            // Інші значення поки що ігноруємо
            break;
        }

        default: {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "Invalid write offset 0x%X", offset);
            logger.debug(COMPONENT, buf);
            break;
        }
    }
}

// ------------------------------------------------------------
// TICK
// ------------------------------------------------------------
void TimerDevice::tick() {
    auto& logger = core::Logger::instance();

    ++m_counter;

    if (m_logPeriod != 0U && (m_counter % m_logPeriod) == 0U) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "Tick = %u", static_cast<unsigned int>(m_counter));
        logger.debug(COMPONENT, buf);
    }
}

}  // namespace elsim
