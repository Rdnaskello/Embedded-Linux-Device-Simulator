# GPIO MMIO Subsystem (ELSIM v0.3)

This document describes the GPIO subsystem in ELSIM, including its MMIO register map,
internal model, board.yaml configuration, and usage examples.

The goal is to provide a stable, precise contract for GPIO behavior, aligned with the
actual implementation in ELSIM v0.3.

---

## A) Overview

This section explains the GPIO model in ELSIM and how it integrates with the MemoryBus
and other virtual devices.

---

## B) GPIO MMIO Register Map

This section defines the complete MMIO register map for the GPIO device, including
offsets, access types, data width, and behavior.

---

## C) Board YAML Schema and Examples

This section describes how to configure GPIO, LEDs, and Buttons using board.yaml,
including validation rules and examples.

---

## D) Run Instructions (Demo: gpio-blinky)

This section explains how to run the GPIO blinky demo and what to expect in logs.

---

## E) Notes and Limitations (v0.3)

This section documents known limitations and non-supported features in GPIO v0.3.
