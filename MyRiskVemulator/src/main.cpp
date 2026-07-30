#include "cpu.h"
#include "decode.h"
#include "debug.h"
#include "memory.h"
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

/*
  The program lives in cpu.mem starting at address 0, just like a real CPU.
  cpu.pc is a byte address. Each fetch does read32(cpu, cpu.pc).
  execute() advances cpu.pc by 4 for sequential instructions, or directly
  sets it for branches and jumps using byte offsets — no unit conversion needed.

  For a real binary: ./rvemu program.bin
  That's the flat .text section you get from:
    riscv32-unknown-elf-objcopy -O binary program.elf program.bin
  ELF loading will just memcpy .text/.data into cpu.mem at the right addresses
  and set cpu.pc to e_entry — decode/execute need no changes.
*/

static u32 loadFile(CPU &cpu, const char *path) {
    ifstream f(path, ios::binary);
    if (!f) { cerr << "cannot open: " << path << "\n"; exit(1); }
    u32 addr = 0;
    u32 word;
    while (f.read(reinterpret_cast<char *>(&word), 4)) {
        if (addr + 4 > (u32)cpu.mem.size()) { cerr << "binary too large for memory\n"; exit(1); }
        write32(cpu, addr, word);
        addr += 4;
    }
    return addr;
}

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
    cpu.mem.resize(4096, 0);

    if (argc >= 2) {
        u32 textEnd = loadFile(cpu, argv[1]);
        cout << "loaded " << (textEnd / 4) << " words from " << argv[1] << "\n\n";
        cpu.pc = 0;
        run(cpu, textEnd, 100000, false);
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
        cout << "--- demo (pass a flat .bin as argv[1] for real programs) ---\n\n";
        run(cpu, textEnd, 200, true);
    }

    return 0;
}
