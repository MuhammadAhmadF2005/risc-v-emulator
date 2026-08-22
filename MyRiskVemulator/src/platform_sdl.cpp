// platform_sdl.cpp — SDL2 implementation of the host platform layer.
// Only this file links against SDL2; the rest of the emulator is SDL-free.
#include "platform.h"
#include <SDL2/SDL.h>
#include <cstdio>
#include <cstring>

// -----------------------------------------------------------------------
// DOOM key constants (must match doomgeneric/doomkeys.h)
// -----------------------------------------------------------------------
#define KEY_RIGHTARROW  0xae
#define KEY_LEFTARROW   0xac
#define KEY_UPARROW     0xad
#define KEY_DOWNARROW   0xaf
#define KEY_STRAFE_L    0xa0
#define KEY_STRAFE_R    0xa1
#define KEY_USE         0x20  // Space
#define KEY_FIRE        0xa3  // Left Ctrl
#define KEY_ESCAPE      27
#define KEY_ENTER       13
#define KEY_TAB         9
#define KEY_F1          0x83
#define KEY_F2          0x84
#define KEY_F3          0x85
#define KEY_F4          0x86
#define KEY_F5          0x87
#define KEY_F6          0x88
#define KEY_F7          0x89
#define KEY_F8          0x8a
#define KEY_F9          0x8b
#define KEY_F10         0x8c
#define KEY_F11         0x8d
#define KEY_F12         0x8e
#define KEY_BACKSPACE   127
#define KEY_PAUSE       0xff
#define KEY_EQUALS      0x3d
#define KEY_MINUS       0x2d
#define KEY_RSHIFT      0xa0
#define KEY_RALT        0xa2
#define KEY_RCTRL       0xa3
#define KEY_LALT        KEY_RALT

// -----------------------------------------------------------------------
// Module-level SDL objects
// -----------------------------------------------------------------------
static SDL_Window*   g_window   = nullptr;
static SDL_Renderer* g_renderer = nullptr;
static SDL_Texture*  g_texture  = nullptr;
static int           g_width    = 320;
static int           g_height   = 200;

// -----------------------------------------------------------------------
static unsigned char sdlKeyToDoom(SDL_Keycode sym)
{
    switch (sym) {
    case SDLK_RIGHT:      return KEY_RIGHTARROW;
    case SDLK_LEFT:       return KEY_LEFTARROW;
    case SDLK_UP:         return KEY_UPARROW;
    case SDLK_DOWN:       return KEY_DOWNARROW;
    case SDLK_ESCAPE:     return KEY_ESCAPE;
    case SDLK_RETURN:     return KEY_ENTER;
    case SDLK_TAB:        return KEY_TAB;
    case SDLK_F1:         return KEY_F1;
    case SDLK_F2:         return KEY_F2;
    case SDLK_F3:         return KEY_F3;
    case SDLK_F4:         return KEY_F4;
    case SDLK_F5:         return KEY_F5;
    case SDLK_F6:         return KEY_F6;
    case SDLK_F7:         return KEY_F7;
    case SDLK_F8:         return KEY_F8;
    case SDLK_F9:         return KEY_F9;
    case SDLK_F10:        return KEY_F10;
    case SDLK_F11:        return KEY_F11;
    case SDLK_F12:        return KEY_F12;
    case SDLK_BACKSPACE:  return KEY_BACKSPACE;
    case SDLK_PAUSE:      return KEY_PAUSE;
    case SDLK_EQUALS:     return KEY_EQUALS;
    case SDLK_MINUS:      return KEY_MINUS;
    case SDLK_SPACE:      return KEY_USE;
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:     return KEY_RSHIFT;
    case SDLK_LALT:
    case SDLK_RALT:       return KEY_RALT;
    case SDLK_LCTRL:
    case SDLK_RCTRL:      return KEY_RCTRL;
    default:
        // Printable ASCII passthrough
        if (sym >= 32 && sym < 127) return (unsigned char)sym;
        return 0;
    }
}

// -----------------------------------------------------------------------
void hostInit(int w, int h)
{
    g_width  = w;
    g_height = h;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return;
    }

    g_window = SDL_CreateWindow(
        "RISC-V DOOM",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        w * 3, h * 3,            // 3x scale for readability
        SDL_WINDOW_SHOWN);
    if (!g_window) {
        fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        return;
    }

    g_renderer = SDL_CreateRenderer(g_window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_renderer) {
        fprintf(stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError());
        return;
    }

    // DOOM framebuffer is 32-bit ARGB/XRGB pixels
    g_texture = SDL_CreateTexture(
        g_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        w, h);
    if (g_texture) {
        SDL_SetTextureBlendMode(g_texture, SDL_BLENDMODE_NONE);
    } else {
        fprintf(stderr, "SDL_CreateTexture error: %s\n", SDL_GetError());
    }
}

// -----------------------------------------------------------------------
void hostDrawFrame(uint32_t* pixels, int w, int h)
{
    if (!g_renderer || !pixels) return;

    if (!g_texture || w != g_width || h != g_height) {
        if (g_texture) SDL_DestroyTexture(g_texture);
        g_width = w;
        g_height = h;
        g_texture = SDL_CreateTexture(
            g_renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            w, h);
        if (g_texture) {
            SDL_SetTextureBlendMode(g_texture, SDL_BLENDMODE_NONE);
        }
    }

    if (!g_texture) return;

    SDL_UpdateTexture(g_texture, nullptr, pixels, w * sizeof(uint32_t));
    SDL_RenderClear(g_renderer);
    SDL_RenderCopy(g_renderer, g_texture, nullptr, nullptr);
    SDL_RenderPresent(g_renderer);
}

// -----------------------------------------------------------------------
int hostPollKey()
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            // Signal the emulator to stop by returning a fake Escape press
            return (1 << 8) | KEY_ESCAPE;
        }
        if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
            unsigned char doomKey = sdlKeyToDoom(e.key.keysym.sym);
            if (doomKey == 0) continue;
            int pressed = (e.type == SDL_KEYDOWN) ? 1 : 0;
            // Pack: high byte = pressed flag, low byte = key code
            return (pressed << 8) | doomKey;
        }
    }
    return 0; // no event
}

// -----------------------------------------------------------------------
uint32_t hostGetTicks()
{
    return SDL_GetTicks();
}

// -----------------------------------------------------------------------
void hostSleep(uint32_t ms)
{
    SDL_Delay(ms);
}
