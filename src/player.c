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
	// forward-declare for clarity
	extern void draw_circle(int center_x, int center_y, int radius, color_t color);
	draw_circle(center_x, center_y, radius, C_WHITE);
}


