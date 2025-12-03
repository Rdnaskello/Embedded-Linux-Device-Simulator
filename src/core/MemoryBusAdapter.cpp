#include "elsim/core/MemoryBusAdapter.hpp"

#include <stdexcept>

namespace elsim::core {

std::uint8_t MemoryBusAdapter::read8(std::uint32_t address) {
    if (!bus_) {
        throw std::runtime_error("MemoryBusAdapter::read8: underlying MemoryBus is null");
    }
    return bus_->read8(address);
}

void MemoryBusAdapter::write8(std::uint32_t address, std::uint8_t value) {
    if (!bus_) {
        throw std::runtime_error("MemoryBusAdapter::write8: underlying MemoryBus is null");
    }
    bus_->write8(address, value);
}

}  // namespace elsim::core
