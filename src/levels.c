#include "levels.h"
#include "bosses.h"
#include "monsters.h"
#include "grindstone.h"
#include "chain.h"

// Global level management
int current_level = 0;

// Level definitions
level_t levels[] = {
    // Level 1: Jerk spawns in top middle, 3 monster types
    {
        .jerk_spawn_row = 0,
        .jerk_spawn_col = GRID_SIZE / 2,
        .monster_count = 3,
        .name = "Level 1"
    },
    // Level 2: Jerk spawns in left middle
    {
        .jerk_spawn_row = GRID_SIZE / 2,
        .jerk_spawn_col = 0,
        .monster_count = 3,
        .name = "Level 2"
    }
};

void levels_init(void)
{
    current_level = 0;
}

void levels_next_level(void)
{
    current_level++;
    // Wrap around to level 1 if we exceed available levels
    if(current_level >= levels_get_count()) {
        current_level = 0;
    }
}

void levels_reset_to_level(int level)
{
    if(level >= 0 && level < levels_get_count()) {
        current_level = level;
    }
}

level_t* levels_get_current(void)
{
    if(current_level >= 0 && current_level < levels_get_count()) {
        return &levels[current_level];
    }
    return &levels[0]; // Default to first level
}

int levels_get_count(void)
{
    return sizeof(levels) / sizeof(levels[0]);
}

void levels_mark_jerk_killed(void)
{
    jerk_killed_this_turn = true;
}

bool levels_check_win_condition(void)
{
    return jerk_was_killed();
}

void levels_handle_level_completion(void)
{
    if(levels_check_win_condition()) {
        levels_next_level();
        
        // Check if we've completed all levels
        if(current_level == 0) {
            // We've wrapped around, meaning we completed all levels
            game_won = true;
        } else {
            // Reset for next level
            chain_color_shape = CREEP_COUNT;
            chain_len = 0;
            grindstone_clear_all();
            randomize_grid();
            clear_all_hostile();
            jerk_reset();
            jerk_spawn(); // Spawn jerk at new level position
        }
    }
}

int levels_get_monster_count(void)
{
    level_t* current = levels_get_current();
    int count = current->monster_count;
    
    if(count < 1) count = 1;
    if(count > 4) count = 4;
    
    return count;
}
