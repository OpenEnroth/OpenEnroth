#pragma once

#include <memory>
#include <string>

#include "Library/Platform/Interface/PlatformEnums.h"
#include "Io/KeyboardController.h"
#include "Io/KeyboardActionMapping.h"
#include "Engine/Time/Duration.h"

class GUIWindow;

enum class WindowInputStatus : int32_t {
    WINDOW_INPUT_NONE = 0,
    WINDOW_INPUT_IN_PROGRESS = 1,
    WINDOW_INPUT_CONFIRMED = 2,
    WINDOW_INPUT_CANCELLED = 3,
};
using enum WindowInputStatus;

namespace Io {
    // Handles events from OSWindow through GameWindowHandler/IKeyboardController
    //      and maps it to game actions/events using KeyboardActionMapping
class KeyboardInputHandler {
 public:
    KeyboardInputHandler(KeyboardController *controller, std::shared_ptr<KeyboardActionMapping> actionMapping) {
        this->controller = controller;
        this->actionMapping = actionMapping;

        lastKeyPressed = PlatformKey::KEY_NONE;
        inputType = TextInputType::None;
        window = nullptr;
    }

    bool IsRunKeyToggled() const;
    bool IsTurnStrafingToggled() const;
    bool IsStealingToggled() const;
    bool IsTakeAllToggled() const;
    bool IsAdventurerBackcycleToggled() const;
    bool IsSpellBackcycleToggled() const;
    bool IsCastOnClickToggled() const;

    void GenerateInputActions();

    void SetWindowInputStatus(WindowInputStatus status);
    inline PlatformKey LastPressedKey() const {
        return lastKeyPressed;
    }

    void StartTextInput(TextInputType type, int max_string_len, GUIWindow *pWindow);
    bool ProcessTextInput(PlatformKey key, int c);
    void EndTextInput();

    const std::string &GetTextInput() const;
    void SetTextInput(std::string_view text);

 private:
    void GeneratePausedActions();
    void GenerateGameplayActions();
    void ProcessPausedAction(InputAction action);
    void ProcessGameplayAction(InputAction action);

    KeyboardController *controller;
    std::shared_ptr<KeyboardActionMapping> actionMapping;

    PlatformKey lastKeyPressed;
    Duration keydelaytimer;
    int max_input_string_len;
    std::string pPressedKeysBuffer;
    TextInputType inputType;
    GUIWindow *window;
};
}  // namespace Io


bool UI_OnKeyDown(PlatformKey key);


extern std::shared_ptr<Io::KeyboardInputHandler> keyboardInputHandler;
