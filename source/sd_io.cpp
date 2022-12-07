#include <stdio.h>
#include <cstring>
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
    return 0;
}

void SD_IO::readFileList(uint32_t count) {
    const char *dir = strtok(NULL, " ");
    if(!dir) dir = "";
    
    const char *p_dir;
    char cwdbuf[FF_LFN_BUF] = {0};
    FRESULT fr;

    if(dir[0]) {
        p_dir = dir;
    } else {
        fr = f_getcwd(cwdbuf, sizeof cwdbuf);
        if(fr != FR_OK) {
            printf("f_getcwd error: %s (%d)\n", FRESULT_str(fr), fr);
            return;
        }
        p_dir = cwdbuf;
    }

    printf("Directory Listing: %s\n", p_dir);

    DIR dj;
    FILINFO fno;
    memset(&dj, 0, sizeof dj);
    memset(&fno, 0, sizeof fno);
    fr = f_findfirst(&dj, &fno, p_dir, "*");
    if(fr != FR_OK) {
        printf("f_findfirst error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }
    while(fr == FR_OK && fno.fname[0]) {
        const char *pcWritableFile = "writeable file",
                   *pcReadOnlyFile = "read only file",
                   *pcDirectory = "directory";
        const char *pcAttrib;
        
        if(fno.fattrib & AM_DIR)
            pcAttrib = pcDirectory;
        else if(fno.fattrib & AM_RDO)
            pcAttrib = pcReadOnlyFile;
        else
            pcAttrib = pcWritableFile;

        printf("%s [%s] [size=%llu]\n", fno.fname, pcAttrib, fno.fsize);

        fr = f_findnext(&dj, &fno);
    }
    f_closedir(&dj);

    return;
}






