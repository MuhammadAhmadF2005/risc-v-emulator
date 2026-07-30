#pragma once

#include "cpu.h"
#include <cstdlib>
#include <iostream>

//sext=sign extension
static i32 sext(u32 val, u32 bit) {
    u32 m = 1u << bit;
    return (val & m) ? (i32)(val | ~(m - 1)) : (i32)val;
}

static i32 immI(u32 w) { return sext(w >> 20, 11); }
static i32 immS(u32 w) { return sext(((w >> 25) << 5) | ((w >> 7) & 0x1F), 11); }
static i32 immB(u32 w) {
    return sext(((w >> 31) << 12) | (((w >> 7) & 1) << 11) |
                (((w >> 25) & 0x3F) << 5) | (((w >> 8) & 0xF) << 1), 12);
}
static i32 immU(u32 w) { return (i32)(w & 0xFFFFF000); }
static i32 immJ(u32 w) {
    return sext(((w >> 31) << 20) | (((w >> 12) & 0xFF) << 12) |
                (((w >> 20) & 1) << 11) | (((w >> 21) & 0x3FF) << 1), 20);
}

inline Instruction decode(u32 w) {
    u32 op  = w & 0x7F;
    int rd  = (w >> 7)  & 0x1F;
    int f3  = (w >> 12) & 0x7;
    int rs1 = (w >> 15) & 0x1F;
    int rs2 = (w >> 20) & 0x1F;
    int f7  = (w >> 25) & 0x7F;

    switch (op) {

    case 0x33: { // R-type
        switch (f3) {
        case 0x0: return { f7 ? SUB : ADD, rd, rs1, rs2, 0 };
        case 0x1: return { SLL,  rd, rs1, rs2, 0 };
        case 0x2: return { SLT,  rd, rs1, rs2, 0 };
        case 0x3: return { SLTU, rd, rs1, rs2, 0 };
        case 0x4: return { XOR,  rd, rs1, rs2, 0 };
        case 0x5: return { f7 ? SRA : SRL, rd, rs1, rs2, 0 };
        case 0x6: return { OR,   rd, rs1, rs2, 0 };
        case 0x7: return { AND,  rd, rs1, rs2, 0 };
        }
        break;
    }

    case 0x13: { // I-type ALU
        i32 imm = immI(w);
        switch (f3) {
        case 0x0: return { ADDI,  rd, rs1, 0, imm };
        case 0x1: return { SLLI,  rd, rs1, 0, imm & 0x1F };
        case 0x2: return { SLTI,  rd, rs1, 0, imm };
        case 0x3: return { SLTIU, rd, rs1, 0, imm };
        case 0x4: return { XORI,  rd, rs1, 0, imm };
        case 0x5: return { f7 ? SRAI : SRLI, rd, rs1, 0, imm & 0x1F };
        case 0x6: return { ORI,   rd, rs1, 0, imm };
        case 0x7: return { ANDI,  rd, rs1, 0, imm };
        }
        break;
    }

    case 0x03: { // Loads
        i32 imm = immI(w);
        switch (f3) {
        case 0x0: return { LB,  rd, rs1, 0, imm };
        case 0x1: return { LH,  rd, rs1, 0, imm };
        case 0x2: return { LW,  rd, rs1, 0, imm };
        case 0x4: return { LBU, rd, rs1, 0, imm };
        case 0x5: return { LHU, rd, rs1, 0, imm };
        }
        break;
    }

    case 0x23: { // Stores
        i32 imm = immS(w);
        switch (f3) {
        case 0x0: return { SB, 0, rs1, rs2, imm };
        case 0x1: return { SH, 0, rs1, rs2, imm };
        case 0x2: return { SW, 0, rs1, rs2, imm };
        }
        break;
    }

    case 0x63: { // Branches
        i32 imm = immB(w);
        switch (f3) {
        case 0x0: return { BEQ,  0, rs1, rs2, imm };
        case 0x1: return { BNE,  0, rs1, rs2, imm };
        case 0x4: return { BLT,  0, rs1, rs2, imm };
        case 0x5: return { BGE,  0, rs1, rs2, imm };
        case 0x6: return { BLTU, 0, rs1, rs2, imm };
        case 0x7: return { BGEU, 0, rs1, rs2, imm };
        }
        break;
    }

    // immU already has lower 12 bits zeroed — store as-is, execute assigns directly
    case 0x37: return { LUI,   rd, 0, 0, immU(w) };
    case 0x17: return { AUIPC, rd, 0, 0, immU(w) };
    case 0x6F: return { JAL,   rd, 0, 0, immJ(w) };
    case 0x67: return { JALR,  rd, rs1, 0, immI(w) };
    case 0x0F: return { FENCE, 0, 0, 0, 0 };

    case 0x73:
        if (w == 0x00000073) return { ECALL,  0, 0, 0, 0 };
        if (w == 0x00100073) return { EBREAK, 0, 0, 0, 0 };
        break;
    }

    std::cerr << "decode: unknown word 0x" << std::hex << w << "\n";
    std::exit(1);
}
