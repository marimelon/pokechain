#include "gamecore.h"

#include "ncursesw_func.h"
#include "utility.h"
#include <ncurses.h>
#include <ncursesw/ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <wctype.h>

#define MSG_BUFF_MAX 100

static void normalize_text(wchar_t *text) {
    for(wchar_t *ptr = text; *ptr != L'\0'; ptr++) {
        if(L'ぁ' <= *ptr && *ptr <= L'ゖ') {
            //ひらがな→カタカナ
            *ptr = *ptr - L'ぁ' + L'ァ';
        } else if(L'0' <= *ptr && *ptr <= L'9') {
            //半角数字→全角数字
            *ptr = *ptr - L'0' + L'０';
        } else if(L'A' <= *ptr && *ptr <= L'Z') {
            //字半角大文アルファベット→大文字全角アルファベット
            *ptr = *ptr - L'A' + L'Ａ';
        } else if(L'a' <= *ptr && *ptr <= L'z') {
            //半角小文字アルファベット→大文字小文字アルファベット
            *ptr = *ptr - L'a' + L'Ａ';
        }
    }
}

static int search_word(const wchar_t *text, const GameData *gamedata) {
    for(int i = 0; i < gamedata->length; i++) {
        const wchar_t *t1 = text, *t2 = gamedata->words[i].name;
        while(true) {
            if(*t1 == L'\0' && *t2 == L'\0')
                return i;

            if(*t1 == *t2) {
                t1++;
                t2++;
                continue;
            }

            if(*t2 == L'・' || *t2 == L'：') {
                t2++;
                continue;
            }

            if(*t2 == L'♂' && *t1 == L'オ' && *(t1 + 1) == L'ス') {
                t1 += 2;
                t2++;
                continue;
            }

            if(*t2 == L'♀' && *t1 == L'メ' && *(t1 + 1) == L'ス') {
                t1 += 2;
                t2++;
                continue;
            }

            break;
        }
    }
    return -1;
}

static bool validate_word(const wchar_t *front, const wchar_t *behind) {
    static const wchar_t mini_charmap_keys[] = {L'ァ', L'ィ', L'ゥ', L'ェ',
                                                L'ォ', L'ャ', L'ュ', L'ョ'};
    static const wchar_t mini_charmap_values[] = {L'ア', L'イ', L'ウ', L'エ',
                                                  L'オ', L'ヤ', L'ユ', L'ヨ'};

    static const wchar_t dakuten_charmap_keys[] = {
        L'ガ', L'ギ', L'グ', L'ゲ', L'ゴ', L'ザ', L'ジ', L'ズ', L'ゼ',
        L'ゾ', L'ダ', L'ヂ', L'ヅ', L'デ', L'ド', L'バ', L'ビ', L'ブ',
        L'ベ', L'ボ', L'パ', L'ピ', L'プ', L'ペ', L'ポ'};
    static const wchar_t dakuten_charmap_values[] = {
        L'カ', L'キ', L'ク', L'ケ', L'コ', L'サ', L'シ', L'ス', L'セ',
        L'ソ', L'タ', L'チ', L'ツ', L'テ', L'ト', L'ハ', L'ヒ', L'フ',
        L'ヘ', L'ホ', L'ハ', L'ヒ', L'フ', L'ヘ', L'ホ'};

    static const wchar_t special_charmap_keys[] = {L'♂', L'♀', L'２', L'Ｚ'};
    static const wchar_t special_charmap_values[] = {L'ス', L'ス', L'ツ',
                                                     L'ト'};

    size_t front_len = wcslen(front);
    size_t behind_len = wcslen(behind);

    if(front_len < 2 || behind_len < 2) {
        return -1;
    }

    wchar_t behind_head = behind[0];
    wchar_t front_tail = front[front_len - 1];

    if(front_tail == L'ー') {
        front_tail = front[front_len - 2];
    }

    if(behind_head == front_tail) {
        return (true);
    }

    for(int i = 0; i < sizeof(mini_charmap_keys) / sizeof(wchar_t); i++) {
        if(front_tail == mini_charmap_keys[i]) {
            return (mini_charmap_values[i] == behind_head) ||
                   (front[front_len - 2] == behind_head &&
                    front_tail == behind[1]);
        }
    }

    for(int i = 0; i < sizeof(dakuten_charmap_keys) / sizeof(wchar_t); i++) {
        if(front_tail == dakuten_charmap_keys[i]) {
            return (behind_head == dakuten_charmap_values[i]);
        }
    }

    for(int i = 0; i < sizeof(special_charmap_keys) / sizeof(wchar_t); i++) {
        if(front_tail == special_charmap_keys[i]) {
            return (behind_head == special_charmap_values[i]);
        }
    }

    return false;
}

static bool exists(const int target[], size_t n, int key) {
    for(int i = 0; i < n; i++) {
        if(target[i] == key)
            return true;
    }
    return false;
}

static int count_legalwords(const GameState *state) {
    const GameData *gamedata = state->gamedata;
    int count = 0;
    for(int i = 0; i < gamedata->length; i++) {
        if(!exists(state->words, state->turn, i) &&
           validate_word(gamedata->words[state->words[state->turn - 1]].name,
                         gamedata->words[i].name)) {
            size_t behind_len = wcslen(gamedata->words[i].name);
            if(gamedata->words[i].name[behind_len - 1] != L'ン') {
                count++;
            }
        }
    }
    return count;
}

static int search_legalword(const GameState *state) {
    const GameData *gamedata = state->gamedata;
    for(int i = 0; i < gamedata->length; i++) {
        if(!exists(state->words, state->turn, i) &&
           validate_word(
               gamedata
                   ->words[state->turn > 0 ? state->words[state->turn - 1]
                                           : state->random_first_word]
                   .name,
               gamedata->words[i].name)) {
            size_t behind_len = wcslen(gamedata->words[i].name);
            if(gamedata->words[i].name[behind_len - 1] != L'ン') {
                return i;
            }
        }
    }
    return -1;
}

static int select_random_legalword(const GameState *state) {
    const GameData *gamedata = state->gamedata;
    int candidate_words[50] = {-1};
    int count = count_legalwords(state);
    int offset = rand() % gamedata->length;
    for(int i = offset; i != offset - 1; i++) {
        if(i == gamedata->length) {
            i = 0;
        }
        if(!exists(state->words, state->turn, i) &&
           validate_word(
               gamedata
                   ->words[state->turn > 0 ? state->words[state->turn - 1]
                                           : state->random_first_word]
                   .name,
               gamedata->words[i].name)) {
            size_t behind_len = wcslen(gamedata->words[i].name);
            if(gamedata->words[i].name[behind_len - 1] != L'ン') {
                return i;
            }
        }
    }
    return -1;
}

static void update_state(GameState *state, int wordindex,
                         const wchar_t *errmsg) {
    if(wordindex < 0 || errmsg != NULL) {
        state->errmsg = errmsg;
    } else {
        state->words[state->turn] = wordindex;
        state->turn++;
        state->count_useable_words = count_legalwords(state);
        state->errmsg = L"";
    }
    state->text[0] = L'\0';
}

static int input_text(int y, int x, wchar_t *out, int max_width) {
    int cpos = 0, width = 0;
    wint_t wch;
    wchar_t msg[MSG_BUFF_MAX] = {'\0'};

    if(max_width > MSG_BUFF_MAX - 1) {
        return ERR;
    }

    if(*out != L'\0') {
        wcscpy(msg, out);
    }

    move(y, x);

    curs_set(1);
    while(true) {
        if(get_wch(&wch) != KEY_CODE_YES) {
            if(iswgraph(wch) && width < max_width) {
                if(msg[cpos] == '\0') {
                    msg[cpos] = wch;
                } else {
                    wmemmove(&msg[cpos + 1], &msg[cpos],
                             (MSG_BUFF_MAX - cpos - 1));
                    msg[cpos] = wch;
                }
                cpos++;
            }
        } else {
        }

        switch(wch) {
        case KEY_LEFT:
            if(cpos > 0) {
                cpos--;
            }
            break;

        case KEY_RIGHT:
            if(msg[cpos] != '\0') {
                cpos++;
            }
            break;

        case KEY_BACKSPACE:
        case 127:
            if(cpos > 0) {
                if(msg[cpos] == '\0') {
                    msg[cpos - 1] = '\0';
                    cpos--;
                } else {
                    wmemmove(&msg[cpos - 1], &msg[cpos], (MSG_BUFF_MAX - cpos));
                    cpos--;
                }
            }
            break;

        case KEY_ENTER:
        case '\n':
            wcscpy(out, msg);
            curs_set(0);
            return 0;

        case 27:
            nodelay(stdscr, true);
            int c = getch();
            nodelay(stdscr, false);
            if(c == -1) {
                wcscpy(out, msg);
                curs_set(0);
                return 1;
            }
        }

        move(y, x);
        for(int i = 0; i < max_width; i++) {
            addstr(" ");
        }

        mvaddwstr(y, x, msg);

        int t = 0;
        width = 0;
        for(int i = 0; msg[i] != '\0'; i++) {
            if(i < cpos) {
                t += wcwidth(msg[i]);
            }
            width += wcwidth(msg[i]);
        }
        move(y, x + t);
    }
}

static int cmp_func(const void *a, const void *b) {
    if(((PlayLogEntity *)b)->play_type == None) {
        return -1;
    }
    return ((PlayLogEntity *)b)->time - ((PlayLogEntity *)a)->time;
}

void save_plog(const GameState *state, PlayType ptype) {
    const GameData *gamedata = state->gamedata;
    FILE *rfp = fopen(PLAYLOG_FILENAME, "r");
    PlayLogsData plogs = {{{None, 0, 0, {0}}}};
    if(rfp != NULL) {
        fread(&plogs, sizeof(PlayLogsData), 1, rfp);
        fclose(rfp);
    }

    if(plogs.logs[MAX_PLAYLOG_ENTITIY - 1].play_type != None) {
        plogs.logs[0].play_type = None;
    }

    for(int i = 0; i < MAX_PLAYLOG_ENTITIY; i++) {
        if(plogs.logs[i].play_type == None) {
            plogs.logs[i].time = time(NULL);
            plogs.logs[i].play_type = ptype;
            plogs.logs[i].count = state->turn;
            for(int j = 0; j < state->turn; j++) {
                plogs.logs[i].words[j] = gamedata->words[state->words[j]].no;
            }
            break;
        }
    }

    qsort(plogs.logs, MAX_PLAYLOG_ENTITIY, sizeof(PlayLogEntity), cmp_func);

    FILE *wfp = fopen(PLAYLOG_FILENAME, "wb");
    if(wfp == NULL) {
        draw_box(6, 20, 11, 46);
        fill_blank(7, 21, 9, 44);
        attron(COLOR_PAIR(COLOR_RED));
        mvaddstr(10, 28, "プレイログの保存に失敗しました。");
        attroff(COLOR_PAIR(COLOR_RED));
        mvaddstr(13, 32, "Please press any Key");
        getch();
        return;
    }

    fwrite(&plogs, sizeof(PlayLogsData), 1, wfp);
    fclose(wfp);
}

void next_turn_auto(GameState *state){
    const GameData *gamedata = state->gamedata;
    int wordindex = select_random_legalword(state);
    update_state(state, wordindex, NULL);
    return;
}

void next_turn(GameState *state) {
    const GameData *gamedata = state->gamedata;
    wchar_t text[30] = L"";
    if(state->text[0] != L'\0') {
        wcscpy(text, state->text);
    }

    if(input_text(22, 4, text, 30) == 1) {
        wcscpy(state->text, text);
        state->is_open_menu = true;
        return;
    }

#ifdef DEBUG
    if(wcscmp(text, L"") == 0) {
        //int w = search_legalword(state);
        int w = select_random_legalword(state);
        if(w >= 0) {
            wcscpy(text, gamedata->words[w].name);
        }
    }
#endif

    normalize_text(text);
    int wordindex = search_word(text, gamedata);
    if(wordindex == -1) {
        update_state(state, -1, L"ポケモンが見つかりませんでした");
        return;
    }

    if(state->turn > 0 || state->is_random_start) {
        const wchar_t *front =
            state->turn == 0
                ? gamedata->words[state->random_first_word].name
                : gamedata->words[state->words[state->turn - 1]].name;
        bool result = validate_word(front, gamedata->words[wordindex].name);
        if(result == false) {
            update_state(state, -1, L"このポケモンは続けることができません");
            return;
        }

        const wchar_t *name = gamedata->words[wordindex].name;
        if(name[wcslen(name) - 1] == L'ン') {
            update_state(state, -1,
                         L"ンで終わるポケモンは続けることができません");
            return;
        }

        if(exists(state->words, state->turn, wordindex)) {
            update_state(state, -1, L"すでに使用されています");
            return;
        }
    }

    update_state(state, wordindex, NULL);
    return;
}