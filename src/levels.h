#ifndef LEVELS_H
#define LEVELS_H

#include <gint/display.h>
#include "game.h"

// Level structure
typedef struct {
    int jerk_spawn_row;
    int jerk_spawn_col;
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

#endif // LEVELS_H
