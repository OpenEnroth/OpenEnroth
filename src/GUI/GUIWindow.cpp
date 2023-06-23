#include "GUI/GUIWindow.h"

#include <stdlib.h>
#include <sstream>
#include <utility>

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/AssetsManager.h"
#include "Engine/Events/Processor.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/PriceCalculator.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Tables/StorylineTextTable.h"
#include "Engine/Tables/AwardTable.h"
#include "Engine/Tables/AutonoteTable.h"
#include "Engine/Tables/MessageScrollTable.h"
#include "Engine/Time.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIBooks.h"
#include "GUI/UI/UICharacter.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIPopup.h"
#include "GUI/UI/UIStatusBar.h"

#include "Io/InputAction.h"
#include "Io/KeyboardInputHandler.h"
#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

#include "Library/Serialization/EnumSerialization.h"

#include "Library/Random/Random.h"

using Io::InputAction;

GUIWindow *pPrimaryWindow;

GUIWindow *pGUIWindow_CurrentMenu;
GUIWindow *pDialogueWindow;
GUIWindow_House *window_SpeakInHouse;
GUIWindow_MessageScroll *pGUIWindow_ScrollWindow; // reading a message scroll
GUIWindow *ptr_507BC8;  // screen 19 - not used?
GUIWindow *pGUIWindow_CastTargetedSpell;
GUIWindow *pGameOverWindow; // UIMSG_ShowGameOverWindow
bool bGameOverWindowCheckExit{ false }; // TODO(pskelton): contain
GUIWindow *pGUIWindow_BranchlessDialogue; // branchless dialougue

enum WindowType current_character_screen_window;
std::list<GUIWindow*> lWindowList;

MENU_STATE sCurrentMenuID;

enum CURRENT_SCREEN current_screen_type = CURRENT_SCREEN::SCREEN_VIDEO;
enum CURRENT_SCREEN prev_screen_type;

struct GUIMessageQueue *pCurrentFrameMessageQueue = new GUIMessageQueue;
struct GUIMessageQueue *pNextFrameMessageQueue = new GUIMessageQueue;

GraphicsImage *ui_exit_cancel_button_background = nullptr;
GraphicsImage *game_ui_right_panel_frame = nullptr;
GraphicsImage *dialogue_ui_x_ok_u = nullptr;
GraphicsImage *dialogue_ui_x_x_u = nullptr;

GraphicsImage *ui_buttdesc2 = nullptr;
GraphicsImage *ui_buttyes2 = nullptr;

GraphicsImage *ui_btn_npc_right = nullptr;
GraphicsImage *ui_btn_npc_left = nullptr;

GraphicsImage *ui_ar_dn_dn = nullptr;
GraphicsImage *ui_ar_dn_up = nullptr;
GraphicsImage *ui_ar_up_dn = nullptr;
GraphicsImage *ui_ar_up_up = nullptr;

GraphicsImage *ui_leather_mm6 = nullptr;
GraphicsImage *ui_leather_mm7 = nullptr;

DIALOGUE_TYPE _dword_F8B1D8_last_npc_topic_menu;
AwardType dword_F8B1AC_award_bit_number;
CharacterSkillType dword_F8B1AC_skill_being_taught; // Address the same as above --- splitting a union into two variables.

std::array<int, 28> possibleAddressingAwardBits = {{1,  2,  3,  4,  5,  7,  32, 33, 36, 37,
                                                    38, 40, 41, 42, 43, 45, 46, 47, 48, 49,
                                                    50, 51, 52, 53, 54, 55, 56, 60}};

// TODO(Nik-RE-dev): move somewhere along with guild join dialogue processing
IndexedArray<int, GUILD_FIRST, GUILD_LAST> priceForMembership = {{
    {GUILD_OF_ELEMENTS, 100},
    {GUILD_OF_SELF,     100},
    {GUILD_OF_AIR,      50},
    {GUILD_OF_EARTH,    50},
    {GUILD_OF_FIRE,     50},
    {GUILD_OF_WATER,    50},
    {GUILD_OF_BODY,     50},
    {GUILD_OF_MIND,     50},
    {GUILD_OF_SPIRIT,   50},
    {GUILD_OF_LIGHT,    1000},
    {GUILD_OF_DARK,     1000}
}};

// TODO(Nik-RE-dev): move somewhere along with mastery teacing dialogue processing
IndexedArray<int, CHARACTER_SKILL_FIRST, CHARACTER_SKILL_LAST> expertSkillMasteryCost = {{
    {CHARACTER_SKILL_STAFF,        2000},
    {CHARACTER_SKILL_SWORD,        2000},
    {CHARACTER_SKILL_DAGGER,       2000},
    {CHARACTER_SKILL_AXE,          2000},
    {CHARACTER_SKILL_SPEAR,        2000},
    {CHARACTER_SKILL_BOW,          2000},
    {CHARACTER_SKILL_MACE,         2000},
    {CHARACTER_SKILL_BLASTER,      0},
    {CHARACTER_SKILL_SHIELD,       1000},
    {CHARACTER_SKILL_LEATHER,      1000},
    {CHARACTER_SKILL_CHAIN,        1000},
    {CHARACTER_SKILL_PLATE,        1000},
    {CHARACTER_SKILL_FIRE,         1000},
    {CHARACTER_SKILL_AIR,          1000},
    {CHARACTER_SKILL_WATER,        1000},
    {CHARACTER_SKILL_EARTH,        1000},
    {CHARACTER_SKILL_SPIRIT,       1000},
    {CHARACTER_SKILL_MIND,         1000},
    {CHARACTER_SKILL_BODY,         1000},
    {CHARACTER_SKILL_LIGHT,        2000},
    {CHARACTER_SKILL_DARK,         2000},
    {CHARACTER_SKILL_ITEM_ID,      500},
    {CHARACTER_SKILL_MERCHANT,     2000},
    {CHARACTER_SKILL_REPAIR,       500},
    {CHARACTER_SKILL_BODYBUILDING, 500},
    {CHARACTER_SKILL_MEDITATION,   500},
    {CHARACTER_SKILL_PERCEPTION,   500},
    {CHARACTER_SKILL_DIPLOMACY,    0}, // not used
    {CHARACTER_SKILL_THIEVERY,     0}, // not used
    {CHARACTER_SKILL_TRAP_DISARM,  500},
    {CHARACTER_SKILL_DODGE,        2000},
    {CHARACTER_SKILL_UNARMED,      2000},
    {CHARACTER_SKILL_MONSTER_ID,   500},
    {CHARACTER_SKILL_ARMSMASTER,   2000},
    {CHARACTER_SKILL_STEALING,     500},
    {CHARACTER_SKILL_ALCHEMY,      500},
    {CHARACTER_SKILL_LEARNING,     2000},
    {CHARACTER_SKILL_CLUB,         500},
    {CHARACTER_SKILL_MISC,         0} // hidden, not used
}};

IndexedArray<int, CHARACTER_SKILL_FIRST, CHARACTER_SKILL_LAST> masterSkillMasteryCost = {{
    {CHARACTER_SKILL_STAFF,        5000},
    {CHARACTER_SKILL_SWORD,        5000},
    {CHARACTER_SKILL_DAGGER,       5000},
    {CHARACTER_SKILL_AXE,          5000},
    {CHARACTER_SKILL_SPEAR,        5000},
    {CHARACTER_SKILL_BOW,          5000},
    {CHARACTER_SKILL_MACE,         5000},
    {CHARACTER_SKILL_BLASTER,      0},
    {CHARACTER_SKILL_SHIELD,       3000},
    {CHARACTER_SKILL_LEATHER,      3000},
    {CHARACTER_SKILL_CHAIN,        3000},
    {CHARACTER_SKILL_PLATE,        3000},
    {CHARACTER_SKILL_FIRE,         4000},
    {CHARACTER_SKILL_AIR,          4000},
    {CHARACTER_SKILL_WATER,        4000},
    {CHARACTER_SKILL_EARTH,        4000},
    {CHARACTER_SKILL_SPIRIT,       4000},
    {CHARACTER_SKILL_MIND,         4000},
    {CHARACTER_SKILL_BODY,         4000},
    {CHARACTER_SKILL_LIGHT,        5000},
    {CHARACTER_SKILL_DARK,         5000},
    {CHARACTER_SKILL_ITEM_ID,      2500},
    {CHARACTER_SKILL_MERCHANT,     5000},
    {CHARACTER_SKILL_REPAIR,       2500},
    {CHARACTER_SKILL_BODYBUILDING, 2500},
    {CHARACTER_SKILL_MEDITATION,   2500},
    {CHARACTER_SKILL_PERCEPTION,   2500},
    {CHARACTER_SKILL_DIPLOMACY,    0}, // not used
    {CHARACTER_SKILL_THIEVERY,     0}, // not used
    {CHARACTER_SKILL_TRAP_DISARM,  2500},
    {CHARACTER_SKILL_DODGE,        5000},
    {CHARACTER_SKILL_UNARMED,      5000},
    {CHARACTER_SKILL_MONSTER_ID,   2500},
    {CHARACTER_SKILL_ARMSMASTER,   5000},
    {CHARACTER_SKILL_STEALING,     2500},
    {CHARACTER_SKILL_ALCHEMY,      2500},
    {CHARACTER_SKILL_LEARNING,     5000},
    {CHARACTER_SKILL_CLUB,         2500},
    {CHARACTER_SKILL_MISC,         0} // hidden, not used
}};

IndexedArray<int, CHARACTER_SKILL_FIRST, CHARACTER_SKILL_LAST> grandmasterSkillMasteryCost = {{
    {CHARACTER_SKILL_STAFF,        8000},
    {CHARACTER_SKILL_SWORD,        8000},
    {CHARACTER_SKILL_DAGGER,       8000},
    {CHARACTER_SKILL_AXE,          8000},
    {CHARACTER_SKILL_SPEAR,        8000},
    {CHARACTER_SKILL_BOW,          8000},
    {CHARACTER_SKILL_MACE,         8000},
    {CHARACTER_SKILL_BLASTER,      0},
    {CHARACTER_SKILL_SHIELD,       7000},
    {CHARACTER_SKILL_LEATHER,      7000},
    {CHARACTER_SKILL_CHAIN,        7000},
    {CHARACTER_SKILL_PLATE,        7000},
    {CHARACTER_SKILL_FIRE,         8000},
    {CHARACTER_SKILL_AIR,          8000},
    {CHARACTER_SKILL_WATER,        8000},
    {CHARACTER_SKILL_EARTH,        8000},
    {CHARACTER_SKILL_SPIRIT,       8000},
    {CHARACTER_SKILL_MIND,         8000},
    {CHARACTER_SKILL_BODY,         8000},
    {CHARACTER_SKILL_LIGHT,        8000},
    {CHARACTER_SKILL_DARK,         8000},
    {CHARACTER_SKILL_ITEM_ID,      6000},
    {CHARACTER_SKILL_MERCHANT,     8000},
    {CHARACTER_SKILL_REPAIR,       6000},
    {CHARACTER_SKILL_BODYBUILDING, 6000},
    {CHARACTER_SKILL_MEDITATION,   6000},
    {CHARACTER_SKILL_PERCEPTION,   6000},
    {CHARACTER_SKILL_DIPLOMACY,    0}, // not used
    {CHARACTER_SKILL_THIEVERY,     0}, // not used
    {CHARACTER_SKILL_TRAP_DISARM,  6000},
    {CHARACTER_SKILL_DODGE,        8000},
    {CHARACTER_SKILL_UNARMED,      8000},
    {CHARACTER_SKILL_MONSTER_ID,   6000},
    {CHARACTER_SKILL_ARMSMASTER,   8000},
    {CHARACTER_SKILL_STEALING,     6000},
    {CHARACTER_SKILL_ALCHEMY,      6000},
    {CHARACTER_SKILL_LEARNING,     8000},
    {CHARACTER_SKILL_CLUB,         6000},
    {CHARACTER_SKILL_MISC,         0} // hidden, not used
}};

void SetCurrentMenuID(MENU_STATE uMenu) {
    sCurrentMenuID = uMenu;
    logger->warning("CurrentMenu = {} \n", toString(uMenu));
}

MENU_STATE GetCurrentMenuID() {
    return sCurrentMenuID;
}

bool PauseGameDrawing() {
    if (current_screen_type != CURRENT_SCREEN::SCREEN_GAME &&
        current_screen_type != CURRENT_SCREEN::SCREEN_NPC_DIALOGUE &&
        current_screen_type != CURRENT_SCREEN::SCREEN_CHANGE_LOCATION) {
        if (current_screen_type == CURRENT_SCREEN::SCREEN_INPUT_BLV) return false; // uCurrentHouse_Animation;
        if (current_screen_type != CURRENT_SCREEN::SCREEN_BRANCHLESS_NPC_DIALOG) return true;
    }
    return false;
}

void GUIMessageQueue::Flush() {
    if (qMessages.size()) {
        GUIMessage message = qMessages.front();
        Clear();
        if (message.field_8 != 0) {
            qMessages.push(message);
        }
    }
}

void GUIMessageQueue::Clear() {
    std::queue<GUIMessage> empty;
    std::swap(qMessages, empty);
}

void GUIMessageQueue::PopMessage(UIMessageType *pType, int *pParam,
    int *a4) {
    *pType = (UIMessageType)-1;
    *pParam = 0;
    *a4 = 0;

    if (qMessages.empty()) {
        return;
    }

    GUIMessage message = qMessages.front();
    qMessages.pop();

    *pType = message.eType;
    *pParam = message.param;
    *a4 = message.field_8;
}

GUIButton *GUI_HandleHotkey(PlatformKey hotkey) {
    for (GUIWindow *pWindow : lWindowList) {
        for (GUIButton *result : pWindow->vButtons) {
            if (result->action != InputAction::Invalid && keyboardActionMapping->IsKeyMatchAction(result->action, hotkey)) {
                pCurrentFrameMessageQueue->AddGUIMessage(result->msg, result->msg_param, 0);
                return result;
            }
        }

        int width = render->GetPresentDimensions().w;
        if (pWindow->uFrameX == 0 && pWindow->uFrameY == 0 &&
            pWindow->uFrameWidth == width && pWindow->uFrameHeight == width) {
            break;
        }
    }
    return 0;
}

//----- (0041D08F) --------------------------------------------------------
void GUIWindow::_41D08F_set_keyboard_control_group(int num_buttons, int a3,
    int a4, int a5) {
    if (num_buttons) {
        this->pNumPresenceButton = num_buttons;
        this->field_30 = a3;
        this->field_34 = a4;
        this->pCurrentPosActiveItem = a5;
        this->pStartingPosActiveItem = a5;
        this->receives_keyboard_input = true;
    } else {
        this->pNumPresenceButton = 0;
        this->field_30 = a3;
        this->field_34 = a4;
        this->pCurrentPosActiveItem = 0;
        this->pStartingPosActiveItem = 0;
        this->receives_keyboard_input = false;
    }
}

void GUIWindow::Init() {
    this->uFrameX = 0;
    this->uFrameY = 0;
    this->uFrameWidth = 0;
    this->uFrameHeight = 0;
    this->uFrameZ = 0;
    this->uFrameW = 0;
    this->eWindowType = WINDOW_null;
    this->field_24 = 0;
    this->pNumPresenceButton = 0;
    this->pCurrentPosActiveItem = 0;
    this->field_30 = 0;
    this->field_34 = 0;
    this->pStartingPosActiveItem = 0;
    this->keyboard_input_status = WINDOW_INPUT_NONE;
    this->receives_keyboard_input = false;
    this->sHint.clear();

    DeleteButtons();
}

void GUIWindow::Release() {
    if (!this || this->eWindowType == WINDOW_null) {
        // added the check to avoid releasing
        // windows already released
        return;
    }
    DeleteButtons();

    lWindowList.remove(this);

    if (this->eWindowType == WINDOW_GameUI)
        nuklear->Release(WINDOW_GameUI);

    log->info("Release window: {}", toString(eWindowType));
}

void GUIWindow::DeleteButtons() {
    while (vButtons.size()) {
        vButtons.front()->Release();
    }
}

GUIButton *GUIWindow::GetControl(unsigned int uID) {
    if (uID >= vButtons.size()) {
        return nullptr;
    }

    return vButtons[uID];
}

void GUIWindow::DrawMessageBox(bool inside_game_viewport) {
    int x = 0;
    int y = 0;
    int z, w;
    if (inside_game_viewport) {
        x = pViewport->uViewportTL_X;
        z = pViewport->uViewportBR_X;
        y = pViewport->uViewportTL_Y;
        w = pViewport->uViewportBR_Y;
    } else {
        Sizei renDims = render->GetRenderDimensions();
        z = renDims.w;
        w = renDims.h;
    }

    Pointi cursor = mouse->GetCursorPos();
    if ((int)this->uFrameX >= x) {
        if ((int)(this->uFrameWidth + this->uFrameX) > z) {
            this->uFrameX = z - this->uFrameWidth;
            this->uFrameY = cursor.y + 30;
        }
    } else {
        this->uFrameX = x;
        this->uFrameY = cursor.y + 30;
    }

    if ((int)this->uFrameY >= y) {
        if ((int)(this->uFrameY + this->uFrameHeight) > w) {
            this->uFrameY = cursor.y - this->uFrameHeight - 30;
        }
    } else {
        this->uFrameY = cursor.y + 30;
    }
    if ((signed int)this->uFrameY < y) {
        this->uFrameY = y;
    }
    if ((signed int)this->uFrameX < x) {
        this->uFrameX = x;
    }
    this->uFrameZ = this->uFrameWidth + this->uFrameX - 1;
    this->uFrameW = this->uFrameHeight + this->uFrameY - 1;

    GUIWindow current_window = *this;
    current_window.uFrameX += 12;
    current_window.uFrameWidth -= 28;
    current_window.uFrameY += 12;
    current_window.uFrameHeight -= 12;
    current_window.uFrameZ =
        current_window.uFrameWidth + current_window.uFrameX - 1;
    current_window.uFrameW =
        current_window.uFrameHeight + current_window.uFrameY - 1;
    unsigned int uBoxHeight;
    if (!sHint.empty()) {
        uBoxHeight =
            pFontLucida->CalcTextHeight(sHint, current_window.uFrameWidth, 0) +
            24;
    } else {
        uBoxHeight = uFrameHeight;
    }
    if (uBoxHeight < 64) {
        uBoxHeight = 64;
    }
    if ((int)(uBoxHeight + this->uFrameY) > 479) {
        uBoxHeight = 479 - this->uFrameY;
    }
    DrawPopupWindow(this->uFrameX, this->uFrameY, this->uFrameWidth,
        uBoxHeight);
    if (!sHint.empty()) {
        current_window.DrawTitleText(
            pFontLucida,
            0, (int)(uBoxHeight - pFontLucida->CalcTextHeight(this->sHint, current_window.uFrameWidth, 0)) / 2 - 14,
            colorTable.White, this->sHint, 3);
    }
}

std::string MakeDateTimeString(GameTime time) {
    int seconds = time.GetSecondsFraction();
    int minutes = time.GetMinutesFraction();
    int hours = time.GetHoursOfDay();
    int days = time.GetDays();

    std::string str = "";
    if (days) {
        auto day_str = localization->GetString(LSTR_DAYS);
        if (days <= 1) day_str = localization->GetString(LSTR_DAY_CAPITALIZED);

        str += fmt::format("{} {} ", days, day_str);
    }

    if (hours) {
        auto hour_str = localization->GetString(LSTR_HOURS);
        if (hours <= 1) hour_str = localization->GetString(LSTR_HOUR);

        str += fmt::format("{} {} ", hours, hour_str);
    }

    if (minutes && !days) {
        auto minute_str = localization->GetString(LSTR_MINUTES);
        if (minutes <= 1) minute_str = localization->GetString(LSTR_MINUTE);

        str += fmt::format("{} {} ", minutes, minute_str);
    }

    if (seconds && !hours) {
        auto seconds_str = localization->GetString(LSTR_SECONDS);
        if (seconds <= 1) seconds_str = localization->GetString(LSTR_SECOND);

        str += fmt::format("{} {} ", seconds, seconds_str);
    }

    return str;
}

//----- (004B1854) --------------------------------------------------------
void GUIWindow::DrawShops_next_generation_time_string(GameTime time) {
    auto str = MakeDateTimeString(time);
    this->DrawTitleText(pFontArrus, 0, (212 - pFontArrus->CalcTextHeight(str, this->uFrameWidth, 0)) / 2 + 101, colorTable.PaleCanary, localization->GetString(LSTR_PLEASE_TRY_BACK_IN) + str, 3);
}

//----- (0044D406) --------------------------------------------------------
void GUIWindow::DrawTitleText(GUIFont *pFont, int horizontalMargin, int verticalMargin, Color color, const std::string &text, int lineSpacing) {
    int width = this->uFrameWidth - horizontalMargin;
    std::string resString = pFont->FitTextInAWindow(text, this->uFrameWidth, horizontalMargin);
    std::istringstream stream(resString);
    std::string line;
    int x = horizontalMargin + this->uFrameX;
    int y = verticalMargin + this->uFrameY;
    while (std::getline(stream, line)) {
        int x_offset = pFont->AlignText_Center(width, line);
        pFont->DrawTextLine(line, color, {x + x_offset, y}, render->GetRenderDimensions().w);
        y += pFont->GetHeight() - lineSpacing;
    }
}

//----- (0044CE08) --------------------------------------------------------
void GUIWindow::DrawText(GUIFont *font, Pointi position, Color color, const std::string &text, int maxHeight, Color shadowColor) {
    font->DrawText(this, position, color, text, maxHeight, shadowColor);
}

//----- (0044CB4F) --------------------------------------------------------
int GUIWindow::DrawTextInRect(GUIFont *pFont, Pointi position,
                              Color uColor, const std::string &text, int rect_width,
                              int reverse_text) {
    return pFont->DrawTextInRect(this, position, uColor, text, rect_width, reverse_text);
}

GUIButton *GUIWindow::CreateButton(Pointi position, Sizei dimensions,
                                   int uButtonType, int uData, UIMessageType msg,
                                   unsigned int msg_param, InputAction action,
                                   const std::string &label,
                                   const std::vector<GraphicsImage *> &textures) {
    GUIButton *pButton = new GUIButton();

    pButton->pParent = this;
    pButton->uWidth = dimensions.w;
    pButton->uHeight = dimensions.h;

    if (uButtonType == 2 && !dimensions.h) {
        pButton->uHeight = dimensions.w;
    }

    pButton->uButtonType = uButtonType;
    pButton->uX = position.x + this->uFrameX;
    pButton->uY = position.y + this->uFrameY;
    pButton->uZ = pButton->uX + dimensions.w;
    pButton->uW = pButton->uY + dimensions.h;
    pButton->field_2C_is_pushed = false;
    pButton->uData = uData;
    pButton->msg = msg;
    pButton->msg_param = msg_param;
    pButton->action = action;
    pButton->sLabel = label;
    pButton->vTextures = textures;

    vButtons.push_back(pButton);

    return pButton;
}

GUIButton *GUIWindow::CreateButton(std::string id, Pointi position, Sizei dimensions, int uButtonType, int uData,
                        UIMessageType msg, unsigned int msg_param, InputAction action, const std::string &label,
                        const std::vector<GraphicsImage *> &textures) {
    GUIButton *result = CreateButton(position, dimensions, uButtonType, uData, msg, msg_param, action, label, textures);
    result->id = std::move(id);
    return result;
}

bool GUIWindow::Contains(unsigned int x, unsigned int y) {
    return (x >= uFrameX && x <= uFrameZ && y >= uFrameY && y <= uFrameW);
}

void GUIWindow::InitializeGUI() {
    SetUserInterface(PartyAlignment::PartyAlignment_Neutral, false);
    MainMenuUI_LoadFontsAndSomeStuff();
}

void GUIWindow::DrawFlashingInputCursor(int uX, int uY, GUIFont *a2) {
    // TODO(pskelton): check tickcount usage here
    if (platform->tickCount() % 1000 > 500) {
        DrawText(a2, {uX, uY}, colorTable.White, "_");
    }
}

GUIWindow::GUIWindow() : eWindowType(WINDOW_null) {
    this->mouse = EngineIocContainer::ResolveMouse();
    this->log = EngineIocContainer::ResolveLogger();
}

GUIWindow::GUIWindow(WindowType windowType, Pointi position, Sizei dimensions, WindowData wData, const std::string &hint): eWindowType(windowType) {
    this->mouse = EngineIocContainer::ResolveMouse();
    this->log = EngineIocContainer::ResolveLogger();

    log->info("New window: {}", toString(windowType));
    lWindowList.push_front(this);
    this->uFrameWidth = dimensions.w;
    this->uFrameHeight = dimensions.h;

    this->uFrameX = position.x;
    this->uFrameY = position.y;
    this->uFrameZ = position.x + dimensions.w - 1;
    this->uFrameW = position.y + dimensions.h - 1;

    this->wData = wData;
    this->sHint = hint;

    this->receives_keyboard_input = false;
}

//----- (004B3EF0) --------------------------------------------------------
void DrawJoinGuildWindow(GUILD_ID guild_id) {
    uDialogueType = DIALOGUE_81_join_guild;
    current_npc_text = pNPCTopics[guild_id + 99].pText;
    GetJoinGuildDialogueOption(guild_id);
    pDialogueWindow->Release();
    pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, {render->GetRenderDimensions().w, 350}, guild_id);
    pBtn_ExitCancel = pDialogueWindow->CreateButton({471, 445}, {169, 35}, 1, 0, UIMSG_Escape, 0, InputAction::Invalid,
        localization->GetString(LSTR_CANCEL), { ui_exit_cancel_button_background }
    );
    pDialogueWindow->CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_HouseScreenClick, 0, InputAction::Invalid, "");
    pDialogueWindow->CreateButton({480, 160}, {140, 30}, 1, 0, UIMSG_ClickNPCTopic, DIALOGUE_82_join_guild, InputAction::Invalid,
        localization->GetString(LSTR_JOIN));
    pDialogueWindow->_41D08F_set_keyboard_control_group(1, 1, 0, 2);
    dialog_menu_id = DIALOGUE_OTHER;
}

void DialogueEnding() {
    sDialogue_SpeakingActorNPC_ID = 0;
    if (pDialogueWindow) {
        pDialogueWindow->Release();
    }
    pDialogueWindow = nullptr;
    pMiscTimer->Resume();
    pEventTimer->Resume();
}

void OnButtonClick::Update() {
    if (bPlaySound) {
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    }
    GUIButton *pButton = static_cast<GUIButton *>(wData.ptr);
    render->DrawTextureNew(uFrameX / 640.0f, uFrameY / 480.0f, pButton->vTextures[0]);
    if (!sHint.empty()) {
        pButton->DrawLabel(sHint, pFontCreate, colorTable.White);
    }
    Release();
}

void OnButtonClick2::Update() {
    if (bPlaySound) {
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    }
    GUIButton *pButton = static_cast<GUIButton *>(wData.ptr);
    Sizei renDims = render->GetRenderDimensions();
    if (pButton->uX >= 0 && pButton->uX <= renDims.w) {
        if (pButton->uY >= 0 && pButton->uY <= renDims.h) {
            render->DrawTextureNew(uFrameX / 640.0f, uFrameY / 480.0f, pButton->vTextures[0]);
        }
    }
    if (!sHint.empty()) {
        pButton->DrawLabel(sHint, pFontCreate, colorTable.White);
    }
    Release();
}

void OnButtonClick3::Update() {
    pAudioPlayer->playUISound(SOUND_StartMainChoice02);

    GUIButton *pButton = static_cast<GUIButton *>(wData.ptr);
    render->DrawTextureNew(uFrameX / 640.0f, uFrameY / 480.0f, pButton->vTextures[1]);
    if (!sHint.empty()) {
        pButton->DrawLabel(sHint, pFontCreate, colorTable.White);
    }
    Release();
}

void OnButtonClick4::Update() {
    if (!sHint.empty()) {
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    }
    GUIButton *pButton = static_cast<GUIButton *>(wData.ptr);
    render->DrawTextureNew(uFrameX / 640.0f, uFrameY / 480.0f, pButton->vTextures[1]);

    Release();
}

void OnSaveLoad::Update() {
    if (!sHint.empty()) {
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    }
    GUIButton *pButton = static_cast<GUIButton *>(wData.ptr);
    render->DrawTextureNew(uFrameX / 640.0f, uFrameY / 480.0f, pButton->vTextures[0]);
    if (!sHint.empty()) {
        pButton->DrawLabel(sHint, pFontCreate, colorTable.White);
    }
    Release();

    if (current_screen_type == CURRENT_SCREEN::SCREEN_SAVEGAME) {
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_SaveGame, 0, 0);
    } else {
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_LoadGame, 0, 0);
    }
}

void OnCancel::Update() {
    if (sHint.empty()) {
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    }
    GUIButton *pGUIButton = static_cast<GUIButton *>(wData.ptr);
    render->DrawTextureNew(uFrameX / 640.0f, uFrameY / 480.0f, pGUIButton->vTextures[0]);
    if (!sHint.empty()) {
        pGUIButton->DrawLabel(sHint, pFontCreate, colorTable.White);
    }
    Release();

    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
}

void OnCancel2::Update() {
    if (!sHint.empty()) {
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    }
    GUIButton *pButton = static_cast<GUIButton *>(wData.ptr);
    render->DrawTextureNew(uFrameX / 640.0f, uFrameY / 480.0f, pButton->vTextures[1]);
    if (!sHint.empty()) {
        pButton->DrawLabel(sHint, pFontCreate, colorTable.White);
    }
    Release();

    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
}

void OnCancel3::Update() {
    if (!sHint.empty()) {
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    }

    GUIButton *pButton = static_cast<GUIButton *>(wData.ptr);
    render->DrawTextureNew(uFrameX / 640.0f, uFrameY / 480.0f, pButton->vTextures[0]);
    if (!sHint.empty()) {
        pButton->DrawLabel(sHint, pFontCreate, colorTable.White);
    }
    Release();

    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
}

void GUI_UpdateWindows() {
    if (GetCurrentMenuID() != MENU_CREATEPARTY) {
        extern bool UI_OnKeyDown(PlatformKey key);
        UI_OnKeyDown(PlatformKey::KEY_PAGEDOWN); // hack to highlight dialog options under mouse cursor
    }

    // should never activte this - gameui window should always be open
    if (lWindowList.size() < 1) __debugbreak();

    std::list<GUIWindow *> tmpWindowList(lWindowList);
    tmpWindowList.reverse();  // new windows are push front - but front should be drawn last?? testing
    for (GUIWindow *pWindow : tmpWindowList) {
        pWindow->Update();
    }

    if (GetCurrentMenuID() == -1) {
        GameUI_DrawFoodAndGold();
    }

    if (isHoldingMouseRightButton()) {
        std::shared_ptr<Mouse> mouse = EngineIocContainer::ResolveMouse();
        UI_OnMouseRightClick(mouse->GetCursorPos().x, mouse->GetCursorPos().y);
    }
}

//----- (004226EF) --------------------------------------------------------
void SetUserInterface(PartyAlignment align, bool bReplace) {
    extern void set_default_ui_skin();
    set_default_ui_skin();

    if (!parchment) {
        parchment = assets->getImage_ColorKey("parchment");
    }

    if (align == PartyAlignment::PartyAlignment_Evil) {
        if (bReplace) {
            game_ui_rightframe = assets->getImage_PCXFromIconsLOD("ib-r-C.pcx");
            game_ui_bottomframe =
                assets->getImage_PCXFromIconsLOD("ib-b-C.pcx");
            game_ui_topframe = assets->getImage_PCXFromIconsLOD("ib-t-C.pcx");
            game_ui_leftframe = assets->getImage_PCXFromIconsLOD("ib-l-C.pcx");
            game_ui_statusbar =
                assets->getImage_PCXFromIconsLOD("IB-Foot-c.pcx");

            game_ui_right_panel_frame = assets->getImage_Alpha("ib-mb-C");

            game_ui_minimap_frame = assets->getImage_Alpha("ib-autmask-c");
            game_ui_minimap_compass =
                assets->getImage_ColorKey("IB-COMP-C");

            game_ui_player_alert_green = assets->getImage_Alpha("IB-InitG-c");
            game_ui_player_alert_yellow = assets->getImage_Alpha("IB-InitY-c");
            game_ui_player_alert_red = assets->getImage_Alpha("IB-InitR-c");

            ui_btn_npc_left = assets->getImage_ColorKey("IB-NPCLD-C");
            ui_btn_npc_right = assets->getImage_ColorKey("IB-NPCRD-C");
            game_ui_btn_zoomin = assets->getImage_Alpha("ib-autout-C");
            game_ui_btn_zoomout = assets->getImage_Alpha("ib-autin-C");
            game_ui_player_selection_frame = assets->getImage_ColorKey("IB-selec-C");
            game_ui_btn_cast = assets->getImage_Alpha("ib-m1d-c");
            game_ui_btn_rest = assets->getImage_Alpha("ib-m2d-c");
            game_ui_btn_quickref = assets->getImage_Alpha("ib-m3d-c");
            game_ui_btn_settings = assets->getImage_Alpha("ib-m4d-c");

            game_ui_playerbuff_bless = assets->getImage_ColorKey("isg-01-c");
            game_ui_playerbuff_preservation = assets->getImage_ColorKey("isg-02-c");
            game_ui_playerbuff_hammerhands = assets->getImage_ColorKey("isg-03-c");
            game_ui_playerbuff_pain_reflection = assets->getImage_ColorKey("isg-04-c");

            pUIAnim_WizardEye->icon = pIconsFrameTable->GetIcon("wizeyeC");
            pIconsFrameTable->InitializeAnimation(pUIAnim_WizardEye->icon->id);

            pUIAnum_Torchlight->icon = pIconsFrameTable->GetIcon("torchC");
            pIconsFrameTable->InitializeAnimation(pUIAnum_Torchlight->icon->id);

            ui_exit_cancel_button_background = assets->getImage_ColorKey("ib-bcu-c");

            game_ui_evtnpc = assets->getImage_ColorKey("evtnpc-c");
            ui_character_inventory_background = assets->getImage_ColorKey("fr_inven-c");
            messagebox_corner_y = assets->getImage_Alpha("cornr_ll-c");
            messagebox_corner_w = assets->getImage_Alpha("cornr_lr-c");
            messagebox_corner_x = assets->getImage_Alpha("cornr_ul-c");
            messagebox_corner_z = assets->getImage_Alpha("cornr_ur-c");
            messagebox_border_bottom = assets->getImage_Alpha("edge_btm-c");
            messagebox_border_left = assets->getImage_Alpha("edge_lf-c");
            messagebox_border_right = assets->getImage_Alpha("edge_rt-c");
            messagebox_border_top = assets->getImage_Alpha("edge_top-c");
            _591428_endcap = assets->getImage_ColorKey("endcap-c");
        } else {
            game_ui_rightframe = assets->getImage_PCXFromIconsLOD("ib-r-C.pcx");
            game_ui_bottomframe =
                assets->getImage_PCXFromIconsLOD("ib-b-c.pcx");
            game_ui_topframe = assets->getImage_PCXFromIconsLOD("ib-t-C.pcx");
            game_ui_leftframe = assets->getImage_PCXFromIconsLOD("ib-l-C.pcx");
            game_ui_statusbar =
                assets->getImage_PCXFromIconsLOD("IB-Foot-c.pcx");

            game_ui_right_panel_frame = assets->getImage_Alpha("ib-mb-C");
            game_ui_minimap_frame = assets->getImage_Alpha("ib-autmask-c");
            game_ui_minimap_compass = assets->getImage_ColorKey("IB-COMP-C");
            game_ui_player_alert_green = assets->getImage_Alpha("IB-InitG-c");
            game_ui_player_alert_yellow = assets->getImage_Alpha("IB-InitY-c");
            game_ui_player_alert_red = assets->getImage_Alpha("IB-InitR-c");

            ui_btn_npc_left = assets->getImage_ColorKey("IB-NPCLD-C");
            ui_btn_npc_right = assets->getImage_ColorKey("IB-NPCRD-C");
            game_ui_btn_zoomin = assets->getImage_ColorKey("ib-autout-C");
            game_ui_btn_zoomout = assets->getImage_ColorKey("ib-autin-C");
            game_ui_player_selection_frame = assets->getImage_ColorKey("IB-selec-C");
            game_ui_btn_cast = assets->getImage_Alpha("ib-m1d-c");
            game_ui_btn_rest = assets->getImage_Alpha("ib-m2d-c");
            game_ui_btn_quickref = assets->getImage_Alpha("ib-m3d-c");
            game_ui_btn_settings = assets->getImage_Alpha("ib-m4d-c");
            ui_exit_cancel_button_background = assets->getImage_ColorKey("ib-bcu-c");

            game_ui_playerbuff_bless = assets->getImage_ColorKey("isg-01-c");
            game_ui_playerbuff_preservation = assets->getImage_ColorKey("isg-02-c");
            game_ui_playerbuff_hammerhands = assets->getImage_ColorKey("isg-03-c");
            game_ui_playerbuff_pain_reflection = assets->getImage_ColorKey("isg-04-c");

            game_ui_evtnpc = assets->getImage_ColorKey("evtnpc-c");
            ui_character_inventory_background = assets->getImage_ColorKey("fr_inven");

            pUIAnim_WizardEye->icon = pIconsFrameTable->GetIcon("wizeyeC");
            pIconsFrameTable->InitializeAnimation(pUIAnim_WizardEye->icon->id);

            pUIAnum_Torchlight->icon = pIconsFrameTable->GetIcon("torchC");
            pIconsFrameTable->InitializeAnimation(pUIAnum_Torchlight->icon->id);
        }
        uGameUIFontMain = colorTable.MediumRed;
        uGameUIFontShadow = colorTable.Diesel;
    } else if (align == PartyAlignment::PartyAlignment_Neutral) {
        if (bReplace) {
            game_ui_rightframe = assets->getImage_PCXFromIconsLOD("ib-r-a.pcx");
            game_ui_bottomframe =
                assets->getImage_PCXFromIconsLOD("ib-b-a.pcx");
            game_ui_topframe = assets->getImage_PCXFromIconsLOD("ib-t-a.pcx");
            game_ui_leftframe = assets->getImage_PCXFromIconsLOD("ib-l-a.pcx");
            game_ui_statusbar =
                assets->getImage_PCXFromIconsLOD("IB-Foot-a.pcx");

            game_ui_right_panel_frame = assets->getImage_Alpha("ib-mb-a");
            game_ui_minimap_frame = assets->getImage_Alpha("ib-autmask-a");
            game_ui_minimap_compass = assets->getImage_ColorKey("IB-COMP-a");
            game_ui_player_alert_green = assets->getImage_Alpha("IB-InitG-a");
            game_ui_player_alert_yellow = assets->getImage_Alpha("IB-InitY-a");
            game_ui_player_alert_red = assets->getImage_Alpha("IB-InitR-a");

            ui_btn_npc_left = assets->getImage_ColorKey("IB-NPCLD-a");
            ui_btn_npc_right = assets->getImage_ColorKey("IB-NPCRD-a");
            game_ui_btn_zoomin = assets->getImage_ColorKey("ib-autout-a");
            game_ui_btn_zoomout = assets->getImage_ColorKey("ib-autin-a");
            game_ui_player_selection_frame = assets->getImage_ColorKey("IB-selec-a");
            game_ui_btn_cast = assets->getImage_Alpha("ib-m1d-a");
            game_ui_btn_rest = assets->getImage_Alpha("ib-m2d-a");
            game_ui_btn_quickref = assets->getImage_Alpha("ib-m3d-a");
            game_ui_btn_settings = assets->getImage_Alpha("ib-m4d-a");

            game_ui_playerbuff_bless = assets->getImage_ColorKey("isg-01-a");
            game_ui_playerbuff_preservation = assets->getImage_ColorKey("isg-02-a");
            game_ui_playerbuff_hammerhands = assets->getImage_ColorKey("isg-03-a");
            game_ui_playerbuff_pain_reflection = assets->getImage_ColorKey("isg-04-a");

            pUIAnim_WizardEye->icon = pIconsFrameTable->GetIcon("wizeyeA");
            pIconsFrameTable->InitializeAnimation(pUIAnim_WizardEye->icon->id);
            pUIAnum_Torchlight->icon = pIconsFrameTable->GetIcon("torchA");
            pIconsFrameTable->InitializeAnimation(pUIAnum_Torchlight->icon->id);

            ui_exit_cancel_button_background = assets->getImage_ColorKey("ib-bcu-a");

            game_ui_evtnpc = assets->getImage_ColorKey("evtnpc");
            ui_character_inventory_background = assets->getImage_ColorKey("fr_inven");
            messagebox_corner_y = assets->getImage_Alpha("cornr_ll");
            messagebox_corner_w = assets->getImage_Alpha("cornr_lr");
            messagebox_corner_x = assets->getImage_Alpha("cornr_ul");
            messagebox_corner_z = assets->getImage_Alpha("cornr_ur");
            messagebox_border_bottom = assets->getImage_Alpha("edge_btm");
            messagebox_border_left = assets->getImage_Alpha("edge_lf");
            messagebox_border_right = assets->getImage_Alpha("edge_rt");
            messagebox_border_top = assets->getImage_Alpha("edge_top");
            _591428_endcap = assets->getImage_ColorKey("endcap");
        } else {
            game_ui_rightframe = assets->getImage_PCXFromIconsLOD("ib-r-A.pcx");
            game_ui_bottomframe =
                assets->getImage_PCXFromIconsLOD("ib-b-A.pcx");
            game_ui_topframe = assets->getImage_PCXFromIconsLOD("ib-t-A.pcx");
            game_ui_leftframe = assets->getImage_PCXFromIconsLOD("ib-l-A.pcx");
            game_ui_statusbar =
                assets->getImage_PCXFromIconsLOD("IB-Foot-a.pcx");

            game_ui_right_panel_frame = assets->getImage_Alpha("ib-mb-A");
            game_ui_minimap_frame = assets->getImage_Alpha("ib-autmask-a");
            game_ui_minimap_compass = assets->getImage_ColorKey("IB-COMP-A");
            game_ui_player_alert_green = assets->getImage_Alpha("IB-InitG-a");
            game_ui_player_alert_yellow = assets->getImage_Alpha("IB-InitY-a");
            game_ui_player_alert_red = assets->getImage_Alpha("IB-InitR-a");

            ui_btn_npc_left = assets->getImage_ColorKey("IB-NPCLD-A");
            ui_btn_npc_right = assets->getImage_ColorKey("IB-NPCRD-A");
            game_ui_player_selection_frame = assets->getImage_ColorKey("IB-selec-A");
            game_ui_btn_cast = assets->getImage_Alpha("ib-m1d-a");
            game_ui_btn_rest = assets->getImage_Alpha("ib-m2d-a");
            game_ui_btn_quickref = assets->getImage_Alpha("ib-m3d-a");
            game_ui_btn_settings = assets->getImage_Alpha("ib-m4d-a");
            game_ui_btn_zoomin = assets->getImage_ColorKey("ib-autout-a");
            game_ui_btn_zoomout = assets->getImage_ColorKey("ib-autin-a");
            ui_exit_cancel_button_background = assets->getImage_ColorKey("ib-bcu-a");

            game_ui_playerbuff_bless = assets->getImage_ColorKey("isg-01-a");
            game_ui_playerbuff_preservation = assets->getImage_ColorKey("isg-02-a");
            game_ui_playerbuff_hammerhands = assets->getImage_ColorKey("isg-03-a");
            game_ui_playerbuff_pain_reflection = assets->getImage_ColorKey("isg-04-a");

            game_ui_evtnpc = assets->getImage_ColorKey("evtnpc");
            ui_character_inventory_background = assets->getImage_ColorKey("fr_inven");

            pUIAnim_WizardEye->icon = pIconsFrameTable->GetIcon("wizeyeA");
            pIconsFrameTable->InitializeAnimation(pUIAnim_WizardEye->icon->id);
            pUIAnum_Torchlight->icon = pIconsFrameTable->GetIcon("torchA");
            pIconsFrameTable->InitializeAnimation(pUIAnum_Torchlight->icon->id);

            messagebox_corner_y = assets->getImage_Alpha("cornr_ll");
            messagebox_corner_w = assets->getImage_Alpha("cornr_lr");
            messagebox_corner_x = assets->getImage_Alpha("cornr_ul");
            messagebox_corner_z = assets->getImage_Alpha("cornr_ur");
            messagebox_border_bottom = assets->getImage_Alpha("edge_btm");
            messagebox_border_left = assets->getImage_Alpha("edge_lf");
            messagebox_border_right = assets->getImage_Alpha("edge_rt");
            messagebox_border_top = assets->getImage_Alpha("edge_top");
            _591428_endcap = assets->getImage_ColorKey("endcap");
        }
        uGameUIFontMain = colorTable.Diesel;
        uGameUIFontShadow = colorTable.StarkWhite;
    } else if (align == PartyAlignment::PartyAlignment_Good) {
        if (bReplace) {
            game_ui_rightframe = assets->getImage_PCXFromIconsLOD("ib-r-B.pcx");
            game_ui_bottomframe =
                assets->getImage_PCXFromIconsLOD("ib-b-B.pcx");
            game_ui_topframe = assets->getImage_PCXFromIconsLOD("ib-t-B.pcx");
            game_ui_leftframe = assets->getImage_PCXFromIconsLOD("ib-l-B.pcx");
            game_ui_statusbar =
                assets->getImage_PCXFromIconsLOD("IB-Foot-b.pcx");

            game_ui_right_panel_frame = assets->getImage_Alpha("ib-mb-B");
            game_ui_minimap_frame = assets->getImage_Alpha("ib-autmask-b");
            game_ui_minimap_compass = assets->getImage_ColorKey("IB-COMP-B");
            game_ui_player_alert_green = assets->getImage_Alpha("IB-InitG-b");
            game_ui_player_alert_yellow = assets->getImage_Alpha("IB-InitY-b");
            game_ui_player_alert_red = assets->getImage_Alpha("IB-InitR-b");

            ui_btn_npc_left = assets->getImage_ColorKey("IB-NPCLD-B");
            ui_btn_npc_right = assets->getImage_ColorKey("IB-NPCRD-B");
            game_ui_btn_zoomin = assets->getImage_ColorKey("ib-autout-B");
            game_ui_btn_zoomout = assets->getImage_ColorKey("ib-autin-B");
            game_ui_player_selection_frame = assets->getImage_ColorKey("IB-selec-B");
            game_ui_btn_cast = assets->getImage_Alpha("ib-m1d-b");
            game_ui_btn_rest = assets->getImage_Alpha("ib-m2d-b");
            game_ui_btn_quickref = assets->getImage_Alpha("ib-m3d-b");
            game_ui_btn_settings = assets->getImage_Alpha("ib-m4d-b");

            game_ui_playerbuff_bless = assets->getImage_ColorKey("isg-01-b");
            game_ui_playerbuff_preservation = assets->getImage_ColorKey("isg-02-b");
            game_ui_playerbuff_hammerhands = assets->getImage_ColorKey("isg-03-b");
            game_ui_playerbuff_pain_reflection = assets->getImage_ColorKey("isg-04-b");

            pUIAnim_WizardEye->icon = pIconsFrameTable->GetIcon("wizeyeB");
            pIconsFrameTable->InitializeAnimation(pUIAnim_WizardEye->icon->id);
            pUIAnum_Torchlight->icon = pIconsFrameTable->GetIcon("torchB");
            pIconsFrameTable->InitializeAnimation(pUIAnum_Torchlight->icon->id);

            ui_exit_cancel_button_background = assets->getImage_ColorKey("ib-bcu-b");
            game_ui_evtnpc = assets->getImage_ColorKey("evtnpc-b");
            ui_character_inventory_background = assets->getImage_ColorKey("fr_inven-b");
            messagebox_corner_y = assets->getImage_Alpha("cornr_ll-b");
            messagebox_corner_w = assets->getImage_Alpha("cornr_lr-b");
            messagebox_corner_x = assets->getImage_Alpha("cornr_ul-b");
            messagebox_corner_z = assets->getImage_Alpha("cornr_ur-b");
            messagebox_border_bottom = assets->getImage_Alpha("edge_btm-b");
            messagebox_border_left = assets->getImage_Alpha("edge_lf-b");
            messagebox_border_right = assets->getImage_Alpha("edge_rt-b");
            messagebox_border_top = assets->getImage_Alpha("edge_top-b");
            _591428_endcap = assets->getImage_ColorKey("endcap-b");
        }
        uGameUIFontMain = colorTable.MediumBlue;
        uGameUIFontShadow = colorTable.White;
    } else {
        Error("Invalid alignment type: %u", align);
    }
}

void DrawBuff_remaining_time_string(int uY, GUIWindow *window, GameTime remaining_time, GUIFont *Font) {
    window->DrawText(Font, {32, uY}, colorTable.White, "\r020" + MakeDateTimeString(remaining_time));
}

void GUIMessageQueue::AddMessageImpl(UIMessageType msg, int param,
    unsigned int a4, const char *file,
    int line) {
    // logger->Warning("{} @ ({} {})", UIMessage2String(msg), file, line);
    GUIMessage message;
    message.eType = msg;
    message.param = param;
    message.field_8 = a4;
    message.file = file;
    message.line = line;
    qMessages.push(message);
}

bool isHoldingMouseRightButton() {
    return holdingMouseRightButton;
}

Color GetSkillColor(CharacterClassType uPlayerClass, CharacterSkillType uPlayerSkillType, CharacterSkillMastery skill_mastery) {
    if (skillMaxMasteryPerClass[uPlayerClass][uPlayerSkillType] >= skill_mastery) {
        return ui_character_skillinfo_can_learn;
    }
    for (CharacterClassType promotionClass : getClassPromotions(uPlayerClass)) {
        if (skillMaxMasteryPerClass[promotionClass][uPlayerSkillType] >= skill_mastery) {
            return ui_character_skillinfo_can_learn_gm;
        }
    }
    return ui_character_skillinfo_cant_learn;
}

void ClickNPCTopic(DIALOGUE_TYPE topic) {
    int pEventNumber;  // ecx@8
    char *v12;         // eax@53
    char *v13;         // eax@56
    char *v14;         // eax@57
    char *v15;         // eax@58
    int pPrice;        // ecx@70

    uDialogueType = (DIALOGUE_TYPE)(topic + 1);
    NPCData *pCurrentNPCInfo = HouseNPCData[pDialogueNPCCount - ((dword_591080 != 0) ? 1 : 0)];  //- 1
    if (topic <= DIALOGUE_SCRIPTED_LINE_6) {
        switch (topic) {
        case DIALOGUE_13_hiring_related:
            current_npc_text = BuildDialogueString(
                pNPCStats->pProfessions[pCurrentNPCInfo->profession].pJoinText,
                pParty->activeCharacterIndex() - 1, 0, HOUSE_INVALID, 0);
            NPCHireableDialogPrepare();
            dialogue_show_profession_details = false;
            BackToHouseMenu();
            return;
        case DIALOGUE_SCRIPTED_LINE_1:
            pEventNumber = pCurrentNPCInfo->dialogue_1_evt_id;
            break;
        case DIALOGUE_SCRIPTED_LINE_2:
            pEventNumber = pCurrentNPCInfo->dialogue_2_evt_id;
            break;
        case DIALOGUE_SCRIPTED_LINE_3:
            pEventNumber = pCurrentNPCInfo->dialogue_3_evt_id;
            break;
        case DIALOGUE_SCRIPTED_LINE_4:
            pEventNumber = pCurrentNPCInfo->dialogue_4_evt_id;
            break;
        case DIALOGUE_SCRIPTED_LINE_5:
            pEventNumber = pCurrentNPCInfo->dialogue_5_evt_id;
            break;
        case DIALOGUE_SCRIPTED_LINE_6:
            pEventNumber = pCurrentNPCInfo->dialogue_6_evt_id;
            break;
        default:
            BackToHouseMenu();
            return;
        }

        if (pEventNumber < 200 || pEventNumber > 310) {
            if (pEventNumber < 400 || pEventNumber > 410) {
                if (pEventNumber == 139) {
                    OracleDialogue();
                } else {
                    if (pEventNumber == 311) {
                        // TODO(Nik-RE-dev): event 311 belongs to one of the teleports in Bracada
                        __debugbreak();
                        //openBountyHuntingDialogue();
                    } else {
                        current_npc_text.clear();
                        activeLevelDecoration = (LevelDecoration *)1;
                        eventProcessor(pEventNumber, 0, 1);
                        activeLevelDecoration = nullptr;
                    }
                }
            } else {
                _dword_F8B1D8_last_npc_topic_menu = topic;
                DrawJoinGuildWindow((GUILD_ID)(pEventNumber - 400));
            }
        } else {
            _4B3FE5_training_dialogue(pEventNumber);
        }
        BackToHouseMenu();
        return;
    }
    if (topic != DIALOGUE_HIRE_FIRE) {
        if (topic == DIALOGUE_PROFESSION_DETAILS) {
            // uBoxHeight = pCurrentNPCInfo->uProfession;
            __debugbreak();  // probably hirelings found in buildings, not
                             // present in MM7, changed
                             // "pCurrentNPCInfo->uProfession - 1" to
                             // "pCurrentNPCInfo->uProfession", have to check in
                             // other versions whether it's ok
            if (dialogue_show_profession_details) {
                current_npc_text = BuildDialogueString(
                    pNPCStats->pProfessions[pCurrentNPCInfo->profession].pJoinText,
                    pParty->activeCharacterIndex() - 1, 0, HOUSE_INVALID, 0);
            } else {
                current_npc_text = BuildDialogueString(
                    pNPCStats->pProfessions[pCurrentNPCInfo->profession].pBenefits,
                    pParty->activeCharacterIndex() - 1, 0, HOUSE_INVALID, 0);
            }
            dialogue_show_profession_details = ~dialogue_show_profession_details;
        } else {
            if (topic == DIALOGUE_79_mastery_teacher) {
                if (guild_membership_approved) {
                    pParty->TakeGold(gold_transaction_amount);
                    if (pParty->hasActiveCharacter()) {
                        pParty->activeCharacter().SetSkillMastery(dword_F8B1AC_skill_being_taught, dword_F8B1B0_MasteryBeingTaught);
                        pParty->activeCharacter().playReaction(SPEECH_SKILL_MASTERY_INC);
                    }
                    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                }
            } else {
                if (topic == DIALOGUE_82_join_guild && guild_membership_approved) {
                    // join guild
                    pParty->TakeGold(gold_transaction_amount, true);
                    for (Character &player : pParty->pCharacters)
                        player.SetVariable(VAR_Award, dword_F8B1AC_award_bit_number);

                    switch (_dword_F8B1D8_last_npc_topic_menu) {
                    case DIALOGUE_SCRIPTED_LINE_1:
                        pEventNumber = pCurrentNPCInfo->dialogue_1_evt_id;
                        if (pEventNumber >= 400 && pEventNumber <= 416)
                            pCurrentNPCInfo->dialogue_1_evt_id = 0;
                        break;
                    case DIALOGUE_SCRIPTED_LINE_2:
                        pEventNumber = pCurrentNPCInfo->dialogue_2_evt_id;
                        if (pEventNumber >= 400 && pEventNumber <= 416)
                            pCurrentNPCInfo->dialogue_2_evt_id = 0;
                        break;
                    case DIALOGUE_SCRIPTED_LINE_3:
                        pEventNumber = pCurrentNPCInfo->dialogue_3_evt_id;
                        if (pEventNumber >= 400 && pEventNumber <= 416)
                            pCurrentNPCInfo->dialogue_3_evt_id = 0;
                        break;
                    case DIALOGUE_SCRIPTED_LINE_4:
                        pEventNumber = pCurrentNPCInfo->dialogue_4_evt_id;
                        if (pEventNumber >= 400 && pEventNumber <= 416)
                            pCurrentNPCInfo->dialogue_4_evt_id = 0;
                        break;
                    case DIALOGUE_SCRIPTED_LINE_5:
                        pEventNumber = pCurrentNPCInfo->dialogue_5_evt_id;
                        if (pEventNumber >= 400 && pEventNumber <= 416)
                            pCurrentNPCInfo->dialogue_5_evt_id = 0;
                        break;
                    case DIALOGUE_SCRIPTED_LINE_6:
                        pEventNumber = pCurrentNPCInfo->dialogue_6_evt_id;
                        if (pEventNumber >= 400 && pEventNumber <= 416)
                            pCurrentNPCInfo->dialogue_6_evt_id = 0;
                        break;
                    default:
                        break;
                    }
                    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                    if (pParty->hasActiveCharacter()) {
                        pParty->activeCharacter().playReaction(SPEECH_JOINED_GUILD);
                        BackToHouseMenu();
                        return;
                    }
                }
            }
        }
        BackToHouseMenu();
        return;
    }

    if (!pParty->pHirelings[0].pName.empty() && !pParty->pHirelings[1].pName.empty()) {
        GameUI_SetStatusBar(LSTR_HIRE_NO_ROOM);
        BackToHouseMenu();
        return;
    }

    if (pCurrentNPCInfo->profession != Burglar) {  // burglars have no hiring price
        __debugbreak();  // probably hirelings found in buildings, not present
                         // in MM7, changed "pCurrentNPCInfo->uProfession - 1"
                         // to "pCurrentNPCInfo->uProfession", have to check in
                         // other versions whether it's ok
        pPrice =
            pNPCStats->pProfessions[pCurrentNPCInfo->profession].uHirePrice;
        if (pParty->GetGold() < (unsigned int)pPrice) {
            GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
            dialogue_show_profession_details = false;
            uDialogueType = DIALOGUE_13_hiring_related;
            current_npc_text = BuildDialogueString(
                pNPCStats->pProfessions[pCurrentNPCInfo->profession].pJoinText,
                pParty->activeCharacterIndex() - 1, 0, HOUSE_INVALID, 0);
            if (pParty->hasActiveCharacter()) {
                pParty->activeCharacter().playReaction(SPEECH_NOT_ENOUGH_GOLD);
            }
            GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
            BackToHouseMenu();
            return;
        } else {
            pParty->TakeGold(pPrice);
        }
    }

    pCurrentNPCInfo->uFlags |= NPC_HIRED;
    pParty->hirelingScrollPosition = 0;
    pParty->CountHirelings();
    if (!pParty->pHirelings[0].pName.empty()) {
        pParty->pHirelings[1] = *pCurrentNPCInfo;
        pParty->pHireling2Name = pCurrentNPCInfo->pName;
    } else {
        pParty->pHirelings[0] = *pCurrentNPCInfo;
        pParty->pHireling1Name = pCurrentNPCInfo->pName;
    }
    pParty->hirelingScrollPosition = 0;
    pParty->CountHirelings();
    PrepareHouse(static_cast<HOUSE_ID>(window_SpeakInHouse->wData.val));
    dialog_menu_id = DIALOGUE_MAIN;

    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
    if (pParty->hasActiveCharacter()) {
        pParty->activeCharacter().playReaction(SPEECH_HIRE_NPC);
    }

    BackToHouseMenu();
}


void _4B3FE5_training_dialogue(int eventId) {
    uDialogueType = DIALOGUE_SKILL_TRAINER;
    current_npc_text = std::string(pNPCTopics[eventId + 168].pText);
    _4B254D_SkillMasteryTeacher(eventId);  // checks whether the facility can be used
    pDialogueWindow->Release();
    pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, {render->GetRenderDimensions().w, 350}, eventId);
    pBtn_ExitCancel = pDialogueWindow->CreateButton({471, 445}, {169, 35}, 1, 0, UIMSG_Escape, 0, InputAction::Invalid,
        localization->GetString(LSTR_CANCEL), { ui_exit_cancel_button_background }
    );
    pDialogueWindow->CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_HouseScreenClick, 0, InputAction::Invalid, "");
    pDialogueWindow->CreateButton({480, 160}, {0x8Cu, 0x1Eu}, 1, 0, UIMSG_ClickNPCTopic, DIALOGUE_79_mastery_teacher, InputAction::Invalid,
        guild_membership_approved ? localization->GetString(LSTR_LEARN) : "");
    pDialogueWindow->_41D08F_set_keyboard_control_group(1, 1, 0, 2);
    dialog_menu_id = DIALOGUE_OTHER;
}

/**
 * @offset 0x004B1ECE.
 *
 * @brief Oracle's 'I lost it!' dialog option
 */
void OracleDialogue() {
    ItemGen *item = nullptr;
    ITEM_TYPE item_id = ITEM_NULL;

    // display "You never had it" if nothing missing will be found
    current_npc_text = pNPCTopics[667].pText;

    // only items with special subquest in range 212-237 and also 241 are recoverable
    for (auto pair : _4F0882_evt_VAR_PlayerItemInHands_vals) {
        int quest_id = pair.first;
        if (pParty->_questBits[quest_id]) {
            ITEM_TYPE search_item_id = pair.second;
            if (!pParty->hasItem(search_item_id) && pParty->pPickedItem.uItemID != search_item_id) {
                item_id = search_item_id;
                break;
            }
        }
    }

    // missing item found
    if (item_id != ITEM_NULL) {
        pParty->pCharacters[0].AddVariable(VAR_PlayerItemInHands, std::to_underlying(item_id));
        // TODO(captainurist): what if fmt throws?
        current_npc_text = fmt::sprintf(pNPCTopics[666].pText, // "Here's %s that you lost. Be careful"
                                        fmt::format("{::}{}\f00000", colorTable.Jonquil.tag(),
                                                    pItemTable->pItems[item_id].pUnidentifiedName));
    }

    // missing item is lich jar and we need to bind soul vessel to lich class character
    // TODO(Nik-RE-dev): this code is walking only through inventory, but item was added to hand, so it will not bind new item if it was acquired
    //                   rather this code will bind jars that already present in inventory to liches that currently do not have binded jars
    if (item_id == ITEM_QUEST_LICH_JAR_FULL) {
        for (int i = 0; i < pParty->pCharacters.size(); i++) {
            if (pParty->pCharacters[i].classType == CHARACTER_CLASS_LICH) {
                bool have_vessels_soul = false;
                for (Character &player : pParty->pCharacters) {
                    for (int idx = 0; idx < Character::INVENTORY_SLOT_COUNT; idx++) {
                        if (player.pInventoryItemList[idx].uItemID == ITEM_QUEST_LICH_JAR_FULL) {
                            if (player.pInventoryItemList[idx].uHolderPlayer == -1) {
                                item = &player.pInventoryItemList[idx];
                            }
                            if (player.pInventoryItemList[idx].uHolderPlayer == i) {
                                have_vessels_soul = true;
                            }
                        }
                    }
                }

                if (item && !have_vessels_soul) {
                    item->uHolderPlayer = i;
                    break;
                }
            }
        }
    }
}

//----- (004B254D) --------------------------------------------------------
std::string _4B254D_SkillMasteryTeacher(int trainerInfo) {
    uint8_t teacherLevel = (trainerInfo - 200) % 3;
    CharacterSkillType skillBeingTaught = static_cast<CharacterSkillType>((trainerInfo - 200) / 3);
    Character *activePlayer = &pParty->activeCharacter();
    CharacterClassType pClassType = activePlayer->classType;
    CharacterSkillMastery currClassMaxMastery = skillMaxMasteryPerClass[pClassType][skillBeingTaught];
    CharacterSkillMastery masteryLevelBeingTaught = dword_F8B1B0_MasteryBeingTaught = static_cast<CharacterSkillMastery>(teacherLevel + 2);
    guild_membership_approved = false;

    if (currClassMaxMastery < masteryLevelBeingTaught) {
        if (skillMaxMasteryPerClass[getTier2Class(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught) {
            return localization->FormatString(LSTR_FMT_HAVE_TO_BE_PROMOTED, localization->GetClassName(getTier2Class(pClassType)));
        } else if (skillMaxMasteryPerClass[getTier3LightClass(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught &&
                skillMaxMasteryPerClass[getTier3DarkClass(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught) {
            return localization->FormatString(LSTR_FMT_HAVE_TO_BE_PROMOTED_2,
                    localization->GetClassName(getTier3LightClass(pClassType)),
                    localization->GetClassName(getTier3DarkClass(pClassType)));
        } else if (skillMaxMasteryPerClass[getTier3LightClass(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught) {
            return localization->FormatString(LSTR_FMT_HAVE_TO_BE_PROMOTED, localization->GetClassName(getTier3LightClass(pClassType)));
        } else if (skillMaxMasteryPerClass[getTier3DarkClass(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught) {
            return localization->FormatString(LSTR_FMT_HAVE_TO_BE_PROMOTED, localization->GetClassName(getTier3DarkClass(pClassType)));
        } else {
            return localization->FormatString(LSTR_FMT_SKILL_CANT_BE_LEARNED, localization->GetClassName(pClassType));
        }
    }

    // Not in your condition!
    if (!activePlayer->CanAct()) {
        return std::string(pNPCTopics[122].pText);
    }

    // You must know the skill before you can become an expert in it!
    int skillLevel = activePlayer->getSkillValue(skillBeingTaught).level();
    if (!skillLevel) {
        return std::string(pNPCTopics[131].pText);
    }

    // You are already have this mastery in this skill.
    CharacterSkillMastery skillMastery = activePlayer->getSkillValue(skillBeingTaught).mastery();
    if (std::to_underlying(skillMastery) > teacherLevel + 1) {
        return std::string(pNPCTopics[teacherLevel + 128].pText);
    }

    dword_F8B1AC_skill_being_taught = skillBeingTaught;

    bool canLearn = true;

    if (masteryLevelBeingTaught == CHARACTER_SKILL_MASTERY_EXPERT) {
        canLearn = skillLevel >= 4;
        gold_transaction_amount = expertSkillMasteryCost[skillBeingTaught];
    }

    if (masteryLevelBeingTaught == CHARACTER_SKILL_MASTERY_MASTER) {
        switch (skillBeingTaught) {
          case CHARACTER_SKILL_LIGHT:
            canLearn = pParty->_questBits[114];
            break;
          case CHARACTER_SKILL_DARK:
            canLearn = pParty->_questBits[110];
            break;
          case CHARACTER_SKILL_MERCHANT:
            canLearn = activePlayer->GetBasePersonality() >= 50;
            break;
          case CHARACTER_SKILL_BODYBUILDING:
            canLearn = activePlayer->GetBaseEndurance() >= 50;
            break;
          case CHARACTER_SKILL_LEARNING:
            canLearn = activePlayer->GetBaseIntelligence() >= 50;
            break;
          default:
            break;
        }
        canLearn = canLearn && (skillLevel >= 7);
        gold_transaction_amount = masterSkillMasteryCost[skillBeingTaught];
    }

    if (masteryLevelBeingTaught == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
        switch (skillBeingTaught) {
          case CHARACTER_SKILL_LIGHT:
            canLearn = activePlayer->isClass(CHARACTER_CLASS_ARCHAMGE) || activePlayer->isClass(CHARACTER_CLASS_PRIEST_OF_SUN);
            break;
          case CHARACTER_SKILL_DARK:
            canLearn = activePlayer->isClass(CHARACTER_CLASS_LICH) || activePlayer->isClass(CHARACTER_CLASS_PRIEST_OF_MOON);
            break;
          case CHARACTER_SKILL_DODGE:
            canLearn = activePlayer->pActiveSkills[CHARACTER_SKILL_UNARMED].level() >= 10;
            break;
          case CHARACTER_SKILL_UNARMED:
            canLearn = activePlayer->pActiveSkills[CHARACTER_SKILL_DODGE].level() >= 10;
            break;
          default:
            break;
        }
        canLearn = canLearn && (skillLevel >= 10);
        gold_transaction_amount = grandmasterSkillMasteryCost[skillBeingTaught];
    }

    // You don't meet the requirements, and cannot be taught until you do.
    if (!canLearn) {
        return std::string(pNPCTopics[127].pText);
    }

    // You don't have enough gold!
    if (gold_transaction_amount > pParty->GetGold()) {
        return std::string(pNPCTopics[124].pText);
    }

    guild_membership_approved = true;

    return localization->FormatString(
        LSTR_FMT_BECOME_S_IN_S_FOR_D_GOLD,
        localization->MasteryNameLong(masteryLevelBeingTaught),
        localization->GetSkillName(skillBeingTaught),
        gold_transaction_amount
    );
}

std::string BuildDialogueString(const char *lpsz, uint8_t uPlayerID, ItemGen *a3, HOUSE_ID houseId, int a5, GameTime *a6) {
    std::string str = std::string(lpsz);
    return BuildDialogueString(str, uPlayerID, a3, houseId, a5, a6);
}

//----- (00495461) --------------------------------------------------------
std::string BuildDialogueString(std::string &str, uint8_t uPlayerID, ItemGen *a3, HOUSE_ID houseId, int shop_screen, GameTime *a6) {
    std::string v1;
    Character *pPlayer;       // ebx@3
    const char *pText;     // esi@7
    int64_t v18;    // qax@18
    int v29;               // eax@68
    std::vector<int> addressingBits;
    SummonedItem v56;      // [sp+80h] [bp-B8h]@107

    pPlayer = &pParty->pCharacters[uPlayerID];

    NPCData *npc = nullptr;
    if (dword_5C35D4)
        npc = HouseNPCData[pDialogueNPCCount - (dword_591080 != 0)];
    else
        npc = GetNPCData(sDialogue_SpeakingActorNPC_ID);

    std::string result;

    uint len = str.length();
    for (int i = 0, dst = 0; i < len; ++i) {
        char c = str[i];  // skip through string till we find insertion point
        if (c != '%') {
            result += c;  // add char to result string
        } else {
            int mask = 10 * (int)(str[i + 1] - '0') + str[i + 2] - '0';  // mask tells what the gap needs filling with
            i += 2;

            switch (mask) {
            case 1:
                result += npc->pName;
                break;
            case 2:
                result += pPlayer->name;
                i += 2;
                break;
            case 3:
            case 4:
                result += v1;
                break;
            case 5:
                v18 = pParty->GetPlayingTime().GetHoursOfDay();
                if (v18 >= 11 && v18 < 20) {
                    pText = localization->GetString(LSTR_DAY);
                } else if (v18 >= 5 && v18 < 11) {
                    pText = localization->GetString(LSTR_MORNING);
                } else {
                    pText = localization->GetString(LSTR_EVENING);
                }
                // TODO(captainurist): ^ and what about night?
                result += pText;
                break;
            case 6:
                if (pPlayer->uSex)
                    result += localization->GetString(LSTR_LADY_LOWERCASE);
                else
                    result += localization->GetString(LSTR_SIR_LOWERCASE);
                break;
            case 7:
                if (pPlayer->uSex)
                    result += localization->GetString(LSTR_LADY);
                else
                    result += localization->GetString(LSTR_SIR);
                break;
            case 8:
                for (int bit : possibleAddressingAwardBits) {
                    if (pPlayer->_achievedAwardsBits[bit]) {
                        addressingBits.push_back(bit);
                    }
                }
                if (addressingBits.size()) {
                    if (currentAddressingAwardBit == -1)
                        currentAddressingAwardBit = addressingBits[vrng->random(addressingBits.size())];
                    result += pAwards[currentAddressingAwardBit].pText;
                } else {
                    result += pNPCTopics[55].pText;
                }
                break;
            case 9:
                if (npc->uSex)
                    result += localization->GetString(LSTR_HER);
                else
                    result += localization->GetString(LSTR_HIS);
                break;
            case 10:
                if (pPlayer->uSex)
                    result += localization->GetString(LSTR_LADY);
                else
                    result += localization->GetString(LSTR_LORD);
                break;
            case 11:
                result += GetReputationString(pParty->GetPartyReputation());
                break;
            case 12:
                result += GetReputationString(npc->rep);
                break;
            case 13:
                result += pNPCStats->sub_495366_MispronounceName(pPlayer->name[0], pPlayer->uSex);
                break;
            case 14:
                if (npc->uSex)
                    result += localization->GetString(LSTR_SISTER);
                else
                    result += localization->GetString(LSTR_BROTHER);
                break;
            case 15:
                result += localization->GetString(LSTR_DAUGHTER);
                break;
            case 16:
                if (npc->uSex)
                    result += localization->GetString(LSTR_SISTER);
                else
                    result += localization->GetString(LSTR_BROTHER);
                break;
            case 17:  // hired npc text
            {
                uint pay_percentage = pNPCStats->pProfessions[npc->profession].uHirePrice / 100;
                if (pay_percentage == 0) pay_percentage = 1;
                v1 = fmt::format("{}", pay_percentage);
                result += v1;
                break;
            }
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
            case 26:
                v1 = str.substr(i + 1, 2);
                if (v1.starts_with('0'))
                    v1 = v1.substr(1);
                result += v1;
                break;
            case 23:
                if (pMapStats->GetMapInfo(pCurrentMapName))
                    result += pMapStats->pInfos[pMapStats->GetMapInfo(pCurrentMapName)].pName;
                else
                    result += localization->GetString(LSTR_UNKNOWN);
                break;

            case 24:  // item name
                v1 = fmt::format("{::}{}\f00000\n", colorTable.PaleCanary.tag(), a3->GetDisplayName());
                result += v1;
                break;

            case 25:  // base prices
                v29 = PriceCalculator::baseItemBuyingPrice(a3->GetValue(), buildingTable[houseId].fPriceMultiplier);
                switch (shop_screen) {
                case 3:
                    v29 = PriceCalculator::baseItemSellingPrice(a3->GetValue(), buildingTable[houseId].fPriceMultiplier);
                    break;
                case 4:
                    v29 = PriceCalculator::baseItemIdentifyPrice(buildingTable[houseId].fPriceMultiplier);
                    break;
                case 5:
                    v29 = PriceCalculator::baseItemRepairPrice(a3->GetValue(), buildingTable[houseId].fPriceMultiplier);
                    break;
                case 6:
                    v29 = PriceCalculator::baseItemSellingPrice(a3->GetValue(), buildingTable[houseId].fPriceMultiplier) / 2;
                    break;
                }
                v1 = fmt::format("{}", v29);
                result += v1;
                break;

            case 27:  // actual price
                v29 = PriceCalculator::itemBuyingPriceForPlayer(pPlayer, a3->GetValue(), buildingTable[houseId].fPriceMultiplier);
                if (shop_screen == 3) {
                    v29 = PriceCalculator::itemSellingPriceForPlayer(pPlayer, *a3, buildingTable[houseId].fPriceMultiplier);
                    v1 = fmt::format("{}", v29);
                    result += v1;
                    break;
                }
                if (shop_screen != 4) { // TODO(captainurist): enums for shop screens
                    if (shop_screen == 5) {
                    v29 = PriceCalculator::itemRepairPriceForPlayer(
                        pPlayer, a3->GetValue(),
                        buildingTable[houseId].fPriceMultiplier);
                    } else {
                        if (shop_screen == 6) {
                            // TODO(captainurist): encapsulate this logic in PriceCalculator
                            v29 = PriceCalculator::itemSellingPriceForPlayer(pPlayer, *a3, buildingTable[houseId].fPriceMultiplier) / 2;
                            if (!v29)  // cannot be 0
                                v29 = 1;
                            v1 = fmt::format("{}", v29);
                            result += v1;
                            break;
                        }
                    }
                    v1 = fmt::format("{}", v29);
                    result += v1;
                    break;
                }
                v1 = fmt::format("{}", PriceCalculator::itemIdentificationPriceForPlayer(pPlayer, buildingTable[houseId].fPriceMultiplier));
                result += v1;
                break;

            case 28:  // shop type - blacksmith ect..
                result += buildingTable[houseId].pProprieterTitle;
                break;

            case 29:  // identify cost
                v1 = fmt::format("{}", PriceCalculator::itemIdentificationPriceForPlayer(pPlayer, buildingTable[houseId].fPriceMultiplier));
                result += v1;
                break;
            case 30:
                if (!a6) {
                    // result += eventId;
                    __debugbreak(); // should never get here?
                    break;
                }
                v56.Initialize(*a6);
                result += localization->FormatString(
                    LSTR_FMT_S_D_D,
                    localization->GetMonthName(v56.field_14_exprie_month),
                    v56.field_C_expire_day + 1,
                    v56.field_18_expire_year);
                break;
            case 31:
            case 32:
            case 33:
            case 34:
                result += pParty->pCharacters[mask - 31].name;
                break;
            default:
                if (mask <= 50 || mask > 70) {
                    v1 = str.substr(i + 1, 2);
                    if (v1.starts_with('0'))
                        v1 = v1.substr(1);
                    result += v1;
                    break;
                }
                if (mask - 51 >= 20) {
                    // result += eventId;
                    __debugbreak(); // should never get here?
                    break;
                }

                v56.Initialize(pParty->PartyTimes._s_times[mask - 51]);
                result += localization->FormatString(
                    LSTR_FMT_S_D_D,
                    localization->GetMonthName(v56.field_14_exprie_month),
                    v56.field_C_expire_day + 1,
                    v56.field_18_expire_year);
                break;
            }
        }
    }

    return result;
}

WindowManager windowManager;

void WindowManager::DeleteAllVisibleWindows() {
    while (lWindowList.size() > 1) {
        GUIWindow *pWindow = lWindowList.front();
        // game ui should never be released
        if (pWindow->eWindowType == WINDOW_GameUI) continue;
        pWindow->Release();
        delete pWindow;
    }

    // reset screen state after deleting all windows
    pGUIWindow_CurrentMenu = nullptr;
    pDialogueWindow = nullptr;
    window_SpeakInHouse = nullptr;
    pGUIWindow_ScrollWindow = nullptr; // reading a message scroll
    ptr_507BC8 = nullptr;  // screen 19 - not used?
    pGUIWindow_CastTargetedSpell = nullptr;
    pGameOverWindow = nullptr; // UIMSG_ShowGameOverWindow
    pGUIWindow_BranchlessDialogue = nullptr; // branchless dialougue

    current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
    pNextFrameMessageQueue->Clear();
    pCurrentFrameMessageQueue->Clear();
    pMediaPlayer->Unload();
}

void MainMenuUI_LoadFontsAndSomeStuff() {
    // pIcons_LOD->SetupPalettes(render->uTargetRBits, render->uTargetGBits, render->uTargetBBits);
    pIcons_LOD->SetupPalettes(5, 6, 5);
    // pPaletteManager->SetColorChannelInfo(render->uTargetRBits, render->uTargetGBits, render->uTargetBBits);
    //pPaletteManager->SetColorChannelInfo(5, 6, 5);
    //pPaletteManager->RecalculateAll();

    // for (uint i = 0; i < 480; ++i) {  // must be 480 - needs sorting
    //     pSRZBufferLineOffsets[i] = 640 * i;  // must be 640 - needs sorting
    // }

    pFontArrus = GUIFont::LoadFont("arrus.fnt", "FONTPAL");
    pFontLucida = GUIFont::LoadFont("lucida.fnt", "FONTPAL");
    pFontCreate = GUIFont::LoadFont("create.fnt", "FONTPAL");
    pFontSmallnum = GUIFont::LoadFont("smallnum.fnt", "FONTPAL");
    pFontComic = GUIFont::LoadFont("comic.fnt", "FONTPAL");
}

static void LoadPartyBuffIcons() {
    for (uint i = 0; i < 14; ++i) {
        //auto temp = assets->GetImage_Paletted(StringPrintf("isn-%02d", i + 1));
        //int booty = temp->GetHeight();
        //party_buff_icons[i] = assets->getImage_ColorKey(StringPrintf("isn-%02d", i + 1), colorTable.TealMask);
        party_buff_icons[i] = assets->getImage_Paletted(fmt::format("isn-{:02}", i + 1));
    }

    uIconIdx_FlySpell = pIconsFrameTable->FindIcon("spell21");
    uIconIdx_WaterWalk = pIconsFrameTable->FindIcon("spell27");
}

void UI_Create() {
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("wizeyeC"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("wizeyeB"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("wizeyeA"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("torchC"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("torchB"));
    pIconsFrameTable->InitializeAnimation(pIconsFrameTable->FindIcon("torchA"));

    game_ui_minimap_dirs[0] = assets->getImage_Alpha("MAPDIR1");
    game_ui_minimap_dirs[1] = assets->getImage_Alpha("MAPDIR2");
    game_ui_minimap_dirs[2] = assets->getImage_Alpha("MAPDIR3");
    game_ui_minimap_dirs[3] = assets->getImage_Alpha("MAPDIR4");
    game_ui_minimap_dirs[4] = assets->getImage_Alpha("MAPDIR5");
    game_ui_minimap_dirs[5] = assets->getImage_Alpha("MAPDIR6");
    game_ui_minimap_dirs[6] = assets->getImage_Alpha("MAPDIR7");
    game_ui_minimap_dirs[7] = assets->getImage_Alpha("MAPDIR8");

    game_ui_bar_blue = assets->getImage_ColorKey("ib-statB");
    game_ui_bar_green = assets->getImage_ColorKey("ib-statG");
    game_ui_bar_yellow = assets->getImage_ColorKey("ib-statY");
    game_ui_bar_red = assets->getImage_ColorKey("ib-statR");
    game_ui_monster_hp_background = assets->getImage_ColorKey("mhp_bg");
    game_ui_monster_hp_border_left = assets->getImage_ColorKey("mhp_capl");
    game_ui_monster_hp_border_right = assets->getImage_ColorKey("mhp_capr");
    game_ui_monster_hp_green = assets->getImage_ColorKey("mhp_grn");
    game_ui_monster_hp_red = assets->getImage_ColorKey("mhp_red");
    game_ui_monster_hp_yellow = assets->getImage_ColorKey("mhp_yel");
    ui_leather_mm7 = assets->getImage_Solid("LEATHER");
    ui_leather_mm6 = assets->getImage_Solid("ibground");
    dialogue_ui_x_x_u = assets->getImage_ColorKey("x_x_u");
    ui_buttdesc2 = assets->getImage_Alpha("BUTTESC2");
    dialogue_ui_x_ok_u = assets->getImage_ColorKey("x_ok_u");
    ui_buttyes2 = assets->getImage_Alpha("BUTTYES2");

    nuklear->Create(WINDOW_GameUI);
    pPrimaryWindow = new GUIWindow(WINDOW_GameUI, {0, 0}, render->GetRenderDimensions(), 0);
    pPrimaryWindow->CreateButton({7, 8}, {460, 343}, 1, 0, UIMSG_MouseLeftClickInGame, 0);

    pPrimaryWindow->CreateButton("Game_Character1", {61, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 1, InputAction::SelectChar1);  // buttons for portraits
    pPrimaryWindow->CreateButton("Game_Character2", {177, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 2, InputAction::SelectChar2);
    pPrimaryWindow->CreateButton("Game_Character3", {292, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 3, InputAction::SelectChar3);
    pPrimaryWindow->CreateButton("Game_Character4", {407, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 4, InputAction::SelectChar4);

    pPrimaryWindow->CreateButton({24, 404}, {5, 49}, 1, 93, UIMSG_0, 1);  // buttons for HP
    pPrimaryWindow->CreateButton({139, 404}, {5, 49}, 1, 93, UIMSG_0, 2);
    pPrimaryWindow->CreateButton({255, 404}, {5, 49}, 1, 93, UIMSG_0, 3);
    pPrimaryWindow->CreateButton({370, 404}, {5, 49}, 1, 93, UIMSG_0, 4);

    pPrimaryWindow->CreateButton({97, 404}, {5, 49}, 1, 93, UIMSG_0, 1);  // buttons for SP
    pPrimaryWindow->CreateButton({212, 404}, {5, 49}, 1, 93, UIMSG_0, 2);
    pPrimaryWindow->CreateButton({328, 404}, {5, 49}, 1, 93, UIMSG_0, 3);
    pPrimaryWindow->CreateButton({443, 404}, {5, 49}, 1, 93, UIMSG_0, 4);

    game_ui_tome_quests = assets->getImage_ColorKey("ib-td1-A");
    pBtn_Quests = pPrimaryWindow->CreateButton({491, 353}, game_ui_tome_quests->size(), 1, 0,
                                               UIMSG_OpenQuestBook, 0, InputAction::Quest,
                                               localization->GetString(LSTR_CURRENT_QUESTS), { game_ui_tome_quests });

    game_ui_tome_autonotes = assets->getImage_ColorKey("ib-td2-A");
    pBtn_Autonotes = pPrimaryWindow->CreateButton({527, 353}, game_ui_tome_autonotes->size(), 1, 0,
                                                  UIMSG_OpenAutonotes, 0, InputAction::Autonotes,
                                                  localization->GetString(LSTR_AUTONOTES), { game_ui_tome_autonotes });

    game_ui_tome_maps = assets->getImage_ColorKey("ib-td3-A");
    pBtn_Maps = pPrimaryWindow->CreateButton({546, 353}, game_ui_tome_maps->size(), 1, 0,
                                             UIMSG_OpenMapBook, 0, InputAction::Mapbook,
                                             localization->GetString(LSTR_MAPS), { game_ui_tome_maps });

    game_ui_tome_calendar = assets->getImage_ColorKey("ib-td4-A");
    pBtn_Calendar = pPrimaryWindow->CreateButton({570, 353}, game_ui_tome_calendar->size(), 1, 0,
                                                 UIMSG_OpenCalendar, 0, InputAction::TimeCal,
                                                 localization->GetString(LSTR_CALENDAR), { game_ui_tome_calendar });

    game_ui_tome_storyline = assets->getImage_ColorKey("ib-td5-A");
    pBtn_History = pPrimaryWindow->CreateButton({600, 361}, game_ui_tome_storyline->size(), 1, 0,
                                                UIMSG_OpenHistoryBook, 0, InputAction::History,
                                                localization->GetString(LSTR_HISTORY), { game_ui_tome_storyline }
    );

    bFlashAutonotesBook = false;
    bFlashQuestBook = false;
    bFlashHistoryBook = false;

    pBtn_ZoomIn = pPrimaryWindow->CreateButton({519, 136}, game_ui_btn_zoomin->size(), 2, 0,
                                               UIMSG_ClickZoomInBtn, 0, InputAction::ZoomIn,
                                               localization->GetString(LSTR_ZOOM_IN), { game_ui_btn_zoomin }
    );

    pBtn_ZoomOut = pPrimaryWindow->CreateButton({574, 136}, game_ui_btn_zoomout->size(), 2, 0,
                                                UIMSG_ClickZoomOutBtn, 0, InputAction::ZoomOut,
                                                localization->GetString(LSTR_ZOOM_OUT), { game_ui_btn_zoomout });

    pPrimaryWindow->CreateButton({481, 0}, {153, 67}, 1, 92, UIMSG_0, 0);
    pPrimaryWindow->CreateButton({491, 149}, {64, 74}, 1, 0, UIMSG_StartHireling1Dialogue, 0, InputAction::SelectNPC1);
    pPrimaryWindow->CreateButton({561, 149}, {64, 74}, 1, 0, UIMSG_StartHireling2Dialogue, 0, InputAction::SelectNPC2);
    pPrimaryWindow->CreateButton({476, 322}, {77, 17}, 1, 100, UIMSG_0, 0);
    pPrimaryWindow->CreateButton({555, 322}, {77, 17}, 1, 101, UIMSG_0, 0);

    pBtn_CastSpell = pPrimaryWindow->CreateButton("Game_CastSpell", {476, 450}, game_ui_btn_cast->size(), 1, 0,
                                                  UIMSG_SpellBookWindow, 0, InputAction::Cast,
                                                  localization->GetString(LSTR_CAST_SPELL), { game_ui_btn_cast });
    pBtn_Rest = pPrimaryWindow->CreateButton({518, 450}, game_ui_btn_rest->size(), 1, 0,
                                             UIMSG_RestWindow, 0, InputAction::Rest,
                                             localization->GetString(LSTR_REST), { game_ui_btn_rest });
    pBtn_QuickReference = pPrimaryWindow->CreateButton({560, 450}, game_ui_btn_quickref->size(), 1, 0,
                                                       UIMSG_QuickReference, 0, InputAction::QuickRef,
                                                       localization->GetString(LSTR_QUICK_REFERENCE), { game_ui_btn_quickref });
    pBtn_GameSettings = pPrimaryWindow->CreateButton({602, 450}, game_ui_btn_settings->size(), 1, 0,
                                                     UIMSG_GameMenuButton, 0, InputAction::Invalid,
                                                     localization->GetString(LSTR_GAME_OPTIONS), { game_ui_btn_settings });
    pBtn_NPCLeft = pPrimaryWindow->CreateButton({469, 178}, ui_btn_npc_left->size(), 1, 0,
                                                UIMSG_ScrollNPCPanel, 0, InputAction::Invalid, "", {ui_btn_npc_left });
    pBtn_NPCRight = pPrimaryWindow->CreateButton({626, 178}, ui_btn_npc_right->size(), 1, 0,
                                                 UIMSG_ScrollNPCPanel, 1, InputAction::Invalid, "", {ui_btn_npc_right });

    LoadPartyBuffIcons();
}


std::string NameAndTitle(const std::string &name, const std::string &title) {
    return localization->FormatString(
        LSTR_FMT_S_THE_S,
        name.c_str(),
        title.c_str()
    );
}


std::string NameAndTitle(const std::string &name, CharacterClassType class_type) {
    return NameAndTitle(
        name,
        localization->GetClassName(class_type)
    );
}


std::string NameAndTitle(const std::string &name, NPCProf profession) {
    return NameAndTitle(
        name,
        localization->GetNpcProfessionName(profession)
    );
}


std::string NameAndTitle(NPCData *npc) {
    if (!npc->pName.empty()) {
        if (npc->profession != NoProfession)
            return NameAndTitle(npc->pName, npc->profession);

        return npc->pName;
    }

    return std::string();
}


std::string GetDisplayName(Actor *actor) {
    if (actor->dword_000334_unique_name)
        return pMonsterStats->pPlaceStrings[actor->dword_000334_unique_name];
    else
        return pMonsterStats->pInfos[actor->pMonsterInfo.uID].pName;
}

//----- (004B29F2) --------------------------------------------------------
const std::string &GetJoinGuildDialogueOption(GUILD_ID guild_id) {
    static const int dialogue_base = 110;
    guild_membership_approved = false;
    dword_F8B1AC_award_bit_number = static_cast<AwardType>(Award_Membership_ElementalGuilds + std::to_underlying(guild_id));
    gold_transaction_amount = priceForMembership[guild_id];

    // TODO(pskelton): check this behaviour
    if (!pParty->hasActiveCharacter())
        pParty->setActiveToFirstCanAct();  // avoid nzi

    if (pParty->activeCharacter().CanAct()) {
        if (pParty->activeCharacter()._achievedAwardsBits[dword_F8B1AC_award_bit_number]) {
            return pNPCTopics[dialogue_base + 13].pText;
        } else {
            if (gold_transaction_amount <= pParty->GetGold()) {
                guild_membership_approved = true;
                return pNPCTopics[dialogue_base + guild_id].pText;
            } else {
                return pNPCTopics[dialogue_base + 14].pText;
            }
        }
    } else {
        return pNPCTopics[dialogue_base + 12].pText;
    }
}
