#include "elsim/device/GpioDevice.hpp"

#include <cstdio>
#include <string_view>
#include <utility>

#include "elsim/core/Logger.hpp"

namespace elsim {
namespace {
constexpr std::string_view COMPONENT = "GPIO";
}  // namespace

GpioDevice::GpioDevice(const std::string& name, std::uint32_t baseAddress, std::uint32_t pinCount,
                       std::shared_ptr<elsim::core::GpioController> gpio)
    : BaseDevice(name, baseAddress, RegisterSize),
      pinCount_(pinCount),
      pinMask_(makePinMask32(pinCount)),
      gpio_(std::move(gpio)) {
    auto& logger = core::Logger::instance();

    if (pinCount_ == 0 || pinCount_ > 32) {
        logger.error(COMPONENT, "Invalid pin_count for GPIO MMIO v0.3 (allowed: 1..32)");
        throw std::runtime_error("GpioDevice: pin_count must be in range 1..32 for v0.3");
    }

    if (!gpio_) {
        logger.error(COMPONENT, "GpioController is null");
        throw std::runtime_error("GpioDevice: gpio controller must not be null");
    }
}

std::uint32_t GpioDevice::makePinMask32(std::uint32_t pinCount) const {
    if (pinCount >= 32) {
        return 0xFFFF'FFFFu;
    }
    return (1u << pinCount) - 1u;
}

void GpioDevice::applyWriteDir(std::uint32_t value) {
    value &= pinMask_;
    for (std::uint32_t pin = 0; pin < pinCount_; ++pin) {
        const bool isOut = ((value >> pin) & 1u) != 0u;
        gpio_->setDirection(pin, isOut);
    }
}

void GpioDevice::applyWriteDataOut(std::uint32_t value) {
    value &= pinMask_;
    for (std::uint32_t pin = 0; pin < pinCount_; ++pin) {
        const bool level = ((value >> pin) & 1u) != 0u;
        gpio_->writeOutput(pin, level);
    }
}

void GpioDevice::applyWriteSet(std::uint32_t value) {
    value &= pinMask_;
    const std::uint32_t cur = static_cast<std::uint32_t>(gpio_->getOutputMask()) & pinMask_;
    const std::uint32_t next = cur | value;
    if (next != cur) {
        applyWriteDataOut(next);
    }
}

void GpioDevice::applyWriteClr(std::uint32_t value) {
    value &= pinMask_;
    const std::uint32_t cur = static_cast<std::uint32_t>(gpio_->getOutputMask()) & pinMask_;
    const std::uint32_t next = cur & ~value;
    if (next != cur) {
        applyWriteDataOut(next);
    }
}

void GpioDevice::applyWriteToggle(std::uint32_t value) {
    value &= pinMask_;
    const std::uint32_t cur = static_cast<std::uint32_t>(gpio_->getOutputMask()) & pinMask_;
    const std::uint32_t next = cur ^ value;
    if (next != cur) {
        applyWriteDataOut(next);
    }
}

std::uint8_t GpioDevice::read(std::uint32_t offset) {
    auto& logger = core::Logger::instance();

    // We expose 32-bit regs split into 4 bytes.
    if (offset < RegisterSize) {
        const std::uint32_t regBase = (offset / 4u) * 4u;
        const std::uint32_t byteOff = offset % 4u;

        std::uint32_t regValue = 0;

        switch (regBase) {
            case REG_DIR:
                regValue = static_cast<std::uint32_t>(gpio_->getDirectionMask()) & pinMask_;
                break;

            case REG_DATA_OUT:
                regValue = static_cast<std::uint32_t>(gpio_->getOutputMask()) & pinMask_;
                break;

            case REG_DATA_IN: {
                // v0.3 policy: for outputs, readback = DATA_OUT
                const std::uint32_t dir = static_cast<std::uint32_t>(gpio_->getDirectionMask()) & pinMask_;
                const std::uint32_t out = static_cast<std::uint32_t>(gpio_->getOutputMask()) & pinMask_;
                const std::uint32_t in = static_cast<std::uint32_t>(gpio_->getInputMask()) & pinMask_;

                // inputs -> injected in, outputs -> out
                regValue = (in & ~dir) | (out & dir);
                break;
            }

            // WO regs -> deterministic 0x00000000 on read
            case REG_SET:
            case REG_CLR:
            case REG_TOGGLE:
                regValue = 0;
                break;

            default: {
                char buf[96];
                std::snprintf(buf, sizeof(buf), "READ unknown regBase 0x%X (offset 0x%X) -> 0x%02X", regBase, offset,
                              static_cast<unsigned int>(kInvalidReadDefault));
                logger.warn(COMPONENT, buf);
                return kInvalidReadDefault;
            }
        }

        const std::uint8_t value = static_cast<std::uint8_t>((regValue >> (8u * byteOff)) & 0xFFu);

        char buf[96];
        std::snprintf(buf, sizeof(buf), "READ offset=0x%X -> 0x%02X", offset, static_cast<unsigned int>(value));
        logger.debug(COMPONENT, buf);

        return value;
    }

    char buf[96];
    std::snprintf(buf, sizeof(buf), "READ out of range offset 0x%X -> 0x%02X", offset,
                  static_cast<unsigned int>(kInvalidReadDefault));
    logger.warn(COMPONENT, buf);
    return kInvalidReadDefault;
}

void GpioDevice::write(std::uint32_t offset, std::uint8_t value) {
    auto& logger = core::Logger::instance();

    if (offset >= RegisterSize) {
        char buf[96];
        std::snprintf(buf, sizeof(buf), "WRITE out of range offset 0x%X value=0x%02X (ignored)", offset,
                      static_cast<unsigned int>(value));
        logger.warn(COMPONENT, buf);
        return;
    }

    const std::uint32_t regBase = (offset / 4u) * 4u;
    const std::uint32_t byteOff = offset % 4u;

    // We do read-modify-write for RW regs because bus is byte-wide.
    auto readReg32 = [&]() -> std::uint32_t {
        switch (regBase) {
            case REG_DIR:
                return static_cast<std::uint32_t>(gpio_->getDirectionMask()) & pinMask_;
            case REG_DATA_OUT:
                return static_cast<std::uint32_t>(gpio_->getOutputMask()) & pinMask_;
            default:
                return 0;
        }
    };

    auto writeReg32 = [&](std::uint32_t newVal) {
        newVal &= pinMask_;

        switch (regBase) {
            case REG_DIR:
                applyWriteDir(newVal);
                break;

            case REG_DATA_OUT:
                applyWriteDataOut(newVal);
                break;

            case REG_SET:
                applyWriteSet(newVal);
                break;

            case REG_CLR:
                applyWriteClr(newVal);
                break;

            case REG_TOGGLE:
                applyWriteToggle(newVal);
                break;

            case REG_DATA_IN: {
                // RO: ignore writes (mmio_contract)
                char b[96];
                std::snprintf(b, sizeof(b), "WRITE to RO DATA_IN offset=0x%X value=0x%02X (ignored)", offset,
                              static_cast<unsigned int>(value));
                logger.warn(COMPONENT, b);
                break;
            }

            default: {
                char b[96];
                std::snprintf(b, sizeof(b), "WRITE unknown regBase 0x%X (offset 0x%X) value=0x%02X (ignored)", regBase,
                              offset, static_cast<unsigned int>(value));
                logger.warn(COMPONENT, b);
                break;
            }
        }
    };

    // Handle RW regs with byte updates, and WO regs as write-1 semantics once full 32-bit value is written.
    // For WO regs: we interpret each byte write as affecting only that byte of the 32-bit bitmask.
    // This matches the bus contract: firmware writes 4 bytes for a 32-bit mask.
    if (regBase == REG_DIR || regBase == REG_DATA_OUT) {
        std::uint32_t cur = readReg32();
        const std::uint32_t shift = 8u * byteOff;
        cur &= ~(0xFFu << shift);
        cur |= (static_cast<std::uint32_t>(value) << shift);
        writeReg32(cur);

        char buf[96];
        std::snprintf(buf, sizeof(buf), "WRITE RW offset=0x%X value=0x%02X", offset, static_cast<unsigned int>(value));
        logger.debug(COMPONENT, buf);
        return;
    }

    if (regBase == REG_SET || regBase == REG_CLR || regBase == REG_TOGGLE) {
        const std::uint32_t maskPart = (static_cast<std::uint32_t>(value) << (8u * byteOff)) & pinMask_;
        writeReg32(maskPart);

        char buf[96];
        std::snprintf(buf, sizeof(buf), "WRITE WO offset=0x%X value=0x%02X", offset, static_cast<unsigned int>(value));
        logger.debug(COMPONENT, buf);
        return;
    }

    // DATA_IN or unknown: handled in writeReg32 warning path
    writeReg32(0);
}

void GpioDevice::tick() {
    // No timing in GPIO v0.3
}

}  // namespace elsim
