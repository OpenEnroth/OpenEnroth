#include "OverlayBindings.h"

#include <GUI/Overlay/OverlaySystem.h>
#include <GUI/Overlay/ScriptedOverlay.h>

#include "ImGuiBindings.h"

#include <memory>

OverlayBindings::OverlayBindings(OverlaySystem &overlaySystem) : _overlaySystem(overlaySystem) {
}

sol::table OverlayBindings::createBindingTable(sol::state_view &solState) const {
    sol::table table = solState.create_table_with(
        "addOverlay", sol::as_function([this, &solState](std::string_view name, sol::table view) {
            _overlaySystem.addOverlay(name, std::make_unique<ScriptedOverlay>(name, solState, view));
        }),
        "removeOverlay", sol::as_function([this](std::string_view name) {
            _overlaySystem.removeOverlay(name);
        })
    );
    ImGuiBindings::Init(solState, table);

    return table;
}
