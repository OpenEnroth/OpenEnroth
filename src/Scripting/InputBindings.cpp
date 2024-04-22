#include "InputBindings.h"

#include <Library/Logger/Logger.h>
#include <string>
#include <optional>

#include "InputScriptEventHandler.h"

InputBindings::InputBindings(const sol::state_view &solState, InputScriptEventHandler &inputScriptEventHandler)
    : _solState(solState)
    , _inputScriptEventHandler(inputScriptEventHandler) {
}

void InputBindings::init() {
    _solState.set_function("initInput", [this]() {
        sol::table mainTable = _solState.create_table_with(
            "registerKeyPress", [this](sol::table keyPressConfig) {
                PlatformKey key = keyPressConfig["key"];
                sol::function callback = keyPressConfig["callback"];
                _inputScriptEventHandler.registerKeyPress(key, callback);
            },
            "unregisterKeyPress", [this](sol::table keyPressConfig) {
                PlatformKey key = keyPressConfig["key"];
                sol::function callback = keyPressConfig["callback"];
                _inputScriptEventHandler.unregisterKeyPress(key, callback);
            }
        );
        return mainTable;
    });
}
