#pragma once
#include "GUI/UI/UIBooks.h"

struct GUIWindow_TownPortalBook : public GUIWindow_Book {
    explicit GUIWindow_TownPortalBook(int casterPid);  // const char *a1);
    virtual ~GUIWindow_TownPortalBook() {}

    virtual void Update();
};

static const int TOWN_PORTAL_DESTINATION_COUNT = 6;

extern std::array<int, TOWN_PORTAL_DESTINATION_COUNT> townPortalQuestBits;
extern int townPortalCasterPid;
