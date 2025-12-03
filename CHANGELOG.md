
# Changelog

All notable changes to this project will be documented in this file.

## [v0.1] — Initial Prototype (ELSIM)

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