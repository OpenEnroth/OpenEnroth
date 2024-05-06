#include "OverlaySystem.h"

#include <memory>
#include <utility>

#include <imgui.h> // NOLINT: not a C system header.
#include <nuklear_config.h> // NOLINT: not a C system header.

#include "Application/GameConfig.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Logger/Logger.h"
#include "Scripting/NuklearLegacyBindings.h"

#include "Overlay.h"
#include "OverlayEventHandler.h"

LogCategory OverlaySystem::OverlayLogCategory("Overlay");

OverlaySystem::OverlaySystem(Renderer &renderer, PlatformApplication &platformApplication)
    : _renderer(renderer)
    , _application(platformApplication) {
    _nuklearContext = std::make_unique<nk_context>();
    _application.installComponent(std::make_unique<OverlayEventHandler>(_nuklearContext.get()));
    NuklearLegacyBindings::setContext(_nuklearContext.get());
}

OverlaySystem::~OverlaySystem() {
    _application.removeComponent<OverlayEventHandler>();
}

void OverlaySystem::addOverlay(std::string_view name, std::unique_ptr<Overlay> overlay) {
    if (_overlays.contains(name)) {
        logger->error(OverlayLogCategory, "Can't add overlay \"{}\". Another overlay with the same name already exists.", name);
        return;
    }

    _overlays.insert({ name, std::move(overlay) });
}

void OverlaySystem::removeOverlay(std::string_view name) {
    _overlays.erase(name);
}

void OverlaySystem::drawOverlays() {
    _update();
    _renderer.drawOverlays(_nuklearContext.get());

    if (!_isEnabled) {
        _renderer.beginOverlays();
        //ImGui::ShowDemoWindow();
        _renderer.endOverlays();
    }
}

void OverlaySystem::_update() {
    if (!_isEnabled) {
        return;
    }

    auto context = _nuklearContext.get();

    if (context->style.font != nullptr) {
        nk_input_end(context);
        for (auto &&[name, overlay] : _overlays) {
            overlay->update(context);
        }
        nk_input_begin(context);
    }
}

bool OverlaySystem::isEnabled() const {
    return _isEnabled;
}

void OverlaySystem::setEnabled(bool enable) {
    _isEnabled = enable;
}
