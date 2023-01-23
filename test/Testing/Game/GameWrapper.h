#pragma once

#include <memory>
#include <string>

#include "Testing/Engine/TestStateHandle.h"

#include "Platform/PlatformEnums.h"

class GUIButton;
class TestPlatform;

class GameWrapper {
 public:
    GameWrapper(TestStateHandle state, const std::string &testDataDir);
    ~GameWrapper();

    void Reset();

    void Tick(int count = 1);

    void PressKey(PlatformKey key);
    void ReleaseKey(PlatformKey key);
    void PressButton(PlatformMouseButton button, int x, int y);
    void ReleaseButton(PlatformMouseButton button, int x, int y);

    void PressAndReleaseKey(PlatformKey key);
    void PressAndReleaseButton(PlatformMouseButton button, int x, int y);

    void PressGuiButton(std::string_view buttonId);

    void GoToMainMenu();
    void LoadGame(const std::string &name);
    void SkipLoadingScreen();

    void PlayTrace(const std::string &name);

 private:
    GUIButton *AssertButton(std::string_view buttonId);

 private:
    TestStateHandle state_;
    std::string testDataDir_;
};

