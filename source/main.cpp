/*
Pico-CHIP-8
A CHIP-8 emulator hardware project - developed by Y2K
Created for the BitBuilt 2022 Secret Santa (https://bitbuilt.net/forums/index.php?threads/bitbuilts-2022-secret-santa.5384/)
*/

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "c8core.h"
#include "sdmenu.h"

#define ROW_MASK          0x0000000F
#define COL_MASK          0x000000F0
#define PIN_SPEAKER       8
#define PIN_CARD_DETECT   14

#define PIN_DISPLAY_MOSI  19
#define PIN_DISPLAY_CS    16
#define PIN_DISPLAY_SCLK  18
#define PIN_DISPLAY_RST   20
#define PIN_DISPLAY_DC    17

enum class MainState {
    SDMenu,
    CoreInit,
    CoreRunning
};
MainState state = MainState::SDMenu;

SSD1306 *display;
C8Core core;
SDMenu menu;

struct repeating_timer cpuTimer;
struct repeating_timer timerUpdateTimer;

//CHIP-8 CPU core clock cycle callback - runs at aprox. 500hz
bool core_cycle_callback(struct repeating_timer *t) {
    core.pollInput();
    core.runCycle();
    core.draw();
    return true;
}

//CHIP-8 CPU core timer callback - runs at aprox. 60hz
bool core_timer_callback(struct repeating_timer *t) {
    core.updateTimers();
    return true;
}

//masked GPIO pull up function, since one doesnt appear to exist in Pico-SDK. I wonder why that is.
void gpio_pull_down_masked(uint32_t mask) {
    for(uint32_t i = 0; i < NUM_BANK0_GPIOS; i++) {
        if(mask & 1) {
            gpio_pull_down(i);
        }
        mask >>= 1;
    }

    return;
}

int main() {
    //init input GPIO
    gpio_init_mask(ROW_MASK + COL_MASK);
    gpio_set_dir_out_masked(ROW_MASK);
    gpio_set_dir_in_masked(COL_MASK);
    gpio_pull_down_masked(COL_MASK);

    //init card detect GPIO
    gpio_init(PIN_CARD_DETECT);
    gpio_set_dir(PIN_CARD_DETECT, false);
    gpio_pull_up(PIN_CARD_DETECT);

    //init speaker GPIO
    gpio_init(PIN_SPEAKER);
    gpio_set_dir(PIN_SPEAKER, true);
    gpio_pull_down(PIN_SPEAKER);

    //init display
    display = new SSD1306(128, 64, spi0, 8000 * 1000, 
                          PIN_DISPLAY_MOSI, 
                          PIN_DISPLAY_CS, 
                          PIN_DISPLAY_SCLK, 
                          PIN_DISPLAY_RST, 
                          PIN_DISPLAY_DC);
    display->init();
    display->clear();
    display->update();

    //init menu
    menu.init(display);

    printf("init done\n");

    while(1) {
        switch(state) {
            case MainState::SDMenu:
                while(1) {
                    menu.update();
                    menu.draw();
                    if(menu.getState() == SDMenu::MenuState::Done)
                        break;
                }

                state = MainState::CoreInit;
                break;
            
            case MainState::CoreInit: {
                //init core
                core.init(display);

                //load ROM
                uint8_t *rom = (uint8_t *)malloc(menu.getSelection()->filesize);
                menu.loadFile(rom);
                core.loadROM(rom, menu.getSelection()->filesize);

                //setup timer interupts
                add_repeating_timer_us(2000, core_cycle_callback, NULL, &cpuTimer); //~500hz
                add_repeating_timer_us(16666, core_timer_callback, NULL, &timerUpdateTimer); //~60hz

                state = MainState::CoreRunning;
                break;
            }

            case MainState::CoreRunning:
                break; //run forever, core is running from hardware timers
        }
    }
}