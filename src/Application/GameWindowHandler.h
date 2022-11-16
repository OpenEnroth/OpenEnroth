#pragma once
#include <memory>

#include "Platform/PlatformKey.h"
#include "Platform/PlatformEventHandler.h"
#include "Io/Mouse.h"

#include "GameKeyboardController.h"

using Io::Mouse;

namespace Application {

// Handles events from game window (OSWindow)
class GameWindowHandler : public PlatformEventHandler {
 public:
    GameWindowHandler();

    GameKeyboardController *KeyboardController() const {
        return keyboardController_.get();
    }

 private:
    void OnMouseLeftClick(int x, int y);
    void OnMouseRightClick(int x, int y);
    void OnMouseLeftUp();
    void OnMouseRightUp();
    void OnMouseLeftDoubleClick(int x, int y);
    void OnMouseRightDoubleClick(int x, int y);
    void OnMouseMove(int x, int y, bool left_button, bool right_button);
    void OnScreenshot();
    void OnToggleFullscreen();
    void OnKey(PlatformKey key);
    bool OnChar(PlatformKey key, int c);
    void OnFocus();
    void OnFocusLost();
    void OnPaint();
    void OnActivated();
    void OnDeactivated();

    virtual void Event(PlatformWindow *window, const PlatformEvent *event) override;
    virtual void KeyPressEvent(PlatformWindow *window, const PlatformKeyEvent *event) override;
    virtual void KeyReleaseEvent(PlatformWindow *window, const PlatformKeyEvent *event) override;
    virtual void MouseMoveEvent(PlatformWindow *window, const PlatformMouseEvent *event) override;
    virtual void MousePressEvent(PlatformWindow *window, const PlatformMouseEvent *event) override;
    virtual void MouseReleaseEvent(PlatformWindow *window, const PlatformMouseEvent *event) override;
    virtual void WheelEvent(PlatformWindow *window, const PlatformWheelEvent *event) override;
    virtual void ActivationEvent(PlatformWindow *window, const PlatformEvent *event) override;

 private:
    std::shared_ptr<Mouse> mouse = nullptr;
    std::unique_ptr<GameKeyboardController> keyboardController_;
};

}  // namespace Application
