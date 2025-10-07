#ifndef PLAYER_H
#define PLAYER_H

#include <gint/display.h>
#include "game.h"

void draw_player(void);
void set_player_pos(int row, int col);
void reset_player_pos(void);
void get_player_pos(int *row, int *col);

#endif // PLAYER_H

