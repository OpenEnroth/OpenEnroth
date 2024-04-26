#include "Nuklear.h"

#include <nuklear_config.h> // NOLINT: not a C system header.
#include <memory>

Nuklear::Nuklear() {
    _context = std::make_unique<nk_context>();
}

Nuklear::~Nuklear() {
}

void Nuklear::update() {
    /*if (wins[winType].state == WINDOW_INITIALIZED) {
        if (wins[winType].ui_draw >= 0) {
            lua_rawgeti(lua, LUA_REGISTRYINDEX, wins[winType].ui_draw);
            lua_pushlightuserdata(lua, (void *)&wins[winType]);
            lua_pushinteger(lua, stage);
            int err = lua_pcall(lua, 2, 0, 0);
            lua_error_check(winType, lua, err);
        } else {
            wins[winType].state = WINDOW_TEMPLATE_ERROR;
        }*/
    nk_input_end(_context.get());
    //if (render->GetPresentDimensions() == render->GetRenderDimensions())
    //    render->NuklearRender(NK_ANTI_ALIASING_ON, NUKLEAR_MAX_VERTEX_MEMORY, NUKLEAR_MAX_ELEMENT_MEMORY);
    nk_input_begin(_context.get());
}
