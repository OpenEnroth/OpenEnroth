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

#include "Engine/Graphics/Viewport.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/String/Ascii.h"

std::array<int, 5> lloydsBeaconsPreviewXs = {{61, 281, 61, 281, 171}};
std::array<int, 5> lloydsBeaconsPreviewYs = {{84, 84, 228, 228, 155}};
std::array<int, 5> lloydsBeacons_SomeXs = {{59, 279, 59, 279, 169}};
std::array<int, 5> lloydsBeacons_SomeYs = {{82, 82, 226, 226, 153}};

GraphicsImage *ui_book_lloyds_border = nullptr;
std::array<GraphicsImage *, 2> ui_book_lloyds_backgrounds;

IndexedArray<int, MASTERY_NONE, MASTERY_LAST> masteryToMaxBeacons = {
        {MASTERY_NONE, 1},
        {MASTERY_NOVICE, 1},
        {MASTERY_EXPERT, 3},
        {MASTERY_MASTER, 5},
        {MASTERY_GRANDMASTER, 5}
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

    pBtn_Book_1 = CreateButton({415, 13}, {39, 36}, 1, 0, UIMSG_LloydBookFlipButton, 0, INPUT_ACTION_INVALID, localization->str(LSTR_SET_BEACON));
    pBtn_Book_2 = CreateButton({415, 48}, {39, 36}, 1, 0, UIMSG_LloydBookFlipButton, 1, INPUT_ACTION_INVALID, localization->str(LSTR_RECALL_BEACON));

    int casterId = casterPid.id();
    assert(casterId < pParty->pCharacters.size());
    if (engine->config->debug.AllMagic.value()) {
        _maxBeacons = 5;
        _waterMastery = MASTERY_GRANDMASTER;
        _spellLevel = 10;
    } else {
        CombinedSkillValue skill = pParty->pCharacters[casterId].getActualSkillValue(SKILL_WATER);
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
    render->DrawQuad2D(ui_exit_cancel_button_background, {471, 445});

    Character *pPlayer = &pParty->pCharacters[_casterPid.id()];
    render->DrawQuad2D(ui_book_lloyds_backgrounds[_recallingBeacon ? 1 : 0], {8, 8});
    std::string pText = localization->str(LSTR_RECALL_BEACON);

    if (!_recallingBeacon) {
        pText = localization->str(LSTR_SET_BEACON);
    }

    Recti pWindow(pViewport.x, pViewport.y, 428, pViewport.h);
    DrawTitleText(assets->pFontBookTitle.get(), 0, 22, colorTable.White, pText, 3, pWindow);
    if (_recallingBeacon) {
        render->DrawQuad2D(ui_book_button1_on, pBtn_Book_1->rect.topLeft());
        render->DrawQuad2D(ui_book_button1_off, pBtn_Book_2->rect.topLeft());
    } else {
        render->DrawQuad2D(ui_book_button1_off, pBtn_Book_1->rect.topLeft());
        render->DrawQuad2D(ui_book_button1_on, pBtn_Book_2->rect.topLeft());
    }

    for (size_t beaconId = 0; beaconId < _maxBeacons; beaconId++) {
        if (_recallingBeacon && !pPlayer->vBeacons[beaconId]) {
            continue;
        }

        pWindow = Recti(lloydsBeaconsPreviewXs[beaconId], lloydsBeaconsPreviewYs[beaconId], 92, 68);

        render->DrawQuad2D(ui_book_lloyds_border, {lloydsBeacons_SomeXs[beaconId], lloydsBeacons_SomeYs[beaconId]});

        if (pPlayer->vBeacons[beaconId]) {
            LloydBeacon &beacon = pPlayer->vBeacons[beaconId].value();
            render->DrawQuad2D(beacon.image, {lloydsBeaconsPreviewXs[beaconId], lloydsBeaconsPreviewYs[beaconId]});
            std::string Str = pMapStats->pInfos[beacon.mapId].name;
            int pTextHeight = assets->pFontBookLloyds->CalcTextHeight(Str, pWindow.w, 0);
            pWindow.y -= 6 + pTextHeight;
            DrawTitleText(assets->pFontBookLloyds.get(), 0, 0, colorTable.Black, Str, 3, pWindow);

            pWindow.y = lloydsBeaconsPreviewYs[beaconId];
            Duration remainingTime = beacon.uBeaconTime - pParty->GetPlayingTime();
            CivilDuration d = remainingTime.toCivilDuration();
            std::string str;
            if (d.days > 0) {
                str = fmt::format("{} {}", d.days, localization->str(d.days == 1 ? LSTR_DAY_CAPITALIZED : LSTR_DAYS));
            } else if (d.hours > 0) {
                str = fmt::format("{} {}", d.hours, localization->str(d.hours == 1 ? LSTR_HOUR : LSTR_HOURS));
            } else {
                str = fmt::format("{} {}", d.minutes, localization->str(d.minutes == 1 ? LSTR_MINUTE : LSTR_MINUTES));
            }
            pWindow.y = pWindow.y + pWindow.h + 4;
            DrawTitleText(assets->pFontBookLloyds.get(), 0, 0, colorTable.Black, str, 3, pWindow);
        } else {
            int pTextHeight = assets->pFontBookLloyds->CalcTextHeight(localization->str(LSTR_AVAILABLE), pWindow.w, 0);
            DrawTitleText(assets->pFontBookLloyds.get(), 0, (int)pWindow.h / 2 - pTextHeight / 2, colorTable.Black, localization->str(LSTR_AVAILABLE), 3, pWindow);
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
            autoSave();
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
        pGUIWindow_CurrentMenu = nullptr;
    } else {
        character.setBeacon(beaconId, Duration::fromDays(7 * _spellLevel));
        engine->_messageQueue->addMessageNextFrame(UIMSG_CloseAfterInstallBeacon, 0, 0);
    }
}
