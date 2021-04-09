#ifndef INITIALIZER_H_
#define INITIALIZER_H_

#include <stdbool.h>

#include "types.h"
#include "config.h"

#define REQUIRE_WINDOW_X 85
#define REQUIRE_WINDOW_Y 25

bool initialize(GameData *gamedata);

#endif // INITIALIZER_H_