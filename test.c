#include "chip8.h"
#include "pwin.h"
#include <stdlib.h>
#include <stdio.h>

char codez[3584];

int update_chip(struct chip8 *chip, char redraw);
char wait_for_key(struct chip8 *chip);

int main(int argc, char* argv[])
{
    struct chip8 chip;
    struct pixel_window pwin;

    chip.end_of_cycle = update_chip;
    chip.get_key = wait_for_key;
    chip.ctx = (void *) &pwin;

    if(argc != 2)
    {
        exit(1);
    }
    FILE* fd = fopen(argv[1], "r");
    if(fd == NULL)
    {
        exit(2);
    }

    size_t program_length = fread(codez, sizeof(char), 3584, fd);

    if(pwin_init(&pwin))
    {
        exit(3);
    }

    chip8_initialize_system(&chip);
    chip8_load_program(&chip, codez, program_length);
    chip8_run(&chip);

    if(fclose(fd))
    {
        exit(3);
    }

    return 0;
}

int update_chip(struct chip8 *chip, char redraw)
{
    if(redraw)
    {
        pwin_draw_image((struct pixel_window *) chip->ctx, chip->screen, SCREEN_WIDTH, SCREEN_HEIGHT);
    }
    pwin_event_loop(chip->key);
    return 0;
}

char wait_for_key(struct chip8 *chip)
{
    return pwin_wait_for_key(chip->key);
}
