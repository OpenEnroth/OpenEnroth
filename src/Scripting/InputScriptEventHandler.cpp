#include "InputScriptEventHandler.h"

InputScriptEventHandler::InputScriptEventHandler() : PlatformEventFilter({ EVENT_KEY_PRESS }) {
}

bool InputScriptEventHandler::keyPressEvent(const PlatformKeyEvent *event) {
    if (auto itr = _keyPressCallbacks.find(event->key); itr != _keyPressCallbacks.end()) {
        for (auto &&callback : itr->second) {
            return callback();
        }
    }

    return false;
}

void InputScriptEventHandler::registerKeyPress(PlatformKey key, const sol::function &callback) {
    if (auto itr = _keyPressCallbacks.find(key); itr != _keyPressCallbacks.end()) {
        itr->second.push_back(callback);
    } else {
        _keyPressCallbacks.insert({ key, { callback } });
    }
}

void InputScriptEventHandler::unregisterKeyPress(PlatformKey key, const sol::function &callbackToRemove) {
    if (auto itr = _keyPressCallbacks.find(key); itr != _keyPressCallbacks.end()) {
        std::erase_if(itr->second, [callbackPointer = callbackToRemove.pointer()](const auto &callback) {
            return callback.pointer() == callbackPointer;
        });
    }
}
