#include "elsim/core/FakeCpu.hpp"

#include <cstdint>

#include "elsim/core/IMemoryBus.hpp"

namespace elsim::core {

void FakeCpu::step() {
    // Рахуємо кроки — це важливо для smoke-тестів
    ++stepCount_;

    // Якщо шина пам'яті не підключена — поводимось як раніше (тільки лічимо)
    if (!memoryBus_) {
        return;
    }

    // Проста тестова логіка:
    // 1. читаємо байт з адреси PC
    // 2. збільшуємо його на 1
    // 3. записуємо назад
    // 4. переходимо до наступної адреси

    std::uint8_t value = memoryBus_->read8(pc_);
    value = static_cast<std::uint8_t>(value + 1U);
    memoryBus_->write8(pc_, value);

    ++pc_;
}

void FakeCpu::reset() {
    stepCount_ = 0;
    imageLoaded_ = false;
    lastImagePath_.clear();
    pc_ = 0;  // важливо для повторного запуску симуляції
}

bool FakeCpu::loadImage(const std::string& path) {
    lastImagePath_ = path;
    imageLoaded_ = !path.empty();
    return imageLoaded_;
}

void FakeCpu::setMemoryBus(std::shared_ptr<IMemoryBus> bus) { memoryBus_ = std::move(bus); }

}  // namespace elsim::core
