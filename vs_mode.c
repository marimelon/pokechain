#include "vs_mode.h"

#include "color.h"
#include "gamecore.h"
#include "ncursesw_func.h"
#include "utility.h"
#include <ncurses.h>
#include <stdlib.h>
#include <wchar.h>

typedef enum { GIVEUP = 0, GOTITLE = 1, HELP = 2, CLOSE = 3 } MenuSelect;

static void init_state(GameState *state, const GameData *gamedata) {}

static void draw(const GameState *state,bool is_color) {
    const GameData *gamedata = state->gamedata;
    erase();
    border_set(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    mvvline(1, 55, '|', 23);

    static const int chat_box_ypos[] = {3, 6, 9, 12, 15};

    // プレイヤー名
    if(is_color) attron(COLOR_PAIR(COLOR_PINK));
    mvprintw(1, 8, "Player1");
    if(is_color) attron(COLOR_PAIR(COLOR_BLUE));
    mvprintw(1, 35, "Player2");
    if(is_color) attroff(COLOR_PAIR(COLOR_BLUE));

    // チャットボックス
    for(int i = 0; i < 5; i++) {
        int turn = state->turn - 5 + i;
        if(turn < 0) {
            continue;
        }

        if(turn == 0 && state->is_random_start == true) {
        }

        if(turn % 2) {
            draw_rchat_box(chat_box_ypos[i], 30,
                           gamedata->words[state->words[turn]].name,is_color);
        } else {
            draw_lchat_box(chat_box_ypos[i], 4,
                           gamedata->words[state->words[turn]].name,is_color);
        }
    }

    mvhline(18, 1, '=', 54);
    if(state->count_useable_words > 0) {
        mvprintw(19, 2, "使用可能なポケモンの数：%d",
                 state->count_useable_words);
    }
    if(state->turn == 0 && state->is_random_start) {
        mvprintw(19, 2, "最初のポケモンは　");
        addwstr(gamedata->words[state->random_first_word].name);
        addstr("　です");
    }
    if(state->turn == 0 && !state->is_random_start) {
        mvprintw(19, 2, "最初のポケモンを入力してください");
    }

    //入力ボックス
    if(is_color) attron(COLOR_PAIR(state->turn % 2 ? COLOR_BLUE : COLOR_PINK));
    draw_box(21, 3, 3, 35);
    if(is_color) attroff(COLOR_PAIR(state->turn % 2 ? COLOR_BLUE : COLOR_PINK));

    //サイドバー
    mvaddstr(2, 60, "ESC:メニュー");
    //ターン表示
    mvprintw(4, 60, "ターン：%d", state->turn);

    //入力エラー
    if(is_color) attron(COLOR_PAIR(COLOR_RED));
    mvaddwstr(20, 4, state->errmsg);
    if(is_color) attroff(COLOR_PAIR(COLOR_RED));

    refresh();
}

static int menu(const GameState *state) {
    const GameData *gamedata = state->gamedata;
    erase();

    attron(COLOR_PAIR(COLOR_GLAY));
    draw(state,false);
    attroff(COLOR_PAIR(COLOR_GLAY));

    // mvvline(1, 85 / 2, '|', 23);

    draw_box(6, 20, 12, 46);
    fill_blank(7, 21, 10, 44);

    mvaddstr(8, 39, "メニュー");
    mvaddstr(10, 34, "降参する");
    mvaddstr(11, 34, "タイトルに戻る");
    mvaddstr(12, 34, "ヘルプ");
    mvaddstr(13, 34, "閉じる");
    mvaddstr(16, 32, "↑↓：選択 Enter：決定");

    refresh();

    int cpos = GIVEUP;

    while(true) {
        mvaddstr(10, 32, cpos == GIVEUP ? ">" : " ");
        mvaddstr(11, 32, cpos == GOTITLE ? ">" : " ");
        mvaddstr(12, 32, cpos == HELP ? ">" : " ");
        mvaddstr(13, 32, cpos == CLOSE ? ">" : " ");

        int c = getch();
        if(c == KEY_UP || c == KEY_DOWN) {
            cpos = c == KEY_DOWN ? (cpos + 1) % 4 : abs((cpos + 3) % 4);
        } else if(c == KEY_ENTER || c == '\n') {
            if(cpos == GIVEUP) {
                draw_box(6, 20, 12, 46);
                fill_blank(7, 21, 10, 44);
                mvaddstr(9, 34, "本当に降参しますか？");
                mvaddstr(13, 35, "はい");
                mvaddstr(13, 45, "いいえ");
                mvaddstr(16, 32, "←→：選択 Enter：決定");
                cpos = 1;
                while(true) {
                    mvaddstr(13, 34, cpos == 0 ? ">" : " ");
                    mvaddstr(13, 39, cpos == 0 ? "<" : " ");
                    mvaddstr(13, 44, cpos == 1 ? ">" : " ");
                    mvaddstr(13, 51, cpos == 1 ? "<" : " ");

                    c = getch();
                    if(c == KEY_RIGHT || c == KEY_LEFT) {
                        cpos = (cpos + 1) % 2;
                    } else if((c == KEY_ENTER || c == '\n') && cpos == 0) {
                        return GIVEUP;
                    } else if((c == KEY_ENTER || c == '\n') && cpos == 1) {
                        return CLOSE;
                    }
                }
            }
            return cpos;
        }
    }
}

static void show_guide() {
    erase();

    border_set(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    // mvvline(1, 85/2, '|', 23);

    mvaddstr(3, 39, "~説明~");
    mvaddstr(5, 5, "このゲームはポケモンを用いてしりとりを行うゲームである。");
    mvaddstr(6, 5,
             "使用できるポケモンは全国図鑑No1~809のポケモン("
             "所謂サン・ムーン世代まで)である。");
    mvaddstr(8, 5, "補足ルール");
    mvaddstr(9, 5, "・最後がーの場合はひつつ前の文字を最後の文字とする。");
    mvaddstr(10, 5, "・小さい文字は大きい文字に直すことができる。");
    mvaddstr(11, 5, "・濁点半濁点は外すことができる。");

    mvaddstr(18, 28, ">なにかキーを押してください<");
    refresh();
    getch();
}

static void show_firstview(GameState *state) {
    erase();

    attron(COLOR_PAIR(COLOR_GLAY));
    draw(state,false);
    attroff(COLOR_PAIR(COLOR_GLAY));

    // mvvline(1, 85 / 2, '|', 23);

    draw_box(6, 20, 11, 46);
    fill_blank(7, 21, 9, 44);

    mvaddstr(8, 23, "最初のポケモンをランダムに選択しますか？");

    mvaddstr(11, 30, "ランダムに選択する");
    mvaddstr(12, 30, "最初のプレイヤーが選択する");

    mvaddstr(14, 32, "↑↓：選択 Enter：決定");

    refresh();

    int cpos = 0;
    mvaddstr(11, 29, ">");

    while(true) {
        int c = getch();

        if(c == KEY_UP || c == KEY_DOWN) {
            cpos = (cpos + 1) % 2;
            mvaddstr(11, 29, cpos == 0 ? ">" : " ");
            mvaddstr(12, 29, cpos == 1 ? ">" : " ");
        } else if(c == KEY_ENTER || c == '\n') {
            state->is_random_start = (cpos == 0);
            return;
        }
    }
}

static void show_result(const GameState *state) {
    const GameData *gamedata = state->gamedata;
    erase();
    border_set(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    draw_box(2, 5, 22, 75);
    // mvvline(1, 85/2, '|', 23);

    mvaddstr(4, 39, "リザルト");

    attron(COLOR_PAIR(state->turn % 2 ? COLOR_PINK : COLOR_BLUE));
    mvprintw(6, 34, "Player%1dの勝利です！", state->turn % 2 ? 1 : 2);
    attroff(COLOR_PAIR(state->turn % 2 ? COLOR_PINK : COLOR_BLUE));

    mvprintw(10, 55, "総ターン数：%d", state->turn);
    mvaddstr(22, 50, "Enter：タイトルに戻る");

    mvaddstr(8, 27, "ログ");
    draw_box(9, 12, 12, 35);
    if(state->turn > 20) {
        mvaddstr(22, 22, "↑↓：ログ操作");
    }

    int cpos = 0;

    while(true) {
        fill_blank(10, 13, 10, 33);
        for(int i = 0; i < 20 && 2 * cpos + i < state->turn; i++) {
            attron(COLOR_PAIR(i % 2 == 0 ? COLOR_PINK : COLOR_BLUE));
            mvprintw(10 + i / 2, i % 2 == 0 ? 14 : 31, "%2d", 2 * cpos + i + 1);
            attroff(COLOR_PAIR(i % 2 == 0 ? COLOR_PINK : COLOR_BLUE));
            addstr(".");
            addwstr(gamedata->words[state->words[2 * cpos + i]].name);
        }
        switch(getch()) {
        case KEY_UP:
            if(cpos > 0)
                cpos--;
            break;
        case KEY_DOWN:
            if(cpos < (state->turn / 2) + (state->turn % 2) - 10)
                cpos++;
            break;
        case KEY_ENTER:
        case '\n':
            return;
        }
    }

    refresh();
    getch();
    return;
}

void run_vsgame(const GameData *gamedata) {
    GameState state = {gamedata, VS, 0, {0}, L"", L"", -1, -1, false, false, false};

    static bool is_first = true;
    if(is_first) {
        show_guide();
        is_first = false;
    }

    show_firstview(&state);

    if(state.is_random_start) {
        do {
            state.random_first_word = rand() % gamedata->length;
        } while(
            gamedata->words[state.random_first_word]
                .name[wcslen(gamedata->words[state.random_first_word].name) -
                      1] == L'ン');
    }

    for(;;) {
        draw(&state,true);
        next_turn(&state);
        if(state.is_open_menu) {
            int result = menu(&state);
            state.is_open_menu = false;
            switch(result) {
            case GIVEUP:
                state.is_show_result = true;
                break;
            case GOTITLE:
                return;
            case HELP:
                show_guide();
                break;
            case CLOSE:
                break;
            }
        }
        if(state.count_useable_words == 0) {
            state.is_show_result = true;
        }
        if(state.is_show_result) {
            show_result(&state);
            save_plog(&state, VS);
            return;
        }
    }
}