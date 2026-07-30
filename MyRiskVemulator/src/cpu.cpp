#include "cpu.h"
#include "memory.h"
#include <iostream>

void execute(CPU &cpu, const Instruction &inst)
{
    switch (inst.op)
    {
    case ADD:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] + cpu.reg[inst.rs2];
        cpu.pc += 4;
        break;
    case SUB:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] - cpu.reg[inst.rs2];
        cpu.pc += 4;
        break;
    case ADDI:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] + (u32)inst.imm;
        cpu.pc += 4;
        break;

    case LB: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        cpu.reg[inst.rd] = (u32)(i32)(i8)read8(cpu, address);
        cpu.pc += 4;
        break;
    }
    case LH: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        if (address % 2 != 0) std::cerr << "LH: misaligned address " << address << std::endl;
        else cpu.reg[inst.rd] = (u32)(i32)(i16)read16(cpu, address);
        cpu.pc += 4;
        break;
    }
    case LW: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        if (address % 4 != 0) std::cerr << "LW: misaligned address " << address << std::endl;
        else cpu.reg[inst.rd] = read32(cpu, address);
        cpu.pc += 4;
        break;
    }
    case LBU: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        cpu.reg[inst.rd] = (u32)read8(cpu, address);
        cpu.pc += 4;
        break;
    }
    case LHU: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        if (address % 2 != 0) std::cerr << "LHU: misaligned address " << address << std::endl;
        else cpu.reg[inst.rd] = (u32)read16(cpu, address);
        cpu.pc += 4;
        break;
    }

    case SB: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        write8(cpu, address, (u8)(cpu.reg[inst.rs2] & 0xFF));
        cpu.pc += 4;
        break;
    }
    case SH: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        if (address % 2 != 0) std::cerr << "SH: misaligned address " << address << std::endl;
        else write16(cpu, address, (u16)(cpu.reg[inst.rs2] & 0xFFFF));
        cpu.pc += 4;
        break;
    }
    case SW: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        if (address % 4 != 0) std::cerr << "SW: misaligned address " << address << std::endl;
        else write32(cpu, address, cpu.reg[inst.rs2]);
        cpu.pc += 4;
        break;
    }

    case BEQ:
        cpu.pc = (cpu.reg[inst.rs1] == cpu.reg[inst.rs2]) ? cpu.pc + (u32)inst.imm : cpu.pc + 4;
        break;
    case BNE:
        cpu.pc = (cpu.reg[inst.rs1] != cpu.reg[inst.rs2]) ? cpu.pc + (u32)inst.imm : cpu.pc + 4;
        break;
    case BLT:
        cpu.pc = ((i32)cpu.reg[inst.rs1] < (i32)cpu.reg[inst.rs2]) ? cpu.pc + (u32)inst.imm : cpu.pc + 4;
        break;
    case BGE:
        cpu.pc = ((i32)cpu.reg[inst.rs1] >= (i32)cpu.reg[inst.rs2]) ? cpu.pc + (u32)inst.imm : cpu.pc + 4;
        break;
    case BLTU:
        cpu.pc = (cpu.reg[inst.rs1] < cpu.reg[inst.rs2]) ? cpu.pc + (u32)inst.imm : cpu.pc + 4;
        break;
    case BGEU:
        cpu.pc = (cpu.reg[inst.rs1] >= cpu.reg[inst.rs2]) ? cpu.pc + (u32)inst.imm : cpu.pc + 4;
        break;

    case JAL:
        cpu.reg[inst.rd] = cpu.pc + 4;
        cpu.pc = cpu.pc + (u32)inst.imm;
        break;

    case NOP:
    case FENCE:
        cpu.pc += 4;
        break;

    case ECALL:
        std::cout << "ECALL at pc=" << cpu.pc << std::endl;
        cpu.pc += 4;
        break;
    case EBREAK:
        std::cout << "EBREAK at pc=" << cpu.pc << std::endl;
        cpu.pc += 4;
        break;

    // LUI loads the upper-20-bit immediate directly into rd (lower 12 bits are zero)
    case LUI:
        cpu.reg[inst.rd] = (u32)inst.imm;
        cpu.pc += 4;
        break;
    // AUIPC adds the upper-20-bit immediate to pc
    case AUIPC:
        cpu.reg[inst.rd] = cpu.pc + (u32)inst.imm;
        cpu.pc += 4;
        break;
    case JALR:
        cpu.reg[inst.rd] = cpu.pc + 4;
        cpu.pc = (cpu.reg[inst.rs1] + (u32)inst.imm) & ~1u;
        break;

    case SLTI:
        cpu.reg[inst.rd] = ((i32)cpu.reg[inst.rs1] < inst.imm) ? 1 : 0;
        cpu.pc += 4;
        break;
    case SLTIU:
        cpu.reg[inst.rd] = (cpu.reg[inst.rs1] < (u32)inst.imm) ? 1 : 0;
        cpu.pc += 4;
        break;
    case XORI:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] ^ (u32)inst.imm;
        cpu.pc += 4;
        break;
    case ORI:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] | (u32)inst.imm;
        cpu.pc += 4;
        break;
    case ANDI:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] & (u32)inst.imm;
        cpu.pc += 4;
        break;
    case SLLI:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] << (inst.imm & 0x1F);
        cpu.pc += 4;
        break;
    case SRLI:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] >> (inst.imm & 0x1F);
        cpu.pc += 4;
        break;
    case SRAI:
        cpu.reg[inst.rd] = (u32)((i32)cpu.reg[inst.rs1] >> (inst.imm & 0x1F));
        cpu.pc += 4;
        break;

    case SLT:
        cpu.reg[inst.rd] = ((i32)cpu.reg[inst.rs1] < (i32)cpu.reg[inst.rs2]) ? 1 : 0;
        cpu.pc += 4;
        break;
    case SLTU:
        cpu.reg[inst.rd] = (cpu.reg[inst.rs1] < cpu.reg[inst.rs2]) ? 1 : 0;
        cpu.pc += 4;
        break;
    case SLL:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] << (cpu.reg[inst.rs2] & 0x1F);
        cpu.pc += 4;
        break;
    case SRL:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] >> (cpu.reg[inst.rs2] & 0x1F);
        cpu.pc += 4;
        break;
    case SRA:
        cpu.reg[inst.rd] = (u32)((i32)cpu.reg[inst.rs1] >> (cpu.reg[inst.rs2] & 0x1F));
        cpu.pc += 4;
        break;
    case XOR:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] ^ cpu.reg[inst.rs2];
        cpu.pc += 4;
        break;
    case OR:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] | cpu.reg[inst.rs2];
        cpu.pc += 4;
        break;
    case AND:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] & cpu.reg[inst.rs2];
        cpu.pc += 4;
        break;
    }

    cpu.reg[0] = 0;
}

//note: the instruciton logic is largely inspired from https://msyksphinz-self.github.io/riscv-isadoc/ ! Do check it out!!!
