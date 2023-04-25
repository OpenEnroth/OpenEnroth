#pragma once
#include "GUI/UI/UIBooks.h"

struct GUIWindow_LloydsBook : public GUIWindow_Book {
    GUIWindow_LloydsBook();
    virtual ~GUIWindow_LloydsBook() {}

    virtual void Update();
 private:
    int _maxBeacons;
};

extern bool bRecallingBeacon;
extern bool isLloydsBeaconBeingInstalled;
extern int lloydsBeaconCasterId;
extern int lloydsBeaconSpellDuration;
