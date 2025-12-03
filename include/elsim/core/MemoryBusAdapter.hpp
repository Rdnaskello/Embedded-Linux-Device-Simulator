#pragma once

#include <cstdint>

#include "elsim/core/IMemoryBus.hpp"
#include "elsim/core/MemoryBus.hpp"

namespace elsim::core {

// Тонка обгортка над MemoryBus, яку бачить CPU через IMemoryBus.
class MemoryBusAdapter : public IMemoryBus {
   public:
    explicit MemoryBusAdapter(MemoryBus* bus) : bus_(bus) {}

    std::uint8_t read8(std::uint32_t address) override;
    void write8(std::uint32_t address, std::uint8_t value) override;

   private:
    // Не володіємо MemoryBus, просто вказівник.
    MemoryBus* bus_{nullptr};
};

}  // namespace elsim::core
