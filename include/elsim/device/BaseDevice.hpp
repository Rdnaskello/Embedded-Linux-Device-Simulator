#pragma once

#include <cstdint>
#include <string>

#include "IDevice.hpp"

namespace elsim {

class BaseDevice : public IDevice {
   public:
    BaseDevice(std::string name, std::uint32_t baseAddress, std::uint32_t size)
        : m_name(std::move(name)), m_baseAddress(baseAddress), m_size(size) {}

    virtual ~BaseDevice() = default;

    // Ім'я пристрою (для логів)
    const std::string& name() const { return m_name; }

    // Базова адреса девайсу в адресному просторі
    std::uint32_t baseAddress() const { return m_baseAddress; }

    // Розмір адресного простору девайсу
    std::uint32_t size() const { return m_size; }

    // Перевірка, чи належить адреса цьому пристрою
    bool containsAddress(std::uint32_t addr) const { return addr >= m_baseAddress && addr < (m_baseAddress + m_size); }

    //
    // Реалізації методів інтерфейсу — поки що порожні
    // Майбутні пристрої ПЕРЕЗАПИСУЮТЬ ці методи
    //

    std::uint8_t read(std::uint32_t offset) override {
        // Тимчасова заглушка
        // (Пізніше можемо додати лог або exception)
        return 0;
    }

    void write(std::uint32_t offset, std::uint8_t value) override {
        // Заглушка — нічого не робить
    }

    void tick() override {
        // Порожнє — більшість пристроїв будуть мати свою логіку
    }

   protected:
    std::string m_name;
    std::uint32_t m_baseAddress;
    std::uint32_t m_size;
};

}  // namespace elsim
