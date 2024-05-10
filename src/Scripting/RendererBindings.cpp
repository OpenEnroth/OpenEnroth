#include "RendererBindings.h"

#include <Engine/Graphics/Renderer/Renderer.h>

sol::table RendererBindings::createBindingTable(sol::state_view &solState) const {
    return solState.create_table_with(
        "reloadShaders", sol::as_function([](std::string_view alignment) {
            render->ReloadShaders();
        })
    );
}
