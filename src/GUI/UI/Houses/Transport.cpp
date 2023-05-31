#include <string>
#include <array>

#include "GUI/UI/Houses/Transport.h"

#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UIShops.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"

#include "Engine/Localization.h"
#include "Engine/SaveLoad.h"
#include "Engine/PriceCalculator.h"
#include "Engine/Graphics/Camera.h"

#include "Utility/IndexedArray.h"

struct TransportInfo {
    unsigned char uMapInfoID;
    unsigned char pSchedule[7];
    unsigned int uTravelTime; // In days.
    int arrival_x;
    int arrival_y;
    int arrival_z;
    int arrival_view_yaw;
    unsigned int uQuestBit;  // quest bit required to set for this travel option to be enabled; otherwise 0
};

// 004F09B0
std::array<TransportInfo, 35> transportSchedule = {{
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
}};

IndexedArray<std::array<unsigned char, 4>, HOUSE_FIRST_TRANSPORT, HOUSE_LAST_TRANSPORT> transportRoutes = {
    {HOUSE_STABLES_HARMONDALE,      { 0, 1, 1, 34 }},
    {HOUSE_STABLES_STEADWICK,       { 2, 3, 4, 5 }},
    {HOUSE_STABLES_TULAREAN_FOREST, { 6, 7, 8, 8 }},
    {HOUSE_STABLES_DEYJA,           { 9, 10, 10, 10 }},
    {HOUSE_STABLES_BRACADA_DESERT,  { 11, 11, 12, 12 }},
    {HOUSE_STABLES_TATALIA,         { 13, 13, 13, 13 }},
    {HOUSE_STABLES_AVLEE,           { 14, 14, 15, 15 }},
    {HOUSE_STABLES_61,              { 255, 255, 255, 255 }},
    {HOUSE_STABLES_62,              { 255, 255, 255, 255 }},
    {HOUSE_BOATS_EMERALD_ISLE,      { 255, 255, 255, 255 }},
    {HOUSE_BOATS_ERATHIA,           { 16, 17, 18, 19 }},
    {HOUSE_BOATS_TULAREAN_FOREST,   { 18, 20, 21, 21 }},
    {HOUSE_BOATS_BRACADA_DESERT,    { 22, 23, 24, 25 }},
    {HOUSE_BOATS_EVENMORN_ISLAND,   { 22, 22, 23, 23 }},
    {HOUSE_BOATS_PIT,               { 255, 255, 255, 255 }},
    {HOUSE_BOATS_TATALIA,           { 27, 28, 29, 30 }},
    {HOUSE_BOATS_AVLEE,             { 31, 32, 33, 33 }},
    {HOUSE_BOATS_CELESTE,           { 24, 24, 24, 24 }},
    {HOUSE_BOATS_72,                { 255, 255, 255, 255 }},
    {HOUSE_BOATS_73,                { 255, 255, 255, 255 }}
};

void GUIWindow_Transport::mainDialogue() {
    GUIWindow travel_window = *this;
    travel_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    travel_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    travel_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    assert(pParty->hasActiveCharacter()); // code in this function couldn't handle pParty->activeCharacterIndex() = 0 and crash

    int pPrice = PriceCalculator::transportCostForPlayer(&pParty->activeCharacter(), buildingTable[wData.val - 1]);

    if (HouseUI_CheckIfPlayerCanInteract()) {
        int index = 0;

        std::string travelcost = localization->FormatString(LSTR_FMT_TRAVEL_COST_D_GOLD, pPrice);
        int pTextHeight = pFontArrus->CalcTextHeight(travelcost, travel_window.uFrameWidth, 0);
        int pRealTextHeight = pTextHeight + (pFontArrus->GetHeight() - 3) + 146;
        int pPrimaryTextHeight = pRealTextHeight;

        int pCurrentButton = 2;
        int lastsched{ 255 };
        std::string pTopicArray[5]{};

        for (int i = pDialogueWindow->pStartingPosActiveItem; i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i) {
            int schedule_id = transportRoutes[houseId()][index];
            GUIButton *pButton = pDialogueWindow->GetControl(i);

            bool route_active = 0;

            if (schedule_id != 255 && (lastsched != schedule_id)) {
                Assert(schedule_id < 35);
                if (pCurrentButton >= 6)
                    route_active = true;
                else
                    route_active = transportSchedule[schedule_id].pSchedule[pParty->uCurrentDayOfMonth % 7];
            }

            lastsched = schedule_id;

            if (schedule_id != 255 && route_active && (!transportSchedule[schedule_id].uQuestBit || pParty->_questBits[transportSchedule[schedule_id].uQuestBit])) {
                Color color;
                if (pDialogueWindow->pCurrentPosActiveItem == pCurrentButton)
                    color = colorTable.PaleCanary;
                else
                    color = colorTable.White;

                pTopicArray[index] = fmt::format("{::}", color.tag());

                int travel_time = getTravelTimeTransportDays(schedule_id);

                std::string str = localization->FormatString(LSTR_FMT_D_DAYS_TO_S, travel_time, pMapStats->pInfos[transportSchedule[schedule_id].uMapInfoID].pName.c_str());
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
            std::string text = fmt::format("{}\n \n{}{}{}{}{}", travelcost, pTopicArray[0], pTopicArray[1], pTopicArray[2], pTopicArray[3], pTopicArray[4]);
            travel_window.DrawTitleText(pFontArrus, 0, 146, Color(), text, 3);
        } else {
            int textHeight = pFontArrus->CalcTextHeight(localization->GetString(LSTR_COME_BACK_ANOTHER_DAY), travel_window.uFrameWidth, 0);
            int vertMargin = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - textHeight) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
            travel_window.DrawTitleText(pFontArrus, 0, vertMargin, colorTable.White, localization->GetString(LSTR_COME_BACK_ANOTHER_DAY), 3);
        }
    }
}

void GUIWindow_Transport::transportDialogue() {
    int pPrice = PriceCalculator::transportCostForPlayer(&pParty->activeCharacter(), buildingTable[wData.val - 1]);

    if (pParty->GetGold() < pPrice) {
        GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
        // TODO(pskelton): correct sound but wrong label - travel house sounds might need different enum
        PlayHouseSound(wData.val, HouseSound_Greeting_2);
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        return;
    }

    int choice_id = dialog_menu_id - DIALOGUE_TRANSPORT_SCHEDULE_1;
    TransportInfo *pTravel = &transportSchedule[transportRoutes[houseId()][choice_id]];

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
        PlayHouseSound(wData.val, HouseSound_NotEnoughMoney);

        PlayerSpeech pSpeech;
        if (isBoat(houseId())) {
            pSpeech = SPEECH_TravelBoat;
        } else {
            pSpeech = SPEECH_TravelHorse;
        }

        restAndHeal(GameTime::FromDays(getTravelTimeTransportDays(transportRoutes[houseId()][choice_id])));
        pParty->activeCharacter().playReaction(pSpeech);
        pAudioPlayer->soundDrain();
        while (HouseDialogPressCloseBtn()) {}
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
    } else {
        dialog_menu_id = DIALOGUE_MAIN;
        pAudioPlayer->playUISound(SOUND_error);
    }
}

void GUIWindow_Transport::houseSpecificDialogue() {
    assert(pParty->hasActiveCharacter()); // code in this function couldn't handle pParty->activeCharacterIndex() = 0 and crash

    switch (dialog_menu_id) {
      case DIALOGUE_MAIN:
        mainDialogue();
        break;
      case DIALOGUE_TRANSPORT_SCHEDULE_1:
      case DIALOGUE_TRANSPORT_SCHEDULE_2:
      case DIALOGUE_TRANSPORT_SCHEDULE_3:
      case DIALOGUE_TRANSPORT_SCHEDULE_4:
        transportDialogue();
        break;
      default:
        break;
    }
}

void GUIWindow_Transport::houseDialogueOptionSelected(DIALOGUE_TYPE option) {
    // Nothing
}

int GUIWindow_Transport::getTravelTimeTransportDays(int schedule_id) {
    int travel_time = transportSchedule[schedule_id].uTravelTime;
    if (isBoat(houseId())) {
        if (CheckHiredNPCSpeciality(Sailor))
            travel_time -= 2;
        if (CheckHiredNPCSpeciality(Navigator))
            travel_time -= 3;
        if (CheckHiredNPCSpeciality(Pirate))
            travel_time -= 2;
    } else {
        if (CheckHiredNPCSpeciality(Horseman))
            travel_time -= 2;
    }
    if (CheckHiredNPCSpeciality(Explorer))
        travel_time -= 1;
    if (travel_time < 1)
        travel_time = 1;
    return travel_time;
}

bool isTravelAvailable(HOUSE_ID houseId) {
    for (char schedule : transportRoutes[houseId]) {
        if (transportSchedule[schedule].pSchedule[pParty->uCurrentDayOfMonth % 7]) {
            if (!transportSchedule[schedule].uQuestBit || pParty->_questBits[transportSchedule[schedule].uQuestBit]) {
                return true;
            }
        }
    }
    return false;
}
