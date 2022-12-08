#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "ff.h"
#include "f_util.h"
#include "hw_config.h"
#include "sd_io.hpp"

SD_IO::SD_IO() {}

void SD_IO::init() {
    pSD = sd_get_by_num(0);
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if(fr != FR_OK) panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
    return;
}

uint32_t SD_IO::readFileCount() {
    uint32_t count = 0;

    char cwdbuf[FF_LFN_BUF] = {0};
    FRESULT fr;

    fr - f_getcwd(cwdbuf, sizeof cwdbuf);
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
    uint32_t expectedFileCount = readFileCount();
    files = new File[expectedFileCount];

    char cwdbuf[FF_LFN_BUF] = {0};
    FRESULT fr;

    fr - f_getcwd(cwdbuf, sizeof cwdbuf);
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
        files[count] = file;

        fr = f_findnext(&dir, &fno);
        count++;
        
    }
    
    f_closedir(&dir);

    return;
}

void SD_IO::loadFileToBuffer(char *dest, uint32_t index) {
    FIL file;
    FRESULT fr;
    
    fr = f_open(&file, files[index].filename, FA_READ);
    if(fr != FR_OK)
        panic("f_open(%s) error: %s (%d)\n", files[index], FRESULT_str(fr), fr);

    unsigned int bytesRead = 0;
    f_read(&file, dest, files[index].filesize, &bytesRead);

    f_close(&file);
}






