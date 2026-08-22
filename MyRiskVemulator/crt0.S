# crt0.S — minimal RISC-V startup for the freestanding DOOM binary.
#
# Responsibilities:
#   1. sp is already set by the emulator (main.cpp writes cpu.reg[2])
#      so we only need to ensure a properly aligned frame here.
#   2. Load argc and argv from the stack (Linux ABI: sp -> argc, sp+4 -> argv).
#   3. Call main(argc, argv).
#   4. Pass the return value to sys_exit (RISC-V Linux syscall 93).
#
# Assembled with:
#   riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 -c crt0.S

    .section .text.start
    .globl  _start
    .type   _start, @function

_start:
    # -----------------------------------------------------------------------
    # sp is already initialised by the host emulator.
    # The host wrote argc at *sp and argv[0..n] above it (Linux ABI).
    # -----------------------------------------------------------------------

    # Ensure 16-byte stack alignment required by the calling convention.
    andi    sp, sp, -16

    # Load argc (a0) and argv (a1) from the stack.
    lw      a0, 0(sp)           # argc
    addi    a1, sp, 4           # argv pointer (points at argv[0] on stack)

    # Clear the frame pointer so stack traces terminate cleanly.
    li      fp, 0

    # Call main(argc, argv)
    call    main

    # main's return value is already in a0.
    # Call sys_exit(a0) — syscall number 93 in a7.
    li      a7, 93
    ecall

    # Should never reach here, but loop just in case.
.Lhalt:
    j       .Lhalt

    .size _start, . - _start
