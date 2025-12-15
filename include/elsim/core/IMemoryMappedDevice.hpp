#pragma once

#include <cstdint>

// IMemoryMappedDevice
// ---------------------
// Базовий інтерфейс для всіх пристроїв, підключених до MemoryBus
// через MMIO (memory-mapped I/O).
//
// CPU бачить 1 великий адресний простір, а деякі діапазони адрес
// віддаються девайсам (регістр, таймер, UART і т.д.).
namespace elsim::core {

class IMemoryMappedDevice {
   public:
    virtual ~IMemoryMappedDevice() = default;

    // Читання 1 байта з девайса.
    // offset — це зсув від базової MMIO-адреси девайса.
    virtual std::uint8_t read8(std::uint32_t offset) = 0;

    // Запис 1 байта в девайс.
    // offset — це зсув від базової MMIO-адреси девайса.
    virtual void write8(std::uint32_t offset, std::uint8_t value) = 0;
};

}  // namespace elsim::core