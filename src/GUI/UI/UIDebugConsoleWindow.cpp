#include "UIDebugConsoleWindow.h"

#include <algorithm>

#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Party.h"

GUIWindow_DebugConsoleMenu::GUIWindow_DebugConsoleMenu()
    : GUIWindow(WINDOW_DebugMenu, {0, 0}, render->GetRenderDimensions()) {

    pEventTimer->setPaused(true);

    CreateButton({8, 8}, {460, 344}, 1, 0, UIMSG_DebugConsoleEdit, 0, Io::InputAction::Invalid, "WRITE A COMMAND");

    keyboardInputHandler->StartTextInput(Io::TextInputType::Text, 100, this);
}

void GUIWindow_DebugConsoleMenu::Update() {
    render->DrawTwodVerts();
    render->EndLines2D();
    render->EndTextNew();

    render->BeginLines2D();

    DrawBackground(8, 8, 460, 344);
    DrawCommandLinePrompt();
    DrawTextHistory();
}

void GUIWindow_DebugConsoleMenu::DrawBackground(int x, int y, int width, int height) {
    render->FillRectFast(x, y, width + 1, height + 1, colorTable.GunmetalGray);
}

void GUIWindow_DebugConsoleMenu::DrawCommandLinePrompt() {
    static const char* promptPrefix = "> ";
    DrawText(assets->pFontArrus.get(), { 8, 300 }, colorTable.White, promptPrefix);
    if (keyboard_input_status == WINDOW_INPUT_IN_PROGRESS) {
        const auto& textInput = keyboardInputHandler->GetTextInput();
        const int marginPromptPrefix = assets->pFontArrus->GetLineWidth(promptPrefix) + 8;
        const int textInputSize = assets->pFontArrus->GetLineWidth(textInput);
        DrawText(assets->pFontArrus.get(), { marginPromptPrefix, 300 }, colorTable.White, textInput);
        DrawFlashingInputCursor(textInputSize + marginPromptPrefix, 300, assets->pFontArrus.get());
    } else if (keyboard_input_status == WINDOW_INPUT_CONFIRMED) {
        const auto& textInput = keyboardInputHandler->GetTextInput();
        if (!textInput.empty()) {
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_DebugConsoleConfirm, 0, 0);
            _historyLines.insert(_historyLines.begin(), textInput);
            keyboard_input_status = WINDOW_INPUT_NONE;
        } else {
            keyboard_input_status = WINDOW_INPUT_IN_PROGRESS;
        }
    }
}

void GUIWindow_DebugConsoleMenu::DrawTextHistory() {
    static const int MAX_LINES_TO_SHOW = 14;
    static const int START_X = 10;
    static const int START_Y = 300;
    for (int i = std::min((int)_historyLines.size() - 1, MAX_LINES_TO_SHOW); i >= 0; --i) {
        const auto& line = _historyLines[i];
        const auto y = START_Y - (assets->pFontArrus->GetHeight() * (i + 1));
        DrawText(assets->pFontArrus.get(), { START_X, y }, colorTable.White, line);
    }
}
