#include "cpu.h"
#include "debug.h"
#include <iostream>
#include <vector>

using namespace std;

// --- Fetch / Decode / Execute cycle ---
Instruction fetch(const CPU &cpu, const vector<Instruction> &program) {
    return program[cpu.pc];
}

Instruction decode(const Instruction &inst) {
    // identity decoder for now; placeholder for future binary decode
    return inst;
}

/*
Note: for mem access we use little endian ordering where reading is done from 4 bytes of memory and converted ro 32 bits
for eg the 32-bit hexadecimal number 0x12345678 in memory would be stored as 0x78, 0x56, 0x34, 0x12 where 2 hex bits represent 8 binary
bits ie one byte!
and writing vice versa..

note to reader: we use SRLI and SRAI instructions for right shifting as
in our RISCV-32-simulator but there is a difference between them:
SRLI is logical right shift and SRAI is arithmetic right shift what this means for us is
in SRLI the most significant bit is filled with 0 while shifting but in SRAI the most significant
bit is filled with the sign bit (ie the most significant bit of the number being shifted)
this is done in order to preserve the sign of the number being shifted.
*/

// helper runner function takes in vector form instructions
void run(CPU &cpu, const vector<Instruction> &program, int maxSteps = 1000, bool verbose = true)
{
    int steps = 0;
    while (cpu.pc < program.size())
    {
        if (++steps > maxSteps)
        {
            cerr << "run: reached max steps !" << endl;
            break;
        }

        Instruction inst = fetch(cpu, program);
        Instruction dec = decode(inst);

        if (verbose) {
            cout << "---\n";
            printInstruction(cpu, dec);
        }

        execute(cpu, dec);

        if (verbose) {
            printRegisters(cpu);
            printMemory(cpu, 4);
            cout << endl;
        }
    }
}

//-----------------MAIN-----------------------//
int main()
{
    cout << "=== RISC-V RV32I Emulator ===" << endl;
    cout << "Loaded with 40 base instructions." << endl << endl;

    CPU demo_cpu;
    demo_cpu.mem.resize(256); // 256 bytes = 64 words

    // A quick demo loop showing off what you built
    vector<Instruction> demo = {
        {ADDI, 1, 0, 0, 10}, // x1 = 10
        {ADDI, 2, 0, 0, 20}, // x2 = 20
        {ADD,  3, 1, 2, 0},  // x3 = x1 + x2 = 30
        {SUB,  4, 2, 1, 0},  // x4 = x2 - x1 = 10
        {SW,   0, 0, 3, 0},  // store x3 at mem[0]
        {LW,   5, 0, 0, 0},  // x5 = mem[0] = 30
        {NOP,  0, 0, 0, 0}   // end
    };

    cout << "--- Running Interactive Demo ---" << endl;
    run(demo_cpu, demo, 200, true);

    // TODO: implement input via terminal directly instead of altering main file...

    return 0;
}
