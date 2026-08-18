# RISC-V Emulator

A RISC-V RV32IM emulator written from scratch in C++ with the goal of running DOOM.

## Status

| Component | Status |
|---|---|
| RV32I Base Integer ISA | ✅ Complete |
| M Extension (Multiply/Divide) | ✅ Complete |
| ELF32 Loader | ✅ Complete |
| Multi-Segment Loading (.text, .data, .bss) | ✅ Complete |
| Linux Syscall Interface | ✅ Partial |
| DOOM WAD Loading | 🔧 In Progress |

## Architecture

```
src/
├── types.h       Fixed-width integer aliases
├── cpu.h         CPU state, opcode enum, instruction struct
├── cpu.cpp       Instruction execution (RV32I + M extension)
├── decode.h      Binary instruction decoding
├── memory.h      Byte-addressable memory with bounds checking
├── elf.h         ELF loader declaration
├── elf.cpp       ELF32 PT_LOAD segment loader
├── debug.h       Register, memory, and instruction printing
└── main.cpp      Entry point, run loop, demo mode
```

## Building

```bash
g++ -std=c++17 -O2 -o rvemu.exe src/cpu.cpp src/elf.cpp src/main.cpp -Isrc
```

Or with CMake:

```bash
cmake -S . -B build
cmake --build build
```

## Usage

Run without arguments for a built-in demo:

```bash
./rvemu.exe
```

Run an ELF binary:

```bash
./rvemu.exe path/to/program.elf
```

## Supported Instructions

**RV32I Base:** ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND, ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI, LUI, AUIPC, LB, LH, LW, LBU, LHU, SB, SH, SW, BEQ, BNE, BLT, BGE, BLTU, BGEU, JAL, JALR, FENCE, ECALL, EBREAK

**M Extension:** MUL, MULH, MULHSU, MULHU, DIV, DIVU, REM, REMU

## Supported Syscalls

| Number | Name | Description |
|---|---|---|
| 56 | sys_openat | File open (stub) |
| 57 | sys_close | File close (stub) |
| 62 | sys_lseek | File seek (stub) |
| 63 | sys_read | File read (stub) |
| 64 | sys_write | Write to stdout |
| 93 | sys_exit | Process exit |
| 214 | sys_brk | Heap allocation |
| 222 | sys_mmap | Memory mapping (stub) |

## ELF Loader

The loader handles standard ELF32 little-endian RISC-V executables:

- Validates ELF magic, class (32-bit), and endianness (little)
- Iterates program headers and loads all PT_LOAD segments
- Zeroes BSS regions (memsz > filesz)
- Sets PC to the ELF entry point
- Initializes the stack pointer (sp) near the top of the 64 MB address space

## Testing

Generate test ELF binaries and run the full suite:

```bash
python MyRiskVemulator/tests/generate_test_elfs.py
python MyRiskVemulator/tests/generate_more_elfs.py
python MyRiskVemulator/tests/run_elf_suite.py
```

Test coverage:

| Test | Validates |
|---|---|
| loop_sum | Branching, loops, arithmetic (1+2+...+10 = 55) |
| loop_fact | M extension MUL (5! = 120) |
| loop_print | sys_write syscall, string output |
| multi_segment | Multi-segment loading (.text + .data + .bss) |
| fibonacci | Entry point offset, Fibonacci F(10) = 55 |
| hello_str | ELF string data via sys_write |
| bad_magic | Rejects invalid ELF magic |
| bad_class64 | Rejects 64-bit ELF |
| bad_endian | Rejects big-endian ELF |

## Repository Structure

- **MyRiskVemulator/** — Main emulator source and tests
- **learning/** — Practice exercises and session files from the KU Leuven CASS course

## Roadmap to DOOM

1. ~~Implement RV32I base ISA~~
2. ~~Add M extension~~
3. ~~Build ELF loader~~
4. ~~Multi-segment and BSS support~~
5. ~~Syscall stubs for file I/O~~
6. Wire syscall stubs to real host file I/O (open/read/close/lseek WAD files)
7. Implement a framebuffer device for video output
8. Add keyboard input handling
9. Cross-compile DOOM for RV32IM and run it

## References

- [RISC-V ISA Manual (Volume 1)](https://riscv.org/technical/specifications/) — Official specification
- [RISC-V Instruction Set Reference](https://msyksphinz-self.github.io/riscv-isadoc/) — Per-instruction documentation (used as primary reference for execution logic)
- [RISC-V Green Card](https://www.cl.cam.ac.uk/teaching/1617/ECAD+Arch/files/docs/RISCVGreenCardv8-20151013.pdf) — Quick reference card
- [KU Leuven CASS Course](https://cass-kul.github.io/) — Computer Architecture and System Software exercises
- [ELF Specification](https://refspecs.linuxfoundation.org/elf/elf.pdf) — Executable and Linkable Format reference
- [RISC-V Linux Syscall Table](https://jborza.com/post/2021-05-11-riscv-linux-syscalls/) — Linux ABI syscall numbers for RISC-V
- [doomgeneric](https://github.com/ozkl/doomgeneric) — Portable DOOM source for custom platforms
- [riscv-doom](https://github.com/nicebyte/riscv-doom) — DOOM on a RISC-V emulator (reference implementation)
- [Writing a RISC-V Emulator](https://book.rvemu.app/) — Step-by-step emulator development guide