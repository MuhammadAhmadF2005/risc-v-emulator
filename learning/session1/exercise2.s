.data
    a :.word 3 b :.word 4 c :.word 0

                                 .text
                                 .globl main
                                     main : la t3,
    a
        lw t0,
    0(t3)

        la t4,
    b
    lw t1,
    0(t4)

        la a2,
    c

    mul t0,
    t0, t0 mul t1, t1, t1 add t2, t0, t1 sw t2, 0(a2)

                                                    li a7,
    10 ecall
