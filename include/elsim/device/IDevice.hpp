#pragma once

#include <cstdint>

namespace elsim {

class IDevice {
   public:
    virtual ~IDevice() = default;

    // Читання одного байта з регістру/офсету всередині пристрою.
    virtual std::uint8_t read(std::uint32_t offset) = 0;

    // Запис одного байта у регістр/офсет всередині пристрою.
    virtual void write(std::uint32_t offset, std::uint8_t value) = 0;

    // Один "крок часу" для пристрою (оновлення внутрішнього стану).
    virtual void tick() = 0;
};

}  // namespace elsim
