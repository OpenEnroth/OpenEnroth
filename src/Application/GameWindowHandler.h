#pragma once

#include <memory>
#include <tuple>
#include <unordered_map>

#include "Platform/PlatformEnums.h"
#include "Platform/PlatformGamepad.h"
#include "Platform/Filters/PlatformEventFilter.h"
#include "Io/Mouse.h"
#include "Utility/Geometry/Size.h"

#include "GameKeyboardController.h"

using Io::Mouse;

namespace Application {
class GameConfig;

// Handles events from game window (OSWindow)
class GameWindowHandler : public PlatformEventFilter {
 public:
    GameWindowHandler();
    virtual ~GameWindowHandler();

    GameKeyboardController *KeyboardController() const {
        return keyboardController_.get();
    }

    // TODO(captainurist): this probably doesn't even belong here. Find a place to move to.
    void UpdateWindowFromConfig(const GameConfig *config);
    void UpdateConfigFromWindow(GameConfig *config);
    std::tuple<int, Pointi, Sizei> GetWindowConfigPosition(const GameConfig *config);
    std::tuple<int, Pointi, Sizei> GetWindowRelativePosition(Pointi *position = nullptr);

 private:
    Pointi MapToRender(Pointi position);
    void OnMouseLeftClick(Pointi position);
    void OnMouseRightClick(Pointi position);
    void OnMouseLeftUp();
    void OnMouseRightUp();
    void OnMouseLeftDoubleClick(Pointi position);
    void OnMouseRightDoubleClick(Pointi position);
    void OnMouseMove(Pointi position, bool left_button, bool right_button);
    void OnScreenshot();
    void OnToggleBorderless();
    void OnToggleFullscreen();
    void OnToggleResizable();
    void OnCycleFilter();
    void OnMouseGrabToggle();
    void OnKey(PlatformKey key);
    bool OnChar(PlatformKey key, int c);
    void OnFocus();
    void OnFocusLost();
    void OnPaint();
    void OnActivated();
    void OnDeactivated();

    virtual bool KeyPressEvent(const PlatformKeyEvent *event) override;
    virtual bool KeyReleaseEvent(const PlatformKeyEvent *event) override;
    virtual bool MouseMoveEvent(const PlatformMouseEvent *event) override;
    virtual bool MousePressEvent(const PlatformMouseEvent *event) override;
    virtual bool MouseReleaseEvent(const PlatformMouseEvent *event) override;
    virtual bool WheelEvent(const PlatformWheelEvent *event) override;
    virtual bool MoveEvent(const PlatformMoveEvent *event) override;
    virtual bool ResizeEvent(const PlatformResizeEvent *event) override;
    virtual bool ActivationEvent(const PlatformWindowEvent *event) override;
    virtual bool CloseEvent(const PlatformWindowEvent *event) override;
    virtual bool GamepadDeviceEvent(const PlatformGamepadDeviceEvent *event) override;

 private:
    std::shared_ptr<Mouse> mouse = nullptr;
    std::unique_ptr<GameKeyboardController> keyboardController_;
    std::unordered_map<uint32_t, std::unique_ptr<PlatformGamepad>> gamepads_;
};

}  // namespace Application
