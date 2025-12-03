#include <cassert>
#include <cstdint>
#include <iostream>

#include "elsim/core/FakeCpu.hpp"
#include "elsim/core/MemoryBus.hpp"
#include "elsim/core/MemoryBusAdapter.hpp"

using namespace elsim::core;

int main() {
    std::cout << "[basic_cpu_memory_test] Starting...\n";

    // 1. Створюємо RAM + шину
    MemoryBus bus(64);  // 64 байти RAM достатньо для тесту
    auto busAdapter = std::make_shared<MemoryBusAdapter>(&bus);

    // 2. Створюємо FakeCpu і підключаємо шину
    FakeCpu cpu;
    cpu.setMemoryBus(busAdapter);

    // Початково RAM має бути вся з нулями
    for (std::uint32_t addr = 0; addr < 5; ++addr) {
        assert(bus.read8(addr) == 0);
    }

    // 3. Виконаємо 5 кроків CPU
    constexpr int kSteps = 5;
    for (int i = 0; i < kSteps; ++i) {
        cpu.step();
    }

    // Тепер FakeCpu мав:
    // PC: проходить адреси 0..4 і у кожній клітинці робить value = value + 1
    // Отже, після 5 кроків RAM[0..4] == 1
    for (std::uint32_t addr = 0; addr < 5; ++addr) {
        std::uint8_t value = bus.read8(addr);
        std::cout << "RAM[" << addr << "] = " << static_cast<int>(value) << "\n";
        assert(value == 1);
    }

    std::cout << "[basic_cpu_memory_test] OK: CPU read/write via IMemoryBus works as expected.\n";
    return 0;
}
