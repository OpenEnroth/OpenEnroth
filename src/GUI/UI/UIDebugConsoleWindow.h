#pragma once

#include <string>
#include <vector>

#include "GUI/GUIWindow.h"

class GUIWindow_DebugConsoleMenu : public GUIWindow {
 public:
     GUIWindow_DebugConsoleMenu();
     virtual void Update() override;

 private:
     void DrawBackground(int x, int y, int width, int height);
     void DrawCommandLinePrompt();
     void DrawTextHistory();

     std::vector<std::string> _historyLines;
};
