#include "InputScriptEventHandler.h"

#include <Library/Logger/Logger.h>

InputScriptEventHandler::InputScriptEventHandler() : PlatformEventFilter({ EVENT_KEY_PRESS }) {
}

bool InputScriptEventHandler::keyPressEvent(const PlatformKeyEvent *event) {
    bool isHandled = false;
    sol::safe_function function = _scriptFunctionProvider("_globalOnKeyPress");
    if (function.valid()) {
        try {
            function.set_error_handler(_scriptFunctionProvider("_errorHandler"));
            auto result = function(event->key);
            if (result.valid()) {
                return result;
            }
        } catch (const sol::error &e) {
            logger->error("[Script] An unexpected error has occurred: ", e.what());
        }
    }
    return false;
}

void InputScriptEventHandler::setScriptFunctionProvider(const ScriptFunctionProvider &scriptFunctionProvider) {
    _scriptFunctionProvider = scriptFunctionProvider;
}
