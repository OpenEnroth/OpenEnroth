#pragma once
#include "GUI/UI/UIBooks.h"

const char *GetMapBookHintText(int mouse_x, int mouse_y);  // sub_444564

struct GUIWindow_MapBook : public GUIWindow_Book {
    GUIWindow_MapBook();
    virtual ~GUIWindow_MapBook() {}

    virtual void Update();
};
