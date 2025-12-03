# Embedded Linux Device Simulator

Software simulator of embedded Linux devices with a modular architecture.  
Designed for testing, prototyping, automation, and validating embedded interactions without physical hardware.

---

## Features in v0.1

This is the **initial working prototype** of ELSIM (`v0.1`).  
Current capabilities:

- **Core architecture**
  - `ICpu` interface and CPU abstraction
  - `IMemoryBus` and `MemoryBus` implementation
  - `IMemoryMappedDevice` interface for MMIO devices

- **Memory & MMIO**
  - Unified `MemoryBus` with RAM + MMIO regions
  - Device mapping by base address and size
  - Safe read/write access with logging for out-of-range operations

- **Devices**
  - **UART device**
    - Transmit (TX) register
    - Byte-level logging for transmitted data
  - **Timer device**
    - Counter register
    - Tick handling via `tick()`
    - Basic control register (enable/reset)

- **Device factory**
  - `DeviceFactory` that creates devices from board configuration
  - Automatic MMIO mapping for UART and Timer

- **CLI interface**
  - `elsim` executable with:
    - `--config <path>` – load board configuration from YAML
    - `--dry-run` – validate config and construct simulator without running it
    - `--log-level <DEBUG|INFO|WARN|ERROR>` – control logging verbosity

- **Board configuration (YAML)**
  - YAML-based description of:
    - CPU
    - RAM
    - Devices (UART, Timer)
    - MMIO mappings

- **Logging system**
  - Central logger with levels: `DEBUG`, `INFO`, `WARN`, `ERROR`
  - Detailed messages for device operations and memory accesses

- **Smoke tests & demos**
  - `basic_memory_test` – validates RAM + MMIO reads/writes
  - `basic_cpu_memory_test` – checks CPU ↔ MemoryBus interaction
  - `simulator_smoke_test` – minimal simulator construction
  - `uart_smoke_test` – UART device TX behavior
  - `timer_smoke_test` – Timer ticks and control
  - `uart_timer_demo` – combined UART + Timer example

---

## How to Build

```bash
mkdir build
cd build
cmake ..
make
```
The main executable and examples will be located in:

```bash
build/
```
---

## Quick Start

### **1. Validate a minimal board configuration**

From the `build/` directory:

```bash
./elsim --dry-run --config ../examples/board-examples/minimal-board.yaml
```
You should see log messages indicating that:
- the config is loaded,
- the board is constructed,
- the simulator can be created successfully.

### **2. Run core smoke tests**

Still in `build/`:

```bash
./basic_memory_test
./basic_cpu_memory_test
./simulator_smoke_test
```
These tests validate basic RAM, MMIO, and simulator wiring.

### **3. Run device smoke tests**

```bash
./uart_smoke_test
./timer_smoke_test
```
You should see UART TX logging and Timer counter updates in the console.

### **4. Run UART + Timer demo**

```bash
./uart_timer_demo
```
This demo shows interaction between UART and Timer in a simple scenario (ticks, counter changes, and UART output logged to the console).

---

### **Project Structure**
```bash
Embedded-Linux-Device-Simulator/
├─ src/                      # Source files
│  ├─ core/                  # Simulator core (Simulator, MemoryBus, Logger, etc.)
│  ├─ cli/                   # Command-line interface (elsim entry point)
│  └─ device/                # Device implementations (UART, Timer, DeviceFactory)
│
├─ include/                  # Public headers
│  └─ elsim/
│     ├─ core/               # Core simulator interfaces & types (ICpu, IMemoryBus, IMemoryMappedDevice, Simulator)
│     └─ device/             # Device interfaces and declarations
│
├─ examples/                 # Smoke tests and demo programs
│  ├─ basic_memory_test.cpp
│  ├─ basic_cpu_memory_test.cpp
│  ├─ simulator_smoke_test.cpp
│  ├─ uart_smoke_test.cpp
│  ├─ timer_smoke_test.cpp
│  └─ uart-timer-demo/       # UART + Timer demo source and board config
│
├─ tests/                    # (Future) GoogleTest-based unit tests
│  └─ test_memory_bus.cpp    # Placeholder for MemoryBus tests
│
├─ examples/board-examples/  # YAML board configuration examples
│  └─ minimal-board.yaml
│
├─ CMakeLists.txt            # Main build configuration
├─ README.md                 # Project documentation
└─ CHANGELOG.md              # Release history (from v0.1)
```
### **Code Style**
This project uses clang-format and clang-tidy to maintain consistent and high-quality C++ code style.

### clang-format

Rules are stored in `.clang-format.`

Format a file manually:

```bash
clang-format -i path/to/file.cpp
```
Or run it over the whole project (example):

```bash
clang-format -i $(find include src examples tests -name "*.hpp" -o -name "*.h" -o -name "*.cpp")
```
### clang-tidy
Configured via `.clang-tidy.`

Run checks:

```bash
clang-tidy path/to/file.cpp -- -I./include
```
---
## Roadmap (v0.2 and beyond)

Planned improvements for future versions:

 - Extend device set (GPIO, virtual sensors, storage I/O)
 - Add TCP communication module for remote control / integration
 - Add configuration presets for common embedded boards
 - Improve test coverage with GoogleTest (tests/)
 - Add more detailed examples and documentation
 - Prepare v0.2 release