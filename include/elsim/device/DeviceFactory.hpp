#pragma once

#include <cstdint>
#include <string>

#include "elsim/core/BoardDescription.hpp"
#include "elsim/device/IDevice.hpp"

namespace elsim {

class DeviceFactory {
   public:
    /// Create device by type/name/base address.
    /// Throws std::runtime_error on error.
    static IDevice* createDevice(const std::string& type, const std::string& name, std::uint32_t baseAddress);

    /// Create device from board description (supports params).
    /// Throws std::runtime_error on error.
    static IDevice* createDevice(const elsim::core::DeviceDescription& desc);
};

}  // namespace elsim
