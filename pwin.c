#include "pwin.h"

#include <stdio.h>

#define GET_RED(x) (((x) & 0x30) << 2)
#define GET_GREEN(x) (((x) & 0x0C) << 4)
#define GET_BLUE(x) (((x) & 0x03) << 6)

int pwin_init(struct pixel_window *pwin)
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
    {
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Pixel Window", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if(win == NULL)
    {
        return 2;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(ren == NULL)
    {
        return 3;
    }

    pwin->win = win;
    pwin->ren = ren;
    return 0;
}

void pwin_draw_image(struct pixel_window *pwin, unsigned char* screen, int width, int height)
{
    int s_width, s_height, x, y;

    SDL_GetWindowSize(pwin->win, &s_width, &s_height);
    SDL_RenderClear(pwin->ren);

    SDL_Rect pixel = {0, 0, s_width / width, s_height / height};

    for(y = 0; y < height; y++)
    {
        for(x = 0; x < width; x++)
        {
            if(screen[(y * width) + x])
            {
                SDL_SetRenderDrawColor(pwin->ren, 0xFF, 0xFF, 0xFF, 0xFF);
            }
            else
            {
                SDL_SetRenderDrawColor(pwin->ren, 0x00, 0x00, 0x00, 0xFF);
            }
            SDL_RenderFillRect(pwin->ren, &pixel);
            pixel.x += s_width / width;
        }
        pixel.x = 0;
        pixel.y += s_height / height;
    }

    SDL_RenderPresent(pwin->ren);
}

int pwin_event_loop(unsigned char *keys)
{
    SDL_Event e;

    while(SDL_PollEvent(&e) != 0)
    {
        if(e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
        {
            int state;
            if(e.type == SDL_KEYDOWN)
            {
                state = 1;
            }
            else if(e.type == SDL_KEYUP)
            {
                state = 0;
            }
            switch(e.key.keysym.sym)
            {
                case SDLK_1:
                    keys[0x1] = state;
                    break;
                case SDLK_2:
                    keys[0x2] = state;
                    break;
                case SDLK_3:
                    keys[0x3] = state;
                    break;
                case SDLK_q:
                    keys[0x4] = state;
                    break;
                case SDLK_w:
                    keys[0x5] = state;
                    break;
                case SDLK_e:
                    keys[0x6] = state;
                    break;
                case SDLK_a:
                    keys[0x7] = state;
                    break;
                case SDLK_s:
                    keys[0x8] = state;
                    break;
                case SDLK_d:
                    keys[0x9] = state;
                    break;
                case SDLK_x:
                    keys[0x0] = state;
                    break;
                case SDLK_z:
                    keys[0xA] = state;
                    break;
                case SDLK_c:
                    keys[0xB] = state;
                    break;
                case SDLK_4:
                    keys[0xC] = state;
                    break;
                case SDLK_r:
                    keys[0xD] = state;
                    break;
                case SDLK_f:
                    keys[0xE] = state;
                    break;
                case SDLK_v:
                    keys[0xF] = state;
                    break;
            }
        }
    }
    return 0;
}

char pwin_wait_for_key(unsigned char *keys)
{
    SDL_Event e;
    char done = 0;
    char key;

    while(!done && SDL_WaitEvent(&e))
    {
        if(e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
        {
            int state;
            if(e.type == SDL_KEYDOWN)
            {
                state = 1;
                done = 1;
            }
            else if(e.type == SDL_KEYUP)
            {
                state = 0;
            }
            switch(e.key.keysym.sym)
            {
                case SDLK_1:
                    key = 0x1;
                    break;
                case SDLK_2:
                    key = 0x2;
                    break;
                case SDLK_3:
                    key = 0x3;
                    break;
                case SDLK_q:
                    key = 0x4;
                    break;
                case SDLK_w:
                    key = 0x5;
                    break;
                case SDLK_e:
                    key = 0x6;
                    break;
                case SDLK_a:
                    key = 0x7;
                    break;
                case SDLK_s:
                    key = 0x8;
                    break;
                case SDLK_d:
                    key = 0x9;
                    break;
                case SDLK_x:
                    key = 0x0;
                    break;
                case SDLK_z:
                    key = 0xA;
                    break;
                case SDLK_c:
                    key = 0xB;
                    break;
                case SDLK_4:
                    key = 0xC;
                    break;
                case SDLK_r:
                    key = 0xD;
                    break;
                case SDLK_f:
                    key = 0xE;
                    break;
                case SDLK_v:
                    key = 0xF;
                    break;
            }
            keys[key] = state;
        }
    }
    return key;
}

void pwin_close(struct pixel_window *pwin)
{
    SDL_DestroyRenderer(pwin->ren);
    SDL_DestroyWindow(pwin->win);
    SDL_Quit();
}
