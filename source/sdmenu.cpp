#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "sdmenu.h"
#include "ssd1306.h"
#include "math.h"

SDMenu::SDMenu() {}

void SDMenu::init(SSD1306 *display) {
    menuState = MENU_LOAD;
    
    gfx = new GFX(display);
    drawLoading();
    
    sdio = new SD_IO();
    sdio->init();
    if(!sdio->cardInserted()) {
        menuState = MENU_NOSD;
    }
    else {
        LoadFiles();
        menuState = MENU_FILE;
    }

    oldState = new InputState();
    newState = new InputState();

}

bool SDMenu::update() {
    if(!sdio->cardInserted())
        menuState = MENU_NOSD;

    switch(menuState) {
        case MENU_NOSD: {
            drawNoSD();
            sdio->init();
            if(sdio->cardInserted()) {
                LoadFiles();
                menuState = MENU_FILE;
            }
            break;
        }
        case MENU_FILE: {
            updateInput();
            if(newState->down && !oldState->down) {
                index++;
                visualIndex++; 

                if(page == pageCount - 1) {
                    if(visualIndex > (sdio->root.count - 1) - (7 * (pageCount - 1))) {
                        visualIndex--;
                    }
                        
                    if(index > sdio->root.count - 1)
                        index = sdio->root.count - 1;
                }
                else {
                    if(visualIndex > 6) {
                        visualIndex = 0;
                        page++;
                    }
                }

                printf("%i\n", visualIndex);
            } 
            if(newState->up && !oldState->up) {
                index--;
                visualIndex--;

                if(page == 0) {
                    if(visualIndex < 0)
                        visualIndex = 0;
                    if(index < 0)
                        index = 0;
                }
                else {
                    if(visualIndex < 0) {
                        visualIndex = 6;
                        page--;
                    }
                }
            }
                
            if(newState->select && !oldState->select) {
                selection = new File{.filesize = sdio->root.files[index].filesize, .filename = sdio->root.files[index].filename};
                menuState = MENU_DONE;
            }
            memcpy(oldState, newState, sizeof(InputState));

            drawFilePicker();
            break;
        }
        case MENU_DONE: {
            return true;
        }
    }
    return false;
}

void SDMenu::drawNoSD() {
    gfx->clear();
    gfx->drawBmp(56, 18, nosd_bmp, 16, 18);
    gfx->drawString(45, 39, (char *)"NO SD");
    gfx->update();
}

void SDMenu::drawLoading() {
    gfx->clear();
    gfx->drawBmp(56, 18, load_bmp, 16, 18);
    gfx->drawString(37, 39, (char *)"LOADING");
    gfx->update();
}

void SDMenu::drawFilePicker() {
    gfx->clear();
    int y = 4;
    
    for(int i = 0; i < 7; i++) {
        if(i + (page * 7) < sdio->root.count) {
            if(i == visualIndex) {
                gfx->drawString(0, y + (i * 8), (char *)"> ");
            }

            gfx->drawString(16, y + (i * 8), sdio->root.files[i + (page * 7)].filename);
        }
    }

    gfx->update();

}

void SDMenu::updateInput() {
    bool upBtn = false, downBtn = false, selBtn = false;

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

void SDMenu::LoadFiles() {
    sdio->readFileList();
    page = 0;
    index = 0;
    visualIndex = 0;
    pageCount = (int)ceil((double)sdio->root.count / 7);
}

