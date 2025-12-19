# MMIO Contract (ELSIM)

This document defines the minimal, stable rules for Memory-Mapped I/O (MMIO) access in ELSIM.
The goal is to make all devices (UART / Timer / future GPIO/LED/Button) behave consistently and predictably.

## Scope

This contract describes:
- how the MemoryBus routes accesses to MMIO devices
- what an MMIO "transaction" means in the current architecture
- how devices must behave on invalid or disallowed operations

## Addressing model

- Each device is mapped into a global address range: [base, base + size).
- MemoryBus converts a global address into a device-local offset:

  offset = address - base

- If an address falls into a mapped device range, MemoryBus routes the access to the device.

## Access sizes

- The current bus-level MMIO access size is **8-bit only**:
  - `read8(address) -> uint8_t`
  - `write8(address, value)`

- There are no `read16/read32` bus operations in v0.3.
- If the CPU or higher-level code needs 16/32-bit register access, it must be implemented as multiple `read8/write8` operations.

## Alignment

- Alignment rules are **not applicable** at the bus level because accesses are 8-bit only.

## Endianness

- Since the bus provides only `read8/write8`, endianness is not defined by the bus.
- For multi-byte registers, **devices must define how bytes are laid out**.
- The current convention used by existing devices is **little-endian byte layout** for multi-byte values (example: Timer COUNTER is split into 4 bytes). 

## Unknown offsets and invalid operations

MemoryBus does NOT validate device register offsets. It simply routes the access to the mapped device. 

Therefore, each MMIO device MUST:
- define which offsets are supported
- define access permissions per register (RO/RW/WO) if applicable
- handle invalid or disallowed operations in a consistent way

### Recommended behavior (current project style)

On invalid/disallowed operations, devices should:
- log a warning (WARN) when practical
- NOT crash the simulation (no exceptions for MMIO register mistakes)
- for invalid reads: return a deterministic default value (device-defined, commonly 0x00 or 0xFF)
- for invalid writes: ignore the write (optionally log WARN)

This matches the current behavior of existing devices:
- Timer invalid offset -> WARN and return 0xFF / ignore write.
- UART unsupported read -> WARN and return 0x00; unsupported write is ignored to avoid log spam. 

## RAM out-of-range vs MMIO invalid offset

- Access outside RAM range when no device is mapped is a hard error (throws out_of_range). 
- Access to a mapped device with an invalid offset is handled by the device using the rules above (warn + default/ignore).

## Minimal negative tests

The repository must contain minimal tests that verify at least:
- writing to a read-only register is ignored (or reported as an error by contract)
- access to an unknown offset returns deterministic default and does not crash
