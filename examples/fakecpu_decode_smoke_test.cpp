#include <cstdint>
#include <iostream>

#include "elsim/core/FakeCpu.hpp"

using elsim::core::FakeCpu;

static std::uint32_t encodeAlu(std::uint8_t opcode, std::uint8_t rd, std::uint8_t rs, bool isImm, std::int16_t imm16) {
    std::uint32_t instr = 0;

    instr |= (static_cast<std::uint32_t>(opcode) & 0xFFu) << 24;
    instr |= (static_cast<std::uint32_t>(rd) & 0x7u) << 21;
    instr |= (static_cast<std::uint32_t>(rs) & 0x7u) << 18;
    instr |= (isImm ? 1u : 0u) << 17;
    // bit 16 reserved = 0
    instr |= static_cast<std::uint16_t>(imm16);

    return instr;
}

static std::uint32_t encodeNop() { return static_cast<std::uint32_t>(0x00u) << 24; }

static std::uint32_t encodeHalt() { return static_cast<std::uint32_t>(0xFFu) << 24; }

int main() {
    FakeCpu cpu;
    cpu.reset();

    std::cout << "Starting FakeCpu decode/execute smoke test...\n";

    // -------------------------------------------------------------
    // 1) MOV R0, #5
    // -------------------------------------------------------------
    {
        std::uint32_t instr = encodeAlu(0x01u, /*rd=*/0, /*rs=*/0, /*isImm=*/true, /*imm16=*/5);
        cpu.decodeAndExecute(instr);

        auto r0 = cpu.getRegister(0);
        auto pc = cpu.getPc();

        std::cout << "[TEST] MOV R0, #5 -> R0=" << r0 << ", PC=" << pc << "\n";
    }

    // -------------------------------------------------------------
    // 2) MOV R1, R0
    // -------------------------------------------------------------
    {
        std::uint32_t instr = encodeAlu(0x01u, /*rd=*/1, /*rs=*/0, /*isImm=*/false, /*imm16=*/0);
        cpu.decodeAndExecute(instr);

        auto r0 = cpu.getRegister(0);
        auto r1 = cpu.getRegister(1);
        auto pc = cpu.getPc();

        std::cout << "[TEST] MOV R1, R0 -> R0=" << r0 << ", R1=" << r1 << ", PC=" << pc << "\n";
    }

    // -------------------------------------------------------------
    // 3) ADD R0, R1   (R0 = R0 + R1)
    // -------------------------------------------------------------
    {
        std::uint32_t instr = encodeAlu(0x02u, /*rd=*/0, /*rs=*/1, /*isImm=*/false, /*imm16=*/0);
        cpu.decodeAndExecute(instr);

        auto r0 = cpu.getRegister(0);
        auto r1 = cpu.getRegister(1);
        auto pc = cpu.getPc();

        std::cout << "[TEST] ADD R0, R1 -> R0=" << r0 << ", R1=" << r1 << ", PC=" << pc << "\n";
    }

    // -------------------------------------------------------------
    // 4) SUB R0, #3   (R0 = R0 - 3)
    // -------------------------------------------------------------
    {
        std::uint32_t instr = encodeAlu(0x03u, /*rd=*/0, /*rs=*/0, /*isImm=*/true, /*imm16=*/3);
        cpu.decodeAndExecute(instr);

        auto r0 = cpu.getRegister(0);
        auto pc = cpu.getPc();

        std::cout << "[TEST] SUB R0, #3 -> R0=" << r0 << ", PC=" << pc << "\n";
    }

    // -------------------------------------------------------------
    // 5) NOP
    // -------------------------------------------------------------
    {
        std::uint32_t instr = encodeNop();
        cpu.decodeAndExecute(instr);

        auto pc = cpu.getPc();
        std::cout << "[TEST] NOP -> PC=" << pc << "\n";
    }

    // -------------------------------------------------------------
    // 6) HALT
    // -------------------------------------------------------------
    {
        std::uint32_t instr = encodeHalt();
        cpu.decodeAndExecute(instr);

        auto halted = cpu.isHalted();
        auto pc = cpu.getPc();
        std::cout << "[TEST] HALT -> halted=" << halted << ", PC=" << pc << "\n";

        // Спроба виконати ще одну інструкцію після HALT:
        cpu.decodeAndExecute(encodeNop());
        auto pcAfter = cpu.getPc();
        std::cout << "[TEST] After HALT, NOP ignored -> PC=" << pcAfter << " (should be unchanged)\n";
    }

    std::cout << "FakeCpu decode/execute smoke test finished.\n";
    return 0;
}
