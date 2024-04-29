#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <utility>
#include <functional>

class Overlay;
class GameConfig;
class IOverlayRenderer;
class PlatformApplication;
class Renderer;
struct nk_context;

/**
 * @brief This system oversees all debugging UI elements. Any console or debug information, referred to as Overlay, is
 * registered within the system and will be displayed by using the immediate mode nuklear library.
 */
class OverlaySystem {
 public:
    OverlaySystem(Renderer& render, GameConfig& gameConfig, PlatformApplication &platformApplication);
    ~OverlaySystem();

    void addOverlay(std::string_view name, std::unique_ptr<Overlay> overlay);
    void removeOverlay(std::string_view name);

    void update();

 private:
    std::unordered_map<std::string, std::unique_ptr<Overlay>> _overlays;
    std::unique_ptr<IOverlayRenderer> _renderer;
    std::unique_ptr<nk_context> _nuklearContext;
    std::function<void()> _unregisterDependencies;
};
