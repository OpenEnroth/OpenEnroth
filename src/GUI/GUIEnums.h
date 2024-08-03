#pragma once

#include <cstdint>

#include "Library/Serialization/SerializationFwd.h"

enum UIMessageType : uint32_t {
    UIMSG_0 = 0,

    UIMSG_ChangeGameState = 5,

    UIMSG_MouseLeftClickInGame = 10,

    UIMSG_CHEST_ClickItem = 12,

    UIMSG_MouseLeftClickInScreen = 14,
    UIMSG_F = 15,

    UIMSG_ChangeCursor = 17,

    UIMSG_OpenInventory = 20,

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
    UIMSG_CastSpell_TargetCharacter = 69,
    UIMSG_CastSpell_TargetActor = 70,
    UIMSG_ClickBooksBtn = 71,
    UIMSG_48 = 72,  // fixed skill 1
    UIMSG_49 = 73,  // fixed skill 2
    UIMSG_PlayerCreationRemoveUpSkill = 74,  // choice skill 1
    UIMSG_PlayerCreationRemoveDownSkill = 75,  // choice skill 2

    UIMSG_HintSelectRemoveQuickSpellBtn = 78,
    UIMSG_Spellbook_ShowHightlightedSpellInfo = 79,

    UIMSG_HouseScreenClick = 81,
    UIMSG_LoadGame = 82,
    UIMSG_SaveGame = 83,
    UIMSG_54 = 84,
    UIMSG_ChangeDetaliz = 85,
    UIMSG_SelectSpell = 86,
    UIMSG_OpenSpellbookPage = 87,
    UIMSG_ClickInstallRemoveQuickSpellBtn = 88,

    UIMSG_OnTravelByFoot = 90,
    UIMSG_CancelTravelByFoot = 91,
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

    UIMSG_WaitTillDawn = 109,
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

    UIMSG_CastSpell_TargetActorBuff = 140,
    //UIMSG_CastSpell_TargetCharacter = 141, // semantics duplicates UIMSG 69
    UIMSG_CastSpellFromBook = 142,
    UIMSG_CastSpell_Hireling = 143,
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
    UIMSG_SelectHouseNPCDialogueOption = 175,
    UIMSG_CycleCharacters = 176,
    UIMSG_OnCastLloydsBeacon = 177,
    UIMSG_LloydBookFlipButton = 178,
    UIMSG_InstallOrRecallBeacon = 179,
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
    UIMSG_HouseTransitionConfirmation = 191,
    UIMSG_ClickAwardScrollBar = 192,
    UIMSG_C1 = 193,
    UIMSG_C2 = 194,

    UIMSG_OnCastTownPortal = 195,
    UIMSG_OnGameOverWindowClose = 196,
    UIMSG_ShowGameOverWindow = 197,
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
    UIMSG_SelectProprietorDialogueOption = 405,

    UIMSG_RentRoom = 409,
    UIMSG_ClickHouseNPCPortrait = 410,
    UIMSG_OnIndoorEntryExit = 411,
    UIMSG_CancelIndoorEntryExit = 412,

    UIMSG_OpenKeyMappingOptions = 415,
    UIMSG_SelectKeyPage1 = 416,
    UIMSG_SelectKeyPage2 = 417,
    UIMSG_ResetKeyMapping = 418,
    UIMSG_ChangeKeyButton = 419,

    UIMSG_OpenVideoOptions = 421,
    UIMSG_ToggleBloodsplats = 422,
    UIMSG_ToggleColoredLights = 423,
    UIMSG_ToggleTint = 424,
    UIMSG_ChangeGammaLevel = 425,

    UIMSG_MMT_MainMenu_MM6 = 426,
    UIMSG_MMT_MainMenu_MM7 = 427,
    UIMSG_MMT_MainMenu_MM8 = 428,
    UIMSG_MMT_MainMenu_Continue = 429,

    UIMSG_SaveLoadScroll = 970,

    UIMSG_DebugReloadShader = 1000,

    UIMSG_QuickSave = 2000,
    UIMSG_QuickLoad = 2001,

    UIMSG_Invalid = 0xffffffff
};

enum class MenuType {
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
    MENU_DebugBLVLevel = 10, // Was used to open a debug blv level in vanilla, not used in OE.
    MENU_CREDITSCLOSE = 11,
    MENU_MMT_MAIN_MENU = 12,
    MENU_NONE = -1,
};
using enum MenuType;

// TODO(captainurist): check against this:
//  https://github.com/GrayFace/MMExtension/blob/4d6600f164315f38157591d7f0307a86594c22ef/Scripts/Core/ConstAndBits.lua#L875
enum class ScreenType {
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
    SCREEN_SHOP_INVENTORY = 14,
    SCREEN_CHEST_INVENTORY = 15,
    SCREEN_VIDEO = 16,
    SCREEN_CHANGE_LOCATION = 17,
    SCREEN_INPUT_BLV = 18,
    SCREEN_BRANCHLESS_NPC_DIALOG = 19,
    SCREEN_20 = 20,
    SCREEN_PARTY_CREATION = 21,
    SCREEN_GAMEOVER_WINDOW = 22,
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
using enum ScreenType;

// TODO(captainurist): #enum class
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
    WINDOW_Travel = 17,
    WINDOW_SpellBook = 18,
    WINDOW_GreetingNPC = 19,
    WINDOW_Chest = 20,
    WINDOW_22 = 0x16,
    WINDOW_SaveLoadButtons = 23,
    WINDOW_MainMenu_Load = 0x18,
    WINDOW_HouseInterior = 0x19,
    WINDOW_IndoorEntryExit = 26,
    WINDOW_CastSpell = 27,  // OnCastTargetedSpell
    WINDOW_Scroll = 0x1E,
    WINDOW_CastSpell_InInventory = 31,
    WINDOW_GameOverWindow = 70,
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
    WINDOW_Unknown,               // i wasnt able to find real value for those
    WINDOW_CharacterCreation,     // new addition, because i wasnt able to find real value for this
    WINDOW_CharacterCreationBtn,  // new addition, because i wasnt able to find real value for this
    WINDOW_GenericCancel,         // new addition, because i wasnt able to find real value for this
    WINDOW_GameUI,                // new addition, because i wasnt able to find real value for this
    WINDOW_Credits,               // new addition, because i wasnt able to find real value for this
    WINDOW_Save,                  // new addition, because i wasnt able to find real value for this
    WINDOW_Load,                  // new addition, because i wasnt able to find real value for this
    // =======
    // ToString (below) is updated until this point
};

MM_DECLARE_SERIALIZATION_FUNCTIONS(WindowType)

enum class ShopScreen {
    SHOP_SCREEN_INVALID = 0,
    SHOP_SCREEN_BUY = 2,
    SHOP_SCREEN_SELL = 3,
    SHOP_SCREEN_IDENTIFY = 4,
    SHOP_SCREEN_REPAIR = 5,
    SHOP_SCREEN_SELL_FOR_CHEAP = 6, // Sell for half the price, was used in MM6 general store, not used in MM7.
};
using enum ShopScreen;
