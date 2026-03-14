#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"
#include "i_video.h"

#define SCREENWIDTH  320
#define SCREENHEIGHT 200

static SDL_Window   *window   = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture  *texture  = NULL;

// Doom's 256-color palette
static SDL_Color palette[256];

// The screen buffer Doom renders into
static uint8_t *screen_buffer = NULL;
// 32-bit RGBA buffer we upload to SDL
static uint32_t *rgb_buffer = NULL;

void I_ShutdownGraphics(void)
{
    if (texture)  { SDL_DestroyTexture(texture);   texture  = NULL; }
    if (renderer) { SDL_DestroyRenderer(renderer); renderer = NULL; }
    if (window)   { SDL_DestroyWindow(window);     window   = NULL; }
    SDL_Quit();
}

void I_StartFrame(void) {}

void I_GetEvent(void)
{
    SDL_Event e;
    extern int Quit;

    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:
            I_Quit();
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            event_t dev;
            dev.type = (e.type == SDL_KEYDOWN) ? ev_keydown : ev_keyup;
            // Map SDL scancode to Doom key
            switch (e.key.keysym.sym)
            {
            case SDLK_LEFT:      dev.data1 = KEY_LEFTARROW;  break;
            case SDLK_RIGHT:     dev.data1 = KEY_RIGHTARROW; break;
            case SDLK_UP:        dev.data1 = KEY_UPARROW;    break;
            case SDLK_DOWN:      dev.data1 = KEY_DOWNARROW;  break;
            case SDLK_ESCAPE:    dev.data1 = KEY_ESCAPE;     break;
            case SDLK_RETURN:    dev.data1 = KEY_ENTER;      break;
            case SDLK_TAB:       dev.data1 = KEY_TAB;        break;
            case SDLK_F1:        dev.data1 = KEY_F1;         break;
            case SDLK_F2:        dev.data1 = KEY_F2;         break;
            case SDLK_F3:        dev.data1 = KEY_F3;         break;
            case SDLK_F4:        dev.data1 = KEY_F4;         break;
            case SDLK_F5:        dev.data1 = KEY_F5;         break;
            case SDLK_F6:        dev.data1 = KEY_F6;         break;
            case SDLK_F7:        dev.data1 = KEY_F7;         break;
            case SDLK_F8:        dev.data1 = KEY_F8;         break;
            case SDLK_F9:        dev.data1 = KEY_F9;         break;
            case SDLK_F10:       dev.data1 = KEY_F10;        break;
            case SDLK_F11:       dev.data1 = KEY_F11;        break;
            case SDLK_F12:       dev.data1 = KEY_F12;        break;
            case SDLK_BACKSPACE: dev.data1 = KEY_BACKSPACE;  break;
            case SDLK_PAUSE:     dev.data1 = KEY_PAUSE;      break;
            case SDLK_EQUALS:    dev.data1 = KEY_EQUALS;     break;
            case SDLK_MINUS:     dev.data1 = KEY_MINUS;      break;
            case SDLK_RSHIFT:
            case SDLK_LSHIFT:    dev.data1 = KEY_RSHIFT;     break;
            case SDLK_RCTRL:
            case SDLK_LCTRL:     dev.data1 = KEY_RCTRL;      break;
            case SDLK_RALT:
            case SDLK_LALT:      dev.data1 = KEY_RALT;       break;
            default:
                dev.data1 = e.key.keysym.sym & 0x7f;
                break;
            }
            D_PostEvent(&dev);
            break;
        }
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        {
            event_t dev;
            dev.type = ev_mouse;
            dev.data1 = (SDL_GetMouseState(NULL, NULL) &
                        (SDL_BUTTON_LMASK|SDL_BUTTON_RMASK|SDL_BUTTON_MMASK));
            dev.data2 = dev.data3 = 0;
            D_PostEvent(&dev);
            break;
        }
        case SDL_MOUSEMOTION:
        {
            event_t dev;
            dev.type   = ev_mouse;
            dev.data1  = (SDL_GetMouseState(NULL, NULL) &
                         (SDL_BUTTON_LMASK|SDL_BUTTON_RMASK|SDL_BUTTON_MMASK));
            dev.data2  = e.motion.xrel << 2;
            dev.data3  = -e.motion.yrel << 2;
            D_PostEvent(&dev);
            break;
        }
        default:
            break;
        }
    }
}

void I_StartTic(void)
{
    I_GetEvent();
}

void I_UpdateNoBlit(void) {}

void I_FinishUpdate(void)
{
    // Convert 8-bit paletted screen to 32-bit RGBA
    for (int i = 0; i < SCREENWIDTH * SCREENHEIGHT; i++)
    {
        uint8_t idx = screen_buffer[i];
        rgb_buffer[i] = (0xFF000000) |
                        (palette[idx].r << 16) |
                        (palette[idx].g << 8)  |
                        (palette[idx].b);
    }

    SDL_UpdateTexture(texture, NULL, rgb_buffer, SCREENWIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void I_ReadScreen(byte *scr)
{
    memcpy(scr, screen_buffer, SCREENWIDTH * SCREENHEIGHT);
}

void I_SetPalette(byte *palette_data)
{
    for (int i = 0; i < 256; i++)
    {
        palette[i].r = palette_data[i*3 + 0];
        palette[i].g = palette_data[i*3 + 1];
        palette[i].b = palette_data[i*3 + 2];
    }
}

void I_InitGraphics(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        I_Error("SDL_Init failed: %s", SDL_GetError());

    window = SDL_CreateWindow("DOOM",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              SCREENWIDTH * 2, SCREENHEIGHT * 2,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window)
        I_Error("SDL_CreateWindow failed: %s", SDL_GetError());

    renderer = SDL_CreateRenderer(window, -1,
                                  SDL_RENDERER_ACCELERATED |
                                  SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
        I_Error("SDL_CreateRenderer failed: %s", SDL_GetError());

    SDL_RenderSetLogicalSize(renderer, SCREENWIDTH, SCREENHEIGHT);

    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                SCREENWIDTH, SCREENHEIGHT);
    if (!texture)
        I_Error("SDL_CreateTexture failed: %s", SDL_GetError());

    // Allocate buffers
    screen_buffer = (uint8_t *)malloc(SCREENWIDTH * SCREENHEIGHT);
    rgb_buffer    = (uint32_t *)malloc(SCREENWIDTH * SCREENHEIGHT * sizeof(uint32_t));

    if (!screen_buffer || !rgb_buffer)
        I_Error("Failed to allocate screen buffers");

    // Point Doom's screens[0] at our buffer
    screens[0] = screen_buffer;
}
