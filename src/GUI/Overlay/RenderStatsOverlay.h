#pragma once

#include "Overlay.h"

class Renderer;

class RenderStatsOverlay : public Overlay {
 public:
    explicit RenderStatsOverlay(Renderer &renderer);

    void update(nk_context &context) override;

 private:
    Renderer& _renderer;
};
