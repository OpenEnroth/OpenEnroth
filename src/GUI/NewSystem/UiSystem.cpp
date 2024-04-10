#include "UiSystem.h"

#include "RmlSystemInterface.h"
#include "OpenGLUiRenderer.h"
#include "UiInputEventHandler.h"

#include <Engine/Graphics/Renderer/Renderer.h>
#include <Library/Logger/Logger.h>
#include <Library/Platform/Application/PlatformApplication.h>
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <Utility/DataPath.h>

#include <utility>
#include <algorithm>

UiSystem::UiSystem(PlatformApplication &platformApplication, Renderer &renderer, bool debugContextEnabled, std::string_view documentSubFolder)
    : _documentSubFolder(documentSubFolder)
    , _debugContextEnabled(debugContextEnabled) {
    _createRenderer(renderer);
    _createSystemInterface();

    platformApplication.installComponent(std::make_unique<UiInputEventHandler>([this]() {
        return _mainContext;
    }));

    Rml::Initialise();

    _createMainContext();

    _loadFonts();
}

UiSystem::~UiSystem() {
    Rml::Shutdown();
}

void UiSystem::update() {
    _mainContext->Update();
}

ScreenHandle UiSystem::loadScreen(std::string_view filename) {
    Rml::ElementDocument *document = _mainContext->LoadDocument(makeDataPath(_documentSubFolder, filename));
    if (document) {
        document->Show();
        return reinterpret_cast<ScreenHandle>(document); //Very temp way of dealing with handles
    }
    return 0;
}

void UiSystem::unloadScreen(ScreenHandle screenHandle) {
    if (screenHandle != 0) {
        auto document = reinterpret_cast<Rml::ElementDocument *>(screenHandle);
        document->Close();
    }
}

void UiSystem::_createRenderer(Renderer &renderer) {
    auto oglRenderer = std::make_unique<OpenGLUiRenderer>();
    renderer.setUiRenderer(oglRenderer.get());
    Rml::SetRenderInterface(oglRenderer.get());
    _uiRenderer = std::move(oglRenderer);
}

void UiSystem::_createSystemInterface() {
    _systemInterface = std::make_unique<RmlSystemInterface>();
    Rml::SetSystemInterface(_systemInterface.get());
}

void UiSystem::_createMainContext() {
    int width, height;
    _uiRenderer->getViewport(width, height);
    float ratio_width = (float)width / 640;
    float ratio_height = (float)height / 480;
    float ratio = std::min(ratio_width, ratio_height);

    float w = 640 * ratio;
    float h = 480 * ratio;
    float x = (float)width / 2 - w / 2;
    float y = (float)height / 2 - h / 2;
    _mainContext = Rml::CreateContext("main", Rml::Vector2i(w, h));
    if (_mainContext && _debugContextEnabled) {
        Rml::Debugger::Initialise(_mainContext);
    }
}

void UiSystem::_loadFonts() {
    Rml::LoadFontFace(makeDataPath(_documentSubFolder, "arial.ttf"));
}
