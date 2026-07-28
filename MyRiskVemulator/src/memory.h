#pragma once

#include "cpu.h"
#include <iostream>

// --- Memory Helpers ---
// Consolidate bounds checking to avoid copy-pasting it
inline bool checkMem(const CPU &cpu, int address, int bytes)
{
    if (address < 0 || address + bytes > (int)cpu.mem.size()) {
        std::cerr << "Memory out of bounds at: " << address << std::endl;
        return false;
    }
    return true;
}

inline u8 read8(const CPU &cpu, int addr) {
    return checkMem(cpu, addr, 1) ? cpu.mem[addr] : 0;
}

inline u16 read16(const CPU &cpu, int addr) {
    return checkMem(cpu, addr, 2) ? (cpu.mem[addr] | (cpu.mem[addr + 1] << 8)) : 0;
}

inline u32 read32(const CPU &cpu, int addr) {
    if (!checkMem(cpu, addr, 4)) return 0;
    return cpu.mem[addr] | (cpu.mem[addr+1] << 8) | (cpu.mem[addr+2] << 16) | (cpu.mem[addr+3] << 24);
}

inline void write8(CPU &cpu, int addr, u8 val) {
    if (checkMem(cpu, addr, 1)) cpu.mem[addr] = val;
}

inline void write16(CPU &cpu, int addr, u16 val) {
    if (checkMem(cpu, addr, 2)) {
        cpu.mem[addr] = val & 0xFF;
        cpu.mem[addr + 1] = (val >> 8) & 0xFF;
    }
}

inline void write32(CPU &cpu, int addr, u32 val) {
    if (checkMem(cpu, addr, 4)) {
        cpu.mem[addr] = val & 0xFF;
        cpu.mem[addr + 1] = (val >> 8) & 0xFF;
        cpu.mem[addr + 2] = (val >> 16) & 0xFF;
        cpu.mem[addr + 3] = (val >> 24) & 0xFF;
    }
}
