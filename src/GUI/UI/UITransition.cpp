#include "UITransition.h"

#include <cstdlib>
#include <string>

#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/Tables/HouseTable.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/MapInfo.h"
#include "Engine/Party.h"
#include "Engine/Time/Timer.h"
#include "Engine/Tables/TransitionTable.h"
#include "Engine/mm7_data.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIDialogue.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

#include "Library/Logger/Logger.h"

#include "Utility/String/Ascii.h"

GraphicsImage *transition_ui_icon = nullptr;

/**
 * all locations which should have special tranfer message:
 * dragon caves, markham, bandit cave, haunted mansion,
 * barrow 7, barrow 9, barrow 10, setag tower,
 * wromthrax cave, toberti, hidden tomb
 * TODO(Nik-RE-dev): Use location enums here.
 */
std::array<std::string, 11> specialTransferMessageLocationNames = {
    "mdt12.blv", "d18.blv",   "mdt14.blv", "d37.blv",
    "mdk01.blv", "mdt01.blv", "mdr01.blv", "mdt10.blv",
    "mdt09.blv", "mdt15.blv", "mdt11.blv"};

/**
 * @offset 0x444810
 * @return Index of special transfer message, 0 otherwise
 */
int getSpecialTransferMessageIndex(std::string_view locationName) {
    for (unsigned i = 0; i < specialTransferMessageLocationNames.size(); ++i)
        if (ascii::noCaseEquals(locationName, specialTransferMessageLocationNames[i]))
            return i + 1;
    return 0;
}

GUIWindow_Transition::GUIWindow_Transition(WindowType windowType, ScreenType screenType) : GUIWindow(windowType, {0, 0}, render->GetRenderDimensions()) {
    pEventTimer->setPaused(true);

    game_ui_dialogue_background = assets->getImage_Solid(dialogueBackgroundResourceByAlignment[pParty->alignment]);

    prev_screen_type = current_screen_type;
    current_screen_type = screenType;
}

void GUIWindow_Transition::createButtons(const std::string &okHint, const std::string &cancelHint, UIMessageType confirmMsg, UIMessageType cancelMsg) {
    this->sHint = okHint;

    pBtn_ExitCancel = CreateButton({556, 445}, {75, 33}, 1, 0, cancelMsg, 0, Io::InputAction::No, cancelHint, {ui_buttdesc2});
    pBtn_YES = CreateButton({476, 445}, {75, 33}, 1, 0, confirmMsg, 0, Io::InputAction::Yes, okHint, {ui_buttyes2});
    CreateButton({pNPCPortraits_x[0][0], pNPCPortraits_y[0][0]}, {63, 73}, 1, 0, confirmMsg, 1, Io::InputAction::EventTrigger, okHint);
    CreateButton({8, 8}, {460, 344}, 1, 0, confirmMsg, 1, Io::InputAction::Invalid, okHint);
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

GUIWindow_Travel::GUIWindow_Travel() : GUIWindow_Transition(WINDOW_Travel, SCREEN_CHANGE_LOCATION) {
    std::string hint;

    transition_ui_icon = assets->getImage_Solid("outside");

    if (engine->_currentLoadedMapId != MAP_INVALID) {
        hint = localization->FormatString(LSTR_LEAVE_S, pMapStats->pInfos[engine->_currentLoadedMapId].name);
    } else {
        hint = localization->GetString(LSTR_EXIT_DIALOGUE);
    }

    createButtons(hint, localization->GetString(LSTR_STAY_IN_THIS_AREA), UIMSG_OnTravelByFoot, UIMSG_CancelTravelByFoot);
}

void GUIWindow_Travel::Update() {
    MapId destinationMap = pOutdoor->getTravelDestination(pParty->pos.x, pParty->pos.y);

    render->DrawTextureNew(477 / 640.0f, 0, game_ui_dialogue_background);
    render->DrawTextureNew(468 / 640.0f, 0, game_ui_right_panel_frame);
    render->DrawTextureNew(pNPCPortraits_x[0][0] / 640.0f, pNPCPortraits_y[0][0] / 480.0f, transition_ui_icon);
    render->DrawTextureNew(556 / 640.0f, 451 / 480.0f, dialogue_ui_x_x_u);
    render->DrawTextureNew(476 / 640.0f, 451 / 480.0f, dialogue_ui_x_ok_u);
    if (destinationMap != MAP_INVALID) {
        GUIWindow travel_window = *pPrimaryWindow;
        travel_window.uFrameX = 493;
        travel_window.uFrameWidth = 126;
        travel_window.uFrameZ = 366;
        travel_window.DrawTitleText(assets->pFontCreate.get(), 0, 4, colorTable.White, pMapStats->pInfos[destinationMap].name, 3);
        travel_window.uFrameX = SIDE_TEXT_BOX_POS_X;
        travel_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
        travel_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

        std::string str;
        if (getTravelTime() == 1) {
            str = localization->FormatString(LSTR_IT_WILL_TAKE_D_DAY_TO_CROSS_TO_S, 1, pMapStats->pInfos[destinationMap].name);
        } else {
            str = localization->FormatString(LSTR_IT_WILL_TAKE_D_DAYS_TO_TRAVEL_TO_S, getTravelTime(), pMapStats->pInfos[destinationMap].name);
        }
        str += "\n \n";
        str += localization->FormatString(LSTR_DO_YOU_WISH_TO_LEAVE_S_1, pMapStats->pInfos[engine->_currentLoadedMapId].name);

        travel_window.DrawTitleText(assets->pFontCreate.get(), 0, (212 - assets->pFontCreate->CalcTextHeight(str, travel_window.uFrameWidth, 0)) / 2 + 101, colorTable.White, str, 3);
    }
}

//----- (00444839) --------------------------------------------------------
GUIWindow_IndoorEntryExit::GUIWindow_IndoorEntryExit(HouseId transitionHouse, unsigned exit_pic_id, Vec3f pos, int yaw,
                                                     int pitch, int zspeed, std::string_view locationName)
    : GUIWindow_Transition(WINDOW_IndoorEntryExit, SCREEN_INPUT_BLV) {
    std::string hint;

    engine->_teleportPoint.setTeleportTarget(pos, yaw, pitch, zspeed);
    engine->_teleportPoint.setTeleportMap(locationName);
    _transitionStringId = std::to_underlying(transitionHouse); // TODO(Nik-RE-dev): is this correct?

    _mapName = locationName;

    transition_ui_icon = assets->getImage_Solid(pHouse_ExitPictures[exit_pic_id]);

    // animation or special transfer message
    if (transitionHouse != HOUSE_INVALID || getSpecialTransferMessageIndex(locationName)) {
        // TODO(Nik-RE-dev): if message is special then no video when entering indoor?
        if (!getSpecialTransferMessageIndex(locationName))
            pMediaPlayer->OpenHouseMovie(pAnimatedRooms[houseTable[transitionHouse].uAnimationID].video_name, 1);

        std::string destMap = std::string(locationName);
        if (locationName[0] == '0') {
            destMap = pMapStats->pInfos[engine->_currentLoadedMapId].fileName;
        }
        if (pMapStats->GetMapInfo(destMap) != MAP_INVALID) {
            hint = localization->FormatString(LSTR_ENTER_S, pMapStats->pInfos[pMapStats->GetMapInfo(destMap)].name);
        } else {
            hint = localization->GetString(LSTR_EXIT_DIALOGUE);
            if (transitionHouse != HOUSE_INVALID && pAnimatedRooms[houseTable[transitionHouse].uAnimationID].uRoomSoundId)
                playHouseSound(transitionHouse, HOUSE_SOUND_GENERAL_GREETING);
        }
        if (uCurrentlyLoadedLevelType == LEVEL_INDOOR && pParty->hasActiveCharacter() && pParty->GetRedOrYellowAlert())
            pParty->activeCharacter().playReaction(SPEECH_LEAVE_DUNGEON);
        if (getSpecialTransferMessageIndex(locationName))
            _transitionStringId = getSpecialTransferMessageIndex(locationName);
    } else if (!getSpecialTransferMessageIndex(locationName)) { // transfer to outdoors - no special message
        if (engine->_currentLoadedMapId != MAP_INVALID) {
            hint = localization->FormatString(LSTR_LEAVE_S, pMapStats->pInfos[engine->_currentLoadedMapId].name);
        } else {
            hint = localization->GetString(LSTR_EXIT_DIALOGUE);
        }
        if (transitionHouse != HOUSE_INVALID && pAnimatedRooms[houseTable[transitionHouse].uAnimationID].uRoomSoundId)
            playHouseSound(transitionHouse, HOUSE_SOUND_GENERAL_GREETING);
        if (uCurrentlyLoadedLevelType == LEVEL_INDOOR && pParty->hasActiveCharacter() && pParty->GetRedOrYellowAlert())
            pParty->activeCharacter().playReaction(SPEECH_LEAVE_DUNGEON);
    }

    createButtons(hint, localization->GetString(LSTR_CANCEL), UIMSG_OnIndoorEntryExit, UIMSG_CancelIndoorEntryExit);
}

void GUIWindow_IndoorEntryExit::Update() {
    render->DrawTextureNew(477 / 640.0f, 0, game_ui_dialogue_background);
    render->DrawTextureNew((pNPCPortraits_x[0][0] - 4) / 640.0f, (pNPCPortraits_y[0][0] - 4) / 480.0f, game_ui_evtnpc);
    render->DrawTextureNew(pNPCPortraits_x[0][0] / 640.0f, pNPCPortraits_y[0][0] / 480.0f, transition_ui_icon);
    render->DrawTextureNew(468 / 640.0f, 0, game_ui_right_panel_frame);
    render->DrawTextureNew(556 / 640.0f, 451 / 480.0f, dialogue_ui_x_x_u);
    render->DrawTextureNew(476 / 640.0f, 451 / 480.0f, dialogue_ui_x_ok_u);

    MapId map_id = engine->_currentLoadedMapId;
    // TODO(captainurist): mm7 map names never starts with ' ', what is this check?
    if ((pMovie_Track || getSpecialTransferMessageIndex(_mapName)) && !engine->_teleportPoint.getTeleportMap().starts_with(' ')) {
        map_id = pMapStats->GetMapInfo(_mapName);
    }

    GUIWindow transition_window = *pPrimaryWindow;
    transition_window.uFrameX = 493;
    transition_window.uFrameWidth = 126;
    transition_window.uFrameZ = 366;
    transition_window.DrawTitleText(assets->pFontCreate.get(), 0, 5, colorTable.White, pMapStats->pInfos[map_id].name, 3);
    transition_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    transition_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    transition_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    if (_transitionStringId) {
        unsigned int vertMargin = (212 - assets->pFontCreate->CalcTextHeight(pTransitionStrings[_transitionStringId], transition_window.uFrameWidth, 0)) / 2 + 101;
        transition_window.DrawTitleText(assets->pFontCreate.get(), 0, vertMargin, colorTable.White, pTransitionStrings[_transitionStringId], 3);
    } else if (map_id != MAP_INVALID) {
        std::string str = localization->FormatString(LSTR_DO_YOU_WISH_TO_LEAVE_S_2, pMapStats->pInfos[map_id].name);
        unsigned int vertMargin = (212 - assets->pFontCreate->CalcTextHeight(str, transition_window.uFrameWidth, 0)) / 2 + 101;
        transition_window.DrawTitleText(assets->pFontCreate.get(), 0, vertMargin, colorTable.White, str, 3);
    } else {
        logger->error("Troubles in da house");
    }
}
