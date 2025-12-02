#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace elsim::core {

// Опис CPU на платі.
struct CpuDescription {
    std::string type;           // Наприклад: "test-cpu", "riscv32", "armv7"
    std::uint64_t frequencyHz;  // Частота в герцах
    std::string endianness;  // "little" або "big" (поки як рядок, можна буде замінити на enum)
};

// Тип регіону пам'яті (можна розширяти пізніше)
enum class MemoryType { Ram, Rom, Mmio };

// Опис одного регіону пам'яті.
struct MemoryRegion {
    std::string name;           // Наприклад: "ram", "flash"
    std::uint64_t baseAddress;  // Базова адреса
    std::uint64_t sizeBytes;    // Розмір у байтах
    MemoryType type;            // Тип пам'яті
};

// Опис одного пристрою (UART, таймер тощо).
struct DeviceDescription {
    std::string type;           // Наприклад: "uart", "timer"
    std::string name;           // Наприклад: "uart0"
    std::uint64_t baseAddress;  // Базова адреса регістрів пристрою

    // Додаткові параметри (baudrate, irq, і т.д.) у вигляді ключ-значення.
    std::map<std::string, std::string> params;
};

// Головний опис плати.
struct BoardDescription {
    std::string name;         // Назва плати
    std::string description;  // Людиночитний опис

    CpuDescription cpu;
    std::vector<MemoryRegion> memory;
    std::vector<DeviceDescription> devices;
};

}  // namespace elsim::core
