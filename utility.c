#include "utility.h"

#include "color.h"
#include "ncursesw_func.h"
#include <ncurses.h>

void draw_box(int y, int x, int h, int w) {
    int cx, cy, i, j;
    getyx(stdscr, cy, cx);

    for(i = 1; i < w - 1; i++) {
        mvaddch(y, x + i, ACS_HLINE);
        mvaddch(y + h - 1, x + i, ACS_HLINE);
    }

    for(i = 1; i < h - 1; i++) {
        mvaddch(y + i, x, ACS_VLINE);
        mvaddch(y + i, x + w - 1, ACS_VLINE);
    }

    mvaddch(y, x, ACS_ULCORNER);

    mvaddch(y, x + w - 1, ACS_URCORNER);

    mvaddch(y + h - 1, x, ACS_LLCORNER);

    mvaddch(y + h - 1, x + w - 1, ACS_LRCORNER);

    move(cy, cx);
}

void draw_lchat_box(int y, int x, const wchar_t *text,bool is_color) {
    int cx, cy;
    getyx(stdscr, cy, cx);

    if(is_color) attron(COLOR_PAIR(COLOR_PINK));
    draw_box(y, x + 1, 3, 20);
    mvaddstr(y + 1, x, "＜ ");
    if(is_color) attroff(COLOR_PAIR(COLOR_PINK));

    addwstr(text);

    move(cy, cx);
}

void draw_rchat_box(int y, int x, const wchar_t *text,bool is_color) {
    int cx, cy;
    getyx(stdscr, cy, cx);

    if(is_color) attron(COLOR_PAIR(COLOR_BLUE));
    draw_box(y, x + 1, 3, 20);
    mvaddstr(y + 1, x + 20, "＞");
    if(is_color) attroff(COLOR_PAIR(COLOR_BLUE));

    mvaddwstr(y + 1, x + 3, text);

    move(cy, cx);
}

void fill_blank(int y, int x, int h, int w) {
    for(int i = y; i < y + h; i++) {
        for(int j = x; j < x + w; j++) {
            mvaddch(i, j, ' ');
        }
    }
}