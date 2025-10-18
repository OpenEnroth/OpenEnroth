#pragma once

#include "Utility/IndexedArray.h"

#include "Library/Platform/Filters/PlatformEventFilter.h"
#include "Library/Platform/Proxy/ProxyEventLoop.h"

class KeyboardController: public PlatformEventFilter, public ProxyEventLoop {
 public:
    KeyboardController();

    bool IsKeyPressedThisFrame(PlatformKey key) const;
    bool IsKeyDown(PlatformKey key) const;

    void reset();

 private:
    virtual bool keyPressEvent(const PlatformKeyEvent *event) override;
    virtual bool keyReleaseEvent(const PlatformKeyEvent *event) override;
    virtual void processMessages(PlatformEventHandler *eventHandler) override;

 private:
    /** Whether the key is currently held down. */
    IndexedArray<bool, PlatformKey::KEY_FIRST, PlatformKey::KEY_LAST> isKeyDown_ = {{}};

    IndexedArray<bool, PlatformKey::KEY_FIRST, PlatformKey::KEY_LAST> isKeyPressedThisFrame_ = {{}};
};
