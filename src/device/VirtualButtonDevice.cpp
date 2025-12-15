#include "elsim/device/VirtualButtonDevice.hpp"

#include <stdexcept>
#include <string_view>
#include <utility>

#include "elsim/core/Logger.hpp"

namespace elsim {

namespace {
constexpr std::string_view COMPONENT = "BUTTON";
}

VirtualButtonDevice::VirtualButtonDevice(std::string name, std::shared_ptr<elsim::core::GpioController> gpio,
                                         std::size_t pin, bool activeHigh, std::string mode)
    : BaseDevice(std::move(name), /*baseAddress=*/0, /*size=*/0),
      gpio_(std::move(gpio)),
      pin_(pin),
      active_high_(activeHigh),
      pressed_(false),
      mode_(std::move(mode)) {
    if (!gpio_) {
        throw std::invalid_argument("VirtualButtonDevice: gpio is null");
    }
    if (pin_ >= gpio_->pinCount()) {
        throw std::out_of_range("VirtualButtonDevice: pin out of range");
    }

    // v0.3: only "momentary" is supported (reserved for future expansion).
    // Normalize to lower-case in factory; here we just validate.
    if (!mode_.empty() && mode_ != "momentary") {
        throw std::invalid_argument("VirtualButtonDevice: unsupported mode '" + mode_ + "'");
    }

    // Initialize released state onto input pin.
    const bool level = levelForPressed_(false);
    gpio_->injectInput(pin_, level);
}

bool VirtualButtonDevice::levelForPressed_(bool pressed) const noexcept {
    // active_high: pressed->1 released->0
    // active_low : pressed->0 released->1
    return active_high_ ? pressed : !pressed;
}

void VirtualButtonDevice::press() {
    if (pressed_) {
        return;
    }
    pressed_ = true;

    const bool level = levelForPressed_(true);
    gpio_->injectInput(pin_, level);

    elsim::core::Logger::instance().debug(
        COMPONENT, name() + " pressed pin=" + std::to_string(pin_) + " level=" + std::to_string(level ? 1 : 0));
}

void VirtualButtonDevice::release() {
    if (!pressed_) {
        return;
    }
    pressed_ = false;

    const bool level = levelForPressed_(false);
    gpio_->injectInput(pin_, level);

    elsim::core::Logger::instance().debug(
        COMPONENT, name() + " released pin=" + std::to_string(pin_) + " level=" + std::to_string(level ? 1 : 0));
}

}  // namespace elsim
