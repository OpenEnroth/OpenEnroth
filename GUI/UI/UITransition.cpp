#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>

#define _CRT_SECURE_NO_WARNINGS
#include "Engine/Engine.h"
#include "Engine/Events2D.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/MapInfo.h"
#include "Engine/Party.h"
#include "Engine/Time.h"
#include "Engine/stru159.h"

#include "Engine/Graphics/Outdoor.h"

#include "GUI/GUIButton.h"
#include "GUI/UI/UITransition.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIProgressBar.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIHouses.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

#include "Platform/Api.h"

Image *transition_ui_icon = nullptr;

String transition_button_label;

void GUIWindow_Travel::Release() {
    // -----------------------------------------
    // 0041C26A void GUIWindow::Release --- part
    if (transition_ui_icon) {
        transition_ui_icon->Release();
        transition_ui_icon = nullptr;
    }

    if (game_ui_dialogue_background) {
        game_ui_dialogue_background->Release();
        game_ui_dialogue_background = nullptr;
    }

    current_screen_type = prev_screen_type;

    GUIWindow::Release();
}

void GUIWindow_Transition::Release() {
    // -----------------------------------------
    // 0041C26A void GUIWindow::Release --- part
    // pVideoPlayer->Unload();
    if (transition_ui_icon) {
        transition_ui_icon->Release();
        transition_ui_icon = nullptr;
    }

    if (game_ui_dialogue_background) {
        game_ui_dialogue_background->Release();
        game_ui_dialogue_background = nullptr;
    }

    current_screen_type = prev_screen_type;

    GUIWindow::Release();
}

//----- (00444839) --------------------------------------------------------
GUIWindow_Transition::GUIWindow_Transition(uint anim_id, uint exit_pic_id,
                                           int x, int y, int z, int directiony,
                                           int directionx, int a8,
                                           const char *pLocationName)
    : GUIWindow(WINDOW_Transition, 0, 0, window->GetWidth(), window->GetHeight(), 0) {
    Party_Teleport_X_Pos = x;
    Party_Teleport_Y_Pos = y;
    Party_Teleport_Z_Pos = z;
    Party_Teleport_Cam_Yaw = directiony;
    Party_Teleport_Cam_Pitch = directionx;
    Party_Teleport_Z_Speed = a8;
    Party_Teleport_Map_Name = (char *)pLocationName;
    uCurrentHouse_Animation = anim_id;
    pEventTimer->Pause();
    pAudioPlayer->StopChannels(-1, -1);

    String filename;
    switch (pParty->alignment) {
        case PartyAlignment::PartyAlignment_Good:
            filename = "evt02-b";
            break;
        case PartyAlignment::PartyAlignment_Neutral:
            filename = "evt02";
            break;
        case PartyAlignment::PartyAlignment_Evil:
            filename = "evt02-c";
            break;
        default:
            Error("Invalid alignment: %u", pParty->alignment);
    }
    game_ui_dialogue_background = assets->GetImage_Solid(filename);

    transition_ui_icon =
        assets->GetImage_Solid(pHouse_ExitPictures[exit_pic_id]);
    if (anim_id) {
        if (!IndoorLocation::GetLocationIndex(pLocationName))
            pMediaPlayer->OpenHouseMovie(
                pAnimatedRooms[p2DEvents[anim_id - 1].uAnimationID].video_name,
                1);

        String v15 = pLocationName;
        if (*pLocationName == 48) {
            v15 = pCurrentMapName;
        }
        if (pMapStats->GetMapInfo(v15)) {
            transition_button_label = localization->FormatString(
                411, pMapStats->pInfos[pMapStats->GetMapInfo(v15)]
                         .pName.c_str());  // Enter %s   Войти в ^Pv[%s]

            if (uCurrentlyLoadedLevelType == LEVEL_Indoor && uActiveCharacter &&
                pParty->uFlags & 0x30)
                pPlayers[uActiveCharacter]->PlaySound(SPEECH_47, 0);
            if (IndoorLocation::GetLocationIndex(pLocationName))
                uCurrentHouse_Animation =
                    IndoorLocation::GetLocationIndex(pLocationName);
        } else {
            transition_button_label = localization->GetString(
                73);  // Enter    Войти
                      // if (
                      // pAnimatedRooms[p2DEvents[anim_id].uAnimationID].uRoomSoundId
                      // )
                      //  PlayHouseSound(anim_id, HouseSound_Greeting);
            if (uCurrentlyLoadedLevelType == LEVEL_Indoor && uActiveCharacter &&
                pParty->uFlags & 0x30)
                pPlayers[uActiveCharacter]->PlaySound(SPEECH_47, 0);
            if (IndoorLocation::GetLocationIndex(pLocationName))
                uCurrentHouse_Animation =
                    IndoorLocation::GetLocationIndex(pLocationName);
        }
    } else if (!IndoorLocation::GetLocationIndex(pLocationName)) {
        if (pMapStats->GetMapInfo(pCurrentMapName)) {
            transition_button_label = localization->FormatString(
                410,
                pMapStats->pInfos[pMapStats->GetMapInfo(pCurrentMapName)]
                    .pName.c_str());  // "Leave %s"
                              // if (
                              // pAnimatedRooms[p2DEvents[anim_id].uAnimationID].uRoomSoundId
                              // ) PlayHouseSound(anim_id, HouseSound_Greeting);
            if (uCurrentlyLoadedLevelType == LEVEL_Indoor && uActiveCharacter &&
                pParty->uFlags & 0x30)
                pPlayers[uActiveCharacter]->PlaySound(SPEECH_47, 0);
            if (IndoorLocation::GetLocationIndex(pLocationName))
                uCurrentHouse_Animation =
                    IndoorLocation::GetLocationIndex(pLocationName);
        } else {
            transition_button_label = localization->GetString(79);
            // if ( pAnimatedRooms[p2DEvents[anim_id].uAnimationID].uRoomSoundId
            // ) PlayHouseSound(anim_id, HouseSound_Greeting);
            if (uCurrentlyLoadedLevelType == LEVEL_Indoor && uActiveCharacter &&
                pParty->uFlags & 0x30)
                pPlayers[uActiveCharacter]->PlaySound(SPEECH_47, 0);
            if (IndoorLocation::GetLocationIndex(pLocationName))
                uCurrentHouse_Animation =
                    IndoorLocation::GetLocationIndex(pLocationName);
        }
    }

    String hint = this->sHint = transition_button_label;

    prev_screen_type = current_screen_type;
    current_screen_type = CURRENT_SCREEN::SCREEN_INPUT_BLV;
    pBtn_ExitCancel = CreateButton(
        0x236u, 0x1BDu, 0x4Bu, 0x21u, 1, 0, UIMSG_TransitionWindowCloseBtn, 0,
        'N', localization->GetString(34), {{ui_buttdesc2}});  // Cancel / Отмена
    pBtn_YES =
        CreateButton(0x1E6u, 0x1BDu, 0x4Bu, 0x21u, 1, 0,
                     UIMSG_TransitionUI_Confirm, 0, 'Y', hint, {{ui_buttyes2}});
    CreateButton(pNPCPortraits_x[0][0], pNPCPortraits_y[0][0], 0x3Fu, 0x49u, 1,
                 0, UIMSG_TransitionUI_Confirm, 1, 0x20u, hint);
    CreateButton(8, 8, 0x1CCu, 0x158u, 1, 0, UIMSG_TransitionUI_Confirm, 1u, 0,
                 hint);
}

GUIWindow_Travel::GUIWindow_Travel()
    : GUIWindow(WINDOW_ChangeLocation, 0, 0, window->GetWidth(), window->GetHeight(), 0) {
    String pContainer;  // [sp+0h] [bp-28h]@1

    pEventTimer->Pause();

    switch (pParty->alignment) {
        case PartyAlignment::PartyAlignment_Good:
            pContainer = "evt02-b";
            break;
        case PartyAlignment::PartyAlignment_Neutral:
            pContainer = "evt02";
            break;
        case PartyAlignment::PartyAlignment_Evil:
            pContainer = "evt02-c";
            break;
        default:
            Error("Invalid alignment: %u", pParty->alignment);
    }
    game_ui_dialogue_background = assets->GetImage_Solid(pContainer);

    transition_ui_icon = assets->GetImage_Solid("outside");
    if (pMapStats->GetMapInfo(pCurrentMapName))
        transition_button_label = localization->FormatString(
            410, pMapStats->pInfos[pMapStats->GetMapInfo(pCurrentMapName)]
                     .pName.c_str());  // "Leave %s"
    else
        transition_button_label = localization->GetString(79);  // Exit

    String hint = this->sHint = transition_button_label;

    prev_screen_type = current_screen_type;
    current_screen_type = CURRENT_SCREEN::SCREEN_CHANGE_LOCATION;
    pBtn_ExitCancel = CreateButton(
        566, 445, 75, 33, 1, 0, UIMSG_CHANGE_LOCATION_ClickCencelBtn, 0, 'N',
        localization->GetString(156),
        {{ui_buttdesc2}});  // Stay in this area / Остаться в этой области
    pBtn_YES = CreateButton(486, 445, 75, 33, 1, 0, UIMSG_OnTravelByFoot, 0,
                            'Y', hint, {{ui_buttyes2}});
    CreateButton(pNPCPortraits_x[0][0], pNPCPortraits_y[0][0], 63, 73, 1, 0,
                 UIMSG_OnTravelByFoot, 1, ' ', hint);
    CreateButton(8, 8, 460, 344, 1, 0, UIMSG_OnTravelByFoot, 1, 0, hint);
}

void GUIWindow_Travel::Update() {
    char pDestinationMapName[32];

    pOutdoor->GetTravelDestination(pParty->vPosition.x, pParty->vPosition.y,
                                   pDestinationMapName, 20);
    render->DrawTextureNew(477 / 640.0f, 0, game_ui_dialogue_background);
    render->DrawTextureAlphaNew(468 / 640.0f, 0, game_ui_right_panel_frame);
    render->DrawTextureNew(pNPCPortraits_x[0][0] / 640.0f,
                           pNPCPortraits_y[0][0] / 480.0f, transition_ui_icon);
    render->DrawTextureAlphaNew(556 / 640.0f, 451 / 480.0f, dialogue_ui_x_x_u);
    render->DrawTextureAlphaNew(476 / 640.0f, 451 / 480.0f, dialogue_ui_x_ok_u);
    if (pMapStats->GetMapInfo(pDestinationMapName)) {
        GUIWindow travel_window = *pPrimaryWindow;
        travel_window.uFrameX = 493;
        travel_window.uFrameWidth = 126;
        travel_window.uFrameZ = 366;
        travel_window.DrawTitleText(
            pFontCreate, 0, 4, 0,
            pMapStats->pInfos[pMapStats->GetMapInfo(pDestinationMapName)].pName,
            3);
        travel_window.uFrameX = 483;
        travel_window.uFrameWidth = 148;
        travel_window.uFrameZ = 334;

        String str;
        if (GetTravelTime() == 1) {
            str = localization->FormatString(
                663, 1,
                pMapStats->pInfos[pMapStats->GetMapInfo(pDestinationMapName)]
                    .pName.c_str());  // It will take %d day to cross to %s.
        } else {
            str = localization->FormatString(
                128, GetTravelTime(),
                pMapStats->pInfos[pMapStats->GetMapInfo(pDestinationMapName)]
                    .pName.c_str());  // It will take %d days to travel to %s.
        }
        str += "\n \n";
        str += localization->FormatString(
            126,
            pMapStats->pInfos[pMapStats->GetMapInfo(pCurrentMapName)].pName.c_str());

        travel_window.DrawTitleText(
            pFontCreate, 0,
            (212 -
             pFontCreate->CalcTextHeight(str, travel_window.uFrameWidth, 0)) /
                    2 +
                101,
            0, str, 3);
        _unused_5B5924_is_travel_ui_drawn = 1;
    }
}

void GUIWindow_Transition::Update() {
    unsigned int v9 =
        IndoorLocation::GetLocationIndex(Party_Teleport_Map_Name);
    render->DrawTextureNew(477 / 640.0f, 0, game_ui_dialogue_background);
    render->DrawTextureAlphaNew((pNPCPortraits_x[0][0] - 4) / 640.0f,
                                (pNPCPortraits_y[0][0] - 4) / 480.0f,
                                game_ui_evtnpc);
    render->DrawTextureNew(pNPCPortraits_x[0][0] / 640.0f,
                           pNPCPortraits_y[0][0] / 480.0f, transition_ui_icon);

    render->DrawTextureAlphaNew(468 / 640.0f, 0, game_ui_right_panel_frame);
    render->DrawTextureAlphaNew(556 / 640.0f, 451 / 480.0f, dialogue_ui_x_x_u);
    render->DrawTextureAlphaNew(476 / 640.0f, 451 / 480.0f, dialogue_ui_x_ok_u);
    unsigned int map_id = pMapStats->GetMapInfo(pCurrentMapName);
    if ((pMovie_Track || v9) && *Party_Teleport_Map_Name != ' ') {
        map_id = pMapStats->GetMapInfo(Party_Teleport_Map_Name);
    }

    GUIWindow transition_window = *pPrimaryWindow;
    transition_window.uFrameX = 493;
    transition_window.uFrameWidth = 126;
    transition_window.uFrameZ = 366;
    transition_window.DrawTitleText(pFontCreate, 0, 5, 0,
                                    pMapStats->pInfos[map_id].pName, 3);
    transition_window.uFrameX = 483;
    transition_window.uFrameWidth = 148;
    transition_window.uFrameZ = 334;

    if (uCurrentHouse_Animation) {
        unsigned int v4 =
            (212 - pFontCreate->CalcTextHeight(
                       pTransitionStrings[uCurrentHouse_Animation],
                       transition_window.uFrameWidth, 0)) /
                2 +
            101;
        transition_window.DrawTitleText(
            pFontCreate, 0, v4, 0, pTransitionStrings[uCurrentHouse_Animation],
            3);
    } else if (map_id) {
        String str = localization->FormatString(
            409, pMapStats->pInfos[map_id].pName.c_str());  // Do you wish to leave %s?
        unsigned int v4 = (212 - pFontCreate->CalcTextHeight(
                                     str, transition_window.uFrameWidth, 0)) /
                              2 +
                          101;
        transition_window.DrawTitleText(pFontCreate, 0, v4, 0, str, 3);
    } else {
        Error("Troubles in da house");
    }

    _unused_5B5924_is_travel_ui_drawn = true;
}
