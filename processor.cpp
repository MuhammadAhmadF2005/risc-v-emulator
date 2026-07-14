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

// Instructions stores as enum for easier understanding

enum Opcode
{
    // R-type ALU
    ADD,
    SUB,
    SLL,
    SLT,
    SLTU,
    XOR,
    SRL,
    SRA,
    OR,
    AND,
    // I-type ALU
    ADDI,
    SLTI,
    SLTIU,
    XORI,
    ORI,
    ANDI,
    SLLI,
    SRLI,
    SRAI,
    // U-type
    LUI,
    AUIPC,
    // Loads
    LW,
    // Stores
    SW,
    // Branches
    BEQ,
    BNE,
    // Jumps
    JAL,
    JALR,
    // Misc
    NOP
};

// Opcode to string name for printing
const char *opName(Opcode op)
{
    switch (op)
    {
    case ADD:
        return "ADD";
    case SUB:
        return "SUB";
    case SLL:
        return "SLL";
    case SLT:
        return "SLT";
    case SLTU:
        return "SLTU";
    case XOR:
        return "XOR";
    case SRL:
        return "SRL";
    case SRA:
        return "SRA";
    case OR:
        return "OR";
    case AND:
        return "AND";
    case ADDI:
        return "ADDI";
    case SLTI:
        return "SLTI";
    case SLTIU:
        return "SLTIU";
    case XORI:
        return "XORI";
    case ORI:
        return "ORI";
    case ANDI:
        return "ANDI";
    case SLLI:
        return "SLLI";
    case SRLI:
        return "SRLI";
    case SRAI:
        return "SRAI";
    case LUI:
        return "LUI";
    case AUIPC:
        return "AUIPC";
    case LW:
        return "LW";
    case SW:
        return "SW";
    case BEQ:
        return "BEQ";
    case BNE:
        return "BNE";
    case JAL:
        return "JAL";
    case JALR:
        return "JALR";
    case NOP:
        return "NOP";
    default:
        return "???";
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

void write32(CPU &cpu, int address, uint32_t value)
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
    cout << "PC=" << cpu.pc << "  " << opName(inst.op);
    switch (inst.op)
    {
    // R-type: op rd, rs1, rs2
    case ADD:
    case SUB:
    case SLL:
    case SLT:
    case SLTU:
    case XOR:
    case SRL:
    case SRA:
    case OR:
    case AND:
        cout << " " << regNames[inst.rd] << ", " << regNames[inst.rs1] << ", " << regNames[inst.rs2];
        break;
    // I-type ALU: op rd, rs1, imm
    case ADDI:
    case SLTI:
    case SLTIU:
    case XORI:
    case ORI:
    case ANDI:
    case SLLI:
    case SRLI:
    case SRAI:
        cout << " " << regNames[inst.rd] << ", " << regNames[inst.rs1] << ", " << inst.imm;
        break;
    // U-type: op rd, imm
    case LUI:
    case AUIPC:
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
    case BEQ:
    case BNE:
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

void printRegisters(const CPU &cpu)
{
    cout << "Registers:\n";
    for (int i = 0; i < 32; ++i)
        cout << "  " << regNames[i] << " = " << cpu.reg[i] << "\n";
}

void printMemory(const CPU &cpu, int words = 8)
{
    cout << "Memory (first " << words << " words):\n";
    for (int i = 0; i < words; ++i)
    {
        int addr = i * 4;
        if (addr + 3 >= (int)cpu.mem.size())
            break;
        cout << "  mem[" << addr << "] = " << read32(cpu, addr) << "\n";
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

// all instructions be executed in this block
void execute(CPU &cpu, const Instruction &inst)
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

    // LUI simply loads a 20bit immediate into the top 20 bits of rd while
    //  remaining lower 12 bits are zero
    case LUI:
        cpu.reg[inst.rd] = (uint32_t)inst.imm << 12;
        cpu.pc++;
        break;

    // AUIPC adds the 20bit immediate (shifted) to pc
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

    // note to reader: SRAI preserves sign bit unlike SRLI
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

void run(CPU &cpu, const vector<Instruction> &program, int maxSteps = 1000, bool verbose = true)
{
    int steps = 0;
    while (cpu.pc < program.size())
    {
        if (++steps > maxSteps) // set upper limit
        {
            cerr << "run: reached max steps (possible infinite loop)" << endl;
            break;
        }

        Instruction inst = fetch(cpu, program);
        Instruction dec = decode(inst);

        if (verbose)
        {
            cout << "---\n";
            printInstruction(cpu, dec);
        }

        execute(cpu, dec);

        if (verbose)
        {
            printRegisters(cpu);
            printMemory(cpu, 4);
            cout << endl;
        }
    }
}

int main()
{
    cout << "=== RISC-V RV32I Emulator - Test Cases ===" << endl;
    cout << "28 instructions implemented" << endl << endl;

    int passed = 0;
    int failed = 0;

    // helper to print a test result
    // we just inline the check each time so its clear whats happening

    CPU cpu;
    cpu.mem.resize(256); // 256 bytes = 64 words

    // ---- R-type ALU tests ----
    cout << "-- R-type ALU --" << endl;

    // ADD: x1 = 10, x2 = 20, x3 = x1 + x2 should be 30
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,10},{ADDI,2,0,0,20},{ADD,3,1,2,0}}, 100, false);
    if (cpu.reg[3] == 30) { cout << "PASS ADD" << endl; passed++; }
    else { cout << "FAIL ADD  got " << cpu.reg[3] << " expected 30" << endl; failed++; }

    // SUB: x1=20, x2=10, x3 = x1 - x2 = 10
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,20},{ADDI,2,0,0,10},{SUB,3,1,2,0}}, 100, false);
    if (cpu.reg[3] == 10) { cout << "PASS SUB" << endl; passed++; }
    else { cout << "FAIL SUB  got " << cpu.reg[3] << " expected 10" << endl; failed++; }

    // SLL: x1=1, x2=4, x3 = 1 << 4 = 16
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,1},{ADDI,2,0,0,4},{SLL,3,1,2,0}}, 100, false);
    if (cpu.reg[3] == 16) { cout << "PASS SLL" << endl; passed++; }
    else { cout << "FAIL SLL  got " << cpu.reg[3] << " expected 16" << endl; failed++; }

    // SLT signed: -1 < 1 = true (result = 1)
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,-1},{ADDI,2,0,0,1},{SLT,3,1,2,0}}, 100, false);
    if (cpu.reg[3] == 1) { cout << "PASS SLT (signed -1 < 1 = true)" << endl; passed++; }
    else { cout << "FAIL SLT  got " << cpu.reg[3] << " expected 1" << endl; failed++; }

    // SLT signed: 5 < 3 = false (result = 0)
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,5},{ADDI,2,0,0,3},{SLT,3,1,2,0}}, 100, false);
    if (cpu.reg[3] == 0) { cout << "PASS SLT (signed 5 < 3 = false)" << endl; passed++; }
    else { cout << "FAIL SLT  got " << cpu.reg[3] << " expected 0" << endl; failed++; }

    // SLTU unsigned: 1 < 0xFFFFFFFF = true
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,1},{ADDI,2,0,0,-1},{SLTU,3,1,2,0}}, 100, false);
    if (cpu.reg[3] == 1) { cout << "PASS SLTU" << endl; passed++; }
    else { cout << "FAIL SLTU  got " << cpu.reg[3] << " expected 1" << endl; failed++; }

    // XOR: 0xFF ^ 0x0F = 0xF0
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,0xFF},{ADDI,2,0,0,0x0F},{XOR,3,1,2,0}}, 100, false);
    if (cpu.reg[3] == 0xF0) { cout << "PASS XOR" << endl; passed++; }
    else { cout << "FAIL XOR  got " << cpu.reg[3] << " expected 240" << endl; failed++; }

    // SRL logical: 0x80 >> 4 = 0x08
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,0x80},{ADDI,2,0,0,4},{SRL,3,1,2,0}}, 100, false);
    if (cpu.reg[3] == 0x08) { cout << "PASS SRL" << endl; passed++; }
    else { cout << "FAIL SRL  got " << cpu.reg[3] << " expected 8" << endl; failed++; }

    // SRA arithmetic: -128 >> 2 = -32 (sign preserved)
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,-128},{ADDI,2,0,0,2},{SRA,3,1,2,0}}, 100, false);
    if ((int32_t)cpu.reg[3] == -32) { cout << "PASS SRA" << endl; passed++; }
    else { cout << "FAIL SRA  got " << (int32_t)cpu.reg[3] << " expected -32" << endl; failed++; }

    // OR: 0xF0 | 0x0F = 0xFF
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,0xF0},{ADDI,2,0,0,0x0F},{OR,3,1,2,0}}, 100, false);
    if (cpu.reg[3] == 0xFF) { cout << "PASS OR" << endl; passed++; }
    else { cout << "FAIL OR  got " << cpu.reg[3] << " expected 255" << endl; failed++; }

    // AND: 0xFF & 0x0F = 0x0F
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,0xFF},{ADDI,2,0,0,0x0F},{AND,3,1,2,0}}, 100, false);
    if (cpu.reg[3] == 0x0F) { cout << "PASS AND" << endl; passed++; }
    else { cout << "FAIL AND  got " << cpu.reg[3] << " expected 15" << endl; failed++; }

    cout << endl;

    // ---- I-type ALU tests ----
    cout << "-- I-type ALU --" << endl;

    // ADDI: x1 = 0 + 42 = 42
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,42}}, 100, false);
    if (cpu.reg[1] == 42) { cout << "PASS ADDI" << endl; passed++; }
    else { cout << "FAIL ADDI  got " << cpu.reg[1] << " expected 42" << endl; failed++; }

    // ADDI negative: x1 = 10, x2 = x1 + (-3) = 7
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,10},{ADDI,2,1,0,-3}}, 100, false);
    if (cpu.reg[2] == 7) { cout << "PASS ADDI (negative imm)" << endl; passed++; }
    else { cout << "FAIL ADDI  got " << cpu.reg[2] << " expected 7" << endl; failed++; }

    // SLTI signed: -5 < 1 = true
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,-5},{SLTI,2,1,0,1}}, 100, false);
    if (cpu.reg[2] == 1) { cout << "PASS SLTI (signed -5 < 1 = true)" << endl; passed++; }
    else { cout << "FAIL SLTI  got " << cpu.reg[2] << " expected 1" << endl; failed++; }

    // SLTI signed: 5 < 3 = false
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,5},{SLTI,2,1,0,3}}, 100, false);
    if (cpu.reg[2] == 0) { cout << "PASS SLTI (signed 5 < 3 = false)" << endl; passed++; }
    else { cout << "FAIL SLTI  got " << cpu.reg[2] << " expected 0" << endl; failed++; }

    // SLTIU unsigned: 1 < 2 = true
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,1},{SLTIU,2,1,0,2}}, 100, false);
    if (cpu.reg[2] == 1) { cout << "PASS SLTIU" << endl; passed++; }
    else { cout << "FAIL SLTIU  got " << cpu.reg[2] << " expected 1" << endl; failed++; }

    // XORI: 0xFF ^ 0x0F = 0xF0
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,0xFF},{XORI,2,1,0,0x0F}}, 100, false);
    if (cpu.reg[2] == 0xF0) { cout << "PASS XORI" << endl; passed++; }
    else { cout << "FAIL XORI  got " << cpu.reg[2] << " expected 240" << endl; failed++; }

    // ORI: 0xF0 | 0x0F = 0xFF
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,0xF0},{ORI,2,1,0,0x0F}}, 100, false);
    if (cpu.reg[2] == 0xFF) { cout << "PASS ORI" << endl; passed++; }
    else { cout << "FAIL ORI  got " << cpu.reg[2] << " expected 255" << endl; failed++; }

    // ANDI: 0xFF & 0x0F = 0x0F
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,0xFF},{ANDI,2,1,0,0x0F}}, 100, false);
    if (cpu.reg[2] == 0x0F) { cout << "PASS ANDI" << endl; passed++; }
    else { cout << "FAIL ANDI  got " << cpu.reg[2] << " expected 15" << endl; failed++; }

    // SLLI: 1 << 8 = 256
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,1},{SLLI,2,1,0,8}}, 100, false);
    if (cpu.reg[2] == 256) { cout << "PASS SLLI" << endl; passed++; }
    else { cout << "FAIL SLLI  got " << cpu.reg[2] << " expected 256" << endl; failed++; }

    // SRLI logical: 256 >> 4 = 16
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,256},{SRLI,2,1,0,4}}, 100, false);
    if (cpu.reg[2] == 16) { cout << "PASS SRLI" << endl; passed++; }
    else { cout << "FAIL SRLI  got " << cpu.reg[2] << " expected 16" << endl; failed++; }

    // SRAI arithmetic: -64 >> 2 = -16
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,-64},{SRAI,2,1,0,2}}, 100, false);
    if ((int32_t)cpu.reg[2] == -16) { cout << "PASS SRAI" << endl; passed++; }
    else { cout << "FAIL SRAI  got " << (int32_t)cpu.reg[2] << " expected -16" << endl; failed++; }

    cout << endl;

    // ---- U-type tests ----
    cout << "-- U-type --" << endl;

    // LUI: imm=1 loads 1 into top 20 bits -> 0x1000
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{LUI,1,0,0,1}}, 100, false);
    if (cpu.reg[1] == 0x1000) { cout << "PASS LUI" << endl; passed++; }
    else { cout << "FAIL LUI  got " << cpu.reg[1] << " expected 4096" << endl; failed++; }

    // AUIPC: pc=0, imm=2, result = 0 + (2 << 12) = 0x2000
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{AUIPC,1,0,0,2}}, 100, false);
    if (cpu.reg[1] == 0x2000) { cout << "PASS AUIPC" << endl; passed++; }
    else { cout << "FAIL AUIPC  got " << cpu.reg[1] << " expected 8192" << endl; failed++; }

    cout << endl;

    // ---- Load/Store tests ----
    cout << "-- Load / Store --" << endl;

    // SW then LW: store 42 at addr 0, load it back into x2
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,42},{SW,0,0,1,0},{LW,2,0,0,0}}, 100, false);
    if (cpu.reg[2] == 42) { cout << "PASS SW + LW" << endl; passed++; }
    else { cout << "FAIL SW+LW  got " << cpu.reg[2] << " expected 42" << endl; failed++; }

    // SW then LW with offset: store 99 at addr 8, load back
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,99},{ADDI,3,0,0,8},{SW,0,3,1,0},{LW,2,3,0,0}}, 100, false);
    if (cpu.reg[2] == 99) { cout << "PASS SW + LW (offset)" << endl; passed++; }
    else { cout << "FAIL SW+LW  got " << cpu.reg[2] << " expected 99" << endl; failed++; }

    cout << endl;

    // ---- Branch tests ----
    cout << "-- Branches --" << endl;

    // BEQ taken: x1==x2 so branch skips the ADDI 99, x3 should stay 0
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,5},{ADDI,2,0,0,5},{BEQ,0,1,2,2},{ADDI,3,0,0,99},{ADDI,4,0,0,77}}, 100, false);
    if (cpu.reg[3] == 0 && cpu.reg[4] == 77) { cout << "PASS BEQ (taken)" << endl; passed++; }
    else { cout << "FAIL BEQ taken  x3=" << cpu.reg[3] << " x4=" << cpu.reg[4] << endl; failed++; }

    // BEQ not taken: x1 != x2 so no branch, x3 gets 99
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,5},{ADDI,2,0,0,10},{BEQ,0,1,2,2},{ADDI,3,0,0,99}}, 100, false);
    if (cpu.reg[3] == 99) { cout << "PASS BEQ (not taken)" << endl; passed++; }
    else { cout << "FAIL BEQ not taken  got " << cpu.reg[3] << " expected 99" << endl; failed++; }

    // BNE taken: x1 != x2 so branch, x3 stays 0
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,5},{ADDI,2,0,0,10},{BNE,0,1,2,2},{ADDI,3,0,0,99},{ADDI,4,0,0,77}}, 100, false);
    if (cpu.reg[3] == 0 && cpu.reg[4] == 77) { cout << "PASS BNE (taken)" << endl; passed++; }
    else { cout << "FAIL BNE taken  x3=" << cpu.reg[3] << " x4=" << cpu.reg[4] << endl; failed++; }

    // BNE not taken: x1 == x2 so no branch, x3 gets 99
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,5},{ADDI,2,0,0,5},{BNE,0,1,2,2},{ADDI,3,0,0,99}}, 100, false);
    if (cpu.reg[3] == 99) { cout << "PASS BNE (not taken)" << endl; passed++; }
    else { cout << "FAIL BNE not taken  got " << cpu.reg[3] << " expected 99" << endl; failed++; }

    cout << endl;

    // ---- Jump tests ----
    cout << "-- Jumps --" << endl;

    // JAL: jump forward by 2, x1 saves return addr (pc+1=1), skips ADDI 99
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{JAL,1,0,0,2},{ADDI,2,0,0,99},{ADDI,3,0,0,77}}, 100, false);
    if (cpu.reg[1] == 1 && cpu.reg[2] == 0 && cpu.reg[3] == 77)
    { cout << "PASS JAL" << endl; passed++; }
    else { cout << "FAIL JAL  x1=" << cpu.reg[1] << " x2=" << cpu.reg[2] << " x3=" << cpu.reg[3] << endl; failed++; }

    // JALR: jump to rs1+imm, x5=3 so jumps to instruction 3
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,5,0,0,3},{JALR,1,5,0,0},{ADDI,2,0,0,99},{ADDI,3,0,0,77}}, 100, false);
    if (cpu.reg[1] == 2 && cpu.reg[2] == 0 && cpu.reg[3] == 77)
    { cout << "PASS JALR" << endl; passed++; }
    else { cout << "FAIL JALR  x1=" << cpu.reg[1] << " x2=" << cpu.reg[2] << " x3=" << cpu.reg[3] << endl; failed++; }

    cout << endl;

    // ---- Misc tests ----
    cout << "-- Misc --" << endl;

    // NOP: just advances pc, doesnt change registers
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,1,0,0,42},{NOP,0,0,0,0},{ADDI,2,0,0,77}}, 100, false);
    if (cpu.reg[1] == 42 && cpu.reg[2] == 77) { cout << "PASS NOP" << endl; passed++; }
    else { cout << "FAIL NOP  x1=" << cpu.reg[1] << " x2=" << cpu.reg[2] << endl; failed++; }

    // x0 hardwired to zero: even if you write to it, stays 0
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu, {{ADDI,0,0,0,42}}, 100, false);
    if (cpu.reg[0] == 0) { cout << "PASS x0 hardwired zero" << endl; passed++; }
    else { cout << "FAIL x0  got " << cpu.reg[0] << " expected 0" << endl; failed++; }

    cout << endl;

    // ---- Integration test: loop countdown ----
    cout << "-- Integration: loop countdown --" << endl;

    // counts x1 down from 3 to 0 using BEQ + JAL
    cpu = CPU(); cpu.mem.resize(256);
    run(cpu,
        {
            {ADDI, 1, 0, 0, 3},  // x1 = 3
            {BEQ, 0, 1, 0, 3},   // if x1 == 0 jump forward to NOP (exit)
            {ADDI, 1, 1, 0, -1}, // x1 = x1 - 1
            {JAL, 8, 0, 0, -2},  // jump back to BEQ
            {NOP, 0, 0, 0, 0}    // end
        },
        100, false);
    if (cpu.reg[1] == 0) { cout << "PASS loop countdown" << endl; passed++; }
    else { cout << "FAIL loop  x1=" << cpu.reg[1] << " expected 0" << endl; failed++; }

    cout << endl;

    // ---- Summary ----
    cout << "Results: " << passed << "/" << (passed + failed) << " passed";
    if (failed > 0) cout << "  (" << failed << " failed)";
    cout << endl;

    // ---- Demo: verbose run to show the step-by-step trace ----
    if (failed == 0)
    {
        cout << endl << "All tests passed! Here is a verbose run as demo:" << endl << endl;

        CPU demo_cpu;
        demo_cpu.mem.resize(256); // 256 bytes = 64 words

        vector<Instruction> demo =
        {
            {ADDI, 1, 0, 0, 10}, // x1 = 10
            {ADDI, 2, 0, 0, 20}, // x2 = 20
            {ADD,  3, 1, 2, 0},  // x3 = x1 + x2 = 30
            {SUB,  4, 2, 1, 0},  // x4 = x2 - x1 = 10
            {SW,   0, 0, 3, 0},  // store x3 at mem[0]
            {LW,   5, 0, 0, 0},  // x5 = mem[0] = 30
            {NOP,  0, 0, 0, 0}   // end

            //you may uncomment the example outputs to run em for yourself
        };

        run(demo_cpu, demo, 200);

        cout << "Final registers:" << endl;
        printRegisters(demo_cpu);
        cout << endl;
        printMemory(demo_cpu, 4);
    }

    return 0;
}

// instrutions hardcoded for now in main, user based input to be taken via terminal...//

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
