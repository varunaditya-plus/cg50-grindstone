#include <gint/display.h>
#include <gint/keyboard.h>
#include <string.h>
#include "game.h"
#include "font.h"
#include "screens.h"

void screens_show_main_menu(void)
{
    dclear(COLOR_BACKGROUND);
    const char* title = "GRINDSTONE";
    int scale = 3;
    int title_w = font_text_width_scaled(title, scale);
    int title_x = (SCREEN_WIDTH - title_w) / 2;
    int title_y = SCREEN_HEIGHT / 2 - 12 - 5;
    const char* prompt = "PRESS EXE TO START";
    int prompt_w = strlen(prompt) * 8;
    int prompt_x = (SCREEN_WIDTH - prompt_w) / 2;
    int prompt_y = title_y + 8*scale + 10;

    int colors[4] = {COLOR_RED, COLOR_GREEN, COLOR_YELLOW, COLOR_BLUE};
    int offset = 0;
    for(;;) {
        dclear(COLOR_BACKGROUND);
        int len = strlen(title);
        for(int i = 0; i < len; i++) {
            char ch[2];
            ch[0] = title[i];
            ch[1] = '\0';
            int cx = title_x + i * 8 * scale;
            int color_index = (i + offset) % 4;
            font_draw_text_scaled(cx, title_y, ch, scale, colors[color_index]);
        }
        font_draw_text(prompt_x, prompt_y, prompt);
        dupdate();
        for(volatile int d = 0; d < 1000000; d++);
        volatile int timeout = 0;
        key_event_t ev = getkey_opt(GETKEY_DEFAULT, &timeout);
        if(ev.key == KEY_EXE) break;
        offset = (offset + 1) % 4;
    }
}

void screens_show_powerup_select(void)
{
    const char* title = "CHOOSE POWERUP";
    const char* options[3] = {"SWORD", "SHIELD", "ARROW"};
    int selected = 0;

    while(1) {
        dclear(COLOR_BACKGROUND);

		int title_scale = 3;
		int title_w = font_text_width_scaled(title, title_scale);
		int title_x = (SCREEN_WIDTH - title_w) / 2;
		int title_y = SCREEN_HEIGHT / 2 - 40;
		font_draw_text_scaled(title_x, title_y, title, title_scale, C_WHITE);

		int spacing = 40;
		int opt_y = title_y + 8*title_scale + 20;
        int total_w = 0;
        int opt_w[3];
        for(int i=0;i<3;i++){ opt_w[i] = strlen(options[i]) * 8; total_w += opt_w[i]; }
        total_w += spacing * 2;
        int start_x = (SCREEN_WIDTH - total_w) / 2;

        int x0 = start_x;
        int x1 = x0 + opt_w[0] + spacing;
        int x2 = x1 + opt_w[1] + spacing;

        font_draw_text(x0, opt_y, options[0]);
        font_draw_text(x1, opt_y, options[1]);
        font_draw_text(x2, opt_y, options[2]);

        int sel_x = (selected==0? x0 : selected==1? x1 : x2);
        int sel_w = opt_w[selected];
        int underline_y = opt_y + 10;
        for(int x = sel_x; x < sel_x + sel_w; x++) dpixel(x, underline_y, C_WHITE);

        const char* prompt = "EXE TO SELECT";
        int prompt_w = strlen(prompt) * 8;
        int prompt_x = (SCREEN_WIDTH - prompt_w) / 2;
        int prompt_y = underline_y + 18;
        font_draw_text(prompt_x, prompt_y, prompt);

        dupdate();

        key_event_t k = getkey();
        if(k.key == KEY_LEFT) { if(selected > 0) selected--; }
        else if(k.key == KEY_RIGHT) { if(selected < 2) selected++; }
        else if(k.key == KEY_EXE) { break; }
    }
}

