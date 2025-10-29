#pragma once

#include "Utility/IndexedArray.h"

#include "Library/Platform/Filters/PlatformEventFilter.h"
#include "Library/Platform/Proxy/ProxyEventLoop.h"

class KeyboardController: public PlatformEventFilter, public ProxyEventLoop {
 public:
    KeyboardController();

    /**
     * @param key                       Key to check.
     * @return                          Whether the key was pressed at least once this frame.
     */
    [[nodiscard]] bool isKeyPressedThisFrame(PlatformKey key) const;

    /**
     * @param key                       Key to check.
     * @return                          Whether the key is held down this frame. Key is considered held down for the
     *                                  frame if it was pressed (and potentially released) this frame, or if it was
     *                                  pressed in one of the previous frames and is still not released.
     */
    [[nodiscard]] bool isKeyDownThisFrame(PlatformKey key) const;

    void reset();

 private:
    virtual bool keyPressEvent(const PlatformKeyEvent *event) override;
    virtual bool keyReleaseEvent(const PlatformKeyEvent *event) override;
    virtual void processMessages(PlatformEventHandler *eventHandler) override;

 private:
    /** Whether the key is currently held down. */
    IndexedArray<bool, PlatformKey::KEY_FIRST, PlatformKey::KEY_LAST> _isKeyDown = {{}};

    /** Whether the key was pressed this frame. */
    IndexedArray<bool, PlatformKey::KEY_FIRST, PlatformKey::KEY_LAST> _isKeyPressedThisFrame = {{}};
};
