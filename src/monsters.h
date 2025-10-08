#ifndef MONSTERS_H
#define MONSTERS_H

#include <gint/display.h>
#include "game.h"
#include "grindstone.h"

extern shape_type_t grid[GRID_SIZE][GRID_SIZE];
extern bool grid_initialized;
extern bool outlined[GRID_SIZE][GRID_SIZE];

void randomize_grid(void);
void draw_monsters(void);
void apply_gravity_and_refill(void);
void animate_gravity_and_refill(void);
void draw_shape(int x, int y, shape_type_t shape);
void draw_outline_cell(int row, int col, int thickness, color_t color);
void add_random_outlines_after_chain(void);
void clear_all_outlines(void);
void draw_circle(int center_x, int center_y, int radius, color_t color);
void draw_triangle(int x, int y, int size, color_t color);
void draw_oval(int x, int y, int width, int height, color_t color);
void draw_square(int x, int y, int size, color_t color);
color_t shape_to_color(shape_type_t shape);

#endif // MONSTERS_H

