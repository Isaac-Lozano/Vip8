#include <SDL2/SDL.h>

struct pixel_window
{
    SDL_Window *win;
    SDL_Renderer *ren;
};

int pwin_init(struct pixel_window *pwin);
void pwin_draw_image(struct pixel_window *pwin, unsigned char* screen, int width, int height);
int pwin_event_loop(unsigned char *keys);
char pwin_wait_for_key(unsigned char *keys);
void pwin_close(struct pixel_window *pwin);
