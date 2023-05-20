#ifndef TETRIS_H_
#define TETRIS_H_

#include "display.h"
#include <TFT_eSPI.h>
#include <sys/_stdint.h>


enum Direction
{
    LEFT,
    RIGHT
};

class Square
{
  public:
    int8_t x;
    int8_t y;
    uint32_t color;
    bool filled;
};

class Block
{
  public:
    static const int8_t SQUARE_NUMBER = 4;

    enum Shape
    {
        L,
        J,
        S,
        Z,
        O,
        I,
        T
    };


    Shape shape;

    uint32_t color;
    Square squares[SQUARE_NUMBER];
    Square center;

    Block();
    Block(const Block& b);
    void init(uint32_t color);
    void set_coords(int8_t x, int8_t y, uint8_t index);
    void rotate(Direction d);
    void move_left();
    void move_right();
    void move_down();
};

class Tetris
{
  private:
    // Playfield constants.
    static const uint16_t Y_BOTTOM = 2;
    static const uint16_t X_LEFT = 1;
    static const uint16_t X_RIGHT = 127;
    static const uint8_t SQUARES_PER_COLUMN = 16;
    static const uint8_t SQUARES_PER_ROW = 10;
    static const uint16_t SQUARE_WIDTH = 12;

    // Gaming constants.
    static const uint16_t MOVE_DELAY = 1000;
    static const uint16_t FPS = 1;
    static const uint8_t PIN_MOVE_LEFT = 20;
    static const uint8_t PIN_MOVE_RIGHT = 18;
    static const uint8_t PIN_ROTATE_LEFT = 19;
    static const uint8_t PIN_ROTATE_RIGHT = 21;

    static const uint32_t BACKGROUND = TFT_DARKGREY;


    // Display.
    Display display;

    // Playfield squares.
    Square field_squares[SQUARES_PER_ROW][SQUARES_PER_COLUMN];

    // Currently active block.
    Block block;


    void init_button_isr();
    void clear_button_flags();

    void move_block_left();
    void move_block_right();
    void move_block_downwards();
    void rotate_block(Direction d);
    void finish_block();
    void clear_full_lines();
    uint32_t get_random_color();
    bool block_finished();
    bool intersect_borders(Block b);
    bool intersection(Block b);

    void refresh_screen();
    void draw_square(Square f);
    void draw_current_block();
    void draw_blocks();
    void draw_playfield();

  public:
    Tetris();
};

#endif