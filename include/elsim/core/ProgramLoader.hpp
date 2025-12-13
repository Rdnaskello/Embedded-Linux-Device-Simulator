#pragma once

#include <cstdint>
#include <string>

#include "elsim/core/MemoryBus.hpp"

namespace elsim::core {
/**
 * Константи та структура для роботи з форматом elsim-bin (v0).
 *
 * Формат заголовка описаний у docs/elsim_binary_format.md:
 *  - magic       : 'ELSB' як uint32_t у little-endian (0x42534C45)
 *  - entry_point : стартова адреса виконання (PC) І база завантаження коду
 *  - code_size   : розмір блоку коду в байтах (без заголовка)
 */
inline constexpr std::uint32_t ELSIM_BINARY_MAGIC = 0x42534C45;  // 'ELSB'

#pragma pack(push, 1)
struct ElsimBinaryHeader {
    std::uint32_t magic;  ///< Магічне число формату ('ELSB').
    std::uint32_t entry_point;  ///< Стартова адреса виконання (PC) і база завантаження коду.
    std::uint32_t code_size;  ///< Розмір блоку машинного коду в байтах (після заголовка).
};
#pragma pack(pop)

static_assert(sizeof(ElsimBinaryHeader) == 12, "ElsimBinaryHeader must be exactly 12 bytes");

/**
 * ProgramLoader відповідає за завантаження файлів формату elsim-bin у MemoryBus.
 *
 * Завдання:
 *  - прочитати заголовок ElsimBinaryHeader з файлу;
 *  - провалідувати magic та code_size;
 *  - прочитати code_size байт коду;
 *  - записати байти коду в память через MemoryBus, починаючи з адреси entry_point;
 *  - повернути entryPoint через вихідний параметр для подальшого встановлення PC у CPU.
 *
 * У разі помилок (файл не відкрився, некоректний формат, обрізаний код,
 * вихід за межі пам'яті під час запису) метод кидає std::runtime_error
 * або похідні std::exception.
 */
class ProgramLoader {
   public:
    ProgramLoader() = default;

    /**
     * Завантажує виконуваний файл формату elsim-bin у пам'ять.
     *
     * @param path       Шлях до файлу .elsim-bin.
     * @param memory     Посилання на MemoryBus, у який буде записано машинний код.
     * @param entryPoint Вихідний параметр: стартова адреса виконання (PC),
     *                   зчитана з поля header.entry_point.
     *
     * @throws std::runtime_error у разі помилки вводу/виводу або некоректного формату.
     * @throws std::out_of_range, якщо MemoryBus::write8 викине виняток при виході за межі RAM.
     */
    void loadBinary(const std::string& path, MemoryBus& memory, std::uint32_t& entryPoint);
};

}  // namespace elsim::core
