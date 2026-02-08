#include <cstring>
#include <string>
#include <algorithm>
#include <memory>

#include "Engine/Engine.h"

#include "Engine/EngineGlobals.h"
#include "Engine/AssetsManager.h"

#include "Engine/Evt/Processor.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Objects/DecorationList.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/LightsStack.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/BspRenderer.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Tables/TextureFrameTable.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/Graphics/TurnBasedOverlay.h"
#include "Engine/Resources/LodTextureCache.h"
#include "Engine/Resources/LodSpriteCache.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Objects/MonsterEnumFunctions.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/Random/Random.h"
#include "Engine/SaveLoad.h"
#include "Engine/Snapshots/TableSerialization.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Tables/AwardTable.h"
#include "Engine/Tables/HouseTable.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Tables/PortraitFrameTable.h"
#include "Engine/Tables/TileTable.h"
#include "Engine/Tables/FactionTable.h"
#include "Engine/Tables/HistoryTable.h"
#include "Engine/Tables/AutonoteTable.h"
#include "Engine/Tables/QuestTable.h"
#include "Engine/Tables/TransitionTable.h"
#include "Engine/Tables/MerchantTable.h"
#include "Engine/Tables/MessageScrollTable.h"
#include "Engine/Time/Timer.h"
#include "Engine/AttackList.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/MapInfo.h"
#include "Engine/Resources/EngineFileSystem.h"
#include "Engine/Resources/LOD.h"
#include "Graphics/TileGenerator.h"

#include "GUI/GUIProgressBar.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UIPopup.h"
#include "GUI/UI/UIMessageScroll.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/Overlay/OverlaySystem.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/Audio/SoundList.h"
#include "Media/MediaPlayer.h"

#include "Io/Mouse.h"

#include "Library/Logger/Logger.h"
#include "Library/BuildInfo/BuildInfo.h"
#include "Tables/ChestTable.h"

#include "Utility/String/Transformations.h"
#include "TurnEngine/TurnEngine.h"

/*

static bool b = false;
static UIAnimation torchA;
static UIAnimation torchB;
static UIAnimation torchC;
if (!b)
{
torchA.icon = pIconsFrameTable->GetIcon("torchA");
torchA.uAnimTime = 0;
torchA.uAnimLength = torchA.icon->GetAnimLength();

torchB.icon = pIconsFrameTable->GetIcon("torchB");
torchB.uAnimTime = 0;
torchB.uAnimLength = torchB.icon->GetAnimLength();

torchC.icon = pIconsFrameTable->GetIcon("torchC");
torchC.uAnimTime = 0;
torchC.uAnimLength = torchC.icon->GetAnimLength();

b = true;
}

auto icon = pIconsFrameTable->GetFrame(torchA.icon->id, GetTickCount()/2);
render->DrawTextureNew(64 / 640.0f, 48 / 480.0f, icon->texture);

icon = pIconsFrameTable->GetFrame(torchB.icon->id, GetTickCount() / 2);
render->DrawTextureNew((64 + torchA.icon->texture->GetWidth())/ 640.0f, 48
/ 480.0f, icon->texture);

icon = pIconsFrameTable->GetFrame(torchC.icon->id, GetTickCount() / 2);
render->DrawTextureNew((64 + torchA.icon->texture->GetWidth() +
torchB.icon->texture->GetWidth()) / 640.0f, 48 / 480.0f, icon->texture);

*/

Engine *engine;
GameState uGameState;

void Engine::drawWorld() {
    engine->SetSaturateFaces(pParty->checkPartyPerceptionAgainstCurrentMap());

    pCamera3D->_viewPitch = pParty->_viewPitch;
    pCamera3D->_viewYaw = pParty->_viewYaw;
    pCamera3D->vCameraPos.x = pParty->pos.x - pParty->_yawGranularity * cosf(2 * pi_double * pParty->_viewYaw / 2048.0);
    pCamera3D->vCameraPos.y = pParty->pos.y - pParty->_yawGranularity * sinf(2 * pi_double * pParty->_viewYaw / 2048.0);
    pCamera3D->vCameraPos.z = pParty->pos.z + pParty->eyeLevel;  // 193, but real 353

    pCamera3D->CalculateRotations(pParty->_viewYaw, pParty->_viewPitch);
    pCamera3D->CreateViewMatrixAndProjectionScale();
    pCamera3D->BuildViewFrustum();

    if (pMovie_Track) {
        /*if ( !render->pRenderD3D )
        {
        render->BeginScene3D();
        pMouse->DrawCursorToTarget();
        render->DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();
        }*/
    } else {
        if (pParty->pos != pParty->lastPos ||
            pParty->_viewYaw != pParty->_viewPrevYaw ||
            pParty->_viewPitch != pParty->_viewPrevPitch ||
            pParty->eyeLevel != pParty->lastEyeLevel)
            pParty->lastPos = pParty->pos;
        // v0 = &render;
        pParty->_viewPrevYaw = pParty->_viewYaw;
        pParty->_viewPrevPitch = pParty->_viewPitch;

        pParty->lastEyeLevel = pParty->eyeLevel;
        render->BeginScene3D();

        // if ( !render->pRenderD3D )
        // pMouse->DrawCursorToTarget();
        if (!PauseGameDrawing()) {
            if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                pIndoor->Draw();
            } else {
                assert(uCurrentlyLoadedLevelType == LEVEL_OUTDOOR);
                render->uFogColor = GetLevelFogColor();
                pOutdoor->Draw();
            }

            decal_builder->DrawBloodsplats();
        }
        render->DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();
    }
}

void Engine::drawOverlay() {
    _overlaySystem.drawOverlays();
}

void Engine::drawHUD() {
    // 2d from now on
    render->BeginScene2D();

    DrawGUI();
    GUI_UpdateWindows();
    pParty->updateCharactersAndHirelingsEmotions();

    // mouse->DrawPickedItem();
    mouse->DrawCursor();
}

//----- (0044103C) --------------------------------------------------------
void Engine::Draw() {
    drawWorld();
    drawHUD();
    render->flushAndScale();
    drawOverlay();
    render->swapBuffers();
}


void Engine::DrawGUI() {
    render->ResetUIClipRect();

    GameUI_DrawRightPanelFrames();
    _statusBar->draw();

    if (!pMovie_Track && uGameState != GAME_STATE_CHANGE_LOCATION) {  // ! pVideoPlayer->pSmackerMovie)
        GameUI_DrawMinimap(Recti(488, 16, 137, 117), viewparams->uMinimapZoom);
    }

    GameUI_DrawPartySpells();
    GameUI_DrawHiredNPCs();

    GameUI_DrawPortraits();
    GameUI_DrawLifeManaBars();
    GameUI_DrawCharacterSelectionFrame();
    if (_44100D_should_alter_right_panel()) GameUI_DrawRightPanel();

    if (!pMovie_Track) {
        spell_fx_renedrer->DrawPlayerBuffAnims();
        turnBasedOverlay.draw();
        GameUI_DrawTorchlightAndWizardEye();
    }

    static bool render_framerate = false;
    static float framerate = 0.0f;
    static unsigned frames_this_second = 0;
    static unsigned last_frame_time = platform->tickCount();
    static unsigned framerate_time_elapsed = 0;

    if (current_screen_type == SCREEN_GAME &&
        uCurrentlyLoadedLevelType == LEVEL_OUTDOOR)
        pWeather->Draw();  // Ritor1: my include

    // while(GetTickCount() - last_frame_time < 33 );//FPS control
    unsigned frame_dt = platform->tickCount() - last_frame_time;
    last_frame_time = platform->tickCount();
    framerate_time_elapsed += frame_dt;
    if (framerate_time_elapsed >= 1000) {
        framerate = frames_this_second * (1000.0f / framerate_time_elapsed);

        framerate_time_elapsed = 0;
        frames_this_second = 0;
        render_framerate = true;
    }

    ++frames_this_second;

    if (engine->config->debug.ShowFPS.value()) {
        if (render_framerate) {
            GUIWindow::DrawText(assets->pFontArrus.get(), {494, 0}, colorTable.White, fmt::format("FPS: {: .4f}", framerate), pPrimaryWindow->frameRect);
        }

        GUIWindow::DrawText(assets->pFontArrus.get(), {300, 0}, colorTable.White, fmt::format("DrawCalls: {}", render->drawcalls), pPrimaryWindow->frameRect);
        render->drawcalls = 0;


        int debug_info_offset = 16;
        GUIWindow::DrawText(assets->pFontArrus.get(), {16, debug_info_offset}, colorTable.White,
                                 fmt::format("Party position:         {:.2f} {:.2f} {:.2f}", pParty->pos.x, pParty->pos.y, pParty->pos.z), pPrimaryWindow->frameRect);
        debug_info_offset += 16;

        GUIWindow::DrawText(assets->pFontArrus.get(), {16, debug_info_offset}, colorTable.White,
                                 fmt::format("Party yaw/pitch:     {} {}", pParty->_viewYaw, pParty->_viewPitch), pPrimaryWindow->frameRect);
        debug_info_offset += 16;

        if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
            int sector_id = pBLVRenderParams->uPartySectorID;
            GUIWindow::DrawText(assets->pFontArrus.get(), { 16, debug_info_offset }, colorTable.White,
                                     fmt::format("Party Sector ID:       {}/{} ({})\n", sector_id, pIndoor->sectors.size(), pBLVRenderParams->uPartyEyeSectorID), pPrimaryWindow->frameRect);
            debug_info_offset += 16;
        }

        std::string floor_level_str;

        if (uGameState == GAME_STATE_CHANGE_LOCATION) {
            floor_level_str = "Loading Level!";
        } else if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
            int uFaceID;
            int sector_id = pBLVRenderParams->uPartySectorID;
            float floor_level = BLV_GetFloorLevel(pParty->pos/* + Vec3f(0,0,40) */, sector_id, &uFaceID);
            floor_level_str = fmt::format("BLV_GetFloorLevel: {}   face_id {}\nNodes: {}, Faces: {} ({}), Sectors: {}\n", floor_level, uFaceID, pBspRenderer->num_nodes, pBspRenderer->num_faces, pBLVRenderParams->uNumFacesRenderedThisFrame, pBspRenderer->uNumVisibleNotEmptySectors);
        } else if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
            bool on_water = false;
            int bmodel_pid;
            float floor_level = ODM_GetFloorLevel(pParty->pos, &on_water, &bmodel_pid);
            floor_level_str = fmt::format(
                "ODM_GetFloorLevel: {}   on_water: {}  on: {}\n",
                floor_level, on_water ? "true" : "false",
                bmodel_pid == 0
                    ? "---"
                    : fmt::format("BModel={} Face={}", bmodel_pid >> 6, bmodel_pid & 0x3F)
            );
        }

        GUIWindow::DrawText(assets->pFontArrus.get(), {16, debug_info_offset}, colorTable.White, floor_level_str, pPrimaryWindow->frameRect);
    }
}

//----- (0047A815) --------------------------------------------------------
void Engine::DrawParticles() {
    particle_engine->Draw();
}

void Engine::StackPartyTorchLight() {
    int TorchLightDistance = engine->config->graphics.TorchlightDistance.value();
    // TODO(pskelton): set this on level load
    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) TorchLightDistance = 1024;
    if (TorchLightDistance > 0) {  // lightspot around party
        if (pParty->TorchlightActive()) {
            // max is 800 * torchlight
            // min is 800
            int MinTorch = TorchLightDistance;
            int MaxTorch = TorchLightDistance * pParty->pPartyBuffs[PARTY_BUFF_TORCHLIGHT].power;

            int torchLightFlicker = engine->config->graphics.TorchlightFlicker.value();
            if (torchLightFlicker > 0) {
                // torchlight flickering effect
                // TorchLightPower *= pParty->pPartyBuffs[PARTY_BUFF_TORCHLIGHT].uPower;  // 2,3,4
                int ran = vrng->random(RAND_MAX);
                int mod = ((ran - (RAND_MAX * .4)) / torchLightFlicker); // TODO(captainurist): this math makes no sense
                TorchLightDistance = (pParty->TorchLightLastIntensity + mod);

                // clamp
                if (TorchLightDistance < MinTorch)
                    TorchLightDistance = MinTorch;
                if (TorchLightDistance > MaxTorch)
                    TorchLightDistance = MaxTorch;
            } else {
                TorchLightDistance = MaxTorch;
            }
        }

        // TODO(pskelton): move this
        // if outdoors and its day turn off
        if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR && !pWeather->bNight)
            TorchLightDistance = 0;

        pParty->TorchLightLastIntensity = TorchLightDistance;

        // TODO: either add conversion functions, or keep only glm / only Vec3_* classes.
        Vec3f pos(pCamera3D->vCameraPos.x, pCamera3D->vCameraPos.y, pCamera3D->vCameraPos.z);

        pMobileLightsStack->AddLight(
            pos, pBLVRenderParams->uPartySectorID, TorchLightDistance,
            colorTable.CarbonGray, _4E94D0_light_type);
    }
}

//----- (0044EE7C) --------------------------------------------------------
bool Engine::draw_debug_outlines() {
    if (/*uFlags & 0x04*/ engine->config->debug.LightmapDecals.value()) {
        DrawLightsDebugOutlines(-1);
        decal_builder->DrawDecalDebugOutlines();
    }
    return true;
}

//----- (0044ED0A) --------------------------------------------------------
int Engine::_44ED0A_saturate_face_blv(BLVFace *a2, int *a3, signed int a4) {
    double v4;  // st7@3
    // double v5; // ST00_8@3
    int v6;  // eax@4
    // double v7; // ST00_8@5
    int result;  // eax@7
    // double v9; // ST00_8@8
    // double v10; // ST00_8@10
    float v11;  // [sp+14h] [bp+8h]@3
    float v12;  // [sp+18h] [bp+Ch]@3
    float v13;  // [sp+18h] [bp+Ch]@5
    float v14;  // [sp+1Ch] [bp+10h]@8
    float v15;  // [sp+1Ch] [bp+10h]@10

    if (engine->IsSaturateFaces() && a2->attributes & FACE_IsSecret) {
        v4 = (double)a4;
        v11 = v4;
        *a3 |= 2u;
        v12 = (1.0 - this->fSaturation) * v4;
        // v5 = v12 + 6.7553994e15;
        if (floorf(v12 + 0.5f) /* SLODWORD(v5)*/ >= 0) {
            v13 = (1.0 - this->fSaturation) * v11;
            // v7 = v13 + 6.7553994e15;
            // v6 = LODWORD(v7);
            v6 = floorf(v13 + 0.5f);
        } else {
            v6 = 0;
        }
        if (a4 >= v6) {
            v14 = (1.0 - fSaturation) * v11;
            // v9 = v14 + 6.7553994e15;
            if (floorf(v14 + 0.5f) /* SLODWORD(v9)*/ >= 0) {
                v15 = (1.0 - fSaturation) * v11;
                // v10 = v15 + 6.7553994e15;
                // result = LODWORD(v10);
                result = floorf(v15 + 0.5f);
            } else {
                result = 0;
            }
        } else {
            result = a4;
        }
    } else {
        result = -1;
    }
    return result;
}

//----- (0044E4B7) --------------------------------------------------------
Engine::Engine(std::shared_ptr<GameConfig> config, OverlaySystem &overlaySystem) : _overlaySystem(overlaySystem) {
    this->config = config;
    this->bloodsplat_container = EngineIocContainer::ResolveBloodsplatContainer();
    this->decal_builder = EngineIocContainer::ResolveDecalBuilder();
    this->spell_fx_renedrer = EngineIocContainer::ResolveSpellFxRenderer();
    this->mouse = EngineIocContainer::ResolveMouse();
    this->particle_engine = EngineIocContainer::ResolveParticleEngine();
    this->vis = EngineIocContainer::ResolveVis();

    uNumStationaryLights_in_pStationaryLightsStack = 0;

    pCamera3D = new Camera3D;

    keyboardInputHandler = ::keyboardInputHandler;
    keyboardActionMapping = ::keyboardActionMapping;

    _resourceManager = std::make_unique<ResourceManager>();
}

//----- (0044E7F3) --------------------------------------------------------
Engine::~Engine() {
    delete pEventTimer;
    delete pCamera3D;
    pAudioPlayer.reset();
}

void Engine::LogEngineBuildInfo() {
    logger->info("OpenEnroth, revision {} built on {}", gitRevision(), buildTime());
    logger->info("Extra build information: {}/{}/{} {}", OE_BUILD_PLATFORM, OE_BUILD_ARCHITECTURE, OE_BUILD_COMPILER, PROJECT_VERSION);
}

//----- (0044EA5E) --------------------------------------------------------
Vis_PIDAndDepth Engine::PickMouse(float fPickDepth, int uMouseX, int uMouseY,
                                  Vis_SelectionFilter *sprite_filter, Vis_SelectionFilter *face_filter) {
    if (pViewport.contains(Pointi(uMouseX, uMouseY))) {
        return vis->PickMouse(fPickDepth, uMouseX, uMouseY, sprite_filter, face_filter);
    } else {
        return Vis_PIDAndDepth();
    }
}

//----- (0044EB12) --------------------------------------------------------
Vis_PIDAndDepth Engine::PickKeyboard(float pick_depth, Vis_SelectionFilter *sprite_filter, Vis_SelectionFilter *face_filter) {
    if (current_screen_type == SCREEN_GAME) {
        return vis->PickKeyboard(pick_depth, sprite_filter, face_filter);
    } else {
        return Vis_PIDAndDepth();
    }
}

Vis_PIDAndDepth Engine::PickMouseInfoPopup() {
    Pointi pt = mouse->position();
    // TODO(captainurist): Right now we can have popups for monsters that are not reachable with a bow, and this is OK.
    //                     However, such monsters also don't get a hint displayed on mouseover. Probably should fix this?
    return PickMouse(pCamera3D->GetMouseInfoDepth(), pt.x, pt.y, &vis_allsprites_filter, &vis_face_filter);
}

Vis_PIDAndDepth Engine::PickMouseTarget() {
    Pointi pt = mouse->position();
    return PickMouse(config->gameplay.RangedAttackDepth.value(), pt.x, pt.y, &vis_sprite_targets_filter, &vis_face_filter);
}

Vis_PIDAndDepth Engine::PickMouseNormal() {
    Pointi pt = mouse->position();
    return PickMouse(config->gameplay.RangedAttackDepth.value(), pt.x, pt.y, &vis_items_filter, &vis_face_filter);
}

void Engine::toggleOverlays() {
    bool isEnabled = _overlaySystem.isEnabled();
    _overlaySystem.setEnabled(!isEnabled);
    mouse->SetMouseLook(false);
}

void Engine::disableOverlays() {
    _overlaySystem.setEnabled(false);
}

/*
Result::Code Game::PickKeyboard(bool bOutline, struct unnamed_F93E6C *a3, struct
unnamed_F93E6C *a4)
{
if (dword_4E28F8_PartyCantJumpIfTrue)
return Result::Generic;

pVis->PickKeyboard(a3, a4);
if (bOutline)
Game_outline_selection((int)this);
return Result::Success;
}
*/
// 4E28F8: using guessed type int current_screen_type;

void PlayButtonClickSound() {
    pAudioPlayer->playNonResetableSound(SOUND_StartMainChoice02);
}

//----- (0046BDC0) --------------------------------------------------------
void UpdateUserInput_and_MapSpecificStuff() {
    if (dword_6BE364_game_settings_1 & GAME_SETTINGS_0080_SKIP_USER_INPUT_THIS_FRAME) {
        dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_0080_SKIP_USER_INPUT_THIS_FRAME;
        return;
    }

    UpdateObjects();

    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR)
        BLV_UpdateUserInputAndOther();
    else if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR)
        ODM_UpdateUserInputAndOther();

    checkDecorationEvents();
    evaluateAoeDamage();
}

//----- (004646F0) --------------------------------------------------------
void PrepareWorld(int _0_box_loading_1_fullscreen) {
    Vis *vis = EngineIocContainer::ResolveVis();

    CastSpellInfoHelpers::cancelSpellCastInProgress();
    pEventTimer->setPaused(true);
    pMiscTimer->setPaused(true);
    DoPrepareWorld(false, (_0_box_loading_1_fullscreen == 0) + 1);

    assert(pEventTimer->isPaused()); // DoPrepareWorld shouldn't un-pause.
    assert(pMiscTimer->isPaused());
    pMiscTimer->setPaused(false);
    pEventTimer->setPaused(false);
}

//----- (00464866) --------------------------------------------------------
void DoPrepareWorld(bool bLoading, int _1_fullscreen_loading_2_box) {
    engine->ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
    pGameLoadingUI_ProgressBar->Initialize(_1_fullscreen_loading_2_box == 1 ? GUIProgressBar::TYPE_Fullscreen : GUIProgressBar::TYPE_Box);

    engine->_OE_transientVariables.fill(0);
    loadMapEventsAndStrings(engine->_transitionMapId);

    // TODO(captainurist): need to zero this one out when loading a save, but is this a proper place to do that?
    attackList.clear();
    // Clearing actors lists mean turn engine queue will have invalid actor ids
    std::erase_if(pTurnEngine->pQueue, [](const auto& item) { return item.uPackedID.type() == OBJECT_Actor; });
    int configLimit = engine->config->gameplay.MaxActors.value();
    ai_near_actors_targets_pid.resize(configLimit, Pid());
    ai_near_actors_ids.resize(configLimit);

    engine->SetUnderwater(isMapUnderwater(engine->_transitionMapId));

    pParty->floor_face_id = 0; // TODO(captainurist): drop?

    engine->_currentLoadedMapId = engine->_transitionMapId;

    if (isMapIndoor(engine->_transitionMapId))
        loadAndPrepareBLV(engine->_transitionMapId, bLoading);
    else
        loadAndPrepareODM(engine->_transitionMapId, bLoading);

    pNPCStats->setNPCNamesOnLoad();
    engine->_461103_load_level_sub();
    if (engine->_currentLoadedMapId == MAP_BREEDING_ZONE || engine->_currentLoadedMapId == MAP_WALLS_OF_MIST) {
        // spawning grounds & walls of mist - no loot & exp from monsters

        for (unsigned i = 0; i < pActors.size(); ++i) {
            // TODO(captainurist): shouldn't we also set uTreasureLevel = ITEM_TREASURE_LEVEL_INVALID?
            pActors[i].monsterInfo.treasureType = RANDOM_ITEM_ANY;
            pActors[i].monsterInfo.goldDiceRolls = 0;
            pActors[i].monsterInfo.exp = 0;
        }
    }

    // OE fix - reduce maximum allowed radius in the Lincoln to stop act actors getting stuck in tight corridors.
    if (engine->_currentLoadedMapId == MAP_LINCOLN) {
        for (Actor& actor : pActors) {
            actor.radius = std::min(actor.radius, static_cast<uint16_t>(140));
        }
    }

    // OE fix - replace spirit lash with bless for clerics of the moon in the temple of baa.
    if (engine->_currentLoadedMapId == MAP_TEMPLE_OF_BAA)
        for (Actor& actor : pActors)
            if (actor.monsterInfo.spell2Id == SPELL_SPIRIT_SPIRIT_LASH)
                actor.monsterInfo.spell2Id = SPELL_SPIRIT_BLESS;

    bDialogueUI_InitializeActor_NPC_ID = 0;
    engine->_transitionMapId = MAP_INVALID;
    onMapLoad();
    pGameLoadingUI_ProgressBar->Progress();
    memset(&render->pBillboardRenderListD3D, 0, sizeof(render->pBillboardRenderListD3D));
    render->pSortedBillboardRenderListD3D.fill(nullptr);
    pGameLoadingUI_ProgressBar->Release();
}

//----- (004647AB) --------------------------------------------------------
void FinalInitialization() {
    InitializeTurnBasedAnimations(&stru_50C198);
    pBitmaps_LOD->reserveLoadedTextures();
    pSprites_LOD->reserveLoadedSprites();
    pIcons_LOD->reserveLoadedTextures();
}

void MM7_LoadLods() {
    engine->resources()->open();

    pIcons_LOD = new LodTextureCache;
    pIcons_LOD->open(dfs->read("data/icons.lod"));

    pBitmaps_LOD = new LodTextureCache;
    pBitmaps_LOD->open(dfs->read("data/bitmaps.lod"));

    pSprites_LOD = new LodSpriteCache;
    pSprites_LOD->open(dfs->read("data/sprites.lod"));

    // TODO(captainurist):
    // on error in `open` we had this:
    // Error(localization->str(LSTR_MIGHT_AND_MAGIC_VII_IS_HAVING_TROUBLE), localization->str(LSTR_REINSTALL_NECESSARY));
    // however, at this point localization isn't initialized yet, so this was a guaranteed crash.
    // Implement proper user-facing error reporting!

    pPaletteManager->load(pBitmaps_LOD);
}

//----- (004651F4) --------------------------------------------------------
void Engine::MM7_Initialize() {
    grng->seed(platform->tickCount());
    vrng->seed(platform->tickCount());

    pEventTimer = new Timer();

    pParty = new Party();

    pParty->pHirelings.fill(NPCData());
    pParty->eyeLevel = engine->config->gameplay.PartyEyeLevel.value();
    pParty->height = engine->config->gameplay.PartyHeight.value();
    pParty->walkSpeed = engine->config->gameplay.PartyWalkSpeed.value();

    _messageQueue = std::make_unique<GUIMessageQueue>();

    MM6_Initialize();

    _statusBar = std::make_unique<StatusBar>();

    MM7_LoadLods();

    localization = new Localization();
    localization->initialize();

    pSpriteFrameTable = new SpriteFrameTable;
    deserialize(engine->resources()->eventsData("dsft.bin"), pSpriteFrameTable);

    pTextureFrameTable = new TextureFrameTable;
    deserialize(engine->resources()->eventsData("dtft.bin"), pTextureFrameTable);

    pTileTable = new TileTable;
    deserialize(engine->resources()->eventsData("dtile.bin"), pTileTable);

    pPortraitFrameTable = new PortraitFrameTable;
    deserialize(engine->resources()->eventsData("dpft.bin"), pPortraitFrameTable);

    pIconsFrameTable = new IconFrameTable;
    deserialize(engine->resources()->eventsData("dift.bin"), pIconsFrameTable);

    pDecorationList = new DecorationList;
    deserialize(engine->resources()->eventsData("ddeclist.bin"), pDecorationList);

    pObjectList = new ObjectList;
    deserialize(engine->resources()->eventsData("dobjlist.bin"), pObjectList);

    pMonsterList = new MonsterList;
    deserialize(engine->resources()->eventsData("dmonlist.bin"), pMonsterList);

    pOverlayList = new OverlayList;
    deserialize(engine->resources()->eventsData("doverlay.bin"), pOverlayList);

    pSoundList = new SoundList;
    deserialize(engine->resources()->eventsData("dsounds.bin"), pSoundList);

    if (!config->debug.NoSound.value())
        pAudioPlayer->Initialize();

    pMediaPlayer = new MPlayer();
    pMediaPlayer->Initialize();

    pTileGenerator = new TileGenerator();
    if (engine->config->graphics.GenerateTiles.value())
        pTileGenerator->fillTable();

    dword_6BE364_game_settings_1 |= GAME_SETTINGS_4000;
}

//----- (00465D0B) --------------------------------------------------------
void Engine::SecondaryInitialization() {
    mouse->Initialize();

    pMapStats = new MapStats();
    pMapStats->Initialize(engine->resources()->eventsData("MapStats.txt"));

    pMonsterStats = new MonsterStats();
    pMonsterStats->Initialize(engine->resources()->eventsData("monsters.txt"));
    pMonsterStats->InitializePlacements(engine->resources()->eventsData("placemon.txt"));

    pSpellStats = new SpellStats();
    pSpellStats->Initialize(engine->resources()->eventsData("spells.txt"));

    pFactionTable = new FactionTable();
    pFactionTable->Initialize(engine->resources()->eventsData("hostile.txt"));

    pHistoryTable = new HistoryTable();
    pHistoryTable->Initialize(engine->resources()->eventsData("history.txt"));

    pItemTable = new ItemTable();
    pItemTable->Initialize(engine->resources());

    initializeHouses(engine->resources()->eventsData("2dEvents.txt"));

    //pPaletteManager->SetMistColor(128, 128, 128);
    //pPaletteManager->RecalculateAll();
    pObjectList->InitializeSprites();
    pOverlayList->InitializeSprites();

    // TODO(captainurist): try resurrecting the food / gold animations using resource files from MM6?
    //for (unsigned i = 0; i < 4; ++i) {
    //    static const char *pUIAnimNames[4] = {"glow03", "glow05", "torchA", "wizeyeA"};
    //    static unsigned short _4E98D0[4][4] = { {479, 0, 329, 0}, {585, 0, 332, 0}, {468, 0, 0, 0}, {606, 0, 0, 0} };
    //
    //    // pUIAnims[i]->uIconID = pIconsFrameTable->FindIcon(pUIAnimNames[i]);
    //    pUIAnims[i]->icon = pIconsFrameTable->GetIcon(pUIAnimNames[i]);
    //
    //    pUIAnims[i]->uAnimLength = 0_ticks;
    //    pUIAnims[i]->uAnimTime = 0_ticks;
    //    pUIAnims[i]->x = _4E98D0[i][0];
    //    pUIAnims[i]->y = _4E98D0[i][2];
    //}

    spell_fx_renedrer->LoadAnimations();

    pNPCStats = new NPCStats();
    pNPCStats->Initialize(engine->resources());

    initializeQuests(engine->resources()->eventsData("quests.txt"));
    initializeAutonotes(engine->resources()->eventsData("autonote.txt"));
    initializeAwards(engine->resources()->eventsData("awards.txt"));
    initializeTransitions(engine->resources()->eventsData("trans.txt"));
    initializeMerchants(engine->resources()->eventsData("merchant.txt"));
    initializeMessageScrolls(engine->resources()->eventsData("scroll.txt"));
    initializeChests();

    engine->_globalEventMap = EvtProgram::load(engine->resources()->eventsData("global.evt"));

    pBitmaps_LOD->reserveLoadedTextures();
    pSprites_LOD->reserveLoadedSprites();

    Initialize_GamesLOD_NewLOD();
}

void Engine::Initialize() {
    _indoor = std::make_unique<IndoorLocation>();
    _outdoor = std::make_unique<OutdoorLocation>();
    _stationaryLights = std::make_unique<LightsStack_StationaryLight_>();
    _mobileLights = std::make_unique<LightsStack_MobileLight_>();

    ::pIndoor = _indoor.get();
    ::pOutdoor = _outdoor.get();
    ::pStationaryLightsStack = _stationaryLights.get();
    ::pMobileLightsStack = _mobileLights.get();

    MM7_Initialize();

    pEventTimer->setPaused(true);

    GUIWindow::InitializeGUI();
}

//----- (00466082) --------------------------------------------------------
void MM6_Initialize() {
    viewparams = new ViewingParams;
    pAudioPlayer = std::make_unique<AudioPlayer>();

    pODMRenderParams = new ODMRenderParams;
    pODMRenderParams->outdoor_no_mist = 0;
    pODMRenderParams->bNoSky = 0;
    pODMRenderParams->bDoNotRenderDecorations = 0;
    pODMRenderParams->outdoor_no_wavy_water = 0;
    pODMRenderParams->terrain_gamma = 0;
    pODMRenderParams->building_gamme = 0;
    pODMRenderParams->shading_dist_shade = 2048;
    pODMRenderParams->shading_dist_shademist = 4096;

    MM7Initialization();
}

//----- (004666D5) --------------------------------------------------------
void MM7Initialization() {
    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        pODMRenderParams->shading_dist_shade = 2048;
        pODMRenderParams->terrain_gamma = 0;
        pODMRenderParams->building_gamme = 0;
        pODMRenderParams->shading_dist_shademist = 4096;
        pODMRenderParams->outdoor_no_wavy_water = 0;
    }
}

//----- (00464479) --------------------------------------------------------
void Engine::ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows() {
    if (mouse)
        mouse->SetCursorImage("MICON1");

    // Render billboards are used in hit tests, but we're releasing textures, so can't use them anymore.
    render->uNumBillboardsToDraw = 0;

    pBitmaps_LOD->releaseUnreserved();
    pSprites_LOD->releaseUnreserved();
    pIcons_LOD->releaseUnreserved();

    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR)
        pIndoor->Release();
    else if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR)
        pOutdoor->Release();

    pAudioPlayer->stopSounds();
    uCurrentlyLoadedLevelType = LEVEL_NULL;
    pSpriteFrameTable->ResetLoadedFlags();
    pParty->armageddon_timer = 0_ticks;

    windowManager.DeleteAllVisibleWindows();
}

//----- (00461103) --------------------------------------------------------
void Engine::_461103_load_level_sub() {
    if (engine->config->debug.NoActors.value())
        pActors.clear();

    GenerateItemsInChest();
    UpdateChestPositions();
    pGameLoadingUI_ProgressBar->Progress();
    pParty->arenaState = ARENA_STATE_INITIAL;
    pParty->arenaLevel = ARENA_LEVEL_INVALID;
    pNPCStats->uNewlNPCBufPos = 0;

    for (size_t i = 0; i < pActors.size(); ++i) {
        MonsterTier tier = monsterTierForMonsterId(pActors[i].monsterInfo.id);
        if (tier == MONSTER_TIER_A)
            continue; // Weakest peasants are just peasants.

        if (pActors[i].npcId && pActors[i].npcId < 5000)
            continue;

        if (isPeasant(pActors[i].monsterInfo.id)) {
            pNPCStats->InitializeAdditionalNPCs(
                &pNPCStats->pAdditionalNPC[pNPCStats->uNewlNPCBufPos],
                pActors[i].monsterInfo.id, HOUSE_INVALID, engine->_currentLoadedMapId);
            pActors[i].npcId = pNPCStats->uNewlNPCBufPos + 5000;
            pNPCStats->uNewlNPCBufPos++;
            continue;
        }

        pActors[i].npcId = 0;
    }

    pGameLoadingUI_ProgressBar->Progress();

    pGameLoadingUI_ProgressBar->Progress();

    if (engine->config->debug.NoActors.value())
        pActors.clear();
    if (engine->config->debug.NoDecorations.value())
        pLevelDecorations.clear();
    initDecorationEvents();

    pGameLoadingUI_ProgressBar->Progress();

    pCamera3D->vCameraPos.x = 0;
    pCamera3D->vCameraPos.y = 0;
    pCamera3D->vCameraPos.z = 100;
    pCamera3D->_viewPitch = 0;
    pCamera3D->_viewYaw = 0;
    uLevel_StartingPointType = MAP_START_POINT_PARTY;
    if (pParty->pPickedItem.itemId != ITEM_NULL)
        mouse->SetCursorBitmapFromItemID(pParty->pPickedItem.itemId);
}

//----- (0042F3D6) --------------------------------------------------------
void InitializeTurnBasedAnimations(void *_this) {
    uSpriteID_Spell11 = pSpriteFrameTable->FastFindSprite("spell11");

    turnBasedOverlay.loadIcons();
}

//----- (0046BDA8) --------------------------------------------------------
int GetGravityStrength() {
    return engine->config->gameplay.Gravity.value();
}

void sub_44861E_set_texture_indoor(unsigned int uFaceCog,
                                   std::string_view filename) {
    for (unsigned i = 1; i < pIndoor->faceExtras.size(); ++i) {
        auto extra = &pIndoor->faceExtras[i];
        if (extra->sCogNumber == uFaceCog) {
            auto face = &pIndoor->faces[extra->face_id];
            face->SetTexture(filename);
        }
    }
}

void sub_44861E_set_texture_outdoor(unsigned int uFaceCog,
                                    std::string_view filename) {
    for (BSPModel &model : pOutdoor->pBModels) {
        for (ODMFace &face : model.faces) {
            if (face.cogNumber == uFaceCog) {
                face.SetTexture(filename);
            }
        }
    }
}

void setTexture(unsigned int uFaceCog, std::string_view pFilename) {
    if (uFaceCog) {
        // unsigned int texture = pBitmaps_LOD->LoadTexture(pFilename);
        // if (texture != -1)
        {
            // pBitmaps_LOD->pTextures[texture].palette_id2 =
            // pPaletteManager->LoadPalette(pBitmaps_LOD->pTextures[texture].palette_id1);

            if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                sub_44861E_set_texture_indoor(uFaceCog, pFilename);
            } else {
                sub_44861E_set_texture_outdoor(uFaceCog, pFilename);
            }
        }
    }
}

void setFacesBit(int sCogNumber, FaceAttribute bit, int on) {
    if (sCogNumber) {
        if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
            for (unsigned i = 1; i < (unsigned int)pIndoor->faceExtras.size(); ++i) {
                if (pIndoor->faceExtras[i].sCogNumber == sCogNumber) {
                    if (on)
                        pIndoor->faces[pIndoor->faceExtras[i].face_id]
                            .attributes |= bit;
                    else
                        pIndoor->faces[pIndoor->faceExtras[i].face_id]
                            .attributes &= ~bit;
                }
            }
        } else {
            for (BSPModel &model : pOutdoor->pBModels) {
                for (ODMFace &face : model.faces) {
                    if (face.cogNumber == sCogNumber) {
                        if (on) {
                            face.attributes |= bit;
                        } else {
                            face.attributes &= ~bit;
                        }
                    }
                }
            }
        }
    }
}

void setDecorationSprite(uint16_t uCog, bool bHide, std::string_view pFileName) {
    for (size_t i = 0; i < pLevelDecorations.size(); i++) {
        if (pLevelDecorations[i].uCog == uCog) {
            if (!pFileName.empty() && pFileName != "0") {
                pLevelDecorations[i].uDecorationDescID = pDecorationList->GetDecorIdByName(pFileName);
                pDecorationList->InitializeDecorationSprite(pLevelDecorations[i].uDecorationDescID);
            }

            if (bHide)
                pLevelDecorations[i].uFlags &= ~LEVEL_DECORATION_INVISIBLE;
            else
                pLevelDecorations[i].uFlags |= LEVEL_DECORATION_INVISIBLE;
        }
    }
}

//----- (004356FF) --------------------------------------------------------
void back_to_game() {
    holdingMouseRightButton = false;
    rightClickItemActionPerformed = false;
    identifyOrRepairReactionPlayed = false;

    if (pGUIWindow_ScrollWindow) {
        pGUIWindow_ScrollWindow->Release();
        pGUIWindow_ScrollWindow = nullptr;
    }

    if (current_screen_type == SCREEN_GAME && sCurrentMenuID == MENU_NONE && !pGUIWindow_CastTargetedSpell) {
        pEventTimer->setPaused(false);
    }
}

//----- (00494035) --------------------------------------------------------
void _494035_timed_effects__water_walking_damage__etc(Duration dt) {
    Time oldTime = pParty->GetPlayingTime();
    Time newTime = oldTime + dt;
    pParty->GetPlayingTime() = newTime;

    CivilTime time = pParty->GetPlayingTime().toCivilTime();
    pParty->uCurrentTimeSecond = time.second;
    pParty->uCurrentMinute = time.minute;
    pParty->uCurrentHour = time.hour;
    pParty->uCurrentMonthWeek = time.week - 1;
    pParty->uCurrentDayOfMonth = time.day - 1;
    pParty->uCurrentMonth = time.month - 1;
    pParty->uCurrentYear = time.year;

    // New day dawns at 3am.
    Time next3am = Time::fromDurationSinceSilence((oldTime.toDurationSinceSilence() - Duration::fromHours(3)).roundedUp(Duration::fromDays(1)) + Duration::fromHours(3));
    if (oldTime < next3am && newTime >= next3am) {
        pParty->pHirelings[0].hasUsedAbility = false;
        pParty->pHirelings[1].hasUsedAbility = false;

        for (unsigned i = 0; i < pNPCStats->uNumNewNPCs; ++i)
            pNPCStats->pNPCData[i].hasUsedAbility = false;

        ++pParty->days_played_without_rest;
        if (pParty->days_played_without_rest > 1) {
            for (Character &character : pParty->pCharacters)
                character.SetCondWeakWithBlockCheck(0);

            // starving
            if (pParty->GetFood() > 0) {
                pParty->TakeFood(1);
            } else {
                for (Character &character : pParty->pCharacters) {
                    character.health = character.health / (pParty->days_played_without_rest + 1) + 1;
                }
            }

            // players go insane without rest
            if (pParty->days_played_without_rest > 3) {
                for (Character &character : pParty->pCharacters) {
                    character.resetTempBonuses();
                    if (!character.IsPetrified() && !character.IsEradicated() && !character.IsDead()) {
                        if (grng->random(100) < 5 * pParty->days_played_without_rest)
                            character.SetCondDeadWithBlockCheck(0);
                        if (grng->random(100) < 10 * pParty->days_played_without_rest)
                            character.SetCondInsaneWithBlockCheck(0);
                    }
                }
            }
        }
        if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR)
            pOutdoor->SetFog();

        for (Character &character : pParty->pCharacters) {
            character.uNumDivineInterventionCastsThisDay = 0;
            character.uNumArmageddonCasts = 0;
            character.uNumFireSpikeCasts = 0; // TODO(pskelton): adding this here for now but behaviour around firespike permanence needs checking
        }
    }

    // TODO(pskelton): do water and lava damage need to be more accurate to dt?
    // water damage
    if (pParty->uFlags & PARTY_FLAG_WATER_DAMAGE && pParty->_6FC_water_lava_timer < pParty->GetPlayingTime()) {
        pParty->_6FC_water_lava_timer = pParty->GetPlayingTime() + 128_ticks;
        for (Character &character : pParty->pCharacters) {
            if (character.wearsItem(ITEM_RELIC_HARECKS_LEATHER) ||
                character.wearsEnchantedItem(ITEM_ENCHANTMENT_OF_WATER_WALKING) ||
                character.pCharacterBuffs[CHARACTER_BUFF_WATER_WALK].Active()) {
                character.playEmotion(PORTRAIT_SMILE, 0_ticks);
            } else {
                if (!character.hasUnderwaterSuitEquipped()) {
                    character.receiveDamage((int64_t)character.GetMaxHealth() * 0.1, DAMAGE_FIRE); // TODO(pskelton): fire damage?
                    engine->_statusBar->setEventShort(LSTR_YOU_ARE_DROWNING);
                } else {
                    character.playEmotion(PORTRAIT_SMILE, 0_ticks);
                }
            }
        }
    }

    // lava damage
    if (pParty->uFlags & PARTY_FLAG_BURNING && pParty->_6FC_water_lava_timer < pParty->GetPlayingTime()) {
        pParty->_6FC_water_lava_timer = pParty->GetPlayingTime() + 128_ticks;

        for (Character &character : pParty->pCharacters) {
            character.receiveDamage((int64_t)character.GetMaxHealth() * 0.1, DAMAGE_FIRE);
        }
        engine->_statusBar->setEventShort(LSTR_YOU_ARE_BURNING);
    }

    RegeneratePartyHealthMana();

    // TODO(captainurist): #time drop once we move to msecs in duration.
    Duration recoveryTimeDt = dt;
    recoveryTimeDt += pParty->_roundingDt;
    pParty->_roundingDt = 0_ticks;
    if (pParty->uFlags2 & PARTY_FLAGS_2_RUNNING && recoveryTimeDt > 0_ticks) {  // half recovery speed if party is running
        pParty->_roundingDt = recoveryTimeDt % 2_ticks;
        recoveryTimeDt /= 2;
    }

    unsigned numPlayersCouldAct = pParty->pCharacters.size();
    for (Character &character : pParty->pCharacters) {
        if (character.timeToRecovery && recoveryTimeDt > 0_ticks)
            character.Recover(recoveryTimeDt);

        if (character.GetBaseEndurance() + character.health >= 1 ||
            character.pCharacterBuffs[CHARACTER_BUFF_PRESERVATION].Active()) {
            if (character.health < 1)
                character.SetCondition(CONDITION_UNCONSCIOUS, 0);
        } else {
            character.SetCondition(CONDITION_DEAD, 0);
        }

        if (!character.CanAct()) {
            --numPlayersCouldAct;
        }

        for (auto &playerBuff : character.pCharacterBuffs) {
            playerBuff.IsBuffExpiredToTime(pParty->GetPlayingTime());
        }

        if (character.pCharacterBuffs[CHARACTER_BUFF_HASTE].Expired()) {
            character.SetCondition(CONDITION_WEAK, 0);
            character.pCharacterBuffs[CHARACTER_BUFF_HASTE].Reset();
        }
    }

    for (auto &partyBuff : pParty->pPartyBuffs) {
        if (partyBuff.IsBuffExpiredToTime(pParty->GetPlayingTime()) == 1) {
            /* Do nothing, check above has side effects. */
        }
    }

    if (pParty->pPartyBuffs[PARTY_BUFF_HASTE].Expired()) {
        for (Character &character : pParty->pCharacters)
            character.SetCondition(CONDITION_WEAK, 0);
        pParty->pPartyBuffs[PARTY_BUFF_HASTE].Reset();
    }

    // Check if Fly/Water Walk caster can act
    for (PartyBuff buffIdx : {PARTY_BUFF_WATER_WALK, PARTY_BUFF_FLY}) {
        SpellBuff *pBuff = &pParty->pPartyBuffs[buffIdx];
        if (pBuff->Inactive()) {
            continue;
        }

        if (!pBuff->isGM) {
            if (!pParty->pCharacters[pBuff->caster - 1].CanAct()) {
                pBuff->Reset();
                if (buffIdx == PARTY_BUFF_FLY) {
                    pParty->bFlying = false;
                }
            }
        }
    }

    maybeWakeSoloSurvivor();
    updatePartyDeathState();

    if (pParty->hasActiveCharacter()) {
        if (current_screen_type != SCREEN_REST) {
            if (!pParty->activeCharacter().CanAct()) {
                pParty->switchToNextActiveCharacter();
            }
        }
    }
}

void maybeWakeSoloSurvivor() {
    if (current_screen_type == SCREEN_REST)
        return;

    if (pParty->canActCount() != 0)
        return;

    // Try waking up a single character.
    for (Character &character : pParty->pCharacters) {
        if (character.conditions.has(CONDITION_SLEEP)) {
            if (character.conditions.hasNone({ CONDITION_PARALYZED, CONDITION_UNCONSCIOUS, CONDITION_DEAD, CONDITION_PETRIFIED, CONDITION_ERADICATED })) {
                character.conditions.reset(CONDITION_SLEEP);
                pParty->setActiveToFirstCanAct();
                break;
            }
        }
    }
}

void updatePartyDeathState() {
    if (current_screen_type != SCREEN_REST && pParty->canActCount() == 0)
        uGameState = GAME_STATE_PARTY_DIED;
}

void RegeneratePartyHealthMana() {
    Duration newTime = pParty->GetPlayingTime().toDurationSinceSilence();
    Duration oldTime = pParty->last_regenerated.toDurationSinceSilence();

    // This used to trigger if:
    // - Current time is at a 5-min mark (time.toMinutes() % 5 == 0),
    // - Or if at least 5 mins have passed.
    //
    // And then there was a loop that went in 5-min increments to do regen several times if for example 15 minutes have
    // passed.
    //
    // New logic is:
    // - Calculate the number of 5-min ticks that have passed. E.g. there is only one 5-min tick between 1 and 9.
    // - Do a single run of the logic below.

    auto ticksBetween = [](Duration lo, Duration hi, Duration interval) {
        // Calculate # of interval-spaced ticks in [lo, hi).
        Duration loUp = lo.roundedUp(interval);
        Duration hiDn = (hi - 1_ticks).roundedDown(interval);
        return (hiDn - loUp) / interval + 1;
    };

    int ticks5 = ticksBetween(oldTime, newTime, Duration::fromMinutes(5));
    if (ticks5 <= 0)
        return;

    // TODO: actually this looks like it never triggers.
    // we get cursed_times, which is a time the character was cursed since the start of the game (a very large number),
    // and compare it with times_triggered, which is a small number

    // See #123 for discussion about this logic.
    // Curse processing seems to be broken here.
#if 0
    // chance to flight break due to a curse
    if (pParty->FlyActive()) {
        if (pParty->bFlying) {
            if (!(pParty->pPartyBuffs[PARTY_BUFF_FLY].uFlags & 1)) {
                // uPower is 0 for GM, 1 for every other skill level
                unsigned short spell_power = times_triggered * pParty->pPartyBuffs[PARTY_BUFF_FLY].uPower;

                int caster = pParty->pPartyBuffs[PARTY_BUFF_FLY].uCaster - 1;
                GameTime cursed_times = pParty->pCharacters[caster].conditions.Get(CONDITION_CURSED);
                if (cursed_times.Valid() && cursed_times.value < spell_power) {
                    // TODO: cursed_times was a pointer before, and we had cursed_times = 0 here,
                    // was this meant to cancel the curse?
                    pParty->uFlags &= 0xFFFFFFBF;
                    pParty->bFlying = false;
                }
            }
        }
    }
#endif

    // See #123 for discussion about this logic.
    // And also current code seems to be broken because it plainly curses Water Walk caster.
#if 0
    // chance to waterwalk drowning due to a curse
    if (pParty->WaterWalkActive()) {
        if (pParty->uFlags & PARTY_FLAG_STANDING_ON_WATER) {
            if (!(pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uFlags & 1)) {  // taking on water
                int caster = pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uCaster - 1;
                GameTime cursed_times = pParty->pCharacters[caster].conditions.Get(CONDITION_CURSED);
                cursed_times.value -= times_triggered;
                if (cursed_times.value <= 0) {
                    cursed_times.value = 0;
                    pParty->uFlags &= ~PARTY_FLAG_STANDING_ON_WATER;
                }
                pParty->pCharacters[caster].conditions.set(CONDITION_CURSED, cursed_times);
            }
        }
    }
#endif

    // Mana drain from flying
    // GM does not drain
    if (!engine->config->debug.AllMagic.value() && pParty->FlyActive() && !pParty->pPartyBuffs[PARTY_BUFF_FLY].isGM) {
        if (pParty->bFlying) {
            int caster = pParty->pPartyBuffs[PARTY_BUFF_FLY].caster - 1;
            pParty->pCharacters[caster].mana = std::max(0, pParty->pCharacters[caster].mana - ticks5);
        }
    }

    // Mana drain from water walk
    // GM does not drain
    if (!engine->config->debug.AllMagic.value() && pParty->WaterWalkActive() && !pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].isGM) {
        if (pParty->uFlags & PARTY_FLAG_STANDING_ON_WATER) {
            int caster = pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].caster - 1;

            int ticksW = ticks5;
            // Vanilla bug: Water Walk drains mana with the same speed as Fly.
            if (engine->config->gameplay.FixWaterWalkManaDrain.value())
                ticksW = ticksBetween(oldTime, newTime, Duration::fromMinutes(20));

            pParty->pCharacters[caster].mana = std::max(0, pParty->pCharacters[caster].mana - ticksW);
        }
    }

    // Immolation fire spell aura damage.
    // Note that immolation hits only once, even if we have more than 1 tick. This is intentional, because why would you
    // care about immolation if you're traveling, or if you're in jail? You won't be getting several ticks here during
    // normal gameplay.
    if (pParty->ImmolationActive()) {
        SpriteObject spellSprite;
        spellSprite.containing_item.Reset();
        spellSprite.spell_level = pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].power;
        spellSprite.spell_skill = pParty->ImmolationSkillLevel();
        spellSprite.uType = SPRITE_SPELL_FIRE_IMMOLATION;
        spellSprite.uSpellID = SPELL_FIRE_IMMOLATION;
        spellSprite.uObjectDescID = pObjectList->ObjectIDByItemID(SpellSpriteMapping[SPELL_FIRE_IMMOLATION]);
        spellSprite.field_60_distance_related_prolly_lod = 0;
        spellSprite.uAttributes = 0;
        spellSprite.uSectorID = 0;
        spellSprite.timeSinceCreated = 0_ticks;
        spellSprite.spell_caster_pid = Pid(OBJECT_Character, pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].caster - 1); // caster is 1 indexed so turn back to 0
        spellSprite.uFacing = 0;
        spellSprite.uSoundID = 0;

        int actorsAffectedByImmolation[100];
        size_t numberOfActorsAffected = pParty->immolationAffectedActors(actorsAffectedByImmolation, 100, 307);
        int totalDmg = 0; int hitCount = 0;
        for (size_t idx = 0; idx < numberOfActorsAffected; ++idx) {
            int actorID = actorsAffectedByImmolation[idx];
            spellSprite.vPosition.x = pActors[actorID].pos.x;
            spellSprite.vPosition.y = pActors[actorID].pos.y;
            spellSprite.vPosition.z = pActors[actorID].pos.z;
            spellSprite.spell_target_pid = Pid(OBJECT_Actor, actorID);
            int thisDmg = Actor::DamageMonsterFromParty(Pid(OBJECT_Sprite, spellSprite.Create(0, 0, 0, 0)), actorID, Vec3f());
            if (thisDmg) hitCount++;
            totalDmg += thisDmg;
        }

        // Override status bar
        if (engine->config->settings.ShowHits.value() && totalDmg > 0) {
            engine->_statusBar->setEvent(LSTR_IMMOLATION_DAMAGE, totalDmg, hitCount);
        }
    }

    bool stacking = engine->config->gameplay.RegenStacking.value();
    for (Character &character : pParty->pCharacters) {
        if (character.conditions.hasAny({CONDITION_DEAD, CONDITION_ERADICATED}))
            continue; // No HP/MP regen/drain for dead characters.

        RegenData thisChar;
        // Item regeneration
        for (InventoryEntry item : character.inventory.functionalEquipment()) {
            if (!isRegular(item->itemId)) {
                if (item->itemId == ITEM_RELIC_ETHRICS_STAFF) {
                    character.health -= ticks5;
                }
                if (item->itemId == ITEM_ARTIFACT_HERMES_SANDALS) {
                    thisChar.hpRegen++;
                    thisChar.spRegen++;
                }
                if (item->itemId == ITEM_ARTIFACT_MINDS_EYE) {
                    thisChar.spRegen++;
                }
                if (item->itemId == ITEM_ARTIFACT_HEROS_BELT) {
                    thisChar.hpRegen++;
                }
            } else {
                ItemEnchantment special_enchantment = item->specialEnchantment;
                if (special_enchantment == ITEM_ENCHANTMENT_OF_REGENERATION
                    || special_enchantment == ITEM_ENCHANTMENT_OF_LIFE
                    || special_enchantment == ITEM_ENCHANTMENT_OF_PHOENIX
                    || special_enchantment == ITEM_ENCHANTMENT_OF_TROLL) {
                    thisChar.hpRegen++;
                }

                if (special_enchantment == ITEM_ENCHANTMENT_OF_MANA
                    || special_enchantment == ITEM_ENCHANTMENT_OF_ECLIPSE
                    || special_enchantment == ITEM_ENCHANTMENT_OF_UNICORN) {
                    thisChar.spRegen++;
                }

                if (special_enchantment == ITEM_ENCHANTMENT_OF_PLENTY) {
                    thisChar.hpRegen++;
                    thisChar.spRegen++;
                }
            }
        }

        // Regeneration buff.
        if (character.pCharacterBuffs[CHARACTER_BUFF_REGENERATION].Active()) {
            thisChar.hpSpellRegen = 5 * character.pCharacterBuffs[CHARACTER_BUFF_REGENERATION].power;
        }

        // Warlock mana regen.
        if (PartyHasDragon() && character.classType == CLASS_WARLOCK) {
            thisChar.spRegen++;
        }

        // Lich mana/health drain/regen.
        if (character.classType == CLASS_LICH) {
            bool lich_has_jar = false;
            for (InventoryEntry jar : character.inventory.entries(ITEM_QUEST_LICH_JAR_FULL))
                if (jar->lichJarCharacterIndex == character.getCharacterIndex())
                    lich_has_jar = true;

            if (lich_has_jar) {
                thisChar.spRegen++;
            } else {
                character.health = std::min(character.health, std::max(character.GetMaxHealth() / 2, character.health - 2 * ticks5));
                character.mana = std::min(character.mana, std::max(character.GetMaxMana() / 2, character.mana - 2 * ticks5));
            }
        }

        character.tickRegeneration(ticks5, thisChar, stacking);

        // Zombie mana/health drain.
        if (character.conditions.has(CONDITION_ZOMBIE)) {
            character.health = std::min(character.health, std::max(character.GetMaxHealth() / 2, character.health - ticks5));
            character.mana = std::max(0, character.mana - ticks5);
        }

        // Wake up unconscious chars due to hp regen.
        if (character.health > 0 && character.conditions.has(CONDITION_UNCONSCIOUS))
            character.conditions.reset(CONDITION_UNCONSCIOUS);

        // Knock out / kill chars due to hp drain.
        if (character.health <= 0) {
            int enduranceCheck = character.health + character.GetBaseEndurance();
            Condition targetCondition = enduranceCheck >= 1 || character.pCharacterBuffs[CHARACTER_BUFF_PRESERVATION].Active() ? CONDITION_UNCONSCIOUS : CONDITION_DEAD;
            if (!character.conditions.has(targetCondition))
                character.conditions.set(targetCondition, pParty->GetPlayingTime());
        }
    }

    pParty->last_regenerated = pParty->GetPlayingTime();
}

Duration timeUntilDawn() {
    Time now = pParty->GetPlayingTime();
    Time next5am = Time::fromDurationSinceSilence((now.toDurationSinceSilence() - Duration::fromHours(5) + 1_ticks).roundedUp(Duration::fromDays(1)) + Duration::fromHours(5));
    return next5am - now;
}

void initLevelStrings(const Blob &blob) {
    engine->_levelStrings.clear();

    int offs = 0;
    while (offs < blob.size()) {
        const char *nextNullTerm = (const char*)memchr(&blob.str()[offs], '\0', blob.size() - offs);
        size_t stringSize = nextNullTerm ? (nextNullTerm - &blob.str()[offs]) : (blob.size() - offs);
        engine->_levelStrings.push_back(trimRemoveQuotes(std::string(&blob.str()[offs], stringSize)));
        offs += stringSize + 1;
    }
}

void loadMapEventsAndStrings(MapId mapid) {
    std::string mapName = pMapStats->pInfos[mapid].fileName;
    std::string mapNameWithoutExt = mapName.substr(0, mapName.rfind('.'));

    initLevelStrings(engine->resources()->eventsData(fmt::format("{}.str", mapNameWithoutExt)));

    engine->_localEventMap = EvtProgram::load(engine->resources()->eventsData(fmt::format("{}.evt", mapNameWithoutExt)));
}

bool _44100D_should_alter_right_panel() {
    return current_screen_type == SCREEN_NPC_DIALOGUE ||
           current_screen_type == SCREEN_CHARACTERS ||
           current_screen_type == SCREEN_HOUSE ||
           current_screen_type == SCREEN_SHOP_INVENTORY ||
           current_screen_type == SCREEN_CHANGE_LOCATION ||
           current_screen_type == SCREEN_INPUT_BLV ||
           current_screen_type == SCREEN_CASTING;
}

void Transition_StopSound_Autosave(std::string_view pMapName,
                                   MapStartPoint start_point) {
    pAudioPlayer->stopSounds();

    // pGameLoadingUI_ProgressBar->Initialize(GUIProgressBar::TYPE_None);

    if (engine->_currentLoadedMapId != pMapStats->GetMapInfo(pMapName)) {
        autoSave();
    }

    uGameState = GAME_STATE_CHANGE_LOCATION;
    engine->_transitionMapId = pMapStats->GetMapInfo(pMapName);
    uLevel_StartingPointType = start_point;
}

//----- (0044C28F) --------------------------------------------------------
void TeleportToNWCDungeon() {
    // return if we are already in the NWC dungeon
    if (engine->_currentLoadedMapId == MAP_STRANGE_TEMPLE) {
        return;
    }

    // reset party teleport
    engine->_teleportPoint.invalidate();

    // start tranistion to dungeon
    pGameLoadingUI_ProgressBar->Initialize(GUIProgressBar::TYPE_Fullscreen);
    Transition_StopSound_Autosave("nwc.blv", MAP_START_POINT_PARTY);
    current_screen_type = SCREEN_GAME;
}
