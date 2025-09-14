#pragma once

#include "Engine/Pid.h"
#include "Engine/Spells/CastSpellInfo.h"

#include "GUI/UI/UIBooks.h"

class GUIWindow_TownPortalBook : public GUIWindow_Book {
 public:
    explicit GUIWindow_TownPortalBook(Pid casterPid, SpellCastFlags castFlags);
    virtual ~GUIWindow_TownPortalBook() {}

    virtual void Update() override;

    void clickTown(int townId);
    void hintTown(int townId);

 private:
    const Pid _casterPid;
    const SpellCastFlags _castFlags;
};
