# Embedded Linux Device Simulator

Software simulator of embedded Linux devices with a modular architecture.  
Designed for testing, prototyping, automation, and validating embedded interactions without physical hardware.

---

## How to Build

```bash
mkdir build
cd build
cmake ..
make
```

Executable will be located in:

```
build/
```

---

## Project Structure

```
Embedded-Linux-Device-Simulator/
├─ src/            # Source files of the simulator
├─ include/        # Header files
├─ build/          # Build artifacts 
├─ CMakeLists.txt  # Main build configuration
└─ README.md       # Project documentation
```

---

## Code Style

This project uses **clang-format** and **clang-tidy** to maintain consistent and high-quality C++ code style.

###  clang-format

Rules are stored in `.clang-format`.

Format a file manually:

```bash
clang-format -i path/to/file.cpp
```

###  clang-tidy

Configured via \`.clang-tidy\`.

Run checks:

```bash
clang-tidy path/to/file.cpp -- -I./include
```

---

## Roadmap (v0.x)

- [ ] Add basic device abstraction layer  
- [ ] Implement TCP communication module  
- [ ] Add virtual sensors (temperature, GPIO, storage I/O)  
- [ ] Logging subsystem  
- [ ] Extend test coverage  
- [ ] Prepare v0.2 release  
