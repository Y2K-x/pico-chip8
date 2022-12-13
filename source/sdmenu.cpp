#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "sdmenu.h"

SDMenu::SDMenu() {}

/* Public Method Definitions */
void SDMenu::init(SSD1306 *display) {
    state = MenuState::Load;
    gfx = new GFX(display);
    drawLoading();
    
    sdio = new SD_IO();
    sdio->init();
    
    newState = new InputState();
    oldState = new InputState();

    index = 0;
    page = 0;
    pageCount = 0;

    return;
}

void SDMenu::update() {
    if(!sdio->cardInserted())
        state = MenuState::NoSD;

    switch(state) {
        case MenuState::NoSD:
            sdio->init();
            if(sdio->cardInserted()) {
                LoadFiles();
                state = MenuState::File;
            }
            break;

        case MenuState::Load:
            if(!sdio->cardInserted()) {
                state = MenuState::NoSD;
                break;
            }
            LoadFiles();
            state = MenuState::File;
            break;
        
        case MenuState::File:
            if(sdio->root.count == 0) {
                state = MenuState::NoFiles;
                break;
            }

            //read GPIO and update current input state
            updateInputState();

            if(newState->up && !oldState->up) {
                index--;

                if(page == 0) {
                    if(index <= -1) {
                        index = 0;
                    }
                }
                else {
                    if(index < (page * 7)) {
                        page--;
                    }
                }
            }

            if(newState->down && !oldState->down) {
                index++;

                if(page == pageCount - 1) {
                    if(index > sdio->root.count - 1)
                        index = sdio->root.count - 1;
                }
                else {
                    if(index >= (((page + 1) * 7)))
                        page++;
                }

                printf("index: %i | page: %i\n", index, page);
            }

            if(newState->select && !oldState->select) {
                selection = new File{.filesize = sdio->root.files[index].filesize, .filename = sdio->root.files[index].filename};
                state = MenuState::Done;
            }

            //copy newState to oldState for input debouncing
            memcpy(oldState, newState, sizeof(InputState));
            break;
    }

    return;
}

void SDMenu::draw() {
    switch(state) {
        case MenuState::NoSD:
            drawNoSD();
            break;

        case MenuState::NoFiles:
            drawNoFiles();
            break;
        
        case MenuState::File:
            drawFilePicker();
            break;
    }

    return;
}

/* Private Method Definitions */
void SDMenu::LoadFiles() {
    sdio->readFileList();
    page = 0;
    index = 0;
    pageCount = (int)ceil((double)sdio->root.count / 7);
    return;
}

void SDMenu::updateInputState() {
    //check up btn
    gpio_put(0, true);
    newState->up = gpio_get(7);
    gpio_put(0, false);

    //check down btn
    gpio_put(1, true);
    newState->down = gpio_get(7);
    gpio_put(1, false);

    //check sel btn
    gpio_put(0, true);
    newState->select = gpio_get(4);
    gpio_put(0, false);
}

void SDMenu::drawNoSD() {
    gfx->clear();
    gfx->drawBmp(56, 18, nosd_bmp, 16, 18);
    gfx->drawString(45, 39, "NO SD");
    gfx->update();

    return;
}

void SDMenu::drawNoFiles() {
    gfx->clear();
    gfx->drawBmp(57, 18, nofile_bmp, 16, 18);
    gfx->drawString(37, 39, "NO FILE");
    gfx->update();

    return;
}

void SDMenu::drawLoading() {
    gfx->clear();
    gfx->drawBmp(56, 18, load_bmp, 16, 18);
    gfx->drawString(37, 39, "LOADING");
    gfx->update();

    return;
}

void SDMenu::drawFilePicker() {
    gfx->clear();

    for(int i = 0; i < MAX_FILES_PER_PAGE; i++) {
        if(i + (page * 7) < sdio->root.count) {
            if(i == (index - (page * MAX_FILES_PER_PAGE)))
                gfx->drawString(0, 4 + (i * 8), "> ");

            gfx->drawString(16, 4 + (i * 8), sdio->root.files[i + (page * 7)].filename);
        }
    }

    gfx->update();

    return;
}

/* Getters and Setters */
SDMenu::MenuState SDMenu::getState() {
    return state;
}