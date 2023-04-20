#pragma once

#include <string>

#include "GUI/GUIWindow.h"


class GUIWindow_Dialogue : public GUIWindow {
 public:
    GUIWindow_Dialogue(Pointi position, Sizei dimensions, WindowData data, const std::string &hint = std::string());
    virtual ~GUIWindow_Dialogue() {}

    virtual void Update();
    virtual void Release();
};

void GameUI_InitializeDialogue(Actor *actor, int bPlayerSaysHello);

class GUIWindow_GenericDialogue : public GUIWindow {
 public:
    GUIWindow_GenericDialogue(Pointi position, Sizei dimensions, WindowData data, const std::string &hint = std::string());
    virtual ~GUIWindow_GenericDialogue() {}

    virtual void Update();
    virtual void Release();
};

void StartBranchlessDialogue(int eventid, int entryline, int button);
void ReleaseBranchlessDialogue();

void OnSelectNPCDialogueOption(DIALOGUE_TYPE option);

extern const IndexedArray<const char *, PartyAlignment_Good, PartyAlignment_Evil> DialogueBackgroundResourceByAlignment;
