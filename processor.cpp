#include <iostream>
#include <vector>
#include <cstdint>
#include <iomanip>
#include <string>

using namespace std;

    // CPU

    struct CPU
    {
        uint32_t reg[32] = {0}; // 32-bit registers
        uint32_t pc = 0;        // program counter (instruction index)
        vector<uint8_t> mem;    // byte-addressable memory
    };

    // Instructions stores as enum for easier understanding

    enum Opcode
    {
        // R-type ALU
        ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,
        // I-type ALU
        ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI,
        // U-type
        LUI, AUIPC,
        // Loads
        LW,
        // Stores
        SW,
        // Branches
        BEQ, BNE,
        // Jumps
        JAL, JALR,
        // Misc
        NOP
    };

    // Opcode to string name for printing
    const char *opName(Opcode op)
    {
        switch (op)
        {
        case ADD:   return "ADD";
        case SUB:   return "SUB";
        case SLL:   return "SLL";
        case SLT:   return "SLT";
        case SLTU:  return "SLTU";
        case XOR:   return "XOR";
        case SRL:   return "SRL";
        case SRA:   return "SRA";
        case OR:    return "OR";
        case AND:   return "AND";
        case ADDI:  return "ADDI";
        case SLTI:  return "SLTI";
        case SLTIU: return "SLTIU";
        case XORI:  return "XORI";
        case ORI:   return "ORI";
        case ANDI:  return "ANDI";
        case SLLI:  return "SLLI";
        case SRLI:  return "SRLI";
        case SRAI:  return "SRAI";
        case LUI:   return "LUI";
        case AUIPC: return "AUIPC";
        case LW:    return "LW";
        case SW:    return "SW";
        case BEQ:   return "BEQ";
        case BNE:   return "BNE";
        case JAL:   return "JAL";
        case JALR:  return "JALR";
        case NOP:   return "NOP";
        default:    return "???";
        }
    }

    struct Instruction
    {
        Opcode op;

        int rd;
        int rs1;
        int rs2;

        int32_t imm; // immediate (signed)
    };

    // Register names for nicer printing ...
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
        cout << "  Executing:  " << opName(inst.op);
        switch (inst.op)
        {
        // R-type: op rd, rs1, rs2
        case ADD: case SUB: case SLL: case SLT: case SLTU:
        case XOR: case SRL: case SRA: case OR: case AND:
            cout << " " << regNames[inst.rd] << ", " << regNames[inst.rs1] << ", " << regNames[inst.rs2];
            break;
        // I-type ALU: op rd, rs1, imm
        case ADDI: case SLTI: case SLTIU: case XORI: case ORI: case ANDI:
        case SLLI: case SRLI: case SRAI:
            cout << " " << regNames[inst.rd] << ", " << regNames[inst.rs1] << ", " << inst.imm;
            break;
        // U-type: op rd, imm
        case LUI: case AUIPC:
            cout << " " << regNames[inst.rd] << ", " << inst.imm;
            break;
        // Load: op rd, imm(rs1)
        case LW:
            cout << " " << regNames[inst.rd] << ", " << inst.imm << "(" << regNames[inst.rs1] << ")";
            break;
        // Store: op rs2, imm(rs1)
        case SW:
            cout << " " << regNames[inst.rs2] << ", " << inst.imm << "(" << regNames[inst.rs1] << ")";
            break;
        // Branch: op rs1, rs2, offset
        case BEQ: case BNE:
            cout << " " << regNames[inst.rs1] << ", " << regNames[inst.rs2] << ", offset=" << inst.imm;
            break;
        // JAL: op rd, offset
        case JAL:
            cout << " " << regNames[inst.rd] << ", offset=" << inst.imm;
            break;
        // JALR: op rd, rs1, offset
        case JALR:
            cout << " " << regNames[inst.rd] << ", " << regNames[inst.rs1] << ", offset=" << inst.imm;
            break;
        case NOP:
            break;
        }
        cout << endl;
    }

    void printRegisters(const CPU &cpu, bool onlyNonZero = true)
    {
        cout << "  Registers" << (onlyNonZero ? " (non-zero)" : "") << ":" << endl;
        bool anyPrinted = false;
        for (int i = 0; i < 32; ++i)
        {
            if (onlyNonZero && cpu.reg[i] == 0) continue;
            cout << "    " << left << setw(10) << regNames[i]
                 << right << " = 0x" << hex << setw(8) << setfill('0') << cpu.reg[i]
                 << dec << setfill(' ') << "  (" << (int32_t)cpu.reg[i] << ")" << endl;
            anyPrinted = true;
        }
        if (!anyPrinted) cout << "    (all zero)" << endl;
    }

    void printMemory(const CPU &cpu, int words = 8)
    {
        cout << "  Memory (first " << words << " words):" << endl;
        bool anyPrinted = false;
        for (int i = 0; i < words; ++i)
        {
            int addr = i * 4;
            if (addr + 3 >= (int)cpu.mem.size())
                break;
            uint32_t val = read32(cpu, addr);
            if (val != 0)
            {
                cout << "    mem[" << addr << "] = " << val
                     << "  (0x" << hex << setw(8) << setfill('0') << val << dec << setfill(' ') << ")" << endl;
                anyPrinted = true;
            }
        }
        if (!anyPrinted) cout << "    (all zero)" << endl;
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
                cpu.pc = (uint32_t)((int32_t)cpu.pc + inst.imm);
            else
                cpu.pc++;
            break;

        case BNE:
            if (cpu.reg[inst.rs1] != cpu.reg[inst.rs2])
                cpu.pc = (uint32_t)((int32_t)cpu.pc + inst.imm);
            else
                cpu.pc++;
            break;

        case JAL:
            cpu.reg[inst.rd] = cpu.pc + 1; // return address (instruction index)
            cpu.pc = (uint32_t)((int32_t)cpu.pc + inst.imm);
            break;

        case NOP:
            cpu.pc++;
            break;

        // I will now try to implement all rv32I complete instructions...

        //LUI simply loads a 20bit immediate into the top 20 bits of rd while
        // remaining lower 12 bits are zero
        case LUI:
            cpu.reg[inst.rd] = (uint32_t)inst.imm << 12;
            cpu.pc++;
            break;

        //AUIPC adds the 20bit immediate (shifted) to pc
        case AUIPC:
            cpu.reg[inst.rd] = cpu.pc + ((uint32_t)inst.imm << 12);
            cpu.pc++;
            break;

        // JALR: jump to rs1 + imm, save return address in rd
        case JALR:
            cpu.reg[inst.rd] = cpu.pc + 1;
            cpu.pc = (uint32_t)((int32_t)cpu.reg[inst.rs1] + inst.imm);
            break;

        // SLTI: set rd=1 if rs1 < imm (SIGNED comparison)
        case SLTI:
            cpu.reg[inst.rd] = ((int32_t)cpu.reg[inst.rs1] < inst.imm) ? 1 : 0;
            cpu.pc++;
            break;

        // SLTIU: set rd=1 if rs1 < imm (UNSIGNED comparison)
        case SLTIU:
            cpu.reg[inst.rd] = (cpu.reg[inst.rs1] < (uint32_t)inst.imm) ? 1 : 0;
            cpu.pc++;
            break;

        case XORI:
            cpu.reg[inst.rd] = cpu.reg[inst.rs1] ^ (uint32_t)inst.imm;
            cpu.pc++;
            break;

        case ORI:
            cpu.reg[inst.rd] = cpu.reg[inst.rs1] | (uint32_t)inst.imm;
            cpu.pc++;
            break;

        case ANDI:
            cpu.reg[inst.rd] = cpu.reg[inst.rs1] & (uint32_t)inst.imm;
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

        //note to reader: SRAI preserves sign bit unlike SRLI
        case SRAI:
            cpu.reg[inst.rd] = (uint32_t)((int32_t)cpu.reg[inst.rs1] >> (inst.imm & 0x1F));
            cpu.pc++;
            break;

        // SLT: set rd=1 if rs1 < rs2 (SIGNED comparison)
        case SLT:
            cpu.reg[inst.rd] = ((int32_t)cpu.reg[inst.rs1] < (int32_t)cpu.reg[inst.rs2]) ? 1 : 0;
            cpu.pc++;
            break;

        // SLTU: set rd=1 if rs1 < rs2 (UNSIGNED comparison)
        case SLTU:
            cpu.reg[inst.rd] = (cpu.reg[inst.rs1] < cpu.reg[inst.rs2]) ? 1 : 0;
            cpu.pc++;
            break;

        // SLL: shift left by lower 5 bits of rs2
        case SLL:
            cpu.reg[inst.rd] = cpu.reg[inst.rs1] << (cpu.reg[inst.rs2] & 0x1F);
            cpu.pc++;
            break;

        // SRL: logical shift right by lower 5 bits of rs2
        case SRL:
            cpu.reg[inst.rd] = cpu.reg[inst.rs1] >> (cpu.reg[inst.rs2] & 0x1F);
            cpu.pc++;
            break;

        // SRA: arithmetic shift right by lower 5 bits of rs2 (preserves sign)
        case SRA:
            cpu.reg[inst.rd] = (uint32_t)((int32_t)cpu.reg[inst.rs1] >> (cpu.reg[inst.rs2] & 0x1F));
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

    void run(CPU & cpu, const vector<Instruction> &program, int maxSteps = 1000, bool verbose = true)
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

            if (verbose)
            {
                cout << "  +-- Step " << steps << " ----------------------------" << endl;
                cout << "  |  PC = " << cpu.pc << endl;
                printInstruction(cpu, dec);
            }

            execute(cpu, dec);

            if (verbose)
            {
                printRegisters(cpu);
                printMemory(cpu, 4);
                cout << "  +------------------------------------------" << endl << endl;
            }
        }
    }

    // ============ TEST FRAMEWORK ============

    struct TestResult
    {
        string name;
        bool passed;
    };

    // Run a test: load program, execute, check expected register values
    TestResult runTest(const string &name, const vector<Instruction> &program,
                       const vector<pair<int, uint32_t>> &expectedRegs,
                       int memSize = 256)
    {
        CPU cpu;
        cpu.mem.resize(memSize);

        // Run silently (verbose = false)
        run(cpu, program, 100, false);

        // Check expected values
        bool passed = true;
        for (const auto &expected : expectedRegs)
        {
            if (cpu.reg[expected.first] != expected.second)
            {
                cout << "  FAIL: " << name << endl;
                cout << "    " << regNames[expected.first] << " = " << cpu.reg[expected.first]
                     << " (expected " << expected.second << ")" << endl;
                passed = false;
            }
        }

        if (passed)
            cout << "  PASS: " << name << endl;

        return {name, passed};
    }

    // Overload for tests that also check memory
    TestResult runTestMem(const string &name, const vector<Instruction> &program,
                          const vector<pair<int, uint32_t>> &expectedRegs,
                          const vector<pair<int, uint32_t>> &expectedMem,
                          int memSize = 256)
    {
        CPU cpu;
        cpu.mem.resize(memSize);

        run(cpu, program, 100, false);

        bool passed = true;
        for (const auto &expected : expectedRegs)
        {
            if (cpu.reg[expected.first] != expected.second)
            {
                cout << "  FAIL: " << name << endl;
                cout << "    " << regNames[expected.first] << " = " << cpu.reg[expected.first]
                     << " (expected " << expected.second << ")" << endl;
                passed = false;
            }
        }
        for (const auto &expected : expectedMem)
        {
            uint32_t actual = read32(cpu, expected.first);
            if (actual != expected.second)
            {
                cout << "  FAIL: " << name << endl;
                cout << "    mem[" << expected.first << "] = " << actual
                     << " (expected " << expected.second << ")" << endl;
                passed = false;
            }
        }

        if (passed)
            cout << "  PASS: " << name << endl;

        return {name, passed};
    }

    int main()
    {
        cout << "========================================" << endl;
        cout << "   RISC-V RV32I Emulator Test Suite    " << endl;
        cout << "   28 Instructions Implemented         " << endl;
        cout << "========================================" << endl << endl;

        vector<TestResult> results;

        // ---- R-type ALU tests ----
        cout << "--- R-type ALU Instructions ---" << endl;

        results.push_back(runTest("ADD: 10 + 20 = 30",
            {{ADDI, 1, 0, 0, 10}, {ADDI, 2, 0, 0, 20}, {ADD, 3, 1, 2, 0}},
            {{3, 30}}));

        results.push_back(runTest("SUB: 20 - 10 = 10",
            {{ADDI, 1, 0, 0, 20}, {ADDI, 2, 0, 0, 10}, {SUB, 3, 1, 2, 0}},
            {{3, 10}}));

        results.push_back(runTest("SLL: 1 << 4 = 16",
            {{ADDI, 1, 0, 0, 1}, {ADDI, 2, 0, 0, 4}, {SLL, 3, 1, 2, 0}},
            {{3, 16}}));

        results.push_back(runTest("SLT: signed -1 < 1 = true",
            {{ADDI, 1, 0, 0, -1}, {ADDI, 2, 0, 0, 1}, {SLT, 3, 1, 2, 0}},
            {{3, 1}}));

        results.push_back(runTest("SLT: signed 5 < 3 = false",
            {{ADDI, 1, 0, 0, 5}, {ADDI, 2, 0, 0, 3}, {SLT, 3, 1, 2, 0}},
            {{3, 0}}));

        results.push_back(runTest("SLTU: unsigned 1 < 0xFFFFFFFF = true",
            {{ADDI, 1, 0, 0, 1}, {ADDI, 2, 0, 0, -1}, {SLTU, 3, 1, 2, 0}},
            {{3, 1}}));

        results.push_back(runTest("XOR: 0xFF ^ 0x0F = 0xF0",
            {{ADDI, 1, 0, 0, 0xFF}, {ADDI, 2, 0, 0, 0x0F}, {XOR, 3, 1, 2, 0}},
            {{3, 0xF0}}));

        results.push_back(runTest("SRL: 0x80 >> 4 = 0x08 (logical)",
            {{ADDI, 1, 0, 0, 0x80}, {ADDI, 2, 0, 0, 4}, {SRL, 3, 1, 2, 0}},
            {{3, 0x08}}));

        results.push_back(runTest("SRA: -128 >> 2 = -32 (arithmetic, preserves sign)",
            {{ADDI, 1, 0, 0, -128}, {ADDI, 2, 0, 0, 2}, {SRA, 3, 1, 2, 0}},
            {{3, (uint32_t)-32}}));

        results.push_back(runTest("OR: 0xF0 | 0x0F = 0xFF",
            {{ADDI, 1, 0, 0, 0xF0}, {ADDI, 2, 0, 0, 0x0F}, {OR, 3, 1, 2, 0}},
            {{3, 0xFF}}));

        results.push_back(runTest("AND: 0xFF & 0x0F = 0x0F",
            {{ADDI, 1, 0, 0, 0xFF}, {ADDI, 2, 0, 0, 0x0F}, {AND, 3, 1, 2, 0}},
            {{3, 0x0F}}));

        cout << endl;

        // ---- I-type ALU tests ----
        cout << "--- I-type ALU Instructions ---" << endl;

        results.push_back(runTest("ADDI: 0 + 42 = 42",
            {{ADDI, 1, 0, 0, 42}},
            {{1, 42}}));

        results.push_back(runTest("ADDI: 10 + (-3) = 7",
            {{ADDI, 1, 0, 0, 10}, {ADDI, 2, 1, 0, -3}},
            {{2, 7}}));

        results.push_back(runTest("SLTI: signed -5 < 1 = true",
            {{ADDI, 1, 0, 0, -5}, {SLTI, 2, 1, 0, 1}},
            {{2, 1}}));

        results.push_back(runTest("SLTI: signed 5 < 3 = false",
            {{ADDI, 1, 0, 0, 5}, {SLTI, 2, 1, 0, 3}},
            {{2, 0}}));

        results.push_back(runTest("SLTIU: unsigned 1 < 2 = true",
            {{ADDI, 1, 0, 0, 1}, {SLTIU, 2, 1, 0, 2}},
            {{2, 1}}));

        results.push_back(runTest("XORI: 0xFF ^ 0x0F = 0xF0",
            {{ADDI, 1, 0, 0, 0xFF}, {XORI, 2, 1, 0, 0x0F}},
            {{2, 0xF0}}));

        results.push_back(runTest("ORI: 0xF0 | 0x0F = 0xFF",
            {{ADDI, 1, 0, 0, 0xF0}, {ORI, 2, 1, 0, 0x0F}},
            {{2, 0xFF}}));

        results.push_back(runTest("ANDI: 0xFF & 0x0F = 0x0F",
            {{ADDI, 1, 0, 0, 0xFF}, {ANDI, 2, 1, 0, 0x0F}},
            {{2, 0x0F}}));

        results.push_back(runTest("SLLI: 1 << 8 = 256",
            {{ADDI, 1, 0, 0, 1}, {SLLI, 2, 1, 0, 8}},
            {{2, 256}}));

        results.push_back(runTest("SRLI: 256 >> 4 = 16 (logical)",
            {{ADDI, 1, 0, 0, 256}, {SRLI, 2, 1, 0, 4}},
            {{2, 16}}));

        results.push_back(runTest("SRAI: -64 >> 2 = -16 (arithmetic)",
            {{ADDI, 1, 0, 0, -64}, {SRAI, 2, 1, 0, 2}},
            {{2, (uint32_t)-16}}));

        cout << endl;

        // ---- U-type tests ----
        cout << "--- U-type Instructions ---" << endl;

        results.push_back(runTest("LUI: load upper imm 1 -> 0x1000",
            {{LUI, 1, 0, 0, 1}},
            {{1, 0x1000}}));

        results.push_back(runTest("LUI: load upper imm 0xABCDE -> 0xABCDE000",
            {{LUI, 1, 0, 0, (int32_t)0xABCDE}},
            {{1, (uint32_t)0xABCDE << 12}}));

        results.push_back(runTest("AUIPC: pc + (imm << 12), pc=0, imm=2 -> 0x2000",
            {{AUIPC, 1, 0, 0, 2}},
            {{1, 0x2000}}));

        cout << endl;

        // ---- Load/Store tests ----
        cout << "--- Load/Store Instructions ---" << endl;

        results.push_back(runTestMem("SW+LW: store 42 at addr 0, load back",
            {{ADDI, 1, 0, 0, 42}, {SW, 0, 0, 1, 0}, {LW, 2, 0, 0, 0}},
            {{2, 42}},
            {{0, 42}}));

        results.push_back(runTestMem("SW+LW: store 99 at addr 8 via offset",
            {{ADDI, 1, 0, 0, 99}, {ADDI, 3, 0, 0, 8}, {SW, 0, 3, 1, 0}, {LW, 2, 3, 0, 0}},
            {{2, 99}},
            {{8, 99}}));

        cout << endl;

        // ---- Branch tests ----
        cout << "--- Branch Instructions ---" << endl;

        results.push_back(runTest("BEQ: equal -> branch taken (skip ADDI 99)",
            {{ADDI, 1, 0, 0, 5}, {ADDI, 2, 0, 0, 5}, {BEQ, 0, 1, 2, 2}, {ADDI, 3, 0, 0, 99}, {ADDI, 4, 0, 0, 77}},
            {{3, 0}, {4, 77}}));

        results.push_back(runTest("BEQ: unequal -> branch not taken",
            {{ADDI, 1, 0, 0, 5}, {ADDI, 2, 0, 0, 10}, {BEQ, 0, 1, 2, 2}, {ADDI, 3, 0, 0, 99}},
            {{3, 99}}));

        results.push_back(runTest("BNE: unequal -> branch taken",
            {{ADDI, 1, 0, 0, 5}, {ADDI, 2, 0, 0, 10}, {BNE, 0, 1, 2, 2}, {ADDI, 3, 0, 0, 99}, {ADDI, 4, 0, 0, 77}},
            {{3, 0}, {4, 77}}));

        results.push_back(runTest("BNE: equal -> branch not taken",
            {{ADDI, 1, 0, 0, 5}, {ADDI, 2, 0, 0, 5}, {BNE, 0, 1, 2, 2}, {ADDI, 3, 0, 0, 99}},
            {{3, 99}}));

        cout << endl;

        // ---- Jump tests ----
        cout << "--- Jump Instructions ---" << endl;

        results.push_back(runTest("JAL: jump forward 2, saves return addr",
            {{JAL, 1, 0, 0, 2}, {ADDI, 2, 0, 0, 99}, {ADDI, 3, 0, 0, 77}},
            {{1, 1}, {2, 0}, {3, 77}}));

        results.push_back(runTest("JALR: jump to rs1+imm, saves return addr",
            {{ADDI, 5, 0, 0, 3}, {JALR, 1, 5, 0, 0}, {ADDI, 2, 0, 0, 99}, {ADDI, 3, 0, 0, 77}},
            {{1, 2}, {2, 0}, {3, 77}}));

        cout << endl;

        // ---- Misc tests ----
        cout << "--- Miscellaneous ---" << endl;

        results.push_back(runTest("NOP: does nothing, pc advances",
            {{ADDI, 1, 0, 0, 42}, {NOP, 0, 0, 0, 0}, {ADDI, 2, 0, 0, 77}},
            {{1, 42}, {2, 77}}));

        results.push_back(runTest("x0 hardwired: writing to x0 stays 0",
            {{ADDI, 0, 0, 0, 42}},
            {{0, 0}}));

        cout << endl;

        // ---- Integration test: loop countdown ----
        cout << "--- Integration Test ---" << endl;

        results.push_back(runTest("Loop: count down from 3 to 0",
            {
                {ADDI, 1, 0, 0, 3},   // x1 = 3
                {BEQ, 0, 1, 0, 3},    // if x1 == 0, jump to pc+3 (exit)
                {ADDI, 1, 1, 0, -1},  // x1 = x1 - 1
                {JAL, 8, 0, 0, -2},   // jump back to BEQ
                {NOP, 0, 0, 0, 0}     // end
            },
            {{1, 0}}));

        cout << endl;

        // ---- Summary ----
        int passed = 0, failed = 0;
        for (const auto &r : results)
        {
            if (r.passed) passed++;
            else failed++;
        }

        cout << "========================================" << endl;
        cout << "   Test Results: " << passed << "/" << (passed + failed) << " PASSED";
        if (failed > 0)
            cout << "  (" << failed << " FAILED)";
        cout << endl;
        cout << "========================================" << endl;

        // ---- Demo: run the integration test verbosely ----
        if (passed == (int)results.size())
        {
            cout << endl;
            cout << "All tests passed! Running verbose demo..." << endl << endl;

            CPU cpu;
            cpu.mem.resize(256); // 256 bytes = 64 words

            vector<Instruction> demo =
            {
                {ADDI, 1, 0, 0, 10}, // x1 = 10
                {ADDI, 2, 0, 0, 20}, // x2 = 20
                {ADD, 3, 1, 2, 0},   // x3 = x1 + x2 = 30
                {SUB, 4, 2, 1, 0},   // x4 = x2 - x1 = 10
                {SW, 0, 0, 3, 0},    // store x3 at mem[0]
                {LW, 5, 0, 0, 0},   // x5 = mem[0] = 30
                {NOP, 0, 0, 0, 0}    // end

                //you may uncomment the example outputs to run em for yourself
            };

            run(cpu, demo, 200);

            cout << "Final state:" << endl;
            printRegisters(cpu, true);
            cout << endl;
            printMemory(cpu, 4);
        }

        return 0;
    }

//instrutions hardcoded for now in main, user based input to be taken via terminal...//

/*
Note: for mem access we use little endian ordering where reading is done from 4 bytes of memory and converted ro 32 bits 
for eg the 32-bit hexadecimal number 0x12345678 in memory would be stored as 0x78, 0x56, 0x34, 0x12 where 2 hex bits represent 8 binary
bits ie one byte!
and writing vice versa..
*/

/*
note to reader: we use SRLI and SRAI instructions for right shifting as 
in our RISCV-32-simulator but there is a difference between them: 
SRLI is logical right shift and SRAI is arithmetic right shift what this means for us is
in SRLI the most significant bit is filled with 0 while shifting but in SRAI the most significant
bit is filled with the sign bit (ie the most significant bit of the number being shifted)
this is done in order to preserve the sign of the number being shifted. 
*/
