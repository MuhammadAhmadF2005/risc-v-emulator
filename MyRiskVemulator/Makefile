# =============================================================================
# Makefile — RISC-V DOOM emulator build system
#
# Targets:
#   rvemu     — host emulator binary (requires SDL2)
#   doom.elf  — guest RISC-V DOOM binary (requires RISC-V cross-compiler)
#   all       — build both (default)
#   clean     — remove all build artifacts
#
# Usage:
#   make rvemu                    # build only the emulator
#   make doom.elf                 # build only the DOOM guest binary
#   ./rvemu doom.elf doom1.wad    # run DOOM
# =============================================================================

# ---------------------------------------------------------------------------
# Host emulator (rvemu)
# ---------------------------------------------------------------------------
CXX       := g++
CXXFLAGS  := -O2 -std=c++17 -Wall -Wextra
SDL2_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null || echo "-I/usr/include/SDL2")
SDL2_LIBS   := $(shell sdl2-config --libs   2>/dev/null || echo "-lSDL2")

EMU_SRCS  := src/main.cpp src/cpu.cpp src/elf.cpp src/platform_sdl.cpp
EMU_HDRS  := src/cpu.h src/decode.h src/debug.h src/memory.h src/elf.h \
             src/types.h src/platform.h
EMU_BIN   := rvemu

$(EMU_BIN): $(EMU_SRCS) $(EMU_HDRS)
	$(CXX) $(CXXFLAGS) $(SDL2_CFLAGS) -Isrc $(EMU_SRCS) $(SDL2_LIBS) -o $@

# Headless build — no SDL2 required; useful for testing the emulator core.
rvemu_headless: src/main.cpp src/cpu.cpp src/elf.cpp src/platform_stub.cpp $(EMU_HDRS)
	$(CXX) $(CXXFLAGS) -Isrc src/main.cpp src/cpu.cpp src/elf.cpp src/platform_stub.cpp -o $@

# ---------------------------------------------------------------------------
# Guest DOOM binary (doom.elf)
#
# Requires:
#   - riscv32-unknown-elf-gcc or riscv64-unknown-elf-gcc with RV32IM support
#   - doomgeneric/ source directory (clone from https://github.com/ozkl/doomgeneric)
# ---------------------------------------------------------------------------
RISCV_CC  ?= $(shell which riscv32-unknown-elf-gcc 2>/dev/null || which riscv64-unknown-elf-gcc 2>/dev/null || echo riscv32-unknown-elf-gcc)
RISCV_AR  ?= $(shell which riscv32-unknown-elf-ar 2>/dev/null || which riscv64-unknown-elf-ar 2>/dev/null || echo riscv32-unknown-elf-ar)

CFLAGS    := -march=rv32im -mabi=ilp32 -O2 -nostdlib -ffreestanding \
             -fno-stack-protector -fno-builtin -Wall \
             -DDOOMGENERIC_RESX=320 -DDOOMGENERIC_RESY=200 \
             -DNORMALUNIX -DLINUX -D_DEFAULT_SOURCE \
             -Idoomgeneric/doomgeneric

LDFLAGS   := -T linker.ld -nostartfiles
LDLIBS    := -lc -lm -lgcc

DOOM_SRC_DIR := doomgeneric/doomgeneric

SRC_DOOM := dummy.o am_map.o doomdef.o doomstat.o dstrings.o d_event.o d_items.o \
            d_iwad.o d_loop.o d_main.o d_mode.o d_net.o f_finale.o f_wipe.o \
            g_game.o hu_lib.o hu_stuff.o info.o i_cdmus.o i_endoom.o i_joystick.o \
            i_scale.o i_sound.o i_system.o i_timer.o memio.o m_argv.o m_bbox.o \
            m_cheat.o m_config.o m_controls.o m_fixed.o m_menu.o m_misc.o \
            m_random.o p_ceilng.o p_doors.o p_enemy.o p_floor.o p_inter.o \
            p_lights.o p_map.o p_maputl.o p_mobj.o p_plats.o p_pspr.o p_saveg.o \
            p_setup.o p_sight.o p_spec.o p_switch.o p_telept.o p_tick.o p_user.o \
            r_bsp.o r_data.o r_draw.o r_main.o r_plane.o r_segs.o r_sky.o \
            r_things.o sha1.o sounds.o statdump.o st_lib.o st_stuff.o s_sound.o \
            tables.o v_video.o wi_stuff.o w_checksum.o w_file.o w_main.o \
            w_wad.o z_zone.o w_file_stdc.o i_input.o i_video.o doomgeneric.o

DOOM_OBJS := $(addprefix $(DOOM_SRC_DIR)/, $(SRC_DOOM)) dg_riscv.o crt0.o

DOOM_ELF  := doom.elf

# Compile doomgeneric C sources
$(DOOM_SRC_DIR)/%.o: $(DOOM_SRC_DIR)/%.c
	$(RISCV_CC) $(CFLAGS) -c $< -o $@

# Compile our C platform file
dg_riscv.o: dg_riscv.c
	$(RISCV_CC) $(CFLAGS) -c $< -o $@

# Assemble startup file
crt0.o: crt0.S
	$(RISCV_CC) $(CFLAGS) -c $< -o $@

$(DOOM_ELF): $(DOOM_OBJS)
	$(RISCV_CC) $(CFLAGS) $(LDFLAGS) $(DOOM_OBJS) $(LDLIBS) -o $@
	@echo ""
	@echo "Built: $@"
	@riscv64-unknown-elf-size $@

# ---------------------------------------------------------------------------
# Convenience targets
# ---------------------------------------------------------------------------
.PHONY: all clean rvemu

all: $(EMU_BIN) $(DOOM_ELF)

clean:
	rm -f $(EMU_BIN) $(DOOM_ELF) $(DOOM_OBJS) dg_riscv.o crt0.o
