#include "OverlaySystem.h"
#include "OverlayEventHandler.h"

#include <Application/GameConfig.h>
#include <Engine/Graphics/Renderer/NuklearOverlayRenderer.h>
#include <Engine/Graphics/Renderer/Renderer.h>
#include <Library/Platform/Application/PlatformApplication.h>
#include <Scripting/NuklearLegacyBindings.h>

#include "Overlay.h"

#include <nuklear_config.h> // NOLINT: not a C system header.

#include <memory>
#include <utility>

OverlaySystem::OverlaySystem(Renderer &render, RendererType rendererType, PlatformApplication &platformApplication) {
    _nuklearContext = std::make_unique<nk_context>();
    platformApplication.installComponent(std::make_unique<OverlayEventHandler>(_nuklearContext.get()));

    if (rendererType != RENDERER_NULL) {
        const bool useOGLES = rendererType == RENDERER_OPENGL_ES;
        _renderer = std::make_unique<NuklearOverlayRenderer>(_nuklearContext.get(), useOGLES);
        render.setOverlayRenderer(_renderer.get());
    }

    _unregisterDependencies = [&render, &platformApplication]() {
        platformApplication.removeComponent<OverlayEventHandler>();
        render.setOverlayRenderer(nullptr);
    };

    NuklearLegacyBindings::setContext(_nuklearContext.get());
}

OverlaySystem::~OverlaySystem() {
    _unregisterDependencies();
}

void OverlaySystem::addOverlay(std::string_view name, std::unique_ptr<Overlay> overlay) {
    _overlays.insert({ name.data(), std::move(overlay) });
}

void OverlaySystem::removeOverlay(std::string_view name) {
    if (auto itr = _overlays.find(name.data()); itr != _overlays.end()) {
        _overlays.erase(itr);
    }
}

void OverlaySystem::update() {
    auto context = _nuklearContext.get();
    nk_input_end(context);
    for (auto &&[name, overlay] : _overlays) {
        overlay->update(context);
    }
    nk_input_begin(context);
}
