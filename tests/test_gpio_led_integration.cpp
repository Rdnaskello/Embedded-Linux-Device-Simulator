#include <gtest/gtest.h>

#include <memory>

#include "elsim/core/DeviceMemoryAdapter.hpp"
#include "elsim/core/GpioController.hpp"
#include "elsim/core/MemoryBus.hpp"
#include "elsim/device/GpioDevice.hpp"
#include "elsim/device/VirtualLedDevice.hpp"

TEST(GpioLedIntegration, MmioDirAndOutDriveVirtualLed) {
    // Arrange
    elsim::core::MemoryBus bus(64 * 1024);

    auto gpio = std::make_shared<elsim::core::GpioController>(32);

    auto* gpioDev = new elsim::GpioDevice("gpio0", 0x3000, 32, gpio);
    auto gpioMmio = std::make_shared<elsim::core::DeviceMemoryAdapter>(gpioDev);
    bus.mapDevice(0x3000, elsim::GpioDevice::RegisterSize, gpioMmio);

    elsim::VirtualLedDevice led("led0", gpio, 3, true);

    auto write32 = [&](std::uint32_t addr, std::uint32_t value) {
        bus.write8(addr + 0, static_cast<std::uint8_t>(value & 0xFFu));
        bus.write8(addr + 1, static_cast<std::uint8_t>((value >> 8) & 0xFFu));
        bus.write8(addr + 2, static_cast<std::uint8_t>((value >> 16) & 0xFFu));
        bus.write8(addr + 3, static_cast<std::uint8_t>((value >> 24) & 0xFFu));
    };

    // Act: set pin 3 as output, then drive it high
    const std::uint32_t bit3 = (1u << 3);
    write32(0x3000 + 0x00, bit3);  // DIR
    write32(0x3000 + 0x08, bit3);  // DATA_OUT

    // Assert
    EXPECT_TRUE(led.isOn());

    // Cleanup
    delete gpioDev;
}
