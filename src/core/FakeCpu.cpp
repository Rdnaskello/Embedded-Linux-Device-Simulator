#include "elsim/core/FakeCpu.hpp"

namespace elsim::core {

void FakeCpu::step() {
    // Мінімальна поведінка: просто рахуємо кроки.
    ++stepCount_;

    // Пізніше тут буде:
    // - зчитування інструкції з пам'яті
    // - декодування
    // - виконання
    // - оновлення PC
    // На цьому етапі нам це не потрібно.
}

void FakeCpu::reset() {
    stepCount_ = 0;
    imageLoaded_ = false;
    lastImagePath_.clear();
    // CPU залишає посилання на memoryBus_, ми його не чіпаємо.
}

bool FakeCpu::loadImage(const std::string& path) {
    // Поки що просто зберігаємо шлях і вважаємо, що все ок.
    lastImagePath_ = path;
    imageLoaded_ = !path.empty();
    return imageLoaded_;
}

void FakeCpu::setMemoryBus(std::shared_ptr<IMemoryBus> bus) { memoryBus_ = std::move(bus); }

}  // namespace elsim::core
