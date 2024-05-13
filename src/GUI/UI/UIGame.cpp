#include <map>
#include <algorithm>
#include <string>

#include "GUI/UI/UIGame.h"

#include "Application/Game.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Events/Processor.h"
#include "Engine/Graphics/BSPModel.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/MapInfo.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/NPC.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Tables/CharacterFrameTable.h"
#include "Engine/Time/Timer.h"
#include "Engine/TurnEngine/TurnEngine.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIWindow.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/Books/LloydsBook.h"
#include "GUI/UI/Books/TownPortalBook.h"
#include "GUI/UI/UICharacter.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UISpellbook.h"

#include "Io/InputAction.h"
#include "Io/Mouse.h"

#include "Utility/Math/TrigLut.h"
#include "Utility/Math/FixPoint.h"

#include "Library/Logger/Logger.h"

using Io::InputAction;

std::array<int, 4> pHealthBarPos = {{23, 138, 251, 366}}; // was 22, 137
std::array<int, 4> pManaBarPos = {{102, 217, 331, 447}};
extern const int pHealthManaBarYPos = 402;

std::array<unsigned int, 2> pHiredNPCsIconsOffsetsX = {{489, 559}};
std::array<unsigned int, 2> pHiredNPCsIconsOffsetsY = {{152, 152}};

std::array<std::array<int, 2>, 14> pPartySpellbuffsUI_XYs = {{
    {{477, 247}},
    {{497, 247}},
    {{522, 247}},
    {{542, 247}},
    {{564, 247}},
    {{581, 247}},
    {{614, 247}},
    {{477, 279}},
    {{497, 279}},
    {{522, 279}},
    {{542, 279}},
    {{564, 279}},
    {{589, 279}},
    {{612, 279}}
}};

std::array<uint8_t, 14> pPartySpellbuffsUI_smthns = {
    {14, 1, 10, 4, 7, 2, 9, 3, 6, 15, 8, 3, 12, 0}};

std::array<PartyBuff, 14> spellBuffsAtRightPanel = {
    {PARTY_BUFF_FEATHER_FALL, PARTY_BUFF_RESIST_FIRE, PARTY_BUFF_RESIST_AIR,
     PARTY_BUFF_RESIST_WATER, PARTY_BUFF_RESIST_MIND, PARTY_BUFF_RESIST_EARTH,
     PARTY_BUFF_RESIST_BODY, PARTY_BUFF_HEROISM, PARTY_BUFF_HASTE,
     PARTY_BUFF_SHIELD, PARTY_BUFF_STONE_SKIN, PARTY_BUFF_PROTECTION_FROM_MAGIC,
     PARTY_BUFF_IMMOLATION, PARTY_BUFF_DAY_OF_GODS}};

std::array<int, 8> BtnTurnCoord = {
    {0xA4, 0x5D, 0x16, 0xB, 0x5, 0xD, 0x7, 0x3B}};

GraphicsImage *game_ui_statusbar = nullptr;
GraphicsImage *game_ui_rightframe = nullptr;
GraphicsImage *game_ui_topframe = nullptr;
GraphicsImage *game_ui_leftframe = nullptr;
GraphicsImage *game_ui_bottomframe = nullptr;

GraphicsImage *game_ui_monster_hp_green = nullptr;
GraphicsImage *game_ui_monster_hp_yellow = nullptr;
GraphicsImage *game_ui_monster_hp_red = nullptr;
GraphicsImage *game_ui_monster_hp_background = nullptr;
GraphicsImage *game_ui_monster_hp_border_left = nullptr;
GraphicsImage *game_ui_monster_hp_border_right = nullptr;

GraphicsImage *game_ui_minimap_frame = nullptr;    // 5079D8
GraphicsImage *game_ui_minimap_compass = nullptr;  // 5079B4
std::array<GraphicsImage *, 8> game_ui_minimap_dirs;

GraphicsImage *game_ui_menu_quit = nullptr;
GraphicsImage *game_ui_menu_resume = nullptr;
GraphicsImage *game_ui_menu_controls = nullptr;
GraphicsImage *game_ui_menu_save = nullptr;
GraphicsImage *game_ui_menu_load = nullptr;
GraphicsImage *game_ui_menu_new = nullptr;
GraphicsImage *game_ui_menu_options = nullptr;

GraphicsImage *game_ui_tome_storyline = nullptr;
GraphicsImage *game_ui_tome_calendar = nullptr;
GraphicsImage *game_ui_tome_maps = nullptr;
GraphicsImage *game_ui_tome_autonotes = nullptr;
GraphicsImage *game_ui_tome_quests = nullptr;

GraphicsImage *game_ui_btn_rest = nullptr;
GraphicsImage *game_ui_btn_cast = nullptr;
GraphicsImage *game_ui_btn_zoomin = nullptr;
GraphicsImage *game_ui_btn_zoomout = nullptr;
GraphicsImage *game_ui_btn_quickref = nullptr;
GraphicsImage *game_ui_btn_settings = nullptr;

GraphicsImage *game_ui_dialogue_background = nullptr;

GraphicsImage *game_ui_menu_options_video_background = nullptr;
GraphicsImage *game_ui_menu_options_video_bloodsplats = nullptr;
GraphicsImage *game_ui_menu_options_video_coloredlights = nullptr;
GraphicsImage *game_ui_menu_options_video_tinting = nullptr;
std::array<GraphicsImage *, 10> game_ui_menu_options_video_gamma_positions;
std::array<GraphicsImage *, 5> game_ui_options_controls;

GraphicsImage *game_ui_evtnpc = nullptr;  // 50795C

std::array<std::array<GraphicsImage *, 56>, 4> game_ui_player_faces;
GraphicsImage *game_ui_player_face_eradicated = nullptr;
GraphicsImage *game_ui_player_face_dead = nullptr;

GraphicsImage *game_ui_player_selection_frame = nullptr;  // 50C98C
GraphicsImage *game_ui_player_alert_yellow = nullptr;     // 5079C8
GraphicsImage *game_ui_player_alert_red = nullptr;        // 5079CC
GraphicsImage *game_ui_player_alert_green = nullptr;      // 5079D0

GraphicsImage *game_ui_bar_red = nullptr;
GraphicsImage *game_ui_bar_yellow = nullptr;
GraphicsImage *game_ui_bar_green = nullptr;
GraphicsImage *game_ui_bar_blue = nullptr;

GraphicsImage *game_ui_playerbuff_pain_reflection = nullptr;
GraphicsImage *game_ui_playerbuff_hammerhands = nullptr;
GraphicsImage *game_ui_playerbuff_preservation = nullptr;
GraphicsImage *game_ui_playerbuff_bless = nullptr;

bool bFlashHistoryBook;
bool bFlashAutonotesBook;
bool bFlashQuestBook;

static bool bookFlashState = false;
static Time bookFlashTimer;

extern InputAction currently_selected_action_for_binding;  // 506E68
extern std::map<InputAction, bool> key_map_conflicted;  // 506E6C
extern std::map<InputAction, PlatformKey> curr_key_map;

GUIWindow_GameMenu::GUIWindow_GameMenu()
    : GUIWindow(WINDOW_GameMenu, {0, 0}, render->GetRenderDimensions()) {
    game_ui_menu_options = assets->getImage_ColorKey("options");
    game_ui_menu_new = assets->getImage_ColorKey("new1");
    game_ui_menu_load = assets->getImage_ColorKey("load1");
    game_ui_menu_save = assets->getImage_ColorKey("save1");
    game_ui_menu_controls = assets->getImage_ColorKey("controls1");
    game_ui_menu_resume = assets->getImage_ColorKey("resume1");
    game_ui_menu_quit = assets->getImage_ColorKey("quit1");

    pBtn_NewGame = CreateButton({0x13u, 0x9Bu}, {0xD6u, 0x28u}, 1, 0,
        UIMSG_StartNewGame, 0, Io::InputAction::NewGame, localization->GetString(LSTR_NEW_GAME), {game_ui_menu_new});
    pBtn_SaveGame = CreateButton("GameMenu_SaveGame", {0x13u, 0xD1u}, {0xD6u, 0x28u}, 1, 0,
        UIMSG_Game_OpenSaveGameDialog, 0, Io::InputAction::SaveGame, localization->GetString(LSTR_SAVE_GAME), {game_ui_menu_save});
    pBtn_LoadGame = CreateButton("GameMenu_LoadGame", {19, 263}, {0xD6u, 0x28u}, 1, 0,
        UIMSG_Game_OpenLoadGameDialog, 0, Io::InputAction::LoadGame, localization->GetString(LSTR_LOAD_GAME), {game_ui_menu_load});
    pBtn_GameControls = CreateButton({241, 155}, {214, 40}, 1, 0,
        UIMSG_Game_OpenOptionsDialog, 0, Io::InputAction::Options, localization->GetString(LSTR_OPTIONS), {game_ui_menu_controls});
    pBtn_QuitGame = CreateButton("GameMenu_Quit", {241, 209}, {214, 40}, 1, 0,
        UIMSG_Quit, 0, Io::InputAction::ExitGame, localization->GetString(LSTR_QUIT), {game_ui_menu_quit});
    pBtn_Resume = CreateButton({241, 263}, {214, 40}, 1, 0,
        UIMSG_GameMenu_ReturnToGame, 0, Io::InputAction::ReturnToGame, localization->GetString(LSTR_RETURN_TO_GAME), {game_ui_menu_resume});

    setKeyboardControlGroup(6, false, 0, 0);
}

void GUIWindow_GameMenu::Update() {
    // -----------------------------------
    // 004156F0 GUI_UpdateWindows --- part
    render->DrawTextureNew(pViewport->uViewportTL_X / 640.0f,
                                pViewport->uViewportTL_Y / 480.0f,
                                game_ui_menu_options);
}

//----- (00491CB5) --------------------------------------------------------
void GameUI_LoadPlayerPortraintsAndVoices() {
    for (unsigned i = 0; i < 4; ++i) {
        for (unsigned j = 0; j < 56; ++j) {
            game_ui_player_faces[i][j] = assets->getImage_ColorKey(
                fmt::format("{}{:02}", pPlayerPortraitsNames[pParty->pCharacters[i].uCurrentFace], j + 1));
        }
    }

    game_ui_player_face_eradicated =
        assets->getImage_ColorKey("ERADCATE");
    game_ui_player_face_dead = assets->getImage_ColorKey("DEAD");
    /*
        if (SoundSetAction[24][0])
        {
            for (unsigned i = 0; i < 4; ++i)
            {
                pSoundList->LoadSound(2 * (SoundSetAction[24][0] + 50 *
       pParty->pCharacters[i].uVoiceID) + 4998, 0); pSoundList->LoadSound(2 *
       (SoundSetAction[24][0] + 50 * pParty->pCharacters[i].uVoiceID) + 4999, 0);
            }
        }
    */
}

//----- (00491DE7) --------------------------------------------------------
void GameUI_ReloadPlayerPortraits(int player_id, int face_id) {  // the transition from the zombies to the normal state
    for (unsigned i = 0; i <= 55; ++i) {
        auto filename =
            fmt::format("{}{:02}", pPlayerPortraitsNames[face_id], i + 1);
        game_ui_player_faces[player_id][i] =
            assets->getImage_ColorKey(filename);
    }
}

//----- (00414D24) --------------------------------------------------------
static Color GameMenuUI_GetKeyBindingColor(InputAction action) {
    if (currently_selected_action_for_binding == action) {
        // TODO(pskelton): #time check tickcount usage here
        if (platform->tickCount() % 1000 < 500)
            return ui_gamemenu_keys_key_selection_blink_color_1;
        else
            return ui_gamemenu_keys_key_selection_blink_color_2;
    } else if (key_map_conflicted[action]) {
        int intensity;

        int time = platform->tickCount() % 800;
        if (time < 400)
            intensity = -70 + 70 * time / 400;
        else
            intensity = +70 - 70 * time / 800;

        return Color(185 + intensity, 40 + intensity / 4, 40 + intensity / 4);
    }

    return ui_gamemenu_keys_key_default_color;
}

GUIWindow_GameKeyBindings::GUIWindow_GameKeyBindings()
    : GUIWindow(WINDOW_KeyMappingOptions, {0, 0}, render->GetPresentDimensions()) {
    game_ui_options_controls[0] = assets->getImage_ColorKey("optkb");
    game_ui_options_controls[1] = assets->getImage_ColorKey("optkb_h");
    game_ui_options_controls[2] = assets->getImage_ColorKey("resume1");
    game_ui_options_controls[3] = assets->getImage_ColorKey("optkb_1");
    game_ui_options_controls[4] = assets->getImage_ColorKey("optkb_2");

    CreateButton({241, 302}, {214, 40}, 1, 0, UIMSG_Escape, 0);

    CreateButton({19, 302}, {108, 20}, 1, 0, UIMSG_SelectKeyPage1, 0);
    CreateButton({127, 302}, {108, 20}, 1, 0, UIMSG_SelectKeyPage2, 0);
    CreateButton("KeyBinding_Default", {127, 324}, {108, 20}, 1, 0, UIMSG_ResetKeyMapping, 0);
    CreateButton({19, 324}, {108, 20}, 1, 0, UIMSG_Game_OpenOptionsDialog, 0);

    CreateButton({129, 148}, {70, 19}, 1, 0, UIMSG_ChangeKeyButton, 0);
    CreateButton({129, 167}, {70, 19}, 1, 0, UIMSG_ChangeKeyButton, 1);
    CreateButton({129, 186}, {70, 19}, 1, 0, UIMSG_ChangeKeyButton, 2);
    CreateButton({129, 205}, {70, 19}, 1, 0, UIMSG_ChangeKeyButton, 3);
    CreateButton({129, 224}, {70, 19}, 1, 0, UIMSG_ChangeKeyButton, 4);
    CreateButton({129, 243}, {70, 19}, 1, 0, UIMSG_ChangeKeyButton, 5);
    CreateButton({129, 262}, {70, 19}, 1, 0, UIMSG_ChangeKeyButton, 6);

    CreateButton({350, 148}, {70, 19}, 1, 0, UIMSG_ChangeKeyButton, 7);
    CreateButton({350, 167}, {70, 19}, 1, 0, UIMSG_ChangeKeyButton, 8);
    CreateButton({350, 186}, {70, 19}, 1, 0, UIMSG_ChangeKeyButton, 9);
    CreateButton({350, 205}, {70, 19}, 1, 0, UIMSG_ChangeKeyButton, 10);
    CreateButton({350, 224}, {70, 19}, 1, 0, UIMSG_ChangeKeyButton, 11);
    CreateButton({350, 243}, {70, 19}, 1, 0, UIMSG_ChangeKeyButton, 12);
    CreateButton({350, 262}, {70, 19}, 1, 0, UIMSG_ChangeKeyButton, 13);

    currently_selected_action_for_binding = Io::InputAction::Invalid;
    KeyboardPageNum = 1;
    for (auto action : VanillaInputActions()) {
        key_map_conflicted[action] = false;
        curr_key_map[action] = keyboardActionMapping->GetKey(action);
    }
}

//----- (004142D3) --------------------------------------------------------
void GUIWindow_GameKeyBindings::Update() {
    // int v4;  // ecx@7
    // int v5;  // eax@8

    if (pGUIWindow_CurrentMenu->keyboard_input_status == WINDOW_INPUT_CONFIRMED) {
        InputAction action = currently_selected_action_for_binding;
        PlatformKey newKey = keyboardInputHandler->LastPressedKey();
        curr_key_map[action] = newKey;

        engine->_statusBar->clearAll();

        for (auto action : VanillaInputActions()) {
            key_map_conflicted[action] = false;
        }

        bool anyConflicts = false;
        for (auto x : curr_key_map) {
            for (auto y : curr_key_map) {
                if (x.first != y.first && x.second == y.second) {
                    key_map_conflicted[x.first] = true;
                    key_map_conflicted[y.first] = true;
                    anyConflicts = true;
                }
            }
        }

        if (anyConflicts)
            engine->_statusBar->setEvent(LSTR_KEY_CONFLICT);
        else
            engine->_statusBar->clearAll();

        keyboardInputHandler->EndTextInput();
        currently_selected_action_for_binding = Io::InputAction::Invalid;
    }
    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, game_ui_options_controls[0]);  // draw base texture

    int base_controls_offset = 0;
    if (KeyboardPageNum == 1) {
        render->DrawTextureNew(19 / 640.0f, 302 / 480.0f, game_ui_options_controls[3]);
    } else {
        base_controls_offset = 14;
        render->DrawTextureNew(127 / 640.0f, 302 / 480.0f, game_ui_options_controls[4]);
    }

    for (int i = 0; i < 7; ++i) {
        InputAction action1 = (InputAction)(base_controls_offset + i);
        pGUIWindow_CurrentMenu->DrawText(assets->pFontLucida.get(), {23, 142 + i * 21}, ui_gamemenu_keys_action_name_color, GetDisplayName(action1));
        pGUIWindow_CurrentMenu->DrawText(assets->pFontLucida.get(), {127, 142 + i * 21}, GameMenuUI_GetKeyBindingColor(action1), GetDisplayName(curr_key_map[action1]));

        int j = i + 7;
        InputAction action2 = (InputAction)(base_controls_offset + j);
        pGUIWindow_CurrentMenu->DrawText(assets->pFontLucida.get(), {247, 142 + i * 21}, ui_gamemenu_keys_action_name_color, GetDisplayName(action2));
        pGUIWindow_CurrentMenu->DrawText(assets->pFontLucida.get(), {350, 142 + i * 21}, GameMenuUI_GetKeyBindingColor(action2), GetDisplayName(curr_key_map[action2]));
    }
}

GUIWindow_GameVideoOptions::GUIWindow_GameVideoOptions()
    : GUIWindow(WINDOW_VideoOptions, {0, 0}, render->GetRenderDimensions()) {
    // -------------------------------------
    // GameMenuUI_OptionsVideo_Load --- part
    game_ui_menu_options_video_background = assets->getImage_ColorKey("optvid");
    game_ui_menu_options_video_bloodsplats = assets->getImage_ColorKey("opvdH-bs");
    game_ui_menu_options_video_coloredlights = assets->getImage_ColorKey("opvdH-cl");
    game_ui_menu_options_video_tinting = assets->getImage_ColorKey("opvdH-tn");

    game_ui_menu_options_video_gamma_positions[0] = assets->getImage_ColorKey("convol10");
    game_ui_menu_options_video_gamma_positions[1] = assets->getImage_ColorKey("convol20");
    game_ui_menu_options_video_gamma_positions[2] = assets->getImage_ColorKey("convol30");
    game_ui_menu_options_video_gamma_positions[3] = assets->getImage_ColorKey("convol40");
    game_ui_menu_options_video_gamma_positions[4] = assets->getImage_ColorKey("convol50");
    game_ui_menu_options_video_gamma_positions[5] = assets->getImage_ColorKey("convol60");
    game_ui_menu_options_video_gamma_positions[6] = assets->getImage_ColorKey("convol70");
    game_ui_menu_options_video_gamma_positions[7] = assets->getImage_ColorKey("convol80");
    game_ui_menu_options_video_gamma_positions[8] = assets->getImage_ColorKey("convol90");
    game_ui_menu_options_video_gamma_positions[9] = assets->getImage_ColorKey("convol00");
    // not_available_bloodsplats_texture_id =
    // pIcons_LOD->LoadTexture("opvdG-bs", TEXTURE_16BIT_PALETTE);
    // not_available_us_colored_lights_texture_id =
    // pIcons_LOD->LoadTexture("opvdG-cl", TEXTURE_16BIT_PALETTE);
    // not_available_tinting_texture_id = pIcons_LOD->LoadTexture("opvdG-tn",
    // TEXTURE_16BIT_PALETTE);

    CreateButton({0xF1u, 0x12Eu}, {0xD6u, 0x28u}, 1, 0, UIMSG_Escape, 0);

    // gamma buttons
    pBtn_SliderLeft = CreateButton({21, 161}, {17, 17}, 1, 0, UIMSG_ChangeGammaLevel, 4, Io::InputAction::Invalid, "", { options_menu_skin.uTextureID_ArrowLeft }); // -
    CreateButton({42, 160}, {170, 17}, 1, 0, UIMSG_ChangeGammaLevel, 0);
    pBtn_SliderRight = CreateButton({213, 161}, {17, 17}, 1, 0, UIMSG_ChangeGammaLevel, 5, Io::InputAction::Invalid, "", { options_menu_skin.uTextureID_ArrowRight }); // +

    // if ( render->pRenderD3D )
    {
        CreateButton({0x13u, 0x118u}, {0xD6u, 0x12u}, 1, 0, UIMSG_ToggleBloodsplats, 0);
        CreateButton({0x13u, 0x12Eu}, {0xD6u, 0x12u}, 1, 0, UIMSG_ToggleColoredLights, 0);
        CreateButton({0x13u, 0x144u}, {0xD6u, 0x12u}, 1, 0, UIMSG_ToggleTint, 0);
    }

    // update gamma preview
    if (gamma_preview_image) {
        gamma_preview_image->Release();
        gamma_preview_image = nullptr;
    }

    render->SaveScreenshot("gamma.pcx", 155, 117);
    gamma_preview_image = assets->getImage_PCXFromFile("gamma.pcx");
}

//----- (00414D9A) --------------------------------------------------------
void GUIWindow_GameVideoOptions::Update() {
    // -----------------------------------
    // 004156F0 GUI_UpdateWindows --- part
    int gammalevel = engine->config->graphics.Gamma.value();

    render->DrawTextureNew(
        8 / 640.0f, 8 / 480.0f,
        game_ui_menu_options_video_background);  // draw base texture
    // if ( !render->bWindowMode && render->IsGammaSupported() )
    {
        render->DrawTextureNew(
            (17 * gammalevel + 42) / 640.0f, 162 / 480.0f,
            game_ui_menu_options_video_gamma_positions[gammalevel]);

        if (gamma_preview_image)
            render->DrawTextureNew(274 / 640.0f, 169 / 480.0f, gamma_preview_image);

        GUIWindow msg_window;
        msg_window.uFrameX = 22;
        msg_window.uFrameY = 190;
        msg_window.uFrameWidth = 211;
        msg_window.uFrameHeight = 79;
        msg_window.uFrameZ = 232;
        msg_window.uFrameW = 268;
        msg_window.DrawTitleText(
            assets->pFontSmallnum.get(), 0, 0, ui_gamemenu_video_gamma_title_color,
            localization->GetString(LSTR_GAMMA_DESCRIPTION), 3
        );
    }

    if (engine->config->graphics.BloodSplats.value())
        render->DrawTextureNew(20 / 640.0f, 281 / 480.0f, game_ui_menu_options_video_bloodsplats);
    if (engine->config->graphics.ColoredLights.value())
        render->DrawTextureNew(20 / 640.0f, 303 / 480.0f, game_ui_menu_options_video_coloredlights);
    if (engine->config->graphics.Tinting.value())
        render->DrawTextureNew(20 / 640.0f, 325 / 480.0f, game_ui_menu_options_video_tinting);
}

OptionsMenuSkin options_menu_skin;  // 507C60
OptionsMenuSkin::OptionsMenuSkin()
    : uTextureID_Background(0),
      uTextureID_ArrowLeft(0),
      uTextureID_ArrowRight(0),
      uTextureID_unused_0(0),
      uTextureID_unused_1(0),
      uTextureID_unused_2(0),
      uTextureID_FlipOnExit(0),
      uTextureID_AlwaysRun(0),
      uTextureID_WalkSound(0),
      uTextureID_ShowDamage(0),
      uTextureID_TurnSpeed(),
      uTextureID_SoundLevels() {
    for (unsigned i = 0; i < 3; ++i) uTextureID_TurnSpeed[i] = 0;
    for (unsigned i = 0; i < 10; ++i) uTextureID_SoundLevels[i] = 0;
}

void OptionsMenuSkin::Release() {
#define RELEASE(img)        \
    {                       \
        if (img) {          \
            img->Release(); \
            img = nullptr;  \
        }                   \
    }

    RELEASE(uTextureID_Background);
    for (unsigned i = 0; i < 3; ++i) RELEASE(uTextureID_TurnSpeed[i]);
    RELEASE(uTextureID_ArrowLeft);
    RELEASE(uTextureID_ArrowRight);
    RELEASE(uTextureID_FlipOnExit);
    for (unsigned i = 0; i < 10; ++i) RELEASE(uTextureID_SoundLevels[i]);
    RELEASE(uTextureID_AlwaysRun);
    RELEASE(uTextureID_WalkSound);
    RELEASE(uTextureID_ShowDamage);

#undef RELEASE
}

GUIWindow_GameOptions::GUIWindow_GameOptions()
    : GUIWindow(WINDOW_GameOptions, {0, 0}, render->GetRenderDimensions()) {
    options_menu_skin.uTextureID_Background = assets->getImage_ColorKey("ControlBG");
    options_menu_skin.uTextureID_TurnSpeed[2] = assets->getImage_ColorKey("con_16x");
    options_menu_skin.uTextureID_TurnSpeed[1] = assets->getImage_ColorKey("con_32x");
    options_menu_skin.uTextureID_TurnSpeed[0] = assets->getImage_ColorKey("con_Smoo");
    options_menu_skin.uTextureID_ArrowLeft = assets->getImage_Alpha("con_ArrL");
    options_menu_skin.uTextureID_ArrowRight = assets->getImage_Alpha("con_ArrR");
    options_menu_skin.uTextureID_SoundLevels[0] = assets->getImage_ColorKey("convol10");
    options_menu_skin.uTextureID_SoundLevels[1] = assets->getImage_ColorKey("convol20");
    options_menu_skin.uTextureID_SoundLevels[2] = assets->getImage_ColorKey("convol30");
    options_menu_skin.uTextureID_SoundLevels[3] = assets->getImage_ColorKey("convol40");
    options_menu_skin.uTextureID_SoundLevels[4] = assets->getImage_ColorKey("convol50");
    options_menu_skin.uTextureID_SoundLevels[5] = assets->getImage_ColorKey("convol60");
    options_menu_skin.uTextureID_SoundLevels[6] = assets->getImage_ColorKey("convol70");
    options_menu_skin.uTextureID_SoundLevels[7] = assets->getImage_ColorKey("convol80");
    options_menu_skin.uTextureID_SoundLevels[8] = assets->getImage_ColorKey("convol90");
    options_menu_skin.uTextureID_SoundLevels[9] = assets->getImage_ColorKey("convol00");
    options_menu_skin.uTextureID_FlipOnExit = assets->getImage_ColorKey("option04");
    options_menu_skin.uTextureID_AlwaysRun = assets->getImage_ColorKey("option03");
    options_menu_skin.uTextureID_ShowDamage = assets->getImage_ColorKey("option02");
    options_menu_skin.uTextureID_WalkSound = assets->getImage_ColorKey("option01");

    CreateButton({22, 270}, options_menu_skin.uTextureID_TurnSpeed[2]->size(), 1, 0,
                 UIMSG_SetTurnSpeed, 0x80);
    CreateButton({93, 270}, options_menu_skin.uTextureID_TurnSpeed[1]->size(), 1, 0,
                 UIMSG_SetTurnSpeed, 0x40u);
    CreateButton({164, 270}, options_menu_skin.uTextureID_TurnSpeed[0]->size(), 1, 0,
                 UIMSG_SetTurnSpeed, 0);

    CreateButton({20, 303}, options_menu_skin.uTextureID_WalkSound->size(), 1, 0,
                 UIMSG_ToggleWalkSound, 0);
    CreateButton({128, 303}, options_menu_skin.uTextureID_ShowDamage->size(), 1, 0,
                 UIMSG_ToggleShowDamage, 0);
    CreateButton({20, 325}, options_menu_skin.uTextureID_AlwaysRun->size(), 1, 0,
                 UIMSG_ToggleAlwaysRun, 0);
    CreateButton({128, 325}, options_menu_skin.uTextureID_FlipOnExit->size(), 1, 0,
                 UIMSG_ToggleFlipOnExit, 0);

    pBtn_SliderLeft = CreateButton({243, 162}, {16, 16}, 1, 0, UIMSG_ChangeSoundVolume, 4, Io::InputAction::Invalid, "",
        {options_menu_skin.uTextureID_ArrowLeft});
    pBtn_SliderRight = CreateButton({435, 162}, {16, 16}, 1, 0, UIMSG_ChangeSoundVolume, 5, Io::InputAction::Invalid, "",
        {options_menu_skin.uTextureID_ArrowRight});
    CreateButton({263, 162}, {172, 17}, 1, 0, UIMSG_ChangeSoundVolume, 0);

    pBtn_SliderLeft = CreateButton({243, 216}, {16, 16}, 1, 0, UIMSG_ChangeMusicVolume, 4, Io::InputAction::Invalid, "",
        {options_menu_skin.uTextureID_ArrowLeft});
    pBtn_SliderRight = CreateButton({435, 216}, {16, 16}, 1, 0, UIMSG_ChangeMusicVolume, 5, Io::InputAction::Invalid, "",
        {options_menu_skin.uTextureID_ArrowRight});
    CreateButton({263, 216}, {172, 17}, 1, 0, UIMSG_ChangeMusicVolume, 0);

    pBtn_SliderLeft = CreateButton({243, 270}, {16, 16}, 1, 0, UIMSG_ChangeVoiceVolume, 4, Io::InputAction::Invalid, "",
        {options_menu_skin.uTextureID_ArrowLeft});
    pBtn_SliderRight = CreateButton({435, 270}, {16, 16}, 1, 0, UIMSG_ChangeVoiceVolume, 5, Io::InputAction::Invalid, "",
        {options_menu_skin.uTextureID_ArrowRight});
    CreateButton({263, 270}, {172, 17}, 1, 0, UIMSG_ChangeVoiceVolume, 0);

    CreateButton({241, 302}, {214, 40}, 1, 0, UIMSG_Escape, 0, Io::InputAction::Invalid, localization->GetString(LSTR_RETURN_TO_GAME));
    CreateButton({19, 140}, {214, 40}, 1, 0, UIMSG_OpenKeyMappingOptions, 0, Io::InputAction::Controls);
    CreateButton({19, 194}, {214, 40}, 1, 0, UIMSG_OpenVideoOptions, 0, Io::InputAction::Options);
}

void GUIWindow_GameOptions::Update() {
    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, game_ui_menu_options);
    render->DrawTextureNew(8 / 640.0f, 132 / 480.0f,
                                options_menu_skin.uTextureID_Background);

    switch ((int) engine->config->settings.TurnSpeed.value()) {
        case 64:
            render->DrawTextureNew(
                BtnTurnCoord[1] / 640.0f, 270 / 480.0f,
                options_menu_skin.uTextureID_TurnSpeed[1]);
            break;
        case 128:
            render->DrawTextureNew(
                BtnTurnCoord[2] / 640.0f, 270 / 480.0f,
                options_menu_skin.uTextureID_TurnSpeed[2]);
            break;
        default:
            render->DrawTextureNew(
                BtnTurnCoord[0] / 640.0f, 270 / 480.0f,
                options_menu_skin.uTextureID_TurnSpeed[0]);
            break;
    }

    if (engine->config->settings.WalkSound.value()) {
        render->DrawTextureNew(
            20 / 640.0f, 303 / 480.0f,
            options_menu_skin.uTextureID_WalkSound);
    }
    if (engine->config->settings.ShowHits.value()) {
        render->DrawTextureNew(
            128 / 640.0f, 303 / 480.0f,
            options_menu_skin.uTextureID_ShowDamage);
    }
    if (engine->config->settings.FlipOnExit.value()) {
        render->DrawTextureNew(
            128 / 640.0f, 325 / 480.0f,
            options_menu_skin.uTextureID_FlipOnExit);
    }
    if (engine->config->settings.AlwaysRun.value()) {
        render->DrawTextureNew(
            20 / 640.0f, 325 / 480.0f,
            options_menu_skin.uTextureID_AlwaysRun);
    }

    render->DrawTextureNew(
        (265 + 17 * engine->config->settings.SoundLevel.value()) / 640.0f, 162 / 480.0f,
        options_menu_skin.uTextureID_SoundLevels[engine->config->settings.SoundLevel.value()]);
    render->DrawTextureNew(
        (265 + 17 * engine->config->settings.MusicLevel.value()) / 640.0f, 216 / 480.0f,
        options_menu_skin.uTextureID_SoundLevels[engine->config->settings.MusicLevel.value()]);
    render->DrawTextureNew(
        (265 + 17 * engine->config->settings.VoiceLevel.value()) / 640.0f, 270 / 480.0f,
        options_menu_skin.uTextureID_SoundLevels[engine->config->settings.VoiceLevel.value()]);
}

void GameUI_OnPlayerPortraitLeftClick(int uPlayerID) {
    Character *player = &pParty->pCharacters[uPlayerID - 1];
    if (pParty->pPickedItem.uItemID != ITEM_NULL) {
        if (int slot = player->AddItem(-1, pParty->pPickedItem.uItemID)) {
            player->pInventoryItemList[slot - 1] = pParty->pPickedItem;
            mouse->RemoveHoldingItem();
            return;
        }

        if (!player->CanAct()) {
            player = &pParty->activeCharacter();
        }
        if (player->CanAct() || !pParty->activeCharacter().CanAct()) {
            player->playReaction(SPEECH_NO_ROOM);
        }
    }

    if (current_screen_type == SCREEN_GAME) {
        if (pParty->hasActiveCharacter()) {
            if (pParty->activeCharacterIndex() != uPlayerID) {
                if (player->timeToRecovery || !player->CanAct()) {
                    return;
                }

                pParty->setActiveCharacterIndex(uPlayerID);
                return;
            }
            pGUIWindow_CurrentMenu = new GUIWindow_CharacterRecord(pParty->activeCharacterIndex(), SCREEN_CHARACTERS);
            return;
        }
        return;
    }

    if (current_screen_type == SCREEN_SPELL_BOOK) {
        return;
    }

    if (current_screen_type == SCREEN_CHEST) {
        if (pParty->activeCharacterIndex() == uPlayerID) {
            current_character_screen_window = WINDOW_CharacterWindow_Inventory;
            current_screen_type = SCREEN_CHEST_INVENTORY;
            return;
        }
        if (player->timeToRecovery) {
            return;
        }
        pParty->setActiveCharacterIndex(uPlayerID);
        return;
    }
    if (current_screen_type != SCREEN_HOUSE) {
        if (current_screen_type == SCREEN_SHOP_INVENTORY || current_screen_type == SCREEN_CHEST_INVENTORY) {
            pParty->setActiveCharacterIndex(uPlayerID);
            return;
        }
        if (current_screen_type != SCREEN_CHEST_INVENTORY) {
            pParty->setActiveCharacterIndex(uPlayerID);
            return;
        }
        if (pParty->activeCharacterIndex() == uPlayerID) {
            current_character_screen_window = WINDOW_CharacterWindow_Inventory;
            current_screen_type = SCREEN_CHEST_INVENTORY;
            pParty->setActiveCharacterIndex(uPlayerID);
            return;
        }
        if (player->timeToRecovery) {
            return;
        }
        pParty->setActiveCharacterIndex(uPlayerID);
        return;
    }
    if (window_SpeakInHouse->keyboard_input_status == WINDOW_INPUT_IN_PROGRESS) {
        return;
    }

    if (pParty->activeCharacterIndex() != uPlayerID) {
        pParty->setActiveCharacterIndex(uPlayerID);
        return;
    }

    if (window_SpeakInHouse->getCurrentDialogue() == DIALOGUE_SHOP_BUY_STANDARD ||
        window_SpeakInHouse->getCurrentDialogue() == DIALOGUE_SHOP_BUY_SPECIAL) {
        current_character_screen_window = WINDOW_CharacterWindow_Inventory;
        pGUIWindow_CurrentMenu = new GUIWindow_CharacterRecord(pParty->activeCharacterIndex(), SCREEN_SHOP_INVENTORY);
        return;
    }
}

std::string GameUI_GetMinimapHintText() {
    double v3;            // st7@1
    int v7;               // eax@4
    int global_coord_X;   // [sp+10h] [bp-1Ch]@1
    int global_coord_Y;   // [sp+14h] [bp-18h]@1
    int pY;      // [sp+1Ch] [bp-10h]@1
    int pX;      // [sp+28h] [bp-4h]@1

    std::string result;
    mouse->GetClickPos(&pX, &pY);
    v3 = 1.0 / (float)((signed int)viewparams->uMinimapZoom * 0.000015258789);
    global_coord_X =
        (int64_t)((double)(pX - 557) * v3 + (double)pParty->pos.x);
    global_coord_Y =
        (int64_t)((double)pParty->pos.y - (double)(pY - 74) * v3);
    if (uCurrentlyLoadedLevelType != LEVEL_OUTDOOR ||
        pOutdoor->pBModels.empty()) {
        if (engine->_currentLoadedMapId == MAP_INVALID)
            result = "No Maze Info for this maze on file!";
        else
            result = pMapStats->pInfos[engine->_currentLoadedMapId].name;
    } else {
        for (BSPModel &model : pOutdoor->pBModels) {
            v7 = int_get_vector_length(
                std::abs((int)model.vBoundingCenter.x - global_coord_X),
                std::abs((int)model.vBoundingCenter.y - global_coord_Y), 0);
            if (v7 < 2 * model.sBoundingRadius) {
                for (ODMFace &face : model.pFaces) {
                    if (face.sCogTriggeredID) {
                        if (!(face.uAttributes & FACE_HAS_EVENT)) {
                            std::string hintString = getEventHintString(face.sCogTriggeredID);
                            if (!hintString.empty())
                                result = hintString;
                        }
                    }
                }
                if (!result.empty())
                    return result;
            }
        }
        if (engine->_currentLoadedMapId == MAP_INVALID)
            result = "No Maze Info for this maze on file!";
        else
            result = pMapStats->pInfos[engine->_currentLoadedMapId].name;
        return result;
    }
    return result;
}

//----- (0041AD6E) --------------------------------------------------------
void GameUI_DrawRightPanelItems() {
    if (bookFlashTimer > pParty->GetPlayingTime()) {
        bookFlashTimer = {};
    }

    if (pParty->GetPlayingTime() - bookFlashTimer > 128_ticks) {
        bookFlashTimer = pParty->GetPlayingTime();
        bookFlashState = !bookFlashState;
    }

    if (bookFlashState && current_screen_type != SCREEN_REST) {
        if (bFlashQuestBook) {
            render->DrawTextureNew(493 / 640.0f, 355 / 480.0f, game_ui_tome_quests);
        }
        if (bFlashAutonotesBook) {
            render->DrawTextureNew(527 / 640.0f, 353 / 480.0f, game_ui_tome_autonotes);
        }
        if (bFlashHistoryBook) {
            render->DrawTextureNew(600 / 640.0f, 361 / 480.0f, game_ui_tome_storyline);
        }
    }

    if (current_screen_type ==  SCREEN_BOOKS) {
        if (pGUIWindow_CurrentMenu->eWindowType == WINDOW_QuestBook) {
            render->DrawTextureNew(493 / 640.0f, 355 / 480.0f, game_ui_tome_quests);
        }
        if (pGUIWindow_CurrentMenu->eWindowType == WINDOW_AutonotesBook) {
            render->DrawTextureNew(527 / 640.0f, 353 / 480.0f, game_ui_tome_autonotes);
        }
        if (pGUIWindow_CurrentMenu->eWindowType == WINDOW_JournalBook) {
            render->DrawTextureNew(600 / 640.0f, 361 / 480.0f, game_ui_tome_storyline);
        }
        if (pGUIWindow_CurrentMenu->eWindowType == WINDOW_MapsBook) {
            render->DrawTextureNew(546 / 640.0f, 353 / 480.0f, game_ui_tome_maps);
        }
        if (pGUIWindow_CurrentMenu->eWindowType == WINDOW_CalendarBook) {
            render->DrawTextureNew(570 / 640.0f, 353 / 480.0f, game_ui_tome_calendar);
        }
    }
}

//----- (0041AEBB) --------------------------------------------------------
void GameUI_DrawFoodAndGold() {
    int text_y;  // esi@2

    if (uGameState != GAME_STATE_FINAL_WINDOW) {
        text_y = _44100D_should_alter_right_panel() != 0 ? 381 : 322;

        pPrimaryWindow->DrawText(assets->pFontSmallnum.get(), {0, text_y}, uGameUIFontMain, fmt::format("\r087{}", pParty->GetFood()), 0, uGameUIFontShadow);
        pPrimaryWindow->DrawText(assets->pFontSmallnum.get(), {0, text_y}, uGameUIFontMain, fmt::format("\r028{}", pParty->GetGold()), 0, uGameUIFontShadow);
        // force to render all queued text now so it wont be delayed and drawn over things it isn't supposed to, like item in hand or nuklear
        render->EndTextNew();
    }
}

//----- (0041B0C9) --------------------------------------------------------
void GameUI_DrawLifeManaBars() {
    for (int i = 0; i < pParty->pCharacters.size(); ++i) {
        if (pParty->pCharacters[i].health > 0) {
            int v17 = 0;
            if (i == 2 || i == 3) v17 = 2;
            double hpFillRatio = (double)pParty->pCharacters[i].health / (double)pParty->pCharacters[i].GetMaxHealth();

            auto pTextureHealth = game_ui_bar_green;
            if (hpFillRatio > 0.5) {
                if (hpFillRatio > 1.0) {
                    hpFillRatio = 1.0;
                }
            } else if (hpFillRatio > 0.25) {
                pTextureHealth = game_ui_bar_yellow;
            } else if (hpFillRatio > 0.0) {
                pTextureHealth = game_ui_bar_red;
            }
            if (hpFillRatio > 0.0) {
                render->SetUIClipRect(
                    v17 + pHealthBarPos[i],
                    (int64_t)((1.0 - hpFillRatio) * pTextureHealth->height()) + pHealthManaBarYPos,
                    v17 + pHealthBarPos[i] + pTextureHealth->width(), pTextureHealth->height() + pHealthManaBarYPos);
                render->DrawTextureNew((v17 + pHealthBarPos[i]) / 640.0f, pHealthManaBarYPos / 480.0f, pTextureHealth);
                render->ResetUIClipRect();
            }
        }
        if (pParty->pCharacters[i].mana > 0) {
            double mpFillRatio = (double)pParty->pCharacters[i].mana / (double)pParty->pCharacters[i].GetMaxMana();
            if (mpFillRatio > 1.0) {
                mpFillRatio = 1.0;
            }
            int v17 = 0;
            if (i == 2) v17 = 1;
            render->SetUIClipRect(
                v17 + pManaBarPos[i],
                (int64_t)((1.0 - mpFillRatio) * game_ui_bar_blue->height()) + pHealthManaBarYPos,
                v17 + pManaBarPos[i] + game_ui_bar_blue->width(), game_ui_bar_blue->height() + pHealthManaBarYPos);
            render->DrawTextureNew((v17 + pManaBarPos[i]) / 640.0f, pHealthManaBarYPos / 480.0f, game_ui_bar_blue);
            render->ResetUIClipRect();
        }
    }
}

//----- (0041B3B6) --------------------------------------------------------
void GameUI_DrawRightPanel() {
    render->DrawTextureNew(pViewport->uViewportBR_X / 640.0f, 0,
                                game_ui_right_panel_frame);
}

//----- (0041B3E2) --------------------------------------------------------
void GameUI_DrawRightPanelFrames() {
    render->DrawTextureNew(0, 0, game_ui_topframe);
    render->DrawTextureNew(0, 8 / 480.0f, game_ui_leftframe);
    render->DrawTextureNew(468 / 640.0f, 0, game_ui_rightframe);
    render->DrawTextureNew(0, 352 / 480.0f, game_ui_bottomframe);
    GameUI_DrawRightPanelItems();

    // render->EndScene();
    // render->Present();
}

void GameUI_WritePointedObjectStatusString() {
    // GUIWindow *pWindow;                // edi@7
    // GUIButton *pButton;                // ecx@11
    int requiredSkillpoints;           // ecx@19
    UIMessageType pMessageType1;  // esi@24
    int invmatrixindex;                // eax@41
    ItemGen *pItemGen;                 // ecx@44
    // int v16;                           // ecx@46
    Vis_PIDAndDepth pickedObject;        // eax@55
    signed int v18b;
    signed int pickedObjectID = 0;     // ecx@63
    BLVFace *pFace;                    // eax@69
    UIMessageType pMessageType2;  // esi@110
    UIMessageType pMessageType3;  // edx@117
    int pX;                   // [sp+D4h] [bp-Ch]@1
    int pY;                   // [sp+D8h] [bp-8h]@1

    // int testing;

    mouse->uPointingObjectID = Pid();
    mouse->GetClickPos(&pX, &pY);
    Sizei renDims = render->GetRenderDimensions();
    if (pX < 0 || pX > renDims.w - 1 || pY < 0 || pY > renDims.h - 1)
        return;

    if (current_screen_type == SCREEN_GAME) {
        if (pX <= (renDims.w - 1) * 0.73125 &&
            pY <= (renDims.h - 1) * 0.73125) {
            if (!pViewport->Contains(pX, pY)) {
                if (uLastPointedObjectID) {
                    engine->_statusBar->clearPermanent();
                }
                uLastPointedObjectID = Pid();
                return;
            }

            auto vis = EngineIocContainer::ResolveVis();

            // get_picked_object_zbuf_val contains both the pid and the depth
            Vis_PIDAndDepth pickedObject = engine->PickMouseNormal();
            mouse->uPointingObjectID = pickedObject.pid;
            pickedObjectID = (signed)pickedObject.pid.id();
            if (pickedObject.pid.type() == OBJECT_Item) {
                if (pObjectList->pObjects[pSpriteObjects[pickedObjectID].uObjectDescID].uFlags & OBJECT_DESC_UNPICKABLE) {
                    mouse->uPointingObjectID = Pid();
                    engine->_statusBar->clearPermanent();
                    uLastPointedObjectID = Pid();
                    return;
                }
                if (pickedObject.depth >= 0x200u || pParty->pPickedItem.uItemID != ITEM_NULL) {
                    engine->_statusBar->setPermanent(pSpriteObjects[pickedObjectID].containing_item.GetDisplayName());
                } else {
                    engine->_statusBar->setPermanent(LSTR_FMT_GET_S, pSpriteObjects[pickedObjectID].containing_item.GetDisplayName());
                }  // intentional fallthrough
            } else if (pickedObject.pid.type() == OBJECT_Decoration) {
                if (!pLevelDecorations[pickedObjectID].uEventID) {
                    std::string pText;                 // ecx@79
                    if (pLevelDecorations[pickedObjectID].IsInteractive())
                        pText = pNPCTopics[engine->_persistentVariables.decorVars[pLevelDecorations[pickedObjectID].eventVarId] + 380].pTopic; // campfire
                    else
                        pText = pDecorationList->GetDecoration(pLevelDecorations[pickedObjectID].uDecorationDescID)->field_20;
                    engine->_statusBar->setPermanent(pText);
                } else {
                    std::string hintString = getEventHintString(pLevelDecorations[pickedObjectID].uEventID);
                    if (!hintString.empty()) {
                        engine->_statusBar->setPermanent(hintString);
                    }
                }  // intentional fallthrough
            } else if (pickedObject.pid.type() == OBJECT_Face) {
                if (pickedObject.depth < 0x200u) {
                    std::string newString;
                    if (uCurrentlyLoadedLevelType != LEVEL_INDOOR) {
                        v18b = pickedObject.pid.id() >> 6;
                        short triggeredId = pOutdoor->pBModels[v18b].pFaces[pickedObjectID & 0x3F].sCogTriggeredID;
                        if (triggeredId != 0) {
                            newString = getEventHintString(pOutdoor->pBModels[v18b].pFaces[pickedObjectID & 0x3F]
                                    .sCogTriggeredID);
                        }
                    } else {
                        pFace = &pIndoor->pFaces[pickedObjectID];
                        if (pFace->uAttributes & FACE_INDICATE) {
                            unsigned short eventId =
                                pIndoor->pFaceExtras[pFace->uFaceExtraID]
                                    .uEventID;
                            if (eventId != 0) {
                                newString = getEventHintString(pIndoor->pFaceExtras[pFace->uFaceExtraID].uEventID);
                            }
                        }
                    }
                    if (!newString.empty()) {
                        engine->_statusBar->setPermanent(newString);
                        if (!mouse->uPointingObjectID && uLastPointedObjectID) {
                            engine->_statusBar->clearPermanent();
                        }
                        uLastPointedObjectID = mouse->uPointingObjectID;
                        return;
                    }
                }
                mouse->uPointingObjectID = Pid();
                engine->_statusBar->clearPermanent();
                uLastPointedObjectID = Pid();
                return;
            } else if (pickedObject.pid.type() == OBJECT_Actor) {
                if (pickedObject.depth >= 0x2000u) {
                    mouse->uPointingObjectID = Pid();
                    if (uLastPointedObjectID) {
                        engine->_statusBar->clearPermanent();
                    }
                    uLastPointedObjectID = Pid();
                    return;
                }
                engine->_statusBar->setPermanent(GetDisplayName(&pActors[pickedObjectID]));
            }
            if (!mouse->uPointingObjectID && uLastPointedObjectID) {
                engine->_statusBar->clearPermanent();
            }
            uLastPointedObjectID = mouse->uPointingObjectID;
            return;
        }
    } else if (current_screen_type == SCREEN_CHEST) {
        if (pX <= (renDims.w - 1) * 0.73125 &&
            pY <= (renDims.h - 1) * 0.73125) {  // if in chest area
            if (Chest::ChestUI_WritePointedObjectStatusString()) {
                return;
            } else if (uLastPointedObjectID) {  // not found so reset
                engine->_statusBar->clearPermanent();
            }
            uLastPointedObjectID = Pid();
            return;
        }
    } else {
        // if (pX <= (window->GetWidth() - 1) * 0.73125 && pY <=
        // (window->GetHeight() - 1) * 0.73125) {
        if (current_screen_type == SCREEN_CHARACTERS) {
            if (current_character_screen_window ==
                WINDOW_CharacterWindow_Inventory) {
                if (pY > 0 && pY < 350 && pX >= 13 &&
                    pX <= 462) {  // inventory poitned
                    // inventoryYCoord = (pY - 17) / 32;
                    // inventoryXCoord = (pX - 14) / 32;
                    // invMatrixIndex = inventoryXCoord + (INVETORYSLOTSWIDTH *
                    // inventoryYCoord);
                    invmatrixindex = ((pX - 14) / 32) + 14 * ((pY - 17) / 32);
                    // if (mouse.x <= 13 || mouse.x >= 462)
                    // return;
                    // testing =
                    // pParty->activeCharacter().GetItemIDAtInventoryIndex(invmatrixindex);
                    pItemGen =
                        pParty->activeCharacter().GetItemAtInventoryIndex(
                            invmatrixindex);  // (ItemGen
                                              // *)&pParty->activeCharacter().pInventoryItemList[testing
                                              // - 1];

                    // TODO(captainurist): get rid of this std::to_underlying cast.
                    if (pItemGen != NULL) pickedObjectID = std::to_underlying(pItemGen->uItemID);
                    // if (!pItemID)
                    // return;
                    // item =
                    // &pParty->activeCharacter().pInventoryItemList[pItemID -
                    // 1];

                    // v14 = render->pActiveZBuffer[pX +
                    // pSRZBufferLineOffsets[pY]];
                    if (pickedObjectID == 0 || pickedObjectID == -65536 ||
                        pickedObjectID >= 5000) {
                        // if (pMouse->uPointingObjectID == 0) {
                        if (uLastPointedObjectID) {
                            engine->_statusBar->clearPermanent();
                        }
                        //}
                        uLastPointedObjectID = Pid();
                        // return;
                    } else {
                        engine->_statusBar->setPermanent(pItemGen->GetDisplayName());
                        uLastPointedObjectID = Pid::dummy();
                        return;
                    }
                }
            }
        }

        // else if inventory

        for (GUIWindow *pWindow : lWindowList) {
            if (pWindow->Contains(pX, pY)) {
                for (GUIButton *pButton : pWindow->vButtons) {
                    switch (pButton->uButtonType) {
                        case 1:  // for dialogue window
                            if (pButton->Contains(pX, pY)) {
                                engine->_statusBar->setPermanent(pButton->sLabel);
                                pMessageType1 = (UIMessageType)pButton->uData;
                                if (pMessageType1)
                                    GameUI_handleHintMessage(pMessageType1, pButton->msg_param);

                                uLastPointedObjectID = Pid::dummy();
                                return;
                            }
                            break;
                        case 2:  // hovering over portraits
                            if (pButton->uWidth != 0 && pButton->uHeight != 0) {
                                unsigned distW = pX - pButton->uX;
                                unsigned distY = pY - pButton->uY;

                                double ratioX =
                                    1.0 * (distW * distW) /
                                    (pButton->uWidth * pButton->uWidth);
                                double ratioY =
                                    1.0 * (distY * distY) /
                                    (pButton->uHeight * pButton->uHeight);

                                if (ratioX + ratioY < 1.0) {
                                    engine->_statusBar->setPermanent(pButton->sLabel);  // for character name
                                    pMessageType2 = (UIMessageType)pButton->uData;
                                    if (pMessageType2 != 0)
                                        GameUI_handleHintMessage(pMessageType2, pButton->msg_param);

                                    uLastPointedObjectID = Pid::dummy();
                                    return;
                                }
                            }
                            break;
                        case 3:  // hovering over buttons
                            if (pButton->Contains(pX, pY)) {
                                CharacterSkillType skill = static_cast<CharacterSkillType>(pButton->msg_param);
                                int skillLevel = pParty->activeCharacter().getSkillValue(skill).level();
                                requiredSkillpoints = skillLevel + 1;

                                if (skills_max_level[skill] <= skillLevel)
                                    engine->_statusBar->setPermanent(LSTR_SKILL_ALREADY_MASTERED);
                                else if (pParty->activeCharacter().uSkillPoints < requiredSkillpoints)
                                    engine->_statusBar->setPermanent(LSTR_FMT_NEED_MORE_SKILL_POINTS, requiredSkillpoints - pParty->activeCharacter().uSkillPoints);
                                else
                                    engine->_statusBar->setPermanent(LSTR_FMT_CLICKING_WILL_SPEND_POINTS, requiredSkillpoints);

                                uLastPointedObjectID = Pid::dummy();
                                return;
                            }
                            break;
                    }
                }
            }

            // ?? if we get here nothing is curos over??
            if (uLastPointedObjectID) {  // not found so reset
                engine->_statusBar->clearPermanent();
            }
            uLastPointedObjectID = Pid();

            if (pWindow->uFrameHeight == 480) {
                // DebugBreak(); //Why is this condition here (in the original
                // too)? Might check fullscreen windows. Let Silvo know if you
                // find out
                // this is to stop the no windows active code below runnning
                return;
            }
        }
        // The game never gets to this point even in the original. It's also
        // bugged(neither branch displays anything).
        // TODO(_) fix these and move them up before the window check loop.

        // gets here when glitched

        /* if (current_screen_type == SCREEN_HOUSE)  // this is required
        when displaying inventory in a house/shop??
        {
        if (window_SpeakInHouse->getCurrentDialogue() != DIALOGUE_SHOP_BUY_STANDARD
        || (v16 = render->pActiveZBuffer[pX + pSRZBufferLineOffsets[pY]], v16 ==
        0)
        || v16 == -65536)
        {
        if (uLastPointedObjectID != 0)
        {
        engine->_statusBar->clearPermanent();
        }
        uLastPointedObjectID = 0;
        return;
        }
        pItemGen = (ItemGen *)((char *)&pParty->pPickedItem + 36 * (v16 + 12 *
        (unsigned int)window_SpeakInHouse->ptr_1C) + 4);
        engine->_statusBar->setPermanent(pItemGen->GetDisplayName());
        engine->_statusBar->clearPermanent();
        uLastPointedObjectID = 0;
        return;
        }
        */

        //}
    }

    // no windows active -outside of game screen area only
    if (!lWindowList.empty()) {
        GUIWindow *win = lWindowList.back();
        if (win->Contains(pX, pY)) {
            for (GUIButton *pButton : win->vButtons) {
                switch (pButton->uButtonType) {
                    case 1:
                        if (pX >= pButton->uX && pX <= pButton->uZ &&
                            pY >= pButton->uY && pY <= pButton->uW) {
                            pMessageType3 = (UIMessageType)pButton->uData;
                            if (pMessageType3 == 0) {  // For books
                                engine->_statusBar->setPermanent(pButton->sLabel);
                            } else {
                                GameUI_handleHintMessage(pMessageType3, pButton->msg_param);
                            }
                            uLastPointedObjectID = Pid::dummy();
                            return;
                        }
                        break;
                    case 2:  // hovering over portraits
                        if (pButton->uWidth != 0 && pButton->uHeight != 0) {
                            unsigned distW = pX - pButton->uX;
                            unsigned distY = pY - pButton->uY;

                            double ratioX = 1.0 * (distW * distW) /
                                            (pButton->uWidth * pButton->uWidth);
                            double ratioY =
                                1.0 * (distY * distY) /
                                (pButton->uHeight * pButton->uHeight);

                            if (ratioX + ratioY < 1.0) {
                                engine->_statusBar->setPermanent(pButton->sLabel);  // for character name
                                pMessageType2 = (UIMessageType)pButton->uData;
                                if (pMessageType2 != 0)
                                    GameUI_handleHintMessage(pMessageType2, pButton->msg_param);
                                uLastPointedObjectID = Pid::dummy();
                                return;
                            }
                        }
                        break;
                    case 3:
                        // is this one needed?
                        assert(false);  // how does this work?
                             /*                if (pX >= pButton->uX && pX <=
                             pButton->uZ
                             && pY >= pButton->uY && pY <= pButton->uW)
                             {
                             requiredSkillpoints =
                             (pParty->activeCharacter().pActiveSkills[pButton->msg_param]
                             & 0x3F) + 1;

                             std::string str;
                             if (pParty->activeCharacter().uSkillPoints <
                             requiredSkillpoints)      str =
                             localization->FormatString(
                             LSTR_FMT_NEED_MORE_SKILL_POINTS, requiredSkillpoints -
                             pParty->activeCharacter().uSkillPoints);
                             else      str =
                             localization->FormatString(
                             LSTR_FMT_CLICKING_WILL_SPEND_POINTS, requiredSkillpoints);
                             engine->_statusBar->setPermanent(str);
                             uLastPointedObjectID = 1;
                             return;
                             }*/
                        break;
                }
            }
        }
    }

    // pMouse->uPointingObjectID = sub_46A99B(); //for software
    if (uLastPointedObjectID) {
        engine->_statusBar->clearPermanent();
    }
    uLastPointedObjectID = Pid();
}

//----- (0044158F) --------------------------------------------------------
void GameUI_DrawCharacterSelectionFrame() {
    if (pParty->hasActiveCharacter())
        render->DrawTextureNew(
            (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing
                 [pParty->activeCharacterIndex() - 1] -
             9) /
                640.0f,
            380 / 480.0f, game_ui_player_selection_frame);
}

//----- (0044162D) --------------------------------------------------------
void GameUI_DrawPartySpells() {
    for (int i = 0; i < spellBuffsAtRightPanel.size(); ++i) {
        if (pParty->pPartyBuffs[spellBuffsAtRightPanel[i]].Active()) {
            render->TexturePixelRotateDraw(pPartySpellbuffsUI_XYs[i][0] / 640.,
                                           pPartySpellbuffsUI_XYs[i][1] / 480.,
                                           party_buff_icons[i],
                                           pMiscTimer->time().realtimeMilliseconds() / 20 + 20 * pPartySpellbuffsUI_smthns[i]);
        }
    }

    if (current_screen_type == SCREEN_GAME || current_screen_type == SCREEN_NPC_DIALOGUE) {
        // Flight / water walk animation is purposefully slowed down compared to what's in the data files.
        Duration frameNum = pMiscTimer->time() * 50 / 128;

        GraphicsImage *spell_texture;  // [sp-4h] [bp-1Ch]@12

        if (pParty->FlyActive()) {
            if (pParty->bFlying)
                spell_texture = pIconsFrameTable->GetFrame(uIconIdx_FlySpell, frameNum)->GetTexture();
            else
                spell_texture = pIconsFrameTable->GetFrame(uIconIdx_FlySpell, 0_ticks)->GetTexture();
            render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, spell_texture);
        }

        if (pParty->WaterWalkActive()) {
            if (pParty->uFlags & PARTY_FLAG_STANDING_ON_WATER)
                spell_texture = pIconsFrameTable->GetFrame(uIconIdx_WaterWalk, frameNum)->GetTexture();
            else
                spell_texture = pIconsFrameTable->GetFrame(uIconIdx_WaterWalk, 0_ticks)->GetTexture();
            render->DrawTextureNew(396 / 640.0f, 8 / 480.0f, spell_texture);
        }
    }

    for (int i = 0; i < pParty->pCharacters.size(); ++i) {
        if (pParty->pCharacters[i].pCharacterBuffs[CHARACTER_BUFF_HAMMERHANDS].Active())
            render->DrawTextureNew((pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72) / 640.0f, 427 / 480.0f, game_ui_playerbuff_hammerhands);
        if (pParty->pCharacters[i].pCharacterBuffs[CHARACTER_BUFF_BLESS].Active())
            render->DrawTextureNew((pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72) / 640.0f, 393 / 480.0f, game_ui_playerbuff_bless);
        if (pParty->pCharacters[i].pCharacterBuffs[CHARACTER_BUFF_PRESERVATION].Active())
            render->DrawTextureNew((pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72) / 640.0f, 410 / 480.0f, game_ui_playerbuff_preservation);
        if (pParty->pCharacters[i].pCharacterBuffs[CHARACTER_BUFF_PAIN_REFLECTION].Active())
            render->DrawTextureNew((pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72) / 640.0f, 444 / 480.0f, game_ui_playerbuff_pain_reflection);
    }
}

//----- (004921C1) --------------------------------------------------------
void GameUI_DrawPortraits() {
    unsigned int face_expression_ID;  // eax@17
    PlayerFrame *pFrame;              // eax@21
    GraphicsImage *pPortrait;                 // [sp-4h] [bp-1Ch]@27

    pParty->updateDelayedReaction();

    for (int i = 0; i < pParty->pCharacters.size(); ++i) {
        Character *pPlayer = &pParty->pCharacters[i];
        if (pPlayer->IsEradicated()) {
            pPortrait = game_ui_player_face_eradicated;
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active())
                render->DrawTextureGrayShade(pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] / 640.0f, 387 / 480.0f, pPortrait); // was 388
            else
                render->DrawTextureNew((pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 1) / 640.0f, 387 / 480.0f, pPortrait); // was 388
            continue;
        }
        if (pPlayer->IsDead()) {
            pPortrait = game_ui_player_face_dead;
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active())
                render->DrawTextureGrayShade(
                    pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] / 640.0f,
                    388 / 480.0f, pPortrait);
            else
                render->DrawTextureNew(
                    (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 1) / 640.0f,
                    388 / 480.0f, pPortrait);
            continue;
        }
        face_expression_ID = 0;
        for (size_t j = 0; j < pPlayerFrameTable->pFrames.size(); ++j)
            if (pPlayerFrameTable->pFrames[j].expression == pPlayer->expression) {
                face_expression_ID = j;
                break;
            }
        if (face_expression_ID == 0)
            face_expression_ID = 1;
        if (pPlayer->expression == CHARACTER_EXPRESSION_TALK)
            pFrame = pPlayerFrameTable->GetFrameBy_y(&pPlayer->_expression21_frameset, &pPlayer->_expression21_animtime, pMiscTimer->dt());
        else
            pFrame = pPlayerFrameTable->GetFrameBy_x(face_expression_ID, pPlayer->uExpressionTimePassed);
        if (true /* || pPlayer->uExpressionImageIndex != pFrame->uTextureID - 1*/) {
            pPlayer->uExpressionImageIndex = pFrame->uTextureID - 1;
            pPortrait = game_ui_player_faces[i][pPlayer->uExpressionImageIndex];  // pFace = (Texture_MM7*)game_ui_player_faces[i][pFrame->uTextureID];
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active())
                render->DrawTextureGrayShade(
                    pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] / 640.0f,
                    388 / 480.0f, pPortrait);
            else
                render->DrawTextureNew(
                    (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 1) / 640.0f,
                    388 / 480.0f, pPortrait);
            continue;
        }
    }
    if (pParty->bTurnBasedModeOn) {
        if (pTurnEngine->turn_stage != TE_WAIT) {
            if (pTurnEngine->pQueue[0].uPackedID.type() == OBJECT_Character) {
                for (unsigned i = 0; i < pTurnEngine->pQueue.size(); ++i) {
                    if (pTurnEngine->pQueue[i].uPackedID.type() != OBJECT_Character)
                        break;

                    auto alert_texture = game_ui_player_alert_green;
                    if (pParty->GetRedAlert())
                        alert_texture = game_ui_player_alert_red;
                    else if (pParty->GetYellowAlert())
                        alert_texture = game_ui_player_alert_yellow;

                    render->DrawTextureNew(
                        (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[pTurnEngine->pQueue[i].uPackedID.id()] - 4) / 640.0f,
                        384 / 480.0f, alert_texture); // was 385
                }
            }
        }
    } else {
        for (int i = 0; i < pParty->pCharacters.size(); ++i) {
            if (pParty->pCharacters[i].CanAct() && !pParty->pCharacters[i].timeToRecovery) {
                auto alert_texture = game_ui_player_alert_green;
                if (pParty->GetRedAlert())
                    alert_texture = game_ui_player_alert_red;
                else if (pParty->GetYellowAlert())
                    alert_texture = game_ui_player_alert_yellow;

                render->DrawTextureNew(
                    (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] - 4) / 640.0f,
                    384 / 480.0f, alert_texture); // was 385
            }
        }
    }
}

//----- (00441D38) --------------------------------------------------------
void GameUI_DrawMinimap(unsigned int uX, unsigned int uY, unsigned int uZ,
                        unsigned int uW, unsigned int uZoom,
                        unsigned int bRedrawOdmMinimap) {
    // signed int pW;   // ebx@23
    int LineGreyDim;         // eax@23
    double startx;      // st7@30
    signed int ypix;  // eax@37
    // uint16_t *v28; // ecx@37
    signed int xpix;       // edi@40
    int pPoint_X;         // edi@72
    int pPoint_Y;         // ebx@72
    // unsigned int lPitch;  // [sp+20h] [bp-34h]@1
    // signed int pY;        // [sp+20h] [bp-34h]@23
    // signed int pX;        // [sp+24h] [bp-30h]@23
    signed int xpixoffset16;       // [sp+24h] [bp-30h]@37
    signed int ypixoffset16;    // [sp+28h] [bp-2Ch]@37
    int map_scale;              // [sp+2Ch] [bp-28h]@30
    // signed int pZ;        // [sp+60h] [bp+Ch]@23
    double starty;            // [sp+60h] [bp+Ch]@30
    Color pColor;

    signed int uCenterX = (uX + uZ) / 2;
    signed int uCenterY = (uY + uW) / 2;
    render->SetUIClipRect(uX, uY, uZ, uW);
    int uHeight = uW - uY;
    signed int uWidth = uZ - uX;

    bool bWizardEyeActive = pParty->wizardEyeActive();
    CharacterSkillMastery uWizardEyeSkillLevel = pParty->wizardEyeSkillLevel();
    if (CheckHiredNPCSpeciality(Cartographer)) {
        bWizardEyeActive = true;
        uWizardEyeSkillLevel = uWizardEyeSkillLevel > CHARACTER_SKILL_MASTERY_EXPERT ? uWizardEyeSkillLevel : CHARACTER_SKILL_MASTERY_EXPERT;
    }

    if (engine->config->debug.WizardEye.value()) {
        bWizardEyeActive = true;
        uWizardEyeSkillLevel = CHARACTER_SKILL_MASTERY_MASTER;
    }

    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        static GraphicsImage *minimaptemp;
        if (!minimaptemp) {
            minimaptemp = GraphicsImage::Create(uWidth, uHeight);
        }

        static uint16_t pOdmMinimap[117][137];
        assert(sizeof(pOdmMinimap) == 137 * 117 * sizeof(short));

        bool partymoved = true;  // TODO(pskelton): actually check for party movement

        if (partymoved) {
            int loc_power = ImageHelper::GetWidthLn2(viewparams->location_minimap);
            map_scale = (1 << (loc_power + 16)) / (signed int)uZoom;
            startx = (double)(pParty->pos.x + 32768) /
                     (double)(1 << (16 - loc_power));
            starty = (double)(32768 - pParty->pos.y) /
                     (double)(1 << (16 - loc_power));
            switch (uZoom) {
            case 512: {
                startx = startx - (double)(uWidth / 2);
                starty = starty - (double)(uHeight / 2);
            } break;
            case 1024: {
                startx = startx - (double)(uWidth / 4);
                starty = starty - (double)(uHeight / 4);
            } break;
            case 2048: {
                startx = startx - (double)(uWidth / 8);
                starty = starty - (double)(uHeight / 8);
            } break;
            default:
                assert(false);
            }

            xpixoffset16 = floorf(startx * 65536.0 + 0.5f);     // LODWORD(v24);
            ypixoffset16 = floorf(starty * 65536.0 + 0.5f);  // LODWORD(v25);
            ypix = ypixoffset16 >> 16;
            xpix = xpixoffset16 >> 16;
            // v28 = &render->pTargetSurface[uX + uY * lPitch];

            // TODO(pskelton): could stretch texture rather than rescale
            if (/*pMapLod0 && */ bRedrawOdmMinimap) {
                assert(uWidth == 137 && uHeight == 117);

                int MapImgWidth = viewparams->location_minimap->width();
                const Color *pMapLod0Line = viewparams->location_minimap->rgba().pixels().data();
                Color *minitempix = minimaptemp->rgba().pixels().data();

                for (int y = 0; y < uHeight; ++y) {
                    for (int x = 0; x < uWidth; ++x) {
                        minitempix[x + y * uWidth] = pMapLod0Line[xpix + ypix * MapImgWidth];
                        xpix = (xpixoffset16 + x * map_scale) >> 16;
                    }
                    ypixoffset16 += map_scale;
                    ypix = ypixoffset16 >> 16;
                }
                // draw image
                render->Update_Texture(minimaptemp);
                render->DrawTextureNew(uX / 640., uY / 480., minimaptemp);
                // minimaptemp->Release();
            }
        } else {
            // no need to update map - just redraw
            render->DrawTextureNew(uX / 640., uY / 480., minimaptemp);
        }
        render->BeginLines2D();
    } else if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        render->FillRectFast(uX, uY, uZ - uX, uHeight, colorTable.NavyBlue);
        uNumBlueFacesInBLVMinimap = 0;
        render->BeginLines2D();
        for (unsigned i = 0; i < (unsigned)pIndoor->pMapOutlines.size(); ++i) {
            BLVMapOutline *pOutline = &pIndoor->pMapOutlines[i];

            if (pIndoor->pFaces[pOutline->uFace1ID].Visible() &&
                pIndoor->pFaces[pOutline->uFace2ID].Visible()) {
                if (pOutline->uFlags & 1 || pIndoor->pFaces[pOutline->uFace1ID].uAttributes & FACE_SeenByParty ||
                    pIndoor->pFaces[pOutline->uFace2ID].uAttributes & FACE_SeenByParty) {
                    pOutline->uFlags = pOutline->uFlags | 1;
                    pIndoor->_visible_outlines[i >> 3] |= 1 << (7 - i % 8);

                    int Vert1X = pIndoor->pVertices[pIndoor->pMapOutlines[i].uVertex1ID].x - pParty->pos.x;
                    int Vert2X = pIndoor->pVertices[pIndoor->pMapOutlines[i].uVertex2ID].x - pParty->pos.x;
                    int Vert1Y = pIndoor->pVertices[pIndoor->pMapOutlines[i].uVertex1ID].y - pParty->pos.y;
                    int Vert2Y = pIndoor->pVertices[pIndoor->pMapOutlines[i].uVertex2ID].y - pParty->pos.y;

                    int linex = uCenterX + fixpoint_mul(uZoom, Vert1X);
                    int liney = uCenterY - fixpoint_mul(uZoom, Vert1Y);
                    int linez = uCenterX + fixpoint_mul(uZoom, Vert2X);
                    int linew = uCenterY - fixpoint_mul(uZoom, Vert2Y);

                    if (bWizardEyeActive && uWizardEyeSkillLevel >= CHARACTER_SKILL_MASTERY_MASTER &&
                        (pIndoor->pFaces[pOutline->uFace1ID].Clickable() ||
                            pIndoor->pFaces[pOutline->uFace2ID].Clickable()) &&
                        (pIndoor->pFaceExtras[pIndoor->pFaces[pOutline->uFace1ID].uFaceExtraID].uEventID ||
                            pIndoor->pFaceExtras[pIndoor->pFaces[pOutline->uFace2ID].uFaceExtraID].uEventID)) {
                        if (uNumBlueFacesInBLVMinimap < 49) {
                            pBlueFacesInBLVMinimapIDs[uNumBlueFacesInBLVMinimap++] = i;
                            continue;
                        }
                    }

                    LineGreyDim = std::abs(pOutline->sZ - pParty->pos.z) / 8;
                    if (LineGreyDim > 100) LineGreyDim = 100;
                    render->RasterLine2D(linex, liney, linez, linew, viewparams->pPalette[-LineGreyDim + 200]);
                }
            }
        }

        for (unsigned i = 0; i < uNumBlueFacesInBLVMinimap; ++i) {
            BLVMapOutline *pOutline = &pIndoor->pMapOutlines[pBlueFacesInBLVMinimapIDs[i]];
            int pX = uCenterX + uZoom * (pIndoor->pVertices[pOutline->uVertex1ID].x - pParty->pos.x) / 65536.0f;
            int pY = uCenterY - uZoom * (pIndoor->pVertices[pOutline->uVertex1ID].y - pParty->pos.y) / 65536.0f;
            int pZ = uCenterX + uZoom * (pIndoor->pVertices[pOutline->uVertex2ID].x - pParty->pos.x) / 65536.0f;
            int pW = uCenterY - uZoom * (pIndoor->pVertices[pOutline->uVertex2ID].y - pParty->pos.y) / 65536.0f;
            render->RasterLine2D(pX, pY, pZ, pW, ui_game_minimap_outline_color);
        }
    }

    // opengl needs slightly modified lines to show up properly
    int lineadj = 1;

    // draw objects on the minimap
    if (bWizardEyeActive) {
        if (uWizardEyeSkillLevel >= CHARACTER_SKILL_MASTERY_EXPERT) {
            for (unsigned i = 0; i < pSpriteObjects.size(); ++i) {
                if (pSpriteObjects[i].uType == SPRITE_NULL || !pSpriteObjects[i].uObjectDescID)
                    continue;
                // if (uWizardEyeSkillLevel == 1
                pPoint_X = uCenterX + (pSpriteObjects[i].vPosition.x - pParty->pos.x) * uZoom / 65536.0f;
                pPoint_Y = uCenterY - (pSpriteObjects[i].vPosition.y - pParty->pos.y) * uZoom / 65536.0f;
                // if ( pPoint_X >= render->raster_clip_x && pPoint_X <=
                // render->raster_clip_z &&
                //     pPoint_Y >= render->raster_clip_y && pPoint_Y <=
                //     render->raster_clip_w)
                {
                    if (pObjectList->pObjects[pSpriteObjects[i].uObjectDescID]
                            .uFlags &
                        OBJECT_DESC_UNPICKABLE) {
                        render->RasterLine2D(pPoint_X, pPoint_Y, pPoint_X + 1,
                                             pPoint_Y + 1, ui_game_minimap_projectile_color);
                    } else if (uZoom > 512) {
                        render->RasterLine2D(pPoint_X - 2, pPoint_Y,
                                             pPoint_X - 2, pPoint_Y + 1 + lineadj, ui_game_minimap_treasure_color);
                        render->RasterLine2D(pPoint_X - 1, pPoint_Y - 1,
                                             pPoint_X - 1, pPoint_Y + 1 + lineadj, ui_game_minimap_treasure_color);
                        render->RasterLine2D(pPoint_X, pPoint_Y - 2, pPoint_X,
                                             pPoint_Y + 1 + lineadj, ui_game_minimap_treasure_color);
                        render->RasterLine2D(pPoint_X + 1, pPoint_Y - 1,
                                             pPoint_X + 1, pPoint_Y + 1 + lineadj, ui_game_minimap_treasure_color);
                        render->RasterLine2D(pPoint_X + 2, pPoint_Y,
                                             pPoint_X + 2, pPoint_Y + 1 + lineadj, ui_game_minimap_treasure_color);
                    } else {
                        render->RasterLine2D(pPoint_X - 1, pPoint_Y - 1,
                                             pPoint_X - 1, pPoint_Y + lineadj, ui_game_minimap_treasure_color);
                        render->RasterLine2D(pPoint_X, pPoint_Y - 1, pPoint_X,
                                             pPoint_Y + lineadj, ui_game_minimap_treasure_color);
                    }
                }
            }
        }
        for (unsigned i = 0; i < pActors.size(); ++i) {  // draw actors(отрисовка монстров и нпс)
            if (pActors[i].aiState != Removed &&
                pActors[i].aiState != Disabled &&
                (pActors[i].aiState == Dead || pActors[i].ActorNearby())) {
                pPoint_X = uCenterX + (pActors[i].pos.x - pParty->pos.x) * uZoom / 65536.0f;
                pPoint_Y = uCenterY - (pActors[i].pos.y - pParty->pos.y) * uZoom / 65536.0f;
                // if ( pPoint_X >= render->raster_clip_x && pPoint_X <=
                // render->raster_clip_z
                //  && pPoint_Y >= render->raster_clip_y && pPoint_Y <=
                //  render->raster_clip_w )
                {
                    pColor = ui_game_minimap_actor_friendly_color;
                    if (pActors[i].attributes & ACTOR_HOSTILE)
                        pColor = ui_game_minimap_actor_hostile_color;
                    if (pActors[i].aiState == Dead)
                        pColor = ui_game_minimap_actor_corpse_color;
                    if (uZoom > 1024) {
                        render->RasterLine2D(pPoint_X - 2, pPoint_Y - 1,
                                             pPoint_X - 2, pPoint_Y + 1 + lineadj,
                                             pColor);
                        render->RasterLine2D(pPoint_X - 1, pPoint_Y - 2,
                                             pPoint_X - 1, pPoint_Y + 2 + lineadj,
                                             pColor);
                        render->RasterLine2D(pPoint_X, pPoint_Y - 2, pPoint_X,
                                             pPoint_Y + 2 + lineadj, pColor);
                        render->RasterLine2D(pPoint_X + 1, pPoint_Y - 2,
                                             pPoint_X + 1, pPoint_Y + 2 + lineadj,
                                             pColor);
                        render->RasterLine2D(pPoint_X + 2, pPoint_Y - 1,
                                             pPoint_X + 2, pPoint_Y + 1 + lineadj,
                                             pColor);
                    } else {
                        render->RasterLine2D(pPoint_X - 1, pPoint_Y - 1,
                                             pPoint_X - 1, pPoint_Y + lineadj, pColor);
                        render->RasterLine2D(pPoint_X, pPoint_Y - 1, pPoint_X,
                                             pPoint_Y + lineadj, pColor);
                    }
                }
            }
        }
        for (unsigned i = 0; i < (signed int)pLevelDecorations.size(); ++i) {  // draw items(отрисовка предметов)
            if (pLevelDecorations[i].uFlags & LEVEL_DECORATION_VISIBLE_ON_MAP) {
                pPoint_X = uCenterX + (pLevelDecorations[i].vPosition.x - pParty->pos.x) * uZoom / 65536.0f;
                pPoint_Y = uCenterY - (pLevelDecorations[i].vPosition.y - pParty->pos.y) * uZoom / 65536.0f;

                // if ( pPoint_X >= render->raster_clip_x && pPoint_X <=
                // render->raster_clip_z
                //  && pPoint_Y >= render->raster_clip_y && pPoint_Y <=
                //  render->raster_clip_w )
                {
                    if ((signed int)uZoom > 512) {
                        render->RasterLine2D(pPoint_X - 1, pPoint_Y - 1, pPoint_X - 1, pPoint_Y + 1, ui_game_minimap_decoration_color_1);
                        render->RasterLine2D(pPoint_X, pPoint_Y - 1, pPoint_X, pPoint_Y + 1, ui_game_minimap_decoration_color_1);
                        render->RasterLine2D(pPoint_X + 1, pPoint_Y - 1, pPoint_X + 1, pPoint_Y + 1, ui_game_minimap_decoration_color_1);
                    } else {
                        render->RasterLine2D(pPoint_X, pPoint_Y, pPoint_X, pPoint_Y, ui_game_minimap_decoration_color_1);
                    }
                }
            }
        }
    }

    render->EndLines2D();

    // draw arrow on the minimap(include. Ritor1)
    unsigned arrow_idx;
    int rotate = pParty->_viewYaw & TrigLUT.uDoublePiMask;
    if (rotate <= 1920) arrow_idx = 6;
    if (rotate < 1664) arrow_idx = 5;
    if (rotate <= 1408) arrow_idx = 4;
    if (rotate < 1152) arrow_idx = 3;
    if (rotate <= 896) arrow_idx = 2;
    if (rotate < 640) arrow_idx = 1;
    if (rotate <= 384) arrow_idx = 0;
    if (rotate < 128 || rotate > 1920) arrow_idx = 7;
    render->DrawTextureNew((uCenterX - 3) / 640.0f, (uCenterY - 3) / 480.0f, game_ui_minimap_dirs[arrow_idx]);

    render->SetUIClipRect(541, 0, 567, 480);
    render->DrawTextureNew((floorf((pParty->_viewYaw * 0.1171875) + 0.5f) + 285) / 640.0f,
        136 / 480.0f, game_ui_minimap_compass);
    render->ResetUIClipRect();
    render->DrawTextureNew(468 / 640.0f, 0, game_ui_minimap_frame);
}

//----- (00441498) --------------------------------------------------------
void GameUI_DrawTorchlightAndWizardEye() {
    if (current_screen_type == SCREEN_GAME ||
        current_screen_type == SCREEN_MENU ||
        current_screen_type == SCREEN_OPTIONS ||
        current_screen_type == SCREEN_REST ||
        current_screen_type == SCREEN_SPELL_BOOK ||
        current_screen_type == SCREEN_CHEST ||
        current_screen_type == SCREEN_SAVEGAME ||
        current_screen_type == SCREEN_LOADGAME ||
        current_screen_type == SCREEN_CHEST_INVENTORY ||
        current_screen_type == SCREEN_BOOKS ||
        current_screen_type == SCREEN_BRANCHLESS_NPC_DIALOG) {
        if (pParty->TorchlightActive()) {
            render->DrawTextureNew(
                pUIAnum_Torchlight->x / 640.0f, pUIAnum_Torchlight->y / 480.0f,
                pIconsFrameTable
                    ->GetFrame(pUIAnum_Torchlight->icon->id, pMiscTimer->time())
                    ->GetTexture());
        }
        if (pParty->wizardEyeActive()) {
            render->DrawTextureNew(
                pUIAnim_WizardEye->x / 640.0f, pUIAnim_WizardEye->y / 480.0f,
                pIconsFrameTable
                    ->GetFrame(pUIAnim_WizardEye->icon->id, pMiscTimer->time())
                    ->GetTexture());
        }
    }
}

//----- (00491F87) --------------------------------------------------------
void GameUI_DrawHiredNPCs() {
    signed int uFrameID;            // [sp+24h] [bp-18h]@19

    if (!isHirelingsBlockedOnMap(engine->_currentLoadedMapId)) {
        FlatHirelings buf;
        buf.Prepare();

        for (int i = pParty->hirelingScrollPosition, count = 0; i < buf.Size() && count < 2; i++, count++) {
            std::string pContainer = fmt::format("NPC{:03}", buf.Get(i)->uPortraitID);
            render->DrawTextureNew(
                    pHiredNPCsIconsOffsetsX[count] / 640.0f,
                    pHiredNPCsIconsOffsetsY[count] / 480.0f,
                    assets->getImage_ColorKey(pContainer));

            // Dark sacrifice animation.
            if (!buf.IsFollower(i) && buf.GetSacrificeStatus(i)->inProgress) {
                render->DrawTextureNew(
                    pHiredNPCsIconsOffsetsX[count] / 640.0f,
                    pHiredNPCsIconsOffsetsY[count] / 480.0f,
                    pIconsFrameTable->GetFrame(pIconsFrameTable->FindIcon("spell96"), buf.GetSacrificeStatus(i)->elapsedTime)->GetTexture());
            }
        }
    }
}

//----- (004178FE) --------------------------------------------------------
Color UI_GetHealthManaAndOtherQualitiesStringColor(int actual_value,
                                                          int base_value) {
    uint16_t R, G, B;

    if (actual_value == base_value) {
        return colorTable.White; // Default - white.
    } else if (actual_value < base_value) {
        if (100 * actual_value / base_value >=
            25)  // Yellow( current_pos > 1/4 )
            R = 255, G = 255, B = 100;
        else  // Red( current_pos < 1/4 )
            R = 255, G = 0, B = 0;
    } else {  // Green
        R = 0, G = 255, B = 0;
    }

    return Color(R, G, B);
}

//----- (00417939) --------------------------------------------------------
Color GetConditionDrawColor(Condition uConditionIdx) {
    switch (uConditionIdx) {
        case CONDITION_ZOMBIE:
        case CONDITION_GOOD:
            return ui_character_condition_normal_color;

        case CONDITION_CURSED:
        case CONDITION_WEAK:
        case CONDITION_FEAR:
        case CONDITION_DRUNK:
        case CONDITION_INSANE:
        case CONDITION_POISON_WEAK:
        case CONDITION_DISEASE_WEAK:
            return ui_character_condition_light_color;

        case CONDITION_SLEEP:
        case CONDITION_POISON_MEDIUM:
        case CONDITION_DISEASE_MEDIUM:
        case CONDITION_PARALYZED:
        case CONDITION_UNCONSCIOUS:
            return ui_character_condition_moderate_color;

        case CONDITION_POISON_SEVERE:
        case CONDITION_DISEASE_SEVERE:
        case CONDITION_DEAD:
        case CONDITION_PETRIFIED:
        case CONDITION_ERADICATED:
            return ui_character_condition_severe_color;
        default:
            assert(false);
            return Color();
    }
}

//----- (00495430) --------------------------------------------------------
std::string GetReputationString(int reputation) {
    if (reputation >= 25)
        return localization->GetString(LSTR_REPUTATION_HATED);
    else if (reputation >= 6)
        return localization->GetString(LSTR_REPUTATION_UNFRIENDLY);
    else if (reputation >= -5)
        return localization->GetString(LSTR_REPUTATION_NEUTRAL);
    else if (reputation >= -24)
        return localization->GetString(LSTR_REPUTATION_FRIENDLY);
    else
        return localization->GetString(LSTR_REPUTATION_RESPECTED);
}

void GameUI_handleHintMessage(UIMessageType type, int param) {
    switch (type) {
        case UIMSG_HintSelectRemoveQuickSpellBtn: {
            if (spellbookSelectedSpell != SPELL_NONE && spellbookSelectedSpell != pParty->activeCharacter().uQuickSpell) {
                engine->_statusBar->setPermanent(LSTR_FMT_SET_S_AS_READY_SPELL, pSpellStats->pInfos[spellbookSelectedSpell].name);
            } else {
                if (pParty->activeCharacter().uQuickSpell != SPELL_NONE)
                    engine->_statusBar->setPermanent(LSTR_CLICK_TO_REMOVE_QUICKSPELL);
                else
                    engine->_statusBar->setPermanent(LSTR_CLICK_TO_SET_QUICKSPELL);
            }
            break;
        }

        case UIMSG_Spellbook_ShowHightlightedSpellInfo: {
            // TODO(pskelton): this used to check if character had the spell activated - no longer required here ??
            if (!pParty->hasActiveCharacter())
                break;
            SpellId selectedSpell = static_cast<SpellId>(param);
            if (isHoldingMouseRightButton()) {
                dword_507B00_spell_info_to_draw_in_popup = selectedSpell;
            }
            if (spellbookSelectedSpell == selectedSpell) {
                engine->_statusBar->setPermanent(LSTR_CAST_S, pSpellStats->pInfos[selectedSpell].name);
            } else {
                engine->_statusBar->setPermanent(LSTR_SELECT_S, pSpellStats->pInfos[selectedSpell].name);
            }
            break;
        }

        case UIMSG_ShowStatus_DateTime: {
            unsigned currHour = pParty->uCurrentHour;
            unsigned uNumSeconds = 1;
            if (pParty->uCurrentHour > 12) {
                if (pParty->uCurrentHour >= 24) uNumSeconds = 0;
                currHour = (currHour - 12);
            } else {
                if (pParty->uCurrentHour < 12)  // 12:00 is PM
                    uNumSeconds = 0;
                if (pParty->uCurrentHour == 0) currHour = 12;
            }
            engine->_statusBar->setPermanent(fmt::format("{}:{:02}{} {} {} {} {}", currHour, pParty->uCurrentMinute, localization->GetAmPm(uNumSeconds),
                localization->GetDayName(pParty->uCurrentDayOfMonth % 7),
                7 * pParty->uCurrentMonthWeek + pParty->uCurrentDayOfMonth % 7 + 1,
                localization->GetMonthName(pParty->uCurrentMonth), pParty->uCurrentYear));
            break;
        }

        case UIMSG_ShowStatus_ManaHP: {
            Character* character = &pParty->pCharacters[param - 1];
            engine->_statusBar->setPermanent(fmt::format("{} / {} {}    {} / {} {}",
                character->GetHealth(), character->GetMaxHealth(), localization->GetString(LSTR_HIT_POINTS),
                character->GetMana(), character->GetMaxMana(), localization->GetString(LSTR_SPELL_POINTS)));
            break;
        }

        case UIMSG_ShowStatus_Player: {
            Character* character = &pParty->pCharacters[param - 1];
            engine->_statusBar->setPermanent(fmt::format("{}: {}", NameAndTitle(character->name, character->classType),
                localization->GetCharacterConditionName(character->GetMajorConditionIdx())));
            engine->mouse->uPointingObjectID = Pid(OBJECT_Character, (unsigned char)(8 * param - 8) | 4);
            break;
        }

        case UIMSG_ShowStatus_Food: {
            engine->_statusBar->setPermanent(LSTR_FMT_YOU_HAVE_D_FOOD, pParty->GetFood());
            break;
        }

        case UIMSG_ShowStatus_Funds: {
            engine->_statusBar->setPermanent(LSTR_FMT_D_TOTAL_GOLD_D_IN_BANK, pParty->GetGold() + pParty->uNumGoldInBank, pParty->uNumGoldInBank);
            break;
        }

        case UIMSG_HintBeaconSlot: {
            if (pGUIWindow_CurrentMenu) {
                ((GUIWindow_LloydsBook*)pGUIWindow_CurrentMenu)->hintBeaconSlot(param);
            }
            break;
        }

        case UIMSG_HintTownPortal: {
            if (pGUIWindow_CurrentMenu) {
                ((GUIWindow_TownPortalBook*)pGUIWindow_CurrentMenu)->hintTown(param);
            }
            break;
        }

        case UIMSG_7A: {
            // unknown
            // button msg UIMSG_InventoryLeftClick
            break;
        }

        default: {
            logger->warning("GameUI_handleHintMessage - Unhandled message type: {}", static_cast<int>(type));
            break;
        }
    }
}
