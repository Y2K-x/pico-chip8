#include <stdio.h>
#include "pico/stdlib.h"
#include "c8core.hpp"

C8Core *core; 

int main() {
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
        core->runCycle();
        printf("\x1b[2J\r");
        core->printStackTrace();
        sleep_ms(100);
    }
}