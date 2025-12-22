# ELSIM v0.3.0 — Candidate Components Map (By Build Targets)

Rules of evidence:
- Source-of-truth artifact set (V): docs/audit/audit_v0_3_0_v_canon.txt
- Evidence edges: docs/audit/audit_v0_3_0_edges_targets.tsv (TARGET_HAS_SOURCE)
- Grouping method: each file is assigned to exactly one build target using lexicographic tie-break if multiple targets reference the same file.
- Snapshot: cb4b80b

## Evidence Artifacts (Edge Lists)
- docs/audit/audit_v0_3_0_edges_targets.tsv (TARGET_HAS_SOURCE)

## Component-A
### Included Artifacts
- examples/basic_cpu_memory_test.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:basic_cpu_memory_test -> examples/basic_cpu_memory_test.cpp

## Component-B
### Included Artifacts
- examples/basic_memory_test.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:basic_memory_test -> examples/basic_memory_test.cpp

## Component-C
### Included Artifacts
- tests/test_board_config_parser_gpio_led_button.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:board_config_parser_gpio_led_button_tests -> tests/test_board_config_parser_gpio_led_button.cpp

## Component-D
### Included Artifacts
- examples/build_gpio_blinky_program.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:build_gpio_blinky_program -> examples/build_gpio_blinky_program.cpp

## Component-E
### Included Artifacts
- examples/build_hello_program.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:build_hello_program -> examples/build_hello_program.cpp

## Component-F
### Included Artifacts
- tests/test_fakecpu_core.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:cpu_tests -> tests/test_fakecpu_core.cpp

## Component-G
### Included Artifacts
- src/cli/CliApp.cpp
- src/cli/commands/HelpCommand.cpp
- src/cli/commands/ListBoardsCommand.cpp
- src/cli/commands/MonitorCommand.cpp
- src/cli/commands/PressCommand.cpp
- src/cli/commands/RunCommand.cpp
- src/cli/main.cpp
- src/cli/monitor/MonitorRenderers.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim -> src/cli/CliApp.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim -> src/cli/commands/HelpCommand.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim -> src/cli/commands/ListBoardsCommand.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim -> src/cli/commands/MonitorCommand.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim -> src/cli/commands/PressCommand.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim -> src/cli/commands/RunCommand.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim -> src/cli/main.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim -> src/cli/monitor/MonitorRenderers.cpp

## Component-H
### Included Artifacts
- src/core/BoardConfigParser.cpp
- src/core/DeviceMemoryAdapter.cpp
- src/core/FakeCpu.cpp
- src/core/GpioController.cpp
- src/core/Logger.cpp
- src/core/MemoryBus.cpp
- src/core/MemoryBusAdapter.cpp
- src/core/ProgramLoader.cpp
- src/core/Simulator.cpp
- src/device/DeviceFactory.cpp
- src/device/GpioDevice.cpp
- src/device/TimerDevice.cpp
- src/device/UartDevice.cpp
- src/device/VirtualButtonDevice.cpp
- src/device/VirtualLedDevice.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim_core -> src/core/BoardConfigParser.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim_core -> src/core/DeviceMemoryAdapter.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim_core -> src/core/FakeCpu.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim_core -> src/core/GpioController.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim_core -> src/core/Logger.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim_core -> src/core/MemoryBus.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim_core -> src/core/MemoryBusAdapter.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim_core -> src/core/ProgramLoader.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim_core -> src/core/Simulator.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim_core -> src/device/DeviceFactory.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim_core -> src/device/GpioDevice.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim_core -> src/device/TimerDevice.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim_core -> src/device/UartDevice.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim_core -> src/device/VirtualButtonDevice.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:elsim_core -> src/device/VirtualLedDevice.cpp

## Component-I
### Included Artifacts
- examples/fakecpu_decode_smoke_test.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:fakecpu_decode_smoke_test -> examples/fakecpu_decode_smoke_test.cpp

## Component-J
### Included Artifacts
- examples/fakecpu_exec_smoke_test.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:fakecpu_exec_smoke_test -> examples/fakecpu_exec_smoke_test.cpp

## Component-K
### Included Artifacts
- tests/test_gpio_cli_e2e_smoke.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:gpio_cli_e2e_smoke_tests -> tests/test_gpio_cli_e2e_smoke.cpp

## Component-L
### Included Artifacts
- tests/test_gpio_led_integration.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:gpio_led_integration_tests -> tests/test_gpio_led_integration.cpp

## Component-M
### Included Artifacts
- tests/test_gpio_mmio.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:gpio_mmio_tests -> tests/test_gpio_mmio.cpp

## Component-N
### Included Artifacts
- tests/test_gpio_controller.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:gpio_tests -> tests/test_gpio_controller.cpp

## Component-O
### Included Artifacts
- tests/test_memory_bus.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:memory_bus_tests -> tests/test_memory_bus.cpp

## Component-P
### Included Artifacts
- examples/program_loader_smoke_test.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:program_loader_smoke_test -> examples/program_loader_smoke_test.cpp

## Component-Q
### Included Artifacts
- examples/simulator_smoke_test.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:simulator_smoke_test -> examples/simulator_smoke_test.cpp

## Component-R
### Included Artifacts
- examples/timer_smoke_test.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:timer_smoke_test -> examples/timer_smoke_test.cpp

## Component-S
### Included Artifacts
- examples/uart_smoke_test.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:uart_smoke_test -> examples/uart_smoke_test.cpp

## Component-T
### Included Artifacts
- examples/uart-timer-demo/main.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:uart_timer_demo -> examples/uart-timer-demo/main.cpp

## Component-U
### Included Artifacts
- tests/test_virtual_button_device.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:virtual_button_tests -> tests/test_virtual_button_device.cpp

## Component-V
### Included Artifacts
- tests/test_virtual_led_device.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): TARGET_HAS_SOURCE: cmake-target:virtual_led_tests -> tests/test_virtual_led_device.cpp

## Standalone Artifacts
### Included Artifacts
- .clang-format
- .clang-tidy
- .gitignore
- CHANGELOG.md
- CMakeLists.txt
- LICENSE
- README.md
- docs/audit/audit_v0_3_0_filelist.txt
- docs/audit/audit_v0_3_0_index.md
- docs/audit/audit_v0_3_0_manifest.sha256
- docs/audit/audit_v0_3_0_snapshot.txt
- docs/elsim_binary_format.md
- docs/fakecpu_isa.md
- docs/gpio_mmio.md
- docs/mmio_contract.md
- examples/board-examples/device-factory-test.yaml
- examples/board-examples/empty-test.yaml
- examples/board-examples/gpio-blinky-board.yaml
- examples/board-examples/gpio-led-button-board.yaml
- examples/board-examples/gpio-mmio-test.yaml
- examples/board-examples/hello-board.yaml
- examples/board-examples/invalid-duplicate-pin.yaml
- examples/board-examples/invalid-led-without-gpio.yaml
- examples/board-examples/invalid-pin-out-of-range.yaml
- examples/board-examples/minimal-board.yaml
- examples/gpio_blinky.elsim-bin
- examples/hello.elsim-bin
- examples/test_program.elsim-bin
- examples/uart-timer-demo/board.yaml
- include/elsim/Version.hpp.in
- include/elsim/core/BoardConfigParser.hpp
- include/elsim/core/BoardDescription.hpp
- include/elsim/core/DeviceMemoryAdapter.hpp
- include/elsim/core/FakeCpu.hpp
- include/elsim/core/GpioController.hpp
- include/elsim/core/ICpu.hpp
- include/elsim/core/IMemoryBus.hpp
- include/elsim/core/IMemoryMappedDevice.hpp
- include/elsim/core/Logger.hpp
- include/elsim/core/MemoryBus.hpp
- include/elsim/core/MemoryBusAdapter.hpp
- include/elsim/core/ProgramLoader.hpp
- include/elsim/core/Simulator.hpp
- include/elsim/device/BaseDevice.hpp
- include/elsim/device/DeviceFactory.hpp
- include/elsim/device/DeviceManager.hpp
- include/elsim/device/GpioDevice.hpp
- include/elsim/device/IDevice.hpp
- include/elsim/device/IDevicesTickable.hpp
- include/elsim/device/TimerDevice.hpp
- include/elsim/device/UartDevice.hpp
- include/elsim/device/VirtualButtonDevice.hpp
- include/elsim/device/VirtualLedDevice.hpp
- src/cli/CliApp.hpp
- src/cli/commands/HelpCommand.hpp
- src/cli/commands/ListBoardsCommand.hpp
- src/cli/commands/MonitorCommand.hpp
- src/cli/commands/PressCommand.hpp
- src/cli/commands/RunCommand.hpp
- src/cli/monitor/MonitorRenderers.hpp
- src/cli/monitor/MonitorSnapshot.hpp
- tests/CMakeLists.txt

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: .clang-format
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: .clang-tidy
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: .gitignore
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: CHANGELOG.md
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: CMakeLists.txt
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: LICENSE
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: README.md
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: docs/audit/audit_v0_3_0_filelist.txt
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: docs/audit/audit_v0_3_0_index.md
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: docs/audit/audit_v0_3_0_manifest.sha256
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: docs/audit/audit_v0_3_0_snapshot.txt
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: docs/elsim_binary_format.md
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: docs/fakecpu_isa.md
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: docs/gpio_mmio.md
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: docs/mmio_contract.md
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: examples/board-examples/device-factory-test.yaml
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: examples/board-examples/empty-test.yaml
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: examples/board-examples/gpio-blinky-board.yaml
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: examples/board-examples/gpio-led-button-board.yaml
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: examples/board-examples/gpio-mmio-test.yaml
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: examples/board-examples/hello-board.yaml
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: examples/board-examples/invalid-duplicate-pin.yaml
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: examples/board-examples/invalid-led-without-gpio.yaml
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: examples/board-examples/invalid-pin-out-of-range.yaml
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: examples/board-examples/minimal-board.yaml
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: examples/gpio_blinky.elsim-bin
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: examples/hello.elsim-bin
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: examples/test_program.elsim-bin
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: examples/uart-timer-demo/board.yaml
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/Version.hpp.in
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/core/BoardConfigParser.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/core/BoardDescription.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/core/DeviceMemoryAdapter.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/core/FakeCpu.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/core/GpioController.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/core/ICpu.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/core/IMemoryBus.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/core/IMemoryMappedDevice.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/core/Logger.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/core/MemoryBus.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/core/MemoryBusAdapter.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/core/ProgramLoader.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/core/Simulator.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/device/BaseDevice.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/device/DeviceFactory.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/device/DeviceManager.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/device/GpioDevice.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/device/IDevice.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/device/IDevicesTickable.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/device/TimerDevice.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/device/UartDevice.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/device/VirtualButtonDevice.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: include/elsim/device/VirtualLedDevice.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: src/cli/CliApp.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: src/cli/commands/HelpCommand.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: src/cli/commands/ListBoardsCommand.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: src/cli/commands/MonitorCommand.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: src/cli/commands/PressCommand.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: src/cli/commands/RunCommand.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: src/cli/monitor/MonitorRenderers.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: src/cli/monitor/MonitorSnapshot.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_targets.tsv): NO_TARGET_HAS_SOURCE: tests/CMakeLists.txt
