#include <string>

#include "Engine/Engine.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/SaveLoad.h"
#include "Engine/AssetsManager.h"
#include "Engine/Evt/Processor.h"
#include "Engine/Spells/Spells.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/MapInfo.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/Books/LloydsBook.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/String/Ascii.h"

std::array<int, 5> lloydsBeaconsPreviewXs = {{61, 281, 61, 281, 171}};
std::array<int, 5> lloydsBeaconsPreviewYs = {{84, 84, 228, 228, 155}};
std::array<int, 5> lloydsBeacons_SomeXs = {{59, 279, 59, 279, 169}};
std::array<int, 5> lloydsBeacons_SomeYs = {{82, 82, 226, 226, 153}};

GraphicsImage *ui_book_lloyds_border = nullptr;
std::array<GraphicsImage *, 2> ui_book_lloyds_backgrounds;

IndexedArray<int, CHARACTER_SKILL_MASTERY_NONE, CHARACTER_SKILL_MASTERY_LAST> masteryToMaxBeacons = {
        {CHARACTER_SKILL_MASTERY_NONE, 1},
        {CHARACTER_SKILL_MASTERY_NOVICE, 1},
        {CHARACTER_SKILL_MASTERY_EXPERT, 3},
        {CHARACTER_SKILL_MASTERY_MASTER, 5},
        {CHARACTER_SKILL_MASTERY_GRANDMASTER, 5}
};

GUIWindow_LloydsBook::GUIWindow_LloydsBook(Pid casterPid, SpellCastFlags castFlags)
        : _casterPid(casterPid), _castFlags(castFlags) {
    this->eWindowType = WindowType::WINDOW_LloydsBeacon;

    _recallingBeacon = false;

    if (!ui_book_lloyds_border) {
        ui_book_lloyds_border = assets->getImage_ColorKey("lb_bordr");
    }

    ui_book_lloyds_backgrounds[0] = assets->getImage_ColorKey("sbmap");
    ui_book_lloyds_backgrounds[1] = assets->getImage_ColorKey("sbmap");
    ui_book_button1_on = assets->getImage_Alpha("tab-an-6b");
    ui_book_button1_off = assets->getImage_Alpha("tab-an-6a");

    pBtn_Book_1 = CreateButton({415, 13}, {39, 36}, 1, 0, UIMSG_LloydBookFlipButton, 0, Io::InputAction::Invalid, localization->GetString(LSTR_SET_BEACON));
    pBtn_Book_2 = CreateButton({415, 48}, {39, 36}, 1, 0, UIMSG_LloydBookFlipButton, 1, Io::InputAction::Invalid, localization->GetString(LSTR_RECALL_BEACON));

    int casterId = casterPid.id();
    assert(casterId < pParty->pCharacters.size());
    if (engine->config->debug.AllMagic.value()) {
        _maxBeacons = 5;
        _waterMastery = CHARACTER_SKILL_MASTERY_GRANDMASTER;
        _spellLevel = 10;
    } else {
        CombinedSkillValue skill = pParty->pCharacters[casterId].getActualSkillValue(CHARACTER_SKILL_WATER);
        _maxBeacons = masteryToMaxBeacons[skill.mastery()];
        if (castFlags & ON_CAST_CastViaScroll) skill = SCROLL_OR_NPC_SPELL_SKILL_VALUE;
        _waterMastery = skill.mastery();
        _spellLevel = skill.level();
    }

    for (int i = 0; i < _maxBeacons; ++i) {
        CreateButton({lloydsBeaconsPreviewXs[i], lloydsBeaconsPreviewYs[i]}, {92, 68}, 1, UIMSG_HintBeaconSlot, UIMSG_InstallOrRecallBeacon, i);
    }

    // purges expired beacons
    pParty->pCharacters[casterId].cleanupBeacons();
}

void GUIWindow_LloydsBook::Update() {
    render->DrawTextureNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);

    Character *pPlayer = &pParty->pCharacters[_casterPid.id()];
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

    pWindow.DrawTitleText(assets->pFontBookTitle.get(), 0, 22, colorTable.White, pText, 3);
    if (_recallingBeacon) {
        render->DrawTextureNew(pBtn_Book_1->uX / 640.0f, pBtn_Book_1->uY / 480.0f, ui_book_button1_on);
        render->DrawTextureNew(pBtn_Book_2->uX / 640.0f, pBtn_Book_2->uY / 480.0f, ui_book_button1_off);
    } else {
        render->DrawTextureNew(pBtn_Book_1->uX / 640.0f, pBtn_Book_1->uY / 480.0f, ui_book_button1_off);
        render->DrawTextureNew(pBtn_Book_2->uX / 640.0f, pBtn_Book_2->uY / 480.0f, ui_book_button1_on);
    }

    for (size_t beaconId = 0; beaconId < _maxBeacons; beaconId++) {
        if (_recallingBeacon && !pPlayer->vBeacons[beaconId]) {
            continue;
        }

        pWindow.uFrameWidth = 92;
        pWindow.uFrameHeight = 68;
        pWindow.uFrameY = lloydsBeaconsPreviewYs[beaconId];
        pWindow.uFrameX = lloydsBeaconsPreviewXs[beaconId];
        pWindow.uFrameW = pWindow.uFrameY + 67;
        pWindow.uFrameZ = lloydsBeaconsPreviewXs[beaconId] + 91;

        render->DrawTextureNew(lloydsBeacons_SomeXs[beaconId] / 640.0f, lloydsBeacons_SomeYs[beaconId] / 480.0f, ui_book_lloyds_border);

        if (pPlayer->vBeacons[beaconId]) {
            LloydBeacon &beacon = pPlayer->vBeacons[beaconId].value();
            render->DrawTextureNew(lloydsBeaconsPreviewXs[beaconId] / 640.0f, lloydsBeaconsPreviewYs[beaconId] / 480.0f, beacon.image);
            std::string Str = pMapStats->pInfos[beacon.mapId].name;
            int pTextHeight = assets->pFontBookLloyds->CalcTextHeight(Str, pWindow.uFrameWidth, 0);
            pWindow.uFrameY -= 6 + pTextHeight;
            pWindow.DrawTitleText(assets->pFontBookLloyds.get(), 0, 0, colorTable.Black, Str, 3);

            pWindow.uFrameY = lloydsBeaconsPreviewYs[beaconId];
            Duration remainingTime = beacon.uBeaconTime - pParty->GetPlayingTime();
            CivilDuration d = remainingTime.toCivilDuration();
            std::string str;
            if (d.days > 0) {
                str = fmt::format("{} {}", d.days, localization->GetString(d.days == 1 ? LSTR_DAY_CAPITALIZED : LSTR_DAYS));
            } else if (d.hours > 0) {
                str = fmt::format("{} {}", d.hours, localization->GetString(d.hours == 1 ? LSTR_HOUR : LSTR_HOURS));
            } else {
                str = fmt::format("{} {}", d.minutes, localization->GetString(d.minutes == 1 ? LSTR_MINUTE : LSTR_MINUTES));
            }
            pWindow.uFrameY = pWindow.uFrameY + pWindow.uFrameHeight + 4;
            pWindow.DrawTitleText(assets->pFontBookLloyds.get(), 0, 0, colorTable.Black, str, 3);
        } else {
            int pTextHeight = assets->pFontBookLloyds->CalcTextHeight(localization->GetString(LSTR_AVAILABLE), pWindow.uFrameWidth, 0);
            pWindow.DrawTitleText(assets->pFontBookLloyds.get(), 0, (int)pWindow.uFrameHeight / 2 - pTextHeight / 2, colorTable.Black, localization->GetString(LSTR_AVAILABLE), 3);
        }
    }
}

void GUIWindow_LloydsBook::flipButtonClicked(bool isRecalling) {
    _recallingBeacon = isRecalling;
    pAudioPlayer->playUISound(_recallingBeacon ? SOUND_TurnPage2 : SOUND_TurnPage1);
}

void GUIWindow_LloydsBook::hintBeaconSlot(int beaconId) {
    Character &character = pParty->pCharacters[_casterPid.id()];

    if (!character.vBeacons[beaconId]) {
        return;
    }

    LloydBeacon &beacon = *character.vBeacons[beaconId];
    if (_recallingBeacon) {
        if (beacon.uBeaconTime) {
            std::string mapName = pMapStats->pInfos[beacon.mapId].name;
            engine->_statusBar->setPermanent(LSTR_RECALL_TO_S, mapName);
        }
    } else {
        std::string mapName = "Not in Map Stats";
        if (engine->_currentLoadedMapId != MAP_INVALID) {
            mapName = pMapStats->pInfos[engine->_currentLoadedMapId].name;
        }

        if (beacon.uBeaconTime) {
            std::string mapName2 = pMapStats->pInfos[beacon.mapId].name;
            engine->_statusBar->setPermanent(LSTR_SET_S_OVER_S, mapName, mapName2);
        } else {
            engine->_statusBar->setPermanent(LSTR_SET_TO_S, mapName);
        }
    }
}

void GUIWindow_LloydsBook::installOrRecallBeacon(int beaconId) {
    Character &character = pParty->pCharacters[_casterPid.id()];
    if (_recallingBeacon && !character.vBeacons[beaconId]) {
        return;
    }

    if (!(_castFlags & ON_CAST_CastViaScroll) && !engine->config->debug.AllMagic.value())
        character.SpendMana(pSpellDatas[SPELL_WATER_LLOYDS_BEACON].mana_per_skill[_waterMastery]);

    Duration sRecoveryTime = pSpellDatas[SPELL_WATER_LLOYDS_BEACON].recovery_per_skill[_waterMastery];
    if (pParty->bTurnBasedModeOn) {
        pParty->pTurnBasedCharacterRecoveryTimes[_casterPid.id()] = sRecoveryTime;
        character.SetRecoveryTime(sRecoveryTime);
        pTurnEngine->ApplyPlayerAction();
    } else {
        character.SetRecoveryTime(debug_non_combat_recovery_mul * flt_debugrecmod3 * sRecoveryTime);
    }
    pAudioPlayer->playSpellSound(SPELL_WATER_LLOYDS_BEACON, false, SOUND_MODE_UI);
    if (_recallingBeacon) {
        LloydBeacon &beacon = *character.vBeacons[beaconId];
        if (engine->_currentLoadedMapId != beacon.mapId) {
            // TODO(Nik-RE-dev): need separate function for teleportation to other maps
            AutoSave();
            onMapLeave();
            engine->_transitionMapId = beacon.mapId;
            dword_6BE364_game_settings_1 |= GAME_SETTINGS_SKIP_WORLD_UPDATE;
            uGameState = GAME_STATE_CHANGE_LOCATION;
            engine->_teleportPoint.setTeleportTarget(beacon._partyPos, beacon._partyViewYaw, beacon._partyViewPitch, 0);
        } else {
            pParty->pos = beacon._partyPos;
            pParty->uFallStartZ = pParty->pos.z;
            pParty->_viewYaw = beacon._partyViewYaw;
            pParty->_viewPitch = beacon._partyViewPitch;
        }
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        pGUIWindow_CurrentMenu->Release();
        pGUIWindow_CurrentMenu = 0;
    } else {
        character.setBeacon(beaconId, Duration::fromDays(7 * _spellLevel));
        engine->_messageQueue->addMessageNextFrame(UIMSG_CloseAfterInstallBeacon, 0, 0);
    }
}
