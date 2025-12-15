#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include "elsim/core/GpioController.hpp"
#include "elsim/device/BaseDevice.hpp"

namespace elsim {

class VirtualButtonDevice final : public BaseDevice {
   public:
    // Button is not MMIO-mapped: baseAddress=0, size=0
    // NOTE(v0.3): No debounce implemented.
    VirtualButtonDevice(std::string name, std::shared_ptr<elsim::core::GpioController> gpio, std::size_t pin,
                        bool activeHigh = true, std::string mode = "momentary");

    ~VirtualButtonDevice() override = default;

    void press();
    void release();
    bool isPressed() const noexcept { return pressed_; }

    // IDevice (no MMIO)
    std::uint8_t read(std::uint32_t /*offset*/) override { return 0; }
    void write(std::uint32_t /*offset*/, std::uint8_t /*value*/) override {}
    void tick() override {}  // no timing

   private:
    bool levelForPressed_(bool pressed) const noexcept;

    std::shared_ptr<elsim::core::GpioController> gpio_;
    std::size_t pin_{0};
    bool active_high_{true};
    bool pressed_{false};
    std::string mode_;
};

}  // namespace elsim
