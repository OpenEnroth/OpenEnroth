#pragma once

#include <memory>
#include <tuple>
#include <vector>

#include "Library/Fsm/FsmEventHandler.h"
#include "Library/Platform/Interface/PlatformEnums.h"
#include "Library/Platform/Filters/PlatformEventFilter.h"
#include "Library/Platform/Application/PlatformApplicationAware.h"
#include "Library/Geometry/Size.h"

namespace Io {
class Mouse;
} // namespace Io

class GameConfig;

class GameWindowHandler : public PlatformEventFilter, private PlatformApplicationAware {
 public:
    GameWindowHandler();
    virtual ~GameWindowHandler();

    // TODO(captainurist): this probably doesn't even belong here. Find a place to move to.
    void UpdateWindowFromConfig(const GameConfig *config);
    void UpdateConfigFromWindow(GameConfig *config);
    std::tuple<int, Pointi, Sizei> GetWindowConfigPosition(const GameConfig *config);
    std::tuple<int, Pointi, Sizei> GetWindowRelativePosition(Pointi *position = nullptr);

 private:
    friend class PlatformIntrospection;

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

    void handleKeyPress(PlatformKey key, PlatformModifiers mods, bool isAutoRepeat);
    void handleKeyRelease(PlatformKey key);

    virtual bool keyPressEvent(const PlatformKeyEvent *event) override;
    virtual bool keyReleaseEvent(const PlatformKeyEvent *event) override;
    virtual bool mouseMoveEvent(const PlatformMouseEvent *event) override;
    virtual bool mousePressEvent(const PlatformMouseEvent *event) override;
    virtual bool mouseReleaseEvent(const PlatformMouseEvent *event) override;
    virtual bool wheelEvent(const PlatformWheelEvent *event) override;
    virtual bool moveEvent(const PlatformMoveEvent *event) override;
    virtual bool resizeEvent(const PlatformResizeEvent *event) override;
    virtual bool activationEvent(const PlatformWindowEvent *event) override;
    virtual bool closeEvent(const PlatformWindowEvent *event) override;
    virtual bool gamepadConnectionEvent(const PlatformGamepadEvent *event) override;
    virtual bool gamepadKeyPressEvent(const PlatformGamepadKeyEvent *event) override;
    virtual bool gamepadKeyReleaseEvent(const PlatformGamepadKeyEvent *event) override;
    virtual bool gamepadAxisEvent(const PlatformGamepadAxisEvent *event) override;

 private:
    std::shared_ptr<Io::Mouse> mouse = nullptr;
    bool _closing = false;
};
