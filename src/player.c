#include <gint/display.h>
#include "player.h"
#include <stdlib.h>

static int current_player_row = PLAYER_ROW;
static int current_player_col = PLAYER_COL;

void set_player_pos(int row, int col)
{
	current_player_row = row;
	current_player_col = col;
}

void reset_player_pos(void)
{
	current_player_row = PLAYER_ROW;
	current_player_col = PLAYER_COL;
}

void get_player_pos(int *row, int *col)
{
	if(row) *row = current_player_row;
	if(col) *col = current_player_col;
}

void draw_player(void)
{
	int cx = GRID_START_X + (current_player_col * GRID_CELL_SIZE) + (GRID_CELL_SIZE / 2);
	int cy = GRID_START_Y + (current_player_row * GRID_CELL_SIZE) + (GRID_CELL_SIZE / 2);
	int s = GRID_CELL_SIZE;

	// --- Torso ---
	int torso_w = s / 2;
	int torso_h = s / 2;
	for (int y = -torso_h / 2; y < torso_h / 2; y++) {
		for (int x = -torso_w / 2; x < torso_w / 2; x++) {
			dpixel(cx + x, cy + y, COLOR_PLAYER_SKIN);
		}
	}

	// --- Nipples ---
	int nipple_offset_y = cy - s/16;
	int nipple_offset_x = s/5;
	int nipple_r = s / 18;
	for (int y = -nipple_r; y <= nipple_r; y++) {
		for (int x = -nipple_r; x <= nipple_r; x++) {
			if (x*x + y*y <= nipple_r*nipple_r) {
				dpixel(cx - nipple_offset_x + x, nipple_offset_y + y, COLOR_PLAYER_MOUSTACHE);
				dpixel(cx + nipple_offset_x + x, nipple_offset_y + y, COLOR_PLAYER_MOUSTACHE);
			}
		}
	}

	// --- Face / Mask ---
	int face_r = s / 4;
	for (int y = -face_r; y <= face_r; y++) {
		for (int x = -face_r; x <= face_r; x++) {
			if (x*x + y*y <= face_r*face_r)
				dpixel(cx + x, cy - torso_h/1.2 + y, COLOR_PLAYER_FACE);
		}
	}

	// --- Eyes ---
	int eye_rx = face_r / 2;
	int eye_ry = (face_r * 3) / 5;

	for (int y = -eye_ry; y <= eye_ry; y++) {
		for (int x = -eye_rx; x <= eye_rx; x++) {
			// (x^2)/(a^2) + (y^2)/(b^2) <= 1 (with a=eye_rx, b=eye_ry)
			if ((x * x) * (eye_ry * eye_ry) + (y * y) * (eye_rx * eye_rx) <= (eye_rx * eye_rx) * (eye_ry * eye_ry)) {
				dpixel(cx - face_r/2 + x, cy - torso_h/1.3 + y, COLOR_PLAYER_EYE_WHITE);
				dpixel(cx + face_r/2 + x, cy - torso_h/1.3 + y, COLOR_PLAYER_EYE_WHITE);
			}
		}
	}
	// Pupils
	int pupil_rx = eye_rx / 3;
	int pupil_ry = eye_ry / 2;
	for (int y = -pupil_ry; y <= pupil_ry; y++) {
		for (int x = -pupil_rx; x <= pupil_rx; x++) {
			if ((x * x) * (pupil_ry * pupil_ry) + (y * y) * (pupil_rx * pupil_rx) <= (pupil_rx * pupil_rx) * (pupil_ry * pupil_ry)) {
				dpixel(cx - face_r/2 + x, cy - torso_h/1.3 + y, COLOR_PLAYER_EYE_RED);
				dpixel(cx + face_r/2 + x, cy - torso_h/1.3 + y, COLOR_PLAYER_EYE_RED);
			}
		}
	}

	// --- Moustache ---
	int moustache_len = face_r;
	int moustache_thick = face_r / 4;
	int moustache_y = cy - torso_h/1.35 + 4;
	// Left moustache
	for (int y = -moustache_thick; y <= moustache_thick; y++) {
		for (int x = -moustache_len; x <= 0; x++) {
			int px = cx + x;
			int py = moustache_y + y + (x / 4);
			dpixel(px, py, COLOR_PLAYER_MOUSTACHE);
		}
	}
	// Right moustache
	for (int y = -moustache_thick; y <= moustache_thick; y++) {
		for (int x = 0; x <= moustache_len; x++) {
			int px = cx + x;
			int py = moustache_y + y + (-x / 4);
			dpixel(px, py, COLOR_PLAYER_MOUSTACHE);
		}
	}


	// --- Upper Muscles ---
	int arm_r = s / 4;
	for (int y = -arm_r; y <= arm_r; y++) {
		for (int x = -arm_r; x <= arm_r; x++) {
			if (x*x + y*y <= arm_r*arm_r) {
				dpixel(cx - torso_w + arm_r/3 + x, cy + y - s/12, COLOR_PLAYER_SKIN);
				dpixel(cx + torso_w - arm_r/3 + x, cy + y - s/12, COLOR_PLAYER_SKIN);
			}
		}
	}

	// --- Lower Muscles ---
	int arm2_r = s / 5;
	for (int y = -arm2_r; y <= arm2_r; y++) {
		for (int x = -arm2_r; x <= arm2_r; x++) {
			if (x*x + y*y <= arm2_r*arm2_r) {
				dpixel(cx - torso_w - s/8 + x, cy + y + s/8, COLOR_PLAYER_SKIN);
				dpixel(cx + torso_w + s/8 + x, cy + y + s/8, COLOR_PLAYER_SKIN);
			}
		}
	}

	// --- Tiny Hands ---
	int hand_r = s / 14; // smaller hands
	for (int y = -hand_r; y <= hand_r; y++) {
		for (int x = -hand_r; x <= hand_r; x++) {
			if (x*x + y*y <= hand_r*hand_r) {
				dpixel(cx - torso_w - (2*s)/7 + x, cy + (2*s)/7 + y, COLOR_PLAYER_SKIN);
				dpixel(cx + torso_w + (2*s)/7 + x, cy + (2*s)/7 + y, COLOR_PLAYER_SKIN);
			}
		}
	}

	// --- Pants ---
	int pants_h = s / 5;
	int pants_w = torso_w / 1.8;
	for (int y = 0; y < pants_h; y++) {
		for (int x = -pants_w; x < pants_w; x++) {
			dpixel(cx + x, cy + torso_h/2 + y, COLOR_PLAYER_PANTS);
		}
	}

	// --- Legs ---
	int leg_h = s / 6;
	for (int y = 0; y < leg_h; y++) {
		dpixel(cx - s/8, cy + torso_h/2 + pants_h + y, COLOR_PLAYER_SKIN);
		dpixel(cx + s/8, cy + torso_h/2 + pants_h + y, COLOR_PLAYER_SKIN);
	}
}



