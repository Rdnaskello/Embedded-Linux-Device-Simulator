#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "elsim/core/BoardConfigParser.hpp"

using elsim::core::BoardConfigErrorCode;
using elsim::core::BoardConfigException;
using elsim::core::BoardConfigParser;

namespace {

std::string srcPath(const std::string& rel) {
    const std::filesystem::path p = std::filesystem::path(ELSIM_SOURCE_DIR) / rel;
    return p.string();
}

TEST(BoardConfigParser_GpioLedButton, LedWithoutGpio_ThrowsMissingField) {
    try {
        (void)BoardConfigParser::loadFromFile(srcPath("examples/board-examples/invalid-led-without-gpio.yaml"));
        FAIL() << "Expected BoardConfigException";
    } catch (const BoardConfigException& ex) {
        EXPECT_EQ(ex.code(), BoardConfigErrorCode::MissingField);
    }
}

TEST(BoardConfigParser_GpioLedButton, PinOutOfRange_ThrowsInvalidValue) {
    try {
        (void)BoardConfigParser::loadFromFile(srcPath("examples/board-examples/invalid-pin-out-of-range.yaml"));
        FAIL() << "Expected BoardConfigException";
    } catch (const BoardConfigException& ex) {
        EXPECT_EQ(ex.code(), BoardConfigErrorCode::InvalidValue);
    }
}

TEST(BoardConfigParser_GpioLedButton, DuplicatePin_ThrowsInvalidValue) {
    try {
        (void)BoardConfigParser::loadFromFile(srcPath("examples/board-examples/invalid-duplicate-pin.yaml"));
        FAIL() << "Expected BoardConfigException";
    } catch (const BoardConfigException& ex) {
        EXPECT_EQ(ex.code(), BoardConfigErrorCode::InvalidValue);
    }
}

}  // namespace
