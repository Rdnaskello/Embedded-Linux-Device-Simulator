#include "elsim/core/Logger.hpp"
#include "elsim/device/UartDevice.hpp"

using elsim::UartDevice;
using elsim::core::Logger;
using elsim::core::LogLevel;

int main() {
    // Enable detailed logging so we can see UART debug messages.
    Logger::instance().set_level(LogLevel::Debug);

    // Create UART device with some base address (not used in minimal model).
    UartDevice uart(0x100);

    // Write bytes to offset 0, which we treat as TX register.
    uart.write(0, 'H');
    uart.write(0, 'i');
    uart.write(0, '!');
    uart.write(0, '\n');

    // In the console you should see "Hi!"
    // + debug logs from UART in stderr/clog.
    return 0;
}
