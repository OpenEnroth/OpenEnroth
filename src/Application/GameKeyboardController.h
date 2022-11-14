#pragma once

#include <array>

#include "Io/IKeyboardController.h"
#include "Utility/IndexedArray.h"

class PlatformKeyEvent;

class GameKeyboardController: public Io::IKeyboardController {
 public:
    GameKeyboardController();

    virtual bool ConsumeKeyPress(PlatformKey key) override;
    virtual bool IsKeyDown(PlatformKey key) const override;

    void ProcessKeyPressEvent(const PlatformKeyEvent *event);
    void ProcessKeyReleaseEvent(const PlatformKeyEvent *event);

 private:
    IndexedArray<bool, PlatformKey::Count> isKeyDown_ = {{}};
    IndexedArray<bool, PlatformKey::Count> isKeyDownReported_ = {{}};
};
