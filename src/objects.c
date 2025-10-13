#include <stdlib.h>
#include <gint/display.h>
#include "objects.h"
#include "grid.h"
#include "player.h"
#include "grindstone.h"
#include "bosses.h"
#include "levels.h"

// Global rock array
rock_t rocks[MAX_ROCKS];
static int active_rock_count = 0;

void objects_init(void)
{
    objects_reset();
}

void objects_reset(void)
{
    for(int i = 0; i < MAX_ROCKS; i++) {
        rocks[i].active = false;
        rocks[i].row = 0;
        rocks[i].col = 0;
    }
    active_rock_count = 0;
}

void objects_spawn_for_level(void)
{
    objects_reset();
    
    // Get current level
    level_t* current_level = levels_get_current();
    if(!current_level) return;
    
    // Spawn rocks based on level configuration
    int rock_index = 0;
    
    // Level 1: Rocks on left and right sides, top to bottom
    if(current_level->rock_positions) {
        for(int i = 0; i < current_level->rock_count && rock_index < MAX_ROCKS; i++) {
            int row = current_level->rock_positions[i].row;
            int col = current_level->rock_positions[i].col;
            
            // Check if position is valid (not player, not grindstone, not jerk)
            if(!(row == PLAYER_ROW && col == PLAYER_COL) &&
               !grindstone_is_at(row, col) &&
               !jerk_is_at(row, col)) {
                rocks[rock_index].row = row;
                rocks[rock_index].col = col;
                rocks[rock_index].active = true;
                rock_index++;
            }
        }
    }
    
    active_rock_count = rock_index;
}

static void draw_rock(int x, int y, int size)
{
    int w = size * 0.7; 
    int h = size * 0.65;

    int offset_x = x + (size - w) / 2;
    int offset_y = y + (size - h) / 2;

    // Top face — medium blue-gray
    for(int dy = 0; dy < h / 2; dy++) {
        int left = offset_x + dy / 3;
        int right = offset_x + w - dy / 3;
        dline(left, offset_y + dy, right, offset_y + dy, COLOR_ROCK_TOP);
    }

    // Left face — dark navy-blue
    for(int dy = h / 2; dy < h; dy++) {
        int lx1 = offset_x + dy / 3;
        int lx2 = offset_x + w / 2;
        dline(lx1, offset_y + dy, lx2, offset_y + dy, COLOR_ROCK_LEFT);
    }

    // Right face — darker blue-gray
    for(int dy = h / 2; dy < h; dy++) {
        int rx1 = offset_x + w / 2;
        int rx2 = offset_x + w - dy / 3;
        dline(rx1, offset_y + dy, rx2, offset_y + dy, COLOR_ROCK_RIGHT);
    }
}

void objects_draw_all(void)
{
    for(int i = 0; i < MAX_ROCKS; i++) {
        if(!rocks[i].active) continue;
        
        int x = GRID_START_X + (rocks[i].col * GRID_CELL_SIZE);
        int y = GRID_START_Y + (rocks[i].row * GRID_CELL_SIZE);

        draw_rock(x, y, GRID_CELL_SIZE);
    }
}

bool rock_is_at(int row, int col)
{
    for(int i = 0; i < MAX_ROCKS; i++) {
        if(rocks[i].active && rocks[i].row == row && rocks[i].col == col) {
            return true;
        }
    }
    return false;
}

int objects_get_rock_count(void)
{
    return active_rock_count;
}

rock_t* objects_get_rock(int index)
{
    if(index >= 0 && index < MAX_ROCKS) {
        return &rocks[index];
    }
    return NULL;
}
