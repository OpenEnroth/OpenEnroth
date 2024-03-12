#pragma once

enum class ArenaLevel {
    ARENA_LEVEL_PAGE = 0,
    ARENA_LEVEL_SQUIRE = 1,
    ARENA_LEVEL_KNIGHT = 2,
    ARENA_LEVEL_LORD = 3,

    ARENA_LEVEL_FIRST = ARENA_LEVEL_PAGE,
    ARENA_LEVEL_LAST = ARENA_LEVEL_LORD,
};
using enum ArenaLevel;

