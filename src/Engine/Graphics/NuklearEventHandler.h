#pragma once

#include "Platform/Filters/PlatformEventFilter.h"

class NuklearEventHandler : public PlatformEventFilter {
 public:
    NuklearEventHandler();

 private:
    virtual bool keyPressEvent(const PlatformKeyEvent *event) override;
    virtual bool keyReleaseEvent(const PlatformKeyEvent *event) override;
    virtual bool mouseMoveEvent(const PlatformMouseEvent *event) override;
    virtual bool mousePressEvent(const PlatformMouseEvent *event) override;
    virtual bool mouseReleaseEvent(const PlatformMouseEvent *event) override;
    virtual bool wheelEvent(const PlatformWheelEvent *event) override;

    bool KeyEvent(PlatformKey key, PlatformModifiers mods, bool down);
    bool MouseEvent(PlatformMouseButton button, const Pointi &pos, bool down);
};
