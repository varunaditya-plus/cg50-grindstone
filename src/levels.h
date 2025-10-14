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
    bool jerk_enabled; // whether this level has a jerk boss
    // optional second jerk for multi-jerk levels LATER: make it so you can add as many in an array or sm
    int jerk2_spawn_row;
    int jerk2_spawn_col;
    int jerk_count; // number of jerks to defeat this level (0,1,2)
    int monster_count;
    int rock_count;
    position_t* rock_positions;
    const char* name;
    const char* win_condition_text;
    int target_smashes; // 0 means jerk-based win condition, >0 means smash n creeps
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

// Creep smash tracking for target-based win conditions
void levels_add_smashed(int n);
int levels_get_remaining_smashes(void);

// Multi-jerk support
void levels_reset_jerk_progress(void);
bool levels_get_next_jerk_spawn(int *row, int *col);
void levels_on_jerk_spawned(void);

// Monster configuration
int levels_get_monster_count(void);

#endif // LEVELS_H
