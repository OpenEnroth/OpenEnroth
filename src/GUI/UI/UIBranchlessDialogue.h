#pragma once

#include <string>

#include "Engine/Evt/EvtEnums.h"

#include "GUI/GUIWindow.h"

class GUIWindow_BranchlessDialogue : public GUIWindow {
 public:
    explicit GUIWindow_BranchlessDialogue(EvtOpcode event);
    virtual ~GUIWindow_BranchlessDialogue() {}

    virtual void Update() override;
    virtual void Release() override;

    EvtOpcode event() const {
        return _event;
    }

 private:
    EvtOpcode _event = EVENT_Invalid;
};

void startBranchlessDialogue(int eventid, int entryline, EvtOpcode type);
void releaseBranchlessDialogue();
