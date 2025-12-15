# CPU unit tests (TASK-6.6)
add_executable(cpu_tests
    test_fakecpu_core.cpp
)

target_link_libraries(cpu_tests
    PRIVATE
        elsim_core
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(cpu_tests)

# MMIO / MemoryBus contract tests (TASK-8.0c)
add_executable(memory_bus_tests
    test_memory_bus.cpp
)

target_link_libraries(memory_bus_tests
    PRIVATE
        elsim_core
        GTest::gtest_main
)

add_test(NAME memory_bus_tests COMMAND memory_bus_tests)
