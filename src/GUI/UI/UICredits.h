#pragma once

#include "GUI/GUIWindow.h"

class GUIFont;

class GUICredits : public GUIWindow {
 public:
    GUICredits();
    virtual ~GUICredits();

    virtual void Update();

    static void ExecuteCredits();
    void EventLoop();

 protected:
    GUIFont *pFontQuick;
    GUIFont *pFontCChar;

    GraphicsImage *mm6title;

    int width;
    int height;
    Texture *cred_texture;
    float move_Y;
};
