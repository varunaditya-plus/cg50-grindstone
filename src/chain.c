#include <stdlib.h>
#include <gint/display.h>
#include <gint/keyboard.h>
#include "chain.h"
#include "monsters.h"
#include "player.h"
#include "grid.h"
#include "grindstone.h"

bool chain_planning = true;
shape_type_t chain_color_shape = SHAPE_COUNT;
int chain_len = 0;
static int chain_rows[GRID_SIZE * GRID_SIZE];
static int chain_cols[GRID_SIZE * GRID_SIZE];

static void get_cell_center(int row, int col, int *cx, int *cy)
{
	*cx = GRID_START_X + (col * GRID_CELL_SIZE) + (GRID_CELL_SIZE / 2);
	*cy = GRID_START_Y + (row * GRID_CELL_SIZE) + (GRID_CELL_SIZE / 2);
}

void draw_chain(void)
{
	if(!chain_planning || chain_len == 0) return;
	int prev_x, prev_y;
	int player_row, player_col;
	get_player_pos(&player_row, &player_col);
	get_cell_center(player_row, player_col, &prev_x, &prev_y);
	color_t color = shape_to_color(chain_color_shape);
	for(int i = 0; i < chain_len; i++) {
		int row = chain_rows[i];
		int col = chain_cols[i];
		int cx, cy;
		get_cell_center(row, col, &cx, &cy);
		// Draw straight segment between consecutive chain points (supports diagonals)
		draw_line(prev_x, prev_y, cx, cy, color);
		prev_x = cx; prev_y = cy;
	}
}

bool add_chain_point(int row, int col)
{
	if(!chain_planning) return false;
	int player_row, player_col;
	get_player_pos(&player_row, &player_col);
	if(row == player_row && col == player_col) return false;
	if(row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) return false;
	if(!grid_initialized) return false;

	int prev_row = (chain_len == 0) ? player_row : chain_rows[chain_len - 1];
	int prev_col = (chain_len == 0) ? player_col : chain_cols[chain_len - 1];
	int drow = abs(prev_row - row);
	int dcol = abs(prev_col - col);
	// Allow 8-directional adjacency (including diagonals), disallow staying in place
	if(drow == 0 && dcol == 0) return false;
	if(drow > 1 || dcol > 1) return false;

	if(chain_len >= 1) {
		int prevprev_row = (chain_len == 1) ? player_row : chain_rows[chain_len - 2];
		int prevprev_col = (chain_len == 1) ? player_col : chain_cols[chain_len - 2];
		if(row == prevprev_row && col == prevprev_col) return false;
	}

	// Disallow revisiting any cell already in the chain
	for(int i = 0; i < chain_len; i++) {
		if(chain_rows[i] == row && chain_cols[i] == col) return false;
	}

	shape_type_t s = grid[row][col];
	if(chain_color_shape == SHAPE_COUNT) {
		chain_color_shape = s;
	}
	if(s != chain_color_shape) return false;

	chain_rows[chain_len] = row;
	chain_cols[chain_len] = col;
	chain_len++;
	return true;
}

bool add_chain_step(int drow, int dcol)
{
	int player_row, player_col;
	get_player_pos(&player_row, &player_col);
	int base_row = (chain_len == 0) ? player_row : chain_rows[chain_len - 1];
	int base_col = (chain_len == 0) ? player_col : chain_cols[chain_len - 1];
	int row = base_row + drow;
	int col = base_col + dcol;

	if(chain_len >= 1) {
		int prevprev_row = (chain_len == 1) ? player_row : chain_rows[chain_len - 2];
		int prevprev_col = (chain_len == 1) ? player_col : chain_cols[chain_len - 2];
		if(row == prevprev_row && col == prevprev_col) {
			chain_len--;
			if(chain_len == 0) {
				chain_color_shape = SHAPE_COUNT;
			}
			return true;
		}
	}

	return add_chain_point(row, col);
}

void draw_line(int x0, int y0, int x1, int y1, color_t color)
{
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2;
	while(1) {
		dpixel(x0, y0, color);
		if(x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if(e2 >= dy) { err += dy; x0 += sx; }
		if(e2 <= dx) { err += dx; y0 += sy; }
	}
}

void draw_hline(int x0, int x1, int y, color_t color)
{
	if(x0 > x1) { int t = x0; x0 = x1; x1 = t; }
	for(int x = x0; x <= x1; x++) dpixel(x, y, color);
}

void draw_vline(int x, int y0, int y1, color_t color)
{
	if(y0 > y1) { int t = y0; y0 = y1; y1 = t; }
	for(int y = y0; y <= y1; y++) dpixel(x, y, color);
}

void draw_ortho_path(int x0, int y0, int x1, int y1, color_t color)
{
	draw_hline(x0, x1, y0, color);
	draw_vline(x1, y0, y1, color);
}


void execute_chain(void)
{
	if(!chain_planning || chain_len == 0) return;
    // Step through each chain point, move player, delete monster at that cell
    int executed_len = chain_len;
	int current_row = PLAYER_ROW;
	int current_col = PLAYER_COL;
	for(int i = 0; i < chain_len; i++) {
		int target_row = chain_rows[i];
		int target_col = chain_cols[i];
		set_player_pos(target_row, target_col);
		// Remove monster at target
		grid[target_row][target_col] = SHAPE_COUNT;
		// Redraw frame
		draw_background();
		draw_grid_lines();
		draw_monsters();
		draw_player();
		dupdate();
		// brief delay so steps are visible
		for(volatile int d=0; d<200000; d++); // simple busy-wait
		current_row = target_row;
		current_col = target_col;
	}

    // Clear chain state
    chain_len = 0;
    chain_color_shape = SHAPE_COUNT;

    // If the executed chain length was 10 or more, spawn a grindstone at random
    if(executed_len >= 10) {
        grindstone_spawn_random();
    }

    // Apply animated gravity/refill (treats grindstones as solid)
    animate_gravity_and_refill();

	// Redraw everything
	draw_background();
	draw_grid_lines();
	draw_monsters();
	draw_player();
	dupdate();
}
