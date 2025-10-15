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

// Multiple jerks active at once
#define MAX_JERKS 4
extern jerk_t jerks[MAX_JERKS];
extern int active_jerk_count;
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
void jerk_kill_at(int row, int col);

// colors
#define COLOR_JERK_HEAD       0x5aa9
#define COLOR_JERK_BODY       0x31c6
#define COLOR_JERK_EYES       0xfe8b
#define COLOR_JERK_PUPIL      0xf26a
#define COLOR_JERK_HORN       0xddaa
#define COLOR_JERK_NIPPLE     0xEAA0
#define COLOR_JERK_OUTLINE    0x0000

#endif // BOSSES_H
