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
        
        case MenuState::NoFiles:
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

                if(index < 0)
                    index = 0;
            }

            if(newState->down && !oldState->down) {
                index++;

                if(index > (int)sdio->root.count - 1)
                    index = sdio->root.count - 1;
            }

            if(newState->select && !oldState->select) {
                selection = new File{.filesize = sdio->root.files[index].filesize, .filename = sdio->root.files[index].filename};
                state = MenuState::Done;
            }

            //copy newState to oldState for input debouncing
            memcpy(oldState, newState, sizeof(InputState));
            break;
        
        case MenuState::Done:
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

        case MenuState::Load:
            drawLoading();
            break;

        case MenuState::File:
            drawFilePicker();
            break;

        case MenuState::Done:
            break;
    }

    return;    index = 0;
}

/* Private Method Definitions */
void SDMenu::LoadFiles() {
    sdio->readFileList();
    index = 0;
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

    int page = index / MAX_FILES_PER_PAGE;

    for(int i = 0; i < MAX_FILES_PER_PAGE; i++) {
        if(i + (page * MAX_FILES_PER_PAGE) < (int)sdio->root.count) {
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

File * SDMenu::getSelection() {
    return selection;
}