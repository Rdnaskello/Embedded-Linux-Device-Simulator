#include "elsim/core/MemoryBus.hpp"

#include <stdexcept>  // std::out_of_range, std::invalid_argument, std::runtime_error

// Конструктор: виділяємо RAM заданого розміру й заповнюємо нулями.
MemoryBus::MemoryBus(std::size_t size) : m_memory(size, 0U) {}

// Пошук MMIO-девайса за глобальною адресою.
const MemoryBus::MappedDevice* MemoryBus::findDevice(std::uint32_t address) const {
    // Лінійний пошук по вектору девайсів.
    // На цьому етапі цього достатньо (потім можна оптимізувати).
    for (const auto& dev : m_devices) {
        // Діапазон девайса: [dev.base, dev.base + dev.size)
        if (address >= dev.base && address < dev.base + dev.size) {
            return &dev;
        }
    }
    return nullptr;
}

// Читання 1 байта з глобальної адреси.
std::uint8_t MemoryBus::read8(std::uint32_t address) const {
    // 1. Спершу перевіряємо, чи адреса належить MMIO-девайсу.
    if (const auto* mapped = findDevice(address)) {
        const auto offset = address - mapped->base;  // локальний зсув у девайсі
        return mapped->device->read8(offset);        // MMIO path
    }

    // 2. Якщо девайс не знайдено — працюємо з RAM.
    if (address >= m_memory.size()) {
        // Адреса поза межами RAM → кидаємо виняток.
        throw std::out_of_range("MemoryBus::read8: address out of range");
    }

    return m_memory[address];  // RAM path
}

// Запис 1 байта в глобальну адресу.
void MemoryBus::write8(std::uint32_t address, std::uint8_t value) {
    // 1. Спершу шукаємо MMIO-девайс.
    if (const auto* mapped = findDevice(address)) {
        const auto offset = address - mapped->base;  // локальний зсув у девайсі
        mapped->device->write8(offset, value);       // MMIO path
        return;
    }

    // 2. Якщо девайса немає — пишемо в RAM.
    if (address >= m_memory.size()) {
        // Адреса поза межами RAM → це помилка.
        throw std::out_of_range("MemoryBus::write8: address out of range");
    }

    m_memory[address] = value;  // RAM path
}

// Підключення MMIO-девайса до шини пам'яті.
void MemoryBus::mapDevice(std::uint32_t baseAddress, std::uint32_t size, std::shared_ptr<IMemoryMappedDevice> device) {
    if (!device) {
        throw std::invalid_argument("MemoryBus::mapDevice: device is null");
    }

    // Перевірка перекриття регіонів.
    //
    // Новий діапазон: [baseAddress, baseAddress + size)
    const auto newEnd = baseAddress + size;

    for (const auto& dev : m_devices) {
        // Існуючий діапазон: [dev.base, dev.base + dev.size)
        const auto end = dev.base + dev.size;

        // Перекриття Є, якщо:
        //  !(праворуч без перетину || ліворуч без перетину)
        const bool overlap = !(newEnd <= dev.base || baseAddress >= end);

        if (overlap) {
            throw std::runtime_error("MemoryBus::mapDevice: overlapping device region");
        }
    }

    // Якщо перекриття немає — додаємо девайс.
    m_devices.push_back(MappedDevice{baseAddress, size, std::move(device)});
}
