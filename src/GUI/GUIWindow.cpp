#include "GUI/GUIWindow.h"

#include <cstdlib>
#include <string>
#include <sstream>
#include <utility>
#include <list>
#include <memory>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/AssetsManager.h"
#include "Engine/EngineCallObserver.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/Random/Random.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/PriceCalculator.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/Tables/AwardTable.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Time/Timer.h"
#include "Engine/MapInfo.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UICharacter.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIPopup.h"
#include "GUI/UI/UIDialogue.h"

#include "Io/InputEnums.h"
#include "Io/KeyboardInputHandler.h"
#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

#include "Library/Serialization/EnumSerialization.h"
#include "Library/Logger/Logger.h"

extern std::array<int, 4> pHealthBarPos;
extern std::array<int, 4> pManaBarPos;
extern const int pHealthManaBarYPos;

GUIWindow *pPrimaryWindow;

GUIWindow *pGUIWindow_CurrentMenu;
GUIWindow_Chest *pGUIWindow_CurrentChest;
GUIWindow *pDialogueWindow;
GUIWindow_House *window_SpeakInHouse;
GUIWindow_MessageScroll *pGUIWindow_ScrollWindow; // reading a message scroll
GUIWindow *ptr_507BC8;  // screen 19 - not used?
TargetedSpellUI *pGUIWindow_CastTargetedSpell;
GUIWindow_GameOver *pGameOverWindow; // UIMSG_ShowGameOverWindow
GUIWindow_BranchlessDialogue *pGUIWindow_BranchlessDialogue; // branchless dialougue

enum WindowType current_character_screen_window = WINDOW_CharacterWindow_Stats;
std::list<GUIWindow*> lWindowList;

MenuType sCurrentMenuID;

ScreenType current_screen_type = SCREEN_VIDEO;
ScreenType prev_screen_type;

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

std::array<int, 28> possibleAddressingAwardBits = {{1,  2,  3,  4,  5,  7,  32, 33, 36, 37,
                                                    38, 40, 41, 42, 43, 45, 46, 47, 48, 49,
                                                    50, 51, 52, 53, 54, 55, 56, 60}};

constinit const IndexedArray<Color, CHARACTER_BUFF_FIRST, CHARACTER_BUFF_LAST> ui_game_character_record_playerbuff_colors = {
    {CHARACTER_BUFF_RESIST_AIR,         colorTable.Anakiwa},
    {CHARACTER_BUFF_BLESS,              colorTable.Mercury},
    {CHARACTER_BUFF_RESIST_BODY,        colorTable.FlushOrange},
    {CHARACTER_BUFF_RESIST_EARTH,       colorTable.Gray},
    {CHARACTER_BUFF_FATE,               colorTable.Mercury},
    {CHARACTER_BUFF_RESIST_FIRE,        colorTable.DarkOrange},
    {CHARACTER_BUFF_HAMMERHANDS,        colorTable.FlushOrange},
    {CHARACTER_BUFF_HASTE,              colorTable.DarkOrange},
    {CHARACTER_BUFF_HEROISM,            colorTable.Mercury},
    {CHARACTER_BUFF_RESIST_MIND,        colorTable.PurplePink},
    {CHARACTER_BUFF_PAIN_REFLECTION,    colorTable.MoonRaker},
    {CHARACTER_BUFF_PRESERVATION,       colorTable.Mercury},
    {CHARACTER_BUFF_REGENERATION,       colorTable.FlushOrange},
    {CHARACTER_BUFF_SHIELD,             colorTable.Anakiwa},
    {CHARACTER_BUFF_STONESKIN,          colorTable.Gray},
    {CHARACTER_BUFF_ACCURACY,           colorTable.PaleCanary},
    {CHARACTER_BUFF_ENDURANCE,          colorTable.PaleCanary},
    {CHARACTER_BUFF_INTELLIGENCE,       colorTable.PaleCanary},
    {CHARACTER_BUFF_LUCK,               colorTable.PaleCanary},
    {CHARACTER_BUFF_STRENGTH,           colorTable.PaleCanary},
    {CHARACTER_BUFF_PERSONALITY,        colorTable.PaleCanary},
    {CHARACTER_BUFF_SPEED,              colorTable.PaleCanary},
    {CHARACTER_BUFF_RESIST_WATER,       colorTable.AzureRadiance},
    {CHARACTER_BUFF_WATER_WALK,         colorTable.AzureRadiance},
};


void SetCurrentMenuID(MenuType uMenu) {
    sCurrentMenuID = uMenu;
}

MenuType GetCurrentMenuID() {
    return sCurrentMenuID;
}

bool PauseGameDrawing() {
    if (current_screen_type != SCREEN_GAME &&
        current_screen_type != SCREEN_NPC_DIALOGUE &&
        current_screen_type != SCREEN_CHANGE_LOCATION) {
        if (current_screen_type == SCREEN_INPUT_BLV) return false; // uCurrentHouse_Animation;
        if (current_screen_type != SCREEN_BRANCHLESS_NPC_DIALOG) return true;
    }
    return false;
}

GUIButton *GUI_HandleHotkey(PlatformKey hotkey) {
    for (GUIWindow *pWindow : lWindowList) {
        for (GUIButton *result : pWindow->vButtons) {
            if (result->action != INPUT_ACTION_INVALID && keyboardActionMapping->isBound(result->action, hotkey)) {
                engine->_messageQueue->addMessageCurrentFrame(result->msg, result->msg_param, 0);
                return result;
            }
        }

        int width = render->GetPresentDimensions().w;
        // TODO(captainurist): Sus. Comparing height to width?
        if (pWindow->frameRect.x == 0 && pWindow->frameRect.y == 0 &&
            pWindow->frameRect.w == width && pWindow->frameRect.h == width) {
            break;
        }
    }
    return 0;
}

void GUIWindow::setKeyboardControlGroup(int buttonsCount, bool msgOnSelect, int selectStep, int initialPosition) {
    this->pNumPresenceButton = buttonsCount;
    this->_msgOnKeyboardSelect = msgOnSelect;
    this->_selectStep = selectStep;

    if (buttonsCount) {
        this->pCurrentPosActiveItem = initialPosition;
        this->pStartingPosActiveItem = initialPosition;
        this->receives_keyboard_input = true;
    } else {
        this->pCurrentPosActiveItem = 0;
        this->pStartingPosActiveItem = 0;
        this->receives_keyboard_input = false;
    }
}

void GUIWindow::Release() {
    if (!this || this->eWindowType == WINDOW_null) {
        // added the check to avoid releasing
        // windows already released
        return;
    }
    DeleteButtons();

    lWindowList.remove(this);

    logger->trace("Release window: {}", toString(eWindowType));
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
    // TODO(pskelton): Derived Messagebox types for different kinds of popup boxes
    if (engine->callObserver) {
        engine->callObserver->notify(CALL_DRAW_MESSAGE_BOX, sHint);
    }

    int x = 0;
    int y = 0;
    int z, w;
    if (inside_game_viewport) {
        x = pViewport.x;
        z = pViewport.x + pViewport.w - 1;
        y = pViewport.y;
        w = pViewport.y + pViewport.h - 1;
    } else {
        Sizei renDims = render->GetRenderDimensions();
        z = renDims.w;
        w = renDims.h;
    }

    Pointi cursor = mouse->position();
    if (frameRect.x >= x) {
        if (frameRect.w + frameRect.x > z) {
            frameRect.x = z - frameRect.w;
            frameRect.y = cursor.y + 30;
        }
    } else {
        frameRect.x = x;
        frameRect.y = cursor.y + 30;
    }

    if (frameRect.y >= y) {
        if (frameRect.y + frameRect.h > w) {
            frameRect.y = cursor.y - frameRect.h - 30;
        }
    } else {
        frameRect.y = cursor.y + 30;
    }
    if (frameRect.y < y) {
        frameRect.y = y;
    }
    if (frameRect.x < x) {
        frameRect.x = x;
    }

    GUIWindow current_window = *this;
    current_window.frameRect.x += 12;
    current_window.frameRect.w -= 28;
    current_window.frameRect.y += 12;
    current_window.frameRect.h -= 12;
    unsigned int uBoxHeight;
    if (!sHint.empty()) {
        uBoxHeight =
            assets->pFontLucida->CalcTextHeight(sHint, current_window.frameRect.w, 0) +
            24;
    } else {
        uBoxHeight = frameRect.h;
    }
    if (uBoxHeight < 64) {
        uBoxHeight = 64;
    }
    if ((int)(uBoxHeight + frameRect.y) > 479) {
        uBoxHeight = 479 - frameRect.y;
    }
    DrawPopupWindow(frameRect.x, frameRect.y, frameRect.w, uBoxHeight);
    if (!sHint.empty()) {
        current_window.DrawTitleText(
            assets->pFontLucida.get(),
            0, (int)(uBoxHeight - assets->pFontLucida->CalcTextHeight(this->sHint, current_window.frameRect.w, 0)) / 2 - 14,
            colorTable.White, this->sHint, 3);
    }
}

std::string MakeDateTimeString(Duration time) {
    CivilDuration d = time.toCivilDuration();

    std::string str;
    if (d.days) {
        auto day_str = localization->str(LSTR_DAYS);
        if (d.days <= 1) day_str = localization->str(LSTR_DAY_CAPITALIZED);

        str += fmt::format("{} {} ", d.days, day_str);
    }

    if (d.hours) {
        auto hour_str = localization->str(LSTR_HOURS);
        if (d.hours <= 1) hour_str = localization->str(LSTR_HOUR);

        str += fmt::format("{} {} ", d.hours, hour_str);
    }

    if (d.minutes && !d.days) {
        auto minute_str = localization->str(LSTR_MINUTES);
        if (d.minutes <= 1) minute_str = localization->str(LSTR_MINUTE);

        str += fmt::format("{} {} ", d.minutes, minute_str);
    }

    if (d.seconds && !d.hours && !d.days) {
        auto seconds_str = localization->str(LSTR_SECONDS);
        if (d.seconds <= 1) seconds_str = localization->str(LSTR_SECOND);

        str += fmt::format("{} {} ", d.seconds, seconds_str);
    }

    if (!str.empty() && str.back() == ' ')
        str.pop_back();

    return str;
}

//----- (004B1854) --------------------------------------------------------
void GUIWindow::DrawShops_next_generation_time_string(Duration time) {
    auto str = MakeDateTimeString(time);
    this->DrawTitleText(assets->pFontArrus.get(), 0, (212 - assets->pFontArrus->CalcTextHeight(str, frameRect.w, 0)) / 2 + 101, colorTable.PaleCanary, localization->str(LSTR_PLEASE_TRY_BACK_IN) + str, 3);
}

//----- (0044D406) --------------------------------------------------------
void GUIWindow::DrawTitleText(GUIFont *pFont, int horizontalMargin, int verticalMargin, Color color, std::string_view text, int lineSpacing) {
    if (engine->callObserver) {
        engine->callObserver->notify(CALL_GUIWINDOW_DRAWTEXT, std::string(text));
    }
    int width = frameRect.w - horizontalMargin;
    std::string resString = pFont->WrapText(text, frameRect.w, horizontalMargin);
    std::istringstream stream(resString);
    std::string line;
    int x = horizontalMargin + frameRect.x;
    int y = verticalMargin + frameRect.y;
    Color lastcolor = color;
    while (std::getline(stream, line)) {
        int x_offset = pFont->AlignText_Center(width, line);
        lastcolor = pFont->DrawTextLine(line, lastcolor, color, {x + x_offset, y});
        y += pFont->GetHeight() - lineSpacing;
    }
}

void GUIWindow::DrawDialoguePanel(std::string_view text) {
    if (text.empty())
        return;

    int frameWidth = pViewport.w;
    int indent = 12;

    GUIFont *font = assets->pFontArrus.get();
    int textHeight = font->CalcTextHeight(text, frameWidth, indent) + 7;
    if (352 - textHeight < 8) {
        font = assets->pFontCreate.get();
        textHeight = font->CalcTextHeight(text, frameWidth, indent) + 7;
    }

    int leatherWidth = ui_leather_mm7->width();
    render->DrawQuad2D(ui_leather_mm7, Recti(0, 0, leatherWidth, textHeight), Recti(8, 352 - textHeight, leatherWidth, textHeight));
    render->DrawQuad2D(_591428_endcap, {8, 347 - textHeight});
    DrawText(font, {indent, 354 - textHeight}, colorTable.White, font->WrapText(text, frameWidth, indent));
}

//----- (0044CE08) --------------------------------------------------------
void GUIWindow::DrawText(GUIFont *font, Pointi position, Color color, std::string_view text, int maxY, Color shadowColor) {
    if (engine->callObserver) {
        engine->callObserver->notify(CALL_GUIWINDOW_DRAWTEXT, std::string(text));
    }
    font->DrawText(frameRect, position, color, text, maxY, shadowColor);
}

//----- (0044CB4F) --------------------------------------------------------
int GUIWindow::DrawTextInRect(GUIFont *pFont, Pointi position,
                              Color uColor, std::string_view text, int rect_width,
                              int reverse_text) {
    return pFont->DrawTextInRect(frameRect, position, uColor, text, rect_width, reverse_text);
}

GUIButton *GUIWindow::CreateButton(Pointi position, Sizei dimensions,
                                   int uButtonType, int uData, UIMessageType msg,
                                   unsigned int msg_param, InputAction action,
                                   std::string_view label,
                                   const std::vector<GraphicsImage *> &textures) {
    GUIButton *pButton = new GUIButton();

    pButton->pParent = this;

    // For button type 2, if height is 0, use width for height (circular button).
    int height = (uButtonType == 2 && !dimensions.h) ? dimensions.w : dimensions.h;

    // Original code used closed intervals [uX, uZ] where uZ = uX + uWidth, so the button covered
    // uWidth + 1 pixels. With standard half-open Rect semantics [x, x+w), we store w + 1 to match.
    pButton->rect = Recti(position.x + frameRect.x, position.y + frameRect.y, dimensions.w + 1, height + 1);
    pButton->uButtonType = uButtonType;
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
                        UIMessageType msg, unsigned int msg_param, InputAction action, std::string_view label,
                        const std::vector<GraphicsImage *> &textures) {
    GUIButton *result = CreateButton(position, dimensions, uButtonType, uData, msg, msg_param, action, label, textures);
    result->id = std::move(id);
    return result;
}

bool GUIWindow::Contains(unsigned int x, unsigned int y) {
    return frameRect.contains(Pointi(x, y));
}

void GUIWindow::InitializeGUI() {
    SetUserInterface(PartyAlignment::PartyAlignment_Neutral);
    MainMenuUI_LoadFontsAndSomeStuff();
}

void GUIWindow::DrawFlashingInputCursor(int uX, int uY, GUIFont *a2) {
    // TODO(pskelton): check tickcount usage here
    if (platform->tickCount() % 1000 > 500) {
        DrawText(a2, {uX, uY}, colorTable.White, "_");
    }
}

GUIWindow::GUIWindow() {
    this->mouse = EngineIocContainer::ResolveMouse();
    this->mouse->SetMouseLook(Io::Mouse::MouseLookState::TempDisabled);
}

GUIWindow::GUIWindow(WindowType windowType, Pointi position, Sizei dimensions, std::string_view hint): eWindowType(windowType) {
    this->mouse = EngineIocContainer::ResolveMouse();
    this->mouse->SetMouseLook(Io::Mouse::MouseLookState::TempDisabled);

    logger->trace("New window: {}", toString(windowType));
    lWindowList.push_front(this);
    frameRect = Recti(position.x, position.y, dimensions.w, dimensions.h);

    this->sHint = hint;

    this->receives_keyboard_input = false;
}

void DialogueEnding() {
    speakingNpcId = 0;
    if (pDialogueWindow) {
        pDialogueWindow->Release();
    }
    pDialogueWindow = nullptr;
    pMiscTimer->setPaused(false);
    pEventTimer->setPaused(false);
}

void OnButtonClick::Update() {
    if (_playSound) {
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    }
    render->DrawQuad2D(_button->vTextures[0], frameRect.topLeft());
    if (!sHint.empty()) {
        _button->DrawLabel(sHint, assets->pFontCreate.get(), colorTable.White);
    }
    Release();

    delete this;
}

void OnButtonClick2::Update() {
    if (_playSound) {
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    }
    if (Recti({}, render->GetRenderDimensions()).intersects(_button->rect)) {
        render->DrawQuad2D(_button->vTextures[0], frameRect.topLeft());
    }
    if (!sHint.empty()) {
        _button->DrawLabel(sHint, assets->pFontCreate.get(), colorTable.White);
    }
    Release();

    delete this;
}

void OnButtonClick3::Update() {
    pAudioPlayer->playUISound(SOUND_StartMainChoice02);

    render->DrawQuad2D(_button->vTextures[1], frameRect.topLeft());
    if (!sHint.empty()) {
        _button->DrawLabel(sHint, assets->pFontCreate.get(), colorTable.White);
    }
    Release();

    delete this;
}

void OnButtonClick4::Update() {
    if (!sHint.empty()) {
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    }
    render->DrawQuad2D(_button->vTextures[1], frameRect.topLeft());

    Release();

    delete this;
}

void OnSaveLoad::Update() {
    if (!sHint.empty()) {
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    }
    render->DrawQuad2D(_button->vTextures[0], frameRect.topLeft());
    if (!sHint.empty()) {
        _button->DrawLabel(sHint, assets->pFontCreate.get(), colorTable.White);
    }
    Release();

    if (current_screen_type == SCREEN_SAVEGAME) {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_SaveGame, 0, 0);
    } else {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_LoadGame, 0, 0);
    }

    delete this;
}

void OnCancel::Update() {
    if (sHint.empty()) {
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    }
    render->DrawQuad2D(_button->vTextures[0], frameRect.topLeft());
    if (!sHint.empty()) {
        _button->DrawLabel(sHint, assets->pFontCreate.get(), colorTable.White);
    }
    Release();

    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);

    delete this;
}

void OnCancel2::Update() {
    if (!sHint.empty()) {
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    }
    render->DrawQuad2D(_button->vTextures[1], frameRect.topLeft());
    if (!sHint.empty()) {
        _button->DrawLabel(sHint, assets->pFontCreate.get(), colorTable.White);
    }
    Release();

    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);

    delete this;
}

void OnCancel3::Update() {
    if (!sHint.empty()) {
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    }

    render->DrawQuad2D(_button->vTextures[0], frameRect.topLeft());
    if (!sHint.empty()) {
        _button->DrawLabel(sHint, assets->pFontCreate.get(), colorTable.White);
    }
    Release();

    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);

    delete this;
}

void GUI_UpdateWindows() {
    if (GetCurrentMenuID() != MENU_CREATEPARTY) {
        extern bool UI_OnKeyDown(PlatformKey key);
        UI_OnKeyDown(PlatformKey::KEY_PAGEDOWN); // hack to highlight dialog options under mouse cursor
    }

    // should never activte this - gameui window should always be open
    if (lWindowList.size() < 1) assert(false);

    std::list<GUIWindow *> tmpWindowList(lWindowList);
    tmpWindowList.reverse();  // new windows are push front - but front should be drawn last?? testing
    for (GUIWindow *pWindow : tmpWindowList) {
        pWindow->Update();
    }

    if (GetCurrentMenuID() == MENU_NONE) {
        GameUI_DrawFoodAndGold();
    }

    if (isHoldingMouseRightButton()) {
        std::shared_ptr<Io::Mouse> mouse = EngineIocContainer::ResolveMouse();
        UI_OnMouseRightClick(mouse->position());
    }
}

//----- (004226EF) --------------------------------------------------------
void SetUserInterface(PartyAlignment align) {
    static std::optional<PartyAlignment> intAlign;
    if (intAlign == align) return;
    intAlign = align;

    extern void set_default_ui_skin_colors();
    set_default_ui_skin_colors();

    if (!parchment) {
        parchment = assets->getImage_ColorKey("parchment");
    }

    if (align == PartyAlignment::PartyAlignment_Evil) {
        game_ui_rightframe = assets->getImage_PCXFromIconsLOD("ib-r-C.pcx");
        game_ui_bottomframe = assets->getImage_PCXFromIconsLOD("ib-b-C.pcx");
        game_ui_topframe = assets->getImage_PCXFromIconsLOD("ib-t-C.pcx");
        game_ui_leftframe = assets->getImage_PCXFromIconsLOD("ib-l-C.pcx");
        game_ui_statusbar = assets->getImage_PCXFromIconsLOD("IB-Foot-c.pcx");

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

        game_ui_wizardEye = pIconsFrameTable->animationId("wizeyeC");
        game_ui_torchLight = pIconsFrameTable->animationId("torchC");

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

        uGameUIFontMain = colorTable.MediumRed;
        uGameUIFontShadow = colorTable.Diesel;
    } else if (align == PartyAlignment::PartyAlignment_Neutral) {
        game_ui_rightframe = assets->getImage_PCXFromIconsLOD("ib-r-A.pcx");
        game_ui_bottomframe = assets->getImage_PCXFromIconsLOD("ib-b-A.pcx");
        game_ui_topframe = assets->getImage_PCXFromIconsLOD("ib-t-A.pcx");
        game_ui_leftframe = assets->getImage_PCXFromIconsLOD("ib-l-A.pcx");
        game_ui_statusbar = assets->getImage_PCXFromIconsLOD("IB-Foot-a.pcx");

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

        game_ui_wizardEye = pIconsFrameTable->animationId("wizeyeA");
        game_ui_torchLight = pIconsFrameTable->animationId("torchA");

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

        uGameUIFontMain = colorTable.Diesel;
        uGameUIFontShadow = colorTable.StarkWhite;
    } else if (align == PartyAlignment::PartyAlignment_Good) {
        game_ui_rightframe = assets->getImage_PCXFromIconsLOD("ib-r-B.pcx");
        game_ui_bottomframe = assets->getImage_PCXFromIconsLOD("ib-b-B.pcx");
        game_ui_topframe = assets->getImage_PCXFromIconsLOD("ib-t-B.pcx");
        game_ui_leftframe = assets->getImage_PCXFromIconsLOD("ib-l-B.pcx");
        game_ui_statusbar = assets->getImage_PCXFromIconsLOD("IB-Foot-b.pcx");

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
        ui_exit_cancel_button_background = assets->getImage_ColorKey("ib-bcu-b");

        game_ui_playerbuff_bless = assets->getImage_ColorKey("isg-01-b");
        game_ui_playerbuff_preservation = assets->getImage_ColorKey("isg-02-b");
        game_ui_playerbuff_hammerhands = assets->getImage_ColorKey("isg-03-b");
        game_ui_playerbuff_pain_reflection = assets->getImage_ColorKey("isg-04-b");

        game_ui_wizardEye = pIconsFrameTable->animationId("wizeyeB");
        game_ui_torchLight = pIconsFrameTable->animationId("torchB");

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

        uGameUIFontMain = colorTable.MediumBlue;
        uGameUIFontShadow = colorTable.White;
    } else {
        assert(false);
    }

    UI_Create();
}

void DrawBuff_remaining_time_string(int uY, GUIWindow *window, Duration remaining_time, GUIFont *Font) {
    window->DrawText(Font, {32, uY}, colorTable.White, "\r020" + MakeDateTimeString(remaining_time));
}

bool isHoldingMouseRightButton() {
    return holdingMouseRightButton;
}

Color GetSkillColor(Class uPlayerClass, Skill uPlayerSkillType, Mastery skill_mastery) {
    if (skillMaxMasteryPerClass[uPlayerClass][uPlayerSkillType] >= skill_mastery) {
        return ui_character_skillinfo_can_learn;
    }
    for (Class promotionClass : promotionsForClass(uPlayerClass)) {
        if (skillMaxMasteryPerClass[promotionClass][uPlayerSkillType] >= skill_mastery) {
            return ui_character_skillinfo_can_learn_gm;
        }
    }
    return ui_character_skillinfo_cant_learn;
}

std::string BuildDialogueString(std::string_view str, int uPlayerID, NPCData *npc, Item *item, HouseId houseId, ShopScreen shop_screen, Time *a6) {
    std::string v1;
    Character *pPlayer;       // ebx@3
    int v29;               // eax@68
    std::vector<AwardId> addressingBits;
    CivilTime time;
    std::string result;

    pPlayer = &pParty->pCharacters[uPlayerID];

    for (int i = 0, dst = 0; i < str.length(); ++i) {
        char c = str[i];  // skip through string till we find insertion point
        if (c != '%') {
            result += c;  // add char to result string
        } else {
            int mask = 10 * (int)(str[i + 1] - '0') + str[i + 2] - '0';  // mask tells what the gap needs filling with
            i += 2;

            switch (mask) {
            case 1:
                result += npc->name;
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
                time = pParty->GetPlayingTime().toCivilTime();
                if (time.hour >= 11 && time.hour < 20) {
                    result += localization->str(LSTR_DAY_LOWERCASE);
                } else if (time.hour >= 5 && time.hour < 11) {
                    result += localization->str(LSTR_MORNING);
                } else {
                    result += localization->str(LSTR_EVENING);
                }
                // TODO(captainurist): ^ and what about night?
                break;
            case 6:
                if (pPlayer->uSex == SEX_FEMALE)
                    result += localization->str(LSTR_LADY_LOWERCASE);
                else
                    result += localization->str(LSTR_SIR_LOWERCASE);
                break;
            case 7:
                if (pPlayer->uSex == SEX_FEMALE)
                    result += localization->str(LSTR_LADY_CAPITALIZED);
                else
                    result += localization->str(LSTR_SIR_CAPITALIZED);
                break;
            case 8:
                // #mm6 remnant, this code simply won't work in mm7, relevant bits don't look like they are related
                // to titles.
                assert(false);
                for (int bit : possibleAddressingAwardBits) {
                    if (pPlayer->_achievedAwardsBits[static_cast<AwardId>(bit)]) {
                        addressingBits.push_back(static_cast<AwardId>(bit));
                    }
                }
                if (!addressingBits.empty()) {
                    AwardId currentAddressingAwardBit = addressingBits[vrng->random(addressingBits.size())];
                    result += pAwards[currentAddressingAwardBit].pText;
                } else {
                    result += pNPCTopics[55].pText; // #mm6 remnant, in mm7 that's a dialogue text for Thieves to Rogues promotion.
                }
                break;
            case 9:
                if (npc->uSex == SEX_FEMALE)
                    result += localization->str(LSTR_HER);
                else
                    result += localization->str(LSTR_HIS);
                break;
            case 10:
                if (pPlayer->uSex == SEX_FEMALE)
                    result += localization->str(LSTR_LADY_CAPITALIZED);
                else
                    result += localization->str(LSTR_LORD);
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
                if (npc->uSex == SEX_FEMALE)
                    result += localization->str(LSTR_SISTER);
                else
                    result += localization->str(LSTR_BROTHER);
                break;
            case 15:
                result += localization->str(LSTR_DAUGHTER);
                break;
            case 16:
                if (npc->uSex == SEX_FEMALE)
                    result += localization->str(LSTR_SISTER);
                else
                    result += localization->str(LSTR_BROTHER);
                break;
            case 17:  // hired npc text
            {
                unsigned pay_percentage = pNPCStats->pProfessions[npc->profession].uHirePrice / 100;
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
                if (engine->_currentLoadedMapId != MAP_INVALID)
                    result += pMapStats->pInfos[engine->_currentLoadedMapId].name;
                else
                    result += localization->str(LSTR_UNKNOWN);
                break;

            case 24:  // item name
                v1 = fmt::format("{::}{}\f00000\n", colorTable.PaleCanary.tag(), item->GetDisplayName());
                result += v1;
                break;

            case 25:  // base prices
                switch (shop_screen) {
                case SHOP_SCREEN_INVALID:
                    assert(false);
                    [[fallthrough]];
                case SHOP_SCREEN_BUY:
                    v29 = PriceCalculator::baseItemBuyingPrice(item->GetValue(), houseTable[houseId].fPriceMultiplier);
                    break;
                case SHOP_SCREEN_SELL:
                    v29 = PriceCalculator::baseItemSellingPrice(item->GetValue(), houseTable[houseId].fPriceMultiplier);
                    break;
                case SHOP_SCREEN_IDENTIFY:
                    v29 = PriceCalculator::baseItemIdentifyPrice(houseTable[houseId].fPriceMultiplier);
                    break;
                case SHOP_SCREEN_REPAIR:
                    v29 = PriceCalculator::baseItemRepairPrice(item->GetValue(), houseTable[houseId].fPriceMultiplier);
                    break;
                case SHOP_SCREEN_SELL_FOR_CHEAP:
                    v29 = PriceCalculator::baseItemSellingPrice(item->GetValue(), houseTable[houseId].fPriceMultiplier) / 2;
                    break;
                }
                v1 = fmt::format("{}", v29);
                result += v1;
                break;

            case 27:  // actual price
                v29 = PriceCalculator::itemBuyingPriceForPlayer(pPlayer, item->GetValue(), houseTable[houseId].fPriceMultiplier);
                if (shop_screen == SHOP_SCREEN_SELL) {
                    v29 = PriceCalculator::itemSellingPriceForPlayer(pPlayer, *item, houseTable[houseId].fPriceMultiplier);
                    v1 = fmt::format("{}", v29);
                    result += v1;
                    break;
                }
                if (shop_screen != SHOP_SCREEN_IDENTIFY) {
                    if (shop_screen == SHOP_SCREEN_REPAIR) {
                    v29 = PriceCalculator::itemRepairPriceForPlayer(
                        pPlayer, item->GetValue(),
                        houseTable[houseId].fPriceMultiplier);
                    } else {
                        if (shop_screen == SHOP_SCREEN_SELL_FOR_CHEAP) {
                            // TODO(captainurist): encapsulate this logic in PriceCalculator
                            v29 = PriceCalculator::itemSellingPriceForPlayer(pPlayer, *item, houseTable[houseId].fPriceMultiplier) / 2;
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
                v1 = fmt::format("{}", PriceCalculator::itemIdentificationPriceForPlayer(pPlayer, houseTable[houseId].fPriceMultiplier));
                result += v1;
                break;

            case 28:  // shop type - blacksmith ect..
                result += houseTable[houseId].pProprieterTitle;
                break;

            case 29:  // identify cost
                v1 = fmt::format("{}", PriceCalculator::itemIdentificationPriceForPlayer(pPlayer, houseTable[houseId].fPriceMultiplier));
                result += v1;
                break;
            case 30:
                if (!a6) {
                    // result += eventId;
                    assert(false); // should never get here?
                    break;
                }
                time = a6->toCivilTime();
                result += localization->format(LSTR_S_D_D, localization->monthName(time.month - 1), time.day, time.year);
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
                    assert(false); // should never get here?
                    break;
                }

                time = pParty->PartyTimes._s_times[mask - 51].toCivilTime();
                result += localization->format(LSTR_S_D_D, localization->monthName(time.month - 1), time.day, time.year);
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
        // game ui should never be released and should always be at the back of the window list
        if (pWindow->eWindowType == WINDOW_GameUI) {
            assert(false && "WINDOW_GameUI is not at back of lWindowList");
        }
        // Child books button should be deleted by parent
        if (pWindow->eWindowType == WINDOW_BooksButtonOverlay) {
            lWindowList.pop_front();
            continue;
        }
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

    current_screen_type = SCREEN_GAME;
    engine->_messageQueue->clearAll();

    // TODO(captainurist): Unload() un-pauses the event timer, which is not always the right thing to do.
    //                     So we hack. Find a better way.
    bool wasPaused = pEventTimer->isPaused();
    pMediaPlayer->Unload();
    pEventTimer->setPaused(wasPaused);
}

void MainMenuUI_LoadFontsAndSomeStuff() {
    // for (unsigned i = 0; i < 480; ++i) {  // must be 480 - needs sorting
    //     pSRZBufferLineOffsets[i] = 640 * i;  // must be 640 - needs sorting
    // }
    if (!assets->pFontArrus)
        assets->pFontArrus = GUIFont::LoadFont("arrus.fnt");
    if (!assets->pFontLucida)
        assets->pFontLucida = GUIFont::LoadFont("lucida.fnt");
    if (!assets->pFontCreate)
        assets->pFontCreate = GUIFont::LoadFont("create.fnt");
    if (!assets->pFontSmallnum)
        assets->pFontSmallnum = GUIFont::LoadFont("smallnum.fnt");
    if (!assets->pFontComic)
        assets->pFontComic = GUIFont::LoadFont("comic.fnt");
}

static void LoadPartyBuffIcons() {
    for (unsigned i = 0; i < 14; ++i)
        party_buff_icons[i] = assets->getImage_Buff(fmt::format("isn-{:02}", i + 1));

    uIconIdx_FlySpell = pIconsFrameTable->animationId("spell21");
    uIconIdx_WaterWalk = pIconsFrameTable->animationId("spell27");
}

void UI_Create() {
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

    if (pPrimaryWindow) pPrimaryWindow->Release();

    pPrimaryWindow = new GUIWindow(WINDOW_GameUI, {0, 0}, render->GetRenderDimensions());
    pPrimaryWindow->CreateButton({7, 8}, {460, 343}, 1, 0, UIMSG_MouseLeftClickInGame, 0);

    pPrimaryWindow->CreateButton("Game_Character1", {61, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 1, INPUT_ACTION_SELECT_CHAR_1);  // buttons for portraits
    pPrimaryWindow->CreateButton("Game_Character2", {177, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 2, INPUT_ACTION_SELECT_CHAR_2);
    pPrimaryWindow->CreateButton("Game_Character3", {292, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 3, INPUT_ACTION_SELECT_CHAR_3);
    pPrimaryWindow->CreateButton("Game_Character4", {407, 424}, {31, 40}, 2, 94, UIMSG_SelectCharacter, 4, INPUT_ACTION_SELECT_CHAR_4);

    pPrimaryWindow->CreateButton({ pHealthBarPos[0], pHealthManaBarYPos }, {5, 49}, 1, UIMSG_ShowStatus_ManaHP, UIMSG_0, 1);  // buttons for HP
    pPrimaryWindow->CreateButton({ pHealthBarPos[1], pHealthManaBarYPos }, {5, 49}, 1, UIMSG_ShowStatus_ManaHP, UIMSG_0, 2);
    pPrimaryWindow->CreateButton({ pHealthBarPos[2], pHealthManaBarYPos }, {5, 49}, 1, UIMSG_ShowStatus_ManaHP, UIMSG_0, 3);
    pPrimaryWindow->CreateButton({ pHealthBarPos[3], pHealthManaBarYPos }, {5, 49}, 1, UIMSG_ShowStatus_ManaHP, UIMSG_0, 4);

    pPrimaryWindow->CreateButton({ pManaBarPos[0], pHealthManaBarYPos }, {5, 49}, 1, UIMSG_ShowStatus_ManaHP, UIMSG_0, 1);  // buttons for SP
    pPrimaryWindow->CreateButton({ pManaBarPos[1], pHealthManaBarYPos }, {5, 49}, 1, UIMSG_ShowStatus_ManaHP, UIMSG_0, 2);
    pPrimaryWindow->CreateButton({ pManaBarPos[2], pHealthManaBarYPos }, {5, 49}, 1, UIMSG_ShowStatus_ManaHP, UIMSG_0, 3);
    pPrimaryWindow->CreateButton({ pManaBarPos[3], pHealthManaBarYPos }, {5, 49}, 1, UIMSG_ShowStatus_ManaHP, UIMSG_0, 4);

    game_ui_tome_quests = assets->getImage_ColorKey("ib-td1-A");
    pBtn_Quests = pPrimaryWindow->CreateButton({491, 353}, game_ui_tome_quests->size(), 1, 0,
                                               UIMSG_OpenQuestBook, 0, INPUT_ACTION_OPEN_QUESTS,
                                               localization->str(LSTR_CURRENT_QUESTS), { game_ui_tome_quests });

    game_ui_tome_autonotes = assets->getImage_ColorKey("ib-td2-A");
    pBtn_Autonotes = pPrimaryWindow->CreateButton({527, 353}, game_ui_tome_autonotes->size(), 1, 0,
                                                  UIMSG_OpenAutonotes, 0, INPUT_ACTION_OPEN_AUTONOTES,
                                                  localization->str(LSTR_AUTO_NOTES), { game_ui_tome_autonotes });

    game_ui_tome_maps = assets->getImage_ColorKey("ib-td3-A");
    pBtn_Maps = pPrimaryWindow->CreateButton({546, 353}, game_ui_tome_maps->size(), 1, 0,
                                             UIMSG_OpenMapBook, 0, INPUT_ACTION_OPEN_MAP,
                                             localization->str(LSTR_MAPS), { game_ui_tome_maps });

    game_ui_tome_calendar = assets->getImage_ColorKey("ib-td4-A");
    pBtn_Calendar = pPrimaryWindow->CreateButton({570, 353}, game_ui_tome_calendar->size(), 1, 0,
                                                 UIMSG_OpenCalendar, 0, INPUT_ACTION_OPEN_CALENDAR,
                                                 localization->str(LSTR_CALENDAR), { game_ui_tome_calendar });

    game_ui_tome_storyline = assets->getImage_ColorKey("ib-td5-A");
    pBtn_History = pPrimaryWindow->CreateButton({600, 361}, game_ui_tome_storyline->size(), 1, 0,
                                                UIMSG_OpenHistoryBook, 0, INPUT_ACTION_OPEN_HISTORY,
                                                localization->str(LSTR_HISTORY), { game_ui_tome_storyline }
    );

    bFlashAutonotesBook = false;
    bFlashQuestBook = false;
    bFlashHistoryBook = false;

    pBtn_ZoomIn = pPrimaryWindow->CreateButton({519, 136}, game_ui_btn_zoomin->size(), 2, 0,
                                               UIMSG_ClickZoomInBtn, 0, INPUT_ACTION_ZOOM_IN,
                                               localization->str(LSTR_ZOOM_IN), { game_ui_btn_zoomin }
    );

    pBtn_ZoomOut = pPrimaryWindow->CreateButton({574, 136}, game_ui_btn_zoomout->size(), 2, 0,
                                                UIMSG_ClickZoomOutBtn, 0, INPUT_ACTION_ZOOM_OUT,
                                                localization->str(LSTR_ZOOM_OUT), { game_ui_btn_zoomout });

    pPrimaryWindow->CreateButton({484, 15}, {138, 116}, 1, UIMSG_ShowStatus_DateTime, UIMSG_0, 0);
    pPrimaryWindow->CreateButton({491, 149}, {64, 74}, 1, 0, UIMSG_StartHireling1Dialogue, 0, INPUT_ACTION_SELECT_NPC_1);
    pPrimaryWindow->CreateButton({561, 149}, {64, 74}, 1, 0, UIMSG_StartHireling2Dialogue, 0, INPUT_ACTION_SELECT_NPC_2);
    pPrimaryWindow->CreateButton({476, 322}, {77, 17}, 1, UIMSG_ShowStatus_Food, UIMSG_0, 0);
    pPrimaryWindow->CreateButton({555, 322}, {77, 17}, 1, UIMSG_ShowStatus_Funds, UIMSG_0, 0);

    pBtn_CastSpell = pPrimaryWindow->CreateButton("Game_CastSpell", {476, 450}, game_ui_btn_cast->size(), 1, 0,
                                                  UIMSG_SpellBookWindow, 0, INPUT_ACTION_OPEN_SPELLBOOK,
                                                  localization->str(LSTR_CAST_SPELL), { game_ui_btn_cast });
    pBtn_Rest = pPrimaryWindow->CreateButton("Game_Rest", {518, 450}, game_ui_btn_rest->size(), 1, 0,
                                             UIMSG_RestWindow, 0, INPUT_ACTION_REST,
                                             localization->str(LSTR_REST), { game_ui_btn_rest });
    pBtn_QuickReference = pPrimaryWindow->CreateButton({560, 450}, game_ui_btn_quickref->size(), 1, 0,
                                                       UIMSG_QuickReference, 0, INPUT_ACTION_OPEN_QUICK_REFERENCE,
                                                       localization->str(LSTR_QUICK_REFERENCE), { game_ui_btn_quickref });
    pBtn_GameSettings = pPrimaryWindow->CreateButton({602, 450}, game_ui_btn_settings->size(), 1, 0,
                                                     UIMSG_GameMenuButton, 0, INPUT_ACTION_INVALID,
                                                     localization->str(LSTR_GAME_OPTIONS), { game_ui_btn_settings });
    pBtn_NPCLeft = pPrimaryWindow->CreateButton({469, 178}, ui_btn_npc_left->size(), 1, 0,
                                                UIMSG_ScrollNPCPanel, 0, INPUT_ACTION_INVALID, "", {ui_btn_npc_left });
    pBtn_NPCRight = pPrimaryWindow->CreateButton({626, 178}, ui_btn_npc_right->size(), 1, 0,
                                                 UIMSG_ScrollNPCPanel, 1, INPUT_ACTION_INVALID, "", {ui_btn_npc_right });

    LoadPartyBuffIcons();
}


std::string NameAndTitle(std::string_view name, std::string_view title) {
    return localization->format(LSTR_S_THE_S, name, title);
}


std::string NameAndTitle(std::string_view name, Class class_type) {
    return NameAndTitle(
        name,
        localization->className(class_type)
    );
}


std::string NameAndTitle(std::string_view name, NpcProfession profession) {
    return NameAndTitle(
        name,
        localization->npcProfessionName(profession)
    );
}


std::string NameAndTitle(NPCData *npc) {
    if (!npc->name.empty()) {
        if (npc->profession != NoProfession)
            return NameAndTitle(npc->name, npc->profession);

        return npc->name;
    }

    return std::string();
}


std::string GetDisplayName(Actor *actor) {
    if (actor->uniqueNameIndex)
        return pMonsterStats->uniqueNames[actor->uniqueNameIndex];
    else
        return pMonsterStats->infos[actor->monsterInfo.id].name;
}
