#include "GUI/GUIWindow.h"

#include <array>
#include <cstdlib>
#include <string>
#include <sstream>
#include <utility>
#include <list>
#include <memory>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/GameType.h"
#include "Engine/AssetsManager.h"
#include "Engine/EngineCallObserver.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/Random/Random.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Party.h"
#include "Engine/PriceCalculator.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/Tables/AwardTable.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Timer.h"
#include "Engine/Tables/MapTable.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UICharacter.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIPopup.h"
#include "GUI/UI/UIDialogue.h"
#include "GUI/UI/UIBranchlessDialogue.h"
#include "GUI/UI/UIGameOver.h"
#include "GUI/UI/UISpell.h"
#include "GUI/UI/UIMessageScroll.h"
#include "GUI/UI/UIChest.h"

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

std::unique_ptr<GUIWindow> pPrimaryWindow;
std::unique_ptr<GUIWindow> pGUIWindow_CurrentMenu;
std::unique_ptr<GUIWindow_Chest> pGUIWindow_CurrentChest;
std::unique_ptr<GUIWindow_House> window_SpeakInHouse;
std::unique_ptr<GUIWindow> pDialogueWindow;
std::unique_ptr<GUIWindow_MessageScroll> pGUIWindow_ScrollWindow; // reading a message scroll
std::unique_ptr<TargetedSpellUI> pGUIWindow_CastTargetedSpell;
std::unique_ptr<GUIWindow_GameOver> pGameOverWindow;
std::unique_ptr<GUIWindow_BranchlessDialogue> pGUIWindow_BranchlessDialogue;

WindowType current_character_screen_window = WINDOW_CHARACTER_STATS;
std::list<GUIWindow*> lWindowList;

MenuType sCurrentMenuID;

ScreenType current_screen_type = SCREEN_VIDEO;
ScreenType prev_screen_type;

std::shared_ptr<GraphicsImage> ui_exit_cancel_button_background;
std::shared_ptr<GraphicsImage> game_ui_right_panel_frame;
std::shared_ptr<GraphicsImage> dialogue_ui_x_ok_u;
std::shared_ptr<GraphicsImage> dialogue_ui_x_x_u;

std::shared_ptr<GraphicsImage> ui_buttdesc2;
std::shared_ptr<GraphicsImage> ui_buttyes2;

std::shared_ptr<GraphicsImage> ui_btn_npc_right;
std::shared_ptr<GraphicsImage> ui_btn_npc_left;

std::shared_ptr<GraphicsImage> ui_ar_dn_dn;
std::shared_ptr<GraphicsImage> ui_ar_dn_up;
std::shared_ptr<GraphicsImage> ui_ar_up_dn;
std::shared_ptr<GraphicsImage> ui_ar_up_up;

std::shared_ptr<GraphicsImage> ui_leather_mm6;
std::shared_ptr<GraphicsImage> ui_leather_mm7;

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

GUIWindow::~GUIWindow() {
    DeleteButtons();
    lWindowList.remove(this);
    // TODO(captainurist): logger can be NULL here if we're called from cxa_finalize.
    MM_TRACE("Release window: {}", toString(eWindowType));
}

void GUIWindow::DeleteButtons() {
    while (vButtons.size())
        vButtons.front()->Release();
}

GUIButton *GUIWindow::GetControl(unsigned int uID) {
    if (uID >= vButtons.size())
        return nullptr;

    return vButtons[uID];
}

// TODO(pskelton): this function may modify frameRect - extract out
void GUIWindow::DrawMessageBox(Recti& frameRect, std::string sHint) {
    // TODO(pskelton): Derived Messagebox types for different kinds of popup boxes
    if (engine->callObserver)
        engine->callObserver->notify(CALL_DRAW_MESSAGE_BOX, sHint);

    Sizei renDims = render->GetRenderDimensions();

    Pointi cursor = EngineIocContainer::ResolveMouse()->position();
    if (frameRect.x >= 0) {
        if (frameRect.w + frameRect.x > renDims.w) {
            frameRect.x = renDims.w - frameRect.w;
            frameRect.y = cursor.y + 30;
        }
    } else {
        frameRect.x = 0;
        frameRect.y = cursor.y + 30;
    }

    if (frameRect.y >= 0) {
        if (frameRect.y + frameRect.h > renDims.h)
            frameRect.y = cursor.y - frameRect.h - 30;
    } else {
        frameRect.y = cursor.y + 30;
    }
    if (frameRect.y < 0)
        frameRect.y = 0;
    if (frameRect.x < 0)
        frameRect.x = 0;

    Recti current_window = frameRect;
    current_window.x += 12;
    current_window.w -= 28;
    current_window.y += 12;
    current_window.h -= 12;
    unsigned int uBoxHeight;
    if (!sHint.empty()) {
        uBoxHeight = assets->pFontLucida->CalcTextHeight(sHint, current_window.w, 0) + 24;
    } else {
        uBoxHeight = frameRect.h;
    }
    if (uBoxHeight < 64)
        uBoxHeight = 64;
    if ((int)(uBoxHeight + frameRect.y) > 479)
        uBoxHeight = 479 - frameRect.y;
    DrawPopupWindow(frameRect.x, frameRect.y, frameRect.w, uBoxHeight);
    if (!sHint.empty()) {
        DrawTitleText(
            assets->pFontLucida.get(),
            0, (int)(uBoxHeight - assets->pFontLucida->CalcTextHeight(sHint, current_window.w, 0)) / 2 - 14,
            colorTable.White, sHint, 3, current_window);
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
void GUIWindow::DrawShops_next_generation_time_string(Duration time, Recti frameRect) {
    auto str = MakeDateTimeString(time);
    DrawTitleText(assets->pFontArrus.get(), 0,
                  (212 - assets->pFontArrus->CalcTextHeight(str, frameRect.w, 0)) / 2 + 101, colorTable.PaleCanary,
                  localization->str(LSTR_PLEASE_TRY_BACK_IN) + str, 3, frameRect);
}

//----- (0044D406) --------------------------------------------------------
void GUIWindow::DrawTitleText(GUIFont *pFont, int horizontalMargin, int verticalMargin, Color color, std::string_view text, int lineSpacing, Recti frameRect) {
    if (engine->callObserver)
        engine->callObserver->notify(CALL_GUIWINDOW_DRAWTEXT, std::string(text));
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

    // Wrap/measure against the leather panel's own width, not pViewport.w: MM8's pViewport is the full 640px frame
    // (the 3D viewport), but this panel's background is the narrower "LEATHER" texture drawn at a fixed width below -
    // using pViewport.w here wrapped text far wider than the visible panel, overflowing it.
    int leatherWidth = ui_leather_mm7->width();
    int frameWidth = leatherWidth;
    int indent = 12;

    GUIFont *font = assets->pFontArrus.get();
    int textHeight = font->CalcTextHeight(text, frameWidth, indent) + 7;
    if (352 - textHeight < 8) {
        font = assets->pFontCreate.get();
        textHeight = font->CalcTextHeight(text, frameWidth, indent) + 7;
    }

    render->DrawQuad2D(ui_leather_mm7, Recti(0, 0, leatherWidth, textHeight), Recti(8, 352 - textHeight, leatherWidth, textHeight));
    render->DrawQuad2D(_591428_endcap, {8, 347 - textHeight});
    DrawText(font, {indent, 354 - textHeight}, colorTable.White, font->WrapText(text, frameWidth, indent), frameRect);
}

//----- (0044CE08) --------------------------------------------------------
void GUIWindow::DrawText(GUIFont *font, Pointi position, Color color, std::string_view text, Recti frameRect, int maxY, Color shadowColor) {
    if (engine->callObserver)
        engine->callObserver->notify(CALL_GUIWINDOW_DRAWTEXT, std::string(text));
    font->DrawText(frameRect, position, color, text, maxY, shadowColor);
}

//----- (0044CB4F) --------------------------------------------------------
int GUIWindow::DrawTextInRect(GUIFont *pFont, Pointi position,
                              Color uColor, std::string_view text, int rect_width,
                              int reverse_text) {
    return pFont->DrawTextInRect(frameRect, position, uColor, text, rect_width, reverse_text);
}

GUIButton *GUIWindow::CreateButton(Pointi position, Sizei dimensions,
                                   ButtonType uButtonType, int uData, UIMessageType msg,
                                   unsigned int msg_param, InputAction action,
                                   std::string_view label,
                                   std::vector<std::shared_ptr<GraphicsImage>> textures) {
    GUIButton *pButton = new GUIButton();

    pButton->pParent = this;

    // Original code used closed intervals [uX, uZ] where uZ = uX + uWidth, so the button covered
    // uWidth + 1 pixels. With standard half-open Rect semantics [x, x+w), we store w + 1 to match.
    pButton->rect = Recti(position.x + frameRect.x, position.y + frameRect.y, dimensions.w + 1, dimensions.h + 1);
    pButton->uButtonType = uButtonType;
    pButton->field_2C_is_pushed = false;
    pButton->uData = uData;
    pButton->msg = msg;
    pButton->msg_param = msg_param;
    pButton->action = action;
    pButton->label = label;
    pButton->vTextures = std::move(textures);

    vButtons.push_back(pButton);

    return pButton;
}

GUIButton *GUIWindow::CreateButton(std::string id, Pointi position, Sizei dimensions, ButtonType uButtonType, int uData,
                        UIMessageType msg, unsigned int msg_param, InputAction action, std::string_view label,
                        std::vector<std::shared_ptr<GraphicsImage>> textures) {
    GUIButton *result = CreateButton(position, dimensions, uButtonType, uData, msg, msg_param, action, label, std::move(textures));
    result->id = std::move(id);
    return result;
}

void GUIWindow::CreateCharacterButtons() {
    if (gameType() == GAME_TYPE_MM8) {
        static constexpr std::array<InputAction, 5> actions = {{INPUT_ACTION_SELECT_CHAR_1, INPUT_ACTION_SELECT_CHAR_2, INPUT_ACTION_SELECT_CHAR_3,
                                                               INPUT_ACTION_SELECT_CHAR_4, INPUT_ACTION_SELECT_CHAR_5}};
        for (int slot = 0; slot < 5; slot++)
            CreateButton(fmt::format("Game_Character{}", slot + 1), { 29 + mm8PortraitSlotShift(slot), 383 }, { 63, 81 }, BUTTON_TYPE_CHARACTER, 94,
                         UIMSG_SelectCharacter, slot, actions[slot]);
        for (int slot = 0; slot < 5; slot++)  // HP and SP bars stand side by side in one frame
            CreateButton({ 82 + mm8PortraitSlotShift(slot), 428 }, { 10, 52 }, BUTTON_TYPE_NORMAL, UIMSG_ShowStatus_ManaHP, UIMSG_0, slot + 1);
        return;
    }

    CreateButton("Game_Character1", { 29, 383 }, { 63, 81 }, BUTTON_TYPE_CHARACTER, 94, UIMSG_SelectCharacter, 0, INPUT_ACTION_SELECT_CHAR_1);  // buttons for portraits
    CreateButton("Game_Character2", { 145, 383 }, { 63, 81 }, BUTTON_TYPE_CHARACTER, 94, UIMSG_SelectCharacter, 1, INPUT_ACTION_SELECT_CHAR_2);
    CreateButton("Game_Character3", { 260, 383 }, { 63, 81 }, BUTTON_TYPE_CHARACTER, 94, UIMSG_SelectCharacter, 2, INPUT_ACTION_SELECT_CHAR_3);
    CreateButton("Game_Character4", { 375, 383 }, { 63, 81 }, BUTTON_TYPE_CHARACTER, 94, UIMSG_SelectCharacter, 3, INPUT_ACTION_SELECT_CHAR_4);

    CreateButton({ pHealthBarPos[0], pHealthManaBarYPos }, { 5, 49 }, BUTTON_TYPE_NORMAL, UIMSG_ShowStatus_ManaHP, UIMSG_0, 1);  // buttons for HP
    CreateButton({ pHealthBarPos[1], pHealthManaBarYPos }, { 5, 49 }, BUTTON_TYPE_NORMAL, UIMSG_ShowStatus_ManaHP, UIMSG_0, 2);
    CreateButton({ pHealthBarPos[2], pHealthManaBarYPos }, { 5, 49 }, BUTTON_TYPE_NORMAL, UIMSG_ShowStatus_ManaHP, UIMSG_0, 3);
    CreateButton({ pHealthBarPos[3], pHealthManaBarYPos }, { 5, 49 }, BUTTON_TYPE_NORMAL, UIMSG_ShowStatus_ManaHP, UIMSG_0, 4);

    CreateButton({ pManaBarPos[0], pHealthManaBarYPos }, { 5, 49 }, BUTTON_TYPE_NORMAL, UIMSG_ShowStatus_ManaHP, UIMSG_0, 1);  // buttons for SP
    CreateButton({ pManaBarPos[1], pHealthManaBarYPos }, { 5, 49 }, BUTTON_TYPE_NORMAL, UIMSG_ShowStatus_ManaHP, UIMSG_0, 2);
    CreateButton({ pManaBarPos[2], pHealthManaBarYPos }, { 5, 49 }, BUTTON_TYPE_NORMAL, UIMSG_ShowStatus_ManaHP, UIMSG_0, 3);
    CreateButton({ pManaBarPos[3], pHealthManaBarYPos }, { 5, 49 }, BUTTON_TYPE_NORMAL, UIMSG_ShowStatus_ManaHP, UIMSG_0, 4);
}

bool GUIWindow::Contains(unsigned int x, unsigned int y) {
    return frameRect.contains(Pointi(x, y));
}

void GUIWindow::InitializeGUI() {
    SetUserInterface(PartyAlignment::PartyAlignment_Neutral);
    MainMenuUI_LoadFontsAndSomeStuff();
}

void GUIWindow::DrawFlashingInputCursor(int uX, int uY, GUIFont *a2, Recti frameRect) {
    // TODO(pskelton): check tickcount usage here
    if (platform->tickCount() % 1000 > 500)
        DrawText(a2, {uX, uY}, colorTable.White, "_", frameRect);
}

GUIWindow::GUIWindow(WindowType windowType, Pointi position, Sizei dimensions, std::string_view hint): eWindowType(windowType) {
    this->mouse = EngineIocContainer::ResolveMouse();

    MM_TRACE("New window: {}", toString(windowType));
    lWindowList.push_front(this);
    frameRect = Recti(position.x, position.y, dimensions.w, dimensions.h);

    this->sHint = hint;
    this->receives_keyboard_input = false;
}

void DialogueEnding() {
    speakingNpcId = 0;
    pDialogueWindow = nullptr;
    animTimer->setPaused(false);
    gameTimer->setPaused(false);
}

void OnButtonClick::Update() {
    if (_playSound)
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    render->DrawQuad2D(_button->vTextures[0], frameRect.topLeft());
    if (!sHint.empty())
        _button->DrawLabel(sHint, assets->pFontCreate.get(), colorTable.White);

    delete this;
}

void OnButtonClick3::Update() {
    pAudioPlayer->playUISound(SOUND_StartMainChoice02);

    render->DrawQuad2D(_button->vTextures[1], frameRect.topLeft());
    if (!sHint.empty())
        _button->DrawLabel(sHint, assets->pFontCreate.get(), colorTable.White);

    delete this;
}

void OnSaveLoad::Update() {
    if (!sHint.empty())
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    render->DrawQuad2D(_button->vTextures[0], frameRect.topLeft());
    if (!sHint.empty())
        _button->DrawLabel(sHint, assets->pFontCreate.get(), colorTable.White);

    if (current_screen_type == SCREEN_SAVEGAME) {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_SaveGame, 0, 0);
    } else {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_LoadGame, 0, 0);
    }

    delete this;
}

void OnCancel::Update() {
    if (sHint.empty())
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    render->DrawQuad2D(_button->vTextures[0], frameRect.topLeft());
    if (!sHint.empty())
        _button->DrawLabel(sHint, assets->pFontCreate.get(), colorTable.White);

    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);

    delete this;
}

void OnCancel2::Update() {
    if (!sHint.empty())
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);
    render->DrawQuad2D(_button->vTextures[1], frameRect.topLeft());
    if (!sHint.empty())
        _button->DrawLabel(sHint, assets->pFontCreate.get(), colorTable.White);

    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);

    delete this;
}

void OnCancel3::Update() {
    if (!sHint.empty())
        pAudioPlayer->playUISound(SOUND_StartMainChoice02);

    render->DrawQuad2D(_button->vTextures[0], frameRect.topLeft());
    if (!sHint.empty())
        _button->DrawLabel(sHint, assets->pFontCreate.get(), colorTable.White);

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
    for (GUIWindow *pWindow : tmpWindowList)
        pWindow->Update();

    if (GetCurrentMenuID() == MENU_NONE)
        GameUI_DrawFoodAndGold();

    if (isHoldingMouseRightButton()) {
        std::shared_ptr<Io::Mouse> mouse = EngineIocContainer::ResolveMouse();
        UI_OnMouseRightClick(mouse->position());
    }
}

//----- (004226EF) --------------------------------------------------------
void SetUserInterface(PartyAlignment align) {
    // TODO(captainurist): function-local static, the applied alignment should live next to the UI skin it caches.
    static std::optional<PartyAlignment> intAlign;
    if (intAlign == align) return;
    intAlign = align;

    extern void set_default_ui_skin_colors();
    set_default_ui_skin_colors();

    if (!parchment)
        parchment = assets->getIcon("parchment");

    if (align == PartyAlignment::PartyAlignment_Evil) {
        // MM8 ships no per-alignment border frame pieces at all (checked every LOD). It has a single combined viewport
        // border ("ui-frscr" in icons.UI.lod) instead of MM7's 5 alignment-tinted pieces. game_ui_leftframe/rightframe/
        // bottomframe/statusbar stay null. GameUI_DrawRightPanelFrames' null-safe DrawQuad2D calls just skip them.
        if (gameType() == GAME_TYPE_MM8) {
            game_ui_topframe = assets->getImage_ColorKeyFromIconsUILOD("ui-frscr");
        } else {
            game_ui_rightframe = assets->getImage_PCXFromIconsLOD("ib-r-C.pcx");
            game_ui_bottomframe = assets->getImage_PCXFromIconsLOD("ib-b-C.pcx");
            game_ui_topframe = assets->getImage_PCXFromIconsLOD("ib-t-C.pcx");
            game_ui_leftframe = assets->getImage_PCXFromIconsLOD("ib-l-C.pcx");
            game_ui_statusbar = assets->getImage_PCXFromIconsLOD("IB-Foot-c.pcx");
        }

        // MM8 ships exactly one (alignment-invariant) copy of "ib-mb-*": only the bare "-a"-suffixed name (matching
        // the Neutral branch's literal below) exists in any LOD.
        game_ui_right_panel_frame = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "ib-mb-A" : "ib-mb-C");
        // game_ui_minimap_frame/minimap_compass are overwritten by the MM8-only block below before UI_Create() draws
        // them, and MM8 ships no "ib-autmask-*"/"IB-COMP-*" asset under any alignment suffix, so the load is skipped for MM8.
        if (gameType() != GAME_TYPE_MM8) {
            game_ui_minimap_frame = assets->getImage_Alpha("ib-autmask-c");
            game_ui_minimap_compass = assets->getImage_ColorKey("IB-COMP-C");
        }

        // MM8 ships no "IB-Init*-a/b/c" alert-glow asset under any alignment suffix (checked every LOD). The draw side
        // never touches these globals for MM8 (GameUI_DrawPortraits' own `isMm8` no-op), so the load is skipped too.
        if (gameType() != GAME_TYPE_MM8) {
            game_ui_player_alert_green = assets->getImage_Alpha("IB-InitG-c");
            game_ui_player_alert_yellow = assets->getImage_Alpha("IB-InitY-c");
            game_ui_player_alert_red = assets->getImage_Alpha("IB-InitR-c");
        }

        // ui_btn_npc_left/right: MM8 ships no equivalent icon under any name searched, but these back real functional
        // buttons (CreateButton dereferences ->size(), and Game.cpp's hotkey handlers dereference the resulting GUIButton*),
        // so skip-drawing would risk a null-deref crash rather than a cosmetic gap. Still loaded for MM8.
        ui_btn_npc_left = assets->getImage_ColorKey("IB-NPCLD-C");
        ui_btn_npc_right = assets->getImage_ColorKey("IB-NPCRD-C");
        // MM8 minimap zoom buttons: alignment-invariant "map+up"/"map-up" (icons.lod, no per-alignment suffix variant
        // exists). CreateButton's own vTextures[0] only flashes for one frame on click, so GameUI_DrawZoomButtons redraws
        // them every frame.
        game_ui_btn_zoomin = assets->getImage_ColorKey(gameType() == GAME_TYPE_MM8 ? "map+up" : "ib-autout-C");
        game_ui_btn_zoomout = assets->getImage_ColorKey(gameType() == GAME_TYPE_MM8 ? "map-up" : "ib-autin-C");
        // MM8's HUD has no right-side character-selection indicator: it always shows the portrait ring instead (see
        // GameUI_DrawCharacterSelectionFrame's `isMm8` early-return), so the load is skipped.
        if (gameType() != GAME_TYPE_MM8) {
            game_ui_player_selection_frame = assets->getImage_ColorKey("IB-selec-C");
        }
        // MM8 ships real, alignment-invariant command-icon art ("butn1u".."butn4u" in icons.lod, no "ib-m*d-*" family),
        // under different names from MM7's. The original's top-bar icon row: rest=bedroll,
        // cast=hood, quickref=magnifying glass, settings=key. See the CreateButton call below for the matching MM8 position.
        game_ui_btn_cast = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "butn2u" : "ib-m1d-c");
        game_ui_btn_rest = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "butn1u" : "ib-m2d-c");
        game_ui_btn_quickref = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "butn3u" : "ib-m3d-c");
        game_ui_btn_settings = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "butn4u" : "ib-m4d-c");
        // ui_exit_cancel_button_background is overwritten by the MM8-only block below (real "c_close_up") before it's drawn.
        if (gameType() != GAME_TYPE_MM8) {
            ui_exit_cancel_button_background = assets->getImage_ColorKey("ib-bcu-c");
        }

        // MM8 ships no "isg-NN-a/b/c" party-buff icon under any alignment suffix (checked every LOD). GameUI_DrawPartySpells'
        // 4 draw calls are gated MM7-only to match (see UIGame.cpp).
        if (gameType() != GAME_TYPE_MM8) {
            game_ui_playerbuff_bless = assets->getImage_ColorKey("isg-01-c");
            game_ui_playerbuff_preservation = assets->getImage_ColorKey("isg-02-c");
            game_ui_playerbuff_hammerhands = assets->getImage_ColorKey("isg-03-c");
            game_ui_playerbuff_pain_reflection = assets->getImage_ColorKey("isg-04-c");
        }

        game_ui_wizardEye = pIconsFrameTable->animationId("wizeyeC");
        game_ui_torchLight = pIconsFrameTable->animationId("torchC");

        // MM8 ships exactly one (alignment-invariant) copy of each of these assets, named with the bare name the Neutral
        // branch below requests verbatim, so that name is used for every alignment.
        game_ui_evtnpc = assets->getImage_ColorKey(gameType() == GAME_TYPE_MM8 ? "evtnpc" : "evtnpc-c");
        ui_character_inventory_background = assets->getImage_ColorKey(gameType() == GAME_TYPE_MM8 ? "fr_inven" : "fr_inven-c");

        messagebox_corner_y = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "cornr_ll" : "cornr_ll-c");
        messagebox_corner_w = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "cornr_lr" : "cornr_lr-c");
        messagebox_corner_x = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "cornr_ul" : "cornr_ul-c");
        messagebox_corner_z = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "cornr_ur" : "cornr_ur-c");
        messagebox_border_bottom = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "edge_btm" : "edge_btm-c");
        messagebox_border_left = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "edge_lf" : "edge_lf-c");
        messagebox_border_right = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "edge_rt" : "edge_rt-c");
        messagebox_border_top = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "edge_top" : "edge_top-c");
        _591428_endcap = assets->getImage_ColorKey(gameType() == GAME_TYPE_MM8 ? "endcap" : "endcap-c");

        uGameUIFontMain = colorTable.MediumRed;
        uGameUIFontShadow = colorTable.Diesel;
    } else if (align == PartyAlignment::PartyAlignment_Neutral) {
        // See the Evil branch above for why MM8 takes a different path here.
        if (gameType() == GAME_TYPE_MM8) {
            game_ui_topframe = assets->getImage_ColorKeyFromIconsUILOD("ui-frscr");
        } else {
            game_ui_rightframe = assets->getImage_PCXFromIconsLOD("ib-r-A.pcx");
            game_ui_bottomframe = assets->getImage_PCXFromIconsLOD("ib-b-A.pcx");
            game_ui_topframe = assets->getImage_PCXFromIconsLOD("ib-t-A.pcx");
            game_ui_leftframe = assets->getImage_PCXFromIconsLOD("ib-l-A.pcx");
            game_ui_statusbar = assets->getImage_PCXFromIconsLOD("IB-Foot-a.pcx");
        }

        game_ui_right_panel_frame = assets->getImage_Alpha("ib-mb-A");
        // See the Evil branch above for why these dead loads are skipped for MM8.
        if (gameType() != GAME_TYPE_MM8) {
            game_ui_minimap_frame = assets->getImage_Alpha("ib-autmask-a");
            game_ui_minimap_compass = assets->getImage_ColorKey("IB-COMP-A");
        }

        if (gameType() != GAME_TYPE_MM8) {
            game_ui_player_alert_green = assets->getImage_Alpha("IB-InitG-a");
            game_ui_player_alert_yellow = assets->getImage_Alpha("IB-InitY-a");
            game_ui_player_alert_red = assets->getImage_Alpha("IB-InitR-a");
        }

        // ui_btn_npc_left/right are loaded for MM8 as well, see the Evil branch above.
        ui_btn_npc_left = assets->getImage_ColorKey("IB-NPCLD-A");
        ui_btn_npc_right = assets->getImage_ColorKey("IB-NPCRD-A");
        if (gameType() != GAME_TYPE_MM8) {
            game_ui_player_selection_frame = assets->getImage_ColorKey("IB-selec-A");
        }
        // See the Evil branch above for the real MM8 "butnNu" names.
        game_ui_btn_cast = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "butn2u" : "ib-m1d-a");
        game_ui_btn_rest = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "butn1u" : "ib-m2d-a");
        game_ui_btn_quickref = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "butn3u" : "ib-m3d-a");
        game_ui_btn_settings = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "butn4u" : "ib-m4d-a");
        // See the Evil branch above for the real MM8 "map+up"/"map-up" names.
        game_ui_btn_zoomin = assets->getImage_ColorKey(gameType() == GAME_TYPE_MM8 ? "map+up" : "ib-autout-a");
        game_ui_btn_zoomout = assets->getImage_ColorKey(gameType() == GAME_TYPE_MM8 ? "map-up" : "ib-autin-a");
        if (gameType() != GAME_TYPE_MM8) {
            ui_exit_cancel_button_background = assets->getImage_ColorKey("ib-bcu-a");
        }

        if (gameType() != GAME_TYPE_MM8) {
            game_ui_playerbuff_bless = assets->getImage_ColorKey("isg-01-a");
            game_ui_playerbuff_preservation = assets->getImage_ColorKey("isg-02-a");
            game_ui_playerbuff_hammerhands = assets->getImage_ColorKey("isg-03-a");
            game_ui_playerbuff_pain_reflection = assets->getImage_ColorKey("isg-04-a");
        }

        game_ui_wizardEye = pIconsFrameTable->animationId("wizeyeA");
        game_ui_torchLight = pIconsFrameTable->animationId("torchA");

        game_ui_evtnpc = assets->getIcon("evtnpc");
        ui_character_inventory_background = assets->getIcon("fr_inven");

        messagebox_corner_y = assets->getIcon("cornr_ll");
        messagebox_corner_w = assets->getIcon("cornr_lr");
        messagebox_corner_x = assets->getIcon("cornr_ul");
        messagebox_corner_z = assets->getIcon("cornr_ur");
        messagebox_border_bottom = assets->getIcon("edge_btm");
        messagebox_border_left = assets->getIcon("edge_lf");
        messagebox_border_right = assets->getIcon("edge_rt");
        messagebox_border_top = assets->getIcon("edge_top");
        _591428_endcap = assets->getIcon("endcap");

        uGameUIFontMain = colorTable.Diesel;
        uGameUIFontShadow = colorTable.StarkWhite;
    } else if (align == PartyAlignment::PartyAlignment_Good) {
        // See the Evil branch above for why MM8 takes a different path here.
        if (gameType() == GAME_TYPE_MM8) {
            game_ui_topframe = assets->getImage_ColorKeyFromIconsUILOD("ui-frscr");
        } else {
            game_ui_rightframe = assets->getImage_PCXFromIconsLOD("ib-r-B.pcx");
            game_ui_bottomframe = assets->getImage_PCXFromIconsLOD("ib-b-B.pcx");
            game_ui_topframe = assets->getImage_PCXFromIconsLOD("ib-t-B.pcx");
            game_ui_leftframe = assets->getImage_PCXFromIconsLOD("ib-l-B.pcx");
            game_ui_statusbar = assets->getImage_PCXFromIconsLOD("IB-Foot-b.pcx");
        }

        // MM8 ships only the "ib-mb-A" name, used here instead of "-B" (see the Evil branch above).
        game_ui_right_panel_frame = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "ib-mb-A" : "ib-mb-B");
        // See the Evil branch above for why these dead loads are skipped for MM8.
        if (gameType() != GAME_TYPE_MM8) {
            game_ui_minimap_frame = assets->getImage_Alpha("ib-autmask-b");
            game_ui_minimap_compass = assets->getImage_ColorKey("IB-COMP-B");
        }

        if (gameType() != GAME_TYPE_MM8) {
            game_ui_player_alert_green = assets->getImage_Alpha("IB-InitG-b");
            game_ui_player_alert_yellow = assets->getImage_Alpha("IB-InitY-b");
            game_ui_player_alert_red = assets->getImage_Alpha("IB-InitR-b");
        }

        // ui_btn_npc_left/right are loaded for MM8 as well, see the Evil branch above.
        ui_btn_npc_left = assets->getImage_ColorKey("IB-NPCLD-B");
        ui_btn_npc_right = assets->getImage_ColorKey("IB-NPCRD-B");
        // See the Evil branch above for the real MM8 "map+up"/"map-up" names.
        game_ui_btn_zoomin = assets->getImage_ColorKey(gameType() == GAME_TYPE_MM8 ? "map+up" : "ib-autout-B");
        game_ui_btn_zoomout = assets->getImage_ColorKey(gameType() == GAME_TYPE_MM8 ? "map-up" : "ib-autin-B");
        if (gameType() != GAME_TYPE_MM8) {
            game_ui_player_selection_frame = assets->getImage_ColorKey("IB-selec-B");
        }
        // See the Evil branch above for the real MM8 "butnNu" names.
        game_ui_btn_cast = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "butn2u" : "ib-m1d-b");
        game_ui_btn_rest = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "butn1u" : "ib-m2d-b");
        game_ui_btn_quickref = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "butn3u" : "ib-m3d-b");
        game_ui_btn_settings = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "butn4u" : "ib-m4d-b");
        if (gameType() != GAME_TYPE_MM8) {
            ui_exit_cancel_button_background = assets->getImage_ColorKey("ib-bcu-b");
        }

        if (gameType() != GAME_TYPE_MM8) {
            game_ui_playerbuff_bless = assets->getImage_ColorKey("isg-01-b");
            game_ui_playerbuff_preservation = assets->getImage_ColorKey("isg-02-b");
            game_ui_playerbuff_hammerhands = assets->getImage_ColorKey("isg-03-b");
            game_ui_playerbuff_pain_reflection = assets->getImage_ColorKey("isg-04-b");
        }

        game_ui_wizardEye = pIconsFrameTable->animationId("wizeyeB");
        game_ui_torchLight = pIconsFrameTable->animationId("torchB");

        // MM8 ships one copy of these assets under the bare name, see the Evil branch above.
        game_ui_evtnpc = assets->getImage_ColorKey(gameType() == GAME_TYPE_MM8 ? "evtnpc" : "evtnpc-b");
        ui_character_inventory_background = assets->getImage_ColorKey(gameType() == GAME_TYPE_MM8 ? "fr_inven" : "fr_inven-b");

        messagebox_corner_y = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "cornr_ll" : "cornr_ll-b");
        messagebox_corner_w = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "cornr_lr" : "cornr_lr-b");
        messagebox_corner_x = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "cornr_ul" : "cornr_ul-b");
        messagebox_corner_z = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "cornr_ur" : "cornr_ur-b");
        messagebox_border_bottom = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "edge_btm" : "edge_btm-b");
        messagebox_border_left = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "edge_lf" : "edge_lf-b");
        messagebox_border_right = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "edge_rt" : "edge_rt-b");
        messagebox_border_top = assets->getImage_Alpha(gameType() == GAME_TYPE_MM8 ? "edge_top" : "edge_top-b");
        _591428_endcap = assets->getImage_ColorKey(gameType() == GAME_TYPE_MM8 ? "endcap" : "endcap-b");

        uGameUIFontMain = colorTable.MediumBlue;
        uGameUIFontShadow = colorTable.White;
    } else {
        assert(false);
    }

    // MM8's always-on bottom-bar HUD (portrait ring + 4 heraldic banners) and top-bar food/gold icon strip don't vary by
    // party alignment like the ib-* set above, so they're loaded once here rather than duplicated in each branch.
    if (gameType() == GAME_TYPE_MM8) {
        if (!game_ui_scorebg) game_ui_scorebg = assets->getImage_ColorKey("scorebg");
        if (!game_ui_portrait_ring) game_ui_portrait_ring = assets->getImage_ColorKey("selring");
        if (!game_ui_party_banners[0]) {
            game_ui_party_banners[0] = assets->getImage_ColorKey("ibshield01");
            game_ui_party_banners[1] = assets->getImage_ColorKey("ibshield02");
            game_ui_party_banners[2] = assets->getImage_ColorKey("ibshield03");
            game_ui_party_banners[3] = assets->getImage_ColorKey("ibshield04");
        }
        // topbar/basebar are the bar chrome that scorebg/selring/ibshieldNN are drawn on top of, see
        // kMm8TopBarBgPos/kMm8BottomBarBgPos in UIGame.cpp.
        if (!game_ui_top_bar_bg) game_ui_top_bar_bg = assets->getImage_ColorKey("topbar");
        if (!game_ui_bottom_bar_bg) game_ui_bottom_bar_bg = assets->getImage_ColorKey("basebar");
        // Unlike the other top-bar sprites above, "asil00"/"asir00" ship with an opaque baked-in backdrop rather than a
        // cyan colorkey surround, so getImage_ColorKey would leave them wrongly punched-through. getImage_Solid matches
        // how ui_leather_mm7 and other baked-background sprites load in this file.
        if (!game_ui_topbar_figurine) game_ui_topbar_figurine = assets->getImage_Solid("asil00");
        if (!game_ui_topbar_skull) game_ui_topbar_skull = assets->getImage_Solid("asir00");

        // The real MM8 minimap frame is "mapframe" (icons.UI.lod, alignment-invariant). The alignment branches above load
        // MM7's "ib-autmask-*"/"IB-COMP-*" names, which are missing in every MM8 LOD. "mapframe"'s border art already bakes
        // in a static zoom/compass-rose decoration, and no separate rotating-compass-needle asset exists for MM8, so
        // game_ui_minimap_compass stays null (GameUI_DrawMinimap's DrawQuad2D is null-safe).
        // "mapframe"'s embedded palette keys its interior with (0,252,252), not colorTable.TealMask's
        // (0,255,255) - MakePaletteColorKey does an exact RGB match (ImageLoader.cpp), so the default
        // colorkey silently fails to punch out the interior (renders solid opaque cyan instead of
        // transparent). Every other MM8 HUD sprite ported so far (scorebg/selring/ibshieldNN/ui-frscr)
        // happens to have its zeroIsTransparent flag set instead, which ignores this colorkey param
        // entirely - that's why they didn't need this override.
        game_ui_minimap_frame = assets->getImage_ColorKeyFromIconsUILOD("mapframe", Color(0, 252, 252));
        game_ui_minimap_compass = nullptr;

        // MM8 ships the bare "-a" name `ib-comp-a` (icons.lod, 325x9, alignment-invariant). It is MM8's own top-bar
        // direction cartouche, not the minimap needle, which is folded into "mapframe" above. A horizontally-scrolled
        // direction-letter strip is drawn behind a narrow clip window, the same mechanic as the minimap-needle code below,
        // in a different screen slot (see GameUI_DrawCompass's draw call in UIGame.cpp for the position and clip window).
        // It is loaded into its own game_ui_compass_strip, not game_ui_minimap_compass (see its declaration in UIGame.cpp).
        game_ui_compass_strip = assets->getImage_ColorKey("ib-comp-a");
        // "compcovr" (icons.UI.lod, 49x22) is the gold cover frame drawn on top, with a colorkeyed hole punched exactly over
        // the compass strip's clip window (screen (306,10)-(332,19) when the frame sits at (295,4)).
        game_ui_compass_frame = assets->getImage_ColorKeyFromIconsUILOD("compcovr");

        // MM8's real portrait-adjacent HP/SP bar. "statbl"/"statg"/"staty"/"statr" (loaded in UI_Create) are a small
        // 19x23 status gem, not a bar. The real fill/frame assets are icons.lod's "mana*" family: "manag"/"manay"/"manar"
        // (green/yellow/red, 3x49) and "manab" (blue/mana, 3x49) match the vertical bar's 3px width and 49px height, placed
        // right of the portrait ring at x=85. "manafrm" (10x51) is the enclosing frame.
        // See GameUI_DrawLifeManaBars in UIGame.cpp for the MM8 draw branch.
        game_ui_mm8_bar_frame = assets->getImage_ColorKey("manafrm");
        game_ui_mm8_bar_green = assets->getImage_ColorKey("manag");
        game_ui_mm8_bar_yellow = assets->getImage_ColorKey("manay");
        game_ui_mm8_bar_red = assets->getImage_ColorKey("manar");
        game_ui_mm8_bar_blue = assets->getImage_ColorKey("manab");

        // The real MM8 "Close" button plaque is "c_close_up" (EnglishD.lod, 83x30, alignment-invariant). The alignment
        // branches above load MM7's "ib-bcu-*" names, which don't exist in any MM8 LOD searched and resolve to a
        // missing-texture placeholder. sLabel captions drawn over this asset (e.g. LSTR_CANCEL) are a status-
        // bar hover hint only (GUIButton.h), not drawn on the button face, so they don't visually
        // conflict with "Close" already baked into the art.
        ui_exit_cancel_button_background = assets->getImage_ColorKeyFromEnglishDLOD("c_close_up");
    }

    UI_Create();
}

void DrawBuff_remaining_time_string(int uY, Recti window, Duration remaining_time, GUIFont *Font) {
    GUIWindow::DrawText(Font, {32, uY}, colorTable.White, "\r020" + MakeDateTimeString(remaining_time), window);
}

bool isHoldingMouseRightButton() {
    return holdingMouseRightButton;
}

Color GetSkillColor(Class uPlayerClass, Skill uPlayerSkillType, Mastery skill_mastery) {
    if (skillMaxMasteryForClass(uPlayerClass, uPlayerSkillType) >= skill_mastery) {
        return ui_character_skillinfo_can_learn;
    }
    // MM8 has a single (race-bound) promotion rather than MM7's light/dark tier tree, so `promotionsForClass`
    // (which relies on MM7's tier bit encoding) doesn't apply - check the one MM8 promotion class directly.
    if (isMm8Class(uPlayerClass)) {
        if (isMm8BaseClass(uPlayerClass)) {
            Class promotedClass = mm8PromotedClassForRace(mm8RaceForClass(uPlayerClass));
            if (skillMaxMasteryForClass(promotedClass, uPlayerSkillType) >= skill_mastery) {
                return ui_character_skillinfo_can_learn_gm;
            }
        }
    } else {
        for (Class promotionClass : promotionsForClass(uPlayerClass)) {
            if (skillMaxMasteryForClass(promotionClass, uPlayerSkillType) >= skill_mastery) {
                return ui_character_skillinfo_can_learn_gm;
            }
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
                if (pPlayer->uSex == SEX_FEMALE) {
                    result += localization->str(LSTR_LADY_LOWERCASE);
                } else {
                    result += localization->str(LSTR_SIR_LOWERCASE);
                }
                break;
            case 7:
                if (pPlayer->uSex == SEX_FEMALE) {
                    result += localization->str(LSTR_LADY_CAPITALIZED);
                } else {
                    result += localization->str(LSTR_SIR_CAPITALIZED);
                }
                break;
            case 8:
                // #mm6 remnant, this code simply won't work in mm7, relevant bits don't look like they are related
                // to titles.
                assert(false);
                for (int bit : possibleAddressingAwardBits)
                    if (pPlayer->_achievedAwardsBits[static_cast<AwardId>(bit)])
                        addressingBits.push_back(static_cast<AwardId>(bit));
                if (!addressingBits.empty()) {
                    AwardId currentAddressingAwardBit = addressingBits[vrng->random(addressingBits.size())];
                    result += pAwards[currentAddressingAwardBit].pText;
                } else {
                    result += pNPCTopics[55].pText; // #mm6 remnant, in mm7 that's a dialogue text for Thieves to Rogues promotion.
                }
                break;
            case 9:
                if (npc->sex == SEX_FEMALE) {
                    result += localization->str(LSTR_HER);
                } else {
                    result += localization->str(LSTR_HIS);
                }
                break;
            case 10:
                if (pPlayer->uSex == SEX_FEMALE) {
                    result += localization->str(LSTR_LADY_CAPITALIZED);
                } else {
                    result += localization->str(LSTR_LORD);
                }
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
                if (npc->sex == SEX_FEMALE) {
                    result += localization->str(LSTR_SISTER);
                } else {
                    result += localization->str(LSTR_BROTHER);
                }
                break;
            case 15:
                result += localization->str(LSTR_DAUGHTER);
                break;
            case 16:
                if (npc->sex == SEX_FEMALE) {
                    result += localization->str(LSTR_SISTER);
                } else {
                    result += localization->str(LSTR_BROTHER);
                }
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
                if (engine->_currentLoadedMapId != MAP_INVALID) {
                    result += pMapTable->pInfos[engine->_currentLoadedMapId].name;
                } else {
                    result += localization->str(LSTR_UNKNOWN);
                }
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
                    } else if (shop_screen == SHOP_SCREEN_SELL_FOR_CHEAP) {
                        // TODO(captainurist): encapsulate this logic in PriceCalculator
                        v29 = PriceCalculator::itemSellingPriceForPlayer(pPlayer, *item, houseTable[houseId].fPriceMultiplier) / 2;
                        if (!v29)  // cannot be 0
                            v29 = 1;
                        v1 = fmt::format("{}", v29);
                        result += v1;
                        break;
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
    pDialogueWindow = nullptr;
    pGUIWindow_ScrollWindow = nullptr;
    pGUIWindow_CastTargetedSpell = nullptr;
    pGameOverWindow = nullptr;
    pGUIWindow_BranchlessDialogue = nullptr;
    window_SpeakInHouse = nullptr;
    pGUIWindow_CurrentChest = nullptr;
    pGUIWindow_CurrentMenu = nullptr;

    // Only primary window should be left now
    assert(lWindowList.size() == 1 && "Leaking GUIWindows");

    // reset screen state after deleting all windows
    current_screen_type = SCREEN_GAME;
    engine->_messageQueue->clearAll();

    // TODO(captainurist): Unload() un-pauses the game timer, which is not always the right thing to do.
    //                     So we hack. Find a better way.
    bool wasPaused = gameTimer->isPaused();
    pMediaPlayer->Unload();
    gameTimer->setPaused(wasPaused);
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
    // MM8 ships no "isn-NN" asset under any name searched (icons.lod, icons.UI.lod, EnglishD.lod). GameUI_DrawPartySpells
    // (UIGame.cpp) is gated MM7-only to match.
    if (gameType() != GAME_TYPE_MM8) {
        for (unsigned i = 0; i < 14; ++i)
            party_buff_icons[i] = assets->getImage_Buff(fmt::format("isn-{:02}", i + 1));
    }

    uIconIdx_FlySpell = pIconsFrameTable->animationId("spell21");
    uIconIdx_WaterWalk = pIconsFrameTable->animationId("spell27");
}

void UI_Create() {
    game_ui_minimap_dirs[0] = assets->getIcon("MAPDIR1");
    game_ui_minimap_dirs[1] = assets->getIcon("MAPDIR2");
    game_ui_minimap_dirs[2] = assets->getIcon("MAPDIR3");
    game_ui_minimap_dirs[3] = assets->getIcon("MAPDIR4");
    game_ui_minimap_dirs[4] = assets->getIcon("MAPDIR5");
    game_ui_minimap_dirs[5] = assets->getIcon("MAPDIR6");
    game_ui_minimap_dirs[6] = assets->getIcon("MAPDIR7");
    game_ui_minimap_dirs[7] = assets->getIcon("MAPDIR8");

    // MM8's status-gem sprites are "statbl"/"statg"/"staty"/"statr" (icons.lod). GameUI_DrawLifeManaBars (UIGame.cpp)
    // draws green/yellow/red for MM8.
    game_ui_bar_blue = assets->getImage_ColorKey("statbl");
    game_ui_bar_green = assets->getImage_ColorKey("statg");
    game_ui_bar_yellow = assets->getImage_ColorKey("staty");
    game_ui_bar_red = assets->getImage_ColorKey("statr");
    game_ui_monster_hp_background = assets->getImage_ColorKey("mhp_bg");
    game_ui_monster_hp_border_left = assets->getImage_ColorKey("mhp_capl");
    game_ui_monster_hp_border_right = assets->getImage_ColorKey("mhp_capr");
    game_ui_monster_hp_green = assets->getImage_ColorKey("mhp_grn");
    game_ui_monster_hp_red = assets->getImage_ColorKey("mhp_red");
    game_ui_monster_hp_yellow = assets->getImage_ColorKey("mhp_yel");
    ui_leather_mm7 = assets->getImage_Solid("LEATHER");
    // Confirmation-dialogue chrome (BUTTESC2/BUTTYES2 click-flash backgrounds, x_x_u/x_ok_u per-frame icon overlays, used
    // by UIHouses.cpp/UITransition.cpp/UIInventory.cpp) has no MM8 equivalent in any LOD searched, so MM8 skips drawing it
    // (DrawQuad2D/GUIButton::vTextures are null-safe). A matching MM8 button family exists (c_ok_up/c_cncl_up in
    // EnglishD.lod, used by UIPartyCreation.cpp's OK/Cancel), but its on-screen position for these dialogues is not measured.
    // UIQuickReference.cpp's Close button reuses ui_exit_cancel_button_background ("c_close_up") instead, unconditionally of this MM7-only load.
    if (gameType() != GAME_TYPE_MM8) {
        dialogue_ui_x_x_u = assets->getImage_ColorKey("x_x_u");
        ui_buttdesc2 = assets->getImage_Alpha("BUTTESC2");
        dialogue_ui_x_ok_u = assets->getImage_ColorKey("x_ok_u");
        ui_buttyes2 = assets->getImage_Alpha("BUTTYES2");
    }

    pPrimaryWindow = std::make_unique<GUIWindow>(WINDOW_GAME_UI, Pointi{0, 0}, render->GetRenderDimensions());
    // The "click in the 3D world" catcher is MM7-viewport-shaped ({7,8}-{467,351}). MM8's top-bar HUD occupies
    // (0,0)-(640,29) and its command icons sit at y=2, so for MM8 the top edge is raised to clear the strip.
    // The bottom edge is unchanged.
    Pointi gameClickCatcherPos = {7, 8};
    Sizei gameClickCatcherSize = {460, 343};
    if (gameType() == GAME_TYPE_MM8) {
        gameClickCatcherPos = {7, 29};
        gameClickCatcherSize = {460, 343 - (29 - 8)};
    }
    pPrimaryWindow->CreateButton("Game_Viewport", gameClickCatcherPos, gameClickCatcherSize, BUTTON_TYPE_NORMAL, 0, UIMSG_MouseLeftClickInGame, 0);

    pPrimaryWindow->CreateCharacterButtons();

    // MM8's minimap zoom buttons sit at the frame's bottom-right corner, (624,373) and (624,460). MM7's inherited
    // {519,136}/{574,136} sit in the old top-bar slot. They are created before the tome buttons below, since
    // Mouse::UI_OnMouseLeftClick takes the first vButtons match and pBtn_History's hitbox overlaps zoom-in's corner.
    Pointi zoomInPos = {519, 136};
    Pointi zoomOutPos = {574, 136};
    if (gameType() == GAME_TYPE_MM8) {
        zoomInPos = {624, 373};
        zoomOutPos = {624, 460};
    }
    pBtn_ZoomIn = pPrimaryWindow->CreateButton(zoomInPos, game_ui_btn_zoomin->size(), BUTTON_TYPE_NORMAL, 0,
                                               UIMSG_ClickZoomInBtn, 0, INPUT_ACTION_ZOOM_IN,
                                               localization->str(LSTR_ZOOM_IN), { game_ui_btn_zoomin }
    );

    pBtn_ZoomOut = pPrimaryWindow->CreateButton(zoomOutPos, game_ui_btn_zoomout->size(), BUTTON_TYPE_NORMAL, 0,
                                                UIMSG_ClickZoomOutBtn, 0, INPUT_ACTION_ZOOM_OUT,
                                                localization->str(LSTR_ZOOM_OUT), { game_ui_btn_zoomout });

    // The five MM7-positioned tome buttons park off-screen for MM8: their MM7 hitboxes, (491,353)..(600,361), would
    // land inside the minimap panel, and nothing draws them for MM8 (GameUI_DrawRightPanelItems early-returns). In MM8's
    // original these books are reached from the tab column on the map/book parchment, not from the game HUD.
    // GUI_HandleHotkey() dispatches the M/Q/N/T/H book hotkeys through exactly these buttons' InputAction field, so the
    // buttons are still created: off-screen, they keep those keys working while no click can land on them.
    Pointi tomeQuestsPos = {491, 353};
    Pointi tomeAutonotesPos = {527, 353};
    Pointi tomeMapsPos = {546, 353};
    Pointi tomeCalendarPos = {570, 353};
    Pointi tomeStorylinePos = {600, 361};
    if (gameType() == GAME_TYPE_MM8) {
        tomeQuestsPos = {-100, -100};
        tomeAutonotesPos = {-100, -100};
        tomeMapsPos = {-100, -100};
        tomeCalendarPos = {-100, -100};
        tomeStorylinePos = {-100, -100};
    }

    game_ui_tome_quests = assets->getImage_ColorKey("ib-td1-A");
    pBtn_Quests = pPrimaryWindow->CreateButton(tomeQuestsPos, game_ui_tome_quests->size(), BUTTON_TYPE_NORMAL, 0,
                                               UIMSG_OpenQuestBook, 0, INPUT_ACTION_OPEN_QUESTS,
                                               localization->str(LSTR_CURRENT_QUESTS), { game_ui_tome_quests });

    game_ui_tome_autonotes = assets->getImage_ColorKey("ib-td2-A");
    pBtn_Autonotes = pPrimaryWindow->CreateButton(tomeAutonotesPos, game_ui_tome_autonotes->size(), BUTTON_TYPE_NORMAL, 0,
                                                  UIMSG_OpenAutonotes, 0, INPUT_ACTION_OPEN_AUTONOTES,
                                                  localization->str(LSTR_AUTO_NOTES), { game_ui_tome_autonotes });

    game_ui_tome_maps = assets->getImage_ColorKey("ib-td3-A");
    pBtn_Maps = pPrimaryWindow->CreateButton(tomeMapsPos, game_ui_tome_maps->size(), BUTTON_TYPE_NORMAL, 0,
                                             UIMSG_OpenMapBook, 0, INPUT_ACTION_OPEN_MAP,
                                             localization->str(LSTR_MAPS), { game_ui_tome_maps });

    game_ui_tome_calendar = assets->getImage_ColorKey("ib-td4-A");
    pBtn_Calendar = pPrimaryWindow->CreateButton(tomeCalendarPos, game_ui_tome_calendar->size(), BUTTON_TYPE_NORMAL, 0,
                                                 UIMSG_OpenCalendar, 0, INPUT_ACTION_OPEN_CALENDAR,
                                                 localization->str(LSTR_CALENDAR), { game_ui_tome_calendar });

    game_ui_tome_storyline = assets->getImage_ColorKey("ib-td5-A");
    pBtn_History = pPrimaryWindow->CreateButton(tomeStorylinePos, game_ui_tome_storyline->size(), BUTTON_TYPE_NORMAL, 0,
                                                UIMSG_OpenHistoryBook, 0, INPUT_ACTION_OPEN_HISTORY,
                                                localization->str(LSTR_HISTORY), { game_ui_tome_storyline }
    );

    bFlashAutonotesBook = false;
    bFlashQuestBook = false;
    bFlashHistoryBook = false;

    // This invisible hover hitbox means "point at the minimap, show the date/time in the status bar" (MM7's own mechanic).
    // MM7's coords are its own top-right minimap slot, so MM8 uses the bottom-right "mapframe" (481,367, 159x113).
    if (gameType() == GAME_TYPE_MM8) {
        pPrimaryWindow->CreateButton({481, 410}, {159, 36}, BUTTON_TYPE_NORMAL, UIMSG_ShowStatus_DateTime, UIMSG_0, 0);
    } else {
        pPrimaryWindow->CreateButton({484, 15}, {138, 116}, BUTTON_TYPE_NORMAL, UIMSG_ShowStatus_DateTime, UIMSG_0, 0);
    }
    pPrimaryWindow->CreateButton("Game_Hireling1", {491, 149}, {64, 74}, BUTTON_TYPE_NORMAL, 0, UIMSG_StartHireling1Dialogue, 0, INPUT_ACTION_SELECT_NPC_1);
    pPrimaryWindow->CreateButton("Game_Hireling2", {561, 149}, {64, 74}, BUTTON_TYPE_NORMAL, 0, UIMSG_StartHireling2Dialogue, 0, INPUT_ACTION_SELECT_NPC_2);
    pPrimaryWindow->CreateButton({476, 322}, {77, 17}, BUTTON_TYPE_NORMAL, UIMSG_ShowStatus_Food, UIMSG_0, 0);
    pPrimaryWindow->CreateButton({555, 322}, {77, 17}, BUTTON_TYPE_NORMAL, UIMSG_ShowStatus_Funds, UIMSG_0, 0);

    // MM8's real command-icon row lives in the top bar, not MM7's (476,450) bottom-right slot, which sits behind the
    // minimap panel. Positions are measured from the original game's screen.
    //
    // MM8's sprite<->action mapping does not match MM7's: at {380,2} MM8 draws "butn1u", a SCROLL (opens the map/book
    // screen), and at {425,2} it draws "butn2u", a TENT (opens rest). So for MM8 the four buttons are created with MM8's
    // own actions, and there is no cast-spell button in the row (the spellbook opens from the skull bookend below).
    // MM7 keeps its own four verbatim. The asset globals are MM7-named: for MM8 game_ui_btn_rest holds "butn1u"
    // (scroll art -> map button) and game_ui_btn_cast holds "butn2u" (tent art -> rest button).
    if (gameType() == GAME_TYPE_MM8) {
        pBtn_MapBook = pPrimaryWindow->CreateButton("Game_MapBook", {380, 2}, game_ui_btn_rest->size(), BUTTON_TYPE_NORMAL, 0,
                                                    UIMSG_OpenMapBook, 0, INPUT_ACTION_INVALID,
                                                    localization->str(LSTR_MAPS), { game_ui_btn_rest });
        pBtn_Rest = pPrimaryWindow->CreateButton("Game_Rest", {425, 2}, game_ui_btn_cast->size(), BUTTON_TYPE_NORMAL, 0,
                                                 UIMSG_RestWindow, 0, INPUT_ACTION_REST,
                                                 localization->str(LSTR_REST), { game_ui_btn_cast });
        pBtn_QuickReference = pPrimaryWindow->CreateButton({470, 2}, game_ui_btn_quickref->size(), BUTTON_TYPE_NORMAL, 0,
                                                           UIMSG_QuickReference, 0, INPUT_ACTION_OPEN_QUICK_REFERENCE,
                                                           localization->str(LSTR_QUICK_REFERENCE), { game_ui_btn_quickref });
        pBtn_GameSettings = pPrimaryWindow->CreateButton({530, 2}, game_ui_btn_settings->size(), BUTTON_TYPE_NORMAL, 0,
                                                         UIMSG_GameMenuButton, 0, INPUT_ACTION_INVALID,
                                                         localization->str(LSTR_GAME_OPTIONS), { game_ui_btn_settings });
        // MM8's top-bar right bookend ("asir00", icons.lod, 64x64, flush against the 640px screen edge) opens the
        // spellbook when clicked anywhere in its box. It carries INPUT_ACTION_OPEN_SPELLBOOK, so the cast-spell hotkey
        // (default KEY_C) still reaches UIMSG_SpellBookWindow, since pBtn_CastSpell does not exist on this path.
        // The button id gives EngineController::pressGuiButton / the hitbox exporter a handle on MM8's spellbook opener.
        pBtn_Skull = pPrimaryWindow->CreateButton("Game_Spellbook", {576, 0}, game_ui_topbar_skull->size(), BUTTON_TYPE_NORMAL, 0,
                                                  UIMSG_SpellBookWindow, 0, INPUT_ACTION_OPEN_SPELLBOOK,
                                                  localization->str(LSTR_CAST_SPELL), { game_ui_topbar_skull });
    } else {
        pBtn_CastSpell = pPrimaryWindow->CreateButton("Game_CastSpell", {476, 450}, game_ui_btn_cast->size(), BUTTON_TYPE_NORMAL, 0,
                                                      UIMSG_SpellBookWindow, 0, INPUT_ACTION_OPEN_SPELLBOOK,
                                                      localization->str(LSTR_CAST_SPELL), { game_ui_btn_cast });
        pBtn_Rest = pPrimaryWindow->CreateButton("Game_Rest", {518, 450}, game_ui_btn_rest->size(), BUTTON_TYPE_NORMAL, 0,
                                                 UIMSG_RestWindow, 0, INPUT_ACTION_REST,
                                                 localization->str(LSTR_REST), { game_ui_btn_rest });
        pBtn_QuickReference = pPrimaryWindow->CreateButton({560, 450}, game_ui_btn_quickref->size(), BUTTON_TYPE_NORMAL, 0,
                                                           UIMSG_QuickReference, 0, INPUT_ACTION_OPEN_QUICK_REFERENCE,
                                                           localization->str(LSTR_QUICK_REFERENCE), { game_ui_btn_quickref });
        pBtn_GameSettings = pPrimaryWindow->CreateButton({602, 450}, game_ui_btn_settings->size(), BUTTON_TYPE_NORMAL, 0,
                                                         UIMSG_GameMenuButton, 0, INPUT_ACTION_INVALID,
                                                         localization->str(LSTR_GAME_OPTIONS), { game_ui_btn_settings });
    }
    pBtn_NPCLeft = pPrimaryWindow->CreateButton({469, 178}, ui_btn_npc_left->size(), BUTTON_TYPE_NORMAL, 0,
                                                UIMSG_ScrollNPCPanel, 0, INPUT_ACTION_INVALID, "", {ui_btn_npc_left });
    pBtn_NPCRight = pPrimaryWindow->CreateButton({626, 178}, ui_btn_npc_right->size(), BUTTON_TYPE_NORMAL, 0,
                                                 UIMSG_ScrollNPCPanel, 1, INPUT_ACTION_INVALID, "", {ui_btn_npc_right });

    LoadPartyBuffIcons();
}

