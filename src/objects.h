#ifndef OBJECTS_H
#define OBJECTS_H

#include <gint/display.h>
#include "game.h"

// Rock structure
typedef struct {
    int row;
    int col;
    bool active;
} rock_t;

void objects_init(void);
void objects_draw_all(void);
void objects_spawn_for_level(void);
void objects_reset(void);
bool rock_is_at(int row, int col);
int objects_get_rock_count(void);
rock_t* objects_get_rock(int index);

// global rock array
extern rock_t rocks[MAX_ROCKS];

#endif // OBJECTS_H
