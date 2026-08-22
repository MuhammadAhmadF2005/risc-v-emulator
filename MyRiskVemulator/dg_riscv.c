/*
 * dg_riscv.c — doomgeneric platform layer for the RISC-V emulator.
 *
 * Implements all 6 required doomgeneric callbacks by firing custom ECALLs
 * that are caught by the host emulator in cpu.cpp.
 *
 * Custom ECALL numbers:
 *   0x100  DRAW_FRAME  — blit DG_ScreenBuffer to the host window
 *   0x101  GET_KEY     — poll one key event (packed: high=pressed, low=keycode)
 *   0x102  GET_TICKS   — host millisecond counter
 *   0x103  SLEEP       — host sleep
 *
 * Compile with:
 *   riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 -O2 -c dg_riscv.c
 */

#include "doomgeneric/doomgeneric.h"   /* DG_ScreenBuffer, DOOMGENERIC_RESX/Y */
#include "doomgeneric/doomkeys.h"      /* DG_KEY_* constants */

#define ECALL_DRAW_FRAME  0x100
#define ECALL_GET_KEY     0x101
#define ECALL_GET_TICKS   0x102
#define ECALL_SLEEP       0x103

/* -----------------------------------------------------------------------
 * ecall helper macros — inline assembly for each arity.
 * RISC-V ABI: a7 = syscall number, a0..a5 = args, a0 = return value.
 * ----------------------------------------------------------------------- */

/* Syscall with no arguments, returns a0 */
static inline long __ecall0(long nr)
{
    register long a7 asm("a7") = nr;
    register long a0 asm("a0");
    asm volatile ("ecall" : "=r"(a0) : "r"(a7) : "memory");
    return a0;
}

/* Syscall with one argument */
static inline long __ecall1(long nr, long arg0)
{
    register long a7 asm("a7") = nr;
    register long a0 asm("a0") = arg0;
    asm volatile ("ecall" : "+r"(a0) : "r"(a7) : "memory");
    return a0;
}

/* Syscall with three arguments */
static inline long __ecall3(long nr, long arg0, long arg1, long arg2)
{
    register long a7 asm("a7") = nr;
    register long a0 asm("a0") = arg0;
    register long a1 asm("a1") = arg1;
    register long a2 asm("a2") = arg2;
    asm volatile ("ecall" : "+r"(a0) : "r"(a7), "r"(a1), "r"(a2) : "memory");
    return a0;
}

/* -----------------------------------------------------------------------
 * DG_Init — nothing to initialise on the guest side; the host SDL window
 * is created before the ELF entry point is reached.
 * ----------------------------------------------------------------------- */
void DG_Init(void)
{
    /* no-op */
}

/* -----------------------------------------------------------------------
 * DG_DrawFrame — send the 320×200 framebuffer to the host.
 * ----------------------------------------------------------------------- */
void DG_DrawFrame(void)
{
    __ecall3(ECALL_DRAW_FRAME,
             (long)DG_ScreenBuffer,
             DOOMGENERIC_RESX,
             DOOMGENERIC_RESY);
}

/* -----------------------------------------------------------------------
 * DG_GetKey — read one pending key event.
 * Returns 1 if an event was available, 0 otherwise.
 * *pressed: 1 = key down, 0 = key up
 * *key:     DOOM key constant
 * ----------------------------------------------------------------------- */
int DG_GetKey(int *pressed, unsigned char *key)
{
    long packed = __ecall0(ECALL_GET_KEY);
    if (packed == 0)
        return 0;

    *pressed = (int)((packed >> 8) & 0xFF);
    *key     = (unsigned char)(packed & 0xFF);
    return 1;
}

/* -----------------------------------------------------------------------
 * DG_GetTicksMs — milliseconds since program start.
 * ----------------------------------------------------------------------- */
uint32_t DG_GetTicksMs(void)
{
    return (uint32_t)__ecall0(ECALL_GET_TICKS);
}

/* -----------------------------------------------------------------------
 * DG_SleepMs — yield the CPU for 'ms' milliseconds.
 * ----------------------------------------------------------------------- */
void DG_SleepMs(uint32_t ms)
{
    __ecall1(ECALL_SLEEP, (long)ms);
}

/* -----------------------------------------------------------------------
 * DG_SetWindowTitle — stub; the host window title is set at init time.
 * ----------------------------------------------------------------------- */
void DG_SetWindowTitle(const char *title)
{
    (void)title;
}

/* -----------------------------------------------------------------------
 * mkdir — stub needed by m_misc.c in freestanding environment.
 * ----------------------------------------------------------------------- */
int mkdir(const char *pathname, unsigned int mode)
{
    (void)pathname;
    (void)mode;
    return 0;
}

/* -----------------------------------------------------------------------
 * main — guest entry point called by crt0.S.
 * ----------------------------------------------------------------------- */
int main(int argc, char **argv)
{
    doomgeneric_Create(argc, argv);

    while (1)
    {
        doomgeneric_Tick();
    }

    return 0;
}


