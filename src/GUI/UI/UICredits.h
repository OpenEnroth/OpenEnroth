#pragma once

#include <memory>

#include "GUI/GUIWindow.h"

class GUIFont;

class GUICredits : public GUIWindow {
 public:
    GUICredits();
    virtual ~GUICredits();

    virtual void Update() override;

 private:
    std::unique_ptr<GUIFont> _fontQuick;
    std::unique_ptr<GUIFont> _fontCChar;

    std::shared_ptr<GraphicsImage> _mm6TitleTexture;
    std::unique_ptr<GraphicsImage> _creditsTexture;
    float _moveY = 0;
};
