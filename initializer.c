#include "initializer.h"

#include <errno.h>
#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "color.h"
#include "ncursesw_func.h"

static bool init_ncurses() {
    setlocale(LC_ALL, "");
    initscr();
    start_color();

    init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_PINK, 205, COLOR_BLACK);
    //init_pair(COLOR_GLAY, 247, COLOR_BLACK);
    init_pair(COLOR_GLAY, 240, COLOR_BLACK);

    int x, y;
    getmaxyx(stdscr, y, x);

    // Check WindowSize
    if(x < REQUIRE_WINDOW_X || y < REQUIRE_WINDOW_Y) {
        endwin();
        fprintf(stderr,
                "画面サイズを 横%d　縦%d 以上にしてから起動してください。\n",
                REQUIRE_WINDOW_X, REQUIRE_WINDOW_Y);
        return false;
    }

    noecho();
    keypad(stdscr, true);
    curs_set(0);
    resizeterm(REQUIRE_WINDOW_Y, REQUIRE_WINDOW_X);

    border_set(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    return true;
}

static bool load_data(GameData *gamedata) {
    errno = 0;
    FILE *fp = fopen(GAMEDATA_FILENAME, "r");
    if(fp == NULL) {
        endwin();
        fprintf(stderr, errno == ENOENT
                            ? "データファイルが見つかりません。\n"
                            : "データファイルの読み込みに失敗しました。\n");
        return false;
    }

    for(int i = 0; i < 1000; i++) {
        if(fwscanf(fp, L"%d,%ls", &gamedata->words[i].no,
                   gamedata->words[i].name) == EOF) {
            gamedata->length = i;
            break;
        }
    }

    fclose(fp);
    return true;
}

bool initialize(GameData *gamedata) {
    srand((unsigned int)time(NULL));
    //srand(4);
    return init_ncurses() && load_data(gamedata);
}