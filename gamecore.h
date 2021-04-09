#ifndef GAMECORE_H_
#define GAMECORE_H_

#include "types.h"
#include <wchar.h>

#include "config.h"

void save_plog(const GameState *state, PlayType ptype);
void next_turn_auto(GameState *state);
void next_turn(GameState *state);

#endif // GAMECORE_H_