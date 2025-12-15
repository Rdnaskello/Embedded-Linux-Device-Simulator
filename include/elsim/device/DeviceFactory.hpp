#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "elsim/core/BoardDescription.hpp"
#include "elsim/core/GpioController.hpp"
#include "elsim/device/IDevice.hpp"

namespace elsim {

class DeviceFactory {
   public:
    struct BoardServices {
        std::shared_ptr<elsim::core::GpioController> gpio;  // shared GPIO controller per-board
    };

    /// Create device by type/name/base address.
    /// Throws std::runtime_error on error.
    static IDevice* createDevice(const std::string& type, const std::string& name, std::uint32_t baseAddress);

    /// Create device from board description (supports params).
    /// Throws std::runtime_error on error.
    static IDevice* createDevice(const elsim::core::DeviceDescription& desc);

    /// Create device from board description with access to board-level shared services (GPIO, etc.).
    /// Throws std::runtime_error on error.
    static IDevice* createDevice(const elsim::core::DeviceDescription& desc, const BoardServices& services);
};

}  // namespace elsim
