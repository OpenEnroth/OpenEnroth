#pragma once

#include <Engine/Graphics/Renderer/RendererEnums.h>

#include <Library/Logger/LogCategory.h>

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
 * @brief An Overlay is a screen built in Immediate Mode GUI ( nuklear ) which is displayed on top of any other ui and it's 
 * mainly used for debug purposes ( Console, Render Stats, Debug View Buttons, etc... ).
 * 
 * The Overlay System is the class responsible to keep track of all the registered Overlays and update them accordingly.
 */
class OverlaySystem {
 public:
    OverlaySystem(Renderer& render, RendererType rendererType, PlatformApplication &platformApplication);
    ~OverlaySystem();

    void addOverlay(std::string_view name, std::unique_ptr<Overlay> overlay);
    void removeOverlay(std::string_view name);

    void update();

    static LogCategory OverlayLogCategory;

 private:
    std::unordered_map<std::string, std::unique_ptr<Overlay>> _overlays;
    std::unique_ptr<IOverlayRenderer> _renderer;
    std::unique_ptr<nk_context> _nuklearContext;
    std::function<void()> _unregisterDependencies;
};
