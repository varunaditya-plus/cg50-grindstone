#include <stdlib.h>
#include <stdbool.h>
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

static void draw_grindstone_icon(int cx, int cy, int size)
{
	for(int dy = -size; dy <= size; dy++) {
		int halfw = size - (dy < 0 ? -dy : dy);
		for(int dx = -halfw; dx <= halfw; dx++) {
			color_t col;
			if(dy < -size/4) col = COLOR_YELLOW;
			else if(dx < -dy) col = COLOR_GREEN;
			else if(dx > dy) col = COLOR_BLUE;
			else col = COLOR_RED;
			dpixel(cx + dx, cy + dy, col);
		}
	}
	for(int dx = -size/2; dx <= size/2; dx++) dpixel(cx + dx, cy - size/3, C_WHITE);
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
	grid[row][col] = SHAPE_COUNT;
	grindstone_place(row, col);
}


