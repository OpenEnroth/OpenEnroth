#pragma once

#include <memory>
#include <string_view>

class PlatformApplication;
class Renderer;
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
    void _createRenderer(Renderer& renderer);
    void _createMainContext();
    void _loadFonts();

    std::unique_ptr<UiRenderer> _uiRenderer;
    std::unique_ptr<RmlSystemInterface> _systemInterface;
    Rml::Context *_mainContext{};
    std::string_view _documentSubFolder;
    bool _debugContextEnabled{};
};
