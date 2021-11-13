#include <map>

#include "GUI/UI/UIGame.h"

#include "src/Application/Game.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Events.h"
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
#include "Engine/stru123.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UICharacter.h"
#include "GUI/UI/UIDialogue.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIStatusBar.h"

#include "Io/InputAction.h"
#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "Platform/Api.h"
#include "Platform/OSWindow.h"


using EngineIoc = Engine_::IocContainer;
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

extern InputAction currently_selected_action_for_binding;  // 506E68
extern std::map<InputAction, bool> key_map_conflicted;  // 506E6C
extern std::map<InputAction, GameKey> prev_key_map;

GUIWindow_GameMenu::GUIWindow_GameMenu()
    : GUIWindow(WINDOW_GameMenu, 0, 0, window->GetWidth(), window->GetHeight(), 0) {
    game_ui_menu_options = assets->GetImage_ColorKey("options", 0x7FF);
    game_ui_menu_new = assets->GetImage_ColorKey("new1", 0x7FF);
    game_ui_menu_load = assets->GetImage_ColorKey("load1", 0x7FF);
    game_ui_menu_save = assets->GetImage_ColorKey("save1", 0x7FF);
    game_ui_menu_controls = assets->GetImage_ColorKey("controls1", 0x7FF);
    game_ui_menu_resume = assets->GetImage_ColorKey("resume1", 0x7FF);
    game_ui_menu_quit = assets->GetImage_ColorKey("quit1", 0x7FF);

    pBtn_NewGame = CreateButton(0x13u, 0x9Bu, 0xD6u, 0x28u, 1, 0,
                                UIMSG_StartNewGame, 0, GameKey::N,
                                localization->GetString(614),  // "New Game"
                                {{game_ui_menu_new}});
    pBtn_SaveGame = CreateButton(0x13u, 0xD1u, 0xD6u, 0x28u, 1, 0,
                                 UIMSG_Game_OpenSaveGameDialog, 0, GameKey::S,
                                 localization->GetString(615),  // "Save Game"
                                 {{game_ui_menu_save}});
    pBtn_LoadGame = CreateButton(19, 263, 0xD6u, 0x28u, 1, 0,
                                 UIMSG_Game_OpenLoadGameDialog, 0, GameKey::L,
                                 localization->GetString(616),  // "Load Game"
                                 {{game_ui_menu_load}});
    pBtn_GameControls = CreateButton(
        241, 155, 214, 40, 1, 0, UIMSG_Game_OpenOptionsDialog, 0, GameKey::C,
        localization->GetString(617),  // ""Sound, Keyboard, Game Options:""
        {{game_ui_menu_controls}});
    pBtn_QuitGame = CreateButton(241, 209, 214, 40, 1, 0, UIMSG_Quit, 0, GameKey::Q,
                                 localization->GetString(618),  // "Quit"
                                 {{game_ui_menu_quit}});
    pBtn_Resume = CreateButton(
        241, 263, 214, 40, 1, 0, UIMSG_GameMenu_ReturnToGame, 0, GameKey::R,
        localization->GetString(619),  // "Return to Game"
        {{game_ui_menu_resume}});
    _41D08F_set_keyboard_control_group(6, 1, 0, 0);
}

void GUIWindow_GameMenu::Update() {
    // -----------------------------------
    // 004156F0 GUI_UpdateWindows --- part
    render->DrawTextureAlphaNew(pViewport->uViewportTL_X / 640.0f,
                                pViewport->uViewportTL_Y / 480.0f,
                                game_ui_menu_options);

    viewparams->bRedrawGameUI = true;
}

//----- (00491CB5) --------------------------------------------------------
void GameUI_LoadPlayerPortraintsAndVoices() {
    for (uint i = 0; i < 4; ++i) {
        for (uint j = 0; j < 56; ++j) {
            game_ui_player_faces[i][j] = assets->GetImage_ColorKey(
                StringPrintf(
                    "%s%02d",
                    pPlayerPortraitsNames[pParty->pPlayers[i].uCurrentFace],
                    j + 1),
                0x7FF);
        }
    }

    game_ui_player_face_eradicated =
        assets->GetImage_ColorKey("ERADCATE", 0x7FF);
    game_ui_player_face_dead = assets->GetImage_ColorKey("DEAD", 0x7FF);
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
            StringPrintf("%s%02d", pPlayerPortraitsNames[face_id], i + 1);
        game_ui_player_faces[player_id][i] =
            assets->GetImage_ColorKey(filename, 0x7FF);
    }
}

extern std::map<InputAction, bool> key_map_conflicted;  // 506E6C
//----- (00414D24) --------------------------------------------------------
static unsigned int GameMenuUI_GetKeyBindingColor(InputAction action) {
    if (currently_selected_action_for_binding == action) {
        if (OS_GetTime() % 1000 < 500)
            return ui_gamemenu_keys_key_selection_blink_color_1;
        else
            return ui_gamemenu_keys_key_selection_blink_color_2;
    } else if (key_map_conflicted[action]) {
        int intensity;

        int time = OS_GetTime() % 800;
        if (time < 400)
            intensity = -70 + 70 * time / 400;
        else
            intensity = +70 - 70 * time / 800;

        return Color16(185 + intensity, 40 + intensity / 4, 40 + intensity / 4);
    }

    return ui_gamemenu_keys_key_default_color;
}

GUIWindow_GameKeyBindings::GUIWindow_GameKeyBindings()
    : GUIWindow(WINDOW_KeyMappingOptions, 0, 0, window->GetWidth(), window->GetHeight(), 0) {
    game_ui_options_controls[0] = assets->GetImage_ColorKey("optkb", 0x7FF);
    game_ui_options_controls[1] = assets->GetImage_ColorKey("optkb_h", 0x7FF);
    game_ui_options_controls[2] = assets->GetImage_ColorKey("resume1", 0x7FF);
    game_ui_options_controls[3] = assets->GetImage_ColorKey("optkb_1", 0x7FF);
    game_ui_options_controls[4] = assets->GetImage_ColorKey("optkb_2", 0x7FF);

    CreateButton(241, 302, 214, 40, 1, 0, UIMSG_Escape, 0);

    CreateButton(19, 302, 108, 20, 1, 0, UIMSG_SelectKeyPage1, 0);
    CreateButton(127, 302, 108, 20, 1, 0, UIMSG_SelectKeyPage2, 0);
    CreateButton(127, 324, 108, 20, 1, 0, UIMSG_ResetKeyMapping, 0);
    CreateButton(19, 324, 108, 20, 1, 0, UIMSG_Game_OpenOptionsDialog, 0);

    CreateButton(129, 148, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 0);
    CreateButton(129, 167, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 1);
    CreateButton(129, 186, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 2);
    CreateButton(129, 205, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 3);
    CreateButton(129, 224, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 4);
    CreateButton(129, 243, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 5);
    CreateButton(129, 262, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 6);

    CreateButton(350, 148, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 7);
    CreateButton(350, 167, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 8);
    CreateButton(350, 186, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 9);
    CreateButton(350, 205, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 10);
    CreateButton(350, 224, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 11);
    CreateButton(350, 243, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 12);
    CreateButton(350, 262, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 13);

    currently_selected_action_for_binding = InputAction::Invalid;
    KeyboardPageNum = 1;
    for (auto action : AllInputActions()) {
        key_map_conflicted[action] = false;
        prev_key_map[action] = keyboardActionMapping->GetKey(action);
    }
}

//----- (004142D3) --------------------------------------------------------
void GUIWindow_GameKeyBindings::Update() {
    // int v4;  // ecx@7
    // int v5;  // eax@8

    if (pGUIWindow_CurrentMenu->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_CONFIRMED) {
        InputAction action = currently_selected_action_for_binding;
        GameKey newKey = keyboardInputHandler->LastPressedKey();
        prev_key_map[action] = newKey;

        for (auto action : AllInputActions()) {
            key_map_conflicted[action] = false;
        }

        for (auto x : prev_key_map) {
            if (x.first != action && x.second == newKey) {
                key_map_conflicted[action] = true;
                key_map_conflicted[x.first] = true;
            }
        }

        keyboardInputHandler->EndTextInput();
        currently_selected_action_for_binding = InputAction::Invalid;
    }
    render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, game_ui_options_controls[0]);  // draw base texture

    int base_controls_offset = 0;
    if (KeyboardPageNum == 1) {
        render->DrawTextureAlphaNew(19 / 640.0f, 302 / 480.0f, game_ui_options_controls[3]);
    } else {
        base_controls_offset = 14;
        render->DrawTextureAlphaNew(127 / 640.0f, 302 / 480.0f, game_ui_options_controls[4]);
    }

    for (int i = 0; i < 7; ++i) {
        InputAction action1 = (InputAction)(base_controls_offset + i);
        pGUIWindow_CurrentMenu->DrawText(
            pFontLucida, 23, 142 + i * 21,
            ui_gamemenu_keys_action_name_color,
            GetDisplayName(action1).c_str(), 0, 0, 0
        );
        pGUIWindow_CurrentMenu->DrawText(
            pFontLucida, 127, 142 + i * 21,
            GameMenuUI_GetKeyBindingColor(action1),
            GetDisplayName(prev_key_map[action1]), 0, 0, 0
        );

        int j = i + 7;
        InputAction action2 = (InputAction)(base_controls_offset + j);
        pGUIWindow_CurrentMenu->DrawText(
            pFontLucida, 247, 142 + i * 21,
            ui_gamemenu_keys_action_name_color,
            GetDisplayName(action2).c_str(), 0, 0, 0
        );
        pGUIWindow_CurrentMenu->DrawText(
            pFontLucida, 350, 142 + i * 21,
            GameMenuUI_GetKeyBindingColor(action2),
            GetDisplayName(prev_key_map[action2]), 0, 0, 0
        );
    }
}

GUIWindow_GameVideoOptions::GUIWindow_GameVideoOptions()
    : GUIWindow(WINDOW_VideoOptions, 0, 0, window->GetWidth(), window->GetHeight(), 0) {
    // -------------------------------------
    // GameMenuUI_OptionsVideo_Load --- part
    game_ui_menu_options_video_background = assets->GetImage_ColorKey("optvid", 0x7FF);
    game_ui_menu_options_video_bloodsplats = assets->GetImage_ColorKey("opvdH-bs", 0x7FF);
    game_ui_menu_options_video_coloredlights = assets->GetImage_ColorKey("opvdH-cl", 0x7FF);
    game_ui_menu_options_video_tinting = assets->GetImage_ColorKey("opvdH-tn", 0x7FF);

    game_ui_menu_options_video_gamma_positions[0] = assets->GetImage_ColorKey("convol10", 0x7FF);
    game_ui_menu_options_video_gamma_positions[1] = assets->GetImage_ColorKey("convol20", 0x7FF);
    game_ui_menu_options_video_gamma_positions[2] = assets->GetImage_ColorKey("convol30", 0x7FF);
    game_ui_menu_options_video_gamma_positions[3] = assets->GetImage_ColorKey("convol40", 0x7FF);
    game_ui_menu_options_video_gamma_positions[4] = assets->GetImage_ColorKey("convol50", 0x7FF);
    game_ui_menu_options_video_gamma_positions[5] = assets->GetImage_ColorKey("convol60", 0x7FF);
    game_ui_menu_options_video_gamma_positions[6] = assets->GetImage_ColorKey("convol70", 0x7FF);
    game_ui_menu_options_video_gamma_positions[7] = assets->GetImage_ColorKey("convol80", 0x7FF);
    game_ui_menu_options_video_gamma_positions[8] = assets->GetImage_ColorKey("convol90", 0x7FF);
    game_ui_menu_options_video_gamma_positions[9] = assets->GetImage_ColorKey("convol00", 0x7FF);
    // not_available_bloodsplats_texture_id =
    // pIcons_LOD->LoadTexture("opvdG-bs", TEXTURE_16BIT_PALETTE);
    // not_available_us_colored_lights_texture_id =
    // pIcons_LOD->LoadTexture("opvdG-cl", TEXTURE_16BIT_PALETTE);
    // not_available_tinting_texture_id = pIcons_LOD->LoadTexture("opvdG-tn",
    // TEXTURE_16BIT_PALETTE);

    CreateButton(0xF1u, 0x12Eu, 0xD6u, 0x28u, 1, 0, UIMSG_Escape, 0);
    // if ( render->pRenderD3D )
    {
        CreateButton(0x13u, 0x118u, 0xD6u, 0x12u, 1, 0, UIMSG_ToggleBloodsplats, 0);
        CreateButton(0x13u, 0x12Eu, 0xD6u, 0x12u, 1, 0, UIMSG_ToggleColoredLights, 0);
        CreateButton(0x13u, 0x144u, 0xD6u, 0x12u, 1, 0, UIMSG_ToggleTint, 0);
    }
}

//----- (00414D9A) --------------------------------------------------------
void GUIWindow_GameVideoOptions::Update() {
    // -----------------------------------
    // 004156F0 GUI_UpdateWindows --- part
    GUIWindow msg_window;  // [sp+8h] [bp-54h]@3

    render->DrawTextureAlphaNew(
        8 / 640.0f, 8 / 480.0f,
        game_ui_menu_options_video_background);  // draw base texture
    // if ( !render->bWindowMode && render->IsGammaSupported() )
    {
        render->DrawTextureAlphaNew(
            (17 * uGammaPos + 42) / 640.0f, 162 / 480.0f,
            game_ui_menu_options_video_gamma_positions[uGammaPos]);

        render->DrawTextureNew(274 / 640.0f, 169 / 480.0f, gamma_preview_image);
        msg_window.uFrameX = 22;
        msg_window.uFrameY = 190;
        msg_window.uFrameWidth = 211;
        msg_window.uFrameHeight = 79;
        msg_window.uFrameZ = 232;
        msg_window.uFrameW = 268;
        msg_window.DrawTitleText(
            pFontSmallnum, 0, 0, ui_gamemenu_video_gamma_title_color,
            localization->GetString(226),
            3);  // "Gamma controls the relative ""brightness"" of the game. May
                 // vary depending on your monitor."
    }

    if (!engine->config->NoBloodsplats())
        render->DrawTextureAlphaNew(20 / 640.0f, 281 / 480.0f, game_ui_menu_options_video_bloodsplats);
    if (render->config->is_using_colored_lights)
        render->DrawTextureAlphaNew(20 / 640.0f, 303 / 480.0f, game_ui_menu_options_video_coloredlights);
    if (render->config->is_tinting)
        render->DrawTextureAlphaNew(20 / 640.0f, 325 / 480.0f, game_ui_menu_options_video_tinting);
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

void OptionsMenuSkin::Relaease() {
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
    : GUIWindow(WINDOW_GameOptions, 0, 0, window->GetWidth(), window->GetHeight(), 0) {
    options_menu_skin.uTextureID_Background = assets->GetImage_ColorKey("ControlBG", 0x7FF);
    options_menu_skin.uTextureID_TurnSpeed[2] = assets->GetImage_ColorKey("con_16x", 0x7FF);
    options_menu_skin.uTextureID_TurnSpeed[1] = assets->GetImage_ColorKey("con_32x", 0x7FF);
    options_menu_skin.uTextureID_TurnSpeed[0] = assets->GetImage_ColorKey("con_Smoo", 0x7FF);
    options_menu_skin.uTextureID_ArrowLeft = assets->GetImage_Alpha("con_ArrL");
    options_menu_skin.uTextureID_ArrowRight = assets->GetImage_Alpha("con_ArrR");
    options_menu_skin.uTextureID_SoundLevels[0] = assets->GetImage_ColorKey("convol10", 0x7FF);
    options_menu_skin.uTextureID_SoundLevels[1] = assets->GetImage_ColorKey("convol20", 0x7FF);
    options_menu_skin.uTextureID_SoundLevels[2] = assets->GetImage_ColorKey("convol30", 0x7FF);
    options_menu_skin.uTextureID_SoundLevels[3] = assets->GetImage_ColorKey("convol40", 0x7FF);
    options_menu_skin.uTextureID_SoundLevels[4] = assets->GetImage_ColorKey("convol50", 0x7FF);
    options_menu_skin.uTextureID_SoundLevels[5] = assets->GetImage_ColorKey("convol60", 0x7FF);
    options_menu_skin.uTextureID_SoundLevels[6] = assets->GetImage_ColorKey("convol70", 0x7FF);
    options_menu_skin.uTextureID_SoundLevels[7] = assets->GetImage_ColorKey("convol80", 0x7FF);
    options_menu_skin.uTextureID_SoundLevels[8] = assets->GetImage_ColorKey("convol90", 0x7FF);
    options_menu_skin.uTextureID_SoundLevels[9] = assets->GetImage_ColorKey("convol00", 0x7FF);
    options_menu_skin.uTextureID_FlipOnExit = assets->GetImage_ColorKey("option04", 0x7FF);
    options_menu_skin.uTextureID_AlwaysRun = assets->GetImage_ColorKey("option03", 0x7FF);
    options_menu_skin.uTextureID_ShowDamage = assets->GetImage_ColorKey("option02", 0x7FF);
    options_menu_skin.uTextureID_WalkSound = assets->GetImage_ColorKey("option01", 0x7FF);

    CreateButton(22, 270, options_menu_skin.uTextureID_TurnSpeed[2]->GetWidth(),
                 options_menu_skin.uTextureID_TurnSpeed[2]->GetHeight(), 1, 0,
                 UIMSG_SetTurnSpeed, 0x80);
    CreateButton(93, 270, options_menu_skin.uTextureID_TurnSpeed[1]->GetWidth(),
                 options_menu_skin.uTextureID_TurnSpeed[1]->GetHeight(), 1, 0,
                 UIMSG_SetTurnSpeed, 0x40u);
    CreateButton(164, 270,
                 options_menu_skin.uTextureID_TurnSpeed[0]->GetWidth(),
                 options_menu_skin.uTextureID_TurnSpeed[0]->GetHeight(), 1, 0,
                 UIMSG_SetTurnSpeed, 0);

    CreateButton(20, 303, options_menu_skin.uTextureID_WalkSound->GetWidth(),
                 options_menu_skin.uTextureID_WalkSound->GetHeight(), 1, 0,
                 UIMSG_ToggleWalkSound, 0);
    CreateButton(128, 303, options_menu_skin.uTextureID_ShowDamage->GetWidth(),
                 options_menu_skin.uTextureID_ShowDamage->GetHeight(), 1, 0,
                 UIMSG_ToggleShowDamage, 0);
    CreateButton(20, 325, options_menu_skin.uTextureID_AlwaysRun->GetWidth(),
                 options_menu_skin.uTextureID_AlwaysRun->GetHeight(), 1, 0,
                 UIMSG_ToggleAlwaysRun, 0);
    CreateButton(128, 325, options_menu_skin.uTextureID_FlipOnExit->GetWidth(),
                 options_menu_skin.uTextureID_FlipOnExit->GetHeight(), 1, 0,
                 UIMSG_ToggleFlipOnExit, 0);

    pBtn_SliderLeft = CreateButton(
        243, 162, 16, 16, 1, 0, UIMSG_ChangeSoundVolume, 4, GameKey::None, "",
        {{options_menu_skin.uTextureID_ArrowLeft}});
    pBtn_SliderRight = CreateButton(
        435, 162, 16, 16, 1, 0, UIMSG_ChangeSoundVolume, 5, GameKey::None, "",
        {{options_menu_skin.uTextureID_ArrowRight}});
    CreateButton(263, 162, 172, 17, 1, 0, UIMSG_ChangeSoundVolume, 0);

    pBtn_SliderLeft = CreateButton(
        243, 216, 16, 16, 1, 0, UIMSG_ChangeMusicVolume, 4, GameKey::None, "",
        {{options_menu_skin.uTextureID_ArrowLeft}});
    pBtn_SliderRight = CreateButton(
        435, 216, 16, 16, 1, 0, UIMSG_ChangeMusicVolume, 5, GameKey::None, "",
        {{options_menu_skin.uTextureID_ArrowRight}});
    CreateButton(263, 216, 172, 17, 1, 0, UIMSG_ChangeMusicVolume, 0);

    pBtn_SliderLeft = CreateButton(
        243, 270, 16, 16, 1, 0, UIMSG_ChangeVoiceVolume, 4, GameKey::None, "",
        {{options_menu_skin.uTextureID_ArrowLeft}});
    pBtn_SliderRight = CreateButton(
        435, 270, 16, 16, 1, 0, UIMSG_ChangeVoiceVolume, 5, GameKey::None, "",
        {{options_menu_skin.uTextureID_ArrowRight}});
    CreateButton(263, 270, 172, 17, 1, 0, UIMSG_ChangeVoiceVolume, 0);

    CreateButton(241, 302, 214, 40, 1, 0, UIMSG_Escape, 0, GameKey::None, localization->GetString(619));  // "Return to Game"
    CreateButton(19, 140, 214, 40, 1, 0, UIMSG_OpenKeyMappingOptions, 0, GameKey::K);
    CreateButton(19, 194, 214, 40, 1, 0, UIMSG_OpenVideoOptions, 0, GameKey::V);
}

void GUIWindow_GameOptions::Update() {
    render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, game_ui_menu_options);
    render->DrawTextureAlphaNew(8 / 640.0f, 132 / 480.0f,
                                options_menu_skin.uTextureID_Background);

    switch (engine->config->turn_speed) {
        case 64:
            render->DrawTextureAlphaNew(
                BtnTurnCoord[1] / 640.0f, 270 / 480.0f,
                options_menu_skin.uTextureID_TurnSpeed[1]);
            break;
        case 128:
            render->DrawTextureAlphaNew(
                BtnTurnCoord[2] / 640.0f, 270 / 480.0f,
                options_menu_skin.uTextureID_TurnSpeed[2]);
            break;
        default:
            render->DrawTextureAlphaNew(
                BtnTurnCoord[0] / 640.0f, 270 / 480.0f,
                options_menu_skin.uTextureID_TurnSpeed[0]);
            break;
    }

    if (!engine->config->no_walk_sound) {
        render->DrawTextureAlphaNew(
            20 / 640.0f, 303 / 480.0f,
            options_menu_skin.uTextureID_WalkSound);
    }
    if (engine->config->show_damage) {
        render->DrawTextureAlphaNew(
            128 / 640.0f, 303 / 480.0f,
            options_menu_skin.uTextureID_ShowDamage);
    }
    if (engine->config->flip_on_exit) {
        render->DrawTextureAlphaNew(
            128 / 640.0f, 325 / 480.0f,
            options_menu_skin.uTextureID_FlipOnExit);
    }
    if (engine->config->always_run) {
        render->DrawTextureAlphaNew(
            20 / 640.0f, 325 / 480.0f,
            options_menu_skin.uTextureID_AlwaysRun);
    }

    render->DrawTextureAlphaNew(
        (265 + 17 * engine->config->sound_level) / 640.0f, 162 / 480.0f,
        options_menu_skin.uTextureID_SoundLevels[engine->config->sound_level]);
    render->DrawTextureAlphaNew(
        (265 + 17 * engine->config->music_level) / 640.0f, 216 / 480.0f,
        options_menu_skin.uTextureID_SoundLevels[engine->config->music_level]);
    render->DrawTextureAlphaNew(
        (265 + 17 * engine->config->voice_level) / 640.0f, 270 / 480.0f,
        options_menu_skin.uTextureID_SoundLevels[engine->config->voice_level]);
}

void GameUI_OnPlayerPortraitLeftClick(unsigned int uPlayerID) {
    Player *player = &pParty->pPlayers[uPlayerID - 1];
    if (pParty->pPickedItem.uItemID) {
        if (int slot = player->AddItem(-1, pParty->pPickedItem.uItemID)) {
            memcpy(&player->pInventoryItemList[slot - 1], &pParty->pPickedItem,
                   0x24u);
            viewparams->bRedrawGameUI = true;
            mouse->RemoveHoldingItem();
            return;
        }

        if (!player->CanAct()) {
            player = pPlayers[uActiveCharacter];
        }
        if (player->CanAct() || !pPlayers[uActiveCharacter]->CanAct()) {
            player->PlaySound(SPEECH_NoRoom, 0);
        }
    }

    if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
        viewparams->bRedrawGameUI = true;
        if (uActiveCharacter != uPlayerID) {
            if (pPlayers[uPlayerID]->uTimeToRecovery || !pPlayers[uPlayerID]->CanAct()) {
                return;
            }

            uActiveCharacter = uPlayerID;
            return;
        }
        pGUIWindow_CurrentMenu = new GUIWindow_CharacterRecord(
            uActiveCharacter,
            CURRENT_SCREEN::SCREEN_CHARACTERS);  // CharacterUI_Initialize(SCREEN_CHARACTERS);
        return;
    }
    if (current_screen_type == CURRENT_SCREEN::SCREEN_SPELL_BOOK) return;
    if (current_screen_type == CURRENT_SCREEN::SCREEN_CHEST) {
        viewparams->bRedrawGameUI = true;
        if (uActiveCharacter == uPlayerID) {
            current_character_screen_window = WINDOW_CharacterWindow_Inventory;
            current_screen_type = CURRENT_SCREEN::SCREEN_CHEST_INVENTORY;
            uActiveCharacter = uPlayerID;
            return;
        }
        if (pPlayers[uPlayerID]->uTimeToRecovery) {
            return;
        }
        uActiveCharacter = uPlayerID;
        return;
    }
    if (current_screen_type != CURRENT_SCREEN::SCREEN_HOUSE) {
        if (current_screen_type == CURRENT_SCREEN::SCREEN_E) {
            uActiveCharacter = uPlayerID;
            return;
        }
        if (current_screen_type != CURRENT_SCREEN::SCREEN_CHEST_INVENTORY) {
            viewparams->bRedrawGameUI = true;
            uActiveCharacter = uPlayerID;
            if (current_character_screen_window ==
                WINDOW_CharacterWindow_Awards) {
                FillAwardsData();
            }
            return;
        }
        viewparams->bRedrawGameUI = true;
        if (uActiveCharacter == uPlayerID) {
            current_character_screen_window = WINDOW_CharacterWindow_Inventory;
            current_screen_type = CURRENT_SCREEN::SCREEN_CHEST_INVENTORY;
            uActiveCharacter = uPlayerID;
            return;
        }
        if (pPlayers[uPlayerID]->uTimeToRecovery) {
            return;
        }
        uActiveCharacter = uPlayerID;
        return;
    }
    if (window_SpeakInHouse->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_IN_PROGRESS) {
        return;
    }
    viewparams->bRedrawGameUI = true;
    if (uActiveCharacter != uPlayerID) {
        uActiveCharacter = uPlayerID;
        return;
    }
    if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_STANDARD ||
        dialog_menu_id == HOUSE_DIALOGUE_SHOP_6) {
        __debugbreak();  // fix indexing
        current_character_screen_window = WINDOW_CharacterWindow_Inventory;
        pGUIWindow_CurrentMenu = new GUIWindow_CharacterRecord(
            uActiveCharacter, CURRENT_SCREEN::SCREEN_E);  // CharacterUI_Initialize(SCREEN_E);
        return;
    }
}

void GameUI_DrawNPCPopup(void *_this) {  // PopupWindowForBenefitAndJoinText
    int v1;                  // edi@2
    NPCData *pNPC;           // eax@16
    const char *pText;       // eax@18
    GUIWindow popup_window;  // [sp+Ch] [bp-60h]@23
    int a2;                  // [sp+60h] [bp-Ch]@16
    const char *lpsz;        // [sp+68h] [bp-4h]@6

    char buf[4096];
    if (bNoNPCHiring != 1) {
        v1 = 0;
        /*do
        {
        if ( v3->pName )
        tmp_str[v1++] = v2;
        ++v3;
        ++v2;
        }
        while ( (signed int)v3 < (signed int)&pParty->pPickedItem );*/
        for (int i = 0; i < 2; ++i) {
            if (pParty->pHirelings[i].pName) buf[v1++] = i;
        }
        lpsz = 0;
        if ((signed int)pNPCStats->uNumNewNPCs > 0) {
            /*v4 = pNPCStats->pNewNPCData;
            do
            {
            if ( v4->uFlags & 0x80
            && (!pParty->pHirelings[0].pName || strcmp(v4->pName,
            pParty->pHirelings[0].pName))
            && (!pParty->pHirelings[1].pName || strcmp(v4->pName,
            pParty->pHirelings[1].pName)) ) tmp_str[v1++] = (char)lpsz + 2;
            ++lpsz;
            ++v4;
            }
            while ( (signed int)lpsz < (signed int)pNPCStats->uNumNewNPCs );*/
            for (uint i = 0; i < pNPCStats->uNumNewNPCs; ++i) {
                if (pNPCStats->pNewNPCData[i].Hired()) {
                    if (!pParty->pHirelings[0].pName ||
                        strcmp((char *)pNPCStats->pNewNPCData[i].pName,
                               (char *)pParty->pHirelings[0].pName)) {
                        if (!pParty->pHirelings[1].pName ||
                            strcmp((char *)pNPCStats->pNewNPCData[i].pName,
                                   (char *)pParty->pHirelings[1].pName))
                            buf[v1++] = i + 2;
                    }
                }
            }
        }
        if ((int64_t)((char *)_this + pParty->hirelingScrollPosition) < v1) {
            sDialogue_SpeakingActorNPC_ID =
                -1 - pParty->hirelingScrollPosition - (int64_t)_this;
            pNPC = GetNewNPCData(sDialogue_SpeakingActorNPC_ID, &a2);
            if (pNPC) {
                if (a2 == 57)
                    pText = pNPCTopics[512].pText;  // Baby dragon
                else
                    pText =
                        (const char *)pNPCStats->pProfessions[pNPC->uProfession]
                            .pBenefits;
                lpsz = pText;
                if (!pText) {
                    lpsz =
                        (const char *)pNPCStats->pProfessions[pNPC->uProfession]
                            .pJoinText;
                    if (!lpsz) lpsz = "";
                }
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

                auto tex_name = StringPrintf("NPC%03d", pNPC->uPortraitID);
                render->DrawTextureAlphaNew(
                    (popup_window.uFrameX + 22) / 640.0f,
                    (popup_window.uFrameY + 36) / 480.0f,
                    assets->GetImage_ColorKey(tex_name, 0x7FF));

                String title;
                if (pNPC->uProfession) {
                    title = localization->FormatString(
                        429, pNPC->pName,
                        localization->GetNpcProfessionName(pNPC->uProfession));
                } else {
                    title = pNPC->pName;
                }
                popup_window.DrawTitleText(
                    pFontArrus, 0, 12, Color16(0xFFu, 0xFFu, 0x9Bu), title, 3);
                popup_window.uFrameWidth -= 24;
                popup_window.uFrameZ =
                    popup_window.uFrameX + popup_window.uFrameWidth - 1;
                popup_window.DrawText(
                    pFontArrus, 100, 36, 0,
                    BuildDialogueString((char *)lpsz, uActiveCharacter - 1, 0,
                                        0, 0));
            }
        }
    }
}

String GameUI_GetMinimapHintText() {
    double v3;            // st7@1
    int v7;               // eax@4
    const char *v14;      // eax@8
    unsigned int pMapID;  // eax@14
    int global_coord_X;   // [sp+10h] [bp-1Ch]@1
    int global_coord_Y;   // [sp+14h] [bp-18h]@1
    unsigned int pY;      // [sp+1Ch] [bp-10h]@1
    unsigned int pX;      // [sp+28h] [bp-4h]@1

    String result;
    mouse->GetClickPos(&pX, &pY);
    v3 = 1.0 / (float)((signed int)viewparams->uMinimapZoom * 0.000015258789);
    global_coord_X =
        (__int64)((double)(pX - 557) * v3 + (double)pParty->vPosition.x);
    global_coord_Y =
        (__int64)((double)pParty->vPosition.y - (double)(pY - 74) * v3);
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
                            v14 = GetEventHintString(face.sCogTriggeredID);
                            if (v14) {
                                if (_stricmp(v14, "")) {
                                    result = v14;
                                }
                            }
                        }
                    }
                }
                return result;
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
        if (player->expression == CHARACTER_EXPRESSION_21)
            v15 = pPlayerFrameTable->GetFrameBy_y(
                &player->_expression21_frameset,
                &player->_expression21_animtime, pMiscTimer->uTimeElapsed);
        else
            v15 = pPlayerFrameTable->GetFrameBy_x(uFramesetID,
                                                  pMiscTimer->Time());
        player->field_1AA2 = v15->uTextureID - 1;
        v13 = game_ui_player_faces[window->par1C][v15->uTextureID - 1];
    }

    render->DrawTextureAlphaNew((window->uFrameX + 24) / 640.0f,
                                (window->uFrameY + 24) / 480.0f, v13);

    auto str =
        StringPrintf("\f%05d", ui_character_header_text_color) +
        localization->FormatString(
            429, player->pName,
            localization->GetClassName(player->classType))  // "%s the %s"
        + "\f00000\n" +
        StringPrintf("%s : \f%05u%d\f00000 / %d\n",
                     localization->GetString(108),  // "Hit Points"
                     UI_GetHealthManaAndOtherQualitiesStringColor(
                         player->sHealth, player->GetMaxHealth()),
                     player->sHealth, player->GetMaxHealth()) +
        StringPrintf("%s : \f%05u%d\f00000 / %d\n",
                     localization->GetString(212),  // "Spell Points"
                     UI_GetHealthManaAndOtherQualitiesStringColor(
                         player->sMana, player->GetMaxMana()),
                     player->sMana, player->GetMaxMana()) +
        StringPrintf("%s: \f%05d%s\f00000\n",
                     localization->GetString(47),  // Condition
                     GetConditionDrawColor(player->GetMajorConditionIdx()),
                     localization->GetCharacterConditionName(
                         player->GetMajorConditionIdx()));

    if (player->uQuickSpell)
        v29 = pSpellStats->pInfos[player->uQuickSpell].pShortName;
    else
        v29 = localization->GetString(153);

    str += StringPrintf("%s: %s", localization->GetString(172),
                        v29);  // "Quick Spell"

    window->DrawText(pFontArrus, 120, 22, 0, str, 0, 0, 0);

    uFramesetIDa = 0;
    for (uint i = 0; i < 24; ++i) {
        SpellBuff *buff = &player->pPlayerBuffs[i];
        if (buff->Active()) {
            v36 = uFramesetIDa++ * pFontComic->GetHeight() + 134;
            window->DrawText(pFontComic, 52, v36,
                             ui_game_character_record_playerbuff_colors[i],
                             localization->GetSpellName(20 + i), 0, 0, 0);
            DrawBuff_remaining_time_string(
                v36, window, buff->expire_time - pParty->GetPlayingTime(),
                pFontComic);
        }
    }

    auto active_spells = localization->FormatString(
        450,  // Active Spells: %s
        uFramesetIDa == 0 ? localization->GetString(153) : "");  // "None"
    window->DrawText(pFontArrus, 14, 114, 0, active_spells, 0, 0, 0);
}

//----- (0041AD6E) --------------------------------------------------------
void GameUI_DrawRightPanelItems() {
    if (GameUI_RightPanel_BookFlashTimer > pParty->GetPlayingTime())
        GameUI_RightPanel_BookFlashTimer = GameTime(0);

    static bool _50697C_book_flasher;

    if (pParty->GetPlayingTime() - GameUI_RightPanel_BookFlashTimer > 128) {
        GameUI_RightPanel_BookFlashTimer = pParty->GetPlayingTime();
        _50697C_book_flasher = !_50697C_book_flasher;
    }

    if (_50697C_book_flasher && current_screen_type != CURRENT_SCREEN::SCREEN_REST) {
        if (bFlashQuestBook)
            render->DrawTextureAlphaNew(493 / 640.0f, 355 / 480.0f,
                                        game_ui_tome_quests);
        if (bFlashAutonotesBook)
            render->DrawTextureAlphaNew(527 / 640.0f, 353 / 480.0f,
                                        game_ui_tome_autonotes);
        if (bFlashHistoryBook)
            render->DrawTextureAlphaNew(600 / 640.0f, 361 / 480.0f,
                                        game_ui_tome_storyline);
    }
}

//----- (0041AEBB) --------------------------------------------------------
void GameUI_DrawFoodAndGold() {
    int text_y;  // esi@2

    if (uGameState != GAME_STATE_FINAL_WINDOW) {
        text_y = _44100D_should_alter_right_panel() != 0 ? 381 : 322;

        pPrimaryWindow->DrawText(
            pFontSmallnum, 0, text_y, uGameUIFontMain,
            StringPrintf("\r087%d", pParty->GetFood()), 0, 0,
            uGameUIFontShadow);
        pPrimaryWindow->DrawText(
            pFontSmallnum, 0, text_y, uGameUIFontMain,
            StringPrintf("\r028%d", pParty->GetGold()), 0, 0,
            uGameUIFontShadow);
    }
}

//----- (0041B0C9) --------------------------------------------------------
void GameUI_DrawLifeManaBars() {
    double v3;  // st7@3
    double v7;  // st7@25

    for (uint i = 0; i < 4; ++i) {
        if (pParty->pPlayers[i].sHealth > 0) {
            int v17 = 0;
            if (i == 2 || i == 3) v17 = 2;
            v3 = (double)pParty->pPlayers[i].sHealth /
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
                    (signed __int64)((1.0 - v3) * pTextureHealth->GetHeight()) +
                        402,
                    v17 + pHealthBarPos[i] + pTextureHealth->GetWidth(),
                    pTextureHealth->GetHeight() + 402);
                render->DrawTextureAlphaNew((v17 + pHealthBarPos[i]) / 640.0f,
                                            402 / 480.0f, pTextureHealth);
                render->ResetUIClipRect();
            }
        }
        if (pParty->pPlayers[i].sMana > 0) {
            v7 = pParty->pPlayers[i].sMana /
                 (double)pParty->pPlayers[i].GetMaxMana();
            if (v7 > 1.0) v7 = 1.0;
            int v17 = 0;
            if (i == 2) v17 = 1;
            render->SetUIClipRect(
                v17 + pManaBarPos[i],
                (signed __int64)((1.0 - v7) * game_ui_bar_blue->GetHeight()) +
                    402,
                v17 + pManaBarPos[i] + game_ui_bar_blue->GetWidth(),
                game_ui_bar_blue->GetHeight() + 402);
            render->DrawTextureAlphaNew((v17 + pManaBarPos[i]) / 640.0f,
                                        402 / 480.0f, game_ui_bar_blue);
            render->ResetUIClipRect();
        }
    }
}

//----- (0041B3B6) --------------------------------------------------------
void GameUI_DrawRightPanel() {
    render->DrawTextureAlphaNew(pViewport->uViewportBR_X / 640.0f, 0,
                                game_ui_right_panel_frame);
}

//----- (0041B3E2) --------------------------------------------------------
void GameUI_DrawRightPanelFrames() {
    render->DrawTextureNew(0, 0, game_ui_topframe);
    render->DrawTextureNew(0, 8 / 480.0f, game_ui_leftframe);
    render->DrawTextureNew(468 / 640.0f, 0, game_ui_rightframe);
    render->DrawTextureNew(0, 352 / 480.0f, game_ui_bottomframe);
    GameUI_DrawFoodAndGold();
    GameUI_DrawRightPanelItems();

    // render->EndScene();
    // render->Present();
}

void GameUI_WritePointedObjectStatusString() {
    // GUIWindow *pWindow;                // edi@7
    // GUIButton *pButton;                // ecx@11
    int requiredSkillpoints;           // ecx@19
    enum UIMessageType pMessageType1;  // esi@24
    int invmatrixindex;                // eax@41
    ItemGen *pItemGen;                 // ecx@44
    // int v16;                           // ecx@46
    Vis_PIDAndDepth pickedObject;        // eax@55
    signed int v18b;
    signed int pickedObjectID = 0;     // ecx@63
    BLVFace *pFace;                    // eax@69
    const char *pText;                 // ecx@79
    enum UIMessageType pMessageType2;  // esi@110
    enum UIMessageType pMessageType3;  // edx@117
    unsigned int pX;                   // [sp+D4h] [bp-Ch]@1
    unsigned int pY;                   // [sp+D8h] [bp-8h]@1

    // int testing;

    mouse->uPointingObjectID = 0;
    mouse->GetClickPos(&pX, &pY);
    if (pX < 0 || pX > window->GetWidth() - 1 || pY < 0 ||
        pY > window->GetHeight() - 1)
        return;

    if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
        if (pX <= (window->GetWidth() - 1) * 0.73125 &&
            pY <= (window->GetHeight() - 1) * 0.73125) {
            if (!pViewport->Contains(pX, pY)) {
                if (uLastPointedObjectID != 0) {
                    game_ui_status_bar_string.clear();
                    bForceDrawFooter = 1;
                }
                uLastPointedObjectID = 0;
                return;
            }

            auto vis = EngineIoc::ResolveVis();

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
                    pParty->pPickedItem.uItemID) {
                    GameUI_StatusBar_Set(pSpriteObjects[pickedObjectID]
                                             .containing_item.GetDisplayName());
                } else {
                    GameUI_StatusBar_Set(localization->FormatString(
                        470, pSpriteObjects[pickedObjectID]
                        .containing_item.GetDisplayName()
                        .c_str()));  // Get %s
                }  // intentional fallthrough
            } else if (PID_TYPE(pickedObject.object_pid) == OBJECT_Decoration) {
                if (!pLevelDecorations[pickedObjectID].uEventID) {
                    if (pLevelDecorations[pickedObjectID].IsInteractive())
                        pText = pNPCTopics[stru_5E4C90_MapPersistVars._decor_events
                                           [pLevelDecorations[pickedObjectID]._idx_in_stru123 -
                                            75] + 380].pTopic;  // неверно для костра
                    else
                        pText = pDecorationList->GetDecoration(pLevelDecorations[pickedObjectID].uDecorationDescID)->field_20;
                    GameUI_StatusBar_Set(pText);
                } else {
                    char *hintString = GetEventHintString(pLevelDecorations[pickedObjectID].uEventID);
                    if (hintString[0] != '\0') {
                        GameUI_StatusBar_Set(hintString);
                    }
                }  // intentional fallthrough
            } else if (PID_TYPE(pickedObject.object_pid) == OBJECT_BModel) {
                if (pickedObject.depth < 0x200u) {
                    char *newString = nullptr;
                    if (uCurrentlyLoadedLevelType != LEVEL_Indoor) {
                        v18b = PID_ID(pickedObject.object_pid) >> 6;
                        short triggeredId = pOutdoor->pBModels[v18b].pFaces[pickedObjectID & 0x3F].sCogTriggeredID;
                        if (triggeredId != 0) {
                            newString = GetEventHintString(
                                pOutdoor->pBModels[v18b]
                                    .pFaces[pickedObjectID & 0x3F]
                                    .sCogTriggeredID);
                        }
                    } else {
                        pFace = &pIndoor->pFaces[pickedObjectID];
                        if (pFace->uAttributes & FACE_INDICATE) {
                            unsigned short eventId =
                                pIndoor->pFaceExtras[pFace->uFaceExtraID]
                                    .uEventID;
                            if (eventId != 0) {
                                newString = GetEventHintString(
                                    pIndoor->pFaceExtras[pFace->uFaceExtraID]
                                        .uEventID);
                            }
                        }
                    }
                    if (newString) {
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
                if (pickedObject.depth >= 0x200u) {
                    mouse->uPointingObjectID = 0;
                    if (uLastPointedObjectID != 0) {
                        game_ui_status_bar_string.clear();
                        bForceDrawFooter = 1;
                    }
                    uLastPointedObjectID = 0;
                    return;
                }
                if (pActors[pickedObjectID].dword_000334_unique_name)
                    pText = pMonsterStats
                                ->pPlaceStrings[pActors[pickedObjectID]
                                                    .dword_000334_unique_name];
                else
                    pText =
                        pMonsterStats
                            ->pInfos[pActors[pickedObjectID].pMonsterInfo.uID]
                            .pName;
                GameUI_StatusBar_Set(pText);  // intentional fallthrough
            }
            if (mouse->uPointingObjectID == 0 && uLastPointedObjectID != 0) {
                game_ui_status_bar_string.clear();
                bForceDrawFooter = 1;
            }
            uLastPointedObjectID = mouse->uPointingObjectID;
            return;
        }
    } else if (current_screen_type == CURRENT_SCREEN::SCREEN_CHEST) {
        if (pX <= (window->GetWidth() - 1) * 0.73125 &&
            pY <= (window->GetHeight() - 1) * 0.73125) {  // if in chest area
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
                    // pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex(invmatrixindex);
                    pItemGen =
                        pPlayers[uActiveCharacter]->GetItemAtInventoryIndex(
                            invmatrixindex);  // (ItemGen
                                              // *)&pPlayers[uActiveCharacter]->pInventoryItemList[testing
                                              // - 1];

                    if (!pItemGen == NULL) pickedObjectID = pItemGen->uItemID;
                    // if (!pItemID)
                    // return;
                    // item =
                    // &pPlayers[uActiveCharacter]->pInventoryItemList[pItemID -
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
                                    (UIMessageType)pButton->field_1C;
                                if (pMessageType1)
                                    pMessageQueue_50CBD0->AddGUIMessage(
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
                                        (UIMessageType)pButton->field_1C;
                                    if (pMessageType2 != 0)
                                        pMessageQueue_50CBD0->AddGUIMessage(
                                            pMessageType2, pButton->msg_param,
                                            0);
                                    GameUI_StatusBar_Set(
                                        pButton->sLabel);  // for character name
                                    uLastPointedObjectID = 1;
                                    return;
                                }
                            }
                            break;
                        case 3:  // click on skill
                            if (pButton->Contains(pX, pY)) {
                                requiredSkillpoints =
                                    (pPlayers[uActiveCharacter]
                                         ->pActiveSkills[pButton->msg_param] &
                                     0x3F) +
                                    1;

                                String str;
                                if (pPlayers[uActiveCharacter]->uSkillPoints <
                                    requiredSkillpoints)
                                    str = localization->FormatString(
                                        469,
                                        requiredSkillpoints -
                                            pPlayers[uActiveCharacter]
                                                ->uSkillPoints);  // "You need
                                                                  // %d more
                                                                  // Skill
                                                                  // Points to
                                                                  // advance
                                                                  // here"
                                else
                                    str = localization->FormatString(
                                        468,
                                        requiredSkillpoints);  // "Clicking here
                                                               // will spend %d
                                                               // Skill Points"

                                GameUI_StatusBar_Set(str);
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
        if (dialog_menu_id != HOUSE_DIALOGUE_SHOP_BUY_STANDARD
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
                            pMessageType3 = (UIMessageType)pButton->field_1C;
                            if (pMessageType3 == 0) {  // For books
                                GameUI_StatusBar_Set(pButton->sLabel);
                            } else {
                                pMessageQueue_50CBD0->AddGUIMessage(
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
                                    (UIMessageType)pButton->field_1C;
                                if (pMessageType2 != 0)
                                    pMessageQueue_50CBD0->AddGUIMessage(
                                        pMessageType2, pButton->msg_param, 0);
                                GameUI_StatusBar_Set(
                                    pButton->sLabel);  // for character name
                                uLastPointedObjectID = 1;
                                return;
                            }
                        }
                        break;
                    case 3:  // is this one needed?
                             /*                if (pX >= pButton->uX && pX <=
                             pButton->uZ
                             && pY >= pButton->uY && pY <= pButton->uW)
                             {
                             requiredSkillpoints =
                             (pPlayers[uActiveCharacter]->pActiveSkills[pButton->msg_param]
                             & 0x3F) + 1;
     
                             String str;
                             if (pPlayers[uActiveCharacter]->uSkillPoints <
                             requiredSkillpoints)      str =
                             localization->FormatString(469, requiredSkillpoints -
                             pPlayers[uActiveCharacter]->uSkillPoints);// "You need
                             %d more Skill Points to advance here"      else      str =
                             localization->FormatString(468, requiredSkillpoints);//
                             "Clicking here will spend %d Skill Points"
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
    if (uActiveCharacter)
        render->DrawTextureAlphaNew(
            (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing
                 [uActiveCharacter - 1] -
             9) /
                640.0f,
            380 / 480.0f, game_ui_player_selection_frame);
}

//----- (0044162D) --------------------------------------------------------
void GameUI_DrawPartySpells() {
    unsigned int v0;       // ebp@1
    Image *spell_texture;  // [sp-4h] [bp-1Ch]@12
    // Texture_MM7 *v9; // [sp-4h] [bp-1Ch]@21

    v0 = OS_GetTime() / 20;
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
            render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, spell_texture);
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
            render->DrawTextureAlphaNew(396 / 640.0f, 8 / 480.0f,
                                        spell_texture);
        }
    }

    for (uint i = 0; i < 4; ++i) {
        if (pParty->pPlayers[i].pPlayerBuffs[PLAYER_BUFF_HAMMERHANDS].Active())
            render->DrawTextureAlphaNew(
                (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72) /
                    640.0f,
                427 / 480.0f, game_ui_playerbuff_hammerhands);
        if (pParty->pPlayers[i].pPlayerBuffs[PLAYER_BUFF_BLESS].Active())
            render->DrawTextureAlphaNew(
                (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72) /
                    640.0f,
                393 / 480.0f, game_ui_playerbuff_bless);
        if (pParty->pPlayers[i].pPlayerBuffs[PLAYER_BUFF_PRESERVATION].Active())
            render->DrawTextureAlphaNew(
                (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72) /
                    640.0f,
                410 / 480.0f, game_ui_playerbuff_preservation);
        if (pParty->pPlayers[i]
                .pPlayerBuffs[PLAYER_BUFF_PAIN_REFLECTION]
                .Active())
            render->DrawTextureAlphaNew(
                (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72) /
                    640.0f,
                444 / 480.0f, game_ui_playerbuff_pain_reflection);
    }
}

//----- (004921C1) --------------------------------------------------------
void GameUI_DrawPortraits(unsigned int _this) {
    unsigned int face_expression_ID;  // eax@17
    PlayerFrame *pFrame;              // eax@21
    Image *pPortrait;                 // [sp-4h] [bp-1Ch]@27

    if (_A750D8_player_speech_timer) {
        _A750D8_player_speech_timer -= (signed int)pMiscTimer->uTimeElapsed;
        if (_A750D8_player_speech_timer <= 0) {
            if (pPlayers[uSpeakingCharacter]->CanAct())
                pPlayers[uSpeakingCharacter]->PlaySound(PlayerSpeechID, 0);
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
                    388 / 480.0f, pPortrait);
            else
                render->DrawTextureAlphaNew(
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
        if (pPlayer->IsDead()) {
            pPortrait = game_ui_player_face_dead;
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active())
                render->DrawTextureGrayShade(
                    pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] /
                        640.0f,
                    388 / 480.0f, pPortrait);
            else
                render->DrawTextureAlphaNew(
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
        for (uint j = 0; j < pPlayerFrameTable->uNumFrames; ++j)
            if (pPlayerFrameTable->pFrames[j].expression ==
                pPlayer->expression) {
                face_expression_ID = j;
                break;
            }
        if (face_expression_ID == 0) face_expression_ID = 1;
        if (pPlayer->expression == CHARACTER_EXPRESSION_21)
            pFrame = pPlayerFrameTable->GetFrameBy_y(
                &pPlayer->_expression21_frameset,
                &pPlayer->_expression21_animtime, pMiscTimer->uTimeElapsed);
        else
            pFrame = pPlayerFrameTable->GetFrameBy_x(
                face_expression_ID, pPlayer->uExpressionTimePassed);
        if (pPlayer->field_1AA2 != pFrame->uTextureID - 1 || _this) {
            pPlayer->field_1AA2 = pFrame->uTextureID - 1;
            pPortrait = game_ui_player_faces
                [i]
                [pPlayer
                     ->field_1AA2];  // pFace = (Texture_MM7
                                     // *)game_ui_player_faces[i][pFrame->uTextureID];
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active())
                render->DrawTextureGrayShade(
                    pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] /
                        640.0f,
                    388 / 480.0f, pPortrait);
            else
                render->DrawTextureAlphaNew(
                    (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] +
                     1) /
                        640.0f,
                    388 / 480.0f, pPortrait);
            if (pPlayer->pPlayerBuffs[PLAYER_BUFF_BLESS].Active() |
                pPlayer->pPlayerBuffs[PLAYER_BUFF_HASTE].Active() |
                pPlayer->pPlayerBuffs[PLAYER_BUFF_HEROISM].Active() |
                pPlayer->pPlayerBuffs[PLAYER_BUFF_SHIELD].Active() |
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

                        render->DrawTextureAlphaNew(
                            (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing
                                 [PID_ID(pTurnEngine->pQueue[i].uPackedID)] -
                             4) /
                                640.0f,
                            385 / 480.0f, alert_texture);
                    }
                }
            }
        }
    } else {
        for (uint i = 0; i < 4; ++i) {
            if (pParty->pPlayers[i].CanAct() &&
                !pParty->pPlayers[i].uTimeToRecovery) {
                auto alert_texture = game_ui_player_alert_green;
                if (pParty->GetRedAlert())
                    alert_texture = game_ui_player_alert_red;
                else if (pParty->GetYellowAlert())
                    alert_texture = game_ui_player_alert_yellow;

                render->DrawTextureAlphaNew(
                    (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] -
                     4) /
                        640.0f,
                    385 / 480.0f, alert_texture);
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
    // unsigned __int16 *v28; // ecx@37
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

    bool bWizardEyeActive = pParty->WizardEyeActive();
    int uWizardEyeSkillLevel = pParty->WizardEyeSkillLevel();
    if (CheckHiredNPCSpeciality(Cartographer)) {
        bWizardEyeActive = true;
        uWizardEyeSkillLevel = 2;
    }

    if (engine->config->debug_wizard_eye) {
        bWizardEyeActive = true;
        uWizardEyeSkillLevel = 3;
    }

    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
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

        static unsigned __int16 pOdmMinimap[117][137];
        assert(sizeof(pOdmMinimap) == 137 * 117 * sizeof(short));

        static Texture *minimaptemp;
        if (!minimaptemp) {
            minimaptemp = render->CreateTexture_Blank(uWidth, uHeight, IMAGE_FORMAT_A8R8G8B8);
        }

        xpixoffset16 = floorf(startx * 65536.0 + 0.5f);     // LODWORD(v24);
        ypixoffset16 = floorf(starty * 65536.0 + 0.5f);  // LODWORD(v25);
        ypix = ypixoffset16 >> 16;
        xpix = xpixoffset16 >> 16;
        // v28 = &render->pTargetSurface[uX + uY * lPitch];

        if (/*pMapLod0 && */ bRedrawOdmMinimap) {
            assert(uWidth == 137 && uHeight == 117);

            ushort MapImgWidth = viewparams->location_minimap->GetWidth();
            auto pMapLod0Line =
                (unsigned __int32 *)viewparams->location_minimap->GetPixels(
                    IMAGE_FORMAT_A8R8G8B8);
           // Image *minimaptemp = Image::Create(uWidth, uHeight, IMAGE_FORMAT_A8R8G8B8);
           auto minitempix = (unsigned __int32 *)minimaptemp->GetPixels(IMAGE_FORMAT_A8R8G8B8);

            for (int y = 0; y < uHeight; ++y) {
                for (int x = 0; x < uWidth; ++x) {
                   minitempix[x + y*uWidth] = pMapLod0Line[xpix + ypix * MapImgWidth];
                   xpix = (xpixoffset16 + x * map_scale) >> 16;
                }
                ypixoffset16 += map_scale;
                ypix = ypixoffset16 >> 16;
            }
            // draw image
            render->Update_Texture(minimaptemp);
            render->DrawTextureAlphaNew(uX / 640., uY / 480., minimaptemp);
            // minimaptemp->Release();
        }
    } else if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        render->FillRectFast(uX, uY, uZ - uX, uHeight, 0xF);

        for (uint i = 0; i < (uint)pIndoor->pMapOutlines->uNumOutlines; ++i) {
            BLVMapOutline *pOutline = &pIndoor->pMapOutlines->pOutlines[i];

            if (pIndoor->pFaces[pOutline->uFace1ID].Visible() &&
                pIndoor->pFaces[pOutline->uFace2ID].Visible()) {
                if (pIndoor->pFaces[pOutline->uFace1ID].uAttributes & FACE_RENDERED ||
                    pIndoor->pFaces[pOutline->uFace2ID].uAttributes & FACE_RENDERED) {
                    pOutline->uFlags = pOutline->uFlags | 1;
                    pIndoor->_visible_outlines[i >> 3] |= 1 << (7 - i % 8);

                    int Vert1X = pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[i].uVertex1ID].x - pParty->vPosition.x;
                    int Vert2X = pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[i].uVertex2ID].x - pParty->vPosition.x;
                    int Vert1Y = pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[i].uVertex1ID].y - pParty->vPosition.y;
                    int Vert2Y = pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[i].uVertex2ID].y - pParty->vPosition.y;

                    int linex = uCenterX + fixpoint_mul(uZoom, Vert1X);
                    int liney = uCenterY - fixpoint_mul(uZoom, Vert1Y);
                    int linez = uCenterX + fixpoint_mul(uZoom, Vert2X);
                    int linew = uCenterY - fixpoint_mul(uZoom, Vert2Y);

                    if (bWizardEyeActive && uWizardEyeSkillLevel >= 3 &&
                        (pIndoor->pFaces[pOutline->uFace1ID].Clickable() ||
                         pIndoor->pFaces[pOutline->uFace2ID].Clickable()) &&
                        (pIndoor->pFaceExtras[pIndoor->pFaces[pOutline->uFace1ID].uFaceExtraID].uEventID ||
                         pIndoor->pFaceExtras[pIndoor->pFaces[pOutline->uFace2ID].uFaceExtraID].uEventID)) {
                        render->RasterLine2D(linex, liney, linez, linew, ui_game_minimap_outline_color);
                    } else {
                        LineGreyDim = abs(pOutline->sZ - pParty->vPosition.z) / 8;
                        if (LineGreyDim > 100) LineGreyDim = 100;
                        render->RasterLine2D(linex, liney, linez, linew, viewparams->pPalette[-LineGreyDim + 200]);
                    }
                }
            }
        }
    }

    // draw arrow on the minimap(include. Ritor1)
    uint arrow_idx;
    unsigned int rotate = pParty->sRotationZ & TrigLUT->uDoublePiMask;
    if ((signed int)rotate <= 1920) arrow_idx = 6;
    if ((signed int)rotate < 1664) arrow_idx = 5;
    if ((signed int)rotate <= 1408) arrow_idx = 4;
    if ((signed int)rotate < 1152) arrow_idx = 3;
    if ((signed int)rotate <= 896) arrow_idx = 2;
    if ((signed int)rotate < 640) arrow_idx = 1;
    if ((signed int)rotate <= 384) arrow_idx = 0;
    if ((signed int)rotate < 128 || (signed int)rotate > 1920) arrow_idx = 7;
    render->DrawTextureAlphaNew((uCenterX - 3) / 640.0f, (uCenterY - 3) / 480.0f, game_ui_minimap_dirs[arrow_idx]);

    // draw objects on the minimap
    if (bWizardEyeActive) {
        if (uWizardEyeSkillLevel >= 2) {
            for (uint i = 0; i < uNumSpriteObjects; ++i) {
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
                        render->RasterLine2D(pPoint_X, pPoint_Y, pPoint_X,
                                             pPoint_Y,
                                             ui_game_minimap_projectile_color);
                    } else if (uZoom > 512) {
                        render->RasterLine2D(pPoint_X - 2, pPoint_Y,
                                             pPoint_X - 2, pPoint_Y + 1,
                                             ui_game_minimap_treasure_color);
                        render->RasterLine2D(pPoint_X - 1, pPoint_Y - 1,
                                             pPoint_X - 1, pPoint_Y + 1,
                                             ui_game_minimap_treasure_color);
                        render->RasterLine2D(pPoint_X, pPoint_Y - 2, pPoint_X,
                                             pPoint_Y + 1,
                                             ui_game_minimap_treasure_color);
                        render->RasterLine2D(pPoint_X + 1, pPoint_Y - 1,
                                             pPoint_X + 1, pPoint_Y + 1,
                                             ui_game_minimap_treasure_color);
                        render->RasterLine2D(pPoint_X + 2, pPoint_Y,
                                             pPoint_X + 2, pPoint_Y + 1,
                                             ui_game_minimap_treasure_color);
                    } else {
                        render->RasterLine2D(pPoint_X - 1, pPoint_Y - 1,
                                             pPoint_X - 1, pPoint_Y,
                                             ui_game_minimap_treasure_color);
                        render->RasterLine2D(pPoint_X, pPoint_Y - 1, pPoint_X,
                                             pPoint_Y,
                                             ui_game_minimap_treasure_color);
                    }
                }
            }
        }
        for (uint i = 0; i < uNumActors;
             ++i) {  // draw actors(отрисовка монстров и нпс)
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
                    pColor = ui_game_minimap_actor_friendly_color;
                    if (pActors[i].uAttributes & ACTOR_HOSTILE)
                        pColor = ui_game_minimap_actor_hostile_color;
                    if (pActors[i].uAIState == Dead)
                        pColor = ui_game_minimap_actor_corpse_color;
                    if (uZoom > 1024) {
                        render->RasterLine2D(pPoint_X - 2, pPoint_Y - 1,
                                             pPoint_X - 2, pPoint_Y + 1,
                                             pColor);
                        render->RasterLine2D(pPoint_X - 1, pPoint_Y - 2,
                                             pPoint_X - 1, pPoint_Y + 2,
                                             pColor);
                        render->RasterLine2D(pPoint_X, pPoint_Y - 2, pPoint_X,
                                             pPoint_Y + 2, pColor);
                        render->RasterLine2D(pPoint_X + 1, pPoint_Y - 2,
                                             pPoint_X + 1, pPoint_Y + 2,
                                             pColor);
                        render->RasterLine2D(pPoint_X + 2, pPoint_Y - 1,
                                             pPoint_X + 2, pPoint_Y + 1,
                                             pColor);
                    } else {
                        render->RasterLine2D(pPoint_X - 1, pPoint_Y - 1,
                                             pPoint_X - 1, pPoint_Y, pColor);
                        render->RasterLine2D(pPoint_X, pPoint_Y - 1, pPoint_X,
                                             pPoint_Y, pColor);
                    }
                }
            }
        }
        for (uint i = 0; i < (signed int)uNumLevelDecorations;
             ++i) {  // draw items(отрисовка предметов)
            if (pLevelDecorations[i].uFlags & 8) {
                pPoint_X =
                    uCenterX + (fixpoint_mul((pLevelDecorations[i].vPosition.x -
                                              pParty->vPosition.x),
                                             uZoom));
                pPoint_Y =
                    uCenterY - (fixpoint_mul((pLevelDecorations[i].vPosition.y -
                                              pParty->vPosition.y),
                                             uZoom));
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

    render->DrawTextureAlphaNew(468 / 640.0f, 0, game_ui_minimap_frame);
    render->SetUIClipRect(541, 0, 567, 480);
    render->DrawTextureAlphaNew((floorf(((double)pParty->sRotationZ * 0.1171875) + 0.5f) + 285) / 640.0f,
        136 / 480.0f, game_ui_minimap_compass);
    render->ResetUIClipRect();
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
            render->DrawTextureAlphaNew(
                pUIAnum_Torchlight->x / 640.0f, pUIAnum_Torchlight->y / 480.0f,
                pIconsFrameTable
                    ->GetFrame(pUIAnum_Torchlight->icon->id,
                               pEventTimer->Time())
                    ->GetTexture());
        }
        if (pParty->WizardEyeActive()) {
            render->DrawTextureAlphaNew(
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
    unsigned __int8 pNPC_limit_ID;  // [sp+3Bh] [bp-1h]@2

    char buf[4096];
    if (bNoNPCHiring != 1) {
        pNPC_limit_ID = 0;
        v22 = 0;
        if (pParty->pHirelings[0].pName) buf[v22++] = 0;
        if (pParty->pHirelings[1].pName) buf[v22++] = 1;

        for (uint i = 0; i < pNPCStats->uNumNewNPCs; ++i) {
            if (pNPCStats->pNewNPCData[i].uFlags & 128) {
                if (!pParty->pHirelings[0].pName ||
                    strcmp(pNPCStats->pNewNPCData[i].pName,
                           pParty->pHirelings[0].pName)) {
                    if (!pParty->pHirelings[1].pName ||
                        strcmp(pNPCStats->pNewNPCData[i].pName,
                               pParty->pHirelings[1].pName))
                        buf[v22++] = i + 2;
                }
            }
        }

        for (int i = pParty->hirelingScrollPosition;
             i < v22 && pNPC_limit_ID < 2; i++) {
            if ((unsigned __int8)buf[i] >= 2) {
                sprintf(pContainer, "NPC%03d",
                        pNPCStats->pNPCData[(unsigned __int8)buf[i] + 499]
                            .uPortraitID);
                render->DrawTextureAlphaNew(
                    pHiredNPCsIconsOffsetsX[pNPC_limit_ID] / 640.0f,
                    pHiredNPCsIconsOffsetsY[pNPC_limit_ID] / 480.0f,
                    assets->GetImage_ColorKey(pContainer, 0x7FF));
            } else {
                sprintf(
                    pContainer, "NPC%03d",
                    pParty->pHirelings[(unsigned __int8)buf[i]].uPortraitID);
                render->DrawTextureAlphaNew(
                    pHiredNPCsIconsOffsetsX[pNPC_limit_ID] / 640.0f,
                    pHiredNPCsIconsOffsetsY[pNPC_limit_ID] / 480.0f,
                    assets->GetImage_ColorKey(pContainer, 0x7FF));
                if (pParty->pHirelings[(unsigned __int8)buf[i]].evt_A == 1) {
                    uFrameID =
                        pParty->pHirelings[(unsigned __int8)buf[i]].evt_B;
                    v13 = 0;
                    if (pIconsFrameTable->uNumIcons) {
                        for (v13 = 0; v13 < pIconsFrameTable->uNumIcons;
                             ++v13) {
                            if (!_stricmp("spell96",
                                          pIconsFrameTable->pIcons[v13]
                                              .GetAnimationName()))
                                break;
                        }
                    }
                    render->DrawTextureAlphaNew(
                        pHiredNPCsIconsOffsetsX[pNPC_limit_ID] / 640.0f,
                        pHiredNPCsIconsOffsetsY[pNPC_limit_ID] / 480.0f,
                        pIconsFrameTable->GetFrame(v13, uFrameID)
                            ->GetTexture());
                }
            }
            ++pNPC_limit_ID;
        }
    }
}

//----- (004178FE) --------------------------------------------------------
unsigned int UI_GetHealthManaAndOtherQualitiesStringColor(int actual_value,
                                                          int base_value) {
    unsigned __int16 R, G, B;

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

    return Color16(R, G, B);
}

//----- (00417939) --------------------------------------------------------
int GetConditionDrawColor(unsigned int uConditionIdx) {
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
        case Condition_Unconcious:
            return ui_character_condition_moderate_color;

        case Condition_Poison_Severe:
        case Condition_Disease_Severe:
        case Condition_Dead:
        case Condition_Pertified:
        case Condition_Eradicated:
            return ui_character_condition_severe_color;
    }
    Error("Invalid condition (%u)", uConditionIdx);
}

//----- (00495430) --------------------------------------------------------
String GetReputationString(int reputation) {
    if (reputation >= 25)
        return localization->GetString(379);  // Hated
    else if (reputation >= 6)
        return localization->GetString(392);  // Unfriendly
    else if (reputation >= -5)
        return localization->GetString(399);  // Neutral
    else if (reputation >= -24)
        return localization->GetString(402);  // Friendly
    else
        return localization->GetString(434);  // Respected
}

__int16 _441A4E_overlay_on_portrait(int a1) {  // for blessing
    __int16 result;                            // ax@1
    int v2;                                    // ebx@1
    bool v5;                                   // ecx@4
    SpriteFrame *pFrame;                       // eax@6
    int v8;                                    // eax@6
    SoftwareBillboard v10;                     // [sp+Ch] [bp-5Ch]@1
    int v11;                                   // [sp+5Ch] [bp-Ch]@6
    int v12;                                   // [sp+60h] [bp-8h]@1

    v10.sParentBillboardID = -1;
    v10.pTargetZ = render->pActiveZBuffer;
    v10.uTargetPitch = render->GetRenderWidth();
    result = 0;
    v2 = a1;
    v10.uViewportX = 0;
    v10.uViewportY = 0;
    v10.uViewportZ = window->GetWidth() - 1;
    v10.uViewportW = window->GetHeight() - 1;
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
                        v10.pPalette = PaletteManager::Get_Dark_or_Red_LUT(
                            pFrame->uPaletteIndex, 0, 1);
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
    : GUIWindow(WINDOW_DebugMenu, 0, 0, window->GetWidth(), window->GetHeight(), 0) {

    pEventTimer->Pause();
    int width = 108;
    int height = 20;

    game_ui_menu_options = assets->GetImage_ColorKey("options", 0x7FF);

    GUIButton *pBtn_DebugTownPortal = CreateButton(13, 140, width, height, 1, 0, UIMSG_DebugTownPortal, 0, GameKey::None, "DEBUG TOWN PORTAL");
    GUIButton *pBtn_DebugGiveGold = CreateButton(127, 140, width, height, 1, 0, UIMSG_DebugGiveGold, 0, GameKey::None, "DEBUG GIVE GOLD (10000)");
    GUIButton *pBtn_DebugGiveEXP = CreateButton(241, 140, width, height, 1, 0, UIMSG_DebugGiveEXP, 0, GameKey::None, "DEBUG GIVE EXP (20000)");
    GUIButton *pBtn_DebugGiveSkillP = CreateButton(354, 140, width, height, 1, 0, UIMSG_DebugGiveSkillP, 0, GameKey::None, "DEBUG GIVE SKILL POINT (50)");

    GUIButton *pBtn_DebugLearnSkill = CreateButton(13, 167, width, height, 1, 0, UIMSG_DebugLearnSkills, 0, GameKey::None, "DEBUG LEARN CLASS SKILLS");
    GUIButton *pBtn_DebugRemoveGold = CreateButton(127, 167, width, height, 1, 0, UIMSG_DebugTakeGold, 0, GameKey::None, "DEBUG REMOVE GOLD");
    GUIButton *pBtn_DebugAddFood = CreateButton(241, 167, width, height, 1, 0, UIMSG_DebugGiveFood, 0, GameKey::None, "DEBUG GIVE FOOD (20)");
    GUIButton *pBtn_DebugTakeFood = CreateButton(354, 167, width, height, 1, 0, UIMSG_DebugTakeFood, 0, GameKey::None, "DEBUG REMOVE FOOD");

    GUIButton *pBtn_DebugCycleAlign = CreateButton(13, 194, width, height, 1, 0, UIMSG_DebugCycleAlign, 0, GameKey::None, "DEBUG CYCLE ALIGNMENT");
    GUIButton *pBtn_DebugWizardEye = CreateButton(127, 194, width, height, 1, 0, UIMSG_DebugWizardEye, 0, GameKey::None, "DEBUG TOGGLE WIZARD EYE");
    GUIButton *pBtn_DebugAllMagic = CreateButton(241, 194, width, height, 1, 0, UIMSG_DebugAllMagic, 0, GameKey::None, "DEBUG TOGGLE All MAGIC");
    GUIButton *pBtn_DebugTerrain = CreateButton(354, 194, width, height, 1, 0, UIMSG_DebugTerrain, 0, GameKey::None, "DEBUG TOGGLE TERRAIN");

    GUIButton *pBtn_DebugLightMap = CreateButton(13, 221, width, height, 1, 0, UIMSG_DebugLightmap, 0, GameKey::None, "DEBUG TOGGLE LIGHTMAP DECAL");
    GUIButton *pBtn_DebugTurbo = CreateButton(127, 221, width, height, 1, 0, UIMSG_DebugTurboSpeed, 0, GameKey::None, "DEBUG TOGGLE TURBO SPEED");
    GUIButton *pBtn_DebugNoActors = CreateButton(241, 221, width, height, 1, 0, UIMSG_DebugNoActors, 0, GameKey::None, "DEBUG TOGGLE ACTORS");
    GUIButton *pBtn_DebugDrawDist = CreateButton(354, 221, width, height, 1, 0, UIMSG_DebugDrawDist, 0, GameKey::None, "DEBUG TOGGLE EXTENDED DRAW DISTANCE");

    GUIButton *pBtn_DebugSnow = CreateButton(13, 248, width, height, 1, 0, UIMSG_DebugSnow, 0, GameKey::None, "DEBUG TOGGLE SNOW");
    GUIButton *pBtn_DebugPortalLines = CreateButton(127, 248, width, height, 1, 0, UIMSG_DebugPortalLines, 0, GameKey::None, "DEBUG TOGGLE PORTAL OUTLINES");
    GUIButton *pBtn_DebugPickedFace = CreateButton(241, 248, width, height, 1, 0, UIMSG_DebugPickedFace, 0, GameKey::None, "DEBUG TOGGLE SHOW PICKED FACE");
    GUIButton *pBtn_DebugShowFPS = CreateButton(354, 248, width, height, 1, 0, UIMSG_DebugShowFPS, 0, GameKey::None, "DEBUG TOGGLE SHOW FPS");

    GUIButton *pBtn_DebugSeasonsChange = CreateButton(13, 275, width, height, 1, 0, UIMSG_DebugSeasonsChange, 0, GameKey::None, "DEBUG TOGGLE SEASONS CHANGE");
    GUIButton *pBtn_DebugFarClipToggle = CreateButton(127, 275, width, height, 1, 0, UIMSG_DebugFarClip, 0, GameKey::None, "DEBUG TOGGLE FAR CLIP DISTANCE");
    GUIButton *pBtn_DebugGenItem = CreateButton(241, 275, width, height, 1, 0, UIMSG_DebugGenItem, 0, GameKey::None, "DEBUG GENERATE RANDOM ITEM");
    GUIButton *pBtn_DebugSpecialItem = CreateButton(354, 275, width, height, 1, 0, UIMSG_DebugSpecialItem, 0, GameKey::None, "DEBUG GENERATE RANDOM SPECIAL ITEM");

    //

    GUIButton *pBtn_DebugKillChar = CreateButton(13, 329, width, height, 1, 0, UIMSG_DebugKillChar, 0, GameKey::None, "DEBUG KILL SELECTED CHARACTER");
    GUIButton *pBtn_DebugEradicate = CreateButton(127, 329, width, height, 1, 0, UIMSG_DebugEradicate, 0, GameKey::None, "DEBUG ERADICATE SELECTED CHARACTER");
    GUIButton *pBtn_DebugNoDamage = CreateButton(241, 329, width, height, 1, 0, UIMSG_DebugNoDamage, 0, GameKey::None, "DEBUG TOGGLE NO DAMAGE");
    GUIButton *pBtn_DebugFullHeal = CreateButton(354, 329, width, height, 1, 0, UIMSG_DebugFullHeal, 0, GameKey::None, "DEBUG FULLY HEAL SELECTED CHARACTER");
}

void GUIWindow_DebugMenu::Update() {
    render->DrawTextureAlphaNew(pViewport->uViewportTL_X / 640.0f,
        pViewport->uViewportTL_Y / 480.0f,
        game_ui_menu_options);

    buttonbox(13, 140, "Town Portal", engine->config->debug_town_portal);
    buttonbox(127, 140, "Give Gold", 2);
    buttonbox(241, 140, "Give EXP", 2);
    buttonbox(354, 140, "Give Skill", 2);

    buttonbox(13, 167, "Learn Skills", 2);
    buttonbox(127, 167, "Take Gold", 2);
    buttonbox(241, 167, "Give Food", 2);
    buttonbox(354, 167, "Take Food", 2);

    int col = 0;
    if (pParty->alignment == PartyAlignment::PartyAlignment_Evil) col = 2;
    if (pParty->alignment == PartyAlignment::PartyAlignment_Neutral) col = 1;
    buttonbox(13, 194, "Alignment", col);
    buttonbox(127, 194, "WizardEye", engine->config->debug_wizard_eye);
    buttonbox(241, 194, "All Magic", engine->config->debug_all_magic);
    buttonbox(354, 194, "Terrain", engine->config->debug_terrain);

    buttonbox(13, 221, "Lightmap", engine->config->debug_lightmaps_decals);
    buttonbox(127, 221, "Turbo", engine->config->debug_turbo_speed);
    buttonbox(241, 221, "Actors", engine->config->no_actors);
    buttonbox(354, 221, "Draw Dist", engine->config->extended_draw_distance);

    buttonbox(13, 248, "Snow", engine->config->allow_snow);
    buttonbox(127, 248, "Portal Lines", engine->config->debug_portal_outlines);
    buttonbox(241, 248, "Picked Face", engine->config->show_picked_face);
    buttonbox(354, 248, "Show FPS", engine->config->show_fps);

    buttonbox(13, 275, "Seasons", engine->config->seasons_change);
    buttonbox(127, 275, "Far Clip", pODMRenderParams->far_clip == 0x6000);
    buttonbox(241, 275, "Gen Item", 2);
    buttonbox(354, 275, "Special Item", 2);

    // times ??
    // conditions ??

    buttonbox(13, 329, "Dead", 2);
    buttonbox(127, 329, "Eradicate", 2);
    buttonbox(241, 329, "No Damage", engine->config->no_damage);
    buttonbox(354, 329, "Full Heal", 2);

    viewparams->bRedrawGameUI = true;
}

void buttonbox(int x, int y, char* text, int col) {
    int width = 108;
    int height = 20;
    render->FillRectFast(x, y, width+1, height+1, Color16(50, 50, 50));

    render->RasterLine2D(x-1, y-1, x+width+1, y-1, Color16(0xE1u, 255, 0x9Bu));
    render->RasterLine2D(x-1, y-1, x-1, y+height+1, Color16(0xE1u, 255, 0x9Bu));
    render->RasterLine2D(x-1, y+height+1, x+width+1, y+height+1, Color16(0xE1u, 255, 0x9Bu));
    render->RasterLine2D(x+width+1, y-1, x+width+1, y+height+1, Color16(0xE1u, 255, 0x9Bu));

    uint colour = ui_character_condition_severe_color;
    if (col == 2) {
        colour = 0;
    }
    if (col == 1) {
        colour = ui_character_bonus_text_color;
    }
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, x+1, y+2, colour, text, 0, 0, 0);
}
