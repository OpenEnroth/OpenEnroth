#pragma once

#include <array>

#include "Utility/IndexedArray.h"

#include "Library/Application/PlatformApplicationAware.h"

#include "Io/IKeyboardController.h"

class PlatformKeyEvent;

// TODO(captainurist): deriving from PlatformApplicationAware is a temporary (and ugly!) measure.
// We need it so that `EngineTracer` can call reset. Just turn this one into an event filter!
class GameKeyboardController: public Io::IKeyboardController, public PlatformApplicationAware {
 public:
    GameKeyboardController();

    virtual bool ConsumeKeyPress(PlatformKey key) override;
    virtual bool IsKeyDown(PlatformKey key) const override;

    void ProcessKeyPressEvent(const PlatformKeyEvent *event);
    void ProcessKeyReleaseEvent(const PlatformKeyEvent *event);

    void reset();

 private:
    IndexedArray<bool, PlatformKey::Count> isKeyDown_ = {{}};
    IndexedArray<bool, PlatformKey::Count> isKeyDownReported_ = {{}};
};
