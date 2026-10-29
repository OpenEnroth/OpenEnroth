#pragma once

#include <string>

#include "GUI/GUIWindow.h"

class Actor;

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
    virtual ~GUIWindow_Dialogue();

    void setDisplayedDialogueType(DialogueId type) {
        _displayedDialogue = type;
    }

    DialogueId displayedDialogueType() {
        return _displayedDialogue;
    }

    virtual void Update() override;

 protected:
    DialogueId _displayedDialogue = DIALOGUE_MAIN;
    GraphicsImage *_dialogueBackground = nullptr;
};

void initializeNPCDialogue(int npcId, int bPlayerSaysHello, Actor *actor = nullptr);

void selectNPCDialogueOption(DialogueId option);

extern int speakingNpcId;

/**
 * @return                              Right panel background of the dialogue windows for the party's alignment.
 */
GraphicsImage *dialogueBackground();
