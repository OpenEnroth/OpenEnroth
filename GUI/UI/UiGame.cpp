#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <crtdbg.h>

#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/Events.h"
#include "Engine/MapInfo.h"
#include "Engine/Party.h"
#include "Engine/LOD.h"
#include "Engine/stru123.h"
#include "Engine/Time.h"
#include "Engine/OurMath.h"

#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Tables/PlayerFrameTable.h"

#include "Engine/TurnEngine/TurnEngine.h"

#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/BSPModel.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Viewport.h"

#include "Engine/Objects/Chest.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/ObjectList.h"

#include "IO/Mouse.h"
#include "IO/Keyboard.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UICharacter.h"
#include "GUI/UI/UiStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

#include "Game/Game.h"

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

Image *game_ui_minimap_frame = nullptr;   // 5079D8
Image *game_ui_minimap_compass = nullptr; // 5079B4
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

Image *game_ui_evtnpc = nullptr; // 50795C

std::array< std::array<Image *, 56>, 4> game_ui_player_faces;
Image *game_ui_player_face_eradicated = nullptr;
Image *game_ui_player_face_dead = nullptr;

Image *game_ui_player_selection_frame = nullptr; // 50C98C
Image *game_ui_player_alert_yellow = nullptr; // 5079C8
Image *game_ui_player_alert_red = nullptr;    // 5079CC
Image *game_ui_player_alert_green = nullptr;  // 5079D0

Image *game_ui_bar_red = nullptr;
Image *game_ui_bar_yellow = nullptr;
Image *game_ui_bar_green = nullptr;
Image *game_ui_bar_blue = nullptr;

Image *game_ui_playerbuff_pain_reflection = nullptr;
Image *game_ui_playerbuff_hammerhands = nullptr;
Image *game_ui_playerbuff_preservation = nullptr;
Image *game_ui_playerbuff_bless = nullptr;

GUIWindow_GameMenu::GUIWindow_GameMenu() :
    GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), 0, nullptr)
{
// -----------------------
// GameMenuUI_Load -- part
    game_ui_menu_options = assets->GetImage_16BitAlpha("options");
    game_ui_menu_new = assets->GetImage_16BitAlpha("new1");
    game_ui_menu_load = assets->GetImage_16BitAlpha("load1");
    game_ui_menu_save = assets->GetImage_16BitAlpha("save1");
    game_ui_menu_controls = assets->GetImage_16BitAlpha("controls1");
    game_ui_menu_resume = assets->GetImage_16BitAlpha("resume1");
    game_ui_menu_quit = assets->GetImage_16BitAlpha("quit1");

    pBtn_NewGame = CreateButton(
        0x13u, 0x9Bu, 0xD6u, 0x28u, 1, 0, UIMSG_StartNewGame, 0, 0x4Eu,
        localization->GetString(614),// "New Game"
        game_ui_menu_new,
        0
    );
    pBtn_SaveGame = CreateButton(
        0x13u, 0xD1u, 0xD6u, 0x28u, 1, 0, UIMSG_Game_OpenSaveGameDialog, 0, 0x53u,
        localization->GetString(615),// "Save Game"
        game_ui_menu_save,
        0
    );
    pBtn_LoadGame = CreateButton(
        19, 263, 0xD6u, 0x28u, 1, 0, UIMSG_Game_OpenLoadGameDialog, 0, 0x4Cu,
        localization->GetString(616),// "Load Game"
        game_ui_menu_load,
        0
    );
    pBtn_GameControls = CreateButton(
        241, 155, 214, 40, 1, 0, UIMSG_Game_OpenOptionsDialog, 0, 0x43u,
        localization->GetString(617),// ""Sound, Keyboard, Game Options:""
        game_ui_menu_controls,
        0
    );
    pBtn_QuitGame = CreateButton(
        241, 209, 214, 40, 1, 0, UIMSG_Quit, 0, 0x51u,
        localization->GetString(618),// "Quit"
        game_ui_menu_quit,
        0
    );
    pBtn_Resume = CreateButton(
        241, 263, 214, 40, 1, 0, UIMSG_GameMenu_ReturnToGame, 0, 0x52u,
        localization->GetString(619),// "Return to Game"
        game_ui_menu_resume,
        0
    );
    _41D08F_set_keyboard_control_group(6, 1, 0, 0);
}

void GUIWindow_GameMenu::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
    render->DrawTextureAlphaNew(
        pViewport->uViewportTL_X/640.0f,
        pViewport->uViewportTL_Y/480.0f,
        game_ui_menu_options
    );

    viewparams->bRedrawGameUI = true;
}





//----- (00491CB5) --------------------------------------------------------
void GameUI_LoadPlayerPortraintsAndVoices()
{
    for (uint i = 0; i < 4; ++i)
    {
        for (uint j = 0; j < 56; ++j)
        {
            game_ui_player_faces[i][j] = assets->GetImage_16BitColorKey(
                StringPrintf(
                    "%s%02d", pPlayerPortraitsNames[pParty->pPlayers[i].uCurrentFace], j + 1
                ),
                0x7FF
            );
        }
    }

    game_ui_player_face_eradicated = assets->GetImage_16BitColorKey("ERADCATE", 0x7FF);
    game_ui_player_face_dead = assets->GetImage_16BitColorKey("DEAD", 0x7FF);

    if (SoundSetAction[24][0])
    {
        for (uint i = 0; i < 4; ++i)
        {
            pSoundList->LoadSound(2 * (SoundSetAction[24][0] + 50 * pParty->pPlayers[i].uVoiceID) + 4998, 0);
            pSoundList->LoadSound(2 * (SoundSetAction[24][0] + 50 * pParty->pPlayers[i].uVoiceID) + 4999, 0);
        }
    }
}

//----- (00491DE7) --------------------------------------------------------
void GameUI_ReloadPlayerPortraits(int player_id, int face_id) //the transition from the zombies to the normal state
{
    for (uint i = 0; i <= 55; ++i)
    {
        auto filename = StringPrintf("%s%02d", pPlayerPortraitsNames[face_id], i + 1);
        game_ui_player_faces[player_id][i] = assets->GetImage_16BitColorKey(filename, 0x7FF);
    }
}


std::array<bool, 28> GameMenuUI_InvaligKeyBindingsFlags; // 506E6C
//----- (00414D24) --------------------------------------------------------
static unsigned int GameMenuUI_GetKeyBindingColor(int key_index)
{
    if (uGameMenuUI_CurentlySelectedKeyIdx == key_index)
    {
        if (OS_GetTime() % 1000 < 500)
            return ui_gamemenu_keys_key_selection_blink_color_1;
        else
            return ui_gamemenu_keys_key_selection_blink_color_2;
    }
    else if (GameMenuUI_InvaligKeyBindingsFlags[key_index])
    {
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



GUIWindow_GameKeyBindings::GUIWindow_GameKeyBindings():
    GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), 0, nullptr)
{
// ------------------------------------------
// GameMenuUI_OptionsKeymapping_Load --- part
    game_ui_options_controls[0] = assets->GetImage_16BitColorKey("optkb", 0x7FF);
    game_ui_options_controls[1] = assets->GetImage_16BitColorKey("optkb_h", 0x7FF);
    game_ui_options_controls[2] = assets->GetImage_16BitColorKey("resume1", 0x7FF);
    game_ui_options_controls[3] = assets->GetImage_16BitColorKey("optkb_1", 0x7FF);
    game_ui_options_controls[4] = assets->GetImage_16BitColorKey("optkb_2", 0x7FF);

    CreateButton(241, 302, 214, 40, 1, 0, UIMSG_Escape, 0, 0, "", 0);

    CreateButton(19, 302, 108, 20, 1, 0, UIMSG_SelectKeyPage1, 0, 0, "", 0);
    CreateButton(127, 302, 108, 20, 1, 0, UIMSG_SelectKeyPage2, 0, 0, "", 0);
    CreateButton(127, 324, 108, 20, 1, 0, UIMSG_ResetKeyMapping, 0, 0, "", 0);
    CreateButton(19, 324, 108, 20, 1, 0, UIMSG_Game_OpenOptionsDialog, 0, 0, "", 0);

    CreateButton(129, 148, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 0, 0, "", 0);
    CreateButton(129, 167, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 1, 0, "", 0);
    CreateButton(129, 186, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 2, 0, "", 0);
    CreateButton(129, 205, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 3, 0, "", 0);
    CreateButton(129, 224, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 4, 0, "", 0);
    CreateButton(129, 243, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 5, 0, "", 0);
    CreateButton(129, 262, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 6, 0, "", 0);

    CreateButton(350, 148, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 7, 0, "", 0);
    CreateButton(350, 167, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 8, 0, "", 0);
    CreateButton(350, 186, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 9, 0, "", 0);
    CreateButton(350, 205, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 10, 0, "", 0);
    CreateButton(350, 224, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 11, 0, "", 0);
    CreateButton(350, 243, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 12, 0, "", 0);
    CreateButton(350, 262, 70, 19, 1, 0, UIMSG_ChangeKeyButton, 13, 0, "", 0);

    uGameMenuUI_CurentlySelectedKeyIdx = -1;
    KeyboardPageNum = 1;
    memset(GameMenuUI_InvaligKeyBindingsFlags.data(), 0, sizeof(GameMenuUI_InvaligKeyBindingsFlags));
    memcpy(pPrevVirtualCidesMapping.data(), pKeyActionMap->pVirtualKeyCodesMapping, 0x78u);
}

//----- (004142D3) --------------------------------------------------------
void GUIWindow_GameKeyBindings::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
    signed int v4; // ecx@7
    signed int v5; // eax@8

    if (pGUIWindow_CurrentMenu->receives_keyboard_input_2 == WINDOW_INPUT_CONFIRMED)
    {
        pPrevVirtualCidesMapping[uGameMenuUI_CurentlySelectedKeyIdx] = pKeyActionMap->pPressedKeysBuffer[0];
        memset(GameMenuUI_InvaligKeyBindingsFlags.data(), 0, sizeof(GameMenuUI_InvaligKeyBindingsFlags));
        v4 = 0;
        do
        {
            v5 = 0;
            do
            {
                if (v4 != v5 && pPrevVirtualCidesMapping[v4] == pPrevVirtualCidesMapping[v5])
                {
                    GameMenuUI_InvaligKeyBindingsFlags[v4] = true;
                    GameMenuUI_InvaligKeyBindingsFlags[v5] = true;
                }
                ++v5;
            } while (v5 < 28);
            ++v4;
        } while (v4 < 28);
        uGameMenuUI_CurentlySelectedKeyIdx = -1;
        pGUIWindow_CurrentMenu->receives_keyboard_input_2 = WINDOW_INPUT_NONE;
    }
    render->DrawTextureAlphaNew(8/640.0f, 8/480.0f, game_ui_options_controls[0]);//draw base texture
    if (KeyboardPageNum == 1)
    {
        render->DrawTextureAlphaNew(19/640.0f, 302/480.0f, game_ui_options_controls[3]);

        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 23, 142, ui_gamemenu_keys_action_name_color, "ВПЕРЁД", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 127, 142, GameMenuUI_GetKeyBindingColor(0), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[0]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 23, 163, ui_gamemenu_keys_action_name_color, "НАЗАД", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 127, 163, GameMenuUI_GetKeyBindingColor(1), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[1]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 23, 184, ui_gamemenu_keys_action_name_color, "ВЛЕВО", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 127, 184, GameMenuUI_GetKeyBindingColor(2), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[2]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 23, 205, ui_gamemenu_keys_action_name_color, "ВПРАВО", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 127, 205, GameMenuUI_GetKeyBindingColor(3), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[3]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 23, 226, ui_gamemenu_keys_action_name_color, "КРИК", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 127, 226, GameMenuUI_GetKeyBindingColor(4), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[4]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 23, 247, ui_gamemenu_keys_action_name_color, "ПРЫЖОК", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 127, 247, GameMenuUI_GetKeyBindingColor(5), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[5]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 23, 268, ui_gamemenu_keys_action_name_color, "П.РЕЖИМ", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 127, 268, GameMenuUI_GetKeyBindingColor(6), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[6]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 247, 142, ui_gamemenu_keys_action_name_color, "ПРИМ. ЗАКЛ.", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 350, 142, GameMenuUI_GetKeyBindingColor(7), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[7]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 247, 163, ui_gamemenu_keys_action_name_color, "АТАКА", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 350, 163, GameMenuUI_GetKeyBindingColor(8), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[8]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 247, 184, ui_gamemenu_keys_action_name_color, "ДЕЙСТВ.", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 350, 184, GameMenuUI_GetKeyBindingColor(9), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[9]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 247, 205, ui_gamemenu_keys_action_name_color, "ЗАКЛИН.", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 350, 205, GameMenuUI_GetKeyBindingColor(10), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[10]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 247, 226, ui_gamemenu_keys_action_name_color, "ИГРОК", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 350, 226, GameMenuUI_GetKeyBindingColor(11), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[11]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 247, 247, ui_gamemenu_keys_action_name_color, "СЛЕД. ИГРОК", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 350, 247, GameMenuUI_GetKeyBindingColor(12), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[12]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 247, 268, ui_gamemenu_keys_action_name_color, "ЗАДАНИЯ", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 350, 268, GameMenuUI_GetKeyBindingColor(13), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[13]), 0, 0, 0);
    }
    else
    {
        render->DrawTextureAlphaNew(127/640.0f, 302/480.0f, game_ui_options_controls[4]);

        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 23, 142, ui_gamemenu_keys_action_name_color, "Б. СПРАВКА", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 127, 142, GameMenuUI_GetKeyBindingColor(14), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[14]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 23, 163, ui_gamemenu_keys_action_name_color, "ОТДЫХ", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 127, 163, GameMenuUI_GetKeyBindingColor(15), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[15]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 23, 184, ui_gamemenu_keys_action_name_color, "ТЕК. ВРЕМЯ", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 127, 184, GameMenuUI_GetKeyBindingColor(16), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[16]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 23, 205, ui_gamemenu_keys_action_name_color, "АВТОЗАМЕТКИ", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 127, 205, GameMenuUI_GetKeyBindingColor(17), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[17]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 23, 226, ui_gamemenu_keys_action_name_color, "КАРТА", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 127, 226, GameMenuUI_GetKeyBindingColor(18), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[18]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 23, 247, ui_gamemenu_keys_action_name_color, "БЕЖАТЬ", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 127, 247, GameMenuUI_GetKeyBindingColor(19), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[19]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 23, 268, ui_gamemenu_keys_action_name_color, "СМ. ВВЕРХ", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 127, 268, GameMenuUI_GetKeyBindingColor(20), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[20]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 247, 142, ui_gamemenu_keys_action_name_color, "СМ. ВНИЗ", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 350, 142, GameMenuUI_GetKeyBindingColor(21), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[21]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 247, 163, ui_gamemenu_keys_action_name_color, "СМ. ВПЕРЁД", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 350, 163, GameMenuUI_GetKeyBindingColor(22), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[22]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 247, 184, ui_gamemenu_keys_action_name_color, "ПРИБЛИЗ", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 350, 184, GameMenuUI_GetKeyBindingColor(23), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[23]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 247, 205, ui_gamemenu_keys_action_name_color, "ОТДАЛИТЬ", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 350, 205, GameMenuUI_GetKeyBindingColor(24), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[24]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 247, 226, ui_gamemenu_keys_action_name_color, "П. ВВЕРХ", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 350, 226, GameMenuUI_GetKeyBindingColor(25), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[25]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 247, 247, ui_gamemenu_keys_action_name_color, "П. ВНИЗ", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 350, 247, GameMenuUI_GetKeyBindingColor(26), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[26]), 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 247, 268, ui_gamemenu_keys_action_name_color, "ПРИЗЕМЛ", 0, 0, 0);
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, 350, 268, GameMenuUI_GetKeyBindingColor(27), pKeyActionMap->GetVKeyDisplayName(pPrevVirtualCidesMapping[27]), 0, 0, 0);
    }
}



GUIWindow_GameVideoOptions::GUIWindow_GameVideoOptions() :
    GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), 0, nullptr)
{
// -------------------------------------
// GameMenuUI_OptionsVideo_Load --- part
    game_ui_menu_options_video_background = assets->GetImage_16BitColorKey("optvid", 0x7FF);
    game_ui_menu_options_video_bloodsplats = assets->GetImage_16BitColorKey("opvdH-bs", 0x7FF);
    game_ui_menu_options_video_coloredlights = assets->GetImage_16BitColorKey("opvdH-cl", 0x7FF);
    game_ui_menu_options_video_tinting = assets->GetImage_16BitColorKey("opvdH-tn", 0x7FF);

    game_ui_menu_options_video_gamma_positions[0] = assets->GetImage_16BitColorKey("convol10", 0x7FF);
    game_ui_menu_options_video_gamma_positions[1] = assets->GetImage_16BitColorKey("convol20", 0x7FF);
    game_ui_menu_options_video_gamma_positions[2] = assets->GetImage_16BitColorKey("convol30", 0x7FF);
    game_ui_menu_options_video_gamma_positions[3] = assets->GetImage_16BitColorKey("convol40", 0x7FF);
    game_ui_menu_options_video_gamma_positions[4] = assets->GetImage_16BitColorKey("convol50", 0x7FF);
    game_ui_menu_options_video_gamma_positions[5] = assets->GetImage_16BitColorKey("convol60", 0x7FF);
    game_ui_menu_options_video_gamma_positions[6] = assets->GetImage_16BitColorKey("convol70", 0x7FF);
    game_ui_menu_options_video_gamma_positions[7] = assets->GetImage_16BitColorKey("convol80", 0x7FF);
    game_ui_menu_options_video_gamma_positions[8] = assets->GetImage_16BitColorKey("convol90", 0x7FF);
    game_ui_menu_options_video_gamma_positions[9] = assets->GetImage_16BitColorKey("convol00", 0x7FF);
    //not_available_bloodsplats_texture_id = pIcons_LOD->LoadTexture("opvdG-bs", TEXTURE_16BIT_PALETTE);
    //not_available_us_colored_lights_texture_id = pIcons_LOD->LoadTexture("opvdG-cl", TEXTURE_16BIT_PALETTE);
    //not_available_tinting_texture_id = pIcons_LOD->LoadTexture("opvdG-tn", TEXTURE_16BIT_PALETTE);

    CreateButton(0xF1u, 0x12Eu, 0xD6u, 0x28u, 1, 0, UIMSG_Escape, 0, 0, "", 0);
    //if ( render->pRenderD3D )
    {
        CreateButton(0x13u, 0x118u, 0xD6u, 0x12u, 1, 0, UIMSG_ToggleBloodsplats, 0, 0, "", 0);
        CreateButton(0x13u, 0x12Eu, 0xD6u, 0x12u, 1, 0, UIMSG_ToggleColoredLights, 0, 0, "", 0);
        CreateButton(0x13u, 0x144u, 0xD6u, 0x12u, 1, 0, UIMSG_ToggleTint, 0, 0, "", 0);
    }
}



//----- (00414D9A) --------------------------------------------------------
void GUIWindow_GameVideoOptions::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
    GUIWindow msg_window; // [sp+8h] [bp-54h]@3

    render->DrawTextureAlphaNew(8/640.0f, 8/480.0f, game_ui_menu_options_video_background);//draw base texture
    //if ( !render->bWindowMode && render->IsGammaSupported() )
    {
        render->DrawTextureAlphaNew(
            (17 * uGammaPos + 42)/640.0f,
            162/480.0f,
            game_ui_menu_options_video_gamma_positions[uGammaPos]);

        render->DrawTextureNew(274/640.0f, 169/480.0f, gamma_preview_image);
        msg_window.uFrameX = 22;
        msg_window.uFrameY = 190;
        msg_window.uFrameWidth = 211;
        msg_window.uFrameHeight = 79;
        msg_window.uFrameZ = 232;
        msg_window.uFrameW = 268;
        msg_window.DrawTitleText(pFontSmallnum, 0, 0, ui_gamemenu_video_gamma_title_color, localization->GetString(226), 3); // "Gamma controls the relative ""brightness"" of the game.  May vary depending on your monitor."
    }


      if (pEngine->uFlags2 & GAME_FLAGS_2_DRAW_BLOODSPLATS)
          render->DrawTextureAlphaNew(20/640.0f, 281/480.0f, game_ui_menu_options_video_bloodsplats);
      if (render->bUseColoredLights)
          render->DrawTextureAlphaNew(20/640.0f, 303/480.0f, game_ui_menu_options_video_coloredlights);
      if (render->bTinting)
          render->DrawTextureAlphaNew(20/640.0f, 325/480.0f, game_ui_menu_options_video_tinting);
}














OptionsMenuSkin options_menu_skin; // 507C60
OptionsMenuSkin::OptionsMenuSkin() :
uTextureID_Background(0),
uTextureID_ArrowLeft(0),
uTextureID_ArrowRight(0),
uTextureID_unused_0(0), uTextureID_unused_1(0), uTextureID_unused_2(0),
uTextureID_FlipOnExit(0),
uTextureID_AlwaysRun(0),
uTextureID_WalkSound(0),
uTextureID_ShowDamage(0)
{
    for (uint i = 0; i < 3; ++i)  uTextureID_TurnSpeed[i] = 0;
    for (uint i = 0; i < 10; ++i) uTextureID_SoundLevels[i] = 0;
}

void OptionsMenuSkin::Relaease()
{
#define RELEASE(img) \
  {\
    if (img)\
    {\
        img->Release(); \
        img = nullptr; \
    }\
  }

    RELEASE(uTextureID_Background);
    for (uint i = 0; i < 3; ++i)
        RELEASE(uTextureID_TurnSpeed[i]);
    RELEASE(uTextureID_ArrowLeft);
    RELEASE(uTextureID_ArrowRight);
    RELEASE(uTextureID_FlipOnExit);
    for (uint i = 0; i < 10; ++i)
        RELEASE(uTextureID_SoundLevels[i]);
    RELEASE(uTextureID_AlwaysRun);
    RELEASE(uTextureID_WalkSound);
    RELEASE(uTextureID_ShowDamage);

#undef RELEASE
}


GUIWindow_GameOptions::GUIWindow_GameOptions() :
    GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), 0, nullptr)
{
// GameMenuUI_Options_Load -- part
    options_menu_skin.uTextureID_Background = assets->GetImage_16BitColorKey("ControlBG", 0x7FF);
    options_menu_skin.uTextureID_TurnSpeed[2] = assets->GetImage_16BitColorKey("con_16x", 0x7FF);
    options_menu_skin.uTextureID_TurnSpeed[1] = assets->GetImage_16BitColorKey("con_32x", 0x7FF);
    options_menu_skin.uTextureID_TurnSpeed[0] = assets->GetImage_16BitColorKey("con_Smoo", 0x7FF);
    options_menu_skin.uTextureID_ArrowLeft = assets->GetImage_16BitAlpha("con_ArrL");
    options_menu_skin.uTextureID_ArrowRight = assets->GetImage_16BitAlpha("con_ArrR");
    options_menu_skin.uTextureID_SoundLevels[0] = assets->GetImage_16BitColorKey("convol10", 0x7FF);
    options_menu_skin.uTextureID_SoundLevels[1] = assets->GetImage_16BitColorKey("convol20", 0x7FF);
    options_menu_skin.uTextureID_SoundLevels[2] = assets->GetImage_16BitColorKey("convol30", 0x7FF);
    options_menu_skin.uTextureID_SoundLevels[3] = assets->GetImage_16BitColorKey("convol40", 0x7FF);
    options_menu_skin.uTextureID_SoundLevels[4] = assets->GetImage_16BitColorKey("convol50", 0x7FF);
    options_menu_skin.uTextureID_SoundLevels[5] = assets->GetImage_16BitColorKey("convol60", 0x7FF);
    options_menu_skin.uTextureID_SoundLevels[6] = assets->GetImage_16BitColorKey("convol70", 0x7FF);
    options_menu_skin.uTextureID_SoundLevels[7] = assets->GetImage_16BitColorKey("convol80", 0x7FF);
    options_menu_skin.uTextureID_SoundLevels[8] = assets->GetImage_16BitColorKey("convol90", 0x7FF);
    options_menu_skin.uTextureID_SoundLevels[9] = assets->GetImage_16BitColorKey("convol00", 0x7FF);
    options_menu_skin.uTextureID_FlipOnExit = assets->GetImage_16BitColorKey("option04", 0x7FF);
    options_menu_skin.uTextureID_AlwaysRun = assets->GetImage_16BitColorKey("option03", 0x7FF);
    options_menu_skin.uTextureID_ShowDamage = assets->GetImage_16BitColorKey("option02", 0x7FF);
    options_menu_skin.uTextureID_WalkSound = assets->GetImage_16BitColorKey("option01", 0x7FF);

    CreateButton(22, 270,
        options_menu_skin.uTextureID_TurnSpeed[2]->GetWidth(),
        options_menu_skin.uTextureID_TurnSpeed[2]->GetHeight(),
        1, 0, UIMSG_SetTurnSpeed, 0x80, 0, "", 0);
    CreateButton(93, 270,
        options_menu_skin.uTextureID_TurnSpeed[1]->GetWidth(),
        options_menu_skin.uTextureID_TurnSpeed[1]->GetHeight(),
        1, 0, UIMSG_SetTurnSpeed, 0x40u, 0, "", 0);
    CreateButton(164, 270,
        options_menu_skin.uTextureID_TurnSpeed[0]->GetWidth(),
        options_menu_skin.uTextureID_TurnSpeed[0]->GetHeight(),
        1, 0, UIMSG_SetTurnSpeed, 0, 0, "", 0);

    CreateButton(20, 303,
        options_menu_skin.uTextureID_WalkSound->GetWidth(),
        options_menu_skin.uTextureID_WalkSound->GetHeight(),
        1, 0, UIMSG_ToggleWalkSound, 0, 0, "", 0);
    CreateButton(128, 303,
        options_menu_skin.uTextureID_ShowDamage->GetWidth(),
        options_menu_skin.uTextureID_ShowDamage->GetHeight(),
        1, 0, UIMSG_ToggleShowDamage, 0, 0, "", 0);
    CreateButton(20, 325,
        options_menu_skin.uTextureID_AlwaysRun->GetWidth(),
        options_menu_skin.uTextureID_AlwaysRun->GetHeight(),
        1, 0, UIMSG_ToggleAlwaysRun, 0, 0, "", 0);
    CreateButton(128, 325,
        options_menu_skin.uTextureID_FlipOnExit->GetWidth(),
        options_menu_skin.uTextureID_FlipOnExit->GetHeight(),
        1, 0, UIMSG_ToggleFlipOnExit, 0, 0, "", 0);

    pBtn_SliderLeft = CreateButton(243, 162, 16, 16, 1, 0, UIMSG_ChangeSoundVolume, 4, 0, "", options_menu_skin.uTextureID_ArrowLeft, 0);
    pBtn_SliderRight = CreateButton(435, 162, 16, 16, 1, 0, UIMSG_ChangeSoundVolume, 5, 0, "", options_menu_skin.uTextureID_ArrowRight, 0);
    CreateButton(263, 162, 172, 17, 1, 0, UIMSG_ChangeSoundVolume, 0, 0, "", 0);

    pBtn_SliderLeft = CreateButton(243, 216, 16, 16, 1, 0, UIMSG_ChangeMusicVolume, 4, 0, "", options_menu_skin.uTextureID_ArrowLeft, 0);
    pBtn_SliderRight = CreateButton(435, 216, 16, 16, 1, 0, UIMSG_ChangeMusicVolume, 5, 0, "", options_menu_skin.uTextureID_ArrowRight, 0);
    CreateButton(263, 216, 172, 17, 1, 0, UIMSG_ChangeMusicVolume, 0, 0, "", 0);

    pBtn_SliderLeft = CreateButton(243, 270, 16, 16, 1, 0, UIMSG_ChangeVoiceVolume, 4, 0, "", options_menu_skin.uTextureID_ArrowLeft, 0);
    pBtn_SliderRight = CreateButton(435, 270, 16, 16, 1, 0, UIMSG_ChangeVoiceVolume, 5, 0, "", options_menu_skin.uTextureID_ArrowRight, 0);
    CreateButton(263, 270, 172, 17, 1, 0, UIMSG_ChangeVoiceVolume, 0, 0, "", 0);

    CreateButton(241, 302, 214, 40, 1, 0, UIMSG_Escape, 0, 0, localization->GetString(619), 0); // "Return to Game"
    CreateButton(19, 140, 214, 40, 1, 0, UIMSG_OpenKeyMappingOptions, 0, 0x4Bu, "", 0);
    CreateButton(19, 194, 214, 40, 1, 0, UIMSG_OpenVideoOptions, 0, 86, "", 0);
}




//----- (00414F82) --------------------------------------------------------
void GUIWindow_GameOptions::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
    render->DrawTextureAlphaNew(8/640.0f, 8/480.0f, game_ui_menu_options);
    render->DrawTextureAlphaNew(8/640.0f, 132/480.0f, options_menu_skin.uTextureID_Background);

    switch (uTurnSpeed)
    {
        case 64:   render->DrawTextureAlphaNew(BtnTurnCoord[1]/640.0f, 270/480.0f, options_menu_skin.uTextureID_TurnSpeed[1]); break;
        case 128:  render->DrawTextureAlphaNew(BtnTurnCoord[2]/640.0f, 270/480.0f, options_menu_skin.uTextureID_TurnSpeed[2]); break;
        default:   render->DrawTextureAlphaNew(BtnTurnCoord[0]/640.0f, 270/480.0f, options_menu_skin.uTextureID_TurnSpeed[0]); break;
    }

    if (bWalkSound)  render->DrawTextureAlphaNew(20/640.0f, 303/480.0f, options_menu_skin.uTextureID_WalkSound);
    if (bShowDamage) render->DrawTextureAlphaNew(128/640.0f, 303/480.0f, options_menu_skin.uTextureID_ShowDamage);
    if (bFlipOnExit) render->DrawTextureAlphaNew(128/640.0f, 325/480.0f, options_menu_skin.uTextureID_FlipOnExit);
    if (bAlwaysRun)  render->DrawTextureAlphaNew(20/640.0f, 325/480.0f, options_menu_skin.uTextureID_AlwaysRun);

    render->DrawTextureAlphaNew((265 + 17 * uSoundVolumeMultiplier)/640.0f, 162/480.0f, options_menu_skin.uTextureID_SoundLevels[uSoundVolumeMultiplier]);
    render->DrawTextureAlphaNew((265 + 17 * uMusicVolimeMultiplier)/640.0f, 216/480.0f, options_menu_skin.uTextureID_SoundLevels[uMusicVolimeMultiplier]);
    render->DrawTextureAlphaNew((265 + 17 * uVoicesVolumeMultiplier)/640.0f, 270/480.0f, options_menu_skin.uTextureID_SoundLevels[uVoicesVolumeMultiplier]);
}










//----- (00421D00) --------------------------------------------------------
void GameUI_OnPlayerPortraitLeftClick(unsigned int uPlayerID)
{
  Player* player = &pParty->pPlayers[uPlayerID - 1];
  if (pParty->pPickedItem.uItemID)
  {
    if (int slot = player->AddItem(-1, pParty->pPickedItem.uItemID))
    {
      memcpy(&player->pInventoryItemList[slot-1], &pParty->pPickedItem, 0x24u);
      viewparams->bRedrawGameUI = true;
      pMouse->RemoveHoldingItem();
      return;
    }

    if (!player->CanAct())
    {
      player = pPlayers[uActiveCharacter];
    }
    if( player->CanAct() || !pPlayers[uActiveCharacter]->CanAct() )
      player->PlaySound(SPEECH_NoRoom, 0);
  }

  if (current_screen_type == SCREEN_GAME)
  {
    viewparams->bRedrawGameUI = true;
    if ( uActiveCharacter != uPlayerID )
    {
      if ( pPlayers[uPlayerID]->uTimeToRecovery )
        return;

      uActiveCharacter = uPlayerID;
      return;
    }
    pGUIWindow_CurrentMenu = new GUIWindow_CharacterRecord(uActiveCharacter, SCREEN_CHARACTERS);//CharacterUI_Initialize(SCREEN_CHARACTERS);
    return;
  }
  if ( current_screen_type == SCREEN_SPELL_BOOK )
    return;
  if ( current_screen_type == SCREEN_CHEST )
  {
    viewparams->bRedrawGameUI = true;
    if ( uActiveCharacter == uPlayerID )
    {
        current_character_screen_window = WINDOW_CharacterWindow_Inventory;
      current_screen_type = SCREEN_CHEST_INVENTORY;
      uActiveCharacter = uPlayerID;
      return;
    }
    if ( pPlayers[uPlayerID]->uTimeToRecovery )
      return;
    uActiveCharacter = uPlayerID;
    return;
  }
  if ( current_screen_type != SCREEN_HOUSE )
  {
    if ( current_screen_type == SCREEN_E )
    {
      uActiveCharacter = uPlayerID;
      return;
    }
    if ( current_screen_type != SCREEN_CHEST_INVENTORY )
    {
      viewparams->bRedrawGameUI = true;
      uActiveCharacter = uPlayerID;
      if (current_character_screen_window == WINDOW_CharacterWindow_Awards)
        FillAwardsData();
      return;
    }
    viewparams->bRedrawGameUI = true;
    if ( uActiveCharacter == uPlayerID )
    {
        current_character_screen_window = WINDOW_CharacterWindow_Inventory;
      current_screen_type = SCREEN_CHEST_INVENTORY;
      uActiveCharacter = uPlayerID;
      return;
    }
    if ( pPlayers[uPlayerID]->uTimeToRecovery )
      return;
    uActiveCharacter = uPlayerID;
    return;
  }
  if ( window_SpeakInHouse->receives_keyboard_input_2 == WINDOW_INPUT_IN_PROGRESS)
    return;
  viewparams->bRedrawGameUI = true;
  if ( uActiveCharacter != uPlayerID )
  {
    uActiveCharacter = uPlayerID;
    return;
  }
  if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_STANDARD || dialog_menu_id == HOUSE_DIALOGUE_SHOP_6)
  {
    __debugbreak(); // fix indexing
    current_character_screen_window = WINDOW_CharacterWindow_Inventory;
    pGUIWindow_CurrentMenu = new GUIWindow_CharacterRecord(uActiveCharacter, SCREEN_E);//CharacterUI_Initialize(SCREEN_E);
    return;
  }
}

//----- (00416B01) --------------------------------------------------------
void GameUI_DrawNPCPopup(void *_this)//PopupWindowForBenefitAndJoinText
{
    int v1; // edi@2
    NPCData *pNPC; // eax@16
    const char *pText; // eax@18
    GUIWindow popup_window; // [sp+Ch] [bp-60h]@23
    int a2; // [sp+60h] [bp-Ch]@16
    const char *lpsz; // [sp+68h] [bp-4h]@6

    char buf[4096];
    if (bNoNPCHiring != 1)
    {
        v1 = 0;
        /*do
        {
          if ( v3->pName )
            tmp_str[v1++] = v2;
          ++v3;
          ++v2;
        }
        while ( (signed int)v3 < (signed int)&pParty->pPickedItem );*/
        for (int i = 0; i < 2; ++i)
        {
            if (pParty->pHirelings[i].pName)
                buf[v1++] = i;
        }
        lpsz = 0;
        if ((signed int)pNPCStats->uNumNewNPCs > 0)
        {
            /*v4 = pNPCStats->pNewNPCData;
            do
            {
              if ( v4->uFlags & 0x80
                && (!pParty->pHirelings[0].pName || strcmp(v4->pName, pParty->pHirelings[0].pName))
                && (!pParty->pHirelings[1].pName || strcmp(v4->pName, pParty->pHirelings[1].pName)) )
                tmp_str[v1++] = (char)lpsz + 2;
              ++lpsz;
              ++v4;
            }
            while ( (signed int)lpsz < (signed int)pNPCStats->uNumNewNPCs );*/
            for (uint i = 0; i < pNPCStats->uNumNewNPCs; ++i)
            {
                if (pNPCStats->pNewNPCData[i].Hired())
                {
                    if (!pParty->pHirelings[0].pName || strcmp((char *)pNPCStats->pNewNPCData[i].pName, (char *)pParty->pHirelings[0].pName))
                    {
                        if (!pParty->pHirelings[1].pName || strcmp((char *)pNPCStats->pNewNPCData[i].pName, (char *)pParty->pHirelings[1].pName))
                            buf[v1++] = i + 2;
                    }
                }
            }
        }
        if ((signed int)((char *)_this + pParty->hirelingScrollPosition) < v1)
        {
            sDialogue_SpeakingActorNPC_ID = -1 - pParty->hirelingScrollPosition - (int)_this;
            pNPC = GetNewNPCData(sDialogue_SpeakingActorNPC_ID, &a2);
            if (pNPC)
            {
                if (a2 == 57)
                    pText = pNPCTopics[512].pText; // Baby dragon
                else
                    pText = (const char *)pNPCStats->pProfessions[pNPC->uProfession].pBenefits;
                lpsz = pText;
                if (!pText)
                {
                    lpsz = (const char *)pNPCStats->pProfessions[pNPC->uProfession].pJoinText;
                    if (!lpsz)
                        lpsz = "";
                }
                popup_window.Hint = nullptr;
                popup_window.uFrameX = 38;
                popup_window.uFrameY = 60;
                popup_window.uFrameWidth = 276;
                popup_window.uFrameZ = 313;
                popup_window.uFrameHeight = pFontArrus->CalcTextHeight(lpsz, &popup_window, 0) + 2 * pFontArrus->GetFontHeight() + 24;
                if ((signed int)popup_window.uFrameHeight < 130)
                    popup_window.uFrameHeight = 130;
                popup_window.uFrameWidth = 400;
                popup_window.uFrameZ = popup_window.uFrameX + 399;
                popup_window.DrawMessageBox(0);

                auto tex_name = StringPrintf("NPC%03d", pNPC->uPortraitID);
                render->DrawTextureAlphaNew(
                    (popup_window.uFrameX + 22) / 640.0f,
                    (popup_window.uFrameY + 36) / 480.0f,
                    assets->GetImage_16BitColorKey(tex_name, 0x7FF)
                    );

                String title;
                if (pNPC->uProfession)
                {
                    title = localization->FormatString(429, pNPC->pName, localization->GetNpcProfessionName(pNPC->uProfession));
                }
                else
                {
                    title = pNPC->pName;
                }
                popup_window.DrawTitleText(pFontArrus, 0, 12, Color16(0xFFu, 0xFFu, 0x9Bu), title, 3);
                popup_window.uFrameWidth -= 24;
                popup_window.uFrameZ = popup_window.uFrameX + popup_window.uFrameWidth - 1;
                popup_window.DrawText(pFontArrus, 100, 36, 0, BuildDialogueString((char *)lpsz, uActiveCharacter - 1, 0, 0, 0));
            }
        }
    }
}

//----- (00445D4A) --------------------------------------------------------
void GameUI_InitializeDialogue(Actor *actor, int bPlayerSaysHello)
{
    NPCData *pNPCInfo; // ebp@1
    int v9; // esi@8
    int pNumberContacts; // eax@11

    dword_A74CDC = -1;
    pNPCStats->dword_AE336C_LastMispronouncedNameFirstLetter = -1;
    pEventTimer->Pause();
    pMiscTimer->Pause();
    pAudioPlayer->StopChannels(-1, -1);
    uDialogueType = 0;
    sDialogue_SpeakingActorNPC_ID = actor->sNPC_ID;
    pDialogue_SpeakingActor = actor;
    pNPCInfo = GetNPCData(actor->sNPC_ID);
    if ((pNPCInfo->uFlags & 3) != 2)
        pNPCInfo->uFlags = pNPCInfo->uFlags + 1;

    String filename;
    switch (pParty->alignment)
    {
        case PartyAlignment_Good:    filename = StringPrintf("evt%02d-b", const_2()); break;
        case PartyAlignment_Neutral: filename = StringPrintf("evt%02d", const_2());   break;
        case PartyAlignment_Evil:    filename = StringPrintf("evt%02d-c", const_2()); break;
    }
    game_ui_dialogue_background = assets->GetImage_16Bit(filename);

    pDialogueNPCCount = 0;
    uNumDialogueNPCPortraits = 1;

    filename = StringPrintf("npc%03u", pNPCInfo->uPortraitID);
    pDialogueNPCPortraits[0] = assets->GetImage_16BitColorKey(filename, 0x7FF);

    v9 = 0;
    if (!pNPCInfo->Hired() && pNPCInfo->Location2D >= 0)
    {
        if ((signed int)pParty->GetPartyFame() <= pNPCInfo->fame
            || (pNumberContacts = pNPCInfo->uFlags & 0xFFFFFF7F, (pNumberContacts & 0x80000000u) != 0))
        {
            v9 = 1;
        }
        else
        {
            if (pNumberContacts > 1)
            {
                if (pNumberContacts == 2)
                {
                    v9 = 3;
                }
                else
                {
                    if (pNumberContacts != 3)
                    {
                        if (pNumberContacts != 4)
                            v9 = 1;
                    }
                    else
                    {
                        v9 = 2;
                    }
                }
            }
            else if (pNPCInfo->rep)
            {
                v9 = 2;
            }
        }
    }
    if (sDialogue_SpeakingActorNPC_ID < 0)
        v9 = 4;
    pDialogueWindow = new GUIWindow_Dialogue(0, 0, window->GetWidth(), window->GetHeight(), 3, 0);//pNumberContacts = 1, v9 = 0; pNumberContacts = 2, v9 = 3;
    if (pNPCInfo->Hired() && !pNPCInfo->bHasUsedTheAbility)
    {
        if (pNPCInfo->uProfession == 10 ||    //Healer
            pNPCInfo->uProfession == 11 ||    //Expert Healer
            pNPCInfo->uProfession == 12 ||    //Master Healer
            pNPCInfo->uProfession == 33 ||    //Cook
            pNPCInfo->uProfession == 34 ||    //Chef
            pNPCInfo->uProfession == 39 ||    //Wind Master
            pNPCInfo->uProfession == 40 ||    //Water Master
            pNPCInfo->uProfession == 41 ||    //Gate Master
            pNPCInfo->uProfession == 42 ||    //Chaplain
            pNPCInfo->uProfession == 43 ||    //Piper
            pNPCInfo->uProfession == 52       //Fallen Wizard
        )
        {
            pDialogueWindow->CreateButton(480, 250, 140, pFontArrus->GetFontHeight() - 3, 1, 0, UIMSG_SelectNPCDialogueOption, 9, 0, "", 0);
            pDialogueWindow->_41D08F_set_keyboard_control_group(4, 1, 0, 1);
        }
    }

    pDialogueWindow->CreateButton(61, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 1, '1', "", 0);
    pDialogueWindow->CreateButton(177, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 2, '2', "", 0);
    pDialogueWindow->CreateButton(292, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 3, '3', "", 0);
    pDialogueWindow->CreateButton(407, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 4, '4', "", 0);

    if (bPlayerSaysHello && uActiveCharacter && !pNPCInfo->Hired())
    {
        if (pParty->uCurrentHour < 5 || pParty->uCurrentHour > 21)
            pPlayers[uActiveCharacter]->PlaySound(SPEECH_GoodEvening, 0);
        else
            pPlayers[uActiveCharacter]->PlaySound(SPEECH_GoodDay, 0);
    }
}

//----- (00445350) --------------------------------------------------------
void GameUI_DrawDialogue()
{
    NPCData *pNPC; // ebx@2
    int pGreetType; // eax@2
    int pTextHeight; // esi@39
    GUIButton *pButton; // eax@43
    int all_text_height; // ebx@93
    signed int index; // esi@99
    int v42; // edi@102
    int v45;
    unsigned __int16 pTextColor; // ax@104
    GUIWindow window; // [sp+ACh] [bp-68h]@42
  //  GUIFont *pOutString; // [sp+10Ch] [bp-8h]@39

    if (!pDialogueWindow)
        return;

    // Window title(Заголовок окна)----
    memcpy(&window, pDialogueWindow, sizeof(window));
    pNPC = GetNPCData(sDialogue_SpeakingActorNPC_ID);
    pGreetType = GetGreetType(sDialogue_SpeakingActorNPC_ID);
    window.uFrameWidth -= 10;
    window.uFrameZ -= 10;
    render->DrawTextureNew(477 / 640.0f, 0, game_ui_dialogue_background);
    render->DrawTextureAlphaNew(468 / 640.0f, 0, game_ui_right_panel_frame);
    render->DrawTextureAlphaNew((pNPCPortraits_x[0][0] - 4) / 640.0f, (pNPCPortraits_y[0][0] - 4) / 480.0f, game_ui_evtnpc);
    render->DrawTextureAlphaNew(pNPCPortraits_x[0][0] / 640.0f, pNPCPortraits_y[0][0] / 480.0f, pDialogueNPCPortraits[0]);

    String title;
    if (pNPC->uProfession)
    {
        assert(pNPC->uProfession < 59); // sometimes buffer overflows; errors emerge both here and in dialogue text
        title = localization->FormatString(429, pNPC->pName, localization->GetNpcProfessionName(pNPC->uProfession));//^Pi[%s] %s
    }
    else if (pNPC->pName)
        title = pNPC->pName;

    window.DrawTitleText(pFontArrus, 483, 112, ui_game_dialogue_npc_name_color, title, 3);

    pParty->GetPartyFame();

    String dialogue_string;
    switch (uDialogueType)
    {
    case DIALOGUE_13:
        dialogue_string = BuildDialogueString(pNPCStats->pProfessions[pNPC->uProfession].pJoinText, uActiveCharacter - 1, 0, 0, 0);
        break;

    case DIALOGUE_PROFESSION_DETAILS:
    {
        //auto prof = pNPCStats->pProfessions[pNPC->uProfession];

        if (dialogue_show_profession_details)
            dialogue_string = BuildDialogueString(pNPCStats->pProfessions[pNPC->uProfession].pBenefits, uActiveCharacter - 1, 0, 0, 0);
        else if (pNPC->Hired())
            dialogue_string = BuildDialogueString(pNPCStats->pProfessions[pNPC->uProfession].pDismissText, uActiveCharacter - 1, 0, 0, 0);
        else
            dialogue_string = BuildDialogueString(pNPCStats->pProfessions[pNPC->uProfession].pJoinText, uActiveCharacter - 1, 0, 0, 0);
    }
    break;

    case DIALOGUE_ARENA_WELCOME:
        dialogue_string = localization->GetString(574); // "Welcome to the Arena of Life and Death.  Remember, you are only allowed one arena combat per visit.  To fight an arena battle, select the option that best describes your abilities and return to me- if you survive:"
        break;

    case DIALOGUE_ARENA_FIGHT_NOT_OVER_YET:
        dialogue_string = localization->GetString(577); //"Get back in there you wimps:"
        break;

    case DIALOGUE_ARENA_REWARD:
        dialogue_string = localization->FormatString(576, gold_transaction_amount);// "Congratulations on your win: here's your stuff: %u gold."
        break;

    case DIALOGUE_ARENA_ALREADY_WON:
        dialogue_string = localization->GetString(582); // "You already won this trip to the Arena:"
        break;

    default:
        if (uDialogueType > DIALOGUE_18 && uDialogueType < DIALOGUE_EVT_E && !byte_5B0938[0])
        {
            dialogue_string = current_npc_text;
        }
        else if (pGreetType == 1)//QuestNPC_greet
        {
            if (pNPC->greet)
            {
                if ((pNPC->uFlags & 3) == 2)
                    dialogue_string = pNPCStats->pNPCGreetings[pNPC->greet].pGreeting2;
                else
                    dialogue_string = pNPCStats->pNPCGreetings[pNPC->greet].pGreeting1;
            }
        }
        else if (pGreetType == 2)//HiredNPC_greet
        {
            NPCProfession* prof = &pNPCStats->pProfessions[pNPC->uProfession];

            if (pNPC->Hired())
                dialogue_string = BuildDialogueString(prof->pDismissText, uActiveCharacter - 1, 0, 0, 0);
            else
                dialogue_string = BuildDialogueString(prof->pJoinText, uActiveCharacter - 1, 0, 0, 0);
        }
        break;
    }

    // Message window(Окно сообщения)---- 
    if (!dialogue_string.empty())
    {
        window.uFrameWidth = game_viewport_width;
        window.uFrameZ = 452;
        GUIFont* font = pFontArrus;
        pTextHeight = pFontArrus->CalcTextHeight(dialogue_string, &window, 13) + 7;
        if (352 - pTextHeight < 8)
        {
            font = pFontCreate;
            pTextHeight = pFontCreate->CalcTextHeight(dialogue_string, &window, 13) + 7;
        }

        if (ui_leather_mm7)
            render->DrawTextureCustomHeight(
                8 / 640.0f,
                (352 - pTextHeight) / 480.0f,
                ui_leather_mm7,
                pTextHeight);

        render->DrawTextureAlphaNew(8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
        pDialogueWindow->DrawText(font, 13, 354 - pTextHeight, 0, FitTextInAWindow(dialogue_string, font, &window, 13), 0, 0, 0);
    }

    // Right panel(Правая панель)------- 
    memcpy(&window, pDialogueWindow, sizeof(window));
    window.uFrameX = 483;
    window.uFrameWidth = 148;
    window.uFrameZ = 334;
    for (int i = window.pStartingPosActiveItem; i < window.pStartingPosActiveItem + window.pNumPresenceButton; ++i)
    {
        pButton = window.GetControl(i);
        if (!pButton)
            break;

        if (pButton->msg_param > 88)
            pButton->pButtonName[0] = 0;
        else if (pButton->msg_param == 88)
            strcpy(pButton->pButtonName, localization->GetString(581)); // Lord
        else if (pButton->msg_param == 87)
            strcpy(pButton->pButtonName, localization->GetString(580)); // Knight
        else if (pButton->msg_param == 86)
            strcpy(pButton->pButtonName, localization->GetString(579)); // Squire
        else if (pButton->msg_param == 85)
            strcpy(pButton->pButtonName, localization->GetString(578)); // Page
        else if (pButton->msg_param == 77)
            strcpy(pButton->pButtonName, localization->GetString(407)); // Details
        else if (pButton->msg_param == 76)
        {
            if (pNPC->Hired())
                sprintf(pButton->pButtonName, localization->GetString(408), pNPC->pName); // Release %s
            else
                strcpy(pButton->pButtonName, localization->GetString(406)); // Hire
        }
        else if (pButton->msg_param == 24)
        {
            __debugbreak(); // learn conditions of this event
            if (!pNPC->evt_F)
            {
                pButton->pButtonName[0] = 0;
                pButton->msg_param = 0;
            }
            else
                strcpy(pButton->pButtonName, pNPCTopics[pNPC->evt_F].pTopic);
        }
        else if (pButton->msg_param == 9)
            strcpy(pButton->pButtonName, GetProfessionActionText(pNPC->uProfession));
        else if (pButton->msg_param == 19) // Scavenger Hunt
        {
            if (!pNPC->evt_A)
            {
                pButton->pButtonName[0] = 0;
                pButton->msg_param = 0;
            }
            else
                strcpy(pButton->pButtonName, pNPCTopics[pNPC->evt_A].pTopic);
        }
        else if (pButton->msg_param == 20) // Scavenger Hunt
        {
            if (!pNPC->evt_B)
            {
                pButton->pButtonName[0] = 0;
                pButton->msg_param = 0;
            }
            else strcpy(pButton->pButtonName, pNPCTopics[pNPC->evt_B].pTopic);
        }
        else if (pButton->msg_param == 21)
        {
            //__debugbreak(); // learn conditions of this event
            if (!pNPC->evt_C)
            {
                pButton->pButtonName[0] = 0;
                pButton->msg_param = 0;
            }
            else strcpy(pButton->pButtonName, pNPCTopics[pNPC->evt_C].pTopic);
        }
        else if (pButton->msg_param == 22)
        {
            //__debugbreak(); // learn conditions of this event
            if (!pNPC->evt_D)
            {
                pButton->pButtonName[0] = 0;
                pButton->msg_param = 0;
            }
            else strcpy(pButton->pButtonName, pNPCTopics[pNPC->evt_D].pTopic);
        }
        else if (pButton->msg_param == 23)
        {
            //__debugbreak(); // learn conditions of this event
            if (!pNPC->evt_E)
            {
                pButton->pButtonName[0] = 0;
                pButton->msg_param = 0;
            }
            else strcpy(pButton->pButtonName, pNPCTopics[pNPC->evt_E].pTopic);
        }
        else if (pButton->msg_param == 13)
        {
            if (pNPC->Hired())
                sprintf(pButton->pButtonName, localization->GetString(408), pNPC->pName); // Release %s
            else
                strcpy(pButton->pButtonName, localization->GetString(122)); // Join
        }
        else
            pButton->pButtonName[0] = 0;

        if (pParty->field_7B5_in_arena_quest && pParty->field_7B5_in_arena_quest != -1)
        {
            int num_dead_actors = 0;
            for (uint i = 0; i < uNumActors; ++i)
            {
                if (pActors[i].uAIState == Dead || pActors[i].uAIState == Removed || pActors[i].uAIState == Disabled)
                    ++num_dead_actors;
                else
                {
                    int sumonner_type = PID_TYPE(pActors[i].uSummonerID);
                    if (sumonner_type == OBJECT_Player)
                        ++num_dead_actors;
                }
            }
            if (num_dead_actors == uNumActors)
                strcpy(pButton->pButtonName, localization->GetString(658)); // Collect Prize
        }
    }

    // Install Buttons(Установка кнопок)-------- 
    index = 0;
    all_text_height = 0;
    for (int i = pDialogueWindow->pStartingPosActiveItem;
    i < pDialogueWindow->pStartingPosActiveItem + pDialogueWindow->pNumPresenceButton; ++i)
    {
        pButton = pDialogueWindow->GetControl(i);
        if (!pButton)
            break;
        all_text_height += pFontArrus->CalcTextHeight(pButton->pButtonName, &window, 0);
        index++;
    }
    if (index)
    {
        v45 = (174 - all_text_height) / index;
        if (v45 > 32)
            v45 = 32;
        v42 = (174 - v45 * index - all_text_height) / 2 - v45 / 2 + 138;
        for (int i = pDialogueWindow->pStartingPosActiveItem;
        i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i)
        {
            pButton = pDialogueWindow->GetControl(i);
            if (!pButton)
                break;
            pButton->uY = (unsigned int)(v45 + v42);
            pTextHeight = pFontArrus->CalcTextHeight(pButton->pButtonName, &window, 0);
            pButton->uHeight = pTextHeight;
            v42 = pButton->uY + pTextHeight - 1;
            pButton->uW = v42;
            pTextColor = ui_game_dialogue_option_normal_color;
            if (pDialogueWindow->pCurrentPosActiveItem == i)
                pTextColor = ui_game_dialogue_option_highlight_color;
            window.DrawTitleText(pFontArrus, 0, pButton->uY, pTextColor, pButton->pButtonName, 3);
        }
    }
    render->DrawTextureAlphaNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);
}

//----- (00444FBE) --------------------------------------------------------
void GameUI_DrawBranchlessDialogue()
{
    int pTextHeight; // esi@4
    GUIWindow BranchlessDlg_window; // [sp+D4h] [bp-58h]@4
    GUIFont *pFont; // [sp+128h] [bp-4h]@1

    pFont = pFontArrus;
    if (current_npc_text.length() > 0 && !byte_5B0938[0])
        strcpy(byte_5B0938.data(), current_npc_text.c_str());
    BranchlessDlg_window.uFrameWidth = game_viewport_width;
    BranchlessDlg_window.uFrameZ = 452;
    pTextHeight = pFontArrus->CalcTextHeight(byte_5B0938.data(), &BranchlessDlg_window, 12) + 7;
    if (352 - pTextHeight < 8)
    {
        pFont = pFontCreate;
        pTextHeight = pFontCreate->CalcTextHeight(byte_5B0938.data(), &BranchlessDlg_window, 12) + 7;
    }

    render->DrawTextureCustomHeight(
        8 / 640.0f,
        (352 - pTextHeight) / 480.0f,
        ui_leather_mm7,
        pTextHeight);
    render->DrawTextureAlphaNew(8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
    pGUIWindow2->DrawText(pFont, 12, 354 - pTextHeight, 0, FitTextInAWindow(byte_5B0938.data(), pFont, &BranchlessDlg_window, 12), 0, 0, 0);
    render->DrawTextureNew(0, 352 / 480.0f, game_ui_statusbar);
    if (pGUIWindow2->receives_keyboard_input_2 != WINDOW_INPUT_IN_PROGRESS)
    {
        if (pGUIWindow2->receives_keyboard_input_2 == WINDOW_INPUT_CONFIRMED)
        {
            pGUIWindow2->receives_keyboard_input_2 = WINDOW_INPUT_NONE;
            GameUI_StatusBar_OnInput(pKeyActionMap->pPressedKeysBuffer);
            sub_4452BB();
            return;
        }
        if (pGUIWindow2->receives_keyboard_input_2 != WINDOW_INPUT_CANCELLED)
            return;
        pGUIWindow2->receives_keyboard_input_2 = WINDOW_INPUT_NONE;
        GameUI_StatusBar_ClearInputString();
        sub_4452BB();
        return;
    }
    if (pGUIWindow2->ptr_1C == (void *)26)
    {
        auto str = StringPrintf("%s %s", GameUI_StatusBar_GetInput().c_str(), pKeyActionMap->pPressedKeysBuffer);
        pGUIWindow2->DrawText(pFontLucida, 13, 357, 0, str, 0, 0, 0);
        pGUIWindow2->DrawFlashingInputCursor(pFontLucida->GetLineWidth(str) + 13, 357, pFontLucida);
        return;
    }
    if (pKeyActionMap->pPressedKeysBuffer[0])
    {
        pKeyActionMap->SetWindowInputStatus(WINDOW_INPUT_NONE);
        GameUI_StatusBar_ClearInputString();
        sub_4452BB();
        return;
    }
}

//----- (004443D5) --------------------------------------------------------
const char *GameUI_GetMinimapHintText()
{
  double v3; // st7@1
  int v7; // eax@4
  const char *v14; // eax@8
  char *result; // eax@12
  unsigned int pMapID; // eax@14
  int global_coord_X; // [sp+10h] [bp-1Ch]@1
  int global_coord_Y; // [sp+14h] [bp-18h]@1
  unsigned int pY; // [sp+1Ch] [bp-10h]@1
  unsigned int pX; // [sp+28h] [bp-4h]@1

  result = 0;
  pMouse->GetClickPos(&pX, &pY);
  v3 = 1.0 / (float)((signed int)viewparams->uMinimapZoom * 0.000015258789);
  global_coord_X = (signed __int64)((double)(pX - 557) * v3 + (double)pParty->vPosition.x);
  global_coord_Y = (signed __int64)((double)pParty->vPosition.y - (double)(pY - 74) * v3);
  if ( uCurrentlyLoadedLevelType != LEVEL_Outdoor || pOutdoor->uNumBModels <= 0 )
  {
    pMapID = pMapStats->GetMapInfo(pCurrentMapName);
    if ( pMapID == 0 )
      result = "No Maze Info for this maze on file!";
    else
      result = pMapStats->pInfos[pMapID].pName;
  }
  else
  {
    for ( uint j = 0; j < (uint)pOutdoor->uNumBModels; ++j )
    {
      v7 = int_get_vector_length(abs((signed)pOutdoor->pBModels[j].vBoundingCenter.x - global_coord_X),
                                 abs((signed)pOutdoor->pBModels[j].vBoundingCenter.y - global_coord_Y), 0);
      if ( v7 < 2 * pOutdoor->pBModels[j].sBoundingRadius )
      {
        if ( pOutdoor->pBModels[j].uNumFaces )
        {
          for ( uint i = 0; i < (uint)pOutdoor->pBModels[j].uNumFaces; ++i )
          {
            if ( pOutdoor->pBModels[j].pFaces[i].sCogTriggeredID )
            {
              if ( !(pOutdoor->pBModels[j].pFaces[i].uAttributes & FACE_HAS_EVENT) )
              {
                v14 = GetEventHintString(pOutdoor->pBModels[j].pFaces[i].sCogTriggeredID);
                if ( v14 )
                {
                  if ( _stricmp(v14, "") )
                    result = (char *)v14;
                }
              }
            }
          }
        }
        if ( result )
          return result;
      }
    }
    pMapID = pMapStats->GetMapInfo(pCurrentMapName);
    if ( pMapID == 0 )
      result = "No Maze Info for this maze on file!";
    else
      result = pMapStats->pInfos[pMapID].pName;
    return result;
  }
  return result;
}

//----- (0041D3B7) --------------------------------------------------------
void GameUI_CharacterQuickRecord_Draw(GUIWindow *window, Player *player)
{
    Image *v13; // eax@6
    PlayerFrame *v15; // eax@12
    const char *v29; // eax@16
    int v36; // esi@22
    signed int uFramesetID; // [sp+20h] [bp-8h]@9
    int uFramesetIDa; // [sp+20h] [bp-8h]@18

    uint numActivePlayerBuffs = 0;
    for (uint i = 0; i < 24; ++i)
    {
        if (player->pPlayerBuffs[i].Active() > 0)
            ++numActivePlayerBuffs;
    }

    window->uFrameHeight = ((pFontArrus->uFontHeight + 162) + ((numActivePlayerBuffs - 1) * pFontArrus->uFontHeight));
    window->uFrameZ = window->uFrameWidth + window->uFrameX - 1;
    window->uFrameW = ((pFontArrus->uFontHeight + 162) + ((numActivePlayerBuffs - 1) * pFontArrus->uFontHeight)) + window->uFrameY - 1;
    window->DrawMessageBox(0);

    if (player->IsEradicated())
        v13 = game_ui_player_face_eradicated;
    else if (player->IsDead())
        v13 = game_ui_player_face_dead;
    else
    {
        uFramesetID = pPlayerFrameTable->GetFrameIdByExpression(player->expression);
        if (!uFramesetID)
            uFramesetID = 1;
        if (player->expression == CHARACTER_EXPRESSION_21)
            v15 = pPlayerFrameTable->GetFrameBy_y(&player->_expression21_frameset, &player->_expression21_animtime, pMiscTimer->uTimeElapsed);
        else
            v15 = pPlayerFrameTable->GetFrameBy_x(uFramesetID, pMiscTimer->Time());
        player->field_1AA2 = v15->uTextureID - 1;
        v13 = game_ui_player_faces[(unsigned int)window->ptr_1C][v15->uTextureID - 1];
    }

    render->DrawTextureAlphaNew((window->uFrameX + 24) / 640.0f, (window->uFrameY + 24) / 480.0f, v13);

    auto str =
        StringPrintf("\f%05d", ui_character_header_text_color)
        + localization->FormatString(429, player->pName, localization->GetClassName(player->classType)) // "%s the %s"
        + "\f00000\n"
        + StringPrintf(
            "%s : \f%05u%d\f00000 / %d\n",
            localization->GetString(108), // "Hit Points"
            UI_GetHealthManaAndOtherQualitiesStringColor(player->sHealth, player->GetMaxHealth()),
            player->sHealth, player->GetMaxHealth()
        )
        + StringPrintf(
            "%s : \f%05u%d\f00000 / %d\n",
            localization->GetString(212), // "Spell Points"
            UI_GetHealthManaAndOtherQualitiesStringColor(player->sMana, player->GetMaxMana()),
            player->sMana, player->GetMaxMana()
        )
        + StringPrintf(
            "%s: \f%05d%s\f00000\n",
            localization->GetString(47), // Condition
            GetConditionDrawColor(player->GetMajorConditionIdx()),
            localization->GetCharacterConditionName(player->GetMajorConditionIdx())
        );

    if (player->uQuickSpell)
        v29 = pSpellStats->pInfos[player->uQuickSpell].pShortName;
    else
        v29 = localization->GetString(153);

    str += StringPrintf("%s: %s", localization->GetString(172), v29); // "Quick Spell"

    window->DrawText(pFontArrus, 120, 22, 0, str, 0, 0, 0);

    uFramesetIDa = 0;
    for (uint i = 0; i < 24; ++i)
    {
        SpellBuff* buff = &player->pPlayerBuffs[i];
        if (buff->Active())
        {
            v36 = uFramesetIDa++ * pFontComic->uFontHeight + 134;
            window->DrawText(pFontComic, 52, v36, ui_game_character_record_playerbuff_colors[i], localization->GetSpellName(20 + i), 0, 0, 0);
            DrawBuff_remaining_time_string(v36, window, buff->expire_time - pParty->GetPlayingTime(), pFontComic);
        }
    }

    auto active_spells = localization->FormatString(
        450,                                                  // Active Spells: %s
        uFramesetIDa == 0 ? localization->GetString(153) : "" // "None"
    );
    window->DrawText(pFontArrus, 14, 114, 0, active_spells, 0, 0, 0);
}

//----- (0041AD6E) --------------------------------------------------------
void GameUI_DrawRightPanelItems()
{
    if (GameUI_RightPanel_BookFlashTimer > pParty->GetPlayingTime())
		GameUI_RightPanel_BookFlashTimer = 0;

	static bool _50697C_book_flasher;
	
	if (pParty->GetPlayingTime() - GameUI_RightPanel_BookFlashTimer > 128)
	{
		GameUI_RightPanel_BookFlashTimer = pParty->GetPlayingTime();
		_50697C_book_flasher = !_50697C_book_flasher;
	}

    if (_50697C_book_flasher && current_screen_type != SCREEN_REST)
    {
        if (bFlashQuestBook)     render->DrawTextureAlphaNew(493 / 640.0f, 355 / 480.0f, game_ui_tome_quests);
        if (bFlashAutonotesBook) render->DrawTextureAlphaNew(527 / 640.0f, 353 / 480.0f, game_ui_tome_autonotes);
        if (bFlashHistoryBook)   render->DrawTextureAlphaNew(600 / 640.0f, 361 / 480.0f, game_ui_tome_storyline);
    }
   
}

//----- (0041AEBB) --------------------------------------------------------
void GameUI_DrawFoodAndGold()
{
  int text_y; // esi@2

  if ( uGameState != GAME_STATE_FINAL_WINDOW )
  {
    text_y = _44100D_should_alter_right_panel() != 0 ? 381 : 322;

    pPrimaryWindow->DrawText(pFontSmallnum, 0, text_y, uGameUIFontMain, StringPrintf("\r087%lu", pParty->uNumFoodRations), 0, 0, uGameUIFontShadow);
    pPrimaryWindow->DrawText(pFontSmallnum, 0, text_y, uGameUIFontMain, StringPrintf("\r028%lu", pParty->uNumGold), 0, 0, uGameUIFontShadow);
  }
}

//----- (0041B0C9) --------------------------------------------------------
void GameUI_DrawLifeManaBars()
{
  double v3; // st7@3
  double v7; // st7@25

  for (uint i = 0; i < 4; ++i)
  {
    if (pParty->pPlayers[i].sHealth > 0)
    {
      int v17 = 0;
      if (i == 2 || i == 3)
        v17 = 2;
      v3 = (double)pParty->pPlayers[i].sHealth / (double)pParty->pPlayers[i].GetMaxHealth();

      auto pTextureHealth = game_ui_bar_green;
      if( v3 > 0.5 )
      {
        if ( v3 > 1.0 )
          v3 = 1.0;
      }
      else if ( v3 > 0.25 )
        pTextureHealth = game_ui_bar_yellow;
      else if ( v3 > 0.0 )
        pTextureHealth = game_ui_bar_red;
      if( v3 > 0.0 )
      {
        render->SetUIClipRect(
            v17 + pHealthBarPos[i],
            (signed __int64)((1.0 - v3) * pTextureHealth->GetHeight()) + 402,
            v17 + pHealthBarPos[i] + pTextureHealth->GetWidth(),
            pTextureHealth->GetHeight() + 402);
        render->DrawTextureAlphaNew((v17 + pHealthBarPos[i])/640.0f, 402/480.0f, pTextureHealth);
        render->ResetUIClipRect();
      }
    }
    if (pParty->pPlayers[i].sMana > 0)
    {
      v7 = pParty->pPlayers[i].sMana / (double)pParty->pPlayers[i].GetMaxMana();
      if ( v7 > 1.0 )
        v7 = 1.0;
      int v17 = 0;
      if (i == 2)
        v17 = 1;
      render->SetUIClipRect(
          v17 + pManaBarPos[i],
          (signed __int64)((1.0 - v7) * game_ui_bar_blue->GetHeight()) + 402,
          v17 + pManaBarPos[i] + game_ui_bar_blue->GetWidth(),
          game_ui_bar_blue->GetHeight() + 402);
      render->DrawTextureAlphaNew((v17 + pManaBarPos[i])/640.0f, 402/480.0f, game_ui_bar_blue);
      render->ResetUIClipRect();
    }
  }
}

//----- (0041B3B6) --------------------------------------------------------
void GameUI_DrawRightPanel()
{
  render->DrawTextureAlphaNew(pViewport->uViewportBR_X/640.0f, 0, game_ui_right_panel_frame);
}

//----- (0041B3E2) --------------------------------------------------------
void GameUI_DrawRightPanelFrames()
{
  render->DrawTextureNew(0,   0, game_ui_topframe);
  render->DrawTextureNew(0,   8/480.0f, game_ui_leftframe);
  render->DrawTextureNew(468/640.0f, 0, game_ui_rightframe);
  render->DrawTextureNew(0,   352 / 480.0f, game_ui_bottomframe);
  GameUI_DrawFoodAndGold();
  GameUI_DrawRightPanelItems();

  //render->EndScene();
  //render->Present();
}


//----- (00420EFF) --------------------------------------------------------
void GameUI_WritePointedObjectStatusString()
{
    GUIWindow *pWindow; // edi@7
    GUIButton *pButton; // ecx@11
    int requiredSkillpoints; // ecx@19
    enum UIMessageType pMessageType1; // esi@24
    int v14; // eax@41
    ItemGen *pItemGen; // ecx@44
    int v16; // ecx@46
    signed int pickedObjectPID; // eax@55
    signed int v18b;
    signed int pickedObjectID; // ecx@63
    BLVFace *pFace; // eax@69
    const char *pText; // ecx@79
    enum UIMessageType pMessageType2; // esi@110
    enum UIMessageType pMessageType3; // edx@117
    unsigned int pX; // [sp+D4h] [bp-Ch]@1
    unsigned int pY; // [sp+D8h] [bp-8h]@1

    pMouse->uPointingObjectID = 0;
    pMouse->GetClickPos(&pX, &pY);
    if (pX < 0 || pX > window->GetWidth() - 1 || pY < 0 || pY > window->GetHeight() - 1)
        return;

    if (current_screen_type == SCREEN_GAME)
    {
        if (pX <= (window->GetWidth() - 1) * 0.73125 && pY <= (window->GetHeight() - 1) * 0.73125)
        {
            //if ( render->pRenderD3D )  // inlined mm8::4C1E01
            {
                pickedObjectPID = pEngine->pVisInstance->get_picked_object_zbuf_val();
                if (pX < (unsigned int)pViewport->uScreen_TL_X || pX >(unsigned int)pViewport->uScreen_BR_X
                    || pY < (unsigned int)pViewport->uScreen_TL_Y || pY >(unsigned int)pViewport->uScreen_BR_Y)
                    pickedObjectPID = -1;
                if (pickedObjectPID == -1)
                {
                    if (uLastPointedObjectID != 0)
                    {
                        game_ui_status_bar_string[0] = 0;
                        bForceDrawFooter = 1;
                    }
                    uLastPointedObjectID = 0;
                    return;
                }
            }
            /*else
            {
              v18 = render->pActiveZBuffer[pX + pSRZBufferLineOffsets[pY]];
            }*/
            pMouse->uPointingObjectID = (unsigned __int16)pickedObjectPID;
            pickedObjectID = (signed)PID_ID(pickedObjectPID);
            if (PID_TYPE(pickedObjectPID) == OBJECT_Item)
            {
                if (pObjectList->pObjects[pSpriteObjects[pickedObjectID].uObjectDescID].uFlags & 0x10)
                {
                    pMouse->uPointingObjectID = 0;
                    game_ui_status_bar_string[0] = 0;
                    bForceDrawFooter = 1;
                    uLastPointedObjectID = 0;
                    return;
                }
                if (pickedObjectPID >= 0x2000000u || pParty->pPickedItem.uItemID)
                {
                    GameUI_StatusBar_Set(pSpriteObjects[pickedObjectID].containing_item.GetDisplayName());
                }
                else
                {
                    GameUI_StatusBar_Set(
                        localization->FormatString(470, pSpriteObjects[pickedObjectID].containing_item.GetDisplayName()) // Get %s   does not display properly ??
                        );
                } //intentional fallthrough
            }
            else if (PID_TYPE(pickedObjectPID) == OBJECT_Decoration)
            {
                if (!pLevelDecorations[pickedObjectID].uEventID)
                {
                    if (pLevelDecorations[pickedObjectID].IsInteractive())
                        pText = pNPCTopics[stru_5E4C90_MapPersistVars._decor_events[pLevelDecorations[pickedObjectID]._idx_in_stru123 - 75] + 380].pTopic;//неверно для костра
                    else
                        pText = pDecorationList->pDecorations[pLevelDecorations[pickedObjectID].uDecorationDescID].field_20;
                    GameUI_StatusBar_Set(pText);
                }
                else
                {
                    char* hintString = GetEventHintString(pLevelDecorations[pickedObjectID].uEventID);
                    if (hintString != '\0')
                    {
                        GameUI_StatusBar_Set(hintString);
                    }
                } //intentional fallthrough
            }
            else if (PID_TYPE(pickedObjectPID) == OBJECT_BModel)
            {
                if (pickedObjectPID < 0x2000000u)
                {
                    char* newString = nullptr;
                    if (uCurrentlyLoadedLevelType != LEVEL_Indoor)
                    {
                        v18b = (signed int)(unsigned __int16)pickedObjectPID >> 9;
                        short triggeredId = pOutdoor->pBModels[v18b].pFaces[pickedObjectID & 0x3F].sCogTriggeredID;
                        if (triggeredId != 0)
                        {
                            newString = GetEventHintString(pOutdoor->pBModels[v18b].pFaces[pickedObjectID & 0x3F].sCogTriggeredID);
                        }
                    }
                    else
                    {
                        pFace = &pIndoor->pFaces[pickedObjectID];
                        if (pFace->uAttributes & FACE_INDICATE)
                        {
                            unsigned short eventId = pIndoor->pFaceExtras[pFace->uFaceExtraID].uEventID;
                            if (eventId != 0)
                            {
                                newString = GetEventHintString(pIndoor->pFaceExtras[pFace->uFaceExtraID].uEventID);
                            }
                        }
                    }
                    if (newString)
                    {
                        GameUI_StatusBar_Set(newString);
                        if (pMouse->uPointingObjectID == 0 && uLastPointedObjectID != 0)
                        {
                            game_ui_status_bar_string[0] = 0;
                            bForceDrawFooter = 1;
                        }
                        uLastPointedObjectID = pMouse->uPointingObjectID;
                        return;
                    }
                }
                pMouse->uPointingObjectID = 0;
                game_ui_status_bar_string[0] = 0;
                bForceDrawFooter = 1;
                uLastPointedObjectID = 0;
                return;
            }
            else if (PID_TYPE(pickedObjectPID) == OBJECT_Actor)
            {
                if (pickedObjectPID >= 0x2000000)
                {
                    pMouse->uPointingObjectID = 0;
                    if (uLastPointedObjectID != 0)
                    {
                        game_ui_status_bar_string[0] = 0;
                        bForceDrawFooter = 1;
                    }
                    uLastPointedObjectID = 0;
                    return;
                }
                if (pActors[pickedObjectID].dword_000334_unique_name)
                    pText = pMonsterStats->pPlaceStrings[pActors[pickedObjectID].dword_000334_unique_name];
                else
                    pText = pMonsterStats->pInfos[pActors[pickedObjectID].pMonsterInfo.uID].pName;
                GameUI_StatusBar_Set(pText); //intentional fallthrough
            }
            if (pMouse->uPointingObjectID == 0 && uLastPointedObjectID != 0)
            {
                game_ui_status_bar_string[0] = 0;
                bForceDrawFooter = 1;
            }
            uLastPointedObjectID = pMouse->uPointingObjectID;
            return;
        }
    }
	else if (current_screen_type == SCREEN_CHEST) {
		if (pX <= (window->GetWidth() - 1) * 0.73125 && pY <= (window->GetHeight() - 1) * 0.73125) { // if in chest area
			if (Chest::ChestUI_WritePointedObjectStatusString()) {
				return;
			}
			else if (uLastPointedObjectID != 0) { // not found so reset
				game_ui_status_bar_string[0] = 0;
				bForceDrawFooter = 1;
			}
			uLastPointedObjectID = 0;
			return;

		}
	}
	//else if (current_screen_type == SCREEN_CHEST_INVENTORY) {
		
	//}
    else {
		//if (pX <= (window->GetWidth() - 1) * 0.73125 && pY <= (window->GetHeight() - 1) * 0.73125) {
		if (current_screen_type == SCREEN_CHARACTERS) {
			if (current_character_screen_window == WINDOW_CharacterWindow_Inventory) {

				if (pY > 0 && pY < 350 && pX >= 13 && pX <= 462) {		// inventory poitned 

					//inventoryYCoord = (pY - 17) / 32;
					//inventoryXCoord = (pX - 14) / 32;
					//invMatrixIndex = inventoryXCoord + (INVETORYSLOTSWIDTH * inventoryYCoord);
					v14 = ((pX - 14) / 32) + 14 * ((pY - 17) / 32);
					//if (mouse.x <= 13 || mouse.x >= 462)
					//return;
					pickedObjectID = pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex(&v14);
					//if (!pItemID)
				//return;
			//item = &pPlayers[uActiveCharacter]->pInventoryItemList[pItemID - 1];

					//v14 = render->pActiveZBuffer[pX + pSRZBufferLineOffsets[pY]];
					if (pickedObjectID == 0 || pickedObjectID == -65536 || pickedObjectID >= 5000) {
						//if (pMouse->uPointingObjectID == 0) {
						if (uLastPointedObjectID != 0) {
							game_ui_status_bar_string[0] = 0;
							bForceDrawFooter = 1;
						}
						//}
						uLastPointedObjectID = 0;
						//return;
					}
					else {
						pItemGen = (ItemGen *)&pPlayers[uActiveCharacter]->pInventoryItemList[pickedObjectID - 1];
						GameUI_StatusBar_Set(pItemGen->GetDisplayName());
						uLastPointedObjectID = 1;
						return;
					}
				}
			}
		}

			//else if inventory


			for (int i = uNumVisibleWindows; i > 0; --i) {
				pWindow = pWindowList[pVisibleWindowsIdxs[i] - 1];
				if ((signed int)pX >= (signed int)pWindow->uFrameX && (signed int)pX <= (signed int)pWindow->uFrameZ
					&& (signed int)pY >= (signed int)pWindow->uFrameY && (signed int)pY <= (signed int)pWindow->uFrameW) {

					for (pButton = pWindow->pControlsHead; pButton != nullptr; pButton = pButton->pNext) {
						switch (pButton->uButtonType) {
						case 1://for dialogue window
							if ((signed int)pX >= (signed int)pButton->uX && (signed int)pX <= (signed int)pButton->uZ
								&& (signed int)pY >= (signed int)pButton->uY && (signed int)pY <= (signed int)pButton->uW) {

								pMessageType1 = (UIMessageType)pButton->field_1C;
								if (pMessageType1)
									pMessageQueue_50CBD0->AddGUIMessage(pMessageType1, pButton->msg_param, 0);
								GameUI_StatusBar_Set(pButton->pButtonName);
								uLastPointedObjectID = 1;
								return;
							}
							break;
						case 2://hovering over portraits
							if (pButton->uWidth != 0 && pButton->uHeight != 0) {
								uint distW = pX - pButton->uX;
								uint distY = pY - pButton->uY;

								double ratioX = 1.0 * (distW*distW) / (pButton->uWidth*pButton->uWidth);
								double ratioY = 1.0 * (distY*distY) / (pButton->uHeight*pButton->uHeight);

								if (ratioX + ratioY < 1.0) {
									pMessageType2 = (UIMessageType)pButton->field_1C;
									if (pMessageType2 != 0)
										pMessageQueue_50CBD0->AddGUIMessage(pMessageType2, pButton->msg_param, 0);
									GameUI_StatusBar_Set(pButton->pButtonName); // for character name
									uLastPointedObjectID = 1;
									return;
								}
							}
							break;
						case 3:// click on skill
							if (pX >= pButton->uX && pX <= pButton->uZ
								&& pY >= pButton->uY && pY <= pButton->uW) {
								requiredSkillpoints = (pPlayers[uActiveCharacter]->pActiveSkills[pButton->msg_param] & 0x3F) + 1;

								String str;
								if (pPlayers[uActiveCharacter]->uSkillPoints < requiredSkillpoints)
									str = localization->FormatString(469, requiredSkillpoints - pPlayers[uActiveCharacter]->uSkillPoints);// "You need %d more Skill Points to advance here"
								else
									str = localization->FormatString(468, requiredSkillpoints);// "Clicking here will spend %d Skill Points"

								GameUI_StatusBar_Set(str);
								uLastPointedObjectID = 1;
								return;
							}
							break;
						}
					}
				}


				// ?? if we get here nothing is curos over??
				if (uLastPointedObjectID != 0) { // not found so reset
					game_ui_status_bar_string[0] = 0;
					bForceDrawFooter = 1;
				}
				uLastPointedObjectID = 0;
				
				if (pWindow->uFrameHeight == 480)
				{
					//DebugBreak(); //Why is this condition here (in the original too)? Might check fullscreen windows. Let Silvo know if you find out

					// this is to stop the no windows active code below runnning


					return;
				}





			}
			//The game never gets to this point even in the original. It's also bugged(neither branch displays anything). 
			//TODO fix these and move them up before the window check loop.

			// gets here when glitched



		/*	if (current_screen_type == SCREEN_HOUSE)  // this is required when displaying inventory in a house/shop??
			{
				if (dialog_menu_id != HOUSE_DIALOGUE_SHOP_BUY_STANDARD
					|| (v16 = render->pActiveZBuffer[pX + pSRZBufferLineOffsets[pY]], v16 == 0)
					|| v16 == -65536)
				{
					if (uLastPointedObjectID != 0)
					{
						game_ui_status_bar_string[0] = 0;
						bForceDrawFooter = 1;
					}
					uLastPointedObjectID = 0;
					return;
				}
				pItemGen = (ItemGen *)((char *)&pParty->pPickedItem + 36 * (v16 + 12 * (unsigned int)window_SpeakInHouse->ptr_1C) + 4);
				GameUI_StatusBar_Set(pItemGen->GetDisplayName());
				game_ui_status_bar_string[0] = 0;
				bForceDrawFooter = 1;
				uLastPointedObjectID = 0;
				return;
			}
			*/



		//}
    }
 
	// no windows active -outside of game screen area only
	if ((signed int)pX >= (signed int)pWindowList[0]->uFrameX && (signed int)pX <= (signed int)pWindowList[0]->uFrameZ
        && (signed int)pY >= (signed int)pWindowList[0]->uFrameY && (signed int)pY <= (signed int)pWindowList[0]->uFrameW)
    {
        for (pButton = pWindowList[0]->pControlsHead; pButton != nullptr; pButton = pButton->pNext)
        {
            switch (pButton->uButtonType)
            {
            case 1:
                if ((signed int)pX >= (signed int)pButton->uX && (signed int)pX <= (signed int)pButton->uZ
                    && (signed int)pY >= (signed int)pButton->uY && (signed int)pY <= (signed int)pButton->uW)
                {
                    pMessageType3 = (UIMessageType)pButton->field_1C;
                    if (pMessageType3 == 0) // For books
                    {
                        GameUI_StatusBar_Set(pButton->pButtonName);
                    }
                    else
                    {
                        pMessageQueue_50CBD0->AddGUIMessage(pMessageType3, pButton->msg_param, 0);
                    }
                    uLastPointedObjectID = 1;
                    return;
                }
                break;
            case 2://hovering over portraits
                if (pButton->uWidth != 0 && pButton->uHeight != 0)
                {
                    uint distW = pX - pButton->uX;
                    uint distY = pY - pButton->uY;

                    double ratioX = 1.0 * (distW*distW) / (pButton->uWidth*pButton->uWidth);
                    double ratioY = 1.0 * (distY*distY) / (pButton->uHeight*pButton->uHeight);

                    if (ratioX + ratioY < 1.0)
                    {
                        pMessageType2 = (UIMessageType)pButton->field_1C;
                        if (pMessageType2 != 0)
                            pMessageQueue_50CBD0->AddGUIMessage(pMessageType2, pButton->msg_param, 0);
                        GameUI_StatusBar_Set(pButton->pButtonName); // for character name
                        uLastPointedObjectID = 1;
                        return;
                    }
                }
                break;
            case 3: // is this one needed?
/*                if (pX >= pButton->uX && pX <= pButton->uZ
                    && pY >= pButton->uY && pY <= pButton->uW)
                {
                    requiredSkillpoints = (pPlayers[uActiveCharacter]->pActiveSkills[pButton->msg_param] & 0x3F) + 1;

                    String str;
                    if (pPlayers[uActiveCharacter]->uSkillPoints < requiredSkillpoints)
                        str = localization->FormatString(469, requiredSkillpoints - pPlayers[uActiveCharacter]->uSkillPoints);// "You need %d more Skill Points to advance here"
                    else
                        str = localization->FormatString(468, requiredSkillpoints);// "Clicking here will spend %d Skill Points"
                    GameUI_StatusBar_Set(str);
                    uLastPointedObjectID = 1;
                    return;
                }*/
                break;
            }
        }
    }
	
    //pMouse->uPointingObjectID = sub_46A99B(); //for software
    if (uLastPointedObjectID != 0)
    {
        game_ui_status_bar_string[0] = 0;
        bForceDrawFooter = 1;
    }
    uLastPointedObjectID = 0;
    return;
}

//----- (0044158F) --------------------------------------------------------
void GameUI_DrawCharacterSelectionFrame()
{
  if ( uActiveCharacter )
    render->DrawTextureAlphaNew(
        (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[uActiveCharacter - 1] - 9)/640.0f,
        380/480.0f,
        game_ui_player_selection_frame);
}

//----- (0044162D) --------------------------------------------------------
void GameUI_DrawPartySpells()
{
    unsigned int v0; // ebp@1
    Image *spell_texture; // [sp-4h] [bp-1Ch]@12
    //Texture_MM7 *v9; // [sp-4h] [bp-1Ch]@21

    v0 = OS_GetTime() / 20;
    for (uint i = 0; i < 14; ++i)
    {
        if (pParty->pPartyBuffs[byte_4E5DD8[i]].Active())
        {
            render->_4A65CC(
                pPartySpellbuffsUI_XYs[i][0],
                pPartySpellbuffsUI_XYs[i][1], party_buff_icons[i], party_buff_icons[i],
                v0 + 20 * pPartySpellbuffsUI_smthns[i], 0, 63
            );
        }
    }

    if (current_screen_type == SCREEN_GAME || current_screen_type == SCREEN_NPC_DIALOGUE)
    {
        if (pParty->FlyActive())
        {
            if (pParty->bFlying)
                spell_texture = pIconsFrameTable->GetFrame(uIconIdx_FlySpell, v0)->GetTexture();
            else
                spell_texture = pIconsFrameTable->GetFrame(uIconIdx_FlySpell, 0)->GetTexture();
            render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f, spell_texture);
        }
        if (pParty->WaterWalkActive())
        {
            if (pParty->uFlags & PARTY_FLAGS_1_STANDING_ON_WATER)
                spell_texture = pIconsFrameTable->GetFrame(uIconIdx_WaterWalk, v0)->GetTexture();
            else
                spell_texture = pIconsFrameTable->GetFrame(uIconIdx_WaterWalk, 0)->GetTexture();
            render->DrawTextureAlphaNew(396 / 640.0f, 8 / 480.0f, spell_texture);
        }
    }

    for (uint i = 0; i < 4; ++i)
    {
        if (pParty->pPlayers[i].pPlayerBuffs[PLAYER_BUFF_HAMMERHANDS].Active())
            render->DrawTextureAlphaNew((pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72) / 640.0f, 427 / 480.0f, game_ui_playerbuff_hammerhands);
        if (pParty->pPlayers[i].pPlayerBuffs[PLAYER_BUFF_BLESS].Active())
            render->DrawTextureAlphaNew((pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72) / 640.0f, 393 / 480.0f, game_ui_playerbuff_bless);
        if (pParty->pPlayers[i].pPlayerBuffs[PLAYER_BUFF_PRESERVATION].Active())
            render->DrawTextureAlphaNew((pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72) / 640.0f, 410 / 480.0f, game_ui_playerbuff_preservation);
        if (pParty->pPlayers[i].pPlayerBuffs[PLAYER_BUFF_PAIN_REFLECTION].Active())
            render->DrawTextureAlphaNew((pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72) / 640.0f, 444 / 480.0f, game_ui_playerbuff_pain_reflection);
    }
}

//----- (004921C1) --------------------------------------------------------
void GameUI_DrawPortraits(unsigned int _this)
{
    unsigned int face_expression_ID; // eax@17
    PlayerFrame *pFrame; // eax@21
    Image *pPortrait; // [sp-4h] [bp-1Ch]@27

    if (_A750D8_player_speech_timer)
    {
        _A750D8_player_speech_timer -= (signed int)pMiscTimer->uTimeElapsed;
        if (_A750D8_player_speech_timer <= 0)
        {
            if (pPlayers[uSpeakingCharacter]->CanAct())
                pPlayers[uSpeakingCharacter]->PlaySound(PlayerSpeechID, 0);
            _A750D8_player_speech_timer = 0i64;
        }
    }

    for (uint i = 0; i < 4; ++i)
    {
        Player* pPlayer = &pParty->pPlayers[i];
        if (pPlayer->IsEradicated())
        {
            pPortrait = game_ui_player_face_eradicated;
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active())
                render->DrawTextureGrayShade(pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] / 640.0f, 388 / 480.0f, pPortrait);
            else
                render->DrawTextureAlphaNew((pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 1) / 640.0f, 388 / 480.0f, pPortrait);
            if (
                pPlayer->pPlayerBuffs[PLAYER_BUFF_BLESS].Active()
                || pPlayer->pPlayerBuffs[PLAYER_BUFF_HASTE].Active()
                || pPlayer->pPlayerBuffs[PLAYER_BUFF_HEROISM].Active()
                || pPlayer->pPlayerBuffs[PLAYER_BUFF_SHIELD].Active()
                || pPlayer->pPlayerBuffs[PLAYER_BUFF_STONESKIN].Active()
            )
            {
                _441A4E_overlay_on_portrait(i);
            }
            continue;
        }
        if (pPlayer->IsDead())
        {
            pPortrait = game_ui_player_face_dead;
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active())
                render->DrawTextureGrayShade(pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] / 640.0f, 388 / 480.0f, pPortrait);
            else
                render->DrawTextureAlphaNew((pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 1) / 640.0f, 388 / 480.0f, pPortrait);
            if (
                pPlayer->pPlayerBuffs[PLAYER_BUFF_BLESS].Active()
                || pPlayer->pPlayerBuffs[PLAYER_BUFF_HASTE].Active()
                || pPlayer->pPlayerBuffs[PLAYER_BUFF_HEROISM].Active()
                || pPlayer->pPlayerBuffs[PLAYER_BUFF_SHIELD].Active()
                || pPlayer->pPlayerBuffs[PLAYER_BUFF_STONESKIN].Active()
            )
            {
                _441A4E_overlay_on_portrait(i);
            }
            continue;
        }
        face_expression_ID = 0;
        for (uint j = 0; j < pPlayerFrameTable->uNumFrames; ++j)
            if (pPlayerFrameTable->pFrames[j].expression == pPlayer->expression)
            {
                face_expression_ID = j;
                break;
            }
        if (face_expression_ID == 0)
            face_expression_ID = 1;
        if (pPlayer->expression == CHARACTER_EXPRESSION_21)
            pFrame = pPlayerFrameTable->GetFrameBy_y(&pPlayer->_expression21_frameset, &pPlayer->_expression21_animtime, pMiscTimer->uTimeElapsed);
        else
            pFrame = pPlayerFrameTable->GetFrameBy_x(face_expression_ID, pPlayer->uExpressionTimePassed);
        if (pPlayer->field_1AA2 != pFrame->uTextureID - 1 || _this)
        {
            pPlayer->field_1AA2 = pFrame->uTextureID - 1;
            pPortrait = game_ui_player_faces[i][pPlayer->field_1AA2];//pFace = (Texture_MM7 *)game_ui_player_faces[i][pFrame->uTextureID];
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active())
                render->DrawTextureGrayShade(pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] / 640.0f, 388 / 480.0f, pPortrait);
            else
                render->DrawTextureAlphaNew((pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 1) / 640.0f, 388 / 480.0f, pPortrait);
            if (
                pPlayer->pPlayerBuffs[PLAYER_BUFF_BLESS].Active()
                | pPlayer->pPlayerBuffs[PLAYER_BUFF_HASTE].Active()
                | pPlayer->pPlayerBuffs[PLAYER_BUFF_HEROISM].Active()
                | pPlayer->pPlayerBuffs[PLAYER_BUFF_SHIELD].Active()
                | pPlayer->pPlayerBuffs[PLAYER_BUFF_STONESKIN].Active()
            )
            {
                _441A4E_overlay_on_portrait(i);
            }
            continue;
        }
    }
    if (pParty->bTurnBasedModeOn == 1)
    {
        if (pTurnEngine->turn_stage != TE_WAIT)
        {
            if (PID_TYPE(pTurnEngine->pQueue[0].uPackedID) == OBJECT_Player)
            {
                if (pTurnEngine->uActorQueueSize > 0)
                {
                    for (uint i = 0; i < (uint)pTurnEngine->uActorQueueSize; ++i)
                    {
                        if (PID_TYPE(pTurnEngine->pQueue[i].uPackedID) != OBJECT_Player)
                            break;

                        auto alert_texture = game_ui_player_alert_green;
                        if (pParty->GetRedAlert())
                            alert_texture = game_ui_player_alert_red;
                        else if (pParty->GetYellowAlert())
                            alert_texture = game_ui_player_alert_yellow;

                        render->DrawTextureAlphaNew(
                            (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[PID_ID(pTurnEngine->pQueue[i].uPackedID)] - 4) / 640.0f,
                            385 / 480.0f,
                            alert_texture
                        );
                    }
                }
            }
        }
    }
    else
    {
        for (uint i = 0; i < 4; ++i)
        {
            if (pParty->pPlayers[i].CanAct() && !pParty->pPlayers[i].uTimeToRecovery)
            {
                auto alert_texture = game_ui_player_alert_green;
                if (pParty->GetRedAlert())
                    alert_texture = game_ui_player_alert_red;
                else if (pParty->GetYellowAlert())
                    alert_texture = game_ui_player_alert_yellow;

                render->DrawTextureAlphaNew(
                    (pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] - 4) / 640.0f,
                    385 / 480.0f,
                    alert_texture
                );
            }
        }
    }
}

//----- (00441D38) --------------------------------------------------------
void GameUI_DrawMinimap(unsigned int uX, unsigned int uY, unsigned int uZ, unsigned int uW, unsigned int uZoom, unsigned int bRedrawOdmMinimap)
{
    int uHeight; // ebx@6
    signed int pW; // ebx@23
    int v15; // eax@23
    double v20; // st7@30
    signed int v27; // eax@37
    //unsigned __int16 *v28; // ecx@37
    signed int v29; // edi@40
    int pPoint_X; // edi@72
    int pPoint_Y; // ebx@72
    unsigned int lPitch; // [sp+20h] [bp-34h]@1
    signed int pY; // [sp+20h] [bp-34h]@23
    signed int pX; // [sp+24h] [bp-30h]@23
    signed int v70; // [sp+24h] [bp-30h]@37
    signed int uBluea; // [sp+28h] [bp-2Ch]@37
    int v73; // [sp+2Ch] [bp-28h]@30
    signed int uCenterY; // [sp+48h] [bp-Ch]@1
    signed int uCenterX; // [sp+4Ch] [bp-8h]@1
    signed int uWidth; // [sp+5Ch] [bp+8h]@30
    signed int pZ; // [sp+60h] [bp+Ch]@23
    float uWb; // [sp+60h] [bp+Ch]@30
    unsigned int pColor;

    uCenterX = (uX + uZ) / 2;
    uCenterY = (uY + uW) / 2;
    lPitch = render->uTargetSurfacePitch;
    bool bWizardEyeActive = pParty->WizardEyeActive();
    int uWizardEyeSkillLevel = pParty->WizardEyeSkillLevel();
    if (CheckHiredNPCSpeciality(Cartographer))
    {
        bWizardEyeActive = true;
        uWizardEyeSkillLevel = 2;
    }

    if (wizard_eye)
    {
        bWizardEyeActive = true;
        uWizardEyeSkillLevel = 3;
    }
    render->SetRasterClipRect(uX, uY, uZ - 1, uW - 1);
    uHeight = uW - uY;
    uWidth = uZ - uX;

    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
    {
        v73 = (1 << (ImageHelper::GetWidthLn2(viewparams->location_minimap) + 16)) / (signed int)uZoom;
        v20 = (double)(pParty->vPosition.x + 32768) / (double)(1 << (16 - ImageHelper::GetWidthLn2(viewparams->location_minimap)));
        uWb = (double)(32768 - pParty->vPosition.y) / (double)(1 << (16 - ImageHelper::GetWidthLn2(viewparams->location_minimap)));
        switch (uZoom)
        {
            case 512:
            {
                v20 = v20 - (double)(uWidth / 2);
                uWb = uWb - (double)(uHeight / 2);
            }
            break;
            case 1024:
            {
                v20 = v20 - (double)(uWidth / 4);
                uWb = uWb - (double)(uHeight / 4);
            }
            break;
            case 2048:
            {
                v20 = v20 - (double)(uWidth / 8);
                uWb = uWb - (double)(uHeight / 8);
            }
            break;
            default: assert(false);
        }

        static unsigned __int16 pOdmMinimap[117][137];
        assert(sizeof(pOdmMinimap) == 137 * 117 * sizeof(short));

        v70 = floorf(v20 * 65536.0 + 0.5f);//LODWORD(v24);
        uBluea = floorf(uWb * 65536.0 + 0.5f);//LODWORD(v25);
        v27 = uBluea >> 16;
        //v28 = &render->pTargetSurface[uX + uY * lPitch];

        if (/*pMapLod0 && */bRedrawOdmMinimap)
        {
            assert(uWidth == 137 && uHeight == 117);

            ushort mapWidth = viewparams->location_minimap->GetWidth();

            v29 = v70 >> 16;
            for (int y = 0; y < uHeight; ++y)
            {
                //uchar* pMapLod0Line = &pMapLod0[v27 * mapWidth];
                auto pMapLod0Line = (unsigned __int16 *)viewparams->location_minimap->GetPixels(IMAGE_FORMAT_R5G6B5) + v27 * mapWidth;
                for (int x = 0; x < uWidth; ++x)
                {
                    //*pMinimap++ = pPal[pMapLod0Line[v29]];
                    render->WritePixel16(uX + x, uY + y, pMapLod0Line[v29]);
                    v29 = (v70 + x * v73) >> 16;
                }
                uBluea += v73;
                v27 = uBluea >> 16;
            }
        }

        uNumBlueFacesInBLVMinimap = 0;
    }
    else// uCurrentlyLoadedLevelType == LEVEL_Indoor
    {
        render->FillRectFast(uX, uY, uZ - uX, uHeight, 0xF);
        uNumBlueFacesInBLVMinimap = 0;

        for (uint i = 0; i < (uint)pIndoor->pMapOutlines->uNumOutlines; ++i)
        {
            BLVMapOutline* pOutline = &pIndoor->pMapOutlines->pOutlines[i];
            //BLVFace* pFace1 = &pIndoor->pFaces[pOutline->uFace1ID];
            //BLVFace* pFace2 = &pIndoor->pFaces[pOutline->uFace2ID];
            if (pIndoor->pFaces[pOutline->uFace1ID].Visible() && pIndoor->pFaces[pOutline->uFace2ID].Visible())
            {
                if (pOutline->uFlags & 1)
                {
                    if (bWizardEyeActive && uWizardEyeSkillLevel >= 3 &&
                        (pIndoor->pFaces[pOutline->uFace1ID].Clickable() || pIndoor->pFaces[pOutline->uFace2ID].Clickable()) &&
                        (pIndoor->pFaceExtras[pIndoor->pFaces[pOutline->uFace1ID].uFaceExtraID].uEventID
                            || pIndoor->pFaceExtras[pIndoor->pFaces[pOutline->uFace2ID].uFaceExtraID].uEventID))
                    {
                        if (uNumBlueFacesInBLVMinimap < 49)
                            pBlueFacesInBLVMinimapIDs[uNumBlueFacesInBLVMinimap++] = i;
                    }
                    else
                    {
                        pX = uCenterX + ((signed int)(((unsigned int)(fixpoint_mul(uZoom, pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[i].uVertex1ID].x)) << 16) - uZoom * pParty->vPosition.x) >> 16);
                        pY = uCenterY - ((signed int)(((unsigned int)(fixpoint_mul(uZoom, pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[i].uVertex1ID].y)) << 16) - uZoom * pParty->vPosition.y) >> 16);
                        pZ = uCenterX + ((signed int)(((unsigned int)(fixpoint_mul(uZoom, pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[i].uVertex2ID].x)) << 16) - uZoom * pParty->vPosition.x) >> 16);
                        pW = uCenterY - ((signed int)(((unsigned int)(fixpoint_mul(uZoom, pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[i].uVertex2ID].y)) << 16) - uZoom * pParty->vPosition.y) >> 16);
                        v15 = abs(pOutline->sZ - pParty->vPosition.z) / 8;
                        if (v15 > 100)
                            v15 = 100;
                        render->RasterLine2D(pX, pY, pZ, pW, viewparams->pPalette[-v15 + 200]);
                    }
                    continue;
                }
                if (pIndoor->pFaces[pOutline->uFace1ID].uAttributes & FACE_UNKNOW7
                    || pIndoor->pFaces[pOutline->uFace2ID].uAttributes & FACE_UNKNOW7)
                {
                    pOutline->uFlags = pOutline->uFlags | 1;
                    pIndoor->_visible_outlines[i >> 3] |= 1 << (7 - i % 8);
                    if (bWizardEyeActive && uWizardEyeSkillLevel >= 3 &&
                        (pIndoor->pFaces[pOutline->uFace1ID].Clickable() || pIndoor->pFaces[pOutline->uFace2ID].Clickable()) &&
                        (pIndoor->pFaceExtras[pIndoor->pFaces[pOutline->uFace1ID].uFaceExtraID].uEventID || pIndoor->pFaceExtras[pIndoor->pFaces[pOutline->uFace2ID].uFaceExtraID].uEventID))
                    {
                        if (uNumBlueFacesInBLVMinimap < 49)
                            pBlueFacesInBLVMinimapIDs[uNumBlueFacesInBLVMinimap++] = i;
                    }
                    else
                    {
                        pX = uCenterX + ((signed int)(((unsigned int)(fixpoint_mul(uZoom, pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[i].uVertex1ID].x)) << 16) - uZoom * pParty->vPosition.x) >> 16);
                        pY = uCenterY - ((signed int)(((unsigned int)(fixpoint_mul(uZoom, pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[i].uVertex1ID].y)) << 16) - uZoom * pParty->vPosition.y) >> 16);
                        pZ = uCenterX + ((signed int)(((unsigned int)(fixpoint_mul(uZoom, pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[i].uVertex2ID].x)) << 16) - uZoom * pParty->vPosition.x) >> 16);
                        pW = uCenterY - ((signed int)(((unsigned int)(fixpoint_mul(uZoom, pIndoor->pVertices[pIndoor->pMapOutlines->pOutlines[i].uVertex2ID].y)) << 16) - uZoom * pParty->vPosition.y) >> 16);
                        v15 = abs(pOutline->sZ - pParty->vPosition.z) / 8;
                        if (v15 > 100)
                            v15 = 100;
                        render->RasterLine2D(pX, pY, pZ, pW, viewparams->pPalette[-v15 + 200]);
                    }
                    continue;
                }
            }
        }

        for (uint i = 0; i < uNumBlueFacesInBLVMinimap; ++i)
        {
            BLVMapOutline* pOutline = &pIndoor->pMapOutlines->pOutlines[pBlueFacesInBLVMinimapIDs[i]];
            pX = uCenterX + ((signed int)(((unsigned int)(fixpoint_mul(uZoom, pIndoor->pVertices[pOutline->uVertex1ID].x)) << 16) - uZoom * pParty->vPosition.x) >> 16);
            pY = uCenterY - ((signed int)(((unsigned int)(fixpoint_mul(uZoom, pIndoor->pVertices[pOutline->uVertex1ID].y)) << 16) - uZoom * pParty->vPosition.y) >> 16);
            pZ = uCenterX + ((signed int)(((unsigned int)(fixpoint_mul(uZoom, pIndoor->pVertices[pOutline->uVertex2ID].x)) << 16) - uZoom * pParty->vPosition.x) >> 16);
            pW = uCenterY - ((signed int)(((unsigned int)(fixpoint_mul(uZoom, pIndoor->pVertices[pOutline->uVertex2ID].y)) << 16) - uZoom * pParty->vPosition.y) >> 16);
            render->RasterLine2D(pX, pY, pZ, pW, ui_game_minimap_outline_color);
        }
    }

    //draw arrow on the minimap(include. Ritor1)
    uint arrow_idx;
    unsigned int rotate = pParty->sRotationY & stru_5C6E00->uDoublePiMask;
    if ((signed int)rotate <= 1920)
        arrow_idx = 6;
    if ((signed int)rotate < 1664)
        arrow_idx = 5;
    if ((signed int)rotate <= 1408)
        arrow_idx = 4;
    if ((signed int)rotate < 1152)
        arrow_idx = 3;
    if ((signed int)rotate <= 896)
        arrow_idx = 2;
    if ((signed int)rotate < 640)
        arrow_idx = 1;
    if ((signed int)rotate <= 384)
        arrow_idx = 0;
    if ((signed int)rotate < 128 || (signed int)rotate > 1920)
        arrow_idx = 7;
    render->DrawTextureAlphaNew(
        (uCenterX - 3) / 640.0f,
        (uCenterY - 3) / 480.0f,
        game_ui_minimap_dirs[arrow_idx]
    );

    //draw objects on the minimap
    if (bWizardEyeActive)
    {
        if (uWizardEyeSkillLevel >= 2)
        {
            for (uint i = 0; i < uNumSpriteObjects; ++i)
            {
                if (!pSpriteObjects[i].uType || !pSpriteObjects[i].uObjectDescID)
                    continue;
                //if (uWizardEyeSkillLevel == 1
                pPoint_X = uCenterX + fixpoint_mul((pSpriteObjects[i].vPosition.x - pParty->vPosition.x), uZoom);
                pPoint_Y = uCenterY - fixpoint_mul((pSpriteObjects[i].vPosition.y - pParty->vPosition.y), uZoom);
                //if ( pPoint_X >= render->raster_clip_x && pPoint_X <= render->raster_clip_z &&
                //     pPoint_Y >= render->raster_clip_y && pPoint_Y <= render->raster_clip_w)
                {
                    if (pObjectList->pObjects[pSpriteObjects[i].uObjectDescID].uFlags & OBJECT_DESC_UNPICKABLE)
                    {
                        render->RasterLine2D(pPoint_X, pPoint_Y, pPoint_X, pPoint_Y, ui_game_minimap_projectile_color);
                    }
                    else if (uZoom > 512)
                    {
                        render->RasterLine2D(pPoint_X - 2, pPoint_Y, pPoint_X - 2, pPoint_Y + 1, ui_game_minimap_treasure_color);
                        render->RasterLine2D(pPoint_X - 1, pPoint_Y - 1, pPoint_X - 1, pPoint_Y + 1, ui_game_minimap_treasure_color);
                        render->RasterLine2D(pPoint_X, pPoint_Y - 2, pPoint_X, pPoint_Y + 1, ui_game_minimap_treasure_color);
                        render->RasterLine2D(pPoint_X + 1, pPoint_Y - 1, pPoint_X + 1, pPoint_Y + 1, ui_game_minimap_treasure_color);
                        render->RasterLine2D(pPoint_X + 2, pPoint_Y, pPoint_X + 2, pPoint_Y + 1, ui_game_minimap_treasure_color);
                    }
                    else
                    {
                        render->RasterLine2D(pPoint_X - 1, pPoint_Y - 1, pPoint_X - 1, pPoint_Y, ui_game_minimap_treasure_color);
                        render->RasterLine2D(pPoint_X, pPoint_Y - 1, pPoint_X, pPoint_Y, ui_game_minimap_treasure_color);
                    }
                }
            }
        }
        for (uint i = 0; i < uNumActors; ++i)//draw actors(отрисовка монстров и нпс)
        {
            if (pActors[i].uAIState != Removed && pActors[i].uAIState != Disabled
                && (pActors[i].uAIState == Dead || pActors[i].ActorNearby()))
            {
                pPoint_X = uCenterX + (fixpoint_mul((pActors[i].vPosition.x - pParty->vPosition.x), uZoom));
                pPoint_Y = uCenterY - (fixpoint_mul((pActors[i].vPosition.y - pParty->vPosition.y), uZoom));
                //if ( pPoint_X >= render->raster_clip_x && pPoint_X <= render->raster_clip_z
                //  && pPoint_Y >= render->raster_clip_y && pPoint_Y <= render->raster_clip_w )
                {
                    pColor = ui_game_minimap_actor_friendly_color;
                    if (pActors[i].uAttributes & ACTOR_HOSTILE)
                        pColor = ui_game_minimap_actor_hostile_color;
                    if (pActors[i].uAIState == Dead)
                        pColor = ui_game_minimap_actor_corpse_color;
                    if (uZoom > 1024)
                    {
                        render->RasterLine2D(pPoint_X - 2, pPoint_Y - 1, pPoint_X - 2, pPoint_Y + 1, pColor);
                        render->RasterLine2D(pPoint_X - 1, pPoint_Y - 2, pPoint_X - 1, pPoint_Y + 2, pColor);
                        render->RasterLine2D(pPoint_X, pPoint_Y - 2, pPoint_X, pPoint_Y + 2, pColor);
                        render->RasterLine2D(pPoint_X + 1, pPoint_Y - 2, pPoint_X + 1, pPoint_Y + 2, pColor);
                        render->RasterLine2D(pPoint_X + 2, pPoint_Y - 1, pPoint_X + 2, pPoint_Y + 1, pColor);
                    }
                    else
                    {
                        render->RasterLine2D(pPoint_X - 1, pPoint_Y - 1, pPoint_X - 1, pPoint_Y, pColor);
                        render->RasterLine2D(pPoint_X, pPoint_Y - 1, pPoint_X, pPoint_Y, pColor);
                    }
                }
            }
        }
        for (uint i = 0; i < (signed int)uNumLevelDecorations; ++i)//draw items(отрисовка предметов)
        {
            if (pLevelDecorations[i].uFlags & 8)
            {
                pPoint_X = uCenterX + (fixpoint_mul((pLevelDecorations[i].vPosition.x - pParty->vPosition.x), uZoom));
                pPoint_Y = uCenterY - (fixpoint_mul((pLevelDecorations[i].vPosition.y - pParty->vPosition.y), uZoom));
                //if ( pPoint_X >= render->raster_clip_x && pPoint_X <= render->raster_clip_z
                //  && pPoint_Y >= render->raster_clip_y && pPoint_Y <= render->raster_clip_w )
                {
                    if ((signed int)uZoom > 512)
                    {
                        render->RasterLine2D(pPoint_X - 1, pPoint_Y - 1, pPoint_X - 1, pPoint_Y + 1, ui_game_minimap_decoration_color_1);
                        render->RasterLine2D(pPoint_X, pPoint_Y - 1, pPoint_X, pPoint_Y + 1, ui_game_minimap_decoration_color_1);
                        render->RasterLine2D(pPoint_X + 1, pPoint_Y - 1, pPoint_X + 1, pPoint_Y + 1, ui_game_minimap_decoration_color_1);
                    }
                    else
                        render->RasterLine2D(pPoint_X, pPoint_Y, pPoint_X, pPoint_Y, ui_game_minimap_decoration_color_1);
                }
            }
        }
    }
    render->DrawTextureAlphaNew(468 / 640.0f, 0, game_ui_minimap_frame);
    render->SetUIClipRect(541, 0, 567, 480);
    render->DrawTextureAlphaNew(
        (floorf(((double)pParty->sRotationY * 0.1171875) + 0.5f) + 285) / 640.0f,
        136 / 480.0f,
        game_ui_minimap_compass);
    render->ResetUIClipRect();
}

//----- (00441498) --------------------------------------------------------
void  GameUI_DrawTorchlightAndWizardEye()
{
    if (current_screen_type == SCREEN_GAME
        || current_screen_type == SCREEN_MENU
        || current_screen_type == SCREEN_OPTIONS
        || current_screen_type == SCREEN_REST
        || current_screen_type == SCREEN_SPELL_BOOK
        || current_screen_type == SCREEN_CHEST
        || current_screen_type == SCREEN_SAVEGAME
        || current_screen_type == SCREEN_LOADGAME
        || current_screen_type == SCREEN_CHEST_INVENTORY
        || current_screen_type == SCREEN_BOOKS
        || current_screen_type == SCREEN_BRANCHLESS_NPC_DIALOG
    )
    {
        if (pParty->TorchlightActive())
        {
            render->DrawTextureAlphaNew(
                pUIAnum_Torchlight->x / 640.0f,
                pUIAnum_Torchlight->y / 480.0f,
                pIconsFrameTable->GetFrame(pUIAnum_Torchlight->icon->id, pEventTimer->Time())->GetTexture()
            );
        }
        if (pParty->WizardEyeActive())
        {
            render->DrawTextureAlphaNew(
                pUIAnim_WizardEye->x / 640.0f,
                pUIAnim_WizardEye->y / 480.0f,
                pIconsFrameTable->GetFrame(pUIAnim_WizardEye->icon->id, pEventTimer->Time())->GetTexture()
            );
        }
    }
}


//----- (00491F87) --------------------------------------------------------
void GameUI_DrawHiredNPCs()
{
    unsigned int v13; // eax@23
    char pContainer[20]; // [sp+Ch] [bp-30h]@18
    signed int uFrameID; // [sp+24h] [bp-18h]@19
    int v22; // [sp+34h] [bp-8h]@2
    unsigned __int8 pNPC_limit_ID; // [sp+3Bh] [bp-1h]@2

    char buf[4096];
    if (bNoNPCHiring != 1)
    {
        pNPC_limit_ID = 0;
        v22 = 0;
        if (pParty->pHirelings[0].pName)
            buf[v22++] = 0;
        if (pParty->pHirelings[1].pName)
            buf[v22++] = 1;

        for (uint i = 0; i < pNPCStats->uNumNewNPCs; ++i)
        {
            if (pNPCStats->pNewNPCData[i].uFlags & 128)
            {
                if (!pParty->pHirelings[0].pName || strcmp(pNPCStats->pNewNPCData[i].pName, pParty->pHirelings[0].pName))
                {
                    if (!pParty->pHirelings[1].pName || strcmp(pNPCStats->pNewNPCData[i].pName, pParty->pHirelings[1].pName))
                        buf[v22++] = i + 2;
                }
            }
        }

        for (int i = pParty->hirelingScrollPosition; i < v22 && pNPC_limit_ID < 2; i++)
        {
            if ((unsigned __int8)buf[i] >= 2)
            {
                sprintf(pContainer, "NPC%03d", pNPCStats->pNPCData[(unsigned __int8)buf[i] + 499].uPortraitID);
                render->DrawTextureAlphaNew(
                    pHiredNPCsIconsOffsetsX[pNPC_limit_ID] / 640.0f,
                    pHiredNPCsIconsOffsetsY[pNPC_limit_ID] / 480.0f,
                    assets->GetImage_16BitColorKey(pContainer, 0x7FF)
                );
            }
            else
            {
                sprintf(pContainer, "NPC%03d", pParty->pHirelings[(unsigned __int8)buf[i]].uPortraitID);
                render->DrawTextureAlphaNew(
                    pHiredNPCsIconsOffsetsX[pNPC_limit_ID] / 640.0f,
                    pHiredNPCsIconsOffsetsY[pNPC_limit_ID] / 480.0f,
                    assets->GetImage_16BitColorKey(pContainer, 0x7FF)
                );
                if (pParty->pHirelings[(unsigned __int8)buf[i]].evt_A == 1)
                {
                    uFrameID = pParty->pHirelings[(unsigned __int8)buf[i]].evt_B;
                    v13 = 0;
                    if (pIconsFrameTable->uNumIcons)
                    {
                        for (v13 = 0; v13 < pIconsFrameTable->uNumIcons; ++v13)
                        {
                            if (!_stricmp("spell96", pIconsFrameTable->pIcons[v13].GetAnimationName()))
                                break;
                        }
                    }
                    render->DrawTextureAlphaNew(
                        pHiredNPCsIconsOffsetsX[pNPC_limit_ID] / 640.0f,
                        pHiredNPCsIconsOffsetsY[pNPC_limit_ID] / 480.0f,
                        pIconsFrameTable->GetFrame(v13, uFrameID)->GetTexture()
                    );
                }
            }
            ++pNPC_limit_ID;
        }
    }
}

//----- (004178FE) --------------------------------------------------------
unsigned int UI_GetHealthManaAndOtherQualitiesStringColor(int actual_value, int base_value)
{
    unsigned __int16 R, G, B;

    if (actual_value == base_value)
    {
        R = 0, G = 0, B = 0; // White
    }
    else if (actual_value < base_value)
    {
        if (100 * actual_value / base_value >= 25)//Yellow( current_pos > 1/4 )
            R = 255, G = 255, B = 100;
        else//Red( current_pos < 1/4 )
            R = 255, G = 0, B = 0;
    }
    else // Green
    {
        R = 0, G = 255, B = 0;
    }

    return Color16(R, G, B);
}

//----- (00417939) --------------------------------------------------------
int GetConditionDrawColor(unsigned int uConditionIdx)
{
  switch (uConditionIdx)
  {
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
String GetReputationString(int reputation)
{
	if (reputation >= 25)
		return localization->GetString(379); // Hated
	else if (reputation >= 6)
		return localization->GetString(392); // Unfriendly
	else if (reputation >= -5)
		return localization->GetString(399); // Neutral
	else if (reputation >= -24)
		return localization->GetString(402); // Friendly
	else
		return localization->GetString(434); // Respected
}

//----- (00441A4E) --------------------------------------------------------
__int16 _441A4E_overlay_on_portrait(int a1)//for blessing
{
	__int16 result; // ax@1
	int v2; // ebx@1
	//  char *v3; // esi@1
	//  int v4; // edi@4
	bool v5; // ecx@4
	SpriteFrame *pFrame; // eax@6
	//SpriteFrame *v7; // edi@6
	int v8; // eax@6
	//  unsigned __int16 v9; // ax@6
	SoftwareBillboard v10; // [sp+Ch] [bp-5Ch]@1
	int v11; // [sp+5Ch] [bp-Ch]@6
	int v12; // [sp+60h] [bp-8h]@1
	//int v13; // [sp+64h] [bp-4h]@6

	v10.sParentBillboardID = -1;
	v10.pTarget = render->pTargetSurface;
	v10.pTargetZ = render->pActiveZBuffer;
	v10.uTargetPitch = render->GetRenderWidth();
	result = 0;
	v2 = a1;
	v10.uViewportX = 0;
	v10.uViewportY = 0;
	v10.uViewportZ = window->GetWidth() - 1;
	v10.uViewportW = window->GetHeight() - 1;
	v12 = 0;
	//v3 = (char *)&pOtherOverlayList->pOverlays[0].field_C;
	//do
	for (uint i = 0; i < 50; ++i)
	{
		if (pOtherOverlayList->pOverlays[i].field_6 > 0)
		{
			result = pOtherOverlayList->pOverlays[i].field_0;
			if (pOtherOverlayList->pOverlays[i].field_0 >= 300)
			{
				//v4 = result;
				v5 = pOtherOverlayList->pOverlays[i].field_0 == v2 + 320
					|| pOtherOverlayList->pOverlays[i].field_0 == v2 + 330
					|| pOtherOverlayList->pOverlays[i].field_0 == v2 + 340
					|| pOtherOverlayList->pOverlays[i].field_0 == v2 + 350;
				pOtherOverlayList->pOverlays[i].field_0 = v2 + 310;
				if (pOtherOverlayList->pOverlays[i].field_0 == v2 + 310 || v5)
				{
					if (!pOtherOverlayList->pOverlays[i].field_0)
					{
						pFrame = pSpriteFrameTable->GetFrame(pOverlayList->pOverlays[pOtherOverlayList->pOverlays[i].field_2].uSpriteFramesetID,
							pOtherOverlayList->pOverlays[i].sprite_frame_time);
						//v7 = v6;
						v11 = pOtherOverlayList->pOverlays[i].field_E;
						//v13 = pFrame->scale;
						//v13 = fixpoint_mul(v11, pFrame->scale);
						v10.screen_space_x = pOtherOverlayList->pOverlays[i].screen_space_x;
						v10.screen_space_y = pOtherOverlayList->pOverlays[i].screen_space_y;
						v10.screenspace_projection_factor_x = fixed::Raw(fixpoint_mul(v11, pFrame->scale._internal));
						v10.screenspace_projection_factor_y = fixed::Raw(fixpoint_mul(v11, pFrame->scale._internal));
						v10.pPalette = PaletteManager::Get_Dark_or_Red_LUT(pFrame->uPaletteIndex, 0, 1);
						v8 = pOtherOverlayList->pOverlays[i].field_2;
                        v10.screen_space_z = 0;
                        v10.object_pid = 0;
						v10.uFlags = 0;
						//v9 = pOverlayList->pOverlays[v8].uOverlayType;
						if (!pOverlayList->pOverlays[v8].uOverlayType || pOverlayList->pOverlays[v8].uOverlayType == 2)
							v10.screen_space_y += pFrame->hw_sprites[0]->sprite_header->uHeight / 2;
						result = pFrame->hw_sprites[0]->sprite_header->_4AD2D1_overlays(&v10, 0);
						++v12;
						if (v12 == 5)
							break;
					}
				}
			}
		}
		//v3 += 20;
	}
	//while ( (signed int)v3 < (signed int)&pOverlayList->pOverlays );
	return result;
}
