#include <stdio.h>
#include "pico/stdlib.h"
#include "c8core.hpp"

const int row_pins[4] = {0,1,2,3};
const int col_pins[4] = {7,6,5,4};

C8Core *core;

struct repeating_timer cpuTimer;
struct repeating_timer timerUpdateTimer;

void readInput();

//CHIP-8 CPU core clock cycle callback - runs at aprox. 500hz
bool core_cycle_callback(struct repeating_timer *t) {
    readInput();
    core->runCycle();
    core->draw();
    return true;
}

//CHIP-8 CPU core timer callback - runs at aprox. 60hz
bool core_timer_callback(struct repeating_timer *t) {
    core->updateTimers();
    return true;
}

int main() {
    //init serial output
    stdio_init_all();

    //init input pins
    for(int i = 0; i < 4; i++) {
        gpio_init(row_pins[i]);
        gpio_init(col_pins[i]);

        gpio_set_dir(row_pins[i], true);
        gpio_set_dir(col_pins[i], false);
        
        gpio_pull_down(col_pins[i]);
    }

    //init speaker
    gpio_init(8);
    gpio_set_dir(8, true);
    gpio_pull_down(8);

    //beep test, may remove later but its kinda charming lol
    gpio_put(8, true);
    sleep_ms(100);
    gpio_put(8, false);

    //init CHIP-8 core
    core = new C8Core();
    core->init();

    //start update timers for CPU cycle & timers
    add_repeating_timer_us(2000, core_cycle_callback, NULL, &cpuTimer);
    add_repeating_timer_us(16666, core_timer_callback, NULL, &timerUpdateTimer);

    //run forever doing "nothing", otherwise timers will not function
   while(1) {}
}

void readInput() {
    //poll all rows
    for(int row = 0; row < 4; row++) {
        //set current row pin high
        gpio_put(row_pins[row], true);

        //poll all columns
        for(int col = 0; col < 4; col++) {
            //map all physical key states to CHIP-8 core keymap & store in core's keypad registers
            core->key[keyMap[row][col]] = (uint8_t)gpio_get(col_pins[col]);
        }

        //done with this row, start polling next row
        gpio_put(row_pins[row], false);
    }
}