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
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/MapInfo.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Tables/PlayerFrameTable.h"
#include "Engine/Time.h"
#include "Engine/TurnEngine/TurnEngine.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UICharacter.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIStatusBar.h"

#include "Io/InputAction.h"
#include "Io/Mouse.h"


#include "Utility/Math/TrigLut.h"

using Io::InputAction;

Image *game_ui_statusbar = nullptr;
Image *game_ui_rightframe = nullptr;
Image *game_ui_topframe = nullptr;
Image *game_ui_leftframe = nullptr;
Image *game_ui_bottomframe = nullptr;

Image *game_ui_monster_hp_green = nullptr;
Image *game_ui_monster_hp_yellow = nullptr;
Image *game_ui_monster_hp_red = nullptr;
Image *game_ui_monster_hp_background = nullptr;
Image *game_ui_monster_hp_border_left = nullptr;
Image *game_ui_monster_hp_border_right = nullptr;

Image *game_ui_minimap_frame = nullptr;    // 5079D8
Image *game_ui_minimap_compass = nullptr;  // 5079B4
std::array<Image *, 8> game_ui_minimap_dirs;

Image *game_ui_menu_quit = nullptr;
Image *game_ui_menu_resume = nullptr;
Image *game_ui_menu_controls = nullptr;
Image *game_ui_menu_save = nullptr;
Image *game_ui_menu_load = nullptr;
Image *game_ui_menu_new = nullptr;
Image *game_ui_menu_options = nullptr;

Image *game_ui_tome_storyline = nullptr;
Image *game_ui_tome_calendar = nullptr;
Image *game_ui_tome_maps = nullptr;
Image *game_ui_tome_autonotes = nullptr;
Image *game_ui_tome_quests = nullptr;

Image *game_ui_btn_rest = nullptr;
Image *game_ui_btn_cast = nullptr;
Image *game_ui_btn_zoomin = nullptr;
Image *game_ui_btn_zoomout = nullptr;
Image *game_ui_btn_quickref = nullptr;
Image *game_ui_btn_settings = nullptr;

Image *game_ui_dialogue_background = nullptr;

Image *game_ui_menu_options_video_background = nullptr;
Image *game_ui_menu_options_video_bloodsplats = nullptr;
Image *game_ui_menu_options_video_coloredlights = nullptr;
Image *game_ui_menu_options_video_tinting = nullptr;
std::array<Image *, 10> game_ui_menu_options_video_gamma_positions;
std::array<Image *, 5> game_ui_options_controls;

Image *game_ui_evtnpc = nullptr;  // 50795C

std::array<std::array<Image *, 56>, 4> game_ui_player_faces;
Image *game_ui_player_face_eradicated = nullptr;
Image *game_ui_player_face_dead = nullptr;

Image *game_ui_player_selection_frame = nullptr;  // 50C98C
Image *game_ui_player_alert_yellow = nullptr;     // 5079C8
Image *game_ui_player_alert_red = nullptr;        // 5079CC
Image *game_ui_player_alert_green = nullptr;      // 5079D0

Image *game_ui_bar_red = nullptr;
Image *game_ui_bar_yellow = nullptr;
Image *game_ui_bar_green = nullptr;
Image *game_ui_bar_blue = nullptr;

Image *game_ui_playerbuff_pain_reflection = nullptr;
Image *game_ui_playerbuff_hammerhands = nullptr;
Image *game_ui_playerbuff_preservation = nullptr;
Image *game_ui_playerbuff_bless = nullptr;

bool bFlashHistoryBook;
bool bFlashAutonotesBook;
bool bFlashQuestBook;

static bool bookFlashState = false;
static GameTime bookFlashTimer = GameTime(0);

extern InputAction currently_selected_action_for_binding;  // 506E68
extern std::map<InputAction, bool> key_map_conflicted;  // 506E6C
extern std::map<InputAction, PlatformKey> curr_key_map;

GUIWindow_GameMenu::GUIWindow_GameMenu()
    : GUIWindow(WINDOW_GameMenu, {0, 0}, render->GetRenderDimensions(), 0) {
    game_ui_menu_options = assets->GetImage_ColorKey("options");
    game_ui_menu_new = assets->GetImage_ColorKey("new1");
    game_ui_menu_load = assets->GetImage_ColorKey("load1");
    game_ui_menu_save = assets->GetImage_ColorKey("save1");
    game_ui_menu_controls = assets->GetImage_ColorKey("controls1");
    game_ui_menu_resume = assets->GetImage_ColorKey("resume1");
    game_ui_menu_quit = assets->GetImage_ColorKey("quit1");

    pBtn_NewGame = CreateButton({0x13u, 0x9Bu}, {0xD6u, 0x28u}, 1, 0,
        UIMSG_StartNewGame, 0, InputAction::NewGame, localization->GetString(LSTR_NEW_GAME), {game_ui_menu_new});
    pBtn_SaveGame = CreateButton("GameMenu_SaveGame", {0x13u, 0xD1u}, {0xD6u, 0x28u}, 1, 0,
        UIMSG_Game_OpenSaveGameDialog, 0, InputAction::SaveGame, localization->GetString(LSTR_SAVE_GAME), {game_ui_menu_save});
    pBtn_LoadGame = CreateButton("GameMenu_LoadGame", {19, 263}, {0xD6u, 0x28u}, 1, 0,
        UIMSG_Game_OpenLoadGameDialog, 0, InputAction::LoadGame, localization->GetString(LSTR_LOAD_GAME), {game_ui_menu_load});
    pBtn_GameControls = CreateButton({241, 155}, {214, 40}, 1, 0,
        UIMSG_Game_OpenOptionsDialog, 0, InputAction::Options, localization->GetString(LSTR_OPTIONS), {game_ui_menu_controls});
    pBtn_QuitGame = CreateButton("GameMenu_Quit", {241, 209}, {214, 40}, 1, 0,
        UIMSG_Quit, 0, InputAction::ExitGame, localization->GetString(LSTR_QUIT), {game_ui_menu_quit});
    pBtn_Resume = CreateButton({241, 263}, {214, 40}, 1, 0,
        UIMSG_GameMenu_ReturnToGame, 0, InputAction::ReturnToGame, localization->GetString(LSTR_RETURN_TO_GAME), {game_ui_menu_resume});

    _41D08F_set_keyboard_control_group(6, 1, 0, 0);
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
    for (uint i = 0; i < 4; ++i) {
        for (uint j = 0; j < 56; ++j) {
            game_ui_player_faces[i][j] = assets->GetImage_ColorKey(
                fmt::format(
                    "{}{:02}",
                    pPlayerPortraitsNames[pParty->pPlayers[i].uCurrentFace],
                    j + 1));
        }
    }

    game_ui_player_face_eradicated =
        assets->GetImage_ColorKey("ERADCATE");
    game_ui_player_face_dead = assets->GetImage_ColorKey("DEAD");
    /*
        if (SoundSetAction[24][0])
        {
            for (uint i = 0; i < 4; ++i)
            {
                pSoundList->LoadSound(2 * (SoundSetAction[24][0] + 50 *
       pParty->pPlayers[i].uVoiceID) + 4998, 0); pSoundList->LoadSound(2 *
       (SoundSetAction[24][0] + 50 * pParty->pPlayers[i].uVoiceID) + 4999, 0);
            }
        }
    */
}

//----- (00491DE7) --------------------------------------------------------
void GameUI_ReloadPlayerPortraits(
    int player_id,
    int face_id) {  // the transition from the zombies to the normal state
    for (uint i = 0; i <= 55; ++i) {
        auto filename =
            fmt::format("{}{:02}", pPlayerPortraitsNames[face_id], i + 1);
        game_ui_player_faces[player_id][i] =
            assets->GetImage_ColorKey(filename);
    }
}

extern std::map<InputAction, bool> key_map_conflicted;  // 506E6C
//----- (00414D24) --------------------------------------------------------
static unsigned int GameMenuUI_GetKeyBindingColor(InputAction action) {
    if (currently_selected_action_for_binding == action) {
        // TODO(pskelton): check tickcount usage here
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

        return color16(185 + intensity, 40 + intensity / 4, 40 + intensity / 4);
    }

    return ui_gamemenu_keys_key_default_color;
}

GUIWindow_GameKeyBindings::GUIWindow_GameKeyBindings()
    : GUIWindow(WINDOW_KeyMappingOptions, {0, 0}, render->GetPresentDimensions(), 0) {
    game_ui_options_controls[0] = assets->GetImage_ColorKey("optkb");
    game_ui_options_controls[1] = assets->GetImage_ColorKey("optkb_h");
    game_ui_options_controls[2] = assets->GetImage_ColorKey("resume1");
    game_ui_options_controls[3] = assets->GetImage_ColorKey("optkb_1");
    game_ui_options_controls[4] = assets->GetImage_ColorKey("optkb_2");

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

    currently_selected_action_for_binding = InputAction::Invalid;
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

        GameUI_StatusBar_Clear();

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
            GameUI_SetStatusBar(localization->GetString(LSTR_KEY_CONFLICT));
        else
            GameUI_StatusBar_Clear();

        keyboardInputHandler->EndTextInput();
        currently_selected_action_for_binding = InputAction::Invalid;
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
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, {23, 142 + i * 21}, ui_gamemenu_keys_action_name_color, GetDisplayName(action1).c_str(), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, {127, 142 + i * 21}, GameMenuUI_GetKeyBindingColor(action1), GetDisplayName(curr_key_map[action1]), 0, 0, 0);

        int j = i + 7;
        InputAction action2 = (InputAction)(base_controls_offset + j);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, {247, 142 + i * 21}, ui_gamemenu_keys_action_name_color, GetDisplayName(action2).c_str(), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, {350, 142 + i * 21}, GameMenuUI_GetKeyBindingColor(action2), GetDisplayName(curr_key_map[action2]), 0, 0, 0);
    }
}

GUIWindow_GameVideoOptions::GUIWindow_GameVideoOptions()
    : GUIWindow(WINDOW_VideoOptions, {0, 0}, render->GetRenderDimensions(), 0) {
    // -------------------------------------
    // GameMenuUI_OptionsVideo_Load --- part
    game_ui_menu_options_video_background = assets->GetImage_ColorKey("optvid");
    game_ui_menu_options_video_bloodsplats = assets->GetImage_ColorKey("opvdH-bs");
    game_ui_menu_options_video_coloredlights = assets->GetImage_ColorKey("opvdH-cl");
    game_ui_menu_options_video_tinting = assets->GetImage_ColorKey("opvdH-tn");

    game_ui_menu_options_video_gamma_positions[0] = assets->GetImage_ColorKey("convol10");
    game_ui_menu_options_video_gamma_positions[1] = assets->GetImage_ColorKey("convol20");
    game_ui_menu_options_video_gamma_positions[2] = assets->GetImage_ColorKey("convol30");
    game_ui_menu_options_video_gamma_positions[3] = assets->GetImage_ColorKey("convol40");
    game_ui_menu_options_video_gamma_positions[4] = assets->GetImage_ColorKey("convol50");
    game_ui_menu_options_video_gamma_positions[5] = assets->GetImage_ColorKey("convol60");
    game_ui_menu_options_video_gamma_positions[6] = assets->GetImage_ColorKey("convol70");
    game_ui_menu_options_video_gamma_positions[7] = assets->GetImage_ColorKey("convol80");
    game_ui_menu_options_video_gamma_positions[8] = assets->GetImage_ColorKey("convol90");
    game_ui_menu_options_video_gamma_positions[9] = assets->GetImage_ColorKey("convol00");
    // not_available_bloodsplats_texture_id =
    // pIcons_LOD->LoadTexture("opvdG-bs", TEXTURE_16BIT_PALETTE);
    // not_available_us_colored_lights_texture_id =
    // pIcons_LOD->LoadTexture("opvdG-cl", TEXTURE_16BIT_PALETTE);
    // not_available_tinting_texture_id = pIcons_LOD->LoadTexture("opvdG-tn",
    // TEXTURE_16BIT_PALETTE);

    CreateButton({0xF1u, 0x12Eu}, {0xD6u, 0x28u}, 1, 0, UIMSG_Escape, 0);

    // gamma buttons
    pBtn_SliderLeft = CreateButton({21, 161}, {17, 17}, 1, 0, UIMSG_ChangeGammaLevel, 4, InputAction::Invalid, "", { options_menu_skin.uTextureID_ArrowLeft }); // -
    CreateButton({42, 160}, {170, 17}, 1, 0, UIMSG_ChangeGammaLevel, 0);
    pBtn_SliderRight = CreateButton({213, 161}, {17, 17}, 1, 0, UIMSG_ChangeGammaLevel, 5, InputAction::Invalid, "", { options_menu_skin.uTextureID_ArrowRight }); // +

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
    gamma_preview_image = assets->GetImage_PCXFromFile("gamma.pcx");
}

//----- (00414D9A) --------------------------------------------------------
void GUIWindow_GameVideoOptions::Update() {
    // -----------------------------------
    // 004156F0 GUI_UpdateWindows --- part
    GUIWindow msg_window;  // [sp+8h] [bp-54h]@3

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
        msg_window.Init();
        msg_window.uFrameX = 22;
        msg_window.uFrameY = 190;
        msg_window.uFrameWidth = 211;
        msg_window.uFrameHeight = 79;
        msg_window.uFrameZ = 232;
        msg_window.uFrameW = 268;
        msg_window.DrawTitleText(
            pFontSmallnum, 0, 0, ui_gamemenu_video_gamma_title_color,
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
    for (uint i = 0; i < 3; ++i) uTextureID_TurnSpeed[i] = 0;
    for (uint i = 0; i < 10; ++i) uTextureID_SoundLevels[i] = 0;
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
    for (uint i = 0; i < 3; ++i) RELEASE(uTextureID_TurnSpeed[i]);
    RELEASE(uTextureID_ArrowLeft);
    RELEASE(uTextureID_ArrowRight);
    RELEASE(uTextureID_FlipOnExit);
    for (uint i = 0; i < 10; ++i) RELEASE(uTextureID_SoundLevels[i]);
    RELEASE(uTextureID_AlwaysRun);
    RELEASE(uTextureID_WalkSound);
    RELEASE(uTextureID_ShowDamage);

#undef RELEASE
}

GUIWindow_GameOptions::GUIWindow_GameOptions()
    : GUIWindow(WINDOW_GameOptions, {0, 0}, render->GetRenderDimensions(), 0) {
    options_menu_skin.uTextureID_Background = assets->GetImage_ColorKey("ControlBG");
    options_menu_skin.uTextureID_TurnSpeed[2] = assets->GetImage_ColorKey("con_16x");
    options_menu_skin.uTextureID_TurnSpeed[1] = assets->GetImage_ColorKey("con_32x");
    options_menu_skin.uTextureID_TurnSpeed[0] = assets->GetImage_ColorKey("con_Smoo");
    options_menu_skin.uTextureID_ArrowLeft = assets->GetImage_Alpha("con_ArrL");
    options_menu_skin.uTextureID_ArrowRight = assets->GetImage_Alpha("con_ArrR");
    options_menu_skin.uTextureID_SoundLevels[0] = assets->GetImage_ColorKey("convol10");
    options_menu_skin.uTextureID_SoundLevels[1] = assets->GetImage_ColorKey("convol20");
    options_menu_skin.uTextureID_SoundLevels[2] = assets->GetImage_ColorKey("convol30");
    options_menu_skin.uTextureID_SoundLevels[3] = assets->GetImage_ColorKey("convol40");
    options_menu_skin.uTextureID_SoundLevels[4] = assets->GetImage_ColorKey("convol50");
    options_menu_skin.uTextureID_SoundLevels[5] = assets->GetImage_ColorKey("convol60");
    options_menu_skin.uTextureID_SoundLevels[6] = assets->GetImage_ColorKey("convol70");
    options_menu_skin.uTextureID_SoundLevels[7] = assets->GetImage_ColorKey("convol80");
    options_menu_skin.uTextureID_SoundLevels[8] = assets->GetImage_ColorKey("convol90");
    options_menu_skin.uTextureID_SoundLevels[9] = assets->GetImage_ColorKey("convol00");
    options_menu_skin.uTextureID_FlipOnExit = assets->GetImage_ColorKey("option04");
    options_menu_skin.uTextureID_AlwaysRun = assets->GetImage_ColorKey("option03");
    options_menu_skin.uTextureID_ShowDamage = assets->GetImage_ColorKey("option02");
    options_menu_skin.uTextureID_WalkSound = assets->GetImage_ColorKey("option01");

    CreateButton({22, 270}, {options_menu_skin.uTextureID_TurnSpeed[2]->GetWidth(), options_menu_skin.uTextureID_TurnSpeed[2]->GetHeight()}, 1, 0,
        UIMSG_SetTurnSpeed, 0x80);
    CreateButton({93, 270}, {options_menu_skin.uTextureID_TurnSpeed[1]->GetWidth(), options_menu_skin.uTextureID_TurnSpeed[1]->GetHeight()}, 1, 0,
        UIMSG_SetTurnSpeed, 0x40u);
    CreateButton({164, 270}, {options_menu_skin.uTextureID_TurnSpeed[0]->GetWidth(), options_menu_skin.uTextureID_TurnSpeed[0]->GetHeight()}, 1, 0,
        UIMSG_SetTurnSpeed, 0);

    CreateButton({20, 303}, {options_menu_skin.uTextureID_WalkSound->GetWidth(), options_menu_skin.uTextureID_WalkSound->GetHeight()}, 1, 0,
        UIMSG_ToggleWalkSound, 0);
    CreateButton({128, 303}, {options_menu_skin.uTextureID_ShowDamage->GetWidth(), options_menu_skin.uTextureID_ShowDamage->GetHeight()}, 1, 0,
        UIMSG_ToggleShowDamage, 0);
    CreateButton({20, 325}, {options_menu_skin.uTextureID_AlwaysRun->GetWidth(), options_menu_skin.uTextureID_AlwaysRun->GetHeight()}, 1, 0,
        UIMSG_ToggleAlwaysRun, 0);
    CreateButton({128, 325}, {options_menu_skin.uTextureID_FlipOnExit->GetWidth(), options_menu_skin.uTextureID_FlipOnExit->GetHeight()}, 1, 0,
        UIMSG_ToggleFlipOnExit, 0);

    pBtn_SliderLeft = CreateButton({243, 162}, {16, 16}, 1, 0, UIMSG_ChangeSoundVolume, 4, InputAction::Invalid, "",
        {options_menu_skin.uTextureID_ArrowLeft});
    pBtn_SliderRight = CreateButton({435, 162}, {16, 16}, 1, 0, UIMSG_ChangeSoundVolume, 5, InputAction::Invalid, "",
        {options_menu_skin.uTextureID_ArrowRight});
    CreateButton({263, 162}, {172, 17}, 1, 0, UIMSG_ChangeSoundVolume, 0);

    pBtn_SliderLeft = CreateButton({243, 216}, {16, 16}, 1, 0, UIMSG_ChangeMusicVolume, 4, InputAction::Invalid, "",
        {options_menu_skin.uTextureID_ArrowLeft});
    pBtn_SliderRight = CreateButton({435, 216}, {16, 16}, 1, 0, UIMSG_ChangeMusicVolume, 5, InputAction::Invalid, "",
        {options_menu_skin.uTextureID_ArrowRight});
    CreateButton({263, 216}, {172, 17}, 1, 0, UIMSG_ChangeMusicVolume, 0);

    pBtn_SliderLeft = CreateButton({243, 270}, {16, 16}, 1, 0, UIMSG_ChangeVoiceVolume, 4, InputAction::Invalid, "",
        {options_menu_skin.uTextureID_ArrowLeft});
    pBtn_SliderRight = CreateButton({435, 270}, {16, 16}, 1, 0, UIMSG_ChangeVoiceVolume, 5, InputAction::Invalid, "",
        {options_menu_skin.uTextureID_ArrowRight});
    CreateButton({263, 270}, {172, 17}, 1, 0, UIMSG_ChangeVoiceVolume, 0);

    CreateButton({241, 302}, {214, 40}, 1, 0, UIMSG_Escape, 0, InputAction::Invalid, localization->GetString(LSTR_RETURN_TO_GAME));
    CreateButton({19, 140}, {214, 40}, 1, 0, UIMSG_OpenKeyMappingOptions, 0, InputAction::Controls);
    CreateButton({19, 194}, {214, 40}, 1, 0, UIMSG_OpenVideoOptions, 0, InputAction::Options);
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

void GameUI_OnPlayerPortraitLeftClick(unsigned int uPlayerID) {
    Player *player = &pParty->pPlayers[uPlayerID - 1];
    if (pParty->pPickedItem.uItemID != ITEM_NULL) {
        if (int slot = player->AddItem(-1, pParty->pPickedItem.uItemID)) {
            memcpy(&player->pInventoryItemList[slot - 1], &pParty->pPickedItem,
                   0x24u);
            mouse->RemoveHoldingItem();
            return;
        }

        if (!player->CanAct()) {
            player = &pParty->activeCharacter();
        }
        if (player->CanAct() || !pParty->activeCharacter().CanAct()) {
            player->playReaction(SPEECH_NoRoom);
        }
    }

    if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
        if (pParty->hasActiveCharacter()) {
            if (pParty->activeCharacterIndex() != uPlayerID) {
                if (pPlayers[uPlayerID]->timeToRecovery || !pPlayers[uPlayerID]->CanAct()) {
                    return;
                }

                pParty->setActiveCharacterIndex(uPlayerID);
                return;
            }
            pGUIWindow_CurrentMenu = new GUIWindow_CharacterRecord(
                pParty->activeCharacterIndex(),
                CURRENT_SCREEN::SCREEN_CHARACTERS);  // CharacterUI_Initialize(SCREEN_CHARACTERS);
            return;
        }
        return;
    }

    if (current_screen_type == CURRENT_SCREEN::SCREEN_SPELL_BOOK) return;
    if (current_screen_type == CURRENT_SCREEN::SCREEN_CHEST) {
        if (pParty->activeCharacterIndex() == uPlayerID) {
            current_character_screen_window = WINDOW_CharacterWindow_Inventory;
            current_screen_type = CURRENT_SCREEN::SCREEN_CHEST_INVENTORY;
            pParty->setActiveCharacterIndex(uPlayerID);
            return;
        }
        if (pPlayers[uPlayerID]->timeToRecovery) {
            return;
        }
        pParty->setActiveCharacterIndex(uPlayerID);
        return;
    }
    if (current_screen_type != CURRENT_SCREEN::SCREEN_HOUSE) {
        if (current_screen_type == CURRENT_SCREEN::SCREEN_SHOP_INVENTORY || current_screen_type == CURRENT_SCREEN::SCREEN_CHEST_INVENTORY) {
            pParty->setActiveCharacterIndex(uPlayerID);
            return;
        }
        if (current_screen_type != CURRENT_SCREEN::SCREEN_CHEST_INVENTORY) {
            pParty->setActiveCharacterIndex(uPlayerID);
            return;
        }
        if (pParty->activeCharacterIndex() == uPlayerID) {
            current_character_screen_window = WINDOW_CharacterWindow_Inventory;
            current_screen_type = CURRENT_SCREEN::SCREEN_CHEST_INVENTORY;
            pParty->setActiveCharacterIndex(uPlayerID);
            return;
        }
        if (pPlayers[uPlayerID]->timeToRecovery) {
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

    if (dialog_menu_id == DIALOGUE_SHOP_BUY_STANDARD || dialog_menu_id == DIALOGUE_SHOP_BUY_SPECIAL) {
        current_character_screen_window = WINDOW_CharacterWindow_Inventory;
        pGUIWindow_CurrentMenu = new GUIWindow_CharacterRecord(
            pParty->activeCharacterIndex(), CURRENT_SCREEN::SCREEN_SHOP_INVENTORY);  // CharacterUI_Initialize(SCREEN_SHOP_INVENTORY);
        return;
    }
}

void GameUI_DrawNPCPopup(void *_this) {  // PopupWindowForBenefitAndJoinText
    NPCData *pNPC;           // eax@16
    const char *pText;       // eax@18
    GUIWindow popup_window;  // [sp+Ch] [bp-60h]@23
    int a2;                  // [sp+60h] [bp-Ch]@16
    const char *lpsz = 0;        // [sp+68h] [bp-4h]@6

    if (bNoNPCHiring != 1) {
        FlatHirelings buf;
        buf.Prepare();

        if ((int64_t)((char *)_this + pParty->hirelingScrollPosition) < buf.Size()) {
            sDialogue_SpeakingActorNPC_ID = -1 - pParty->hirelingScrollPosition - (int64_t)_this;
            pNPC = GetNewNPCData(sDialogue_SpeakingActorNPC_ID, &a2);
            if (pNPC) {
                if (a2 == 57)
                    pText = pNPCTopics[512].pText;  // Baby dragon
                else
                    pText = pNPCStats->pProfessions[pNPC->profession].pBenefits;
                lpsz = pText;
                if (!pText) {
                    lpsz = pNPCStats->pProfessions[pNPC->profession].pJoinText;
                    if (!lpsz) lpsz = "";
                }
                popup_window.Init();
                popup_window.sHint.clear();
                popup_window.uFrameX = 38;
                popup_window.uFrameY = 60;
                popup_window.uFrameWidth = 276;
                popup_window.uFrameZ = 313;
                popup_window.uFrameHeight =
                    pFontArrus->CalcTextHeight(lpsz, popup_window.uFrameWidth,
                                               0) +
                    2 * pFontArrus->GetHeight() + 24;
                if ((signed int)popup_window.uFrameHeight < 130)
                    popup_window.uFrameHeight = 130;
                popup_window.uFrameWidth = 400;
                popup_window.uFrameZ = popup_window.uFrameX + 399;
                popup_window.DrawMessageBox(0);

                auto tex_name = fmt::format("NPC{:03}", pNPC->uPortraitID);
                render->DrawTextureNew(
                    (popup_window.uFrameX + 22) / 640.0f,
                    (popup_window.uFrameY + 36) / 480.0f,
                    assets->GetImage_ColorKey(tex_name));

                popup_window.DrawTitleText(pFontArrus, 0, 12, colorTable.PaleCanary.c16(), NameAndTitle(pNPC), 3);
                popup_window.uFrameWidth -= 24;
                popup_window.uFrameZ = popup_window.uFrameX + popup_window.uFrameWidth - 1;
                popup_window.DrawText(pFontArrus, {100, 36}, 0, BuildDialogueString((char *)lpsz, pParty->activeCharacterIndex() - 1, 0, 0, 0));
            }
        }
    }
}

std::string GameUI_GetMinimapHintText() {
    double v3;            // st7@1
    int v7;               // eax@4
    unsigned int pMapID;  // eax@14
    int global_coord_X;   // [sp+10h] [bp-1Ch]@1
    int global_coord_Y;   // [sp+14h] [bp-18h]@1
    int pY;      // [sp+1Ch] [bp-10h]@1
    int pX;      // [sp+28h] [bp-4h]@1

    std::string result;
    mouse->GetClickPos(&pX, &pY);
    v3 = 1.0 / (float)((signed int)viewparams->uMinimapZoom * 0.000015258789);
    global_coord_X =
        (int64_t)((double)(pX - 557) * v3 + (double)pParty->vPosition.x);
    global_coord_Y =
        (int64_t)((double)pParty->vPosition.y - (double)(pY - 74) * v3);
    if (uCurrentlyLoadedLevelType != LEVEL_Outdoor ||
        pOutdoor->pBModels.empty()) {
        pMapID = pMapStats->GetMapInfo(pCurrentMapName);
        if (pMapID == 0)
            result = "No Maze Info for this maze on file!";
        else
            result = pMapStats->pInfos[pMapID].pName;
    } else {
        for (BSPModel &model : pOutdoor->pBModels) {
            v7 = int_get_vector_length(
                abs((int)model.vBoundingCenter.x - global_coord_X),
                abs((int)model.vBoundingCenter.y - global_coord_Y), 0);
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
                if (!result.empty()) return result;
            }
        }
        pMapID = pMapStats->GetMapInfo(pCurrentMapName);
        if (pMapID == 0)
            result = "No Maze Info for this maze on file!";
        else
            result = pMapStats->pInfos[pMapID].pName;
        return result;
    }
    return result;
}

//----- (0041D3B7) --------------------------------------------------------
void GameUI_CharacterQuickRecord_Draw(GUIWindow *window, Player *player) {
    Image *v13;              // eax@6
    PlayerFrame *v15;        // eax@12
    const char *v29;         // eax@16
    int v36;                 // esi@22
    signed int uFramesetID;  // [sp+20h] [bp-8h]@9
    int uFramesetIDa;        // [sp+20h] [bp-8h]@18

    uint numActivePlayerBuffs = 0;
    for (uint i = 0; i < 24; ++i) {
        if (player->pPlayerBuffs[i].Active()) ++numActivePlayerBuffs;
    }

    window->uFrameHeight =
        ((pFontArrus->GetHeight() + 162) +
         ((numActivePlayerBuffs - 1) * pFontArrus->GetHeight()));
    window->uFrameZ = window->uFrameWidth + window->uFrameX - 1;
    window->uFrameW = ((pFontArrus->GetHeight() + 162) +
                       ((numActivePlayerBuffs - 1) * pFontArrus->GetHeight())) +
                      window->uFrameY - 1;
    window->DrawMessageBox(0);

    if (player->IsEradicated()) {
        v13 = game_ui_player_face_eradicated;
    } else if (player->IsDead()) {
        v13 = game_ui_player_face_dead;
    } else {
        uFramesetID =
            pPlayerFrameTable->GetFrameIdByExpression(player->expression);
        if (!uFramesetID) uFramesetID = 1;
        if (player->expression == CHARACTER_EXPRESSION_TALK)
            v15 = pPlayerFrameTable->GetFrameBy_y(
                &player->_expression21_frameset,
                &player->_expression21_animtime, pMiscTimer->uTimeElapsed);
        else
            v15 = pPlayerFrameTable->GetFrameBy_x(uFramesetID,
                                                  pMiscTimer->Time());
        player->uExpressionImageIndex = v15->uTextureID - 1;
        v13 = game_ui_player_faces[window->wData.val][v15->uTextureID - 1];
    }

    render->DrawTextureNew((window->uFrameX + 24) / 640.0f,
                                (window->uFrameY + 24) / 480.0f, v13);

    // TODO(captainurist): do a 2nd rewrite here
    auto str =
        fmt::format("\f{:05}", ui_character_header_text_color)
        + NameAndTitle(player->name, player->classType)
        + "\f00000\n"
        + fmt::format("{} : \f{:05}{}\f00000 / {}\n",
                      localization->GetString(LSTR_HIT_POINTS),
                      UI_GetHealthManaAndOtherQualitiesStringColor(player->health, player->GetMaxHealth()),
                      player->health, player->GetMaxHealth())
        + fmt::format("{} : \f{:05}{}\f00000 / {}\n",
                      localization->GetString(LSTR_SPELL_POINTS),
                      UI_GetHealthManaAndOtherQualitiesStringColor(player->mana, player->GetMaxMana()),
                      player->mana, player->GetMaxMana())
        + fmt::format("{}: \f{:05}{}\f00000\n",
                     localization->GetString(LSTR_CONDITION),
                     GetConditionDrawColor(player->GetMajorConditionIdx()),
                     localization->GetCharacterConditionName(player->GetMajorConditionIdx()));

    if (player->uQuickSpell != SPELL_NONE)
        v29 = pSpellStats->pInfos[player->uQuickSpell].pShortName;
    else
        v29 = localization->GetString(LSTR_NONE);

    str += fmt::format("{}: {}", localization->GetString(LSTR_QUICK_SPELL), v29);

    window->DrawText(pFontArrus, {120, 22}, 0, str, 0, 0, 0);

    uFramesetIDa = 0;
    for (uint i = 0; i < 24; ++i) {
        SpellBuff *buff = &player->pPlayerBuffs[i];
        if (buff->Active()) {
            v36 = uFramesetIDa++ * pFontComic->GetHeight() + 134;
            window->DrawText(pFontComic, {52, v36},
                             ui_game_character_record_playerbuff_colors[i],
                             localization->GetSpellName(20 + i), 0, 0, 0);
            DrawBuff_remaining_time_string(
                v36, window, buff->expireTime - pParty->GetPlayingTime(),
                pFontComic);
        }
    }

    auto active_spells = localization->FormatString(
        LSTR_FMT_ACTIVE_SPELLS_S,
        uFramesetIDa == 0 ? localization->GetString(LSTR_NONE) : "");
    window->DrawText(pFontArrus, {14, 114}, 0, active_spells, 0, 0, 0);
}

//----- (0041AD6E) --------------------------------------------------------
void GameUI_DrawRightPanelItems() {
    if (bookFlashTimer > pParty->GetPlayingTime()) {
        bookFlashTimer = GameTime(0);
    }

    if (pParty->GetPlayingTime() - bookFlashTimer > GameTime(Timer::Second)) {
        bookFlashTimer = pParty->GetPlayingTime();
        bookFlashState = !bookFlashState;
    }

    if (bookFlashState && current_screen_type != CURRENT_SCREEN::SCREEN_REST) {
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

    if (current_screen_type ==  CURRENT_SCREEN::SCREEN_BOOKS) {
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

        pPrimaryWindow->DrawText(pFontSmallnum, {0, text_y}, uGameUIFontMain, fmt::format("\r087{}", pParty->GetFood()), 0, 0, uGameUIFontShadow);
        pPrimaryWindow->DrawText(pFontSmallnum, {0, text_y}, uGameUIFontMain, fmt::format("\r028{}", pParty->GetGold()), 0, 0, uGameUIFontShadow);
        // force to render all queued text now so it wont be delayed and drawn over things it isn't supposed to, like item in hand or nuklear
        render->EndTextNew();
    }
}

//----- (0041B0C9) --------------------------------------------------------
void GameUI_DrawLifeManaBars() {
    double v3;  // st7@3
    double v7;  // st7@25

    for (uint i = 0; i < 4; ++i) {
        if (pParty->pPlayers[i].health > 0) {
            int v17 = 0;
            if (i == 2 || i == 3) v17 = 2;
            v3 = (double)pParty->pPlayers[i].health /
                 (double)pParty->pPlayers[i].GetMaxHealth();

            auto pTextureHealth = game_ui_bar_green;
            if (v3 > 0.5) {
                if (v3 > 1.0) v3 = 1.0;
            } else if (v3 > 0.25) {
                pTextureHealth = game_ui_bar_yellow;
            } else if (v3 > 0.0) {
                pTextureHealth = game_ui_bar_red;
            }
            if (v3 > 0.0) {
                render->SetUIClipRect(
                    v17 + pHealthBarPos[i],
                    (int64_t)((1.0 - v3) * pTextureHealth->GetHeight()) +
                        402,
                    v17 + pHealthBarPos[i] + pTextureHealth->GetWidth(),
                    pTextureHealth->GetHeight() + 402);
                render->DrawTextureNew((v17 + pHealthBarPos[i]) / 640.0f,
                                            402 / 480.0f, pTextureHealth);
                render->ResetUIClipRect();
            }
        }
        if (pParty->pPlayers[i].mana > 0) {
            v7 = pParty->pPlayers[i].mana /
                 (double)pParty->pPlayers[i].GetMaxMana();
            if (v7 > 1.0) v7 = 1.0;
            int v17 = 0;
            if (i == 2) v17 = 1;
            render->SetUIClipRect(
                v17 + pManaBarPos[i],
                (int64_t)((1.0 - v7) * game_ui_bar_blue->GetHeight()) +
                    402,
                v17 + pManaBarPos[i] + game_ui_bar_blue->GetWidth(),
                game_ui_bar_blue->GetHeight() + 402);
            render->DrawTextureNew((v17 + pManaBarPos[i]) / 640.0f,
                                        402 / 480.0f, game_ui_bar_blue);
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

    mouse->uPointingObjectID = 0;
    mouse->GetClickPos(&pX, &pY);
    Sizei renDims = render->GetPresentDimensions();
    if (pX < 0 || pX > renDims.w - 1 || pY < 0 ||
        pY > renDims.h - 1)
        return;

    if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
        if (pX <= (renDims.w - 1) * 0.73125 &&
            pY <= (renDims.h - 1) * 0.73125) {
            if (!pViewport->Contains(pX, pY)) {
                if (uLastPointedObjectID != 0) {
                    game_ui_status_bar_string.clear();
                    bForceDrawFooter = 1;
                }
                uLastPointedObjectID = 0;
                return;
            }

            auto vis = EngineIocContainer::ResolveVis();

            // get_picked_object_zbuf_val contains both the pid and the depth
            pickedObject = vis->get_picked_object_zbuf_val();
            mouse->uPointingObjectID = pickedObject.object_pid;
            pickedObjectID = (signed)PID_ID(pickedObject.object_pid);
            if (PID_TYPE(pickedObject.object_pid) == OBJECT_Item) {
                if (pObjectList->pObjects[pSpriteObjects[pickedObjectID].uObjectDescID].uFlags & OBJECT_DESC_UNPICKABLE) {
                    mouse->uPointingObjectID = 0;
                    game_ui_status_bar_string.clear();
                    bForceDrawFooter = 1;
                    uLastPointedObjectID = 0;
                    return;
                }
                if (pickedObject.depth >= 0x200u ||
                    pParty->pPickedItem.uItemID != ITEM_NULL) {
                    GameUI_StatusBar_Set(pSpriteObjects[pickedObjectID]
                                             .containing_item.GetDisplayName());
                } else {
                    GameUI_StatusBar_Set(localization->FormatString(
                        LSTR_FMT_GET_S,
                        pSpriteObjects[pickedObjectID].containing_item.GetDisplayName()
                        .c_str()));
                }  // intentional fallthrough
            } else if (PID_TYPE(pickedObject.object_pid) == OBJECT_Decoration) {
                if (!pLevelDecorations[pickedObjectID].uEventID) {
                    const char *pText;                 // ecx@79
                    if (pLevelDecorations[pickedObjectID].IsInteractive())
                        pText = pNPCTopics[mapEventVariables.decorVars[pLevelDecorations[pickedObjectID]._idx_in_stru123 - 75] + 380].pTopic; // campfire
                    else
                        pText = pDecorationList->GetDecoration(pLevelDecorations[pickedObjectID].uDecorationDescID)->field_20.data();
                    GameUI_StatusBar_Set(pText);
                } else {
                    std::string hintString = getEventHintString(pLevelDecorations[pickedObjectID].uEventID);
                    if (!hintString.empty()) {
                        GameUI_StatusBar_Set(hintString);
                    }
                }  // intentional fallthrough
            } else if (PID_TYPE(pickedObject.object_pid) == OBJECT_Face) {
                if (pickedObject.depth < 0x200u) {
                    std::string newString;
                    if (uCurrentlyLoadedLevelType != LEVEL_Indoor) {
                        v18b = PID_ID(pickedObject.object_pid) >> 6;
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
                        GameUI_StatusBar_Set(newString);
                        if (mouse->uPointingObjectID == 0 &&
                            uLastPointedObjectID != 0) {
                            game_ui_status_bar_string.clear();
                            bForceDrawFooter = 1;
                        }
                        uLastPointedObjectID = mouse->uPointingObjectID;
                        return;
                    }
                }
                mouse->uPointingObjectID = 0;
                game_ui_status_bar_string.clear();
                bForceDrawFooter = 1;
                uLastPointedObjectID = 0;
                return;
            } else if (PID_TYPE(pickedObject.object_pid) == OBJECT_Actor) {
                if (pickedObject.depth >= 0x2000u) {
                    mouse->uPointingObjectID = 0;
                    if (uLastPointedObjectID != 0) {
                        game_ui_status_bar_string.clear();
                        bForceDrawFooter = 1;
                    }
                    uLastPointedObjectID = 0;
                    return;
                }
                GameUI_StatusBar_Set(GetDisplayName(&pActors[pickedObjectID]));
            } else if (mouse->uPointingObjectID == 0xFFFF) {
                mouse->uPointingObjectID = 0;
            }
            if (mouse->uPointingObjectID == 0 && uLastPointedObjectID != 0) {
                game_ui_status_bar_string.clear();
                bForceDrawFooter = 1;
            }
            uLastPointedObjectID = mouse->uPointingObjectID;
            return;
        }
    } else if (current_screen_type == CURRENT_SCREEN::SCREEN_CHEST) {
        if (pX <= (renDims.w - 1) * 0.73125 &&
            pY <= (renDims.h - 1) * 0.73125) {  // if in chest area
            if (Chest::ChestUI_WritePointedObjectStatusString()) {
                return;
            } else if (uLastPointedObjectID != 0) {  // not found so reset
                game_ui_status_bar_string.clear();
                bForceDrawFooter = 1;
            }
            uLastPointedObjectID = 0;
            return;
        }
    } else {
        // if (pX <= (window->GetWidth() - 1) * 0.73125 && pY <=
        // (window->GetHeight() - 1) * 0.73125) {
        if (current_screen_type == CURRENT_SCREEN::SCREEN_CHARACTERS) {
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
                        if (uLastPointedObjectID != 0) {
                            game_ui_status_bar_string.clear();
                            bForceDrawFooter = 1;
                        }
                        //}
                        uLastPointedObjectID = 0;
                        // return;
                    } else {
                        GameUI_StatusBar_Set(pItemGen->GetDisplayName());
                        uLastPointedObjectID = 1;
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
                                pMessageType1 =
                                    (UIMessageType)pButton->uData;
                                if (pMessageType1)
                                    pCurrentFrameMessageQueue->AddGUIMessage(
                                        pMessageType1, pButton->msg_param, 0);
                                GameUI_StatusBar_Set(pButton->sLabel);
                                uLastPointedObjectID = 1;
                                return;
                            }
                            break;
                        case 2:  // hovering over portraits
                            if (pButton->uWidth != 0 && pButton->uHeight != 0) {
                                uint distW = pX - pButton->uX;
                                uint distY = pY - pButton->uY;

                                double ratioX =
                                    1.0 * (distW * distW) /
                                    (pButton->uWidth * pButton->uWidth);
                                double ratioY =
                                    1.0 * (distY * distY) /
                                    (pButton->uHeight * pButton->uHeight);

                                if (ratioX + ratioY < 1.0) {
                                    pMessageType2 =
                                        (UIMessageType)pButton->uData;
                                    if (pMessageType2 != 0)
                                        pCurrentFrameMessageQueue->AddGUIMessage(
                                            pMessageType2, pButton->msg_param,
                                            0);
                                    GameUI_StatusBar_Set(
                                        pButton->sLabel);  // for character name
                                    uLastPointedObjectID = 1;
                                    return;
                                }
                            }
                            break;
                        case 3:  // hovering over buttons
                            if (pButton->Contains(pX, pY)) {
                                PLAYER_SKILL_TYPE skill = static_cast<PLAYER_SKILL_TYPE>(pButton->msg_param);
                                PLAYER_SKILL_LEVEL skillLevel = pParty->activeCharacter().GetSkillLevel(skill);
                                requiredSkillpoints = skillLevel + 1;

                                if (skills_max_level[skill] <= skillLevel)
                                    GameUI_StatusBar_Set(localization->GetString(LSTR_SKILL_ALREADY_MASTERED));
                                else if (pParty->activeCharacter().uSkillPoints < requiredSkillpoints)
                                    GameUI_StatusBar_Set(localization->FormatString(LSTR_FMT_NEED_MORE_SKILL_POINTS, requiredSkillpoints - pParty->activeCharacter().uSkillPoints));
                                else
                                    GameUI_StatusBar_Set(localization->FormatString(LSTR_FMT_CLICKING_WILL_SPEND_POINTS, requiredSkillpoints));

                                uLastPointedObjectID = 1;
                                return;
                            }
                            break;
                    }
                }
            }

            // ?? if we get here nothing is curos over??
            if (uLastPointedObjectID != 0) {  // not found so reset
                game_ui_status_bar_string.clear();
                bForceDrawFooter = 1;
            }
            uLastPointedObjectID = 0;

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
        if (dialog_menu_id != DIALOGUE_SHOP_BUY_STANDARD
        || (v16 = render->pActiveZBuffer[pX + pSRZBufferLineOffsets[pY]], v16 ==
        0)
        || v16 == -65536)
        {
        if (uLastPointedObjectID != 0)
        {
        game_ui_status_bar_string.clear();
        bForceDrawFooter = 1;
        }
        uLastPointedObjectID = 0;
        return;
        }
        pItemGen = (ItemGen *)((char *)&pParty->pPickedItem + 36 * (v16 + 12 *
        (unsigned int)window_SpeakInHouse->ptr_1C) + 4);
        GameUI_StatusBar_Set(pItemGen->GetDisplayName());
        game_ui_status_bar_string.clear();
        bForceDrawFooter = 1;
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
                                GameUI_StatusBar_Set(pButton->sLabel);
                            } else {
                                pCurrentFrameMessageQueue->AddGUIMessage(
                                    pMessageType3, pButton->msg_param, 0);
                            }
                            uLastPointedObjectID = 1;
                            return;
                        }
                        break;
                    case 2:  // hovering over portraits
                        if (pButton->uWidth != 0 && pButton->uHeight != 0) {
                            uint distW = pX - pButton->uX;
                            uint distY = pY - pButton->uY;

                            double ratioX = 1.0 * (distW * distW) /
                                            (pButton->uWidth * pButton->uWidth);
                            double ratioY =
                                1.0 * (distY * distY) /
                                (pButton->uHeight * pButton->uHeight);

                            if (ratioX + ratioY < 1.0) {
                                pMessageType2 =
                                    (UIMessageType)pButton->uData;
                                if (pMessageType2 != 0)
                                    pCurrentFrameMessageQueue->AddGUIMessage(
                                        pMessageType2, pButton->msg_param, 0);
                                GameUI_StatusBar_Set(
                                    pButton->sLabel);  // for character name
                                uLastPointedObjectID = 1;
                                return;
                            }
                        }
                        break;
                    case 3:
                        // is this one needed?
                        __debugbreak();  // how does this work?
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
                             GameUI_StatusBar_Set(str);
                             uLastPointedObjectID = 1;
                             return;
                             }*/
                        break;
                }
            }
        }
    }

    // pMouse->uPointingObjectID = sub_46A99B(); //for software
    if (uLastPointedObjectID != 0) {
        game_ui_status_bar_string.clear();
        bForceDrawFooter = 1;
    }
    uLastPointedObjectID = 0;
    return;
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
    // TODO(pskelton): check tickcount usage here
    unsigned int v0 = platform->tickCount() / 20;
    Image *spell_texture;  // [sp-4h] [bp-1Ch]@12

    for (uint i = 0; i < 14; ++i) {
        if (pParty->pPartyBuffs[byte_4E5DD8[i]].Active()) {
            render->TexturePixelRotateDraw(pPartySpellbuffsUI_XYs[i][0] / 640.,
                pPartySpellbuffsUI_XYs[i][1] / 480., party_buff_icons[i],
                v0 + 20 * pPartySpellbuffsUI_smthns[i]);
        }
    }

    if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME ||
        current_screen_type == CURRENT_SCREEN::SCREEN_NPC_DIALOGUE) {
        if (pParty->FlyActive()) {
            if (pParty->bFlying)
                spell_texture =
                    pIconsFrameTable->GetFrame(uIconIdx_FlySpell, v0)
                        ->GetTexture();
            else
                spell_texture = pIconsFrameTable->GetFrame(uIconIdx_FlySpell, 0)
                                    ->GetTexture();
            render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, spell_texture);
        }
        if (pParty->WaterWalkActive()) {
            if (pParty->uFlags & PARTY_FLAGS_1_STANDING_ON_WATER)
                spell_texture =
                    pIconsFrameTable->GetFrame(uIconIdx_WaterWalk, v0)
                        ->GetTexture();
            else
                spell_texture =
                    pIconsFrameTable->GetFrame(uIconIdx_WaterWalk, 0)
                        ->GetTexture();
            render->DrawTextureNew(396 / 640.0f, 8 / 480.0f,
                                        spell_texture);
        }
    }

    for (uint i = 0; i < 4; ++i) {
        if (pParty->pPlayers[i].pPlayerBuffs[PLAYER_BUFF_HAMMERHANDS].Active())
            render->DrawTextureNew(
                (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72) /
                    640.0f,
                427 / 480.0f, game_ui_playerbuff_hammerhands);
        if (pParty->pPlayers[i].pPlayerBuffs[PLAYER_BUFF_BLESS].Active())
            render->DrawTextureNew(
                (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72) /
                    640.0f,
                393 / 480.0f, game_ui_playerbuff_bless);
        if (pParty->pPlayers[i].pPlayerBuffs[PLAYER_BUFF_PRESERVATION].Active())
            render->DrawTextureNew(
                (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72) /
                    640.0f,
                410 / 480.0f, game_ui_playerbuff_preservation);
        if (pParty->pPlayers[i]
                .pPlayerBuffs[PLAYER_BUFF_PAIN_REFLECTION]
                .Active())
            render->DrawTextureNew(
                (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72) /
                    640.0f,
                444 / 480.0f, game_ui_playerbuff_pain_reflection);
    }
}

//----- (004921C1) --------------------------------------------------------
void GameUI_DrawPortraits() {
    unsigned int face_expression_ID;  // eax@17
    PlayerFrame *pFrame;              // eax@21
    Image *pPortrait;                 // [sp-4h] [bp-1Ch]@27

    if (_A750D8_player_speech_timer) {
        _A750D8_player_speech_timer -= (signed int)pMiscTimer->uTimeElapsed;
        if (_A750D8_player_speech_timer <= 0) {
            if (pParty->pPlayers[uSpeakingCharacter].CanAct()) {
                pParty->pPlayers[uSpeakingCharacter].playReaction(PlayerSpeechID);
            }
            _A750D8_player_speech_timer = 0;
        }
    }

    for (uint i = 0; i < 4; ++i) {
        Player *pPlayer = &pParty->pPlayers[i];
        if (pPlayer->IsEradicated()) {
            pPortrait = game_ui_player_face_eradicated;
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active())
                render->DrawTextureGrayShade(
                    pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] /
                        640.0f,
                    387 / 480.0f, pPortrait); // was 388
            else
                render->DrawTextureNew(
                    (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] +
                     1) /
                        640.0f,
                    387 / 480.0f, pPortrait); // was 388
            if (pPlayer->pPlayerBuffs[PLAYER_BUFF_BLESS].Active() ||
                pPlayer->pPlayerBuffs[PLAYER_BUFF_HASTE].Active() ||
                pPlayer->pPlayerBuffs[PLAYER_BUFF_HEROISM].Active() ||
                pPlayer->pPlayerBuffs[PLAYER_BUFF_SHIELD].Active() ||
                pPlayer->pPlayerBuffs[PLAYER_BUFF_STONESKIN].Active()) {
                _441A4E_overlay_on_portrait(i);
            }
            continue;
        }
        if (pPlayer->IsDead()) {
            pPortrait = game_ui_player_face_dead;
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active())
                render->DrawTextureGrayShade(
                    pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] /
                        640.0f,
                    388 / 480.0f, pPortrait);
            else
                render->DrawTextureNew(
                    (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] +
                     1) /
                        640.0f,
                    388 / 480.0f, pPortrait);
            if (pPlayer->pPlayerBuffs[PLAYER_BUFF_BLESS].Active() ||
                pPlayer->pPlayerBuffs[PLAYER_BUFF_HASTE].Active() ||
                pPlayer->pPlayerBuffs[PLAYER_BUFF_HEROISM].Active() ||
                pPlayer->pPlayerBuffs[PLAYER_BUFF_SHIELD].Active() ||
                pPlayer->pPlayerBuffs[PLAYER_BUFF_STONESKIN].Active()) {
                _441A4E_overlay_on_portrait(i);
            }
            continue;
        }
        face_expression_ID = 0;
        for (size_t j = 0; j < pPlayerFrameTable->pFrames.size(); ++j)
            if (pPlayerFrameTable->pFrames[j].expression ==
                pPlayer->expression) {
                face_expression_ID = j;
                break;
            }
        if (face_expression_ID == 0) face_expression_ID = 1;
        if (pPlayer->expression == CHARACTER_EXPRESSION_TALK)
            pFrame = pPlayerFrameTable->GetFrameBy_y(
                &pPlayer->_expression21_frameset,
                &pPlayer->_expression21_animtime, pMiscTimer->uTimeElapsed);
        else
            pFrame = pPlayerFrameTable->GetFrameBy_x(
                face_expression_ID, pPlayer->uExpressionTimePassed);
        if (true /* || pPlayer->uExpressionImageIndex != pFrame->uTextureID - 1*/) {
            pPlayer->uExpressionImageIndex = pFrame->uTextureID - 1;
            pPortrait = game_ui_player_faces
                [i]
                [pPlayer
                     ->uExpressionImageIndex];  // pFace = (Texture_MM7
                                     // *)game_ui_player_faces[i][pFrame->uTextureID];
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active())
                render->DrawTextureGrayShade(
                    pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] /
                        640.0f,
                    388 / 480.0f, pPortrait);
            else
                render->DrawTextureNew(
                    (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] +
                     1) /
                        640.0f,
                    388 / 480.0f, pPortrait);
            if (pPlayer->pPlayerBuffs[PLAYER_BUFF_BLESS].Active() ||
                pPlayer->pPlayerBuffs[PLAYER_BUFF_HASTE].Active() ||
                pPlayer->pPlayerBuffs[PLAYER_BUFF_HEROISM].Active() ||
                pPlayer->pPlayerBuffs[PLAYER_BUFF_SHIELD].Active() ||
                pPlayer->pPlayerBuffs[PLAYER_BUFF_STONESKIN].Active()) {
                _441A4E_overlay_on_portrait(i);
            }
            continue;
        }
    }
    if (pParty->bTurnBasedModeOn) {
        if (pTurnEngine->turn_stage != TE_WAIT) {
            if (PID_TYPE(pTurnEngine->pQueue[0].uPackedID) == OBJECT_Player) {
                if (pTurnEngine->uActorQueueSize > 0) {
                    for (uint i = 0; i < (uint)pTurnEngine->uActorQueueSize;
                         ++i) {
                        if (PID_TYPE(pTurnEngine->pQueue[i].uPackedID) !=
                            OBJECT_Player)
                            break;

                        auto alert_texture = game_ui_player_alert_green;
                        if (pParty->GetRedAlert())
                            alert_texture = game_ui_player_alert_red;
                        else if (pParty->GetYellowAlert())
                            alert_texture = game_ui_player_alert_yellow;

                        render->DrawTextureNew(
                            (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing
                                 [PID_ID(pTurnEngine->pQueue[i].uPackedID)] -
                             4) /
                                640.0f,
                            384 / 480.0f, alert_texture); // was 385
                    }
                }
            }
        }
    } else {
        for (uint i = 0; i < 4; ++i) {
            if (pParty->pPlayers[i].CanAct() &&
                !pParty->pPlayers[i].timeToRecovery) {
                auto alert_texture = game_ui_player_alert_green;
                if (pParty->GetRedAlert())
                    alert_texture = game_ui_player_alert_red;
                else if (pParty->GetYellowAlert())
                    alert_texture = game_ui_player_alert_yellow;

                render->DrawTextureNew(
                    (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] -
                     4) /
                        640.0f,
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
    unsigned int pColor;

    signed int uCenterX = (uX + uZ) / 2;
    signed int uCenterY = (uY + uW) / 2;
    render->SetUIClipRect(uX, uY, uZ, uW);
    int uHeight = uW - uY;
    signed int uWidth = uZ - uX;

    bool bWizardEyeActive = pParty->wizardEyeActive();
    PLAYER_SKILL_MASTERY uWizardEyeSkillLevel = pParty->wizardEyeSkillLevel();
    if (CheckHiredNPCSpeciality(Cartographer)) {
        bWizardEyeActive = true;
        uWizardEyeSkillLevel = uWizardEyeSkillLevel > PLAYER_SKILL_MASTERY_EXPERT ? uWizardEyeSkillLevel : PLAYER_SKILL_MASTERY_EXPERT;
    }

    if (engine->config->debug.WizardEye.value()) {
        bWizardEyeActive = true;
        uWizardEyeSkillLevel = PLAYER_SKILL_MASTERY_MASTER;
    }

    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
        static Texture *minimaptemp;
        if (!minimaptemp) {
            minimaptemp = render->CreateTexture_Blank(uWidth, uHeight, IMAGE_FORMAT_A8B8G8R8);
        }

        static uint16_t pOdmMinimap[117][137];
        assert(sizeof(pOdmMinimap) == 137 * 117 * sizeof(short));

        bool partymoved = pParty->uFlags & PARTY_FLAGS_1_ForceRedraw;

        if (partymoved) {
            int loc_power = ImageHelper::GetWidthLn2(viewparams->location_minimap);
            map_scale = (1 << (loc_power + 16)) / (signed int)uZoom;
            startx = (double)(pParty->vPosition.x + 32768) /
                (double)(1 << (16 - loc_power));
            starty = (double)(32768 - pParty->vPosition.y) /
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

            // TODO: could stretch texture rather than rescale
            if (/*pMapLod0 && */ bRedrawOdmMinimap) {
                assert(uWidth == 137 && uHeight == 117);

                ushort MapImgWidth = viewparams->location_minimap->GetWidth();
                auto pMapLod0Line = (uint32_t*)viewparams->location_minimap->GetPixels(IMAGE_FORMAT_A8B8G8R8);
                // Image *minimaptemp = Image::Create(uWidth, uHeight, IMAGE_FORMAT_A8R8G8B8);
                auto minitempix = (uint32_t*)minimaptemp->GetPixels(IMAGE_FORMAT_A8B8G8R8);

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
    } else if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        render->FillRectFast(uX, uY, uZ - uX, uHeight, colorTable.NavyBlue.c32());
        uNumBlueFacesInBLVMinimap = 0;
        render->BeginLines2D();
        for (uint i = 0; i < (uint)pIndoor->pMapOutlines.size(); ++i) {
            BLVMapOutline *pOutline = &pIndoor->pMapOutlines[i];

            if (pIndoor->pFaces[pOutline->uFace1ID].Visible() &&
                pIndoor->pFaces[pOutline->uFace2ID].Visible()) {
                if (pOutline->uFlags & 1 || pIndoor->pFaces[pOutline->uFace1ID].uAttributes & FACE_SeenByParty ||
                    pIndoor->pFaces[pOutline->uFace2ID].uAttributes & FACE_SeenByParty) {
                    pOutline->uFlags = pOutline->uFlags | 1;
                    pIndoor->_visible_outlines[i >> 3] |= 1 << (7 - i % 8);

                    int Vert1X = pIndoor->pVertices[pIndoor->pMapOutlines[i].uVertex1ID].x - pParty->vPosition.x;
                    int Vert2X = pIndoor->pVertices[pIndoor->pMapOutlines[i].uVertex2ID].x - pParty->vPosition.x;
                    int Vert1Y = pIndoor->pVertices[pIndoor->pMapOutlines[i].uVertex1ID].y - pParty->vPosition.y;
                    int Vert2Y = pIndoor->pVertices[pIndoor->pMapOutlines[i].uVertex2ID].y - pParty->vPosition.y;

                    int linex = uCenterX + fixpoint_mul(uZoom, Vert1X);
                    int liney = uCenterY - fixpoint_mul(uZoom, Vert1Y);
                    int linez = uCenterX + fixpoint_mul(uZoom, Vert2X);
                    int linew = uCenterY - fixpoint_mul(uZoom, Vert2Y);

                    if (bWizardEyeActive && uWizardEyeSkillLevel >= PLAYER_SKILL_MASTERY_MASTER &&
                        (pIndoor->pFaces[pOutline->uFace1ID].Clickable() ||
                            pIndoor->pFaces[pOutline->uFace2ID].Clickable()) &&
                        (pIndoor->pFaceExtras[pIndoor->pFaces[pOutline->uFace1ID].uFaceExtraID].uEventID ||
                            pIndoor->pFaceExtras[pIndoor->pFaces[pOutline->uFace2ID].uFaceExtraID].uEventID)) {
                        if (uNumBlueFacesInBLVMinimap < 49) {
                            pBlueFacesInBLVMinimapIDs[uNumBlueFacesInBLVMinimap++] = i;
                            continue;
                        }
                    }

                    LineGreyDim = abs(pOutline->sZ - pParty->vPosition.z) / 8;
                    if (LineGreyDim > 100) LineGreyDim = 100;
                    render->RasterLine2D(linex, liney, linez, linew, viewparams->pPalette[-LineGreyDim + 200]);
                }
            }
        }

        for (uint i = 0; i < uNumBlueFacesInBLVMinimap; ++i) {
            BLVMapOutline *pOutline = &pIndoor->pMapOutlines[pBlueFacesInBLVMinimapIDs[i]];
            int pX = uCenterX + ((signed int)(((unsigned int)(fixpoint_mul(uZoom, pIndoor->pVertices[pOutline->uVertex1ID].x)) << 16) - uZoom * pParty->vPosition.x) >> 16);
            int pY = uCenterY - ((signed int)(((unsigned int)(fixpoint_mul(uZoom, pIndoor->pVertices[pOutline->uVertex1ID].y)) << 16) - uZoom * pParty->vPosition.y) >> 16);
            int pZ = uCenterX + ((signed int)(((unsigned int)(fixpoint_mul(uZoom, pIndoor->pVertices[pOutline->uVertex2ID].x)) << 16) - uZoom * pParty->vPosition.x) >> 16);
            int pW = uCenterY - ((signed int)(((unsigned int)(fixpoint_mul(uZoom, pIndoor->pVertices[pOutline->uVertex2ID].y)) << 16) - uZoom * pParty->vPosition.y) >> 16);
            render->RasterLine2D(pX, pY, pZ, pW, color32(ui_game_minimap_outline_color));
        }
    }

    // opengl needs slightly modified lines to show up properly
    int lineadj = 1;

    // draw objects on the minimap
    if (bWizardEyeActive) {
        if (uWizardEyeSkillLevel >= PLAYER_SKILL_MASTERY_EXPERT) {
            for (uint i = 0; i < pSpriteObjects.size(); ++i) {
                if (!pSpriteObjects[i].uType ||
                    !pSpriteObjects[i].uObjectDescID)
                    continue;
                // if (uWizardEyeSkillLevel == 1
                pPoint_X =
                    uCenterX + fixpoint_mul((pSpriteObjects[i].vPosition.x -
                                             pParty->vPosition.x),
                                            uZoom);
                pPoint_Y =
                    uCenterY - fixpoint_mul((pSpriteObjects[i].vPosition.y -
                                             pParty->vPosition.y),
                                            uZoom);
                // if ( pPoint_X >= render->raster_clip_x && pPoint_X <=
                // render->raster_clip_z &&
                //     pPoint_Y >= render->raster_clip_y && pPoint_Y <=
                //     render->raster_clip_w)
                {
                    if (pObjectList->pObjects[pSpriteObjects[i].uObjectDescID]
                            .uFlags &
                        OBJECT_DESC_UNPICKABLE) {
                        render->RasterLine2D(pPoint_X, pPoint_Y, pPoint_X + 1,
                                             pPoint_Y + 1, color32(ui_game_minimap_projectile_color));
                    } else if (uZoom > 512) {
                        render->RasterLine2D(pPoint_X - 2, pPoint_Y,
                                             pPoint_X - 2, pPoint_Y + 1 + lineadj, color32(ui_game_minimap_treasure_color));
                        render->RasterLine2D(pPoint_X - 1, pPoint_Y - 1,
                                             pPoint_X - 1, pPoint_Y + 1 + lineadj, color32(ui_game_minimap_treasure_color));
                        render->RasterLine2D(pPoint_X, pPoint_Y - 2, pPoint_X,
                                             pPoint_Y + 1 + lineadj, color32(ui_game_minimap_treasure_color));
                        render->RasterLine2D(pPoint_X + 1, pPoint_Y - 1,
                                             pPoint_X + 1, pPoint_Y + 1 + lineadj, color32(ui_game_minimap_treasure_color));
                        render->RasterLine2D(pPoint_X + 2, pPoint_Y,
                                             pPoint_X + 2, pPoint_Y + 1 + lineadj, color32(ui_game_minimap_treasure_color));
                    } else {
                        render->RasterLine2D(pPoint_X - 1, pPoint_Y - 1,
                                             pPoint_X - 1, pPoint_Y + lineadj, color32(ui_game_minimap_treasure_color));
                        render->RasterLine2D(pPoint_X, pPoint_Y - 1, pPoint_X,
                                             pPoint_Y + lineadj, color32(ui_game_minimap_treasure_color));
                    }
                }
            }
        }
        for (uint i = 0; i < pActors.size(); ++i) {  // draw actors(отрисовка монстров и нпс)
            if (pActors[i].uAIState != Removed &&
                pActors[i].uAIState != Disabled &&
                (pActors[i].uAIState == Dead || pActors[i].ActorNearby())) {
                pPoint_X =
                    uCenterX +
                    (fixpoint_mul(
                        (pActors[i].vPosition.x - pParty->vPosition.x), uZoom));
                pPoint_Y =
                    uCenterY -
                    (fixpoint_mul(
                        (pActors[i].vPosition.y - pParty->vPosition.y), uZoom));
                // if ( pPoint_X >= render->raster_clip_x && pPoint_X <=
                // render->raster_clip_z
                //  && pPoint_Y >= render->raster_clip_y && pPoint_Y <=
                //  render->raster_clip_w )
                {
                    pColor = color32(ui_game_minimap_actor_friendly_color);
                    if (pActors[i].uAttributes & ACTOR_HOSTILE)
                        pColor = color32(ui_game_minimap_actor_hostile_color);
                    if (pActors[i].uAIState == Dead)
                        pColor = color32(ui_game_minimap_actor_corpse_color);
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
        for (uint i = 0; i < (signed int)pLevelDecorations.size(); ++i) {  // draw items(отрисовка предметов)
            if (pLevelDecorations[i].uFlags & LEVEL_DECORATION_VISIBLE_ON_MAP) {
                pPoint_X =
                    uCenterX + (fixpoint_mul((pLevelDecorations[i].vPosition.x -
                                              pParty->vPosition.x), uZoom));
                pPoint_Y =
                    uCenterY - (fixpoint_mul((pLevelDecorations[i].vPosition.y -
                                              pParty->vPosition.y), uZoom));

                // if ( pPoint_X >= render->raster_clip_x && pPoint_X <=
                // render->raster_clip_z
                //  && pPoint_Y >= render->raster_clip_y && pPoint_Y <=
                //  render->raster_clip_w )
                {
                    if ((signed int)uZoom > 512) {
                        render->RasterLine2D(pPoint_X - 1, pPoint_Y - 1, pPoint_X - 1, pPoint_Y + 1, color32(ui_game_minimap_decoration_color_1));
                        render->RasterLine2D(pPoint_X, pPoint_Y - 1, pPoint_X, pPoint_Y + 1, color32(ui_game_minimap_decoration_color_1));
                        render->RasterLine2D(pPoint_X + 1, pPoint_Y - 1, pPoint_X + 1, pPoint_Y + 1, color32(ui_game_minimap_decoration_color_1));
                    } else {
                        render->RasterLine2D(pPoint_X, pPoint_Y, pPoint_X, pPoint_Y, color32(ui_game_minimap_decoration_color_1));
                    }
                }
            }
        }
    }

    render->EndLines2D();

    // draw arrow on the minimap(include. Ritor1)
    uint arrow_idx;
    unsigned int rotate = pParty->_viewYaw & TrigLUT.uDoublePiMask;
    if ((signed int)rotate <= 1920) arrow_idx = 6;
    if ((signed int)rotate < 1664) arrow_idx = 5;
    if ((signed int)rotate <= 1408) arrow_idx = 4;
    if ((signed int)rotate < 1152) arrow_idx = 3;
    if ((signed int)rotate <= 896) arrow_idx = 2;
    if ((signed int)rotate < 640) arrow_idx = 1;
    if ((signed int)rotate <= 384) arrow_idx = 0;
    if ((signed int)rotate < 128 || (signed int)rotate > 1920) arrow_idx = 7;
    render->DrawTextureNew((uCenterX - 3) / 640.0f, (uCenterY - 3) / 480.0f, game_ui_minimap_dirs[arrow_idx]);

    render->SetUIClipRect(541, 0, 567, 480);
    render->DrawTextureNew((floorf(((double)pParty->_viewYaw * 0.1171875) + 0.5f) + 285) / 640.0f,
        136 / 480.0f, game_ui_minimap_compass);
    render->ResetUIClipRect();
    render->DrawTextureNew(468 / 640.0f, 0, game_ui_minimap_frame);
}

//----- (00441498) --------------------------------------------------------
void GameUI_DrawTorchlightAndWizardEye() {
    if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME ||
        current_screen_type == CURRENT_SCREEN::SCREEN_MENU ||
        current_screen_type == CURRENT_SCREEN::SCREEN_OPTIONS ||
        current_screen_type == CURRENT_SCREEN::SCREEN_REST ||
        current_screen_type == CURRENT_SCREEN::SCREEN_SPELL_BOOK ||
        current_screen_type == CURRENT_SCREEN::SCREEN_CHEST ||
        current_screen_type == CURRENT_SCREEN::SCREEN_SAVEGAME ||
        current_screen_type == CURRENT_SCREEN::SCREEN_LOADGAME ||
        current_screen_type == CURRENT_SCREEN::SCREEN_CHEST_INVENTORY ||
        current_screen_type == CURRENT_SCREEN::SCREEN_BOOKS ||
        current_screen_type == CURRENT_SCREEN::SCREEN_BRANCHLESS_NPC_DIALOG) {
        if (pParty->TorchlightActive()) {
            render->DrawTextureNew(
                pUIAnum_Torchlight->x / 640.0f, pUIAnum_Torchlight->y / 480.0f,
                pIconsFrameTable
                    ->GetFrame(pUIAnum_Torchlight->icon->id,
                               pEventTimer->Time())
                    ->GetTexture());
        }
        if (pParty->wizardEyeActive()) {
            render->DrawTextureNew(
                pUIAnim_WizardEye->x / 640.0f, pUIAnim_WizardEye->y / 480.0f,
                pIconsFrameTable
                    ->GetFrame(pUIAnim_WizardEye->icon->id, pEventTimer->Time())
                    ->GetTexture());
        }
    }
}

//----- (00491F87) --------------------------------------------------------
void GameUI_DrawHiredNPCs() {
    unsigned int v13;               // eax@23
    char pContainer[20];            // [sp+Ch] [bp-30h]@18
    signed int uFrameID;            // [sp+24h] [bp-18h]@19
    int v22;                        // [sp+34h] [bp-8h]@2
    uint8_t pNPC_limit_ID;  // [sp+3Bh] [bp-1h]@2

    if (bNoNPCHiring != 1) {
        FlatHirelings buf;
        buf.Prepare();

        pNPC_limit_ID = 0;

        for (int i = pParty->hirelingScrollPosition; i < buf.Size() && pNPC_limit_ID < 2; i++) {
            sprintf(pContainer, "NPC%03d", buf.Get(i)->uPortraitID);
            render->DrawTextureNew(
                    pHiredNPCsIconsOffsetsX[pNPC_limit_ID] / 640.0f,
                    pHiredNPCsIconsOffsetsY[pNPC_limit_ID] / 480.0f,
                    assets->GetImage_ColorKey(pContainer));

            if (!buf.IsFollower(i) && buf.Get(i)->dialogue_1_evt_id == 1) {
                uFrameID = buf.Get(i)->dialogue_2_evt_id;
                v13 = 0;
                if (!pIconsFrameTable->pIcons.empty()) {
                    for (v13 = 0; v13 < pIconsFrameTable->pIcons.size(); ++v13) {
                        if (iequals("spell96", pIconsFrameTable->pIcons[v13].GetAnimationName()))
                            break;
                    }
                }
                render->DrawTextureNew(
                    pHiredNPCsIconsOffsetsX[pNPC_limit_ID] / 640.0f,
                    pHiredNPCsIconsOffsetsY[pNPC_limit_ID] / 480.0f,
                    pIconsFrameTable->GetFrame(v13, uFrameID)->GetTexture());
            }
            ++pNPC_limit_ID;
        }
    }
}

//----- (004178FE) --------------------------------------------------------
unsigned int UI_GetHealthManaAndOtherQualitiesStringColor(int actual_value,
                                                          int base_value) {
    uint16_t R, G, B;

    if (actual_value == base_value) {
        R = 0, G = 0, B = 0;  // White
    } else if (actual_value < base_value) {
        if (100 * actual_value / base_value >=
            25)  // Yellow( current_pos > 1/4 )
            R = 255, G = 255, B = 100;
        else  // Red( current_pos < 1/4 )
            R = 255, G = 0, B = 0;
    } else {  // Green
        R = 0, G = 255, B = 0;
    }

    return color16(R, G, B);
}

//----- (00417939) --------------------------------------------------------
int GetConditionDrawColor(Condition uConditionIdx) {
    switch (uConditionIdx) {
        case Condition_Zombie:
        case Condition_Good:
            return ui_character_condition_normal_color;

        case Condition_Cursed:
        case Condition_Weak:
        case Condition_Fear:
        case Condition_Drunk:
        case Condition_Insane:
        case Condition_Poison_Weak:
        case Condition_Disease_Weak:
            return ui_character_condition_light_color;

        case Condition_Sleep:
        case Condition_Poison_Medium:
        case Condition_Disease_Medium:
        case Condition_Paralyzed:
        case Condition_Unconscious:
            return ui_character_condition_moderate_color;

        case Condition_Poison_Severe:
        case Condition_Disease_Severe:
        case Condition_Dead:
        case Condition_Petrified:
        case Condition_Eradicated:
            return ui_character_condition_severe_color;
    }
    Error("Invalid condition (%u)", uConditionIdx);
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

int16_t _441A4E_overlay_on_portrait(int a1) {  // for blessing
    int16_t result;                            // ax@1
    int v2;                                    // ebx@1
    bool v5;                                   // ecx@4
    SpriteFrame *pFrame;                       // eax@6
    int v8;                                    // eax@6
    SoftwareBillboard v10 {};                     // [sp+Ch] [bp-5Ch]@1
    int v11;                                   // [sp+5Ch] [bp-Ch]@6
    int v12;                                   // [sp+60h] [bp-8h]@1
    Sizei renDims = render->GetPresentDimensions();

    v10.sParentBillboardID = -1;
    v10.pTargetZ = render->pActiveZBuffer;
    v10.uTargetPitch = renDims.w;
    result = 0;
    v2 = a1;
    v10.uViewportX = 0;
    v10.uViewportY = 0;
    v10.uViewportZ = renDims.w - 1;
    v10.uViewportW = renDims.h - 1;
    v12 = 0;
    // v3 = (char *)&pOtherOverlayList->pOverlays[0].field_C;
    // do
    for (uint i = 0; i < 50; ++i) {
        if (pOtherOverlayList->pOverlays[i].field_6 > 0) {
            result = pOtherOverlayList->pOverlays[i].field_0;
            if (pOtherOverlayList->pOverlays[i].field_0 >= 300) {
                // v4 = result;
                v5 = pOtherOverlayList->pOverlays[i].field_0 == v2 + 320 ||
                     pOtherOverlayList->pOverlays[i].field_0 == v2 + 330 ||
                     pOtherOverlayList->pOverlays[i].field_0 == v2 + 340 ||
                     pOtherOverlayList->pOverlays[i].field_0 == v2 + 350;
                pOtherOverlayList->pOverlays[i].field_0 = v2 + 310;
                if (pOtherOverlayList->pOverlays[i].field_0 == v2 + 310 || v5) {
                    if (!pOtherOverlayList->pOverlays[i].field_0) {
                        pFrame = pSpriteFrameTable->GetFrame(
                            pOverlayList
                                ->pOverlays[pOtherOverlayList->pOverlays[i]
                                                .field_2]
                                .uSpriteFramesetID,
                            pOtherOverlayList->pOverlays[i].sprite_frame_time);
                        // v7 = v6;
                        v11 = pOtherOverlayList->pOverlays[i].field_E;
                        // v13 = pFrame->scale;
                        // v13 = fixpoint_mul(v11, pFrame->scale);
                        v10.screen_space_x =
                            pOtherOverlayList->pOverlays[i].screen_space_x;
                        v10.screen_space_y =
                            pOtherOverlayList->pOverlays[i].screen_space_y;
                        v10.screenspace_projection_factor_x = v11 * pFrame->scale;
                        v10.screenspace_projection_factor_y = v11 * pFrame->scale;
                        // TODO (pskelton): what is this?
                        v10.pPalette = 0; /*PaletteManager::Get_Dark_or_Red_LUT(pFrame->uPaletteIndex, 0, 1);*/
                        v8 = pOtherOverlayList->pOverlays[i].field_2;
                        v10.screen_space_z = 0;
                        v10.object_pid = 0;
                        v10.uFlags = 0;
                        // v9 = pOverlayList->pOverlays[v8].uOverlayType;
                        if (!pOverlayList->pOverlays[v8].uOverlayType ||
                            pOverlayList->pOverlays[v8].uOverlayType == 2)
                            v10.screen_space_y +=
                                pFrame->hw_sprites[0]->sprite_header->uHeight /
                                2;
                        result = (int64_t)pFrame->hw_sprites[0]->sprite_header->_4AD2D1_overlays(&v10, 0);
                        ++v12;
                        if (v12 == 5) break;
                    }
                }
            }
        }
        // v3 += 20;
    }
    // while ( (signed int)v3 < (signed int)&pOverlayList->pOverlays );
    return result;
}


GUIWindow_DebugMenu::GUIWindow_DebugMenu()
    : GUIWindow(WINDOW_DebugMenu, {0, 0}, render->GetRenderDimensions(), 0) {

    pEventTimer->Pause();
    int width = 108;
    int height = 20;

    game_ui_menu_options = assets->GetImage_ColorKey("options");

    GUIButton *pBtn_DebugTownPortal = CreateButton({13, 140}, {width, height}, 1, 0, UIMSG_DebugTownPortal, 0, InputAction::Invalid, "DEBUG TOWN PORTAL");
    GUIButton *pBtn_DebugGiveGold = CreateButton({127, 140}, {width, height}, 1, 0, UIMSG_DebugGiveGold, 0, InputAction::Invalid, "DEBUG GIVE GOLD (10000)");
    GUIButton *pBtn_DebugGiveEXP = CreateButton({241, 140}, {width, height}, 1, 0, UIMSG_DebugGiveEXP, 0, InputAction::Invalid, "DEBUG GIVE EXP (20000)");
    GUIButton *pBtn_DebugGiveSkillP = CreateButton({354, 140}, {width, height}, 1, 0, UIMSG_DebugGiveSkillP, 0, InputAction::Invalid, "DEBUG GIVE SKILL POINT (50)");

    GUIButton *pBtn_DebugLearnSkill = CreateButton({13, 167}, {width, height}, 1, 0, UIMSG_DebugLearnSkills, 0, InputAction::Invalid, "DEBUG LEARN CLASS SKILLS");
    GUIButton *pBtn_DebugRemoveGold = CreateButton({127, 167}, {width, height}, 1, 0, UIMSG_DebugTakeGold, 0, InputAction::Invalid, "DEBUG REMOVE GOLD");
    GUIButton *pBtn_DebugAddFood = CreateButton({241, 167}, {width, height}, 1, 0, UIMSG_DebugGiveFood, 0, InputAction::Invalid, "DEBUG GIVE FOOD (20)");
    GUIButton *pBtn_DebugTakeFood = CreateButton({354, 167}, {width, height}, 1, 0, UIMSG_DebugTakeFood, 0, InputAction::Invalid, "DEBUG REMOVE FOOD");

    GUIButton *pBtn_DebugCycleAlign = CreateButton({13, 194}, {width, height}, 1, 0, UIMSG_DebugCycleAlign, 0, InputAction::Invalid, "DEBUG CYCLE ALIGNMENT");
    GUIButton *pBtn_DebugWizardEye = CreateButton({127, 194}, {width, height}, 1, 0, UIMSG_DebugWizardEye, 0, InputAction::Invalid, "DEBUG TOGGLE WIZARD EYE");
    GUIButton *pBtn_DebugAllMagic = CreateButton({241, 194}, {width, height}, 1, 0, UIMSG_DebugAllMagic, 0, InputAction::Invalid, "DEBUG TOGGLE All MAGIC");
    GUIButton *pBtn_DebugTerrain = CreateButton({354, 194}, {width, height}, 1, 0, UIMSG_DebugTerrain, 0, InputAction::Invalid, "DEBUG TOGGLE TERRAIN");

    GUIButton *pBtn_DebugLightMap = CreateButton({13, 221}, {width, height}, 1, 0, UIMSG_DebugLightmap, 0, InputAction::Invalid, "DEBUG TOGGLE LIGHTMAP DECAL");
    GUIButton *pBtn_DebugTurbo = CreateButton({127, 221}, {width, height}, 1, 0, UIMSG_DebugTurboSpeed, 0, InputAction::Invalid, "DEBUG TOGGLE TURBO SPEED");
    GUIButton *pBtn_DebugNoActors = CreateButton({241, 221}, {width, height}, 1, 0, UIMSG_DebugNoActors, 0, InputAction::Invalid, "DEBUG TOGGLE ACTORS");
    GUIButton *pBtn_DebugUnused = CreateButton({354, 221}, {width, height}, 1, 0, UIMSG_DebugFog, 0, InputAction::Invalid, "DEBUG TOGGLE FOG");

    GUIButton *pBtn_DebugSnow = CreateButton({13, 248}, {width, height}, 1, 0, UIMSG_DebugSnow, 0, InputAction::Invalid, "DEBUG TOGGLE SNOW");
    GUIButton *pBtn_DebugPortalLines = CreateButton({127, 248}, {width, height}, 1, 0, UIMSG_DebugPortalLines, 0, InputAction::Invalid, "DEBUG TOGGLE PORTAL OUTLINES");
    GUIButton *pBtn_DebugPickedFace = CreateButton({241, 248}, {width, height}, 1, 0, UIMSG_DebugPickedFace, 0, InputAction::Invalid, "DEBUG TOGGLE SHOW PICKED FACE");
    GUIButton *pBtn_DebugShowFPS = CreateButton({354, 248}, {width, height}, 1, 0, UIMSG_DebugShowFPS, 0, InputAction::Invalid, "DEBUG TOGGLE SHOW FPS");

    GUIButton *pBtn_DebugSeasonsChange = CreateButton({13, 275}, {width, height}, 1, 0, UIMSG_DebugSeasonsChange, 0, InputAction::Invalid, "DEBUG TOGGLE SEASONS CHANGE");
    GUIButton *pBtn_DebugVerboseLogging = CreateButton({127, 275}, {width, height}, 1, 0, UIMSG_DebugVerboseLogging, 0, InputAction::Invalid, "DEBUG TOGGLE VERBOSE LOGGING");
    GUIButton *pBtn_DebugGenItem = CreateButton({241, 275}, {width, height}, 1, 0, UIMSG_DebugGenItem, 0, InputAction::Invalid, "DEBUG GENERATE RANDOM ITEM");
    GUIButton *pBtn_DebugSpecialItem = CreateButton({354, 275}, {width, height}, 1, 0, UIMSG_DebugSpecialItem, 0, InputAction::Invalid, "DEBUG GENERATE RANDOM SPECIAL ITEM");

    GUIButton *pBtn_DebugReloadShaders = CreateButton({13, 302}, {width, height}, 1, 0, UIMSG_DebugReloadShader, 0, InputAction::ReloadShaders, "DEBUG RELOAD SHADERS");
    GUIButton *pBtn_DebugUnused1 = CreateButton({127, 302}, {width, height}, 1, 0, UIMSG_DebugUnused, 0, InputAction::Invalid, "DEBUG unused1");
    GUIButton *pBtn_DebugUnused2 = CreateButton({241, 302}, {width, height}, 1, 0, UIMSG_DebugUnused, 0, InputAction::Invalid, "DEBUG unused2");
    GUIButton *pBtn_DebugUnused3 = CreateButton({354, 302}, {width, height}, 1, 0, UIMSG_DebugUnused, 0, InputAction::Invalid, "DEBUG unused3");

    GUIButton *pBtn_DebugKillChar = CreateButton({13, 329}, {width, height}, 1, 0, UIMSG_DebugKillChar, 0, InputAction::Invalid, "DEBUG KILL SELECTED CHARACTER");
    GUIButton *pBtn_DebugEradicate = CreateButton({127, 329}, {width, height}, 1, 0, UIMSG_DebugEradicate, 0, InputAction::Invalid, "DEBUG ERADICATE SELECTED CHARACTER");
    GUIButton *pBtn_DebugNoDamage = CreateButton({241, 329}, {width, height}, 1, 0, UIMSG_DebugNoDamage, 0, InputAction::Invalid, "DEBUG TOGGLE NO DAMAGE");
    GUIButton *pBtn_DebugFullHeal = CreateButton({354, 329}, {width, height}, 1, 0, UIMSG_DebugFullHeal, 0, InputAction::Invalid, "DEBUG FULLY HEAL SELECTED CHARACTER");
}

void GUIWindow_DebugMenu::Update() {
    // could move the drwa flush to update windows?
    render->DrawTwodVerts();
    render->EndLines2D();
    render->EndTextNew();

    render->BeginLines2D();

    render->DrawTextureNew(pViewport->uViewportTL_X / 640.0f,
        pViewport->uViewportTL_Y / 480.0f,
        game_ui_menu_options);

    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {0, 10}, 0, "Debug Menu", 0, 0, 0);

    buttonbox(13, 140, "Town Portal", engine->config->debug.TownPortal.value());
    buttonbox(127, 140, "Give Gold", 2);
    buttonbox(241, 140, "Give EXP", 2);
    buttonbox(354, 140, "Give Skill", 2);

    buttonbox(13, 167, "Learn Skills", 2);
    buttonbox(127, 167, "Take Gold", 2);
    buttonbox(241, 167, "Give Food", 2);
    buttonbox(354, 167, "Take Food", 2);

    int col = 0;
    if (pParty->alignment == PartyAlignment::PartyAlignment_Evil) col = 0;
    if (pParty->alignment == PartyAlignment::PartyAlignment_Good) col = 1;
    if (pParty->alignment == PartyAlignment::PartyAlignment_Neutral) col = 2;
    buttonbox(13, 194, "Alignment", col);
    buttonbox(127, 194, "WizardEye", engine->config->debug.WizardEye.value());
    buttonbox(241, 194, "All Magic", engine->config->debug.AllMagic.value());
    buttonbox(354, 194, "Terrain", engine->config->debug.Terrain.value());

    buttonbox(13, 221, "Lightmap", engine->config->debug.LightmapDecals.value());
    buttonbox(127, 221, "Turbo", engine->config->debug.TurboSpeed.value());
    buttonbox(241, 221, "No Actors", engine->config->debug.NoActors.value());
    buttonbox(354, 221, "Fog", engine->config->graphics.Fog.value());

    buttonbox(13, 248, "Snow", engine->config->graphics.Snow.value());
    buttonbox(127, 248, "Portal Lines", engine->config->debug.PortalOutlines.value());
    buttonbox(241, 248, "Picked Face", engine->config->debug.ShowPickedFace.value());
    buttonbox(354, 248, "Show FPS", engine->config->debug.ShowFPS.value());

    buttonbox(13, 275, "Seasons", engine->config->graphics.SeasonsChange.value());
    buttonbox(127, 275, "Verbose Log", engine->config->debug.VerboseLogging.value());
    buttonbox(241, 275, "Gen Item", 2);
    buttonbox(354, 275, "Special Item", 2);

    buttonbox(13, 302, "HOT Shaders", 2);
    buttonbox(127, 302, "Unused1", 2);
    buttonbox(241, 302, "Unused2", 2);
    buttonbox(354, 302, "Unused3", 2);

    // times ??
    // conditions ??

    buttonbox(13, 329, "Dead", 2);
    buttonbox(127, 329, "Eradicate", 2);
    buttonbox(241, 329, "No Damage", engine->config->debug.NoDamage.value());
    buttonbox(354, 329, "Full Heal", 2);

    //render->DrawTwodVerts();
    //render->EndLines2D();
}

void buttonbox(int x, int y, const char *text, int col) {
    int width = 108;
    int height = 20;
    render->FillRectFast(x, y, width+1, height+1, color32(50, 50, 50));

    //render->BeginLines2D();
    render->RasterLine2D(x-1, y-1, x+width+1, y-1, colorTable.Jonquil.c32());
    render->RasterLine2D(x-1, y-1, x-1, y+height+1, colorTable.Jonquil.c32());
    render->RasterLine2D(x-1, y+height+1, x+width+1, y+height+1, colorTable.Jonquil.c32());
    render->RasterLine2D(x+width+1, y-1, x+width+1, y+height+1, colorTable.Jonquil.c32());
    //render->EndLines2D();

    uint16_t colour = ui_character_condition_severe_color;
    if (col == 2) {
        colour = 0;
    }
    if (col == 1) {
        colour = ui_character_bonus_text_color;
    }
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {x+1, y+2}, colour, text, 0, 0, 0);
}
