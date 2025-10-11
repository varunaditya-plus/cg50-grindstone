#include <gint/display.h>
#include "player.h"

static int current_player_row = PLAYER_ROW;
static int current_player_col = PLAYER_COL;

void set_player_pos(int row, int col)
{
	current_player_row = row;
	current_player_col = col;
}

void reset_player_pos(void)
{
	current_player_row = PLAYER_ROW;
	current_player_col = PLAYER_COL;
}

void get_player_pos(int *row, int *col)
{
	if(row) *row = current_player_row;
	if(col) *col = current_player_col;
}

void draw_player(void)
{
	int center_x = GRID_START_X + (current_player_col * GRID_CELL_SIZE) + (GRID_CELL_SIZE / 2);
	int center_y = GRID_START_Y + (current_player_row * GRID_CELL_SIZE) + (GRID_CELL_SIZE / 2);
	int radius = GRID_CELL_SIZE / 3;
	
	// Draw a simple white circle for the player
	int r2 = radius * radius;
	for (int y = -radius; y <= radius; y++) {
		for (int x = -radius; x <= radius; x++) {
			if (x*x + y*y <= r2) {
				int px = center_x + x;
				int py = center_y + y;
				if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
					dpixel(px, py, C_WHITE);
				}
			}
		}
	}
}


