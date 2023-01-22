#pragma once

#include <memory>

#include "Platform/Filters/PlatformEventFilter.h"

class NuklearEventHandler : public PlatformEventFilter {
 public:
    NuklearEventHandler();

 private:
    virtual bool KeyPressEvent(const PlatformKeyEvent *event) override;
    virtual bool KeyReleaseEvent(const PlatformKeyEvent *event) override;
    virtual bool MouseMoveEvent(const PlatformMouseEvent *event) override;
    virtual bool MousePressEvent(const PlatformMouseEvent *event) override;
    virtual bool MouseReleaseEvent(const PlatformMouseEvent *event) override;
    virtual bool WheelEvent(const PlatformWheelEvent *event) override;

    bool KeyEvent(PlatformKey key, PlatformModifiers mods, bool down);
    bool MouseEvent(PlatformMouseButton button, const Pointi &pos, bool down);
};
