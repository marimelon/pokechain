#include "playlog_mode.h"
#include "color.h"
#include "config.h"
#include "ncursesw_func.h"
#include "utility.h"
#include <ncursesw/ncurses.h>
#include <stdbool.h>

typedef struct {
    const PlayLogsData *plogs;
    int targetIndex;
    int logpos;
} PlayLogState;

typedef enum { UP, DOWN, LOGUP, LOGDOWN, ENTER, ESC } PlayLogKey;

static const wchar_t *getname_from_no(const GameData *gamedata, int no) {
    for(int i = 0; i < gamedata->length; i++) {
        if(gamedata->words[i].no == no)
            return gamedata->words[i].name;
    }
    return NULL;
}

static PlayLogKey input_key() {
    while(true) {
        switch(getch()) {
        case KEY_UP:
            return UP;
        case KEY_DOWN:
            return DOWN;
        case 'w':
        case 'W':
            return LOGUP;
        case 's':
        case 'S':
            return LOGDOWN;
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

static bool load_plog(PlayLogsData *out) {
    FILE *rfp = fopen(PLAYLOG_FILENAME, "rb");
    if(rfp == NULL) {
        return false;
    }

    fread(out, sizeof(PlayLogsData), 1, rfp);
    fclose(rfp);

    return true;
}

static void update_state(PlayLogState *state, PlayLogKey key) {
    switch(key) {
    case UP:
        if(state->targetIndex > 0) {
            state->targetIndex -= 1;
            state->logpos = 0;
        }
        break;
    case DOWN:
        if(state->targetIndex < MAX_PLAYLOG_ENTITIY &&
           state->plogs->logs[state->targetIndex + 1].play_type != None) {
            state->targetIndex += 1;
            state->logpos = 0;
        }
        break;
    case LOGUP:
        if(state->logpos > 0)
            state->logpos -= 1;
        break;
    case LOGDOWN:
        if(state->logpos <
           (state->plogs->logs[state->targetIndex].count / 2) +
               (state->plogs->logs[state->targetIndex].count % 2) - 10)
            state->logpos += 1;
        break;
    }
}

static void draw_base() {
    border_set(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    mvaddstr(2, 38, "プレイログ");
    draw_box(3, 3, 20, 21);
    draw_box(3, 25, 20, 55);

    mvaddstr(23, 24, "↑↓ ：ログ切り替え　　ESC：タイトルに戻る");
}

static void show_error() {
    erase();

    attron(COLOR_PAIR(COLOR_GLAY));
    draw_base();
    attroff(COLOR_PAIR(COLOR_GLAY));

    // mvvline(1, 85 / 2, '|', 23);

    draw_box(7, 20, 10, 46);
    fill_blank(8, 21, 8, 44);

    attron(COLOR_PAIR(COLOR_RED));
    mvaddstr(10, 30, "プレイログが見つかりません");
    attroff(COLOR_PAIR(COLOR_RED));

    mvaddstr(13, 29, ">Keyを入力してタイトルに戻る<");

    refresh();

    getch();
}

static void draw(const PlayLogState *state, const PlayLogsData *plogs,
                 const GameData *gamedata) {
    const PlayLogEntity *target = &(state->plogs->logs[state->targetIndex]);
    erase();
    draw_base();

    mvprintw(5, 41, "プレイタイプ：%s",
             target->play_type == VS ? "VS" : "SOLO");
    mvprintw(6, 45, "ターン数：%d", target->count);
    mvaddstr(8, 27 + 23, "ログ");
    draw_box(9, 12 + 23, 12, 35);
    if(target->count > 20) {
        mvaddstr(21, 32 + 10, "ログ操作[ 下:S 上:W ]");
    }

    for(int i = 0; i < 20 && 2 * state->logpos + i < target->count; i++) {
        attron(COLOR_PAIR(
            i % 2 == 0 ? COLOR_PINK : COLOR_BLUE));
        mvprintw(10 + i / 2, (i % 2 == 0 ? 14 : 31) + 23, "%2d",
                 2 * state->logpos + i + 1);
        attroff(COLOR_PAIR(
            i % 2 == 0 ? COLOR_PINK : COLOR_BLUE));
        addstr(".");
        addwstr(
            getname_from_no(gamedata, target->words[2 * state->logpos + i]));
    }

    //サイトバー
    mvaddstr(4 + state->targetIndex, 5, ">");
    for(int i = 0; i < 15; i++) {
        if(state->plogs->logs[i].play_type == None)
            break;
        struct tm *t_st = localtime(&state->plogs->logs[i].time);
        mvprintw(4 + i, 6, "%s[%2d/%2d %2d:%2d:%2d]",
                 state->plogs->logs[i].play_type == VS ? "V" : "S",
                 t_st->tm_mon + 1, t_st->tm_mday, t_st->tm_hour, t_st->tm_min,
                 t_st->tm_sec);
    }
    refresh();
}

void run_playlog(const GameData *gamedata) {
    PlayLogsData plogs;
    if(load_plog(&plogs) == false) {
        show_error();
        return;
    }

    PlayLogState state = {&plogs, 0, 0};

    while(true) {
        draw(&state, &plogs, gamedata);
        PlayLogKey key = input_key();
        if(key == ESC) {
            return;
        }
        update_state(&state, key);
    }
}