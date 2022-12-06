#include <stdio.h>
#include "pico/stdlib.h"
#include "c8core.hpp"

const int row_pins[4] = {6, 7, 8, 9};
const int col_pins[4] = {2, 3, 4, 5};

C8Core *core;

void readInput();

int main() {
    stdio_init_all();

    //init input pins
    for(int i = 0; i < 4; i++) {
        gpio_init(row_pins[i]);
        gpio_init(col_pins[i]);

        gpio_set_dir(row_pins[i], true);
        gpio_set_dir(col_pins[i], false);
        
        gpio_pull_down(col_pins[i]);
    }

    core = new C8Core();
    core->init();

    while(1) {
        readInput();
        core->runCycle();
        core->draw();
        sleep_us(2000);
    }
}

void readInput() {
    for(int row = 0; row < 4; row++) {
        gpio_put(row_pins[row], true);

        for(int col = 0; col < 4; col++) {
            core->key[keyMap[row][col]] = (unsigned char)gpio_get(col_pins[col]);
        }

        gpio_put(row_pins[row], false);
    }
}