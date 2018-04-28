#pragma once

/*  284 */
enum InputAction : __int32 {
    INPUT_MoveForward = 0x0,
    INPUT_MoveBackwards = 0x1,
    INPUT_TurnLeft = 0x2,
    INPUT_TurnRight = 0x3,
    INPUT_Yell = 0x4,
    INPUT_Jump = 0x5,
    INPUT_Combat = 0x6,
    INPUT_CastReady = 0x7,
    INPUT_Attack = 0x8,
    INPUT_EventTrigger = 0x9,
    INPUT_Cast = 0xA,
    INPUT_Pass = 0xB,
    INPUT_CharCycle = 0xC,
    INPUT_Quest = 0xD,
    INPUT_QuickRef = 0xE,
    INPUT_Rest = 0xF,
    INPUT_TimeCal = 0x10,
    INPUT_Autonotes = 0x11,
    INPUT_Mapbook = 0x12,
    INPUT_AlwaysRun = 0x13,
    INPUT_LookUp = 0x14,
    INPUT_LookDown = 0x15,
    INPUT_CenterView = 0x16,
    INPUT_ZoomIn = 0x17,
    INPUT_ZoomOut = 0x18,
    INPUT_FlyUp = 0x19,
    INPUT_FlyDown = 0x1A,
    INPUT_Land = 0x1B,
    INPUT_StrafeLeft = 0x1C,
    INPUT_StrafeRight = 0x1D,
};

enum KeyToggleType : __int32 {
    TOGGLE_Continuously = 0x0,
    TOGGLE_OneTimePress = 0x1,
};

class GUIWindow;

#pragma pack(push, 1)
struct KeyboardActionMapping {
    KeyboardActionMapping();

    void SetKeyMapping(int uAction, int vKey, KeyToggleType type);
    unsigned int GetActionVKey(enum InputAction eAction);
    const char *GetVKeyDisplayName(unsigned char a1);
    const unsigned __int8 TranslateKeyNameToKeyCode(const char *Str);
    void ReadMappings();
    void StoreMappings();
    bool ProcessTextInput(unsigned int a2);
    void SetWindowInputStatus(int a2);
    void EnterText(int a2, int max_string_len, GUIWindow *pWindow);
    void ResetKeys();
    void SetDefaultMapping();

    unsigned int uLastKeyPressed;
    int field_4;
    int field_8;
    unsigned int pVirtualKeyCodesMapping[30];
    KeyToggleType pToggleTypes[30];
    int max_input_string_len;
    __int8 pPressedKeysBuffer[257];
    unsigned __int8 uNumKeysPressed;
    char field_202;
    char field_203;
    int field_204;
    GUIWindow *pWindow;
};
#pragma pack(pop)

#pragma pack(push, 1)
class Keyboard {
 public:
    inline Keyboard() : bUsingAsynKeyboard(false) {}
    bool WasKeyPressed(int vKey);
    static bool IsKeyBeingHeld(int vKey);
    void ProcessInputActions();
    bool IsShiftHeld();
    void EnterCriticalSection();

    void (***vdestructor_ptr)(Keyboard *, bool);
    unsigned int bUsingAsynKeyboard;
};
#pragma pack(pop)

void OnPressSpace();

extern struct KeyboardActionMapping *pKeyActionMap;
