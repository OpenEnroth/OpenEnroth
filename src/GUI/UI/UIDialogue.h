#pragma once

#include <string>

#include "GUI/GUIWindow.h"

#include "Utility/IndexedArray.h"

class GUIWindow_Dialogue : public GUIWindow {
 public:
    explicit GUIWindow_Dialogue(WindowData data);
    virtual ~GUIWindow_Dialogue() {}

    void setDisplayedDialogueType(DIALOGUE_TYPE type) {
        _displayedDialogue = type;
    }

    DIALOGUE_TYPE getDisplayedDialogueType() {
        return _displayedDialogue;
    }

    virtual void Update() override;
    virtual void Release() override;

 protected:
    DIALOGUE_TYPE _displayedDialogue = DIALOGUE_MAIN;
};

void initializeNPCDialogue(Actor *actor, int bPlayerSaysHello);

void selectNPCDialogueOption(DIALOGUE_TYPE option);

extern const IndexedArray<std::string, PartyAlignment_Good, PartyAlignment_Evil> dialogueBackgroundResourceByAlignment;
