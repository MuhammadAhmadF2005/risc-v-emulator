#pragma once

#include "types.h"
#include <vector>

// CPU
struct CPU
{
    u32 reg[32] = {0}; // 32-bit registers
    u32 pc = 0;        // program counter (instruction index)
    std::vector<u8> mem;    // byte-addressable memory
};

// Instructions stored as enum for encoding rather than using opcode
enum Opcode
{
    // R-type ALU instructions
    ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,
    // I-type ALU
    ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI,
    // U-type
    LUI, AUIPC,
    // Loads
    LB, LH, LW, LBU, LHU,
    // Stores
    SB, SH, SW,
    // Branches
    BEQ, BNE, BLT, BGE, BLTU, BGEU,
    // Jumps
    JAL, JALR,
    // System / Misc
    FENCE, ECALL, EBREAK, NOP,
    // M-extension
    MUL, MULH, MULHSU, MULHU, DIV, DIVU, REM, REMU
};

// Directly map enum to string (quicker lookup)
inline const char* opNames[] = {
    "ADD", "SUB", "SLL", "SLT", "SLTU", "XOR", "SRL", "SRA", "OR", "AND",
    "ADDI", "SLTI", "SLTIU", "XORI", "ORI", "ANDI", "SLLI", "SRLI", "SRAI",
    "LUI", "AUIPC",
    "LB", "LH", "LW", "LBU", "LHU",
    "SB", "SH", "SW",
    "BEQ", "BNE", "BLT", "BGE", "BLTU", "BGEU",
    "JAL", "JALR",
    "FENCE", "ECALL", "EBREAK", "NOP",
    "MUL", "MULH", "MULHSU", "MULHU", "DIV", "DIVU", "REM", "REMU"
};

struct Instruction
{
    Opcode op;
    int rd;
    int rs1;
    int rs2;
    i32 imm; // immediate (signed)
};

// Register names for nicer printing
inline const char *regNames[32] = {
    "x0(zero)", "x1(ra)", "x2(sp)", "x3(gp)", "x4(tp)", "x5(t0)", "x6(t1)", "x7(t2)",
    "x8(s0)", "x9(s1)", "x10(a0)", "x11(a1)", "x12(a2)", "x13(a3)", "x14(a4)", "x15(a5)",
    "x16(a6)", "x17(a7)", "x18(s2)", "x19(s3)", "x20(s4)", "x21(s5)", "x22(s6)", "x23(s7)",
    "x24(s8)", "x25(s9)", "x26(s10)", "x27(s11)", "x28(t3)", "x29(t4)", "x30(t5)", "x31(t6)"
};

// --- Core functions (defined in cpu.cpp) ---
void execute(CPU &cpu, const Instruction &inst);
