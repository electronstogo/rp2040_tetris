#include "TFT_eSPI.h"
#include "tetris.h"
#include <Arduino.h>
#include <algorithm>
#include <cstring>
#include <math.h>
#include <sys/_stdint.h>

volatile bool g_move_left = false;
volatile bool g_move_right = false;
volatile bool g_rotate_left = false;
volatile bool g_rotate_right = false;


void move_left() { g_move_left = true; }
void move_right() { g_move_right = true; }
void rotate_left() { g_rotate_left = true; }
void rotate_right() { g_rotate_right = true; }


Tetris::Tetris()
{
    Serial.begin(9600);

    // Init field.
    for(uint8_t x = 0; x < SQUARES_PER_ROW; x++)
    {
        for(uint8_t y = 0; y < SQUARES_PER_COLUMN; y++)
        {
            field_squares[x][y].filled = false;
            field_squares[x][y].x = x;
            field_squares[x][y].y = y;
        }
    }

    // Initial screen.
    display.fill(BACKGROUND);
    draw_playfield();
    init_button_isr();

    block.init(get_random_color());
    uint32_t timestamp = 0;
    uint32_t timestamp_draw = 0;

    while(true)
    {
        if(g_move_left)
        {
            move_block_left();
            refresh_screen();
        }

        if(g_move_right)
        {
            move_block_right();
            refresh_screen();
        }

        if(g_rotate_left)
        {
            rotate_block(-90);
            refresh_screen();
        }

        if(g_rotate_right)
        {
            rotate_block(90);
            refresh_screen();
        }

        clear_button_flags();

        if(millis() - timestamp > MOVE_DELAY)
        {
            move_block_downwards();
            timestamp = millis();
        }

        if(millis() - timestamp_draw > 1000 / FPS)
        {
            refresh_screen();
            timestamp_draw = millis();
        }
    }
}


void Tetris::init_button_isr()
{
    attachInterrupt(PIN_MOVE_LEFT, move_left, FALLING);
    attachInterrupt(PIN_MOVE_RIGHT, move_right, FALLING);
    attachInterrupt(PIN_ROTATE_LEFT, rotate_left, FALLING);
    attachInterrupt(PIN_ROTATE_RIGHT, rotate_right, FALLING);
}


void Tetris::clear_button_flags()
{
    g_move_left = false;
    g_move_right = false;
    g_rotate_left = false;
    g_rotate_right = false;
}


void Tetris::finish_block()
{
    for(uint8_t i = 0; i < block.SQUARE_NUMBER; i++)
    {
        uint8_t x = block.squares[i].x + block.center.x;
        uint8_t y = block.squares[i].y + block.center.y;

        field_squares[x][y].filled = true;
        field_squares[x][y].color = block.color;
    }



    block.init(get_random_color());
}


void Tetris::move_block_left()
{
    Block dummy_block(block);
    dummy_block.move_left();

    if(intersect_borders(dummy_block) || intersection(dummy_block))
    {
        return;
    }

    block.move_left();
}


void Tetris::move_block_right()
{
    Block b(block);
    b.move_right();

    if(intersect_borders(b) || intersection(b))
    {
        return;
    }

    block.move_right();
}


void Tetris::move_block_downwards()
{
    if(block_finished())
    {
        finish_block();
        return;
    }

    block.move_down();
}


void Tetris::rotate_block(int16_t degree)
{
    Block b(block);
    b.rotate(degree);

    if(intersect_borders(b) || intersection(b))
    {
        return;
    }

    block.rotate(degree);
}


void Tetris::clear_full_lines() {}


bool Tetris::block_finished()
{
    Block b(block);
    b.move_down();

    for(uint8_t i = 0; i < block.SQUARE_NUMBER; i++)
    {
        if((block.center.y + block.squares[i].y == 0) || intersection(b))
        {
            return true;
        }
    }

    return false;
}

/*
 * Checks if the current clock intersects with any field border.
 */
bool Tetris::intersect_borders(Block b)
{
    for(uint8_t i = 0; i < block.SQUARE_NUMBER; i++)
    {
        uint8_t x = b.squares[i].x + b.center.x;
        uint8_t y = b.squares[i].y + b.center.y;

        if(x >= SQUARES_PER_ROW || y >= SQUARES_PER_COLUMN)
        {
            return true;
        }
    }

    return false;
}


/*
 * Checks if the current clock intersects any other block on the field.
 */
bool Tetris::intersection(Block b)
{
    for(uint8_t i = 0; i < block.SQUARE_NUMBER; i++)
    {
        uint8_t x = b.squares[i].x + b.center.x;
        uint8_t y = b.squares[i].y + b.center.y;

        if(x >= SQUARES_PER_ROW || y >= SQUARES_PER_COLUMN)
        {
            return true;
        }

        if(field_squares[x][y].filled)
        {
            return true;
        }
    }

    return false;
}


void Tetris::refresh_screen()
{
    display.fill(BACKGROUND);
    draw_playfield();
    draw_blocks();
    display.flush();
}


void Tetris::draw_playfield()
{
    display.vline(X_LEFT, Y_BOTTOM, SQUARES_PER_COLUMN * SQUARE_WIDTH, 0xFFFFFF);
    display.vline(X_RIGHT, Y_BOTTOM, SQUARES_PER_COLUMN * SQUARE_WIDTH, 0xFFFFFF);
    display.line(X_RIGHT, Y_BOTTOM, X_LEFT, Y_BOTTOM, 0xFFFFFF);
}


void Tetris::draw_blocks()
{
    for(uint8_t x = 0; x < SQUARES_PER_ROW; x++)
    {
        for(uint8_t y = 0; y < SQUARES_PER_COLUMN; y++)
        {
            if(field_squares[x][y].filled)
            {
                draw_square(field_squares[x][y]);
            }
        }
    }

    draw_current_block();
}


void Tetris::draw_current_block()
{
    Block b(block);

    for(uint8_t i = 0; i < block.SQUARE_NUMBER; i++)
    {
        b.squares[i].x = b.center.x + b.squares[i].x;
        b.squares[i].y = b.center.y + b.squares[i].y;
        draw_square(b.squares[i]);
    }
}


// Draws one square of a tetris block.
void Tetris::draw_square(Square f)
{
    uint16_t x_pixel = f.x * SQUARE_WIDTH + 3 + X_LEFT;
    uint16_t y_pixel = f.y * SQUARE_WIDTH + 1 + Y_BOTTOM;

    display.filled_rectangle(x_pixel, y_pixel, 10, 10, f.color);
}





uint32_t Tetris::get_random_color()
{
    switch(rp2040.hwrand32() % 7)
    {
    case 0:
        return TFT_RED;
    case 1:
        return TFT_BLUE;
    case 2:
        return TFT_GREEN;
    case 3:
        return TFT_YELLOW;
    case 4:
        return TFT_CYAN;
    case 5:
        return TFT_ORANGE;
    case 6:
        return TFT_PURPLE;

    }

    return TFT_RED;
}


Block::Block() {}


Block::Block(const Block& b)
{
    color = b.color;
    center.x = b.center.x;
    center.y = b.center.y;


    for(uint8_t i = 0; i < b.SQUARE_NUMBER; i++)
    {
        squares[i].x = b.squares[i].x;
        squares[i].y = b.squares[i].y;
        squares[i].color = b.squares[i].color;
    }
}


// Creates a new block per random selection.
void Block::init(uint32_t color)
{
    this->color = color;
    squares[0].color = color;
    squares[1].color = color;
    squares[2].color = color;
    squares[3].color = color;
    center.x = 4;
    center.y = 14;

    Shape shape = Shape(rp2040.hwrand32() % 7);

    switch(shape)
    {
    case I:
        set_coords(-2, 1, 0);
        set_coords(-1, 1, 1);
        set_coords(0, 1, 2);
        set_coords(1, 1, 3);
        break;

    case J:
        set_coords(-1, 1, 0);
        set_coords(-1, 0, 1);
        set_coords(0, 0, 2);
        set_coords(1, 0, 3);
        break;

    case S:
        set_coords(1, 1, 0);
        set_coords(0, 1, 1);
        set_coords(0, 0, 2);
        set_coords(-1, 0, 3);
        break;

    case Z:
        set_coords(-1, 1, 0);
        set_coords(0, 1, 1);
        set_coords(0, 0, 2);
        set_coords(1, 0, 3);
        break;

    case O:
        set_coords(-1, 0, 0);
        set_coords(0, 0, 1);
        set_coords(-1, -1, 2);
        set_coords(0, -1, 3);
        break;

    case L:
        set_coords(-1, 0, 0);
        set_coords(0, 0, 1);
        set_coords(1, 0, 2);
        set_coords(1, 1, 3);
        break;

    case T:
        set_coords(-1, 0, 0);
        set_coords(0, 0, 1);
        set_coords(0, 1, 2);
        set_coords(1, 0, 3);
        break;
    }
}


void Block::set_coords(int8_t x, int8_t y, uint8_t index)
{
    if(index >= SQUARE_NUMBER)
    {
        return;
    }

    squares[index].x = x;
    squares[index].y = y;
}


int WALLKICK_NORMAL_180[4][11][2] = {
    {{1, 0}, {2, 0}, {1, 1}, {2, 1}, {-1, 0}, {-2, 0}, {-1, 1}, {-2, 1}, {0, -1}, {3, 0}, {-3, 0}},    // 0>>2─┐
    {{0, 1}, {0, 2}, {-1, 1}, {-1, 2}, {0, -1}, {0, -2}, {-1, -1}, {-1, -2}, {1, 0}, {0, 3}, {0, -3}}, // 1>>3─┼┐
    {{-1, 0}, {-2, 0}, {-1, -1}, {-2, -1}, {1, 0}, {2, 0}, {1, -1}, {2, -1}, {0, 1}, {-3, 0}, {3, 0}}, // 2>>0─┘│
    {{0, 1}, {0, 2}, {1, 1}, {1, 2}, {0, -1}, {0, -2}, {1, -1}, {1, -2}, {-1, 0}, {0, 3}, {0, -3}},    // 3>>1──┘
};
int WALLKICK_I_180[4][5][2] = {
    {{-1, 0}, {-2, 0}, {1, 0}, {2, 0}, {0, 1}},  // 0>>2─┐
    {{0, 1}, {0, 2}, {0, -1}, {0, -2}, {-1, 0}}, // 1>>3─┼┐
    {{1, 0}, {2, 0}, {-1, 0}, {-2, 0}, {0, -1}}, // 2>>0─┘│
    {{0, 1}, {0, 2}, {0, -1}, {0, -2}, {1, 0}},  // 3>>1──┘
};


void Block::rotate(int16_t degree)
{
    if(shape == O)
    {
        return;
    }

    // int8_t factor_1 = cos(radians(degree));
    // int8_t factor_2 = sin(radians(degree));

    int8_t factor_1 = 0;
    int8_t factor_2 = -1;


    for(uint8_t i = 0; i < SQUARE_NUMBER; i++)
    {
        Serial.println(squares[i].x);
        Serial.println(squares[i].y);
        Serial.println("-----------------");

        int8_t x_temp = squares[i].x;
        squares[i].x = squares[i].x * factor_1 - squares[i].y * factor_2;
        squares[i].y = x_temp * factor_2 + squares[i].y * factor_1;

        Serial.println(squares[i].x);
        Serial.println(squares[i].y);
        Serial.println("######################");
    }
}

void Block::move_left() { center.x--; }
void Block::move_right(){ center.x++; }
void Block::move_down(){ center.y--; }
