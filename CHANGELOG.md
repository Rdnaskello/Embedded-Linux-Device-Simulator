# Changelog

All notable changes to this project are documented in this file.

## [v0.3.0] — GPIO Subsystem + CLI Workflow (monitor/press)

### Added
- GPIO subsystem:
  - GpioController core model (direction mask, input mask injection, output latch)
  - GPIO MMIO device with 8-bit transactions and 32-bit register map (DIR, DATA_IN, DATA_OUT, SET/CLR/TOGGLE)
  - Defined readback policy for DATA_IN based on pin direction
- Virtual devices:
  - Virtual LED device subscribing to GPIO output state (active_high/active_low)
  - Virtual Button device injecting GPIO input levels (momentary)
- Board configuration:
  - Extended board.yaml schema: `gpio`, `leds`, `buttons`
  - Example boards for GPIO demos and validation
- CLI:
  - Subcommands: `run`, `list-boards`, `monitor`, `press`, `help`
  - `monitor` supports text + JSON output (pretty JSON in one-shot mode, NDJSON in streaming mode)
  - `press` allows injecting a button press (hold/repeat) during simulation

### Changed
- CLI routing now supports subcommand style while keeping backward-compat:
  - `elsim --config ...` behaves like `elsim run --config ...`

### Tests
- Added unit and integration tests for:
  - GPIO controller behavior
  - GPIO MMIO register semantics and invalid offsets
  - Virtual LED / Virtual Button devices
  - BoardConfigParser rules for gpio/leds/buttons
  - E2E smoke test validating `press` impact and stable monitor JSON contract

### Documentation
- Added/updated docs:
  - `docs/gpio_mmio.md`
  - `docs/mmio_contract.md`

### Known limitations
- No button debounce model.
- No interrupts / edge-triggered GPIO events (polling only).
- MMIO transactions are 8-bit only; 32-bit registers require byte-wise access.

---

## [v0.2.0] — Fake CPU Execution & Program Loading

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

## [v0.1.0] — Initial Prototype

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

