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

# GPIO unit tests (TASK-8.1)
add_executable(gpio_tests
    test_gpio_controller.cpp
)

target_link_libraries(gpio_tests
    PRIVATE
        elsim_core
        GTest::gtest_main
)

gtest_discover_tests(gpio_tests)

# GPIO MMIO tests (TASK-8.2)
add_executable(gpio_mmio_tests
    test_gpio_mmio.cpp
)

target_link_libraries(gpio_mmio_tests
    PRIVATE
        elsim_core
        GTest::gtest_main
)

gtest_discover_tests(gpio_mmio_tests)

# Virtual LED device tests (TASK-8.3)
add_executable(virtual_led_tests    
    test_virtual_led_device.cpp
)   
target_link_libraries(virtual_led_tests
    PRIVATE
        elsim_core
        GTest::gtest_main
)
gtest_discover_tests(virtual_led_tests)


# GPIO MMIO + Virtual LED integration test (TASK-8.3b)
add_executable(gpio_led_integration_tests
    test_gpio_led_integration.cpp
)

target_link_libraries(gpio_led_integration_tests
    PRIVATE
        elsim_core
        GTest::gtest_main
)

gtest_discover_tests(gpio_led_integration_tests)

# Virtual BUTTON device tests (TASK-8.4)
add_executable(virtual_button_tests
    test_virtual_button_device.cpp
)

target_link_libraries(virtual_button_tests
    PRIVATE
        elsim_core
        GTest::gtest_main
)

gtest_discover_tests(virtual_button_tests)

# BoardConfigParser GPIO/LED/Button YAML tests (TASK-8.5)
add_executable(board_config_parser_gpio_led_button_tests
    test_board_config_parser_gpio_led_button.cpp
)

target_link_libraries(board_config_parser_gpio_led_button_tests
    PRIVATE
        elsim_core
        GTest::gtest_main
)

target_compile_definitions(board_config_parser_gpio_led_button_tests
    PRIVATE
        ELSIM_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

gtest_discover_tests(board_config_parser_gpio_led_button_tests)

# GPIO CLI E2E smoke test (TASK-10.5)
add_executable(gpio_cli_e2e_smoke_tests
    test_gpio_cli_e2e_smoke.cpp
    ../src/cli/monitor/MonitorRenderers.cpp
)

target_link_libraries(gpio_cli_e2e_smoke_tests
    PRIVATE
        elsim_core
        GTest::gtest_main
)

target_compile_definitions(gpio_cli_e2e_smoke_tests
    PRIVATE
        ELSIM_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_include_directories(gpio_cli_e2e_smoke_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}
)


gtest_discover_tests(gpio_cli_e2e_smoke_tests)