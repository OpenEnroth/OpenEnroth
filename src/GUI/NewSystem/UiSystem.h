#pragma once

#include <memory>
#include <string_view>

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
}

class UiSystem {
 public:
    UiSystem(PlatformApplication& platformApplication, Renderer& renderer, bool debugContextEnabled, std::string_view documentSubFolder);
    ~UiSystem();

    void update();
    ScreenHandle loadScreen(std::string_view filename);
    void unloadScreen(ScreenHandle screenHandle);

 private:
    void _createSystemInterface();
    void _createEventListenerInstancer();
    void _createFontEngineInterface();
    void _createFileInterface();
    void _createRenderer();
    void _createMainContext();
    void _loadFonts();

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
