#include "elsim/core/ProgramLoader.hpp"

#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "elsim/core/Logger.hpp"

namespace {

constexpr std::string_view COMPONENT = "LOADER";

}  // namespace

namespace elsim::core {

void ProgramLoader::loadBinary(const std::string& path, MemoryBus& memory, std::uint32_t& entryPoint) {
    auto& logger = Logger::instance();

    // 1. Відкриваємо файл у двійковому режимі
    logger.info(COMPONENT, std::string("Loading program from '") + path + "'");

    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("ProgramLoader: failed to open file: " + path);
    }

    // 2. Читаємо заголовок
    ElsimBinaryHeader header{};
    in.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (in.gcount() != static_cast<std::streamsize>(sizeof(header))) {
        throw std::runtime_error("ProgramLoader: file is too short to contain valid header: " + path);
    }

    // 3. Перевіряємо magic
    if (header.magic != ELSIM_BINARY_MAGIC) {
        char buf[128];
        std::snprintf(buf, sizeof(buf), "Invalid magic in file '%s': got 0x%08X, expected 0x%08X", path.c_str(),
                      static_cast<unsigned int>(header.magic), static_cast<unsigned int>(ELSIM_BINARY_MAGIC));
        logger.error(COMPONENT, buf);
        throw std::runtime_error("ProgramLoader: invalid magic in file: " + path);
    }

    // 4. Перевіряємо code_size
    if (header.code_size == 0) {
        logger.error(COMPONENT, "Program has zero code_size");
        throw std::runtime_error("ProgramLoader: code_size is zero in file: " + path);
    }

    // (опційно) попередження, якщо розмір не кратний 4 байтам (розмір інструкції FakeCpu)
    if ((header.code_size % 4u) != 0u) {
        char buf[128];
        std::snprintf(buf, sizeof(buf), "Warning: code_size (0x%X) is not aligned to 4-byte instruction size",
                      header.code_size);
        logger.error(COMPONENT, buf);  // поки як error, можна змінити на warning, коли буде рівень warn
    }

    char hdrBuf[160];
    std::snprintf(hdrBuf, sizeof(hdrBuf),
                  "ElsimBinaryHeader: magic=0x%08X, entry_point=0x%08X, code_size=0x%X (%u bytes)",
                  static_cast<unsigned int>(header.magic), static_cast<unsigned int>(header.entry_point),
                  header.code_size, header.code_size);
    logger.debug(COMPONENT, hdrBuf);

    // 5. Читаємо блок коду
    std::vector<std::uint8_t> code(header.code_size);
    in.read(reinterpret_cast<char*>(code.data()), header.code_size);

    if (in.gcount() != static_cast<std::streamsize>(header.code_size)) {
        logger.error(COMPONENT, "Program code section is truncated");
        throw std::runtime_error("ProgramLoader: code section is truncated in file: " + path);
    }

    // 6. Завантажуємо код у пам'ять, починаючи з entry_point
    const std::uint32_t base = header.entry_point;

    char loadBuf[160];
    std::snprintf(loadBuf, sizeof(loadBuf), "Writing code to MemoryBus: base=0x%08X, size=0x%X (%u bytes)",
                  static_cast<unsigned int>(base), header.code_size, header.code_size);
    logger.debug(COMPONENT, loadBuf);

    for (std::uint32_t i = 0; i < header.code_size; ++i) {
        // Якщо адреса вийде за межі RAM, MemoryBus::write8 кине std::out_of_range.
        memory.write8(base + i, code[i]);
    }

    // 7. Повертаємо entryPoint назовні
    entryPoint = header.entry_point;

    char doneBuf[128];
    std::snprintf(doneBuf, sizeof(doneBuf), "Program loaded successfully. Entry point = 0x%08X",
                  static_cast<unsigned int>(entryPoint));
    logger.info(COMPONENT, doneBuf);
}

}  // namespace elsim::core
