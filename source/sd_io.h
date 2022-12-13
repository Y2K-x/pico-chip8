#ifndef SD_IO_H
#define SD_IO_H

#include "ff.h"
#include "f_util.h"
#include "hw_config.h"

typedef struct File {
    uint32_t filesize;
    char *filename;
} File;

typedef struct Directory {
    uint32_t count;
    File *files;
} Directory;

class SD_IO {
    public:
        SD_IO();
        void init();
        uint32_t readFileCount();
        void readFileList();
        //void loadFileToBuffer(char *dest, uint32_t index);
        void loadFileToBuffer(uint8_t *dest, File *file);
        bool cardInserted();
        Directory * getRoot();
    private:
        Directory *root;
        sd_card_t *pSD;
};

#endif