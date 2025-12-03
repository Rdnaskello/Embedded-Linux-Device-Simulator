#pragma once

#include <cstddef>
#include <cstdint>  // <-- додано для uint32_t
#include <memory>
#include <string>

#include "elsim/core/ICpu.hpp"

namespace elsim::core {

class FakeCpu : public ICpu {
   public:
    FakeCpu() = default;
    ~FakeCpu() override = default;

    // Основні методи CPU
    void step() override;
    void reset() override;
    bool loadImage(const std::string& path) override;
    void setMemoryBus(std::shared_ptr<IMemoryBus> bus) override;

    // Debug API
    [[nodiscard]] std::size_t stepCount() const noexcept { return stepCount_; }
    [[nodiscard]] bool imageLoaded() const noexcept { return imageLoaded_; }
    [[nodiscard]] const std::string& lastImagePath() const noexcept { return lastImagePath_; }

   private:
    // Лічильник виконаних step() — залишається з TASK-старої логіки
    std::size_t stepCount_{0};

    // Завантажене зображення (поки що не використовується реально)
    bool imageLoaded_{false};
    std::string lastImagePath_{};

    // Лічильник команд (Program Counter)
    std::uint16_t pc_{0};

    // Абстрактна шина пам'яті
    std::shared_ptr<IMemoryBus> memoryBus_{};
};

}  // namespace elsim::core
