#ifndef CHAIN_H
#define CHAIN_H

#include <gint/display.h>
#include "game.h"

extern bool chain_planning;
extern shape_type_t chain_color_shape;
extern int chain_len;

bool add_chain_point(int row, int col);
bool add_chain_step(int drow, int dcol);
bool undo_last_chain_step(void);
void draw_chain(void);
void execute_chain(void);
void reset_chain_state(void);
void draw_chain_hud(void);

// Line utilities (used only here)
void draw_line(int x0, int y0, int x1, int y1, color_t color);
void draw_hline(int x0, int x1, int y, color_t color);
void draw_vline(int x, int y0, int y1, color_t color);
void draw_ortho_path(int x0, int y0, int x1, int y1, color_t color);

#endif // CHAIN_H

