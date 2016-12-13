#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS
#include <io.h>

#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/LOD.h"
#include "Engine/Timer.h"
#include "Engine/stru159.h"
#include "Engine/Events2D.h"
#include "Engine/MapInfo.h"

#include "GUI/UI/UITransition.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIProgressBar.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIGame.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"




Image *transition_ui_icon = nullptr;

std::string transition_button_label;

void GUIWindow_Travel::Release()
{
// -----------------------------------------
// 0041C26A void GUIWindow::Release --- part
    if (transition_ui_icon)
    {
        transition_ui_icon->Release();
        transition_ui_icon = nullptr;
    }

    if (game_ui_dialogue_background)
    {
        game_ui_dialogue_background->Release();
        game_ui_dialogue_background = nullptr;
    }

    current_screen_type = prev_screen_type;

    GUIWindow::Release();
}

void GUIWindow_Transition::Release()
{
// -----------------------------------------
// 0041C26A void GUIWindow::Release --- part
    //pVideoPlayer->Unload();
    if (transition_ui_icon)
    {
        transition_ui_icon->Release();
        transition_ui_icon = nullptr;
    }

    if (game_ui_dialogue_background)
    {
        game_ui_dialogue_background->Release();
        game_ui_dialogue_background = nullptr;
    }

    current_screen_type = prev_screen_type;

    GUIWindow::Release();
}


//----- (00444839) --------------------------------------------------------
GUIWindow_Transition::GUIWindow_Transition(uint anim_id, uint exit_pic_id, int x, int y, int z, int directiony, int directionx, int a8, const char *pLocationName) :
    GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), 0, nullptr)
{
// ----------------------------------
// 00444839 TransitionUI_Load -- part
  const char *v15; // eax@14

  dword_59117C_teleportx = x;
  dword_591178_teleporty = y;
  dword_591174_teleportz = z;
  dword_591170_teleport_directiony = directiony;
  dword_59116C_teleport_directionx = directionx;
  dword_591168_teleport_speedz = a8;
  dword_591164_teleport_map_name = (char *)pLocationName;
  uCurrentHouse_Animation = anim_id;
  pEventTimer->Pause();
  pAudioPlayer->StopChannels(-1, -1);

  char filename[40];
  switch (pParty->alignment)
  {
    case PartyAlignment_Good:    sprintf(filename, "evt%02d-b", const_2()); break;
    case PartyAlignment_Neutral: sprintf(filename, "evt%02d", const_2());   break;
    case PartyAlignment_Evil:    sprintf(filename, "evt%02d-c", const_2()); break;
    default: Error("Invalid alignment: %u", pParty->alignment);
  }
  game_ui_dialogue_background = assets->GetImage_16Bit(filename);

  transition_ui_icon = assets->GetImage_16Bit(pHouse_ExitPictures[exit_pic_id]);
  if (anim_id)
  {
    if ( !IndoorLocation::GetLocationIndex(pLocationName) )
      pMediaPlayer->OpenHouseMovie(pAnimatedRooms[p2DEvents[anim_id - 1].uAnimationID].video_name, 1);

    v15 = pLocationName;
    if (*pLocationName == 48)
        v15 = pCurrentMapName;
    if (pMapStats->GetMapInfo(v15))
    {
        transition_button_label = localization->FormatString(411, pMapStats->pInfos[pMapStats->GetMapInfo(v15)].pName); // Enter %s   Войти в ^Pv[%s]

        if (uCurrentlyLoadedLevelType == LEVEL_Indoor && uActiveCharacter && pParty->uFlags & 0x30)
            pPlayers[uActiveCharacter]->PlaySound(SPEECH_47, 0);
        if (IndoorLocation::GetLocationIndex(pLocationName))
            uCurrentHouse_Animation = IndoorLocation::GetLocationIndex(pLocationName);
    }
    else
    {
        transition_button_label = localization->GetString(73); // Enter    Войти
        //if ( pAnimatedRooms[p2DEvents[anim_id].uAnimationID].uRoomSoundId )
        //  PlayHouseSound(anim_id, HouseSound_Greeting);
        if (uCurrentlyLoadedLevelType == LEVEL_Indoor && uActiveCharacter && pParty->uFlags & 0x30)
            pPlayers[uActiveCharacter]->PlaySound(SPEECH_47, 0);
        if (IndoorLocation::GetLocationIndex(pLocationName))
            uCurrentHouse_Animation = IndoorLocation::GetLocationIndex(pLocationName);
    }
  }
  else if ( !IndoorLocation::GetLocationIndex(pLocationName) )
  {
    if ( pMapStats->GetMapInfo(pCurrentMapName) )
    {
        transition_button_label = localization->FormatString(410, pMapStats->pInfos[pMapStats->GetMapInfo(pCurrentMapName)].pName); // "Leave %s"
      //if ( pAnimatedRooms[p2DEvents[anim_id].uAnimationID].uRoomSoundId )
        //PlayHouseSound(anim_id, HouseSound_Greeting);
      if ( uCurrentlyLoadedLevelType == LEVEL_Indoor && uActiveCharacter && pParty->uFlags & 0x30 )
        pPlayers[uActiveCharacter]->PlaySound(SPEECH_47, 0);
      if ( IndoorLocation::GetLocationIndex(pLocationName) )
        uCurrentHouse_Animation = IndoorLocation::GetLocationIndex(pLocationName);
    }
    else
    {
        transition_button_label = localization->GetString(79);
        //if ( pAnimatedRooms[p2DEvents[anim_id].uAnimationID].uRoomSoundId )
        //PlayHouseSound(anim_id, HouseSound_Greeting);
        if (uCurrentlyLoadedLevelType == LEVEL_Indoor && uActiveCharacter && pParty->uFlags & 0x30)
            pPlayers[uActiveCharacter]->PlaySound(SPEECH_47, 0);
        if (IndoorLocation::GetLocationIndex(pLocationName))
            uCurrentHouse_Animation = IndoorLocation::GetLocationIndex(pLocationName);
    }
  }

  auto hint = this->Hint = transition_button_label.c_str();

//
// --------------------------------
// 0041C432 GUIWindow c-tor -- part
  prev_screen_type = current_screen_type;
  current_screen_type = SCREEN_INPUT_BLV;
  pBtn_ExitCancel = CreateButton(0x236u, 0x1BDu, 0x4Bu, 0x21u, 1, 0, UIMSG_TransitionWindowCloseBtn, 0, 'N', localization->GetString(34), ui_buttdesc2, 0); // Cancel / Отмена
  pBtn_YES = CreateButton(0x1E6u, 0x1BDu, 0x4Bu, 0x21u, 1, 0, UIMSG_TransitionUI_Confirm, 0, 'Y', hint, ui_buttyes2, 0);
  CreateButton(pNPCPortraits_x[0][0], pNPCPortraits_y[0][0], 0x3Fu, 0x49u, 1, 0, UIMSG_TransitionUI_Confirm, 1, 0x20u, hint, 0);
  CreateButton(8, 8, 0x1CCu, 0x158u, 1, 0, UIMSG_TransitionUI_Confirm, 1u, 0, hint, 0);
}



//----- (00444C8F) --------------------------------------------------------
GUIWindow_Travel::GUIWindow_Travel() :
    GUIWindow(0, 0, window->GetWidth(), window->GetHeight(), 0, nullptr)
{
// ------------------------------
// 00444C8F TravelUI_Load -- part
  char pContainer[32]; // [sp+0h] [bp-28h]@1

  pEventTimer->Pause();

  switch (pParty->alignment)
  {
    case PartyAlignment_Good:    sprintf(pContainer, "evt%02d-b", const_2()); break;
    case PartyAlignment_Neutral: sprintf(pContainer, "evt%02d", const_2());   break;
    case PartyAlignment_Evil:    sprintf(pContainer, "evt%02d-c", const_2()); break;
    default: Error("Invalid alignment: %u", pParty->alignment);
  }
  game_ui_dialogue_background = assets->GetImage_16Bit(pContainer);

  transition_ui_icon = assets->GetImage_16Bit("outside");
  if (pMapStats->GetMapInfo(pCurrentMapName))
      transition_button_label = localization->FormatString(410, pMapStats->pInfos[pMapStats->GetMapInfo(pCurrentMapName)].pName);// "Leave %s"
  else
      transition_button_label = localization->GetString(79); // Exit



  auto hint = this->Hint = transition_button_label.c_str();

// --------------------------------
// 0041C432 GUIWindow c-tor -- part
  prev_screen_type = current_screen_type;
  current_screen_type = SCREEN_CHANGE_LOCATION;
  pBtn_ExitCancel = CreateButton(
      566, 445, 75, 33, 1, 0,
      UIMSG_CHANGE_LOCATION_ClickCencelBtn,
      0,
      'N',
      localization->GetString(156),
      ui_buttdesc2,
      0
  );// Stay in this area / Остаться в этой области
  pBtn_YES = CreateButton(
      486, 445, 75, 33, 1, 0,
      UIMSG_OnTravelByFoot,
      0,
      'Y',
      hint,
      ui_buttyes2,
      0
  );
  CreateButton(pNPCPortraits_x[0][0], pNPCPortraits_y[0][0], 63, 73, 1, 0, UIMSG_OnTravelByFoot, 1, ' ', hint, 0, 0, 0);
  CreateButton(8, 8, 460, 344, 1, 0, UIMSG_OnTravelByFoot, 1, 0, hint, 0);
}



//----- (00444DCA) --------------------------------------------------------
void GUIWindow_Travel::Update()
{
    // -----------------------------------
    // 004156F0 GUI_UpdateWindows --- part
    GUIWindow travel_window; // [sp+Ch] [bp-78h]@1
    char pDestinationMapName[32]; // [sp+60h] [bp-24h]@1

    memcpy(&travel_window, pPrimaryWindow, sizeof(travel_window));
    pOutdoor->GetTravelDestination(pParty->vPosition.x, pParty->vPosition.y, pDestinationMapName, 20);
    pRenderer->DrawTextureNew(477 / 640.0f, 0, game_ui_dialogue_background);
    pRenderer->DrawTextureAlphaNew(468 / 640.0f, 0, game_ui_right_panel_frame);
    pRenderer->DrawTextureNew(pNPCPortraits_x[0][0] / 640.0f, pNPCPortraits_y[0][0] / 480.0f, transition_ui_icon);
    pRenderer->DrawTextureAlphaNew(556 / 640.0f, 451 / 480.0f, dialogue_ui_x_x_u);
    pRenderer->DrawTextureAlphaNew(476 / 640.0f, 451 / 480.0f, dialogue_ui_x_ok_u);
    if (pMapStats->GetMapInfo(pDestinationMapName))
    {
        travel_window.uFrameX = 493;
        travel_window.uFrameWidth = 126;
        travel_window.uFrameZ = 366;
        travel_window.DrawTitleText(pFontCreate, 0, 4, 0, pMapStats->pInfos[pMapStats->GetMapInfo(pDestinationMapName)].pName, 3);
        travel_window.uFrameX = 483;
        travel_window.uFrameWidth = 148;
        travel_window.uFrameZ = 334;

        String str;
        if (GetTravelTime() == 1)
            str = localization->FormatString(663, 1, pMapStats->pInfos[pMapStats->GetMapInfo(pDestinationMapName)].pName); // It will take %d day to cross to %s.
        else
            str = localization->FormatString(128, GetTravelTime(), pMapStats->pInfos[pMapStats->GetMapInfo(pDestinationMapName)].pName); // It will take %d days to travel to %s.
        str += "\n \n";
        str += localization->FormatString(126, pMapStats->pInfos[pMapStats->GetMapInfo(pCurrentMapName)].pName);

        travel_window.DrawTitleText(pFontCreate, 0, (212 - pFontCreate->CalcTextHeight(str, &travel_window, 0)) / 2 + 101, 0, str, 3);
        _unused_5B5924_is_travel_ui_drawn = 1;
    }
}



//----- (00444A51) --------------------------------------------------------
void GUIWindow_Transition::Update()
{
    // -----------------------------------
    // 004156F0 GUI_UpdateWindows --- part
    unsigned int v4; // [sp-10h] [bp-7Ch]@12
    GUIWindow transition_window; // [sp+Ch] [bp-60h]@1
    unsigned int v9; // [sp+60h] [bp-Ch]@1
    unsigned int map_id; // [sp+64h] [bp-8h]@1

    memcpy(&transition_window, pPrimaryWindow, sizeof(transition_window));
    v9 = IndoorLocation::GetLocationIndex(dword_591164_teleport_map_name);
    pRenderer->DrawTextureNew(477 / 640.0f, 0, game_ui_dialogue_background);
    pRenderer->DrawTextureAlphaNew((pNPCPortraits_x[0][0] - 4) / 640.0f, (pNPCPortraits_y[0][0] - 4) / 480.0f, game_ui_evtnpc);
    pRenderer->DrawTextureNew(pNPCPortraits_x[0][0] / 640.0f, pNPCPortraits_y[0][0] / 480.0f, transition_ui_icon);

    pRenderer->DrawTextureAlphaNew(468 / 640.0f, 0, game_ui_right_panel_frame);
    pRenderer->DrawTextureAlphaNew(556 / 640.0f, 451 / 480.0f, dialogue_ui_x_x_u);
    pRenderer->DrawTextureAlphaNew(476 / 640.0f, 451 / 480.0f, dialogue_ui_x_ok_u);
    map_id = pMapStats->GetMapInfo(pCurrentMapName);
    if ((pMovie_Track || v9) && *dword_591164_teleport_map_name != ' ')
        map_id = pMapStats->GetMapInfo(dword_591164_teleport_map_name);
    transition_window.uFrameX = 493;
    transition_window.uFrameWidth = 126;
    transition_window.uFrameZ = 366;
    transition_window.DrawTitleText(pFontCreate, 0, 5, 0, pMapStats->pInfos[map_id].pName, 3);
    transition_window.uFrameX = 483;
    transition_window.uFrameWidth = 148;
    transition_window.uFrameZ = 334;

    if (uCurrentHouse_Animation)
    {
        v4 = (212 - pFontCreate->CalcTextHeight(pTransitionStrings[uCurrentHouse_Animation], &transition_window, 0)) / 2 + 101;
        transition_window.DrawTitleText(pFontCreate, 0, v4, 0, pTransitionStrings[uCurrentHouse_Animation], 3);
    }
    else if (map_id)
    {
        auto str = localization->FormatString(409, pMapStats->pInfos[map_id].pName); // Do you wish to leave %s?
        v4 = (212 - pFontCreate->CalcTextHeight(str, &transition_window, 0)) / 2 + 101;
        transition_window.DrawTitleText(pFontCreate, 0, v4, 0, str, 3);
    }
    else Error("Troubles in da house");

    _unused_5B5924_is_travel_ui_drawn = true;
}