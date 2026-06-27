#include <iostream>
#include <vector>

using namespace std;

// CPU

struct CPU
{
    int reg[32] = {0}; // 32b architectire
    int pc = 0;        // init pc to 0
};

// Instr

enum Opcode
{
    ADD,
    SUB,
    ADDI
};

struct Instruction
{
    Opcode op;

    int rd;
    int rs1;
    int rs2;

    int imm; // imm val
};

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