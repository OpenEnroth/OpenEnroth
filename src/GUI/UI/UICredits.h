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

    GraphicsImage *_mm6TitleTexture = nullptr;
    GraphicsImage *_creditsTexture = nullptr;
    float _moveY = 0;
};
