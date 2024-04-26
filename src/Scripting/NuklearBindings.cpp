#include "NuklearBindings.h"

#include <Engine/Graphics/Nuklear.h>

NuklearBindings::NuklearBindings(const sol::state_view &solState, Nuklear *nuklear) : _nuklear(nuklear) {
}

sol::table NuklearBindings::getBindingTable() {
    //We already have a lot of bindings done without sol2. It's going to take a bit before we move everything here.
    return _bindingTable;
}
