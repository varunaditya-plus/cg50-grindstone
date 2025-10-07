#ifndef GRINDSTONE_H
#define GRINDSTONE_H

#include <gint/display.h>
#include "game.h"

// Initializes/clears all grindstones
void grindstone_clear_all(void);

// Returns true if a grindstone exists at (row, col)
bool grindstone_is_at(int row, int col);

// Place a grindstone at (row, col)
void grindstone_place(int row, int col);

// Remove a grindstone at (row, col) if present
void grindstone_remove(int row, int col);

// Spawn a grindstone at a random non-player cell; removes monster there
void grindstone_spawn_random(void);

// Draw all grindstones
void grindstone_draw_all(void);

#endif // GRINDSTONE_H


