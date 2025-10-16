#include <gint/display.h>
#include <math.h>
#include <stdlib.h>
#include "game.h"
#include "powerups.h"

static void fill_rect(int x, int y, int w, int h, color_t color)
{
    for(int yy = 0; yy < h; yy++)
        for(int xx = 0; xx < w; xx++)
            dpixel(x + xx, y + yy, color);
}

static void draw_thick_line(int x0, int y0, int x1, int y1, int thickness, color_t color)
{
    if(thickness < 1) thickness = 1;
    int dx = x1 - x0;
    int dy = y1 - y0;
    int steps = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
    if(steps == 0) {
        fill_rect(x0 - thickness/2, y0 - thickness/2, thickness, thickness, color);
        return;
    }
    double xi = (double)dx / (double)steps;
    double yi = (double)dy / (double)steps;
    double x = (double)x0;
    double y = (double)y0;
    for(int s = 0; s <= steps; s++) {
        fill_rect((int)x - thickness/2, (int)y - thickness/2, thickness, thickness, color);
        x += xi;
        y += yi;
    }
}

static void fill_tri_up(int cx, int base_y, int base_w, int height, color_t color)
{
    for(int dy = 0; dy < height; dy++) {
        int half = (base_w * (height - dy)) / (2 * height);
        int y = base_y - dy;
        for(int x = cx - half; x <= cx + half; x++) dpixel(x, y, color);
    }
}

static void fill_wedge_towards_tip(int ex, int ey, double dirx, double diry, int head_len, int base_w, color_t color)
{
    for(int i = 0; i <= head_len; i++) {
        int cx = (int)(ex - dirx * i);
        int cy = (int)(ey - diry * i);
        int w = (base_w * (head_len - i)) / (head_len == 0 ? 1 : head_len);
        if(w < 1) w = 1;
        fill_rect(cx - w/2, cy - w/2, w, w, color);
    }
}

void draw_powerup_sword(int cx, int cy, int size)
{
    const color_t GREY_LIGHT = 0x53CE;
    const color_t GREY_DARK  = 0x7A06;

    double angle = 0.45;
    double dirx = cos(angle);
    double diry = sin(angle);

    int blade_len = (size * 5) / 6;
    int blade_w = size / 7; if(blade_w < 2) blade_w = 2;

    int hx = (int)(cx - dirx * (blade_len/3));
    int hy = (int)(cy - diry * (blade_len/3));
    int tipx = (int)(hx + dirx * blade_len);
    int tipy = (int)(hy + diry * blade_len);

    // blade
    draw_thick_line(hx, hy, tipx, tipy, blade_w, GREY_LIGHT);
    // tip wedge
    fill_wedge_towards_tip(tipx, tipy, dirx, diry, blade_w/2, blade_w, GREY_LIGHT);

    // crossguard (perpendicular small line)
    double px = -diry, py = dirx;
    int guard_half = size / 4;
    int gx0 = (int)(hx - px * guard_half);
    int gy0 = (int)(hy - py * guard_half);
    int gx1 = (int)(hx + px * guard_half);
    int gy1 = (int)(hy + py * guard_half);
    draw_thick_line(gx0, gy0, gx1, gy1, blade_w/2, GREY_DARK);

    // handle
    int handle_len = size / 3;
    int backx = (int)(hx - dirx * handle_len);
    int backy = (int)(hy - diry * handle_len);
    draw_thick_line(hx, hy, backx, backy, blade_w/2, GREY_DARK);
}

void draw_powerup_shield(int cx, int cy, int size)
{
    // greys
    const color_t GREY_LIGHT = 0x838A;
    const color_t GREY_DARK  = 0x6287;

    // square top + equilateral triangle below
    int w = (size * 3) / 4;
    int square_h = (size * 2) / 3;
    int tri_h = size / 3;
    
    // position square
    int square_y = cy - size/2 + square_h/2 + 1;
    int square_x = cx - w/2;
    
    // draw square
    fill_rect(square_x, square_y - square_h/2, w, square_h, GREY_LIGHT);
    
    // draw triangle below square
    int tri_base_y = square_y + square_h/2;
    int tri_base_w = w; // same width as square
    
    for(int y = 0; y < tri_h; y++) {
        int width = (tri_base_w * (tri_h - y)) / tri_h;
        for(int x = 0; x < width; x++) {
            dpixel(cx - width/2 + x, tri_base_y + y, GREY_LIGHT);
        }
    }
    
    // outline around entire shield
    int outline_thickness = 2;
    
    for(int i = 0; i < outline_thickness; i++) {
        // top edge
        for(int x = square_x - i; x < square_x + w + i; x++) {
            dpixel(x, square_y - square_h/2 - i, GREY_DARK);
        }
        // left edge
        for(int y = square_y - square_h/2 - i; y < square_y + square_h/2 + i; y++) {
            dpixel(square_x - i, y, GREY_DARK);
        }
        // right edge
        for(int y = square_y - square_h/2 - i; y < square_y + square_h/2 + i; y++) {
            dpixel(square_x + w - 1 + i, y, GREY_DARK);
        }
    }
    
    // outline triangle sides
    int tri_outline_thickness = outline_thickness * 1.5;
    for(int y = 0; y < tri_h; y++) {
        int width = (tri_base_w * (tri_h - y)) / tri_h;
        for(int i = 0; i < tri_outline_thickness; i++) {
            // left side
            dpixel(cx - width/2 - i +1.5, tri_base_y + y, GREY_DARK);
            // right side
            dpixel(cx + width/2 + i -1.5, tri_base_y + y, GREY_DARK);
        }
    }
}

void draw_powerup_arrow(int cx, int cy, int size)
{
    const color_t GREY_LIGHT = 0x53CE; // line
    const color_t GREY_DARK  = 0xD6DA; // tip + tail rect
    const color_t TIP_COLOR  = 0x9E37;

    // Shift whole arrow up and right
    cx += 3;
    cy -= 3;

    // Calculate arrow dimensions
    int shaft_length = size * 2 / 3;  // 2/3 of size for line
    int head_size = size / 3;         // 1/3 of size for arrowhead (bigger)
    
    int dx = shaft_length / 2;
    int dy = -shaft_length / 2;
    
    draw_thick_line(cx - dx, cy - dy, cx + dx, cy + dy, 4, GREY_LIGHT);
    
    double dir_x = (double)dx / (double)shaft_length;
    double dir_y = (double)dy / (double)shaft_length;
    
    // Position triangle tip at the end of the shaft
    int tip_x = cx + dx + 1;
    int tip_y = cy + dy - 1;
    
    // Calculate triangle base points perpendicular to the direction
    int perp_x = (int)(-dir_y * head_size / 2);
    int perp_y = (int)(dir_x * head_size / 2);
    
    // Triangle vertices
    int base1_x = tip_x - (int)(dir_x * head_size) + perp_x;
    int base1_y = tip_y - (int)(dir_y * head_size) + perp_y;
    
    int base2_x = tip_x - (int)(dir_x * head_size) - perp_x;
    int base2_y = tip_y - (int)(dir_y * head_size) - perp_y;
    
    int base_mid_x = tip_x - (int)(dir_x * head_size);
    int base_mid_y = tip_y - (int)(dir_y * head_size);
    int half_base = head_size / 2;
    double px = -dir_y, py = dir_x; // unit perpendicular
    for(int s = 0; s <= head_size; s++) {
        double t = (double)s / (double)(head_size == 0 ? 1 : head_size);
        int cx2 = (int)(base_mid_x + dir_x * s);
        int cy2 = (int)(base_mid_y + dir_y * s);
        int half_w = (int)((1.0 - t) * half_base);
        if(half_w < 1) half_w = 1;
        for(int w = -half_w; w <= half_w; w++) {
            int x = (int)(cx2 + px * w);
            int y = (int)(cy2 + py * w);
            dpixel(x, y, TIP_COLOR);
        }
    }

    draw_thick_line(tip_x, tip_y, base1_x, base1_y, 3, TIP_COLOR);
    draw_thick_line(tip_x, tip_y, base2_x, base2_y, 3, TIP_COLOR);
    draw_thick_line(base1_x, base1_y, base2_x, base2_y, 3, TIP_COLOR);

    int tail_x = cx - dx;
    int tail_y = cy - dy;
    int rect_len = size / 3; if(rect_len < 4) rect_len = 4;   // along arrow direction (backwards)
    int rect_half_w = (size / 8); if(rect_half_w < 2) rect_half_w = 2; // half-width across perpendicular
    
    px = -dir_y;
    py =  dir_x;
    for(int s = 0; s < rect_len; s++) {
        int cx2 = (int)(tail_x - dir_x * s);
        int cy2 = (int)(tail_y - dir_y * s);
        for(int w = -rect_half_w; w <= rect_half_w; w++) {
            int x = (int)(cx2 + px * w);
            int y = (int)(cy2 + py * w);
            dpixel(x, y, GREY_DARK);
        }
    }

    fill_rect(tip_x - 1, tip_y - 1, 3, 3, TIP_COLOR);
}
