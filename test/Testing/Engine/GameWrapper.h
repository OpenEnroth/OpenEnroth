#pragma once

#include <memory>

#include "GameTestStateHandle.h"

class GameWrapper {
 public:
    GameWrapper(GameTestStateHandle state);
    ~GameWrapper();

    void Tick(int count = 1);

    void LClick(int x, int y);

 private:
    GameTestStateHandle state_;
};

