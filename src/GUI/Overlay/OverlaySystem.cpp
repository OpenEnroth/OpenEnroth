#include "OverlaySystem.h"

#include <memory>
#include <utility>

#include <imgui/imgui.h> // NOLINT: not a C system header.

#include "Application/GameConfig.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Library/Platform/Application/PlatformApplication.h"
#include "Library/Logger/Logger.h"

#include "Overlay.h"
#include "OverlayEventHandler.h"

LogCategory OverlaySystem::OverlayLogCategory("Overlay");

OverlaySystem::OverlaySystem(Renderer &renderer, PlatformApplication &platformApplication)
    : _renderer(renderer)
    , _application(platformApplication) {
    _application.installComponent(std::make_unique<OverlayEventHandler>());
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
    // Cycle Imgui even if overlays are disabled and we dont draw anything. This allows it to consume input events at the correct time - #1824.
    _renderer.beginOverlays();
    if (_isEnabled) {
        for (auto &&[name, overlay] : _overlays) {
            overlay->update();
        }
    }
    _renderer.endOverlays();
}

bool OverlaySystem::isEnabled() const {
    return _isEnabled;
}

void OverlaySystem::setEnabled(bool enable) {
    _isEnabled = enable;
}
