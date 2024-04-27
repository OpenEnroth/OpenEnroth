#include "InputScriptEventHandler.h"
#include "ScriptingSystem.h"

#include <Library/Logger/Logger.h>
#include <memory>

InputScriptEventHandler::InputScriptEventHandler(std::weak_ptr<sol::state_view> solState) : PlatformEventFilter({ EVENT_KEY_PRESS }), _solState(solState) {
}

bool InputScriptEventHandler::keyPressEvent(const PlatformKeyEvent *event) {
    if (!_solState.expired()) {
        auto sharedSolState = _solState.lock();
        sol::safe_function function = (*sharedSolState)["_globalOnKeyPress"];
        if (function.valid()) {
            try {
                sol::function errorHandler = (*sharedSolState)["_globalErrorHandler"];
                if (errorHandler.valid()) {
                    function.set_error_handler(errorHandler);
                }
                auto result = function(event->key);
                if (result.valid()) {
                    return result;
                }
            } catch (const sol::error &e) {
                logger->error(ScriptingSystem::LogCategory, "An unexpected error has occurred: ", e.what());
            }
        }
    }
    return false;
}
