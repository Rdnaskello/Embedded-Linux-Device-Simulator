#pragma once

#include <cstdint>
#include <string>

#include "elsim/device/IDevice.hpp"

namespace elsim {

class DeviceFactory {
   public:
    /// Create device by type/name/base address.
    /// Returns raw pointer to IDevice or nullptr on error.
    /// Caller is responsible for managing the lifetime (wrap into std::unique_ptr).
    static IDevice* createDevice(const std::string& type, const std::string& name, std::uint32_t baseAddress);
};

}  // namespace elsim
