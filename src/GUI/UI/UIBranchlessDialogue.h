#pragma once

#include <optional>
#include <string>

#include "Engine/Evt/EvtContext.h"
#include "Engine/Evt/EvtEnums.h"

#include "GUI/GUIWindow.h"

class GUIWindow_BranchlessDialogue : public GUIWindow {
 public:
    GUIWindow_BranchlessDialogue(EvtOpcode event, std::optional<EvtContinuation> continuation);
    virtual ~GUIWindow_BranchlessDialogue();

    virtual void Update() override;

    EvtOpcode event() const {
        return _event;
    }

    [[nodiscard]] std::optional<EvtContinuation> continuation() const {
        return _continuation;
    }

 private:
    EvtOpcode _event = EVENT_Invalid;
    std::optional<EvtContinuation> _continuation;
};

/**
 * @param type                          Command that opens the dialogue, or `EVENT_Invalid` for an NPC's catchphrase.
 * @param continuation                  Event paused on the dialogue, or none for a catchphrase.
 */
void startBranchlessDialogue(EvtOpcode type, std::optional<EvtContinuation> continuation);

/**
 * Closes the dialogue, returns to the game screen and resumes the event paused on the dialogue, if there is one.
 */
void releaseBranchlessDialogue();
