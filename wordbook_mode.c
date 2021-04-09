#include "wordbook_mode.h"

#include "ncursesw_func.h"
#include "utility.h"
#include <ncurses.h>
#include <wchar.h>

typedef struct {
    int cursor[2];
    bool isopen[15];
} WordBookState;

typedef enum { UP, DOWN, LEFT, RIGHT, ENTER, MODECHANGE, ESC } WordBookKey;

#define CHARMAP_SIZE1 15
#define CHARMAP_SIZE2 5
static wchar_t char_map[CHARMAP_SIZE1][CHARMAP_SIZE2] = {
    {L'ア', L'イ', L'ウ', L'エ', L'オ'}, {L'カ', L'キ', L'ク', L'ケ', L'コ'},
    {L'ガ', L'ギ', L'グ', L'ゲ', L'ゴ'}, {L'サ', L'シ', L'ス', L'セ', L'ソ'},
    {L'ザ', L'ジ', L'ズ', L'ゼ', L'ゾ'}, {L'タ', L'チ', L'ツ', L'テ', L'ト'},
    {L'ダ', L'ヂ', L'ヅ', L'デ', L'ド'}, {L'ナ', L'ニ', L'ヌ', L'ネ', L'ノ'},
    {L'ハ', L'ヒ', L'フ', L'ヘ', L'ホ'}, {L'バ', L'ビ', L'ブ', L'ベ', L'ボ'},
    {L'パ', L'ピ', L'プ', L'ペ', L'ポ'}, {L'マ', L'ミ', L'ム', L'メ', L'モ'},
    {L'ヤ', L'ユ', L'ヨ', L'\0', L'\0'}, {L'ラ', L'リ', L'ル', L'レ', L'ロ'},
    {L'ワ', L'\0', L'\0', L'\0', L'\0'}};

static int count(const wchar_t *array, int n, wchar_t ignore) {
    int count = 0;
    for(int i = 0; i < n; i++) {
        if(array[i] != ignore) {
            count++;
        }
    }
    return count;
}

static int next_word(const GameData *gamedata, wchar_t key, int start) {
    for(int i = start; i < MAX_GAME_WORDS; i++) {
        if(gamedata->words[i].name[0] == key) {
            return i;
        }
    }
    return -1;
}

static WordBookKey input_key() {
    while(true) {
        switch(getch()) {
        case KEY_UP:
            return UP;
        case KEY_DOWN:
            return DOWN;
        case KEY_RIGHT:
            return RIGHT;
        case KEY_LEFT:
            return LEFT;
        case KEY_ENTER:
        case '\n':
            return ENTER;
        case 27:
            nodelay(stdscr, true);
            int c = getch();
            nodelay(stdscr, false);
            if(c == -1) {
                return ESC;
            }
        }
    }
}

static void update_state(WordBookState *state, const WordBookKey key) {
    switch(key) {
    case UP:
        if(state->cursor[1] == -1) {
            state->cursor[0] = state->cursor[0] == 0 ? CHARMAP_SIZE1 - 1
                                                     : (state->cursor[0] - 1);
        } else {
            state->cursor[1] =
                state->cursor[1] == 0
                    ? (count(char_map[state->cursor[0]], CHARMAP_SIZE2, L'\0') -
                       1)
                    : (state->cursor[1] - 1);
        }

        break;
    case DOWN:
        if(state->cursor[1] == -1) {
            state->cursor[0] = state->cursor[0] == CHARMAP_SIZE1 - 1
                                   ? 0
                                   : (state->cursor[0] + 1);
        } else {
            state->cursor[1] =
                state->cursor[1] == (count(char_map[state->cursor[0]],
                                           CHARMAP_SIZE2, L'\0') -
                                     1)
                    ? 0
                    : (state->cursor[1] + 1);
        }
        break;
    case RIGHT:
        if(state->cursor[1] == -1) {
            state->cursor[1] = 0;
        }
        break;
    case LEFT:
        if(state->cursor[1] != -1) {
            state->cursor[1] = -1;
        }
        break;
    }
}

static void draw(const WordBookState *state, const GameData *gamedata) {
    erase();
    border_set(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    mvaddstr(2, 7, "単語リスト");
    draw_box(3, 4, 20, 20);
    draw_box(3, 25, 20, 55);

    //サイド
    for(int i = 0; i < CHARMAP_SIZE1; i++) {
        mvaddnwstr(4 + i, 8, &char_map[i][0], 1);
    }

    for(int i = 0; i < CHARMAP_SIZE2; i++) {
        mvaddnwstr(4 + i, 15, &char_map[state->cursor[0]][i], 1);
    }

    //カーソル
    if(state->cursor[1] == -1) {
        mvaddstr(state->cursor[0] + 4, 7, ">");
    } else {
        mvaddstr(state->cursor[1] + 4, 14, ">");
    }

    //メイン
    wchar_t target = char_map[state->cursor[0]]
                             [state->cursor[1] == -1 ? 0 : state->cursor[1]];
    mvaddnwstr(4, 27, &target, 1);
    mvaddstr(4, 29, "から始まるポケモン");
    for(int i = 0, index = 0;; i++) {
        index = next_word(gamedata, target, index + 1);
        if(index == -1) {
            break;
        }
        mvaddwstr(5 + i / 3, 30 + i % 3 * 15, gamedata->words[index].name);
    }

    mvaddstr(23, 25, "↑↓ ← →：移動　　ESC：タイトルに戻る");
    refresh();
}

void run_wordbook(const GameData *gamedata) {
    WordBookState state = {{0, -1}, {false, false}};
    while(true) {
        draw(&state, gamedata);
        WordBookKey key = input_key();
        if(key == ESC) {
            break;
        }
        update_state(&state, key);
    }
    return;
}