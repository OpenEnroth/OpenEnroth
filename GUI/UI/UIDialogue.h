#pragma once

#include "GUI/GUIWindow.h"

class GUIWindow_Dialogue : public GUIWindow {
 public:
    GUIWindow_Dialogue(unsigned int x, unsigned int y, unsigned int width, unsigned int height, GUIButton *button, const String &hint = String());
    virtual ~GUIWindow_Dialogue() {}

    virtual void Update();
    virtual void Release();
};

void GameUI_InitializeDialogue(struct Actor *actor, int bPlayerSaysHello);

class GUIWindow_GenericDialogue : public GUIWindow {
 public:
    GUIWindow_GenericDialogue(unsigned int x, unsigned int y, unsigned int width, unsigned int height, GUIButton *button, const String &hint = String());
    virtual ~GUIWindow_GenericDialogue() {}

    virtual void Update();
    virtual void Release();
};

void sub_4451A8_press_any_key(int a1, int a2, int a4);

void OnSelectNPCDialogueOption(DIALOGUE_TYPE newDialogueType);
