#include "chip8.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>

using namespace std;

random_device              rd;
mt19937                    gen;
uniform_int_distribution<> distr(0, 255);

unsigned short shiftHex(unsigned short op, int pos) {
    return op >> 4 * pos;
}

unsigned short getRegX(unsigned short op) {
    return shiftHex(op & 0x0F00, 2);
}

unsigned short getRegY(unsigned short op) {
    return shiftHex(op & 0x00F0, 1);
}

int Chip8::initialize() {
    pc          = 0x200;
    opcode      = 0;
    I           = 0;
    sp          = 0;
    delay_timer = 0;
    sound_timer = 0;

    memset(gfx, 0, 64 * 32);
    memset(stack, 0, 2 * 16);
    memset(V, 0, 16);
    memset(memory, 0, 4096);

    // fontset
    for (int i = 0; i < CHIP8_FONT_SIZE; ++i) {
        memory[i + 0x50] = chip8_fontset[i];
    }

    gen.seed(rd());
    drawFlag = false;

    return 0;
}

int Chip8::loadGame(char *filename) {
    ifstream f_stream{filename, ios::binary | ios::ate};
    int      f_size{(int)f_stream.tellg()};
    char     rom_buf[f_size];

    f_stream.seekg(ios::beg);
    f_stream.read(rom_buf, f_size);
    f_stream.close();

    for (int i = 0; i < f_size; i++) {
        memory[i + 0x200] = rom_buf[i];
    }

    return 0;
}

int Chip8::emulateCycle() {
    opcode = memory[pc] << 8 | memory[pc + 1];

    switch (opcode & 0xF000) {
        // data registers
        case 0x6000:
            V[getRegX(opcode)]  = opcode & 0x00FF;
            pc                 += 2;
            break;
        case 0x7000:
            V[getRegX(opcode)] += opcode & 0x00FF;
            pc                 += 2;
            break;
        case 0x8000: {
            unsigned short ix = getRegX(opcode);
            unsigned short iy = getRegY(opcode);
            switch (opcode & 0x000F) {
                case 0x0000:
                    V[ix]  = V[iy];
                    pc    += 2;
                    break;
                case 0x0001:
                    V[ix] |= V[iy];
                    pc    += 2;
                    break;
                case 0x0002:
                    V[ix] &= V[iy];
                    pc    += 2;
                    break;
                case 0x0003:
                    V[ix] ^= V[iy];
                    pc    += 2;
                    break;
                case 0x0004:
                    if (V[ix] > 0xFF - V[iy]) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[ix] += V[iy];
                    pc    += 2;
                    break;
                case 0x0005:
                    if (V[ix] >= V[iy]) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[ix] -= V[iy];
                    pc    += 2;
                    break;
                case 0x0006: {
                    V[0xF]   = V[iy] & 0x1;
                    V[iy]  >>= 1;
                    V[ix]    = V[iy];
                    pc      += 2;
                    break;
                }
                case 0x0007:
                    if (V[ix] <= V[iy]) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[ix]  = V[iy] - V[ix];
                    pc    += 2;
                    break;
                case 0x000E: {
                    V[0xF]   = V[iy] >> 7;
                    V[iy]  <<= 1;
                    V[ix]    = V[iy];
                    pc      += 2;
                    break;
                }
                default:
                    printf("Failed register operation: 0x%X\n", opcode);
                    return -1;
            }
            break;
        }
        case 0xC000:
            V[getRegX(opcode)]  = distr(gen) & (opcode & 0x00FF);
            pc                 += 2;
            break;

        // jumps
        case 0x1000:
            pc = opcode & 0x0FFF;
            break;
        case 0xB000:
            pc = V[0] + (opcode & 0x0FFF);
            break;

        case 0x0000: {
            switch (opcode & 0x0FFF) {
                // clears the screen
                case 0x00E0:
                    memset(gfx, 0, 64 * 32);
                    pc += 2;
                    break;

                // return from subroutine
                case 0x00EE:
                    pc = stack[--sp];
                    break;

                // 0x0NNN considered legacy. just ignoring it
                default:
                    pc += 2;
            }
            break;
        }

        // calls subroutines
        case 0x2000:
            stack[sp++] = pc + 2;
            pc          = opcode & 0x0FFF;
            break;

        // conditional branching
        case 0x3000:
            if (V[getRegX(opcode)] == (opcode & 0x00FF)) {
                pc += 2;
            }
            pc += 2;
            break;
        case 0x4000:
            if (V[getRegX(opcode)] != (opcode & 0x00FF)) {
                pc += 2;
            }
            pc += 2;
            break;
        case 0x5000:
            if (V[getRegX(opcode)] == V[getRegY(opcode)]) {
                pc += 2;
            }
            pc += 2;
            break;
        case 0x9000:
            if (V[getRegX(opcode)] != V[getRegY(opcode)]) {
                pc += 2;
            }
            pc += 2;
            break;

        case 0xF000: {
            switch (opcode & 0x00FF) {
                // timer
                case 0x0007:
                    V[getRegX(opcode)]  = delay_timer;
                    pc                 += 2;
                    break;
                case 0x0015:
                    delay_timer  = V[getRegX(opcode)];
                    pc          += 2;
                    break;
                case 0x0018:
                    sound_timer  = V[getRegX(opcode)];
                    pc          += 2;
                    break;

                // TODO: key press operation
                case 0x000A:
                    break;

                // I register operation
                case 0x001E:
                    I  += V[getRegX(opcode)];
                    pc += 2;
                    break;

                // font operation
                case 0x0029:
                    I   = chip8_fontset[(V[getRegX(opcode)] & 0xF) * 5];
                    pc += 2;
                    break;

                // BCD operation
                case 0x0033:
                    memory[I]      = V[getRegX(opcode)] / 100;
                    memory[I + 1]  = V[getRegX(opcode)] / 10 % 10;
                    memory[I + 2]  = V[getRegX(opcode)] % 100 % 10;
                    pc            += 2;
                    break;

                // register + memory operation
                case 0x0055:
                    for (int i = 0; i <= getRegX(opcode); i++) {
                        memory[I + i] = V[i];
                    }
                    I  += getRegX(opcode) + 1;
                    pc += 2;
                    break;
                case 0x0065:
                    for (int i = 0; i <= getRegX(opcode); i++) {
                        V[i] = memory[I + i];
                    }
                    I  += getRegX(opcode) + 1;
                    pc += 2;
                    break;

                default:
                    printf("Failed unknown operation: 0x%X\n", opcode);
                    return -1;
            }
        }

        // TODO: keyboard input
        case 0xE000: {
            break;
        }

        // I register
        case 0xA000:
            I   = opcode & 0x0FFF;
            pc += 2;
            break;

        // display
        case 0xD000: {
            unsigned short x      = V[getRegX(opcode)];
            unsigned short y      = V[getRegY(opcode)];
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;

            V[0xF] = 0;
            for (int yline = 0; yline < height; yline++) {
                pixel = memory[I + yline];
                for (int xline = 0; xline < 8; xline++) {
                    if ((pixel & (0x80 >> xline)) != 0) {
                        if (gfx[(x + xline + ((y + yline) * 64))] == 1)
                            V[0xF] = 1;
                        gfx[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }

            drawFlag  = true;
            pc       += 2;
            break;
        }

        default:
            printf("Unknown opcode: 0x%X\n", opcode);
            return -1;
    }

    return 0;
}

int Chip8::tickTimers() {
    if (delay_timer > 0)
        --delay_timer;

    if (sound_timer > 0) {
        if (sound_timer == 1)
            printf("BEEP!\n");
        --sound_timer;
    }

    return 0;
}

unsigned char *Chip8::getGfx() {
    return gfx;
}
