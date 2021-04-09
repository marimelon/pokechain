#include <ncurses.h>
#include <stdbool.h>

#include "initializer.h"
#include "solo_mode.h"
#include "title.h"
#include "types.h"
#include "vs_mode.h"
#include "wordbook_mode.h"
#include "playlog_mode.h"

int main() {
    GameData gamedata = {0, {{0, L""}}};

    if(!initialize(&gamedata)) {
        return -1;
    }

    while(true) {
        Mode mode = run_title();
        switch(mode) {
        case VSGAME:
            run_vsgame(&gamedata);
            break;
        case SOLOGAME:
            run_sologame(&gamedata);
            break;
        case SCORELOG:
            run_playlog(&gamedata);
            break;
        case WORDLIST:
            run_wordbook(&gamedata);
            break;
        case EXIT:
            endwin();
            return 0;
        }
    }

    return -2;
}