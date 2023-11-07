#pragma once

#include <string>

#include "GUI/GUIWindow.h"

#include "Utility/IndexedArray.h"

enum class DialogWindowType {
    /** This one doesn't seem to be used on MM7, only creates the profession details & hire/fire topics. */
    DIALOG_WINDOW_HIRE_FIRE_SHORT = 1,

    /** Creates all appropriate dialog options, including scripted ones. */
    DIALOG_WINDOW_FULL = 3,
};
using enum DialogWindowType;

class GUIWindow_Dialogue : public GUIWindow {
 public:
    explicit GUIWindow_Dialogue(DialogWindowType type);
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
