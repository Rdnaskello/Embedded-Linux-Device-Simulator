#include <array>
#include <cstdint>
#include <iostream>
#include <memory>

#include "elsim/core/IMemoryMappedDevice.hpp"
#include "elsim/core/MemoryBus.hpp"
using elsim::core::IMemoryMappedDevice;
using elsim::core::MemoryBus;
// Простий MMIO-девайс на 16 байт.
// Використовується лише для перевірки MemoryBus.
class TestDevice : public IMemoryMappedDevice {
   public:
    std::uint8_t read8(std::uint32_t offset) override {
        if (offset >= storage_.size()) {
            return 0xFF;
        }
        return storage_[offset];
    }

    void write8(std::uint32_t offset, std::uint8_t value) override {
        if (offset >= storage_.size()) {
            return;
        }
        storage_[offset] = value;
    }

   private:
    std::array<std::uint8_t, 16> storage_{};  // zero-initialized
};

int main() {
    try {
        // 64 KiB RAM
        MemoryBus bus(64 * 1024);

        // --- Test 1: RAM ---
        bus.write8(0x10, 0x42);
        auto ramValue = bus.read8(0x10);

        std::cout << "RAM test value: 0x" << std::hex << static_cast<int>(ramValue) << std::dec << '\n';

        // --- Test 2: MMIO ---
        auto device = std::make_shared<TestDevice>();
        constexpr std::uint32_t mmioBase = 0x1000;
        constexpr std::uint32_t mmioSize = 16;

        bus.mapDevice(mmioBase, mmioSize, device);

        bus.write8(mmioBase + 4, 0xAB);
        auto mmioValue = bus.read8(mmioBase + 4);

        std::cout << "MMIO test value: 0x" << std::hex << static_cast<int>(mmioValue) << std::dec << '\n';

        if (ramValue == 0x42 && mmioValue == 0xAB) {
            std::cout << "MemoryBus basic tests passed.\n";
            return 0;
        }

        std::cout << "MemoryBus basic tests FAILED.\n";
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << '\n';
        return 1;
    }
}
