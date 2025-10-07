#include <stdlib.h>
#include <gint/display.h>
#include "monsters.h"
#include "grid.h"
#include "player.h"
#include "grindstone.h"
#include "chain.h"

shape_type_t grid[GRID_SIZE][GRID_SIZE];
bool grid_initialized = false;

void draw_shape(int x, int y, shape_type_t shape)
{
	int size = GRID_CELL_SIZE / 2;
	int circle_size = GRID_CELL_SIZE / 3;

	switch(shape) {
		case SHAPE_CIRCLE:
			draw_circle(x, y, circle_size, COLOR_RED);
			break;
		case SHAPE_TRIANGLE:
			draw_triangle(x, y, size, COLOR_GREEN);
			break;
		case SHAPE_OVAL:
			draw_oval(x - size/2, y - size/2, size, size, COLOR_YELLOW);
			break;
		case SHAPE_SQUARE:
			draw_square(x - size/2, y - size/2, size, COLOR_BLUE);
			break;
		default:
			break;
	}
}

void draw_monsters(void)
{
	if(!grid_initialized) return;
	int player_row, player_col;
	get_player_pos(&player_row, &player_col);
	for(int row = 0; row < GRID_SIZE; row++) {
		for(int col = 0; col < GRID_SIZE; col++) {
            if(row == player_row && col == player_col) continue;
            if(grid[row][col] == SHAPE_COUNT) continue;
			int x = GRID_START_X + (col * GRID_CELL_SIZE) + (GRID_CELL_SIZE / 2);
			int y = GRID_START_Y + (row * GRID_CELL_SIZE) + (GRID_CELL_SIZE / 2);
			draw_shape(x, y, grid[row][col]);
		}
	}

    // Draw grindstones on top
    grindstone_draw_all();
}

void randomize_grid(void)
{
	for(int row = 0; row < GRID_SIZE; row++) {
		for(int col = 0; col < GRID_SIZE; col++) {
			if(row == PLAYER_ROW && col == PLAYER_COL) continue;
			grid[row][col] = (shape_type_t)(rand() % SHAPE_COUNT);
		}
	}
	grid_initialized = true;
}

color_t shape_to_color(shape_type_t shape)
{
	switch(shape) {
		case SHAPE_CIRCLE: return COLOR_RED;
		case SHAPE_TRIANGLE: return COLOR_GREEN;
		case SHAPE_OVAL: return COLOR_YELLOW;
		case SHAPE_SQUARE: return COLOR_BLUE;
		default: return C_WHITE;
	}
}

void draw_circle(int center_x, int center_y, int radius, color_t color)
{
	for(int y = -radius; y <= radius; y++) {
		for(int x = -radius; x <= radius; x++) {
			if(x*x + y*y <= radius*radius) {
				dpixel(center_x + x, center_y + y, color);
			}
		}
	}
}

void draw_triangle(int x, int y, int size, color_t color)
{
	int half_size = size / 2;
	for(int i = 0; i < size; i++) {
		int line_width = i + 1;
		int start_x = x - line_width/2;
		int line_y = y - half_size + i;
		for(int j = 0; j < line_width; j++) {
			if(start_x + j >= 0 && start_x + j < SCREEN_WIDTH && 
			   line_y >= 0 && line_y < SCREEN_HEIGHT) {
				dpixel(start_x + j, line_y, color);
			}
		}
	}
}

void draw_oval(int x, int y, int width, int height, color_t color)
{
	int center_x = x + width / 2;
	int center_y = y + height / 2;
	int a = width / 2;
	int b = height / 2;
	for(int py = y; py < y + height; py++) {
		for(int px = x; px < x + width; px++) {
			int dx = px - center_x;
			int dy = py - center_y;
			if((dx * dx * b * b + dy * dy * a * a) <= (a * a * b * b)) {
				dpixel(px, py, color);
			}
		}
	}
}

void draw_square(int x, int y, int size, color_t color)
{
	for(int py = y; py < y + size; py++) {
		for(int px = x; px < x + size; px++) {
			if(px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
				dpixel(px, py, color);
			}
		}
	}
}

void apply_gravity_and_refill(void)
{
	if(!grid_initialized) return;
	int player_row, player_col;
	get_player_pos(&player_row, &player_col);
	for(int col = 0; col < GRID_SIZE; col++) {
		int write_row = GRID_SIZE - 1;
		if(write_row == player_row && col == player_col) write_row--;
		for(int row = GRID_SIZE - 1; row >= 0; row--) {
			if(row == player_row && col == player_col) continue;
            if(grid[row][col] != SHAPE_COUNT || grindstone_is_at(row, col)) {
				if(write_row != row) {
                    // If destination has a grindstone, move write_row above it
                    if(grindstone_is_at(write_row, col)) {
                        write_row--;
                        if(write_row == player_row && col == player_col) write_row--;
                    }
                    if(grid[row][col] != SHAPE_COUNT) {
                        grid[write_row][col] = grid[row][col];
                        grid[row][col] = SHAPE_COUNT;
                    }
				}
				write_row--;
				if(write_row == player_row && col == player_col) write_row--;
			}
		}
		for(int row = write_row; row >= 0; row--) {
			if(row == player_row && col == player_col) continue;
			grid[row][col] = (shape_type_t)(rand() % SHAPE_COUNT);
		}
	}
}

// Animate gravity by repeatedly applying single-row drops and redrawing.
// This keeps the player cell empty while monsters fall into place.
void animate_gravity_and_refill(void)
{
    if(!grid_initialized) return;
    int player_row, player_col;
    get_player_pos(&player_row, &player_col);

    // Repeat passes until no movement happens in a full sweep.
    bool moved;
    do {
        moved = false;
        for(int col = 0; col < GRID_SIZE; col++) {
            for(int row = GRID_SIZE - 2; row >= 0; row--) {
                if(row == player_row && col == player_col) continue;
                if(grid[row][col] == SHAPE_COUNT) continue;
                int below = row + 1;
                // Skip the player cell when checking the below position
                if(below == player_row && col == player_col) below++;
                if(below < GRID_SIZE && grid[below][col] == SHAPE_COUNT) {
                    grid[below][col] = grid[row][col];
                    grid[row][col] = SHAPE_COUNT;
                    moved = true;
                }
            }
        }

        // Draw intermediate frame if anything moved
        if(moved) {
            draw_background();
            draw_grid_lines();
            draw_chain();
            draw_monsters();
            draw_player();
            dupdate();
            // Small delay for visible falling
            for(volatile int d=0; d<80000; d++);
        }
    } while(moved);

    // Finally, refill any remaining empty cells (excluding player cell) from the top,
    // animating the spawn descent to match falling behavior.
    for(int col = 0; col < GRID_SIZE; col++) {
        // Count empties above the lowest filled cell (skipping player position)
        int empties = 0;
        for(int row = 0; row < GRID_SIZE; row++) {
            if(row == player_row && col == player_col) continue;
            if(grid[row][col] == SHAPE_COUNT) empties++;
        }
        // For each empty, spawn above the grid and slide down per step
        while(empties-- > 0) {
            // Find the first empty from top to place a new tile
            int spawn_row = 0;
            while(spawn_row < GRID_SIZE && !(grid[spawn_row][col] == SHAPE_COUNT && !(spawn_row == player_row && col == player_col))) {
                spawn_row++;
            }
            if(spawn_row >= GRID_SIZE) break;
            grid[spawn_row][col] = (shape_type_t)(rand() % SHAPE_COUNT);

            // Let the spawned tile fall until it rests
            int r = spawn_row;
            while(true) {
                int below = r + 1;
                if(below == player_row && col == player_col) below++;
                if(below < GRID_SIZE && grid[below][col] == SHAPE_COUNT) {
                    grid[below][col] = grid[r][col];
                    grid[r][col] = SHAPE_COUNT;
                    r = below;
                    draw_background();
                    draw_grid_lines();
                    draw_monsters();
                    draw_player();
                    dupdate();
                    for(volatile int d=0; d<80000; d++);
                } else {
                    break;
                }
            }
        }
    }
}


