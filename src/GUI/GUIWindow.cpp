#include "GUI/GUIWindow.h"

#include <stdlib.h>
#include <sstream>
#include <utility>

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/AssetsManager.h"
#include "Engine/Events/Processor.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Image.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Objects/NPC.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/PriceCalculator.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Tables/AwardTable.h"
#include "Engine/Tables/MessageScrollTable.h"
#include "Engine/Time.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"
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
#include "Library/Logger/Logger.h"

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

std::array<int, 28> possibleAddressingAwardBits = {{1,  2,  3,  4,  5,  7,  32, 33, 36, 37,
                                                    38, 40, 41, 42, 43, 45, 46, 47, 48, 49,
                                                    50, 51, 52, 53, 54, 55, 56, 60}};
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

GUIButton *GUI_HandleHotkey(PlatformKey hotkey) {
    for (GUIWindow *pWindow : lWindowList) {
        for (GUIButton *result : pWindow->vButtons) {
            if (result->action != Io::InputAction::Invalid && keyboardActionMapping->IsKeyMatchAction(result->action, hotkey)) {
                engine->_messageQueue->addMessageCurrentFrame(result->msg, result->msg_param, 0);
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
                                   unsigned int msg_param, Io::InputAction action,
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
                        UIMessageType msg, unsigned int msg_param, Io::InputAction action, const std::string &label,
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
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_SaveGame, 0, 0);
    } else {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_LoadGame, 0, 0);
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

    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
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

    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
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

    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
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
        std::shared_ptr<Io::Mouse> mouse = EngineIocContainer::ResolveMouse();
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

    NPCData *npc;

    if (houseNpcs.size()) {
        npc = houseNpcs[currentHouseNpc].npc;
    } else {
        npc = GetNPCData(sDialogue_SpeakingActorNPC_ID);
    }

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
    engine->_messageQueue->clearAll();
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

    pPrimaryWindow->CreateButton("Game_Character1", {61, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 1, Io::InputAction::SelectChar1);  // buttons for portraits
    pPrimaryWindow->CreateButton("Game_Character2", {177, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 2, Io::InputAction::SelectChar2);
    pPrimaryWindow->CreateButton("Game_Character3", {292, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 3, Io::InputAction::SelectChar3);
    pPrimaryWindow->CreateButton("Game_Character4", {407, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 4, Io::InputAction::SelectChar4);

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
                                               UIMSG_OpenQuestBook, 0, Io::InputAction::Quest,
                                               localization->GetString(LSTR_CURRENT_QUESTS), { game_ui_tome_quests });

    game_ui_tome_autonotes = assets->getImage_ColorKey("ib-td2-A");
    pBtn_Autonotes = pPrimaryWindow->CreateButton({527, 353}, game_ui_tome_autonotes->size(), 1, 0,
                                                  UIMSG_OpenAutonotes, 0, Io::InputAction::Autonotes,
                                                  localization->GetString(LSTR_AUTONOTES), { game_ui_tome_autonotes });

    game_ui_tome_maps = assets->getImage_ColorKey("ib-td3-A");
    pBtn_Maps = pPrimaryWindow->CreateButton({546, 353}, game_ui_tome_maps->size(), 1, 0,
                                             UIMSG_OpenMapBook, 0, Io::InputAction::Mapbook,
                                             localization->GetString(LSTR_MAPS), { game_ui_tome_maps });

    game_ui_tome_calendar = assets->getImage_ColorKey("ib-td4-A");
    pBtn_Calendar = pPrimaryWindow->CreateButton({570, 353}, game_ui_tome_calendar->size(), 1, 0,
                                                 UIMSG_OpenCalendar, 0, Io::InputAction::TimeCal,
                                                 localization->GetString(LSTR_CALENDAR), { game_ui_tome_calendar });

    game_ui_tome_storyline = assets->getImage_ColorKey("ib-td5-A");
    pBtn_History = pPrimaryWindow->CreateButton({600, 361}, game_ui_tome_storyline->size(), 1, 0,
                                                UIMSG_OpenHistoryBook, 0, Io::InputAction::History,
                                                localization->GetString(LSTR_HISTORY), { game_ui_tome_storyline }
    );

    bFlashAutonotesBook = false;
    bFlashQuestBook = false;
    bFlashHistoryBook = false;

    pBtn_ZoomIn = pPrimaryWindow->CreateButton({519, 136}, game_ui_btn_zoomin->size(), 2, 0,
                                               UIMSG_ClickZoomInBtn, 0, Io::InputAction::ZoomIn,
                                               localization->GetString(LSTR_ZOOM_IN), { game_ui_btn_zoomin }
    );

    pBtn_ZoomOut = pPrimaryWindow->CreateButton({574, 136}, game_ui_btn_zoomout->size(), 2, 0,
                                                UIMSG_ClickZoomOutBtn, 0, Io::InputAction::ZoomOut,
                                                localization->GetString(LSTR_ZOOM_OUT), { game_ui_btn_zoomout });

    pPrimaryWindow->CreateButton({484, 15}, {138, 116}, 1, UIMSG_ShowStatus_DateTime, UIMSG_0, 0);
    pPrimaryWindow->CreateButton({491, 149}, {64, 74}, 1, 0, UIMSG_StartHireling1Dialogue, 0, Io::InputAction::SelectNPC1);
    pPrimaryWindow->CreateButton({561, 149}, {64, 74}, 1, 0, UIMSG_StartHireling2Dialogue, 0, Io::InputAction::SelectNPC2);
    pPrimaryWindow->CreateButton({476, 322}, {77, 17}, 1, UIMSG_ShowStatus_Food, UIMSG_0, 0);
    pPrimaryWindow->CreateButton({555, 322}, {77, 17}, 1, UIMSG_ShowStatus_Funds, UIMSG_0, 0);

    pBtn_CastSpell = pPrimaryWindow->CreateButton("Game_CastSpell", {476, 450}, game_ui_btn_cast->size(), 1, 0,
                                                  UIMSG_SpellBookWindow, 0, Io::InputAction::Cast,
                                                  localization->GetString(LSTR_CAST_SPELL), { game_ui_btn_cast });
    pBtn_Rest = pPrimaryWindow->CreateButton({518, 450}, game_ui_btn_rest->size(), 1, 0,
                                             UIMSG_RestWindow, 0, Io::InputAction::Rest,
                                             localization->GetString(LSTR_REST), { game_ui_btn_rest });
    pBtn_QuickReference = pPrimaryWindow->CreateButton({560, 450}, game_ui_btn_quickref->size(), 1, 0,
                                                       UIMSG_QuickReference, 0, Io::InputAction::QuickRef,
                                                       localization->GetString(LSTR_QUICK_REFERENCE), { game_ui_btn_quickref });
    pBtn_GameSettings = pPrimaryWindow->CreateButton({602, 450}, game_ui_btn_settings->size(), 1, 0,
                                                     UIMSG_GameMenuButton, 0, Io::InputAction::Invalid,
                                                     localization->GetString(LSTR_GAME_OPTIONS), { game_ui_btn_settings });
    pBtn_NPCLeft = pPrimaryWindow->CreateButton({469, 178}, ui_btn_npc_left->size(), 1, 0,
                                                UIMSG_ScrollNPCPanel, 0, Io::InputAction::Invalid, "", {ui_btn_npc_left });
    pBtn_NPCRight = pPrimaryWindow->CreateButton({626, 178}, ui_btn_npc_right->size(), 1, 0,
                                                 UIMSG_ScrollNPCPanel, 1, Io::InputAction::Invalid, "", {ui_btn_npc_right });

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
        return pMonsterStats->pInfos[actor->monsterInfo.uID].pName;
}
