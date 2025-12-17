#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include "elsim/core/GpioController.hpp"
#include "elsim/device/BaseDevice.hpp"

namespace elsim {

class VirtualLedDevice final : public BaseDevice {
   public:
    // LED is not MMIO-mapped: baseAddress=0, size=0
    VirtualLedDevice(std::string name, std::shared_ptr<elsim::core::GpioController> gpio, std::size_t pin,
                     bool activeHigh = true);

    ~VirtualLedDevice() override;

    bool isOn() const noexcept { return is_on_; }
    bool state() const noexcept { return is_on_; }
    std::size_t pin() const noexcept { return pin_; }
    bool activeHigh() const noexcept { return active_high_; }

    // IDevice
    std::uint8_t read(std::uint32_t /*offset*/) override { return 0; }
    void write(std::uint32_t /*offset*/, std::uint8_t /*value*/) override {}
    void tick() override {}  // no timing

   private:
    std::shared_ptr<elsim::core::GpioController> gpio_;
    std::size_t pin_;
    bool active_high_;
    bool is_on_;
    elsim::core::GpioController::SubscriptionId sub_id_{0};
};

}  // namespace elsim
