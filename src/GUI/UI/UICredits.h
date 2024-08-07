#pragma once

#include <memory>

#include "GUI/GUIWindow.h"

class GUIFont;

class GUICredits : public GUIWindow {
 public:
    explicit GUICredits(std::function<void()> onReachEndOfCredits);
    virtual ~GUICredits();

    virtual void Update() override;

 private:
    std::unique_ptr<GUIFont> _fontQuick;
    std::unique_ptr<GUIFont> _fontCChar;

    GraphicsImage *_mm6TitleTexture;
    GraphicsImage *_creditsTexture;
    float _moveY;
    std::function<void()> _onReachEndOfCredits;
};
