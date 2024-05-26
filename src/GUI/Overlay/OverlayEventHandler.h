#pragma once

#include "Library/Platform/Filters/PlatformEventFilter.h"

struct nk_context;

class OverlayEventHandler : public PlatformEventFilter {
 public:
    explicit OverlayEventHandler(struct nk_context* context);

 private:
    virtual bool keyPressEvent(const PlatformKeyEvent *event) override;
    virtual bool keyReleaseEvent(const PlatformKeyEvent *event) override;
    virtual bool mouseMoveEvent(const PlatformMouseEvent *event) override;
    virtual bool mousePressEvent(const PlatformMouseEvent *event) override;
    virtual bool mouseReleaseEvent(const PlatformMouseEvent *event) override;
    virtual bool wheelEvent(const PlatformWheelEvent *event) override;
    virtual bool textInputEvent(const PlatformTextInputEvent *event) override;
    virtual bool nativeEvent(const PlatformNativeEvent *event) override;

    bool keyEvent(PlatformKey key, PlatformModifiers mods, bool down);
    bool mouseEvent(PlatformMouseButton button, const Pointi &pos, bool down);

    struct nk_context *_context;
};
