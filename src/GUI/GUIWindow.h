#pragma once

#include <array>
#include <cstdint>
#include <list>
#include <memory>
#include <vector>
#include <string>

#include "Engine/PartyEnums.h"
#include "Engine/Objects/NPCEnums.h"
#include "Engine/Objects/ItemEnums.h"

#include "GUI/GUIEnums.h"
#include "GUI/GUIDialogues.h"
#include "GUI/UI/UIHouseEnums.h"

#include "Io/InputAction.h"
#include "Io/KeyboardInputHandler.h"

#include "Platform/PlatformEnums.h"

#include "Library/Color/ColorTable.h"

#include "Utility/Geometry/Size.h"
#include "Utility/Geometry/Point.h"

namespace Io {
class Mouse;
} // namespace Io

class Actor;
class GUIFont;
class GUIButton;
class GUIWindow_House;
class GUIWindow_MessageScroll;
class Character;
class NPCData;
class GraphicsImage;
class Logger;

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
                            UIMessageType msg, unsigned int msg_param, Io::InputAction action = Io::InputAction::Invalid, const std::string &label = {},
                            const std::vector<GraphicsImage *> &textures = {});

    GUIButton *CreateButton(std::string id, Pointi position, Sizei dimensions, int uButtonType, int uData,
                            UIMessageType msg, unsigned int msg_param, Io::InputAction action = Io::InputAction::Invalid, const std::string &label = {},
                            const std::vector<GraphicsImage *> &textures = {});

    bool Contains(unsigned int x, unsigned int y);
    void DrawFlashingInputCursor(int uX, int uY, GUIFont *a2);

    int DrawTextInRect(GUIFont *font, Pointi position, Color color, const std::string &text, int rect_width, int reverse_text);

    void DrawText(GUIFont *font, Pointi position, Color color, const std::string &text, int maxHeight = 0, Color shadowColor = colorTable.Black);

    void DrawTitleText(GUIFont *font, int horizontalMargin, int verticalMargin, Color color, const std::string &text, int lineSpacing);

    void DrawShops_next_generation_time_string(GameTime time);
    void DrawMessageBox(bool inside_game_viewport);
    GUIButton *GetControl(unsigned int uID);

    /**
     * @offset 0x41D08F
     */
    void setKeyboardControlGroup(int buttonsCount, bool msgOnSelect, int selectStep, int initialPosition);

    virtual void Update() {}
    virtual void Release();
    void DeleteButtons();

    static void InitializeGUI();

    int uFrameX = 0;
    int uFrameY = 0;
    int uFrameWidth = 0;
    int uFrameHeight = 0;
    int uFrameZ = 0;
    int uFrameW = 0;
    WindowType eWindowType = WINDOW_null;
    WindowData wData; // Window-specific
    int field_24 = 0;
    int pNumPresenceButton = 0;
    int pCurrentPosActiveItem = 0;
    bool _msgOnKeyboardSelect = true;
    int _selectStep = 0;
    int pStartingPosActiveItem = 0;
    WindowInputStatus keyboard_input_status = WINDOW_INPUT_NONE;
    bool receives_keyboard_input = false;
    std::string sHint;
    std::vector<GUIButton*> vButtons;

    std::shared_ptr<Io::Mouse> mouse = nullptr;
    Logger *log = nullptr;
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
                        unsigned int uW, unsigned int uZoom, unsigned int bRedrawOdmMinimap);
std::string GameUI_GetMinimapHintText();
void GameUI_DrawPartySpells();
void GameUI_DrawTorchlightAndWizardEye();
void GameUI_DrawCharacterSelectionFrame();

// character ui
void CharacterUI_InventoryTab_Draw(Character *player, bool a2);
void CharacterUI_DrawPaperdoll(Character *player);
void CharacterUI_DrawPaperdollWithRingOverlay(Character *player);
void CharacterUI_ReleaseButtons();

/**
 * @offset 0x417AD4
 */
Color GetSkillColor(CharacterClassType uPlayerClass, CharacterSkillType uPlayerSkillType, CharacterSkillMastery skill_mastery);

void UI_OnMouseRightClick(int mouse_x, int mouse_y);

void GUI_UpdateWindows();
Color GetConditionDrawColor(Condition uConditionIdx);  // idb
Color UI_GetHealthManaAndOtherQualitiesStringColor(int current_pos, int base_pos);
unsigned int GetSizeInInventorySlots(unsigned int uNumPixels);
class GUIButton *GUI_HandleHotkey(PlatformKey hotkey);
void GUI_ReplaceHotkey(PlatformKey oldKey, PlatformKey newKey, char bFirstCall);
void DrawBuff_remaining_time_string(int uY, GUIWindow *window,
                                    GameTime remaining_time, GUIFont *Font);
void SetUserInterface(PartyAlignment alignment, bool bReplace);
void CreateMsgScrollWindow(ItemId mscroll_id);
void CreateScrollWindow();
void OnPaperdollLeftClick();
void DialogueEnding();

/**
 * @offset 0x4637E0
 */
bool isHoldingMouseRightButton();

std::string BuildDialogueString(const char *lpsz, uint8_t uPlayerID,
                           struct ItemGen *a3, HOUSE_ID houseId, int a5,
                           GameTime *a6 = nullptr);
std::string BuildDialogueString(std::string &str, uint8_t uPlayerID,
                           struct ItemGen *a3, HOUSE_ID houseId, int shop_screen,
                           GameTime *a6 = nullptr);


std::string NameAndTitle(const std::string &name, const std::string &title);
std::string NameAndTitle(const std::string &name, CharacterClassType class_type);
std::string NameAndTitle(const std::string &name, NPCProf profession);
std::string NameAndTitle(NPCData *npc);

std::string GetDisplayName(Actor *actor);

extern GUIWindow *pPrimaryWindow;
//extern GUIWindow *pChestWindow;
extern GUIWindow *pDialogueWindow;
extern GUIWindow_House *window_SpeakInHouse;
extern GUIWindow_MessageScroll *pGUIWindow_ScrollWindow;
extern GUIWindow *ptr_507BC8;
extern GUIWindow *pGUIWindow_CurrentMenu;
//extern GUIWindow *ptr_507BD0;
extern GUIWindow *pGUIWindow_CastTargetedSpell;
extern GUIWindow *pGameOverWindow;
extern bool bGameOverWindowCheckExit;
//extern GUIWindow *pGUIWindow_EscMessageWindow;
extern GUIWindow *pGUIWindow_BranchlessDialogue;

extern Color ui_mainmenu_copyright_color;
extern Color ui_character_tooltip_header_default_color;
extern Color ui_character_default_text_color;
extern Color ui_character_skill_highlight_color;
extern Color ui_character_header_text_color;
extern Color ui_character_bonus_text_color;
extern Color ui_character_bonus_text_color_neg;
extern Color ui_character_skill_upgradeable_color;
extern Color ui_character_skill_default_color;
extern Color ui_character_stat_default_color;
extern Color ui_character_stat_buffed_color;
extern Color ui_character_stat_debuffed_color;
extern Color ui_character_skillinfo_can_learn;
extern Color ui_character_skillinfo_can_learn_gm;
extern Color ui_character_skillinfo_cant_learn;
extern Color ui_character_condition_normal_color;
extern Color ui_character_condition_light_color;
extern Color ui_character_condition_moderate_color;
extern Color ui_character_condition_severe_color;
extern std::array<Color, 6> ui_character_award_color;
extern Color ui_game_minimap_outline_color;
extern Color ui_game_minimap_actor_friendly_color;
extern Color ui_game_minimap_actor_hostile_color;
extern Color ui_game_minimap_actor_corpse_color;
extern Color ui_game_minimap_decoration_color_1;
extern Color ui_game_minimap_projectile_color;
extern Color ui_game_minimap_treasure_color;
extern std::array<Color, 24> ui_game_character_record_playerbuff_colors;
extern Color ui_gamemenu_video_gamma_title_color;
extern Color ui_gamemenu_keys_action_name_color;
extern Color ui_gamemenu_keys_key_selection_blink_color_1;
extern Color ui_gamemenu_keys_key_selection_blink_color_2;
extern Color ui_gamemenu_keys_key_default_color;
extern Color ui_book_quests_title_color;
extern Color ui_book_quests_text_color;
extern Color ui_book_autonotes_title_color;
extern Color ui_book_autonotes_text_color;
extern Color ui_book_map_title_color;
extern Color ui_book_map_coordinates_color;
extern Color ui_book_calendar_title_color;
extern Color ui_book_calendar_time_color;
extern Color ui_book_calendar_day_color;
extern Color ui_book_calendar_month_color;
extern Color ui_book_calendar_year_color;
extern Color ui_book_calendar_moon_color;
extern Color ui_book_calendar_location_color;
extern Color ui_book_journal_title_color;
extern Color ui_book_journal_text_color;
extern Color ui_book_journal_text_shadow;
extern Color ui_game_dialogue_npc_name_color;
extern Color ui_game_dialogue_option_highlight_color;
extern Color ui_game_dialogue_option_normal_color;
extern Color ui_house_player_cant_interact_color;

class GraphicsImage;
extern GraphicsImage *ui_exit_cancel_button_background;
extern GraphicsImage *game_ui_right_panel_frame;
extern GraphicsImage *dialogue_ui_x_ok_u;
extern GraphicsImage *dialogue_ui_x_x_u;

extern GraphicsImage *ui_buttdesc2;
extern GraphicsImage *ui_buttyes2;

extern GraphicsImage *ui_btn_npc_right;
extern GraphicsImage *ui_btn_npc_left;

extern GraphicsImage *ui_ar_dn_dn;
extern GraphicsImage *ui_ar_dn_up;
extern GraphicsImage *ui_ar_up_dn;
extern GraphicsImage *ui_ar_up_up;

extern GraphicsImage *ui_leather_mm6;
extern GraphicsImage *ui_leather_mm7;

extern MenuType sCurrentMenuID;
void SetCurrentMenuID(MenuType);
MenuType GetCurrentMenuID();

extern ScreenType current_screen_type;
extern ScreenType prev_screen_type;
