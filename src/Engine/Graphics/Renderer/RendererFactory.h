#pragma once

#include <memory>
#include <string>

#include "Engine/Graphics/Renderer/RendererEnums.h"

class Renderer;
class GameConfig;

// TODO(captainurist): merge into GameStarter
class RendererFactory {
 public:
    std::shared_ptr<Renderer> Create(std::shared_ptr<GameConfig> config);
};
