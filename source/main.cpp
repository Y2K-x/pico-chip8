#include <stdio.h>
#include "pico/stdlib.h"
#include "c8core.hpp"

const int row_pins[4] = {6, 7, 8, 9};
const int col_pins[4] = {2, 3, 4, 5};

C8Core *core;

void readInput();

int main() {
    //init input pins
    for(int i = 0; i < 4; i++) {
        gpio_init(row_pins[i]);
        gpio_init(col_pins[i]);

        gpio_set_dir(row_pins[i], true);

        gpio_set_dir(col_pins[i], false);
        gpio_pull_down(col_pins[i]);
    }

    core = new C8Core();

    stdio_init_all();
    sleep_ms(5000); //give the serial terminal time to catch up
    printf("\x1b[2J\r"); //clear screen on supported terminals (minicom)

    //init CHIP-8 core
    if(core->init() == 1) {
        printf("Core init failed!\n");
        while(1) {sleep_ms(1000);} //run forever
    }

    printf("Core init success!");

    while(1) {
        readInput();
        core->runCycle();
        //core->printStackTrace();
        core->debugDraw();
    }
}

void readInput() {
    for(int row = 0; row < 4; row++) {
        for(int col = 0; col < 4; col++) {
            gpio_put(row_pins[row], true);
            core->key[keyMap[row][col]] = (unsigned char)gpio_get(col_pins[col]);
        }
        gpio_put(row_pins[row], false);
    }
}