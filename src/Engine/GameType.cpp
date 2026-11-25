#include "GameType.h"

static GameType globalGameType = GAME_TYPE_MM7;

GameType gameType() {
    return globalGameType;
}

void setGameType(GameType type) {
    globalGameType = type;
}
