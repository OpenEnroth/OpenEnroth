#pragma once

// The engine-wide runtime game version. Set once at boot from the detected data path (see
// `PathResolver::detectGameType`), and queryable from anywhere via `gameType()`. Defaults to `GAME_TYPE_MM7` so
// that every existing MM7 code path keeps working untouched until boot explicitly detects otherwise.
enum class GameType {
    GAME_TYPE_MM6,
    GAME_TYPE_MM7,
    GAME_TYPE_MM8,

    GAME_TYPE_FIRST = GAME_TYPE_MM6,
    GAME_TYPE_LAST = GAME_TYPE_MM8
};
using enum GameType;

GameType gameType();

void setGameType(GameType type);
