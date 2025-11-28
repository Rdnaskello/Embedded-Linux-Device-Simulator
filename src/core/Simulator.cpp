#include "elsim/core/Simulator.hpp"

#include <string>

#include "elsim/core/ICpu.hpp"
#include "elsim/core/Logger.hpp"
#include "elsim/device/IDevicesTickable.hpp"

namespace elsim::core {

Simulator::Simulator(ICpu& cpu, device::IDevicesTickable& devices, std::ostream& log)
    : cpu_(cpu), devices_(devices), log_(log), running_(false), cycleCount_(0) {}

void Simulator::start(std::uint64_t maxCycles) {
    if (running_) {
        Logger::instance().error("Simulator", "start() called, but simulation is already running.");
        return;
    }

    running_ = true;

    // Формуємо повідомлення про старт
    std::string msg = "Starting simulation";
    if (maxCycles != 0) {
        msg += " for max " + std::to_string(maxCycles) + " cycles";
    }
    msg += "...";

    Logger::instance().info("Simulator", msg);

    const std::uint64_t startCycle = cycleCount_;

    while (running_ && (maxCycles == 0 || (cycleCount_ - startCycle) < maxCycles)) {
        runOneTick();
    }

    const std::uint64_t executed = cycleCount_ - startCycle;

    std::string finishedMsg = "Simulation finished. Executed " + std::to_string(executed) +
                              " cycles, total cycleCount = " + std::to_string(cycleCount_) + ".";

    Logger::instance().info("Simulator", finishedMsg);

    running_ = false;
}

void Simulator::stop() {
    if (!running_) {
        Logger::instance().error("Simulator", "stop() called, but simulation is not running.");
        return;
    }

    Logger::instance().info("Simulator", "Stop requested.");
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
