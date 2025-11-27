cmake_minimum_required(VERSION 3.16) 

project(EmbeddedLinuxDeviceSimulator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Core engine library (CPU etc.)
add_library(elsim_core
    src/core/FakeCpu.cpp
    src/core/MemoryBus.cpp
)

target_include_directories(elsim_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Example: basic memory bus smoke test
add_executable(basic_memory_test
    examples/basic_memory_test.cpp
)

target_link_libraries(basic_memory_test
    PRIVATE
        elsim_core
)