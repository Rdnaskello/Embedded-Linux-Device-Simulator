#include "elsim/core/GpioController.hpp"

#include <stdexcept>

namespace elsim::core {

GpioController::GpioController(std::size_t pinCount) : pin_count_(pinCount) {
    if (pinCount == 0 || pinCount > 64) {
        throw std::invalid_argument("GpioController: pinCount must be in range [1..64]");
    }
}

std::size_t GpioController::pinCount() const noexcept { return pin_count_; }

void GpioController::setDirection(std::size_t pin, bool isOutput) {
    validatePin_(pin);
    const auto b = bit_(pin);

    // Old effective level for this pin.
    const bool was = (effective_out_ & b) != 0;

    // Update DIR
    if (isOutput) {
        dir_ |= b;
    } else {
        dir_ &= ~b;
    }

    const GpioMask new_effective = dir_ & out_;
    const bool now = (new_effective & b) != 0;

    if (was == now) {
        // Even if DIR changed, externally observable output did not.
        effective_out_ = new_effective;
        return;
    }

    effective_out_ = new_effective;

    auto subs = out_subs_;
    for (const auto& [id, cb] : subs) {
        (void)id;
        if (cb) {
            cb(pin, now);
        }
    }
}

void GpioController::writeOutput(std::size_t pin, bool level) {
    validatePin_(pin);
    const auto b = bit_(pin);

    // Update OUT latch
    if (level) {
        out_ |= b;
    } else {
        out_ &= ~b;
    }

    const GpioMask new_effective = dir_ & out_;
    const bool was = (effective_out_ & b) != 0;
    const bool now = (new_effective & b) != 0;

    if (was == now) {
        return;  // no observable change
    }

    effective_out_ = new_effective;

    auto subs = out_subs_;
    for (const auto& [id, cb] : subs) {
        (void)id;
        if (cb) {
            cb(pin, now);
        }
    }
}

bool GpioController::readInput(std::size_t pin) const {
    validatePin_(pin);
    const auto b = bit_(pin);
    return (in_ & b) != 0;
}

void GpioController::injectInput(std::size_t pin, bool level) {
    validatePin_(pin);
    const auto b = bit_(pin);

    if (level) {
        in_ |= b;
    } else {
        in_ &= ~b;
    }
}

GpioController::GpioMask GpioController::getDirectionMask() const noexcept { return dir_; }
GpioController::GpioMask GpioController::getOutputMask() const noexcept { return out_; }
GpioController::GpioMask GpioController::getInputMask() const noexcept { return in_; }

GpioController::SubscriptionId GpioController::subscribeOnOutputChanged(OutputCallback cb) {
    if (!cb) {
        throw std::invalid_argument("GpioController::subscribeOnOutputChanged: callback is empty");
    }
    const auto id = next_sub_id_++;
    out_subs_.emplace(id, std::move(cb));
    return id;
}

void GpioController::unsubscribe(SubscriptionId id) { out_subs_.erase(id); }

void GpioController::validatePin_(std::size_t pin) const {
    if (pin >= pin_count_) {
        throw std::out_of_range("GpioController: pin index out of range");
    }
}

GpioController::GpioMask GpioController::bit_(std::size_t pin) { return (static_cast<GpioMask>(1) << pin); }

void GpioController::unsubscribeOnOutputChanged(std::size_t id) { out_subs_.erase(id); }

}  // namespace elsim::core
