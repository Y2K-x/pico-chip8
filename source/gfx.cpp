#include <stdio.h>
#include <string.h>
#include <utility>
#include <cmath>
#include "gfx.h"
#include "ssd1306.h"

GFX::GFX(SSD1306 *ssd1306) {
    this->ssd1306 = ssd1306;
}

void GFX::drawChar(int x, int y, char chr) {
	if(chr > 0x7E) return;

	for(uint8_t i=0; i < this->font[1]; i++ )
	{
        uint8_t line = (uint8_t)(this->font)[(chr-0x20) * (this->font)[1] + i + 2];

        for(int8_t j=0; j<this->font[0]; j++, line >>= 1)
        {
            if(line & 1)
            {
            	ssd1306->draw_pixel(x+i, y+j, SSD1306_COLOR_ON);
            }
        }
    }
}

void GFX::drawString(int x, int y, const char *str) {
    int length = strlen(str);
    if (length > 14) length = 14; //truncate string length to fit display

    for(int i = 0; i < length; i++)
        drawChar((x + (i * 8)), y, str[i]);
}

void GFX::drawBmp(int x, int y, const uint8_t *bmp, int w, int h) {
    int16_t byteWidth = (w + 7) / 8;
    uint8_t b = 0;

    for(int16_t j = 0; j < h; j++, y++) {
        for(int16_t i = 0; i < w; i++) {
            if(i & 7)
                b <<= 1;
            else
                b = bmp[j * byteWidth + i / 8];
            
            if(b & 0x80)
                ssd1306->draw_pixel(x + i, y, SSD1306_COLOR_ON);
        }
    }
}

void GFX::clear() {
    ssd1306->clear();
}

void GFX::update() {
    ssd1306->update();
}

void GFX::setFont(const uint8_t *font) {
    this->font = font;
}

const uint8_t* GFX::getFont() {
    return font;
}

SSD1306* GFX::display() {
    return ssd1306;
}