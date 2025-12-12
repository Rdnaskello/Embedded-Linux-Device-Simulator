#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include "elsim/core/ICpu.hpp"

namespace elsim::core {

class FakeCpu : public ICpu {
   public:
    // === Архітектура FakeCPU згідно ISA ===
    static constexpr std::size_t kNumRegisters = 8;
    using Register = std::uint32_t;

    // Прапорці FLAGS (молодші біти регістра)
    enum class Flag : std::uint32_t {
        Zero = 1u << 0,      // Z
        Negative = 1u << 1,  // N
        Carry = 1u << 2,     // C
        Overflow = 1u << 3,  // V
    };

    // Повний архітектурний стан CPU
    struct CpuState {
        std::array<Register, kNumRegisters> regs{};  // R0–R7
        Register pc{0};                              // Program Counter
        Register sp{0};                              // Stack Pointer
        Register flags{0};                           // FLAGS
    };

    FakeCpu() = default;
    ~FakeCpu() override = default;

    // Основні методи CPU
    void step() override;
    void reset() override;
    bool loadImage(const std::string& path) override;
    void setMemoryBus(std::shared_ptr<IMemoryBus> bus) override;

    // Декодування та виконання однієї 32-бітної інструкції
    void decodeAndExecute(std::uint32_t instruction);

    // Перевірити, чи CPU перебуває в стані HALT
    [[nodiscard]] bool isHalted() const noexcept { return halted_; }

    // --- Доступ до стану CPU (для майбутніх тестів / дебагу) ---
    [[nodiscard]] const CpuState& state() const noexcept { return state_; }

    Register getRegister(std::size_t index) const;
    void setRegister(std::size_t index, Register value);

    [[nodiscard]] Register getPc() const noexcept { return state_.pc; }
    void setPc(Register value) noexcept { state_.pc = value; }

    [[nodiscard]] Register getSp() const noexcept { return state_.sp; }
    void setSp(Register value) noexcept { state_.sp = value; }

    [[nodiscard]] Register getFlags() const noexcept { return state_.flags; }
    void setFlags(Register value) noexcept { state_.flags = value; }

    void clearFlags() noexcept;
    void setFlag(Flag flag, bool value) noexcept;
    [[nodiscard]] bool isFlagSet(Flag flag) const noexcept;

    // Debug API (як було раніше)
    [[nodiscard]] std::size_t stepCount() const noexcept { return stepCount_; }
    [[nodiscard]] bool imageLoaded() const noexcept { return imageLoaded_; }
    [[nodiscard]] const std::string& lastImagePath() const noexcept { return lastImagePath_; }

   private:
    // Стартова адреса програми згідно ISA:
    // PC = 0x00000000 після reset
    static constexpr Register kProgramStart = 0x00000000u;

    // Архітектурний стан FakeCPU
    CpuState state_{};

    // Лічильник виконаних step() — залишається з існуючої логіки
    std::size_t stepCount_{0};

    // Завантажене зображення (поки що не використовується реально)
    bool imageLoaded_{false};
    std::string lastImagePath_{};

    // Тимчасовий "старий" PC, який використовується існуючою step()-логікою.
    // Ми залишаємо його, щоб не ламати поточні smoke-тести (варіант A).
    std::uint32_t pc_{0};

    // Статус HALT для нового виконуючого ядра
    bool halted_{false};

    // Хелпери для роботи з регістрами та прапорцями
    Register readReg(std::size_t index) const;
    void writeReg(std::size_t index, Register value);
    void updateZNFlags(Register value);

    // Абстрактна шина пам'яті
    std::shared_ptr<IMemoryBus> memoryBus_{};
};

}  // namespace elsim::core
