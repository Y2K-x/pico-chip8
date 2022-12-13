#ifndef SDMENU_H
#define SDMENU_H

#include "gfx.h"
#include "sd_io.h"
#include "ssd1306.h"

#define MAX_FILES_PER_PAGE 7

const uint8_t nosd_bmp[36] = {
    0x3f, 0xfc, 
    0x20, 0x04, 
    0x20, 0x04, 
    0x20, 0x04, 
    0x21, 0x04, 
    0x21, 0x04, 
    0x21, 0x04, 
    0x41, 0x04, 
    0x81, 0x04, 
    0x81, 0x04, 
    0x41, 0x04, 
    0x80, 0x04, 
    0x80, 0x04, 
    0x81, 0x04, 
    0x80, 0x04, 
    0x80, 0x04, 
    0x80, 0x04, 
    0xff, 0xfc, 
};
const uint8_t nofile_bmp[36] = {
    0xFF, 0xC0,
    0x80, 0x20,
    0x80, 0x10,
    0x80, 0x08,
    0x80, 0x08,
    0x80, 0x08,
    0x87, 0x08,
    0x88, 0x88,
    0x80, 0x88,
    0x82, 0x08,
    0x80, 0x08,
    0x80, 0x08,
    0x82, 0x08,
    0x80, 0x08,
    0x80, 0x08,
    0x80, 0x08,
    0x80, 0x08,
    0xFF, 0xF8
};
const uint8_t load_bmp[36] = {
    0xFF, 0xFE,
    0x40, 0x04,
    0x40, 0x04,
    0x40, 0x04,
    0x42, 0x84,
    0x41, 0x04,
    0x20, 0x08,
    0x11, 0x10,
    0x10, 0x10,
    0x10, 0x10,
    0x11, 0x10,
    0x20, 0x08,
    0x40, 0x04,
    0x41, 0x04,
    0x43, 0x84,
    0x47, 0xC4,
    0x40, 0x04,
    0xFF, 0xFE
};

typedef struct InputState {
    bool up;
    bool down;
    bool select;
} InputState;

class SDMenu {
    public:
        enum class MenuState {
            NoSD,
            NoFiles,
            Load,
            File,
            Done
        };
        
        SDMenu();
        void init(SSD1306 *display);
        void update();
        void draw();
        
        MenuState getState();
    private:
        void LoadFiles();
        void updateInputState();
        void drawNoSD();
        void drawNoFiles();
        void drawLoading();
        void drawFilePicker();
        
        GFX *gfx;
        SD_IO *sdio;

        File *selection;
        InputState *newState;
        InputState *oldState;
        MenuState state;

        uint32_t index;
        uint32_t page;
        uint32_t pageCount;
        
};

#endif