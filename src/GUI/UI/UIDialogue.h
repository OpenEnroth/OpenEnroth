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

    void setDisplayedDialogueType(DialogueId type) {
        _displayedDialogue = type;
    }

    DialogueId getDisplayedDialogueType() {
        return _displayedDialogue;
    }

    virtual void Update() override;
    virtual void Release() override;

 protected:
    DialogueId _displayedDialogue = DIALOGUE_MAIN;
};

void initializeNPCDialogue(int npcId, int bPlayerSaysHello, Actor *actor = nullptr);

void selectNPCDialogueOption(DialogueId option);

extern int speakingNpcId;
extern const IndexedArray<std::string, PartyAlignment_Good, PartyAlignment_Evil> dialogueBackgroundResourceByAlignment;
