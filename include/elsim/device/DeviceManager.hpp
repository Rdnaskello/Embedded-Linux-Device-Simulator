#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "IDevice.hpp"

namespace elsim {

class DeviceManager {
   public:
    using DevicePtr = std::unique_ptr<IDevice>;

    DeviceManager() = default;
    ~DeviceManager() = default;

    // Додає новий пристрій під керування менеджера
    void registerDevice(DevicePtr device);

    // Пошук пристрою за глобальною адресою (через baseAddress/size)
    IDevice* findDeviceByAddress(std::uint32_t address) const;

    // Читання / запис за глобальною адресою (делегується потрібному девайсу)
    std::uint8_t read(std::uint32_t address);
    void write(std::uint32_t address, std::uint8_t value);

    // Виклик tick() для всіх зареєстрованих пристроїв
    void tickAll();

   private:
    std::vector<DevicePtr> m_devices;
};

}  // namespace elsim
