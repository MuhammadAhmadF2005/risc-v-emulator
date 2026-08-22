#pragma once

#include <cstdint>

// --- Host platform interface ---
// Implemented by platform_sdl.cpp (or a stub for headless mode).
// cpu.cpp calls these from the custom DOOM ECALL handlers (0x100-0x103).

// Called once before the run loop: creates the SDL window/renderer/texture.
void hostInit(int w, int h);

// Called by ECALL 0x100: blit 'w*h' RGBA pixels to the SDL window.
void hostDrawFrame(uint32_t* pixels, int w, int h);

// Called by ECALL 0x101: poll one SDL event and return a packed int:
//   bits [31..8] = pressed (1 = key down, 0 = key up)
//   bits [ 7..0] = DOOM key code
// Returns 0 if no pending key event.
int  hostPollKey();

// Called by ECALL 0x102: milliseconds since program start (wraps after ~49 days).
uint32_t hostGetTicks();

// Called by ECALL 0x103: sleep for 'ms' milliseconds on the host.
void hostSleep(uint32_t ms);
