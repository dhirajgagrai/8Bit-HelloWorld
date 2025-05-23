#ifndef CHIP8_H
#define CHIP8_H

#define CHIP8_FONT_SIZE 80

class Chip8 {
    private:
        unsigned short opcode;
        unsigned char  memory[4096];
        unsigned char  V[16];
        unsigned short I;
        unsigned short pc;
        unsigned char  gfx[64 * 32];
        unsigned char  delay_timer;
        unsigned char  sound_timer;
        unsigned short stack[16];
        unsigned short sp;
        unsigned char  key[16];

        static constexpr unsigned char chip8_fontset[CHIP8_FONT_SIZE] = {
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

    public:
        int            initialize();
        int            loadGame(char *filename);
        int            emulateCycle();
        int            tickTimers();
        int            setKey(unsigned char key_val);
        int            unsetKey(unsigned char key_val);
        unsigned char *getGfx();

        bool drawFlag;
};

#endif
