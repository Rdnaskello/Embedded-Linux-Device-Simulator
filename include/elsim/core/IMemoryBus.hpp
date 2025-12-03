#pragma once

#include <cstdint>

namespace elsim::core {

class IMemoryBus {
   public:
    virtual ~IMemoryBus() = default;

    // Читання 1 байта з глобальної адреси.
    virtual std::uint8_t read8(std::uint32_t address) = 0;

    // Запис 1 байта в глобальну адресу.
    virtual void write8(std::uint32_t address, std::uint8_t value) = 0;
};

}  // namespace elsim::core
