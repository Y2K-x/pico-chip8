/*
pico-chip8 Emulator Core
Written by Y2K for the Bitbuilt 2022 Secret Santa (https://bitbuilt.net/forums/index.php?threads/bitbuilts-2022-secret-santa.5384/)
Some opcode implementations borrowed from here due to time restraints: https://github.com/JamesGriffin/CHIP-8-Emulator/blob/master/src/chip8.cpp
*/

#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "ssd1306.h"
#include "c8core.h"

//comment to disable printing to serial when CPU core encounters an unknown opcode
//#define PRINT_UNK_OPCODE

//Standard CHIP-8 font
const unsigned char font[80] = {
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

//opcode test ROM
/*
const uint8_t data[478] = {
    0x12, 0x4E, 0xEA, 0xAC, 0xAA, 0xEA, 0xCE, 0xAA, 0xAA, 0xAE, 0xE0, 0xA0, 0xA0, 0xE0, 0xC0, 0x40, 
    0x40, 0xE0, 0xE0, 0x20, 0xC0, 0xE0, 0xE0, 0x60, 0x20, 0xE0, 0xA0, 0xE0, 0x20, 0x20, 0x60, 0x40, 
    0x20, 0x40, 0xE0, 0x80, 0xE0, 0xE0, 0xE0, 0x20, 0x20, 0x20, 0xE0, 0xE0, 0xA0, 0xE0, 0xE0, 0xE0, 
    0x20, 0xE0, 0x40, 0xA0, 0xE0, 0xA0, 0xE0, 0xC0, 0x80, 0xE0, 0xE0, 0x80, 0xC0, 0x80, 0xA0, 0x40, 
    0xA0, 0xA0, 0xA2, 0x02, 0xDA, 0xB4, 0x00, 0xEE, 0xA2, 0x02, 0xDA, 0xB4, 0x13, 0xDC, 0x68, 0x01, 
    0x69, 0x05, 0x6A, 0x0A, 0x6B, 0x01, 0x65, 0x2A, 0x66, 0x2B, 0xA2, 0x16, 0xD8, 0xB4, 0xA2, 0x3E, 
    0xD9, 0xB4, 0xA2, 0x02, 0x36, 0x2B, 0xA2, 0x06, 0xDA, 0xB4, 0x6B, 0x06, 0xA2, 0x1A, 0xD8, 0xB4, 
    0xA2, 0x3E, 0xD9, 0xB4, 0xA2, 0x06, 0x45, 0x2A, 0xA2, 0x02, 0xDA, 0xB4, 0x6B, 0x0B, 0xA2, 0x1E, 
    0xD8, 0xB4, 0xA2, 0x3E, 0xD9, 0xB4, 0xA2, 0x06, 0x55, 0x60, 0xA2, 0x02, 0xDA, 0xB4, 0x6B, 0x10, 
    0xA2, 0x26, 0xD8, 0xB4, 0xA2, 0x3E, 0xD9, 0xB4, 0xA2, 0x06, 0x76, 0xFF, 0x46, 0x2A, 0xA2, 0x02, 
    0xDA, 0xB4, 0x6B, 0x15, 0xA2, 0x2E, 0xD8, 0xB4, 0xA2, 0x3E, 0xD9, 0xB4, 0xA2, 0x06, 0x95, 0x60, 
    0xA2, 0x02, 0xDA, 0xB4, 0x6B, 0x1A, 0xA2, 0x32, 0xD8, 0xB4, 0xA2, 0x3E, 0xD9, 0xB4, 0x22, 0x42, 
    0x68, 0x17, 0x69, 0x1B, 0x6A, 0x20, 0x6B, 0x01, 0xA2, 0x0A, 0xD8, 0xB4, 0xA2, 0x36, 0xD9, 0xB4, 
    0xA2, 0x02, 0xDA, 0xB4, 0x6B, 0x06, 0xA2, 0x2A, 0xD8, 0xB4, 0xA2, 0x0A, 0xD9, 0xB4, 0xA2, 0x06, 
    0x87, 0x50, 0x47, 0x2A, 0xA2, 0x02, 0xDA, 0xB4, 0x6B, 0x0B, 0xA2, 0x2A, 0xD8, 0xB4, 0xA2, 0x0E, 
    0xD9, 0xB4, 0xA2, 0x06, 0x67, 0x2A, 0x87, 0xB1, 0x47, 0x2B, 0xA2, 0x02, 0xDA, 0xB4, 0x6B, 0x10, 
    0xA2, 0x2A, 0xD8, 0xB4, 0xA2, 0x12, 0xD9, 0xB4, 0xA2, 0x06, 0x66, 0x78, 0x67, 0x1F, 0x87, 0x62, 
    0x47, 0x18, 0xA2, 0x02, 0xDA, 0xB4, 0x6B, 0x15, 0xA2, 0x2A, 0xD8, 0xB4, 0xA2, 0x16, 0xD9, 0xB4, 
    0xA2, 0x06, 0x66, 0x78, 0x67, 0x1F, 0x87, 0x63, 0x47, 0x67, 0xA2, 0x02, 0xDA, 0xB4, 0x6B, 0x1A, 
    0xA2, 0x2A, 0xD8, 0xB4, 0xA2, 0x1A, 0xD9, 0xB4, 0xA2, 0x06, 0x66, 0x8C, 0x67, 0x8C, 0x87, 0x64, 
    0x47, 0x18, 0xA2, 0x02, 0xDA, 0xB4, 0x68, 0x2C, 0x69, 0x30, 0x6A, 0x34, 0x6B, 0x01, 0xA2, 0x2A, 
    0xD8, 0xB4, 0xA2, 0x1E, 0xD9, 0xB4, 0xA2, 0x06, 0x66, 0x8C, 0x67, 0x78, 0x87, 0x65, 0x47, 0xEC, 
    0xA2, 0x02, 0xDA, 0xB4, 0x6B, 0x06, 0xA2, 0x2A, 0xD8, 0xB4, 0xA2, 0x22, 0xD9, 0xB4, 0xA2, 0x06, 
    0x66, 0xE0, 0x86, 0x6E, 0x46, 0xC0, 0xA2, 0x02, 0xDA, 0xB4, 0x6B, 0x0B, 0xA2, 0x2A, 0xD8, 0xB4, 
    0xA2, 0x36, 0xD9, 0xB4, 0xA2, 0x06, 0x66, 0x0F, 0x86, 0x66, 0x46, 0x07, 0xA2, 0x02, 0xDA, 0xB4, 
    0x6B, 0x10, 0xA2, 0x3A, 0xD8, 0xB4, 0xA2, 0x1E, 0xD9, 0xB4, 0xA3, 0xE8, 0x60, 0x00, 0x61, 0x30, 
    0xF1, 0x55, 0xA3, 0xE9, 0xF0, 0x65, 0xA2, 0x06, 0x40, 0x30, 0xA2, 0x02, 0xDA, 0xB4, 0x6B, 0x15, 
    0xA2, 0x3A, 0xD8, 0xB4, 0xA2, 0x16, 0xD9, 0xB4, 0xA3, 0xE8, 0x66, 0x89, 0xF6, 0x33, 0xF2, 0x65, 
    0xA2, 0x02, 0x30, 0x01, 0xA2, 0x06, 0x31, 0x03, 0xA2, 0x06, 0x32, 0x07, 0xA2, 0x06, 0xDA, 0xB4, 
    0x6B, 0x1A, 0xA2, 0x0E, 0xD8, 0xB4, 0xA2, 0x3E, 0xD9, 0xB4, 0x12, 0x48, 0x13, 0xDC
};
*/

//keypad test ROM
const uint8_t data[114] = {
    0x12, 0x4E, 0x08, 0x19, 0x01, 0x01, 0x08, 0x01, 0x0F, 0x01, 0x01, 0x09, 0x08, 0x09, 0x0F, 0x09, 
    0x01, 0x11, 0x08, 0x11, 0x0F, 0x11, 0x01, 0x19, 0x0F, 0x19, 0x16, 0x01, 0x16, 0x09, 0x16, 0x11, 
    0x16, 0x19, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0x00, 0xA2, 0x02, 0x82, 0x0E, 0xF2, 0x1E, 
    0x82, 0x06, 0xF1, 0x65, 0x00, 0xEE, 0xA2, 0x02, 0x82, 0x0E, 0xF2, 0x1E, 0x82, 0x06, 0xF1, 0x55, 
    0x00, 0xEE, 0x6F, 0x10, 0xFF, 0x15, 0xFF, 0x07, 0x3F, 0x00, 0x12, 0x46, 0x00, 0xEE, 0x00, 0xE0, 
    0x62, 0x00, 0x22, 0x2A, 0xF2, 0x29, 0xD0, 0x15, 0x70, 0xFF, 0x71, 0xFF, 0x22, 0x36, 0x72, 0x01, 
    0x32, 0x10, 0x12, 0x52, 0xF2, 0x0A, 0x22, 0x2A, 0xA2, 0x22, 0xD0, 0x17, 0x22, 0x42, 0xD0, 0x17, 
    0x12, 0x64
};

C8Core::C8Core() {}

void C8Core::init(SSD1306 *display) {
    //init clear display
    this->display = display;

    //init registers and memory
    pc = 0x200;
    opcode = 0;
    index = 0;
    sp = 0;

    //clear vram, stack, ram, and registers
    memset(vram, 0, sizeof vram);
    memset(stack, 0, sizeof stack);
    memset(ram, 0, sizeof ram);
    memset(V, 0, sizeof V);
    memset(key, 0, sizeof key);

    //load fontset, which must be resident in chip8 ram due to some shenannigans some programs do with it
    for(int i = 0; i < 0x50; i++) {
        ram[i] = font[i];
    }

    //reset timers
    t_delay = 0x0;
    t_sound = 0x0;

    //load ROM, also must be resident in RAM, chip8 is weird, yes programs can and will override themselves
    for(int i = 0; i < file->filesize; i++) {
        ram[i + 0x200] = (uint8_t)rom[i];
    }

    return;
}

void C8Core::runCycle() {
    //fetch opcode
    opcode = ram[pc] << 8 | ram[pc + 1];

    //decode opcode
    switch(opcode & 0xF000) {
        case 0x0000: {
            switch(opcode & 0x00FF) {
                case 0x00E0: { //00E0 - CLS - Clear Screen
                    memset(vram, 0, sizeof vram); //clear vram
                    pc += 2;
                    break;
                }

                case 0x00EE: { //00EE - RET - Return from Subroutine
                    sp--;
                    pc = stack[sp];
                    pc += 2;
                    break;
                }

                
                default: {
#ifdef PRINT_UNK_OPCODE
                    printf("Unknown Opcode: 0x%X\n", opcode);
#endif
                    break;
                }
            }
            break;
        }

        case 0x1000: { //1nnn - JP addr - Jump to location nnn
            pc = opcode & 0x0FFF;
            break;
        }

        case 0x2000: { //2nnn - CALL addr - Call subroutine at nnn
            stack[sp] = pc;
            sp++;
            pc = opcode & 0x0FFF;
            break;
        }

        case 0x3000: { //3xkk - SE Vx, byte - Skip next instruction if Vx = KK
            if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
                pc += 2;
            pc += 2;
            break;
        }

        case 0x4000: { //4xkk - SNE Vx, byte - Skip next instruction if Vx != KK
            if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                pc += 2;
            pc += 2;
            break;
        }

        case 0x5000: { //5xy0 - SE Vx, Vy - Skip next instruction if Vx = Vy
            if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
                pc += 2;
            pc += 2;
            break;
        }

        case 0x6000: { //6xkk - LD Vx, byte - Set Vx = kk
            V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
            pc += 2;
            break;
        }

        case 0x7000: { //7xkk - ADD Vx, byte - Set Vx = Vx + kk
            V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
            pc += 2;
            break;
        }

        case 0x8000: {
            switch(opcode & 0x000F) {
                case 0x0000: { //8xy0 - LD Vx, Vy - Set Vx = Vy
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                }

                case 0x0001: { //8xy1 - OR Vx, Vy - Set Vx = Vx OR Vy
                    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                }

                case 0x0002: { //8xy2 - AND Vx, Vy - Set Vx = Vx AND Vy
                    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                }

                case 0x0003: { //8xy3 - XOR Vx, Vy - Set Vx = Vx XOR Vy
                    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                }

                case 0x0004: { //8xy4 - ADD Vx, Vy - Set Vx = Vx - Vy, set VF = NOT borrow
                    if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode && 0x0F00) >> 8]))
                        V[0xF] = 1; //carry
                    else
                        V[0xF] = 0; //no carry
                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                    pc += 2; 
                    break;
                }

                case 0x0005: { //8xy5 - SUB Vx, Vy - Set Vx = Vx - Vy, set VF = NOT borrow.
                    if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                }

                case 0x0006: { //8xy6 - SHR Vx {, Vy} - Set Vx = Vx SHR 1.
                    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
                    V[(opcode & 0x0F00) >> 8] >>= 1;
                    pc += 2;
                    break;
                }

                case 0x0007: { //8xy7 - SUBN Vx, Vy - Set Vx = Vy - Vx, set VF = NOT borrow.
                    if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                }

                case 0x000E: { //8xyE - SHL Vx {, Vy} - Set Vx = Vx SHL 1.
                    V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
                    V[(opcode & 0x0F00) >> 8] <<= 1;
                    pc += 2;
                    break;
                }
                    
                default: {
#ifdef PRINT_UNK_OPCODE
                    printf("Unknown Opcode: 0x%X\n", opcode);
#endif
                    break;
                }
            }
            break;
        }

        case 0x9000: { //9xy0 - SNE Vx, Vy - Skip next instruction if Vx != Vy.
            if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
                pc += 2;
            pc += 2;
            break;
        }

        case 0xA000: { //ANNN - LD I, addr - Sets I to the address NNN
            index = (opcode & 0x0FFF);
            pc += 2;
            break;
        }

        case 0xB000: { //Bnnn - JP V0, addr - Jump to location nnn + V0
            pc = (opcode & 0x0FFF) + V[0];
            break;
        }

        case 0xC000: { //Cxkk - RND Vx, byte - Set Vx = random byte AND kk.
            V[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF);
            pc += 2;
            break;
        }

        case 0xD000: { //Dxyn - DRW Vx, Vy, nibble - Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
            uint16_t x = V[(opcode & 0x0F00) >> 8];
            uint16_t y = V[(opcode & 0x00F0) >> 4];
            uint16_t h = (opcode & 0x000F);
            uint16_t px;

            V[0xF] = 0;
            for (int yline = 0; yline < h; yline++) {
                px = ram[index + yline];
                for(int xline = 0; xline < 8; xline++) {
                    if((px & (0x80 >> xline)) != 0) {
                        if(vram[(x + xline + ((y + yline) * SCREEN_WIDTH))] == 1){
                            V[0xF] = 1;
                        }
                        vram[x + xline + ((y + yline) * SCREEN_WIDTH)] ^= 1;
                    }
                }
            }
            drawReady = 1;
            pc += 2;
        }

        case 0xE000: { //Ex9E - SKP Vx - Skip next instruction if key with the value of Vx is pressed.
            switch (opcode & 0x00FF) {
                case 0x009E: {
                    if(key[V[(opcode & 0x0F00) >> 8]] != 0)
                        pc += 2;
                    pc += 2;
                }

                case 0x00A1: { //ExA1 - SKNP Vx - Skip next instruction if key with the value of Vx is not pressed.
                    if (key[V[(opcode & 0x0F00) >> 8]] == 0)
                        pc +=  2;
                    pc += 2;
                    break;
                }

                default: {
#ifdef PRINT_UNK_OPCODE
                    printf("Unknown Opcode: 0x%X\n", opcode);
#endif
                    break;
                }
            }
            
            break;
        }

        case 0xF000: {
            switch(opcode & 0x00FF) {
                case 0x0007: { //Fx07 - LD Vx, DT - Set Vx = delay timer value.
                    V[(opcode & 0x0F00) >> 8] = t_delay;
                    pc += 2;
                    break;
                }

                case 0x000A: { //Fx0A - LD Vx, K - Wait for a key press, store the value of the key in Vx.
                    bool keyDown = false;
                    for(int i = 0; i < 16; i++) {
                        if(key[i] != 0) {
                            V[(opcode & 0x0F00) >> 8] = i;
                            keyDown = true;
                        }
                    }
                    if(!keyDown)
                        return;
                    pc += 2;
                    break;
                }

                case 0x0015: { //Fx15 - LD DT, Vx - Set delay timer = Vx.
                    t_delay = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                }

                case 0x0018: { //Fx18 - LD ST, Vx - Set sound timer = Vx.
                    t_sound = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                }

                case 0x001E: { //Fx1E - ADD I, Vx - Set I = I + Vx.
                    if(index + V[(opcode & 0x0F00) >> 8] > 0xFFF)
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
                    index += V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                }

                case 0x0029: { //Fx29 - LD F, Vx - Set I = location of sprite for digit Vx.
                    index = V[(opcode & 0x0F00) >> 8] * 0x5;
                    pc += 2;
                    break;
                }

                case 0x0033: { //Fx33 - LD B, Vx - Store BCD representation of Vx in memory locations I, I+1, and I+2.
                    ram[index] = V[(opcode & 0x0F00) >> 8] / 100;
                    ram[index + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
                    ram[index + 2] = V[(opcode & 0x0F00) >> 8] % 10;
                    pc += 2;
                    break;
                }

                case 0x0055: { //Fx55 - LD [I], Vx - Store registers V0 through Vx in memory starting at location I.
                    for(int i = 0; i <= (opcode & 0x0F00) >> 8; i++) {
                        ram[index + i] = V[i];
                    }
                    index += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;
                }

                case 0x0065: { //Fx65 - LD Vx, [I] - Read registers V0 through Vx from memory starting at location I.
                    for(int i = 0; i <= (opcode & 0x0F00) >> 8; i++) {
                        V[i] = ram[index + i];
                    }
                    index += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;
                }

                default: {
#ifdef PRINT_UNK_OPCODE
                    printf("Unknown Opcode: 0x%X\n", opcode);
#endif
                    break;
                }
            }
            break;
        }

        default: {
#ifdef PRINT_UNK_OPCODE
            printf("Unknown Opcode: 0x%X\n", opcode);
#endif
            break;
        }
    }

    return;
}

void C8Core::updateTimers() {
    //update delay timer
    if(t_delay > 0)
        --t_delay;

    //update sound timer & beep the speaker
    if(t_sound > 0) {
        gpio_put(8, true);
        --t_sound;
    }
    else
        gpio_put(8, false);
}

void C8Core::draw() {
    if(drawReady) {
        display->clear(); //clear display

        //iterate through CHIP-8 VRAM
        for(int y = 0; y < SCREEN_HEIGHT; y++) {
            for(int x = 0; x < SCREEN_WIDTH; x++) {
                //check if VRAM position contains an on pixel
                if(vram[(y * SCREEN_WIDTH) + x] > 0) {
                    display->draw_pixel(x * 2, y * 2, SSD1306_COLOR_ON);
                    display->draw_pixel((x * 2) + 1, y * 2, SSD1306_COLOR_ON);
                    display->draw_pixel(x * 2, (y * 2) + 1, SSD1306_COLOR_ON);
                    display->draw_pixel((x * 2) + 1, (y * 2) + 1, SSD1306_COLOR_ON);
                }
            }
        }

        display->update(); //update display
        drawReady = 0; //done
    }
    return;
}

void C8Core::printStackTrace() {
    printf("\x1b[2J\r");
    printf("CORE STACK TRACE\n");
    for(int i = 0; i < 16; i++) {
        printf("V%x: ", i & 0xF);
        printf("0x%x ", V[i]);
    }

    printf("\n");
    printf("PC: 0x%x ", (unsigned short)pc);
    printf("SP: 0x%x ", (unsigned short)sp);
    printf("I: 0x%x\n", (unsigned short)index);
    printf("Current Opcode: %x\n", (unsigned short)opcode);

    printf("Stack:\n");
    for(int i = 0; i < 16; i++) {
        printf("0x%x ", stack[i]);
    }

    return;
}