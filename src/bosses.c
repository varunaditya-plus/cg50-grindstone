#include <stdlib.h>
#include <gint/display.h>
#include "bosses.h"
#include "grid.h"
#include "player.h"
#include "grindstone.h"
#include "chain.h"
#include "levels.h"
#include "font.h"

// Global jerk instance
jerk_t jerk = {0, 0, false};
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
    
    // Get current level and spawn jerk at the specified position
    level_t* current_level = levels_get_current();
    jerk.row = current_level->jerk_spawn_row;
    jerk.col = current_level->jerk_spawn_col;
    
    // Check if the spawn position is valid (not player, not grindstone)
    if((jerk.row == PLAYER_ROW && jerk.col == PLAYER_COL) || 
       grindstone_is_at(jerk.row, jerk.col)) {
        // If spawn position is invalid, find a nearby valid spot
        bool found = false;
        for(int radius = 1; radius < GRID_SIZE && !found; radius++) {
            for(int dr = -radius; dr <= radius && !found; dr++) {
                for(int dc = -radius; dc <= radius && !found; dc++) {
                    if(abs(dr) == radius || abs(dc) == radius) { // Only check perimeter
                        int test_row = jerk.row + dr;
                        int test_col = jerk.col + dc;
                        if(test_row >= 0 && test_row < GRID_SIZE && 
                           test_col >= 0 && test_col < GRID_SIZE &&
                           !(test_row == PLAYER_ROW && test_col == PLAYER_COL) &&
                           !grindstone_is_at(test_row, test_col)) {
                            jerk.row = test_row;
                            jerk.col = test_col;
                            found = true;
                        }
                    }
                }
            }
        }
    }
    
    jerk.active = true;
}

void jerk_move_towards_player(void)
{
    if(!jerk.active || !grid_initialized) return;
    
    int player_row, player_col;
    get_player_pos(&player_row, &player_col);
    
    // Calculate the direction to move towards the player
    int delta_row = 0;
    int delta_col = 0;
    
    if(jerk.row < player_row) delta_row = 1;
    else if(jerk.row > player_row) delta_row = -1;
    
    if(jerk.col < player_col) delta_col = 1;
    else if(jerk.col > player_col) delta_col = -1;
    
    // Calculate target position
    int target_row = jerk.row + delta_row;
    int target_col = jerk.col + delta_col;
    
    // Check if target position is valid (not player, not grindstone, within bounds)
    if(target_row >= 0 && target_row < GRID_SIZE && 
       target_col >= 0 && target_col < GRID_SIZE &&
       !(target_row == player_row && target_col == player_col) &&
       !grindstone_is_at(target_row, target_col)) {
        
        // Store the creep that's in the target position
        creep_type_t target_creep = grid[target_row][target_col];
        bool target_hostile = hostile[target_row][target_col];
        
        // Move the jerk to the target position
        jerk.row = target_row;
        jerk.col = target_col;
        
        // Put the target creep in the jerk's old position (if there was one)
        if(target_creep != CREEP_COUNT) {
            // Find the jerk's old position (current position minus the delta)
            int old_row = jerk.row - delta_row;
            int old_col = jerk.col - delta_col;
            
            // Only place the creep if the old position is valid and empty
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

void jerk_draw(void)
{
    if(!jerk.active) return;
    
    // Draw jerk as a black dot
    int x = GRID_START_X + (jerk.col * GRID_CELL_SIZE) + (GRID_CELL_SIZE / 2);
    int y = GRID_START_Y + (jerk.row * GRID_CELL_SIZE) + (GRID_CELL_SIZE / 2);
    
    // Draw a small black circle for the jerk
    int radius = 4;
    for(int dy = -radius; dy <= radius; dy++) {
        for(int dx = -radius; dx <= radius; dx++) {
            if(dx*dx + dy*dy <= radius*radius) {
                int px = x + dx;
                int py = y + dy;
                if(px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                    dpixel(px, py, COLOR_BLACK);
                }
            }
        }
    }
    
    // Draw "10" text with red color and white outline at bottom right
    int text_x = x + 6; // Position to the right of the jerk
    int text_y = y + 6; // Position below the jerk
    draw_outlined_text(text_x, text_y, "10", COLOR_RED, COLOR_WHITE);
}

void jerk_reset(void)
{
    jerk.active = false;
    jerk.row = 0;
    jerk.col = 0;
    jerk_killed_this_turn = false;
}

bool jerk_is_at(int row, int col)
{
    return jerk.active && jerk.row == row && jerk.col == col;
}

bool jerk_is_passable(void)
{
    return jerk.active && chain_len >= 10;
}

bool jerk_was_killed(void)
{
    return jerk_killed_this_turn;
}

bool jerk_is_adjacent_to_player(void)
{
    if(!jerk.active) return false;
    
    int player_row, player_col;
    get_player_pos(&player_row, &player_col);
    
    // Check all 8 directions (including diagonals)
    int row_diff = abs(jerk.row - player_row);
    int col_diff = abs(jerk.col - player_col);
    
    // Adjacent if either row or col difference is 1 (or both for diagonal)
    return (row_diff <= 1 && col_diff <= 1 && (row_diff + col_diff > 0));
}

int jerk_damage_player_if_adjacent(void)
{
    if(jerk_is_adjacent_to_player()) {
        // Jerk damages player - return 1 to indicate damage dealt
        return 1;
    }
    return 0; // No damage
}
