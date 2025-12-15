#include <gtest/gtest.h>

#include "elsim/core/GpioController.hpp"
#include "elsim/device/VirtualButtonDevice.hpp"

using elsim::VirtualButtonDevice;
using elsim::core::GpioController;

TEST(VirtualButtonDevice, ActiveHigh_PressRelease_InjectsExpectedInputLevel) {
    auto gpio = std::make_shared<GpioController>(8);
    VirtualButtonDevice btn("btn0", gpio, 2, true);

    // released -> level 0
    EXPECT_FALSE(btn.isPressed());
    EXPECT_FALSE(gpio->readInput(2));

    btn.press();
    EXPECT_TRUE(btn.isPressed());
    EXPECT_TRUE(gpio->readInput(2));

    btn.release();
    EXPECT_FALSE(btn.isPressed());
    EXPECT_FALSE(gpio->readInput(2));
}

TEST(VirtualButtonDevice, ActiveLow_InvertsLevels) {
    auto gpio = std::make_shared<GpioController>(8);
    VirtualButtonDevice btn("btn0", gpio, 3, false);

    // released -> level 1
    EXPECT_FALSE(btn.isPressed());
    EXPECT_TRUE(gpio->readInput(3));

    btn.press();
    EXPECT_TRUE(btn.isPressed());
    EXPECT_FALSE(gpio->readInput(3));

    btn.release();
    EXPECT_FALSE(btn.isPressed());
    EXPECT_TRUE(gpio->readInput(3));
}
