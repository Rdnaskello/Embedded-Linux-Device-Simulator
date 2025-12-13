#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

// ВАЖЛИВО: magic/структуру заголовка візьми 1:1 з твого program_loader_smoke_test.cpp або ProgramLoader.
// Нижче — типовий варіант, який у тебе вже використовувався в smoke test.
struct ElsimBinaryHeader {
    std::uint32_t magic;        // наприклад 'ELSB'
    std::uint32_t entry_point;  // куди ставити PC
    std::uint32_t code_size;    // bytes
};

static std::uint32_t Encode(std::uint8_t opcode, std::uint8_t rd, std::uint8_t rs, bool isImm, std::int16_t imm16) {
    std::uint32_t w = 0;
    w |= (static_cast<std::uint32_t>(opcode) << 24);
    w |= (static_cast<std::uint32_t>(rd & 0x7u) << 21);
    w |= (static_cast<std::uint32_t>(rs & 0x7u) << 18);
    w |= (static_cast<std::uint32_t>(isImm ? 1u : 0u) << 17);
    w |= static_cast<std::uint16_t>(imm16);
    return w;
}

static void AppendWordLE(std::vector<std::uint8_t>& out, std::uint32_t w) {
    out.push_back(static_cast<std::uint8_t>(w & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((w >> 8) & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((w >> 16) & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((w >> 24) & 0xFFu));
}

int main() {
    constexpr std::uint8_t OPC_MOV = 0x01;
    constexpr std::uint8_t OPC_STORE = 0x05;
    constexpr std::uint8_t OPC_HALT = 0xFF;

    constexpr std::uint8_t R0 = 0;
    constexpr std::uint8_t R1 = 1;

    constexpr std::int16_t UART_BASE = 0x7000;
    constexpr std::int16_t UART_TX_OFF = 0;

    std::vector<std::uint8_t> code;

    AppendWordLE(code, Encode(OPC_MOV, R1, 0, true, UART_BASE));

    const char* text = "Hello from FakeCPU\n";
    for (const char* p = text; *p; ++p) {
        const std::int16_t ch = static_cast<unsigned char>(*p);
        AppendWordLE(code, Encode(OPC_MOV, R0, 0, true, ch));
        AppendWordLE(code, Encode(OPC_STORE, R1, R0, false, UART_TX_OFF));
    }

    AppendWordLE(code, Encode(OPC_HALT, 0, 0, false, 0));

    ElsimBinaryHeader hdr{};
    hdr.magic = 0x42534C45u;
    hdr.entry_point = 0x00000000u;
    hdr.code_size = static_cast<std::uint32_t>(code.size());

    const std::filesystem::path outPath = std::filesystem::path(PROJECT_SOURCE_DIR) / "examples" / "hello.elsim-bin";

    std::ofstream out(outPath, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open output file: " << outPath << "\n";
        return 1;
    }

    out.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));
    out.write(reinterpret_cast<const char*>(code.data()), static_cast<std::streamsize>(code.size()));

    std::cout << "Wrote " << outPath << " (" << code.size() << " bytes of code)\n";
    return 0;
}