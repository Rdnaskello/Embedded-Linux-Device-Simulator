#include <iostream>

#include "elsim/core/Simulator.hpp"

int main() {
    using elsim::core::Simulator;

    // Створюємо симулятор з логом у std::cout
    Simulator sim(std::cout);

    std::cout << "[simulator_smoke_test] Simulator constructed successfully\n";

    // Пізніше, коли буде повна реалізація loadBoard(),
    // сюди додамо:
    //  - парсинг BoardDescription
    //  - sim.loadBoard(board);
    //  - sim.start(...);

    return 0;
}
