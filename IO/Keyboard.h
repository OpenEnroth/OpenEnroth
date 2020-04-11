#pragma once

#include <map>
#include <memory>
#include <string>

#include "IO/GameKey.h"
#include "IO/InputAction.h"

enum class KeyToggleType : int32_t {
    TOGGLE_Continuously = 0x0,
    TOGGLE_OneTimePress = 0x1,
};

KeyToggleType GetToggleType(InputAction action);


class GUIWindow;

//#pragma pack(push, 1)
//class Keyboard {
//public:
//    bool WasKeyPressed(int vKey);
//    static bool IsKeyBeingHeld(int vKey);
//    void ProcessInputActions();
//    bool IsShiftHeld() const;
//};
//#pragma pack(pop)

enum class TextInputType : int {
    None = 0,
    Text = 1,
    Number = 2,
};

#pragma pack(push, 1)
struct KeyboardActionMapping { //its actually key->action mapper, controller is UserInputHandler
    KeyboardActionMapping();

    void MapKey(InputAction action, GameKey key);
    void MapKey(InputAction action, GameKey key, KeyToggleType type);
    GameKey MapDefaultKey(InputAction action);

    GameKey GetKey(InputAction action) const;
    KeyToggleType GetToggleType(InputAction action) const;


    //const char *GetVKeyDisplayName(unsigned char a1);
    //const uint8_t TranslateKeyNameToKeyCode(const char *Str);
    void ReadMappings();
    void StoreMappings();
    void SetDefaultMapping();

private:
    //GameKey pVirtualKeyCodesMapping[30];
    std::map<InputAction, GameKey> actionKeyMap;
    //KeyToggleType pToggleTypes[30];
    std::map<InputAction, KeyToggleType> keyToggleMap;
};
#pragma pack(pop)

void OnPressSpace();

extern KeyboardActionMapping *keyboardActionMapping;
