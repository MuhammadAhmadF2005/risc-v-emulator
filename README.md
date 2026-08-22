# RISC-V Emulator (RV32IM + DOOM)

A high-performance RISC-V RV32IM emulator written from scratch in C++ capable of running DOOM via `doomgeneric` with SDL2 graphics and keyboard input.

## Features & Status

| Component | Status |
|---|---|
| RV32I Base Integer ISA (40 instructions) | ✅ Complete |
| RV32M Extension (MUL, DIV, REM) | ✅ Complete |
| ELF32 Loader (.text, .data, .bss, multi-segment) | ✅ Complete |
| Dynamic Heap Allocation (`sys_brk`) | ✅ Complete |
| Host File I/O Syscalls (open, read, write, lseek, close, fstat) | ✅ Complete |
| Custom Display & Input ECALLs (0x100–0x103) | ✅ Complete |
| SDL2 Interactive Video (320×200 ARGB8888) & Input | ✅ Complete |
| DOOM Execution (`doomgeneric` + `doom1.wad`) | ✅ Complete & Verified |

## Architecture

```
MyRiskVemulator/
├── src/
│   ├── types.h              Fixed-width integer aliases
│   ├── cpu.h                CPU state, opcode enum, instruction struct
│   ├── cpu.cpp              Instruction execution (RV32IM + Syscalls + Custom ECALLs)
│   ├── decode.h             Binary instruction decoding
│   ├── memory.h             Byte-addressable 64MB memory with bounds checking
│   ├── elf.h                ELF loader declaration
│   ├── elf.cpp              ELF32 PT_LOAD segment loader & heapBase setup
│   ├── debug.h              Register, memory, and instruction printing
│   ├── platform.h           Host platform interface (display, input, timer)
│   ├── platform_sdl.cpp     SDL2 frontend implementation
│   ├── platform_stub.cpp    Headless stub frontend
│   └── main.cpp             Entry point, stack argument setup, demo loop
├── dg_riscv.c               doomgeneric RISC-V guest platform layer
├── crt0.S                   RISC-V startup code for guest binaries
├── linker.ld                Linker script (entry at 0x10000)
└── Makefile                 Build system for rvemu and doom.elf
```

## Quick Start

### 1. Requirements

- **C++17 Compiler** (`g++` / `clang++`)
- **SDL2** (`libsdl2-dev` on Linux, `mingw-w64-x86_64-SDL2` on Windows/MSYS2)
- **RISC-V Cross Compiler** (`riscv32-unknown-elf-gcc` or `riscv64-unknown-elf-gcc`)

### 2. Building the Emulator and DOOM

```bash
cd MyRiskVemulator

# Build the host emulator (rvemu)
make rvemu

# (Optional) Build headless emulator without SDL2
make rvemu_headless

# Cross-compile guest DOOM binary (requires doomgeneric/ clone)
make doom.elf
```

### 3. Running DOOM

```bash
cd MyRiskVemulator
./rvemu doom.elf doom1.wad
```

#### Controls:
- **Move / Turn**: Arrow Keys
- **Fire**: Ctrl
- **Use / Open Doors**: Space
- **Speed**: Shift
- **Menu / Back**: Escape
- **Select**: Enter

### 4. Running Built-in Demo

Run without arguments to execute the 6-instruction demo:

```bash
./rvemu
```

## Supported Syscalls & ECALLs

### Linux / Newlib Syscalls
| Number | Name | Description |
|---|---|---|
| 56 / 1024 | `sys_openat` / `sys_open` | Open host files (e.g. WAD files) |
| 57 / 1026 | `sys_close` | Close file descriptor |
| 62 / 1029 | `sys_lseek` | Reposition file read/write offset |
| 63 / 1030 | `sys_read` | Read data from file / stdin |
| 64 / 1031 | `sys_write` | Write data to stdout / stderr / file |
| 80 / 1028 | `sys_fstat` | Query file status and size |
| 93 | `sys_exit` | Clean exit |
| 169 | `sys_gettimeofday` | High-resolution wall-clock time |
| 214 | `sys_brk` | Dynamic heap expansion |
| 222 | `sys_mmap` | Memory mapping |

### Custom Game ECALLs
| ECALL | Name | Arguments | Description |
|---|---|---|---|
| `0x100` | `DRAW_FRAME` | `a0=fb_ptr, a1=width, a2=height` | Blit 32-bit pixel buffer to host window |
| `0x101` | `GET_KEY` | None (returns packed key in `a0`) | Poll keyboard events |
| `0x102` | `GET_TICKS` | None (returns ms in `a0`) | Monotonic milliseconds since boot |
| `0x103` | `SLEEP` | `a0=ms` | Sleep / yield CPU |

## References

- [RISC-V ISA Specification](https://riscv.org/technical/specifications/)
- [RISC-V Instruction Set Reference](https://msyksphinz-self.github.io/riscv-isadoc/)
- [doomgeneric](https://github.com/ozkl/doomgeneric)
- [Writing a RISC-V Emulator](https://book.rvemu.app/)
