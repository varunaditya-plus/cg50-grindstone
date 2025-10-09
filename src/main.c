#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/timer.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include "game.h"
#include "grid.h"
#include "monsters.h"
#include "player.h"
#include "chain.h"
#include "grindstone.h"

// 3x5 digit bitmaps for a simple font
static const unsigned char DIGIT_BITMAPS[10][5] = {
	{0b111,0b101,0b101,0b101,0b111}, // 0
	{0b010,0b110,0b010,0b010,0b111}, // 1
	{0b111,0b001,0b111,0b100,0b111}, // 2
	{0b111,0b001,0b111,0b001,0b111}, // 3
	{0b101,0b101,0b111,0b001,0b001}, // 4
	{0b111,0b100,0b111,0b001,0b111}, // 5
	{0b111,0b100,0b111,0b101,0b111}, // 6
	{0b111,0b001,0b001,0b001,0b001}, // 7
	{0b111,0b101,0b111,0b101,0b111}, // 8
	{0b111,0b101,0b111,0b001,0b111}, // 9
};

static void draw_big_digit(int x, int y, int digit, int scale, color_t color)
{
	if(digit < 0 || digit > 9) return;
	for(int r = 0; r < 5; r++) {
		unsigned char row = DIGIT_BITMAPS[digit][r];
		for(int c = 0; c < 3; c++) {
			if(row & (1 << (2 - c))) {
				int rx0 = x + c * scale;
				int ry0 = y + r * scale;
				// filled rectangle block for this pixel
				for(int py = 0; py < scale; py++) {
					for(int px = 0; px < scale; px++) {
						dpixel(rx0 + px, ry0 + py, color);
					}
				}
			}
		}
	}
}

static int draw_big_number(int x, int y, int value, int scale)
{
	// returns width drawn; supports 0..99
	if(value < 0) value = 0;
	if(value > 99) value = 99;
	int tens = value / 10;
	int ones = value % 10;
	int digit_w = 3 * scale;
	int spacing = scale; // space between digits
	int width = (tens > 0 ? (digit_w + spacing + digit_w) : digit_w);
	int draw_x = x;
	if(tens > 0) {
		draw_big_digit(draw_x, y, tens, scale, C_WHITE);
		draw_x += digit_w + spacing;
	}
	draw_big_digit(draw_x, y, ones, scale, C_WHITE);
	return width;
}

// Draw a rainbow colored diamond with pixelated 3D bevels
static void draw_rainbow_diamond(int cx, int cy, int size)
{
	// Define rainbow colors in order
	color_t rainbow_colors[] = {COLOR_RED, COLOR_YELLOW, COLOR_GREEN, COLOR_BLUE};
	int num_colors = 4;
	
	// Diamond shape: |x| + |y| <= size
	for(int y = -size; y <= size; y++) {
		for(int x = -size; x <= size; x++) {
			// Check if point is within diamond
			if(abs(x) + abs(y) <= size) {
				color_t pixel_color;
				
				// Determine if this pixel is on a bevel edge
				bool is_top_edge = (abs(x) + abs(y) == size) && (y < 0); // Top half of diamond edge
				bool is_bottom_edge = (abs(x) + abs(y) == size) && (y > 0); // Bottom half of diamond edge
				bool is_left_edge = (abs(x) + abs(y) == size) && (x < 0); // Left half of diamond edge
				bool is_right_edge = (abs(x) + abs(y) == size) && (x > 0); // Right half of diamond edge
				
				// Apply bevel colors
				if(is_top_edge || is_left_edge) {
					// White highlight on top and left edges
					pixel_color = C_WHITE;
				} else if(is_bottom_edge || is_right_edge) {
					// Black shadow on bottom and right edges
					pixel_color = C_BLACK;
				} else {
					// Interior: rainbow color based on angle
					double angle = atan2(y, x);
					// Convert from [-π, π] to [0, 2π]
					if(angle < 0) angle += 2 * 3.14159265359;
					// Map to color index (0 to num_colors-1)
					int color_index = (int)((angle / (2 * 3.14159265359)) * num_colors) % num_colors;
					pixel_color = rainbow_colors[color_index];
				}
				
				dpixel(cx + x, cy + y, pixel_color);
			}
		}
	}
}

static void draw_grindstone_icon(int cx, int cy, int size) // make better in the future
{
	draw_rainbow_diamond(cx, cy, size);
}

static void draw_chain_hud(void)
{
	// Bottom-right placement with margin
	int margin = 6;
	int scale = 4; // makes numbers larger
	int digit_w = 3 * scale;
	int spacing = scale;
	int count = chain_len;
	int is_two_digits = (count >= 10);
	int width = is_two_digits ? (digit_w + spacing + digit_w) : digit_w;
	int height = 5 * scale;

	int box_right = SCREEN_WIDTH - margin;
	int box_bottom = SCREEN_HEIGHT - margin;
	int x = box_right - width;
	int y = box_bottom - height;

	// Draw gem when chain is 10+
	if(count >= 10) {
		int gs_size = (height > width ? height : width) / 2 + 4;
		int cx = x + width/2;
		int cy = y + height/2;
		draw_grindstone_icon(cx, cy, gs_size);
	}

	// Draw big number on top
	draw_big_number(x, y, count, scale);
}

int main(void)
{
    srand(time(NULL));
    dclear(C_WHITE);
    
    draw_background();
    grindstone_clear_all();
    randomize_grid();
    clear_all_outlines();
    draw_grid_lines();
    draw_chain();
    draw_monsters();
    draw_player();
    draw_chain_hud();
    dupdate();
    
    while(1) {
        key_event_t key = getkey();
        
        // chain planning controls and randomize
        bool needs_redraw = false;
        if(key.key == KEY_F1) {
            // Reset planning state and monsters
            chain_color_shape = SHAPE_COUNT;
            chain_len = 0;
            grindstone_clear_all();
            randomize_grid();
            clear_all_outlines();
            needs_redraw = true;
        }
        else if(key.key == KEY_ACON) {
            // Clear current chain
            chain_color_shape = SHAPE_COUNT;
            chain_len = 0;
            needs_redraw = true;
        }
        else if(key.key == KEY_EXE) {
            execute_chain();
            // execute_chain handles redrawing
            continue;
        }
        else if(key.key == KEY_5) {
            if(undo_last_chain_step()) {
                // Redraw after undo
                draw_background();
                draw_grid_lines();
                draw_chain();
                draw_monsters();
                draw_player();
                draw_chain_hud();
            }
            // move to next loop
            dupdate();
            continue;
        }
        else if(key.key == KEY_EXIT) {
            // Reset current chain on Exit
            chain_color_shape = SHAPE_COUNT;
            chain_len = 0;
            needs_redraw = true;
        }
        else {
            // Map numpad keys to movement: 8 up, 2 down, 4 left, 6 right
            // 7 up-left, 9 up-right, 1 down-left, 3 down-right
            int drow = 0;
            int dcol = 0;

            // Prioritize diagonals if a diagonal key is pressed
            if(keydown(KEY_7)) { drow = -1; dcol = -1; }
            else if(keydown(KEY_9)) { drow = -1; dcol = 1; }
            else if(keydown(KEY_1)) { drow = 1; dcol = -1; }
            else if(keydown(KEY_3)) { drow = 1; dcol = 1; }
            else if(keydown(KEY_8)) { drow = -1; }
            else if(keydown(KEY_2)) { drow = 1; }
            else if(keydown(KEY_4)) { dcol = -1; }
            else if(keydown(KEY_6)) { dcol = 1; }

            if(drow != 0 || dcol != 0) {
                if(add_chain_step(drow, dcol)) needs_redraw = true;
            }
        }

        if(needs_redraw) {
            draw_background();
            draw_grid_lines();
            draw_chain();
            draw_monsters();
            draw_player();
            draw_chain_hud();
        }
        
        // Update display
        dupdate();
    }
    
    return 0;
}