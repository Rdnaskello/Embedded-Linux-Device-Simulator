#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "elsim/core/IMemoryMappedDevice.hpp"

// MemoryBus
// ---------
// Модуль пам'яті, який поєднує:
//  - звичайну RAM (масив байтів),
//  - набір MMIO-девайсів, змеплених у певні діапазони адрес.
//
// При читанні/записі байта:
//  1. Спочатку шукається MMIO-девайс, що закриває адресу.
//  2. Якщо девайс знайдено — делегуємо операцію йому.
//  3. Якщо ні — працюємо з RAM.
//  4. Якщо адреса за межами RAM — кидаємо exception.
class MemoryBus {
   public:
    // Створюємо шину з заданим розміром RAM у байтах.
    explicit MemoryBus(std::size_t size);

    // Читання 1 байта з глобальної адреси.
    std::uint8_t read8(std::uint32_t address) const;

    // Запис 1 байта в глобальну адресу.
    void write8(std::uint32_t address, std::uint8_t value);

    // Підключити MMIO-девайс до діапазону [baseAddress, baseAddress + size).
    //
    // Вимоги:
    //  - device != nullptr
    //  - діапазон не перетинається з уже змепленими девайсами
    void mapDevice(std::uint32_t baseAddress, std::uint32_t size, std::shared_ptr<IMemoryMappedDevice> device);

   private:
    // Внутрішній опис підключеного девайса.
    struct MappedDevice {
        std::uint32_t base;  // базова адреса в глобальному адресному просторі
        std::uint32_t size;  // розмір регіону
        std::shared_ptr<IMemoryMappedDevice> device;
    };

    // Власне RAM (суцільний байтовий буфер).
    std::vector<std::uint8_t> m_memory;

    // Список усіх MMIO-девайсів.
    std::vector<MappedDevice> m_devices;

    // Пошук девайса за глобальною адресою.
    // Повертає вказівник на MappedDevice або nullptr, якщо не знайдено.
    const MappedDevice* findDevice(std::uint32_t address) const;
};
