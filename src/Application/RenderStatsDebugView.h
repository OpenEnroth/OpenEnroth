#pragma once

#include "DebugView.h"

class Renderer;

class RenderStatsDebugView : public DebugView {
 public:
    explicit RenderStatsDebugView(Renderer &renderer);

    void update(Nuklear &nuklear) override;

 private:
    Renderer& _renderer;
};
