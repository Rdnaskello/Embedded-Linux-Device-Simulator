cmake_minimum_required(VERSION 3.16)

project(EmbeddedLinuxDeviceSimulator VERSION 0.2.0 LANGUAGES CXX)
# --- Project version header (generated) ---
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/elsim/Version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/elsim/Version.hpp
    @ONLY
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Знаходимо yaml-cpp (потрібен для парсера board.yaml)
find_package(yaml-cpp REQUIRED)

# Core engine library (CPU etc.)
add_library(elsim_core
    src/core/FakeCpu.cpp
    src/core/MemoryBus.cpp
    src/core/Simulator.cpp
    src/core/Logger.cpp
    src/core/BoardConfigParser.cpp
    src/device/DeviceFactory.cpp
    src/device/UartDevice.cpp
    src/device/TimerDevice.cpp
    src/core/MemoryBusAdapter.cpp
    src/core/DeviceMemoryAdapter.cpp
    src/core/ProgramLoader.cpp
    src/core/GpioController.cpp
    src/device/GpioDevice.cpp
    src/device/VirtualLedDevice.cpp
    src/device/VirtualButtonDevice.cpp
)

target_include_directories(elsim_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/generated
)

target_link_libraries(elsim_core
    PUBLIC
        yaml-cpp
)

# Example: basic memory bus smoke test
add_executable(basic_memory_test
    examples/basic_memory_test.cpp
)

target_link_libraries(basic_memory_test
    PRIVATE
        elsim_core
)

# Example: basic CPU memory test
add_executable(basic_cpu_memory_test
    examples/basic_cpu_memory_test.cpp
)

target_link_libraries(basic_cpu_memory_test
    PRIVATE
        elsim_core
)

# Example: simulator smoke test
add_executable(simulator_smoke_test
    examples/simulator_smoke_test.cpp
)

target_link_libraries(simulator_smoke_test
    PRIVATE
        elsim_core
)

# Example: UART smoke test
add_executable(uart_smoke_test
    examples/uart_smoke_test.cpp
)

target_link_libraries(uart_smoke_test
    PRIVATE
        elsim_core
)

# Example: Timer device smoke test
add_executable(timer_smoke_test
    examples/timer_smoke_test.cpp
)

target_link_libraries(timer_smoke_test
    PRIVATE
        elsim_core
)

add_executable(uart_timer_demo
    examples/uart-timer-demo/main.cpp
)

target_link_libraries(uart_timer_demo
    PRIVATE
        elsim_core
)

# Example fakecpu decode test
add_executable(fakecpu_decode_smoke_test
    examples/fakecpu_decode_smoke_test.cpp
)

target_link_libraries(fakecpu_decode_smoke_test
    PRIVATE
        elsim_core
)

# Example fakecpu_exec_smoke_test
add_executable(fakecpu_exec_smoke_test
    examples/fakecpu_exec_smoke_test.cpp
)

target_link_libraries(fakecpu_exec_smoke_test
    PRIVATE
        elsim_core
)

# Example: program loader smoke test
add_executable(program_loader_smoke_test
    examples/program_loader_smoke_test.cpp
)

target_link_libraries(program_loader_smoke_test
    PRIVATE
        elsim_core
)

# Example: hello from ELSIM
add_executable(build_hello_program
    examples/build_hello_program.cpp
)

target_link_libraries(build_hello_program
    PRIVATE
        elsim_core
)

# Щоб build_hello_program завжди міг писати в repo/examples незалежно від cwd
target_compile_definitions(build_hello_program
    PRIVATE
        PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

# Example: gpio blinky program builder (TASK-8.6)
add_executable(build_gpio_blinky_program
    examples/build_gpio_blinky_program.cpp
)

target_link_libraries(build_gpio_blinky_program
    PRIVATE
        elsim_core
)

# Щоб build_gpio_blinky_program завжди міг писати в repo/examples незалежно від cwd
target_compile_definitions(build_gpio_blinky_program
    PRIVATE
        PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

# CLI executable: elsim
add_executable(elsim
    src/cli/main.cpp
    src/cli/CliApp.cpp
    src/cli/commands/RunCommand.cpp
    src/cli/commands/ListBoardsCommand.cpp
    src/cli/commands/HelpCommand.cpp
    src/cli/commands/MonitorCommand.cpp
    src/cli/commands/PressCommand.cpp
)

target_link_libraries(elsim
    PRIVATE
        elsim_core
)

target_compile_features(elsim PRIVATE cxx_std_20)

# === Unit tests for FakeCPU core (TASK-6.6) ===

include(FetchContent)

# Прибираємо warning про timestamps для FetchContent (CMP0135)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Для Windows/MSVC — уникнути конфлікту CRT
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()
add_subdirectory(tests)
