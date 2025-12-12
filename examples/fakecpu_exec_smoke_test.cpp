#include <cstdint>
#include <iostream>
#include <vector>

#include "elsim/core/FakeCpu.hpp"
#include "elsim/core/IMemoryBus.hpp"
#include "elsim/core/Logger.hpp"
#include "elsim/core/MemoryBusAdapter.hpp"

// MemoryBus оголошений у глобальному просторі імен (див. MemoryBus.cpp)
#include "elsim/core/MemoryBus.hpp"

// Короткі імена
using elsim::core::FakeCpu;
using elsim::core::Logger;
using elsim::core::LogLevel;
using elsim::core::MemoryBusAdapter;

// OPC-коди згідно fakecpu_isa.md / FakeCpu.cpp
enum : std::uint8_t {
    OPC_NOP = 0x00,
    OPC_MOV = 0x01,
    OPC_ADD = 0x02,
    OPC_SUB = 0x03,
    OPC_LOAD = 0x04,
    OPC_STORE = 0x05,
    OPC_JMP = 0x06,
    OPC_JZ = 0x07,
    OPC_JNZ = 0x08,
    OPC_HALT = 0xFF
};

// Загальне кодування для R-формату (MOV/ADD/SUB регістрові)
constexpr std::uint32_t ENCODE_R(std::uint8_t opc, std::uint8_t rd, std::uint8_t rs) {
    return (static_cast<std::uint32_t>(opc) << 24) | ((static_cast<std::uint32_t>(rd & 0x7)) << 21) |
           ((static_cast<std::uint32_t>(rs & 0x7)) << 18);
}

// I-формат (MOV/ADD/SUB з immediate)
constexpr std::uint32_t ENCODE_I(std::uint8_t opc, std::uint8_t rd, std::int16_t imm16) {
    return (static_cast<std::uint32_t>(opc) << 24) | ((static_cast<std::uint32_t>(rd & 0x7)) << 21) |
           (1u << 17) |                          // isImm = 1
           (static_cast<std::uint16_t>(imm16));  // нижні 16 біт
}

// LOAD Rd, [Rs + imm16]
constexpr std::uint32_t ENCODE_LOAD(std::uint8_t rd, std::uint8_t rs, std::int16_t imm16) {
    return (static_cast<std::uint32_t>(OPC_LOAD) << 24) | ((static_cast<std::uint32_t>(rd & 0x7)) << 21) |
           ((static_cast<std::uint32_t>(rs & 0x7)) << 18) | (static_cast<std::uint16_t>(imm16));
}

// STORE Rs, [Rd + imm16]
constexpr std::uint32_t ENCODE_STORE(std::uint8_t rs, std::uint8_t rd, std::int16_t imm16) {
    return (static_cast<std::uint32_t>(OPC_STORE) << 24) |
           ((static_cast<std::uint32_t>(rd & 0x7)) << 21) |  // база в Rd
           ((static_cast<std::uint32_t>(rs & 0x7)) << 18) |  // значення в Rs
           (static_cast<std::uint16_t>(imm16));
}

// JMP/JZ/JNZ з offset16 (у словах)
constexpr std::uint32_t ENCODE_J(std::uint8_t opc, std::int16_t offsetWords) {
    return (static_cast<std::uint32_t>(opc) << 24) | (static_cast<std::uint16_t>(offsetWords));
}

// Запис 32-бітної інструкції в RAM (little-endian)
void write_instr(::MemoryBus& bus, std::uint32_t addr, std::uint32_t instr) {
    bus.write8(addr + 0, static_cast<std::uint8_t>(instr & 0xFFu));
    bus.write8(addr + 1, static_cast<std::uint8_t>((instr >> 8) & 0xFFu));
    bus.write8(addr + 2, static_cast<std::uint8_t>((instr >> 16) & 0xFFu));
    bus.write8(addr + 3, static_cast<std::uint8_t>((instr >> 24) & 0xFFu));
}

constexpr std::uint32_t DATA_ADDR = 64;  // 0x40, далеко від коду

int main() {
    // Увімкнемо детальний лог
    Logger::instance().set_level(LogLevel::Debug);

    // 1. Створюємо RAM і CPU
    ::MemoryBus ram(256);  // 256 байт RAM достатньо
    FakeCpu cpu;

    // Адаптер шини для CPU (як у Simulator)
    auto busAdapter = std::make_shared<MemoryBusAdapter>(&ram);
    cpu.setMemoryBus(busAdapter);

    // 2. Кодуємо невелику програму в RAM починаючи з адреси 0x00000000
    //
    // Програма:
    // 0: MOV  R1, #16        ; база = 0x10
    // 1: MOV  R0, #5         ; значення 5
    // 2: STORE R0, [R1 + 0]  ; MEM[0x10] = 5
    // 3: MOV  R0, #0         ; очистимо R0 (щоб перевірити LOAD)
    // 4: LOAD R2, [R1 + 0]   ; R2 = MEM[0x10] -> 5
    // 5: SUB  R2, #5         ; R2 = 0, ZF = 1
    // 6: JZ   +1             ; якщо Z==1, перескочити інструкцію 7
    // 7: MOV  R3, #0xDEAD    ; НЕ повинна виконатися, якщо JZ працює
    // 8: HALT

    std::vector<std::uint32_t> program;
    program.push_back(ENCODE_I(OPC_MOV, 1, DATA_ADDR));               // R1 = DATA_ADDR (0x40)
    program.push_back(ENCODE_I(OPC_MOV, 0, 5));                       // R0 = 5
    program.push_back(ENCODE_STORE(0, 1, 0));                         // STORE R0 -> [R1 + 0]
    program.push_back(ENCODE_I(OPC_MOV, 0, 0));                       // R0 = 0
    program.push_back(ENCODE_LOAD(2, 1, 0));                          // LOAD R2, [R1 + 0]
    program.push_back(ENCODE_I(OPC_SUB, 2, 5));                       // SUB R2, #5
    program.push_back(ENCODE_J(OPC_JZ, 1));                           // JZ +1
    program.push_back(ENCODE_I(OPC_MOV, 3, 0xDEAD));                  // MOV R3, #0xDEAD
    program.push_back((static_cast<std::uint32_t>(OPC_HALT) << 24));  // HALT

    // Записуємо програму в RAM (по 4 байти на інструкцію)
    std::uint32_t addr = 0;
    for (std::size_t i = 0; i < program.size(); ++i) {
        write_instr(ram, addr, program[i]);
        addr += 4;
    }

    // 3. Скидаємо CPU (PC = 0, регістри = 0, FLAGS = 0)
    cpu.reset();

    // 4. Виконуємо кілька кроків
    constexpr int maxSteps = 32;
    for (int i = 0; i < maxSteps; ++i) {
        cpu.step();
        // Після HALT step() просто буде нічого не робити,
        // тому луп безперечний, але це ок для smoke-тесту.
    }

    // 5. Перевіримо результати
    auto R0 = cpu.getRegister(0);
    auto R1 = cpu.getRegister(1);
    auto R2 = cpu.getRegister(2);
    auto R3 = cpu.getRegister(3);

    std::uint8_t mem = ram.read8(DATA_ADDR);  // значення з адреси DATA_ADDR

    std::cout << "R0 = " << R0 << "\n";
    std::cout << "R1 = " << R1 << "\n";
    std::cout << "R2 = " << R2 << "\n";
    std::cout << "R3 = " << R3 << "\n";
    std::cout << "MEM[DATA_ADDR] = " << static_cast<int>(mem) << "\n";

    bool ok = true;
    if (mem != 5) {
        std::cout << "FAIL: MEM[DATA_ADDR] expected 5\n";
        ok = false;
    }
    if (R2 != 0) {
        std::cout << "FAIL: R2 expected 0 (LOAD+SUB)\n";
        ok = false;
    }
    if (R3 != 0) {
        std::cout << "FAIL: R3 expected 0 (JZ must skip MOV R3)\n";
        ok = false;
    }

    if (ok) {
        std::cout << "==== FakeCpu EXEC smoke test PASSED ====\n";
        return 0;
    } else {
        std::cout << "==== FakeCpu EXEC smoke test FAILED ====\n";
        return 1;
    }
}
