#include "KeyPressEventHandler.h"

KeyPressEventHandler::KeyPressEventHandler(PlatformKey keyToPress, const std::function<void()> &callback)
    : _keyToPress(keyToPress)
    , _callback(callback) {
}

bool KeyPressEventHandler::keyPressEvent(const PlatformKeyEvent *event) {
    if (event->key == _keyToPress) {
        _callback();
        return true;
    }
    return false;
}
