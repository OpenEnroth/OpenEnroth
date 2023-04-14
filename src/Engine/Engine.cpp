#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"

#include "Engine/Events.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/IRenderFactory.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/LightsStack.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/Graphics/PortalFunctions.h"
#include "Engine/Graphics/ClippingFunctions.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/MapsLongTimer.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/SaveLoad.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Tables/PlayerFrameTable.h"
#include "Engine/Time.h"
#include "Engine/AttackList.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIProgressBar.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIStatusBar.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

#include "Library/Random/Random.h"

using Graphics::IRenderFactory;

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

std::shared_ptr<Engine> engine;
GAME_STATE uGameState;

void Engine_DeinitializeAndTerminate(int exitCode) {
    engine->ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
    engine->Deinitialize();  // called twice?

    if (render)
        render->Release();

    exit(exitCode);
}

//----- (0044103C) --------------------------------------------------------
void Engine::Draw() {
    engine->SetSaturateFaces(pParty->_497FC5_check_party_perception_against_level());

    pCamera3D->_viewPitch = pParty->_viewPitch;
    pCamera3D->_viewYaw = pParty->_viewYaw;
    pCamera3D->vCameraPos.x = pParty->vPosition.x - pParty->_yawGranularity * cosf(2 * pi_double * pParty->_viewYaw / 2048.0);
    pCamera3D->vCameraPos.y = pParty->vPosition.y - pParty->_yawGranularity * sinf(2 * pi_double * pParty->_viewYaw / 2048.0);
    pCamera3D->vCameraPos.z = pParty->vPosition.z + pParty->sEyelevel;  // 193, but real 353

    // pIndoorCamera->Initialize2();
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
        if (pParty->vPosition.x != pParty->vPrevPosition.x ||
            pParty->_viewYaw != pParty->_viewPrevYaw ||
            pParty->vPosition.y != pParty->vPrevPosition.y ||
            pParty->_viewPitch != pParty->_viewPrevPitch ||
            pParty->vPosition.z != pParty->vPrevPosition.z ||
            pParty->sEyelevel != pParty->sPrevEyelevel)
            pParty->uFlags |= PARTY_FLAGS_1_ForceRedraw;

        pParty->vPrevPosition.x = pParty->vPosition.x;
        pParty->vPrevPosition.y = pParty->vPosition.y;
        pParty->vPrevPosition.z = pParty->vPosition.z;
        // v0 = &render;
        pParty->_viewPrevYaw = pParty->_viewYaw;
        pParty->_viewPrevPitch = pParty->_viewPitch;

        pParty->sPrevEyelevel = pParty->sEyelevel;
        render->BeginScene3D();

        // if ( !render->pRenderD3D )
        // pMouse->DrawCursorToTarget();
        if (!PauseGameDrawing()) {
            // if ( render->pRenderD3D )
            {
                float v2 =
                    (double)(((signed int)pMiscTimer->uTotalGameTimeElapsed >> 2) & 0x1F) * 0.032258064 * 6.0;
                // v3 = v2 + 6.7553994e15;
                // render->field_1036A8_bitmapid = LODWORD(v3);
                render->hd_water_current_frame = floorf(v2 + 0.5f);
            }

            if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
                pIndoor->Draw();
            else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
                pOutdoor->Draw();
            else
                Error("Invalid level type: %u", uCurrentlyLoadedLevelType);

             decal_builder->DrawBloodsplats();
        }
        render->DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();
    }

    // 2d from now on
    render->BeginScene2D();
    nuklear->Draw(nuklear->NUKLEAR_STAGE_PRE, WINDOW_GameUI, 1);
    if (nuklear->Mode(WINDOW_GameUI) == nuklear->NUKLEAR_MODE_EXCLUSIVE) {
        nuklear->Draw(nuklear->NUKLEAR_STAGE_POST, WINDOW_GameUI, 1);
    } else {
        DrawGUI();
        GUI_UpdateWindows();
        pParty->UpdatePlayersAndHirelingsEmotions();
        _unused_5B5924_is_travel_ui_drawn = false;

        // if (v4)
    }
    mouse->bRedraw = true;

    // mouse->DrawPickedItem();
    mouse->DrawCursor();
    mouse->Activate();

    engine->nuklear->Draw(nuklear->NUKLEAR_STAGE_POST, WINDOW_GameUI, 1);

    render->Present();
    pParty->uFlags &= ~PARTY_FLAGS_1_ForceRedraw;
}


void Engine::DrawGUI() {
    render->ResetUIClipRect();

    // if (render->pRenderD3D)
    mouse->DrawCursorToTarget();
    GameUI_StatusBar_DrawForced();
    GameUI_DrawRightPanelFrames();
    GameUI_StatusBar_Draw();


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
        pOtherOverlayList->DrawTurnBasedIcon();
        GameUI_DrawTorchlightAndWizardEye();
    }

    static bool render_framerate = false;
    static float framerate = 0.0f;
    static uint frames_this_second = 0;
    static uint last_frame_time = platform->tickCount();
    static uint framerate_time_elapsed = 0;

    if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME &&
        uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        pWeather->Draw();  // Ritor1: my include

                           // while(GetTickCount() - last_frame_time < 33 );//FPS control
    uint frame_dt = platform->tickCount() - last_frame_time;
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
            pPrimaryWindow->DrawText(pFontArrus, {494, 0}, colorTable.White.c16(), fmt::format("FPS: {: .4f}", framerate), 0, 0, 0);
        }

        pPrimaryWindow->DrawText(pFontArrus, {300, 0}, colorTable.White.c16(), fmt::format("DrawCalls: {}", render->drawcalls), 0, 0, 0);
        render->drawcalls = 0;


        int debug_info_offset = 0;
        pPrimaryWindow->DrawText(pFontArrus, {16, debug_info_offset + 16}, colorTable.White.c16(),
                                 fmt::format("Party position:         {} {} {}", pParty->vPosition.x, pParty->vPosition.y, pParty->vPosition.z), 0, 0, 0);

        if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
            debug_info_offset += 16;
            int sector_id = pBLVRenderParams->uPartySectorID;
            pPrimaryWindow->DrawText(pFontArrus, { 16, debug_info_offset + 16 }, colorTable.White.c16(),
                                     fmt::format("Party Sector ID:        {}/{}\n", sector_id, pIndoor->pSectors.size()), 0, 0, 0);
        }

        std::string floor_level_str;

        if (uGameState == GAME_STATE_CHANGE_LOCATION) {
            floor_level_str = "Loading Level!";
        } else if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
            uint uFaceID;
            int sector_id = pBLVRenderParams->uPartySectorID;
            int floor_level = BLV_GetFloorLevel(pParty->vPosition/* + Vec3i(0,0,40) */, sector_id, &uFaceID);
            floor_level_str = fmt::format("BLV_GetFloorLevel: {}   face_id {}\n", floor_level, uFaceID);
        } else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
            bool on_water = false;
            int bmodel_pid;
            int floor_level = ODM_GetFloorLevel(pParty->vPosition, 0, &on_water, &bmodel_pid, false);
            floor_level_str = fmt::format(
                "ODM_GetFloorLevel: {}   on_water: {}  on: {}\n",
                floor_level, on_water ? "true" : "false",
                bmodel_pid == 0
                    ? "---"
                    : fmt::format("BModel={} Face={}", bmodel_pid >> 6, bmodel_pid & 0x3F)
            );
        }

        pPrimaryWindow->DrawText(pFontArrus, {16, debug_info_offset + 16 + 16}, colorTable.White.c16(), floor_level_str, 0, 0, 0);
    }
}

//----- (0047A815) --------------------------------------------------------
void Engine::DrawParticles() {
    particle_engine->Draw();
}

//----- (0044F120) --------------------------------------------------------
void Engine::PushStationaryLights(int a2) {
    Game__StationaryLight *pLight;

    for (int i = 0; i < uNumStationaryLights; ++i) {
        pLight = &pStationaryLights[i];
        pStationaryLightsStack->AddLight(
            pLight->vPosition,
            pLight->flt_18, pLight->vRGBColor.x, pLight->vRGBColor.y,
            pLight->vRGBColor.z, _4E94D0_light_type);
    }
}

void Engine::StackPartyTorchLight() {
    int TorchLightDistance = engine->config->graphics.TorchlightDistance.value();
    // TODO(pskelton): set this on level load
    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) TorchLightDistance = 1024;
    if (TorchLightDistance > 0) {  // lightspot around party
        if (pParty->TorchlightActive()) {
            // max is 800 * torchlight
            // min is 800
            int MinTorch = TorchLightDistance;
            int MaxTorch = TorchLightDistance * pParty->pPartyBuffs[PARTY_BUFF_TORCHLIGHT].uPower;

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
        if (uCurrentlyLoadedLevelType == LEVEL_Outdoor && !pWeather->bNight)
            TorchLightDistance = 0;

        pParty->TorchLightLastIntensity = TorchLightDistance;

        // problem with deserializing this ??
        if (pParty->flt_TorchlightColorR == 0) {
            // __debugbreak();
            pParty->flt_TorchlightColorR = 96;
            pParty->flt_TorchlightColorG = 96;
            pParty->flt_TorchlightColorB = 96;

            logger->verbose("Torchlight doesn't have color");
        }

        // TODO: either add conversion functions, or keep only glm / only Vec3_* classes.
        Vec3f pos(pCamera3D->vCameraPos.x, pCamera3D->vCameraPos.y, pCamera3D->vCameraPos.z);

        pMobileLightsStack->AddLight(
            pos, pBLVRenderParams->uPartySectorID, TorchLightDistance,
            floorf(pParty->flt_TorchlightColorR + 0.5f),
            floorf(pParty->flt_TorchlightColorG + 0.5f),
            floorf(pParty->flt_TorchlightColorB + 0.5f), _4E94D0_light_type);
    }
}

//----- (0044EEA7) --------------------------------------------------------
bool Engine::_44EEA7() {  // cursor picking - particle update
    float depth;               // ST00_4@9
    // int64_t v6;                // kr00_8@21
    Vis_SelectionFilter *sprite_filter;  // [sp+10h] [bp-18h]@2
    Vis_SelectionFilter *face_filter;  // [sp+14h] [bp-14h]@2

    ++qword_5C6DF0;
    particle_engine->UpdateParticles();
    Pointi pt = mouse->GetCursorPos();

    // x = cursor.y;
    // y = cursor.x;
    if (sub_4637E0_is_there_popup_onscreen()) {
        face_filter = &vis_face_filter;
        sprite_filter = &vis_sprite_filter_2;
        depth = pCamera3D->GetMouseInfoDepth();
    } else {
        if (engine->IsTargetingMode()) {
            face_filter = &vis_face_filter;
            sprite_filter = &vis_sprite_filter_1;
        } else {
            face_filter = &vis_face_filter;
            sprite_filter = &vis_sprite_filter_4;
        }
        depth = config->gameplay.RangedAttackDepth.value();
    }
    // depth = v2;

    PickMouse(depth, pt.x, pt.y, false, sprite_filter, face_filter);
    //lightmap_builder->StationaryLightsCount = 0;
    //lightmap_builder->MobileLightsCount = 0;

    // decal reset but actually want bloodsplat reset
    // decal_builder->DecalsCount = 0;
    // decal_builder->curent_decal_id = 0;
    decal_builder->bloodsplat_container->uNumBloodsplats = 0;

    if (/*render->pRenderD3D &&*/ uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        render->uFogColor = GetLevelFogColor() & colorTable.White.c32();
    // if (uFlags & GAME_FLAGS_1_400)
    //    engine->config->SetForceRedraw(true);
    /*if ( !render->pRenderD3D && uCurrentlyLoadedLevelType == LEVEL_Outdoor &&
    pMobileLightsStack->uNumLightsActive )
    {
    uFlags2 |= 0x01;
    field_E10 = qword_5C6DF0;
    }*/
    // v6 = qword_5C6DF0 - field_E10;
    if (qword_5C6DF0/* - field_E10 == 1*/)
        engine->SetForceRedraw(true);

    if (uNumStationaryLights_in_pStationaryLightsStack != pStationaryLightsStack->uNumLightsActive) {
        engine->SetForceRedraw(true);
        uNumStationaryLights_in_pStationaryLightsStack = pStationaryLightsStack->uNumLightsActive;
    }
    return true;
}

//----- (0044EDE4) --------------------------------------------------------
bool Engine::AlterGamma_BLV(BLVFace *pFace, unsigned int *pColor) {
    if (engine->IsSaturateFaces() && pFace->uAttributes & FACE_IsSecret) {
        *pColor = ReplaceHSV(*pColor, 1.0, fSaturation, -1.0);
        return true;
    } else {
        return false;
    }
}

bool Engine::AlterGamma_ODM(ODMFace *pFace, unsigned int *pColor) {
    if (engine->IsSaturateFaces() && pFace->uAttributes & FACE_IsSecret) {
        *pColor = ReplaceHSV(*pColor, 1.0, fSaturation, -1.0);
        return true;
    } else {
        return false;
    }
}

//----- (004645FA) --------------------------------------------------------
void Engine::Deinitialize() {
    if (mouse)
        mouse->Deactivate();

    if (pItemTable)
        pItemTable->Release();

    if (pNPCStats)
        pNPCStats->Release();

    if (pNew_LOD)
        pNew_LOD->FreeSubIndexAndIO();

    delete pEventTimer;
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
    this->log = EngineIocContainer::ResolveLogger();
    this->bloodsplat_container = EngineIocContainer::ResolveBloodsplatContainer();
    this->decal_builder = EngineIocContainer::ResolveDecalBuilder();
    this->spell_fx_renedrer = EngineIocContainer::ResolveSpellFxRenderer();
    this->mouse = EngineIocContainer::ResolveMouse();
    this->nuklear = EngineIocContainer::ResolveNuklear();
    this->particle_engine = EngineIocContainer::ResolveParticleEngine();
    this->vis = EngineIocContainer::ResolveVis();

    uNumStationaryLights = 0;
    uNumStationaryLights_in_pStationaryLightsStack = 0;

    // pThreadWardInstance = nullptr;
    // pParticleEngine = new ParticleEngine;
    // pMouse = pMouseInstance = new Mouse;
    // pVisInstance = new Vis;
    // spellfx = new SpellFxRenderer;
    pCamera3D = new Camera3D;
    pStru9Instance = new stru9;
    pStru10Instance = new stru10;
    // pStru11Instance = new stru11;
    // pStru11Instance = nullptr;
    // pStru12Instance = new stru12(pStru11Instance);
    // pStru12Instance = nullptr;
    // pCShow = new CShow;
    // pCShow = nullptr;
    // pKeyboardInstance = new Keyboard;
    // pGammaController = new GammaController;

    keyboardInputHandler = ::keyboardInputHandler;
    keyboardActionMapping = ::keyboardActionMapping;
}

//----- (0044E7F3) --------------------------------------------------------
Engine::~Engine() {
    // delete pGammaController;
    // delete pKeyboardInstance;
    /*delete pCShow;
    delete pStru12Instance;
    delete pStru11Instance;*/
    delete pStru10Instance;
    delete pStru9Instance;
    delete pCamera3D;
    // delete spellfx;
    // delete pVisInstance;
    // delete pMouseInstance;
    // delete pParticleEngine;
    // delete pThreadWardInstance;
}

void Engine::LogEngineBuildInfo() {
    logger->info("OpenEnroth, compiled: {} {}", __DATE__, __TIME__);
    logger->info("Extra build information: {}/{}/{} {}", BUILD_PLATFORM, BUILD_TYPE, BUILD_COMPILER, PROJECT_VERSION);
}

//----- (0044EA5E) --------------------------------------------------------
bool Engine::PickMouse(float fPickDepth, unsigned int uMouseX,
                       unsigned int uMouseY, bool bOutline,
                       Vis_SelectionFilter *sprite_filter,
                       Vis_SelectionFilter *face_filter) {
    /*if (current_screen_type != SCREEN_GAME|| !render->pRenderD3D)
    return false;*/

    // if (!vis) {
    //     log->Warning(
    //         L"The 'Vis' object pointer has not been instatiated, but "
    //         L"CGame::Pick() is trying to call through it.");
    //     return false;
    // }

    if (uMouseX >= (signed int)pViewport->uScreen_TL_X &&
        uMouseX <= (signed int)pViewport->uScreen_BR_X &&
        uMouseY >= (signed int)pViewport->uScreen_TL_Y &&
        uMouseY <= (signed int)pViewport->uScreen_BR_Y) {
        vis->PickMouse(fPickDepth, uMouseX, uMouseY, sprite_filter, face_filter);

        if (bOutline)
            OutlineSelection();
    }

    return true;
}
// 4E28F8: using guessed type int current_screen_type;

//----- (0044EB12) --------------------------------------------------------
bool Engine::PickKeyboard(float pick_depth, bool bOutline, Vis_SelectionFilter *sprite_filter,
                          Vis_SelectionFilter *face_filter) {
    if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
        bool r = vis->PickKeyboard(pick_depth, &vis->default_list, sprite_filter, face_filter);

        if (bOutline)
            OutlineSelection();
        return r;
    }
    return false;
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

//----- (0044EB5A) --------------------------------------------------------
void Engine::OutlineSelection() {
    if (!vis->default_list.uSize)
        return;

    Vis_ObjectInfo *object_info = vis->default_list.object_pointers[0];
    if (object_info) {
        switch (object_info->object_type) {
            case VisObjectType_Sprite: {
                log->warning("Sprite outline currently unsupported");
                return;
            }

            case VisObjectType_Face: {
                if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
                    ODMFace *face = std::get<ODMFace *>(object_info->object);
                    if (face->uAttributes & FACE_OUTLINED)
                        face->uAttributes &= ~FACE_OUTLINED;
                    else
                        face->uAttributes |= FACE_OUTLINED;
                } else if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                    BLVFace *face = std::get<BLVFace *>(object_info->object);
                    if (face->uAttributes & FACE_OUTLINED)
                        face->uAttributes &= ~FACE_OUTLINED;
                    else
                        face->uAttributes |= FACE_OUTLINED;
                } else {
                    Error("Invalid level type", uCurrentlyLoadedLevelType);
                }
            } break;

            default: {
                Error(
                    "Undefined CObjectInfo type requested in "
                    "CGame::outline_selection()");
                Engine_DeinitializeAndTerminate(0);
            }
        }
    }
}



void PlayButtonClickSound() {
    pAudioPlayer->playNonResetableSound(SOUND_StartMainChoice02);
}

//----- (0046BDC0) --------------------------------------------------------
void UpdateUserInput_and_MapSpecificStuff() {
    if (dword_6BE364_game_settings_1 & GAME_SETTINGS_0080_SKIP_USER_INPUT_THIS_FRAME) {
        dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_0080_SKIP_USER_INPUT_THIS_FRAME;
        return;
    }

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        BLV_UpdateUserInputAndOther();
    else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        ODM_UpdateUserInputAndOther();

    evaluateAoeDamage();
}

//----- (004646F0) --------------------------------------------------------
void PrepareWorld(unsigned int _0_box_loading_1_fullscreen) {
    Vis *vis = EngineIocContainer::ResolveVis();
    vis->_4C1A02();

    pEventTimer->Pause();
    pMiscTimer->Pause();
    pParty->uFlags |= PARTY_FLAGS_1_ForceRedraw;
    CastSpellInfoHelpers::cancelSpellCastInProgress();
    engine->ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
    DoPrepareWorld(false, (_0_box_loading_1_fullscreen == 0) + 1);
    pMiscTimer->Resume();
    pEventTimer->Resume();
}

//----- (00464866) --------------------------------------------------------
void DoPrepareWorld(bool bLoading, int _1_fullscreen_loading_2_box) {
    // char *v3;         // eax@1
    unsigned int v5;  // eax@3

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
    LoadLevel_InitializeLevelEvt();

    v5 = pMapStats->GetMapInfo(pCurrentMapName);

    uLevelMapStatsID = v5;

    // TODO(captainurist): need to zero this one out when loading a save, but is this a proper place to do that?
    attackList.clear();

    engine->SetUnderwater(Is_out15odm_underwater());

    pParty->floor_face_pid = 0;
    if (iequals(mapExt, "blv"))
        PrepareToLoadBLV(bLoading);
    else
        PrepareToLoadODM(bLoading, 0);

    engine->_461103_load_level_sub();
    if ((pCurrentMapName == "d11.blv") ||
        (pCurrentMapName == "d10.blv")) {
        // spawning grounds & walls of mist - no loot & exp from monsters

        for (uint i = 0; i < pActors.size(); ++i) {
            pActors[i].pMonsterInfo.uTreasureType = 0;
            pActors[i].pMonsterInfo.uTreasureDiceRolls = 0;
            pActors[i].pMonsterInfo.uExp = 0;
        }
    }
    bDialogueUI_InitializeActor_NPC_ID = 0;
    OnMapLoad();
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
    pBitmaps_LOD->_inlined_sub1();
    pSprites_LOD->_inlined_sub1();
    pIcons_LOD->_inlined_sub1();
}

bool MM7_LoadLods() {
    pIcons_LOD = new LODFile_IconsBitmaps;
    if (!pIcons_LOD->Load(MakeDataPath("data", "icons.lod"), "icons")) {
        Error("Some files are missing\n\nPlease Reinstall.");
        return false;
    }
    pIcons_LOD->_011BA4_debug_paletted_pixels_uncompressed = false;

    pEvents_LOD = new LODFile_IconsBitmaps;
    if (!pEvents_LOD->Load(MakeDataPath("data", "events.lod"), "icons")) {
        Error("Some files are missing\n\nPlease Reinstall.");
        return false;
    }

    pBitmaps_LOD = new LODFile_IconsBitmaps;
    if (!pBitmaps_LOD->Load(MakeDataPath("data", "bitmaps.lod"), "bitmaps")) {
        Error(
            localization->GetString(LSTR_PLEASE_REINSTALL),
            localization->GetString(LSTR_REINSTALL_NECESSARY)
        );
        return false;
    }

    pSprites_LOD = new LODFile_Sprites;
    if (!pSprites_LOD->Load(MakeDataPath("data", "sprites.lod"), "sprites08")) {
        Error(
            localization->GetString(LSTR_PLEASE_REINSTALL),
            localization->GetString(LSTR_REINSTALL_NECESSARY)
        );
        return false;
    }

    return true;
}

//----- (004651F4) --------------------------------------------------------
bool Engine::MM7_Initialize() {
    grng->seed(platform->tickCount());
    vrng->seed(platform->tickCount());

    pEventTimer = Timer::Create();
    pEventTimer->Initialize();

    pParty = new Party();

    pParty->pHirelings.fill(NPCData());
    pParty->uDefaultEyelevel = pParty->sEyelevel = engine->config->gameplay.PartyEyeLevel.value();
    pParty->uDefaultPartyHeight = pParty->uPartyHeight = engine->config->gameplay.PartyHeight.value();
    pParty->uWalkSpeed = engine->config->gameplay.PartyWalkSpeed.value();

    MM6_Initialize();

    OnTimer(1);
    GameUI_StatusBar_Update(true);

    MM7_LoadLods();

    localization = new Localization();
    localization->Initialize();

    {
        Blob sft_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("dsft.bin") : Blob();
        Blob sft_mm8;
        Blob sft_mm7 = pEvents_LOD->LoadCompressedTexture("dsft.bin");
        pSpriteFrameTable = new SpriteFrameTable;
        pSpriteFrameTable->FromFile(sft_mm6, sft_mm7, sft_mm8);

        Blob tft_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("dtft.bin") : Blob();
        Blob tft_mm8;
        Blob tft_mm7 = pEvents_LOD->LoadCompressedTexture("dtft.bin");
        pTextureFrameTable = new TextureFrameTable;
        pTextureFrameTable->FromFile(tft_mm6, tft_mm7, tft_mm8);

        Blob tiles_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("dtile.bin") : Blob();
        Blob tiles_mm8;
        Blob tiles_mm7 = pEvents_LOD->LoadCompressedTexture("dtile.bin");
        pTileTable = new TileTable;
        pTileTable->FromFile(tiles_mm6, tiles_mm7, tiles_mm8);

        Blob pft_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("dpft.bin") : Blob();
        Blob pft_mm8;
        Blob pft_mm7 = pEvents_LOD->LoadCompressedTexture("dpft.bin");
        pPlayerFrameTable = new PlayerFrameTable;
        pPlayerFrameTable->FromFile(pft_mm6, pft_mm7, pft_mm8);

        Blob ift_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("dift.bin") : Blob();
        Blob ift_mm8;
        Blob ift_mm7 = pEvents_LOD->LoadCompressedTexture("dift.bin");
        pIconsFrameTable = new IconFrameTable;
        pIconsFrameTable->FromFile(ift_mm6, ift_mm7, ift_mm8);

        Blob decs_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("ddeclist.bin") : Blob();
        Blob decs_mm8;
        Blob decs_mm7 = pEvents_LOD->LoadCompressedTexture("ddeclist.bin");
        pDecorationList = new DecorationList;
        pDecorationList->FromFile(decs_mm6, decs_mm7, decs_mm8);

        Blob objs_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("dobjlist.bin") : Blob();
        Blob objs_mm8;
        Blob objs_mm7 = pEvents_LOD->LoadCompressedTexture("dobjlist.bin");
        pObjectList = new ObjectList;
        pObjectList->FromFile(objs_mm6, objs_mm7, objs_mm8);

        Blob mons_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("dmonlist.bin") : Blob();
        Blob mons_mm8;
        Blob mons_mm7 = pEvents_LOD->LoadCompressedTexture("dmonlist.bin");
        pMonsterList = new MonsterList;
        pMonsterList->FromFile(mons_mm6, mons_mm7, mons_mm8);

        Blob chests_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("dchest.bin") : Blob();
        Blob chests_mm8;
        Blob chests_mm7 = pEvents_LOD->LoadCompressedTexture("dchest.bin");
        pChestList = new ChestList;
        pChestList->FromFile(chests_mm6, chests_mm7, chests_mm8);

        Blob overlays_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("doverlay.bin") : Blob();
        Blob overlays_mm8;
        Blob overlays_mm7 = pEvents_LOD->LoadCompressedTexture("doverlay.bin");
        pOverlayList = new OverlayList;
        pOverlayList->FromFile(overlays_mm6, overlays_mm7, overlays_mm8);

        Blob sounds_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("dsounds.bin") : Blob();
        Blob sounds_mm8;
        Blob sounds_mm7 = pEvents_LOD->LoadCompressedTexture("dsounds.bin");
        pSoundList = new SoundList;
        pSoundList->FromFile(sounds_mm6, sounds_mm7, sounds_mm8);
    }

    if (!config->debug.NoSound.value())
        pAudioPlayer->Initialize();

    pMediaPlayer = new MPlayer();
    pMediaPlayer->Initialize();

    dword_6BE364_game_settings_1 |= GAME_SETTINGS_4000;

    return true;
}

//----- (00465D0B) --------------------------------------------------------
void Engine::SecondaryInitialization() {
    mouse->Initialize();

    pItemTable = new ItemTable;
    pItemTable->Initialize();

    // pBitmaps_LOD->can_load_hardware_sprites = 1;
    // pBitmaps_LOD->SetupPalettes(render->uTargetRBits, render->uTargetGBits,
    // render->uTargetBBits);
    pBitmaps_LOD->SetupPalettes(5, 6, 5);
    // pIcons_LOD->SetupPalettes(render->uTargetRBits, render->uTargetGBits,
    // render->uTargetBBits);
    pIcons_LOD->SetupPalettes(5, 6, 5);
    // pPaletteManager->SetColorChannelInfo(render->uTargetRBits,
    // render->uTargetGBits, render->uTargetBBits);
    //pPaletteManager->SetColorChannelInfo(5, 6, 5);

    //pPaletteManager->SetMistColor(128, 128, 128);
    //pPaletteManager->RecalculateAll();
    // pSprites_LOD->can_load_hardware_sprites = 1;
    pObjectList->InitializeSprites();
    pOverlayList->InitializeSprites();

    if (!engine->config->debug.NoSound.value())
        pSoundList->Initialize();

    for (uint i = 0; i < 4; ++i) {
        static const char *pUIAnimNames[4] = {"glow03", "glow05", "torchA", "wizeyeA"};
        static unsigned short _4E98D0[4][4] = { {479, 0, 329, 0}, {585, 0, 332, 0}, {468, 0, 0, 0}, {606, 0, 0, 0} };

        // pUIAnims[i]->uIconID = pIconsFrameTable->FindIcon(pUIAnimNames[i]);
        pUIAnims[i]->icon = pIconsFrameTable->GetIcon(pUIAnimNames[i]);
        pIconsFrameTable->InitializeAnimation(pUIAnims[i]->icon->id);

        pUIAnims[i]->uAnimLength = 0;
        pUIAnims[i]->uAnimTime = 0;
        pUIAnims[i]->x = _4E98D0[i][0];
        pUIAnims[i]->y = _4E98D0[i][2];
    }

    pObjectList->InitializeColors();

    UI_Create();

    spell_fx_renedrer->LoadAnimations();

    for (uint i = 0; i < 7; ++i) {
        char container_name[64];
        sprintf(container_name, "HDWTR%03u", i);
        render->pHDWaterBitmapIDs[i] =
            pBitmaps_LOD->LoadTexture(container_name);
        render->hd_water_tile_anim[i] = assets->GetBitmap(container_name);
    }

    pNPCStats = new NPCStats;
    pNPCStats->pNPCData.fill(NPCData());
    pNPCStats->Initialize();

    Initialize_GlobalEVT();
    pBitmaps_LOD->_inlined_sub0();
    pSprites_LOD->_inlined_sub0();

    Initialize_GamesLOD_NewLOD();
}

void Engine::Initialize() {
    if (!MM7_Initialize()) {
        log->warning("MM7_Initialize: failed");

        if (engine != nullptr) {
            engine->Deinitialize();
        }
        exit(-1);
    }

    pEventTimer->Pause();

    GUIWindow::InitializeGUI();
}

//----- (00466082) --------------------------------------------------------
void MM6_Initialize() {
    size_t v3;                       // ebx@32
    size_t v4;                       // edi@36
    char pDefaultGroundTexture[16];  // [sp+FCh] [bp-8Ch]@32

    viewparams = new ViewingParams;
    Sizei wsize = window->size();
    game_viewport_x = viewparams->uScreen_topL_X = engine->config->graphics.ViewPortX1.value(); //8
    game_viewport_y = viewparams->uScreen_topL_Y = engine->config->graphics.ViewPortY1.value(); //8
    game_viewport_z = viewparams->uScreen_BttmR_X = wsize.w - engine->config->graphics.ViewPortX2.value(); //468;
    game_viewport_w = viewparams->uScreen_BttmR_Y = wsize.h - engine->config->graphics.ViewPortY2.value(); //352;

    game_viewport_width = game_viewport_z - game_viewport_x;
    game_viewport_height = game_viewport_w - game_viewport_y;

    pAudioPlayer = new AudioPlayer;

    pODMRenderParams = new ODMRenderParams;
    pODMRenderParams->outdoor_no_mist = 0;
    pODMRenderParams->bNoSky = 0;
    pODMRenderParams->bDoNotRenderDecorations = 0;
    pODMRenderParams->outdoor_no_wavy_water = 0;
    // outdoor_grid_band_1 = GetPrivateProfileIntW(L"outdoor", L"gridband1", 10,
    // pIniFilename); outdoor_grid_band_2 = GetPrivateProfileIntW(L"outdoor",
    // L"gridband2", 15, pIniFilename); outdoor_grid_band_3 =
    // GetPrivateProfileIntW(L"outdoor", L"gridband3", 25, pIniFilename);
    pODMRenderParams->terrain_gamma = 0;
    pODMRenderParams->building_gamme = 0;
    pODMRenderParams->shading_dist_shade = 2048;
    pODMRenderParams->shading_dist_shademist = 4096;

    // pODMRenderParams->shading_dist_mist = 0x2000;//drawing dist 0x2000

    sprintf(pDefaultSkyTexture.data(), "%s", "plansky1");
    sprintf(pDefaultGroundTexture, "%s", "dirt");

    debug_non_combat_recovery_mul = 1.0f;
    debug_combat_recovery_mul = 1.0f;

    // this makes very little sense, but apparently this is how it was done in the original binary.
    debug_turn_based_monster_movespeed_mul = debug_non_combat_recovery_mul * 1.666666666666667f;

    flt_debugrecmod3 = 2.133333333333333f;

    v3 = 0;
    if (strlen(pDefaultSkyTexture.data())) {
        do {
            if (pDefaultSkyTexture[v3] == ' ') pDefaultSkyTexture[v3] = 0;
            ++v3;
        } while (v3 < strlen(pDefaultSkyTexture.data()));
    }
    v4 = 0;
    if (strlen(pDefaultGroundTexture)) {
        do {
            if (pDefaultGroundTexture[v4] == ' ') pDefaultGroundTexture[v4] = 0;
            ++v4;
        } while (v4 < strlen(pDefaultGroundTexture));
    }

    MM7Initialization();
}

//----- (004666D5) --------------------------------------------------------
void MM7Initialization() {
    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
        pODMRenderParams->shading_dist_shade = 2048;
        pODMRenderParams->terrain_gamma = 0;
        pODMRenderParams->building_gamme = 0;
        pODMRenderParams->shading_dist_shademist = 4096;
        pODMRenderParams->outdoor_no_wavy_water = 0;
        // _47F4D3_initialize_terrain_bezier_stuff(outdoor_grid_band_1,
        // outdoor_grid_band_2, outdoor_grid_band_3);
        {
            pODMRenderParams->outdoor_grid_band_3 = 25;  // outdoor_grid_band_3;
        }
    } else {
        viewparams->field_20 &= 0xFFFFFF00;
    }
    pParty->uFlags |= PARTY_FLAGS_1_ForceRedraw;
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
    uCurrentlyLoadedLevelType = LEVEL_Outdoor;

    ODM_LoadAndInitialize(pCurrentMapName, a2);
    if (!bLoading)
        TeleportToStartingPoint(uLevel_StartingPointType);

    viewparams->_443365();
    PlayLevelMusic();

    //  level decoration sound
    if (_6807E0_num_decorations_with_sounds_6807B8) {
        for (int i = 0; i < _6807E0_num_decorations_with_sounds_6807B8; i++) {
            int ind = _6807B8_level_decorations_ids[i];
            LevelDecoration dec = pLevelDecorations[ind];
            const DecorationDesc *decoration = pDecorationList->GetDecoration(dec.uDecorationDescID);
            pAudioPlayer->playSound(SoundID(decoration->uSoundID), PID(OBJECT_Decoration, ind), 0, 0, 0);
        }
    }
}

//----- (00464479) --------------------------------------------------------
void Engine::ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows() {
    if (mouse)
        mouse->SetCursorImage("MICON1");

    pPaletteManager->Reset();
    pBitmaps_LOD->ReleaseAll2();
    pSprites_LOD->DeleteSomeOtherSprites();
    pIcons_LOD->ReleaseAll2();

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        pIndoor->Release();
    else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        pOutdoor->Release();

    pAudioPlayer->stopSounds();
    uCurrentlyLoadedLevelType = LEVEL_null;
    pSpriteFrameTable->ResetLoadedFlags();
    pParty->armageddon_timer = 0;

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
    int v19;         // [sp+18h] [bp-44h]@1
    int v20;  // [sp+18h] [bp-44h]@14
    int v21[16] {};     // [sp+1Ch] [bp-40h]@17

    if (engine->config->debug.NoActors.value())
        pActors.clear();

    GenerateItemsInChest();
    pGameLoadingUI_ProgressBar->Progress();
    pParty->uFlags |= PARTY_FLAGS_1_ForceRedraw;
    pParty->field_7B5_in_arena_quest = 0;
    pNPCStats->uNewlNPCBufPos = 0;
    v19 = pMapStats->GetMapInfo(pCurrentMapName);

    // v15 = 0;
    for (uint i = 0; i < pActors.size(); ++i) {
        // Actor *pActor = &pActors[i];
        // v2 = (char *)&pActors[0].uNPC_ID;
        // do
        //{
        // v3 = pActors[i].pMonsterInfo.uID;
        v17 = 0;
        if (pActors[i].pMonsterInfo.uID >= 115 &&
                pActors[i].pMonsterInfo.uID <= 186 ||
            pActors[i].pMonsterInfo.uID >= 232 &&
                pActors[i].pMonsterInfo.uID <= 249)
            v17 = 1;
        // v1 = 0;
        v4 = (pActors[i].pMonsterInfo.uID - 1) % 3;
        if (2 == v4) {
            if (pActors[i].sNPC_ID && pActors[i].sNPC_ID < 5000) continue;
        } else {
            if (v4 != 1) {
                if (v4 == 0 && pActors[i].sNPC_ID == 0) pActors[i].sNPC_ID = 0;
                continue;
            }
        }
        if (pActors[i].sNPC_ID > 0 && pActors[i].sNPC_ID < 5000) continue;
        if (v17) {
            pNPCStats->InitializeAdditionalNPCs(
                &pNPCStats->pAdditionalNPC[pNPCStats->uNewlNPCBufPos],
                pActors[i].pMonsterInfo.uID, 0, v19);
            v14 = (unsigned short)pNPCStats->uNewlNPCBufPos + 5000;
            ++pNPCStats->uNewlNPCBufPos;
            pActors[i].sNPC_ID = v14;
            continue;
        }
        pActors[i].sNPC_ID = 0;
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

    for (uint i = 0; i < pActors.size(); ++i) {
        // v7 = (char *)&pActors[0].pMonsterInfo;
        // do
        //{
        for (v8 = 0; v8 < v6; ++v8) {
            if (v21[v8] == pActors[i].pMonsterInfo.uID - 1) break;
        }

        if (v8 == v6) {
            v21[v6++] = pActors[i].pMonsterInfo.uID - 1;
            v20 = v6;
            if (v6 == 16) break;
        }
        // ++v16;
        // v7 += 836;
        //}
        // while ( v16 < (signed int)v5 );
    }

    pGameLoadingUI_ProgressBar->Progress();

    if (engine->config->debug.NoActors.value())
        pActors.clear();
    if (engine->config->debug.NoDecorations.value())
        pLevelDecorations.clear();
    init_event_triggers();

    pGameLoadingUI_ProgressBar->Progress();

    pCamera3D->vCameraPos.x = 0;
    pCamera3D->vCameraPos.y = 0;
    pCamera3D->vCameraPos.z = 100;
    pCamera3D->_viewPitch = 0;
    pCamera3D->_viewYaw = 0;
    uLevel_StartingPointType = MapStartPoint_Party;
    pSprites_LOD->_461397();
    if (pParty->pPickedItem.uItemID != ITEM_NULL)
        mouse->SetCursorBitmapFromItemID(pParty->pPickedItem.uItemID);
}

//----- (0042F3D6) --------------------------------------------------------
void InitializeTurnBasedAnimations(void *_this) {
    for (unsigned int i = 0; i < pIconIDs_Turn.size(); ++i) {
        char icon_name[32];
        sprintf(icon_name, "turn%u", i);
        pIconIDs_Turn[i] = pIconsFrameTable->FindIcon(icon_name);
        pIconsFrameTable->InitializeAnimation(pIconIDs_Turn[i]);
    }

    uIconID_TurnStop = pIconsFrameTable->FindIcon("turnstop");
    uIconID_TurnHour = pIconsFrameTable->FindIcon("turnhour");
    uIconID_TurnStart = pIconsFrameTable->FindIcon("turnstart");
    uIconID_CharacterFrame = pIconsFrameTable->FindIcon("aframe1");
    uSpriteID_Spell11 = pSpriteFrameTable->FastFindSprite("spell11");

    pIconsFrameTable->InitializeAnimation(uIconID_TurnHour);
    pIconsFrameTable->InitializeAnimation(uIconID_TurnStop);
    pIconsFrameTable->InitializeAnimation(uIconID_TurnStart);
    pIconsFrameTable->InitializeAnimation(uIconID_CharacterFrame);
}

//----- (0046BDA8) --------------------------------------------------------
unsigned int GetGravityStrength() {
    return engine->config->gameplay.Gravity.value();
}

//----- (00448B45) --------------------------------------------------------
void GameUI_StatusBar_Update(bool force_hide) {
    if (force_hide ||
        game_ui_status_bar_event_string_time_left &&
            platform->tickCount() >= game_ui_status_bar_event_string_time_left && !pEventTimer->bPaused) {
        game_ui_status_bar_event_string_time_left = 0;
    }
}

void sub_44861E_set_texture_indoor(unsigned int uFaceCog,
                                   const std::string &filename) {
    for (uint i = 1; i < pIndoor->pFaceExtras.size(); ++i) {
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

//----- (0044861E) --------------------------------------------------------
void sub_44861E_set_texture(unsigned int uFaceCog, const char *pFilename) {
    if (uFaceCog) {
        // unsigned int texture = pBitmaps_LOD->LoadTexture(pFilename);
        // if (texture != -1)
        {
            // pBitmaps_LOD->pTextures[texture].palette_id2 =
            // pPaletteManager->LoadPalette(pBitmaps_LOD->pTextures[texture].palette_id1);

            if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                sub_44861E_set_texture_indoor(uFaceCog, pFilename);
            } else {
                sub_44861E_set_texture_outdoor(uFaceCog, pFilename);
            }

            pParty->uFlags |= PARTY_FLAGS_1_ForceRedraw;
        }
    }
}

//----- (0044892E) --------------------------------------------------------
void sub_44892E_set_faces_bit(int sCogNumber, FaceAttribute bit, int on) {
    if (sCogNumber) {
        if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
            for (uint i = 1; i < (unsigned int)pIndoor->pFaceExtras.size(); ++i) {
                if (pIndoor->pFaceExtras[i].sCogNumber == sCogNumber) {
                    if (on)
                        pIndoor->pFaces[pIndoor->pFaceExtras[i].face_id]
                            .uAttributes |= bit;
                    else
                        pIndoor->pFaces[pIndoor->pFaceExtras[i].face_id]
                            .uAttributes &= ~bit;
                }
            }
            pParty->uFlags |= PARTY_FLAGS_1_ForceRedraw;
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
        pParty->uFlags |= PARTY_FLAGS_1_ForceRedraw;
    }
}

//----- (0044882F) --------------------------------------------------------
void SetDecorationSprite(uint16_t uCog, bool bHide, const char *pFileName) {
    for (size_t i = 0; i < pLevelDecorations.size(); i++) {
        if (pLevelDecorations[i].uCog == uCog) {
            if (pFileName && strcmp(pFileName, "0")) {
                pLevelDecorations[i].uDecorationDescID = pDecorationList->GetDecorIdByName(pFileName);
                pDecorationList->InitializeDecorationSprite(pLevelDecorations[i].uDecorationDescID);
            }

            if (bHide)
                pLevelDecorations[i].uFlags &= ~LEVEL_DECORATION_INVISIBLE;
            else
                pLevelDecorations[i].uFlags |= LEVEL_DECORATION_INVISIBLE;

            pParty->uFlags |= PARTY_FLAGS_1_ForceRedraw;
        }
    }
}

//----- (004356FF) --------------------------------------------------------
void back_to_game() {
    dword_507BF0_is_there_popup_onscreen = 0;
    dword_4E455C = 1;

    extern int no_rightlick_in_inventory;
    no_rightlick_in_inventory = false;

    if (pGUIWindow_ScrollWindow) free_book_subwindow();
    if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME && !pGUIWindow_CastTargetedSpell)
        pEventTimer->Resume();
}

//----- (00494035) --------------------------------------------------------
void _494035_timed_effects__water_walking_damage__etc() {
    int old_day = pParty->uCurrentDayOfMonth;
    int old_hour = pParty->uCurrentHour;
    int old_year = pParty->uCurrentYear;

    pParty->GetPlayingTime().value += pEventTimer->uTimeElapsed;
    pParty->uCurrentTimeSecond = pParty->GetPlayingTime().GetSecondsFraction();
    pParty->uCurrentMinute = pParty->GetPlayingTime().GetMinutesFraction();
    pParty->uCurrentHour = pParty->GetPlayingTime().GetHoursOfDay();
    pParty->uCurrentMonthWeek = pParty->GetPlayingTime().GetDays() / 7 & 3;
    pParty->uCurrentDayOfMonth = pParty->GetPlayingTime().GetDays() % 28;
    pParty->uCurrentMonth = pParty->GetPlayingTime().GetMonthsOfYear();
    pParty->uCurrentYear = pParty->GetPlayingTime().GetYears() + game_starting_year;

    // New day dawns
    // TODO(pskelton): ticks over at 3 in the morning?? check
    // TODO(pskelton): store GetDays() somewhere for a neater check here
    if ((pParty->uCurrentYear > old_year) || pParty->uCurrentHour >= 3 && (old_hour < 3 || pParty->uCurrentDayOfMonth > old_day)) {
        pParty->pHirelings[0].bHasUsedTheAbility = false;
        pParty->pHirelings[1].bHasUsedTheAbility = false;

        for (uint i = 0; i < pNPCStats->uNumNewNPCs; ++i)
            pNPCStats->pNewNPCData[i].bHasUsedTheAbility = false;

        ++pParty->days_played_without_rest;
        if (pParty->days_played_without_rest > 1) {
            for (Player &player : pParty->pPlayers)
                player.SetCondWeakWithBlockCheck(0);

            // starving
            if (pParty->GetFood() > 0) {
                pParty->TakeFood(1);
            } else {
                for (Player &player : pParty->pPlayers) {
                    player.sHealth = player.sHealth / (pParty->days_played_without_rest + 1) + 1;
                }
            }

            // players go insane without rest
            if (pParty->days_played_without_rest > 3) {
                for (Player &player : pParty->pPlayers) {
                    // TODO(pskelton): rename Zero to ResetBonuses
                    player.Zero();
                    if (!player.IsPertified() && !player.IsEradicated() && !player.IsDead()) {
                        if (grng->random(100) < 5 * pParty->days_played_without_rest)
                            player.SetCondDeadWithBlockCheck(0);
                        if (grng->random(100) < 10 * pParty->days_played_without_rest)
                            player.SetCondInsaneWithBlockCheck(0);
                    }
                }
            }
        }
        if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) pOutdoor->SetFog();

        for (Player &player : pParty->pPlayers)
            player.uNumDivineInterventionCastsThisDay = 0;
    }

    // water damage
    if (pParty->uFlags & PARTY_FLAGS_1_WATER_DAMAGE &&
        pParty->_6FC_water_lava_timer < pParty->GetPlayingTime().value) {
        pParty->_6FC_water_lava_timer = pParty->GetPlayingTime().value + 128;
        for (Player &player : pParty->pPlayers) {
            if (player.WearsItem(ITEM_RELIC_HARECKS_LEATHER, ITEM_SLOT_ARMOUR) ||
                player.HasEnchantedItemEquipped(ITEM_ENCHANTMENT_OF_WATER_WALKING) ||
                player.pPlayerBuffs[PLAYER_BUFF_WATER_WALK].expire_time) {
                player.playEmotion(CHARACTER_EXPRESSION_SMILE, 0);
            } else {
                if (!player.hasUnderwaterSuitEquipped()) {
                    player.ReceiveDamage((int64_t)player.GetMaxHealth() * 0.1, DMGT_FIRE);
                    if (pParty->uFlags & PARTY_FLAGS_1_WATER_DAMAGE) {
                        GameUI_SetStatusBarShortNotification(localization->GetString(LSTR_YOURE_DROWNING));
                    }
                } else {
                    player.playEmotion(CHARACTER_EXPRESSION_SMILE, 0);
                }
            }
        }
    }

    // lava damage
    if (pParty->uFlags & PARTY_FLAGS_1_BURNING &&
        pParty->_6FC_water_lava_timer < pParty->GetPlayingTime().value) {
        pParty->_6FC_water_lava_timer = pParty->GetPlayingTime().value + 128;

        for (Player &player : pParty->pPlayers) {
            player.ReceiveDamage((int64_t)player.GetMaxHealth() * 0.1, DMGT_FIRE);
            if (pParty->uFlags & PARTY_FLAGS_1_BURNING) {
                GameUI_SetStatusBarShortNotification(localization->GetString(LSTR_ON_FIRE));
            }
        }
    }

    RegeneratePartyHealthMana();

    uint recoveryTimeDt = pEventTimer->uTimeElapsed;
    recoveryTimeDt += pParty->_roundingDt;
    pParty->_roundingDt = 0;
    if (pParty->uFlags2 & PARTY_FLAGS_2_RUNNING && recoveryTimeDt > 0) {  // half recovery speed if party is running
        pParty->_roundingDt = recoveryTimeDt % 2;
        recoveryTimeDt /= 2;
    }

    uint numPlayersCouldAct = pParty->pPlayers.size();
    for (Player &player : pParty->pPlayers) {
        if (player.uTimeToRecovery && recoveryTimeDt > 0)
            player.Recover(GameTime(recoveryTimeDt));

        if (player.GetItemsBonus(CHARACTER_ATTRIBUTE_ENDURANCE) +
            player.sHealth + player.uEndurance >= 1 ||
            player.pPlayerBuffs[PLAYER_BUFF_PRESERVATION].Active()) {
            if (player.sHealth < 1)
                player.SetCondition(Condition_Unconscious, 0);
        } else {
            player.SetCondition(Condition_Dead, 0);
        }

        if (player.field_E0) {
            int v24 = player.field_E0 - pEventTimer->uTimeElapsed;
            if (v24 > 0) {
                player.field_E0 = v24;
            } else {
                player.field_E0 = 0;
            }
        }
        if (player.field_E4) {
            int v26 = player.field_E4 - pEventTimer->uTimeElapsed;
            if (v26 > 0) {
                player.field_E4 = v26;
            } else {
                player.field_E4 = 0;
            }
        }
        if (player.field_E8) {
            int v28 = player.field_E8 - pEventTimer->uTimeElapsed;
            if (v28 > 0) {
                player.field_E8 = v28;
            } else {
                player.field_E8 = 0;
            }
        }
        if (player.field_EC) {
            int v30 = player.field_EC - pEventTimer->uTimeElapsed;
            if (v30 > 0) {
                player.field_EC = v30;
            } else {
                player.field_EC = 0;
            }
        }
        if (player.field_F0) {
            int v32 = player.field_F0 - pEventTimer->uTimeElapsed;
            if (v32 > 0) {
                player.field_F0 = v32;
            } else {
                player.field_F0 = 0;
            }
        }
        if (player.field_F4) {
            int v34 = player.field_F4 - pEventTimer->uTimeElapsed;
            if (v34 > 0) {
                player.field_F4 = v34;
            } else {
                player.field_F4 = 0;
            }
        }
        if (player.field_F8) {
            int v36 = player.field_F8 - pEventTimer->uTimeElapsed;
            if (v36 > 0) {
                player.field_F8 = v36;
            } else {
                player.field_F8 = 0;
            }
        }
        if (player.field_FC) {
            int v38 = player.field_FC - pEventTimer->uTimeElapsed;
            if (v38 > 0) {
                player.field_FC = v38;
            } else {
                player.field_FC = 0;
            }
        }
        if (player.field_100) {
            int v40 = player.field_100 - pEventTimer->uTimeElapsed;
            if (v40 > 0) {
                player.field_100 = v40;
            } else {
                player.field_100 = 0;
            }
        }
        if (player.field_104) {
            int v42 = player.field_104 - pEventTimer->uTimeElapsed;
            if (v42 > 0) {
                player.field_104 = v42;
            } else {
                player.field_104 = 0;
            }
        }

        if (!player.CanAct()) {
            --numPlayersCouldAct;
        }

        for (uint k = 0; k < 24; ++k) {
            player.pPlayerBuffs[k].IsBuffExpiredToTime(
                pParty->GetPlayingTime());
        }

        if (player.pPlayerBuffs[PLAYER_BUFF_HASTE].Expired()) {
            player.SetCondition(Condition_Weak, 0);
        }
    }

    for (uint i = 0; i < 20; ++i) {
        if (pParty->pPartyBuffs[i].IsBuffExpiredToTime(pParty->GetPlayingTime()) == 1) {
            /* Do nothing, check above has side effects. */
        }
    }

    if (pParty->pPartyBuffs[PARTY_BUFF_HASTE].Expired()) {
        for (Player &player : pParty->pPlayers)
            player.SetCondition(Condition_Weak, 0);
    }

    // Check if Fly/Water Walk caster can act
    for (PARTY_BUFF_INDEX buffIdx : {PARTY_BUFF_WATER_WALK, PARTY_BUFF_FLY}) {
        SpellBuff *pBuff = &pParty->pPartyBuffs[buffIdx];
        if (!pBuff->expire_time.Valid()) {
            continue;
        }

        if (!pBuff->isGMBuff) {
            if (!pPlayers[pBuff->uCaster]->CanAct()) {
                pBuff->Reset();
                if (buffIdx == PARTY_BUFF_FLY) {
                    pParty->bFlying = false;
                }
            }
        }
    }

    if (!numPlayersCouldAct) {
        if (current_screen_type != CURRENT_SCREEN::SCREEN_REST) {
            for (Player &player : pParty->pPlayers) {
                // if someone is sleeping - wake them up
                if (player.conditions.Has(Condition_Sleep)) {
                    player.conditions.Reset(Condition_Sleep);
                    numPlayersCouldAct = 1;
                    break;
                }
            }
            if (!numPlayersCouldAct || _5C35C0_force_party_death)
                uGameState = GAME_STATE_PARTY_DIED;
        }
    }

    if (pParty->hasActiveCharacter()) {
        if (current_screen_type != CURRENT_SCREEN::SCREEN_REST) {
            if (!pPlayers[pParty->getActiveCharacter()]->CanAct()) {
                pParty->switchToNextActiveCharacter();
            }
        }
    }
}

void RegeneratePartyHealthMana() {
    int cur_minutes = pParty->GetPlayingTime().GetMinutes();
    int last_minutes = pParty->last_regenerated.GetMinutes();

    if (cur_minutes == last_minutes) {
        return;
    }

    if ((cur_minutes % 5) == 0) {
        //int times_triggered = (current_time - last_reg_time) / 5;

        // TODO: actually this looks like it never triggers.
        // we get cursed_times, which is a time the player was cursed since the start of the game (a very large number),
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
                    GameTime cursed_times = pParty->pPlayers[caster].conditions.Get(Condition_Cursed);
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
            if (pParty->uFlags & PARTY_FLAGS_1_STANDING_ON_WATER) {
                if (!(pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uFlags & 1)) {  // taking on water
                    int caster = pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uCaster - 1;
                    GameTime cursed_times = pParty->pPlayers[caster].conditions.Get(Condition_Cursed);
                    cursed_times.value -= times_triggered;
                    if (cursed_times.value <= 0) {
                        cursed_times.value = 0;
                        pParty->uFlags &= ~PARTY_FLAGS_1_STANDING_ON_WATER;
                    }
                    pParty->pPlayers[caster].conditions.Set(Condition_Cursed, cursed_times);
                }
            }
        }
#endif

        // Mana drain from flying
        // GM does not drain
        if (pParty->FlyActive() && !pParty->pPartyBuffs[PARTY_BUFF_FLY].isGMBuff) {
            if (pParty->bFlying) {
                int caster = pParty->pPartyBuffs[PARTY_BUFF_FLY].uCaster - 1;
                assert(caster >= 0);
                if (pParty->pPlayers[caster].sMana > 0 && !engine->config->debug.AllMagic.value()) {
                    pParty->pPlayers[caster].sMana -= 1;
                }
            }
        }

        // Mana drain from water walk
        // GM does not drain
        if (pParty->WaterWalkActive() && !pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].isGMBuff) {
            if (pParty->uFlags & PARTY_FLAGS_1_STANDING_ON_WATER) {
                int caster = pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uCaster - 1;
                int mana_drain = 1;
                assert(caster >= 0);
                // Vanilla bug: Water Walk drains mana with the same speed as Fly
                if (engine->config->gameplay.FixWaterWalkManaDrain.value() && ((cur_minutes % 20) != 0)) {
                    mana_drain = 0;
                }
                if (pParty->pPlayers[caster].sMana > 0 && !engine->config->debug.AllMagic.value()) {
                    pParty->pPlayers[caster].sMana -= mana_drain;
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
            spellSprite.spell_level = pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].uPower;
            spellSprite.spell_skill = pParty->ImmolationSkillLevel();
            spellSprite.uType = SPRITE_SPELL_FIRE_IMMOLATION;
            spellSprite.uSpellID = SPELL_FIRE_IMMOLATION;
            spellSprite.uObjectDescID = pObjectList->ObjectIDByItemID(SpellSpriteMapping[SPELL_FIRE_IMMOLATION]);
            spellSprite.field_60_distance_related_prolly_lod = 0;
            spellSprite.uAttributes = 0;
            spellSprite.uSectorID = 0;
            spellSprite.uSpriteFrameID = 0;
            spellSprite.spell_caster_pid = PID(OBJECT_Player, pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].uCaster);
            spellSprite.uFacing = 0;
            spellSprite.uSoundID = 0;

            int actorsAffectedByImmolation[100];
            size_t numberOfActorsAffected = pParty->immolationAffectedActors(actorsAffectedByImmolation, 100, 307);
            for (size_t idx = 0; idx < numberOfActorsAffected; ++idx) {
                int actorID = actorsAffectedByImmolation[idx];
                spellSprite.vPosition.x = pActors[actorID].vPosition.x;
                spellSprite.vPosition.y = pActors[actorID].vPosition.y;
                spellSprite.vPosition.z = pActors[actorID].vPosition.z;
                spellSprite.spell_target_pid = PID(OBJECT_Actor, actorID);
                Actor::DamageMonsterFromParty(PID(OBJECT_Item, spellSprite.Create(0, 0, 0, 0)), actorID, &cords);
            }
        }

        // HP/SP regeneration and HP deterioration
        for (Player &player : pParty->pPlayers) {
            for (ITEM_SLOT idx : AllItemSlots()) {
                bool recovery_HP = false;
                bool decrease_HP = false;
                bool recovery_SP = false;
                if (player.HasItemEquipped(idx)) {
                    uint _idx = player.pEquipment.pIndices[idx];
                    ItemGen equppedItem = player.pInventoryItemList[_idx - 1];
                    if (!IsRegular(equppedItem.uItemID)) {
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
                        ITEM_ENCHANTMENT special_enchantment = equppedItem.special_enchantment;
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

                    if (recovery_HP && player.conditions.HasNone({Condition_Dead, Condition_Eradicated})) {
                        if (player.sHealth < player.GetMaxHealth()) {
                            player.sHealth++;
                        }
                        if (player.conditions.Has(Condition_Unconscious) && player.sHealth > 0) {
                            player.conditions.Reset(Condition_Unconscious);
                        }
                    }

                    if (recovery_SP && player.conditions.HasNone({Condition_Dead, Condition_Eradicated})) {
                        if (player.sMana < player.GetMaxMana()) {
                            player.sMana++;
                        }
                    }

                    if (decrease_HP && player.conditions.HasNone({Condition_Dead, Condition_Eradicated})) {
                        player.sHealth--;
                        if (!(player.conditions.Has(Condition_Unconscious)) && player.sHealth < 0) {
                            player.conditions.Set(Condition_Unconscious, pParty->GetPlayingTime());
                        }
                        if (player.sHealth < 1) {
                            int enduranceCheck = player.sHealth + player.uEndurance + player.GetItemsBonus(CHARACTER_ATTRIBUTE_ENDURANCE);
                            if (enduranceCheck >= 1 || player.pPlayerBuffs[PLAYER_BUFF_PRESERVATION].Active()) {
                                player.conditions.Set(Condition_Unconscious, pParty->GetPlayingTime());
                            } else if (!player.conditions.Has(Condition_Dead)) {
                                player.conditions.Set(Condition_Dead, pParty->GetPlayingTime());
                            }
                        }
                    }
                }
            }

            // regeneration buff
            if (player.pPlayerBuffs[PLAYER_BUFF_REGENERATION].Active() && player.conditions.HasNone({Condition_Dead, Condition_Eradicated})) {
                player.sHealth += 5 * player.pPlayerBuffs[PLAYER_BUFF_REGENERATION].uPower;
                if (player.sHealth > player.GetMaxHealth()) {
                    player.sHealth = player.GetMaxHealth();
                }
                if (player.conditions.Has(Condition_Unconscious) && player.sHealth > 0) {
                    player.conditions.Reset(Condition_Unconscious);
                }
            }

            // for warlock
            if (PartyHasDragon() && player.classType == PLAYER_CLASS_WARLOCK) {
                if (player.sMana < player.GetMaxMana()) {
                    player.sMana++;
                }
            }

            // for lich
            if (player.classType == PLAYER_CLASS_LICH) {
                bool lich_has_jar = false;
                for (int idx = 0; idx < Player::INVENTORY_SLOT_COUNT; ++idx) {
                    if (player.pInventoryItemList[idx].uItemID == ITEM_QUEST_LICH_JAR_FULL)
                        lich_has_jar = true;
                }

                if (player.conditions.HasNone({Condition_Dead, Condition_Eradicated})) {
                    if (player.sHealth > (player.GetMaxHealth() / 2)) {
                        player.sHealth = player.sHealth - 2;
                    }
                    if (player.sMana > (player.GetMaxMana() / 2)) {
                        player.sMana = player.sMana - 2;
                    }
                }

                if (lich_has_jar) {
                    if (player.sMana < player.GetMaxMana()) {
                        player.sMana++;
                    }
                }
            }

            // for zombie
            if (player.conditions.Has(Condition_Zombie) &&
                player.conditions.HasNone({Condition_Dead, Condition_Eradicated})) {
                if (player.sHealth > (player.GetMaxHealth() / 2)) {
                    player.sHealth = player.sHealth--;
                }
                if (player.sMana > 0) {
                    player.sMana = player.sMana--;
                }
            }
        }

        pParty->last_regenerated = pParty->GetPlayingTime();
    }
}

//----- (00494820) --------------------------------------------------------
unsigned int _494820_training_time(unsigned int a1) {
    signed int v1;  // eax@1

    v1 = 5;
    if (a1 % 24 >= 5) v1 = 29;
    return v1 - a1 % 24;
}

//----- (00443E31) --------------------------------------------------------
void LoadLevel_InitializeLevelStr() {
    //  char Args[100];
    int string_num;
    int max_string_length;
    //  int current_string_length;
    int prev_string_offset;

    if (sizeof(pLevelStrOffsets) != 2000)
        logger->warning("pLevelStrOffsets: deserialization warning");
    memset(pLevelStrOffsets.data(), 0, 2000);

    max_string_length = 0;
    string_num = 1;
    prev_string_offset = 0;
    pLevelStrOffsets[0] = 0;
    for (uint i = 0; i < uLevelStrFileSize; ++i) {
        if (!pLevelStr[i]) {
            pLevelStrOffsets[string_num] = i + 1;
            ++string_num;
            if (i - prev_string_offset > max_string_length)
                max_string_length = i - prev_string_offset;
            prev_string_offset = i;
        }
    }

    uLevelStrNumStrings = string_num - 1;
    if (max_string_length > 800)
        Error("MAX_EVENT_TEXT_LENGTH needs to be increased to %lu",
              max_string_length + 1);

    if (uLevelStrNumStrings > 0) {
        for (uint i = 0; i < uLevelStrNumStrings; ++i) {
            if (removeQuotes(&pLevelStr[pLevelStrOffsets[i]]) !=
                &pLevelStr[pLevelStrOffsets[i]])
                ++pLevelStrOffsets[i];
        }
    }
}

//----- (00443F95) --------------------------------------------------------
void OnMapLeave() {
    _evt_raw *test_event;
    if (uLevelEVT_NumEvents > 0) {
        for (uint i = 0; i < uLevelEVT_NumEvents; ++i) {
            test_event = (_evt_raw *)&pLevelEVT[pLevelEVT_Index[i].uEventOffsetInEVT];
            if (test_event->_e_type == EVENT_OnMapLeave) {
                EventProcessor(
                    pLevelEVT_Index[i].event_id,
                    0,
                    1,
                    pLevelEVT_Index[i].event_step
                );
            }
        }
    }
}

//----- (00443FDC) --------------------------------------------------------
void OnMapLoad() {
    int v6;                // eax@9
    int hours;             // ebx@26
    GameTime v18;          // [sp+Ch] [bp-44h]@12
    unsigned int seconds;  // [sp+14h] [bp-3Ch]@26
    GameTime v20;          // [sp+1Ch] [bp-34h]@7
    unsigned int minutes;  // [sp+2Ch] [bp-24h]@26
    unsigned int years;    // [sp+34h] [bp-1Ch]@26
    unsigned int weeks;    // [sp+38h] [bp-18h]@26
    unsigned int days;     // [sp+3Ch] [bp-14h]@26
    unsigned int months;   // [sp+40h] [bp-10h]@26

    // TODO(captainurist): if we don't set this one here, then some events might fire on map load in OnTimer()
    // because delta time is calculated there using this variable. Redo this properly.
    _5773B8_event_timer = pParty->GetPlayingTime();

    for (uint i = 0; i < uLevelEVT_NumEvents; ++i) {
        EventIndex pEvent = pLevelEVT_Index[i];

        _evt_raw *_evt = (_evt_raw *)(&pLevelEVT[pEvent.uEventOffsetInEVT]);

        //        if (_evt->_e_type == EVENT_PlaySound)
        //            pSoundList->LoadSound(EVT_DWORD(_evt->v5), 0);
        //        else
        if (_evt->_e_type == EVENT_OnMapReload) {
            EventProcessor(pEvent.event_id, 0, 0, pEvent.event_step);
        } else if (_evt->_e_type == EVENT_OnTimer ||
                 _evt->_e_type == EVENT_OnLongTimer) {
            // v3 = &MapsLongTimersList[MapsLongTimers_count];
            v20 = pOutdoor->loc_time.last_visit;
            if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
                v20 = pIndoor->stru1.last_visit;

            MapsLongTimersList[MapsLongTimers_count].timer_evt_type = _evt->_e_type;
            MapsLongTimersList[MapsLongTimers_count].timer_evt_ID = pEvent.event_id;
            MapsLongTimersList[MapsLongTimers_count].timer_evt_seq_num = pEvent.event_step;

            MapsLongTimersList[MapsLongTimers_count].YearsInterval = _evt->v5;
            MapsLongTimersList[MapsLongTimers_count].MonthsInterval = _evt->v6;
            MapsLongTimersList[MapsLongTimers_count].WeeksInterval = _evt->v7;
            MapsLongTimersList[MapsLongTimers_count].HoursInterval = _evt->v8;
            MapsLongTimersList[MapsLongTimers_count].MinutesInterval = _evt->v9;
            MapsLongTimersList[MapsLongTimers_count].SecondsInterval = _evt->v10;

            v6 = ((unsigned short)_evt->v12 << 8) + _evt->v11;

            MapsLongTimersList[MapsLongTimers_count].time_left_to_fire =
                ((unsigned short)_evt->v12 << 8) + _evt->v11;
            MapsLongTimersList[MapsLongTimers_count].IntervalHalfMins =
                ((unsigned short)_evt->v12 << 8) + _evt->v11;
            if (MapsLongTimersList[MapsLongTimers_count].timer_evt_type ==
                    EVENT_OnLongTimer &&
                !(short)v6) {
                if (v20)
                    v18 = pParty->GetPlayingTime() - v20;
                else
                    v18 = GameTime(0);

                if (v18.GetYears() != 0 &&
                        MapsLongTimersList[MapsLongTimers_count]
                            .YearsInterval ||
                    v18.GetMonths() != 0 &&
                        MapsLongTimersList[MapsLongTimers_count]
                                .MonthsInterval != 0 ||
                    v18.GetWeeks() != 0 &&
                        MapsLongTimersList[MapsLongTimers_count]
                                .WeeksInterval != 0 ||
                    v18.GetDays() != 0 || !v20) {
                    ++MapsLongTimers_count;
                    MapsLongTimersList[MapsLongTimers_count].NextStartTime = GameTime(0);
                    continue;
                }
            } else {
                seconds = pParty->GetPlayingTime().GetSecondsFraction();
                minutes = pParty->GetPlayingTime().GetMinutesFraction();
                hours = pParty->GetPlayingTime().GetHoursOfDay();
                days = pParty->GetPlayingTime().GetDaysOfWeek();
                weeks = pParty->GetPlayingTime().GetWeeksOfMonth();
                months = pParty->GetPlayingTime().GetMonthsOfYear();
                years = pParty->GetPlayingTime().GetYears();

                if (MapsLongTimersList[MapsLongTimers_count].YearsInterval) {
                    ++years;
                } else if (MapsLongTimersList[MapsLongTimers_count]
                    .MonthsInterval) {
                    ++months;
                } else if (MapsLongTimersList[MapsLongTimers_count].WeeksInterval) {
                    ++weeks;
                } else {
                    ++days;
                    hours =
                        MapsLongTimersList[MapsLongTimers_count].HoursInterval;
                    minutes = MapsLongTimersList[MapsLongTimers_count]
                                  .MinutesInterval;
                    seconds = MapsLongTimersList[MapsLongTimers_count]
                                  .SecondsInterval;
                }
                MapsLongTimersList[MapsLongTimers_count].NextStartTime =
                    GameTime(seconds, minutes, hours, days, weeks, months,
                             years);
                ++MapsLongTimers_count;
            }
        }
    }
}

void Level_LoadEvtAndStr(const std::string &pLevelName) {
    uLevelEVT_Size = LoadEventsToBuffer(pLevelName + ".evt", pLevelEVT.data(), 9216);
    uLevelStrFileSize = LoadEventsToBuffer(pLevelName + ".str", pLevelStr.data(), 9216);
    if (uLevelStrFileSize) LoadLevel_InitializeLevelStr();
}

void ReleaseBranchlessDialogue() {
    pGUIWindow2->Release();
    pGUIWindow2 = 0;
    activeLevelDecoration = _591094_decoration;
    EventProcessor(dword_5C3418, 0, 1, dword_5C341C);
    activeLevelDecoration = nullptr;
    pEventTimer->Resume();
}

bool _44100D_should_alter_right_panel() {
    return current_screen_type == CURRENT_SCREEN::SCREEN_NPC_DIALOGUE ||
           current_screen_type == CURRENT_SCREEN::SCREEN_CHARACTERS ||
           current_screen_type == CURRENT_SCREEN::SCREEN_HOUSE ||
           current_screen_type == CURRENT_SCREEN::SCREEN_SHOP_INVENTORY ||
           current_screen_type == CURRENT_SCREEN::SCREEN_CHANGE_LOCATION ||
           current_screen_type == CURRENT_SCREEN::SCREEN_INPUT_BLV ||
           current_screen_type == CURRENT_SCREEN::SCREEN_CASTING;
}

void Transition_StopSound_Autosave(const char *pMapName,
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

void OnTimer(int) {
    if (pEventTimer->bPaused) {
        return;
    }

    int64_t v13 = (pParty->GetPlayingTime() - _5773B8_event_timer).value / 128;
    if (!v13) return;

    _5773B8_event_timer = pParty->GetPlayingTime();

    for (uint i = 0; i < MapsLongTimers_count; ++i) {
        MapsLongTimer *timer = &MapsLongTimersList[i];
        if (timer->time_left_to_fire) {
            if (v13 < timer->time_left_to_fire) {
                timer->time_left_to_fire -= v13;
            } else {
                timer->time_left_to_fire = timer->IntervalHalfMins;
                EventProcessor(timer->timer_evt_ID, 0, 1,
                               timer->timer_evt_seq_num);
            }
        } else {
            if (timer->NextStartTime < pParty->GetPlayingTime()) {
                uint next_trigger_time = 1 * 60 * 60 * 24;  // 1 day
                if (timer->YearsInterval)
                    next_trigger_time = 336 * 60 * 60 * 24;  // 1 year
                else if (timer->MonthsInterval)
                    next_trigger_time = 28 * 60 * 60 * 24;  // 1 month
                else if (timer->WeeksInterval)
                    next_trigger_time = 7 * 60 * 60 * 24;  // 1 week

                timer->NextStartTime.value += (next_trigger_time * 128) / 3.0f;
                if (timer->NextStartTime <
                    pParty->GetPlayingTime())  // make sure in wont fire several
                                               // times in a row if big time
                                               // interval has lapsed
                    timer->NextStartTime = pParty->GetPlayingTime();

                EventProcessor(timer->timer_evt_ID, 0, 1,
                               timer->timer_evt_seq_num);
            }
        }
    }
}

//----- (0044C28F) --------------------------------------------------------
void TeleportToNWCDungeon() {
    // return if we are already in the NWC dungeon
    if ("nwc.blv" == pCurrentMapName) {
        return;
    }

    // reset party teleport
    Party_Teleport_X_Pos = 0;
    Party_Teleport_Y_Pos = 0;
    Party_Teleport_Z_Pos = 0;
    Party_Teleport_Cam_Yaw = 0;
    Party_Teleport_Cam_Pitch = 0;
    Party_Teleport_Z_Speed = 0;
    Start_Party_Teleport_Flag = 0;

    // start tranistion to dungeon
    pGameLoadingUI_ProgressBar->Initialize(GUIProgressBar::TYPE_Fullscreen);
    Transition_StopSound_Autosave("nwc.blv", MapStartPoint_Party);
    current_screen_type = CURRENT_SCREEN::SCREEN_GAME;
    return;
}
