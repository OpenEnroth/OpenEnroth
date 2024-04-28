#include "PlatformBindings.h"

#include "Library/Platform/Application/PlatformApplication.h"

PlatformBindings::PlatformBindings(PlatformApplication &platformApplication)
    : _platformApplication(platformApplication) {
}

PlatformBindings::~PlatformBindings() {
}

sol::table PlatformBindings::createBindingTable(sol::state_view &solState) const {
    return solState.create_table_with(
        "window", solState.create_table_with(
            "dimensions", [this]() {
                const Sizei size = _platformApplication.window()->size();
                return std::tuple{size.w, size.h};
            }
        )
    );
}
