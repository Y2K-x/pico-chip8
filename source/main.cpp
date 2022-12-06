#include <stdio.h>
#include "pico/stdlib.h"
#include "c8core.hpp"

const int row_pins[4] = {6, 7, 8, 9};
const int col_pins[4] = {2, 3, 4, 5};

C8Core *core;

struct repeating_timer cpuTimer;
struct repeating_timer timerUpdateTimer;

void readInput();

bool core_cycle_callback(struct repeating_timer *t) {
    readInput();
    core->runCycle();
    core->draw();
    return true;
}

bool core_timer_callback(struct repeating_timer *t) {
    core->updateTimers();
    return true;
}

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

    add_repeating_timer_us(2000, core_cycle_callback, NULL, &cpuTimer);
    add_repeating_timer_us(16666, core_timer_callback, NULL, &timerUpdateTimer);

   while(1) {}
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