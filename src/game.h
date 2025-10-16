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

// Creep types
typedef enum {
	CREEP_RED = 0,
	CREEP_GREEN = 1,
	CREEP_YELLOW = 2,
	CREEP_BLUE = 3,
	CREEP_COUNT = 4
} creep_type_t;

// Colors (RGB565 format)
#define COLOR_BACKGROUND 0x2167
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

//red
#define COLOR_RED_DARK 0xA208

// player
#define COLOR_PLAYER_SKIN     0x259B
#define COLOR_PLAYER_FACE     0x0011
#define COLOR_PLAYER_EYE_WHITE 0xFFFF
#define COLOR_PLAYER_EYE_RED   0xF800
#define COLOR_PLAYER_MOUTH     0xBA80
#define COLOR_PLAYER_PANTS     0x49A8
#define COLOR_PLAYER_LEG       0x0841
#define COLOR_PLAYER_HAND      0x2104
#define COLOR_PLAYER_OUTLINE   0x0000
#define COLOR_PLAYER_MOUSTACHE 0xFBA0

// Grid colors
#define COLOR_GRID 0x424a
#define COLOR_GRID_BACKGROUND 0x2125

// Game state
#define MAX_LIVES 3
#define MAX_ROCKS 20

// Global game state variables
extern int player_lives;
extern bool game_over;
extern bool game_won;

// Function declarations
void draw_hearts_hud(void);
void draw_level_hud(void);
void draw_win_condition_hud(void);
int check_damage_from_hostile_monsters(void);

#endif // GAME_H

