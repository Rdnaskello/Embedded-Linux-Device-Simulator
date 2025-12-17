# GPIO MMIO Subsystem (ELSIM v0.3)

This document describes the GPIO subsystem in ELSIM, including its MMIO register map,
internal model, board.yaml configuration, and usage examples.

The goal is to provide a stable, precise contract for GPIO behavior, aligned with the
actual implementation in ELSIM v0.3.

---

## A) Overview

The GPIO subsystem in ELSIM provides a simple, deterministic model for general-purpose
input/output pins exposed to the simulated CPU via memory-mapped I/O (MMIO).

From the CPU perspective, GPIO appears as a standard MMIO device mapped into the
MemoryBus address space. All accesses are performed using 8-bit MMIO transactions,
with 32-bit GPIO registers exposed as four consecutive byte-wide locations in
little-endian layout.

Internally, the GPIO subsystem is split into two clearly separated parts:

- **GpioController** represents the logical model of GPIO pins. It stores the direction
  (input/output), output latch state, and injected input levels. The controller computes
  the effective output level as `DIR & OUT` and notifies subscribers when the externally
  observable output state changes.

- **GpioDevice** is the MMIO-facing device that connects the controller to the MemoryBus.
  It decodes MMIO register offsets, enforces register access rules (RO/RW/WO), applies
  write semantics (including SET/CLR/TOGGLE behavior), and implements the defined
  readback policy for input registers.

Virtual devices are layered on top of the GPIO controller:

- **VirtualLedDevice** subscribes to output changes of a specific GPIO pin and logs
  state transitions (`ON` / `OFF`) based on the configured polarity (`active_high`).

- **VirtualButtonDevice** injects input levels into a GPIO pin to simulate a physical
  button. In v0.3, only momentary buttons are supported, and no debounce or interrupt
  logic is implemented.

This separation ensures that the GPIO MMIO contract remains stable and predictable,
while higher-level virtual devices can be extended independently

---

## B) GPIO MMIO Register Map

The GPIO device exposes a set of 32-bit registers via an 8-bit MMIO bus. Each 32-bit
register is mapped to four consecutive byte offsets using little-endian layout.
Firmware must perform four `read8` / `write8` operations to access a full 32-bit value.

The total MMIO size of the GPIO device is 0x18 bytes.

### Register Map

| Offset | Name      | Access | Width | Description |
|------:|-----------|--------|-------|-------------|
| 0x00  | DIR       | RW     | 32    | Direction register. Bit = 1 configures the pin as output, 0 as input. |
| 0x04  | DATA_IN   | RO     | 32    | Input data register with defined readback policy. |
| 0x08  | DATA_OUT  | RW     | 32    | Output data latch register. |
| 0x0C  | SET       | WO     | 32    | Write-1-to-set bits in DATA_OUT. |
| 0x10  | CLR       | WO     | 32    | Write-1-to-clear bits in DATA_OUT. |
| 0x14  | TOGGLE    | WO     | 32    | Write-1-to-toggle bits in DATA_OUT. |

### Access semantics

- All registers are accessed using byte-wide MMIO transactions (`read8` / `write8`).
- For RW registers (DIR, DATA_OUT), byte writes use read-modify-write semantics to update
  the corresponding byte of the 32-bit value.
- For WO registers (SET, CLR, TOGGLE), each byte write is interpreted as part of a 32-bit
  write-1 mask and applied immediately.
- Reading from write-only registers returns a deterministic value of `0x00000000`.

### DATA_IN readback policy

The DATA_IN register follows a defined readback rule that depends on pin direction:

- For pins configured as input (`DIR = 0`), the value reflects the injected input level.
- For pins configured as output (`DIR = 1`), the value reflects the output latch state.

Formally, the value returned by DATA_IN is computed as:
```pgsql
DATA_IN = (INPUT_MASK & ~DIR_MASK) | (OUTPUT_MASK & DIR_MASK)
```

### Invalid and unsupported operations

GPIO MMIO behavior follows the generic MMIO contract defined in `docs/mmio_contract.md`:

- Reads from unknown or out-of-range offsets return a deterministic value (`0x00`) and log a warning.
- Writes to unknown or out-of-range offsets are ignored and log a warning.
- Writes to read-only registers (DATA_IN) are ignored and log a warning.
- MMIO register errors never terminate the simulation.

---

## C) Board YAML Schema and Examples

GPIO, LEDs, and Buttons are configured using dedicated sections in `board.yaml`. These
sections act as syntactic sugar and are internally translated into device descriptions
during board parsing.

The GPIO section is required if LEDs or Buttons are used.

### GPIO section

```yaml
gpio:
  mmio_base: <address>
  pin_count: <number>   # optional, default: 32
```
- `mmio_base` (required): Base address of the GPIO MMIO registers. A matching MMIO
memory region with the same base address must exist.

- `pin_count` (optional): Number of GPIO pins. Valid range is 1..32. Default is 32.

The GPIO section cannot be used together with a legacy `devices[].type == gpio` definition.

### LEDs section

```yaml
leds:
  - name: <string>
    pin: <number>
    active_high: <bool>   # optional, default: true
```
Each LED:

- subscribes to a GPIO output pin

- logs state changes as `ON` / `OFF`

- requires a unique `name`

- must use a `pin` in range `0 .. pin_count-1`

Pins used by LEDs and Buttons must be globally unique.

### Buttons section
```yaml
buttons:
  - name: <string>
    pin: <number>
    active_high: <bool>   # optional, default: true
    mode: momentary      # optional, default
```
In v0.3:

- only `momentary` buttons are supported

- no debounce or interrupt logic is implemented

Buttons inject input levels into the GPIO controller based on their state and polarity.

### Example: GPIO blinky board
```yaml
board:
  name: gpio-blinky
  description: Demo board with GPIO MMIO + Virtual LED blinky (FakeCPU program toggles pin0)
  version: 1

  cpu:
    type: test-cpu
    frequency_hz: 1000000
    endianness: little

  memory:
    - name: ram
      type: ram
      base: 0x00000000
      size: 65536

    - name: gpio_mmio
      type: mmio
      base: 0x00006000
      size: 0x00000100

  gpio:
    mmio_base: 0x00006000
    pin_count: 32

  leds:
    - name: led1
      pin: 0
      active_high: true
```
### Example: GPIO with LED and Button
```yaml
board:
  name: gpio-led-button-board
  description: Demo board with GPIO MMIO + Virtual LED + Virtual Button
  version: 1

  cpu:
    type: test-cpu
    frequency_hz: 1000000
    endianness: little

  memory:
    - name: ram
      type: ram
      base: 0x00000000
      size: 65536

    - name: gpio_mmio
      type: mmio
      base: 0x20000000
      size: 0x00000100

  gpio:
    mmio_base: 0x20000000
    pin_count: 32

  leds:
    - name: led1
      pin: 0
      active_high: true

  buttons:
    - name: btn1
      pin: 1
      active_high: true
      mode: momentary
```
---

## D) Run Instructions (Demo: gpio-blinky)

This section shows how to run the `gpio-blinky` demo board and verify that GPIO MMIO
and the Virtual LED integration work as expected.

### 1) Validate the board configuration (dry-run)

From the build directory:

```bash
./elsim --dry-run --config ../examples/board-examples/gpio-blinky-board.yaml
```
The command must succeed without errors. This verifies that the YAML schema is valid
and that the `gpio.mmio_base` matches an MMIO memory region.

### 2) Run the demo program
Run the simulator with the demo board and the prebuilt FakeCPU program:

```bash
./elsim --config ../examples/board-examples/gpio-blinky-board.yaml \
  --program ../examples/gpio_blinky.elsim-bin \
  --log-level debug
```
### 3) What to expect in logs
When the program toggles GPIO pin 0, the Virtual LED will report state transitions.
At minimum, you should see LED logs like:

- `led1 -> ON`

- `led1 -> OFF`

With `--log-level debug`, you will also see GPIO MMIO read/write traces from the GPIO
device and may see button traces on other boards (VirtualButtonDevice logs presses and
releases at DEBUG level).

---

## E) Notes and Limitations (v0.3)

The following limitations apply to the GPIO subsystem in ELSIM v0.3:

- The GPIO MMIO device supports a maximum of 32 pins. While the internal controller
  can represent up to 64 pins, the MMIO-facing device enforces a range of 1..32 pins.

- GPIO registers are exposed as 32-bit values over an 8-bit MMIO bus. Firmware must
  perform multiple byte accesses to read or write full register values.

- No timing model is implemented for GPIO. The GPIO device does not participate in
  simulator ticks and has no notion of delays or propagation time.

- Virtual buttons do not implement debounce logic. Each press or release immediately
  injects an input level into the GPIO controller.

- Interrupts and edge-triggered events are not supported. GPIO interaction is limited
  to polling via MMIO registers.

- Only momentary buttons are supported. Other button modes are reserved for future
  versions but are not implemented in v0.3.
