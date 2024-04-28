#include "DebugViewBindings.h"

DebugViewBindings::DebugViewBindings() {
}

sol::table DebugViewBindings::createBindingTable(sol::state_view &solState) const {
    return solState.create_table_with(
        "addView", [](sol::table view) {
        }
    );
}
