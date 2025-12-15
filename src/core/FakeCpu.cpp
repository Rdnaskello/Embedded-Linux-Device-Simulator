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

// --- Допоміжні функції роботи з пам'яттю (32-бітні слова) ---

FakeCpu::Register FakeCpu::read32(std::uint32_t address) {
    if (!memoryBus_) {
        Logger::instance().warn("CPU", "read32 called without memoryBus attached");
        return 0;
    }

    // Little-endian: молодший байт за найменшою адресою.
    const std::uint32_t b0 = memoryBus_->read8(address + 0);
    const std::uint32_t b1 = memoryBus_->read8(address + 1);
    const std::uint32_t b2 = memoryBus_->read8(address + 2);
    const std::uint32_t b3 = memoryBus_->read8(address + 3);

    const std::uint32_t value = (b0) | (b1 << 8) | (b2 << 16) | (b3 << 24);

    {
        std::ostringstream oss;
        oss << "FETCH32 addr=0x" << std::hex << address << " -> 0x" << value;
        Logger::instance().debug("CPU", oss.str());
    }

    return static_cast<Register>(value);
}

void FakeCpu::write32(std::uint32_t address, Register value) {
    if (!memoryBus_) {
        Logger::instance().warn("CPU", "write32 called without memoryBus attached");
        return;
    }

    const std::uint32_t v = static_cast<std::uint32_t>(value);

    memoryBus_->write8(address + 0, static_cast<std::uint8_t>(v & 0xFFu));
    memoryBus_->write8(address + 1, static_cast<std::uint8_t>((v >> 8) & 0xFFu));
    memoryBus_->write8(address + 2, static_cast<std::uint8_t>((v >> 16) & 0xFFu));
    memoryBus_->write8(address + 3, static_cast<std::uint8_t>((v >> 24) & 0xFFu));

    {
        std::ostringstream oss;
        oss << "WRITE32 addr=0x" << std::hex << address << " value=0x" << v;
        Logger::instance().debug("CPU", oss.str());
    }
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
    constexpr std::uint8_t OPC_LOAD = 0x04;
    constexpr std::uint8_t OPC_STORE = 0x05;
    // Стрибки додамо трохи пізніше:
    constexpr std::uint8_t OPC_JMP = 0x06;
    constexpr std::uint8_t OPC_JZ = 0x07;
    constexpr std::uint8_t OPC_JNZ = 0x08;
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

        case OPC_LOAD: {
            // LOAD Rd, [Rs + imm16]
            // EA = Rs + sign_extend(imm16)
            // Rd = MEM32[EA]

            const Register base = readReg(rsIndex);
            const Register offset = signExtendImm16();
            const std::uint32_t ea =
                static_cast<std::uint32_t>(static_cast<std::uint32_t>(base) + static_cast<std::uint32_t>(offset));

            const Register value = read32(ea);

            {
                std::ostringstream oss;
                oss << "LOAD R" << rdIndex << ", [R" << rsIndex << " + " << imm16 << "] " << "(EA=0x" << std::hex << ea
                    << ", value=0x" << value << ")";
                Logger::instance().debug("CPU", oss.str());
            }

            writeReg(rdIndex, value);
            // За ISA: LOAD оновлює Z/N, не чіпаючи Carry/Overflow
            updateZNFlags(value);

            // Звичайна інструкція → PC = PC + 4
            state_.pc += 4;
            break;
        }

        case OPC_STORE: {
            // STORE Rs, [Rd + imm16]
            // EA = Rd + sign_extend(imm16)
            // MEM32[EA] = Rs

            const Register base = readReg(rdIndex);   // база адресації
            const Register value = readReg(rsIndex);  // значення для запису
            const Register offset = signExtendImm16();

            const std::uint32_t ea =
                static_cast<std::uint32_t>(static_cast<std::uint32_t>(base) + static_cast<std::uint32_t>(offset));

            write32(ea, value);

            {
                std::ostringstream oss;
                oss << "STORE R" << rsIndex << " -> [R" << rdIndex << " + " << imm16 << "] " << "(EA=0x" << std::hex
                    << ea << ", value=0x" << value << ")";
                Logger::instance().debug("CPU", oss.str());
            }

            // За ISA: STORE не змінює FLAGS

            state_.pc += 4;
            break;
        }

        case OPC_JMP: {
            // JMP offset16
            // PC = PC + 4 + (sign_extend(offset16) << 2)

            const std::int32_t offsetWords = static_cast<std::int32_t>(imm16);
            const std::int32_t offsetBytes = offsetWords << 2;  // множимо на розмір інструкції (4 байти)

            const std::uint32_t oldPc = state_.pc;
            const std::uint32_t nextPc = oldPc + 4;
            const std::uint32_t targetPc = static_cast<std::uint32_t>(static_cast<std::int32_t>(nextPc) + offsetBytes);

            {
                std::ostringstream oss;
                oss << "JMP " << imm16 << " (words) " << "oldPC=0x" << std::hex << oldPc << " -> targetPC=0x"
                    << targetPc;
                Logger::instance().debug("CPU", oss.str());
            }

            state_.pc = targetPc;
            break;
        }

        case OPC_JZ: {
            // JZ offset16
            // if Z == 1:
            //     PC = PC + 4 + (sign_extend(offset16) << 2)
            // else:
            //     PC = PC + 4

            const bool zSet = isFlagSet(Flag::Zero);

            const std::int32_t offsetWords = static_cast<std::int32_t>(imm16);
            const std::int32_t offsetBytes = offsetWords << 2;

            const std::uint32_t oldPc = state_.pc;
            const std::uint32_t nextPc = oldPc + 4;

            std::uint32_t newPc = nextPc;

            if (zSet) {
                newPc = static_cast<std::uint32_t>(static_cast<std::int32_t>(nextPc) + offsetBytes);
            }

            {
                std::ostringstream oss;
                oss << "JZ " << imm16 << " (words), Z=" << (zSet ? 1 : 0) << " oldPC=0x" << std::hex << oldPc
                    << " -> newPC=0x" << newPc << (zSet ? " (taken)" : " (not taken)");
                Logger::instance().debug("CPU", oss.str());
            }

            state_.pc = newPc;
            break;
        }

        case OPC_JNZ: {
            // JNZ offset16
            // if Z == 0:
            //     PC = PC + 4 + (sign_extend(offset16) << 2)
            // else:
            //     PC = PC + 4

            const bool zSet = isFlagSet(Flag::Zero);

            const std::int32_t offsetWords = static_cast<std::int32_t>(imm16);
            const std::int32_t offsetBytes = offsetWords << 2;

            const std::uint32_t oldPc = state_.pc;
            const std::uint32_t nextPc = oldPc + 4;

            std::uint32_t newPc = nextPc;

            if (!zSet) {
                newPc = static_cast<std::uint32_t>(static_cast<std::int32_t>(nextPc) + offsetBytes);
            }

            {
                std::ostringstream oss;
                oss << "JNZ " << imm16 << " (words), Z=" << (zSet ? 1 : 0) << " oldPC=0x" << std::hex << oldPc
                    << " -> newPC=0x" << newPc << (!zSet ? " (taken)" : " (not taken)");
                Logger::instance().debug("CPU", oss.str());
            }

            state_.pc = newPc;
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

// --- Основна логіка CPUEDITOR CPU (нова, 32-бітна) ---

void FakeCpu::step() {
    // Рахуємо кроки — це важливо для smoke-тестів
    ++stepCount_;

    // Якщо CPU вже зупинений — нічого не робимо
    if (halted_) {
        Logger::instance().debug("CPU", "step() called while HALTED — skipping");
        return;
    }

    // Без підключеної шини пам'яті ми не можемо виконувати інструкції
    if (!memoryBus_) {
        Logger::instance().warn("CPU", "step() called without memoryBus attached");
        return;
    }

    // 1. Fetch: читаємо 32-бітну інструкцію з пам'яті за PC
    const Register rawInstr = read32(state_.pc);
    const std::uint32_t instruction = static_cast<std::uint32_t>(rawInstr);

    // 2. Decode + Execute: передаємо інструкцію у decodeAndExecute()
    decodeAndExecute(instruction);
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
}

bool FakeCpu::loadImage(const std::string& path) {
    lastImagePath_ = path;
    imageLoaded_ = !path.empty();
    return imageLoaded_;
}

void FakeCpu::setPC(std::uint32_t value) noexcept { state_.pc = value; }

void FakeCpu::setMemoryBus(std::shared_ptr<IMemoryBus> bus) { memoryBus_ = std::move(bus); }

}  // namespace elsim::core
