# Component B — As-Is State (ELSIM v0.3.0)

> **Component ID:** B  
> **Component name:** MemoryBus basic smoke path (RAM + MMIO routing)  
> **Audit type:** As-Is (no judgments, only verifiable facts with file:line evidence)  
> **Version scope:** ELSIM v0.3.0 (repo snapshot)  
> **Snapshot:** cb4b80b (per docs/audit_v0_3_0_components_by_target.md)  
> **Date:** 2026-01-06  
> **Branch:** feature/TASK-AUDIT-1.2-component-b-as-is

---

## Evidence & Scope (why these artifacts)
**Primary component grouping evidence (by build target):**
- `docs/audit_v0_3_0_components_by_target.md`: Component-B includes `examples/basic_memory_test.cpp` with basis:
  - `docs/audit/audit_v0_3_0_edges_targets.tsv`: `TARGET_HAS_SOURCE: cmake-target:basic_memory_test -> examples/basic_memory_test.cpp` (map excerpt provided by user)

**Artifacts examined in this As-Is (direct and supporting):**
- Executable example:
  - `examples/basic_memory_test.cpp` (main smoke test for RAM + MMIO path)
- Core bus API & implementation:
  - `include/elsim/core/MemoryBus.hpp`
  - `src/core/MemoryBus.cpp`
- MMIO device interface:
  - `include/elsim/core/IMemoryMappedDevice.hpp`
- Logging used by MemoryBus:
  - `include/elsim/core/Logger.hpp`
  - `src/core/Logger.cpp`
- Negative tests for MMIO contract expectations (bus + fake device):
  - `tests/test_memory_bus.cpp`
- Documentation contract relevant to MMIO routing:
  - `docs/mmio_contract.md`
- Build target confirmation:
  - `CMakeLists.txt`

---

## 1. Responsibility (Factual)
- The `basic_memory_test` executable performs a minimal runtime check that:
  - `MemoryBus` can store and load a byte in RAM via `write8/read8`.  
    Evidence: `examples/basic_memory_test.cpp:37–41`  
  - `MemoryBus` can route accesses to an MMIO device mapped via `mapDevice`, and that MMIO reads/writes are delegated to the device.  
    Evidence: `examples/basic_memory_test.cpp:43–53`, `src/core/MemoryBus.cpp:37–48`, `src/core/MemoryBus.cpp:74–85`
- `MemoryBus` provides a unified 8-bit access API over:
  - a RAM byte buffer, and
  - a set of memory-mapped devices (MMIO) mapped to address ranges.  
  Evidence: `include/elsim/core/MemoryBus.hpp:9–19`, `include/elsim/core/MemoryBus.hpp:22–56`

---

## 2. Public API / External Contract
### Public API (MemoryBus)
- Constructor:
  - `explicit MemoryBus(std::size_t size)` creates RAM of the requested byte size.  
    Evidence: `include/elsim/core/MemoryBus.hpp:24–25`, `src/core/MemoryBus.cpp:18`
- Read/write (8-bit only at bus level):
  - `std::uint8_t read8(std::uint32_t address) const`  
    Evidence: `include/elsim/core/MemoryBus.hpp:27–28`, `src/core/MemoryBus.cpp:34–68`
  - `void write8(std::uint32_t address, std::uint8_t value)`  
    Evidence: `include/elsim/core/MemoryBus.hpp:30–31`, `src/core/MemoryBus.cpp:71–104`
- MMIO mapping:
  - `void mapDevice(std::uint32_t baseAddress, std::uint32_t size, std::shared_ptr<IMemoryMappedDevice> device)`  
    Evidence: `include/elsim/core/MemoryBus.hpp:33–38`, `src/core/MemoryBus.cpp:107–143`

### External MMIO contract (device interface)
- MMIO devices must implement:
  - `read8(offset) -> uint8_t`
  - `write8(offset, value)`  
  Evidence: `include/elsim/core/IMemoryMappedDevice.hpp:14–21`

### Inputs / outputs and externally observable effects
- When an address belongs to a mapped device region, bus converts global address to device-local `offset = address - base` and calls `device->read8(offset)` / `device->write8(offset, value)`.  
  Evidence: `src/core/MemoryBus.cpp:38–41`, `src/core/MemoryBus.cpp:75–84`
- When an address is not mapped to a device, bus accesses RAM buffer `m_memory[address]`.  
  Evidence: `src/core/MemoryBus.cpp:50–67`, `src/core/MemoryBus.cpp:87–104`
- For out-of-range RAM access (when no device mapped), bus throws `std::out_of_range`.  
  Evidence: `src/core/MemoryBus.cpp:51–59`, `src/core/MemoryBus.cpp:88–96`
- Logging side effects:
  - MemoryBus emits debug logs on RAM/MMIO reads and writes and error logs on out-of-range, using `Logger::instance()`.  
    Evidence: `src/core/MemoryBus.cpp:35–47`, `src/core/MemoryBus.cpp:52–58`, `src/core/MemoryBus.cpp:63–66`, `src/core/MemoryBus.cpp:72–85`, `src/core/MemoryBus.cpp:90–96`, `src/core/MemoryBus.cpp:98–101`

### Build target / entry point
- The target `basic_memory_test` builds `examples/basic_memory_test.cpp` and links it with `elsim_core`.  
  Evidence: `CMakeLists.txt:50–58`

---

## 3. Internal State Model
### MemoryBus internal state
- RAM storage:
  - `std::vector<std::uint8_t> m_memory;` stores RAM bytes.  
    Evidence: `include/elsim/core/MemoryBus.hpp:48–50`
- MMIO mapping table:
  - `std::vector<MappedDevice> m_devices;` contains device regions.  
    Evidence: `include/elsim/core/MemoryBus.hpp:51–53`
- Each mapped device entry stores:
  - `base` (global base), `size` (region size), `device` (shared_ptr).  
    Evidence: `include/elsim/core/MemoryBus.hpp:41–46`
- Device lookup is performed by `findDevice(address)` and uses a linear scan across `m_devices`.  
  Evidence: `include/elsim/core/MemoryBus.hpp:54–56`, `src/core/MemoryBus.cpp:21–31`

### basic_memory_test internal state (local)
- Uses a local `TestDevice` implementing `IMemoryMappedDevice` backed by `std::array<uint8_t, 16> storage_{}` (zero-initialized).  
  Evidence: `examples/basic_memory_test.cpp:12–30`

---

## 4. Runtime Behavior
(Format: **Trigger → Preconditions → Steps → Observable effects → Post-state**)

### Scenario 1: RAM byte read/write
- **Trigger:** `bus.write8(0x10, 0x42)` followed by `bus.read8(0x10)` in `basic_memory_test`.  
  Evidence: `examples/basic_memory_test.cpp:37–40`
- **Preconditions:**
  - `MemoryBus bus(64 * 1024)` created successfully (RAM allocated).  
    Evidence: `examples/basic_memory_test.cpp:34–35`, `src/core/MemoryBus.cpp:18`
  - Address `0x10` is within `m_memory.size()`.  
    Evidence: `src/core/MemoryBus.cpp:88–96` (out-of-range condition defined)
- **Steps (bus behavior):**
  1. `write8` calls `findDevice(address)`; if no device covers address, writes to RAM.  
     Evidence: `src/core/MemoryBus.cpp:74–86`, `src/core/MemoryBus.cpp:87–104`
  2. `read8` calls `findDevice(address)`; if no device covers address, returns `m_memory[address]`.  
     Evidence: `src/core/MemoryBus.cpp:37–49`, `src/core/MemoryBus.cpp:50–67`
- **Observable effects:**
  - `basic_memory_test` prints `RAM test value: 0x42` (hex formatting applied).  
    Evidence: `examples/basic_memory_test.cpp:41`
  - MemoryBus emits debug logs for RAM operations when logger level allows it.  
    Evidence: `src/core/MemoryBus.cpp:63–66`, `src/core/Logger.cpp:44–59` (level filtering)
- **Post-state:**
  - `m_memory[0x10] == 0x42` after write.  
    Evidence: `src/core/MemoryBus.cpp:103`

### Scenario 2: MMIO routing to a mapped device
- **Trigger:** `bus.mapDevice(mmioBase, mmioSize, device)`, then `bus.write8(mmioBase + 4, 0xAB)` and `bus.read8(mmioBase + 4)`.  
  Evidence: `examples/basic_memory_test.cpp:43–52`
- **Preconditions:**
  - `device != nullptr` for `mapDevice`.  
    Evidence: `src/core/MemoryBus.cpp:110–113`
  - New mapping range does not overlap any existing mapped device region.  
    Evidence: `src/core/MemoryBus.cpp:118–135`
  - Address `mmioBase + 4` falls into `[base, base + size)`.  
    Evidence: `src/core/MemoryBus.cpp:25–29`, `src/core/MemoryBus.cpp:38–41`
- **Steps (bus behavior):**
  1. `mapDevice` validates `device` pointer; checks overlap; pushes `MappedDevice{baseAddress, size, device}`.  
     Evidence: `src/core/MemoryBus.cpp:110–114`, `src/core/MemoryBus.cpp:118–135`, `src/core/MemoryBus.cpp:141–142`
  2. For writes/reads within mapped region:
     - `findDevice(address)` returns the matching mapping.  
       Evidence: `src/core/MemoryBus.cpp:21–31`
     - bus computes `offset = address - mapped->base` and delegates to `device->write8(offset, value)` / `device->read8(offset)`.  
       Evidence: `src/core/MemoryBus.cpp:38–41`, `src/core/MemoryBus.cpp:75–84`
- **Observable effects:**
  - `basic_memory_test` prints `MMIO test value: 0xab`.  
    Evidence: `examples/basic_memory_test.cpp:53`
  - MemoryBus emits debug logs for MMIO reads/writes when logger level allows it.  
    Evidence: `src/core/MemoryBus.cpp:42–46`, `src/core/MemoryBus.cpp:78–82`, `src/core/Logger.cpp:44–59`
- **Post-state:**
  - `TestDevice::storage_[4] == 0xAB` after the MMIO write, and subsequent MMIO read returns `0xAB`.  
    Evidence: `examples/basic_memory_test.cpp:21–26`, `examples/basic_memory_test.cpp:50–52`

---

## 5. Error and Edge Behavior
### MemoryBus edge/error behavior
- **RAM out-of-range (no MMIO mapped to the address):**
  - `read8` throws `std::out_of_range("MemoryBus::read8: address out of range")`.  
    Evidence: `src/core/MemoryBus.cpp:51–59`
  - `write8` throws `std::out_of_range("MemoryBus::write8: address out of range")`.  
    Evidence: `src/core/MemoryBus.cpp:88–96`
- **mapDevice called with null device pointer:**
  - throws `std::invalid_argument("MemoryBus::mapDevice: device is null")`.  
    Evidence: `src/core/MemoryBus.cpp:110–113`
- **Overlapping device region on mapDevice:**
  - throws `std::runtime_error("MemoryBus::mapDevice: overlapping device region")`.  
    Evidence: `src/core/MemoryBus.cpp:123–134`
- **Unknown offsets inside a mapped MMIO region:**
  - MemoryBus does not validate register offsets; it always routes to the device.  
    Evidence: `docs/mmio_contract.md:41–48` (explicit statement), `src/core/MemoryBus.cpp:38–41`
  - Result of unknown offsets is therefore device-defined (not validated by bus).  
    Evidence: `docs/mmio_contract.md:43–48`

### basic_memory_test error behavior
- Entire main is wrapped in `try/catch (const std::exception&)`, printing exception message and returning `1` on exception.  
  Evidence: `examples/basic_memory_test.cpp:32–35`, `examples/basic_memory_test.cpp:62–65`
- If expected values do not match (`ramValue != 0x42` or `mmioValue != 0xAB`), the program prints failure and returns `1`.  
  Evidence: `examples/basic_memory_test.cpp:55–61`

---

## 6. Test Coverage (As-Is)
### Tests that explicitly target this area
- `tests/test_memory_bus.cpp` defines `FakeMmioDevice` and tests routing + MMIO semantics through `MemoryBus`.  
  Evidence: `tests/test_memory_bus.cpp:11–50`, `tests/test_memory_bus.cpp:54–92`

### What these tests verify (factual)
- Writing to a read-only MMIO register (offset `0x00` in fake device) is ignored and does not crash the test.  
  Evidence: `tests/test_memory_bus.cpp:54–75` (write happens at `:67`, value remains at `:70–71`)
- Reading from an unknown MMIO offset returns a deterministic default (`0xFF` in fake device) and does not crash; unknown writes are ignored (device tracks attempts).  
  Evidence: `tests/test_memory_bus.cpp:77–92`, `tests/test_memory_bus.cpp:18–26`, `tests/test_memory_bus.cpp:28–40`

### Aspects with no tests found in scope (factual)
- No tests in provided scope assert `MemoryBus` throws `std::out_of_range` on RAM out-of-range reads/writes.  
  Evidence: behavior defined in `src/core/MemoryBus.cpp:51–59` and `:88–96`; not referenced in `tests/test_memory_bus.cpp`
- No tests in provided scope assert `mapDevice` overlap detection throws `std::runtime_error`.  
  Evidence: `src/core/MemoryBus.cpp:118–135`; no overlap test in `tests/test_memory_bus.cpp`

---

## 7. Documentation Alignment
### Documents that describe relevant behavior
- `docs/mmio_contract.md` defines routing rules and negative expectations:
  - bus routes MMIO by range, converts address to offset, and does not validate offsets.  
    Evidence: `docs/mmio_contract.md:13–20`, `docs/mmio_contract.md:41–48`
  - bus-level access is 8-bit only (`read8/write8`).  
    Evidence: `docs/mmio_contract.md:22–29`
  - “RAM out-of-range vs MMIO invalid offset” distinction: out-of-range RAM is a hard error; invalid MMIO offset is device-handled.  
    Evidence: `docs/mmio_contract.md:62–66`

### What is not documented (factual)
- No documentation was provided in scope that explains the existence/purpose of the specific example program `basic_memory_test` (its role, how/when to run it, expected output).  
  Evidence: `CMakeLists.txt:50–58` defines the target; `docs/mmio_contract.md` describes MMIO rules but does not reference `basic_memory_test`.

### Document vs implementation consistency (factual)
- The MMIO contract statement “MemoryBus does NOT validate device register offsets” matches the implementation (bus directly delegates `read8/write8` to device with computed offset).  
  Evidence: `docs/mmio_contract.md:41–48`, `src/core/MemoryBus.cpp:38–41`, `src/core/MemoryBus.cpp:75–84`
- The contract statement “Access outside RAM range when no device is mapped is a hard error (throws out_of_range)” matches the implementation.  
  Evidence: `docs/mmio_contract.md:62–66`, `src/core/MemoryBus.cpp:51–59`, `src/core/MemoryBus.cpp:88–96`

(No contradictions found in the provided scope.)

---

## 8. Real Usage / Observed Usage
- The example `basic_memory_test` uses:
  - RAM access at address `0x10`.  
    Evidence: `examples/basic_memory_test.cpp:37–41`
  - MMIO mapping at base `0x1000` of size `16` bytes and accesses offset `4`.  
    Evidence: `examples/basic_memory_test.cpp:45–52`
- Unit tests use `MemoryBus` to map a fake MMIO device at bases `0x1000` and `0x2000` and verify expected device-defined behavior for specific offsets.  
  Evidence: `tests/test_memory_bus.cpp:54–61`, `tests/test_memory_bus.cpp:77–84`

---

## 9. Dependencies & Assumptions (Factual)
### Dependencies
- `MemoryBus` depends on the MMIO interface `IMemoryMappedDevice` via `std::shared_ptr<IMemoryMappedDevice>`.  
  Evidence: `include/elsim/core/MemoryBus.hpp:7–8`, `include/elsim/core/MemoryBus.hpp:33–38`
- `MemoryBus` uses `elsim::core::Logger::instance()` and emits log lines via `debug()` / `error()`.  
  Evidence: `src/core/MemoryBus.cpp:35–36`, `src/core/MemoryBus.cpp:45–46`, `src/core/MemoryBus.cpp:56–57`, `src/core/MemoryBus.cpp:101–102`
- Logging implementation writes to `std::clog` and applies log-level filtering.  
  Evidence: `src/core/Logger.cpp:44–59`

### Assumptions / invariants visible in implementation
- Device mapping ranges are treated as half-open intervals: `[base, base + size)`.  
  Evidence: `include/elsim/core/MemoryBus.hpp:33–34`, `src/core/MemoryBus.cpp:25–29`, `src/core/MemoryBus.cpp:115–116`
- `findDevice` is linear and returns the first matching mapping in `m_devices` iteration order.  
  Evidence: `src/core/MemoryBus.cpp:21–31`
- `mapDevice` rejects overlaps using the condition:
  - `overlap = !(newEnd <= dev.base || baseAddress >= end)`  
  Evidence: `src/core/MemoryBus.cpp:123–125`
- `basic_memory_test` assumes successful execution equals:
  - RAM readback equals `0x42`, and MMIO readback equals `0xAB`.  
  Evidence: `examples/basic_memory_test.cpp:55–61`
