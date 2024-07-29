#include "Transport.h"

#include <string>
#include <array>
#include <vector>

#include "GUI/UI/UIStatusBar.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIMessageQueue.h"

#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/SaveLoad.h"
#include "Engine/PriceCalculator.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Objects/NPC.h"
#include "Engine/MapInfo.h"
#include "Engine/Party.h"
#include "Engine/Engine.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/IndexedArray.h"

struct TransportInfo {
    MapId uMapInfoID;
    std::array<unsigned char, 7> pSchedule;
    int uTravelTime; // In days.
    Vec3f arrivalPos;
    int arrival_view_yaw;
    QuestBit uQuestBit;  // quest bit required to set for this travel option to be enabled; otherwise 0
};

// 004F09B0
static constexpr std::array<TransportInfo, 35> transportSchedule = {{
//    location name        schedule            days  pos                     yaw   qbit
    { MAP_ERATHIA,         {1, 0, 1, 0, 1, 0, 0}, 2, {-18048,  4636,  833},  1536, QBIT_INVALID },  // for stable
    { MAP_TULAREAN_FOREST, {0, 1, 0, 1, 0, 1, 0}, 2, {-2527,  -6773,  1153}, 896,  QBIT_INVALID },
    { MAP_TATALIA,         {1, 0, 1, 0, 1, 0, 0}, 2, { 4730,  -10580, 320},  1024, QBIT_INVALID },
    { MAP_HARMONDALE,      {0, 1, 0, 1, 0, 1, 0}, 2, {-5692,   11137, 1},    1024, QBIT_INVALID },
    { MAP_DEYJA,           {1, 0, 0, 1, 0, 0, 0}, 3, { 7227,  -16007, 2625}, 640,  QBIT_INVALID },
    { MAP_BRACADA_DESERT,  {0, 0, 1, 0, 0, 1, 0}, 3, { 8923,   17191, 1},    512,  QBIT_INVALID },
    { MAP_AVLEE,           {1, 0, 1, 0, 1, 0, 0}, 3, { 17059,  12331, 512},  1152, QBIT_INVALID },
    { MAP_DEYJA,           {0, 1, 0, 0, 1, 0, 1}, 2, { 7227,  -16007, 2625}, 640,  QBIT_INVALID },
    { MAP_HARMONDALE,      {0, 1, 0, 1, 0, 1, 0}, 2, {-5692,   11137, 1},    1024, QBIT_INVALID },
    { MAP_ERATHIA,         {1, 0, 1, 0, 1, 0, 0}, 3, {-18048,  4636,  833},  1536, QBIT_INVALID },
    { MAP_TULAREAN_FOREST, {0, 1, 0, 1, 0, 1, 0}, 2, {-2527,  -6773,  1153}, 896,  QBIT_INVALID },
    { MAP_ERATHIA,         {1, 0, 1, 0, 1, 0, 1}, 3, {-18048,  4636,  833},  1536, QBIT_INVALID },
    { MAP_HARMONDALE,      {0, 1, 0, 0, 0, 1, 0}, 5, {-5692,   11137, 1},    1024, QBIT_INVALID },
    { MAP_ERATHIA,         {0, 1, 0, 1, 0, 1, 0}, 2, {-18048,  4636,  833},  1536, QBIT_INVALID },
    { MAP_TULAREAN_FOREST, {0, 1, 0, 1, 0, 1, 0}, 3, {-2527,  -6773,  1153}, 896,  QBIT_INVALID },
    { MAP_DEYJA,           {0, 0, 1, 0, 0, 0, 1}, 5, { 7227,  -16007, 2625}, 640,  QBIT_INVALID },
    { MAP_TATALIA,         {0, 1, 0, 1, 0, 1, 0}, 2, {-2183,  -6941,  97},   0,    QBIT_INVALID },
    { MAP_AVLEE,           {1, 0, 0, 0, 1, 0, 0}, 4, { 7913,   9476,  193},  0,    QBIT_INVALID },
    { MAP_EVENMORN_ISLAND, {0, 0, 0, 0, 0, 0, 1}, 7, { 15616,  6390,  193},  1536, QBIT_EVENMORN_MAP_FOUND },
    { MAP_BRACADA_DESERT,  {0, 0, 1, 0, 0, 0, 0}, 6, { 19171, -19722, 193},  1024, QBIT_INVALID },
    { MAP_AVLEE,           {0, 1, 0, 1, 0, 1, 0}, 3, { 7913,   9476,  193},  0,    QBIT_INVALID },
    { MAP_BRACADA_DESERT,  {1, 0, 1, 0, 0, 0, 0}, 6, { 19171, -19722, 193},  1024, QBIT_INVALID },
    { MAP_TATALIA,         {1, 0, 1, 0, 1, 0, 0}, 4, {-2183,  -6941,  97},   0,    QBIT_INVALID },
    { MAP_TULAREAN_FOREST, {0, 0, 0, 0, 0, 1, 0}, 6, {-709,   -14087, 193},  1024, QBIT_INVALID },  // for boat
    { MAP_ERATHIA,         {0, 0, 0, 0, 0, 0, 1}, 6, {-10471,  13497, 193},  1536, QBIT_INVALID },
    { MAP_EVENMORN_ISLAND, {0, 1, 0, 1, 0, 0, 0}, 1, { 15616,  6390,  193},  1536, QBIT_EVENMORN_MAP_FOUND },
    { MAP_BRACADA_DESERT,  {0, 1, 0, 1, 0, 0, 0}, 1, { 19171, -19722, 193},  1024, QBIT_INVALID },
    { MAP_ERATHIA,         {0, 1, 0, 1, 0, 1, 0}, 2, {-10471,  13497, 193},  1536, QBIT_INVALID },
    { MAP_BRACADA_DESERT,  {1, 0, 1, 0, 0, 0, 0}, 4, { 19171, -19722, 193},  1024, QBIT_INVALID },
    { MAP_EVENMORN_ISLAND, {0, 0, 0, 0, 0, 0, 1}, 5, { 15616,  6390,  193},  1536, QBIT_EVENMORN_MAP_FOUND },
    { MAP_AVLEE,           {0, 0, 0, 0, 1, 0, 0}, 5, { 7913,   9476,  193},  0,    QBIT_INVALID },
    { MAP_ERATHIA,         {0, 1, 0, 0, 0, 1, 0}, 4, {-10471,  13497, 193},  1536, QBIT_INVALID },
    { MAP_TULAREAN_FOREST, {1, 0, 1, 0, 1, 0, 0}, 3, {-709,   -14087, 193},  1024, QBIT_INVALID },
    { MAP_TATALIA,         {0, 0, 0, 1, 0, 0, 0}, 5, {-2183,  -6941,  97},   0,    QBIT_INVALID },
    { MAP_ARENA,           {0, 0, 0, 0, 0, 0, 1}, 4, { 3844,   2906,  193},  512,  QBIT_INVALID }
}};

static constexpr IndexedArray<std::array<int, 4>, HOUSE_FIRST_TRANSPORT, HOUSE_LAST_TRANSPORT> transportRoutes = {
    {HOUSE_STABLE_HARMONDALE,       { 0, 1, 1, 34 }},
    {HOUSE_STABLE_ERATHIA,          { 2, 3, 4, 5 }},
    {HOUSE_STABLE_TULAREAN_FOREST,  { 6, 7, 8, 8 }},
    {HOUSE_STABLE_DEYJA,            { 9, 10, 10, 10 }},
    {HOUSE_STABLE_BRACADA_DESERT,   { 11, 11, 12, 12 }},
    {HOUSE_STABLE_TATALIA,          { 13, 13, 13, 13 }},
    {HOUSE_STABLE_AVLEE,            { 14, 14, 15, 15 }},
    {HOUSE_61,                      { 255, 255, 255, 255 }},
    {HOUSE_62,                      { 255, 255, 255, 255 }},
    {HOUSE_BOAT_EMERALD_ISLAND,     { 255, 255, 255, 255 }},
    {HOUSE_BOAT_ERATHIA,            { 16, 17, 18, 19 }},
    {HOUSE_BOAT_TULAREAN_FOREST,    { 18, 20, 21, 21 }},
    {HOUSE_BOAT_BRACADA_DESERT,     { 22, 23, 24, 25 }},
    {HOUSE_BOAT_EVENMORN_ISLAND,    { 22, 22, 23, 23 }},
    {HOUSE_68,                      { 255, 255, 255, 255 }},
    {HOUSE_BOAT_TATALIA,            { 27, 28, 29, 30 }},
    {HOUSE_BOAT_AVLEE,              { 31, 32, 33, 33 }},
    {HOUSE_71,                      { 24, 24, 24, 24 }},
    {HOUSE_72,                      { 255, 255, 255, 255 }},
    {HOUSE_73,                      { 255, 255, 255, 255 }}
};

void GUIWindow_Transport::mainDialogue() {
    GUIWindow travel_window = *this;
    travel_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    travel_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    travel_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    assert(pParty->hasActiveCharacter()); // code in this function couldn't handle pParty->activeCharacterIndex() = 0 and crash

    if (!checkIfPlayerCanInteract()) {
        return;
    }

    std::vector<std::string> optionsText;
    int price = PriceCalculator::transportCostForPlayer(&pParty->activeCharacter(), buildingTable[houseId()]);
    std::string travelCost = localization->FormatString(LSTR_FMT_TRAVEL_COST_D_GOLD, price);
    int startingOffset = assets->pFontArrus->CalcTextHeight(travelCost, travel_window.uFrameWidth, 0) + (assets->pFontArrus->GetHeight() - 3) + 146;
    int lastsched = 255;
    bool hasActiveRoute = false;

    for (int schedule_id : transportRoutes[houseId()]) {
        bool routeActive = false;

        if (schedule_id != 255 && (lastsched != schedule_id)) {
            assert(schedule_id < transportSchedule.size());
            routeActive = transportSchedule[schedule_id].pSchedule[pParty->uCurrentDayOfMonth % 7];
        }

        lastsched = schedule_id;

        if (routeActive && (transportSchedule[schedule_id].uQuestBit == QBIT_INVALID || pParty->_questBits[transportSchedule[schedule_id].uQuestBit])) {
            int travel_time = getTravelTimeTransportDays(schedule_id);
            optionsText.push_back(localization->FormatString(LSTR_FMT_D_DAYS_TO_S, travel_time, pMapStats->pInfos[transportSchedule[schedule_id].uMapInfoID].name));
            hasActiveRoute = true;
        } else {
            optionsText.push_back("");
        }
    }

    if (hasActiveRoute) {
        travel_window.DrawTitleText(assets->pFontArrus.get(), 0, 146, colorTable.White, travelCost, 3);
        drawOptions(optionsText, colorTable.PaleCanary, startingOffset, true);
    } else {
        int textHeight = assets->pFontArrus->CalcTextHeight(localization->GetString(LSTR_COME_BACK_ANOTHER_DAY), travel_window.uFrameWidth, 0);
        int vertMargin = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - textHeight) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
        travel_window.DrawTitleText(assets->pFontArrus.get(), 0, vertMargin, colorTable.White, localization->GetString(LSTR_COME_BACK_ANOTHER_DAY), 3);
    }
}

void GUIWindow_Transport::transportDialogue() {
    int pPrice = PriceCalculator::transportCostForPlayer(&pParty->activeCharacter(), buildingTable[houseId()]);

    if (pParty->GetGold() < pPrice) {
        engine->_statusBar->setEvent(LSTR_NOT_ENOUGH_GOLD);
        playHouseSound(houseId(), HOUSE_SOUND_TRANSPORT_NOT_ENOUGH_GOLD);
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        return;
    }

    int choice_id = std::to_underlying(_currentDialogue) - std::to_underlying(DIALOGUE_TRANSPORT_SCHEDULE_1);
    const TransportInfo *pTravel = &transportSchedule[transportRoutes[houseId()][choice_id]];

    if (pTravel->pSchedule[pParty->uCurrentDayOfMonth % 7]) {
        if (engine->_currentLoadedMapId != pTravel->uMapInfoID) {
            AutoSave();
            engine->_transitionMapId = pTravel->uMapInfoID;

            dword_6BE364_game_settings_1 |= GAME_SETTINGS_SKIP_WORLD_UPDATE;
            uGameState = GAME_STATE_CHANGE_LOCATION;
            engine->_teleportPoint.setTeleportTarget(pTravel->arrivalPos, pTravel->arrival_view_yaw, 0, 0);
        } else {
            // travelling to map we are already in
            pCamera3D->_viewYaw = 0;
            pParty->pos = pTravel->arrivalPos;
            pParty->uFallStartZ = pParty->pos.z;
            pParty->_viewPitch = 0;
            pParty->_viewYaw = pTravel->arrival_view_yaw;
        }

        pParty->TakeGold(pPrice);
        playHouseSound(houseId(), HOUSE_SOUND_TRANSPORT_TRAVEL);

        CharacterSpeech pSpeech;
        if (isBoat(houseId())) {
            pSpeech = SPEECH_TRAVEL_BOAT;
        } else {
            pSpeech = SPEECH_TRAVEL_HORSE;
        }

        restAndHeal(Duration::fromDays(getTravelTimeTransportDays(transportRoutes[houseId()][choice_id])));
        pParty->activeCharacter().playReaction(pSpeech);
        pAudioPlayer->soundDrain();
        while (houseDialogPressEscape()) {}
    } else {
        pAudioPlayer->playUISound(SOUND_error);
    }
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
}

void GUIWindow_Transport::houseSpecificDialogue() {
    assert(pParty->hasActiveCharacter()); // code in this function couldn't handle pParty->activeCharacterIndex() = 0 and crash

    switch (_currentDialogue) {
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

void GUIWindow_Transport::houseDialogueOptionSelected(DialogueId option) {
    _currentDialogue = option;
}

std::vector<DialogueId> GUIWindow_Transport::listDialogueOptions() {
    switch (_currentDialogue) {
      case DIALOGUE_MAIN:
        return {DIALOGUE_TRANSPORT_SCHEDULE_1, DIALOGUE_TRANSPORT_SCHEDULE_2, DIALOGUE_TRANSPORT_SCHEDULE_3, DIALOGUE_TRANSPORT_SCHEDULE_4};
      default:
        return {};
    }
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

bool isTravelAvailable(HouseId houseId) {
    for (int schedule : transportRoutes[houseId]) {
        if (transportSchedule[schedule].pSchedule[pParty->uCurrentDayOfMonth % 7]) {
            if (transportSchedule[schedule].uQuestBit == QBIT_INVALID || pParty->_questBits[transportSchedule[schedule].uQuestBit]) {
                return true;
            }
        }
    }
    return false;
}
