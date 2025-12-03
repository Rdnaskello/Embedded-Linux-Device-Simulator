cmake_minimum_required(VERSION 3.16) 

project(EmbeddedLinuxDeviceSimulator VERSION 0.1.0 LANGUAGES CXX)

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
)

target_include_directories(elsim_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
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

# CLI executable: elsim
add_executable(elsim
    src/cli/main.cpp
)

target_link_libraries(elsim
    PRIVATE
        elsim_core
)

target_compile_features(elsim PRIVATE cxx_std_20)
