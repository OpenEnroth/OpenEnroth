#pragma once

#include <Engine/Graphics/Nuklear.h>

#include <unordered_map>
#include <string>
#include <memory>
#include <utility>
#include <functional>

class DebugView;
class GameConfig;
class IDebugViewRenderer;
class PlatformApplication;
class Renderer;

/** This system oversees all debugging UI elements. 
* Any console or debug information, referred to as DebugView, is 
* registered within the system and will be displayed by using
* the ImGUI nuklear library
*/
class DebugViewSystem {
 public:
    DebugViewSystem(Renderer& render, bool useOGLES, PlatformApplication &platformApplication);
    ~DebugViewSystem();

    static std::unique_ptr<DebugViewSystem> create(Renderer &render, GameConfig &gameConfig, PlatformApplication &platformApplication);

    template<typename TBindings, typename ...TArgs>
    void addView(std::string_view viewName, TArgs &&... args) {
        auto bindings = std::make_unique<TBindings>(std::forward<TArgs>(args) ...);
        _addView(viewName, std::move(bindings));
    }

    void removeView(std::string_view name) {
    }

    void update();

 private:
    void _addView(std::string_view viewName, std::unique_ptr<DebugView> view);

    std::unordered_map<std::string, std::unique_ptr<DebugView>> _views;
    std::unique_ptr<IDebugViewRenderer> _renderer;
    std::unique_ptr<Nuklear> _nuklear;
    std::function<void()> _unregisterDependencies;
};
