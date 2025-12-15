#include <gtest/gtest.h>

#include "elsim/core/GpioController.hpp"
#include "elsim/device/VirtualLedDevice.hpp"

using elsim::VirtualLedDevice;
using elsim::core::GpioController;

TEST(VirtualLedDevice, ActiveHigh_TogglesOnEffectiveOutput) {
    auto gpio = std::make_shared<GpioController>(8);
    VirtualLedDevice led("led0", gpio, 0, true);

    EXPECT_FALSE(led.isOn());

    // OUT=1 while input -> still off
    gpio->writeOutput(0, true);
    EXPECT_FALSE(led.isOn());

    // Make it output -> effective becomes 1 -> on
    gpio->setDirection(0, true);
    EXPECT_TRUE(led.isOn());

    // Output low -> off
    gpio->writeOutput(0, false);
    EXPECT_FALSE(led.isOn());
}

TEST(VirtualLedDevice, ActiveLow_Inverts) {
    auto gpio = std::make_shared<GpioController>(8);
    VirtualLedDevice led("led0", gpio, 0, false);

    // active_low: when effective level is 0 -> LED is ON
    EXPECT_TRUE(led.isOn());

    // OUT=1 while input -> effective 0, still ON
    gpio->writeOutput(0, true);
    EXPECT_TRUE(led.isOn());

    // Make it output -> effective becomes 1 -> LED should turn OFF
    gpio->setDirection(0, true);
    EXPECT_FALSE(led.isOn());
}

TEST(VirtualLedDevice, UnsubscribeOnDestruction_NoCrashOnFurtherGpioChanges) {
    auto gpio = std::make_shared<elsim::core::GpioController>(8);

    {
        elsim::VirtualLedDevice led("led0", gpio, 1, true);
        // optional: drive something while alive
        gpio->setDirection(1, true);
        gpio->writeOutput(1, true);
        EXPECT_TRUE(led.isOn());
    }  // led destroyed -> must unsubscribe safely

    // If unsubscribe is correct, these must NOT call a dead callback (no UAF).
    gpio->setDirection(1, true);
    gpio->writeOutput(1, false);
    gpio->writeOutput(1, true);

    SUCCEED();
}