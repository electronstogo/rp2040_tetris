#include <sys/_stdint.h>
#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>

class Display
{
  private:
  public:
    static const uint8_t WIDTH = 128;
    static const uint8_t HEIGHT = 160;

    Display();
    void line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint32_t color);
    void vline(uint8_t x1, uint8_t y1, uint8_t length, uint32_t color);
    void filled_rectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint32_t color);
    void fill(uint32_t color);
    void flush();
    uint32_t rgb888_to_rgb565(uint32_t color);
};

#endif
