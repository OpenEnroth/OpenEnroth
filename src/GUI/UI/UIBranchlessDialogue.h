#pragma once

#include <string>

#include "Engine/Events/EventEnums.h"
#include "GUI/GUIWindow.h"

enum class EventType : uint8_t;

class GUIWindow_BranchlessDialogue : public GUIWindow {
 public:
    explicit GUIWindow_BranchlessDialogue(WindowData data);
    virtual ~GUIWindow_BranchlessDialogue() {}

    virtual void Update() override;
    virtual void Release() override;
};

void startBranchlessDialogue(int eventid, int entryline, EventType type);
void releaseBranchlessDialogue();
