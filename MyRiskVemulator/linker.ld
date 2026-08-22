/*
 * linker.ld — Linker script for the freestanding DOOM/RV32IM ELF binary.
 *
 * Memory map
 * ----------
 *  0x00010000  .text  (code + read-only data)
 *  after text  .data  (initialised data)
 *  after data  .bss   (zero-initialised data, zeroed by ELF loader)
 *  0x3FFFFFF0  stack top (emulator sets sp here before entry)
 *
 * The emulator's flat 64 MB address space spans 0x00000000–0x03FFFFFF.
 * We start code at 0x10000 to give room for any ELF headers that might
 * be placed at very low addresses by the loader.
 */

OUTPUT_ARCH(riscv)
ENTRY(_start)

MEMORY
{
    /* Single flat region covering the emulator's 64 MB address space */
    RAM (rwx) : ORIGIN = 0x00010000, LENGTH = 0x03FF0000
}

SECTIONS
{
    /* ------------------------------------------------------------------ */
    /* .text — executable code                                             */
    /* ------------------------------------------------------------------ */
    .text ORIGIN(RAM) :
    {
        /* crt0 must be first so _start lands at the ELF entry point */
        KEEP(*(.text.start))
        *(.text .text.* .gnu.linkonce.t.*)
        *(.rodata .rodata.* .gnu.linkonce.r.*)

        . = ALIGN(4);
        _etext = .;
    } > RAM

    /* ------------------------------------------------------------------ */
    /* .data — initialised read/write data                                 */
    /* ------------------------------------------------------------------ */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data .data.* .gnu.linkonce.d.*)
        . = ALIGN(4);
        _edata = .;
    } > RAM

    /* ------------------------------------------------------------------ */
    /* .bss — zero-initialised data (zeroed by the ELF segment loader)    */
    /* ------------------------------------------------------------------ */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /* Symbol marking the end of all loaded sections — heapBase starts here */
    _end = .;

    /* ------------------------------------------------------------------ */
    /* Stack — grows downward from the top of the address space.           */
    /* The emulator initialises sp before jumping to _start.               */
    /* ------------------------------------------------------------------ */
    _stack_top = 0x3FFFFFF0;

    /* Discard debug / comment sections that are not needed in the binary  */
    /DISCARD/ :
    {
        *(.comment)
        *(.note*)
    }
}
