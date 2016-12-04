#pragma once
#include "GUI/UI/UIBooks.h"

struct GUIWindow_AutonotesBook : public GUIWindow_Book
{
             GUIWindow_AutonotesBook();
    virtual ~GUIWindow_AutonotesBook() {}

    virtual void Update();
};