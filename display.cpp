#include "display.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <sys/_stdint.h>


TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);


Display::Display()
{
    tft.init();
    tft.setRotation(2);

    sprite.createSprite(WIDTH, HEIGHT);
}


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

void Display::filled_rectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint32_t color)
{
    sprite.fillRect(x, y, width, height, color);
}


void Display::fill(uint32_t color) { sprite.fillScreen(color); }

void Display::flush() { sprite.pushSprite(0, 0); }


uint32_t Display::rgb888_to_rgb565(uint32_t color)
{
    uint8_t r8 = color >> 16;
    uint8_t g8 = color >> 8;
    uint8_t b8 = color;

    color = (r8 * 31 / 255) << 16;
    color += (g8 * 63 / 255) << 8;
    color += g8 * 31 / 255;

    return color;


    // R = (0b1111100000000000 & input) >> 11;
    // G = (0b0000011111100000 & input) >> 5;
    // B = (0b0000000000011111 & input);
}