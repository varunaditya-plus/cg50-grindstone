#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/timer.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
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

static int draw_big_number(int x, int y, int value, int scale, color_t color)
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

static void draw_grindstone_icon(int cx, int cy, int size) // make better in the future
{
	// Draw a gem using simple coloring
	for(int dy = -size; dy <= size; dy++) {
		int halfw = size - (dy < 0 ? -dy : dy);
		for(int dx = -halfw; dx <= halfw; dx++) {
			// Regions split by diagonals to mimic facets
			color_t col;
			if(dy < -size/4) {
				col = COLOR_YELLOW; // top facet
			}
			else if(dx < -dy) {
				col = COLOR_GREEN; // left facet
			}
			else if(dx > dy) {
				col = COLOR_BLUE; // right facet
			}
			else {
				col = COLOR_RED; // center facet
			}
			dpixel(cx + dx, cy + dy, col);
		}
	}
	// subtle white highlight line across top
	for(int dx = -size/2; dx <= size/2; dx++) {
		dpixel(cx + dx, cy - size/3, C_WHITE);
	}
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
	draw_big_number(x, y, count, scale, C_WHITE);
}

int main(void)
{
    srand(time(NULL));
    dclear(C_WHITE);
    
    draw_background();
    grindstone_clear_all();
    randomize_grid();
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
        else if(key.key == KEY_EXIT) {
            // Reset current chain on Exit
            chain_color_shape = SHAPE_COUNT;
            chain_len = 0;
            needs_redraw = true;
        }
        else {
            // Read simultaneous arrow key states to allow diagonals
            int drow = 0;
            int dcol = 0;
            if(keydown(KEY_UP)) drow -= 1;
            if(keydown(KEY_DOWN)) drow += 1;
            if(keydown(KEY_LEFT)) dcol -= 1;
            if(keydown(KEY_RIGHT)) dcol += 1;

            // If only one axis is pressed, give a brief grace period to catch the second key
            if((drow != 0) ^ (dcol != 0)) {
                // Longer grace window if starting a new chain, since timing is trickier
                int iterations = (chain_len == 0) ? 60000 : 8000;
                for(volatile int i = 0; i < iterations; i++) {
                    if(dcol == 0) {
                        if(keydown(KEY_LEFT)) { dcol = -1; break; }
                        if(keydown(KEY_RIGHT)) { dcol = 1; break; }
                    }
                    if(drow == 0) {
                        if(keydown(KEY_UP)) { drow = -1; break; }
                        if(keydown(KEY_DOWN)) { drow = 1; break; }
                    }
                }
            }

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