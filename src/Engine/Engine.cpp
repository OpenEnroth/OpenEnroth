#include <cstring>
#include <memory>

#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/AssetsManager.h"

#include "Engine/Events/Processor.h"
#include "Engine/Events/RawEvent.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/LightsStack.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/TextureFrameTable.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/Graphics/PortalFunctions.h"
#include "Engine/Graphics/Polygon.h"
#include "Engine/Graphics/TurnBasedOverlay.h"
#include "Engine/LodTextureCache.h"
#include "Engine/LodSpriteCache.h"
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
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Tables/CharacterFrameTable.h"
#include "Engine/Tables/TileTable.h"
#include "Engine/Tables/FactionTable.h"
#include "Engine/Tables/StorylineTextTable.h"
#include "Engine/Tables/AwardTable.h"
#include "Engine/Tables/AutonoteTable.h"
#include "Engine/Tables/QuestTable.h"
#include "Engine/Tables/TransitionTable.h"
#include "Engine/Tables/MerchantTable.h"
#include "Engine/Tables/MessageScrollTable.h"
#include "Engine/Time/Timer.h"
#include "Engine/AttackList.h"
#include "Engine/GameResourceManager.h"
#include "Engine/MapInfo.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIProgressBar.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UIPopup.h"
#include "GUI/UI/UIMessageScroll.h"
#include "GUI/GUIMessageQueue.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/Audio/SoundList.h"
#include "Media/MediaPlayer.h"

#include "Io/Mouse.h"

#include "Library/Logger/Logger.h"
#include "Library/BuildInfo/BuildInfo.h"

#include "Utility/DataPath.h"

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
    engine->SetSaturateFaces(pParty->_497FC5_check_party_perception_against_level());

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
            // Water animation in vanilla was borked, or so it seems. Water has 7 frames, frame durations are:
            //
            // Frame    0       1       2       3       4       5       6       Total
            // Vanilla  1/12s   1/6s    1/6s    1/6s    1/6s    1/6s    1/12s   1s
            // OE       1/7s    1/7s    1/7s    1/7s    1/7s    1/7s    1/7s    1s
            render->hd_water_current_frame =
                std::floor(std::fmod(pMiscTimer->uTotalTimeElapsed.toFloatRealtimeSeconds(), 1.0f) * 7.0f);

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

void Engine::drawHUD() {
    // 2d from now on
    render->BeginScene2D();
    nuklear->Draw(nuklear->NUKLEAR_STAGE_PRE, WINDOW_GameUI, 1);
    if (nuklear->Mode(WINDOW_GameUI) == nuklear->NUKLEAR_MODE_EXCLUSIVE) {
        nuklear->Draw(nuklear->NUKLEAR_STAGE_POST, WINDOW_GameUI, 1);
    } else {
        DrawGUI();
        GUI_UpdateWindows();
        pParty->updateCharactersAndHirelingsEmotions();

        // if (v4)
    }

    // mouse->DrawPickedItem();
    mouse->DrawCursor();
    mouse->Activate();

    engine->nuklear->Draw(nuklear->NUKLEAR_STAGE_POST, WINDOW_GameUI, 1);
}

//----- (0044103C) --------------------------------------------------------
void Engine::Draw() {
    drawWorld();
    drawHUD();

    render->Present();
}


void Engine::DrawGUI() {
    render->ResetUIClipRect();

    // if (render->pRenderD3D)
    mouse->DrawCursorToTarget();
    GameUI_DrawRightPanelFrames();
    _statusBar->draw();

    if (!pMovie_Track && uGameState != GAME_STATE_CHANGE_LOCATION) {  // ! pVideoPlayer->pSmackerMovie)
        GameUI_DrawMinimap(488, 16, 625, 133, viewparams->uMinimapZoom, true);  // redraw = pParty->uFlags & 2);
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
            pPrimaryWindow->DrawText(assets->pFontArrus.get(), {494, 0}, colorTable.White, fmt::format("FPS: {: .4f}", framerate));
        }

        pPrimaryWindow->DrawText(assets->pFontArrus.get(), {300, 0}, colorTable.White, fmt::format("DrawCalls: {}", render->drawcalls));
        render->drawcalls = 0;


        int debug_info_offset = 16;
        pPrimaryWindow->DrawText(assets->pFontArrus.get(), {16, debug_info_offset}, colorTable.White,
                                 fmt::format("Party position:         {:.2f} {:.2f} {:.2f}", pParty->pos.x, pParty->pos.y, pParty->pos.z));
        debug_info_offset += 16;

        pPrimaryWindow->DrawText(assets->pFontArrus.get(), {16, debug_info_offset}, colorTable.White,
                                 fmt::format("Party yaw/pitch:     {} {}", pParty->_viewYaw, pParty->_viewPitch));
        debug_info_offset += 16;

        if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
            int sector_id = pBLVRenderParams->uPartySectorID;
            pPrimaryWindow->DrawText(assets->pFontArrus.get(), { 16, debug_info_offset }, colorTable.White,
                                     fmt::format("Party Sector ID:       {}/{}\n", sector_id, pIndoor->pSectors.size()));
            debug_info_offset += 16;
        }

        std::string floor_level_str;

        if (uGameState == GAME_STATE_CHANGE_LOCATION) {
            floor_level_str = "Loading Level!";
        } else if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
            int uFaceID;
            int sector_id = pBLVRenderParams->uPartySectorID;
            int floor_level = BLV_GetFloorLevel(pParty->pos.toInt()/* + Vec3i(0,0,40) */, sector_id, &uFaceID);
            floor_level_str = fmt::format("BLV_GetFloorLevel: {}   face_id {}\n", floor_level, uFaceID);
        } else if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
            bool on_water = false;
            int bmodel_pid;
            int floor_level = ODM_GetFloorLevel(pParty->pos.toInt(), 0, &on_water, &bmodel_pid, false);
            floor_level_str = fmt::format(
                "ODM_GetFloorLevel: {}   on_water: {}  on: {}\n",
                floor_level, on_water ? "true" : "false",
                bmodel_pid == 0
                    ? "---"
                    : fmt::format("BModel={} Face={}", bmodel_pid >> 6, bmodel_pid & 0x3F)
            );
        }

        pPrimaryWindow->DrawText(assets->pFontArrus.get(), {16, debug_info_offset}, colorTable.White, floor_level_str);
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

//----- (0044EC23) --------------------------------------------------------
int Engine::_44EC23_saturate_face_odm(Polygon *a2, int *a3, signed int a4) {
    double v4;  // st7@4
    // double v5; // ST00_8@4
    signed int v6;  // eax@5
    // double v7; // ST00_8@6
    signed int result;  // eax@8
    // double v9; // ST00_8@9
    // double v10; // ST00_8@11
    float a2a;  // [sp+14h] [bp+8h]@4
    float a3a;  // [sp+18h] [bp+Ch]@4
    float a3b;  // [sp+18h] [bp+Ch]@6
    float a4a;  // [sp+1Ch] [bp+10h]@9
    float a4b;  // [sp+1Ch] [bp+10h]@11

    if (engine->IsSaturateFaces() && a2->field_59 == 5 &&
        a2->pODMFace->uAttributes & FACE_IsSecret) {
        v4 = (double)a4;
        a2a = v4;
        *a3 |= 2u;
        a3a = (1.0 - this->fSaturation) * v4;
        // v5 = a3a + 6.7553994e15;
        // if ( SLODWORD(v5) >= 0 )
        if (floorf(a3a + 0.5f) >= 0) {
            a3b = (1.0 - this->fSaturation) * a2a;
            // v7 = a3b + 6.7553994e15;
            // v6 = LODWORD(v7);
            v6 = floorf(a3b + 0.5f);
        } else {
            v6 = 0;
        }
        if (a4 >= v6) {
            a4a = (1.0 - fSaturation) * a2a;
            // v9 = a4a + 6.7553994e15;
            // if ( SLODWORD(v9) >= 0 )
            if (floorf(a4a + 0.5f) >= 0) {
                a4b = (1.0 - fSaturation) * a2a;
                // v10 = a4b + 6.7553994e15;
                // result = LODWORD(v10);
                result = floorf(a4b + 0.5f);
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

    if (engine->IsSaturateFaces() && a2->uAttributes & FACE_IsSecret) {
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
Engine::Engine(std::shared_ptr<GameConfig> config) {
    this->config = config;
    this->bloodsplat_container = EngineIocContainer::ResolveBloodsplatContainer();
    this->decal_builder = EngineIocContainer::ResolveDecalBuilder();
    this->spell_fx_renedrer = EngineIocContainer::ResolveSpellFxRenderer();
    this->mouse = EngineIocContainer::ResolveMouse();
    this->nuklear = EngineIocContainer::ResolveNuklear();
    this->particle_engine = EngineIocContainer::ResolveParticleEngine();
    this->vis = EngineIocContainer::ResolveVis();

    uNumStationaryLights_in_pStationaryLightsStack = 0;

    pCamera3D = new Camera3D;
    pStru10Instance = new stru10;

    keyboardInputHandler = ::keyboardInputHandler;
    keyboardActionMapping = ::keyboardActionMapping;
}

//----- (0044E7F3) --------------------------------------------------------
Engine::~Engine() {
    if (mouse)
        mouse->Deactivate();

    delete pEventTimer;
    delete pStru10Instance;
    delete pCamera3D;
    pAudioPlayer.reset();
}

void Engine::LogEngineBuildInfo() {
    logger->info("OpenEnroth, revision {} built on {}", gitRevision(), buildTime());
    logger->info("Extra build information: {}/{}/{} {}", BUILD_PLATFORM, BUILD_ARCHITECTURE, BUILD_COMPILER, PROJECT_VERSION);
}

//----- (0044EA5E) --------------------------------------------------------
Vis_PIDAndDepth Engine::PickMouse(float fPickDepth, unsigned int uMouseX, unsigned int uMouseY,
                                  Vis_SelectionFilter *sprite_filter, Vis_SelectionFilter *face_filter) {
    if (uMouseX >= (signed int)pViewport->uScreen_TL_X &&
        uMouseX <= (signed int)pViewport->uScreen_BR_X &&
        uMouseY >= (signed int)pViewport->uScreen_TL_Y &&
        uMouseY <= (signed int)pViewport->uScreen_BR_Y) {
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
    Pointi pt = mouse->GetCursorPos();
    // TODO(captainurist): Right now we can have popups for monsters that are not reachable with a bow, and this is OK.
    //                     However, such monsters also don't get a hint displayed on mouseover. Probably should fix this?
    return PickMouse(pCamera3D->GetMouseInfoDepth(), pt.x, pt.y, &vis_allsprites_filter, &vis_face_filter);
}

Vis_PIDAndDepth Engine::PickMouseTarget() {
    Pointi pt = mouse->GetCursorPos();
    return PickMouse(config->gameplay.RangedAttackDepth.value(), pt.x, pt.y, &vis_sprite_targets_filter, &vis_face_filter);
}

Vis_PIDAndDepth Engine::PickMouseNormal() {
    Pointi pt = mouse->GetCursorPos();
    return PickMouse(config->gameplay.RangedAttackDepth.value(), pt.x, pt.y, &vis_items_filter, &vis_face_filter);
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
void PrepareWorld(unsigned int _0_box_loading_1_fullscreen) {
    Vis *vis = EngineIocContainer::ResolveVis();

    pEventTimer->Pause();
    pMiscTimer->Pause();
    CastSpellInfoHelpers::cancelSpellCastInProgress();
    engine->ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
    DoPrepareWorld(false, (_0_box_loading_1_fullscreen == 0) + 1);
    pMiscTimer->Resume();
    pEventTimer->Resume();
}

//----- (00464866) --------------------------------------------------------
void DoPrepareWorld(bool bLoading, int _1_fullscreen_loading_2_box) {
    // char *v3;         // eax@1
    MapId v5;  // eax@3

    // v9 = bLoading;
    engine->ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
    pGameLoadingUI_ProgressBar->Initialize(_1_fullscreen_loading_2_box == 1
                                               ? GUIProgressBar::TYPE_Fullscreen
                                               : GUIProgressBar::TYPE_Box);
    size_t pos = pCurrentMapName.rfind('.');
    std::string mapName = pCurrentMapName.substr(0, pos);
    std::string mapExt = pCurrentMapName.substr(pos + 1);  // This magically works even when pos == std::string::npos, in this case
                                                      // maxExt == pCurrentMapName.

    Level_LoadEvtAndStr(mapName);

    v5 = pMapStats->GetMapInfo(pCurrentMapName);

    uLevelMapStatsID = v5;

    // TODO(captainurist): need to zero this one out when loading a save, but is this a proper place to do that?
    attackList.clear();

    engine->SetUnderwater(Is_out15odm_underwater());

    pParty->floor_face_id = 0;
    if (iequals(mapExt, "blv"))
        PrepareToLoadBLV(bLoading);
    else
        PrepareToLoadODM(bLoading, 0);

    pNPCStats->setNPCNamesOnLoad();
    engine->_461103_load_level_sub();
    if ((pCurrentMapName == "d11.blv") ||
        (pCurrentMapName == "d10.blv")) {
        // spawning grounds & walls of mist - no loot & exp from monsters

        for (unsigned i = 0; i < pActors.size(); ++i) {
            // TODO(captainurist): shouldn't we also set uTreasureLevel = ITEM_TREASURE_LEVEL_INVALID?
            pActors[i].monsterInfo.treasureType = RANDOM_ITEM_ANY;
            pActors[i].monsterInfo.goldDiceRolls = 0;
            pActors[i].monsterInfo.exp = 0;
        }
    }
    bDialogueUI_InitializeActor_NPC_ID = 0;
    onMapLoad();
    pGameLoadingUI_ProgressBar->Progress();
    memset(&render->pBillboardRenderListD3D, 0,
           sizeof(render->pBillboardRenderListD3D));
    pGameLoadingUI_ProgressBar->Release();
}

//----- (004647AB) --------------------------------------------------------
void FinalInitialization() {
    pViewport->SetScreen(
        viewparams->uSomeX,
        viewparams->uSomeY,
        viewparams->uSomeZ,
        viewparams->uSomeW
    );
    pViewport->ResetScreen();

    InitializeTurnBasedAnimations(&stru_50C198);
    pBitmaps_LOD->reserveLoadedTextures();
    pSprites_LOD->reserveLoadedSprites();
    pIcons_LOD->reserveLoadedTextures();
}

void MM7_LoadLods() {
    engine->_gameResourceManager = std::make_unique<GameResourceManager>();
    engine->_gameResourceManager->openGameResources();

    pIcons_LOD = new LodTextureCache;
    pIcons_LOD->open(makeDataPath("data", "icons.lod"));

    pBitmaps_LOD = new LodTextureCache;
    pBitmaps_LOD->open(makeDataPath("data", "bitmaps.lod"));

    pSprites_LOD = new LodSpriteCache;
    pSprites_LOD->open(makeDataPath("data", "sprites.lod"));

    // TODO(captainurist):
    // on error in `open` we had this:
    // Error(localization->GetString(LSTR_PLEASE_REINSTALL), localization->GetString(LSTR_REINSTALL_NECESSARY));
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
    localization->Initialize();

    auto triLoad = [](const std::string &name) {
        TriBlob result;
        result.mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture(name) : Blob();
        result.mm7 = engine->_gameResourceManager->getEventsFile(name);
        return result;
    };

    pSpriteFrameTable = new SpriteFrameTable;
    deserialize(triLoad("dsft.bin"), pSpriteFrameTable);

    pTextureFrameTable = new TextureFrameTable;
    deserialize(triLoad("dtft.bin"), pTextureFrameTable);

    pTileTable = new TileTable;
    deserialize(triLoad("dtile.bin"), pTileTable);

    pPlayerFrameTable = new PlayerFrameTable;
    deserialize(triLoad("dpft.bin"), pPlayerFrameTable);

    pIconsFrameTable = new IconFrameTable;
    deserialize(triLoad("dift.bin"), pIconsFrameTable);

    pDecorationList = new DecorationList;
    deserialize(triLoad("ddeclist.bin"), pDecorationList);

    pObjectList = new ObjectList;
    deserialize(triLoad("dobjlist.bin"), pObjectList);

    pMonsterList = new MonsterList;
    deserialize(triLoad("dmonlist.bin"), pMonsterList);

    pChestList = new ChestDescList;
    deserialize(triLoad("dchest.bin"), pChestList);

    pOverlayList = new OverlayList;
    deserialize(triLoad("doverlay.bin"), pOverlayList);

    // TODO(captainurist): move to TableSnapshots.h/cpp
    Blob sounds_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("dsounds.bin") : Blob();
    Blob sounds_mm8;
    Blob sounds_mm7 = engine->_gameResourceManager->getEventsFile("dsounds.bin");

    pSoundList = new SoundList;
    pSoundList->FromFile(sounds_mm6, sounds_mm7, sounds_mm8);

    if (!config->debug.NoSound.value())
        pAudioPlayer->Initialize();

    pMediaPlayer = new MPlayer();
    pMediaPlayer->Initialize();

    dword_6BE364_game_settings_1 |= GAME_SETTINGS_4000;
}

//----- (00465D0B) --------------------------------------------------------
void Engine::SecondaryInitialization() {
    mouse->Initialize();

    pMapStats = new MapStats();
    pMapStats->Initialize(engine->_gameResourceManager->getEventsFile("MapStats.txt"));

    pMonsterStats = new MonsterStats();
    pMonsterStats->Initialize(engine->_gameResourceManager->getEventsFile("monsters.txt"));
    pMonsterStats->InitializePlacements(engine->_gameResourceManager->getEventsFile("placemon.txt"));

    pSpellStats = new SpellStats();
    pSpellStats->Initialize(engine->_gameResourceManager->getEventsFile("spells.txt"));

    pFactionTable = new FactionTable();
    pFactionTable->Initialize(engine->_gameResourceManager->getEventsFile("hostile.txt"));

    pStorylineText = new StorylineText();
    pStorylineText->Initialize(engine->_gameResourceManager->getEventsFile("history.txt"));

    pItemTable = new ItemTable();
    pItemTable->Initialize(engine->_gameResourceManager.get());

    initializeBuildings(engine->_gameResourceManager->getEventsFile("2dEvents.txt"));

    //pPaletteManager->SetMistColor(128, 128, 128);
    //pPaletteManager->RecalculateAll();
    pObjectList->InitializeSprites();
    pOverlayList->InitializeSprites();

    for (unsigned i = 0; i < 4; ++i) {
        static const char *pUIAnimNames[4] = {"glow03", "glow05", "torchA", "wizeyeA"};
        static unsigned short _4E98D0[4][4] = { {479, 0, 329, 0}, {585, 0, 332, 0}, {468, 0, 0, 0}, {606, 0, 0, 0} };

        // pUIAnims[i]->uIconID = pIconsFrameTable->FindIcon(pUIAnimNames[i]);
        pUIAnims[i]->icon = pIconsFrameTable->GetIcon(pUIAnimNames[i]);

        pUIAnims[i]->uAnimLength = 0_ticks;
        pUIAnims[i]->uAnimTime = 0;
        pUIAnims[i]->x = _4E98D0[i][0];
        pUIAnims[i]->y = _4E98D0[i][2];
    }

    UI_Create();

    spell_fx_renedrer->LoadAnimations();

    for (unsigned i = 0; i < 7; ++i) {
        std::string container_name = fmt::format("HDWTR{:03}", i);
        render->hd_water_tile_anim[i] = assets->getBitmap(container_name);
    }

    pNPCStats = new NPCStats();
    pNPCStats->Initialize(engine->_gameResourceManager.get());

    initializeQuests(engine->_gameResourceManager->getEventsFile("quests.txt"));
    initializeAutonotes(engine->_gameResourceManager->getEventsFile("autonote.txt"));
    initializeAwards(engine->_gameResourceManager->getEventsFile("awards.txt"));
    initializeTransitions(engine->_gameResourceManager->getEventsFile("trans.txt"));
    initializeMerchants(engine->_gameResourceManager->getEventsFile("merchant.txt"));
    initializeMessageScrolls(engine->_gameResourceManager->getEventsFile("scroll.txt"));

    engine->_globalEventMap = EventMap::load(engine->_gameResourceManager->getEventsFile("global.evt"));

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

    pEventTimer->Pause();

    GUIWindow::InitializeGUI();
}

//----- (00466082) --------------------------------------------------------
void MM6_Initialize() {
    viewparams = new ViewingParams;
    Sizei wsize = window->size();
    game_viewport_x = viewparams->uScreen_topL_X = engine->config->graphics.ViewPortX1.value(); //8
    game_viewport_y = viewparams->uScreen_topL_Y = engine->config->graphics.ViewPortY1.value(); //8
    game_viewport_z = viewparams->uScreen_BttmR_X = wsize.w - engine->config->graphics.ViewPortX2.value(); //468;
    game_viewport_w = viewparams->uScreen_BttmR_Y = wsize.h - engine->config->graphics.ViewPortY2.value(); //352;

    game_viewport_width = game_viewport_z - game_viewport_x;
    game_viewport_height = game_viewport_w - game_viewport_y;

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

    // pODMRenderParams->shading_dist_mist = 0x2000;//drawing dist 0x2000

    debug_non_combat_recovery_mul = 1.0f;
    debug_combat_recovery_mul = 1.0f;

    // this makes very little sense, but apparently this is how it was done in the original binary.
    debug_turn_based_monster_movespeed_mul = debug_non_combat_recovery_mul * 1.666666666666667f;

    flt_debugrecmod3 = 2.133333333333333f;

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
    } else {
        viewparams->field_20 &= 0xFFFFFF00;
    }

    viewparams->uSomeY = viewparams->uScreen_topL_Y;
    viewparams->uSomeX = viewparams->uScreen_topL_X;
    viewparams->uSomeZ = viewparams->uScreen_BttmR_X;
    viewparams->uSomeW = viewparams->uScreen_BttmR_Y;

    pViewport->SetScreen(viewparams->uScreen_topL_X, viewparams->uScreen_topL_Y,
                         viewparams->uScreen_BttmR_X,
                         viewparams->uScreen_BttmR_Y);
}

// TODO(pskelton): move to outdoor?
//----- (004610AA) --------------------------------------------------------
void PrepareToLoadODM(bool bLoading, ODMRenderParams *a2) {
    pGameLoadingUI_ProgressBar->Reset(27);
    uCurrentlyLoadedLevelType = LEVEL_OUTDOOR;

    ODM_LoadAndInitialize(pCurrentMapName, a2);
    if (!bLoading)
        TeleportToStartingPoint(uLevel_StartingPointType);

    viewparams->_443365();
    PlayLevelMusic();

    //  level decoration sound
    for (int decorIdx : decorationsWithSound) {
        const DecorationDesc *decoration = pDecorationList->GetDecoration(pLevelDecorations[decorIdx].uDecorationDescID);
        pAudioPlayer->playSound(decoration->uSoundID, SOUND_MODE_PID, Pid(OBJECT_Decoration, decorIdx));
    }
}

//----- (00464479) --------------------------------------------------------
void Engine::ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows() {
    if (mouse)
        mouse->SetCursorImage("MICON1");

    if (assets->winnerCert) {
        assets->winnerCert->Release();
        assets->winnerCert = nullptr;
    }

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
    int v4;          // edx@8
    int v6;   // esi@14
    int v8;   // ecx@16
    // int v12;         // esi@25
    // int v13;         // eax@26
    int16_t v14;     // ax@41
    int v17;  // [sp+14h] [bp-48h]@3
    // int v18;  // [sp+14h] [bp-48h]@23
    MapId v19;         // [sp+18h] [bp-44h]@1
    int v20;  // [sp+18h] [bp-44h]@14
    int v21[16] {};     // [sp+1Ch] [bp-40h]@17

    if (engine->config->debug.NoActors.value())
        pActors.clear();

    GenerateItemsInChest();
    UpdateChestPositions();
    pGameLoadingUI_ProgressBar->Progress();
    pParty->field_7B5_in_arena_quest = 0;
    pNPCStats->uNewlNPCBufPos = 0;
    v19 = pMapStats->GetMapInfo(pCurrentMapName);

    // v15 = 0;
    for (unsigned i = 0; i < pActors.size(); ++i) {
        // Actor *pActor = &pActors[i];
        // v2 = (char *)&pActors[0].uNPC_ID;
        // do
        //{
        // v3 = pActors[i].pMonsterInfo.uID;
        v17 = 0;
        if (isPeasant(pActors[i].monsterInfo.id))
            v17 = 1;
        // v1 = 0;
        v4 = (std::to_underlying(pActors[i].monsterInfo.id) - 1) % 3; // TODO(captainurist): encapsulate monster tier calculation.
        if (2 == v4) {
            if (pActors[i].npcId && pActors[i].npcId < 5000) continue;
        } else {
            if (v4 != 1) {
                if (v4 == 0 && pActors[i].npcId == 0) pActors[i].npcId = 0;
                continue;
            }
        }
        if (pActors[i].npcId > 0 && pActors[i].npcId < 5000) continue;
        if (v17) {
            pNPCStats->InitializeAdditionalNPCs(
                &pNPCStats->pAdditionalNPC[pNPCStats->uNewlNPCBufPos],
                pActors[i].monsterInfo.id, HOUSE_INVALID, v19);
            v14 = (unsigned short)pNPCStats->uNewlNPCBufPos + 5000;
            ++pNPCStats->uNewlNPCBufPos;
            pActors[i].npcId = v14;
            continue;
        }
        pActors[i].npcId = 0;
        // ++v15;
        // v2 += 836;
        //}
        // while ( v15 < (signed int)uNumActors );
    }

    pGameLoadingUI_ProgressBar->Progress();

    // v5 = uNumActors;
    v6 = 0;
    v20 = 0;
    // v16 = v1;

    // TODO(captainurist): can drop this code?
#if 0
    for (unsigned i = 0; i < pActors.size(); ++i) {
        // v7 = (char *)&pActors[0].pMonsterInfo;
        // do
        //{
        for (v8 = 0; v8 < v6; ++v8) {
            if (v21[v8] == pActors[i].monsterInfo.uID - 1) break;
        }

        if (v8 == v6) {
            v21[v6++] = pActors[i].monsterInfo.uID - 1;
            v20 = v6;
            if (v6 == 16) break;
        }
        // ++v16;
        // v7 += 836;
        //}
        // while ( v16 < (signed int)v5 );
    }
#endif

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
    if (pParty->pPickedItem.uItemID != ITEM_NULL)
        mouse->SetCursorBitmapFromItemID(pParty->pPickedItem.uItemID);
}

//----- (0042F3D6) --------------------------------------------------------
void InitializeTurnBasedAnimations(void *_this) {
    uIconID_TurnHour = pIconsFrameTable->FindIcon("turnhour");
    uIconID_CharacterFrame = pIconsFrameTable->FindIcon("aframe1");
    uSpriteID_Spell11 = pSpriteFrameTable->FastFindSprite("spell11");

    turnBasedOverlay.loadIcons();
}

//----- (0046BDA8) --------------------------------------------------------
unsigned int GetGravityStrength() {
    return engine->config->gameplay.Gravity.value();
}

void sub_44861E_set_texture_indoor(unsigned int uFaceCog,
                                   const std::string &filename) {
    for (unsigned i = 1; i < pIndoor->pFaceExtras.size(); ++i) {
        auto extra = &pIndoor->pFaceExtras[i];
        if (extra->sCogNumber == uFaceCog) {
            auto face = &pIndoor->pFaces[extra->face_id];
            face->SetTexture(filename);
        }
    }
}

void sub_44861E_set_texture_outdoor(unsigned int uFaceCog,
                                    const std::string &filename) {
    for (BSPModel &model : pOutdoor->pBModels) {
        for (ODMFace &face : model.pFaces) {
            if (face.sCogNumber == uFaceCog) {
                face.SetTexture(filename);
            }
        }
    }
}

void setTexture(unsigned int uFaceCog, const std::string &pFilename) {
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
            for (unsigned i = 1; i < (unsigned int)pIndoor->pFaceExtras.size(); ++i) {
                if (pIndoor->pFaceExtras[i].sCogNumber == sCogNumber) {
                    if (on)
                        pIndoor->pFaces[pIndoor->pFaceExtras[i].face_id]
                            .uAttributes |= bit;
                    else
                        pIndoor->pFaces[pIndoor->pFaceExtras[i].face_id]
                            .uAttributes &= ~bit;
                }
            }
        } else {
            for (BSPModel &model : pOutdoor->pBModels) {
                for (ODMFace &face : model.pFaces) {
                    if (face.sCogNumber == sCogNumber) {
                        if (on) {
                            face.uAttributes |= bit;
                        } else {
                            face.uAttributes &= ~bit;
                        }
                    }
                }
            }
        }
    }
}

void setDecorationSprite(uint16_t uCog, bool bHide, const std::string &pFileName) {
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

    if (current_screen_type == SCREEN_GAME && !pGUIWindow_CastTargetedSpell) {
        pEventTimer->Resume();
    }
}

//----- (00494035) --------------------------------------------------------
void _494035_timed_effects__water_walking_damage__etc() {
    int old_day = pParty->uCurrentDayOfMonth;
    int old_hour = pParty->uCurrentHour;
    int old_year = pParty->uCurrentYear;

    pParty->GetPlayingTime() += pEventTimer->uTimeElapsed;

    CivilTime time = pParty->GetPlayingTime().toCivilTime();
    pParty->uCurrentTimeSecond = time.second;
    pParty->uCurrentMinute = time.minute;
    pParty->uCurrentHour = time.hour;
    pParty->uCurrentMonthWeek = time.week - 1;
    pParty->uCurrentDayOfMonth = time.day - 1;
    pParty->uCurrentMonth = time.month - 1;
    pParty->uCurrentYear = time.year;

    // New day dawns
    // TODO(pskelton): ticks over at 3 in the morning?? check
    // TODO(pskelton): store GetDays() somewhere for a neater check here
    if ((pParty->uCurrentYear > old_year) || pParty->uCurrentHour >= 3 && (old_hour < 3 || pParty->uCurrentDayOfMonth > old_day)) {
        pParty->pHirelings[0].bHasUsedTheAbility = false;
        pParty->pHirelings[1].bHasUsedTheAbility = false;

        for (unsigned i = 0; i < pNPCStats->uNumNewNPCs; ++i)
            pNPCStats->pNewNPCData[i].bHasUsedTheAbility = false;

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
        if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) pOutdoor->SetFog();

        for (Character &character : pParty->pCharacters)
            character.uNumDivineInterventionCastsThisDay = 0;
    }

    // water damage
    if (pParty->uFlags & PARTY_FLAG_WATER_DAMAGE && pParty->_6FC_water_lava_timer < pParty->GetPlayingTime()) {
        pParty->_6FC_water_lava_timer = pParty->GetPlayingTime() + 128_ticks;
        for (Character &character : pParty->pCharacters) {
            if (character.WearsItem(ITEM_RELIC_HARECKS_LEATHER, ITEM_SLOT_ARMOUR) ||
                character.HasEnchantedItemEquipped(ITEM_ENCHANTMENT_OF_WATER_WALKING) ||
                character.pCharacterBuffs[CHARACTER_BUFF_WATER_WALK].Active()) {
                character.playEmotion(CHARACTER_EXPRESSION_SMILE, Duration::zero());
            } else {
                if (!character.hasUnderwaterSuitEquipped()) {
                    character.receiveDamage((int64_t)character.GetMaxHealth() * 0.1, DAMAGE_FIRE);
                    if (pParty->uFlags & PARTY_FLAG_WATER_DAMAGE) {
                        engine->_statusBar->setEventShort(LSTR_YOURE_DROWNING);
                    }
                } else {
                    character.playEmotion(CHARACTER_EXPRESSION_SMILE, Duration::zero());
                }
            }
        }
    }

    // lava damage
    if (pParty->uFlags & PARTY_FLAG_BURNING && pParty->_6FC_water_lava_timer < pParty->GetPlayingTime()) {
        pParty->_6FC_water_lava_timer = pParty->GetPlayingTime() + 128_ticks;

        for (Character &character : pParty->pCharacters) {
            character.receiveDamage((int64_t)character.GetMaxHealth() * 0.1, DAMAGE_FIRE);
            if (pParty->uFlags & PARTY_FLAG_BURNING) {
                engine->_statusBar->setEventShort(LSTR_ON_FIRE);
            }
        }
    }

    RegeneratePartyHealthMana();

    // TODO(captainurist): #time drop once we move to msecs in duration.
    Duration recoveryTimeDt = pEventTimer->uTimeElapsed;
    recoveryTimeDt += pParty->_roundingDt;
    pParty->_roundingDt = 0_ticks;
    if (pParty->uFlags2 & PARTY_FLAGS_2_RUNNING && recoveryTimeDt > Duration::zero()) {  // half recovery speed if party is running
        pParty->_roundingDt = recoveryTimeDt % 2_ticks;
        recoveryTimeDt /= 2;
    }

    unsigned numPlayersCouldAct = pParty->pCharacters.size();
    for (Character &character : pParty->pCharacters) {
        if (character.timeToRecovery && recoveryTimeDt > Duration::zero())
            character.Recover(recoveryTimeDt);

        if (character.GetItemsBonus(CHARACTER_ATTRIBUTE_ENDURANCE) +
            character.health + character.uEndurance >= 1 ||
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

        if (!pBuff->isGMBuff) {
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
        if (character.conditions.Has(CONDITION_SLEEP)) {
            if (character.conditions.HasNone({ CONDITION_PARALYZED, CONDITION_UNCONSCIOUS, CONDITION_DEAD, CONDITION_PETRIFIED, CONDITION_ERADICATED })) {
                character.conditions.Reset(CONDITION_SLEEP);
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
    constexpr int MINUTES_BETWEEN_REGEN = 5;
    int cur_minutes = pParty->GetPlayingTime().toMinutes();
    int last_minutes = pParty->last_regenerated.toMinutes();

    if (cur_minutes == last_minutes) {
        return;
    }

    bool tickover = last_minutes + MINUTES_BETWEEN_REGEN <= cur_minutes;

    if ((cur_minutes % MINUTES_BETWEEN_REGEN) == 0 || tickover) {
        // repeat for missed intervals
        while (last_minutes + MINUTES_BETWEEN_REGEN <= cur_minutes) {
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
                        pParty->pCharacters[caster].conditions.Set(CONDITION_CURSED, cursed_times);
                    }
                }
            }
#endif

            // Mana drain from flying
            // GM does not drain
            if (pParty->FlyActive() && !pParty->pPartyBuffs[PARTY_BUFF_FLY].isGMBuff) {
                if (pParty->bFlying) {
                    int caster = pParty->pPartyBuffs[PARTY_BUFF_FLY].caster - 1;
                    assert(caster >= 0);
                    if (pParty->pCharacters[caster].mana > 0 && !engine->config->debug.AllMagic.value()) {
                        pParty->pCharacters[caster].mana -= 1;
                    }
                }
            }

            // Mana drain from water walk
            // GM does not drain
            if (pParty->WaterWalkActive() && !pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].isGMBuff) {
                if (pParty->uFlags & PARTY_FLAG_STANDING_ON_WATER) {
                    int caster = pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].caster - 1;
                    int mana_drain = 1;
                    assert(caster >= 0);
                    // Vanilla bug: Water Walk drains mana with the same speed as Fly
                    if (engine->config->gameplay.FixWaterWalkManaDrain.value() && ((cur_minutes % 20) != 0)) {
                        mana_drain = 0;
                    }
                    if (pParty->pCharacters[caster].mana > 0 && !engine->config->debug.AllMagic.value()) {
                        pParty->pCharacters[caster].mana -= mana_drain;
                    }
                }
            }

            // immolation fire spell aura damage
            if (pParty->ImmolationActive()) {
                Vec3i cords;
                cords.x = 0;
                cords.y = 0;
                cords.z = 0;

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
                spellSprite.spell_caster_pid = Pid(OBJECT_Character, pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].caster);
                spellSprite.uFacing = 0;
                spellSprite.uSoundID = 0;

                int actorsAffectedByImmolation[100];
                size_t numberOfActorsAffected = pParty->immolationAffectedActors(actorsAffectedByImmolation, 100, 307);
                for (size_t idx = 0; idx < numberOfActorsAffected; ++idx) {
                    int actorID = actorsAffectedByImmolation[idx];
                    spellSprite.vPosition.x = pActors[actorID].pos.x;
                    spellSprite.vPosition.y = pActors[actorID].pos.y;
                    spellSprite.vPosition.z = pActors[actorID].pos.z;
                    spellSprite.spell_target_pid = Pid(OBJECT_Actor, actorID);
                    Actor::DamageMonsterFromParty(Pid(OBJECT_Item, spellSprite.Create(0, 0, 0, 0)), actorID, &cords);
                }
            }

            // HP/SP regeneration and HP deterioration
            for (Character &character : pParty->pCharacters) {
                for (ItemSlot idx : allItemSlots()) {
                    bool recovery_HP = false;
                    bool decrease_HP = false;
                    bool recovery_SP = false;
                    if (character.HasItemEquipped(idx)) {
                        unsigned _idx = character.pEquipment[idx];
                        ItemGen equppedItem = character.pInventoryItemList[_idx - 1];
                        if (!isRegular(equppedItem.uItemID)) {
                            if (equppedItem.uItemID == ITEM_RELIC_ETHRICS_STAFF) {
                                decrease_HP = true;
                            }
                            if (equppedItem.uItemID == ITEM_ARTIFACT_HERMES_SANDALS) {
                                recovery_HP = true;
                                recovery_SP = true;
                            }
                            if (equppedItem.uItemID == ITEM_ARTIFACT_MINDS_EYE) {
                                recovery_SP = true;
                            }
                            if (equppedItem.uItemID == ITEM_ARTIFACT_HEROS_BELT) {
                                recovery_HP = true;
                            }
                        } else {
                            ItemEnchantment special_enchantment = equppedItem.special_enchantment;
                            if (special_enchantment == ITEM_ENCHANTMENT_OF_REGENERATION
                                || special_enchantment == ITEM_ENCHANTMENT_OF_LIFE
                                || special_enchantment == ITEM_ENCHANTMENT_OF_PHOENIX
                                || special_enchantment == ITEM_ENCHANTMENT_OF_TROLL) {
                                recovery_HP = true;
                            }

                            if (special_enchantment == ITEM_ENCHANTMENT_OF_MANA
                                || special_enchantment == ITEM_ENCHANTMENT_OF_ECLIPSE
                                || special_enchantment == ITEM_ENCHANTMENT_OF_UNICORN) {
                                recovery_SP = true;
                            }

                            if (special_enchantment == ITEM_ENCHANTMENT_OF_PLENTY) {
                                recovery_HP = true;
                                recovery_SP = true;
                            }
                        }

                        if (recovery_HP && character.conditions.HasNone({ CONDITION_DEAD, CONDITION_ERADICATED })) {
                            if (character.health < character.GetMaxHealth()) {
                                character.health++;
                            }
                            if (character.conditions.Has(CONDITION_UNCONSCIOUS) && character.health > 0) {
                                character.conditions.Reset(CONDITION_UNCONSCIOUS);
                            }
                        }

                        if (recovery_SP && character.conditions.HasNone({ CONDITION_DEAD, CONDITION_ERADICATED })) {
                            if (character.mana < character.GetMaxMana()) {
                                character.mana++;
                            }
                        }

                        if (decrease_HP && character.conditions.HasNone({ CONDITION_DEAD, CONDITION_ERADICATED })) {
                            character.health--;
                            if (!(character.conditions.Has(CONDITION_UNCONSCIOUS)) && character.health < 0) {
                                character.conditions.Set(CONDITION_UNCONSCIOUS, pParty->GetPlayingTime());
                            }
                            if (character.health < 1) {
                                int enduranceCheck = character.health + character.uEndurance + character.GetItemsBonus(CHARACTER_ATTRIBUTE_ENDURANCE);
                                if (enduranceCheck >= 1 || character.pCharacterBuffs[CHARACTER_BUFF_PRESERVATION].Active()) {
                                    character.conditions.Set(CONDITION_UNCONSCIOUS, pParty->GetPlayingTime());
                                } else if (!character.conditions.Has(CONDITION_DEAD)) {
                                    character.conditions.Set(CONDITION_DEAD, pParty->GetPlayingTime());
                                }
                            }
                        }
                    }
                }

                // regeneration buff
                if (character.pCharacterBuffs[CHARACTER_BUFF_REGENERATION].Active() && character.conditions.HasNone({ CONDITION_DEAD, CONDITION_ERADICATED })) {
                    character.health += 5 * character.pCharacterBuffs[CHARACTER_BUFF_REGENERATION].power;
                    if (character.health > character.GetMaxHealth()) {
                        character.health = character.GetMaxHealth();
                    }
                    if (character.conditions.Has(CONDITION_UNCONSCIOUS) && character.health > 0) {
                        character.conditions.Reset(CONDITION_UNCONSCIOUS);
                    }
                }

                // for warlock
                if (PartyHasDragon() && character.classType == CLASS_WARLOCK) {
                    if (character.mana < character.GetMaxMana()) {
                        character.mana++;
                    }
                }

                // for lich
                if (character.classType == CLASS_LICH) {
                    bool lich_has_jar = false;
                    for (int idx = 0; idx < Character::INVENTORY_SLOT_COUNT; ++idx) {
                        if (character.pInventoryItemList[idx].uItemID == ITEM_QUEST_LICH_JAR_FULL)
                            lich_has_jar = true;
                    }

                    if (character.conditions.HasNone({ CONDITION_DEAD, CONDITION_ERADICATED })) {
                        if (character.health > (character.GetMaxHealth() / 2)) {
                            character.health = character.health - 2;
                        }
                        if (character.mana > (character.GetMaxMana() / 2)) {
                            character.mana = character.mana - 2;
                        }
                    }

                    if (lich_has_jar) {
                        if (character.mana < character.GetMaxMana()) {
                            character.mana++;
                        }
                    }
                }

                // for zombie
                if (character.conditions.Has(CONDITION_ZOMBIE) &&
                    character.conditions.HasNone({ CONDITION_DEAD, CONDITION_ERADICATED })) {
                    if (character.health > (character.GetMaxHealth() / 2)) {
                        character.health = character.health--;
                    }
                    if (character.mana > 0) {
                        character.mana = character.mana--;
                    }
                }
            }
            last_minutes += MINUTES_BETWEEN_REGEN;
        }
        pParty->last_regenerated = Time::fromMinutes(last_minutes);
    }
}

Duration timeUntilDawn() {
    const Duration dawnHour = Duration::fromHours(5);
    Duration currentTimeInDay = Duration::fromHours(pParty->uCurrentHour) + Duration::fromMinutes(pParty->uCurrentMinute);

    if (currentTimeInDay < dawnHour) {
        return dawnHour - currentTimeInDay;
    }
    return Duration::fromDays(1) + dawnHour - currentTimeInDay;
}

void initLevelStrings(const Blob &blob) {
    engine->_levelStrings.clear();

    int offs = 0;
    while (offs < blob.size()) {
        const char *nextNullTerm = (const char*)memchr(&blob.string_view()[offs], '\0', blob.size() - offs);
        size_t stringSize = nextNullTerm ? (nextNullTerm - &blob.string_view()[offs]) : (blob.size() - offs);
        engine->_levelStrings.push_back(trimRemoveQuotes(std::string(&blob.string_view()[offs], stringSize)));
        offs += stringSize + 1;
    }
}

void Level_LoadEvtAndStr(const std::string &pLevelName) {
    initLevelStrings(engine->_gameResourceManager->getEventsFile(pLevelName + ".str"));

    engine->_localEventMap = EventMap::load(engine->_gameResourceManager->getEventsFile(pLevelName + ".evt"));
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

void Transition_StopSound_Autosave(const std::string &pMapName,
                                   MapStartPoint start_point) {
    pAudioPlayer->stopSounds();

    // pGameLoadingUI_ProgressBar->Initialize(GUIProgressBar::TYPE_None);

    if (pCurrentMapName != pMapName) {
        SaveGame(1, 0);
    }

    uGameState = GAME_STATE_CHANGE_LOCATION;
    pCurrentMapName = pMapName;
    uLevel_StartingPointType = start_point;
}

//----- (0044C28F) --------------------------------------------------------
void TeleportToNWCDungeon() {
    // return if we are already in the NWC dungeon
    if ("nwc.blv" == pCurrentMapName) {
        return;
    }

    // reset party teleport
    engine->_teleportPoint.invalidate();

    // start tranistion to dungeon
    pGameLoadingUI_ProgressBar->Initialize(GUIProgressBar::TYPE_Fullscreen);
    Transition_StopSound_Autosave("nwc.blv", MAP_START_POINT_PARTY);
    current_screen_type = SCREEN_GAME;
}
