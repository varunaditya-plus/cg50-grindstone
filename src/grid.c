#include <gint/display.h>
#include "grid.h"

void draw_background(void)
{
	for(int y = 0; y < SCREEN_HEIGHT; y++) {
		for(int x = 0; x < SCREEN_WIDTH; x++) {
			dpixel(x, y, COLOR_BACKGROUND);
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
				dpixel(x, py, C_BLACK);
			}
		}

		if(y < SCREEN_HEIGHT) {
			for(int px = GRID_START_X; px < GRID_START_X + (GRID_SIZE * GRID_CELL_SIZE); px++) {
				dpixel(px, y, C_BLACK);
			}
		}
	}
}


