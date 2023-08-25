#pragma once

#include <memory>

#include "GUI/GUIWindow.h"

class GUIFont;
class GraphicsImage;

class GUICredits : public GUIWindow {
 public:
    GUICredits();
    virtual ~GUICredits();

    virtual void Update() override;

    static void ExecuteCredits();
    void EventLoop();

 protected:
    std::unique_ptr<GUIFont> pFontQuick;
    std::unique_ptr<GUIFont> pFontCChar;

    GraphicsImage *mm6title;

    int width;
    int height;
    GraphicsImage *cred_texture;
    float move_Y;
};
