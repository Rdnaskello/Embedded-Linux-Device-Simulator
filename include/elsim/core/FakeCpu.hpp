#pragma once

#include <cstddef>
#include <memory>
#include <string>

#include "elsim/core/ICpu.hpp"

namespace elsim::core {

class FakeCpu : public ICpu {
   public:
    FakeCpu() = default;
    ~FakeCpu() override = default;

    void step() override;
    void reset() override;
    bool loadImage(const std::string& path) override;
    void setMemoryBus(std::shared_ptr<IMemoryBus> bus) override;

    // Невеликий debug-API, щоб потім було зручно тестувати
    [[nodiscard]] std::size_t stepCount() const noexcept { return stepCount_; }
    [[nodiscard]] bool imageLoaded() const noexcept { return imageLoaded_; }
    [[nodiscard]] const std::string& lastImagePath() const noexcept { return lastImagePath_; }

   private:
    std::size_t stepCount_{0};
    bool imageLoaded_{false};
    std::string lastImagePath_{};

    std::shared_ptr<IMemoryBus> memoryBus_{};
};

}  // namespace elsim::core