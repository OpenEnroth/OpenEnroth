#pragma once

#include <memory>

#include "TestStateHandle.h"

class GameWrapper {
 public:
    GameWrapper(TestStateHandle state);
    ~GameWrapper();

    void Tick(int count = 1);

    void LClick(int x, int y);

 private:
    TestStateHandle state_;
};

