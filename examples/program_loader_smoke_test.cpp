#include <iostream>
#include <vector>

#include "elsim/core/Logger.hpp"
#include "elsim/core/MemoryBus.hpp"
#include "elsim/core/ProgramLoader.hpp"

using namespace elsim::core;

int main() {
    Logger::instance().set_level(LogLevel::Debug);

    try {
        // 1. Створюємо шину пам'яті (64 KB)
        MemoryBus memory(64 * 1024);

        // 2. Створюємо loader
        ProgramLoader loader;

        // 3. Параметри
        std::string path = "../examples/test_program.elsim-bin";
        uint32_t entry = 0;

        // 4. Завантажуємо файл
        loader.loadBinary(path, memory, entry);

        std::cout << "Entry point: 0x" << std::hex << entry << std::dec << "\n";

        // 5. Читаємо перші 8 байт завантаженої програми
        std::cout << "First bytes: ";
        for (int i = 0; i < 8; i++) {
            std::cout << "0x" << std::hex << static_cast<int>(memory.read8(entry + i)) << " ";
        }
        std::cout << "\n";

    } catch (const std::exception& ex) {
        std::cerr << "ERROR: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
