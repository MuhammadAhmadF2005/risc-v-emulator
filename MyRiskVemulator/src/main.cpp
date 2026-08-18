#include "cpu.h"
#include "decode.h"
#include "debug.h"
#include "memory.h"
#include "elf.h"
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

static void run(CPU &cpu, u32 textEnd, int maxSteps = 1000, bool verbose = true)
{
    int steps = 0;
    while (cpu.pc < textEnd)
    {
        if (++steps > maxSteps) { cerr << "run: reached max steps\n"; break; }

        u32 raw = read32(cpu, cpu.pc);
        Instruction inst = decode(raw);

        if (verbose) {
            cout << "---\n";
            printInstruction(cpu, inst);
        }

        execute(cpu, inst);

        if (verbose) {
            printRegisters(cpu);
            printMemory(cpu, 0x40, 4);
            cout << "\n";
        }
    }
}

int main(int argc, char *argv[])
{
    cout << "=== RISC-V RV32I Emulator ===\n\n";

    CPU cpu;
    cpu.mem.resize(64 * 1024 * 1024, 0);

    if (argc >= 2) {
        if (!loadELF(cpu, argv[1])) {
            cerr << "Failed to load ELF file: " << argv[1] << "\n";
            return 1;
        }
        cout << "Loaded ELF file: " << argv[1] << "\n\n";
        // Run until end of memory or exit via sys_exit syscall / max steps
        run(cpu, (u32)cpu.mem.size(), 1000000, false);
        printRegisters(cpu);
    } else {
        // program occupies bytes 0x00-0x17 (6 instructions * 4 bytes)
        // data at 0x40 avoids overwriting the program
        vector<u32> demo = {
            0x00A00093,  // addi x1, x0, 10
            0x01400113,  // addi x2, x0, 20
            0x002081B3,  // add  x3, x1, x2   -> 30
            0x40110233,  // sub  x4, x2, x1   -> 10
            0x04302023,  // sw   x3, 64(x0)   -> mem[0x40] = 30
            0x04002283,  // lw   x5, 64(x0)   -> x5 = 30
        };

        u32 addr = 0;
        for (u32 w : demo) { write32(cpu, addr, w); addr += 4; }
        u32 textEnd = addr;

        cpu.pc = 0;
        cout << "--- demo (pass an ELF binary as argv[1] for real programs) ---\n\n";
        run(cpu, textEnd, 200, true);
    }

    return 0;
}
