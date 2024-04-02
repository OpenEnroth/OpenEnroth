#pragma once

#include <string>
#include <vector>

#include "GUI/GUIWindow.h"
#include "Library/Geometry/Rect.h"

class GUIFont;

struct HistoryLine {
    std::string text;
    Color color;
};

struct ConsoleState {
    void addToHistory(const std::string& line, const Color& color);
    void addToCommandHistory(const std::string& line, const Color& color);
    std::string prevCommand();
    std::string nextCommand();

    std::vector<HistoryLine> historyLines;
    std::vector<HistoryLine> commandHistoryLines;
    int currentCommandHistoryIndex{};
};

class GUIWindow_DebugConsoleMenu : public GUIWindow {
 public:
     GUIWindow_DebugConsoleMenu();
     virtual void Update() override;
     virtual void Release() override;

 private:
     void drawBackground();
     void drawCommandLineBackground();
     void drawCommandLinePrompt();
     void drawTextHistory();
     void onKeyboardInputInProgress(const Recti& rect);
     void onKeyboardInputConfirmed();
     void updateCommandHistoryIndex();
     GUIFont* getFont() const;
     Recti getWindowRect() const;
     Recti getCommandLineRect() const;

     static ConsoleState* _consoleState;
     int _commandExecuteHandle{};
     bool _isUpPressed{};
     bool _isDownPressed{};
};
