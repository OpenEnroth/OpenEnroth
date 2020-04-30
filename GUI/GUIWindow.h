#pragma once

#include <array>
#include <cstdint>
#include <list>
#include <queue>
#include <vector>

#include "Engine/Objects/Player.h"
#include "Engine/Strings.h"
#include "Engine/Party.h"

enum UIMessageType : uint32_t {
    UIMSG_0 = 0,

    UIMSG_ChangeGameState = 5,

    UIMSG_MouseLeftClickInGame = 10,

    UIMSG_CHEST_ClickItem = 12,

    UIMSG_MouseLeftClickInScreen = 14,
    UIMSG_F = 15,

    UIMSG_ChangeCursor = 17,

    UIMSG_Attack = 23,

    UIMSG_CastQuickSpell = 25,

    UIMSG_STEALFROMACTOR = 27,
    UIMSG_1C = 28,
    UIMSG_PlayArcomage = 29,

    UIMSG_31 = 49,
    UIMSG_32 = 50,
    UIMSG_SpellBook_PressTab = 51,
    UIMSG_34 = 52,
    UIMSG_35 = 53,
    UIMSG_MainMenu_ShowPartyCreationWnd = 54,
    UIMSG_MainMenu_ShowLoadWindow = 55,
    UIMSG_ShowCredits = 56,
    UIMSG_ExitToWindows = 57,
    UIMSG_DebugBlv = 58,

    UIMSG_PlayerCreationChangeName = 60,

    UIMSG_PlayerCreationClickPlus = 62,
    UIMSG_PlayerCreationClickMinus = 63,
    UIMSG_PlayerCreationSelectActiveSkill = 64,
    UIMSG_PlayerCreationSelectClass = 65,
    UIMSG_PlayerCreationClickOK = 66,
    UIMSG_PlayerCreationClickReset = 67,
    UIMSG_44 = 68,
    UIMSG_CastSpell_Character_Big_Improvement = 69,
    UIMSG_CastSpell_Shoot_Monster = 70,
    UIMSG_ClickBooksBtn = 71,
    UIMSG_48 = 72,
    UIMSG_49 = 73,
    UIMSG_PlayerCreationRemoveUpSkill = 74,
    UIMSG_PlayerCreationRemoveDownSkill = 75,

    UIMSG_HintSelectRemoveQuickSpellBtn = 78,
    UIMSG_SPellbook_ShowHightlightedSpellInfo = 79,

    UIMSG_BuyInShop_Identify_Repair = 81,
    UIMSG_LoadGame = 82,
    UIMSG_SaveGame = 83,
    UIMSG_54 = 84,
    UIMSG_ChangeDetaliz = 85,
    UIMSG_SelectSpell = 86,
    UIMSG_OpenSpellbookPage = 87,
    UIMSG_ClickInstallRemoveQuickSpellBtn = 88,

    UIMSG_OnTravelByFoot = 90,
    UIMSG_CHANGE_LOCATION_ClickCencelBtn = 91,
    UIMSG_ShowStatus_DateTime = 92,
    UIMSG_ShowStatus_ManaHP = 93,
    UIMSG_ShowStatus_Player = 94,
    UIMSG_Wait5Minutes = 95,
    UIMSG_Wait1Hour = 96,
    UIMSG_Rest8Hour = 97,

    UIMSG_ShowStatus_Food = 100,
    UIMSG_ShowStatus_Funds = 101,

    UIMSG_RestWindow = 104,
    UIMSG_SpellBookWindow = 105,
    UIMSG_QuickReference = 106,
    UIMSG_GameMenuButton = 107,

    UIMSG_AlreadyResting = 109,
    UIMSG_SelectCharacter = 110,
    UIMSG_ChangeSoundVolume = 111,
    UIMSG_ChangeMusicVolume = 112,
    UIMSG_Escape = 113,
    UIMSG_ClickSkillsBtn = 114,
    UIMSG_ClickStatsBtn = 115,
    UIMSG_ClickInventoryBtn = 116,
    UIMSG_ClickAwardsBtn = 117,
    UIMSG_PlayerCreation_SelectAttribute = 118,

    UIMSG_InventoryLeftClick = 120,
    UIMSG_SkillUp = 121,
    UIMSG_7A = 122,
    UIMSG_GameMenu_ReturnToGame = 123,
    UIMSG_StartNewGame = 124,
    UIMSG_Game_OpenLoadGameDialog = 125,
    UIMSG_Game_OpenSaveGameDialog = 126,
    UIMSG_Game_OpenOptionsDialog = 127,
    UIMSG_80 = 128,

    UIMSG_SetGraphicsMode = 131,
    UIMSG_Quit = 132,
    UIMSG_ClickPaperdoll = 133,
    UIMSG_StartHireling1Dialogue = 134,
    UIMSG_StartHireling2Dialogue = 135,
    UIMSG_SelectNPCDialogueOption = 136,

    UIMSG_CastSpell_Monster_Improvement = 140,
    UIMSG_CastSpell_Character_Small_Improvement = 141,
    UIMSG_CastSpellFromBook = 142,
    UIMSG_HiredNPC_CastSpell = 143,
    UIMSG_PlayerCreation_VoicePrev = 144,
    UIMSG_PlayerCreation_VoiceNext = 145,
    UIMSG_SpellScrollUse = 146,

    UIMSG_StartNPCDialogue = 161,
    UIMSG_ArrowUp = 162,
    UIMSG_DownArrow = 163,
    UIMSG_SaveLoadBtn = 164,
    UIMSG_SelectLoadSlot = 165,
    UIMSG_Cancel = 166,
    UIMSG_ExitRest = 167,
    UIMSG_ClickExitCharacterWindowBtn = 168,
    UIMSG_ClickAwardsUpBtn = 169,
    UIMSG_ClickAwardsDownBtn = 170,
    UIMSG_PlayerCreation_FacePrev = 171,
    UIMSG_PlayerCreation_FaceNext = 172,
    UIMSG_AD = 173,
    UIMSG_DebugBlv2 = 174,
    UIMSG_ClickNPCTopic = 175,
    UIMSG_CycleCharacters = 176,
    UIMSG_OnCastLloydsBeacon = 177,
    UIMSG_LloydsBeacon_FlippingBtn = 178,
    UIMSG_InstallBeacon = 179,
    UIMSG_HintBeaconSlot = 180,
    UIMSG_CloseAfterInstallBeacon = 181,
    UIMSG_HintTownPortal = 182,
    UIMSG_ClickTownInTP = 183,
    UIMSG_SetTurnSpeed = 184,
    UIMSG_ToggleWalkSound = 185,
    UIMSG_ChangeVoiceVolume = 186,
    UIMSG_ToggleShowDamage = 187,
    UIMSG_ScrollNPCPanel = 188,
    UIMSG_BD = 189,
    UIMSG_CastSpell_Telekinesis = 190,
    UIMSG_BF = 191,
    UIMSG_ClickAwardScrollBar = 192,
    UIMSG_C1 = 192,
    UIMSG_C2 = 192,

    UIMSG_OnCastTownPortal = 195,
    UIMSG_OnFinalWindowClose = 196,
    UIMSG_ShowFinalWindow = 197,
    UIMSG_C6 = 198,
    UIMSG_C7 = 199,
    UIMSG_OpenQuestBook = 200,
    UIMSG_OpenAutonotes = 201,
    UIMSG_OpenMapBook = 202,
    UIMSG_OpenCalendar = 203,
    UIMSG_CC = 204,
    UIMSG_CD = 205,
    UIMSG_CE = 206,
    UIMSG_CF = 207,
    UIMSG_D0 = 208,
    UIMSG_D1 = 209,
    UIMSG_D2 = 210,
    UIMSG_D3 = 211,
    UIMSG_D4 = 212,
    UIMSG_D5 = 213,
    UIMSG_D6 = 214,

    UIMSG_DD = 221,

    UIMSG_OpenHistoryBook = 224,
    UIMSG_ToggleAlwaysRun = 225,
    UIMSG_ToggleFlipOnExit = 226,

    UIMSG_ClickZoomOutBtn = 367,
    UIMSG_ClickZoomInBtn = 368,

    UIMSG_Game_Action = 404,
    UIMSG_SelectShopDialogueOption = 405,

    UIMSG_RentRoom = 409,
    UIMSG_ClickHouseNPCPortrait = 410,
    UIMSG_TransitionUI_Confirm = 411,
    UIMSG_TransitionWindowCloseBtn = 412,

    UIMSG_OpenKeyMappingOptions = 415,
    UIMSG_SelectKeyPage1 = 416,
    UIMSG_SelectKeyPage2 = 417,
    UIMSG_ResetKeyMapping = 418,
    UIMSG_ChangeKeyButton = 419,

    UIMSG_OpenVideoOptions = 421,
    UIMSG_ToggleBloodsplats = 422,
    UIMSG_ToggleColoredLights = 423,
    UIMSG_ToggleTint = 424,
    UIMSG_1A9 = 425,

    UIMSG_MMT_MainMenu_MM6 = 426,
    UIMSG_MMT_MainMenu_MM7 = 427,
    UIMSG_MMT_MainMenu_MM8 = 428,
    UIMSG_MMT_MainMenu_Continue = 429,

    UIMSG_DebugSpecialItem = 971,
    UIMSG_DebugGenItem = 972,
    UIMSG_DebugFarClip = 973,
    UIMSG_DebugSeasonsChange = 974,
    UIMSG_DebugShowFPS = 975,
    UIMSG_DebugPickedFace = 976,
    UIMSG_DebugPortalLines = 977,
    UIMSG_DebugKillChar = 978,
    UIMSG_DebugEradicate = 979,
    UIMSG_DebugNoDamage = 980,
    UIMSG_DebugFullHeal = 981,
    UIMSG_DebugSnow = 982,
    UIMSG_DebugDrawDist = 983,
    UIMSG_DebugNoActors = 984,
    UIMSG_DebugTurboSpeed = 985,
    UIMSG_DebugLightmap = 986,
    UIMSG_DebugTerrain = 987,
    UIMSG_DebugAllMagic = 988,
    UIMSG_DebugWizardEye = 989,
    UIMSG_DebugCycleAlign = 990,
    UIMSG_DebugTakeFood = 991,
    UIMSG_DebugGiveFood = 992,
    UIMSG_DebugTakeGold = 993,
    UIMSG_DebugLearnSkills = 994,
    UIMSG_DebugGiveSkillP = 995,
    UIMSG_DebugGiveEXP = 996,
    UIMSG_DebugGiveGold = 997,
    UIMSG_DebugTownPortal = 998,
    UIMSG_OpenDebugMenu = 999,
};

/*  251 */
enum MENU_STATE : __int32 {
    MENU_MAIN = 0,
    MENU_NEWGAME = 1,
    MENU_CREDITS = 2,
    MENU_SAVELOAD = 3,
    MENU_EXIT_GAME = 4,
    MENU_5 = 5,
    MENU_CREATEPARTY = 6,
    MENU_NAMEPANELESC = 7,
    MENU_CREDITSPROC = 8,
    MENU_LoadingProcInMainMenu = 9,
    MENU_DebugBLVLevel = 10,
    MENU_CREDITSCLOSE = 11,
    MENU_MMT_MAIN_MENU = 12,
};

enum DIALOGUE_TYPE {
    DIALOGUE_USE_NPC_ABILITY = 9,
    DIALOGUE_13 = 0xD,
    DIALOGUE_18 = 18,
    DIALOGUE_EVT_A = 19,
    DIALOGUE_EVT_B = 20,
    DIALOGUE_EVT_C = 21,
    DIALOGUE_EVT_D = 22,
    DIALOGUE_EVT_E = 23,
    DIALOGUE_EVT_F = 0x18,
    DIALOGUE_76 = 76,
    DIALOGUE_PROFESSION_DETAILS = 77,
    DIALOGUE_SKILL_TRAINER = 78,
    DIALOGUE_84 = 84,
    DIALOGUE_ARENA_SELECT_PAGE = 85,
    DIALOGUE_ARENA_SELECT_SQUIRE = 86,
    DIALOGUE_ARENA_SELECT_KNIGHT = 87,
    DIALOGUE_ARENA_SELECT_CHAMPION = 88,
    DIALOGUE_ARENA_WELCOME = 89,
    DIALOGUE_ARENA_FIGHT_NOT_OVER_YET = 90,
    DIALOGUE_ARENA_REWARD = 91,
    DIALOGUE_ARENA_ALREADY_WON = 92,
};

enum WindowType {
    WINDOW_null = 0,
    WINDOW_MainMenu = 1,
    WINDOW_GameMenu = 3,
    WINDOW_CharacterRecord = 4,
    WINDOW_Options = 6,
    WINDOW_8 = 8,
    WINDOW_Book = 9,
    WINDOW_Dialogue = 10,
    WINDOW_QuickReference = 12,
    WINDOW_F = 15,
    WINDOW_Rest = 16,
    WINDOW_ChangeLocation = 17,
    WINDOW_SpellBook = 18,
    WINDOW_GreetingNPC = 19,
    WINDOW_Chest = 20,
    WINDOW_22 = 0x16,
    WINDOW_SaveLoadButtons = 23,
    WINDOW_MainMenu_Load = 0x18,
    WINDOW_HouseInterior = 0x19,
    WINDOW_Transition = 26,
    WINDOW_CastSpell = 27,  // OnCastTargetedSpell
    WINDOW_Scroll = 0x1E,
    WINDOW_CastSpell_InInventory = 31,
    WINDOW_ModalWindow = 70,
    WINDOW_50 = 80,  // Debug
    WINDOW_59 = 89,  // Debug: Item Generation Window
    WINDOW_PressedButton2 = 90,      // OnButtonClick2?
    WINDOW_CharactersPressedButton = 91,
    WINDOW_PressedButton = 92,
    WINDOW_5D = 93,
    WINDOW_SaveLoadBtn = 94,
    WINDOW_LoadGame_CancelBtn = 95,  // OnCancel3?
    WINDOW_CloseRestWindowBtn = 96,
    WINDOW_ExitCharacterWindow = 97,  // OnCancel2
    WINDOW_RestWindow = 0x62,
    WINDOW_BooksButtonOverlay = 99,
    WINDOW_CharacterWindow_Stats = 0x64,
    WINDOW_CharacterWindow_Skills = 0x65,
    WINDOW_CharacterWindow_Awards = 0x66,
    WINDOW_CharacterWindow_Inventory = 0x67,
    WINDOW_GameOptions = 104,
    WINDOW_KeyMappingOptions = 0x69,
    WINDOW_VideoOptions = 0x6A,
    WINDOW_LloydsBeacon = 177,
    WINDOW_TownPortal = 195,
    WINDOW_QuestBook = 200,
    WINDOW_AutonotesBook = 0xC9,
    WINDOW_MapsBook = 0xCA,
    WINDOW_CalendarBook = 0xCB,
    WINDOW_JournalBook = 0xE0,

    WINDOW_Unknown,              // i wasnt able to find real value for those
    WINDOW_CharacterCreation,    // new addition, because i wasnt able to find real value for this
    WINDOW_CharacterCreationBtn,  // new addition, because i wasnt able to find real value for this
    WINDOW_GenericCancel,        // new addition, because i wasnt able to find real value for this
    WINDOW_GameUI,               // new addition, because i wasnt able to find real value for this
    WINDOW_Credits,              // new addition, because i wasnt able to find real value for this
    WINDOW_Save,                 // new addition, because i wasnt able to find real value for this
    WINDOW_Load,                 // new addition, because i wasnt able to find real value for this
    WINDOW_DebugMenu,

    // =======
    // ToString updated until here
};

#define ENUM_TO_STRING_CASE(val) case val: return #val;
inline const char* ToString(WindowType windowType) {
    switch (windowType) {
        ENUM_TO_STRING_CASE(WINDOW_null)
        ENUM_TO_STRING_CASE(WINDOW_MainMenu)
        ENUM_TO_STRING_CASE(WINDOW_GameMenu)
        ENUM_TO_STRING_CASE(WINDOW_CharacterRecord)
        ENUM_TO_STRING_CASE(WINDOW_Options)
        ENUM_TO_STRING_CASE(WINDOW_8)
        ENUM_TO_STRING_CASE(WINDOW_Book)
        ENUM_TO_STRING_CASE(WINDOW_Dialogue)
        ENUM_TO_STRING_CASE(WINDOW_QuickReference)
        ENUM_TO_STRING_CASE(WINDOW_F)
        ENUM_TO_STRING_CASE(WINDOW_Rest)
        ENUM_TO_STRING_CASE(WINDOW_ChangeLocation)
        ENUM_TO_STRING_CASE(WINDOW_SpellBook)
        ENUM_TO_STRING_CASE(WINDOW_GreetingNPC)
        ENUM_TO_STRING_CASE(WINDOW_Chest)
        ENUM_TO_STRING_CASE(WINDOW_22)
        ENUM_TO_STRING_CASE(WINDOW_SaveLoadButtons)
        ENUM_TO_STRING_CASE(WINDOW_MainMenu_Load)
        ENUM_TO_STRING_CASE(WINDOW_HouseInterior)
        ENUM_TO_STRING_CASE(WINDOW_Transition)
        ENUM_TO_STRING_CASE(WINDOW_CastSpell)
        ENUM_TO_STRING_CASE(WINDOW_Scroll)
        ENUM_TO_STRING_CASE(WINDOW_CastSpell_InInventory)
        ENUM_TO_STRING_CASE(WINDOW_ModalWindow)
        ENUM_TO_STRING_CASE(WINDOW_50)
        ENUM_TO_STRING_CASE(WINDOW_59)
        ENUM_TO_STRING_CASE(WINDOW_PressedButton2)
        ENUM_TO_STRING_CASE(WINDOW_CharactersPressedButton)
        ENUM_TO_STRING_CASE(WINDOW_PressedButton)
        ENUM_TO_STRING_CASE(WINDOW_5D)
        ENUM_TO_STRING_CASE(WINDOW_SaveLoadBtn)
        ENUM_TO_STRING_CASE(WINDOW_LoadGame_CancelBtn)
        ENUM_TO_STRING_CASE(WINDOW_CloseRestWindowBtn)
        ENUM_TO_STRING_CASE(WINDOW_ExitCharacterWindow)
        ENUM_TO_STRING_CASE(WINDOW_RestWindow)
        ENUM_TO_STRING_CASE(WINDOW_BooksButtonOverlay)
        ENUM_TO_STRING_CASE(WINDOW_CharacterWindow_Stats)
        ENUM_TO_STRING_CASE(WINDOW_CharacterWindow_Skills)
        ENUM_TO_STRING_CASE(WINDOW_CharacterWindow_Awards)
        ENUM_TO_STRING_CASE(WINDOW_CharacterWindow_Inventory)
        ENUM_TO_STRING_CASE(WINDOW_GameOptions)
        ENUM_TO_STRING_CASE(WINDOW_KeyMappingOptions)
        ENUM_TO_STRING_CASE(WINDOW_VideoOptions)
        ENUM_TO_STRING_CASE(WINDOW_LloydsBeacon)
        ENUM_TO_STRING_CASE(WINDOW_TownPortal)
        ENUM_TO_STRING_CASE(WINDOW_QuestBook)
        ENUM_TO_STRING_CASE(WINDOW_AutonotesBook)
        ENUM_TO_STRING_CASE(WINDOW_MapsBook)
        ENUM_TO_STRING_CASE(WINDOW_CalendarBook)
        ENUM_TO_STRING_CASE(WINDOW_JournalBook)
        ENUM_TO_STRING_CASE(WINDOW_Unknown)
        ENUM_TO_STRING_CASE(WINDOW_CharacterCreation)
        ENUM_TO_STRING_CASE(WINDOW_CharacterCreationBtn)
        ENUM_TO_STRING_CASE(WINDOW_GenericCancel)
        ENUM_TO_STRING_CASE(WINDOW_GameUI)
        ENUM_TO_STRING_CASE(WINDOW_Credits)
        ENUM_TO_STRING_CASE(WINDOW_Save)
        ENUM_TO_STRING_CASE(WINDOW_Load)
        ENUM_TO_STRING_CASE(WINDOW_DebugMenu)
    }

    return nullptr;
}

struct Texture_MM7;

#define WINDOW_INPUT_NONE 0
#define WINDOW_INPUT_IN_PROGRESS 1
#define WINDOW_INPUT_CONFIRMED 2
#define WINDOW_INPUT_CANCELLED 3

class GUIFont;
class GUIButton;

class GUIWindow {
 public:
    GUIWindow();
    GUIWindow(WindowType windowType, unsigned int uX, unsigned int uY, unsigned int uWidth, unsigned int uHeight, GUIButton *pButton, const String &hint = String());
    virtual ~GUIWindow() {}

    GUIButton *CreateButton(int x, int y, int width, int height, int a6, int a7,
    UIMessageType msg, unsigned int msg_param, uint8_t hotkey, const String &label,
    const std::vector<Image*> &textures = std::vector<Image*>());

    bool Contains(unsigned int x, unsigned int y);
    void DrawFlashingInputCursor(int uX, int uY, GUIFont *a2);

    int DrawTextInRect(GUIFont *font, unsigned int x, unsigned int y, unsigned int color, const char *text, int rect_width, int reverse_text);
    int DrawTextInRect(GUIFont *font, unsigned int x, unsigned int y, unsigned int color, String &str, int rect_width, int reverse_text);

    void DrawText(GUIFont *font, int x, int y, unsigned short uFontColor, const char *str, bool present_time_transparency = false, int max_text_height = 0, int uFontShadowColor = 0);
    void DrawText(GUIFont *font, int x, int y, unsigned short uFontColor, const String &str, bool present_time_transparency = false, int max_text_height = 0, int uFontShadowColor = 0);

    void DrawTitleText(GUIFont *font, int horizontal_margin, int vertical_margin, unsigned __int16 uDefaultColor, const char *pInString, int line_spacing);
    void DrawTitleText(GUIFont *font, int horizontal_margin, int vertical_margin, unsigned __int16 uDefaultColor, const String &str, int line_spacing);

    void DrawShops_next_generation_time_string(GameTime time);
    void HouseDialogManager();
    void DrawMessageBox(bool inside_game_viewport);
    GUIButton *GetControl(unsigned int uID);
    void _41D08F_set_keyboard_control_group(int num_buttons, int a3, int a4, int a5);
    void _41D73D_draw_buff_tooltip();

    virtual void Update() {}
    virtual void Release();
    void DeleteButtons();

    static void InitializeGUI();

    unsigned int uFrameX;
    unsigned int uFrameY;
    unsigned int uFrameWidth;
    unsigned int uFrameHeight;
    unsigned int uFrameZ;
    unsigned int uFrameW;
    WindowType eWindowType;
    union {
        void *ptr_1C;       // sometimes BuildID_2Events - book open, sometimes Button
        unsigned int par1C;  // in shops and houses - int parameter
                            // sometimes WindowType
    };
    int field_24;
    int pNumPresenceButton;
    int pCurrentPosActiveItem;
    int field_30;
    int field_34;
    int pStartingPosActiveItem;
    int receives_keyboard_input_2;  // 0  no input   1 currently typing   2 enter pressed   3 escape pressed
    bool receives_keyboard_input;
    String sHint;
    std::vector<GUIButton*> vButtons;

    Mouse *mouse = nullptr;
    Log *log = nullptr;
};

class GUIWindow_Scroll : public GUIWindow {
 public:
    GUIWindow_Scroll(unsigned int x, unsigned int y, unsigned int width, unsigned int height, GUIButton *button, const String &hint = String()) :
        GUIWindow(WINDOW_Scroll, x, y, width, height, button, hint) {
        CreateButton(61, 424, 31, 0, 2, 94, UIMSG_SelectCharacter, 1, '1', "");
        CreateButton(177, 424, 31, 0, 2, 94, UIMSG_SelectCharacter, 2, '2', "");
        CreateButton(292, 424, 31, 0, 2, 94, UIMSG_SelectCharacter, 3, '3', "");
        CreateButton(407, 424, 31, 0, 2, 94, UIMSG_SelectCharacter, 4, '4', "");
        CreateButton(0, 0, 0, 0, 1, 0, UIMSG_CycleCharacters, 0, '\t', "");
    }
    virtual ~GUIWindow_Scroll() {}

    virtual void Update();
};

class OnButtonClick : public GUIWindow {
 public:
    OnButtonClick(unsigned int x, unsigned int y, unsigned int width, unsigned int height, GUIButton *button, const String &hint = String(), bool play_sound = true) :
        GUIWindow(WINDOW_CharacterCreationBtn, x, y, width, height, button, hint),
        bPlaySound(play_sound)
    {}
    virtual ~OnButtonClick() {}

    virtual void Update();

    bool bPlaySound;
};

class OnButtonClick2 : public GUIWindow {
 public:
    OnButtonClick2(unsigned int x, unsigned int y, unsigned int width, unsigned int height, GUIButton *button, const String &hint = String(), bool play_sound = true) :
        GUIWindow(WINDOW_PressedButton2, x, y, width, height, button, hint),
        bPlaySound(play_sound)
    {}
    virtual ~OnButtonClick2() {}

    virtual void Update();

    bool bPlaySound;
};

class OnButtonClick3 : public GUIWindow {
 public:
    OnButtonClick3(WindowType windowType, unsigned int x, unsigned int y, unsigned int width, unsigned int height, GUIButton *button, const String &hint = String()) :
        GUIWindow(windowType, x, y, width, height, button, hint) {
    }

    virtual ~OnButtonClick3() {}

    virtual void Update();
};

// something debug? not really sure, unused
class OnButtonClick4 : public GUIWindow {
 public:
    OnButtonClick4(unsigned int x, unsigned int y, unsigned int width, unsigned int height, GUIButton *button, const String &hint = String()) :
        GUIWindow(WINDOW_59, x, y, width, height, button, hint)
    {}
    virtual ~OnButtonClick4() {}

    virtual void Update();
};

class OnSaveLoad : public GUIWindow {
 public:
    OnSaveLoad(unsigned int x, unsigned int y, unsigned int width, unsigned int height, GUIButton *button, const String &hint = String()) :
        GUIWindow(WINDOW_SaveLoadBtn, x, y, width, height, button, hint)
    {}
    virtual ~OnSaveLoad() {}

    virtual void Update();
};

class OnCancel : public GUIWindow {
 public:
    OnCancel(unsigned int x, unsigned int y, unsigned int width, unsigned int height, GUIButton *button, const String &hint = String()) :
        GUIWindow(WINDOW_GenericCancel, x, y, width, height, button, hint)
    {}
    virtual ~OnCancel() {}

    virtual void Update();
};

class OnCancel2 : public GUIWindow {
 public:
    OnCancel2(unsigned int x, unsigned int y, unsigned int width, unsigned int height, GUIButton *button, const String &hint = String()) :
        GUIWindow(WINDOW_ExitCharacterWindow, x, y, width, height, button, hint)
    {}
    virtual ~OnCancel2() {}

    virtual void Update();
};

class OnCancel3 : public GUIWindow {
 public:
    OnCancel3(unsigned int x, unsigned int y, unsigned int width, unsigned int height, GUIButton *button, const String &hint = String()) :
        GUIWindow(WINDOW_LoadGame_CancelBtn, x, y, width, height, button, hint)
    {}
    virtual ~OnCancel3() {}

    virtual void Update();
};

class OnCastTargetedSpell : public GUIWindow {
 public:
    OnCastTargetedSpell(unsigned int x, unsigned int y, unsigned int width, unsigned int height, GUIButton *button, const String &hint = String());
    virtual ~OnCastTargetedSpell() {}
};

enum class CURRENT_SCREEN {
    SCREEN_GAME = 0,
    SCREEN_MENU = 1,
    SCREEN_OPTIONS = 2,
    SCREEN_BOOKS = 3,
    SCREEN_NPC_DIALOGUE = 4,
    SCREEN_REST = 5,
    SCREEN_6 = 6,
    SCREEN_CHARACTERS = 7,
    SCREEN_SPELL_BOOK = 8,
    SCREEN_CREATORS = 9,
    SCREEN_CHEST = 10,
    SCREEN_SAVEGAME = 11,
    SCREEN_LOADGAME = 12,
    SCREEN_HOUSE = 13,
    SCREEN_E = 14,
    SCREEN_CHEST_INVENTORY = 15,
    SCREEN_VIDEO = 16,
    SCREEN_CHANGE_LOCATION = 17,
    SCREEN_INPUT_BLV = 18,
    SCREEN_BRANCHLESS_NPC_DIALOG = 19,
    SCREEN_20 = 20,
    SCREEN_PARTY_CREATION = 21,
    SCREEN_MODAL_WINDOW = 22,
    SCREEN_CASTING = 23,
    SCREEN_24 = 24,
    SCREEN_19 = 25,
    SCREEN_KEYBOARD_OPTIONS = 26,
    SCREEN_1B = 27,
    SCREEN_VIDEO_OPTIONS = 28,

    SCREEN_63 = 0x63,
    SCREEN_64 = 0x64,
    SCREEN_67 = 0x67,
    SCREEN_QUICK_REFERENCE = 0x68,

    SCREEN_DEBUG = 999,
};

struct GUIMessage {
    enum UIMessageType eType;
    int param;
    int field_8;
    String file;
    unsigned int line;
};

#define AddGUIMessage(msg, param, a4) \
    AddMessageImpl((msg), (param), (a4), __FILE__, __LINE__)

struct GUIMessageQueue {
    GUIMessageQueue() {}

    void Flush();
    void Clear();
    bool Empty() { return qMessages.empty(); }
    void PopMessage(UIMessageType *pMsg, int *pParam, int *a4);
    void AddMessageImpl(UIMessageType msg, int param, unsigned int a4,
                        const char *file = nullptr, int line = 0);

    std::queue<GUIMessage> qMessages;
};

extern struct GUIMessageQueue *pMessageQueue_50CBD0;  // idb

extern struct GUIMessageQueue *pMessageQueue_50C9E8;  // idb

extern enum WindowType current_character_screen_window;
extern std::list<GUIWindow*> lWindowList;

class WindowManager {
 public:
    WindowManager() {}
    virtual ~WindowManager() {}

    void DeleteAllVisibleWindows();
};

extern WindowManager windowManager;

void draw_leather();

// main menu ui
void MainMenuUI_LoadFontsAndSomeStuff();
void UI_Create();

// game ui
void GameUI_DrawRightPanel();
void GameUI_DrawRightPanelFrames();
void GameUI_DrawRightPanelItems();
void GameUI_DrawFoodAndGold();
void GameUI_DrawLifeManaBars();
void GameUI_DrawHiredNPCs();
void GameUI_DrawPortraits(unsigned int _this);
void GameUI_DrawMinimap(unsigned int uX, unsigned int uY, unsigned int uZ,
                        unsigned int uW, unsigned int uZoom,
                        unsigned int bRedrawOdmMinimap);
String GameUI_GetMinimapHintText();
void GameUI_DrawPartySpells();
void GameUI_DrawTorchlightAndWizardEye();
void GameUI_DrawCharacterSelectionFrame();
void GameUI_CharacterQuickRecord_Draw(GUIWindow *window, Player *player);
void GameUI_DrawNPCPopup(void *_this);

// character ui
String CharacterUI_GetSkillDescText(unsigned int uPlayerID,
                                    PLAYER_SKILL_TYPE uPlayerSkillType);
void CharacterUI_SkillsTab_ShowHint();
void CharacterUI_StatsTab_ShowHint();
void CharacterUI_InventoryTab_Draw(Player *player, bool a2);
void CharacterUI_DrawPaperdoll(Player *player);
void CharacterUI_DrawPaperdollWithRingOverlay(Player *player);
void CharacterUI_ReleaseButtons();

unsigned int GetSkillColor(unsigned int uPlayerClass,
                           PLAYER_SKILL_TYPE uPlayerSkillType,
                           signed int skill_level);

void DrawSpellDescriptionPopup(int spell_index);

void UI_OnMouseRightClick(int mouse_x, int mouse_y);

void DrawPopupWindow(unsigned int uX, unsigned int uY, unsigned int uWidth,
                     unsigned int uHeight);  // idb
void DrawMM7CopyrightWindow();
void GUI_UpdateWindows();
int GetConditionDrawColor(unsigned int uConditionIdx);  // idb
void FillAwardsData();
void CreateAwardsScrollBar();
void ReleaseAwardsScrollBar();
void Inventory_ItemPopupAndAlchemy();
unsigned int UI_GetHealthManaAndOtherQualitiesStringColor(int current_pos,
                                                          int base_pos);
unsigned int GetSizeInInventorySlots(unsigned int uNumPixels);
class GUIButton *GUI_HandleHotkey(uint8_t uHotkey);   // idb
void GUI_ReplaceHotkey(uint8_t uOldHotkey, uint8_t uNewHotkey, char bFirstCall);
void DrawBuff_remaining_time_string(int uY, GUIWindow *window,
                                    GameTime remaining_time, GUIFont *Font);
void GameUI_DrawItemInfo(struct ItemGen* inspect_item);   // idb
void MonsterPopup_Draw(unsigned int uActorID, GUIWindow *window);
void SetUserInterface(enum PartyAlignment alignment, bool bReplace);
void CreateMsgScrollWindow(signed int mscroll_id);
void free_book_subwindow();
void CreateScrollWindow();
void OnPaperdollLeftClick();
void DrawJoinGuildWindow(int pEventCode);
void DialogueEnding();
char sub_4637E0_is_there_popup_onscreen();
void ClickNPCTopic(int uMessageParam);
void _4B3FE5_training_dialogue(int a4);
void OracleDialogue();
void CheckBountyRespawnAndAward();
String _4B254D_SkillMasteryTeacher(int trainerInfo);
String BuildDialogueString(const char *lpsz, unsigned __int8 uPlayerID,
                           struct ItemGen *a3, char *a4, int a5,
                           GameTime *a6 = nullptr);
String BuildDialogueString(String &str, unsigned __int8 uPlayerID,
                           struct ItemGen *a3, char *a4, int shop_screen,
                           GameTime *a6 = nullptr);
int const_2();

extern GUIWindow *pPrimaryWindow;
extern GUIWindow *pChestWindow;
extern GUIWindow *pDialogueWindow;
extern GUIWindow *window_SpeakInHouse;
extern GUIWindow *pGUIWindow_ScrollWindow;
extern GUIWindow *ptr_507BC8;
extern GUIWindow *pGUIWindow_CurrentMenu;
extern GUIWindow *ptr_507BD0;
extern GUIWindow *pGUIWindow_CastTargetedSpell;
extern GUIWindow *pModalWindow;
extern GUIWindow *pGUIWindow_EscMessageWindow;
extern GUIWindow *pBooksButtonOverlay;
extern GUIWindow *pGUIWindow2;

extern unsigned int ui_mainmenu_copyright_color;
extern unsigned int ui_character_tooltip_header_default_color;
extern unsigned int ui_character_default_text_color;
extern unsigned int ui_character_skill_highlight_color;
extern unsigned int ui_character_header_text_color;
extern unsigned int ui_character_bonus_text_color;
extern unsigned int ui_character_bonus_text_color_neg;
extern unsigned int ui_character_skill_upgradeable_color;
extern unsigned int ui_character_skill_default_color;
extern unsigned int ui_character_stat_default_color;
extern unsigned int ui_character_stat_buffed_color;
extern unsigned int ui_character_stat_debuffed_color;
extern unsigned int ui_character_skillinfo_can_learn;
extern unsigned int ui_character_skillinfo_can_learn_gm;
extern unsigned int ui_character_skillinfo_cant_learn;
extern unsigned int ui_character_condition_normal_color;
extern unsigned int ui_character_condition_light_color;
extern unsigned int ui_character_condition_moderate_color;
extern unsigned int ui_character_condition_severe_color;
extern std::array<unsigned int, 6> ui_character_award_color;
extern unsigned int ui_game_minimap_outline_color;
extern unsigned int ui_game_minimap_actor_friendly_color;
extern unsigned int ui_game_minimap_actor_hostile_color;
extern unsigned int ui_game_minimap_actor_corpse_color;
extern unsigned int ui_game_minimap_decoration_color_1;
extern unsigned int ui_game_minimap_projectile_color;
extern unsigned int ui_game_minimap_treasure_color;
extern std::array<unsigned int, 24> ui_game_character_record_playerbuff_colors;
extern unsigned int ui_gamemenu_video_gamma_title_color;
extern unsigned int ui_gamemenu_keys_action_name_color;
extern unsigned int ui_gamemenu_keys_key_selection_blink_color_1;
extern unsigned int ui_gamemenu_keys_key_selection_blink_color_2;
extern unsigned int ui_gamemenu_keys_key_default_color;
extern unsigned int ui_book_quests_title_color;
extern unsigned int ui_book_quests_text_color;
extern unsigned int ui_book_autonotes_title_color;
extern unsigned int ui_book_autonotes_text_color;
extern unsigned int ui_book_map_title_color;
extern unsigned int ui_book_map_coordinates_color;
extern unsigned int ui_book_calendar_title_color;
extern unsigned int ui_book_calendar_time_color;
extern unsigned int ui_book_calendar_day_color;
extern unsigned int ui_book_calendar_month_color;
extern unsigned int ui_book_calendar_year_color;
extern unsigned int ui_book_calendar_moon_color;
extern unsigned int ui_book_calendar_location_color;
extern unsigned int ui_book_journal_title_color;
extern unsigned int ui_book_journal_text_color;
extern unsigned int ui_book_journal_text_shadow;
extern unsigned int ui_game_dialogue_npc_name_color;
extern unsigned int ui_game_dialogue_option_highlight_color;
extern unsigned int ui_game_dialogue_option_normal_color;
extern unsigned int ui_house_player_cant_interact_color;

class Image;
extern Image *ui_exit_cancel_button_background;
extern Image *game_ui_right_panel_frame;
extern Image *dialogue_ui_x_ok_u;
extern Image *dialogue_ui_x_x_u;

extern Image *ui_buttdesc2;
extern Image *ui_buttyes2;

extern Image *ui_btn_npc_right;
extern Image *ui_btn_npc_left;

extern Image *ui_ar_dn_dn;
extern Image *ui_ar_dn_up;
extern Image *ui_ar_up_dn;
extern Image *ui_ar_up_up;

extern Image *ui_leather_mm6;
extern Image *ui_leather_mm7;

extern MENU_STATE sCurrentMenuID;
void SetCurrentMenuID(MENU_STATE);
MENU_STATE GetCurrentMenuID();

extern enum CURRENT_SCREEN current_screen_type;
extern enum CURRENT_SCREEN prev_screen_type;
