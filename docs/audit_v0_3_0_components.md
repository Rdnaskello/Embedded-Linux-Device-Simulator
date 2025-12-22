# ELSIM v0.3.0 — Candidate Components Map

## Rules of Evidence
Input source: Audit Index (TASK-AUDIT-0.1). No other sources allowed.

Snapshot:
- git: cb4b80b

Relationship types used for clustering:
- DIRECT_INCLUDE: extracted from `#include "..."` among artifacts in V
- CO_TEST / CO_EXAMPLE / DIRECT_USE_LITERAL: extracted from exact string-literal path references

Clustering algorithm:
- Undirected graph over V using the relationship types above
- Candidate Components = connected components
- Standalone Artifact = vertex with degree 0
## Evidence Artifacts (Edge Lists)
- docs/audit/audit_v0_3_0_edges_includes.tsv (DIRECT_INCLUDE)
- docs/audit/audit_v0_3_0_edges_literals.tsv (CO_TEST / CO_EXAMPLE / DIRECT_USE_LITERAL)


## Inputs used (files)
- docs/audit/audit_v0_3_0_edges_includes.tsv
- docs/audit/audit_v0_3_0_edges_literals.tsv

## Input Artifact Set (V)
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
- examples/basic_cpu_memory_test.cpp
- examples/basic_memory_test.cpp
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
- examples/build_gpio_blinky_program.cpp
- examples/build_hello_program.cpp
- examples/fakecpu_decode_smoke_test.cpp
- examples/fakecpu_exec_smoke_test.cpp
- examples/gpio_blinky.elsim-bin
- examples/hello.elsim-bin
- examples/program_loader_smoke_test.cpp
- examples/simulator_smoke_test.cpp
- examples/test_program.elsim-bin
- examples/timer_smoke_test.cpp
- examples/uart-timer-demo/board.yaml
- examples/uart-timer-demo/main.cpp
- examples/uart_smoke_test.cpp
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
- src/cli/CliApp.cpp
- src/cli/CliApp.hpp
- src/cli/commands/HelpCommand.cpp
- src/cli/commands/HelpCommand.hpp
- src/cli/commands/ListBoardsCommand.cpp
- src/cli/commands/ListBoardsCommand.hpp
- src/cli/commands/MonitorCommand.cpp
- src/cli/commands/MonitorCommand.hpp
- src/cli/commands/PressCommand.cpp
- src/cli/commands/PressCommand.hpp
- src/cli/commands/RunCommand.cpp
- src/cli/commands/RunCommand.hpp
- src/cli/main.cpp
- src/cli/monitor/MonitorRenderers.cpp
- src/cli/monitor/MonitorRenderers.hpp
- src/cli/monitor/MonitorSnapshot.hpp
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
- tests/CMakeLists.txt
- tests/test_board_config_parser_gpio_led_button.cpp
- tests/test_fakecpu_core.cpp
- tests/test_gpio_cli_e2e_smoke.cpp
- tests/test_gpio_controller.cpp
- tests/test_gpio_led_integration.cpp
- tests/test_gpio_mmio.cpp
- tests/test_memory_bus.cpp
- tests/test_virtual_button_device.cpp
- tests/test_virtual_led_device.cpp

## Component-A
### Included Artifacts
- CMakeLists.txt
- include/elsim/Version.hpp.in

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_literals.tsv): DIRECT_USE_LITERAL: CMakeLists.txt -> include/elsim/Version.hpp.in

## Component-B
### Included Artifacts
- examples/basic_cpu_memory_test.cpp
- examples/basic_memory_test.cpp
- examples/board-examples/gpio-blinky-board.yaml
- examples/board-examples/gpio-led-button-board.yaml
- examples/board-examples/invalid-duplicate-pin.yaml
- examples/board-examples/invalid-led-without-gpio.yaml
- examples/board-examples/invalid-pin-out-of-range.yaml
- examples/board-examples/minimal-board.yaml
- examples/fakecpu_decode_smoke_test.cpp
- examples/fakecpu_exec_smoke_test.cpp
- examples/program_loader_smoke_test.cpp
- examples/simulator_smoke_test.cpp
- examples/test_program.elsim-bin
- examples/timer_smoke_test.cpp
- examples/uart-timer-demo/main.cpp
- examples/uart_smoke_test.cpp
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
- include/elsim/device/TimerDevice.hpp
- include/elsim/device/UartDevice.hpp
- include/elsim/device/VirtualButtonDevice.hpp
- include/elsim/device/VirtualLedDevice.hpp
- src/cli/CliApp.cpp
- src/cli/CliApp.hpp
- src/cli/commands/HelpCommand.cpp
- src/cli/commands/HelpCommand.hpp
- src/cli/commands/ListBoardsCommand.cpp
- src/cli/commands/ListBoardsCommand.hpp
- src/cli/commands/MonitorCommand.cpp
- src/cli/commands/MonitorCommand.hpp
- src/cli/commands/PressCommand.cpp
- src/cli/commands/PressCommand.hpp
- src/cli/commands/RunCommand.cpp
- src/cli/commands/RunCommand.hpp
- src/cli/main.cpp
- src/cli/monitor/MonitorRenderers.cpp
- src/cli/monitor/MonitorRenderers.hpp
- src/cli/monitor/MonitorSnapshot.hpp
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
- tests/test_board_config_parser_gpio_led_button.cpp
- tests/test_fakecpu_core.cpp
- tests/test_gpio_cli_e2e_smoke.cpp
- tests/test_gpio_controller.cpp
- tests/test_gpio_led_integration.cpp
- tests/test_gpio_mmio.cpp
- tests/test_memory_bus.cpp
- tests/test_virtual_button_device.cpp
- tests/test_virtual_led_device.cpp

### Basis for Grouping
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: examples/basic_cpu_memory_test.cpp -> include/elsim/core/FakeCpu.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: examples/basic_cpu_memory_test.cpp -> include/elsim/core/MemoryBus.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: examples/basic_cpu_memory_test.cpp -> include/elsim/core/MemoryBusAdapter.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/FakeCpu.hpp -> examples/fakecpu_decode_smoke_test.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/FakeCpu.hpp -> examples/fakecpu_exec_smoke_test.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/FakeCpu.hpp -> include/elsim/core/ICpu.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/FakeCpu.hpp -> src/core/FakeCpu.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/FakeCpu.hpp -> src/core/Simulator.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/FakeCpu.hpp -> tests/test_fakecpu_core.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/MemoryBus.hpp -> examples/basic_memory_test.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/MemoryBus.hpp -> examples/program_loader_smoke_test.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/MemoryBus.hpp -> include/elsim/core/IMemoryMappedDevice.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/MemoryBus.hpp -> include/elsim/core/ProgramLoader.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/MemoryBus.hpp -> include/elsim/core/Simulator.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/MemoryBus.hpp -> src/core/MemoryBus.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/MemoryBus.hpp -> tests/test_gpio_led_integration.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/MemoryBus.hpp -> tests/test_gpio_mmio.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/MemoryBus.hpp -> tests/test_memory_bus.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/MemoryBusAdapter.hpp -> include/elsim/core/IMemoryBus.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/MemoryBusAdapter.hpp -> src/core/MemoryBusAdapter.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: examples/fakecpu_exec_smoke_test.cpp -> include/elsim/core/Logger.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: src/core/Simulator.cpp -> include/elsim/core/BoardDescription.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: src/core/Simulator.cpp -> include/elsim/core/DeviceMemoryAdapter.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: src/core/Simulator.cpp -> include/elsim/device/VirtualButtonDevice.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: src/core/Simulator.cpp -> include/elsim/device/VirtualLedDevice.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_literals.tsv): CO_EXAMPLE: examples/program_loader_smoke_test.cpp -> examples/test_program.elsim-bin
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/ProgramLoader.hpp -> src/cli/commands/MonitorCommand.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/ProgramLoader.hpp -> src/cli/commands/PressCommand.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/ProgramLoader.hpp -> src/cli/commands/RunCommand.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/ProgramLoader.hpp -> src/core/ProgramLoader.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/Simulator.hpp -> examples/simulator_smoke_test.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/Simulator.hpp -> include/elsim/core/GpioController.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/Simulator.hpp -> include/elsim/device/IDevice.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/Simulator.hpp -> tests/test_gpio_cli_e2e_smoke.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: tests/test_gpio_led_integration.cpp -> include/elsim/device/GpioDevice.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/Logger.hpp -> examples/timer_smoke_test.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/Logger.hpp -> examples/uart-timer-demo/main.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/Logger.hpp -> examples/uart_smoke_test.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/Logger.hpp -> include/elsim/device/TimerDevice.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/Logger.hpp -> src/core/Logger.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/Logger.hpp -> src/device/DeviceFactory.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/Logger.hpp -> src/device/GpioDevice.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/Logger.hpp -> src/device/TimerDevice.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/Logger.hpp -> src/device/UartDevice.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/Logger.hpp -> src/device/VirtualButtonDevice.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/Logger.hpp -> src/device/VirtualLedDevice.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/BoardDescription.hpp -> include/elsim/core/BoardConfigParser.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/BoardDescription.hpp -> include/elsim/device/DeviceFactory.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/DeviceMemoryAdapter.hpp -> src/core/DeviceMemoryAdapter.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/device/VirtualButtonDevice.hpp -> include/elsim/device/BaseDevice.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/device/VirtualButtonDevice.hpp -> tests/test_virtual_button_device.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/device/VirtualLedDevice.hpp -> tests/test_virtual_led_device.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: src/cli/commands/MonitorCommand.cpp -> src/cli/commands/MonitorCommand.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: src/cli/commands/MonitorCommand.cpp -> src/cli/monitor/MonitorRenderers.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: src/cli/commands/MonitorCommand.cpp -> src/cli/monitor/MonitorSnapshot.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: src/cli/commands/PressCommand.cpp -> src/cli/commands/PressCommand.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: src/cli/commands/RunCommand.cpp -> src/cli/commands/RunCommand.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/GpioController.hpp -> src/core/GpioController.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/GpioController.hpp -> tests/test_gpio_controller.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/device/IDevice.hpp -> include/elsim/device/DeviceManager.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_literals.tsv): CO_TEST: tests/test_gpio_cli_e2e_smoke.cpp -> examples/board-examples/gpio-led-button-board.yaml
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: examples/uart-timer-demo/main.cpp -> include/elsim/device/UartDevice.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/BoardConfigParser.hpp -> src/core/BoardConfigParser.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: include/elsim/core/BoardConfigParser.hpp -> tests/test_board_config_parser_gpio_led_button.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: src/cli/commands/MonitorCommand.hpp -> src/cli/CliApp.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: src/cli/monitor/MonitorRenderers.hpp -> src/cli/monitor/MonitorRenderers.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_literals.tsv): CO_TEST: tests/test_board_config_parser_gpio_led_button.cpp -> examples/board-examples/invalid-duplicate-pin.yaml
- EVIDENCE (docs/audit/audit_v0_3_0_edges_literals.tsv): CO_TEST: tests/test_board_config_parser_gpio_led_button.cpp -> examples/board-examples/invalid-led-without-gpio.yaml
- EVIDENCE (docs/audit/audit_v0_3_0_edges_literals.tsv): CO_TEST: tests/test_board_config_parser_gpio_led_button.cpp -> examples/board-examples/invalid-pin-out-of-range.yaml
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: src/cli/CliApp.cpp -> src/cli/CliApp.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: src/cli/CliApp.cpp -> src/cli/commands/HelpCommand.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: src/cli/CliApp.cpp -> src/cli/commands/ListBoardsCommand.hpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: src/cli/CliApp.hpp -> src/cli/main.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: src/cli/commands/HelpCommand.hpp -> src/cli/commands/HelpCommand.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_includes.tsv): DIRECT_INCLUDE: src/cli/commands/ListBoardsCommand.hpp -> src/cli/commands/ListBoardsCommand.cpp
- EVIDENCE (docs/audit/audit_v0_3_0_edges_literals.tsv): DIRECT_USE_LITERAL: src/cli/commands/HelpCommand.cpp -> examples/board-examples/gpio-blinky-board.yaml
- EVIDENCE (docs/audit/audit_v0_3_0_edges_literals.tsv): DIRECT_USE_LITERAL: src/cli/commands/HelpCommand.cpp -> examples/board-examples/minimal-board.yaml

## Standalone Artifacts
### .clang-format
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### .clang-tidy
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### .gitignore
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### CHANGELOG.md
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### LICENSE
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### README.md
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### docs/audit/audit_v0_3_0_filelist.txt
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### docs/audit/audit_v0_3_0_index.md
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### docs/audit/audit_v0_3_0_manifest.sha256
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### docs/audit/audit_v0_3_0_snapshot.txt
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### docs/elsim_binary_format.md
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### docs/fakecpu_isa.md
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### docs/gpio_mmio.md
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### docs/mmio_contract.md
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### examples/board-examples/device-factory-test.yaml
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### examples/board-examples/empty-test.yaml
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### examples/board-examples/gpio-mmio-test.yaml
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### examples/board-examples/hello-board.yaml
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### examples/build_gpio_blinky_program.cpp
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### examples/build_hello_program.cpp
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### examples/gpio_blinky.elsim-bin
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### examples/hello.elsim-bin
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### examples/uart-timer-demo/board.yaml
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### include/elsim/device/IDevicesTickable.hpp
Basis: No recorded relationships (degree 0) under the relationship types listed above.

### tests/CMakeLists.txt
Basis: No recorded relationships (degree 0) under the relationship types listed above.
