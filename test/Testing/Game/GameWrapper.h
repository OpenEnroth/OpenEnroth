#pragma once

#include <memory>

#include "Testing/Engine/TestStateHandle.h"

#include "Platform/PlatformKey.h"
#include "Platform/PlatformMouseButton.h"

class GUIButton;

class GameWrapper {
 public:
    explicit GameWrapper(TestStateHandle state);
    ~GameWrapper();

    void Tick(int count = 1);

    void PressKey(PlatformKey key);
    void ReleaseKey(PlatformKey key);
    void PressButton(PlatformMouseButton button, int x, int y);
    void ReleaseButton(PlatformMouseButton button, int x, int y);

    void PressAndReleaseKey(PlatformKey key);
    void PressAndReleaseButton(PlatformMouseButton button, int x, int y);

    void PressGuiButton(std::string_view buttonId);
    void GoToMainMenu();

 private:
    GUIButton *AssertButton(std::string_view buttonId);

 private:
    TestStateHandle state_;
};

