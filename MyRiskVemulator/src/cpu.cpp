#include "cpu.h"
#include "memory.h"
#include <iostream>

// all instructions be executed in this block
void execute(CPU &cpu, const Instruction &inst)
{
    switch (inst.op)
    {
    case ADD:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] + cpu.reg[inst.rs2];
        cpu.pc++;
        break;
    case SUB:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] - cpu.reg[inst.rs2];
        cpu.pc++;
        break;
    case ADDI:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] + (u32)inst.imm;
        cpu.pc++;
        break;

    // LB: load byte, sign-extend to 32 bits
    case LB: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        cpu.reg[inst.rd] = (u32)(i32)(i8)read8(cpu, address);
        cpu.pc++;
        break;
    }
    // LH: load halfword, sign-extend to 32 bits (must be 2-byte aligned)
    case LH: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        if (address % 2 != 0) std::cerr << "LH: misaligned address " << address << std::endl;
        else cpu.reg[inst.rd] = (u32)(i32)(i16)read16(cpu, address);
        cpu.pc++;
        break;
    }
    case LW: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        if (address % 4 != 0) std::cerr << "LW: misaligned address " << address << std::endl;
        else cpu.reg[inst.rd] = read32(cpu, address);
        cpu.pc++;
        break;
    }
    // LBU: load byte, zero-extend (no sign extension)
    case LBU: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        cpu.reg[inst.rd] = (u32)read8(cpu, address);
        cpu.pc++;
        break;
    }
    // LHU: load halfword, zero-extend (must be 2-byte aligned)
    case LHU: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        if (address % 2 != 0) std::cerr << "LHU: misaligned address " << address << std::endl;
        else cpu.reg[inst.rd] = (u32)read16(cpu, address);
        cpu.pc++;
        break;
    }

    // SB: store the lowest byte of rs2
    case SB: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        write8(cpu, address, (u8)(cpu.reg[inst.rs2] & 0xFF));
        cpu.pc++;
        break;
    }
    // SH: store the lowest halfword of rs2 (must be 2-byte aligned)
    case SH: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        if (address % 2 != 0) std::cerr << "SH: misaligned address " << address << std::endl;
        else write16(cpu, address, (u16)(cpu.reg[inst.rs2] & 0xFFFF));
        cpu.pc++;
        break;
    }
    case SW: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        if (address % 4 != 0) std::cerr << "SW: misaligned address " << address << std::endl;
        else write32(cpu, address, cpu.reg[inst.rs2]);
        cpu.pc++;
        break;
    }

    case BEQ:
        cpu.pc = (cpu.reg[inst.rs1] == cpu.reg[inst.rs2]) ? (u32)((i32)cpu.pc + inst.imm) : cpu.pc + 1;
        break;
    case BNE:
        cpu.pc = (cpu.reg[inst.rs1] != cpu.reg[inst.rs2]) ? (u32)((i32)cpu.pc + inst.imm) : cpu.pc + 1;
        break;
    // BLT: branch if rs1 < rs2 (SIGNED)
    case BLT:
        cpu.pc = ((i32)cpu.reg[inst.rs1] < (i32)cpu.reg[inst.rs2]) ? (u32)((i32)cpu.pc + inst.imm) : cpu.pc + 1;
        break;
    // BGE: branch if rs1 >= rs2 (SIGNED)
    case BGE:
        cpu.pc = ((i32)cpu.reg[inst.rs1] >= (i32)cpu.reg[inst.rs2]) ? (u32)((i32)cpu.pc + inst.imm) : cpu.pc + 1;
        break;
    // BLTU: branch if rs1 < rs2 (UNSIGNED)
    case BLTU:
        cpu.pc = (cpu.reg[inst.rs1] < cpu.reg[inst.rs2]) ? (u32)((i32)cpu.pc + inst.imm) : cpu.pc + 1;
        break;
    // BGEU: branch if rs1 >= rs2 (UNSIGNED)
    case BGEU:
        cpu.pc = (cpu.reg[inst.rs1] >= cpu.reg[inst.rs2]) ? (u32)((i32)cpu.pc + inst.imm) : cpu.pc + 1;
        break;

    case JAL:
        cpu.reg[inst.rd] = cpu.pc + 1; // return address
        cpu.pc = (u32)((i32)cpu.pc + inst.imm);
        break;

    case NOP: FENCE:
        cpu.pc++;
        break;

    // ECALL / EBREAK (stubs)
    case ECALL:
        std::cout << "ECALL at pc=" << cpu.pc << std::endl;
        cpu.pc++;
        break;
    case EBREAK:
        std::cout << "EBREAK at pc=" << cpu.pc << std::endl;
        cpu.pc++;
        break;

    // LUI simply loads a 20bit immediate into the top 20 bits of rd
    case LUI:
        cpu.reg[inst.rd] = (u32)inst.imm << 12;
        cpu.pc++;
        break;
    // AUIPC adds the 20bit immediate (shifted) to pc
    case AUIPC:
        cpu.reg[inst.rd] = cpu.pc + ((u32)inst.imm << 12);
        cpu.pc++;
        break;
    // JALR: jump to rs1 + imm, save return address in rd
    case JALR:
        cpu.reg[inst.rd] = cpu.pc + 1;
        cpu.pc = (u32)((i32)cpu.reg[inst.rs1] + inst.imm);
        break;

    case SLTI:
        cpu.reg[inst.rd] = ((i32)cpu.reg[inst.rs1] < inst.imm) ? 1 : 0;
        cpu.pc++;
        break;
    case SLTIU:
        cpu.reg[inst.rd] = (cpu.reg[inst.rs1] < (u32)inst.imm) ? 1 : 0;
        cpu.pc++;
        break;
    case XORI:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] ^ (u32)inst.imm;
        cpu.pc++;
        break;
    case ORI:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] | (u32)inst.imm;
        cpu.pc++;
        break;
    case ANDI:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] & (u32)inst.imm;
        cpu.pc++;
        break;
    case SLLI:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] << (inst.imm & 0x1F);
        cpu.pc++;
        break;
    case SRLI:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] >> (inst.imm & 0x1F);
        cpu.pc++;
        break;
    
    // note to reader: SRAI preserves sign bit unlike SRLI
    case SRAI:
        cpu.reg[inst.rd] = (u32)((i32)cpu.reg[inst.rs1] >> (inst.imm & 0x1F));
        cpu.pc++;
        break;

    case SLT:
        cpu.reg[inst.rd] = ((i32)cpu.reg[inst.rs1] < (i32)cpu.reg[inst.rs2]) ? 1 : 0;
        cpu.pc++;
        break;
    case SLTU:
        cpu.reg[inst.rd] = (cpu.reg[inst.rs1] < cpu.reg[inst.rs2]) ? 1 : 0;
        cpu.pc++;
        break;
    case SLL:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] << (cpu.reg[inst.rs2] & 0x1F);
        cpu.pc++;
        break;
    case SRL:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] >> (cpu.reg[inst.rs2] & 0x1F);
        cpu.pc++;
        break;
    case SRA:
        cpu.reg[inst.rd] = (u32)((i32)cpu.reg[inst.rs1] >> (cpu.reg[inst.rs2] & 0x1F));
        cpu.pc++;
        break;
    case XOR:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] ^ cpu.reg[inst.rs2];
        cpu.pc++;
        break;
    case OR:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] | cpu.reg[inst.rs2];
        cpu.pc++;
        break;
    case AND:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] & cpu.reg[inst.rs2];
        cpu.pc++;
        break;
    }

    // x0 is hardwired to zero in RISC-V, always enforce this
    cpu.reg[0] = 0;
}

//note: the instruciton logic is largely inspired from https://msyksphinz-self.github.io/riscv-isadoc/ ! Do check it out!!!
