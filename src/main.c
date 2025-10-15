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
#include "bosses.h"
#include "levels.h"
#include "objects.h"

// Game state
int player_lives = MAX_LIVES;
bool game_over = false;
bool game_won = false;

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
    if (size < 4) return;

    int r = size / 3;
    int offset = r;
    int tri_h = r + (size - 2*r);
    if (tri_h < r) tri_h = r;

    int cxL = x - offset;
    int cxR = x + offset;
    int cy  = y;

    int minX = x - (offset + r);
    int maxX = x + (offset + r);
    int minY = y - r;
    int maxY = y + tri_h;

    for (int py = minY; py <= maxY; ++py) {
        for (int px = minX; px <= maxX; ++px) {

            int dxL = px - cxL;
            int dxR = px - cxR;
            int dyC = py - cy;

            int inside = 0;

            if (py <= y) {
                if (dxL*dxL + dyC*dyC <= r*r || dxR*dxR + dyC*dyC <= r*r) {
                    inside = 1;
                }
            } else {
                int dy = py - y;
                int hw = ( (offset + r) * (tri_h - dy) ) / tri_h;
                if (px >= x - hw && px <= x + hw) {
                    inside = 1;
                }
            }

            if (inside) dpixel(px, py, color);
        }
    }
}


void draw_hearts_hud(void)
{
	int heart_size = 20;
	int spacing = 8;
	int start_x = 26;
	int start_y = 35;
	
	for(int i = 0; i < MAX_LIVES; i++) {
		int x = start_x + i * (heart_size + spacing);
		color_t color = (i < player_lives) ? COLOR_RED : C_BLACK;
		draw_heart(x, start_y, heart_size, color);
	}
}

void draw_win_condition_hud(void)
{
	level_t* current = levels_get_current();
    if(!current) return;
    // Top-right placement with margin
    int margin = 6;
    int y = margin;
    if(current->target_smashes > 0) {
        // Show remaining smashes
        int remaining = levels_get_remaining_smashes();
        char buf[24];
        snprintf(buf, sizeof(buf), "SMASH %d", remaining);
        int text_width = strlen(buf) * 8;
        int x = SCREEN_WIDTH - margin - text_width;
        font_draw_text(x, y, buf);
    } else if(current->win_condition_text) {
        int text_width = strlen(current->win_condition_text) * 8;
        int x = SCREEN_WIDTH - margin - text_width;
        font_draw_text(x, y, current->win_condition_text);
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

static void draw_game_win_screen(void)
{
	dclear(C_BLACK);
	
	// Draw "GAME WIN" text in the center
	const char* game_win_text = "GAME WIN";
	int text_width = strlen(game_win_text) * 8; // 8 pixels per character
	int text_x = (SCREEN_WIDTH - text_width) / 2;
	int text_y = SCREEN_HEIGHT / 2 - 4; // Center vertically
	
	font_draw_text(text_x, text_y, game_win_text);
}

int check_damage_from_hostile_monsters(void)
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
		
		// Check if there's a hostile monster at this position
		if(hostile[check_row][check_col] && grid[check_row][check_col] != CREEP_COUNT) {
			// Remove hostile status from this monster
			hostile[check_row][check_col] = false;
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
    dclear(COLOR_BACKGROUND);
    
    // Title screen
    const char* title = "GRINDSTONE";
    int scale = 3;
    int title_w = font_text_width_scaled(title, scale);
    int title_x = (SCREEN_WIDTH - title_w) / 2;
    int title_y = SCREEN_HEIGHT / 2 - 12 - 5;
    // Animated title
    {
        int colors[4] = {COLOR_RED, COLOR_GREEN, COLOR_YELLOW, COLOR_BLUE};
        const char* prompt = "PRESS EXE TO START";
        int prompt_w = strlen(prompt) * 8;
        int prompt_x = (SCREEN_WIDTH - prompt_w) / 2;
        int prompt_y = title_y + 8*scale + 10;
        int offset = 0;
        for(;;) {
            dclear(COLOR_BACKGROUND);
            int len = strlen(title);
            for(int i = 0; i < len; i++) {
                char ch[2];
                ch[0] = title[i];
                ch[1] = '\0';
                int cx = title_x + i * 8 * scale;
                int color_index = (i + offset) % 4;
                font_draw_text_scaled(cx, title_y, ch, scale, colors[color_index]);
            }
            font_draw_text(prompt_x, prompt_y, prompt);
            dupdate();

            for(volatile int d = 0; d < 1000000; d++);
            if(keydown(KEY_EXE)) break;
            offset = (offset + 1) % 4;
        }
    }
    
    dclear(C_WHITE);
    
    levels_init();
    objects_init();
    draw_background();
    grindstone_clear_all();
    randomize_grid();
    clear_all_hostile();
    reset_player_pos();
    jerk_spawn(); // Now uses level-specific spawn positions
    objects_spawn_for_level(); // Spawn rocks for current level
    draw_monsters();
    objects_draw_all(); // Draw rocks
    jerk_draw();
    draw_player();
    draw_chain();
    draw_chain_hud();
    draw_hearts_hud();
    draw_win_condition_hud();
    dupdate();
    
    while(1) {
        key_event_t key = getkey();
        
        // Check for game over state
        if(game_over) {
            draw_game_over_screen();
            dupdate();
            continue;
        }
        
        // Check for game win state
        if(game_won) {
            draw_game_win_screen();
            dupdate();
            continue;
        }
        
        // chain planning controls and randomize
        bool needs_redraw = false;
        if(key.key == KEY_F1) {
            // Reset planning state and monsters
            chain_color_shape = CREEP_COUNT;
            chain_len = 0;
            grindstone_clear_all();
            randomize_grid();
            clear_all_hostile();
            jerk_reset();
            reset_player_pos();
            jerk_spawn(); // Now uses level-specific spawn positions
            objects_reset();
            objects_spawn_for_level(); // Spawn rocks for current level
            player_lives = MAX_LIVES; // Reset lives
            game_over = false;
            game_won = false;
            // Reset level progress counters
            levels_reset_to_level(current_level);
            needs_redraw = true;
        }
        else if(key.key == KEY_F2) {
            // Advance to next level
            levels_next_level();
            chain_color_shape = CREEP_COUNT;
            chain_len = 0;
            grindstone_clear_all();
            randomize_grid();
            clear_all_hostile();
            jerk_reset();
            reset_player_pos();
            jerk_spawn(); // Spawn jerk at new level position
            objects_reset();
            objects_spawn_for_level(); // Spawn rocks for new level
            needs_redraw = true;
        }
        else if(key.key == KEY_ACON) {
            // Clear current chain
            chain_color_shape = CREEP_COUNT;
            chain_len = 0;
            needs_redraw = true;
        }
        else if(key.key == KEY_EXE) {
            execute_chain();
            continue;
        }
        else if(key.key == KEY_EXIT) {
            // Reset current chain on Exit
            chain_color_shape = CREEP_COUNT;
            chain_len = 0;
            needs_redraw = true;
        }
        else {
            // Read simultaneous arrow key states to allow diagonals
            int drow = 0;
            int dcol = 0;
            
            // shift modifier diagonals - clockwise
            if(keydown(KEY_SHIFT)) {
                if(keydown(KEY_UP)) {
                    // Shift+Up = top-right diagonal
                    drow = -1;
                    dcol = 1;
                }
                else if(keydown(KEY_RIGHT)) {
                    // Shift+Right = bottom-right diagonal
                    drow = 1;
                    dcol = 1;
                }
                else if(keydown(KEY_DOWN)) {
                    // Shift+Down = bottom-left diagonal
                    drow = 1;
                    dcol = -1;
                }
                else if(keydown(KEY_LEFT)) {
                    // Shift+Left = top-left diagonal
                    drow = -1;
                    dcol = -1;
                }
            }
            else {
                // Regular arrow key movement (existing logic)
                if(keydown(KEY_UP)) drow -= 1;
                if(keydown(KEY_DOWN)) drow += 1;
                if(keydown(KEY_LEFT)) dcol -= 1;
                if(keydown(KEY_RIGHT)) dcol += 1;
            }

            if(!keydown(KEY_SHIFT) && (drow != 0) ^ (dcol != 0)) {
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
            draw_monsters();
            objects_draw_all(); // Draw rocks
            jerk_draw();
            draw_player();
            draw_chain();
            draw_chain_hud();
            draw_hearts_hud();
            draw_win_condition_hud();
        }
        
        // Update display
        dupdate();
    }
    
    return 0;
}