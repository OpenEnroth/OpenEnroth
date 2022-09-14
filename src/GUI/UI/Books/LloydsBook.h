#pragma once
#include "GUI/UI/UIBooks.h"

struct GUIWindow_LloydsBook : public GUIWindow_Book {
    GUIWindow_LloydsBook();
    virtual ~GUIWindow_LloydsBook() {}

    virtual void Update();
};

extern bool _506360_installing_beacon;  // 506360
