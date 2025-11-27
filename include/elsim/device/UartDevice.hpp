#pragma once

#include <cstdint>

#include "BaseDevice.hpp"

namespace elsim {

// Простий шаблон UART-пристрою
class UartDevice : public BaseDevice {
   public:
    // Конструктор: задаємо імʼя, адресу та розмір регістрів
    UartDevice(std::uint32_t baseAddress) : BaseDevice("UART", baseAddress, RegisterSize) {}

    // Карта регістрів UART
    enum Registers : std::uint32_t {
        REG_DATA = 0x00,    // TX / RX data
        REG_STATUS = 0x04,  // флаги UART (TX_ready, RX_ready)
        REG_BAUD = 0x08,    // конфігурація baud rate
    };

    // Розмір регістрів UART у байтах
    static constexpr std::uint32_t RegisterSize = 0x0C;

    // Перевизначення логіки читання/запису
    std::uint8_t read(std::uint32_t offset) override;
    void write(std::uint32_t offset, std::uint8_t value) override;
    void tick() override;

   private:
    // Пізніше тут будуть внутрішні поля:
    //  - TX buffer
    //  - RX buffer
    //  - status flags
    //  - baud rate
};

}  // namespace elsim
