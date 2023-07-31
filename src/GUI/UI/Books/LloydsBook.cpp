#include <string>

#include "Engine/Engine.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/SaveLoad.h"
#include "Engine/AssetsManager.h"
#include "Engine/Events/Processor.h"
#include "Engine/Spells/Spells.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/MapInfo.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/Books/LloydsBook.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

std::array<int, 5> lloydsBeaconsPreviewXs = {{61, 281, 61, 281, 171}};
std::array<int, 5> lloydsBeaconsPreviewYs = {{84, 84, 228, 228, 155}};
std::array<int, 5> lloydsBeacons_SomeXs = {{59, 279, 59, 279, 169}};
std::array<int, 5> lloydsBeacons_SomeYs = {{82, 82, 226, 226, 153}};

GraphicsImage *ui_book_lloyds_border = nullptr;
std::array<GraphicsImage *, 2> ui_book_lloyds_backgrounds;

GUIWindow_LloydsBook::GUIWindow_LloydsBook(int casterId, int spellLevel) : GUIWindow_Book() {
    this->eWindowType = WindowType::WINDOW_LloydsBeacon;
    this->wData.val = WINDOW_LloydsBeacon;

    _recallingBeacon = false;
    _casterId = casterId;
    _spellLevel = spellLevel;

    if (!ui_book_lloyds_border) {
        ui_book_lloyds_border = assets->getImage_ColorKey("lb_bordr");
    }

    ui_book_lloyds_backgrounds[0] = assets->getImage_ColorKey("sbmap");
    ui_book_lloyds_backgrounds[1] = assets->getImage_ColorKey("sbmap");
    ui_book_button1_on = assets->getImage_Alpha("tab-an-6b");
    ui_book_button1_off = assets->getImage_Alpha("tab-an-6a");

    pBtn_Book_1 = CreateButton({415, 13}, {39, 36}, 1, 0, UIMSG_LloydBookFlipButton, 0, Io::InputAction::Invalid, localization->GetString(LSTR_SET_BEACON));
    pBtn_Book_2 = CreateButton({415, 48}, {39, 36}, 1, 0, UIMSG_LloydBookFlipButton, 1, Io::InputAction::Invalid, localization->GetString(LSTR_RECALL_BEACON));

    CharacterSkillMastery waterMastery = pParty->pCharacters[_casterId].getActualSkillValue(CHARACTER_SKILL_WATER).mastery();

    _maxBeacons = 1;
    if (waterMastery == CHARACTER_SKILL_MASTERY_GRANDMASTER || waterMastery == CHARACTER_SKILL_MASTERY_MASTER) {
        _maxBeacons = 5;
    } else if (waterMastery == CHARACTER_SKILL_MASTERY_EXPERT) {
        _maxBeacons = 3;
    }

    if (engine->config->debug.AllMagic.value()) {
        _maxBeacons = 5;
    }

    for (int i = 0; i < _maxBeacons; ++i) {
        CreateButton({lloydsBeaconsPreviewXs[i], lloydsBeaconsPreviewYs[i]}, {92, 68}, 1, UIMSG_HintBeaconSlot, UIMSG_InstallOrRecallBeacon, i);
    }

    // purges expired beacons
    pParty->pCharacters[_casterId].cleanupBeacons();
}

void GUIWindow_LloydsBook::Update() {
    render->DrawTextureNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);

    Character *pPlayer = &pParty->pCharacters[_casterId];
    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, ui_book_lloyds_backgrounds[_recallingBeacon ? 1 : 0]);
    std::string pText = localization->GetString(LSTR_RECALL_BEACON);

    GUIWindow pWindow;
    pWindow.uFrameX = game_viewport_x;
    pWindow.uFrameY = game_viewport_y;
    pWindow.uFrameWidth = 428;
    pWindow.uFrameHeight = game_viewport_height;
    pWindow.uFrameZ = 435;
    pWindow.uFrameW = game_viewport_w;
    if (!_recallingBeacon) {
        pText = localization->GetString(LSTR_SET_BEACON);
    }

    pWindow.DrawTitleText(pFontBookTitle, 0, 22, colorTable.White, pText, 3);
    if (_recallingBeacon) {
        render->DrawTextureNew(pBtn_Book_1->uX / 640.0f, pBtn_Book_1->uY / 480.0f, ui_book_button1_on);
        render->DrawTextureNew(pBtn_Book_2->uX / 640.0f, pBtn_Book_2->uY / 480.0f, ui_book_button1_off);
    } else {
        render->DrawTextureNew(pBtn_Book_1->uX / 640.0f, pBtn_Book_1->uY / 480.0f, ui_book_button1_off);
        render->DrawTextureNew(pBtn_Book_2->uX / 640.0f, pBtn_Book_2->uY / 480.0f, ui_book_button1_on);
    }

    for (size_t beaconId = 0; beaconId < _maxBeacons; beaconId++) {
        if ((beaconId >= pPlayer->vBeacons.size()) && _recallingBeacon) {
            break;
        }

        pWindow.uFrameWidth = 92;
        pWindow.uFrameHeight = 68;
        pWindow.uFrameY = lloydsBeaconsPreviewYs[beaconId];
        pWindow.uFrameX = lloydsBeaconsPreviewXs[beaconId];
        pWindow.uFrameW = pWindow.uFrameY + 67;
        pWindow.uFrameZ = lloydsBeaconsPreviewXs[beaconId] + 91;

        render->DrawTextureNew(lloydsBeacons_SomeXs[beaconId] / 640.0f, lloydsBeacons_SomeYs[beaconId] / 480.0f, ui_book_lloyds_border);

        if (beaconId < pPlayer->vBeacons.size()) {
            LloydBeacon &beacon = pPlayer->vBeacons[beaconId];
            render->DrawTextureNew(lloydsBeaconsPreviewXs[beaconId] / 640.0f, lloydsBeaconsPreviewYs[beaconId] / 480.0f, beacon.image);
            std::string Str = pMapStats->pInfos[beacon.mapId].pName;
            unsigned int pTextHeight = pFontBookLloyds->CalcTextHeight(Str, pWindow.uFrameWidth, 0);
            pWindow.uFrameY -= 6 + pTextHeight;
            pWindow.DrawTitleText(pFontBookLloyds, 0, 0, colorTable.Black, Str, 3);

            pWindow.uFrameY = lloydsBeaconsPreviewYs[beaconId];
            GameTime remainingTime = beacon.uBeaconTime - pParty->GetPlayingTime();
            unsigned int pHours = remainingTime.GetHoursOfDay();
            unsigned int pDays = remainingTime.GetDays();
            std::string str;
            if (pDays > 1) {
                str = fmt::format("{} {}", pDays + 1, localization->GetString(LSTR_DAYS));
            } else if (pHours + 1 <= 23) {
                str = fmt::format("{} {}", pHours + 1, localization->GetString((pHours < 1) ? LSTR_HOUR : LSTR_HOURS));
            } else {
                str = fmt::format("{} {}", pDays + 1, localization->GetString(LSTR_DAY_CAPITALIZED));
            }
            pWindow.uFrameY = pWindow.uFrameY + pWindow.uFrameHeight + 4;
            pWindow.DrawTitleText(pFontBookLloyds, 0, 0, colorTable.Black, str, 3);
        } else {
            unsigned int pTextHeight = pFontBookLloyds->CalcTextHeight(localization->GetString(LSTR_AVAILABLE), pWindow.uFrameWidth, 0);
            pWindow.DrawTitleText(pFontBookLloyds, 0, (int)pWindow.uFrameHeight / 2 - pTextHeight / 2, colorTable.Black, localization->GetString(LSTR_AVAILABLE), 3);
        }
    }
}

void GUIWindow_LloydsBook::flipButtonClicked(bool isRecalling) {
    _recallingBeacon = isRecalling;
    pAudioPlayer->playUISound(_recallingBeacon ? SOUND_TurnPage2 : SOUND_TurnPage1);
}

void GUIWindow_LloydsBook::hintBeaconSlot(int beaconId) {
    Character &character = pParty->pCharacters[_casterId];

    if (beaconId >= character.vBeacons.size()) {
        return;
    }

    LloydBeacon &beacon = character.vBeacons[beaconId];
    if (_recallingBeacon) {
        if (beacon.uBeaconTime) {
            std::string mapName = pMapStats->pInfos[beacon.mapId].pName;
            GameUI_StatusBar_Set(localization->FormatString(LSTR_FMT_RECALL_TO_S, mapName));
        }
    } else {
        MAP_TYPE mapId = pMapStats->GetMapInfo(pCurrentMapName);
        std::string mapName = "Not in Map Stats";
        if (mapId != MAP_INVALID) {
            mapName = pMapStats->pInfos[mapId].pName;
        }

        if (beacon.uBeaconTime) {
            std::string mapName2 = pMapStats->pInfos[beacon.mapId].pName;
            GameUI_StatusBar_Set(localization->FormatString(LSTR_FMT_SET_S_OVER_S, mapName, mapName2));
        } else {
            GameUI_StatusBar_Set(localization->FormatString(LSTR_FMT_SET_S_TO_S, mapName));
        }
    }
}

void GUIWindow_LloydsBook::installOrRecallBeacon(int beaconId) {
    Character &character = pParty->pCharacters[_casterId];
    if ((character.vBeacons.size() <= beaconId) && _recallingBeacon) {
        return;
    }

    assert(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelMana == pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uExpertLevelMana);
    assert(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelMana == pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uMasterLevelMana);
    assert(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelMana == pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uMagisterLevelMana);
    character.SpendMana(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelMana);

    assert(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelRecovery == pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uExpertLevelRecovery);
    assert(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelRecovery == pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uMasterLevelRecovery);
    assert(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelRecovery == pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uMagisterLevelRecovery);

    signed int sRecoveryTime = pSpellDatas[SPELL_WATER_LLOYDS_BEACON].uNormalLevelRecovery;
    if (pParty->bTurnBasedModeOn) {
        pParty->pTurnBasedCharacterRecoveryTimes[_casterId] = sRecoveryTime;
        character.SetRecoveryTime(sRecoveryTime);
        pTurnEngine->ApplyPlayerAction();
    } else {
        character.SetRecoveryTime(debug_non_combat_recovery_mul * sRecoveryTime * flt_debugrecmod3);
    }
    pAudioPlayer->playSpellSound(SPELL_WATER_LLOYDS_BEACON, false, SOUND_MODE_UI);
    if (_recallingBeacon) {
        if (toLower(pCurrentMapName) != toLower(pMapStats->pInfos[character.vBeacons[beaconId].mapId].pFilename)) {
            // TODO(Nik-RE-dev): need separate function for teleportation to other maps
            SaveGame(1, 0);
            onMapLeave();
            pCurrentMapName = pMapStats->pInfos[character.vBeacons[beaconId].mapId].pFilename;
            dword_6BE364_game_settings_1 |= GAME_SETTINGS_0001;
            uGameState = GAME_STATE_CHANGE_LOCATION;
            Party_Teleport_X_Pos = character.vBeacons[beaconId].PartyPos_X;
            Party_Teleport_Y_Pos = character.vBeacons[beaconId].PartyPos_Y;
            Party_Teleport_Z_Pos = character.vBeacons[beaconId].PartyPos_Z;
            Party_Teleport_Cam_Yaw = character.vBeacons[beaconId]._partyViewYaw;
            Party_Teleport_Cam_Pitch = character.vBeacons[beaconId]._partyViewPitch;
            Start_Party_Teleport_Flag = 1;
        } else {
            pParty->pos.x = character.vBeacons[beaconId].PartyPos_X;
            pParty->pos.y = character.vBeacons[beaconId].PartyPos_Y;
            pParty->pos.z = character.vBeacons[beaconId].PartyPos_Z;
            pParty->uFallStartZ = pParty->pos.z;
            pParty->_viewYaw = character.vBeacons[beaconId]._partyViewYaw;
            pParty->_viewPitch = character.vBeacons[beaconId]._partyViewPitch;
        }
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        pGUIWindow_CurrentMenu->Release();
        pGUIWindow_CurrentMenu = 0;
    } else {
        character.setBeacon(beaconId, GameTime::FromDays(7 * _spellLevel));
        engine->_messageQueue->addMessageNextFrame(UIMSG_CloseAfterInstallBeacon, 0, 0);
    }
}
