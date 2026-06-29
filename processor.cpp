#include <iostream>
#include <vector> //for dynamic allocation for memory

using namespace std;

// CPU

struct CPU
{
    int reg[32] = {0}; // 32-bit architecture
    int pc = 0;        // init pc to 0
    vector<int> mem;   // word mem
};

// Instr

enum Opcode
{
    ADD,
    SUB,
    ADDI,
    LW,
    SW
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
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] + cpu.reg[inst.rs2];
        break;

    case SUB:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] - cpu.reg[inst.rs2];
        break;

    case ADDI:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] + inst.imm;
        break;

    case LW:
    {
        int address = cpu.reg[inst.rs1] + inst.imm;
        if (address < 0 || address % 4 != 0 || address / 4 >= cpu.mem.size())
        // if adrdress is negative or not a multiple of 4 a(word adressed) or greater than allocated size
        {
            cerr << "Memory load error at address " << address << endl;
            break;
        }
        cpu.reg[inst.rd] = cpu.mem[address / 4];
        break;
    }

    case SW:
    {
        int address = cpu.reg[inst.rs1] + inst.imm;
        if (address < 0 || address % 4 != 0 || address / 4 >= cpu.mem.size())
        {
            cerr << "Memory store error at address " << address << endl;
            break;
        }
        cpu.mem[address / 4] = cpu.reg[inst.rs2];
        break;
    }
    }

    cpu.reg[0] = 0; // x0 is hardwired to zero
    cpu.pc++;
}

int main()
{
    CPU cpu;
    cpu.mem.resize(64); // 64 words of memory

    vector<Instruction> program =
        {
            {ADDI, 1, 0, 0, 10}, // x1 = 10
            {ADDI, 2, 0, 0, 20}, // x2 = 20
            {SW, 0, 0, 1, 0},    // mem[0] = x1
            {SW, 0, 0, 2, 4},    // mem[1] = x2
            {LW, 3, 0, 0, 0},    // x3 = mem[0]
            {LW, 4, 0, 0, 4},    // x4 = mem[1]
            {ADD, 5, 3, 4, 0},   // x5 = x3 + x4
            {SUB, 6, 5, 1, 0}    // x6 = x5 - x1
        };

    while (cpu.pc < program.size())
    {
        execute(cpu, program[cpu.pc]);
    }

    cout << "Registers after execution\n\n";

    for (int i = 0; i <= 6; i++)
    {
        cout << "x" << i << " = " << cpu.reg[i] << endl;
    }

    cout << "\nMemory words after execution\n";
    cout << "mem[0] = " << cpu.mem[0] << endl;
    cout << "mem[1] = " << cpu.mem[1] << endl;

    return 0;
}