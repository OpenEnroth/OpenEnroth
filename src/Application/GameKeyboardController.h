#pragma once

#include "Utility/IndexedArray.h"

#include "Library/Platform/Filters/PlatformEventFilter.h"

#include "Io/IKeyboardController.h"

class GameKeyboardController: public Io::IKeyboardController, public PlatformEventFilter {
 public:
    GameKeyboardController();

    virtual bool ConsumeKeyPress(PlatformKey key) override;
    virtual bool IsKeyDown(PlatformKey key) const override;

    void reset();

 private:
    virtual bool keyPressEvent(const PlatformKeyEvent *event) override;
    virtual bool keyReleaseEvent(const PlatformKeyEvent *event) override;

 private:
    /** Whether the key is currently held down. */
    IndexedArray<bool, PlatformKey::KEY_FIRST, PlatformKey::KEY_LAST> isKeyDown_ = {{}};

    /** Whether there was a key down event that hasn't yet been consumed with a call to `ConsumeKeyPress`.
     * Note that the key might already be released at this point, e.g. if press & release events were received one
     * after another. */
    IndexedArray<bool, PlatformKey::KEY_FIRST, PlatformKey::KEY_LAST> isKeyDownReportPending_ = {{}};
};
