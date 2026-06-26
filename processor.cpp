#include <iostream>
#include <vector>

using namespace std;

// CPU

struct CPU
{
    int reg[32] = {0}; // 32 registers
    int pc = 0;        // Program Counter
};

//----------------------
// Supported Instructions
//----------------------

enum Opcode
{
    ADD,
    SUB,
    ADDI
};

//----------------------
// Instruction Format
//----------------------

struct Instruction
{
    Opcode op;

    int rd;  // destination register
    int rs1; // source register 1
    int rs2; // source register 2

    int imm; // immediate value (used by ADDI)
};

//----------------------
// Execute One Instruction
//----------------------

void execute(CPU &cpu, const Instruction &inst)
{
    switch (inst.op)
    {
    case ADD:
        cpu.reg[inst.rd] =
            cpu.reg[inst.rs1] +
            cpu.reg[inst.rs2];
        break;

    case SUB:
        cpu.reg[inst.rd] =
            cpu.reg[inst.rs1] -
            cpu.reg[inst.rs2];
        break;

    case ADDI:
        cpu.reg[inst.rd] =
            cpu.reg[inst.rs1] +
            inst.imm;
        break;
    }

    cpu.pc++;
}

//----------------------
// Main
//----------------------

int main()
{
    CPU cpu;

    vector<Instruction> program =
        {
            {ADDI, 1, 0, 0, 10}, // x1 = x0 + 10
            {ADDI, 2, 0, 0, 20}, // x2 = x0 + 20
            {ADD, 3, 1, 2, 0},   // x3 = x1 + x2
            {SUB, 4, 3, 1, 0}    // x4 = x3 - x1
        };

    while (cpu.pc < program.size())
    {
        Instruction current = program[cpu.pc];

        execute(cpu, current);
    }

    cout << "Registers after execution\n\n";

    for (int i = 0; i < 5; i++)
    {
        cout << "x" << i << " = "
             << cpu.reg[i]
             << endl;
    }

    return 0;
}