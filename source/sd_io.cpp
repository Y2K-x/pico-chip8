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

//    for(int i = 0; i < expectedFileCount; i++) {
//        printf("%s : %i\n", files[i], strlen(files[i].filename));
//    }

    return;
}

void hexDump (
    const char * desc,
    const void * addr,
    const int len,
    int perLine
) {
    // Silently ignore silly per-line values.

    if (perLine < 4 || perLine > 64) perLine = 16;

    int i;
    unsigned char buff[perLine+1];
    const unsigned char * pc = (const unsigned char *)addr;

    // Output description if given.

    if (desc != NULL) printf ("%s:\n", desc);

    // Length checks.

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %d\n", len);
        return;
    }

    // Process every byte in the data.

    for (i = 0; i < len; i++) {
        // Multiple of perLine means new or first line (with line offset).

        if ((i % perLine) == 0) {
            // Only print previous-line ASCII buffer for lines beyond first.

            if (i != 0) printf ("  %s\n", buff);

            // Output the offset of current line.

            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.

        printf (" %02x", pc[i]);

        // And buffer a printable ASCII character for later.

        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) // isprint() may be better.
            buff[i % perLine] = '.';
        else
            buff[i % perLine] = pc[i];
        buff[(i % perLine) + 1] = '\0';
    }

    // Pad out last line if not exactly perLine characters.

    while ((i % perLine) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII buffer.

    printf ("  %s\n", buff);
}

void SD_IO::loadFileToBuffer(char *dest) {
    FIL file;
    FRESULT fr;
    
    fr = f_open(&file, files[0].filename, FA_READ);
    if(fr != FR_OK)
        panic("f_open(%s) error: %s (%d)\n", files[0], FRESULT_str(fr), fr);
    
    unsigned int bytesRead = 0;
    f_read(&file, &dest, 300, &bytesRead);

    hexDump(files[0].filename, &dest, 300, 16);
}






