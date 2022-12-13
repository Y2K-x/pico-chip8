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

C8Core::C8Core() {}

void C8Core::init(SSD1306 *display) {
    this->display = display;

    //clear registers and memory
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

void C8Core::loadROM(uint8_t *rom, uint32_t size) {
    //return if ROM is too big to fit into Chip8 RAM
    if(size > 3584)
        return;

    //load ROM, also must be resident in RAM, chip8 is weird, yes programs can and will override themselves
    memcpy(&ram[0x200], rom, size);
}

void C8Core::pollInput() {
    //poll all rows
    for(int row = 0; row < 4; row++) {
        //set current row pin high
        gpio_put(row_pins[row], true);

        //poll all columns
        for(int col = 0; col < 4; col++) {
            //map all physical key states to CHIP-8 core keymap & store in core's keypad registers
            key[keyMap[row][col]] = (uint8_t)gpio_get(col_pins[col]);
        }

        //done with this row, start polling next row
        gpio_put(row_pins[row], false);
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