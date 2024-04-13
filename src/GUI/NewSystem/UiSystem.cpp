#include "UiSystem.h"

#include "KeyPressEventHandler.h"
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
#include <memory>
#include <string>
#include <tuple>
#include <vector>

UiSystem::UiSystem(
    PlatformApplication &platformApplication,
    Renderer &renderer,
    bool debugContextEnabled,
    bool enableReload,
    std::string_view documentSubFolder)
    : _renderer(renderer)
    , _documentSubFolder(documentSubFolder)
    , _debugContextEnabled(debugContextEnabled) {
    _createRenderer();
    _createFileInterface();
    _createFontEngineInterface();
    _createSystemInterface();
    _createEventListenerInstancer();
    _createEventHandler(platformApplication, enableReload);

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

void UiSystem::loadScreen(std::string_view filename, std::string_view id) {
    Rml::ElementDocument *document = _mainContext->LoadDocument(makeDataPath(_documentSubFolder, filename));
    if (document) {
        document->Show();
        auto screen = std::make_unique<Screen>();
        screen->filename = filename;
        screen->document = document->GetObserverPtr();
        _screens.insert({ id.data(), std::move(screen) });
    }
}

void UiSystem::unloadScreen(std::string_view id) {
    if (auto itr = _screens.find(id.data()); itr != _screens.end()) {
        if (itr->second->isValid()) {
            itr->second->asDocument()->Close();
            _screens.erase(itr);
        }
    }
}

void UiSystem::_reloadAllScreens() {
    std::vector<std::tuple<std::string, std::string>> screensToReload;
    for (auto &&screenPair : _screens) {
        if (screenPair.second->isValid()) {
            screensToReload.push_back({ screenPair.second->filename, screenPair.first });
            screenPair.second->asDocument()->Close();
        }
    }

    _screens.clear();

    for (auto &&screenToLoad : screensToReload) {
        loadScreen(std::get<0>(screenToLoad), std::get<1>(screenToLoad));
    }
}

void UiSystem::_hideAllScreens() {
    for (auto &&screenPair : _screens) {
        if (screenPair.second->isValid()) {
            screenPair.second->asDocument()->Hide();
        }
    }
}

void UiSystem::_showAllScreens() {
    for (auto &&screenPair : _screens) {
        if (screenPair.second->isValid()) {
            screenPair.second->asDocument()->Show();
        }
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

void UiSystem::_createEventHandler(PlatformApplication &platformApplication, bool enableReload) {
    auto eventHandler = std::make_unique<UiEventHandler>(_renderer, [this]() {
        return _mainContext;
    });
    if (enableReload) {
        eventHandler->addKeyPressEventHandler(PlatformKey::KEY_F11, [this]() {
            _reloadAllScreens();
        });
        eventHandler->addKeyPressEventHandler(PlatformKey::KEY_F10, [this]() {
            static bool bToggle = false;
            if (bToggle) {
                _showAllScreens();
            } else {
                _hideAllScreens();
            }
            bToggle = !bToggle;
        });
    }
    platformApplication.installComponent(std::move(eventHandler));
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

bool UiSystem::Screen::isValid() const {
    return (bool)document;
}

Rml::ElementDocument *UiSystem::Screen::asDocument() const {
    return static_cast<Rml::ElementDocument *>(document.get());
}
