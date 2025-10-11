// Shared game configuration and types
#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

// Screen dimensions
#define SCREEN_WIDTH 396
#define SCREEN_HEIGHT 224

// Grid configuration
#define GRID_SIZE 7
#define GRID_CELL_SIZE 26
#define GRID_START_X ((SCREEN_WIDTH - (GRID_SIZE * GRID_CELL_SIZE)) / 2)
#define GRID_START_Y ((SCREEN_HEIGHT - (GRID_SIZE * GRID_CELL_SIZE)) / 2)

// Player position (bottom-middle cell)
#define PLAYER_ROW (GRID_SIZE - 1)
#define PLAYER_COL (GRID_SIZE / 2)

// Shape types
typedef enum {
	SHAPE_CIRCLE = 0,
	SHAPE_TRIANGLE = 1,
	SHAPE_OVAL = 2,
	SHAPE_SQUARE = 3,
	SHAPE_COUNT = 4
} shape_type_t;

// Colors (RGB565 format)
#define COLOR_BACKGROUND 0x31c8
#define COLOR_RED 0xf2ab
#define COLOR_GREEN 0x9dee
#define COLOR_YELLOW 0xfe8b
#define COLOR_BLUE 0x4bd2
#define COLOR_ORANGE 0xFC60
#define COLOR_CYAN   0x07FF
#define COLOR_BLACK  0x0000
#define COLOR_WHITE  0xFFFF

//blue
#define COLOR_BLUE_DARK   0x2A49
#define COLOR_EYE_PALE    0xFEE6
#define COLOR_MOUTH_PINK  0xF88B

//green
#define COLOR_GREEN_DARK  0x7DCC  // darker green shade for shadows/fin/legs
#define COLOR_IVORY       0xFFDD  // for teeth



// Game state
#define MAX_LIVES 3

// Global game state variables
extern int player_lives;
extern bool game_over;

// Function declarations
void draw_hearts_hud(void);
int check_damage_from_outlined_monsters(void);

#endif // GAME_H

