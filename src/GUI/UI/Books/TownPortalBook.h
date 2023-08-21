#pragma once
#include "GUI/UI/UIBooks.h"
#include "Engine/Pid.h"

class GUIWindow_TownPortalBook : public GUIWindow_Book {
 public:
    explicit GUIWindow_TownPortalBook(Pid casterPid);
    virtual ~GUIWindow_TownPortalBook() {}

    virtual void Update() override;

    void clickTown(int townId);
    void hintTown(int townId);

 private:
    Pid _casterPid;
};
