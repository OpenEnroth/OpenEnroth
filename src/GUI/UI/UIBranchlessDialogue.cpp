#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "UIBranchlessDialogue.h"

#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/Evt/Processor.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Party.h"
#include "Engine/mm7_data.h"
#include "Engine/Graphics/Viewport.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIGame.h"

#include "Io/KeyboardInputHandler.h"

#include "Utility/String/Ascii.h"

// State of the pending EVENT_InputString question, set by startBranchlessQuestion.
static std::vector<std::string> branchlessAnswers;
static int branchlessMatchStep = 0;

GUIWindow_BranchlessDialogue::GUIWindow_BranchlessDialogue(EvtOpcode opcode, std::optional<EvtContinuation> continuation)
    : GUIWindow(WINDOW_GREETING_NPC, {0, 0}, render->GetRenderDimensions()), _opcode(opcode), _continuation(continuation) {
    prev_screen_type = current_screen_type;
    keyboardInputHandler->StartTextInput(TEXT_INPUT_TEXT, 15, this);
    current_screen_type = SCREEN_BRANCHLESS_NPC_DIALOG;

    CreateCharacterButtons();
}

GUIWindow_BranchlessDialogue::~GUIWindow_BranchlessDialogue() {
    current_screen_type = prev_screen_type;
    keyboardInputHandler->EndTextInput(this);
}

void GUIWindow_BranchlessDialogue::Update() {
    if (current_npc_text.length() > 0 && branchless_dialogue_str.empty())
        branchless_dialogue_str = current_npc_text;

    pGUIWindow_BranchlessDialogue->DrawDialoguePanel(branchless_dialogue_str);
    render->DrawQuad2D(game_ui_statusbar, {0, 352});

    if (_opcode == EVENT_InputString) {
        if (!_answered && keyboard_input_status == WINDOW_INPUT_CONFIRMED) {
            // Case-insensitive, whitespace-trimmed compare against each accepted answer.
            std::string typed = ascii::toLower(keyboardInputHandler->GetTextInput());
            while (!typed.empty() && ascii::isSpace(typed.back()))
                typed.pop_back();
            while (!typed.empty() && ascii::isSpace(typed.front()))
                typed.erase(typed.begin());
            bool matched = false;
            for (const std::string &answer : branchlessAnswers)
                matched |= !typed.empty() && typed == ascii::toLower(answer);
            if (matched && _continuation)
                _continuation->step = branchlessMatchStep;
            _answered = true;
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
            return;
        }
        if (!_answered && keyboard_input_status != WINDOW_INPUT_IN_PROGRESS) {
            // Escape: no answer given, resume at the fall-through step.
            _answered = true;
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
            return;
        }
        std::string str = keyboardInputHandler->GetTextInput();
        Sizei dims = render->GetRenderDimensions();
        DrawText(assets->pFontLucida.get(), {13, 357}, colorTable.White, str, Recti(0, 0, dims.w, dims.h));
        DrawFlashingInputCursor(assets->pFontLucida->GetLineWidth(str) + 13, 357, assets->pFontLucida.get(), Recti(0, 0, dims.w, dims.h));
        return;
    }

    // Close branchless dialog on any keypress
    if (!keyboardInputHandler->GetTextInput().empty()) {
        keyboardInputHandler->EndTextInput();
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
        return;
    }

    // Also close branchless dialog on enter
    if (pGUIWindow_BranchlessDialogue->keyboard_input_status != WINDOW_INPUT_IN_PROGRESS) {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
        return;
    }
}

void startBranchlessDialogue(EvtOpcode opcode, std::optional<EvtContinuation> continuation) {
    if (!pGUIWindow_BranchlessDialogue) {
        animTimer->setPaused(true);
        gameTimer->setPaused(true);
        pGUIWindow_BranchlessDialogue = std::make_unique<GUIWindow_BranchlessDialogue>(opcode, continuation);
    }
}

void startBranchlessQuestion(std::optional<EvtContinuation> continuation, int matchStep, std::vector<std::string> answers) {
    branchlessAnswers = std::move(answers);
    branchlessMatchStep = matchStep;
    startBranchlessDialogue(EVENT_InputString, continuation);
}

void releaseBranchlessDialogue() {
    assert(pGUIWindow_BranchlessDialogue);
    std::optional<EvtContinuation> continuation = pGUIWindow_BranchlessDialogue->continuation();
    pGUIWindow_BranchlessDialogue.reset();
    DialogueEnding();
    current_screen_type = SCREEN_GAME;
    if (continuation)
        runEvent(continuation->context, continuation->step); // Can open another dialogue, so nothing may run after it.
}

