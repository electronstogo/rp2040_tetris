#include "display.h"
#include <sys/_stdint.h>
#include "Free_Fonts.h"


Display::Display()
{
    tft.init();
    tft.setRotation(2);

    sprite.createSprite(WIDTH, HEIGHT);
}

/*
* Line from (x1, y1) to (x2, y2).
*/
void Display::line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint32_t color)
{
    sprite.drawLine(x1, y1, x2, y2, color);
}

/**
 * Draws a vertical line.
 */
void Display::vline(uint8_t x1, uint8_t y1, uint8_t length, uint32_t color)
{
    sprite.drawLine(x1, y1, x1, y1 + length, color);
}

/*
* Filled rectangle.
*/
void Display::filled_rectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint32_t color)
{
    sprite.fillRect(x, y, width, height, color);
}


void Display::number(uint32_t number, uint16_t x, uint16_t y)
{
    sprite.setTextColor(TFT_WHITE, TFT_DARKGREY);
    sprite.setFreeFont(FSB9); 
    //sprite.drawString(String(number), x, y, GFXFF);
    sprite.drawRightString(String(number), x, y, GFXFF);
}

/*
* Fill display with color.
*/
void Display::fill(uint32_t color) { sprite.fillScreen(color); }

/*
* Flush sprite buffer into display.
*/
void Display::flush() { sprite.pushSprite(0, 0); }
