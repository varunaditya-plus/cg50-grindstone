#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/timer.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "game.h"
#include "grid.h"
#include "monsters.h"
#include "player.h"
#include "chain.h"
#include "grindstone.h"
#include "font.h"

// Game state
int player_lives = MAX_LIVES;
bool game_over = false;

// Helper function to convert number to string
static void int_to_string(int value, char* buffer, int buffer_size)
{
	if(value < 0) value = 0;
	if(value > 99) value = 99;
	
	if(value >= 10) {
		snprintf(buffer, buffer_size, "%d", value);
	} else {
		snprintf(buffer, buffer_size, "%d", value);
	}
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

static void draw_heart(int x, int y, int size, color_t color)
{
	// Simple heart shape using two circles and a triangle
	int half_size = size / 2;
	
	// Left circle
	for(int dy = -half_size; dy <= 0; dy++) {
		for(int dx = -half_size; dx <= 0; dx++) {
			if(dx*dx + dy*dy <= half_size*half_size) {
				dpixel(x + dx, y + dy, color);
			}
		}
	}
	
	// Right circle
	for(int dy = -half_size; dy <= 0; dy++) {
		for(int dx = 0; dx <= half_size; dx++) {
			if(dx*dx + dy*dy <= half_size*half_size) {
				dpixel(x + dx, y + dy, color);
			}
		}
	}
	
	// Triangle bottom
	for(int dy = 0; dy <= half_size; dy++) {
		int width = half_size - dy;
		for(int dx = -width; dx <= width; dx++) {
			dpixel(x + dx, y + dy, color);
		}
	}
}

void draw_hearts_hud(void)
{
	int margin = 6;
	int heart_size = 8;
	int spacing = 4;
	int start_x = margin;
	int start_y = margin;
	
	for(int i = 0; i < MAX_LIVES; i++) {
		int x = start_x + i * (heart_size + spacing);
		color_t color = (i < player_lives) ? COLOR_RED : C_BLACK;
		draw_heart(x, start_y, heart_size, color);
	}
}

static void draw_game_over_screen(void)
{
	// Clear screen
	dclear(C_BLACK);
	
	// Draw "GAME OVER" text in the center
	const char* game_over_text = "GAME OVER";
	int text_width = strlen(game_over_text) * 8; // 8 pixels per character
	int text_x = (SCREEN_WIDTH - text_width) / 2;
	int text_y = SCREEN_HEIGHT / 2 - 4; // Center vertically
	
	font_draw_text(text_x, text_y, game_over_text);
}

int check_damage_from_outlined_monsters(void)
{
	int player_row, player_col;
	get_player_pos(&player_row, &player_col);
	int damage_count = 0;
	
	// Check only the 4 cardinal directions (up, down, left, right)
	int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}; // up, down, left, right
	
	for(int i = 0; i < 4; i++) {
		int drow = directions[i][0];
		int dcol = directions[i][1];
		
		int check_row = player_row + drow;
		int check_col = player_col + dcol;
		
		// Check bounds
		if(check_row < 0 || check_row >= GRID_SIZE || 
		   check_col < 0 || check_col >= GRID_SIZE) continue;
		
		// Check if there's an outlined monster at this position
		if(outlined[check_row][check_col] && grid[check_row][check_col] != SHAPE_COUNT) {
			// Remove outline from this monster
			outlined[check_row][check_col] = false;
			damage_count++; // Count this monster as dealing damage
		}
	}
	return damage_count; // Return number of monsters that dealt damage
}

void draw_chain_hud(void)
{
	// Bottom-right placement with margin
	int margin = 6;
	int count = chain_len;
	
	// Convert count to string
	char count_str[4];
	int_to_string(count, count_str, sizeof(count_str));
	
	// Calculate text position (bottom-right)
	int text_width = strlen(count_str) * 8; // 8 pixels per character
	int x = SCREEN_WIDTH - margin - text_width;
	int y = SCREEN_HEIGHT - margin - 8; // 8 pixels for font height

	// Draw gem when chain is 10+
	if(count >= 10) {
		int gs_size = 12;
		int cx = x - gs_size - 4; // Position gem to the left of text
		int cy = y + 4; // Center vertically with text
		draw_grindstone_icon(cx, cy, gs_size);
	}

	// Draw chain count using font
	font_draw_text(x, y, count_str);
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
    draw_hearts_hud();
    dupdate();
    
    while(1) {
        key_event_t key = getkey();
        
        // Check for game over state
        if(game_over) {
            draw_game_over_screen();
            dupdate();
            continue;
        }
        
        // chain planning controls and randomize
        bool needs_redraw = false;
        if(key.key == KEY_F1) {
            // Reset planning state and monsters
            chain_color_shape = SHAPE_COUNT;
            chain_len = 0;
            grindstone_clear_all();
            randomize_grid();
            clear_all_outlines();
            player_lives = MAX_LIVES; // Reset lives
            game_over = false;
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

            // If we have movement, add it to the chain
            if(drow != 0 || dcol != 0) {
                if(add_chain_step(drow, dcol)) {
                    needs_redraw = true;
                }
            }
		}

        if(needs_redraw) {
            draw_background();
            draw_grid_lines();
            draw_chain();
            draw_monsters();
            draw_player();
            draw_chain_hud();
            draw_hearts_hud();
        }
        
        // Update display
        dupdate();
    }
    
    return 0;
}