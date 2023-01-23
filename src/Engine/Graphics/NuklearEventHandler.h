#pragma once

#include <memory>

#include "Platform/PlatformEventHandler.h"

class NuklearEventHandler : public PlatformEventHandler {
 protected:
    virtual void KeyPressEvent(PlatformWindow *window, const PlatformKeyEvent *event) override;
    virtual void KeyReleaseEvent(PlatformWindow *window, const PlatformKeyEvent *event) override;
    virtual void MouseMoveEvent(PlatformWindow *window, const PlatformMouseEvent *event) override;
    virtual void MousePressEvent(PlatformWindow *window, const PlatformMouseEvent *event) override;
    virtual void MouseReleaseEvent(PlatformWindow *window, const PlatformMouseEvent *event) override;
    virtual void WheelEvent(PlatformWindow *window, const PlatformWheelEvent *event) override;

 private:
    void KeyEvent(PlatformKey key, PlatformModifiers mods, bool down);
    void MouseEvent(PlatformMouseButton button, const Pointi &pos, bool down);
};

extern std::shared_ptr<NuklearEventHandler> nuklearEventHandler; // TODO(captainurist) : convert into an event filter
