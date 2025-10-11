#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <gint/display.h>
#include "grindstone.h"
#include "monsters.h"
#include "player.h"

static bool grindstone_grid[GRID_SIZE][GRID_SIZE];

void grindstone_clear_all(void)
{
	for(int r = 0; r < GRID_SIZE; r++) {
		for(int c = 0; c < GRID_SIZE; c++) {
			grindstone_grid[r][c] = false;
		}
	}
}

bool grindstone_is_at(int row, int col)
{
	if(row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) return false;
	return grindstone_grid[row][col];
}

void grindstone_place(int row, int col)
{
	if(row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) return;
	grindstone_grid[row][col] = true;
}

void grindstone_remove(int row, int col)
{
	if(row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) return;
	grindstone_grid[row][col] = false;
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

static void draw_grindstone_icon(int cx, int cy, int size)
{
	draw_rainbow_diamond(cx, cy, size);
}

void grindstone_draw_all(void)
{
	for(int row = 0; row < GRID_SIZE; row++) {
		for(int col = 0; col < GRID_SIZE; col++) {
			if(!grindstone_grid[row][col]) continue;
			int cx = GRID_START_X + col * GRID_CELL_SIZE + (GRID_CELL_SIZE / 2);
			int cy = GRID_START_Y + row * GRID_CELL_SIZE + (GRID_CELL_SIZE / 2);
			int size = (GRID_CELL_SIZE / 2) - 2;
			draw_grindstone_icon(cx, cy, size);
		}
	}
}

void grindstone_spawn_random(void)
{
	int player_row, player_col;
	get_player_pos(&player_row, &player_col);

	// Collect candidates
	int candidates[GRID_SIZE * GRID_SIZE][2];
	int count = 0;
	for(int r = 0; r < GRID_SIZE; r++) {
		for(int c = 0; c < GRID_SIZE; c++) {
			if(r == player_row && c == player_col) continue;
			if(grindstone_grid[r][c]) continue;
			candidates[count][0] = r;
			candidates[count][1] = c;
			count++;
		}
	}
	if(count == 0) return;

	int idx = rand() % count;
	int row = candidates[idx][0];
	int col = candidates[idx][1];

	// Remove any monster at that cell and place grindstone
	grid[row][col] = CREEP_COUNT;
	grindstone_place(row, col);
}


