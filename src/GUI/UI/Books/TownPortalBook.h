#pragma once
#include "GUI/UI/UIBooks.h"

class GUIWindow_TownPortalBook : public GUIWindow_Book {
 public:
    explicit GUIWindow_TownPortalBook(int casterPid);
    virtual ~GUIWindow_TownPortalBook() {}

    virtual void Update();

    void clickTown(int townId);
    void hintTown(int townId);
 private:
    int _casterPid;
};
