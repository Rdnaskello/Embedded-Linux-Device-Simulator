#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

struct ElsimBinaryHeader {
    std::uint32_t magic;
    std::uint32_t entry_point;
    std::uint32_t code_size;
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
    constexpr std::uint8_t OPC_JMP = 0x06;

    constexpr std::uint8_t R0 = 0;
    constexpr std::uint8_t R1 = 1;

    // GPIO base for gpio-blinky-board.yaml (low address so it fits in imm16)
    constexpr std::int16_t GPIO_BASE = 0x6000;

    // GPIO MMIO offsets (see GpioDevice)
    constexpr std::int16_t GPIO_DIR_OFF = 0x00;
    constexpr std::int16_t GPIO_TOGGLE_OFF = 0x14;

    std::vector<std::uint8_t> code;

    // R1 = GPIO_BASE
    AppendWordLE(code, Encode(OPC_MOV, R1, 0, true, GPIO_BASE));

    // R0 = 1 (mask for pin0)
    AppendWordLE(code, Encode(OPC_MOV, R0, 0, true, 1));

    // DIR |= 1  (set pin0 as output)
    AppendWordLE(code, Encode(OPC_STORE, R1, R0, false, GPIO_DIR_OFF));

    // --- loop start ---
    const std::size_t loopStartInstrIndex = code.size() / 4;

    // TOGGLE pin0
    AppendWordLE(code, Encode(OPC_STORE, R1, R0, false, GPIO_TOGGLE_OFF));

    // crude delay: NOP loop (NOP = opcode 0x00)
    // We keep it small to avoid huge logs but still visibly toggle.
    constexpr std::uint8_t OPC_NOP = 0x00;
    constexpr int kDelayNops = 200;
    for (int i = 0; i < kDelayNops; ++i) {
        AppendWordLE(code, Encode(OPC_NOP, 0, 0, false, 0));
    }

    // JMP back to loop start
    // offset16 is in "words" relative to (PC + 4)
    // current PC points to JMP instruction; next PC is after it (+4).
    // We need to jump back to loopStartInstrIndex.
    const std::size_t jmpInstrIndex = code.size() / 4;
    const std::int32_t nextInstrIndex = static_cast<std::int32_t>(jmpInstrIndex + 1);
    const std::int32_t targetInstrIndex = static_cast<std::int32_t>(loopStartInstrIndex);
    const std::int32_t offsetWords = targetInstrIndex - nextInstrIndex;

    AppendWordLE(code, Encode(OPC_JMP, 0, 0, true, static_cast<std::int16_t>(offsetWords)));

    // header
    ElsimBinaryHeader hdr{};
    hdr.magic = 0x42534C45u;        // 'ELSB'
    hdr.entry_point = 0x00000000u;  // start at 0
    hdr.code_size = static_cast<std::uint32_t>(code.size());

    const std::filesystem::path outPath =
        std::filesystem::path(PROJECT_SOURCE_DIR) / "examples" / "gpio_blinky.elsim-bin";

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
