#pragma once

#include <string>

#include "Engine/Events/EventEnums.h"

#include "GUI/GUIWindow.h"

class GUIWindow_BranchlessDialogue : public GUIWindow {
 public:
    explicit GUIWindow_BranchlessDialogue(EventType event);
    virtual ~GUIWindow_BranchlessDialogue() {}

    virtual void Update() override;
    virtual void Release() override;

    EventType event() const {
        return _event;
    }

 private:
    EventType _event = EVENT_Invalid;
};

void startBranchlessDialogue(int eventid, int entryline, EventType type);
void releaseBranchlessDialogue();
