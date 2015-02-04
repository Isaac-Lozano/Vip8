#include <stdio.h>

#define MEMORY_SIZE 4096
#define NUM_REGISTERS 16
#define SCREEN_WIDTH  64
#define SCREEN_HEIGHT 32
#define SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)
#define STACK_SIZE    16
#define NUM_KEYS      16

struct chip8
{
    unsigned char memory[MEMORY_SIZE];
    unsigned char V[NUM_REGISTERS];
    unsigned short I;
    unsigned short pc;

    unsigned char screen[SCREEN_SIZE];

    unsigned char delay_timer;
    unsigned char sound_timer;

    unsigned short stack[STACK_SIZE];
    unsigned short sp;

    unsigned char key[NUM_KEYS];

    /* Callbacks */
    int (*end_of_cycle) (struct chip8 *op_chip, char redraw);
    char (*get_key) (struct chip8 *op_chip);

    void *ctx;
};

void chip8_initialize_system(struct chip8 *op_chip);
void chip8_load_program(struct chip8 *op_chip, char* buffer, size_t buf_size);
void chip8_run(struct chip8 *op_chip);
