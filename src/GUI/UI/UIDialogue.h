#pragma once

#include <string>

#include "GUI/GUIWindow.h"


class GUIWindow_Dialogue : public GUIWindow {
 public:
    GUIWindow_Dialogue(unsigned int x, unsigned int y, unsigned int width, unsigned int height, WindowData data, const std::string &hint = std::string());
    virtual ~GUIWindow_Dialogue() {}

    virtual void Update();
    virtual void Release();
};

void GameUI_InitializeDialogue(Actor *actor, int bPlayerSaysHello);

class GUIWindow_GenericDialogue : public GUIWindow {
 public:
    GUIWindow_GenericDialogue(unsigned int x, unsigned int y, unsigned int width, unsigned int height, WindowData data, const std::string &hint = std::string());
    virtual ~GUIWindow_GenericDialogue() {}

    virtual void Update();
    virtual void Release();
};

void StartBranchlessDialogue(int eventid, int entryline, int button);

void OnSelectNPCDialogueOption(DIALOGUE_TYPE option);
