#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "ff.h"
#include "f_util.h"
#include "diskio.h"
#include "hw_config.h"
#include "sd_io.h"

static void card_detect_callback(uint gpio, uint32_t events) {
    static bool busy;
    if(busy) return;
    busy = true;

    sd_card_t *sd = sd_get_by_num(0);
    if(sd->card_detect_gpio == gpio) {
        if(sd->mounted) {
            printf("Card Detect Interrupt: unmounting %s\n", sd->pcName);
            FRESULT fr = f_unmount(sd->pcName);
            if(fr == FR_OK)
                sd->mounted = false;
            else
                printf("f_unmount error: %s (%d)\n", FRESULT_str(fr), fr);
        }

        sd->m_Status |= STA_NOINIT;
        sd_card_detect(sd);
    }
    busy = false;
}

SD_IO::SD_IO() {}

void SD_IO::init() {
    pSD = sd_get_by_num(0); 
    if(pSD->use_card_detect)
        gpio_set_irq_enabled_with_callback(pSD->card_detect_gpio, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &card_detect_callback);
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);

    if(!cardInserted())
        return;

    if(fr != FR_OK) panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
    return;
}

uint32_t SD_IO::readFileCount() {
    uint32_t count = 0;

    char cwdbuf[FF_LFN_BUF] = {0};
    FRESULT fr;

    fr = f_getcwd(cwdbuf, sizeof cwdbuf);
    if(fr != FR_OK) {
        printf("f_getcwd error: %s (%d)\n", FRESULT_str(fr), fr);
        return 0;
    }
    
    DIR dir;
    FILINFO fno;
    memset(&dir, 0, sizeof dir);
    memset(&fno, 0, sizeof fno);

    fr = f_findfirst(&dir, &fno, cwdbuf, "*.ch8");
    if(fr != FR_OK) {
        printf("f_findfirst error: %s (%d)\n", FRESULT_str(fr), fr);
        return 0;
    }

    while(fr == FR_OK && fno.fname[0] && !(fno.fattrib & AM_DIR)) {
        count++;
        fr = f_findnext(&dir, &fno);
    }

    f_closedir(&dir);
    return count;
}

void SD_IO::readFileList() {
    uint32_t fileCount = readFileCount();
    root = new Directory{.count = fileCount, .files = new File[fileCount]};

    char cwdbuf[FF_LFN_BUF] = {0};
    FRESULT fr;

    fr = f_getcwd(cwdbuf, sizeof cwdbuf);
    if(fr != FR_OK) {
        printf("f_getcwd error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }
    
    DIR dir;
    FILINFO fno;
    memset(&dir, 0, sizeof dir);
    memset(&fno, 0, sizeof fno);
    
    int count = 0;
    fr = f_findfirst(&dir, &fno, cwdbuf, "*.ch8");
    if(fr != FR_OK) {
        printf("f_findfirst error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }

    while(fr == FR_OK && fno.fname[0]) {
        File file = {.filesize = (uint32_t)fno.fsize, .filename = (char *)malloc(strlen(fno.fname) + 1)};
        strncpy(file.filename, fno.fname, strlen(fno.fname) + 1);
        root->files[count] = file;

        fr = f_findnext(&dir, &fno);
        count++;
        
    }
    
    f_closedir(&dir);

    return;
}

void SD_IO::loadFileToBuffer(uint8_t *dest, File *file) {
    FIL fil;
    FRESULT fr;
    
    fr = f_open(&fil, file->filename, FA_READ);
    if(fr != FR_OK)
        panic("f_open(%s) error: %s (%d)\n", file->filename, FRESULT_str(fr), fr);

    unsigned int bytesRead = 0;
    f_read(&fil, dest, file->filesize, &bytesRead);

    f_close(&fil);
}

bool SD_IO::cardInserted() {
    return pSD->card_type == 0 ? false : true;
}

Directory * SD_IO::getRoot() {
    return root;
}