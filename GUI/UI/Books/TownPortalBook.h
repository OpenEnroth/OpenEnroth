#pragma once
#include "GUI/UI/UIBooks.h"

struct GUIWindow_TownPortalBook : public GUIWindow_Book {
    explicit GUIWindow_TownPortalBook(const char *a1);
    virtual ~GUIWindow_TownPortalBook() {}

    virtual void Update();
};
