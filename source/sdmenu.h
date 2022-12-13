#ifndef MENU_H
#define MENU_H

#include "gfx.h"
#include "ssd1306.h"
#include "sd_io.h"

typedef struct InputState {
    bool up;
    bool down;
    bool select;
} InputState;

class SDMenu {
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

    public:
        enum class MenuState {
            NoSD,
            Load,
            File,
            Done
        };
        
        SDMenu();
        void init(SSD1306 *display);

        bool update();
        void drawNoSD();
        void drawLoading();
        void drawFilePicker();
        void updateInput();
        void LoadFiles();

        File *selection;
        SD_IO *sdio;
    private:
        GFX *gfx;
        

        MenuState menuState;

        InputState *newState;
        InputState *oldState;

        int page;
        int pageCount;
        int index;
        int visualIndex;
};

#endif