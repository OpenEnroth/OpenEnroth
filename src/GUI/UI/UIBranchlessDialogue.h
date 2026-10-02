#pragma once

#include <string>

#include "Engine/Evt/EvtEnums.h"

#include "GUI/GUIWindow.h"

class GUIWindow_BranchlessDialogue : public GUIWindow {
 public:
    explicit GUIWindow_BranchlessDialogue(EvtOpcode event);
    virtual ~GUIWindow_BranchlessDialogue();

    virtual void Update() override;

    EvtOpcode event() const {
        return _event;
    }

 private:
    EvtOpcode _event = EVENT_Invalid;
};

/**
 * @param type                          Command that opens the dialogue, or `EVENT_Invalid` for an NPC's catchphrase.
 */
void startBranchlessDialogue(EvtOpcode type);
void releaseBranchlessDialogue();
