#include "UiSystem.h"

#include "OpenGLUiRenderer.h"
#include "RmlEventListenerInstancer.h"
#include "RmlFileInterface.h"
#include "RmlFontEngineInterface.h"
#include "RmlSystemInterface.h"
#include "UiEventHandler.h"

#include <Engine/Graphics/Renderer/Renderer.h>
#include <Library/Logger/Logger.h>
#include <Library/Platform/Application/PlatformApplication.h>
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <Utility/DataPath.h>

#include <utility>
#include <algorithm>

UiSystem::UiSystem(PlatformApplication &platformApplication, Renderer &renderer, bool debugContextEnabled, std::string_view documentSubFolder)
    : _renderer(renderer)
    , _documentSubFolder(documentSubFolder)
    , _debugContextEnabled(debugContextEnabled) {
    _createRenderer();
    _createFileInterface();
    _createFontEngineInterface();
    _createSystemInterface();
    _createEventListenerInstancer();

    platformApplication.installComponent(std::make_unique<UiEventHandler>(_renderer, [this]() {
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

void UiSystem::_createRenderer() {
    auto oglRenderer = std::make_unique<OpenGLUiRenderer>();
    _renderer.setUiRenderer(oglRenderer.get());
    Rml::SetRenderInterface(oglRenderer.get());
    _uiRenderer = std::move(oglRenderer);
}

void UiSystem::_createFileInterface() {
    _fileInterface = std::make_unique<RmlFileInterface>();
    Rml::SetFileInterface(_fileInterface.get());
}

void UiSystem::_createFontEngineInterface() {
    _fontEngineInterface = std::make_unique<RmlFontEngineInterface>();
    Rml::SetFontEngineInterface(_fontEngineInterface.get());
}

void UiSystem::_createSystemInterface() {
    _systemInterface = std::make_unique<RmlSystemInterface>();
    Rml::SetSystemInterface(_systemInterface.get());
}

void UiSystem::_createEventListenerInstancer() {
    _eventListenerInstancer = std::make_unique<RmlEventListenerInstancer>();
    Rml::Factory::RegisterEventListenerInstancer(_eventListenerInstancer.get());
}

void UiSystem::_createMainContext() {
    auto renderDimensions = _renderer.GetRenderDimensions();
    _mainContext = Rml::CreateContext("main", Rml::Vector2i(renderDimensions.w, renderDimensions.h));
    if (_mainContext && _debugContextEnabled) {
        Rml::Debugger::Initialise(_mainContext);
    }
}

void UiSystem::_loadFonts() {
    Rml::LoadFontFace(makeDataPath(_documentSubFolder, "arial.ttf"));
}
