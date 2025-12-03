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

    std::uint8_t value = 0xFF;
    bool valid = true;

    switch (offset) {
        // COUNTER — 32-бітне значення, розбите на 4 байти (little-endian)
        case REG_COUNTER:
            value = static_cast<std::uint8_t>((m_counter >> 0) & 0xFF);
            break;
        case REG_COUNTER + 1:
            value = static_cast<std::uint8_t>((m_counter >> 8) & 0xFF);
            break;
        case REG_COUNTER + 2:
            value = static_cast<std::uint8_t>((m_counter >> 16) & 0xFF);
            break;
        case REG_COUNTER + 3:
            value = static_cast<std::uint8_t>((m_counter >> 24) & 0xFF);
            break;

        default: {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "Invalid read offset 0x%X", offset);
            logger.warn(COMPONENT, buf);
            valid = false;
            break;
        }
    }

    if (valid) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "READ offset=0x%X -> 0x%02X", offset, static_cast<unsigned int>(value));
        logger.debug(COMPONENT, buf);
    }

    return value;
}

// ------------------------------------------------------------
// WRITE
// ------------------------------------------------------------
void TimerDevice::write(std::uint32_t offset, std::uint8_t value) {
    auto& logger = core::Logger::instance();

    switch (offset) {
        case REG_CONTROL: {
            if (value == 1U) {
                m_counter = 0;
                logger.debug(COMPONENT, "CONTROL=1 -> counter reset");
            } else {
                char buf[64];
                std::snprintf(buf, sizeof(buf), "WRITE CONTROL offset=0x%X value=0x%02X (ignored)", offset,
                              static_cast<unsigned int>(value));
                logger.debug(COMPONENT, buf);
            }
            break;
        }

        default: {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "Invalid write offset 0x%X value=0x%02X", offset,
                          static_cast<unsigned int>(value));
            logger.warn(COMPONENT, buf);
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
