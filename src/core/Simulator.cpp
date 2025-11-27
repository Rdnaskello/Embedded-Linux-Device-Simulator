#include "elsim/core/Simulator.hpp"

#include "elsim/core/ICpu.hpp"
#include "elsim/device/IDevicesTickable.hpp"

namespace elsim::core {

Simulator::Simulator(ICpu& cpu, device::IDevicesTickable& devices, std::ostream& log)
    : cpu_(cpu), devices_(devices), log_(log), running_(false), cycleCount_(0) {}

void Simulator::start(std::uint64_t maxCycles) {
    if (running_) {
        log_ << "[SIM] start() called, but simulation is already running.\n";
        return;
    }

    running_ = true;

    log_ << "[SIM] Starting simulation";
    if (maxCycles != 0) {
        log_ << " for max " << maxCycles << " cycles";
    }
    log_ << "...\n";

    const std::uint64_t startCycle = cycleCount_;

    while (running_ && (maxCycles == 0 || (cycleCount_ - startCycle) < maxCycles)) {
        runOneTick();
    }

    const std::uint64_t executed = cycleCount_ - startCycle;

    log_ << "[SIM] Simulation finished. Executed " << executed << " cycles, total cycleCount = " << cycleCount_
         << ".\n";

    running_ = false;
}

void Simulator::stop() {
    if (!running_) {
        log_ << "[SIM] stop() called, but simulation is not running.\n";
        return;
    }

    log_ << "[SIM] Stop requested.\n";
    running_ = false;
}

void Simulator::runOneTick() {
    // Один логічний такт симуляції:
    //  1) CPU виконує крок.
    //  2) Оновлюємо всі пристрої.
    cpu_.step();
    devices_.tickAll();
    ++cycleCount_;
}

bool Simulator::isRunning() const noexcept { return running_; }

std::uint64_t Simulator::cycleCount() const noexcept { return cycleCount_; }

}  // namespace elsim::core
