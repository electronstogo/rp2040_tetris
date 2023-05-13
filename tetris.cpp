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
            continue;
        }

        if(g_move_right)
        {
            move_block_right();
            refresh_screen();
            continue;
        }

        if(g_rotate_left)
        {
            rotate_block(-90);
            refresh_screen();
            continue;
        }

        if(g_rotate_left)
        {
            rotate_block(90);
            refresh_screen();
            continue;
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
        uint8_t x = block.squares[i].x;
        uint8_t y = block.squares[i].y;

        field_squares[x][y].filled = true;
        field_squares[x][y].color = block.color;
    }
}


void Tetris::move_block_left()
{
    copy_block(&block, &dummy_block);
    dummy_block.move_left();

    if(intersect_borders(dummy_block) || intersection(dummy_block))
    {
        return;
    }

    block.move_left();
}


void Tetris::move_block_right()
{
    copy_block(&block, &dummy_block);
    dummy_block.move_right();

    if(intersect_borders(dummy_block) || intersection(dummy_block))
    {
        return;
    }

    block.move_right();
}


void Tetris::move_block_downwards()
{
    copy_block(&block, &dummy_block);
    dummy_block.move_down();

    if(intersect_borders(dummy_block))
    {
      finish_block();
      return;
    }

    if(intersection(dummy_block))
    {
        return;
    }

    block.move_down();
}


void Tetris::rotate_block(int16_t degree)
{
    copy_block(&block, &dummy_block);
    dummy_block.rotate(degree);

    if(intersect_borders(dummy_block) || intersection(dummy_block))
    {
        return;
    }

    block.rotate(degree);
}


void Tetris::clear_full_lines() {}


/*
 * Checks if the current clock intersects with any field border.
 */
bool Tetris::intersect_borders(Block block)
{
    for(uint8_t i = 0; i < block.SQUARE_NUMBER; i++)
    {
        if(block.squares[i].x > SQUARES_PER_ROW)
        {
            return true;
        }

        if(block.squares[i].y > SQUARES_PER_COLUMN)
        {
            return true;
        }
    }

    return false;
}


/*
 * Checks if the current clock intersects any other block on the field.
 */
bool Tetris::intersection(Block block)
{
    for(uint8_t i = 0; i < block.SQUARE_NUMBER; i++)
    {
        uint8_t x = block.squares[i].x;
        uint8_t y = block.squares[i].y;

        if(block.squares[i].x == field_squares[x][y].x)
        {
            return true;
        }

        if(block.squares[i].y == field_squares[x][y].y)
        {
            return true;
        }
    }

    return false;
}


void Tetris::copy_block(Block* source, Block* destination) { memcpy(destination, source, sizeof(destination)); }


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
    for(uint8_t x = 0; x < block.SQUARE_NUMBER; x++)
    {
        draw_square(block.squares[x]);
    }
}


// Draws one square of a tetris block.
void Tetris::draw_square(Square f)
{
    uint16_t x_pixel = f.x * SQUARE_WIDTH + 1 + X_LEFT;
    uint16_t y_pixel = f.y * SQUARE_WIDTH + 1 + Y_BOTTOM;

    display.filled_rectangle(x_pixel, y_pixel, 10, 10, f.color);
}


uint32_t Tetris::get_random_color()
{
    uint8_t rand = random(6);

    return TFT_RED;
}


Block::Block() {}


// Creates a new block per random selection.
void Block::init(uint32_t color)
{
    this->color = color;
    squares[0].color = color;
    squares[1].color = color;
    squares[2].color = color;
    squares[3].color = color;

    Shape shape = Shape(random(7));

    switch(shape)
    {
    case I:
        for(uint8_t i = 0; i < 4; i++)
        {
            squares[i].x = 4 + i;
            squares[i].y = 15;
        }
        break;

    case J:
        for(uint8_t i = 0; i < 3; i++)
        {
            squares[i].x = 4 + i;
            squares[i].y = 15;
        }
        squares[3].x = 6;
        squares[3].y = 14;
        break;

    case S:
        squares[0].x = 4;
        squares[1].x = squares[2].x = 5;
        squares[3].x = 6;
        squares[0].y = squares[2].y = 15;
        squares[1].y = squares[3].y = 14;
        break;

    case Z:
        squares[0].x = 6;
        squares[1].x = squares[2].x = 5;
        squares[3].x = 4;
        squares[0].y = squares[2].y = 15;
        squares[1].y = squares[3].y = 14;
        break;

    case O:
        squares[0].x = squares[1].x = 4;
        squares[2].x = squares[3].x = 5;
        squares[0].y = squares[2].y = 15;
        squares[1].y = squares[3].y = 14;
        break;

    case L:
        for(uint8_t i = 0; i < 3; i++)
        {
            squares[i].x = 4 + i;
            squares[i].y = 15;
        }
        squares[3].x = 6;
        squares[3].y = 14;
        break;

    case T:
        for(uint8_t i = 0; i < 3; i++)
        {
            squares[i].x = 4 + i;
            squares[i].y = 15;
        }
        squares[3].x = 5;
        squares[3].y = 14;
        break;
    }
}

void Block::rotate(int16_t degree)
{
    if(shape == O)
    {
        return;
    }

    for(uint8_t i = 0; i < SQUARE_NUMBER; i++)
    {
        squares[i].x = (float)squares[i].x * cos(degree) + (float)squares[i].y * sin(degree);
        squares[i].y = (float)squares[i].x * sin(degree) + (float)squares[i].y * cos(degree);
    }
}

void Block::move_left()
{
    for(uint8_t i = 0; i < SQUARE_NUMBER; i++)
    {
        squares[i].x--;
    }
}

void Block::move_right()
{
    for(uint8_t i = 0; i < SQUARE_NUMBER; i++)
    {
        squares[i].x++;
    }
}

void Block::move_down()
{
    for(uint8_t i = 0; i < SQUARE_NUMBER; i++)
    {
        squares[i].y--;
    }
}
