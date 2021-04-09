#include "title.h"

#include "ncursesw_func.h"
#include <ncurses.h>

typedef Mode TitleState;

typedef enum { UP, DOWN, ENTER } TitleKey;

static const int cursor_x[MODE_NUM] = {34, 37, 36, 36, 39};

static void init_state(TitleState *state){
    *state = VSGAME;
}

static TitleKey input_key() {
    while(true) {
        switch(getch()) {
        case KEY_UP:
            return UP;
        case KEY_DOWN:
            return DOWN;
        case KEY_ENTER:
        case '\n':
            return ENTER;
        }
    }
}

static void update_state(TitleState *state, TitleKey key) {
    switch(key) {
    case UP:
        *state = *state == VSGAME ? EXIT : ((*state) - 1);
        break;
    case DOWN:
        *state = *state == EXIT ? VSGAME : ((*state) + 1);
        break;
    }
}

static void draw(const TitleState *state) {
    erase();
    border_set(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    // mvvline(1, 85/2, '|', 23);

    mvaddstr(9, 38, "PokeChain");

    mvaddstr(*state + 18, cursor_x[*state], ">");

    mvaddstr(18, cursor_x[VSGAME] + 1, "プレイヤー対戦");
    mvaddstr(19, cursor_x[SOLOGAME] + 1, "NPC 対戦");
    mvaddstr(20, cursor_x[SCORELOG] + 1, "プレイログ");
    mvaddstr(21, cursor_x[WORDLIST] + 1, "単語リスト");
    mvaddstr(22, cursor_x[EXIT] + 1, "終了");

    refresh();
}

Mode run_title() {
    TitleState state;
    init_state(&state);

    while(true) {
        draw(&state);
        TitleKey key = input_key();
        if(key == ENTER) {
            return (Mode)state;
        }
        update_state(&state, key);
    }
}