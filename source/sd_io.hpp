#ifndef SD_IO_H
#define SD_IO_H

#include "ff.h"
#include "f_util.h"
#include "hw_config.h"

class SD_IO {
    public:
        SD_IO();
        void init();
        uint32_t readFileCount();
        void readFileList(uint32_t count);
    private:
        sd_card_t *pSD;

};


#endif