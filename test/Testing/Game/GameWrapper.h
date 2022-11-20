#pragma once

#include <memory>

#include "Testing/Engine/TestStateHandle.h"

#include "Platform/PlatformKey.h"

class GUIButton;

class GameWrapper {
 public:
    GameWrapper(TestStateHandle state);
    ~GameWrapper();

    void Tick(int count = 1);

    void Type(PlatformKey key);
    void LClick(std::string_view buttonId);
    void LClick(int x, int y);

    void GoToMainMenu();

 private:
    GUIButton *AssertButton(std::string_view buttonId);

 private:
    TestStateHandle state_;
};

