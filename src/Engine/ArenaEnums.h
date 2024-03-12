#pragma once

enum class ArenaLevel {
    ARENA_LEVEL_INVALID = 0,
    ARENA_LEVEL_PAGE = 1,
    ARENA_LEVEL_SQUIRE = 2,
    ARENA_LEVEL_KNIGHT = 3,
    ARENA_LEVEL_LORD = 4,

    ARENA_LEVEL_FIRST_VALID = ARENA_LEVEL_PAGE,
    ARENA_LEVEL_LAST_VALID = ARENA_LEVEL_LORD,
};
using enum ArenaLevel;

enum class ArenaState {
    ARENA_STATE_INITIAL = 0,
    ARENA_STATE_FIGHTING = 1,
    ARENA_STATE_WON = 2,
};
using enum ArenaState;

