#pragma once

#include <cstdint>
#include <string>

#include "elsim/core/GpioController.hpp"
#include "elsim/device/BaseDevice.hpp"

namespace elsim {

class GpioDevice final : public BaseDevice {
   public:
    static constexpr std::uint32_t RegisterSize = 0x18;

    GpioDevice(const std::string& name, std::uint32_t baseAddress, std::uint32_t pinCount = 32);

    std::uint8_t read(std::uint32_t offset) override;
    void write(std::uint32_t offset, std::uint8_t value) override;
    void tick() override;

    std::uint32_t pinCount() const noexcept { return pinCount_; }

   private:
    static constexpr std::uint32_t REG_DIR = 0x00;
    static constexpr std::uint32_t REG_DATA_IN = 0x04;
    static constexpr std::uint32_t REG_DATA_OUT = 0x08;
    static constexpr std::uint32_t REG_SET = 0x0C;
    static constexpr std::uint32_t REG_CLR = 0x10;
    static constexpr std::uint32_t REG_TOGGLE = 0x14;

    static constexpr std::uint8_t kInvalidReadDefault = 0x00;

    std::uint32_t makePinMask32(std::uint32_t pinCount) const;

    void applyWriteDir(std::uint32_t value);
    void applyWriteDataOut(std::uint32_t value);
    void applyWriteSet(std::uint32_t value);
    void applyWriteClr(std::uint32_t value);
    void applyWriteToggle(std::uint32_t value);

   private:
    std::uint32_t pinCount_;
    std::uint32_t pinMask_;
    elsim::core::GpioController gpio_;
};

}  // namespace elsim
