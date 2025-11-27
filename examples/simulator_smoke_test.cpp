#include <iostream>

#include "elsim/core/ICpu.hpp"
#include "elsim/core/Simulator.hpp"
#include "elsim/device/IDevicesTickable.hpp"

using namespace elsim;

// ---------------------------
// Фейковий CPU, щоб тестувати Simulator
// ---------------------------
class TestCpu : public core::ICpu {
   public:
    int stepCount = 0;

    void step() override { ++stepCount; }

    void reset() override { stepCount = 0; }

    bool loadImage(const std::string& /*path*/) override {
        // Для цього smoke-тесту нам все одно, просто повернемо true
        return true;
    }

    void setMemoryBus(std::shared_ptr<core::IMemoryBus> /*bus*/) override {
        // У цьому тесті шину не використовуємо
    }
};
// ---------------------------
// Фейковий DeviceManager
// ---------------------------
class TestDevices : public device::IDevicesTickable {
   public:
    int tickCount = 0;

    void tickAll() override { ++tickCount; }
};

int main() {
    TestCpu cpu;
    TestDevices dev;

    core::Simulator sim(cpu, dev);

    sim.start(5);  // запускаємо рівно 5 тiкiв

    std::cout << "CPU stepCount = " << cpu.stepCount << "\n";
    std::cout << "DEV tickCount = " << dev.tickCount << "\n";
    std::cout << "sim.cycleCount() = " << sim.cycleCount() << "\n";

    if (cpu.stepCount == 5 && dev.tickCount == 5 && sim.cycleCount() == 5) {
        std::cout << "Simulator smoke test PASSED\n";
    } else {
        std::cout << "Simulator smoke test FAILED\n";
    }

    return 0;
}
