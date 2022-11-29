#ifndef C8CORE_H
#define C8CORE_H

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

const unsigned char keyMap[4][4] = {
    {0x1, 0x2, 0x3, 0xC},
    {0x4, 0x5, 0x6, 0xD},
    {0x7, 0x8, 0x9, 0xE},
    {0xA, 0x0, 0xB, 0xF}
};

class C8Core {
    public:
        C8Core();
        int init();
        void runCycle();
        void draw();
        void debugDraw();
        void printStackTrace();

        unsigned char key[16];
    private:
        unsigned char ram[4096];
        unsigned char vram[SCREEN_WIDTH * SCREEN_HEIGHT];

        unsigned short stack[16];
        unsigned short sp;

        unsigned short opcode;

        unsigned char V[16];
        unsigned short index;
        unsigned short pc;     

        unsigned char t_delay;
        unsigned char t_sound;

        char drawReady;
};

#endif