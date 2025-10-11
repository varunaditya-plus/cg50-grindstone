#include <gint/display.h>
#include "grid.h"
#include "game.h"

void draw_background(void)
{
	for(int y = 0; y < SCREEN_HEIGHT; y++) {
		for(int x = 0; x < SCREEN_WIDTH; x++) {
			dpixel(x, y, COLOR_BACKGROUND);
		}
	}
}

void draw_grid_background(void)
{
	// Fill each grid cell with the background color
	for(int row = 0; row < GRID_SIZE; row++) {
		for(int col = 0; col < GRID_SIZE; col++) {
			int start_x = GRID_START_X + (col * GRID_CELL_SIZE);
			int start_y = GRID_START_Y + (row * GRID_CELL_SIZE);
			
			// Fill the entire cell with the background color
			for(int y = start_y; y < start_y + GRID_CELL_SIZE; y++) {
				for(int x = start_x; x < start_x + GRID_CELL_SIZE; x++) {
					dpixel(x, y, COLOR_GRID_BACKGROUND);
				}
			}
		}
	}
}

void draw_grid_lines(void)
{
	for(int i = 0; i <= GRID_SIZE; i++) {
		int x = GRID_START_X + (i * GRID_CELL_SIZE);
		int y = GRID_START_Y + (i * GRID_CELL_SIZE);

		if(x < SCREEN_WIDTH) {
			for(int py = GRID_START_Y; py < GRID_START_Y + (GRID_SIZE * GRID_CELL_SIZE); py++) {
				dpixel(x, py, COLOR_GRID);
			}
		}

		if(y < SCREEN_HEIGHT) {
			for(int px = GRID_START_X; px < GRID_START_X + (GRID_SIZE * GRID_CELL_SIZE); px++) {
				dpixel(px, y, COLOR_GRID);
			}
		}
	}
}


