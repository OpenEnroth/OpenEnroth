#pragma once

#include <string>

#include "GUI/GUIWindow.h"

#include "Utility/IndexedArray.h"

class GUIWindow_Dialogue : public GUIWindow {
 public:
    explicit GUIWindow_Dialogue(WindowData data);
    virtual ~GUIWindow_Dialogue() {}

    virtual void Update() override;
    virtual void Release() override;
};

void initializeNPCDialogue(Actor *actor, int bPlayerSaysHello);

void selectNPCDialogueOption(DIALOGUE_TYPE option);

extern const IndexedArray<std::string, PartyAlignment_Good, PartyAlignment_Evil> dialogueBackgroundResourceByAlignment;
