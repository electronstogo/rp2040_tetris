#include "TFT_eSPI.h"
#include "tetris.h"
#include <Arduino.h>
#include <algorithm>
#include <cstring>
#include <math.h>
#include <sys/_stdint.h>


uint32_t Tetris::debounce;
bool Tetris::move_left_flag;
bool Tetris::move_right_flag;
bool Tetris::rotate_left_flag;
bool Tetris::rotate_right_flag;

const float Tetris::SPEED_TABLE[12] = {48.0, 43.0, 38.0, 33.0, 28.0, 18.0, 13.0, 8.0,  6.0,  5.0, 5.0, 5.0};


Tetris::Tetris()
{
    pinMode(PIN_MOVE_LEFT, INPUT_PULLDOWN);
    pinMode(PIN_MOVE_RIGHT, INPUT_PULLDOWN);
    pinMode(PIN_ROTATE_LEFT, INPUT_PULLDOWN);
    pinMode(PIN_ROTATE_RIGHT, INPUT_PULLDOWN);

    score = 0;
    level = 6;
    game_over = false;
    cleared_lines = 0;

    // Movement delay of blocks depends on level.
    move_delay = (uint16_t)(SPEED_TABLE[level] / 60.0 * 1000.0);

    // Init field.
    for(uint8_t x = 0; x < SQUARES_PER_ROW; x++)
    {
        for(uint8_t y = 0; y < SQUARES_PER_COLUMN; y++)
        {
            field_squares[x][y].init(x, y, TFT_BLACK, false);
        }
    }

    // Initial screen.
    display.fill(BACKGROUND);
    draw_playfield();
    init_button_isr();

    // Create first block.
    block.init();

    // Enter main thread.
    run();
}


/*
 * Init button interrupts.
 */
void Tetris::init_button_isr()
{
    attachInterrupt(PIN_MOVE_LEFT, Tetris::move_left, RISING);
    attachInterrupt(PIN_MOVE_RIGHT, Tetris::move_right, RISING);
    attachInterrupt(PIN_ROTATE_LEFT, Tetris::rotate_left, RISING);
    attachInterrupt(PIN_ROTATE_RIGHT, Tetris::rotate_right, RISING);
}


/*
 * Clear button flags.
 */
void Tetris::clear_flags()
{
    move_left_flag = false;
    move_right_flag = false;
    rotate_left_flag = false;
    rotate_right_flag = false;
}

/*
 * Move block to the left.
 */
void Tetris::move_left()
{
    if(millis() - debounce > DEBOUNCE_DELAY)
    {
        move_left_flag = true;
        debounce = millis();
    }
}


/*
 * Move block to the right.
 */
void Tetris::move_right()
{
    if(millis() - debounce > DEBOUNCE_DELAY)
    {
        move_right_flag = true;
        debounce = millis();
    }
}

/*
 * Block rotation to the left.
 */
void Tetris::rotate_left()
{
    if(millis() - debounce > DEBOUNCE_DELAY)
    {
        rotate_left_flag = true;
        debounce = millis();
    }
}

/*
 * Block rotation to the right.
 */
void Tetris::rotate_right()
{
    if(millis() - debounce > DEBOUNCE_DELAY)
    {
        rotate_right_flag = true;
        debounce = millis();
    }
}


/*
 * Tetris thread.
 */
void Tetris::run()
{
    uint32_t timestamp = 0;
    uint32_t timestamp_draw = 0;

    while(true)
    {
        if(game_over)
        {
            fill_playfield();
            refresh_screen();
            continue;
        }

        if(move_left_flag)
        {
            move_block_left();
            refresh_screen();
            clear_flags();
            continue;
        }

        if(move_right_flag)
        {
            move_block_right();
            refresh_screen();
            clear_flags();
            continue;
        }

        if(rotate_left_flag)
        {
            rotate_block(Block::LEFT);
            refresh_screen();
            clear_flags();
            continue;
        }

        if(rotate_right_flag)
        {
            rotate_block(Block::RIGHT);
            refresh_screen();
            clear_flags();
            continue;
        }

        // Block movement.
        if(millis() - timestamp > move_delay)
        {
            move_block_downwards();
            timestamp = millis();
        }

        // Drawing of screen.
        if(millis() - timestamp_draw > 1000 / FPS)
        {
            refresh_screen();
            timestamp_draw = millis();
        }
    }
}


/*
 * Finish block after it reached ground.
 */
void Tetris::finish_block()
{
    for(uint8_t i = 0; i < block.SQUARE_NUMBER; i++)
    {
        uint8_t x = block.squares[i].x + block.center.x;
        uint8_t y = block.squares[i].y + block.center.y;

        field_squares[x][y].filled = true;
        field_squares[x][y].color = block.color;
    }

    clear_full_lines();
    block.init();
}


/*
 * Moves active tetris block to the left.
 */
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


/*
 * Moves active tetris block to the right.
 */
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


/*
 * Moves active tetris block downwards.
 */
void Tetris::move_block_downwards()
{
    if(block_finished())
    {
        finish_block();
        return;
    }

    block.move_down();
}


/*
 * Rotates block.
 */
void Tetris::rotate_block(Block::Direction d)
{
    Block b(block);
    b.rotate(d);

    if(intersect_borders(b) || intersection(b))
    {
        return;
    }

    block.rotate(d);
}


void Tetris::update_score(uint8_t full_lines)
{
    switch(full_lines)
    {
        case 1:
            score += ONE_LINE_POINTS * (level + 1);
            break;

        case 2:
            score += TWO_LINES_POINTS * (level + 1);
            break;

        case 3:
            score += THREE_LINES_POINTS * (level + 1);
            break;

        case 4:
            score += FOUR_LINES_POINTS * (level + 1);
            break;
    }

    cleared_lines += full_lines;

    if(cleared_lines >= (level + 1) * 10)
    {
        level++;
        move_delay = (uint16_t)(SPEED_TABLE[level] / 60.0 * 1000.0);
    }
}


/*
 * Clears lines filled by user.
 */
void Tetris::clear_full_lines()
{
    bool full = true;
    uint8_t full_lines = 0;

    for(uint8_t i = SQUARES_PER_COLUMN - 1; i > 0; i--)
    {
        full = true;

        for(uint8_t j = 0; j < SQUARES_PER_ROW; j++)
        {
            if(!field_squares[j][i].filled)
            {
                full = false;
                break;
            }
        }

        if(full)
        {
            full_lines++;
        }
        else if(full_lines)
        {
            for(uint8_t k = 0; k < full_lines; k++)
            {
                shift_field_down(i + k + 1);
                clear_line(k + 1);
            }

            update_score(full_lines);
            full_lines = 0;
        }
    }
}

/*
 * Shifts part of field down.
 */
void Tetris::shift_field_down(uint8_t index)
{
    for(uint8_t i = index; i > 0; i--)
    {
        shift_line_down(i);
    }
}


/*
 * Shifts down line.
 */
void Tetris::shift_line_down(uint8_t index)
{
    for(uint8_t i = 0; i < SQUARES_PER_ROW; i++)
    {
        field_squares[i][index].init(i, index, field_squares[i][index - 1].color, field_squares[i][index - 1].filled);
    }
}


/*
 * Clears line.
 */
void Tetris::clear_line(uint8_t index)
{
    for(uint8_t i = 0; i < SQUARES_PER_ROW; i++)
    {
        field_squares[i][index].init(i, index, TFT_BLACK, false);
    }
}


/*
 * Checks if the active block has reached any ground and is finished.
 */
bool Tetris::block_finished()
{
    Block b(block);
    b.move_down();

    for(uint8_t i = 0; i < block.SQUARE_NUMBER; i++)
    {
        if((block.center.y + block.squares[i].y == SQUARES_PER_COLUMN - 1) || intersection(b))
        {
            Serial.println(block.center.y + block.squares[i].y);


            if(block.center.y == 0)
            {
                game_over = true;
            }

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


        if(field_squares[x][y].filled)
        {
            return true;
        }
    }

    return false;
}


/**
 * Fills the whole playfield with blocks.
 */
void Tetris::fill_playfield()
{
    for(uint8_t x = 0; x < SQUARES_PER_ROW; x++)
    {
        for(uint8_t y = 0; y < SQUARES_PER_COLUMN; y++)
        {
            field_squares[x][y].init(x, y, TFT_SKYBLUE, true);
        }
    }
}

/*
 * Refresh the screen with current data.
 */
void Tetris::refresh_screen()
{
    display.fill(BACKGROUND);
    draw_blocks();
    draw_playfield();
    display.flush();
}


/*
 * Draws the tetris field.
 */
void Tetris::draw_playfield()
{
    display.vline(X_LEFT, 0, SQUARES_PER_COLUMN * SQUARE_WIDTH, TFT_WHITE);
    display.vline(X_LEFT + 1, 0, SQUARES_PER_COLUMN * SQUARE_WIDTH, TFT_WHITE);
    display.vline(X_RIGHT, 0, SQUARES_PER_COLUMN * SQUARE_WIDTH, TFT_WHITE);
    display.vline(X_RIGHT - 1, 0, SQUARES_PER_COLUMN * SQUARE_WIDTH, TFT_WHITE);
    display.line(X_RIGHT, 160 - 3, X_LEFT, 160 - 3, TFT_WHITE);
    display.line(X_RIGHT, 160 - 2, X_LEFT, 160 - 2, TFT_WHITE);

    // Draw score and level.
    display.number(score, 121, 0, TFT_WHITE);
    display.number(level, 20, 0, TFT_GREENYELLOW);
}


/*
 * Draws all blocks existing in the field.
 */
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


/*
 * Draws a tetris block.
 */
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


/*
 * Draws one square of a tetris block.
 */
void Tetris::draw_square(Square f)
{
    uint16_t x_pixel = f.x * SQUARE_WIDTH + 2 + X_LEFT;
    uint16_t y_pixel = f.y * SQUARE_WIDTH + 1;

    display.filled_rectangle(x_pixel, y_pixel, 10, 10, f.color);
}


Block::Block() {}


Block::Block(const Block& b)
{
    color = b.color;
    center.x = b.center.x;
    center.y = b.center.y;
    shape = b.shape;


    for(uint8_t i = 0; i < b.SQUARE_NUMBER; i++)
    {
        squares[i].x = b.squares[i].x;
        squares[i].y = b.squares[i].y;
        squares[i].color = b.squares[i].color;
    }
}

/*
 * Creates a new block per random selection.
 */
void Block::init()
{
    center.x = 4;
    center.y = 0;

    shape = Shape(rp2040.hwrand32() % 7);

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

    color = get_color();
    squares[0].color = color;
    squares[1].color = color;
    squares[2].color = color;
    squares[3].color = color;
}


/*
 * Get block color.
 */
uint32_t Block::get_color()
{
    switch(shape)
    {
        case L:
            return TFT_RED;
        case J:
            return TFT_BLUE;
        case S:
            return TFT_GREEN;
        case Z:
            return TFT_YELLOW;
        case O:
            return TFT_CYAN;
        case I:
            return TFT_ORANGE;
        case T:
            return TFT_PURPLE;
    }

    return TFT_RED;
}


/*
 * Set block coordinates.
 */
void Block::set_coords(int8_t x, int8_t y, uint8_t index)
{
    if(index >= SQUARE_NUMBER)
    {
        return;
    }

    squares[index].x = x;
    squares[index].y = y;
}


/*
 * 90° block rotation.
 */
void Block::rotate(Direction d)
{
    if(shape == O)
    {
        return;
    }
    else if(shape == I)
    {
    }

    // Simplified rotation matrix for left or right.
    int8_t factor = (d == RIGHT) ? 1 : -1;

    for(uint8_t i = 0; i < SQUARE_NUMBER; i++)
    {
        int8_t x_temp = squares[i].x;
        squares[i].x = squares[i].y * factor * (-1);
        squares[i].y = x_temp * factor;
    }
}

void Block::move_left() { center.x++; }
void Block::move_right() { center.x--; }
void Block::move_down() { center.y++; }


/*
 * Set square values.
 */
void Square::init(int8_t x, int8_t y, uint32_t color, bool filled)
{
    this->x = x;
    this->y = y;
    this->color = color;
    this->filled = filled;
}
