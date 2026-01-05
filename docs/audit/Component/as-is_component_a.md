# As-Is Audit — Component A (CPU ↔ Memory via IMemoryBus) — ELSIM v0.3.0

> **Component ID:** A  
> **Component name:** CPU ↔ Memory integration path (FakeCpu + MemoryBus + MemoryBusAdapter)  
> **Audit type:** As-Is (no judgments, only verifiable facts with file:line evidence)  
> **Version scope:** ELSIM v0.3.0 (repo snapshot)  
> **Date:** 2026-01-05  
> **Branch:** feature/TASK-AUDIT-1.1-component-a-as-is

---

## 1) Purpose / Intent (As-Is)

Component A забезпечує базову інтеграцію CPU ↔ Memory:

- **FakeCpu** виконує інструкції, роблячи memory access через **IMemoryBus** (8-bit read/write).
- **MemoryBusAdapter** — тонкий адаптер, який дозволяє FakeCpu бачити конкретний **MemoryBus** як IMemoryBus.
- **MemoryBus** реалізує байтовий доступ до RAM та маршрутизацію на MMIO девайси.
- Є smoke-приклад, який перевіряє, що CPU реально читає/пише через IMemoryBus.

Доказ існування інтеграційного сценарію:  
`examples/basic_cpu_memory_test.cpp:15–21, 28–40`

---

## 2) Scope

### In-Scope
- FakeCpu: fetch/decode/execute pipeline, PC/FLAGS/reset semantics, інструкції ISA: MOV/ADD/SUB/LOAD/STORE/JMP/JZ/JNZ/HALT.  
  `src/core/FakeCpu.cpp:107–424, 426–456`
- MemoryBus: RAM path, MMIO path, mapDevice overlap rules, out-of-range behavior.  
  `src/core/MemoryBus.cpp:20–143`
- MemoryBusAdapter: null-check + делегування read8/write8.  
  `src/core/MemoryBusAdapter.cpp:7–19`
- Logger: рівні, фільтрація, ANSI-кольори, singleton.  
  `src/core/Logger.cpp:30–62`
- ISA doc як специфікація очікуваної моделі.  
  `docs/fakecpu_isa.md:272–520`

### Out-of-Scope
- loader/binary format (реальне завантаження інструкцій у RAM), інші девайси/перефирія (UART/Timer/GPIO), simulator loop.  
  (не входить у даний компонент)

---

## 3) Source of Truth (Artifacts)

### Example / smoke
- `examples/basic_cpu_memory_test.cpp`  
  `examples/basic_cpu_memory_test.cpp:11–44`

### CPU core
- `include/elsim/core/FakeCpu.hpp`  
  `include/elsim/core/FakeCpu.hpp:13–104`
- `src/core/FakeCpu.cpp`  
  `src/core/FakeCpu.cpp:14–456`

### Memory bus
- `include/elsim/core/MemoryBus.hpp`  
  `include/elsim/core/MemoryBus.hpp:22–57`
- `src/core/MemoryBus.cpp`  
  `src/core/MemoryBus.cpp:17–143`

### Adapter
- `include/elsim/core/MemoryBusAdapter.hpp` (thin wrapper over MemoryBus, via IMemoryBus)  
  (line-refs не надані в цьому лог-виводі, але реалізація підтверджена нижче)
- `src/core/MemoryBusAdapter.cpp`  
  `src/core/MemoryBusAdapter.cpp:7–19`

### Logging
- `include/elsim/core/Logger.hpp` (API)  
  (line-refs не надані у цьому лог-виводі)
- `src/core/Logger.cpp`  
  `src/core/Logger.cpp:30–62`

### ISA spec
- `docs/fakecpu_isa.md`  
  `docs/fakecpu_isa.md:272–520`

---

## 4) Component Structure (As-Is)

### 4.1 MemoryBus: RAM + MMIO routing
- RAM: `m_memory` (vector<uint8_t>), доступ байтами.  
  `include/elsim/core/MemoryBus.hpp:48–52`
- MMIO mappings: `m_devices` list, елементи містять `base/size/device`.  
  `include/elsim/core/MemoryBus.hpp:41–46, 51–56`
- Пошук девайса — лінійний scan за діапазоном `[base, base+size)`.  
  `src/core/MemoryBus.cpp:20–31`

### 4.2 MemoryBusAdapter: IMemoryBus wrapper
- Реалізація read8/write8 робить null-check `bus_` і делегує у MemoryBus.  
  `src/core/MemoryBusAdapter.cpp:7–19`

### 4.3 FakeCpu: fetch → decode → execute
- Архітектурний стан: `R0..R7`, `PC`, `SP`, `FLAGS`.  
  `include/elsim/core/FakeCpu.hpp:15–33`
- HALT-стан `halted_`, step counter `stepCount_`, memory bus pointer `memoryBus_`.  
  `include/elsim/core/FakeCpu.hpp:83–103`

---

## 5) External Interfaces / Contracts (As-Is)

### 5.1 MemoryBus read8/write8 behavior
1) **MMIO path:** якщо адреса належить змепленому девайсу → делегування `device->read8(offset)` / `device->write8(offset, value)` + debug log.  
   `src/core/MemoryBus.cpp:37–48` (read MMIO)  
   `src/core/MemoryBus.cpp:74–85` (write MMIO)

2) **RAM path:** якщо адреса не в MMIO → доступ до `m_memory[address]` + debug log.  
   `src/core/MemoryBus.cpp:50–67` (read RAM)  
   `src/core/MemoryBus.cpp:87–104` (write RAM)

3) **Out-of-range:** якщо адреса не в MMIO і `address >= m_memory.size()` → error log + `std::out_of_range`.  
   `src/core/MemoryBus.cpp:51–59` (read OOR)  
   `src/core/MemoryBus.cpp:88–96` (write OOR)

### 5.2 MemoryBus mapDevice behavior
- `device == nullptr` → `std::invalid_argument`.  
  `src/core/MemoryBus.cpp:110–113`
- Overlap діапазонів → `std::runtime_error`.  
  `src/core/MemoryBus.cpp:118–134`
- Успіх: додає запис у `m_devices`.  
  `src/core/MemoryBus.cpp:141–142`

### 5.3 FakeCpu memory access contract (via IMemoryBus)
- Word fetch/read/write реалізовані через 4× `read8` / `write8` у little-endian.  
  `src/core/FakeCpu.cpp:62–83` (read32)  
  `src/core/FakeCpu.cpp:85–103` (write32)

---

## 6) ISA alignment (documented spec ↔ implementation)

ISA документ визначає:
- 32-bit, byte-addressable, little-endian, instruction size 4 bytes.  
  (попередні секції ISA; у наданому шматку підтверджується модель для LOAD/STORE/JMP/JZ/JNZ/HALT та execution model)  
- Семантика LOAD/STORE/JMP/JZ/JNZ/HALT та reset/halt model.  
  `docs/fakecpu_isa.md:272–520`

В реалізації FakeCpu:
- LOAD: обчислює EA = base(Rs) + sign_extend(imm16), читає MEM32[EA], записує у Rd, оновлює Z/N, PC += 4.  
  `src/core/FakeCpu.cpp:241–266`
- STORE: EA = base(Rd) + sign_extend(imm16), MEM32[EA] = Rs, FLAGS unchanged, PC += 4.  
  `src/core/FakeCpu.cpp:269–294`
- JMP/JZ/JNZ: offset16 у “словах” → `offsetBytes = imm16 << 2`, target = PC+4+offsetBytes (умовно для JZ/JNZ).  
  `src/core/FakeCpu.cpp:296–379`
- HALT: ставить `halted_=true`, PC не змінює.  
  `src/core/FakeCpu.cpp:382–387`
- Reset state: regs=0, PC=0, SP=0, FLAGS=0, halted=false, stepCount=0.  
  `src/core/FakeCpu.cpp:426–445`

---

## 7) Execution pipeline (As-Is)

### 7.1 step()
- `step()` інкрементить `stepCount_`.  
  `src/core/FakeCpu.cpp:402–405`
- Якщо halted_ → нічого не робить.  
  `src/core/FakeCpu.cpp:406–410`
- Якщо memoryBus_ не підключений → warn і return.  
  `src/core/FakeCpu.cpp:412–416`
- Fetch: `read32(state_.pc)`; Decode+Execute: `decodeAndExecute(instruction)`.  
  `src/core/FakeCpu.cpp:418–424`

### 7.2 decodeAndExecute()
- Якщо halted_ → ігнорує інструкцію.  
  `src/core/FakeCpu.cpp:107–112`
- Opcode = instr >> 24, rd/rs/isImm/imm16 декодуються з нижніх бітів.  
  `src/core/FakeCpu.cpp:114–122`
- Невідомий opcode → “treating as NOP”, PC += 4.  
  `src/core/FakeCpu.cpp:389–395`

---

## 8) Smoke scenario (As-Is) — `basic_cpu_memory_test`

Smoke тест робить:
1) `MemoryBus bus(64)` + `MemoryBusAdapter(&bus)`  
   `examples/basic_cpu_memory_test.cpp:15–17`
2) `FakeCpu cpu; cpu.setMemoryBus(busAdapter);`  
   `examples/basic_cpu_memory_test.cpp:19–21`
3) Перевіряє, що `RAM[0..4] == 0`  
   `examples/basic_cpu_memory_test.cpp:23–25`
4) Викликає `cpu.step()` 5 разів  
   `examples/basic_cpu_memory_test.cpp:28–31`
5) Перевіряє, що `RAM[0..4] == 1`  
   `examples/basic_cpu_memory_test.cpp:36–40`

Фактичний механізм, який забезпечує RAM-модифікацію в цьому тесті:
- `step()` fetch-ить 32-бітне слово з RAM за PC і виконує інструкцію.  
  `src/core/FakeCpu.cpp:418–424`
- Зміни пам’яті можливі лише через `write32()` (STORE) або `write8()` (через bus), але у CPU core memory writes виконуються через `write32()`.  
  `src/core/FakeCpu.cpp:85–103, 269–294`

(Тобто test проходить лише якщо у RAM на адресах `PC=0,4,8,...` лежать валідні інструкції, які в підсумку приводять до записів у RAM[0..4]. Сам файл тесту не завантажує інструкцій у RAM.)

---

## 9) Failure modes (As-Is)

- `MemoryBusAdapter::read8/write8`: якщо `bus_ == nullptr` → `std::runtime_error`.  
  `src/core/MemoryBusAdapter.cpp:7–19`
- `MemoryBus::read8/write8`: якщо адреса не у MMIO і поза RAM → `std::out_of_range`.  
  `src/core/MemoryBus.cpp:51–59, 88–96`
- `FakeCpu::step`: якщо memoryBus_ не підключений → warn і return (без exception).  
  `src/core/FakeCpu.cpp:412–416`
- `FakeCpu::read32/write32`: якщо memoryBus_ не підключений → warn + (0/return).  
  `src/core/FakeCpu.cpp:63–66, 86–89`

---

## 10) Logging (As-Is)

- Logger singleton, дефолт INFO, фільтрація `level < current_level_`, ANSI кольори, вивід у `std::clog`.  
  `src/core/Logger.cpp:30–54`
- MemoryBus логує read/write для RAM і MMIO (debug).  
  `src/core/MemoryBus.cpp:42–47, 63–66, 78–84, 98–103`
- FakeCpu логує fetch/write32 (debug) та виконання інструкцій у decodeAndExecute (debug).  
  `src/core/FakeCpu.cpp:76–80, 98–102, 128–134`

---

## 11) Acceptance checklist (Component A)

- [x] Вказані source-of-truth артефакти з file:line.
- [x] Зафіксовано контракти MemoryBus (RAM/MMIO/OOR) з доказами.
- [x] Зафіксовано FakeCpu execution pipeline (step + decodeAndExecute) з доказами.
- [x] Зафіксовано ISA alignment для LOAD/STORE/JMP/JZ/JNZ/HALT і reset/halt model з доказами.
- [x] Зафіксовано smoke сценарій та очікування тесту з доказами.
