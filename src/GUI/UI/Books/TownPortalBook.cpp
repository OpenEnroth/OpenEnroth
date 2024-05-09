#include "TownPortalBook.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Pid.h"
#include "Engine/SaveLoad.h"
#include "Engine/Objects/Actor.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/Events/Processor.h"
#include "Engine/Spells/Spells.h"
#include "Engine/MapInfo.h"

#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UIGame.h"

#include "Media/Audio/AudioPlayer.h"

#include "Io/Mouse.h"

#include "Library/Geometry/Rect.h"

struct TownPortalData {
    Vec3f pos;
    int viewYaw;
    int viewPitch;
    MapId mapInfoID;
    QuestBit qBit;
};

static const int TOWN_PORTAL_DESTINATION_COUNT = 6;

std::array<TownPortalData, TOWN_PORTAL_DESTINATION_COUNT> townPortalList = {{
    {Vec3f( -5121,   2107,    1), 1536, 0, MAP_CASTLE_HARMONDALE,   QBIT_FOUNTAIN_IN_HARMONDALE_ACTIVATED},
    {Vec3f(-15148, -10240, 1473),    0, 0, MAP_TULAREAN_FOREST,     QBIT_FOUNTAIN_IN_PIERPONT_ACTIVATED},
    {Vec3f(-10519,   5375,  753),  512, 0, MAP_ERATHIA,             QBIT_FOUNTAIN_IN_STEADWICK_ACTIVATED},
    {Vec3f(  3114, -11055,  513),    0, 0, MAP_MOUNT_NIGHON,        QBIT_FOUNTAIN_IN_MOUNT_NIGHON_ACTIVATED},
    {Vec3f(  -158,   7624,    1),  512, 0, MAP_CELESTE,             QBIT_FOUNTAIN_IN_CELESTIA_ACTIVATED},
    {Vec3f( -1837,  -4247,   65),   65, 0, MAP_PIT,                 QBIT_FOUNTAIN_IN_THE_PIT_ACTIVATED}
}};

static std::array<Recti, TOWN_PORTAL_DESTINATION_COUNT> townPortalButtonsPos = {{
    {260, 206, 80, 55},
    {324,  84, 66, 56},
    {147, 182, 68, 65},
    {385, 239, 72, 67},
    {390,  17, 67, 67},
    { 19, 283, 74, 59}
}};

static std::array<GraphicsImage *, TOWN_PORTAL_DESTINATION_COUNT> ui_book_townportal_icons;

GraphicsImage *ui_book_townportal_background = nullptr;

GUIWindow_TownPortalBook::GUIWindow_TownPortalBook(Pid casterPid) {
    this->eWindowType = WindowType::WINDOW_TownPortal;

    _casterPid = casterPid;

    ui_book_townportal_background = assets->getImage_Solid("townport");

    ui_book_townportal_icons[0] = assets->getImage_ColorKey("tpharmndy");
    ui_book_townportal_icons[1] = assets->getImage_ColorKey("tpelf");
    ui_book_townportal_icons[2] = assets->getImage_ColorKey("tpwarlock");
    ui_book_townportal_icons[3] = assets->getImage_ColorKey("tpisland");
    ui_book_townportal_icons[4] = assets->getImage_ColorKey("tpheaven");
    ui_book_townportal_icons[5] = assets->getImage_ColorKey("tphell");

    for (int i = 0; i < TOWN_PORTAL_DESTINATION_COUNT; ++i) {
        CreateButton(townPortalButtonsPos[i].topLeft(), townPortalButtonsPos[i].size(), 1, UIMSG_HintTownPortal, UIMSG_ClickTownInTP, i);
    }
}

void GUIWindow_TownPortalBook::Update() {
    render->DrawTextureNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);

    GUIWindow townPortalWindow;

    render->ClearZBuffer();
    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, ui_book_townportal_background);
    render->DrawTextureNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);

    townPortalWindow.uFrameX = game_viewport_x;
    townPortalWindow.uFrameY = game_viewport_y;
    townPortalWindow.uFrameWidth = game_viewport_width;
    townPortalWindow.uFrameHeight = game_viewport_height;
    townPortalWindow.uFrameZ = game_viewport_z;
    townPortalWindow.uFrameW = game_viewport_w;

    for (int i = 0; i < TOWN_PORTAL_DESTINATION_COUNT; ++i) {
        if (pParty->_questBits[townPortalList[i].qBit] || engine->config->debug.TownPortal.value()) {
            render->ZDrawTextureAlpha(townPortalButtonsPos[i].x / 640.0f, townPortalButtonsPos[i].y / 480.0f, ui_book_townportal_icons[i], i + 1);
        }
    }

    Pointi pt = mouse->GetCursorPos();
    int iconPointing = render->pActiveZBuffer[pt.x + pt.y * render->GetRenderDimensions().w] & 0xFFFF;

    if (iconPointing) {
        if (pParty->_questBits[townPortalList[iconPointing - 1].qBit] || engine->config->debug.TownPortal.value()) {
            render->DrawTextureNew(townPortalButtonsPos[iconPointing - 1].x / 640.0f, townPortalButtonsPos[iconPointing - 1].y / 480.0f,
                                   ui_book_townportal_icons[iconPointing - 1]);
        }
    }

    townPortalWindow.DrawTitleText(assets->pFontBookTitle.get(), 0, 22, colorTable.White, localization->GetString(LSTR_TOWN_PORTAL), 3);
}

void GUIWindow_TownPortalBook::clickTown(int townId) {
    // check if tp location is unlocked
    if (!pParty->_questBits[townPortalList[townId].qBit] && !engine->config->debug.TownPortal.value()) {
        return;
    }

    // begin TP
    AutoSave();
    // if in current map
    // TODO(Nik-RE-dev): need separate function for teleportation to other maps
    if (pMapStats->GetMapInfo(pCurrentMapName) == townPortalList[townId].mapInfoID) {
        pParty->pos = townPortalList[townId].pos;
        pParty->uFallStartZ = pParty->pos.z;
        pParty->_viewYaw = townPortalList[townId].viewYaw;
        pParty->_viewPitch = townPortalList[townId].viewPitch;
    } else {  // if change map
        onMapLeave();
        dword_6BE364_game_settings_1 |= GAME_SETTINGS_SKIP_WORLD_UPDATE;
        uGameState = GAME_STATE_CHANGE_LOCATION;
        pCurrentMapName = pMapStats->pInfos[townPortalList[townId].mapInfoID].fileName;
        engine->_teleportPoint.setTeleportTarget(townPortalList[townId].pos, townPortalList[townId].viewYaw, townPortalList[townId].viewPitch, 0);
        Actor::InitializeActors();
    }

    assert(_casterPid.type() == OBJECT_Character);

    int casterId = _casterPid.id();
    if (casterId < pParty->pCharacters.size()) {
        // Town portal casted by character
        assert(pSpellDatas[SPELL_WATER_TOWN_PORTAL].mana_per_skill[CHARACTER_SKILL_MASTERY_NOVICE] == pSpellDatas[SPELL_WATER_TOWN_PORTAL].mana_per_skill[CHARACTER_SKILL_MASTERY_EXPERT]);
        assert(pSpellDatas[SPELL_WATER_TOWN_PORTAL].mana_per_skill[CHARACTER_SKILL_MASTERY_NOVICE] == pSpellDatas[SPELL_WATER_TOWN_PORTAL].mana_per_skill[CHARACTER_SKILL_MASTERY_MASTER]);
        assert(pSpellDatas[SPELL_WATER_TOWN_PORTAL].mana_per_skill[CHARACTER_SKILL_MASTERY_NOVICE] == pSpellDatas[SPELL_WATER_TOWN_PORTAL].mana_per_skill[CHARACTER_SKILL_MASTERY_GRANDMASTER]);
        pParty->pCharacters[casterId].SpendMana(pSpellDatas[SPELL_WATER_TOWN_PORTAL].mana_per_skill[CHARACTER_SKILL_MASTERY_NOVICE]);

        assert(pSpellDatas[SPELL_WATER_TOWN_PORTAL].recovery_per_skill[CHARACTER_SKILL_MASTERY_NOVICE] == pSpellDatas[SPELL_WATER_TOWN_PORTAL].recovery_per_skill[CHARACTER_SKILL_MASTERY_EXPERT]);
        assert(pSpellDatas[SPELL_WATER_TOWN_PORTAL].recovery_per_skill[CHARACTER_SKILL_MASTERY_NOVICE] == pSpellDatas[SPELL_WATER_TOWN_PORTAL].recovery_per_skill[CHARACTER_SKILL_MASTERY_MASTER]);
        assert(pSpellDatas[SPELL_WATER_TOWN_PORTAL].recovery_per_skill[CHARACTER_SKILL_MASTERY_NOVICE] == pSpellDatas[SPELL_WATER_TOWN_PORTAL].recovery_per_skill[CHARACTER_SKILL_MASTERY_GRANDMASTER]);
        Duration sRecoveryTime = pSpellDatas[SPELL_WATER_TOWN_PORTAL].recovery_per_skill[CHARACTER_SKILL_MASTERY_NOVICE];
        if (pParty->bTurnBasedModeOn) {
            pParty->pTurnBasedCharacterRecoveryTimes[casterId] = sRecoveryTime;
            pParty->pCharacters[casterId].SetRecoveryTime(sRecoveryTime);
            pTurnEngine->ApplyPlayerAction();
        } else {
            pParty->pCharacters[casterId].SetRecoveryTime(debug_non_combat_recovery_mul * flt_debugrecmod3 * sRecoveryTime);
        }
    } else {
        // Town portal casted by hireling
        pParty->pHirelings[casterId - pParty->pCharacters.size()].bHasUsedTheAbility = 1;
    }

    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
}

void GUIWindow_TownPortalBook::hintTown(int townId) {
    if (!pParty->_questBits[townPortalList[townId].qBit] && !engine->config->debug.TownPortal.value()) {
        render->DrawTextureNew(0, 352 / 480.0f, game_ui_statusbar);
        return;
    }

    engine->_statusBar->setPermanent(LSTR_TOWN_PORTAL_TO_S, pMapStats->pInfos[townPortalList[townId].mapInfoID].name);
}
