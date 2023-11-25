#pragma once

#include <memory>

#include "RendererEnums.h"

class Renderer;
class GameConfig;

class RendererFactory {
 public:
    std::unique_ptr<Renderer> createRenderer(RendererType type, std::shared_ptr<GameConfig> config);
};
