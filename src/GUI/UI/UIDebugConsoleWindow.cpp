#include "UIDebugConsoleWindow.h"

#include <algorithm>
#include <iostream>

#include "Application/GameKeyboardController.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Party.h"

#include "Library/Platform/Application/PlatformApplication.h"

const char* const PROMPT_PREFIX = "> ";

ConsoleState* GUIWindow_DebugConsoleMenu::_consoleState = new ConsoleState();

bool isKeyPressed(PlatformKey key, bool &state) {
    bool isKeyPressed = ::application->component<GameKeyboardController>()->IsKeyDown(key);
    if (!isKeyPressed) {
        state = false;
    } else if(!state) {
        state = true;
    } else {
        isKeyPressed = false;
    }

    return isKeyPressed;
}

GUIWindow_DebugConsoleMenu::GUIWindow_DebugConsoleMenu()
    : GUIWindow(WINDOW_DebugMenu, {0, 0}, render->GetRenderDimensions()) {

    _commandExecuteHandle = engine->commandManager->onCommandExecuted([this](const std::string& resultMessage, const std::string& command, bool isSuccess) {
        if (!resultMessage.empty()) {
            _consoleState->addToHistory(command, colorTable.White);
            _consoleState->addToHistory(resultMessage, isSuccess ? colorTable.White : colorTable.Red);
            _consoleState->addToCommandHistory(command, colorTable.White);
        }
    });

    pEventTimer->setPaused(true);

    keyboardInputHandler->StartTextInput(Io::TextInputType::Text, 100, this);
}

void GUIWindow_DebugConsoleMenu::Release() {
    engine->commandManager->removeOnCommandExecutedCallback(_commandExecuteHandle);
    GUIWindow::Release();
}

void GUIWindow_DebugConsoleMenu::Update() {
    drawBackground();
    drawCommandLineBackground();
    drawCommandLinePrompt();
    drawTextHistory();
    updateCommandHistoryIndex();
}

void GUIWindow_DebugConsoleMenu::drawBackground() {
    auto rect = getWindowRect();
    render->FillRectFast(rect.x, rect.y, rect.w, rect.h, colorTable.GunmetalGray);
}

void GUIWindow_DebugConsoleMenu::drawCommandLineBackground() {
    auto rect = getCommandLineRect();
    render->FillRectFast(rect.x, rect.y, rect.w, rect.h, colorTable.DarkGray);
}

void GUIWindow_DebugConsoleMenu::drawCommandLinePrompt() {
    auto rect = getCommandLineRect();
    DrawText(getFont(), { rect.x + 2, rect.y }, colorTable.White, PROMPT_PREFIX);
    switch (keyboard_input_status) {
    case WINDOW_INPUT_IN_PROGRESS: {
        onKeyboardInputInProgress(rect);
        break;
    }
    case WINDOW_INPUT_CONFIRMED: {
        onKeyboardInputConfirmed();
        break;
    }
    }
}

void GUIWindow_DebugConsoleMenu::onKeyboardInputInProgress(const Recti &rect) {
    const auto& textInput = keyboardInputHandler->GetTextInput();
    const int marginPromptPrefix = getFont()->GetLineWidth(PROMPT_PREFIX) + 8;
    const int textInputSize = getFont()->GetLineWidth(textInput);
    DrawText(getFont(), { marginPromptPrefix, rect.y }, colorTable.White, textInput);
    DrawFlashingInputCursor(textInputSize + marginPromptPrefix, rect.y, getFont());
}

void GUIWindow_DebugConsoleMenu::onKeyboardInputConfirmed() {
    const auto& textInput = keyboardInputHandler->GetTextInput();
    if (!textInput.empty()) {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_DebugConsoleConfirm, 0, 0);
        keyboard_input_status = WINDOW_INPUT_NONE;
    } else {
        keyboard_input_status = WINDOW_INPUT_IN_PROGRESS;
    }
}

void GUIWindow_DebugConsoleMenu::drawTextHistory() {
    auto windowRect = getWindowRect();
    auto lineHeight = getFont()->GetHeight() - 2;
    auto offset = 4;
    auto maxLines = ((windowRect.h - offset) / lineHeight) - 1;
    for (int i = std::min((int)_consoleState->historyLines.size() - 1, maxLines); i >= 0; --i) {
        const auto& line = _consoleState->historyLines[i];
        const auto y = (windowRect.h - (lineHeight * (i + 1))) - offset;
        DrawText(getFont(), { windowRect.x + 2, y }, line.color, line.text);
    }
}

void GUIWindow_DebugConsoleMenu::updateCommandHistoryIndex() {
    if (isKeyPressed(PlatformKey::KEY_UP, _isUpPressed)) {
        keyboardInputHandler->SetTextInput(_consoleState->prevCommand());
    } else if (isKeyPressed(PlatformKey::KEY_DOWN, _isDownPressed)) {
        keyboardInputHandler->SetTextInput(_consoleState->nextCommand());
    }
}

GUIFont* GUIWindow_DebugConsoleMenu::getFont() const {
    return assets->pFontLucida.get();
}

Recti GUIWindow_DebugConsoleMenu::getWindowRect() const {
    return Rect(8, 8, 460, 350 - getFont()->GetHeight());
}

Recti GUIWindow_DebugConsoleMenu::getCommandLineRect() const {
    auto height = getFont()->GetHeight();
    return Rect(8, 350 - height, 460, height);
}

void ConsoleState::addToHistory(const std::string& text, const Color& color) {
    historyLines.insert(historyLines.begin(), { text, color });
}

void ConsoleState::addToCommandHistory(const std::string& text, const Color& color) {
    commandHistoryLines.push_back({ text, color });
    currentCommandHistoryIndex = commandHistoryLines.size();
}

std::string ConsoleState::prevCommand() {
    if (currentCommandHistoryIndex - 1 >= 0) {
        --currentCommandHistoryIndex;
    }

    if (currentCommandHistoryIndex < commandHistoryLines.size()) {
        return commandHistoryLines[currentCommandHistoryIndex].text;
    }

    return "";
}

std::string ConsoleState::nextCommand() {
    if (currentCommandHistoryIndex + 1 < commandHistoryLines.size()) {
        ++currentCommandHistoryIndex;
    }
    if (currentCommandHistoryIndex < commandHistoryLines.size()) {
        return commandHistoryLines[currentCommandHistoryIndex].text;
    }

    return "";
}
