#include "cpu.h"
#include "memory.h"
#include "platform.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <chrono>

void execute(CPU &cpu, const Instruction &inst)
{
    switch (inst.op)
    {
    case ADD:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] + cpu.reg[inst.rs2];
        cpu.pc += 4;
        break;
    case SUB:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] - cpu.reg[inst.rs2];
        cpu.pc += 4;
        break;
    case ADDI:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] + (u32)inst.imm;
        cpu.pc += 4;
        break;

    case LB: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        cpu.reg[inst.rd] = (u32)(i32)(i8)read8(cpu, address);
        cpu.pc += 4;
        break;
    }
    case LH: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        if (address % 2 != 0) std::cerr << "LH: misaligned address " << address << std::endl;
        else cpu.reg[inst.rd] = (u32)(i32)(i16)read16(cpu, address);
        cpu.pc += 4;
        break;
    }
    case LW: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        if (address % 4 != 0) std::cerr << "LW: misaligned address " << address << std::endl;
        else cpu.reg[inst.rd] = read32(cpu, address);
        cpu.pc += 4;
        break;
    }
    case LBU: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        cpu.reg[inst.rd] = (u32)read8(cpu, address);
        cpu.pc += 4;
        break;
    }
    case LHU: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        if (address % 2 != 0) std::cerr << "LHU: misaligned address " << address << std::endl;
        else cpu.reg[inst.rd] = (u32)read16(cpu, address);
        cpu.pc += 4;
        break;
    }

    case SB: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        write8(cpu, address, (u8)(cpu.reg[inst.rs2] & 0xFF));
        cpu.pc += 4;
        break;
    }
    case SH: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        if (address % 2 != 0) std::cerr << "SH: misaligned address " << address << std::endl;
        else write16(cpu, address, (u16)(cpu.reg[inst.rs2] & 0xFFFF));
        cpu.pc += 4;
        break;
    }
    case SW: {
        int address = (int)(cpu.reg[inst.rs1] + (u32)inst.imm);
        if (address % 4 != 0) std::cerr << "SW: misaligned address " << address << std::endl;
        else write32(cpu, address, cpu.reg[inst.rs2]);
        cpu.pc += 4;
        break;
    }

    case BEQ:
        cpu.pc = (cpu.reg[inst.rs1] == cpu.reg[inst.rs2]) ? cpu.pc + (u32)inst.imm : cpu.pc + 4;
        break;
    case BNE:
        cpu.pc = (cpu.reg[inst.rs1] != cpu.reg[inst.rs2]) ? cpu.pc + (u32)inst.imm : cpu.pc + 4;
        break;
    case BLT:
        cpu.pc = ((i32)cpu.reg[inst.rs1] < (i32)cpu.reg[inst.rs2]) ? cpu.pc + (u32)inst.imm : cpu.pc + 4;
        break;
    case BGE:
        cpu.pc = ((i32)cpu.reg[inst.rs1] >= (i32)cpu.reg[inst.rs2]) ? cpu.pc + (u32)inst.imm : cpu.pc + 4;
        break;
    case BLTU:
        cpu.pc = (cpu.reg[inst.rs1] < cpu.reg[inst.rs2]) ? cpu.pc + (u32)inst.imm : cpu.pc + 4;
        break;
    case BGEU:
        cpu.pc = (cpu.reg[inst.rs1] >= cpu.reg[inst.rs2]) ? cpu.pc + (u32)inst.imm : cpu.pc + 4;
        break;

    case JAL:
        cpu.reg[inst.rd] = cpu.pc + 4;
        cpu.pc = cpu.pc + (u32)inst.imm;
        break;

    case NOP:
    case FENCE:
        cpu.pc += 4;
        break;

    case ECALL:
    {
        // Linux RISC-V ABI syscall handling:
        // Syscall number in reg[17] (a7)
        // Args in reg[10-15] (a0-a5)
        // Return value in reg[10] (a0)

        // Host-side file descriptor table: indices 0-2 are reserved for
        // stdin/stdout/stderr (not stored here; handled directly).
        // Slots 3-15 are available for guest openat calls.
        static FILE* fdTable[16] = {nullptr};

        switch (cpu.reg[17]) {

        // ------------------------------------------------------------------
        // sys_openat(dirfd, path_addr, flags, mode) [56] or sys_open [1024]
        // Linux 56:   a0=dirfd, a1=path_addr, a2=flags, a3=mode
        // Newlib 1024: a0=path_addr, a1=flags, a2=mode
        // ------------------------------------------------------------------
        case 56:
        case 1024: {
            u32 pathAddr = (cpu.reg[17] == 1024) ? cpu.reg[10] : cpu.reg[11];
            u32 flags    = (cpu.reg[17] == 1024) ? cpu.reg[11] : cpu.reg[12];

            // Extract path string from guest memory
            char path[512] = {0};
            for (int i = 0; i < 511; ++i) {
                if (pathAddr + (u32)i >= (u32)cpu.mem.size()) break;
                char c = (char)cpu.mem[pathAddr + i];
                path[i] = c;
                if (c == '\0') break;
            }

            // O_WRONLY=1, O_RDWR=2, O_CREAT=0x40, O_TRUNC=0x200, O_APPEND=0x400
            const char* mode = "rb";
            bool write_flag = (flags & 1) || (flags & 2);
            bool create_flag = (flags & 0x40);
            bool append_flag = (flags & 0x400);
            bool trunc_flag  = (flags & 0x200);
            if (write_flag && create_flag && trunc_flag)  mode = "wb";
            else if (write_flag && create_flag && append_flag) mode = "ab";
            else if (write_flag && (flags & 2))           mode = "r+b";
            else if (write_flag)                          mode = "wb";

            // Find a free slot (skip 0-2 = stdio)
            int fd = -1;
            for (int i = 3; i < 16; ++i) {
                if (!fdTable[i]) { fd = i; break; }
            }
            if (fd == -1) { cpu.reg[10] = (u32)-1; break; }

            FILE* f = fopen(path, mode);
            if (!f) {
                cpu.reg[10] = (u32)-1;
                break;
            }
            fdTable[fd] = f;
            cpu.reg[10] = (u32)fd;
            break;
        }

        // ------------------------------------------------------------------
        // sys_close(fd) [57 or 1026]
        // ------------------------------------------------------------------
        case 57:
        case 1026: {
            u32 fd = cpu.reg[10];
            if (fd >= 3 && fd < 16 && fdTable[fd]) {
                fclose(fdTable[fd]);
                fdTable[fd] = nullptr;
            }
            cpu.reg[10] = 0;
            break;
        }

        // ------------------------------------------------------------------
        // sys_lseek(fd, offset, whence) [62 or 1029]
        // a0=fd, a1=offset, a2=whence
        // ------------------------------------------------------------------
        case 62:
        case 1029: {
            u32 fd     = cpu.reg[10];
            i32 offset = (i32)cpu.reg[11];
            u32 whence = cpu.reg[12];
            if (fd >= 3 && fd < 16 && fdTable[fd]) {
                int w = (whence == 0) ? SEEK_SET : (whence == 1) ? SEEK_CUR : SEEK_END;
                fseek(fdTable[fd], offset, w);
                cpu.reg[10] = (u32)ftell(fdTable[fd]);
            } else {
                cpu.reg[10] = (u32)-1;
            }
            break;
        }

        // ------------------------------------------------------------------
        // sys_read(fd, buf_addr, count) [63 or 1030]
        // a0=fd, a1=buf_addr, a2=count
        // ------------------------------------------------------------------
        case 63:
        case 1030: {
            u32 fd      = cpu.reg[10];
            u32 bufAddr = cpu.reg[11];
            u32 count   = cpu.reg[12];

            if (bufAddr + count > (u32)cpu.mem.size()) {
                cpu.reg[10] = (u32)-1;
                break;
            }

            if (fd == 0) {
                // stdin
                size_t n = fread(&cpu.mem[bufAddr], 1, count, stdin);
                cpu.reg[10] = (u32)n;
            } else if (fd >= 3 && fd < 16 && fdTable[fd]) {
                size_t n = fread(&cpu.mem[bufAddr], 1, count, fdTable[fd]);
                cpu.reg[10] = (u32)n;
            } else {
                cpu.reg[10] = (u32)-1;
            }
            break;
        }

        // ------------------------------------------------------------------
        // sys_write(fd, buf_addr, count) [64 or 1031]
        // a0=fd, a1=buf_addr, a2=count
        // ------------------------------------------------------------------
        case 64:
        case 1031: {
            u32 fd      = cpu.reg[10];
            u32 bufAddr = cpu.reg[11];
            u32 count   = cpu.reg[12];
            if (bufAddr + count <= (u32)cpu.mem.size()) {
                FILE* outF = (fd == 2) ? stderr : (fd >= 3 && fd < 16 && fdTable[fd]) ? fdTable[fd] : stdout;
                fwrite(&cpu.mem[bufAddr], 1, count, outF);
                fflush(outF);
                cpu.reg[10] = count;
            } else {
                cpu.reg[10] = (u32)-1;
            }
            break;
        }

        // ------------------------------------------------------------------
        // sys_fstat(fd, stat_addr) [80 or 1028]
        // ------------------------------------------------------------------
        case 80:
        case 1028: {
            u32 fd       = cpu.reg[10];
            u32 statAddr = cpu.reg[11];

            if (statAddr + 128 > (u32)cpu.mem.size()) {
                cpu.reg[10] = (u32)-1;
                break;
            }

            // Zero out the stat region
            std::memset(&cpu.mem[statAddr], 0, 128);

            long fileSize = 0;
            if (fd >= 3 && fd < 16 && fdTable[fd]) {
                long cur = ftell(fdTable[fd]);
                fseek(fdTable[fd], 0, SEEK_END);
                fileSize = ftell(fdTable[fd]);
                fseek(fdTable[fd], cur, SEEK_SET);
            }

            // Write st_size at offset 48 as a 64-bit little-endian value
            u32 sizeLo = (u32)(fileSize & 0xFFFFFFFF);
            u32 sizeHi = (u32)(((u64)fileSize >> 32) & 0xFFFFFFFF);
            write32(cpu, (int)(statAddr + 48), sizeLo);
            write32(cpu, (int)(statAddr + 52), sizeHi);

            cpu.reg[10] = 0;
            break;
        }

        // ------------------------------------------------------------------
        // sys_exit(code) — set halted flag instead of calling host exit()
        // ------------------------------------------------------------------
        case 93:
            std::cerr << "sys_exit(" << cpu.reg[10] << ") at pc=0x"
                      << std::hex << cpu.pc << std::dec << "\n";
            cpu.halted = true;
            break;

        // ------------------------------------------------------------------
        // sys_uname(utsname_addr)
        // Writes a minimal utsname struct (each field is 65 bytes on Linux).
        // ------------------------------------------------------------------
        case 160: {
            u32 addr = cpu.reg[10];
            if (addr + 65 * 6 > (u32)cpu.mem.size()) {
                cpu.reg[10] = (u32)-1;
                break;
            }
            std::memset(&cpu.mem[addr], 0, 65 * 6);
            // sysname
            std::memcpy(&cpu.mem[addr + 65 * 0], "Linux",    5);
            // nodename
            std::memcpy(&cpu.mem[addr + 65 * 1], "riscv-emu", 9);
            // release
            std::memcpy(&cpu.mem[addr + 65 * 2], "5.15.0",   6);
            // version
            std::memcpy(&cpu.mem[addr + 65 * 3], "#1",       2);
            // machine
            std::memcpy(&cpu.mem[addr + 65 * 4], "riscv32",  7);
            cpu.reg[10] = 0;
            break;
        }

        // ------------------------------------------------------------------
        // sys_gettimeofday(timeval_addr, timezone_addr)
        // timeval: { tv_sec (4 bytes), tv_usec (4 bytes) }
        // ------------------------------------------------------------------
        case 169: {
            u32 tvAddr = cpu.reg[10];
            if (tvAddr + 8 <= (u32)cpu.mem.size()) {
                auto now = std::chrono::system_clock::now();
                auto duration = now.time_since_epoch();
                auto sec = std::chrono::duration_cast<std::chrono::seconds>(duration);
                auto usec = std::chrono::duration_cast<std::chrono::microseconds>(duration - sec);
                write32(cpu, (int)tvAddr,     (u32)sec.count());
                write32(cpu, (int)tvAddr + 4, (u32)usec.count());
            }
            cpu.reg[10] = 0;
            break;
        }

        // ------------------------------------------------------------------
        // sys_brk(addr) — use cpu.heapBase instead of hardcoded value
        // ------------------------------------------------------------------
        case 214: {
            static u32 currentBrk = 0;
            // Initialise lazily from cpu.heapBase on first call
            if (currentBrk == 0) currentBrk = cpu.heapBase ? cpu.heapBase : 0x800000u;
            if (cpu.reg[10] != 0 && cpu.reg[10] >= cpu.heapBase) {
                currentBrk = cpu.reg[10];
            }
            cpu.reg[10] = currentBrk;
            break;
        }

        // ------------------------------------------------------------------
        // sys_mmap — unchanged from original
        // ------------------------------------------------------------------
        case 222: {
            static u32 mmapBase = 0x2000000;
            u32 len = cpu.reg[11];
            cpu.reg[10] = mmapBase;
            mmapBase += (len + 0xFFF) & ~0xFFFu;
            break;
        }

        // ------------------------------------------------------------------
        // Custom DOOM ECALLs (0x100–0x103)
        // ------------------------------------------------------------------

        // ECALL_DRAW_FRAME: reg[10]=framebuffer_addr, reg[11]=w, reg[12]=h
        case 0x100: {
            u32 fbAddr = cpu.reg[10];
            int w      = (int)cpu.reg[11];
            int h      = (int)cpu.reg[12];
            u32 bytes  = (u32)(w * h * 4);
            if (fbAddr + bytes <= (u32)cpu.mem.size()) {
                hostDrawFrame((uint32_t*)&cpu.mem[fbAddr], w, h);
            }
            cpu.reg[10] = 0;
            break;
        }

        // ECALL_GET_KEY: returns packed int (high byte=pressed, low byte=key)
        case 0x101: {
            cpu.reg[10] = (u32)hostPollKey();
            break;
        }

        // ECALL_GET_TICKS: milliseconds since start
        case 0x102: {
            cpu.reg[10] = hostGetTicks();
            break;
        }

        // ECALL_SLEEP: reg[10]=ms
        case 0x103: {
            hostSleep(cpu.reg[10]);
            cpu.reg[10] = 0;
            break;
        }

        default:
            std::cerr << "Unhandled ECALL syscall: " << cpu.reg[17]
                      << " at pc=" << cpu.pc << std::endl;
            break;
        }
        cpu.pc += 4;
        break;
    }

    case EBREAK:
        std::cout << "EBREAK at pc=" << cpu.pc << std::endl;
        cpu.pc += 4;
        break;

    // M-extension instructions
    case MUL:
        cpu.reg[inst.rd] = (u32)((i64)(i32)cpu.reg[inst.rs1] * (i64)(i32)cpu.reg[inst.rs2]);
        cpu.pc += 4;
        break;
    case MULH:
        cpu.reg[inst.rd] = (u32)(((i64)(i32)cpu.reg[inst.rs1] * (i64)(i32)cpu.reg[inst.rs2]) >> 32);
        cpu.pc += 4;
        break;
    case MULHSU:
        cpu.reg[inst.rd] = (u32)(((i64)(i32)cpu.reg[inst.rs1] * (u64)cpu.reg[inst.rs2]) >> 32);
        cpu.pc += 4;
        break;
    case MULHU:
        cpu.reg[inst.rd] = (u32)(((u64)cpu.reg[inst.rs1] * (u64)cpu.reg[inst.rs2]) >> 32);
        cpu.pc += 4;
        break;
    case DIV: {
        i32 dividend = (i32)cpu.reg[inst.rs1];
        i32 divisor  = (i32)cpu.reg[inst.rs2];
        if (divisor == 0) {
            cpu.reg[inst.rd] = 0xFFFFFFFF;
        } else if (dividend == (i32)0x80000000 && divisor == -1) {
            cpu.reg[inst.rd] = (u32)0x80000000;
        } else {
            cpu.reg[inst.rd] = (u32)(dividend / divisor);
        }
        cpu.pc += 4;
        break;
    }
    case DIVU: {
        u32 dividend = cpu.reg[inst.rs1];
        u32 divisor  = cpu.reg[inst.rs2];
        if (divisor == 0) {
            cpu.reg[inst.rd] = 0xFFFFFFFF;
        } else {
            cpu.reg[inst.rd] = dividend / divisor;
        }
        cpu.pc += 4;
        break;
    }
    case REM: {
        i32 dividend = (i32)cpu.reg[inst.rs1];
        i32 divisor  = (i32)cpu.reg[inst.rs2];
        if (divisor == 0) {
            cpu.reg[inst.rd] = (u32)dividend;
        } else if (dividend == (i32)0x80000000 && divisor == -1) {
            cpu.reg[inst.rd] = 0;
        } else {
            cpu.reg[inst.rd] = (u32)(dividend % divisor);
        }
        cpu.pc += 4;
        break;
    }
    case REMU: {
        u32 dividend = cpu.reg[inst.rs1];
        u32 divisor  = cpu.reg[inst.rs2];
        if (divisor == 0) {
            cpu.reg[inst.rd] = dividend;
        } else {
            cpu.reg[inst.rd] = dividend % divisor;
        }
        cpu.pc += 4;
        break;
    }

    // LUI loads the upper-20-bit immediate directly into rd (lower 12 bits are zero)
    case LUI:
        cpu.reg[inst.rd] = (u32)inst.imm;
        cpu.pc += 4;
        break;
    // AUIPC adds the upper-20-bit immediate to pc
    case AUIPC:
        cpu.reg[inst.rd] = cpu.pc + (u32)inst.imm;
        cpu.pc += 4;
        break;
    case JALR:
        cpu.reg[inst.rd] = cpu.pc + 4;
        cpu.pc = (cpu.reg[inst.rs1] + (u32)inst.imm) & ~1u;
        break;

    case SLTI:
        cpu.reg[inst.rd] = ((i32)cpu.reg[inst.rs1] < inst.imm) ? 1 : 0;
        cpu.pc += 4;
        break;
    case SLTIU:
        cpu.reg[inst.rd] = (cpu.reg[inst.rs1] < (u32)inst.imm) ? 1 : 0;
        cpu.pc += 4;
        break;
    case XORI:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] ^ (u32)inst.imm;
        cpu.pc += 4;
        break;
    case ORI:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] | (u32)inst.imm;
        cpu.pc += 4;
        break;
    case ANDI:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] & (u32)inst.imm;
        cpu.pc += 4;
        break;
    case SLLI:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] << (inst.imm & 0x1F);
        cpu.pc += 4;
        break;
    case SRLI:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] >> (inst.imm & 0x1F);
        cpu.pc += 4;
        break;
    case SRAI:
        cpu.reg[inst.rd] = (u32)((i32)cpu.reg[inst.rs1] >> (inst.imm & 0x1F));
        cpu.pc += 4;
        break;

    case SLT:
        cpu.reg[inst.rd] = ((i32)cpu.reg[inst.rs1] < (i32)cpu.reg[inst.rs2]) ? 1 : 0;
        cpu.pc += 4;
        break;
    case SLTU:
        cpu.reg[inst.rd] = (cpu.reg[inst.rs1] < cpu.reg[inst.rs2]) ? 1 : 0;
        cpu.pc += 4;
        break;
    case SLL:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] << (cpu.reg[inst.rs2] & 0x1F);
        cpu.pc += 4;
        break;
    case SRL:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] >> (cpu.reg[inst.rs2] & 0x1F);
        cpu.pc += 4;
        break;
    case SRA:
        cpu.reg[inst.rd] = (u32)((i32)cpu.reg[inst.rs1] >> (cpu.reg[inst.rs2] & 0x1F));
        cpu.pc += 4;
        break;
    case XOR:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] ^ cpu.reg[inst.rs2];
        cpu.pc += 4;
        break;
    case OR:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] | cpu.reg[inst.rs2];
        cpu.pc += 4;
        break;
    case AND:
        cpu.reg[inst.rd] = cpu.reg[inst.rs1] & cpu.reg[inst.rs2];
        cpu.pc += 4;
        break;
    }

    cpu.reg[0] = 0;
}

//note: the instruciton logic is largely inspired from https://msyksphinz-self.github.io/riscv-isadoc/ ! Do check it out!!!
