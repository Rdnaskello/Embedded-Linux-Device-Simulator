# Embedded Linux Device Simulator

Software simulator of embedded-style boards with a modular architecture (CPU + RAM + MMIO devices).
Designed for testing, prototyping, automation, and validating device interactions without physical hardware.


---

## Features in v0.2

ELSIM `v0.2` extends the initial simulator prototype with a minimal CPU execution model and a program loader.

- **Core architecture**
  - `Simulator` orchestration (CPU + memory bus + devices)
  - `ICpu` abstraction for pluggable CPU implementations
  - `IMemoryBus` / `MemoryBus` with RAM and MMIO regions
  - `IMemoryMappedDevice` interface for MMIO devices

- **Memory & MMIO**
  - Unified `MemoryBus` with RAM + MMIO regions
  - Device mapping by base address and size
  - Safe read/write access with logging for out-of-range operations

- **Fake CPU**
  - Minimal instruction set and CPU state (registers, PC, FLAGS)
  - Instruction fetch/decode/execute loop
  - Debug-level logging of executed instructions

- **Program loading**
  - Simple executable format: `elsim-bin` (header + code section)
  - `ProgramLoader` that loads code into RAM and sets CPU entry point

- **Devices**
  - **UART device**
    - Transmit (TX) register
    - Byte-level logging for transmitted data
  - **Timer device**
    - Counter register
    - Tick handling via `tick()`
    - Basic control register (enable/reset)

- **Device factory**
  - `DeviceFactory` creates devices from board configuration
  - Automatic MMIO mapping for UART and Timer

- **CLI**
  - `elsim` executable with:
    - `--config <path>` – load board configuration from YAML
    - `--program <path>` – load `elsim-bin` program into RAM and start CPU execution
    - `--dry-run` – validate config and construct simulator without running it
    - `--log-level <DEBUG|INFO|WARN|ERROR>` – control logging verbosity

- **Examples & tests**
  - Smoke tests for MemoryBus, devices, simulator wiring
  - Demo board configs in `examples/board-examples/`
  - Minimal hello-style demo program for Fake CPU

---

## How to Build

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```
Binaries will be located in `build/`.

---

## Quick Start
From the `build/` directory:  
### **1. Validate a minimal board configuration**

```bash
./elsim --dry-run --config ../examples/board-examples/minimal-board.yaml
```
### **2. Run core smoke tests**

Still in `build/`:

```bash
./basic_memory_test
./basic_cpu_memory_test
./simulator_smoke_test
```
### **3. Run device smoke tests**

```bash
./uart_smoke_test
./timer_smoke_test
```
### **4. Run Fake CPU demo program**
```bash
./elsim --config ../examples/board-examples/hello-board.yaml --program ../examples/hello.elsim-bin --log-level INFO
```

---

### **Project Structure**
```bash
Embedded-Linux-Device-Simulator/
├─ src/
│  ├─ core/                  # Simulator core (Simulator, MemoryBus, Logger, etc.)
│  ├─ cli/                   # Command-line interface (elsim entry point)
│  ├─ cpu/                   # CPU implementations (FakeCpu, etc.)
│  └─ device/                # Device implementations (UART, Timer, DeviceFactory)
│
├─ include/
│  └─ elsim/
│     ├─ core/
│     ├─ cpu/
│     └─ device/
│
├─ examples/
│  ├─ board-examples/        # YAML board configuration examples
│  ├─ build_hello_program.cpp
│  └─ hello.elsim-bin
│
├─ CMakeLists.txt
├─ README.md
└─ CHANGELOG.md
```
### **Code Style**
This project uses clang-format and clang-tidy.

### clang-format

Rules are stored in `.clang-format.`

```bash
clang-format -i path/to/file.cpp
```
Example: format the whole project:

```bash
clang-format -i $(find include src examples tests -name "*.hpp" -o -name "*.h" -o -name "*.cpp")
```
### clang-tidy
Configured via `.clang-tidy.`


```bash
clang-tidy path/to/file.cpp -- -I./include
```
---
## Roadmap (v0.3+)

Planned improvements for future versions:

- Extend the device set (GPIO, sensors, storage I/O)
- Improve test coverage and CI automation
- Add more detailed examples and documentation
