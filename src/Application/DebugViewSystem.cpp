#include "DebugViewSystem.h"
#include "DebugViewEventHandler.h"

#include <Application/GameConfig.h>
#include <Engine/Graphics/Renderer/NuklearDebugViewRenderer.h>
#include <Engine/Graphics/Renderer/Renderer.h>
#include <Library/Platform/Application/PlatformApplication.h>

#include <Engine/Graphics/Nuklear.h>

#include "DebugView.h"

#include <nuklear_config.h> // NOLINT: not a C system header.

#include <memory>
#include <utility>

DebugViewSystem::DebugViewSystem(Renderer &render, bool useOGLES, PlatformApplication &platformApplication) {
    _nuklear = std::make_unique<Nuklear>();
    platformApplication.installComponent(std::make_unique<DebugViewEventHandler>(_nuklear->getContext()));

    _renderer = std::make_unique<NuklearDebugViewRenderer>(_nuklear->getContext(), useOGLES);
    render.setDebugViewRenderer(_renderer.get());

    _unregisterDependencies = [&render, &platformApplication]() {
        platformApplication.removeComponent<DebugViewEventHandler>();
        render.setDebugViewRenderer(nullptr);
    };
}

DebugViewSystem::~DebugViewSystem() {
    _unregisterDependencies();
}

std::unique_ptr<DebugViewSystem> DebugViewSystem::create(Renderer &render, GameConfig &gameConfig, PlatformApplication &platformApplication) {
    const bool useOGLES = gameConfig.graphics.Renderer.value() == RENDERER_OPENGL_ES;
    return std::make_unique<DebugViewSystem>(render, useOGLES, platformApplication);
}

void DebugViewSystem::_addView(std::string_view viewName, std::unique_ptr<DebugView> view) {
    _views.insert({ viewName.data(), std::move(view) });
}

void DebugViewSystem::update() {
    nk_input_end(_nuklear->getContext());
    for (auto &viewPair : _views) {
        viewPair.second->update(*_nuklear);
    }
    nk_input_begin(_nuklear->getContext());
}
