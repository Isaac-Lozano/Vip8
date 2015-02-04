#include "chip8.h"

#include <string.h>
#include <time.h>
#include <stdlib.h>

static int _timespec_subtract(struct timespec* result, struct timespec* x, struct timespec* y);

/* Memory map
   0x000 - 0x1FF - Chip 8 intrpreter (contains fron set in emu)
   0x050 - 0x0A0 - Used for built in 4x5 pixel font set (0 - F)
   0x200 - 0xFFF - Program ROM and work RAM
   */

unsigned char fontset[80] =
{ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


// Set memory and registers to 0
void chip8_initialize_system(struct chip8 *op_chip)
{
    memset(op_chip->memory, 0, MEMORY_SIZE * sizeof(unsigned char));
    memset(op_chip->V, 0, NUM_REGISTERS * sizeof(unsigned char));
    memset(op_chip->screen, 0, SCREEN_SIZE * sizeof(unsigned char));
    op_chip->I = 0x0;
    op_chip->pc = 0x200;
    op_chip->delay_timer = 0;
    op_chip->sound_timer = 0;
    op_chip->sp = 0;
    srand(time(NULL));
    for(int i = 0; i < 80; i++)
    {
        op_chip->memory[i] = fontset[i];
    }
    for(int i = 0; i < NUM_KEYS; i++)
    {
        op_chip->key[i] = 0;
    }
}

static void _chip8_done(const char* reason, unsigned short opcode)
{
    fprintf(stderr, "Error [0x%4X]: %s\n", opcode, reason);
    exit(1);
}

// Put program into memory
void chip8_load_program(struct chip8 *op_chip, char* buffer, size_t buf_size)
{
    for(int i = 0; i < buf_size; i++)
    {
        op_chip->memory[0x200 + i] = buffer[i]; // Program memory starts at 0x200
    }
}

// Emulate chip8
void chip8_run(struct chip8 *op_chip)
{
    struct timespec time_start, time_end, time_diff;
    struct timespec time_60_htz;
    unsigned short opcode;
    char redraw = 0;

    time_60_htz.tv_sec = 0;
    time_60_htz.tv_nsec = 1666666;

    for(;;)
    {

//        if(clock_gettime(CLOCK_MONOTONIC, &time_start))
//        {
//            _chip8_done("Cannot get time", opcode);
//        }

        // Get next opcode
        opcode = op_chip->memory[op_chip->pc] << 8 | op_chip->memory[op_chip->pc + 1];

        switch(opcode & 0xF000)
        {
            case 0x0000:
                    // Clear screen
                switch(opcode)
                {
                    case 0x00E0: // 0x00E0 - CLS
                        // Clear Screen
                        memset(op_chip->screen, 0, SCREEN_SIZE * sizeof(unsigned char));
                        op_chip->pc += 2;
                        break;

                    case 0x00EE: // 0x00EE - RET
                        // Return
                        op_chip->sp--;
                        op_chip->pc = op_chip->stack[op_chip->sp];
                        op_chip->pc += 2;
                        break;

//                    default:
//                        printf("ERROR [0x%X]: The SYS opcode is not supported.\n",  opcode);
//                        exit(1);
                }
                break;

            case 0x1000: // 0x1nnn - JP
                // Jump to address 0x0nnn
                op_chip->pc = opcode & 0x0FFF;
                break;

            case 0x2000: // 0x2nnn - CALL
                // Call subroutine at 0x0nnn
                op_chip->stack[op_chip->sp] = op_chip->pc;
                op_chip->sp++;
                op_chip->pc = opcode & 0x0FFF;
                break;

            case 0x3000: // 0x3xkk - SE Vx, byte
                // Skip next opcode if Vx == 0xkk
                if( op_chip->V[ (opcode & 0x0F00) >> 8 ] == (opcode & 0x00FF) )
                {
                    op_chip->pc += 2;
                }
                op_chip->pc += 2;
                break;

            case 0x4000: // 0x4xkk - SNE Vx, byte
                //Skip next opcode if Vx != 0xkk
                if( op_chip->V[ (opcode & 0x0F00) >> 8 ] != (opcode & 0x00FF) )
                {
                    op_chip->pc += 2;
                }
                op_chip->pc += 2;
                break;

            case 0x5000: // 0x5xy0 - SE Vx, Vy
                //Skip next opcode if Vx == Vy
                if( op_chip->V[ (opcode & 0x0F00) >> 8 ] == op_chip->V[ (opcode & 0x00F0) >> 4 ] )
                {
                    op_chip->pc += 2;
                }
                op_chip->pc += 2;
                break;

            case 0x6000: // 0x6xkk - LD Vx, byte
                // Set Vx to 0xkk
                op_chip->V[ (opcode & 0x0F00) >> 8 ] = opcode & 0x00FF;
                op_chip->pc += 2;
                break;

            case 0x7000: // 0x7xkk - ADD Vx, byte
                // Add byte to Vx and set Vx to result
                op_chip->V[ (opcode & 0x0F00) >> 8 ] += opcode & 0x00FF;
                op_chip->pc += 2;
                break;

            case 0x8000:

                switch(opcode & 0x000F)
                {
                    case 0x0000: // 0x8xy0 - LD Vx, Vy
                        // Stores Vy in Vx
                        op_chip->V[ (opcode & 0x0F00) >> 8 ] = op_chip->V[ (opcode & 0x00F0) >> 4 ];
                        op_chip->pc += 2;
                        break;

                    case 0x0001: // 0x8xy1 - OR Vx, Vy
                        // ORs Vx and Vy and stores in Vx
                        op_chip->V[ (opcode & 0x0F00) >> 8 ] |= op_chip->V[ (opcode & 0x00F0) >> 4 ];
                        op_chip->pc += 2;
                        break;

                    case 0x0002: // 0x8xy2 - AND Vx, Vy
                        // ANDs Vx and Vy and stores in Vx
                        op_chip->V[ (opcode & 0x0F00) >> 8 ] &= op_chip->V[ (opcode & 0x00F0) >> 4 ];
                        op_chip->pc += 2;
                        break;

                    case 0x0003: // 0x8xy3 - XOR Vx, Vy
                        // XORs Vx and Vy and stores in Vx
                        op_chip->V[ (opcode & 0x0F00) >> 8 ] ^= op_chip->V[ (opcode & 0x00F0) >> 4 ];
                        op_chip->pc += 2;
                        break;

                    case 0x0004: // 0x8xy4 - ADD Vx, Vy
                        // ADDs Vy to Vx. Sets VF if carry
                        if( op_chip->V[ (opcode & 0x0F00) >> 8 ] > (0x00FF - op_chip->V[ (opcode & 0x00F0) >> 4]) )
                        {
                            op_chip->V[0xF] = 1;
                        }
                        else
                        {
                            op_chip->V[0xF] = 0;
                        }
                        op_chip->V[ (opcode & 0x0F00) >> 8 ] += op_chip->V[ (opcode & 0x00F0) >> 4 ];
                        op_chip->pc += 2;
                        break;

                    case 0x0005: // 0x8xy5 - SUB Vx, Vy
                        // Subtracts Vy from Vx and stores in Vx. Sets VF if NOT borrow
                        if( op_chip->V[ (opcode & 0x0F00) >> 8 ] < op_chip->V[ (opcode & 0x00F0) >> 4] )
                        {
                            op_chip->V[0xF] = 0;
                        }
                        else
                        {
                            op_chip->V[0xF] = 1;
                        }
                        op_chip->V[ (opcode & 0x0F00) >> 8 ] -= op_chip->V[ (opcode & 0x00F0) >> 4 ];
                        op_chip->pc += 2;
                        break;

                    case 0x0006: // 0x8xy6 - SHR Vx {, Vy}
                        // Puts LSB in VF and shifts Vx right by one
                        op_chip->V[0xF] = op_chip->V[ (opcode & 0x0F00) >> 8 ] & 0x0001;
                        op_chip->V[ (opcode & 0x0F00) >> 8 ] >>= 1;
                        op_chip->pc += 2;
                        break;

                    case 0x0007: // 0x8xy7 - SUBN Vx, Vy
                        // Subtracts Vx from Vy and stores in Vx. Sets VF if NOT borrow
                        if( op_chip->V[ (opcode & 0x00F0) >> 4 ] < op_chip->V[ (opcode & 0x0F00) >> 8] )
                        {
                            op_chip->V[0xF] = 0;
                        }
                        else
                        {
                            op_chip->V[0xF] = 1;
                        }
                        op_chip->V[ (opcode & 0x0F00) >> 8 ] = op_chip->V[ (opcode & 0x00F0) >> 4 ] - op_chip->V[ (opcode & 0x0F00) >> 8 ];
                        op_chip->pc += 2;
                        break;

                    case 0x000E: // 0x8xyE - SHL Vx {, Vy}
                        // Puts MSB in VF and shifts Vx left by one
                        op_chip->V[0xF] = op_chip->V[ (opcode & 0x0F00) >> 8 ] >> 7;
                        op_chip->V[ (opcode & 0x0F00) >> 8 ] <<= 1;
                        op_chip->pc += 2;
                        break;

                    default:
                        _chip8_done("Opcode not found", opcode);
                }
                break;

            case 0x9000: // 0x9xy0 - SNE Vx, Vy
                // Skip next opcode if Vx != Vy
                if( op_chip->V[ (opcode & 0x0F00) >> 8 ] != op_chip->V[ (opcode & 0x00F0) >> 4] )
                {
                    op_chip->pc += 2;
                }
                op_chip->pc += 2;
                break;

            case 0xA000: // 0xAnnn - LD I, addr
                // Sets I to 0xnnn
                op_chip->I = opcode & 0x0FFF;
                op_chip->pc += 2;
                break;

            case 0xB000: // 0xBnnn - JP V0, addr
                // Sets pc to 0xnnn + V0
                op_chip->pc = (opcode & 0x0FFF) + op_chip->V[0x0];
                break;

            case 0xC000: // 0xCxkk - RND Vx, byte
                // Sets Vx to a random number ANDed by 0xkk
                op_chip->V[ (opcode & 0x0F00) >> 8 ] = rand() & opcode & 0x00FF;
                op_chip->pc += 2;
                break;

            case 0xD000: // 0xDxyn - DRW Vx, Vy, nibble
                // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
                redraw = 1;
                op_chip->V[0xF] = 0;
                unsigned int x_major = op_chip->V[(opcode & 0x0F00) >> 8];
                unsigned int y_major = op_chip->V[(opcode & 0x00F0) >> 4];
                for(int y = 0; y < (opcode & 0x000F); y++)
                {
                    for(int x = 0; x < 8; x++)
                    {
                        /* If there is a pixel to update and it isn't off the bottom of the screen */
                        if((op_chip->memory[op_chip->I + y] & (0x80 >> x)) &&
                           (y_major + y + ((x_major + x) / SCREEN_WIDTH)) < SCREEN_HEIGHT)
                        {
                            if(op_chip->screen[(y_major + y) * SCREEN_WIDTH + x_major + x])
                            {
                                op_chip->V[0xF] = 1;
                            }
                            op_chip->screen[(y_major + y) * SCREEN_WIDTH + x_major + x] ^= 1;
                        }
                    }
                }
                op_chip->pc += 2;
                break;

            case 0xE000:
                switch(opcode & 0x00FF)
                {
                    case 0x009E: // 0xEx9E - SKP Vx
                        // Skip next instruction if key Vx is pressed
                        if( op_chip->key[ op_chip->V[ (opcode & 0x0F00) >> 8 ] ] != 0 )
                        {
                            op_chip->pc += 2;
                        }
                        op_chip->pc += 2;
                        break;

                    case 0x00A1: // SKNP Vx
                        // Skip next instruction if key Vx is NOT pressed
                        if( op_chip->key[ op_chip->V[ (opcode & 0x0F00) >> 8 ] ] == 0 )
                        {
                            op_chip->pc += 2;
                        }
                        op_chip->pc += 2;
                        break;

                    default:
                        _chip8_done("Opcode not found", opcode);
                }
                break;

            case 0xF000:
                switch(opcode & 0x00FF)
                {
                    case 0x0007: // 0xFx07 - LD Vx, DT
                        // Puts the value of the delay timer in Vx
                        op_chip->V[ (opcode & 0x0F00) >> 8 ] = op_chip->delay_timer;
                        op_chip->pc += 2;
                        break;

                    case 0x000A: // 0xFx0A - LD Vx, K
                        // Waits for a keypress and puts the value in Vx
                        op_chip->V[ (opcode & 0x0F00) >> 8 ] = op_chip->get_key(op_chip);
                        op_chip->pc += 2;
                        break;

                    case 0x0015: // 0xFx15 - LD DT, Vx
                        // Sets delay timer to Vx
                        op_chip->delay_timer = op_chip->V[ (opcode & 0x0F00) >> 8 ];
                        op_chip->pc += 2;
                        break;

                    case 0x0018: // 0xFx18 - LD ST, Vx
                        // Sets sound timer to Vx
                        op_chip->sound_timer = op_chip->V[ (opcode & 0x0F00) >> 8 ];
                        op_chip->pc += 2;
                        break;

                    case 0x001E: // 0xFx1E - ADD I, Vx
                        // Increments I by Vx
                        if(op_chip->I + op_chip->V[ (opcode & 0x0F00) >> 8 ] > 0xFFF)
                        {
                            op_chip->V[0xF] = 1;
                        }
                        else
                        {
                            op_chip->V[0xF] = 0;
                        }
                        op_chip->I += op_chip->V[ (opcode & 0x0F00) >> 8 ];
                        op_chip->pc += 2;
                        break;

                    case 0x0029: // 0xFx29 - LD F, Vx
                        // Set I to the location of the sprite for digit Vx
                        op_chip->I = 5 * op_chip->V[ ((opcode & 0x0F00) >> 8) ];
                        op_chip->pc += 2;
                        break;

                    case 0x0033: // 0xFx33 - LD B, Vx
                        // Stores the BCD representation of Vx in memory locations I, I+1, I+2
                        op_chip->memory[op_chip->I] = op_chip->V[ (opcode & 0x0F00) >> 8 ] / 100;
                        op_chip->memory[op_chip->I + 1] = op_chip->V[ (opcode & 0x0F00) >> 8 ] / 10 % 10;
                        op_chip->memory[op_chip->I + 2] = op_chip->V[ (opcode & 0x0F00) >> 8 ] % 10;
                        op_chip->pc += 2;
                        break;

                    case 0x0055: // 0xFx55 - LD [I], Vx
                        // Stores registers V0 through Vx starting at address I
                        for(int i = 0; i <= (opcode & 0x0F00) >> 8; i++)
                        {
                            op_chip->memory[op_chip->I + i] = op_chip->V[i];
                        }
                        op_chip->pc += 2;
                        break;

                    case 0x0065: // 0xFx65 - LD Vx, [I]
                        // Reads registers V0 through Vx starting at address I
                        for(int i = 0; i <= (opcode & 0x0F00) >> 8; i++)
                        {
                            op_chip->V[i] = op_chip->memory[op_chip->I + i];
                        }
                        op_chip->pc += 2;
                        break;

                    default:
                        _chip8_done("Opcode not found", opcode);
                }
                break;
        }


        if(op_chip->delay_timer > 0)
        {
            --op_chip->delay_timer;
        }
        if(op_chip->sound_timer > 0)
        {
            --op_chip->sound_timer;
        }

        op_chip->end_of_cycle(op_chip, redraw);
        redraw = 0;

//        if(clock_gettime(CLOCK_MONOTONIC, &time_end))
//        {
//            _chip8_done("Cannot get time end", opcode);
//        }
//
//        if(_timespec_subtract(&time_diff, &time_end, &time_start))
//        {
//            _chip8_done("Cannot get time diff", opcode);
//        }
//
//        if(_timespec_subtract(&time_diff, &time_60_htz, &time_diff))
//        {
//            continue;
//        }
//
//        if(nanosleep(&time_diff, NULL))
//        {
//            continue;
//        }
    }
}

/* Subtract the `struct timeval' values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0. */

static int _timespec_subtract(struct timespec* result, struct timespec* x, struct timespec* y)
{

  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_nsec < y->tv_nsec) {
    int nsec = (y->tv_nsec - x->tv_nsec) / 1000000000 + 1;
    y->tv_nsec -= 1000000000 * nsec;
    y->tv_sec += nsec;
  }

  if (x->tv_nsec - y->tv_nsec > 1000000000) {
    int nsec = (x->tv_nsec - y->tv_nsec) / 1000000000;
    y->tv_nsec += 1000000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_nsec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_nsec = x->tv_nsec - y->tv_nsec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}
