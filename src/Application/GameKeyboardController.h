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
    IndexedArray<bool, PlatformKey::KEY_FIRST, PlatformKey::KEY_LAST> isKeyDown_ = {{}};
    IndexedArray<bool, PlatformKey::KEY_FIRST, PlatformKey::KEY_LAST> isKeyDownReported_ = {{}};
};
