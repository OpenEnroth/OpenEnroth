#pragma once

#include <memory>

#include "Platform/PlatformEventHandler.h"

class NuklearEventHandler : public PlatformEventHandler {
 protected:
    virtual bool KeyPressEvent(const PlatformKeyEvent *event) override;
    virtual bool KeyReleaseEvent(const PlatformKeyEvent *event) override;
    virtual bool MouseMoveEvent(const PlatformMouseEvent *event) override;
    virtual bool MousePressEvent(const PlatformMouseEvent *event) override;
    virtual bool MouseReleaseEvent(const PlatformMouseEvent *event) override;
    virtual bool WheelEvent(const PlatformWheelEvent *event) override;

 private:
    bool KeyEvent(PlatformKey key, PlatformModifiers mods, bool down);
    bool MouseEvent(PlatformMouseButton button, const Pointi &pos, bool down);
};

extern std::shared_ptr<NuklearEventHandler> nuklearEventHandler;
