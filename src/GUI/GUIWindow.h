#pragma once

#include <array>
#include <cstdint>
#include <list>
#include <memory>
#include <vector>
#include <string>

#include "Engine/Data/HouseEnums.h"
#include "Engine/PartyEnums.h"
#include "Engine/Objects/NPCEnums.h"
#include "Engine/Objects/ItemEnums.h"
#include "Engine/Time/Time.h"

#include "GUI/GUIEnums.h"
#include "GUI/GUIDialogues.h"

#include "Io/InputAction.h"
#include "Io/KeyboardInputHandler.h"

#include "Library/Platform/Interface/PlatformEnums.h"
#include "Library/Color/ColorTable.h"
#include "Library/Geometry/Size.h"
#include "Library/Geometry/Point.h"
#include "Library/Geometry/Rect.h"

#include "Utility/IndexedArray.h"

namespace Io {
class Mouse;
} // namespace Io

class Actor;
class GUIFont;
class GUIButton;
class GUIWindow_House;
class GUIWindow_Chest;
class GUIWindow_MessageScroll;
class GUIWindow_BranchlessDialogue;
class GUIWindow_GameOver;
class Character;
struct NPCData;
class GraphicsImage;
class TargetedSpellUI;
struct Item;

class GUIWindow {
 public:
    GUIWindow();
    GUIWindow(WindowType windowType, Pointi position, Sizei dimensions, std::string_view hint = {});
    virtual ~GUIWindow() = default;

    GUIButton *CreateButton(Pointi position, Sizei dimensions, int uButtonType, int uData,
                            UIMessageType msg, unsigned int msg_param, Io::InputAction action = Io::InputAction::Invalid, std::string_view label = {},
                            const std::vector<GraphicsImage *> &textures = {});

    GUIButton *CreateButton(std::string id, Pointi position, Sizei dimensions, int uButtonType, int uData,
                            UIMessageType msg, unsigned int msg_param, Io::InputAction action = Io::InputAction::Invalid, std::string_view label = {},
                            const std::vector<GraphicsImage *> &textures = {});

    bool Contains(unsigned int x, unsigned int y);
    void DrawFlashingInputCursor(int uX, int uY, GUIFont *a2);

    int DrawTextInRect(GUIFont *font, Pointi position, Color color, std::string_view text, int rect_width, int reverse_text);

    void DrawText(GUIFont *font, Pointi position, Color color, std::string_view text, int maxHeight = 0, Color shadowColor = colorTable.Black);

    void DrawTitleText(GUIFont *font, int horizontalMargin, int verticalMargin, Color color, std::string_view text, int lineSpacing);

    void DrawShops_next_generation_time_string(Duration time);
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
    int uFrameHeight = 0; // TODO(captainurist): frameRect
    int uFrameZ = 0;
    int uFrameW = 0;
    WindowType eWindowType = WINDOW_null;
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
};

class OnButtonClick : public GUIWindow {
 public:
    OnButtonClick(Pointi position, Sizei dimensions, GUIButton *button, std::string_view hint = {}, bool playSound = true) :
        GUIWindow(WINDOW_CharacterCreationBtn, position, dimensions, hint),
        _playSound(playSound),
        _button(button)
    {}

    virtual void Update() override;

 private:
    bool _playSound = false;
    GUIButton *_button = nullptr;
};

class OnButtonClick2 : public GUIWindow {
 public:
    OnButtonClick2(Pointi position, Sizei dimensions, GUIButton *button, std::string_view hint = {}, bool play_sound = true) :
        GUIWindow(WINDOW_PressedButton2, position, dimensions, hint),
        _playSound(play_sound),
        _button(button)
    {}

    virtual void Update() override;

 private:
    bool _playSound = false;
    GUIButton *_button = nullptr;
};

class OnButtonClick3 : public GUIWindow {
 public:
    OnButtonClick3(WindowType windowType, Pointi position, Sizei dimensions, GUIButton *button, std::string_view hint = {}) :
        GUIWindow(windowType, position, dimensions, hint),
        _button(button)
    {}

    virtual void Update() override;

 private:
    GUIButton *_button = nullptr;
};

// something debug? not really sure, unused
class OnButtonClick4 : public GUIWindow {
 public:
    OnButtonClick4(Pointi position, Sizei dimensions, GUIButton *button, std::string_view hint = {}) :
        GUIWindow(WINDOW_59, position, dimensions, hint),
        _button(button)
    {}

    virtual void Update() override;

 private:
    GUIButton *_button = nullptr;
};

class OnSaveLoad : public GUIWindow {
 public:
    OnSaveLoad(Pointi position, Sizei dimensions, GUIButton *button, std::string_view hint = {}) :
        GUIWindow(WINDOW_SaveLoadBtn, position, dimensions, hint),
        _button(button)
    {}

    virtual void Update() override;

 private:
    GUIButton *_button = nullptr;
};

class OnCancel : public GUIWindow {
 public:
    OnCancel(Pointi position, Sizei dimensions, GUIButton *button, std::string_view hint = {}) :
        GUIWindow(WINDOW_GenericCancel, position, dimensions, hint),
        _button(button)
    {}

    virtual void Update() override;

 private:
    GUIButton *_button = nullptr;
};

class OnCancel2 : public GUIWindow {
 public:
    OnCancel2(Pointi position, Sizei dimensions, GUIButton *button, std::string_view hint = {}) :
        GUIWindow(WINDOW_ExitCharacterWindow, position, dimensions, hint),
        _button(button)
    {}

    virtual void Update() override;

 private:
    GUIButton *_button = nullptr;
};

class OnCancel3 : public GUIWindow {
 public:
    OnCancel3(Pointi position, Sizei dimensions, GUIButton *button, std::string_view hint = {}) :
        GUIWindow(WINDOW_LoadGame_CancelBtn, position, dimensions, hint),
        _button(button)
    {}

    virtual void Update() override;

 private:
    GUIButton *_button = nullptr;
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

/**
 * @param rect                          Screen rect to draw the minimap at.
 * @param zoom                          The number of screen pixels a location map should take. Default outdoor zoom
 *                                      level is 512, so that means that an outdoor location map would take 512x512
 *                                      pixels on screen if not cropped. For indoor locations, this is the number of
 *                                      screen pixels an indoor location the size of a regular outdoor location would
 *                                      take. Note that outdoor location size is 2^16x2^16 in in-game coordinates.
 */
void GameUI_DrawMinimap(const Recti &rect, int zoom);
std::string GameUI_GetMinimapHintText();
void GameUI_DrawPartySpells();
void GameUI_DrawTorchlightAndWizardEye();
void GameUI_DrawCharacterSelectionFrame();

// character ui
void CharacterUI_InventoryTab_Draw(Character *player, bool a2);
void CharacterUI_DrawPaperdoll(Character *player);
void CharacterUI_DrawPaperdollWithRingOverlay(Character *player);
void CharacterUI_ReleaseButtons();
void CharacterUI_DrawPickedItemUnderlay(Vec2i offset);

/**
 * @offset 0x417AD4
 */
Color GetSkillColor(CharacterClass uPlayerClass, CharacterSkillType uPlayerSkillType, CharacterSkillMastery skill_mastery);

void UI_OnMouseRightClick(Pointi mousePos);

void GUI_UpdateWindows();
Color GetConditionDrawColor(Condition uConditionIdx);  // idb
Color UI_GetHealthManaAndOtherQualitiesStringColor(int current_pos, int base_pos);
int GetSizeInInventorySlots(int uNumPixels);
GUIButton *GUI_HandleHotkey(PlatformKey hotkey);
void GUI_ReplaceHotkey(PlatformKey oldKey, PlatformKey newKey, char bFirstCall);
void DrawBuff_remaining_time_string(int uY, GUIWindow *window,
                                    Duration remaining_time, GUIFont *Font);
void SetUserInterface(PartyAlignment alignment);
void CreateMsgScrollWindow(ItemId mscroll_id);
void CreateScrollWindow();
void OnPaperdollLeftClick();
void DialogueEnding();

/**
 * @offset 0x4637E0
 */
bool isHoldingMouseRightButton();

/**
 * @offset 0x495461
 */
std::string BuildDialogueString(std::string_view str, int uPlayerID, NPCData *npc,
                                Item *item = nullptr, HouseId houseId = HOUSE_INVALID, ShopScreen shop_screen = SHOP_SCREEN_INVALID,
                                Time *a6 = nullptr);


std::string NameAndTitle(std::string_view name, std::string_view title);
std::string NameAndTitle(std::string_view name, CharacterClass class_type);
std::string NameAndTitle(std::string_view name, NpcProfession profession);
std::string NameAndTitle(NPCData *npc);

std::string GetDisplayName(Actor *actor);

extern GUIWindow *pPrimaryWindow;
extern GUIWindow *pDialogueWindow;
extern GUIWindow_House *window_SpeakInHouse;
extern GUIWindow_MessageScroll *pGUIWindow_ScrollWindow;
extern GUIWindow *ptr_507BC8;
extern GUIWindow *pGUIWindow_CurrentMenu;
extern GUIWindow_Chest *pGUIWindow_CurrentChest;
//extern GUIWindow *ptr_507BD0;
extern TargetedSpellUI *pGUIWindow_CastTargetedSpell;
extern GUIWindow_GameOver *pGameOverWindow;
//extern GUIWindow *pGUIWindow_EscMessageWindow;
extern GUIWindow_BranchlessDialogue *pGUIWindow_BranchlessDialogue;

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

extern const IndexedArray<Color, CHARACTER_BUFF_FIRST, CHARACTER_BUFF_LAST> ui_game_character_record_playerbuff_colors;
