#pragma once
#include "GUI/UI/UIBooks.h"

struct GUIWindow_QuestBook : public GUIWindow_Book {
    GUIWindow_QuestBook();
    virtual ~GUIWindow_QuestBook() {}

    virtual void Update();
};
