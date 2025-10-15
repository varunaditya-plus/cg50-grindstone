#ifndef FONT_H
#define FONT_H

#include <gint/display.h>

// Function declarations
void font_draw_char(int x, int y, char c);
void font_draw_text(int x, int y, const char* text);
void font_draw_char_with_color(int x, int y, char c, color_t color);
void font_draw_text_scaled(int x, int y, const char* text, int scale, color_t color);
int font_text_width_scaled(const char* text, int scale);

#endif // FONT_H
