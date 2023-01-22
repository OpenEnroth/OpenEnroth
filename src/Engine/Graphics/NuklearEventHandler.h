#pragma once

#include <memory>

#include "Platform/PlatformEventHandler.h"

class NuklearEventHandler : public PlatformEventHandler {
 protected:
    virtual void KeyPressEvent(const PlatformKeyEvent *event) override;
    virtual void KeyReleaseEvent(const PlatformKeyEvent *event) override;
    virtual void MouseMoveEvent(const PlatformMouseEvent *event) override;
    virtual void MousePressEvent(const PlatformMouseEvent *event) override;
    virtual void MouseReleaseEvent(const PlatformMouseEvent *event) override;
    virtual void WheelEvent(const PlatformWheelEvent *event) override;

 private:
    void KeyEvent(PlatformKey key, PlatformModifiers mods, bool down);
    void MouseEvent(PlatformMouseButton button, const Pointi &pos, bool down);
};

extern std::shared_ptr<NuklearEventHandler> nuklearEventHandler; // TODO(captainurist) : convert into an event filter
