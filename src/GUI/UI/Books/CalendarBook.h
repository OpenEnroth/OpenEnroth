#pragma once
#include "GUI/UI/UIBooks.h"

struct GUIWindow_CalendarBook : public GUIWindow_Book {
    GUIWindow_CalendarBook();
    virtual ~GUIWindow_CalendarBook() {}

    virtual void Update();
};
