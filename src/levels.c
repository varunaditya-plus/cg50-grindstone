#include "levels.h"
#include "bosses.h"
#include "monsters.h"
#include "grindstone.h"
#include "chain.h"
#include "objects.h"
#include "player.h"

// Global level management
int current_level = 0;
// Smash tracking
static int creeps_smashed_this_level = 0;
// Multi-jerk tracking
static int remaining_jerks_this_level = 0;
static int jerk_spawns_given_this_level = 0;

// Rock position arrays for each level
// Level 1
static position_t level1_rocks[] = {
    // top
    {0,0},{0,1},{0,2},{0,3},{0,4},{0,5},{0,6},
    // left
    {1,0},{2,0},{3,0},{4,0},{5,0},{6,0},
    // right
    {1,6},{2,6},{3,6},{4,6},{5,6},{6,6}
};

// Level 2
static position_t level2_rocks[] = {
    // left
    {0,0},{1,0},{2,0},{3,0},{4,0},{5,0},{6,0},
    // right
    {0,6},{1,6},{2,6},{3,6},{4,6},{5,6},{6,6}
};

// Level 3
static position_t level3_rocks[] = {
    // top-left
    {0,0},{0,1},{1,0},{1,1},
    // top-right
    {0,5},{0,6},{1,5},{1,6},
    // bottom-left
    {5,0},{5,1},{6,0},{6,1},
    // bottom-right
    {5,5},{5,6},{6,5},{6,6}
};

// Level 4
static position_t level4_rocks[] = {
    {GRID_SIZE-1,0},{GRID_SIZE-1,1},{GRID_SIZE-1,2},
    {GRID_SIZE-1,4},{GRID_SIZE-1,5},{GRID_SIZE-1,6}
};

// Level 5
static position_t level5_rocks[] = {
    {GRID_SIZE-2,0},{GRID_SIZE-2,GRID_SIZE-1}
};

// Level definitions
level_t levels[] = {
    // Level 1: No jerk, smash 30 creeps, rocks on sides and top
    {
        .jerk_spawn_row = 0,
        .jerk_spawn_col = GRID_SIZE / 2,
        .jerk_enabled = false,
        .monster_count = 3,
        .rock_count = sizeof(level1_rocks) / sizeof(level1_rocks[0]),
        .rock_positions = level1_rocks,
        .name = "Level 1",
        .win_condition_text = "SMASH 30 CREEPS",
        .target_smashes = 30
    },
    // Level 2: No jerk, smash 60 creeps, rocks on sides
    {
        .jerk_spawn_row = GRID_SIZE / 2,
        .jerk_spawn_col = 0,
        .jerk_enabled = false,
        .monster_count = 3,
        .rock_count = sizeof(level2_rocks) / sizeof(level2_rocks[0]),
        .rock_positions = level2_rocks,
        .name = "Level 2",
        .win_condition_text = "SMASH 60 CREEPS",
        .target_smashes = 60
    },
    // Level 3: One jerk (top middle), kill all jerks, 2x2 rocks in corners
    {
        .jerk_spawn_row = 0,
        .jerk_spawn_col = GRID_SIZE / 2,
        .jerk_enabled = true,
        .monster_count = 3,
        .rock_count = sizeof(level3_rocks) / sizeof(level3_rocks[0]),
        .rock_positions = level3_rocks,
        .name = "Level 3",
        .win_condition_text = "KILL ALL JERKS",
        .target_smashes = 0
    }
    ,
    // Level 4: Two jerks (top-left, top-right), kill all jerks, bottom row rocks (except middle)
    {
        .jerk_spawn_row = 0,
        .jerk_spawn_col = 0,
        .jerk_enabled = true,
        .jerk2_spawn_row = 0,
        .jerk2_spawn_col = GRID_SIZE - 1,
        .jerk_count = 2,
        .monster_count = 3,
        .rock_count = sizeof(level4_rocks) / sizeof(level4_rocks[0]),
        .rock_positions = level4_rocks,
        .name = "Level 4",
        .win_condition_text = "KILL ALL JERKS",
        .target_smashes = 0
    }
    ,
    // Level 5: Two jerks (bottom-left, bottom-right), smash 50 creeps, rocks above bottom corners
    {
        .jerk_spawn_row = GRID_SIZE - 1,
        .jerk_spawn_col = 0,
        .jerk_enabled = true,
        .jerk2_spawn_row = GRID_SIZE - 1,
        .jerk2_spawn_col = GRID_SIZE - 1,
        .jerk_count = 2,
        .monster_count = 3,
        .rock_count = sizeof(level5_rocks) / sizeof(level5_rocks[0]),
        .rock_positions = level5_rocks,
        .name = "Level 5",
        .win_condition_text = "SMASH 50 CREEPS",
        .target_smashes = 50
    }
};

void levels_init(void)
{
    current_level = 0;
    // reset smash progress
    creeps_smashed_this_level = 0;
    // reset jerk progress
    level_t* lvl = levels_get_current();
    remaining_jerks_this_level = (lvl && lvl->jerk_enabled) ? (lvl->jerk_count > 0 ? lvl->jerk_count : 1) : 0;
    jerk_spawns_given_this_level = 0;
}

void levels_next_level(void)
{
    current_level++;
    // Wrap around to level 1 if we exceed available levels
    if(current_level >= levels_get_count()) {
        current_level = 0;
    }
    // reset smash progress for new level
    creeps_smashed_this_level = 0;
    // reset jerk progress for new level
    level_t* lvl = levels_get_current();
    remaining_jerks_this_level = (lvl && lvl->jerk_enabled) ? (lvl->jerk_count > 0 ? lvl->jerk_count : 1) : 0;
    jerk_spawns_given_this_level = 0;
}

void levels_reset_to_level(int level)
{
    if(level >= 0 && level < levels_get_count()) {
        current_level = level;
        // reset smash progress on explicit reset
        creeps_smashed_this_level = 0;
        // reset jerk progress on explicit reset
        level_t* lvl = levels_get_current();
        remaining_jerks_this_level = (lvl && lvl->jerk_enabled) ? (lvl->jerk_count > 0 ? lvl->jerk_count : 1) : 0;
        jerk_spawns_given_this_level = 0;
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
    if(remaining_jerks_this_level > 0) remaining_jerks_this_level--;
}

bool levels_check_win_condition(void)
{
    level_t* current = levels_get_current();
    if(!current) return false;
    if(current->target_smashes > 0) {
        return levels_get_remaining_smashes() <= 0;
    }
    // jerk-based win when all jerks for this level are defeated
    return remaining_jerks_this_level <= 0;
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
            reset_player_pos();
            jerk_spawn(); // Spawn jerk at new level position if enabled
            objects_spawn_for_level(); // Spawn rocks for new level
        }
    }
    else {
        // Not yet complete. If a jerk was killed this turn on a jerk-based level, spawn next jerk
        level_t* current = levels_get_current();
        if(current && current->target_smashes == 0 && jerk_was_killed()) {
            jerk_killed_this_turn = false; // consume the event
            jerk_spawn();
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

void levels_add_smashed(int n)
{
    if(n <= 0) return;
    level_t* current = levels_get_current();
    if(!current) return;
    if(current->target_smashes <= 0) return; // only track on smash-based levels
    creeps_smashed_this_level += n;
}

int levels_get_remaining_smashes(void)
{
    level_t* current = levels_get_current();
    if(!current) return 0;
    if(current->target_smashes <= 0) return 0;
    int remaining = current->target_smashes - creeps_smashed_this_level;
    return (remaining > 0) ? remaining : 0;
}

