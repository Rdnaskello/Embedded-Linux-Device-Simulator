#pragma once

#include <cstdint>

#include "elsim/core/IMemoryMappedDevice.hpp"
#include "elsim/device/IDevice.hpp"

// Адаптер, який дозволяє використовувати elsim::IDevice як IMemoryMappedDevice
class DeviceMemoryAdapter : public IMemoryMappedDevice {
   public:
    explicit DeviceMemoryAdapter(elsim::IDevice* device) : device_(device) {}

    std::uint8_t read8(std::uint32_t offset) override {
        if (!device_) {
            return 0;
        }
        return device_->read(offset);
    }

    void write8(std::uint32_t offset, std::uint8_t value) override {
        if (!device_) {
            return;
        }
        device_->write(offset, value);
    }

   private:
    elsim::IDevice* device_;  // не володіємо, життям керує Simulator через unique_ptr
};
