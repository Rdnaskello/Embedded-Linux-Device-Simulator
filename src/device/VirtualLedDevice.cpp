#include "elsim/device/VirtualLedDevice.hpp"

#include <stdexcept>
#include <string>
#include <string_view>

#include "elsim/core/Logger.hpp"

namespace elsim {

namespace {
constexpr std::string_view COMPONENT = "LED";
}

VirtualLedDevice::VirtualLedDevice(std::string name, std::shared_ptr<elsim::core::GpioController> gpio, std::size_t pin,
                                   bool activeHigh)
    : BaseDevice(std::move(name), /*baseAddress=*/0, /*size=*/0),
      gpio_(std::move(gpio)),
      pin_(pin),
      active_high_(activeHigh),
      is_on_(false) {
    if (!gpio_) {
        throw std::invalid_argument("VirtualLedDevice: gpio is null");
    }
    if (pin_ >= gpio_->pinCount()) {
        throw std::out_of_range("VirtualLedDevice: pin out of range");
    }

    const auto dir = gpio_->getDirectionMask();
    const auto out = gpio_->getOutputMask();
    const auto bit = (1ULL << pin_);
    const bool level = ((dir & bit) != 0) && ((out & bit) != 0);
    is_on_ = active_high_ ? level : !level;

    sub_id_ = gpio_->subscribeOnOutputChanged([this](std::size_t changed_pin, bool levelNow) {
        if (changed_pin != pin_) {
            return;
        }

        const bool new_on = active_high_ ? levelNow : !levelNow;
        if (new_on == is_on_) {
            return;
        }

        is_on_ = new_on;
        elsim::core::Logger::instance().debug(COMPONENT, this->name() + " -> " + (is_on_ ? "ON" : "OFF"));
    });
}

VirtualLedDevice::~VirtualLedDevice() {
    if (gpio_ && sub_id_ != 0) {
        gpio_->unsubscribe(sub_id_);
    }
}

}  // namespace elsim
