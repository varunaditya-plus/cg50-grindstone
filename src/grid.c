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