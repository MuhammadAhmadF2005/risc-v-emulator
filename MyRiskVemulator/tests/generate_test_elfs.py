import struct
import os
import subprocess
import sys

def create_elf(filename, inst_list, data_bytes=b""):
    # ELF32 Header & Program Header generator
    # Entry point: 0x10000
    e_entry = 0x10000
    
    code_bytes = b''.join(struct.pack('<I', inst) for inst in inst_list)
    
    # Align data after instructions (padded to 4-byte boundary)
    code_len = len(code_bytes)
    data_offset_in_segment = (code_len + 3) & ~3
    padding = b'\x00' * (data_offset_in_segment - code_len)
    
    payload = code_bytes + padding + data_bytes
    
    e_ident = b'\x7fELF\x01\x01\x01\x00' + b'\x00' * 8
    e_type = 2      # ET_EXEC
    e_machine = 243 # EM_RISCV
    e_version = 1
    e_phoff = 52
    e_shoff = 0
    e_flags = 0
    e_ehsize = 52
    e_phentsize = 32
    e_phnum = 1
    e_shentsize = 0
    e_shnum = 0
    e_shstrndx = 0

    ehdr = struct.pack('<16sHHIIIIIHHHHHH',
        e_ident, e_type, e_machine, e_version, e_entry,
        e_phoff, e_shoff, e_flags, e_ehsize, e_phentsize,
        e_phnum, e_shentsize, e_shnum, e_shstrndx
    )

    p_type = 1  # PT_LOAD
    p_offset = 52 + 32
    p_vaddr = e_entry
    p_paddr = e_entry
    p_filesz = len(payload)
    p_memsz = len(payload)
    p_flags = 7
    p_align = 0x1000

    phdr = struct.pack('<IIIIIIII',
        p_type, p_offset, p_vaddr, p_paddr,
        p_filesz, p_memsz, p_flags, p_align
    )

    with open(filename, 'wb') as f:
        f.write(ehdr)
        f.write(phdr)
        f.write(payload)

# Instruction Encoders
def enc_i(imm, rs1, f3, rd, op):
    return ((imm & 0xFFF) << 20) | ((rs1 & 0x1F) << 15) | ((f3 & 7) << 12) | ((rd & 0x1F) << 7) | (op & 0x7F)

def enc_r(f7, rs2, rs1, f3, rd, op):
    return ((f7 & 0x7F) << 25) | ((rs2 & 0x1F) << 20) | ((rs1 & 0x1F) << 15) | ((f3 & 7) << 12) | ((rd & 0x1F) << 7) | (op & 0x7F)

def enc_u(imm, rd, op):
    return (imm & 0xFFFFF000) | ((rd & 0x1F) << 7) | (op & 0x7F)

def enc_b(imm, rs2, rs1, f3, op):
    b12 = (imm >> 12) & 1
    b11 = (imm >> 11) & 1
    b10_5 = (imm >> 5) & 0x3F
    b4_1 = (imm >> 1) & 0xF
    imm_encoded = (b12 << 31) | (b10_5 << 25) | (rs2 << 20) | (rs1 << 15) | (f3 << 12) | (b4_1 << 8) | (b11 << 7) | (op & 0x7F)
    return imm_encoded

def build_all_tests():
    os.makedirs("MyRiskVemulator/tests/bin", exist_ok=True)
    
    # 1. loop_sum.elf: Sum numbers 1..10 (Result = 55)
    sum_insts = [
        enc_i(0, 0, 0, 10, 0x13),   # addi a0, zero, 0
        enc_i(1, 0, 0, 5, 0x13),    # addi t0, zero, 1
        enc_i(11, 0, 0, 6, 0x13),   # addi t1, zero, 11
        enc_r(0, 5, 10, 0, 10, 0x33),# add  a0, a0, t0
        enc_i(1, 5, 0, 5, 0x13),    # addi t0, t0, 1
        enc_b(-8, 6, 5, 1, 0x63),   # bne  t0, t1, -8
        enc_i(93, 0, 0, 17, 0x13),  # addi a7, zero, 93
        0x00000073                  # ecall
    ]
    create_elf("MyRiskVemulator/tests/bin/loop_sum.elf", sum_insts)
    
    # 2. loop_fact.elf: Factorial of 5 using MUL (Result = 120)
    fact_insts = [
        enc_i(1, 0, 0, 10, 0x13),   # addi a0, zero, 1 (fact = 1)
        enc_i(1, 0, 0, 5, 0x13),    # addi t0, zero, 1 (i = 1)
        enc_i(6, 0, 0, 6, 0x13),    # addi t1, zero, 6 (limit = 6)
        enc_r(1, 5, 10, 0, 10, 0x33),# mul  a0, a0, t0 (fact = fact * i)
        enc_i(1, 5, 0, 5, 0x13),    # addi t0, t0, 1
        enc_b(-8, 6, 5, 1, 0x63),   # bne  t0, t1, -8
        enc_i(93, 0, 0, 17, 0x13),  # addi a7, zero, 93
        0x00000073                  # ecall
    ]
    create_elf("MyRiskVemulator/tests/bin/loop_fact.elf", fact_insts)
    
    # 3. loop_print.elf: Print "Loop iteration\n" 3 times via sys_write (64)
    msg = b"Loop iteration!\n" # 16 bytes
    # Instructions end at 0x10000 + 13*4 = 0x10034. Padded to 0x10034 -> msg starts at 0x10034
    print_insts = [
        enc_i(0, 0, 0, 8, 0x13),    # addi s0, zero, 0
        enc_i(3, 0, 0, 9, 0x13),    # addi s1, zero, 3
        enc_i(64, 0, 0, 17, 0x13),  # addi a7, zero, 64
        enc_i(1, 0, 0, 10, 0x13),   # addi a0, zero, 1
        enc_u(0x10000, 11, 0x37),   # lui  a1, 0x10
        enc_i(0x34, 11, 0, 11, 0x13),# addi a1, a1, 0x34
        enc_i(len(msg), 0, 0, 12, 0x13), # addi a2, zero, len
        0x00000073,                 # ecall (sys_write)
        enc_i(1, 8, 0, 8, 0x13),    # addi s0, s0, 1
        enc_b(-28, 9, 8, 1, 0x63),  # bne  s0, s1, -28
        enc_i(93, 0, 0, 17, 0x13),  # addi a7, zero, 93
        enc_i(0, 0, 0, 10, 0x13),   # addi a0, zero, 0
        0x00000073                  # ecall (sys_exit)
    ]
    create_elf("MyRiskVemulator/tests/bin/loop_print.elf", print_insts, msg)
    print("Generated all test binaries in MyRiskVemulator/tests/bin/")

if __name__ == "__main__":
    build_all_tests()
