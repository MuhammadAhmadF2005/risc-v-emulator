#include "elf.h"
#include <fstream>
#include <iostream>
#include <vector>

// 32-bit ELF Header layout
struct Elf32_Ehdr {
    u8  e_ident[16];
    u16 e_type;
    u16 e_machine;
    u32 e_version;
    u32 e_entry;
    u32 e_phoff;
    u32 e_shoff;
    u32 e_flags;
    u16 e_ehsize;
    u16 e_phentsize;
    u16 e_phnum;
    u16 e_shentsize;
    u16 e_shnum;
    u16 e_shstrndx;
};

// 32-bit ELF Program Header layout
struct Elf32_Phdr {
    u32 p_type;
    u32 p_offset;
    u32 p_vaddr;
    u32 p_paddr;
    u32 p_filesz;
    u32 p_memsz;
    u32 p_flags;
    u32 p_align;
};

constexpr u32 PT_LOAD = 1;

bool loadELF(CPU &cpu, const char *path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open ELF file: " << path << "\n";
        return false;
    }

    Elf32_Ehdr ehdr;
    if (!file.read(reinterpret_cast<char*>(&ehdr), sizeof(ehdr))) {
        std::cerr << "Failed to read ELF header: " << path << "\n";
        return false;
    }

    // Verify ELF magic: 0x7F 'E' 'L' 'F'
    if (ehdr.e_ident[0] != 0x7F || ehdr.e_ident[1] != 'E' ||
        ehdr.e_ident[2] != 'L' || ehdr.e_ident[3] != 'F') {
        std::cerr << "Invalid ELF magic numbers: " << path << "\n";
        return false;
    }

    // Class: 1 = ELF32, Data: 1 = Little Endian
    if (ehdr.e_ident[4] != 1 || ehdr.e_ident[5] != 1) {
        std::cerr << "Not a 32-bit Little-Endian ELF file: " << path << "\n";
        return false;
    }

    cpu.pc = ehdr.e_entry;

    // Load PT_LOAD segments
    for (int i = 0; i < ehdr.e_phnum; ++i) {
        Elf32_Phdr phdr;
        file.seekg(ehdr.e_phoff + i * ehdr.e_phentsize);
        if (!file.read(reinterpret_cast<char*>(&phdr), sizeof(phdr))) {
            std::cerr << "Failed to read program header " << i << "\n";
            return false;
        }

        if (phdr.p_type == PT_LOAD) {
            if (phdr.p_vaddr + phdr.p_memsz > cpu.mem.size()) {
                std::cerr << "Segment out of memory bounds\n";
                return false;
            }

            // Copy file data into CPU memory
            file.seekg(phdr.p_offset);
            if (phdr.p_filesz > 0) {
                if (!file.read(reinterpret_cast<char*>(&cpu.mem[phdr.p_vaddr]), phdr.p_filesz)) {
                    std::cerr << "Failed to read segment data\n";
                    return false;
                }
            }

            // Zero out remaining memsz - filesz bytes
            if (phdr.p_memsz > phdr.p_filesz) {
                std::fill(cpu.mem.begin() + phdr.p_vaddr + phdr.p_filesz,
                          cpu.mem.begin() + phdr.p_vaddr + phdr.p_memsz,
                          0);
            }
        }
    }

    return true;
}
