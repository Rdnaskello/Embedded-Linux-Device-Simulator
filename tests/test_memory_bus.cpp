#include <gtest/gtest.h>

#include <cstdint>
#include <memory>

#include "elsim/core/IMemoryMappedDevice.hpp"
#include "elsim/core/MemoryBus.hpp"

namespace {

// Tiny fake MMIO device for negative contract tests.
// Contract for this fake device:
// - offset 0x00: RO register, always returns 0xAB
// - offset 0x04: RW register, stores last written byte
// - any other offset: "unknown" -> returns 0xFF, writes ignored
class FakeMmioDevice final : public elsim::core::IMemoryMappedDevice {
   public:
    std::uint8_t read8(std::uint32_t offset) override {
        if (offset == 0x00) {
            return ro_value_;
        }
        if (offset == 0x04) {
            return rw_value_;
        }
        return 0xFF;
    }

    void write8(std::uint32_t offset, std::uint8_t value) override {
        if (offset == 0x00) {
            // RO: ignore writes
            ro_write_attempts_++;
            return;
        }
        if (offset == 0x04) {
            rw_value_ = value;
            return;
        }
        // unknown: ignore
        unknown_write_attempts_++;
    }

    int roWriteAttempts() const { return ro_write_attempts_; }
    int unknownWriteAttempts() const { return unknown_write_attempts_; }

   private:
    std::uint8_t ro_value_ = 0xAB;
    std::uint8_t rw_value_ = 0x00;
    int ro_write_attempts_ = 0;
    int unknown_write_attempts_ = 0;
};

}  // namespace

TEST(MmioContract, WriteToReadOnlyRegister_IsIgnoredAndDoesNotCrash) {
    elsim::core::MemoryBus bus(/*ram_size=*/256);

    auto dev = std::make_shared<FakeMmioDevice>();
    constexpr std::uint32_t kBase = 0x1000;
    constexpr std::uint32_t kSize = 0x100;  // map enough space so "unknown offsets" still route to device
    bus.mapDevice(kBase, kSize, dev);

    // initial RO value
    const auto before = bus.read8(kBase + 0x00);
    ASSERT_EQ(before, 0xAB);

    // attempt to write to RO
    bus.write8(kBase + 0x00, 0x11);

    // RO must remain unchanged
    const auto after = bus.read8(kBase + 0x00);
    ASSERT_EQ(after, 0xAB);

    // confirm device noticed an RO write attempt (optional but useful)
    ASSERT_EQ(dev->roWriteAttempts(), 1);
}

TEST(MmioContract, ReadFromUnknownOffset_ReturnsDeterministicDefaultAndDoesNotCrash) {
    elsim::core::MemoryBus bus(/*ram_size=*/256);

    auto dev = std::make_shared<FakeMmioDevice>();
    constexpr std::uint32_t kBase = 0x2000;
    constexpr std::uint32_t kSize = 0x100;
    bus.mapDevice(kBase, kSize, dev);

    // unknown offset read should return device-defined default (0xFF here)
    const auto v = bus.read8(kBase + 0x10);
    ASSERT_EQ(v, 0xFF);

    // unknown offset write is ignored (and should not crash)
    bus.write8(kBase + 0x10, 0x22);
    ASSERT_EQ(dev->unknownWriteAttempts(), 1);
}
