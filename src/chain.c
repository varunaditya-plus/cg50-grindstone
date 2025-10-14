#include <stdlib.h>
#include <gint/display.h>
#include <gint/keyboard.h>
#include "chain.h"
#include "monsters.h"
#include "player.h"
#include "grid.h"
#include "grindstone.h"
#include "bosses.h"
#include "objects.h"
#include "levels.h"

// Forward declarations
void draw_win_condition_hud(void);
int jerk_damage_player_if_adjacent(void);
void jerk_move_towards_player(void);
void levels_handle_level_completion(void);

// External variables
extern int player_lives;
extern bool game_over;

bool chain_planning = true;
creep_type_t chain_color_shape = CREEP_COUNT;
int chain_len = 0;
static int chain_rows[GRID_SIZE * GRID_SIZE];
static int chain_cols[GRID_SIZE * GRID_SIZE];
static bool chain_can_recolor = false; // becomes true after passing through a grindstone
static bool grindstone_entered = false; // tracks if we're currently inside a grindstone

static void get_cell_center(int row, int col, int *cx, int *cy)
{
	*cx = GRID_START_X + (col * GRID_CELL_SIZE) + (GRID_CELL_SIZE / 2);
	*cy = GRID_START_Y + (row * GRID_CELL_SIZE) + (GRID_CELL_SIZE / 2);
}

static color_t lighten_color(color_t c)
{
    unsigned int r = (c >> 11) & 0x1f;
    unsigned int g = (c >> 5) & 0x3f;
    unsigned int b = c & 0x1f;
    r = (r + 31) >> 1;
    g = (g + 63) >> 1;
    b = (b + 31) >> 1;
    return (color_t)((r << 11) | (g << 5) | b);
}

static void draw_line_thick(int x0, int y0, int x1, int y1, color_t color, int thickness)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    int half = (thickness <= 1) ? 0 : (thickness / 2);
    int x = x0, y = y0;
    while(1) {
        for(int oy = -half; oy <= half; oy++) {
            for(int ox = -half; ox <= half; ox++) {
                dpixel(x + ox, y + oy, color);
            }
        }
        if(x == x1 && y == y1) break;
        e2 = 2 * err;
        if(e2 >= dy) { err += dy; x += sx; }
        if(e2 <= dx) { err += dx; y += sy; }
    }
}

static void draw_chevron(int cx, int cy, int dirx, int diry, int size, color_t color)
{
    // Continuous chevron resembling '>' shape pointing in (dirx, diry)
    int tip_x = cx + dirx * size;
    int tip_y = cy + diry * size;
    int px = -diry; // perpendicular unit (axis-aligned/diagonal values -1,0,1)
    int py = dirx;
    int base_offset = size / 3; // slightly closer base for a wider look
    int base_x = cx - dirx * base_offset;
    int base_y = cy - diry * base_offset;
    int wing = size; // wider chevrons
    // Two strokes from base corners to tip (thinner)
    draw_line_thick(base_x + px * wing, base_y + py * wing, tip_x, tip_y, color, 1);
    draw_line_thick(base_x - px * wing, base_y - py * wing, tip_x, tip_y, color, 1);
}

void draw_chain(void)
{
	if(!chain_planning || chain_len == 0) return;
	int prev_x, prev_y;
	int player_row, player_col;
	get_player_pos(&player_row, &player_col);
    get_cell_center(player_row, player_col, &prev_x, &prev_y);
    // Skip already-visited steps so arrows always point forward from current player pos
    int start_index = 0;
    for(int i = 0; i < chain_len; i++) {
        if(chain_rows[i] == player_row && chain_cols[i] == player_col) {
            start_index = i + 1;
            break;
        }
    }
    // color can change after grindstone the moment we move into a non-grindstone cell when recolor is pending.
    creep_type_t current_shape = CREEP_COUNT;
    bool recolor_pending = false;
    for(int i = start_index; i < chain_len; i++) {
		int row = chain_rows[i];
		int col = chain_cols[i];
		int cx, cy;
		get_cell_center(row, col, &cx, &cy);
        bool is_grind = grindstone_is_at(row, col);
        creep_type_t cell_shape = grid[row][col];
        if(is_grind) {
            // Hitting a grindstone schedules a recolor on the next colored cell
            recolor_pending = true;
        } else if(cell_shape != CREEP_COUNT) {
            if(current_shape == CREEP_COUNT) {
                current_shape = cell_shape;
            } else if(recolor_pending) {
                current_shape = cell_shape;
                recolor_pending = false;
            }
        }
        // Fallback color if no color has been established yet
        color_t seg_color = (current_shape == CREEP_COUNT)
            ? lighten_color(COLOR_BLUE)
            : lighten_color(creep_to_color(current_shape));
        int dx = cx - prev_x;
        int dy = cy - prev_y;
        int dirx = (dx > 0) - (dx < 0);
        int diry = (dy > 0) - (dy < 0);
        int seg_len = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);

		int spacing = 5;
        int arrow_size = 5;
        for(int t = spacing/2; t <= seg_len; t += spacing) {
            int ax = prev_x + (dx * t) / (seg_len == 0 ? 1 : seg_len);
            int ay = prev_y + (dy * t) / (seg_len == 0 ? 1 : seg_len);
            draw_chevron(ax, ay, dirx, diry, arrow_size, seg_color);
        }
		prev_x = cx; prev_y = cy;
	}
}

bool add_chain_point(int row, int col)
{
	if(!chain_planning) return false;
	int player_row, player_col;
	get_player_pos(&player_row, &player_col);
	if(row == player_row && col == player_col) return false;
	if(row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) return false;
	if(!grid_initialized) return false;

	int prev_row = (chain_len == 0) ? player_row : chain_rows[chain_len - 1];
	int prev_col = (chain_len == 0) ? player_col : chain_cols[chain_len - 1];
	int drow = abs(prev_row - row);
	int dcol = abs(prev_col - col);
	// Allow 8-directional adjacency (including diagonals), disallow staying in place
	if(drow == 0 && dcol == 0) return false;
	if(drow > 1 || dcol > 1) return false;

	if(chain_len >= 1) {
		int prevprev_row = (chain_len == 1) ? player_row : chain_rows[chain_len - 2];
		int prevprev_col = (chain_len == 1) ? player_col : chain_cols[chain_len - 2];
		if(row == prevprev_row && col == prevprev_col) return false;
	}

	// Disallow revisiting any cell already in the chain
	for(int i = 0; i < chain_len; i++) {
		if(chain_rows[i] == row && chain_cols[i] == col) return false;
	}

    bool is_grind = grindstone_is_at(row, col);
    bool is_jerk = jerk_is_at(row, col);
    bool is_rock = rock_is_at(row, col);
    creep_type_t s = grid[row][col];
    
    if(is_rock) {
        return false; // Can't pass through rocks
    }
    
    if(is_jerk) {
        // Check if jerk is passable (chain length >= 10)
        if(!jerk_is_passable()) return false;
        // If jerk is passable, treat it like a grindstone for chain logic
        grindstone_entered = true;
    }
    else if(is_grind) {
        // Entering a grindstone - mark that we're inside one
        grindstone_entered = true;
    }
    else {
        // Exiting a grindstone - if we were inside one, enable recolor
        if(grindstone_entered) {
            chain_can_recolor = true;
            grindstone_entered = false;
        }
        
        // Cannot step on empty unless it's a grindstone
        if(s == CREEP_COUNT) return false;
        if(chain_color_shape == CREEP_COUNT) {
            // First colored piece selects the chain color
            chain_color_shape = s;
        }
        else if(chain_can_recolor) {
            // After a grindstone, allow changing the chain color once
            chain_color_shape = s;
            chain_can_recolor = false;
        }
        if(s != chain_color_shape) return false;
    }

	chain_rows[chain_len] = row;
	chain_cols[chain_len] = col;
	chain_len++;
	return true;
}

bool add_chain_step(int drow, int dcol)
{
	int player_row, player_col;
	get_player_pos(&player_row, &player_col);
	int base_row = (chain_len == 0) ? player_row : chain_rows[chain_len - 1];
	int base_col = (chain_len == 0) ? player_col : chain_cols[chain_len - 1];
	int row = base_row + drow;
	int col = base_col + dcol;

	if(chain_len >= 1) {
		int prevprev_row = (chain_len == 1) ? player_row : chain_rows[chain_len - 2];
		int prevprev_col = (chain_len == 1) ? player_col : chain_cols[chain_len - 2];
		if(row == prevprev_row && col == prevprev_col) {
			chain_len--;
			if(chain_len == 0) {
				chain_color_shape = CREEP_COUNT;
				chain_can_recolor = false;
				grindstone_entered = false;
			} else {
				// Check if we're going back to before a grindstone
				// Reset grindstone state if we're backing up
				grindstone_entered = false;
				chain_can_recolor = false;
			}
			return true;
		}
	}

	return add_chain_point(row, col);
}

void draw_line(int x0, int y0, int x1, int y1, color_t color)
{
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2;
	while(1) {
		dpixel(x0, y0, color);
		if(x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if(e2 >= dy) { err += dy; x0 += sx; }
		if(e2 <= dx) { err += dx; y0 += sy; }
	}
}

void draw_hline(int x0, int x1, int y, color_t color)
{
	if(x0 > x1) { int t = x0; x0 = x1; x1 = t; }
	for(int x = x0; x <= x1; x++) dpixel(x, y, color);
}

void draw_vline(int x, int y0, int y1, color_t color)
{
	if(y0 > y1) { int t = y0; y0 = y1; y1 = t; }
	for(int y = y0; y <= y1; y++) dpixel(x, y, color);
}

void draw_ortho_path(int x0, int y0, int x1, int y1, color_t color)
{
	draw_hline(x0, x1, y0, color);
	draw_vline(x1, y0, y1, color);
}


void reset_chain_state(void)
{
    chain_len = 0;
    chain_color_shape = CREEP_COUNT;
    chain_can_recolor = false;
    grindstone_entered = false;
}

void execute_chain(void)
{
	if(!chain_planning || chain_len == 0) return;
    // Step through each chain point, move player, delete monster at that cell
    int executed_len = chain_len;
    int creeps_smashed_now = 0;
	for(int i = 0; i < chain_len; i++) {
		int target_row = chain_rows[i];
		int target_col = chain_cols[i];
		set_player_pos(target_row, target_col);
        // Remove monster, grindstone, or jerk at target
        if(grindstone_is_at(target_row, target_col)) {
            grindstone_remove(target_row, target_col);
        } else if(jerk_is_at(target_row, target_col)) {
            // Jerk is defeated when chain length >= 10
            jerk_reset();
            // Mark that jerk was killed this turn
            levels_mark_jerk_killed();
        } else {
            if(grid[target_row][target_col] != CREEP_COUNT) {
                creeps_smashed_now++;
                grid[target_row][target_col] = CREEP_COUNT;
            }
        }
		// Redraw frame
		draw_background();
        // Chain behind entities
        draw_chain();
        draw_monsters();
        objects_draw_all(); // Draw rocks
        jerk_draw();
		draw_player();
		// Draw HUD elements
		draw_chain_hud();
		draw_hearts_hud();
		draw_win_condition_hud();
		dupdate();
		// brief delay so steps are visible
		for(volatile int d=0; d<200000; d++); // simple busy-wait
	}

    // Clear chain state
    reset_chain_state();

    // Wait 1 second before checking for damage
    for(volatile int d=0; d<1000000; d++); // 1 second delay
    
    // Check for damage from hostile monsters and apply it
    int damage_count = check_damage_from_hostile_monsters();
    if(damage_count > 0) {
        player_lives -= damage_count;
        if(player_lives <= 0) {
            player_lives = 0; // Don't go below 0
            game_over = true;
        }
    }

    // If the executed chain length was 10 or more, spawn a grindstone at random
    if(executed_len >= 10) {
        grindstone_spawn_random();
    }

    // Update smash-based win progress
    if(creeps_smashed_now > 0) {
        levels_add_smashed(creeps_smashed_now);
    }

    // Apply animated gravity/refill (treats grindstones as solid)
    animate_gravity_and_refill();

    // Check if jerk damages player (if player ends up adjacent to jerk)
    int jerk_damage = jerk_damage_player_if_adjacent();
    if(jerk_damage > 0) {
        player_lives -= jerk_damage;
        if(player_lives <= 0) {
            player_lives = 0;
            game_over = true;
        }
        // Jerk doesn't move when it damages the player
    } else {
        // Only move jerk if it didn't damage the player
        jerk_move_towards_player();
    }
    
    // Check for win condition after chain execution
    levels_handle_level_completion();

	// Redraw everything
	draw_background();
	// After resolving the board, mark new hostile monsters for next round
	add_random_hostile_after_chain();
	draw_monsters();
	objects_draw_all(); // Draw rocks
	jerk_draw();
	draw_player();
	// Draw HUD elements
	draw_chain_hud();
	draw_hearts_hud();
	draw_win_condition_hud();
	dupdate();
}
