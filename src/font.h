#ifndef FONT_H
#define FONT_H

#include <gint/display.h>

// Function declarations
void font_draw_char(int x, int y, char c);
void font_draw_text(int x, int y, const char* text);

#endif // FONT_H
