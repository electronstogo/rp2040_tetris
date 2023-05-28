#include "display.h"
#include <sys/_stdint.h>



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

/*
* Fill display with color.
*/
void Display::fill(uint32_t color) { sprite.fillScreen(color); }

/*
* Flush sprite buffer into display.
*/
void Display::flush() { sprite.pushSprite(0, 0); }
