#ifndef BOSSES_H
#define BOSSES_H

#include <gint/display.h>
#include "game.h"
#include "monsters.h"

// Jerk boss
typedef struct {
    int row;
    int col;
    bool active;
} jerk_t;

// Global jerk
extern jerk_t jerk;
extern bool jerk_killed_this_turn;

void jerk_spawn(void);
void jerk_move_towards_player(void);
void jerk_draw(void);
void jerk_reset(void);
bool jerk_is_at(int row, int col);
bool jerk_is_passable(void);
bool jerk_was_killed(void);
bool jerk_is_adjacent_to_player(void);
int jerk_damage_player_if_adjacent(void);

#endif // BOSSES_H
