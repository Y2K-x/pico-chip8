#ifndef C8CORE_H
#define C8CORE_H

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

class C8Core {
    public:
        C8Core();
        int init();
        void runCycle();
        void draw();
        void setKeys();
        void printStackTrace();
    private:
        unsigned char vram[SCREEN_WIDTH * SCREEN_HEIGHT];
        unsigned char ram[4096];

        unsigned short stack[16];
        unsigned short sp;

        unsigned short opcode;

        unsigned char V[16];
        unsigned short index;
        unsigned short pc;     
        
        unsigned char key[16];

        unsigned char t_delay;
        unsigned char t_sound;
};

#endif