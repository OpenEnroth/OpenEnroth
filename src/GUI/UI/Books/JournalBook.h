#pragma once
#include "GUI/UI/UIBooks.h"

struct GUIWindow_JournalBook : public GUIWindow_Book {
    GUIWindow_JournalBook();
    virtual ~GUIWindow_JournalBook() {}

    virtual void Update();
};
