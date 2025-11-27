cmake_minimum_required(VERSION 3.16) 

project(EmbeddedLinuxDeviceSimulator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Core engine library (CPU etc.)
add_library(elsim_core
src/core/FakeCpu.cpp
)

target_include_directories(elsim_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)