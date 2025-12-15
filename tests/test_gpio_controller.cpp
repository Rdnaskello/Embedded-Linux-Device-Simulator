#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>

#include "elsim/core/GpioController.hpp"

TEST(GpioController, ResetState_IsAllZeros) {
    elsim::core::GpioController gpio(/*pinCount=*/8);

    EXPECT_EQ(gpio.pinCount(), 8u);
    EXPECT_EQ(gpio.getDirectionMask(), 0u);
    EXPECT_EQ(gpio.getOutputMask(), 0u);
    EXPECT_EQ(gpio.getInputMask(), 0u);

    for (std::size_t i = 0; i < gpio.pinCount(); ++i) {
        EXPECT_FALSE(gpio.readInput(i));
    }
}

TEST(GpioController, SetDirectionAndWriteOutput_UpdatesOutMask) {
    elsim::core::GpioController gpio(/*pinCount=*/8);

    gpio.setDirection(/*pin=*/3, /*isOutput=*/true);
    gpio.writeOutput(/*pin=*/3, /*level=*/true);

    EXPECT_EQ(gpio.getDirectionMask(), (1ull << 3));
    EXPECT_EQ(gpio.getOutputMask(), (1ull << 3));

    gpio.writeOutput(/*pin=*/3, /*level=*/false);
    EXPECT_EQ(gpio.getOutputMask(), 0u);
}

TEST(GpioController, InjectInputAndReadInput_Works) {
    elsim::core::GpioController gpio(/*pinCount=*/8);

    gpio.injectInput(/*pin=*/5, /*level=*/true);
    EXPECT_TRUE(gpio.readInput(5));
    EXPECT_EQ(gpio.getInputMask(), (1ull << 5));

    gpio.injectInput(/*pin=*/5, /*level=*/false);
    EXPECT_FALSE(gpio.readInput(5));
    EXPECT_EQ(gpio.getInputMask(), 0u);
}

TEST(GpioController, InvalidPin_ThrowsOutOfRange) {
    elsim::core::GpioController gpio(/*pinCount=*/8);

    EXPECT_THROW(gpio.readInput(999), std::out_of_range);
    EXPECT_THROW(gpio.setDirection(999, true), std::out_of_range);
    EXPECT_THROW(gpio.writeOutput(999, true), std::out_of_range);
    EXPECT_THROW(gpio.injectInput(999, true), std::out_of_range);
}

TEST(GpioController, Callback_FiresOnlyOnChangeAndOnlyWhenDirIsOutput) {
    elsim::core::GpioController gpio(/*pinCount=*/8);

    int calls = 0;
    std::size_t lastPin = 0;
    bool lastLevel = false;

    const auto id = gpio.subscribeOnOutputChanged([&](std::size_t pin, bool level) {
        ++calls;
        lastPin = pin;
        lastLevel = level;
    });

    // DIR is input by default -> should NOT fire
    gpio.writeOutput(/*pin=*/2, /*level=*/true);
    EXPECT_EQ(calls, 0);

    // Set output, then write -> should fire once
    gpio.setDirection(/*pin=*/2, /*isOutput=*/true);
    gpio.writeOutput(/*pin=*/2, /*level=*/true);
    EXPECT_EQ(calls, 1);
    EXPECT_EQ(lastPin, 2u);
    EXPECT_TRUE(lastLevel);

    // Same level again -> no change -> no fire
    gpio.writeOutput(/*pin=*/2, /*level=*/true);
    EXPECT_EQ(calls, 1);

    // Toggle -> fire
    gpio.writeOutput(/*pin=*/2, /*level=*/false);
    EXPECT_EQ(calls, 2);
    EXPECT_EQ(lastPin, 2u);
    EXPECT_FALSE(lastLevel);

    // Unsubscribe -> no more fires
    gpio.unsubscribe(id);
    gpio.writeOutput(/*pin=*/2, /*level=*/true);
    EXPECT_EQ(calls, 2);
}
