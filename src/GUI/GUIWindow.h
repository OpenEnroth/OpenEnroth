#pragma once
#include <array>
#include <cstdint>
#include <list>
#include <memory>
#include <queue>
#include <vector>
#include <string>

#include "Engine/Objects/Player.h"
#include "Engine/Party.h"

#include "GUI/GUIEnums.h"
#include "GUI/GUIDialogues.h"

#include "Platform/PlatformEnums.h"
#include "Io/Mouse.h"
#include "Io/InputAction.h"
#include "Io/KeyboardInputHandler.h"

#include "Library/Serialization/SerializationFwd.h"

#include "Utility/Geometry/Size.h"

using Io::Mouse;
using Io::InputAction;

struct Texture_MM7;

class GUIFont;
class GUIButton;
class GUIWindow_House;

struct WindowData {
    WindowData() {}
    WindowData(int value): val(value) {} // NOLINT: constructor is intentionally implicit
    WindowData(void *value): ptr(value) {} // NOLINT: constructor is intentionally implicit

    int val = 0;
    void *ptr = nullptr;
};

class GUIWindow {
 public:
    GUIWindow();
    GUIWindow(WindowType windowType, Pointi position, Sizei dimensions, WindowData wData, const std::string &hint = std::string());
    virtual ~GUIWindow() {}

    GUIButton *CreateButton(Pointi position, Sizei dimensions, int uButtonType, int uData,
                            UIMessageType msg, unsigned int msg_param, InputAction action = InputAction::Invalid, const std::string &label = {},
                            const std::vector<Image *> &textures = {});

    GUIButton *CreateButton(std::string id, Pointi position, Sizei dimensions, int uButtonType, int uData,
                            UIMessageType msg, unsigned int msg_param, InputAction action = InputAction::Invalid, const std::string &label = {},
                            const std::vector<Image *> &textures = {});

    bool Contains(unsigned int x, unsigned int y);
    void DrawFlashingInputCursor(int uX, int uY, GUIFont *a2);

    int DrawTextInRect(GUIFont *font, Pointi position, unsigned int color, const char *text, int rect_width, int reverse_text);
    int DrawTextInRect(GUIFont *font, Pointi position, unsigned int color, std::string &str, int rect_width, int reverse_text);

    void DrawText(GUIFont *font, Pointi position, uint16_t uFontColor, const char *str, bool present_time_transparency = false, int max_text_height = 0, int uFontShadowColor = 0);
    void DrawText(GUIFont *font, Pointi position, uint16_t uFontColor, const std::string &str, bool present_time_transparency = false, int max_text_height = 0, int uFontShadowColor = 0);

    void DrawTitleText(GUIFont *font, int horizontal_margin, int vertical_margin, uint16_t uDefaultColor, const char *pInString, int line_spacing);
    void DrawTitleText(GUIFont *font, int horizontal_margin, int vertical_margin, uint16_t uDefaultColor, const std::string &str, int line_spacing);

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
    void Init();

    int uFrameX;
    int uFrameY;
    int uFrameWidth;
    int uFrameHeight;
    int uFrameZ;
    int uFrameW;
    WindowType eWindowType;
    WindowData wData; // Window-specific
    int field_24;
    int pNumPresenceButton;
    int pCurrentPosActiveItem;
    int field_30;
    int field_34;
    int pStartingPosActiveItem;
    WindowInputStatus keyboard_input_status;
    bool receives_keyboard_input;
    std::string sHint;
    std::vector<GUIButton*> vButtons;

    std::shared_ptr<Mouse> mouse = nullptr;
    Logger *log = nullptr;
};

class GUIWindow_Scroll : public GUIWindow {
 public:
    GUIWindow_Scroll(Pointi position, Sizei dimensions, ITEM_TYPE scroll_type, const std::string &hint = std::string()) :
        GUIWindow(WINDOW_Scroll, position, dimensions, 0, hint) {
        Assert(isMessageScroll(scroll_type));

        this->scroll_type = scroll_type;
        CreateButton({61, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 1, InputAction::SelectChar1, "");
        CreateButton({177, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 2, InputAction::SelectChar2, "");
        CreateButton({292, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 3, InputAction::SelectChar3, "");
        CreateButton({407, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 4, InputAction::SelectChar4, "");
        CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_CycleCharacters, 0, InputAction::CharCycle, "");
    }
    virtual ~GUIWindow_Scroll() {}

    virtual void Update() override;

    ITEM_TYPE scroll_type = ITEM_NULL;
};

class OnButtonClick : public GUIWindow {
 public:
    OnButtonClick(Pointi position, Sizei dimensions, WindowData data, const std::string &hint = std::string(), bool play_sound = true) :
        GUIWindow(WINDOW_CharacterCreationBtn, position, dimensions, data, hint),
        bPlaySound(play_sound)
    {}
    virtual ~OnButtonClick() {}

    virtual void Update() override;

    bool bPlaySound;
};

class OnButtonClick2 : public GUIWindow {
 public:
    OnButtonClick2(Pointi position, Sizei dimensions, WindowData data, const std::string &hint = std::string(), bool play_sound = true) :
        GUIWindow(WINDOW_PressedButton2, position, dimensions, data, hint),
        bPlaySound(play_sound)
    {}
    virtual ~OnButtonClick2() {}

    virtual void Update() override;

    bool bPlaySound;
};

class OnButtonClick3 : public GUIWindow {
 public:
    OnButtonClick3(WindowType windowType, Pointi position, Sizei dimensions, WindowData data, const std::string &hint = std::string())
        : GUIWindow(windowType, position, dimensions, data, hint) {
    }

    virtual ~OnButtonClick3() {}

    virtual void Update() override;
};

// something debug? not really sure, unused
class OnButtonClick4 : public GUIWindow {
 public:
    OnButtonClick4(Pointi position, Sizei dimensions, WindowData data, const std::string &hint = std::string()) :
        GUIWindow(WINDOW_59, position, dimensions, data, hint)
    {}
    virtual ~OnButtonClick4() {}

    virtual void Update() override;
};

class OnSaveLoad : public GUIWindow {
 public:
    OnSaveLoad(Pointi position, Sizei dimensions, WindowData data, const std::string &hint = std::string()) :
        GUIWindow(WINDOW_SaveLoadBtn, position, dimensions, data, hint)
    {}
    virtual ~OnSaveLoad() {}

    virtual void Update() override;
};

class OnCancel : public GUIWindow {
 public:
    OnCancel(Pointi position, Sizei dimensions, WindowData data, const std::string &hint = std::string()) :
        GUIWindow(WINDOW_GenericCancel, position, dimensions, data, hint)
    {}
    virtual ~OnCancel() {}

    virtual void Update() override;
};

class OnCancel2 : public GUIWindow {
 public:
    OnCancel2(Pointi position, Sizei dimensions, WindowData data, const std::string &hint = std::string()) :
        GUIWindow(WINDOW_ExitCharacterWindow, position, dimensions, data, hint)
    {}
    virtual ~OnCancel2() {}

    virtual void Update() override;
};

class OnCancel3 : public GUIWindow {
 public:
    OnCancel3(Pointi position, Sizei dimensions, WindowData data, const std::string &hint = std::string()) :
        GUIWindow(WINDOW_LoadGame_CancelBtn, position, dimensions, data, hint)
    {}
    virtual ~OnCancel3() {}

    virtual void Update() override;
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

struct GUIMessage {
    enum UIMessageType eType;
    int param;
    int field_8;
    std::string file;
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

/**
 * Message queue for current frame.
 *
 * @offset 0x50CBD0
 */
extern struct GUIMessageQueue *pCurrentFrameMessageQueue;

/**
 * Message queue that will be processed on next frame.
 * @offset 0x50C9E8
 */
extern struct GUIMessageQueue *pNextFrameMessageQueue;

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
void GameUI_DrawPortraits();
void GameUI_DrawMinimap(unsigned int uX, unsigned int uY, unsigned int uZ,
                        unsigned int uW, unsigned int uZoom,
                        unsigned int bRedrawOdmMinimap);
std::string GameUI_GetMinimapHintText();
void GameUI_DrawPartySpells();
void GameUI_DrawTorchlightAndWizardEye();
void GameUI_DrawCharacterSelectionFrame();
void GameUI_CharacterQuickRecord_Draw(GUIWindow *window, Player *player);
void GameUI_DrawNPCPopup(void *_this);

// character ui
std::string CharacterUI_GetSkillDescText(unsigned int uPlayerID,
                                    PLAYER_SKILL_TYPE uPlayerSkillType);
void CharacterUI_SkillsTab_ShowHint();
void CharacterUI_StatsTab_ShowHint();
void CharacterUI_InventoryTab_Draw(Player *player, bool a2);
void CharacterUI_DrawPaperdoll(Player *player);
void CharacterUI_DrawPaperdollWithRingOverlay(Player *player);
void CharacterUI_ReleaseButtons();

/**
 * @offset 0x417AD4
 */
unsigned int GetSkillColor(PLAYER_CLASS_TYPE uPlayerClass, PLAYER_SKILL_TYPE uPlayerSkillType, PLAYER_SKILL_MASTERY skill_mastery);

void DrawSpellDescriptionPopup(int spell_index_in_book);

void UI_OnMouseRightClick(int mouse_x, int mouse_y);

void DrawPopupWindow(unsigned int uX, unsigned int uY, unsigned int uWidth,
                     unsigned int uHeight);  // idb
void DrawMM7CopyrightWindow();
void GUI_UpdateWindows();
int GetConditionDrawColor(Condition uConditionIdx);  // idb
void CreateAwardsScrollBar();
void ReleaseAwardsScrollBar();
void Inventory_ItemPopupAndAlchemy();
unsigned int UI_GetHealthManaAndOtherQualitiesStringColor(int current_pos,
                                                          int base_pos);
unsigned int GetSizeInInventorySlots(unsigned int uNumPixels);
class GUIButton *GUI_HandleHotkey(PlatformKey hotkey);
void GUI_ReplaceHotkey(PlatformKey oldKey, PlatformKey newKey, char bFirstCall);
void DrawBuff_remaining_time_string(int uY, GUIWindow *window,
                                    GameTime remaining_time, GUIFont *Font);
void GameUI_DrawItemInfo(struct ItemGen *inspect_item);   // idb
void MonsterPopup_Draw(unsigned int uActorID, GUIWindow *window);
void SetUserInterface(PartyAlignment alignment, bool bReplace);
void CreateMsgScrollWindow(ITEM_TYPE mscroll_id);
void free_book_subwindow();
void CreateScrollWindow();
void OnPaperdollLeftClick();
void DrawJoinGuildWindow(GUILD_ID guild_id);
const char *GetJoinGuildDialogueOption(GUILD_ID guild_id);
void DialogueEnding();

/**
 * @offset 0x4637E0
 */
bool isHoldingMouseRightButton();
void ClickNPCTopic(DIALOGUE_TYPE topic);
void _4B3FE5_training_dialogue(int a4);
void OracleDialogue();
std::string _4B254D_SkillMasteryTeacher(int trainerInfo);
std::string BuildDialogueString(const char *lpsz, uint8_t uPlayerID,
                           struct ItemGen *a3, int eventId, int a5,
                           GameTime *a6 = nullptr);
std::string BuildDialogueString(std::string &str, uint8_t uPlayerID,
                           struct ItemGen *a3, int eventId, int shop_screen,
                           GameTime *a6 = nullptr);
int const_2();


std::string NameAndTitle(const std::string &name, const std::string &title);
std::string NameAndTitle(const std::string &name, PLAYER_CLASS_TYPE class_type);
std::string NameAndTitle(const std::string &name, NPCProf profession);
std::string NameAndTitle(NPCData *npc);

std::string GetDisplayName(Actor *actor);

void SeekKnowledgeElswhereDialogueOption(GUIWindow *dialogue, Player *player);
void SkillTrainingDialogue(GUIWindow *dialogue, int num_skills_avaiable, int all_text_height, int skill_price);

extern GUIWindow *pPrimaryWindow;
//extern GUIWindow *pChestWindow;
extern GUIWindow *pDialogueWindow;
extern GUIWindow_House *window_SpeakInHouse;
extern GUIWindow_Scroll *pGUIWindow_ScrollWindow;
extern GUIWindow *ptr_507BC8;
extern GUIWindow *pGUIWindow_CurrentMenu;
//extern GUIWindow *ptr_507BD0;
extern GUIWindow *pGUIWindow_CastTargetedSpell;
extern GUIWindow *pGameOverWindow;
extern bool bGameOverWindowCheckExit;
//extern GUIWindow *pGUIWindow_EscMessageWindow;
extern GUIWindow *pGUIWindow_BranchlessDialogue;

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

extern DIALOGUE_TYPE _dword_F8B1D8_last_npc_topic_menu;
