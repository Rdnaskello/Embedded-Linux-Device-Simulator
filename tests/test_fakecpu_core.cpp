#include <gtest/gtest.h>

#include <cstdint>
#include <memory>

#include "elsim/core/FakeCpu.hpp"
#include "elsim/core/MemoryBus.hpp"
#include "elsim/core/MemoryBusAdapter.hpp"

using elsim::core::FakeCpu;
using elsim::core::MemoryBus;
using elsim::core::MemoryBusAdapter;

namespace {

constexpr std::uint8_t OPC_MOV = 0x01;
constexpr std::uint8_t OPC_ADD = 0x02;
constexpr std::uint8_t OPC_SUB = 0x03;
constexpr std::uint8_t OPC_LOAD = 0x04;
constexpr std::uint8_t OPC_STORE = 0x05;
constexpr std::uint8_t OPC_JMP = 0x06;
constexpr std::uint8_t OPC_JZ = 0x07;
constexpr std::uint8_t OPC_JNZ = 0x08;
constexpr std::uint8_t OPC_HALT = 0xFF;

// Базовий енкодер "R"-формату:
// 31..24 — opcode
// 23..21 — Rd (3 біти)
// 20..18 — Rs (3 біти)
// 17     — isImm (0 для R-формату)
// 15..0  — imm16 (0)
inline std::uint32_t ENCODE_R(std::uint8_t opcode, std::uint8_t rd, std::uint8_t rs) {
    std::uint32_t word = 0;
    word |= static_cast<std::uint32_t>(opcode) << 24;
    word |= (static_cast<std::uint32_t>(rd) & 0x7u) << 21;
    word |= (static_cast<std::uint32_t>(rs) & 0x7u) << 18;
    // isImm = 0
    // imm16 = 0
    return word;
}

// "I"-формат (імедіат):
// isImm = 1, Rs = 0 або ігнорується (залежно від інструкції)
// 31..24 — opcode
// 23..21 — Rd
// 20..18 — Rs (часто 0)
// 17     — isImm = 1
// 15..0  — imm16 (signed)
inline std::uint32_t ENCODE_I(std::uint8_t opcode, std::uint8_t rd, std::uint8_t rs, std::int16_t imm) {
    std::uint32_t word = 0;
    word |= static_cast<std::uint32_t>(opcode) << 24;
    word |= (static_cast<std::uint32_t>(rd) & 0x7u) << 21;
    word |= (static_cast<std::uint32_t>(rs) & 0x7u) << 18;
    word |= (1u << 17);  // isImm = 1
    word |= static_cast<std::uint16_t>(imm);
    return word;
}

// Спеціалізовані хелпери для читабельності тестів:

inline std::uint32_t MOV_IMM(std::uint8_t rd, std::int16_t imm) { return ENCODE_I(OPC_MOV, rd, 0, imm); }

inline std::uint32_t MOV_REG(std::uint8_t rd, std::uint8_t rs) { return ENCODE_R(OPC_MOV, rd, rs); }

inline std::uint32_t ADD_REG(std::uint8_t rd, std::uint8_t rs) { return ENCODE_R(OPC_ADD, rd, rs); }

inline std::uint32_t ADD_IMM(std::uint8_t rd, std::int16_t imm) { return ENCODE_I(OPC_ADD, rd, 0, imm); }

inline std::uint32_t SUB_REG(std::uint8_t rd, std::uint8_t rs) { return ENCODE_R(OPC_SUB, rd, rs); }

inline std::uint32_t SUB_IMM(std::uint8_t rd, std::int16_t imm) { return ENCODE_I(OPC_SUB, rd, 0, imm); }

// LOAD Rd, [Rs + imm16]
inline std::uint32_t LOAD_ENC(std::uint8_t rd, std::uint8_t rs, std::int16_t imm) {
    return ENCODE_I(OPC_LOAD, rd, rs, imm);
}

// STORE Rs, [Rd + imm16]
// (за домовленістю ISA: Rd — база адреси, Rs — джерело даних)
inline std::uint32_t STORE_ENC(std::uint8_t rs, std::uint8_t rd, std::int16_t imm) {
    return ENCODE_I(OPC_STORE, rd, rs, imm);
}

// Для переходів використовуємо тільки imm16 (PC-relative offset у словах)
inline std::uint32_t JMP_ENC(std::int16_t offsetWords) { return ENCODE_I(OPC_JMP, 0, 0, offsetWords); }

inline std::uint32_t JZ_ENC(std::int16_t offsetWords) { return ENCODE_I(OPC_JZ, 0, 0, offsetWords); }

inline std::uint32_t JNZ_ENC(std::int16_t offsetWords) { return ENCODE_I(OPC_JNZ, 0, 0, offsetWords); }

inline std::uint32_t HALT_ENC() { return ENCODE_R(OPC_HALT, 0, 0); }

// Допоміжні функції для запису/читання 32-бітного слова в MemoryBus (little-endian)
inline void write_word32(MemoryBus& bus, std::uint32_t addr, std::uint32_t value) {
    bus.write8(addr + 0, static_cast<std::uint8_t>(value & 0xFFu));
    bus.write8(addr + 1, static_cast<std::uint8_t>((value >> 8) & 0xFFu));
    bus.write8(addr + 2, static_cast<std::uint8_t>((value >> 16) & 0xFFu));
    bus.write8(addr + 3, static_cast<std::uint8_t>((value >> 24) & 0xFFu));
}

inline std::uint32_t read_word32(MemoryBus& bus, std::uint32_t addr) {
    std::uint32_t b0 = bus.read8(addr + 0);
    std::uint32_t b1 = bus.read8(addr + 1);
    std::uint32_t b2 = bus.read8(addr + 2);
    std::uint32_t b3 = bus.read8(addr + 3);
    return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

class FakeCpuCoreTest : public ::testing::Test {
   protected:
    FakeCpu cpu;
    MemoryBus bus;
    std::shared_ptr<MemoryBusAdapter> busAdapter;

    FakeCpuCoreTest()
        : cpu(),
          bus(256)  // 256 байт RAM для тестів більш ніж достатньо
          ,
          busAdapter(std::make_shared<MemoryBusAdapter>(&bus)) {}

    void SetUp() override {
        cpu.reset();
        cpu.setMemoryBus(busAdapter);
    }
};

// Тести для основного виконуючого ядра FakeCpu (decodeAndExecute)
// Тестуємо окремі інструкції в ізоляції
TEST_F(FakeCpuCoreTest, AddRegBasic) {
    // R1 = 10, R2 = 20
    cpu.setRegister(1, 10);
    cpu.setRegister(2, 20);
    cpu.setPc(0);

    std::uint32_t instr = ADD_REG(1, 2);  // ADD R1, R2

    cpu.decodeAndExecute(instr);

    EXPECT_EQ(cpu.getRegister(1), 30u);  // 10 + 20
    EXPECT_EQ(cpu.getPc(), 4u);          // звичайна інструкція → PC + 4
    EXPECT_FALSE(cpu.isFlagSet(FakeCpu::Flag::Zero));
    EXPECT_FALSE(cpu.isFlagSet(FakeCpu::Flag::Negative));
}
// тест, де результат = 0 → Zero = 1
TEST_F(FakeCpuCoreTest, AddImmSetsZeroFlag) {
    cpu.setRegister(0, 5);
    cpu.setPc(0);

    std::uint32_t instr = ADD_IMM(0, -5);  // 5 + (-5) = 0

    cpu.decodeAndExecute(instr);

    EXPECT_EQ(cpu.getRegister(0), 0u);
    EXPECT_TRUE(cpu.isFlagSet(FakeCpu::Flag::Zero));
    EXPECT_FALSE(cpu.isFlagSet(FakeCpu::Flag::Negative));
}
// тест, де результат < 0 → Negative = 1
TEST_F(FakeCpuCoreTest, SubImmSetsNegativeFlag) {
    cpu.setRegister(0, 5);
    cpu.setPc(0);

    std::uint32_t instr = SUB_IMM(0, 10);  // 5 - 10 = -5 (0xFFFFFFFB)

    cpu.decodeAndExecute(instr);

    EXPECT_EQ(cpu.getRegister(0), static_cast<std::uint32_t>(-5));
    EXPECT_FALSE(cpu.isFlagSet(FakeCpu::Flag::Zero));
    EXPECT_TRUE(cpu.isFlagSet(FakeCpu::Flag::Negative));
}
// Тест базового безумовного переходу JMP
TEST_F(FakeCpuCoreTest, JmpPcRelativeForward) {
    cpu.setPc(0);  // Початковий PC = 0

    // offset = +2 слова → PC = 0 + 4 + 2*4 = 12
    std::uint32_t instr = JMP_ENC(+2);

    cpu.decodeAndExecute(instr);

    EXPECT_EQ(cpu.getPc(), 12u);
}
// Тест безумовного переходу JMP назад
TEST_F(FakeCpuCoreTest, JmpPcRelativeBackward) {
    cpu.setPc(16);  // 0x10

    // offset = -1 слово → PC = 16 + 4 + (-1)*4 = 16
    // Чому так: 16 + 4 - 4 = 16, стрибок "на поточну інструкцію" (петля)
    std::uint32_t instr = JMP_ENC(-1);

    cpu.decodeAndExecute(instr);

    EXPECT_EQ(cpu.getPc(), 16u);
}
// Тести умовних переходів JZ / JNZ
TEST_F(FakeCpuCoreTest, JzTakenWhenZeroFlagSet) {
    cpu.setPc(0);
    cpu.setFlag(FakeCpu::Flag::Zero, true);

    // offset = +1 слово → PC = 0 + 4 + 1*4 = 8
    std::uint32_t instr = JZ_ENC(+1);

    cpu.decodeAndExecute(instr);

    EXPECT_EQ(cpu.getPc(), 8u);
}
// Тест, коли JZ не повинен братись
TEST_F(FakeCpuCoreTest, JzNotTakenWhenZeroFlagClear) {
    cpu.setPc(0);
    cpu.setFlag(FakeCpu::Flag::Zero, false);

    std::uint32_t instr = JZ_ENC(+1);

    cpu.decodeAndExecute(instr);

    // Перехід не взято → просто PC + 4
    EXPECT_EQ(cpu.getPc(), 4u);
}
// Тест, коли JNZ повинен братись
TEST_F(FakeCpuCoreTest, JnzTakenWhenZeroFlagClear) {
    cpu.setPc(0);
    cpu.setFlag(FakeCpu::Flag::Zero, false);

    // offset = +2 → PC = 0 + 4 + 2*4 = 12
    std::uint32_t instr = JNZ_ENC(+2);

    cpu.decodeAndExecute(instr);

    EXPECT_EQ(cpu.getPc(), 12u);
}
// Тест, коли JNZ не повинен братись
TEST_F(FakeCpuCoreTest, JnzNotTakenWhenZeroFlagSet) {
    cpu.setPc(0);
    cpu.setFlag(FakeCpu::Flag::Zero, true);

    std::uint32_t instr = JNZ_ENC(+1);

    cpu.decodeAndExecute(instr);

    // Z = 1 → перехід не береться
    EXPECT_EQ(cpu.getPc(), 4u);
}
// Тест для LOAD
TEST_F(FakeCpuCoreTest, LoadReadsWordFromMemoryAndUpdatesFlags) {
    // Адреса, куди покладемо слово
    const std::uint32_t baseAddr = 16;
    const std::int16_t offset = 4;
    const std::uint32_t ea = baseAddr + static_cast<std::uint32_t>(offset);

    const std::uint32_t value = 0x11223344u;

    // Пишемо значення напряму в RAM через MemoryBus-хелпер
    write_word32(bus, ea, value);

    // R1 — база адреси для LOAD
    cpu.setRegister(1, baseAddr);
    cpu.setPc(0);

    // LOAD R0, [R1 + offset]
    std::uint32_t instr = LOAD_ENC(/*rd=*/0, /*rs=*/1, offset);

    cpu.decodeAndExecute(instr);

    // Перевіряємо, що слово зчиталось у R0
    EXPECT_EQ(cpu.getRegister(0), value);

    // Звичайна інструкція → PC + 4
    EXPECT_EQ(cpu.getPc(), 4u);

    // Для цього значення Z=0, N залежить від старшого біта (0x11... → N=0)
    EXPECT_FALSE(cpu.isFlagSet(FakeCpu::Flag::Zero));
    EXPECT_FALSE(cpu.isFlagSet(FakeCpu::Flag::Negative));
}
// Тест для STORE
TEST_F(FakeCpuCoreTest, StoreWritesWordToMemoryAndKeepsFlags) {
    const std::uint32_t baseAddr = 32;
    const std::int16_t offset = 8;
    const std::uint32_t ea = baseAddr + static_cast<std::uint32_t>(offset);

    const std::uint32_t value = 0xDEADBEEFu;

    // Rd — база адреси, Rs — значення для запису
    cpu.setRegister(1, baseAddr);  // Rd
    cpu.setRegister(2, value);     // Rs
    cpu.setPc(0);

    // Попередньо виставимо Z-флаг, щоб перевірити, що STORE його не змінює
    cpu.setFlags(static_cast<std::uint32_t>(FakeCpu::Flag::Zero));

    // STORE R2, [R1 + offset]
    std::uint32_t instr = STORE_ENC(/*rs=*/2, /*rd=*/1, offset);

    cpu.decodeAndExecute(instr);

    // Перевіряємо, що в пам'ять записалось саме наше слово
    EXPECT_EQ(read_word32(bus, ea), value);

    // PC повинен збільшитися на 4
    EXPECT_EQ(cpu.getPc(), 4u);

    // FLAGS мають лишитися такими, як були (Z встановлений)
    EXPECT_TRUE(cpu.isFlagSet(FakeCpu::Flag::Zero));
}

}  // namespace
