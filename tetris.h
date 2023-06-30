#ifndef TETRIS_H_
#define TETRIS_H_

#include "display.h"
#include <TFT_eSPI.h>
#include <sys/_stdint.h>


/**
* Tetris square part of block.
*/
class Square
{
  public:
    int8_t x;
    int8_t y;
    uint32_t color;
    bool filled;

    void init(int8_t x, int8_t y, uint32_t color, bool filled);
};

/**
* Tetris block.
*/
class Block
{
  public:
    static const uint8_t SQUARE_NUMBER = 4;

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

    enum Direction
    {
        LEFT,
        RIGHT
    };


    Shape shape;

    uint32_t color;
    Square squares[SQUARE_NUMBER];
    Square center;

    Block();
    Block(const Block& b);
    void init();
    uint32_t get_color();
    void set_coords(int8_t x, int8_t y, uint8_t index);
    void rotate(Direction d);
    void move_left();
    void move_right();
    void move_down();
};


class Tetris
{
  private:
    public:
    // Playfield constants.
    static const uint16_t Y_BOTTOM = 4;
    static const uint16_t X_LEFT = 3;
    static const uint16_t X_RIGHT = 125;
    static const uint16_t SQUARE_WIDTH = 12;
    static const uint8_t SQUARES_PER_COLUMN = (160 - Y_BOTTOM) / SQUARE_WIDTH;
    static const uint8_t SQUARES_PER_ROW = 10;
    static const uint32_t BACKGROUND = TFT_DARKGREY;
    

    // Display refresh.
    static const uint16_t FPS = 10;
    static const float SPEED_TABLE[10];


    // Hardware pins.
    static const uint8_t PIN_MOVE_LEFT = 20;
    static const uint8_t PIN_MOVE_RIGHT = 18;
    static const uint8_t PIN_ROTATE_LEFT = 19;
    static const uint8_t PIN_ROTATE_RIGHT = 21;

    // Score data.
    static const uint16_t ONE_LINE_POINTS = 40;
    static const uint16_t TWO_LINES_POINTS = 100;
    static const uint16_t THREE_LINES_POINTS = 300;
    static const uint16_t FOUR_LINES_POINTS = 1200;


    static const uint16_t DEBOUNCE_DELAY = 150;
    static uint32_t debounce;

    static bool move_left_flag;
    static bool move_right_flag;
    static bool rotate_left_flag;
    static bool rotate_right_flag;

    // User score from cleared lines.
    uint32_t score;
    // Current block speed level.
    uint8_t level;
    // Game over flag.
    bool game_over;
    // Delay between block steps.
    uint16_t move_delay;
    // Number of overall cleared lines.
    uint16_t cleared_lines;

    // Display interface.
    Display display;

    // Playfield squares.
    Square field_squares[SQUARES_PER_ROW][SQUARES_PER_COLUMN];

    // Currently active block.
    Block block;


    void init_button_isr();
    void clear_flags();
    void run();

    static void move_left();
    static void move_right();
    static void rotate_left();
    static void rotate_right();

    void move_block_left();
    void move_block_right();
    void move_block_downwards();
    void rotate_block(Block::Direction d);
    void update_score(uint8_t full_lines);
    void finish_block();
    void clear_full_lines();
    void shift_field_down(uint8_t index);
    void shift_line_down(uint8_t index);
    void clear_line(uint8_t index);
    bool block_finished();
    bool intersect_borders(Block b);
    bool intersection(Block b);
    void fill_playfield();

    void refresh_screen();
    void draw_square(Square f);
    void draw_current_block();
    void draw_blocks();
    void draw_playfield();


    Tetris();
};

#endif