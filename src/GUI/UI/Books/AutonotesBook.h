#pragma once
#include "GUI/UI/UIBooks.h"
#include "Engine/Autonotes.h"

struct GUIWindow_AutonotesBook : public GUIWindow_Book {
    GUIWindow_AutonotesBook();
    virtual ~GUIWindow_AutonotesBook() {}

    virtual void Update();
};

extern AUTONOTE_TYPE autonoteBookDisplayType;
