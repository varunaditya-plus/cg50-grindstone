#include <stdlib.h>
#include <gint/display.h>
#include "monsters.h"
#include "grid.h"
#include "player.h"
#include "grindstone.h"
#include "chain.h"
#include "bosses.h"
#include "levels.h"
#include "objects.h"

// Forward declarations
void draw_level_hud(void);
void draw_win_condition_hud(void);

creep_type_t grid[GRID_SIZE][GRID_SIZE];
bool hostile[GRID_SIZE][GRID_SIZE];
bool grid_initialized = false;
static int move_count = 0;

void draw_creep(int x, int y, creep_type_t creep)
{
	int size = GRID_CELL_SIZE / 2;
	int circle_size = GRID_CELL_SIZE / 3;

	switch(creep) {
		case CREEP_RED:
			draw_circle(x, y, circle_size, COLOR_RED);
			break;
		case CREEP_GREEN:
			draw_triangle(x, y, size, COLOR_GREEN);
			break;
		case CREEP_YELLOW:
			draw_oval(x - size/2, y - size/2, size, size, COLOR_YELLOW);
			break;
		case CREEP_BLUE:
			draw_square(x - size/2, y - size/2, size, COLOR_BLUE);
			break;
		default:
			break;
	}
}

void draw_monsters(void)
{
	if(!grid_initialized) return;
	int player_row, player_col;
	get_player_pos(&player_row, &player_col);
	for(int row = 0; row < GRID_SIZE; row++) {
		for(int col = 0; col < GRID_SIZE; col++) {
            if(row == player_row && col == player_col) continue;
            if(grid[row][col] == CREEP_COUNT) continue;
            // Don't draw monsters on grindstone squares
            if(grindstone_is_at(row, col)) continue;
            // Don't draw monsters where the jerk is
            if(jerk_is_at(row, col)) continue;
            // Don't draw monsters where rocks are
            if(rock_is_at(row, col)) continue;
			int x = GRID_START_X + (col * GRID_CELL_SIZE) + (GRID_CELL_SIZE / 2);
			int y = GRID_START_Y + (row * GRID_CELL_SIZE) + (GRID_CELL_SIZE / 2);
            
            if(hostile[row][col]) {
                int start_x = GRID_START_X + (col * GRID_CELL_SIZE);
                int start_y = GRID_START_Y + (row * GRID_CELL_SIZE);
                
                for(int cell_y = start_y; cell_y < start_y + GRID_CELL_SIZE; cell_y++) {
                    for(int cell_x = start_x; cell_x < start_x + GRID_CELL_SIZE; cell_x++) {
                        dpixel(cell_x, cell_y, COLOR_GRID);
                    }
                }
            }
            
			draw_creep(x, y, grid[row][col]);
		}
	}

    // Draw grindstones on top
    grindstone_draw_all();
}

void randomize_grid(void)
{
	// Get the number of monster types for level
	int monster_count = levels_get_monster_count();
	
	for(int row = 0; row < GRID_SIZE; row++) {
		for(int col = 0; col < GRID_SIZE; col++) {
			if(row == PLAYER_ROW && col == PLAYER_COL) continue;
			if(jerk_is_at(row, col)) continue;
			if(rock_is_at(row, col)) continue;
			// Use level-specific monster count
			grid[row][col] = (creep_type_t)(rand() % monster_count);
            hostile[row][col] = false;
		}
	}
	grid_initialized = true;
}

color_t creep_to_color(creep_type_t creep)
{
	switch(creep) {
		case CREEP_RED: return COLOR_RED;
		case CREEP_GREEN: return COLOR_GREEN;
		case CREEP_YELLOW: return COLOR_YELLOW;
		case CREEP_BLUE: return COLOR_BLUE;
		default: return C_WHITE;
	}
}

void add_random_hostile_after_chain(void)
{
    if(!grid_initialized) return;
    
    // Skip adding hostile monsters for the first 2 moves
    move_count++;
    if(move_count <= 2) return;
    
    // Weighted count 1-4 (favor fewer hostile): ~40%,30%,20%,10%
    int r = rand() % 10;
    int count = (r < 4) ? 1 : (r < 7) ? 2 : (r < 9) ? 3 : 4;
    // Choose unique random cells that have monsters and are not the player or grindstones
    for(int k = 0; k < count; k++) {
        // Attempt limited tries to find a suitable cell
        for(int tries = 0; tries < 64; tries++) {
            int row = rand() % GRID_SIZE;
            int col = rand() % GRID_SIZE;
            if(row == PLAYER_ROW && col == PLAYER_COL) continue;
            if(grindstone_is_at(row, col)) continue;
            if(jerk_is_at(row, col)) continue;
            if(rock_is_at(row, col)) continue;
            if(grid[row][col] == CREEP_COUNT) continue;
            if(hostile[row][col]) continue; // keep hostile constant; avoid reselecting
            hostile[row][col] = true;
            break;
        }
    }
}

void clear_all_hostile(void)
{
    for(int row = 0; row < GRID_SIZE; row++) {
        for(int col = 0; col < GRID_SIZE; col++) {
            hostile[row][col] = false;
        }
    }
    move_count = 0;
}

void draw_circle(int cx, int cy, int r, color_t color)
{
    color_t BODY   = COLOR_RED;
    color_t FACE   = COLOR_YELLOW;
    color_t EYE    = C_WHITE;
    color_t PUPIL  = COLOR_RED;
    color_t MOUTH  = COLOR_GREEN;
    color_t TEETH  = C_WHITE;

    // Helpers
    #define IN_BOUNDS(px,py) ((px) >= 0 && (px) < SCREEN_WIDTH && (py) >= 0 && (py) < SCREEN_HEIGHT)
    auto void fill_circle(int x0, int y0, int rr, color_t c) {
        int rr2 = rr * rr;
        for (int y = -rr; y <= rr; y++) {
            for (int x = -rr; x <= rr; x++) {
                if (x*x + y*y <= rr2) {
                    int px = x0 + x, py = y0 + y;
                    if (IN_BOUNDS(px,py)) dpixel(px, py, c);
                }
            }
        }
    };
    auto void fill_ellipse(int x0, int y0, int rx, int ry, color_t c) {
        // Midpoint-style check using normalized quadratics
        long rx2 = (long)rx * rx, ry2 = (long)ry * ry;
        for (int y = -ry; y <= ry; y++) {
            for (int x = -rx; x <= rx; x++) {
                long v = (long)x*x * ry2 + (long)y*y * rx2;
                if (v <= (long)rx2 * (long)ry2) {
                    int px = x0 + x, py = y0 + y;
                    if (IN_BOUNDS(px,py)) dpixel(px, py, c);
                }
            }
        }
    };
    auto void ring_ellipse(int x0, int y0, int rx, int ry, int t, color_t c) {
        // draw an elliptical ring of thickness t
        fill_ellipse(x0, y0, rx, ry, c);
        if (rx - t > 0 && ry - t > 0) {
            // "erase" interior with background of body so it looks like a ring
            fill_ellipse(x0, y0, rx - t, ry - t, C_NONE);
        }
    };

    // --- BODY ---
    fill_circle(cx, cy, r, BODY);

    // --- EARS ---
    int ear_r = r * 3 / 10;
    int ear_y = cy - (r * 7 / 10);
    fill_circle(cx - (r * 7 / 10), ear_y, ear_r, FACE);
    fill_circle(cx + (r * 7 / 10), ear_y, ear_r, FACE);

    // --- CREST ---
    int crest_h = r / 2;
    draw_triangle(cx, cy - r + crest_h/2, crest_h, BODY);

    // --- FACE MASK ---
    int face_rx = r * 7 / 10;
    int face_ry = r * 4 / 10;
    fill_ellipse(cx, cy - (r / 10), face_rx, face_ry, FACE);

    // --- EYES ---
    int eye_rx = r * 3 / 10, eye_ry = r * 3 / 10;
    int eye_y  = cy - r / 10;
    int eye_dx = r * 4 / 10;
    fill_ellipse(cx - eye_dx, eye_y, eye_rx, eye_ry, EYE);
    fill_ellipse(cx + eye_dx, eye_y, eye_rx, eye_ry, EYE);

    int pup_r = r * 12 / 100;
    fill_circle(cx - eye_dx + (r * 4 / 100), eye_y + (r * 2 / 100), pup_r, PUPIL);
    fill_circle(cx + eye_dx - (r * 4 / 100), eye_y + (r * 2 / 100), pup_r, PUPIL);

    // --- MOUTH ---
    int mouth_rx = r * 5 / 10, mouth_ry = r * 2 / 10;
    fill_ellipse(cx, cy + (r * 2 / 10), mouth_rx, mouth_ry, MOUTH);
    if (mouth_rx - 2 > 0 && mouth_ry - 2 > 0)
        fill_ellipse(cx, cy + (r * 2 / 10), mouth_rx - 2, mouth_ry - 2, FACE);

    // --- TEETH ---
    int ty = cy + (r * 2 / 10) - (mouth_ry / 2);
    int th = mouth_ry;
    int tw = r / 8;
    for (int k = -1; k <= 1; k++) {
        int tx = cx + k * (r / 4) - tw/2;
        for (int py = 0; py < th; py++) {
            for (int px = 0; px < tw; px++) {
                int X = tx + px, Y = ty + py;
                if (IN_BOUNDS(X,Y)) dpixel(X, Y, TEETH);
            }
        }
    }

    // --- ARMS / SPIRAL DISKS ---
    int disk_r  = r * 6 / 10;
    int disk_y  = cy + (r * 4 / 10);
    int disk_dx = r;
    // Left disk
    fill_circle(cx - disk_dx, disk_y, disk_r, COLOR_RED_DARK);
    fill_circle(cx - disk_dx, disk_y, disk_r - 5, COLOR_RED_DARK);
    fill_circle(cx - disk_dx, disk_y, disk_r - 10, COLOR_RED_DARK);
    fill_circle(cx - disk_dx, disk_y, disk_r - 15, COLOR_RED_DARK);
    // Right disk
    fill_circle(cx + disk_dx, disk_y, disk_r, COLOR_RED_DARK);
    fill_circle(cx + disk_dx, disk_y, disk_r - 5, COLOR_RED_DARK);
    fill_circle(cx + disk_dx, disk_y, disk_r - 10, COLOR_RED_DARK);
    fill_circle(cx + disk_dx, disk_y, disk_r - 15, COLOR_RED_DARK);

    // --- FEET ---
    int foot_r = r * 15 / 100;
    fill_circle(cx - (r * 4 / 10), cy + (r * 11 / 10), foot_r, BODY);
    fill_circle(cx + (r * 4 / 10), cy + (r * 11 / 10), foot_r, BODY);

    #undef IN_BOUNDS
}

void draw_triangle(int x, int y, int size, color_t color)
{
    (void)color; // unused

    // Helper: filled ellipse
    void fill_ellipse(int cx, int cy, int a, int b, color_t col) {
        int aa = a * a, bb = b * b;
        for (int dy = -b; dy <= b; dy++) {
            for (int dx = -a; dx <= a; dx++) {
                if (dx*dx*bb + dy*dy*aa <= aa*bb)
                    dpixel(cx + dx, cy + dy, col);
            }
        }
    }

    // Helper: filled circle
    void fill_circle(int cx, int cy, int r, color_t col) {
        int rr = r * r;
        for (int dy = -r; dy <= r; dy++) {
            for (int dx = -r; dx <= r; dx++) {
                if (dx*dx + dy*dy <= rr)
                    dpixel(cx + dx, cy + dy, col);
            }
        }
    }

    // Helper: filled fin (small triangle)
    void fill_fin(int cx, int cy, int h, int w, color_t col) {
        for (int i = 0; i < h; i++) {
            int half = (w * (h - i)) / (2 * h);
            int yy = cy - i;
            for (int dx = -half; dx <= half; dx++)
                dpixel(cx + dx, yy, col);
        }
    }

    // === Monster geometry ===
    int s  = size * 1.3;   // make monster bigger
    int cx = x;
    int cy = y;

    // Body dome (light green)
    int head_a = (3*s)/5;
    int head_b = (2*s)/5;
    fill_ellipse(cx, cy, head_a, head_b, COLOR_GREEN);

    // Bottom shadow (darker green)
    for (int dy = 0; dy <= head_b; dy++) {
        for (int dx = -head_a; dx <= head_a; dx++) {
            if (dx*dx*head_b*head_b + dy*dy*head_a*head_a <= head_a*head_a*head_b*head_b)
                dpixel(cx + dx, cy + dy, COLOR_GREEN_DARK);
        }
    }

    // Faceplate (slightly brighter/lighter)
    int face_a = (int)(0.48f * s);
    int face_b = (int)(0.28f * s);
    int face_cy = cy + (int)(0.10f * s);
    fill_ellipse(cx, face_cy, face_a, face_b, COLOR_GREEN);

    // Eyes (white ovals)
    int eye_r = (int)(0.20f * s);
    int eye_dx = (int)(0.42f * s);
    int eye_y  = face_cy - (int)(0.08f * s);
    fill_circle(cx - eye_dx, eye_y, eye_r, COLOR_WHITE);
    fill_circle(cx + eye_dx, eye_y, eye_r, COLOR_WHITE);

    // Pupils (dark green crescents)
    for (int dy = -eye_r; dy <= eye_r; dy++) {
        for (int dx = -eye_r; dx <= eye_r; dx++) {
            if (dx*dx + dy*dy <= eye_r*eye_r && dx < -eye_r/6)
                dpixel((cx - eye_dx) + dx, eye_y + dy, COLOR_GREEN_DARK);
            if (dx*dx + dy*dy <= eye_r*eye_r && dx > eye_r/6)
                dpixel((cx + eye_dx) + dx, eye_y + dy, COLOR_GREEN_DARK);
        }
    }

    // Mouth (red outline with ivory teeth)
    int mouth_a = (int)(0.25f * s);
    int mouth_b = (int)(0.12f * s);
    int mouth_cy = face_cy + (int)(0.20f * s);
    fill_ellipse(cx, mouth_cy, mouth_a, mouth_b, COLOR_RED);
    fill_ellipse(cx, mouth_cy, mouth_a - 2, mouth_b - 2, COLOR_GREEN);

    // Teeth (4 rectangles)
    int tooth_w = (int)(0.07f * s);
    int tooth_h = (int)(0.17f * s);
    int tooth_y0 = mouth_cy - tooth_h/2;
    int offsets[4] = { -(int)(0.15f*s), -(int)(0.05f*s), (int)(0.05f*s), (int)(0.15f*s) };
    for (int k = 0; k < 4; k++) {
        for (int py = tooth_y0; py < tooth_y0 + tooth_h; py++) {
            for (int px = cx + offsets[k] - tooth_w/2; px <= cx + offsets[k] + tooth_w/2; px++)
                dpixel(px, py, COLOR_IVORY);
        }
    }

    // Fin (small darker green triangle on top)
    int fin_h = (int)(0.40f * s);
    int fin_w = (int)(0.20f * s);
    fill_fin(cx, cy - head_b, fin_h, fin_w, COLOR_GREEN_DARK);

    // Legs (dark green)
    int leg_w = (int)(0.18f * s);
    int leg_h = (int)(0.40f * s);
    int leg_y0 = cy + head_b - (int)(0.05f * s);
    int leg_dx = (int)(0.26f * s);
    for (int py = leg_y0; py < leg_y0 + leg_h; py++) {
        for (int px = cx - leg_dx - leg_w/2; px < cx - leg_dx + leg_w/2; px++)
            dpixel(px, py, COLOR_GREEN_DARK);
        for (int px = cx + leg_dx - leg_w/2; px < cx + leg_dx + leg_w/2; px++)
            dpixel(px, py, COLOR_GREEN_DARK);
    }
}


void draw_oval(int x, int y, int width, int height, color_t color __attribute__((unused)))
{
    color_t YELLOW  = COLOR_YELLOW;
    color_t ORANGE  = COLOR_ORANGE;
    color_t CYAN    = COLOR_CYAN;
    color_t BLACK   = COLOR_BLACK;
    color_t WHITE   = C_WHITE;

    int cx = x + width / 2;
    int cy = y + height / 2;
    int rx = (width * 2) / 3;
    int ry = (height * 2) / 3;

    // Helpers
    #define IN_BOUNDS(px,py) ((px) >= 0 && (px) < SCREEN_WIDTH && (py) >= 0 && (py) < SCREEN_HEIGHT)

    auto void fill_circle(int x0, int y0, int r, color_t c) {
        int r2 = r*r;
        for (int yy = -r; yy <= r; yy++)
            for (int xx = -r; xx <= r; xx++)
                if (xx*xx + yy*yy <= r2) {
                    int px = x0 + xx, py = y0 + yy;
                    if (IN_BOUNDS(px,py)) dpixel(px, py, c);
                }
    };

    auto void fill_ellipse(int x0, int y0, int a, int b, color_t c) {
        long a2=a*a, b2=b*b, ab2=a2*b2;
        for (int yy = -b; yy <= b; yy++)
            for (int xx = -a; xx <= a; xx++)
                if ((long)xx*xx*b2 + (long)yy*yy*a2 <= ab2) {
                    int px = x0 + xx, py = y0 + yy;
                    if (IN_BOUNDS(px,py)) dpixel(px, py, c);
                }
    };

    // --- BODY ---
    long a2 = (long)rx*rx, b2 = (long)ry*ry, ab2 = a2*b2;
    for (int yy = -ry; yy <= ry; yy++) {
        for (int xx = -rx; xx <= rx; xx++) {
            if ((long)xx*xx*b2 + (long)yy*yy*a2 <= ab2) {
                int px = cx + xx, py = cy + yy;
                if (!IN_BOUNDS(px,py)) continue;
                color_t c = (yy < 0) ? YELLOW : ORANGE;
                dpixel(px, py, c);
            }
        }
    }

    // --- TOP SPIKES ---
    int spike_h = ry / 3;
    draw_triangle(cx,        cy - ry - spike_h/2, spike_h, ORANGE);
    draw_triangle(cx - rx/3, cy - ry - spike_h/3, spike_h, ORANGE);
    draw_triangle(cx + rx/3, cy - ry - spike_h/3, spike_h, ORANGE);

    // --- HUGE SIDE EYES ---
    int orig_rx = width / 2;
    int orig_ry = height / 2;
    int eye_rx = orig_rx * 3 / 5;
    int eye_ry = orig_ry * 3 / 5;
    int eye_y  = cy - orig_ry / 5;
    int eye_dx = orig_rx * 5 / 4;

    // White eyes
    fill_ellipse(cx - eye_dx, eye_y, eye_rx, eye_ry, WHITE);
    fill_ellipse(cx + eye_dx, eye_y, eye_rx, eye_ry, WHITE);

    // Orange pupils
    int pup_rx = eye_rx / 2;
    int pup_ry = eye_ry / 2;
    fill_ellipse(cx - eye_dx, eye_y, pup_rx, pup_ry, ORANGE);
    fill_ellipse(cx + eye_dx, eye_y, pup_rx, pup_ry, ORANGE);

    // --- MOUTH ---
    int mouth_rx = rx / 6;  // Smaller width
    int mouth_ry = ry / 3;  // Smaller height
    int mouth_y  = cy + ry / 8;
    fill_ellipse(cx, mouth_y, mouth_rx + 2, mouth_ry + 2, CYAN);
    fill_ellipse(cx, mouth_y, mouth_rx, mouth_ry, BLACK);

    // --- TEETH ---
    int tooth_w = mouth_rx / 3 + 1;
    int tooth_h = mouth_ry / 3 + 1;
    for (int i = -2; i <= 2; i++) {
        int tx0 = cx + i * tooth_w - tooth_w/2;
        int ty0 = mouth_y - tooth_h/2;
        for (int yy = 0; yy < tooth_h; yy++)
            for (int xx = 0; xx < tooth_w; xx++) {
                int px = tx0 + xx, py = ty0 + yy;
                if (IN_BOUNDS(px,py)) dpixel(px, py, WHITE);
            }
    }

    // --- FEET ---
    int leg_h = ry / 2;
    int leg_y = cy + ry * 4 / 5;  // Move feet up by 25%
    for (int i = -2; i <= 2; i += 2) {
        int lx = cx + i * (rx / 3);
        for (int py = 0; py < leg_h; py++)
            if (IN_BOUNDS(lx, leg_y + py)) dpixel(lx, leg_y + py, ORANGE);
    }

    #undef IN_BOUNDS
}

void draw_square(int x, int y, int size, color_t color __attribute__((unused)))
{
    int cx = x + size / 2;
    int cy = y + size / 2;

    // --- HEAD ---
    int head_w = (int)(size * 1.25);     // wide head
    int head_h = (int)(size * 1.15);     // tall head
    int head_x = cx - head_w / 2;
    int head_y = cy - head_h / 2 + (int)(size * 0.05);

    // --- HAIR ---
    int hair_r1 = (int)(size * 0.36);
    int hair_r2 = (int)(size * 0.30);
    int hair_cx1 = cx - (int)(size * 0.18);
    int hair_cx2 = cx + (int)(size * 0.24);
    int hair_cy  = head_y - (int)(size * 0.23);

    // --- EYES ---
    int eye_w = (int)(size * 0.30);
    int eye_h = (int)(size * 0.42);
    int eye_dx = (int)(size * 0.30);
    int eye_cy = head_y + (int)(head_h * 0.42);
    int eye_l_cx = cx - eye_dx;
    int eye_r_cx = cx + eye_dx;
    int pupil_w = (int)(size * 0.07);
    if (pupil_w < 1) pupil_w = 1;

    // --- MOUTH ---
    int mouth_w = (int)(size * 0.70);
    int mouth_h = (int)(size * 0.28);
    int mouth_x = cx - mouth_w / 2;
    int mouth_y = head_y + (int)(head_h * 0.63);

    // --- FEET ---
    int leg_h = (int)(size * 0.50);
    int leg_w = (int)(size * 0.09);
    int leg_gap = (int)(size * 0.18);
    int leg_y0 = y + size - (int)(size * 0.05); // near bottom
    int leg1_x = cx - leg_gap / 2 - leg_w;
    int leg2_x = cx + leg_gap / 2;

    // --- HAIR ---
    for (int py = hair_cy - hair_r1; py <= hair_cy + hair_r1; ++py)
        for (int px = hair_cx1 - hair_r1; px <= hair_cx1 + hair_r1; ++px)
            if ((px - hair_cx1)*(px - hair_cx1) + (py - hair_cy)*(py - hair_cy) <= hair_r1*hair_r1)
                dpixel(px, py, COLOR_BLUE_DARK);

    for (int py = hair_cy - hair_r2; py <= hair_cy + hair_r2; ++py)
        for (int px = hair_cx2 - hair_r2; px <= hair_cx2 + hair_r2; ++px)
            if ((px - hair_cx2)*(px - hair_cx2) + (py - hair_cy)*(py - hair_cy) <= hair_r2*hair_r2)
                dpixel(px, py, COLOR_BLUE_DARK);

    // --- HEAD ---
    int a = head_w / 2, b = head_h / 2;
    int cxh = head_x + a, cyh = head_y + b;
    int a2 = a * a, b2 = b * b, a2b2 = a2 * b2;
    for (int py = head_y; py < head_y + head_h; ++py)
        for (int px = head_x; px < head_x + head_w; ++px)
            if ((px - cxh)*(px - cxh)*b2 + (py - cyh)*(py - cyh)*a2 <= a2b2)
                dpixel(px, py, COLOR_BLUE);

    // --- EYES ---
    int ea = eye_w / 2, eb = eye_h / 2;
    int ea2 = ea * ea, eb2 = eb * eb, ea2eb2 = ea2 * eb2;

    // Left eye
    for (int py = eye_cy - eb; py <= eye_cy + eb; ++py)
        for (int px = eye_l_cx - ea; px <= eye_l_cx + ea; ++px)
            if ((px - eye_l_cx)*(px - eye_l_cx)*eb2 + (py - eye_cy)*(py - eye_cy)*ea2 <= ea2eb2)
                dpixel(px, py, COLOR_EYE_PALE);
    // Right eye
    for (int py = eye_cy - eb; py <= eye_cy + eb; ++py)
        for (int px = eye_r_cx - ea; px <= eye_r_cx + ea; ++px)
            if ((px - eye_r_cx)*(px - eye_r_cx)*eb2 + (py - eye_cy)*(py - eye_cy)*ea2 <= ea2eb2)
                dpixel(px, py, COLOR_EYE_PALE);

    // Pupils (vertical dark bars)
    for (int py = eye_cy - eb; py <= eye_cy + eb; ++py) {
        for (int px = eye_l_cx - pupil_w / 2; px <= eye_l_cx + pupil_w / 2; ++px)
            dpixel(px, py, COLOR_BLUE_DARK);
        for (int px = eye_r_cx - pupil_w / 2; px <= eye_r_cx + pupil_w / 2; ++px)
            dpixel(px, py, COLOR_BLUE_DARK);
    }

    // --- MOUTH ---
    int mw = mouth_w, mh = mouth_h;
    int mcx = mouth_x + mw / 2, mcy = mouth_y + mh / 2;
    int ma = mw / 2, mb = mh / 2;
    int ma2 = ma * ma, mb2 = mb * mb, ma2mb2 = ma2 * mb2;

    for (int py = mouth_y - 1; py <= mouth_y + mh + 1; ++py)
        for (int px = mouth_x - 1; px <= mouth_x + mw + 1; ++px) {
            int dx = px - mcx, dy = py - mcy;
            int v = dx*dx*mb2 + dy*dy*ma2;
            if (v <= ma2mb2) dpixel(px, py, COLOR_MOUTH_PINK);
            if (v > (int)(ma2mb2 * 0.85) && v <= ma2mb2)
                dpixel(px, py, COLOR_RED);
        }

    // --- TEETH ---
    int teeth_x0 = mouth_x + (int)(mw * 0.10);
    int teeth_x1 = mouth_x + (int)(mw * 0.90);
    int teeth_y0 = mouth_y + (int)(mh * 0.20);
    int teeth_y1 = mouth_y + (int)(mh * 0.70);
    for (int py = teeth_y0; py <= teeth_y1; ++py)
        for (int px = teeth_x0; px <= teeth_x1; ++px)
            dpixel(px, py, COLOR_WHITE);

    int gap1 = teeth_x0 + (teeth_x1 - teeth_x0) / 3;
    int gap2 = teeth_x0 + 2 * (teeth_x1 - teeth_x0) / 3;
    for (int py = teeth_y0; py <= teeth_y1; ++py) {
        dpixel(gap1, py, COLOR_BLACK);
        dpixel(gap2, py, COLOR_BLACK);
    }

    // --- FEET ---
    for (int py = leg_y0; py < leg_y0 + leg_h; ++py) {
        for (int px = leg1_x; px < leg1_x + leg_w; ++px)
            dpixel(px, py, COLOR_BLUE_DARK);
        for (int px = leg2_x; px < leg2_x + leg_w; ++px)
            dpixel(px, py, COLOR_BLUE_DARK);
    }
}

void apply_gravity_and_refill(void)
{
	if(!grid_initialized) return;
	int player_row, player_col;
	get_player_pos(&player_row, &player_col);
	for(int col = 0; col < GRID_SIZE; col++) {
		int write_row = GRID_SIZE - 1;
		if(write_row == player_row && col == player_col) write_row--;
		for(int row = GRID_SIZE - 1; row >= 0; row--) {
			if(row == player_row && col == player_col) continue;
			// Skip grindstone squares - they stay empty
			if(grindstone_is_at(row, col)) continue;
			// Skip jerk position - jerk doesn't fall
			if(jerk_is_at(row, col)) continue;
			// Skip rock positions - rocks don't fall
			if(rock_is_at(row, col)) continue;
			// Only process non-empty squares
			if(grid[row][col] != CREEP_COUNT) {
				// Find the next available position below
				while(write_row > row && (write_row == player_row || grindstone_is_at(write_row, col) || jerk_is_at(write_row, col) || rock_is_at(write_row, col))) {
					write_row--;
				}
				if(write_row != row) {
					grid[write_row][col] = grid[row][col];
					hostile[write_row][col] = hostile[row][col];
					grid[row][col] = CREEP_COUNT;
					hostile[row][col] = false;
				}
				write_row--;
				if(write_row == player_row && col == player_col) write_row--;
			}
		}
		// Refill empty spaces from top, but skip grindstone squares, jerk position, and rocks
		for(int row = write_row; row >= 0; row--) {
			if(row == player_row && col == player_col) continue;
			if(grindstone_is_at(row, col)) continue;
			if(jerk_is_at(row, col)) continue;
			if(rock_is_at(row, col)) continue;
			grid[row][col] = (creep_type_t)(rand() % levels_get_monster_count());
			hostile[row][col] = false; // New monsters are not hostile
		}
	}
}

// Animate gravity by repeatedly applying single-row drops and redrawing.
// This keeps the player cell empty while monsters fall into place.
void animate_gravity_and_refill(void)
{
    if(!grid_initialized) return;
    int player_row, player_col;
    get_player_pos(&player_row, &player_col);

    // Repeat passes until no movement happens in a full sweep.
    bool moved;
    do {
        moved = false;
        for(int col = 0; col < GRID_SIZE; col++) {
            for(int row = GRID_SIZE - 2; row >= 0; row--) {
                if(row == player_row && col == player_col) continue;
                if(grid[row][col] == CREEP_COUNT) continue;
                // Don't move monsters from grindstone squares
                if(grindstone_is_at(row, col)) continue;
                // Don't move monsters from jerk position
                if(jerk_is_at(row, col)) continue;
                // Don't move monsters from rock positions
                if(rock_is_at(row, col)) continue;
                
                int below = row + 1;
                // Skip the player cell when checking the below position
                if(below == player_row && col == player_col) below++;
                // Skip grindstone squares, jerk position, and rocks when checking below
                while(below < GRID_SIZE && (grindstone_is_at(below, col) || jerk_is_at(below, col) || rock_is_at(below, col))) {
                    below++;
                    if(below == player_row && col == player_col) below++;
                }
                if(below < GRID_SIZE && grid[below][col] == CREEP_COUNT) {
                    grid[below][col] = grid[row][col];
                    hostile[below][col] = hostile[row][col];
                    grid[row][col] = CREEP_COUNT;
                    hostile[row][col] = false;
                    moved = true;
                }
            }
        }

        // Draw intermediate frame if anything moved
        if(moved) {
            draw_background();
            draw_monsters();
            objects_draw_all(); // Draw rocks
            draw_player();
            draw_chain();
            draw_level_hud();
            draw_chain_hud();
            draw_hearts_hud();
            draw_win_condition_hud();
            dupdate();
            // Small delay for visible falling
            for(volatile int d=0; d<80000; d++);
        }
    } while(moved);

    // Finally, refill any remaining empty cells (excluding player cell) from the top,
    // animating the spawn descent to match falling behavior.
    for(int col = 0; col < GRID_SIZE; col++) {
        // Count empties above the lowest filled cell (skipping player position and jerk)
        int empties = 0;
        for(int row = 0; row < GRID_SIZE; row++) {
            if(row == player_row && col == player_col) continue;
            if(jerk_is_at(row, col)) continue;
            if(rock_is_at(row, col)) continue;
            if(grid[row][col] == CREEP_COUNT) empties++;
        }
        // For each empty, spawn above the grid and slide down per step
        while(empties-- > 0) {
            // Find the first empty from top to place a new tile (skip grindstone squares, jerk, and rocks)
            int spawn_row = 0;
            while(spawn_row < GRID_SIZE && !(grid[spawn_row][col] == CREEP_COUNT && !(spawn_row == player_row && col == player_col) && !grindstone_is_at(spawn_row, col) && !jerk_is_at(spawn_row, col) && !rock_is_at(spawn_row, col))) {
                spawn_row++;
            }
            if(spawn_row >= GRID_SIZE) break;
            grid[spawn_row][col] = (creep_type_t)(rand() % levels_get_monster_count());
            hostile[spawn_row][col] = false; // New monsters are not hostile

            // Let the spawned tile fall until it rests
            int r = spawn_row;
            while(true) {
                int below = r + 1;
                if(below == player_row && col == player_col) below++;
            // Skip grindstone squares, any jerk position, and rocks when falling
            while(below < GRID_SIZE && (grindstone_is_at(below, col) || jerk_is_at(below, col) || rock_is_at(below, col))) {
                    below++;
                    if(below == player_row && col == player_col) below++;
                }
                if(below < GRID_SIZE && grid[below][col] == CREEP_COUNT) {
                    grid[below][col] = grid[r][col];
                    hostile[below][col] = hostile[r][col];
                    grid[r][col] = CREEP_COUNT;
                    hostile[r][col] = false;
                    r = below;
                    draw_background();
                    draw_monsters();
                    objects_draw_all(); // Draw rocks
                    draw_player();
                    draw_chain();
                    draw_level_hud();
                    draw_chain_hud();
                    draw_hearts_hud();
                    draw_win_condition_hud();
                    dupdate();
                    for(volatile int d=0; d<80000; d++);
                } else {
                    break;
                }
            }
        }
    }
}


