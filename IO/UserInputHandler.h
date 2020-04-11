#pragma once

#include <memory>
#include <string>

#include "IO/GameKey.h"
#include "IO/IUserInputProvider.h"

class UserInputHandler {
public:
    UserInputHandler(std::shared_ptr<IUserInputProvider> inputProvider) {
        this->inputProvider = inputProvider;

        uLastKeyPressed = 0;
        inputType = TextInputType::None;
        pWindow = nullptr;
        uNumKeysPressed = 0;
        ResetKeys();
    }

    bool IsKeyboardPickingOutlineToggled() const;
    bool IsRunKeyToggled() const;
    bool IsTurnStrafingToggled() const;


    void GenerateInputActions();

    void SetWindowInputStatus(int a2);
    GameKey LastPressedKey() const;
    void ResetKeys();

    void StartTextInput(TextInputType type, int max_string_len, GUIWindow* pWindow);
    bool ProcessTextInput(GameKey key, int c);

    std::string GetTextInput() const;
    void SetTextInput(const std::string &text);
    void SetTextInput(const char *text);

private:
    std::shared_ptr<IUserInputProvider> inputProvider;

    unsigned int uLastKeyPressed;
    int field_4;
    int field_8;
    int max_input_string_len;
    char pPressedKeysBuffer[257];
    uint8_t uNumKeysPressed;
    TextInputType inputType;
    GUIWindow* pWindow;
};

extern std::shared_ptr<UserInputHandler> userInputHandler;