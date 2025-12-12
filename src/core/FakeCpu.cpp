#include "elsim/core/FakeCpu.hpp"

#include <algorithm>  // std::fill
#include <cstdint>
#include <sstream>  // std::ostringstream

#include "elsim/core/IMemoryBus.hpp"
#include "elsim/core/Logger.hpp"

namespace elsim::core {

// === Допоміжні хелпери для регістрів та прапорців ===

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

FakeCpu::Register FakeCpu::readReg(std::size_t index) const { return getRegister(index); }

void FakeCpu::writeReg(std::size_t index, Register value) { setRegister(index, value); }

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

void FakeCpu::updateZNFlags(Register value) {
    // Оновлюємо Zero та Negative, не чіпаючи Carry/Overflow
    setFlag(Flag::Zero, value == 0);

    // Negative — інтерпретуємо як signed 32-бітне число
    auto signedValue = static_cast<std::int32_t>(value);
    setFlag(Flag::Negative, signedValue < 0);
}

// --- Декодування та виконання інструкцій ---

void FakeCpu::decodeAndExecute(std::uint32_t instruction) {
    // Якщо CPU вже в HALT — нічого не робимо
    if (halted_) {
        Logger::instance().debug("CPU", "decodeAndExecute called while HALTED — ignoring instruction");
        return;
    }

    // Витягуємо opcode (старший байт)
    const std::uint8_t opcode = static_cast<std::uint8_t>(instruction >> 24);

    // Розбір полів аргументів (для ALU-інструкцій)
    const std::uint32_t rdIndex = (instruction >> 21) & 0x7u;  // 3 біти
    const std::uint32_t rsIndex = (instruction >> 18) & 0x7u;  // 3 біти
    const bool isImm = ((instruction >> 17) & 0x1u) != 0;
    const std::int16_t imm16 = static_cast<std::int16_t>(instruction & 0xFFFFu);

    // Невеликий хелпер для sign-extend 16-бітного значення до 32 біт
    const auto signExtendImm16 = [imm16]() -> Register {
        return static_cast<Register>(static_cast<std::int32_t>(imm16));
    };

    // Лог поточного інструкшена (opcode + PC)
    {
        std::ostringstream oss;
        oss << "Executing instruction: opcode=0x" << std::hex << static_cast<int>(opcode) << " PC=0x" << state_.pc
            << " Rd=" << std::dec << rdIndex << " Rs=" << rsIndex << " isImm=" << (isImm ? 1 : 0) << " imm16=" << imm16;
        Logger::instance().debug("CPU", oss.str());
    }

    // Опкоди
    constexpr std::uint8_t OPC_NOP = 0x00;
    constexpr std::uint8_t OPC_MOV = 0x01;
    constexpr std::uint8_t OPC_ADD = 0x02;
    constexpr std::uint8_t OPC_SUB = 0x03;
    constexpr std::uint8_t OPC_HALT = 0xFF;

    switch (opcode) {
        case OPC_NOP: {
            Logger::instance().debug("CPU", "NOP");
            // Нічого не робимо, просто рухаємо PC
            state_.pc += 4;
            break;
        }

        case OPC_MOV: {
            // MOV Rd, Rs  або  MOV Rd, #imm16
            Register src = 0;

            if (isImm) {
                src = signExtendImm16();
                std::ostringstream oss;
                oss << "MOV R" << rdIndex << ", #" << src;
                Logger::instance().debug("CPU", oss.str());
            } else {
                src = readReg(rsIndex);
                std::ostringstream oss;
                oss << "MOV R" << rdIndex << ", R" << rsIndex << " (src=" << src << ")";
                Logger::instance().debug("CPU", oss.str());
            }

            writeReg(rdIndex, src);
            updateZNFlags(src);
            state_.pc += 4;
            break;
        }

        case OPC_ADD: {
            // ADD Rd, Rs  або  ADD Rd, #imm16
            const Register lhs = readReg(rdIndex);
            Register rhs = 0;

            if (isImm) {
                rhs = signExtendImm16();
            } else {
                rhs = readReg(rsIndex);
            }

            const Register result = static_cast<Register>(lhs + rhs);

            {
                std::ostringstream oss;
                if (isImm) {
                    oss << "ADD R" << rdIndex << ", #" << rhs << " (old=" << lhs << ", new=" << result << ")";
                } else {
                    oss << "ADD R" << rdIndex << ", R" << rsIndex << " (" << lhs << " + " << rhs << " = " << result
                        << ")";
                }
                Logger::instance().debug("CPU", oss.str());
            }

            writeReg(rdIndex, result);
            updateZNFlags(result);
            // TODO: за потреби пізніше оновити Carry/Overflow
            state_.pc += 4;
            break;
        }

        case OPC_SUB: {
            // SUB Rd, Rs  або  SUB Rd, #imm16
            const Register lhs = readReg(rdIndex);
            Register rhs = 0;

            if (isImm) {
                rhs = signExtendImm16();
            } else {
                rhs = readReg(rsIndex);
            }

            const Register result = static_cast<Register>(lhs - rhs);

            {
                std::ostringstream oss;
                if (isImm) {
                    oss << "SUB R" << rdIndex << ", #" << rhs << " (old=" << lhs << ", new=" << result << ")";
                } else {
                    oss << "SUB R" << rdIndex << ", R" << rsIndex << " (" << lhs << " - " << rhs << " = " << result
                        << ")";
                }
                Logger::instance().debug("CPU", oss.str());
            }

            writeReg(rdIndex, result);
            updateZNFlags(result);
            // TODO: за потреби пізніше оновити Carry/Overflow
            state_.pc += 4;
            break;
        }

        case OPC_HALT: {
            Logger::instance().debug("CPU", "HALT");
            // Переводимо CPU в стан HALT. PC залишаємо як є.
            halted_ = true;
            break;
        }

        default: {
            // Невідомий opcode — поводимось як NOP, щоб не зависнути назавжди.
            std::ostringstream oss;
            oss << "Unknown opcode 0x" << std::hex << static_cast<int>(opcode) << " — treating as NOP";
            Logger::instance().debug("CPU", oss.str());
            state_.pc += 4;
            break;
        }
    }
}

// --- Основна логіка CPU (поки що стара, байтово-інкрементна) ---

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
    halted_ = false;

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
