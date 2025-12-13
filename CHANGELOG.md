# Changelog

All notable changes to this project are documented in this file.

---

## [v0.2] — Fake CPU Execution & Program Loading

### Added
- Fake CPU implementation:
  - CPU state (registers, PC, FLAGS)
  - Instruction fetch / decode / execute cycle
  - Minimal instruction set with control-flow and memory access
- Program loading support:
  - Simple executable format (`elsim-bin`)
  - ProgramLoader that loads code into RAM and sets entry point
- CLI enhancements:
  - `--program <path>` option to load and execute binaries
- Demo programs and boards:
  - Minimal "hello" program for Fake CPU
  - Corresponding board configuration examples
- Additional smoke and execution tests for CPU and loader

### Changed
- Simulator startup flow to support program loading before execution
- Logging extended for CPU instruction execution and loader activity

---

## [v0.1] — Initial Prototype

### Added
- Core architecture:
  - `ICpu` interface
  - `IMemoryBus` and `MemoryBus` implementation
  - `IMemoryMappedDevice` interface for MMIO devices
- CLI interface:
  - `elsim` executable with `--config`, `--dry-run`, and `--log-level`
- YAML-based board configuration format
- UART device:
  - TX register
  - Logging of transmitted bytes
- Timer device:
  - Counter register
  - Tick handling and basic control
- DeviceFactory:
  - Creation of UART and Timer devices from board configuration
  - Automatic MMIO mapping for devices
- Logging system:
  - Central logger with levels: DEBUG / INFO / WARN / ERROR
  - Detailed messages for memory and device operations
- Memory & MMIO support:
  - Unified MemoryBus for RAM + MMIO regions
  - Safe read/write with diagnostics for out-of-range accesses
- Smoke tests and demos:
  - `basic_memory_test`
  - `basic_cpu_memory_test`
  - `simulator_smoke_test`
  - `uart_smoke_test`
  - `timer_smoke_test`
  - `uart_timer_demo`

