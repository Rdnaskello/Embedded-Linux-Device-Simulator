# ELSIM v0.3.0 — Audit Index (Source-of-Truth Map)

## repo-meta
### Source code
- `.clang-format`
- `.clang-tidy`
- `.gitignore`
- `CMakeLists.txt`

### Tests

### Documentation
- `CHANGELOG.md`
- `LICENSE`
- `README.md`
- `docs/audit/as-is_template_v1.md` (As-Is Template v1.0, TASK-AUDIT-0.2)
- `docs/audit/audit_v0_3_0_filelist.txt`
- `docs/audit/audit_v0_3_0_snapshot.txt`
- `docs/audit/audit_v0_3_0_index.md`
- `docs/audit/audit_v0_3_0_manifest.sha256`

### Examples / Demos

### CLI usage


## core/fakecpu-isa
### Source code
- `include/elsim/core/FakeCpu.hpp`
- `include/elsim/core/ICpu.hpp`
- `src/core/FakeCpu.cpp`

### Tests
- `tests/test_fakecpu_core.cpp`

### Documentation
- `docs/fakecpu_isa.md`

### Examples / Demos
- `examples/fakecpu_decode_smoke_test.cpp`
- `examples/fakecpu_exec_smoke_test.cpp`
- `examples/basic_cpu_memory_test.cpp`

### CLI usage


## core/memory-bus-mmio
### Source code
- `include/elsim/core/IMemoryBus.hpp`
- `include/elsim/core/IMemoryMappedDevice.hpp`
- `include/elsim/core/MemoryBus.hpp`
- `include/elsim/core/MemoryBusAdapter.hpp`
- `src/core/MemoryBus.cpp`
- `src/core/MemoryBusAdapter.cpp`

### Tests
- `tests/test_memory_bus.cpp`

### Documentation
- `docs/mmio_contract.md`

### Examples / Demos
- `examples/basic_memory_test.cpp`

### CLI usage


## core/board-config
### Source code
- `include/elsim/core/BoardConfigParser.hpp`
- `include/elsim/core/BoardDescription.hpp`
- `src/core/BoardConfigParser.cpp`

### Tests
- `tests/test_board_config_parser_gpio_led_button.cpp`

### Documentation

### Examples / Demos
- `examples/board-examples/device-factory-test.yaml`
- `examples/board-examples/empty-test.yaml`
- `examples/board-examples/gpio-blinky-board.yaml`
- `examples/board-examples/gpio-led-button-board.yaml`
- `examples/board-examples/gpio-mmio-test.yaml`
- `examples/board-examples/hello-board.yaml`
- `examples/board-examples/invalid-duplicate-pin.yaml`
- `examples/board-examples/invalid-led-without-gpio.yaml`
- `examples/board-examples/invalid-pin-out-of-range.yaml`
- `examples/board-examples/minimal-board.yaml`

### CLI usage


## core/simulator
### Source code
- `include/elsim/core/Simulator.hpp`
- `src/core/Simulator.cpp`

### Tests

### Documentation

### Examples / Demos
- `examples/simulator_smoke_test.cpp`

### CLI usage


## core/logging-adapters
### Source code
- `include/elsim/core/DeviceMemoryAdapter.hpp`
- `include/elsim/core/Logger.hpp`
- `src/core/DeviceMemoryAdapter.cpp`
- `src/core/Logger.cpp`

### Tests

### Documentation

### Examples / Demos

### CLI usage


## loader/binary-format
### Source code
- `include/elsim/Version.hpp.in`
- `include/elsim/core/ProgramLoader.hpp`
- `src/core/ProgramLoader.cpp`

### Tests

### Documentation
- `docs/elsim_binary_format.md`

### Examples / Demos
- `examples/build_gpio_blinky_program.cpp`
- `examples/build_hello_program.cpp`
- `examples/program_loader_smoke_test.cpp`
- `examples/gpio_blinky.elsim-bin`
- `examples/hello.elsim-bin`
- `examples/test_program.elsim-bin`

### CLI usage


## devices/framework
### Source code
- `include/elsim/device/BaseDevice.hpp`
- `include/elsim/device/DeviceFactory.hpp`
- `include/elsim/device/DeviceManager.hpp`
- `include/elsim/device/IDevice.hpp`
- `include/elsim/device/IDevicesTickable.hpp`
- `src/device/DeviceFactory.cpp`

### Tests

### Documentation

### Examples / Demos

### CLI usage


## devices/gpio
### Source code
- `include/elsim/core/GpioController.hpp`
- `include/elsim/device/GpioDevice.hpp`
- `src/core/GpioController.cpp`
- `src/device/GpioDevice.cpp`

### Tests
- `tests/test_gpio_controller.cpp`
- `tests/test_gpio_mmio.cpp`
- `tests/test_gpio_led_integration.cpp`

### Documentation
- `docs/gpio_mmio.md`

### Examples / Demos

### CLI usage


## devices/virtual-led
### Source code
- `include/elsim/device/VirtualLedDevice.hpp`
- `src/device/VirtualLedDevice.cpp`

### Tests
- `tests/test_virtual_led_device.cpp`

### Documentation

### Examples / Demos

### CLI usage


## devices/virtual-button
### Source code
- `include/elsim/device/VirtualButtonDevice.hpp`
- `src/device/VirtualButtonDevice.cpp`

### Tests
- `tests/test_virtual_button_device.cpp`

### Documentation

### Examples / Demos

### CLI usage


## devices/uart
### Source code
- `include/elsim/device/UartDevice.hpp`
- `src/device/UartDevice.cpp`

### Tests

### Documentation

### Examples / Demos
- `examples/uart_smoke_test.cpp`
- `examples/uart-timer-demo/board.yaml`
- `examples/uart-timer-demo/main.cpp`

### CLI usage


## devices/timer
### Source code
- `include/elsim/device/TimerDevice.hpp`
- `src/device/TimerDevice.cpp`

### Tests

### Documentation

### Examples / Demos
- `examples/timer_smoke_test.cpp`

### CLI usage


## cli
### Source code
- `src/cli/CliApp.cpp`
- `src/cli/CliApp.hpp`
- `src/cli/main.cpp`
- `src/cli/commands/HelpCommand.cpp`
- `src/cli/commands/HelpCommand.hpp`
- `src/cli/commands/ListBoardsCommand.cpp`
- `src/cli/commands/ListBoardsCommand.hpp`
- `src/cli/commands/MonitorCommand.cpp`
- `src/cli/commands/MonitorCommand.hpp`
- `src/cli/commands/PressCommand.cpp`
- `src/cli/commands/PressCommand.hpp`
- `src/cli/commands/RunCommand.cpp`
- `src/cli/commands/RunCommand.hpp`
- `src/cli/monitor/MonitorRenderers.cpp`
- `src/cli/monitor/MonitorRenderers.hpp`
- `src/cli/monitor/MonitorSnapshot.hpp`

### Tests
- `tests/test_gpio_cli_e2e_smoke.cpp`

### Documentation

### Examples / Demos

### CLI usage
- elsim help [flags]
- elsim list-boards [flags]
- elsim run [flags]
- elsim monitor [flags]
- elsim press [flags]


## tests
### Source code

### Tests
- `tests/CMakeLists.txt`

### Documentation

### Examples / Demos

### CLI usage
