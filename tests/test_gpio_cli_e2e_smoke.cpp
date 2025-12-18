#include <gtest/gtest.h>

#include <cstdint>
#include <sstream>
#include <string>

#include "elsim/core/BoardConfigParser.hpp"
#include "elsim/core/BoardDescription.hpp"
#include "elsim/core/Logger.hpp"
#include "elsim/core/Simulator.hpp"
#include "elsim/device/VirtualButtonDevice.hpp"
#include "elsim/device/VirtualLedDevice.hpp"
#include "src/cli/monitor/MonitorRenderers.hpp"
#include "src/cli/monitor/MonitorSnapshot.hpp"

namespace {

std::string BoardPath() {
#ifdef ELSIM_SOURCE_DIR
    return std::string(ELSIM_SOURCE_DIR) + "/examples/board-examples/gpio-led-button-board.yaml";
#else
    // Fallback for local runs if compile definition is missing (should not be used in CI).
    return "../examples/board-examples/gpio-led-button-board.yaml";
#endif
}

elsim::cli::monitor::MonitorSnapshot MakeSnapshot(const elsim::core::BoardDescription& board,
                                                  const elsim::core::Simulator& sim) {
    elsim::cli::monitor::MonitorSnapshot s{};
    s.board_name = board.name;

    if (auto gpio = sim.gpioController()) {
        s.gpio_dir = static_cast<std::uint32_t>(gpio->getDirectionMask());
        s.gpio_in = static_cast<std::uint32_t>(gpio->getInputMask());
        s.gpio_out = static_cast<std::uint32_t>(gpio->getOutputMask());
    }

    auto leds = sim.ledDevices();
    s.leds.reserve(leds.size());
    for (const auto* led : leds) {
        elsim::cli::monitor::LedSnapshot ls{};
        ls.name = led->name();
        ls.pin = static_cast<std::uint32_t>(led->pin());
        ls.active_high = led->activeHigh();
        ls.is_on = led->isOn();
        s.leds.push_back(std::move(ls));
    }

    return s;
}

}  // namespace

TEST(GpioCliE2eSmoke, PressAffectsGpioInAndMonitorJsonContractIsStable) {
    const std::string configPath = BoardPath();

    // Keep output clean in tests.
    elsim::core::Logger::instance().set_level(elsim::core::LogLevel::Off);

    const auto board = elsim::core::BoardConfigParser::loadFromFile(configPath);

    std::ostringstream simLog;
    elsim::core::Simulator sim(simLog);
    sim.loadBoard(board);

    // Sanity: we expect exactly one button and one LED from the demo board.
    const auto buttons = sim.buttonDevices();
    ASSERT_FALSE(buttons.empty());
    ASSERT_NE(buttons[0], nullptr);
    EXPECT_EQ(buttons[0]->name(), "btn1");

    const auto leds = sim.ledDevices();
    ASSERT_FALSE(leds.empty());
    ASSERT_NE(leds[0], nullptr);
    EXPECT_EQ(leds[0]->name(), "led1");

    // Initial snapshot.
    auto snap0 = MakeSnapshot(board, sim);

    // btn1 is pin=1 active_high=true => pressed should set bit1 in gpio_in.
    constexpr std::uint32_t kBtnMask = (1u << 1);

    EXPECT_EQ((snap0.gpio_in & kBtnMask), 0u);

    // Verify JSON schema basics (TASK-10.4 stable contract).
    const std::string json0 = elsim::cli::monitor::RenderMonitorJson(snap0);
    EXPECT_NE(json0.find("\"board\""), std::string::npos);
    EXPECT_NE(json0.find("\"name\": \"" + board.name + "\""), std::string::npos);
    EXPECT_NE(json0.find("\"gpio\""), std::string::npos);
    EXPECT_NE(json0.find("\"dir\": \""), std::string::npos);
    EXPECT_NE(json0.find("\"in\":  \""), std::string::npos);
    EXPECT_NE(json0.find("\"out\": \""), std::string::npos);
    EXPECT_NE(json0.find("\"leds\""), std::string::npos);
    EXPECT_NE(json0.find("\"state\": \""), std::string::npos);

    // Press.
    auto* btn = buttons[0];
    btn->press();

    auto snap1 = MakeSnapshot(board, sim);
    EXPECT_EQ((snap1.gpio_in & kBtnMask), kBtnMask);

    // Release.
    btn->release();

    auto snap2 = MakeSnapshot(board, sim);
    EXPECT_EQ((snap2.gpio_in & kBtnMask), 0u);

    // Optional: LED should not change automatically without a program (document current behavior).
    // We keep this non-fatal because it depends on future ISA/demo evolution.
    // EXPECT_EQ(snap2.leds[0].state, "OFF"); // (we only have is_on bool in snapshot)
}
