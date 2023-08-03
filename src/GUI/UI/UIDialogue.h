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

void GameUI_InitializeDialogue(Actor *actor, int bPlayerSaysHello);

class GUIWindow_GenericDialogue : public GUIWindow {
 public:
    GUIWindow_GenericDialogue(Pointi position, Sizei dimensions, WindowData data, const std::string &hint = std::string());
    virtual ~GUIWindow_GenericDialogue() {}

    virtual void Update() override;
    virtual void Release() override;
};

void StartBranchlessDialogue(int eventid, int entryline, int button);
void ReleaseBranchlessDialogue();

void OnSelectNPCDialogueOption(DIALOGUE_TYPE option);

extern const IndexedArray<std::string, PartyAlignment_Good, PartyAlignment_Evil> dialogueBackgroundResourceByAlignment;
