#ifndef LEVELS_H
#define LEVELS_H

#include <gint/display.h>
#include "game.h"

// Position structure for rocks
typedef struct {
    int row;
    int col;
} position_t;

// Level structure
typedef struct {
    int jerk_spawn_row;
    int jerk_spawn_col;
    int monster_count;
    int rock_count;
    position_t* rock_positions;
    const char* name;
} level_t;

// Global level management
extern int current_level;
extern level_t levels[];

void levels_init(void);
void levels_next_level(void);
void levels_reset_to_level(int level);
level_t* levels_get_current(void);
int levels_get_count(void);

// Win condition and jerk kill detection
void levels_mark_jerk_killed(void);
bool levels_check_win_condition(void);
void levels_handle_level_completion(void);

// Monster configuration
int levels_get_monster_count(void);

#endif // LEVELS_H
