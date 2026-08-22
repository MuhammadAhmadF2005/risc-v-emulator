// platform_stub.cpp — headless stub of the host platform layer.
// Used when compiling without SDL2 (e.g. CI, demo mode, Windows without SDL).
// Compile with -DHEADLESS or simply link this instead of platform_sdl.cpp.
#include "platform.h"
#include <cstdio>
#include <ctime>

void hostInit(int w, int h)
{
    fprintf(stderr, "[headless] hostInit(%d, %d) — no display\n", w, h);
}

void hostDrawFrame(uint32_t* /*pixels*/, int w, int h)
{
    (void)w; (void)h;
    // Silently drop frames in headless mode
}

int hostPollKey()
{
    return 0; // no input
}

uint32_t hostGetTicks()
{
    // Use clock() as a fallback timing source
    return (uint32_t)(clock() * 1000 / CLOCKS_PER_SEC);
}

void hostSleep(uint32_t /*ms*/)
{
    // No-op in headless mode
}
