#pragma once
#include <memory>

#include "Io/GameKey.h"
#include "Io/Mouse.h"

using Io::GameKey;
using Io::Mouse;

namespace Application {

// Handles events from game window (OSWindow)
class GameWindowHandler {
 public:
    GameWindowHandler();

    void OnMouseLeftClick(int x, int y);
    void OnMouseRightClick(int x, int y);
    void OnMouseLeftUp();
    void OnMouseRightUp();
    void OnMouseLeftDoubleClick(int x, int y);
    void OnMouseRightDoubleClick(int x, int y);
    void OnMouseMove(int x, int y, bool left_button, bool right_button);
    void OnScreenshot();
    void OnToggleFullscreen();
    void OnKey(GameKey key);
    bool OnChar(GameKey key, int c);
    void OnFocus();
    void OnFocusLost();
    void OnPaint();
    void OnActivated();
    void OnDeactivated();

 private:
    std::shared_ptr<Mouse> mouse = nullptr;
};

}  // namespace Application
