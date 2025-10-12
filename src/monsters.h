#ifndef MONSTERS_H
#define MONSTERS_H

#include <gint/display.h>
#include "game.h"
#include "grindstone.h"

extern creep_type_t grid[GRID_SIZE][GRID_SIZE];
extern bool hostile[GRID_SIZE][GRID_SIZE];
extern bool grid_initialized;

void randomize_grid(void);
void draw_monsters(void);
void apply_gravity_and_refill(void);
void animate_gravity_and_refill(void);
void draw_creep(int x, int y, creep_type_t creep);
void draw_circle(int center_x, int center_y, int radius, color_t color);
void draw_triangle(int x, int y, int size, color_t color);
void draw_oval(int x, int y, int width, int height, color_t color);
void draw_square(int x, int y, int size, color_t color);
color_t creep_to_color(creep_type_t creep);
void add_random_hostile_after_chain(void);
void clear_all_hostile(void);

#endif // MONSTERS_H

