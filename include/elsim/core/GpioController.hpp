#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <unordered_map>

namespace elsim::core {

class GpioController final {
   public:
    using GpioMask = std::uint64_t;
    using OutputCallback = std::function<void(std::size_t pin, bool level)>;
    using SubscriptionId = std::size_t;

    explicit GpioController(std::size_t pinCount);

    std::size_t pinCount() const noexcept;

    void setDirection(std::size_t pin, bool isOutput);
    void writeOutput(std::size_t pin, bool level);
    bool readInput(std::size_t pin) const;

    void injectInput(std::size_t pin, bool level);

    GpioMask getDirectionMask() const noexcept;
    GpioMask getOutputMask() const noexcept;
    GpioMask getInputMask() const noexcept;

    SubscriptionId subscribeOnOutputChanged(OutputCallback cb);
    void unsubscribe(SubscriptionId id);
    void unsubscribeOnOutputChanged(std::size_t id);

   private:
    void validatePin_(std::size_t pin) const;
    static GpioMask bit_(std::size_t pin);

    std::size_t pin_count_{0};
    GpioMask dir_{0};
    GpioMask out_{0};
    GpioMask in_{0};
    GpioMask effective_out_{0};

    SubscriptionId next_sub_id_{1};
    std::unordered_map<SubscriptionId, OutputCallback> out_subs_;
};

}  // namespace elsim::core
