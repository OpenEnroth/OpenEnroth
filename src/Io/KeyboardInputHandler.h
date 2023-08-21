#pragma once

#include <stdint.h>
#include <memory>
#include <string>

#include "Platform/PlatformEnums.h"
#include "Io/IKeyboardController.h"
#include "Io/KeyboardActionMapping.h"
#include "Engine/Time.h"

class GUIWindow;
namespace Io {
class IKeyboardController;
}  // namespace Io

constexpr int DELAY_TOGGLE_TIME_FIRST = Timer::Second / 2;
constexpr int DELAY_TOGGLE_TIME_AFTER = Timer::Second / 15;

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
    KeyboardInputHandler(IKeyboardController *controller, std::shared_ptr<KeyboardActionMapping> actionMapping) {
        this->controller = controller;
        this->actionMapping = actionMapping;

        lastKeyPressed = PlatformKey::KEY_NONE;
        inputType = TextInputType::None;
        window = nullptr;
        ResetKeys();
    }

    bool IsKeyboardPickingOutlineToggled() const;
    bool IsRunKeyToggled() const;
    bool IsTurnStrafingToggled() const;
    bool IsStealingToggled() const;
    bool IsTakeAllToggled() const;
    bool IsAdventurerBackcycleToggled() const;
    bool IsSpellBackcycleToggled() const;
    bool IsCastOnClickToggled() const;
    bool IsKeyHeld(PlatformKey key) const;

    void GenerateInputActions();

    void SetWindowInputStatus(WindowInputStatus status);
    inline PlatformKey LastPressedKey() const {
        return lastKeyPressed;
    }
    void ResetKeys();

    void StartTextInput(TextInputType type, int max_string_len, GUIWindow *pWindow);
    bool ProcessTextInput(PlatformKey key, int c);
    void EndTextInput();

    const std::string &GetTextInput() const;
    void SetTextInput(const std::string &text);

 private:
    void GeneratePausedActions();
    void GenerateGameplayActions();

    IKeyboardController *controller;
    std::shared_ptr<KeyboardActionMapping> actionMapping;

    PlatformKey lastKeyPressed;
    int keydelaytimer;
    int max_input_string_len;
    std::string pPressedKeysBuffer;
    TextInputType inputType;
    GUIWindow *window;
};
}  // namespace Io


bool UI_OnKeyDown(PlatformKey key);


extern std::shared_ptr<Io::KeyboardInputHandler> keyboardInputHandler;
