#include "cpu.h"
#include "decode.h"
#include "debug.h"
#include "memory.h"
#include "elf.h"
#include "platform.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

using namespace std;

// ---------------------------------------------------------------------------
// run() — fetch-decode-execute loop.
// Stops when cpu.halted is set (sys_exit), when pc leaves [0, memSize),
// or when maxSteps is reached (safety guard for the demo mode).
// ---------------------------------------------------------------------------
static void run(CPU &cpu, u32 textEnd, int maxSteps = 0, bool verbose = true)
{
    int steps = 0;
    while (!cpu.halted && cpu.pc < textEnd)
    {
        if (maxSteps > 0 && ++steps > maxSteps) {
            cerr << "run: reached max steps (" << maxSteps << ")\n";
            break;
        }

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

// ---------------------------------------------------------------------------
// pushString — writes a null-terminated C string into cpu.mem at addr,
// returns the address of the byte after the terminating '\0'.
// ---------------------------------------------------------------------------
static u32 pushString(CPU &cpu, u32 addr, const char* str)
{
    size_t len = strlen(str);
    for (size_t i = 0; i <= len; ++i) {
        if (addr + (u32)i < (u32)cpu.mem.size())
            cpu.mem[addr + (u32)i] = (u8)str[i];
    }
    return addr + (u32)len + 1;
}

// ---------------------------------------------------------------------------
// setupLinuxStack — writes an argc/argv array onto the stack in the standard
// Linux ELF ABI format so that crt0.S can pass them directly to main().
//
// Stack layout (growing downward, sp points to argc):
//   [sp+0 ] = argc (4 bytes)
//   [sp+4 ] = argv[0] ptr
//   [sp+8 ] = argv[1] ptr   (if argc==2)
//   [sp+12] = NULL          (argv sentinel)
//   [sp+16] = NULL          (env sentinel)
//   ...string data follows above sp in memory (lower addresses)...
//
// We place strings just below the initial sp position and build the table
// above them, then return the adjusted sp value.
// ---------------------------------------------------------------------------
static u32 setupLinuxStack(CPU &cpu, u32 sp, const std::vector<std::string>& args)
{
    // Reserve space for strings below sp (reserve 1024 bytes)
    u32 strBase = sp - 1024;
    u32 ptr     = strBase;

    std::vector<u32> argAddrs;
    for (const auto& arg : args) {
        argAddrs.push_back(ptr);
        ptr = pushString(cpu, ptr, arg.c_str());
    }

    int argc = (int)args.size();
    u32 tableBase = strBase - 64 - argc * 4;
    tableBase &= ~15u; // 16-byte align

    write32(cpu, (int)tableBase, (u32)argc);
    for (int i = 0; i < argc; ++i) {
        write32(cpu, (int)tableBase + 4 * (i + 1), argAddrs[i]);
    }
    write32(cpu, (int)tableBase + 4 * (argc + 1), 0); // argv NULL terminator
    write32(cpu, (int)tableBase + 4 * (argc + 2), 0); // envp NULL terminator

    return tableBase; // new sp points at argc
}

// ---------------------------------------------------------------------------
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
        cout << "Loaded ELF file: " << argv[1] << "\n";
        cout << "heapBase = 0x" << hex << cpu.heapBase << dec << "\n\n";

        // Fix 1: initialise stack pointer near top of 64 MB address space
        u32 sp = (u32)(64 * 1024 * 1024) - 16;

        // Fix 2: set up Linux ABI stack with argc/argv
        std::vector<std::string> guestArgs;
        guestArgs.push_back(argv[1]);
        if (argc >= 3) {
            guestArgs.push_back("-iwad");
            guestArgs.push_back(argv[2]);
        }
        for (int i = 3; i < argc; ++i) {
            guestArgs.push_back(argv[i]);
        }

        sp = setupLinuxStack(cpu, sp, guestArgs);
        cpu.reg[2] = sp; // x2 = sp

        // Init SDL host display (no-op if platform_sdl is not linked)
        hostInit(320, 200);

        // Run until sys_exit or end of memory — no step limit for ELF mode
        run(cpu, (u32)cpu.mem.size(), 0, false);
        printRegisters(cpu);

    } else {
        // --- Hardcoded demo (no args): unchanged behaviour ---
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
