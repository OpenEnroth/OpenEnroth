#pragma once

#include <RmlUi/Core/ObserverPtr.h>
#include <RmlUi/Core/Element.h>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

class PlatformApplication;
class Renderer;
class RmlFileInterface;
class RmlEventListenerInstancer;
class RmlFontEngineInterface;
class RmlSystemInterface;
class UiRenderer;

typedef long long ScreenHandle;

namespace Rml {
class SystemInterface;
class Context;
class ElementDocument;
}

class UiSystem {
 public:
    UiSystem(
        PlatformApplication& platformApplication,
        Renderer& renderer,
        bool debugContextEnabled,
        bool enableReload,
        std::string_view documentSubFolder
    );
    ~UiSystem();

    void update();
    void loadScreen(std::string_view filename, std::string_view id);
    void unloadScreen(std::string_view id);

 private:
    void _createSystemInterface();
    void _createEventHandler(PlatformApplication &platformApplication, bool enableReload);
    void _createEventListenerInstancer();
    void _createFontEngineInterface();
    void _createFileInterface();
    void _createRenderer();
    void _createMainContext();
    void _loadFonts();

    void _reloadAllScreens();
    void _hideAllScreens();
    void _showAllScreens();

    struct Screen {
        Rml::ObserverPtr<Rml::Element> document;
        std::string filename;

        bool isValid() const;
        Rml::ElementDocument *asDocument() const;
    };

    std::unordered_map<std::string, std::unique_ptr<Screen>> _screens;
    std::unique_ptr<UiRenderer> _uiRenderer;
    std::unique_ptr<RmlSystemInterface> _systemInterface;
    std::unique_ptr<RmlFileInterface> _fileInterface;
    std::unique_ptr<RmlFontEngineInterface> _fontEngineInterface;
    std::unique_ptr<RmlEventListenerInstancer> _eventListenerInstancer;
    Renderer &_renderer;
    Rml::Context *_mainContext{};
    std::string_view _documentSubFolder;
    bool _debugContextEnabled{};
};
