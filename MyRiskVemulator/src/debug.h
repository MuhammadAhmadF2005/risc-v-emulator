#pragma once

#include "cpu.h"
#include "memory.h"
#include <iostream>

// --- Printing utilities --- //
inline void printInstruction(const CPU &cpu, const Instruction &inst)
{
    std::cout << "PC=" << cpu.pc << "  " << opNames[inst.op];
    
    // Grouping by instruction format type massively shrinks this switch
    switch (inst.op) {
        // R-type
        case ADD: case SUB: case SLL: case SLT: case SLTU: case XOR: case SRL: case SRA: case OR: case AND:
            std::cout << " " << regNames[inst.rd] << ", " << regNames[inst.rs1] << ", " << regNames[inst.rs2]; break;
        // I-type ALU
        case ADDI: case SLTI: case SLTIU: case XORI: case ORI: case ANDI: case SLLI: case SRLI: case SRAI:
            std::cout << " " << regNames[inst.rd] << ", " << regNames[inst.rs1] << ", " << inst.imm; break;
        // U-type
        case LUI: case AUIPC:
            std::cout << " " << regNames[inst.rd] << ", " << inst.imm; break;
        // Loads
        case LB: case LH: case LW: case LBU: case LHU:
            std::cout << " " << regNames[inst.rd] << ", " << inst.imm << "(" << regNames[inst.rs1] << ")"; break;
        // Stores
        case SB: case SH: case SW:
            std::cout << " " << regNames[inst.rs2] << ", " << inst.imm << "(" << regNames[inst.rs1] << ")"; break;
        // Branches
        case BEQ: case BNE: case BLT: case BGE: case BLTU: case BGEU:
            std::cout << " " << regNames[inst.rs1] << ", " << regNames[inst.rs2] << ", offset=" << inst.imm; break;
        // Jumps
        case JAL:
            std::cout << " " << regNames[inst.rd] << ", offset=" << inst.imm; break;
        case JALR:
            std::cout << " " << regNames[inst.rd] << ", " << regNames[inst.rs1] << ", offset=" << inst.imm; break;
        // Misc
        case FENCE: case ECALL: case EBREAK: case NOP:
            break;
    }
    std::cout << "\n";
}

inline void printRegisters(const CPU &cpu)
{
    std::cout << "Registers:\n";
    for (int i = 0; i < 32; ++i) {
        if (cpu.reg[i] != 0) 
            std::cout << "  " << regNames[i] << " = " << cpu.reg[i] << "\n";
    }
}

inline void printMemory(const CPU &cpu, int startAddr = 0, int words = 8)
{
    std::cout << "Memory [" << startAddr << ".." << startAddr + words * 4 - 4 << "]:\n";
    for (int i = 0; i < words; ++i) {
        int addr = startAddr + i * 4;
        if (addr + 3 >= (int)cpu.mem.size()) break;
        u32 val = read32(cpu, addr);
        if (val != 0)
            std::cout << "  mem[" << addr << "] = " << val << "\n";
    }
}
