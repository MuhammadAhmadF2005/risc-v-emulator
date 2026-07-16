#include <iostream>
#include <vector>
#include <cstdint>

using namespace std;

// CPU
struct CPU
{
    uint32_t reg[32] = {0}; // 32-bit registers
    uint32_t pc = 0;        // program counter (instruction index)
    vector<uint8_t> mem;    // byte-addressable memory
};

// Instructions stored as enum for encoding rather than using opcode
enum Opcode
{
    // R-type ALU instructions
    ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,
    // I-type ALU
    ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI,
    // U-type
    LUI, AUIPC,
    // Loads
    LB, LH, LW, LBU, LHU,
    // Stores
    SB, SH, SW,
    // Branches
    BEQ, BNE, BLT, BGE, BLTU, BGEU,
    // Jumps
    JAL, JALR,
    // System / Misc
    FENCE, ECALL, EBREAK, NOP
};

// Directly map enum to string (quicker lookup)
const char* opNames[] = {
    "ADD", "SUB", "SLL", "SLT", "SLTU", "XOR", "SRL", "SRA", "OR", "AND",
    "ADDI", "SLTI", "SLTIU", "XORI", "ORI", "ANDI", "SLLI", "SRLI", "SRAI",
    "LUI", "AUIPC",
    "LB", "LH", "LW", "LBU", "LHU",
    "SB", "SH", "SW",
    "BEQ", "BNE", "BLT", "BGE", "BLTU", "BGEU",
    "JAL", "JALR",
    "FENCE", "ECALL", "EBREAK", "NOP"
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
    "x24(s8)", "x25(s9)", "x26(s10)", "x27(s11)", "x28(t3)", "x29(t4)", "x30(t5)", "x31(t6)"
};

// --- Memory Helpers ---
// Consolidate bounds checking to avoid copy-pasting it
bool checkMem(const CPU &cpu, int address, int bytes)
{
    if (address < 0 || address + bytes > (int)cpu.mem.size()) {
        cerr << "Memory out of bounds at: " << address << endl;
        return false;
    }
    return true;
}

uint8_t read8(const CPU &cpu, int addr) {
    return checkMem(cpu, addr, 1) ? cpu.mem[addr] : 0;
}

uint16_t read16(const CPU &cpu, int addr) {
    return checkMem(cpu, addr, 2) ? (cpu.mem[addr] | (cpu.mem[addr + 1] << 8)) : 0;
}

uint32_t read32(const CPU &cpu, int addr) {
    if (!checkMem(cpu, addr, 4)) return 0;
    return cpu.mem[addr] | (cpu.mem[addr+1] << 8) | (cpu.mem[addr+2] << 16) | (cpu.mem[addr+3] << 24);
}

void write8(CPU &cpu, int addr, uint8_t val) {
    if (checkMem(cpu, addr, 1)) cpu.mem[addr] = val;
}

void write16(CPU &cpu, int addr, uint16_t val) {
    if (checkMem(cpu, addr, 2)) {
        cpu.mem[addr] = val & 0xFF;
        cpu.mem[addr + 1] = (val >> 8) & 0xFF;
    }
}

void write32(CPU &cpu, int addr, uint32_t val) {
    if (checkMem(cpu, addr, 4)) {
        cpu.mem[addr] = val & 0xFF;
        cpu.mem[addr + 1] = (val >> 8) & 0xFF;
        cpu.mem[addr + 2] = (val >> 16) & 0xFF;
        cpu.mem[addr + 3] = (val >> 24) & 0xFF;
    }
}

// --- Printing utilities ---
void printInstruction(const CPU &cpu, const Instruction &inst)
{
    cout << "PC=" << cpu.pc << "  " << opNames[inst.op];
    
    // Grouping by instruction format type massively shrinks this switch
    switch (inst.op) {
        // R-type
        case ADD: case SUB: case SLL: case SLT: case SLTU: case XOR: case SRL: case SRA: case OR: case AND:
            cout << " " << regNames[inst.rd] << ", " << regNames[inst.rs1] << ", " << regNames[inst.rs2]; break;
        // I-type ALU
        case ADDI: case SLTI: case SLTIU: case XORI: case ORI: case ANDI: case SLLI: case SRLI: case SRAI:
            cout << " " << regNames[inst.rd] << ", " << regNames[inst.rs1] << ", " << inst.imm; break;
        // U-type
        case LUI: case AUIPC:
            cout << " " << regNames[inst.rd] << ", " << inst.imm; break;
        // Loads
        case LB: case LH: case LW: case LBU: case LHU:
            cout << " " << regNames[inst.rd] << ", " << inst.imm << "(" << regNames[inst.rs1] << ")"; break;
        // Stores
        case SB: case SH: case SW:
            cout << " " << regNames[inst.rs2] << ", " << inst.imm << "(" << regNames[inst.rs1] << ")"; break;
        // Branches
        case BEQ: case BNE: case BLT: case BGE: case BLTU: case BGEU:
            cout << " " << regNames[inst.rs1] << ", " << regNames[inst.rs2] << ", offset=" << inst.imm; break;
        // Jumps
        case JAL:
            cout << " " << regNames[inst.rd] << ", offset=" << inst.imm; break;
        case JALR:
            cout << " " << regNames[inst.rd] << ", " << regNames[inst.rs1] << ", offset=" << inst.imm; break;
        // Misc
        case FENCE: case ECALL: case EBREAK: case NOP:
            break;
    }
    cout << "\n";
}

void printRegisters(const CPU &cpu)
{
    cout << "Registers:\n";
    for (int i = 0; i < 32; ++i) {
        if (cpu.reg[i] != 0) 
            cout << "  " << regNames[i] << " = " << cpu.reg[i] << "\n";
    }
}

void printMemory(const CPU &cpu, int words = 8)
{
    cout << "Memory (first " << words << " words):\n";
    for (int i = 0; i < words; ++i) {
        int addr = i * 4;
        if (addr + 3 >= (int)cpu.mem.size()) break;
        uint32_t val = read32(cpu, addr);
        if (val != 0) // Only print non-zero memory
            cout << "  mem[" << addr << "] = " << val << "\n";
    }
}

// --- Fetch / Decode / Execute cycle ---
Instruction fetch(const CPU &cpu, const vector<Instruction> &program) {
    return program[cpu.pc];
}

Instruction decode(const Instruction &inst) {
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

    // LB: load byte, sign-extend to 32 bits
    case LB: {
        int address = (int)(cpu.reg[inst.rs1] + (uint32_t)inst.imm);
        cpu.reg[inst.rd] = (uint32_t)(int32_t)(int8_t)read8(cpu, address);
        cpu.pc++;
        break;
    }
    // LH: load halfword, sign-extend to 32 bits (must be 2-byte aligned)
    case LH: {
        int address = (int)(cpu.reg[inst.rs1] + (uint32_t)inst.imm);
        if (address % 2 != 0) cerr << "LH: misaligned address " << address << endl;
        else cpu.reg[inst.rd] = (uint32_t)(int32_t)(int16_t)read16(cpu, address);
        cpu.pc++;
        break;
    }
    case LW: {
        int address = (int)(cpu.reg[inst.rs1] + (uint32_t)inst.imm);
        if (address % 4 != 0) cerr << "LW: misaligned address " << address << endl;
        else cpu.reg[inst.rd] = read32(cpu, address);
        cpu.pc++;
        break;
    }
    // LBU: load byte, zero-extend (no sign extension)
    case LBU: {
        int address = (int)(cpu.reg[inst.rs1] + (uint32_t)inst.imm);
        cpu.reg[inst.rd] = (uint32_t)read8(cpu, address);
        cpu.pc++;
        break;
    }
    // LHU: load halfword, zero-extend (must be 2-byte aligned)
    case LHU: {
        int address = (int)(cpu.reg[inst.rs1] + (uint32_t)inst.imm);
        if (address % 2 != 0) cerr << "LHU: misaligned address " << address << endl;
        else cpu.reg[inst.rd] = (uint32_t)read16(cpu, address);
        cpu.pc++;
        break;
    }

    // SB: store the lowest byte of rs2
    case SB: {
        int address = (int)(cpu.reg[inst.rs1] + (uint32_t)inst.imm);
        write8(cpu, address, (uint8_t)(cpu.reg[inst.rs2] & 0xFF));
        cpu.pc++;
        break;
    }
    // SH: store the lowest halfword of rs2 (must be 2-byte aligned)
    case SH: {
        int address = (int)(cpu.reg[inst.rs1] + (uint32_t)inst.imm);
        if (address % 2 != 0) cerr << "SH: misaligned address " << address << endl;
        else write16(cpu, address, (uint16_t)(cpu.reg[inst.rs2] & 0xFFFF));
        cpu.pc++;
        break;
    }
    case SW: {
        int address = (int)(cpu.reg[inst.rs1] + (uint32_t)inst.imm);
        if (address % 4 != 0) cerr << "SW: misaligned address " << address << endl;
        else write32(cpu, address, cpu.reg[inst.rs2]);
        cpu.pc++;
        break;
    }

    case BEQ:
        cpu.pc = (cpu.reg[inst.rs1] == cpu.reg[inst.rs2]) ? (uint32_t)((int32_t)cpu.pc + inst.imm) : cpu.pc + 1;
        break;
    case BNE:
        cpu.pc = (cpu.reg[inst.rs1] != cpu.reg[inst.rs2]) ? (uint32_t)((int32_t)cpu.pc + inst.imm) : cpu.pc + 1;
        break;
    // BLT: branch if rs1 < rs2 (SIGNED)
    case BLT:
        cpu.pc = ((int32_t)cpu.reg[inst.rs1] < (int32_t)cpu.reg[inst.rs2]) ? (uint32_t)((int32_t)cpu.pc + inst.imm) : cpu.pc + 1;
        break;
    // BGE: branch if rs1 >= rs2 (SIGNED)
    case BGE:
        cpu.pc = ((int32_t)cpu.reg[inst.rs1] >= (int32_t)cpu.reg[inst.rs2]) ? (uint32_t)((int32_t)cpu.pc + inst.imm) : cpu.pc + 1;
        break;
    // BLTU: branch if rs1 < rs2 (UNSIGNED)
    case BLTU:
        cpu.pc = (cpu.reg[inst.rs1] < cpu.reg[inst.rs2]) ? (uint32_t)((int32_t)cpu.pc + inst.imm) : cpu.pc + 1;
        break;
    // BGEU: branch if rs1 >= rs2 (UNSIGNED)
    case BGEU:
        cpu.pc = (cpu.reg[inst.rs1] >= cpu.reg[inst.rs2]) ? (uint32_t)((int32_t)cpu.pc + inst.imm) : cpu.pc + 1;
        break;

    case JAL:
        cpu.reg[inst.rd] = cpu.pc + 1; // return address
        cpu.pc = (uint32_t)((int32_t)cpu.pc + inst.imm);
        break;

    case NOP: FENCE:
        cpu.pc++;
        break;

    // ECALL / EBREAK (stubs)
    case ECALL:
        cout << "ECALL at pc=" << cpu.pc << endl;
        cpu.pc++;
        break;
    case EBREAK:
        cout << "EBREAK at pc=" << cpu.pc << endl;
        cpu.pc++;
        break;

    // LUI simply loads a 20bit immediate into the top 20 bits of rd
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

    case SLTI:
        cpu.reg[inst.rd] = ((int32_t)cpu.reg[inst.rs1] < inst.imm) ? 1 : 0;
        cpu.pc++;
        break;
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

    case SLT:
        cpu.reg[inst.rd] = ((int32_t)cpu.reg[inst.rs1] < (int32_t)cpu.reg[inst.rs2]) ? 1 : 0;
        cpu.pc++;
        break;
    case SLTU:
        cpu.reg[inst.rd] = (cpu.reg[inst.rs1] < cpu.reg[inst.rs2]) ? 1 : 0;
        cpu.pc++;
        break;
    case SLL:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] << (cpu.reg[inst.rs2] & 0x1F);
        cpu.pc++;
        break;
    case SRL:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] >> (cpu.reg[inst.rs2] & 0x1F);
        cpu.pc++;
        break;
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

//note: the instruciton logic is largely inspired from https://msyksphinz-self.github.io/riscv-isadoc/ ! Do check it out!!!

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