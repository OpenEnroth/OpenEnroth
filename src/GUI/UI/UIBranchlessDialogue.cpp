#include "UIBranchlessDialogue.h"

#include "Engine/AssetsManager.h"
#include "Engine/Events/Processor.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Party.h"
#include "Engine/mm7_data.h"

#include "GUI/GUIFont.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIGame.h"

#include "Io/KeyboardInputHandler.h"

GUIWindow_BranchlessDialogue::GUIWindow_BranchlessDialogue(EventType event) : GUIWindow(WINDOW_GreetingNPC, {0, 0}, render->GetRenderDimensions()), _event(event) {
    prev_screen_type = current_screen_type;
    keyboardInputHandler->StartTextInput(Io::TextInputType::Text, 15, this);
    current_screen_type = SCREEN_BRANCHLESS_NPC_DIALOG;

    CreateButton({61, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 1, Io::InputAction::SelectChar1);
    CreateButton({177, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 2, Io::InputAction::SelectChar2);
    CreateButton({292, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 3, Io::InputAction::SelectChar3);
    CreateButton({407, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 4, Io::InputAction::SelectChar4);
    CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_CycleCharacters, 0, Io::InputAction::CharCycle, "");
}

void GUIWindow_BranchlessDialogue::Release() {
    current_screen_type = prev_screen_type;
    keyboardInputHandler->SetWindowInputStatus(WINDOW_INPUT_CANCELLED);

    GUIWindow::Release();
}

void GUIWindow_BranchlessDialogue::Update() {
    GUIFont *pFont = assets->pFontArrus.get();

    if (current_npc_text.length() > 0 && branchless_dialogue_str.empty())
        branchless_dialogue_str = current_npc_text;

    GUIWindow BranchlessDlg_window;
    BranchlessDlg_window.uFrameWidth = game_viewport_width;
    BranchlessDlg_window.uFrameZ = 452;
    int pTextHeight = assets->pFontArrus->CalcTextHeight(branchless_dialogue_str, BranchlessDlg_window.uFrameWidth, 12) + 7;
    if (352 - pTextHeight < 8) {
        pFont = assets->pFontCreate.get();
        pTextHeight = assets->pFontCreate->CalcTextHeight(branchless_dialogue_str, BranchlessDlg_window.uFrameWidth, 12) + 7;
    }

    render->DrawTextureCustomHeight(8 / 640.0f, (352 - pTextHeight) / 480.0f, ui_leather_mm7, pTextHeight);
    render->DrawTextureNew(8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
    pGUIWindow_BranchlessDialogue->DrawText(pFont, {12, 354 - pTextHeight}, colorTable.White,
                                            pFont->FitTextInAWindow(branchless_dialogue_str, BranchlessDlg_window.uFrameWidth, 12));
    render->DrawTextureNew(0, 352 / 480.0f, game_ui_statusbar);

    // TODO(Nik-RE-dev): this code related to text input in MM6/MM8, revisit
    // this functionality when it's time to support it.
#if 0
    if (pGUIWindow_BranchlessDialogue->keyboard_input_status != WINDOW_INPUT_IN_PROGRESS) {
        if (pGUIWindow_BranchlessDialogue->keyboard_input_status == WINDOW_INPUT_CONFIRMED) {
            pGUIWindow_BranchlessDialogue->keyboard_input_status = WINDOW_INPUT_NONE;
            GameUI_StatusBar_OnInput(keyboardInputHandler->GetTextInput());
            releaseBranchlessDialogue();
            return;
        }
        if (pGUIWindow_BranchlessDialogue->keyboard_input_status != WINDOW_INPUT_CANCELLED)
            return;
        pGUIWindow_BranchlessDialogue->keyboard_input_status = WINDOW_INPUT_NONE;
        GameUI_StatusBar_ClearInputString();
        releaseBranchlessDialogue();
        return;
    }

    if (pGUIWindow_BranchlessDialogue->event() == EVENT_InputString) {
        auto str = fmt::format("{} {}", GameUI_StatusBar_GetInput(), keyboardInputHandler->GetTextInput());
        pGUIWindow_BranchlessDialogue->DrawText(pFontLucida, {13, 357}, colorTable.White, str);
        pGUIWindow_BranchlessDialogue->DrawFlashingInputCursor(pFontLucida->GetLineWidth(str) + 13, 357, pFontLucida);
        return;
    }
#endif

    // Close branchless dialog on any keypress
    if (!keyboardInputHandler->GetTextInput().empty()) {
        keyboardInputHandler->SetWindowInputStatus(WINDOW_INPUT_NONE);
        releaseBranchlessDialogue();
        return;
    }

    // Also close branchless dialog on enter
    if (pGUIWindow_BranchlessDialogue->keyboard_input_status != WINDOW_INPUT_IN_PROGRESS) {
        releaseBranchlessDialogue();
        return;
    }
}

void startBranchlessDialogue(int eventid, int entryline, EventType type) {
    if (!pGUIWindow_BranchlessDialogue) {
        pMiscTimer->setPaused(true);
        pEventTimer->setPaused(true);
        savedEventID = eventid;
        savedEventStep = entryline;
        savedDecoration = activeLevelDecoration;
        pGUIWindow_BranchlessDialogue = new GUIWindow_BranchlessDialogue(type);
    }
}

void releaseBranchlessDialogue() {
    pGUIWindow_BranchlessDialogue->Release();
    pGUIWindow_BranchlessDialogue = nullptr;
    if (savedEventID) {
        // Do not run event engine whith no event, it may happen when you close talk window
        // with NPC that only say catch phrases
        activeLevelDecoration = savedDecoration;
        eventProcessor(savedEventID, Pid(), 1, savedEventStep);
    }
    activeLevelDecoration = nullptr;
    pEventTimer->setPaused(false);
}

