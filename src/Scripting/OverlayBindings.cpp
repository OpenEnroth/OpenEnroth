#include "OverlayBindings.h"

#include "NuklearLegacyBindings.h"

#include <GUI/Overlay/OverlaySystem.h>
#include <GUI/Overlay/ScriptedOverlay.h>

#include <memory>

OverlayBindings::OverlayBindings(OverlaySystem &overlaySystem) : _overlaySystem(overlaySystem) {
}

sol::table OverlayBindings::createBindingTable(sol::state_view &solState) const {
    NuklearLegacyBindings::initBindings(solState.lua_state());

    // Here we're referencing the global ui table exposed by the nuklear legacy bindings
    // and we're going to expose it under the "nk" table inside the Bindings Table.
    // By doing so we're not going to be forced to port all the lua bindings to sol2
    sol::table nuklearTable = solState["_nuklear_legacy_ui"];
    nuklearTable.new_enum<false>("EditState",
        "NK_EDIT_ACTIVE", NK_EDIT_ACTIVE,
        "NK_EDIT_INACTIVE", NK_EDIT_INACTIVE,
        "NK_EDIT_ACTIVATED", NK_EDIT_ACTIVATED,
        "NK_EDIT_DEACTIVATED", NK_EDIT_DEACTIVATED,
        "NK_EDIT_COMMITED", NK_EDIT_COMMITED
    );

    return solState.create_table_with(
        "addOverlay", sol::as_function([this, &solState](std::string_view name, sol::table view) {
            _overlaySystem.addOverlay(name, std::make_unique<ScriptedOverlay>(name, solState, view));
        }),
        "removeOverlay", sol::as_function([this](std::string_view name) {
            _overlaySystem.removeOverlay(name);
        }),
        "nk", nuklearTable
    );
}
