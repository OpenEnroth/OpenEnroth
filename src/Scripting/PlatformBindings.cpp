#include "PlatformBindings.h"

#include "Library/Platform/Application/PlatformApplication.h"

PlatformBindings::PlatformBindings(const sol::state_view &solState, PlatformApplication &platformApplication)
    : _solState(solState)
    , _platformApplication(platformApplication) {
}

PlatformBindings::~PlatformBindings() {
}

sol::table PlatformBindings::getBindingTable() {
    if(!_bindingTable) {
        _bindingTable = _solState.create_table_with(
            "window", _solState.create_table_with(
                "dimensions", [this]() {
                    const Sizei size = _platformApplication.window()->size();
                    return std::tuple{size.w, size.h};
                }
            )
        );
    }
    return _bindingTable;
}
