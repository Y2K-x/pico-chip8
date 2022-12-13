#ifndef C8CORE_H
#define C8CORE_H

#include "ssd1306.h"
#include "sd_io.h"

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

const uint8_t keyMap[4][4] = {
    {0x1, 0x2, 0x3, 0xC},
    {0x4, 0x5, 0x6, 0xD},
    {0x7, 0x8, 0x9, 0xE},
    {0xA, 0x0, 0xB, 0xF}
};

const int row_pins[4] = {0,1,2,3};
const int col_pins[4] = {7,6,5,4};

class C8Core {
    public:
        C8Core();
        void init(SSD1306 *display);
        void runCycle();
        void updateTimers();
        void draw();
        void debugDraw();
        void loadROM(uint8_t *rom, uint32_t size);
        void pollInput();
        void printStackTrace();

        
    private:
        SSD1306 *display;

        uint8_t ram[4096];
        uint8_t vram[SCREEN_WIDTH * SCREEN_HEIGHT];

        uint16_t stack[16];
        uint16_t sp;

        uint16_t opcode;

        uint8_t V[16];
        uint16_t index;
        uint16_t pc;

        uint8_t key[16];

        uint8_t t_delay;
        uint8_t t_sound;
        uint8_t beepCounter;

        uint8_t drawReady;
        
};

#endif