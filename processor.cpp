#include <iostream>
#include <vector>
#include <cstdint>
#include <iomanip>

        using namespace std;

    // CPU

    struct CPU
    {
        uint32_t reg[32] = {0}; // 32-bit registers
        uint32_t pc = 0;        // program counter (instruction index)
        vector<uint8_t> mem;    // byte-addressable memory
    };

    // Instr

    enum Opcode
    {
        ADD,
        SUB,
        ADDI,
        LW,
        SW,
        BEQ,
        BNE,
        JAL,
        NOP
    };

    struct Instruction
    {
        Opcode op;

        int rd;
        int rs1;
        int rs2;

        int32_t imm; // immediate (signed)
    };

    // Register names for nicer printing
    const char *regNames[32] = {
        "x0(zero)", "x1(ra)", "x2(sp)", "x3(gp)", "x4(tp)", "x5(t0)", "x6(t1)", "x7(t2)",
        "x8(s0)", "x9(s1)", "x10(a0)", "x11(a1)", "x12(a2)", "x13(a3)", "x14(a4)", "x15(a5)",
        "x16(a6)", "x17(a7)", "x18(s2)", "x19(s3)", "x20(s4)", "x21(s5)", "x22(s6)", "x23(s7)",
        "x24(s8)", "x25(s9)", "x26(s10)", "x27(s11)", "x28(t3)", "x29(t4)", "x30(t5)", "x31(t6)"};

    // Memory helpers (little-endian)
    uint32_t read32(const CPU &cpu, int address)
    {
        if (address < 0 || address + 3 >= (int)cpu.mem.size())
        {
            cerr << "read32: address out of range: " << address << endl;
            return 0;
        }
        uint32_t b0 = cpu.mem[address];
        uint32_t b1 = cpu.mem[address + 1];
        uint32_t b2 = cpu.mem[address + 2];
        uint32_t b3 = cpu.mem[address + 3];
        return (b0) | (b1 << 8) | (b2 << 16) | (b3 << 24);
    }

    void write32(CPU & cpu, int address, uint32_t value)
    {
        if (address < 0 || address + 3 >= (int)cpu.mem.size())
        {
            cerr << "write32: address out of range: " << address << endl;
            return;
        }
        cpu.mem[address] = value & 0xff;
        cpu.mem[address + 1] = (value >> 8) & 0xff;
        cpu.mem[address + 2] = (value >> 16) & 0xff;
        cpu.mem[address + 3] = (value >> 24) & 0xff;
    }

    // Printing utilities
    void printInstruction(const CPU &cpu, const Instruction &inst)
    {
        cout << "PC=" << cpu.pc << "  ";
        switch (inst.op)
        {
        case ADD:
            cout << "ADD " << regNames[inst.rd] << ", " << regNames[inst.rs1] << ", " << regNames[inst.rs2];
            break;
        case SUB:
            cout << "SUB " << regNames[inst.rd] << ", " << regNames[inst.rs1] << ", " << regNames[inst.rs2];
            break;
        case ADDI:
            cout << "ADDI " << regNames[inst.rd] << ", " << regNames[inst.rs1] << ", " << inst.imm;
            break;
        case LW:
            cout << "LW " << regNames[inst.rd] << ", " << inst.imm << "(" << regNames[inst.rs1] << ")";
            break;
        case SW:
            cout << "SW " << regNames[inst.rs2] << ", " << inst.imm << "(" << regNames[inst.rs1] << ")";
            break;
        case BEQ:
            cout << "BEQ " << regNames[inst.rs1] << ", " << regNames[inst.rs2] << ", offset=" << inst.imm;
            break;
        case BNE:
            cout << "BNE " << regNames[inst.rs1] << ", " << regNames[inst.rs2] << ", offset=" << inst.imm;
            break;
        case JAL:
            cout << "JAL " << regNames[inst.rd] << ", offset=" << inst.imm;
            break;
        case NOP:
            cout << "NOP";
            break;
        }
        cout << endl;
    }

    void printRegisters(const CPU &cpu)
    {
        cout << "Registers:\n";
        for (int i = 0; i < 32; ++i)
        {
            cout << left << setw(10) << regNames[i] << ": 0x" << hex << setw(8) << setfill('0') << cpu.reg[i] << dec << setfill(' ') << "\n";
        }
    }

    void printMemory(const CPU &cpu, int words = 8)
    {
        cout << "Memory (first " << words << " words):\n";
        for (int i = 0; i < words; ++i)
        {
            int addr = i * 4;
            if (addr + 3 >= (int)cpu.mem.size())
                break;
            cout << "mem[" << i << "] (0x" << hex << setw(3) << setfill('0') << addr << dec << setfill(' ') << ") = " << read32(cpu, addr) << "\n";
        }
    }

    // Fetch / Decode / Execute cycle 
    Instruction fetch(const CPU &cpu, const vector<Instruction> &program)
    {
        return program[cpu.pc];
    }

    Instruction decode(const Instruction &inst)
    {
        // identity decoder for now; placeholder for future binary decode
        return inst;
    }
    //all instructions be executed in this block
    void execute(CPU & cpu, const Instruction &inst)
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
            cpu.reg[inst.rd] = cpu.reg[inst.rs1] + (uint32_t)inst.imm;
            cpu.pc++;
            break;

        case LW:
        {
            int address = (int)(cpu.reg[inst.rs1] + (uint32_t)inst.imm);
            if (address < 0 || address % 4 != 0)
            {
                cerr << "LW: misaligned or invalid address " << address << endl;
                cpu.pc++;
                break;
            }
            cpu.reg[inst.rd] = read32(cpu, address);
            cpu.pc++;
            break;
        }

        case SW:
        {
            int address = (int)(cpu.reg[inst.rs1] + (uint32_t)inst.imm);
            if (address < 0 || address % 4 != 0)
            {
                cerr << "SW: misaligned or invalid address " << address << endl;
                cpu.pc++;
                break;
            }
            write32(cpu, address, cpu.reg[inst.rs2]);
            cpu.pc++;
            break;
        }

        case BEQ:
            if (cpu.reg[inst.rs1] == cpu.reg[inst.rs2])
            {
                cpu.pc = (uint32_t)((int32_t)cpu.pc + inst.imm);
            }
            else
            {
                cpu.pc++;
            }
            break;

        case BNE:
            if (cpu.reg[inst.rs1] != cpu.reg[inst.rs2])
            {
                cpu.pc = (uint32_t)((int32_t)cpu.pc + inst.imm);
            }
            else
            {
                cpu.pc++;
            }
            break;

        case JAL:
            cpu.reg[inst.rd] = cpu.pc + 1; // return address (instruction index)
            cpu.pc = (uint32_t)((int32_t)cpu.pc + inst.imm);
            break;

        case NOP:
            cpu.pc++;
            break;
        }

        cpu.reg[0] = 0; // enforce x0 == 0
    }

    void run(CPU & cpu, const vector<Instruction> &program, int maxSteps = 1000)
    {
        int steps = 0;
        while (cpu.pc < program.size())
        {
            if (++steps > maxSteps) //set upper limit
            {
                cerr << "run: reached max steps (possible infinite loop)" << endl;
                break;
            }

            Instruction inst = fetch(cpu, program);
            Instruction dec = decode(inst);

            cout << "----------------------------------------\n";
            printInstruction(cpu, dec);

            execute(cpu, dec);

            printRegisters(cpu);
            printMemory(cpu, 4);
            cout << "----------------------------------------\n\n";
        }
    }

    int main()
    {
        CPU cpu;
        cpu.mem.resize(256); // 256 bytes = 64 words

        vector<Instruction> program =
            {
                {ADDI, 1, 0, 0, 10}, // x1 = 10
                {ADDI, 2, 0, 0, 20}, // x2 = 20
                {SW, 0, 0, 1, 0},    // store x1 at mem[0]
                {SW, 0, 0, 2, 4},    // store x2 at mem[1]
                {LW, 3, 0, 0, 0},    // x3 = mem[0]
                {LW, 4, 0, 0, 4},    // x4 = mem[1]
                {ADD, 5, 3, 4, 0},   // x5 = x3 + x4
                {SUB, 6, 5, 1, 0},   // x6 = x5 - x1
                {ADDI, 7, 0, 0, 3},  // x7 = 3 (loop counter)
                {BEQ, 0, 7, 0, 3},   // if x7==0 jump forward to end (PC+3)
                {ADDI, 7, 7, 0, -1}, // x7 = x7 - 1
                {JAL, 8, 0, 0, -2},  // jump back two instructions to BEQ
                {BNE, 0, 1, 2, 1},   // if x1!=x2 skip 1 (which it will, 10 != 20)
                {NOP, 0, 0, 0, 0},   // skipped
                {NOP, 0, 0, 0, 0}    // end
            };

        run(cpu, program, 200);

        cout << "Final registers and memory:\n";
        printRegisters(cpu);
        printMemory(cpu, 4);

        return 0;
    }

//instrutions hardcoded for now in main, user based input to be implemented soon//
