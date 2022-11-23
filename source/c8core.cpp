/*
pico-chip8 Emulator Core
Written by Y2K for the Bitbuilt 2022 Secret Santa (https://bitbuilt.net/forums/index.php?threads/bitbuilts-2022-secret-santa.5384/)
*/

#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "c8core.hpp"

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

const unsigned char rom[246] = {
    0x6A, 0x02, 0x6B, 0x0C, 0x6C, 0x3F, 0x6D, 0x0C, 0xA2, 0xEA, 0xDA, 0xB6, 0xDC, 0xD6, 0x6E, 0x00, 
    0x22, 0xD4, 0x66, 0x03, 0x68, 0x02, 0x60, 0x60, 0xF0, 0x15, 0xF0, 0x07, 0x30, 0x00, 0x12, 0x1A, 
    0xC7, 0x17, 0x77, 0x08, 0x69, 0xFF, 0xA2, 0xF0, 0xD6, 0x71, 0xA2, 0xEA, 0xDA, 0xB6, 0xDC, 0xD6, 
    0x60, 0x01, 0xE0, 0xA1, 0x7B, 0xFE, 0x60, 0x04, 0xE0, 0xA1, 0x7B, 0x02, 0x60, 0x1F, 0x8B, 0x02, 
    0xDA, 0xB6, 0x8D, 0x70, 0xC0, 0x0A, 0x7D, 0xFE, 0x40, 0x00, 0x7D, 0x02, 0x60, 0x00, 0x60, 0x1F, 
    0x8D, 0x02, 0xDC, 0xD6, 0xA2, 0xF0, 0xD6, 0x71, 0x86, 0x84, 0x87, 0x94, 0x60, 0x3F, 0x86, 0x02, 
    0x61, 0x1F, 0x87, 0x12, 0x46, 0x02, 0x12, 0x78, 0x46, 0x3F, 0x12, 0x82, 0x47, 0x1F, 0x69, 0xFF, 
    0x47, 0x00, 0x69, 0x01, 0xD6, 0x71, 0x12, 0x2A, 0x68, 0x02, 0x63, 0x01, 0x80, 0x70, 0x80, 0xB5, 
    0x12, 0x8A, 0x68, 0xFE, 0x63, 0x0A, 0x80, 0x70, 0x80, 0xD5, 0x3F, 0x01, 0x12, 0xA2, 0x61, 0x02, 
    0x80, 0x15, 0x3F, 0x01, 0x12, 0xBA, 0x80, 0x15, 0x3F, 0x01, 0x12, 0xC8, 0x80, 0x15, 0x3F, 0x01, 
    0x12, 0xC2, 0x60, 0x20, 0xF0, 0x18, 0x22, 0xD4, 0x8E, 0x34, 0x22, 0xD4, 0x66, 0x3E, 0x33, 0x01, 
    0x66, 0x03, 0x68, 0xFE, 0x33, 0x01, 0x68, 0x02, 0x12, 0x16, 0x79, 0xFF, 0x49, 0xFE, 0x69, 0xFF, 
    0x12, 0xC8, 0x79, 0x01, 0x49, 0x02, 0x69, 0x01, 0x60, 0x04, 0xF0, 0x18, 0x76, 0x01, 0x46, 0x40, 
    0x76, 0xFE, 0x12, 0x6C, 0xA2, 0xF2, 0xFE, 0x33, 0xF2, 0x65, 0xF1, 0x29, 0x64, 0x14, 0x65, 0x00, 
    0xD4, 0x55, 0x74, 0x15, 0xF2, 0x29, 0xD4, 0x55, 0x00, 0xEE, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00
};

C8Core::C8Core() {}

int C8Core::init() {
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

    //load fontset
    for(int i = 0; i = 0x50; i++) {
        ram[i] = font[i];
    }

    //reset timers
    t_delay = 0x0;
    t_sound = 0x0;

    //load ROM into RAM
    for(int i = 0; i < (sizeof(rom) / sizeof(rom[0])); i++) {
        ram[i + 0x200] = rom[i];
    }

    return 0; //WIP, return 1 until fully implemented
}

void C8Core::runCycle() {
    //fetch opcode
    opcode = ram[pc] << 8 | ram[pc + 1];

    //decode opcode
    switch(opcode & 0xF000) {
        case 0x0000:
            switch(opcode & 0x00FF) {
                case 0x00E0: //00E0 - CLS - Clear Screen
                    memset(vram, 0, sizeof vram); //clear vram
                    pc += 2;
                    break;

                case 0x00EE: //00EE - RET - Return from Subroutine
                    pc = stack[sp];
                    sp = sp > 0 ? sp - 1 : 0;
                    break;

                default:
                    printf("Unknown Opcode: 0x%X\n", opcode);
                    break;
            }
            break;

        case 0x1000: //1nnn - JP addr - Jump to location nnn
            pc = opcode & 0x0FFF;
            break;

        case 0x2000: //2nnn - CALL addr - Call subroutine at nnn
            sp++;
            stack[sp] = pc;
            pc = opcode & 0x0FFF;
            break;

        case 0x3000: //3xkk - SE Vx, byte - Skip next instruction if Vx = KK
            if(V[opcode & 0x0F00] == opcode & 0x00FF)
                pc += 2;
            pc += 2;
            break;

        case 0x4000: //4xkk - SNE Vx, byte - Skip next instruction if Vx != KK
            if(V[opcode & 0x0F00] != opcode & 0x00FF)
                pc += 2;
            pc += 2;
            break;

        case 0x5000: //5xy0 - SE Vx, Vy - Skip next instruction if Vx = Vy
            if(V[opcode & 0x0F00] == V[opcode & 0x00F0])
                pc += 2;
            pc += 2;
            break;

        case 0x6000: //6xkk - LD Vx, byte - Set Vx = kk
            V[opcode & 0x0F00] = opcode & 0x00FF;
            pc += 2;
            break;

        case 0x7000: //7xkk - ADD Vx, byte - Set Vx = Vx + kk
            V[opcode & 0x0F00] = V[opcode & 0x0F00] + opcode & 0x00FF;
            pc += 2;
            break;

        case 0x8000:
            switch(opcode & 0x000F) {
                case 0x0000: //8xy0 - LD Vx, Vy - Set Vx = Vy
                    V[opcode & 0x0F00] == V[opcode & 0x00F0];
                    pc += 2;
                    break;

                case 0x0001: //8xy1 - OR Vx, Vy - Set Vx = Vx OR Vy
                    V[opcode & 0x0F00] |= V[opcode & 0x00F0];
                    pc += 2;
                    break;

                case 0x0002: //8xy2 - AND Vx, Vy - Set Vx = Vx AND Vy
                    V[opcode & 0x0F00] &= V[opcode & 0x00F0];
                    pc += 2;
                    break;

                case 0x0003: //8xy3 - XOR Vx, Vy - Set Vx = Vx XOR Vy
                    V[opcode & 0x0F00] ^= V[opcode & 0x00F0];
                    pc += 2;
                    break;

                case 0x0004: //8xy4 - ADD Vx, Vy - Set Vx = Vx - Vy, set VF = NOT borrow
                    if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode && 0x0F00) >> 8]))
                        V[0xF] = 1; //carry
                    else
                        V[0xF] = 0; //no carry
                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                    pc += 2; 
                    break;

                case 0x0005: //8xy5 - SUB Vx, Vy - Set Vx = Vx - Vy, set VF = NOT borrow
                    if(V[opcode & 0x0F00] > V[opcode & 0x00F0])
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
                    V[opcode & 0x0F00] -= V[opcode & 0x00F0];
                    pc += 2;
                    break;

                case 0x0006: //8xy6 - SHR Vx {, Vy} - Set Vx = Vx SHR 1.
                    if(V[opcode & 0x0F00] & 1)
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
                    V[opcode & 0x0F00] /= 2;
                    pc += 2;
                    break;

                case 0x0007: //8xy7 - SUBN Vx, Vy - Set Vx = Vy - Vx, set VF = NOT borrow.
                    if(V[opcode & 0x00F0] > V[opcode & 0x0F00])
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
                    V[opcode & 0x0F00] -= V[opcode & 0x00F0];
                    pc += 2;
                    break;

                case 0x000E: //8xyE - SHL Vx {, Vy} - Set Vx = Vx SHL 1.
                    if(V[opcode & 0x0F00] & (1 << ((sizeof(char) * 8) - 1)))
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
                    V[opcode & 0x0F00] *= 2;
                    pc += 2;
                    break;
                    
                default:
                    printf("Unknown Opcode: 0x%X\n", opcode);
                    break;
            }
            break;

        case 0x9000: //9xy0 - SNE Vx, Vy - Skip next instruction if Vx != Vy.
            if(V[opcode & 0x0F00] != V[opcode & 0x00F0])
                pc += 2;
            pc += 2;
            break;

        case 0xA000: //ANNN - LD I, addr - Sets I to the address NNN
            index = opcode & 0x0FFF;
            pc += 2;
            break;

        case 0xB000: //Bnnn - JP V0, addr - Jump to location nnn + V0
            pc = (opcode & 0x0FFF) + V[0];
            break;

        case 0xC000: //Cxkk - RND Vx, byte - Set Vx = random byte AND kk.
            V[opcode & 0x0F00] = (rand() % 256) & (opcode & 0x00FF);
            pc += 2;
            break;

        case 0xD000: //Dxyn - DRW Vx, Vy, nibble - Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
            unsigned short x = V[opcode & 0x0F00];
            unsigned short y = V[opcode & 0x00F0];
            unsigned short h = opcode & 0x000F;
            unsigned short px;

            V[0xF] = 0;
            for(int yLine = 0; yLine < h; yLine++) {
                px = ram[index + yLine];
                for(int xLine = 0; xLine < 8; xLine++) {
                    if((px & (0x80 >> xLine)) != 0) {
                        if(vram[(x + xLine + ((y + yLine) * 64))] == 1)
                            V[0xF] = 1;
                        ram[x + xLine + ((y + yLine) * 64)] ^= 1;
                    }
                }
            }
            pc += 2;
            break;

        case 0xE000: //Ex9E - SKP Vx - Skip next instruction if key with the value of Vx is pressed.
            if(key[opcode & 0x0F00] == 1)
                pc += 2;
            pc += 2;
            break;

        case 0xF000:
            switch(opcode & 0x00FF) {
                case 0x0007: //Fx07 - LD Vx, DT - Set Vx = delay timer value.
                    V[opcode & 0x0F00] = t_delay;
                    pc += 2;
                    break;

                case 0x000A: //Fx0A - LD Vx, K - Wait for a key press, store the value of the key in Vx.
                    bool keyDown = false;
                    for(int i = 0; i < 16; i++) {
                        if(key[i] != 0) {
                            V[opcode & 0x0F00] = i;
                            keyDown = true;
                        }
                    }
                    if(!keyDown)
                        return;
                    pc += 2;
                    break;

                case 0x0015: //Fx15 - LD DT, Vx - Set delay timer = Vx.
                    t_delay = V[opcode & 0x0F00];
                    pc += 2;
                    break;

                case 0x0018: //Fx18 - LD ST, Vx - Set sound timer = Vx.
                    t_sound = V[opcode & 0x0F00];
                    pc += 2;
                    break;

                case 0x001E: //Fx1E - ADD I, Vx - Set I = I + Vx.
                    index += V[opcode & 0x0F00];
                    pc += 2;
                    break;

                case 0x0029: //Fx29 - LD F, Vx - Set I = location of sprite for digit Vx.
                    index = V[opcode & 0x0F00] * 0x5;
                    pc += 2;
                    break;

                case 0x0033: //Fx33 - LD B, Vx - Store BCD representation of Vx in memory locations I, I+1, and I+2.
                    ram[index] = V[(opcode & 0x0F00) >> 8] / 100;
                    ram[index + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
                    ram[index + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 100;
                    pc += 2;
                    break;

                case 0x0055: //Fx55 - LD [I], Vx - Store registers V0 through Vx in memory starting at location I.
                    for(int i = 0; i <= (opcode & 0x0F00); i++) {
                        ram[index + i] = V[i];
                    }
                    index += (opcode & 0x0F00) + 1;
                    pc += 2;
                    break;

                case 0x0065: //Fx65 - LD Vx, [I] - Read registers V0 through Vx from memory starting at location I.
                    for(int i = 0; i <= (opcode & 0x0F00); i++) {
                        V[i] = ram[index + i];
                    }
                    index += (opcode & 0x0F00) + 1;
                    pc += 2;
                    break;

                default:
                    printf("Unknown Opcode: 0x%X\n", opcode);
                    break;
            }
            break;

        default:
            printf("Unknown Opcode: 0x%X\n", opcode);
            break;
    }

    //update timers
    if(t_delay > 0)
        --t_delay;

    if(t_sound > 0) {
        if(t_sound == 1) {
            //beep
        }
        --t_sound;
    }

    return;
}

void C8Core::draw() {
    return;
}

void C8Core::setKeys() {
    return;
}

void C8Core::printStackTrace() {
    return;
}