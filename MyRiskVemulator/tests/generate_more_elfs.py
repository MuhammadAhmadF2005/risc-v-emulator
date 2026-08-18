import struct
import os

# Helper to pack 32-bit little-endian ELF header & multi-segment program headers
def create_multi_segment_elf(filename, entry_point, segments):
    """
    segments is a list of dicts:
    {
        'vaddr': 0x10000,
        'paddr': 0x10000,
        'flags': 7,
        'file_data': bytes,
        'memsz': int
    }
    """
    num_segments = len(segments)
    e_phoff = 52
    ph_size = 32
    first_payload_offset = e_phoff + num_segments * ph_size

    # Calculate file offsets for each segment
    current_offset = first_payload_offset
    ph_list = []
    payload_list = []

    for seg in segments:
        file_data = seg['file_data']
        filesz = len(file_data)
        memsz = max(seg['memsz'], filesz)
        vaddr = seg['vaddr']
        paddr = seg.get('paddr', vaddr)
        flags = seg.get('flags', 7)

        # Build program header
        phdr = struct.pack('<IIIIIIII',
            1,              # p_type = PT_LOAD
            current_offset, # p_offset
            vaddr,          # p_vaddr
            paddr,          # p_paddr
            filesz,         # p_filesz
            memsz,          # p_memsz
            flags,          # p_flags
            0x1000          # p_align
        )
        ph_list.append(phdr)
        payload_list.append(file_data)
        current_offset += filesz

    # ELF32 Header
    e_ident = b'\x7fELF\x01\x01\x01\x00' + b'\x00' * 8
    ehdr = struct.pack('<16sHHIIIIIHHHHHH',
        e_ident,
        2,              # e_type = ET_EXEC
        243,            # e_machine = EM_RISCV
        1,              # e_version
        entry_point,    # e_entry
        e_phoff,        # e_phoff
        0,              # e_shoff
        0,              # e_flags
        52,             # e_ehsize
        ph_size,        # e_phentsize
        num_segments,   # e_phnum
        0, 0, 0
    )

    with open(filename, 'wb') as f:
        f.write(ehdr)
        for ph in ph_list:
            f.write(ph)
        for payload in payload_list:
            f.write(payload)

# Instruction Encoders
def enc_i(imm, rs1, f3, rd, op):
    return ((imm & 0xFFF) << 20) | ((rs1 & 0x1F) << 15) | ((f3 & 7) << 12) | ((rd & 0x1F) << 7) | (op & 0x7F)

def enc_r(f7, rs2, rs1, f3, rd, op):
    return ((f7 & 0x7F) << 25) | ((rs2 & 0x1F) << 20) | ((rs1 & 0x1F) << 15) | ((f3 & 7) << 12) | ((rd & 0x1F) << 7) | (op & 0x7F)

def enc_u(imm, rd, op):
    return (imm & 0xFFFFF000) | ((rd & 0x1F) << 7) | (op & 0x7F)

def enc_s(imm, rs2, rs1, f3, op):
    imm11_5 = (imm >> 5) & 0x7F
    imm4_0  = imm & 0x1F
    return (imm11_5 << 25) | ((rs2 & 0x1F) << 20) | ((rs1 & 0x1F) << 15) | ((f3 & 7) << 12) | (imm4_0 << 7) | (op & 0x7F)

def enc_b(imm, rs2, rs1, f3, op):
    b12 = (imm >> 12) & 1
    b11 = (imm >> 11) & 1
    b10_5 = (imm >> 5) & 0x3F
    b4_1 = (imm >> 1) & 0xF
    return (b12 << 31) | (b10_5 << 25) | ((rs2 & 0x1F) << 20) | ((rs1 & 0x1F) << 15) | ((f3 & 7) << 12) | (b4_1 << 8) | (b11 << 7) | (op & 0x7F)


def generate_all():
    os.makedirs("MyRiskVemulator/tests/bin", exist_ok=True)

    # -------------------------------------------------------------
    # 1. Multi-Segment & BSS Test Binary: multi_segment.elf
    # Segment 1 (.text) at 0x10000
    # Segment 2 (.data) at 0x20000 (4 words: 10, 20, 30, 40)
    # Segment 3 (.bss)  at 0x30000 (filesz=0, memsz=64)
    # -------------------------------------------------------------
    text_code = [
        # Set up a0 = 0 (sum)
        enc_i(0, 0, 0, 10, 0x13),      # addi a0, zero, 0
        
        # Load address of .data (0x20000) into s0 (x8)
        enc_u(0x20000, 8, 0x37),       # lui  s0, 0x20
        
        # Load 4 words from .data and sum them
        enc_i(0, 8, 2, 5, 0x03),       # lw   t0, 0(s0)  -> 10
        enc_r(0, 5, 10, 0, 10, 0x33),  # add  a0, a0, t0 -> 10
        enc_i(4, 8, 2, 5, 0x03),       # lw   t0, 4(s0)  -> 20
        enc_r(0, 5, 10, 0, 10, 0x33),  # add  a0, a0, t0 -> 30
        enc_i(8, 8, 2, 5, 0x03),       # lw   t0, 8(s0)  -> 30
        enc_r(0, 5, 10, 0, 10, 0x33),  # add  a0, a0, t0 -> 60
        enc_i(12, 8, 2, 5, 0x03),      # lw   t0, 12(s0) -> 40
        enc_r(0, 5, 10, 0, 10, 0x33),  # add  a0, a0, t0 -> 100
        
        # Check BSS segment at 0x30000 is zero
        enc_u(0x30000, 9, 0x37),       # lui  s1, 0x30
        enc_i(0, 9, 2, 6, 0x03),       # lw   t1, 0(s1)  -> 0 (from BSS)
        enc_r(0, 6, 10, 0, 10, 0x33),  # add  a0, a0, t1 -> 100
        
        # Store sum 100 into BSS at 0x30000
        enc_s(0, 10, 9, 2, 0x23),      # sw   a0, 0(s1)
        
        # Exit with a0 (100)
        enc_i(93, 0, 0, 17, 0x13),     # addi a7, zero, 93
        0x00000073                     # ecall
    ]
    text_bytes = b''.join(struct.pack('<I', inst) for inst in text_code)
    data_bytes = struct.pack('<4I', 10, 20, 30, 40)

    create_multi_segment_elf("MyRiskVemulator/tests/bin/multi_segment.elf", 0x10000, [
        {'vaddr': 0x10000, 'file_data': text_bytes, 'memsz': len(text_bytes)},
        {'vaddr': 0x20000, 'file_data': data_bytes, 'memsz': len(data_bytes)},
        {'vaddr': 0x30000, 'file_data': b'', 'memsz': 64} # BSS zero segment
    ])

    # -------------------------------------------------------------
    # 2. Entry Point Offset & Fibonacci Test Binary: fibonacci.elf
    # Entry Point at 0x10020 (offset inside segment)
    # Calculates F(10) = 55
    # -------------------------------------------------------------
    # NOP padding from 0x10000 to 0x1001F (8 instructions)
    nop = enc_i(0, 0, 0, 0, 0x13) # addi x0, x0, 0
    fib_padding = [nop] * 8       # 32 bytes padding -> entry at 0x10020
    
    fib_code = [
        # F(1)=1, F(2)=1
        enc_i(1, 0, 0, 5, 0x13),       # addi t0, zero, 1 (F_prev)
        enc_i(1, 0, 0, 6, 0x13),       # addi t1, zero, 1 (F_curr)
        enc_i(3, 0, 0, 7, 0x13),       # addi t2, zero, 3 (counter i = 3)
        enc_i(11, 0, 0, 28, 0x13),     # addi t3, zero, 11 (limit = 11)
        
        # Loop body (starts at 0x10030)
        enc_r(0, 6, 5, 0, 29, 0x33),   # add  t4, t0, t1 (next = F_prev + F_curr)
        enc_i(0, 6, 0, 5, 0x13),       # addi t0, t1, 0  (F_prev = F_curr)
        enc_i(0, 29, 0, 6, 0x13),      # addi t1, t4, 0  (F_curr = next)
        enc_i(1, 7, 0, 7, 0x13),       # addi t2, t2, 1  (i++)
        enc_b(-16, 28, 7, 1, 0x63),    # bne  t2, t3, -16 (loop back 16 bytes)
        
        # Result in a0 = t1 (55)
        enc_i(0, 6, 0, 10, 0x13),      # addi a0, t1, 0
        enc_i(93, 0, 0, 17, 0x13),     # addi a7, zero, 93
        0x00000073                     # ecall
    ]
    
    fib_bytes = b''.join(struct.pack('<I', inst) for inst in (fib_padding + fib_code))
    create_multi_segment_elf("MyRiskVemulator/tests/bin/fibonacci.elf", 0x10020, [
        {'vaddr': 0x10000, 'file_data': fib_bytes, 'memsz': len(fib_bytes)}
    ])

    # -------------------------------------------------------------
    # 3. String Output & Syscall Write Test Binary: hello_str.elf
    # -------------------------------------------------------------
    str_msg = b"RISC-V ELF Loader Verification OK!\n" # 35 bytes
    hello_code = [
        enc_i(64, 0, 0, 17, 0x13),     # addi a7, zero, 64 (sys_write)
        enc_i(1, 0, 0, 10, 0x13),      # addi a0, zero, 1  (fd=1 stdout)
        enc_u(0x10000, 11, 0x37),      # lui  a1, 0x10
        enc_i(0x24, 11, 0, 11, 0x13),   # addi a1, a1, 0x24 -> msg at 0x10024
        enc_i(len(str_msg), 0, 0, 12, 0x13), # addi a2, zero, len
        0x00000073,                    # ecall (sys_write)
        enc_i(93, 0, 0, 17, 0x13),     # addi a7, zero, 93
        enc_i(0, 0, 0, 10, 0x13),      # addi a0, zero, 0
        0x00000073                     # ecall (sys_exit)
    ]
    hello_bytes = b''.join(struct.pack('<I', inst) for inst in hello_code) + str_msg
    create_multi_segment_elf("MyRiskVemulator/tests/bin/hello_str.elf", 0x10000, [
        {'vaddr': 0x10000, 'file_data': hello_bytes, 'memsz': len(hello_bytes)}
    ])

    # -------------------------------------------------------------
    # 4. Malformed ELF Binaries for Error Handling Tests
    # -------------------------------------------------------------
    # Invalid magic
    with open("MyRiskVemulator/tests/bin/bad_magic.elf", "wb") as f:
        f.write(b"NOT_AN_ELF_FILE_HEADER_DATA_1234567890")
    
    # 64-bit ELF (EI_CLASS = 2)
    ehdr_64 = b'\x7fELF\x02\x01\x01\x00' + b'\x00' * 8 + b'\x00' * 36
    with open("MyRiskVemulator/tests/bin/bad_class64.elf", "wb") as f:
        f.write(ehdr_64)

    # Big-endian ELF (EI_DATA = 2)
    ehdr_be = b'\x7fELF\x01\x02\x01\x00' + b'\x00' * 8 + b'\x00' * 36
    with open("MyRiskVemulator/tests/bin/bad_endian.elf", "wb") as f:
        f.write(ehdr_be)

    print("Generated all extended ELF binaries successfully!")

if __name__ == "__main__":
    generate_all()
