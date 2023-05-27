#include <string>

#include "GUI/UI/Houses/Transport.h"

#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UIShops.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"

#include "Engine/Localization.h"
#include "Engine/SaveLoad.h"
#include "Engine/PriceCalculator.h"
#include "Engine/Graphics/Camera.h"

stru365_travel_info transport_schedule[35] = {  // 004F09B0
                                                // location name   days    x        y       z
    { MAP_STEADWICK, { 1, 0, 1, 0, 1, 0, 0 }, 2, -18048, 4636, 833, 1536, 0 },  // for stable
    { MAP_PIERPONT, { 0, 1, 0, 1, 0, 1, 0 }, 2, -2527, -6773, 1153, 896, 0 },
    { MAP_TATALIA, { 1, 0, 1, 0, 1, 0, 0 }, 2, 4730, -10580, 320, 1024, 0 },
    { MAP_HARMONDALE, { 0, 1, 0, 1, 0, 1, 0 }, 2, -5692, 11137, 1, 1024, 0 },
    { MAP_DEYJA, { 1, 0, 0, 1, 0, 0, 0 }, 3, 7227, -16007, 2625, 640, 0 },
    { MAP_BRAKADA_DESERT, { 0, 0, 1, 0, 0, 1, 0 }, 3, 8923, 17191, 1, 512, 0 },
    { MAP_AVLEE, { 1, 0, 1, 0, 1, 0, 0 }, 3, 17059, 12331, 512, 1152, 0 },
    { MAP_DEYJA, { 0, 1, 0, 0, 1, 0, 1 }, 2, 7227, -16007, 2625, 640, 0 },
    { MAP_HARMONDALE, { 0, 1, 0, 1, 0, 1, 0 }, 2, -5692, 11137, 1, 1024, 0 },
    { MAP_STEADWICK, { 1, 0, 1, 0, 1, 0, 0 }, 3, -18048, 4636, 833, 1536, 0 },
    { MAP_PIERPONT, { 0, 1, 0, 1, 0, 1, 0 }, 2, -2527, -6773, 1153, 896, 0 },
    { MAP_STEADWICK, { 1, 0, 1, 0, 1, 0, 1 }, 3, -18048, 4636, 833, 1536, 0 },
    { MAP_HARMONDALE, { 0, 1, 0, 0, 0, 1, 0 }, 5, -5692, 11137, 1, 1024, 0 },
    { MAP_STEADWICK, { 0, 1, 0, 1, 0, 1, 0 }, 2, -18048, 4636, 833, 1536, 0 },
    { MAP_PIERPONT, { 0, 1, 0, 1, 0, 1, 0 }, 3, -2527, -6773, 1153, 896, 0 },
    { MAP_DEYJA, { 0, 0, 1, 0, 0, 0, 1 }, 5, 7227, -16007, 2625, 640, 0 },
    { MAP_TATALIA, { 0, 1, 0, 1, 0, 1, 0 }, 2, -2183, -6941, 97, 0, 0 },
    { MAP_AVLEE, { 1, 0, 0, 0, 1, 0, 0 }, 4, 7913, 9476, 193, 0, 0 },
    { MAP_EVENMORN_ISLE, { 0, 0, 0, 0, 0, 0, 1 }, 7, 15616, 6390, 193, 1536, QBIT_EVENMORN_MAP_FOUND },
    { MAP_BRAKADA_DESERT, { 0, 0, 1, 0, 0, 0, 0 }, 6, 19171, -19722, 193, 1024, 0 },
    { MAP_AVLEE, { 0, 1, 0, 1, 0, 1, 0 }, 3, 7913, 9476, 193, 0, 0 },
    { MAP_BRAKADA_DESERT, { 1, 0, 1, 0, 0, 0, 0 }, 6, 19171, -19722, 193, 1024, 0 },
    { MAP_TATALIA, { 1, 0, 1, 0, 1, 0, 0 }, 4, -2183, -6941, 97, 0, 0 },
    { MAP_PIERPONT, { 0, 0, 0, 0, 0, 1, 0 }, 6, -709, -14087, 193, 1024, 0 },  // for boat
    { MAP_STEADWICK, { 0, 0, 0, 0, 0, 0, 1 }, 6, -10471, 13497, 193, 1536, 0 },
    { MAP_EVENMORN_ISLE, { 0, 1, 0, 1, 0, 0, 0 }, 1, 15616, 6390, 193, 1536, QBIT_EVENMORN_MAP_FOUND },
    { MAP_BRAKADA_DESERT, { 0, 1, 0, 1, 0, 0, 0 }, 1, 19171, -19722, 193, 1024, 0 },
    { MAP_STEADWICK, { 0, 1, 0, 1, 0, 1, 0 }, 2, -10471, 13497, 193, 1536, 0 },
    { MAP_BRAKADA_DESERT, { 1, 0, 1, 0, 0, 0, 0 }, 4, 19171, -19722, 193, 1024, 0 },
    { MAP_EVENMORN_ISLE, { 0, 0, 0, 0, 0, 0, 1 }, 5, 15616, 6390, 193, 1536, QBIT_EVENMORN_MAP_FOUND },
    { MAP_AVLEE, { 0, 0, 0, 0, 1, 0, 0 }, 5, 7913, 9476, 193, 0, 0 },
    { MAP_STEADWICK, { 0, 1, 0, 0, 0, 1, 0 }, 4, -10471, 13497, 193, 1536, 0 },
    { MAP_PIERPONT, { 1, 0, 1, 0, 1, 0, 0 }, 3, -709, -14087, 193, 1024, 0 },
    { MAP_TATALIA, { 0, 0, 0, 1, 0, 0, 0 }, 5, -2183, -6941, 97, 0, 0 },
    { MAP_ARENA, { 0, 0, 0, 0, 0, 0, 1 }, 4, 3844, 2906, 193, 512, 0 }
};

unsigned char transport_routes[20][4] = {
    { 0, 1, 1, 34 },         // HOUSE_STABLES_HARMONDALE
    { 2, 3, 4, 5 },          // HOUSE_STABLES_STEADWICK
    { 6, 7, 8, 8 },          // HOUSE_STABLES_TULAREAN_FOREST
    { 9, 10, 10, 10 },       // HOUSE_STABLES_DEYJA
    { 11, 11, 12, 12 },      // HOUSE_STABLES_BRACADA_DESERT
    { 13, 13, 13, 13 },      // HOUSE_STABLES_TATALIA
    { 14, 14, 15, 15 },      // HOUSE_STABLES_AVLEE
    { 255, 255, 255, 255 },  // HOUSE_STABLES_61
    { 255, 255, 255, 255 },  // HOUSE_STABLES_62
    { 255, 255, 255, 255 },  // HOUSE_BOATS_EMERALD_ISLE
    { 16, 17, 18, 19 },      // HOUSE_BOATS_ERATHIA
    { 18, 20, 21, 21 },      // HOUSE_BOATS_TULAREAN_FOREST
    { 22, 23, 24, 25 },      // HOUSE_BOATS_BRACADA_DESERT
    { 22, 22, 23, 23 },      // HOUSE_BOATS_EVENMORN_ISLAND
    { 255, 255, 255, 255 },  // HOUSE_BOATS_68
    { 27, 28, 29, 30 },      // HOUSE_BOATS_TATALIA
    { 31, 32, 33, 33 },      // HOUSE_BOATS_AVLEE
    { 24, 24, 24, 24 },      // HOUSE_BOATS_71
    { 255, 255, 255, 255 },  // HOUSE_BOATS_72
    { 255, 255, 255, 255 }   // HOUSE_BOATS_73
};

int GetTravelTimeTransportDays(int schedule_id) {
    int travel_time = transport_schedule[schedule_id].uTravelTime;
    if (window_SpeakInHouse->wData.val >= HOUSE_BOATS_EMERALD_ISLE) {
        if (CheckHiredNPCSpeciality(Sailor)) travel_time -= 2;
        if (CheckHiredNPCSpeciality(Navigator)) travel_time -= 3;
        if (CheckHiredNPCSpeciality(Pirate)) travel_time -= 2;
    } else {
        if (CheckHiredNPCSpeciality(Horseman)) travel_time -= 2;
    }
    if (CheckHiredNPCSpeciality(Explorer)) --travel_time;
    if (travel_time < 1) travel_time = 1;
    return travel_time;
}

void TravelByTransport() {
    GUIWindow travel_window = *window_SpeakInHouse;
    travel_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    travel_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    travel_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    assert(pParty->hasActiveCharacter()); // code in this function couldn't handle pParty->activeCharacterIndex() = 0 and crash

    int pPrice = PriceCalculator::transportCostForPlayer(&pParty->activeCharacter(),
                                                         buildingTable[window_SpeakInHouse->wData.val - 1]);
    int route_id = window_SpeakInHouse->wData.val - HOUSE_STABLES_HARMONDALE;

    if (dialog_menu_id == DIALOGUE_MAIN) {
        if (HouseUI_CheckIfPlayerCanInteract()) {
            int index = 0;

            std::string travelcost = localization->FormatString(LSTR_FMT_TRAVEL_COST_D_GOLD, pPrice);
            int pTextHeight = pFontArrus->CalcTextHeight(travelcost, travel_window.uFrameWidth, 0);
            int pRealTextHeight = pTextHeight + (pFontArrus->GetHeight() - 3) + 146;
            int pPrimaryTextHeight = pRealTextHeight;

            int pCurrentButton = 2;
            int lastsched{ 255 };
            std::string pTopicArray[5]{};

            for (uint i = pDialogueWindow->pStartingPosActiveItem; i < (unsigned int)(pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem); ++i) {
                int schedule_id = transport_routes[route_id][index];
                GUIButton *pButton = pDialogueWindow->GetControl(i);

                bool route_active = 0;

                if (schedule_id != 255 && (lastsched != schedule_id)) {
                    Assert(schedule_id < 35);
                    if (pCurrentButton >= 6)
                        route_active = true;
                    else
                        route_active = transport_schedule[schedule_id].pSchedule[pParty->uCurrentDayOfMonth % 7];
                }

                lastsched = schedule_id;

                if (schedule_id != 255 && route_active &&
                    (!transport_schedule[schedule_id].uQuestBit || pParty->_questBits[transport_schedule[schedule_id].uQuestBit])) {
                    uint16_t color{};
                    if (pDialogueWindow->pCurrentPosActiveItem == pCurrentButton)
                        color = colorTable.PaleCanary.c16();
                    else
                        color = colorTable.White.c16();

                    pTopicArray[index] = fmt::format("\f{:05}", color);

                    int travel_time = GetTravelTimeTransportDays(schedule_id);

                    std::string str = localization->FormatString(LSTR_FMT_D_DAYS_TO_S, travel_time, pMapStats->pInfos[transport_schedule[schedule_id].uMapInfoID].pName.c_str());
                    pTopicArray[index] += str + "\n \n";
                    pButton->uY = pRealTextHeight;
                    pTextHeight = pFontArrus->CalcTextHeight(str, travel_window.uFrameWidth, 0);
                    pButton->uHeight = pTextHeight;
                    pButton->uW = pButton->uY + pTextHeight - 1 + 6;
                    pRealTextHeight += (pFontArrus->GetHeight() - 3) + pTextHeight;
                } else {
                    pTopicArray[index] = "";
                    if (pButton) {
                        pButton->uW = 0;
                        pButton->uHeight = 0;
                        pButton->uY = 0;
                    }
                }
                ++index;
                ++pCurrentButton;
            }

            if (pRealTextHeight != pPrimaryTextHeight) {
                // height differences means we have travel options
                travel_window.DrawTitleText(pFontArrus, 0, 146, Color(),
                    fmt::format("{}\n \n{}{}{}{}{}", travelcost, pTopicArray[0], pTopicArray[1], pTopicArray[2], pTopicArray[3], pTopicArray[4]), 3);
            } else {
                travel_window.DrawTitleText(pFontArrus, 0, (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - pFontArrus->CalcTextHeight(localization->GetString(LSTR_COME_BACK_ANOTHER_DAY), travel_window.uFrameWidth, 0)) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET,
                                            colorTable.White, localization->GetString(LSTR_COME_BACK_ANOTHER_DAY), 3);
            }
        }
    } else {  //после нажатия топика - travel option selected
        if (dialog_menu_id >= DIALOGUE_TRANSPORT_SCHEDULE_1 &&
            dialog_menu_id <= DIALOGUE_TRANSPORT_SCHEDULE_4) {
            if (pParty->GetGold() < pPrice) {
                GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
                // TODO(pskelton): correct sound but wrong label - travel house sounds might need different enum
                PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_Greeting_2);
                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                return;
            }

            int choice_id = dialog_menu_id - DIALOGUE_TRANSPORT_SCHEDULE_1;
            stru365_travel_info *pTravel = &transport_schedule[transport_routes[route_id][choice_id]];

            if (pTravel->pSchedule[pParty->uCurrentDayOfMonth % 7]) {
                if (pCurrentMapName != pMapStats->pInfos[pTravel->uMapInfoID].pFilename) {
                    SaveGame(1, 0);
                    pCurrentMapName = pMapStats->pInfos[pTravel->uMapInfoID].pFilename;

                    dword_6BE364_game_settings_1 |= GAME_SETTINGS_0001;
                    Party_Teleport_Cam_Pitch = 0;
                    Party_Teleport_Z_Speed = 0;
                    Party_Teleport_Cam_Yaw = pTravel->arrival_view_yaw;
                    uGameState = GAME_STATE_CHANGE_LOCATION;
                    Party_Teleport_X_Pos = pTravel->arrival_x;
                    Party_Teleport_Y_Pos = pTravel->arrival_y;
                    Party_Teleport_Z_Pos = pTravel->arrival_z;
                    Start_Party_Teleport_Flag = pTravel->arrival_x | pTravel->arrival_y | pTravel->arrival_z | pTravel->arrival_view_yaw;
                } else {
                    // travelling to map we are already in
                    pCamera3D->_viewYaw = 0;

                    pParty->uFlags |= PARTY_FLAGS_1_ForceRedraw;
                    pParty->vPosition.x = pTravel->arrival_x;
                    pParty->vPosition.y = pTravel->arrival_y;
                    pParty->vPosition.z = pTravel->arrival_z;
                    pParty->uFallStartZ = pParty->vPosition.z;
                    pParty->_viewPitch = 0;
                    pParty->_viewYaw = pTravel->arrival_view_yaw;
                }

                pParty->TakeGold(pPrice);
                // TODO(pskelton): correct sound but wrong label - travel house sounds might need different enum
                PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_NotEnoughMoney);
                int traveltimedays = GetTravelTimeTransportDays(transport_routes[route_id][choice_id]);

                PlayerSpeech pSpeech;
                if (route_id >= HOUSE_BOATS_EMERALD_ISLE - HOUSE_STABLES_HARMONDALE) {
                    pSpeech = SPEECH_TravelBoat;
                } else {
                    pSpeech = SPEECH_TravelHorse;
                }

                restAndHeal(GameTime::FromDays(traveltimedays));
                pParty->activeCharacter().playReaction(pSpeech);
                pAudioPlayer->soundDrain();
                while (HouseDialogPressCloseBtn()) {}
                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
            } else {
                dialog_menu_id = DIALOGUE_MAIN;
                pAudioPlayer->playUISound(SOUND_error);
            }
        }
    }
}

bool IsTravelAvailable(int a1) {
    for (uint i = 0; i < 4; ++i) {
        if (transport_schedule[transport_routes[a1][i]]
            .pSchedule[pParty->uCurrentDayOfMonth % 7]) {
            if (!transport_schedule[transport_routes[a1][i]].uQuestBit || pParty->_questBits[transport_schedule[transport_routes[a1][i]].uQuestBit]) {
                return true;
            }
        }
    }
    return false;
}
