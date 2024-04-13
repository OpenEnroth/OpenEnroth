#pragma once

#include <Library/Platform/Interface/PlatformEvents.h>

#include <functional>

class KeyPressEventHandler {
 public:
    explicit KeyPressEventHandler(PlatformKey keyToPress, const std::function<void()> &callback);

    bool keyPressEvent(const PlatformKeyEvent *event);

 private:
    std::function<void()> _callback;
    PlatformKey _keyToPress;
};
