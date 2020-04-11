#pragma once

#include "IO/GameKey.h"

class Mouse;

namespace Application {

// Handles events from game window (OSWindow) and transforms it to game actions/events
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
    Mouse *mouse = nullptr;
};

}  // namespace Application
