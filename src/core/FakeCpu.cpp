#include "elsim/core/FakeCpu.hpp"

#include <algorithm>  // std::fill
#include <cstdint>

#include "elsim/core/IMemoryBus.hpp"

namespace elsim::core {

// --- Доступ до регістрів ---

FakeCpu::Register FakeCpu::getRegister(std::size_t index) const {
    // Проста перевірка меж. У майбутньому можна замінити на assert або виняток.
    if (index >= kNumRegisters) {
        return 0;
    }
    return state_.regs[index];
}

void FakeCpu::setRegister(std::size_t index, Register value) {
    if (index >= kNumRegisters) {
        return;
    }
    state_.regs[index] = value;
}

// --- Операції з FLAGS ---

void FakeCpu::clearFlags() noexcept { state_.flags = 0; }

void FakeCpu::setFlag(Flag flag, bool value) noexcept {
    auto bit = static_cast<std::uint32_t>(flag);
    if (value) {
        state_.flags |= bit;
    } else {
        state_.flags &= ~bit;
    }
}

bool FakeCpu::isFlagSet(Flag flag) const noexcept {
    auto bit = static_cast<std::uint32_t>(flag);
    return (state_.flags & bit) != 0;
}

// --- Основна логіка CPU ---

void FakeCpu::step() {
    // Рахуємо кроки — це важливо для smoke-тестів
    ++stepCount_;

    // Якщо шина пам'яті не підключена — поводимось як раніше (тільки лічимо)
    if (!memoryBus_) {
        return;
    }

    // Проста тестова логіка:
    // 1. читаємо байт з адреси pc_
    // 2. збільшуємо його на 1
    // 3. записуємо назад
    // 4. переходимо до наступної адреси
    //
    // ВАЖЛИВО: тут ми все ще використовуємо старий pc_,
    // щоб не зламати існуючі smoke-тести (варіант A).
    std::uint8_t value = memoryBus_->read8(pc_);
    value = static_cast<std::uint8_t>(value + 1U);
    memoryBus_->write8(pc_, value);

    ++pc_;
}

void FakeCpu::reset() {
    // Скидаємо службові поля
    stepCount_ = 0;
    imageLoaded_ = false;
    lastImagePath_.clear();

    // Скидаємо архітектурний стан згідно ISA:
    // R0..R7 = 0
    std::fill(state_.regs.begin(), state_.regs.end(), 0u);

    // PC = початок програми
    state_.pc = kProgramStart;

    // SP = 0 (зарезервований, але не використовується у v0.2)
    state_.sp = 0u;

    // FLAGS = 0 (Z = N = C = V = 0)
    state_.flags = 0u;

    // Для сумісності з поточною step()-логікою тримаємо pc_ синхронізованим
    pc_ = state_.pc;
}

bool FakeCpu::loadImage(const std::string& path) {
    lastImagePath_ = path;
    imageLoaded_ = !path.empty();
    return imageLoaded_;
}

void FakeCpu::setMemoryBus(std::shared_ptr<IMemoryBus> bus) { memoryBus_ = std::move(bus); }

}  // namespace elsim::core
