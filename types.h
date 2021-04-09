#ifndef TYPES_H_
#define TYPES_H_

#include <stdbool.h>
#include <wchar.h>
#include <time.h>
#include "config.h"

#define MAX_GAME_WORDS 1000

typedef struct {
    int no;
    wchar_t name[10];
} WordData;

typedef struct {
    int length;
    WordData words[MAX_GAME_WORDS];
} GameData;

typedef enum { None, VS, SOLO } PlayType;

typedef struct {
    const GameData *gamedata;
    PlayType play_type;
    int turn;
    int words[1000];
    wchar_t text[30];
    const wchar_t *errmsg;
    int count_useable_words;
    int random_first_word;
    bool is_random_start;
    bool is_open_menu;
    bool is_show_result;
} GameState;

typedef struct {
    PlayType play_type;
    time_t time;
    int count;
    int words[1000];
} PlayLogEntity;

typedef struct 
{
    PlayLogEntity logs[MAX_PLAYLOG_ENTITIY];
} PlayLogsData;


typedef enum { VSGAME, SOLOGAME, SCORELOG, WORDLIST, EXIT, MODE_NUM } Mode;

#endif // TYPES_H_