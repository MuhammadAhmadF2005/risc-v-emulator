# RISC-V Emulator

A RISC-V RV32IM emulator written from scratch in C++ with the goal of running DOOM.


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


## Repository Structure

- **MyRiskVemulator/** — Main emulator source and tests
- **learning/** — Practice exercises and session files from the KU Leuven CASS course


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
