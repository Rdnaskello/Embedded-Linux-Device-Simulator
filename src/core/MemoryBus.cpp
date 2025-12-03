#include "elsim/core/MemoryBus.hpp"

#include <cstdio>
#include <stdexcept>  // std::out_of_range, std::invalid_argument, std::runtime_error
#include <string>
#include <string_view>

#include "elsim/core/Logger.hpp"

namespace {

constexpr std::string_view COMPONENT = "MMIO";

}  // namespace

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
    auto& logger = elsim::core::Logger::instance();

    // 1. Спершу перевіряємо, чи адреса належить MMIO-девайсу.
    if (const auto* mapped = findDevice(address)) {
        const auto offset = address - mapped->base;        // локальний зсув у девайсі
        const auto value = mapped->device->read8(offset);  // MMIO path

        char buf[128];
        std::snprintf(buf, sizeof(buf), "READ dev_base=0x%08X addr=0x%08X offset=0x%08X size=1 -> 0x%02X", mapped->base,
                      address, offset, static_cast<unsigned int>(value));
        logger.debug(COMPONENT, buf);

        return value;
    }

    // 2. Якщо девайс не знайдено — працюємо з RAM.
    if (address >= m_memory.size()) {
        // Адреса поза межами RAM → кидаємо виняток.
        char buf[128];
        std::snprintf(buf, sizeof(buf), "READ out-of-range addr=0x%08X size=1 (ram_size=%zu)", address,
                      m_memory.size());
        logger.error(COMPONENT, buf);

        throw std::out_of_range("MemoryBus::read8: address out of range");
    }

    const auto value = m_memory[address];  // RAM path

    char buf[128];
    std::snprintf(buf, sizeof(buf), "READ RAM addr=0x%08X size=1 -> 0x%02X", address, static_cast<unsigned int>(value));
    logger.debug(COMPONENT, buf);

    return value;
}

// Запис 1 байта в глобальну адресу.
void MemoryBus::write8(std::uint32_t address, std::uint8_t value) {
    auto& logger = elsim::core::Logger::instance();

    // 1. Спершу шукаємо MMIO-девайс.
    if (const auto* mapped = findDevice(address)) {
        const auto offset = address - mapped->base;  // локальний зсув у девайсі

        char buf[128];
        std::snprintf(buf, sizeof(buf), "WRITE dev_base=0x%08X addr=0x%08X offset=0x%08X size=1 value=0x%02X",
                      mapped->base, address, offset, static_cast<unsigned int>(value));
        logger.debug(COMPONENT, buf);

        mapped->device->write8(offset, value);  // MMIO path
        return;
    }

    // 2. Якщо девайса немає — пишемо в RAM.
    if (address >= m_memory.size()) {
        // Адреса поза межами RAM → це помилка.
        char buf[128];
        std::snprintf(buf, sizeof(buf), "WRITE out-of-range addr=0x%08X size=1 value=0x%02X (ram_size=%zu)", address,
                      static_cast<unsigned int>(value), m_memory.size());
        logger.error(COMPONENT, buf);

        throw std::out_of_range("MemoryBus::write8: address out of range");
    }

    char buf[128];
    std::snprintf(buf, sizeof(buf), "WRITE RAM addr=0x%08X size=1 value=0x%02X", address,
                  static_cast<unsigned int>(value));
    logger.debug(COMPONENT, buf);

    m_memory[address] = value;  // RAM path
}

// Підключення MMIO-девайса до шини пам'яті.
void MemoryBus::mapDevice(std::uint32_t baseAddress, std::uint32_t size, std::shared_ptr<IMemoryMappedDevice> device) {
    auto& logger = elsim::core::Logger::instance();

    if (!device) {
        logger.error(COMPONENT, "mapDevice called with null device pointer");
        throw std::invalid_argument("MemoryBus::mapDevice: device is null");
    }

    // Новий діапазон: [baseAddress, baseAddress + size)
    const auto newEnd = baseAddress + size;

    // Перевірка перекриття регіонів.
    for (const auto& dev : m_devices) {
        // Існуючий діапазон: [dev.base, dev.base + dev.size)
        const auto end = dev.base + dev.size;

        const bool overlap = !(newEnd <= dev.base || baseAddress >= end);

        if (overlap) {
            char buf[160];
            std::snprintf(buf, sizeof(buf),
                          "Overlapping device region: new [0x%08X..0x%08X) conflicts with "
                          "existing [0x%08X..0x%08X)",
                          baseAddress, newEnd, dev.base, end);
            logger.error(COMPONENT, buf);

            throw std::runtime_error("MemoryBus::mapDevice: overlapping device region");
        }
    }

    char buf[128];
    std::snprintf(buf, sizeof(buf), "Map device region [0x%08X..0x%08X) size=0x%X", baseAddress, newEnd, size);
    logger.debug(COMPONENT, buf);

    // Якщо перекриття немає — додаємо девайс.
    m_devices.push_back(MappedDevice{baseAddress, size, std::move(device)});
}
