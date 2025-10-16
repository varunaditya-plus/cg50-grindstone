#include <stdlib.h>
#include <gint/display.h>
#include "bosses.h"
#include "grid.h"
#include "player.h"
#include "grindstone.h"
#include "chain.h"
#include "levels.h"
#include "font.h"
#include "objects.h"

// Multiple jerk instances
jerk_t jerks[MAX_JERKS];
int active_jerk_count = 0;
bool jerk_killed_this_turn = false;


// Function to draw outlined text
static void draw_outlined_text(int x, int y, const char* text, color_t text_color, color_t outline_color)
{
    int pos_x = x;
    while (*text)
    {
        if (*text == ' ')
        {
            pos_x += 8; // Space width
        }
        else
        {
            // Draw outline in all 8 directions
            for(int dy = -1; dy <= 1; dy++) {
                for(int dx = -1; dx <= 1; dx++) {
                    if(dx == 0 && dy == 0) continue; // Skip center
                    font_draw_char_with_color(pos_x + dx, y + dy, *text, outline_color);
                }
            }
            // Draw main text on top
            font_draw_char_with_color(pos_x, y, *text, text_color);
            pos_x += 8; // Character width
        }
        text++;
    }
}

void jerk_spawn(void)
{
    if(!grid_initialized) return;
    level_t* current_level = levels_get_current();
    if(current_level && !current_level->jerk_enabled) {
        jerk_reset();
        return;
    }
    // If jerks already active for this level, do not respawn on subsequent calls
    if(active_jerk_count > 0) return;
    // Spawn all jerks configured for this level at once (without consuming iterator)
    active_jerk_count = 0;
    for(int idx = 0; idx < current_level->jerk_spawn_count && active_jerk_count < MAX_JERKS; idx++) {
        int row = current_level->jerk_spawns[idx].row;
        int col = current_level->jerk_spawns[idx].col;
        // Adjust invalid spawn positions away from player/grindstone
        if((row == PLAYER_ROW && col == PLAYER_COL) || grindstone_is_at(row, col)) {
            bool found = false;
            for(int radius = 1; radius < GRID_SIZE && !found; radius++) {
                for(int dr = -radius; dr <= radius && !found; dr++) {
                    for(int dc = -radius; dc <= radius && !found; dc++) {
                        if(abs(dr) == radius || abs(dc) == radius) {
                            int test_row = row + dr;
                            int test_col = col + dc;
                            if(test_row >= 0 && test_row < GRID_SIZE &&
                               test_col >= 0 && test_col < GRID_SIZE &&
                               !(test_row == PLAYER_ROW && test_col == PLAYER_COL) &&
                               !grindstone_is_at(test_row, test_col)) {
                                row = test_row;
                                col = test_col;
                                found = true;
                            }
                        }
                    }
                }
            }
        }
        jerks[active_jerk_count].row = row;
        jerks[active_jerk_count].col = col;
        jerks[active_jerk_count].active = true;
        active_jerk_count++;
    }
}

void jerk_move_towards_player(void)
{
    if(!grid_initialized) return;
    int player_row, player_col;
    get_player_pos(&player_row, &player_col);
    for(int i = 0; i < active_jerk_count; i++) {
        if(!jerks[i].active) continue;
        int delta_row = 0;
        int delta_col = 0;
        if(jerks[i].row < player_row) delta_row = 1;
        else if(jerks[i].row > player_row) delta_row = -1;
        if(jerks[i].col < player_col) delta_col = 1;
        else if(jerks[i].col > player_col) delta_col = -1;
        int target_row = jerks[i].row + delta_row;
        int target_col = jerks[i].col + delta_col;
        if(target_row >= 0 && target_row < GRID_SIZE &&
           target_col >= 0 && target_col < GRID_SIZE &&
           !(target_row == player_row && target_col == player_col) &&
           !grindstone_is_at(target_row, target_col) &&
           !rock_is_at(target_row, target_col) &&
           !jerk_is_at(target_row, target_col)) {
            creep_type_t target_creep = grid[target_row][target_col];
            bool target_hostile = hostile[target_row][target_col];
            int old_row = jerks[i].row;
            int old_col = jerks[i].col;
            jerks[i].row = target_row;
            jerks[i].col = target_col;
            if(target_creep != CREEP_COUNT) {
                if(old_row >= 0 && old_row < GRID_SIZE &&
                   old_col >= 0 && old_col < GRID_SIZE &&
                   !(old_row == player_row && old_col == player_col) &&
                   !grindstone_is_at(old_row, old_col) &&
                   grid[old_row][old_col] == CREEP_COUNT) {
                    grid[old_row][old_col] = target_creep;
                    hostile[old_row][old_col] = target_hostile;
                }
            }
        }
    }
}

void jerk_draw(void)
{
    for(int i = 0; i < active_jerk_count; i++) {
        if(!jerks[i].active) continue;
        int cx = GRID_START_X + (jerks[i].col * GRID_CELL_SIZE) + (GRID_CELL_SIZE / 2);
        int cy = GRID_START_Y + (jerks[i].row * GRID_CELL_SIZE) + (GRID_CELL_SIZE / 2);

    // --- Torso ---
    for(int dy = -6; dy <= 6; dy++) {
        int width = 12 - abs(dy); // triangle shape instead of oval
        for(int dx = -width; dx <= width; dx++) {
            dpixel(cx + dx, cy + dy, COLOR_JERK_BODY);
        }
    }

    // --- Upper muscles ---
    for(int i = -1; i <= 1; i += 2) {
        int mx = cx + i * 9;
        int my = cy - 2;
        for(int dy = -4; dy <= 4; dy++) {
            for(int dx = -4; dx <= 4; dx++) {
                if(dx*dx + dy*dy <= 16)
                    dpixel(mx + dx, my + dy, COLOR_JERK_HEAD);
            }
        }
    }

    // --- Head ---
    int head_w = 6, head_h = 4;
    for(int dy = -head_h; dy <= head_h; dy++) {
        for(int dx = -head_w; dx <= head_w; dx++) {
            if((dx*dx)/(head_w*head_w) + (dy*dy)/(head_h*head_h) <= 1)
                dpixel(cx + dx, cy - 11 + dy, COLOR_JERK_HEAD);
        }
    }

    // --- Horn ---
    for(int dy = 0; dy < 4; dy++) {
        for(int dx = -dy; dx <= dy; dx++) {
            dpixel(cx + dx, cy - 19 + dy, COLOR_JERK_HORN);
        }
    }

    // --- Eyes ---
    for(int i = -1; i <= 1; i += 2) {
        int ex = cx + i * 4;
        int ey = cy - 12;
        for(int dy = -1; dy <= 1; dy++) {
            for(int dx = -3; dx <= 3; dx++) {
                if((dx*dx)/9 + (dy*dy)/4 <= 1)
                    dpixel(ex + dx, ey + dy, COLOR_JERK_EYES);
            }
        }
    }
    
    // --- Pupils ---
    for(int i = -1; i <= 1; i += 2) {
        int px = cx + i * 4;
        int py = cy - 12;
        for(int dy = -1; dy <= 1; dy++) {
            for(int dx = -1; dx <= 1; dx++) {
                if(dx*dx + dy*dy <= 1)
                    dpixel(px + dx, py + dy, COLOR_JERK_PUPIL);
            }
        }
    }

    // --- Nipples ---
    for(int i = -1; i <= 1; i += 2) {
        int nx = cx + i * 6;
        int ny = cy - 1;
        for(int dy = -1; dy <= 1; dy++) {
            for(int dx = -1; dx <= 1; dx++) {
                if(dx*dx + dy*dy <= 2)
                    dpixel(nx + dx, ny + dy, COLOR_JERK_NIPPLE);
            }
        }
    }

    // --- Legs ---
    for(int i = -1; i <= 1; i += 2) {
        int lx = cx + i * 6;
        int ly = cy + 7;
        for(int dy = 0; dy < 4; dy++) {
            for(int dx = -2; dx <= 2; dx++) {
                if(dx*dx + dy*dy <= 4)
                    dpixel(lx + dx, ly + dy, COLOR_JERK_BODY);
            }
        }
    }

    // --- Arms ---
    for(int i = -1; i <= 1; i += 2) {
        int arm_x = cx + i * 13; // moved outward
        int arm_y = cy - 1;
        for(int dy = -3; dy <= 3; dy++) {
            for(int dx = -2; dx <= 2; dx++) {
                if(dx*dx + dy*dy <= 6)
                    dpixel(arm_x + dx, arm_y + dy, COLOR_JERK_HEAD);
            }
        }
    }

        draw_outlined_text(cx + 10, cy + 6, "10", COLOR_RED, COLOR_WHITE);
    }
}


void jerk_reset(void)
{
    for(int i = 0; i < MAX_JERKS; i++) {
        jerks[i].active = false;
        jerks[i].row = 0;
        jerks[i].col = 0;
    }
    active_jerk_count = 0;
    jerk_killed_this_turn = false;
}

bool jerk_is_at(int row, int col)
{
    for(int i = 0; i < active_jerk_count; i++) {
        if(jerks[i].active && jerks[i].row == row && jerks[i].col == col) return true;
    }
    return false;
}

bool jerk_is_passable(void)
{
    bool any_active = false;
    for(int i = 0; i < active_jerk_count; i++) if(jerks[i].active) { any_active = true; break; }
    return any_active && chain_len >= 10;
}

bool jerk_was_killed(void)
{
    return jerk_killed_this_turn;
}

bool jerk_is_adjacent_to_player(void)
{
    int player_row, player_col;
    get_player_pos(&player_row, &player_col);
    for(int i = 0; i < active_jerk_count; i++) {
        if(!jerks[i].active) continue;
        int row_diff = abs(jerks[i].row - player_row);
        int col_diff = abs(jerks[i].col - player_col);
        if(row_diff <= 1 && col_diff <= 1 && (row_diff + col_diff > 0)) return true;
    }
    return false;
}

int jerk_damage_player_if_adjacent(void)
{
    // Keep damage simple: at most 1 per turn even with multiple
    return jerk_is_adjacent_to_player() ? 1 : 0;
}

void jerk_kill_at(int row, int col)
{
    for(int i = 0; i < active_jerk_count; i++) {
        if(jerks[i].active && jerks[i].row == row && jerks[i].col == col) {
            jerks[i].active = false;
            jerk_killed_this_turn = true;
            levels_mark_jerk_killed();
            return;
        }
    }
}
