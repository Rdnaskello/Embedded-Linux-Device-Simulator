#include <gtest/gtest.h>

#include <cstdint>
#include <memory>

#include "elsim/core/DeviceMemoryAdapter.hpp"
#include "elsim/core/GpioController.hpp"
#include "elsim/core/MemoryBus.hpp"
#include "elsim/device/GpioDevice.hpp"

namespace {

constexpr std::uint32_t GPIO_BASE = 0x00003000u;

// Register offsets
constexpr std::uint32_t REG_DIR = 0x00;
constexpr std::uint32_t REG_DATA_IN = 0x04;
constexpr std::uint32_t REG_DATA_OUT = 0x08;
constexpr std::uint32_t REG_SET = 0x0C;
constexpr std::uint32_t REG_CLR = 0x10;
constexpr std::uint32_t REG_TOGGLE = 0x14;

// Helpers: little-endian 32-bit over byte-wide bus
void write32(elsim::core::MemoryBus& bus, std::uint32_t addr, std::uint32_t v) {
    bus.write8(addr + 0, static_cast<std::uint8_t>((v >> 0) & 0xFFu));
    bus.write8(addr + 1, static_cast<std::uint8_t>((v >> 8) & 0xFFu));
    bus.write8(addr + 2, static_cast<std::uint8_t>((v >> 16) & 0xFFu));
    bus.write8(addr + 3, static_cast<std::uint8_t>((v >> 24) & 0xFFu));
}

std::uint32_t read32(elsim::core::MemoryBus& bus, std::uint32_t addr) {
    const std::uint32_t b0 = bus.read8(addr + 0);
    const std::uint32_t b1 = bus.read8(addr + 1);
    const std::uint32_t b2 = bus.read8(addr + 2);
    const std::uint32_t b3 = bus.read8(addr + 3);
    return (b0 << 0) | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

struct MappedGpio {
    std::shared_ptr<elsim::core::GpioController> ctrl;  // shared board-level GPIO controller
    std::unique_ptr<elsim::GpioDevice> gpio;            // owns the device lifetime
    std::shared_ptr<elsim::core::DeviceMemoryAdapter> mmio;
};

MappedGpio mapGpio(elsim::core::MemoryBus& bus, std::uint32_t base, std::uint32_t windowSize, std::uint32_t pinCount) {
    MappedGpio m{};

    m.ctrl = std::make_shared<elsim::core::GpioController>(pinCount);
    m.gpio = std::make_unique<elsim::GpioDevice>("gpio0", base, pinCount, m.ctrl);

    // DeviceMemoryAdapter expects raw IDevice*, so keep m.gpio alive for whole test.
    m.mmio = std::make_shared<elsim::core::DeviceMemoryAdapter>(m.gpio.get());

    // MemoryBus uses mapDevice(base, size, shared_ptr<IMemoryMappedDevice>)
    bus.mapDevice(base, windowSize, m.mmio);

    return m;
}

}  // namespace

TEST(GpioMmio, DirAndDataOutReadback) {
    elsim::core::MemoryBus bus(64 * 1024);

    auto mapped = mapGpio(bus, GPIO_BASE, 0x100, 32);

    write32(bus, GPIO_BASE + REG_DIR, 0x0000000Fu);       // pins 0..3 output
    write32(bus, GPIO_BASE + REG_DATA_OUT, 0x00000005u);  // 0b0101 on pins 0..3

    EXPECT_EQ(read32(bus, GPIO_BASE + REG_DIR), 0x0000000Fu);
    EXPECT_EQ(read32(bus, GPIO_BASE + REG_DATA_OUT), 0x00000005u);
}

TEST(GpioMmio, SetClrToggleAffectDataOut) {
    elsim::core::MemoryBus bus(64 * 1024);

    auto mapped = mapGpio(bus, GPIO_BASE, 0x100, 32);

    write32(bus, GPIO_BASE + REG_DIR, 0xFFFFFFFFu);       // all output
    write32(bus, GPIO_BASE + REG_DATA_OUT, 0x00000000u);  // start

    write32(bus, GPIO_BASE + REG_SET, 0x00000003u);  // set bits 0,1
    EXPECT_EQ(read32(bus, GPIO_BASE + REG_DATA_OUT), 0x00000003u);

    write32(bus, GPIO_BASE + REG_CLR, 0x00000001u);  // clear bit 0
    EXPECT_EQ(read32(bus, GPIO_BASE + REG_DATA_OUT), 0x00000002u);

    write32(bus, GPIO_BASE + REG_TOGGLE, 0x00000006u);              // toggle bits 1,2
    EXPECT_EQ(read32(bus, GPIO_BASE + REG_DATA_OUT), 0x00000004u);  // bit2=1, bit1 toggled off
}

TEST(GpioMmio, WriteToDataInIsIgnoredAndReadIsDeterministic) {
    elsim::core::MemoryBus bus(64 * 1024);

    auto mapped = mapGpio(bus, GPIO_BASE, 0x100, 32);

    // Attempt to write to RO register DATA_IN should not crash; value should remain deterministic.
    write32(bus, GPIO_BASE + REG_DATA_IN, 0xFFFFFFFFu);

    // With no injected inputs and DIR default = input (0), read should be 0.
    EXPECT_EQ(read32(bus, GPIO_BASE + REG_DATA_IN), 0x00000000u);
}
