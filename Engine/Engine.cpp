#include "Engine/Engine.h"

#ifdef _WINDOWS
#include <direct.h>
#include <io.h>
#endif

#include <algorithm>

#include "src/Application/Game.h"

#include "Arcomage/Arcomage.h"

#include "Engine/EngineConfig.h"
#include "Engine/Events.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/IRenderConfig.h"
#include "Engine/Graphics/IRenderFactory.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/Lights.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/Graphics/stru10.h"
#include "Engine/Graphics/stru9.h"
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
#include "Engine/stru123.h"
#include "Engine/Tables/FactionTable.h"
#include "Engine/Tables/FrameTableInc.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Tables/PlayerFrameTable.h"
#include "Engine/Tables/StorylineTextTable.h"
#include "Engine/Time.h"
#include "Engine/TurnEngine/TurnEngine.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIProgressBar.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UICredits.h"
#include "GUI/UI/UIDialogue.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIMainMenu.h"
#include "GUI/UI/UIPartyCreation.h"
#include "GUI/UI/UISaveLoad.h"
#include "GUI/UI/UIStatusBar.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

#include "Platform/Api.h"
#include "Platform/OSWindow.h"
#include "Platform/OSWindowFactory.h"

using EngineIoc = Engine_::IocContainer;

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
render->DrawTextureAlphaNew(64 / 640.0f, 48 / 480.0f, icon->texture);

icon = pIconsFrameTable->GetFrame(torchB.icon->id, GetTickCount() / 2);
render->DrawTextureAlphaNew((64 + torchA.icon->texture->GetWidth())/ 640.0f, 48
/ 480.0f, icon->texture);

icon = pIconsFrameTable->GetFrame(torchC.icon->id, GetTickCount() / 2);
render->DrawTextureAlphaNew((64 + torchA.icon->texture->GetWidth() +
torchB.icon->texture->GetWidth()) / 640.0f, 48 / 480.0f, icon->texture);

*/

static std::string s_data_path;

void SetDataPath(const std::string &data_path) { s_data_path = data_path; }

std::string MakeDataPath(const char *file_rel_path) {
    return s_data_path + OS_GetDirSeparator() + file_rel_path;
}

std::shared_ptr<Engine> engine;




uint32_t Color32(uint16_t color16) {
    uint32_t c = color16;
    uint32_t b = (c & 31) * 8;
    uint32_t g = ((c >> 5) & 63) * 4;
    uint32_t r = ((c >> 11) & 31) * 8;

    return Color32(r, g, b);
}

uint32_t Color32(uint32_t r, uint32_t g, uint32_t b, uint32_t a) {
    return (a << 24) | (r << 16) | (g << 8) | b;
}

uint16_t Color16(uint32_t r, uint32_t g, uint32_t b) {
    return (b >> (8 - 5)) | 0x7E0 & (g << (6 + 5 - 8)) |
        0xF800 & (r << (6 + 5 + 5 - 8));
}

bool FileExists(const char *fname) {
    return access(fname, 0) != -1;
}

void Engine_DeinitializeAndTerminate(int exitCode) {
    engine->ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
    engine->Deinitialize();  // called twice?
    if (render) render->Release();
    window = nullptr;
    exit(exitCode);
}

//----- (0044103C) --------------------------------------------------------
void Engine::Draw() {
    SetSaturateFaces(
        pParty->_497FC5_check_party_perception_against_level());

    pIndoorCameraD3D->sRotationX = pParty->sRotationX;
    pIndoorCameraD3D->sRotationZ = pParty->sRotationZ;
    pIndoorCameraD3D->vPartyPos.x = pParty->vPosition.x - pParty->y_rotation_granularity * cosf(2 * pi_double * pParty->sRotationZ / 2048.0);
    pIndoorCameraD3D->vPartyPos.y = pParty->vPosition.y - pParty->y_rotation_granularity * sinf(2 * pi_double * pParty->sRotationZ / 2048.0);
    pIndoorCameraD3D->vPartyPos.z = pParty->vPosition.z + pParty->sEyelevel;  // 193, but real 353

    // pIndoorCamera->Initialize2();
    pIndoorCameraD3D->CalculateRotations(pParty->sRotationX, pParty->sRotationZ);
    pIndoorCameraD3D->CreateWorldMatrixAndSomeStuff();
    pIndoorCameraD3D->BuildViewFrustum();

    if (pMovie_Track) {
        /*if ( !render->pRenderD3D )
        {
        render->BeginSceneD3D();
        pMouse->DrawCursorToTarget();
        render->DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();
        }*/
    } else {
        if (pParty->vPosition.x != pParty->vPrevPosition.x ||
            pParty->sRotationZ != pParty->sPrevRotationY ||
            pParty->vPosition.y != pParty->vPrevPosition.y ||
            pParty->sRotationX != pParty->sPrevRotationX ||
            pParty->vPosition.z != pParty->vPrevPosition.z ||
            pParty->sEyelevel != pParty->sPrevEyelevel)
            pParty->uFlags |= PARTY_FLAGS_1_0002;

        pParty->vPrevPosition.x = pParty->vPosition.x;
        pParty->vPrevPosition.y = pParty->vPosition.y;
        pParty->vPrevPosition.z = pParty->vPosition.z;
        // v0 = &render;
        pParty->sPrevRotationY = pParty->sRotationZ;
        pParty->sPrevRotationX = pParty->sRotationX;

        pParty->sPrevEyelevel = pParty->sEyelevel;
        render->BeginSceneD3D();

        // if ( !render->pRenderD3D )
        // pMouse->DrawCursorToTarget();
        if (!PauseGameDrawing() || viewparams->field_48 == 1) {
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
             lightmap_builder->DrawLightmapsType(2);  // mobile lights - sparks ??
        }
        render->DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();
    }

    // 2d from now on
    // DEBUG: force redraw gui
    viewparams->bRedrawGameUI = true;

    render->BeginScene();
    DrawGUI();
    int v4 = viewparams->bRedrawGameUI;
    GUI_UpdateWindows();
    pParty->UpdatePlayersAndHirelingsEmotions();
    _unused_5B5924_is_travel_ui_drawn = false;

    // if (v4)
        mouse->bRedraw = true;

    // mouse->DrawPickedItem();
    mouse->DrawCursor();
    mouse->Activate();
    render->EndScene();
    render->Present();
    pParty->uFlags &= ~PARTY_FLAGS_1_0002;
}

void Engine::DrawGUI() {
    render->ResetUIClipRect();

    // if (render->pRenderD3D)
    mouse->DrawCursorToTarget();
    if (pOtherOverlayList->bRedraw)
        viewparams->bRedrawGameUI = true;
    int v4 = viewparams->bRedrawGameUI;
    GameUI_StatusBar_DrawForced();
    if (!viewparams->bRedrawGameUI) {
        GameUI_DrawRightPanelItems();
    } else {
        GameUI_DrawRightPanelFrames();
        GameUI_StatusBar_Draw();
        viewparams->bRedrawGameUI = false;
    }


    if (!pMovie_Track) {  // ! pVideoPlayer->pSmackerMovie)
        GameUI_DrawMinimap(488, 16, 625, 133, viewparams->uMinimapZoom, true);  // redraw = pParty->uFlags & 2);
        if (v4) {
            if (!PauseGameDrawing() /*&& render->pRenderD3D*/) {
                // clear game viewport with transparent color
                render->InvalidateGameViewport();
                viewparams->field_48 = 0;
            }
        }
    }

    viewparams->bRedrawGameUI = pOtherOverlayList->bRedraw;
    pOtherOverlayList->bRedraw = 0;

    GameUI_DrawPartySpells();
    if (v4 || pParty->pHirelings[0].evt_C || pParty->pHirelings[1].evt_C)
        GameUI_DrawHiredNPCs();
    GameUI_DrawPortraits(v4);
    GameUI_DrawLifeManaBars();
    GameUI_DrawCharacterSelectionFrame();
    if (_44100D_should_alter_right_panel()) GameUI_DrawRightPanel();
    if (!pMovie_Track) {
        spell_fx_renedrer->DrawPlayerBuffAnims();
        pOtherOverlayList->DrawTurnBasedIcon(v4);
        GameUI_DrawTorchlightAndWizardEye();
    }

    static bool render_framerate = false;
    static float framerate = 0.0f;
    static uint frames_this_second = 0;
    static uint last_frame_time = OS_GetTime();
    static uint framerate_time_elapsed = 0;

    if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME &&
        uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        pWeather->Draw();  // Ritor1: my include

                           // while(GetTickCount() - last_frame_time < 33 );//FPS control
    uint frame_dt = OS_GetTime() - last_frame_time;
    last_frame_time = OS_GetTime();
    framerate_time_elapsed += frame_dt;
    if (framerate_time_elapsed >= 1000) {
        framerate = frames_this_second * (1000.0f / framerate_time_elapsed);

        framerate_time_elapsed = 0;
        frames_this_second = 0;
        render_framerate = true;
    }

    ++frames_this_second;

    if (engine->config->show_fps) {
        if (render_framerate) {
            pPrimaryWindow->DrawText(pFontArrus, 494, 0, Color16(0, 0, 0),
                StringPrintf("FPS: % .4f", framerate), 0,
                0, 0);
        }

        pPrimaryWindow->DrawText(pFontArrus, 300, 0, Color16(0, 0, 0),
            StringPrintf("DrawCalls: %d", render->drawcalls), 0, 0, 0);
        render->drawcalls = 0;


        int debug_info_offset = 0;
        if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
            int sector_id = pIndoor->GetSector(pParty->vPosition.x, pParty->vPosition.y, pParty->vPosition.z);
            pPrimaryWindow->DrawText(
                pFontArrus, 16, debug_info_offset = 16, Color16(255, 255, 255),
                StringPrintf("Party Sector ID:        %u/%u\n", sector_id,
                    pIndoor->uNumSectors),
                0, 0, 0);
        }
        pPrimaryWindow->DrawText(
            pFontArrus, 16, debug_info_offset + 16, Color16(255, 255, 255),
            StringPrintf("Party Position:         % d % d % d",
                pParty->vPosition.x, pParty->vPosition.y,
                pParty->vPosition.z),
            0, 0, 0);

        String floor_level_str;
        if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
            uint uFaceID;
            int sector_id = pIndoor->GetSector(
                pParty->vPosition.x, pParty->vPosition.y, pParty->vPosition.z);
            int floor_level = BLV_GetFloorLevel(
                pParty->vPosition.x, pParty->vPosition.y,
                pParty->vPosition.z/* + 40*/, sector_id, &uFaceID);
            floor_level_str = StringPrintf(
                "BLV_GetFloorLevel: %d   face_id %d\n", floor_level, uFaceID);
        } else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
            bool on_water = false;
            int _a6;
            int floor_level = ODM_GetFloorLevel(
                pParty->vPosition.x, pParty->vPosition.y, pParty->vPosition.z,
                0, &on_water, &_a6, false);
            floor_level_str = StringPrintf(
                "ODM_GetFloorLevel: %d   on_water: %s    a6 = %d\n",
                floor_level, on_water ? "true" : "false", _a6);
        }
        pPrimaryWindow->DrawText(pFontArrus, 16, debug_info_offset + 16 + 16,
            Color16(255, 255, 255), floor_level_str, 0, 0,
            0);
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
            pLight->vPosition.x, pLight->vPosition.y, pLight->vPosition.z,
            pLight->flt_18, pLight->vRGBColor.x, pLight->vRGBColor.y,
            pLight->vRGBColor.z, _4E94D0_light_type);
    }
}

//----- (0044EEA7) --------------------------------------------------------
bool Engine::_44EEA7() {  // cursor picking - particle update
    float depth;               // ST00_4@9
    // __int64 v6;                // kr00_8@21
    Vis_SelectionFilter *sprite_filter;  // [sp+10h] [bp-18h]@2
    Vis_SelectionFilter *face_filter;  // [sp+14h] [bp-14h]@2

    ++qword_5C6DF0;
    particle_engine->UpdateParticles();
    Point pt = mouse->GetCursorPos();

    // x = cursor.y;
    // y = cursor.x;
    if (sub_4637E0_is_there_popup_onscreen()) {
        face_filter = &vis_face_filter;
        sprite_filter = &vis_sprite_filter_2;
        depth = pIndoorCameraD3D->GetPickDepth();
    } else {
        if (config->IsTargetingMode()) {
            face_filter = &vis_face_filter;
            sprite_filter = &vis_sprite_filter_1;
        } else {
            face_filter = &vis_face_filter;
            sprite_filter = &vis_sprite_filter_4;
        }
        depth = 5120.0;
    }
    // depth = v2;

    PickMouse(depth, pt.x, pt.y, false, sprite_filter, face_filter);
    lightmap_builder->StationaryLightsCount = 0;
    lightmap_builder->MobileLightsCount = 0;
    decal_builder->DecalsCount = 0;
    decal_builder->curent_decal_id = 0;

    if (engine->config->DrawBlvDebugs())
        pStru10Instance->bDoNotDrawPortalFrustum = false;
    if (/*render->pRenderD3D &&*/ uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        render->uFogColor = GetLevelFogColor() & 0xFFFFFF;
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
    if (CanSaturateFaces() && pFace->uAttributes & FACE_CAN_SATURATE_COLOR) {
        *pColor = ReplaceHSV(*pColor, 1.0, fSaturation, -1.0);
        return true;
    } else {
        return false;
    }
}

bool Engine::AlterGamma_ODM(ODMFace *pFace, unsigned int *pColor) {
    if (engine->CanSaturateFaces() && pFace->uAttributes & FACE_CAN_SATURATE_COLOR) {
        *pColor = ReplaceHSV(*pColor, 1.0, fSaturation, -1.0);
        return true;
    } else {
        return false;
    }
}

//----- (004645FA) --------------------------------------------------------
void Engine::Deinitialize() {
    OS_SetAppInt("startinwindow", 1);  // render->bWindowMode);
    // if (render->bWindowMode)
    {
        OS_SetAppInt("window X", window->GetX());
        OS_SetAppInt("window Y", window->GetY());
    }
    OS_SetAppInt("valAlwaysRun", config->always_run ? 1 : 0);
    pItemsTable->Release();
    pNPCStats->Release();

    if (mouse)
        mouse->Deactivate();

    render = nullptr;

    pNew_LOD->FreeSubIndexAndIO();

    delete pEventTimer;
}

//----- (0044EE7C) --------------------------------------------------------
bool Engine::draw_debug_outlines() {
    if (/*uFlags & 0x04*/ engine->config->debug_lightmaps_decals) {
        lightmap_builder->DrawDebugOutlines(-1);
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

    if (CanSaturateFaces() && a2->field_59 == 5 &&
        a2->pODMFace->uAttributes & FACE_CAN_SATURATE_COLOR) {
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

    if (engine->CanSaturateFaces() && a2->uAttributes & FACE_CAN_SATURATE_COLOR) {
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
Engine::Engine() {
    this->log = EngineIoc::ResolveLogger();
    this->bloodsplat_container = EngineIoc::ResolveBloodsplatContainer();
    this->decal_builder = EngineIoc::ResolveDecalBuilder();
    this->spell_fx_renedrer = EngineIoc::ResolveSpellFxRenderer();
    this->lightmap_builder = EngineIoc::ResolveLightmapBuilder();
    this->mouse = EngineIoc::ResolveMouse();
    this->particle_engine = EngineIoc::ResolveParticleEngine();
    this->vis = EngineIoc::ResolveVis();

    uNumStationaryLights = 0;
    uNumStationaryLights_in_pStationaryLightsStack = 0;

    // pThreadWardInstance = nullptr;
    // pParticleEngine = new ParticleEngine;
    // pMouse = pMouseInstance = new Mouse;
    // pLightmapBuilder = new LightmapBuilder;
    // pVisInstance = new Vis;
    // spellfx = new SpellFxRenderer;
    pIndoorCameraD3D = new IndoorCameraD3D;
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
    delete pIndoorCameraD3D;
    // delete spellfx;
    // delete pVisInstance;
    // delete pLightmapBuilder;
    // delete pMouseInstance;
    // delete pParticleEngine;
    // delete pThreadWardInstance;
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
bool Engine::PickKeyboard(bool bOutline, Vis_SelectionFilter *sprite_filter,
                          Vis_SelectionFilter *face_filter) {
    if (current_screen_type == CURRENT_SCREEN::SCREEN_GAME) {
        bool r = vis->PickKeyboard(&vis->default_list, sprite_filter, face_filter);

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
    if (!vis->default_list.uNumPointers)
        return;

    Vis_ObjectInfo *object_info = vis->default_list.object_pointers[0];
    if (object_info) {
        switch (object_info->object_type) {
            case VisObjectType_Sprite: {
                log->Warning("Sprite outline currently unsupported");
                return;
            }

            case VisObjectType_Face: {
                if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
                    ODMFace *face = (ODMFace *)object_info->object;
                    if (face->uAttributes & FACE_OUTLINED)
                        face->uAttributes &= ~FACE_OUTLINED;
                    else
                        face->uAttributes |= FACE_OUTLINED;
                } else if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                    BLVFace *face = (BLVFace *)object_info->object;
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



//----- (0042FC15) --------------------------------------------------------
void CloseWindowBackground() {
    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, -2, 0, -1, 0, 0);
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

    area_of_effect__damage_evaluate();
}

//----- (004646F0) --------------------------------------------------------
void PrepareWorld(unsigned int _0_box_loading_1_fullscreen) {
    Vis *vis = EngineIoc::ResolveVis();
    vis->_4C1A02();

    pEventTimer->Pause();
    pMiscTimer->Pause();
    pParty->uFlags = PARTY_FLAGS_1_0002;
    CastSpellInfoHelpers::Cancel_Spell_Cast_In_Progress();
    engine->ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
    DoPrepareWorld(0, (_0_box_loading_1_fullscreen == 0) + 1);
    pMiscTimer->Resume();
    pEventTimer->Resume();
}

//----- (00464866) --------------------------------------------------------
void DoPrepareWorld(unsigned int bLoading, int _1_fullscreen_loading_2_box) {
    char *v3;         // eax@1
    unsigned int v5;  // eax@3

    // v9 = bLoading;
    engine->ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
    pGameLoadingUI_ProgressBar->Initialize(_1_fullscreen_loading_2_box == 1
                                               ? GUIProgressBar::TYPE_Fullscreen
                                               : GUIProgressBar::TYPE_Box);
    char Str1[20];    // [sp+Ch] [bp-18h]@1
    strcpy(Str1, pCurrentMapName.c_str());
    v3 = strtok(Str1, ".");
    strcpy(Str1, v3);
    Level_LoadEvtAndStr(Str1);
    LoadLevel_InitializeLevelEvt();
    strcpy(Str1, pCurrentMapName.c_str());
    _strrev(Str1);
    v3 = strtok(Str1, ".");
    _strrev(Str1);

    for (uint i = 0; i < 1000; ++i)
        pSpriteObjects[i].uObjectDescID = 0;

    v5 = pMapStats->GetMapInfo(pCurrentMapName);

    uLevelMapStatsID = v5;

    engine->SetUnderwater(Is_out15odm_underwater());

    pParty->floor_face_pid = 0;
    if (_stricmp(Str1, "blv"))
        PrepareToLoadODM(bLoading, 0);
    else
        PrepareToLoadBLV(bLoading);

    engine->_461103_load_level_sub();
    if ((pCurrentMapName == "d11.blv") ||
        (pCurrentMapName == "d10.blv")) {
        // spawning grounds & walls of mist - no loot & exp from monsters

        for (uint i = 0; i < uNumActors; ++i) {
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
    _flushall();
}

//----- (004647AB) --------------------------------------------------------
void FinalInitialization() {
    pViewport->SetScreen(viewparams->uSomeX, viewparams->uSomeY,
                         viewparams->uSomeZ, viewparams->uSomeW);
    pViewport->SetFOV(_6BE3A0_fov);

    InitializeTurnBasedAnimations(&stru_50C198);
    pBitmaps_LOD->_inlined_sub1();
    pSprites_LOD->_inlined_sub1();
    pIcons_LOD->_inlined_sub1();
}

bool MM7_LoadLods() {
    pIcons_LOD = new LODFile_IconsBitmaps;
    if (!pIcons_LOD->Load(MakeDataPath("data/icons.lod"), "icons")) {
        Error("Some files are missing\n\nPlease Reinstall.");
        return false;
    }
    pIcons_LOD->_011BA4_debug_paletted_pixels_uncompressed = false;

    pEvents_LOD = new LODFile_IconsBitmaps;
    if (!pEvents_LOD->Load(MakeDataPath("data/events.lod").c_str(), "icons")) {
        Error("Some files are missing\n\nPlease Reinstall.");
        return false;
    }

    pBitmaps_LOD = new LODFile_IconsBitmaps;
    if (!pBitmaps_LOD->Load(MakeDataPath("data/bitmaps.lod").c_str(), "bitmaps")) {
        Error(localization->GetString(63), localization->GetString(184));
        return false;
    }

    pSprites_LOD = new LODFile_Sprites;
    if (!pSprites_LOD->LoadSprites(MakeDataPath("data/sprites.lod"))) {
        Error(localization->GetString(63), localization->GetString(184));
        return false;
    }

    return true;
}

const int default_party_walk_speed = 384;
const int default_party_eye_level = 160;
const int default_party_height = 192;

//----- (004651F4) --------------------------------------------------------
bool Engine::MM7_Initialize() {
    srand(OS_GetTime());

    pEventTimer = Timer::Create();
    pEventTimer->Initialize();

    pParty = new Party();

    memset(&pParty->pHirelings, 0, sizeof(pParty->pHirelings));
    pParty->uWalkSpeed = default_party_walk_speed;
    pParty->uDefaultEyelevel = default_party_eye_level;
    pParty->sEyelevel = default_party_eye_level;
    pParty->uDefaultPartyHeight = default_party_height;
    pParty->uPartyHeight = default_party_height;

    MM6_Initialize();

    OnTimer(1);
    GameUI_StatusBar_Update(true);

    MM7_LoadLods();

    localization = new Localization();
    localization->Initialize();

    {
        void *sft_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("dsft.bin") : nullptr,
             *sft_mm8 = nullptr;
        void *sft_mm7 = pEvents_LOD->LoadCompressedTexture("dsft.bin");
        pSpriteFrameTable = new SpriteFrameTable;
        pSpriteFrameTable->FromFile(sft_mm6, sft_mm7, sft_mm8);
        free(sft_mm6);
        free(sft_mm7);
        free(sft_mm8);

        void *tft_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("dtft.bin") : nullptr,
             *tft_mm8 = nullptr;
        void *tft_mm7 = pEvents_LOD->LoadCompressedTexture("dtft.bin");
        pTextureFrameTable = new TextureFrameTable;
        pTextureFrameTable->FromFile(tft_mm6, tft_mm7, tft_mm8);
        free(tft_mm6);
        free(tft_mm7);
        free(tft_mm8);

        void *tiles_mm6 = pIcons_LOD_mm6
                              ? pIcons_LOD_mm6->LoadCompressedTexture("dtile.bin") : nullptr,
             *tiles_mm8 = nullptr;
        void *tiles_mm7 = pEvents_LOD->LoadCompressedTexture("dtile.bin");
        pTileTable = new TileTable;
        pTileTable->FromFile(tiles_mm6, tiles_mm7, tiles_mm8);
        free(tiles_mm6);
        free(tiles_mm7);
        free(tiles_mm8);

        void *pft_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("dpft.bin") : nullptr,
             *pft_mm8 = nullptr;
        void *pft_mm7 = pEvents_LOD->LoadCompressedTexture("dpft.bin");
        pPlayerFrameTable = new PlayerFrameTable;
        pPlayerFrameTable->FromFile(pft_mm6, pft_mm7, pft_mm8);
        free(pft_mm6);
        free(pft_mm7);
        free(pft_mm8);

        void *ift_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("dift.bin") : nullptr,
             *ift_mm8 = nullptr;
        void *ift_mm7 = pEvents_LOD->LoadCompressedTexture("dift.bin");
        pIconsFrameTable = new IconFrameTable;
        pIconsFrameTable->FromFile(ift_mm6, ift_mm7, ift_mm8);
        free(ift_mm6);
        free(ift_mm7);
        free(ift_mm8);

        void *decs_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("ddeclist.bin") : nullptr,
             *decs_mm8 = nullptr;
        void *decs_mm7 = pEvents_LOD->LoadCompressedTexture("ddeclist.bin");
        pDecorationList = new DecorationList;
        pDecorationList->FromFile(decs_mm6, decs_mm7, decs_mm8);
        free(decs_mm6);
        free(decs_mm7);
        free(decs_mm8);

        void *objs_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("dobjlist.bin") : nullptr,
             *objs_mm8 = nullptr;
        void *objs_mm7 = pEvents_LOD->LoadCompressedTexture("dobjlist.bin");
        pObjectList = new ObjectList;
        pObjectList->FromFile(objs_mm6, objs_mm7, objs_mm8);
        free(objs_mm6);
        free(objs_mm7);
        free(objs_mm8);

        void *mons_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("dmonlist.bin") : nullptr,
             *mons_mm8 = nullptr;
        void *mons_mm7 = pEvents_LOD->LoadCompressedTexture("dmonlist.bin");
        pMonsterList = new MonsterList;
        pMonsterList->FromFile(mons_mm6, mons_mm7, mons_mm8);
        free(mons_mm6);
        free(mons_mm7);
        free(mons_mm8);

        void *chests_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("dchest.bin") : nullptr,
             *chests_mm8 = nullptr;
        void *chests_mm7 = pEvents_LOD->LoadCompressedTexture("dchest.bin");
        pChestList = new ChestList;
        pChestList->FromFile(chests_mm6, chests_mm7, chests_mm8);
        free(chests_mm6);
        free(chests_mm7);
        free(chests_mm8);

        void *overlays_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("doverlay.bin") : nullptr,
             *overlays_mm8 = nullptr;
        void *overlays_mm7 = pEvents_LOD->LoadCompressedTexture("doverlay.bin");
        pOverlayList = new OverlayList;
        pOverlayList->FromFile(overlays_mm6, overlays_mm7, overlays_mm8);
        free(overlays_mm6);
        free(overlays_mm7);
        free(overlays_mm8);

        void *sounds_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadCompressedTexture("dsounds.bin") : nullptr,
             *sounds_mm8 = nullptr;
        void *sounds_mm7 = pEvents_LOD->LoadCompressedTexture("dsounds.bin");
        pSoundList = new SoundList;
        pSoundList->FromFile(sounds_mm6, sounds_mm7, sounds_mm8);
        free(sounds_mm6);
        free(sounds_mm7);
        free(sounds_mm8);
    }

    if (!config->NoSound())
        pAudioPlayer->Initialize();

    pMediaPlayer = new MPlayer();
    pMediaPlayer->Initialize();

    dword_6BE364_game_settings_1 |= GAME_SETTINGS_4000;

    return true;
}

//----- (00465D0B) --------------------------------------------------------
void Engine::SecondaryInitialization() {
    mouse->Initialize(window);

    pItemsTable = new ItemsTable;
    pItemsTable->Initialize();

    // pBitmaps_LOD->can_load_hardware_sprites = 1;
    // pBitmaps_LOD->SetupPalettes(render->uTargetRBits, render->uTargetGBits,
    // render->uTargetBBits);
    pBitmaps_LOD->SetupPalettes(5, 6, 5);
    // pIcons_LOD->SetupPalettes(render->uTargetRBits, render->uTargetGBits,
    // render->uTargetBBits);
    pIcons_LOD->SetupPalettes(5, 6, 5);
    // pPaletteManager->SetColorChannelInfo(render->uTargetRBits,
    // render->uTargetGBits, render->uTargetBBits);
    pPaletteManager->SetColorChannelInfo(5, 6, 5);

    pPaletteManager->SetMistColor(128, 128, 128);
    pPaletteManager->RecalculateAll();
    // pSprites_LOD->can_load_hardware_sprites = 1;
    pObjectList->InitializeSprites();
    pOverlayList->InitializeSprites();

    if (!engine->config->NoSound())
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
    memset(pNPCStats->pNPCData, 0, sizeof(pNPCStats->pNPCData));
    pNPCStats->Initialize();

    Initialize_GlobalEVT();
    pBitmaps_LOD->_inlined_sub0();
    pSprites_LOD->_inlined_sub0();
    pPaletteManager->LockAll();

    Initialize_GamesLOD_NewLOD();
}

void Engine::Initialize() {
    if (!MM7_Initialize()) {
        log->Warning("MM7_Initialize: failed");

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
    game_viewport_x = viewparams->uScreen_topL_X = 8;
    game_viewport_y = viewparams->uScreen_topL_Y = 8;
    game_viewport_z = viewparams->uScreen_BttmR_X = 468;
    game_viewport_w = viewparams->uScreen_BttmR_Y = 351;
    game_viewport_width = game_viewport_z - game_viewport_x;
    game_viewport_height = game_viewport_w - game_viewport_y + 1;

    pAudioPlayer = new AudioPlayer;

    sprintf(pStartingMapName, "%s", "out01.odm");

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

    flt_6BE3A4_debug_recmod1 = 1.0;
    flt_6BE3A8_debug_recmod2 = 1.0;
    flt_6BE3AC_debug_recmod1_x_1_6 = flt_6BE3A4_debug_recmod1 * 1.666666666666667;

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
            pODMRenderParams->uPickDepth =
                25 * 512;  // outdoor_grid_band_3 * 512;
        }
    } else {
        viewparams->field_20 &= 0xFFFFFF00;
    }
    pParty->uFlags |= PARTY_FLAGS_1_0002;
    viewparams->uSomeY = viewparams->uScreen_topL_Y;
    viewparams->uSomeX = viewparams->uScreen_topL_X;
    viewparams->uSomeZ = viewparams->uScreen_BttmR_X;
    viewparams->uSomeW = viewparams->uScreen_BttmR_Y;

    pViewport->SetScreen(viewparams->uScreen_topL_X, viewparams->uScreen_topL_Y,
                         viewparams->uScreen_BttmR_X,
                         viewparams->uScreen_BttmR_Y);
    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        pODMRenderParams->Initialize();
}

//----- (004610AA) --------------------------------------------------------
void PrepareToLoadODM(unsigned int bLoading, ODMRenderParams *a2) {
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
            DecorationDesc* decoration = pDecorationList->GetDecoration(dec.uDecorationDescID);
            pAudioPlayer->PlaySound(SoundID(decoration->uSoundID), PID(OBJECT_Decoration, ind), 0, 0, 0, 0);
        }
    }
}

//----- (00464479) --------------------------------------------------------
void Engine::ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows() {
    if (mouse)
        mouse->SetCursorImage("MICON1");

    pPaletteManager->ResetNonLocked();
    pBitmaps_LOD->ReleaseAll2();
    pSprites_LOD->DeleteSomeOtherSprites();
    pIcons_LOD->ReleaseAll2();

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        pIndoor->Release();
    else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        pOutdoor->Release();

    pAudioPlayer->StopChannels(-1, -1);
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
    __int16 v14;     // ax@41
    int v17;  // [sp+14h] [bp-48h]@3
    // int v18;  // [sp+14h] [bp-48h]@23
    int v19;         // [sp+18h] [bp-44h]@1
    int v20;  // [sp+18h] [bp-44h]@14
    int v21[16];     // [sp+1Ch] [bp-40h]@17

    if (engine->config->no_actors)
        uNumActors = 0;

    GenerateItemsInChest();
    pGameLoadingUI_ProgressBar->Progress();
    pParty->uFlags |= PARTY_FLAGS_1_0002;
    pParty->field_7B5_in_arena_quest = 0;
    dword_5C6DF8 = 1;
    pNPCStats->uNewlNPCBufPos = 0;
    v19 = pMapStats->GetMapInfo(pCurrentMapName);

    // v15 = 0;
    for (uint i = 0; i < uNumActors; ++i) {
        // Actor* pActor = &pActors[i];
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

    for (uint i = 0; i < uNumActors; ++i) {
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

    if (engine->config->NoActors())
        uNumActors = 0;
    if (engine->config->NoDecorations())
        uNumLevelDecorations = 0;
    init_event_triggers();

    pGameLoadingUI_ProgressBar->Progress();

    pIndoorCameraD3D->vPartyPos.x = 0;
    pIndoorCameraD3D->vPartyPos.y = 0;
    pIndoorCameraD3D->vPartyPos.z = 100;
    pIndoorCameraD3D->sRotationX = 0;
    pIndoorCameraD3D->sRotationZ = 0;
    viewparams->bRedrawGameUI = true;
    uLevel_StartingPointType = MapStartPoint_Party;
    pSprites_LOD->_461397();
    pPaletteManager->LockTestAll();
    if (pParty->pPickedItem.uItemID != 0)
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
    int v0 = ~(unsigned char)engine->config->flags2 & GAME_FLAGS_2_ALTER_GRAVITY;
    v0 |= 2;
    return (unsigned int)v0 >> 1;
}

//----- (00448B45) --------------------------------------------------------
void GameUI_StatusBar_Update(bool force_hide) {
    if (force_hide ||
        game_ui_status_bar_event_string_time_left &&
            OS_GetTime() >= game_ui_status_bar_event_string_time_left && !pEventTimer->bPaused) {
        game_ui_status_bar_event_string_time_left = 0;
    }
}

void sub_44861E_set_texture_indoor(unsigned int uFaceCog,
                                   const String &filename) {
    for (uint i = 1; i < pIndoor->uNumFaceExtras; ++i) {
        auto extra = &pIndoor->pFaceExtras[i];
        if (extra->sCogNumber == uFaceCog) {
            auto face = &pIndoor->pFaces[extra->face_id];
            face->SetTexture(filename);
        }
    }
}

void sub_44861E_set_texture_outdoor(unsigned int uFaceCog,
                                    const String &filename) {
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

            pParty->uFlags |= PARTY_FLAGS_1_0002;
        }
    }
}

//----- (0044892E) --------------------------------------------------------
void sub_44892E_set_faces_bit(int sCogNumber, int bit, int on) {
    if (sCogNumber) {
        if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
            for (uint i = 1; i < (unsigned int)pIndoor->uNumFaceExtras; ++i) {
                if (pIndoor->pFaceExtras[i].sCogNumber == sCogNumber) {
                    if (on)
                        pIndoor->pFaces[pIndoor->pFaceExtras[i].face_id]
                            .uAttributes |= bit;
                    else
                        pIndoor->pFaces[pIndoor->pFaceExtras[i].face_id]
                            .uAttributes &= ~bit;
                }
            }
            pParty->uFlags |= PARTY_FLAGS_1_0002;
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
        pParty->uFlags |= PARTY_FLAGS_1_0002;
    }
}

//----- (0044882F) --------------------------------------------------------
void SetDecorationSprite(uint16_t uCog, bool bHide, const char *pFileName) {
    for (size_t i = 0; i < uNumLevelDecorations; i++) {
        if (pLevelDecorations[i].uCog == uCog) {
            if (pFileName && strcmp(pFileName, "0")) {
                pLevelDecorations[i].uDecorationDescID = pDecorationList->GetDecorIdByName(pFileName);
                pDecorationList->InitializeDecorationSprite(pLevelDecorations[i].uDecorationDescID);
            }

            if (bHide)
                pLevelDecorations[i].uFlags &= ~LEVEL_DECORATION_INVISIBLE;
            else
                pLevelDecorations[i].uFlags |= LEVEL_DECORATION_INVISIBLE;

            pParty->uFlags |= PARTY_FLAGS_1_0002;
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
    viewparams->bRedrawGameUI = 1;
}

//----- (00494035) --------------------------------------------------------
void _494035_timed_effects__water_walking_damage__etc() {
    //    unsigned int v4; // edi@1
    int v24;             // ecx@60
    int v26;             // ecx@64
    int v28;             // ecx@68
    int v30;             // ecx@72
    int v32;             // ecx@76
    int v34;             // ecx@80
    int v36;             // ecx@84
    int v38;             // ecx@88
    int v40;             // ecx@92
    int v42;             // ecx@96
    signed int a2a;      // [sp+18h] [bp-18h]@47
    signed int old_day;  // [sp+1Ch] [bp-14h]@47
    signed int old_hour;

    old_day = pParty->uCurrentDayOfMonth;
    old_hour = pParty->uCurrentHour;

    pParty->GetPlayingTime().value += pEventTimer->uTimeElapsed;

    pParty->uCurrentTimeSecond = pParty->GetPlayingTime().GetSecondsFraction();
    pParty->uCurrentMinute = pParty->GetPlayingTime().GetMinutesFraction();
    pParty->uCurrentHour = pParty->GetPlayingTime().GetHoursOfDay();
    pParty->uCurrentMonthWeek = pParty->GetPlayingTime().GetDays() / 7 & 3;
    pParty->uCurrentDayOfMonth = pParty->GetPlayingTime().GetDays() % 28;
    pParty->uCurrentMonth = pParty->GetPlayingTime().GetMonthsOfYear();
    pParty->uCurrentYear =
        pParty->GetPlayingTime().GetMonths() / 12 + game_starting_year;
    if (pParty->uCurrentHour >= 3 &&
        (old_hour < 3 ||
         pParty->uCurrentDayOfMonth > old_day)) {  // new day dawns
        pParty->pHirelings[0].bHasUsedTheAbility = false;
        pParty->pHirelings[1].bHasUsedTheAbility = false;

        for (uint i = 0; i < pNPCStats->uNumNewNPCs; ++i)
            pNPCStats->pNewNPCData[i].bHasUsedTheAbility = false;

        ++pParty->days_played_without_rest;
        if (pParty->days_played_without_rest > 1) {
            for (uint i = 0; i < 4; ++i)
                pParty->pPlayers[i].SetCondWeakWithBlockCheck(0);

            if (pParty->GetFood() > 0) {
                Party::TakeFood(1);
            } else {
                for (uint i = 0; i < 4; ++i) {
                    pParty->pPlayers[i].sHealth =
                        pParty->pPlayers[i].sHealth /
                        (pParty->days_played_without_rest + 1) +
                        1;
                }
            }

            if (pParty->days_played_without_rest > 3) {
                for (uint i = 0; i < 4; ++i) {
                    pParty->pPlayers[i].Zero();
                    if (!pParty->pPlayers[i].IsPertified() &&
                        !pParty->pPlayers[i].IsEradicated() &&
                        !pParty->pPlayers[i].IsDead()) {
                        if (rand() % 100 < 5 * pParty->days_played_without_rest)
                            pParty->pPlayers[i].SetCondDeadWithBlockCheck(0);
                        if (rand() % 100 <
                            10 * pParty->days_played_without_rest)
                            pParty->pPlayers[i].SetCondInsaneWithBlockCheck(0);
                    }
                }
            }
        }
        if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) pOutdoor->SetFog();

        for (uint i = 0; i < 4; ++i)
            pParty->pPlayers[i].uNumDivineInterventionCastsThisDay = 0;
    }

    // water damage
    if (pParty->uFlags & PARTY_FLAGS_1_WATER_DAMAGE &&
        pParty->_6FC_water_lava_timer < pParty->GetPlayingTime().value) {
        pParty->_6FC_water_lava_timer = pParty->GetPlayingTime().value + 128;
        viewparams->bRedrawGameUI = true;
        for (uint pl = 1; pl <= 4; ++pl) {
            if (pPlayers[pl]->WearsItem(ITEM_RELIC_HARECS_LEATHER,
                EQUIP_ARMOUR) ||
                pPlayers[pl]->HasEnchantedItemEquipped(71) ||
                pPlayers[pl]->pPlayerBuffs[PLAYER_BUFF_WATER_WALK].expire_time) {
                pPlayers[pl]->PlayEmotion(CHARACTER_EXPRESSION_37, 0);
            } else {
                if (!pPlayers[pl]->HasUnderwaterSuitEquipped()) {
                    pPlayers[pl]->ReceiveDamage(
                        (int64_t)pPlayers[pl]->GetMaxHealth() * 0.1,
                        DMGT_FIRE);
                    if (pParty->uFlags & PARTY_FLAGS_1_WATER_DAMAGE) {
                        GameUI_StatusBar_OnEvent_128ms(
                            localization->GetString(660));  // You're drowning!
                    }
                } else {
                    pPlayers[pl]->PlayEmotion(CHARACTER_EXPRESSION_37, 0);
                }
            }
        }
    }

    // lava damage
    if (pParty->uFlags & PARTY_FLAGS_1_BURNING &&
        pParty->_6FC_water_lava_timer < pParty->GetPlayingTime().value) {
        viewparams->bRedrawGameUI = true;
        pParty->_6FC_water_lava_timer = pParty->GetPlayingTime().value + 128;

        for (uint pl = 1; pl <= 4; pl++) {
            pPlayers[pl]->ReceiveDamage(
                (signed __int64)pPlayers[pl]->GetMaxHealth() * 0.1, DMGT_FIRE);
            if (pParty->uFlags & PARTY_FLAGS_1_BURNING) {
                GameUI_StatusBar_OnEvent_128ms(
                    localization->GetString(661));  // On fire!
            }
        }
    }
    _493938_regenerate();
    uint party_condition_flag = 4;
    a2a = pEventTimer->uTimeElapsed;
    if (pParty->uFlags2 &
        PARTY_FLAGS_2_RUNNING) {  //замедление восстановления при беге
        a2a *= 0.5f;
        if (a2a < 1) a2a = 1;
    }

    for (uint pl = 1; pl <= 4; pl++) {
        if (pPlayers[pl]->uTimeToRecovery)
            pPlayers[pl]->Recover(a2a);  //восстановление активности
        if (pPlayers[pl]->GetItemsBonus(CHARACTER_ATTRIBUTE_ENDURANCE) +
                    pPlayers[pl]->sHealth + pPlayers[pl]->uEndurance >=
                1 ||
            pPlayers[pl]->pPlayerBuffs[PLAYER_BUFF_PRESERVATION].Active()) {
            if (pPlayers[pl]->sHealth < 1)
                pPlayers[pl]->SetCondition(Condition_Unconcious, 0);
        } else {
            pPlayers[pl]->SetCondition(Condition_Dead, 0);
        }
        if (pPlayers[pl]->field_E0) {
            v24 = pPlayers[pl]->field_E0 - pEventTimer->uTimeElapsed;
            if (v24 > 0) {
                pPlayers[pl]->field_E0 = v24;
            } else {
                pPlayers[pl]->field_E0 = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->field_E4) {
            v26 = pPlayers[pl]->field_E4 - pEventTimer->uTimeElapsed;
            if (v26 > 0) {
                pPlayers[pl]->field_E4 = v26;
            } else {
                pPlayers[pl]->field_E4 = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->field_E8) {
            v28 = pPlayers[pl]->field_E8 - pEventTimer->uTimeElapsed;
            if (v28 > 0) {
                pPlayers[pl]->field_E8 = v28;
            } else {
                pPlayers[pl]->field_E8 = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->field_EC) {
            v30 = pPlayers[pl]->field_EC - pEventTimer->uTimeElapsed;
            if (v30 > 0) {
                pPlayers[pl]->field_EC = v30;
            } else {
                pPlayers[pl]->field_EC = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->field_F0) {
            v32 = pPlayers[pl]->field_F0 - pEventTimer->uTimeElapsed;
            if (v32 > 0) {
                pPlayers[pl]->field_F0 = v32;
            } else {
                pPlayers[pl]->field_F0 = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->field_F4) {
            v34 = pPlayers[pl]->field_F4 - pEventTimer->uTimeElapsed;
            if (v34 > 0) {
                pPlayers[pl]->field_F4 = v34;
            } else {
                pPlayers[pl]->field_F4 = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->field_F8) {
            v36 = pPlayers[pl]->field_F8 - pEventTimer->uTimeElapsed;
            if (v36 > 0) {
                pPlayers[pl]->field_F8 = v36;
            } else {
                pPlayers[pl]->field_F8 = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->field_FC) {
            v38 = pPlayers[pl]->field_FC - pEventTimer->uTimeElapsed;
            if (v38 > 0) {
                pPlayers[pl]->field_FC = v38;
            } else {
                pPlayers[pl]->field_FC = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->field_100) {
            v40 = pPlayers[pl]->field_100 - pEventTimer->uTimeElapsed;
            if (v40 > 0) {
                pPlayers[pl]->field_100 = v40;
            } else {
                pPlayers[pl]->field_100 = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->field_104) {
            v42 = pPlayers[pl]->field_104 - pEventTimer->uTimeElapsed;
            if (v42 > 0) {
                pPlayers[pl]->field_104 = v42;
            } else {
                pPlayers[pl]->field_104 = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->conditions_times[Condition_Sleep].Valid() ||
            pPlayers[pl]->conditions_times[Condition_Paralyzed].Valid() ||
            pPlayers[pl]->conditions_times[Condition_Unconcious].Valid() ||
            pPlayers[pl]->conditions_times[Condition_Dead].Valid() ||
            pPlayers[pl]->conditions_times[Condition_Pertified].Valid() ||
            pPlayers[pl]->conditions_times[Condition_Eradicated].Valid()) {
            --party_condition_flag;
        }

        for (uint k = 0; k < 24; ++k) {
            pPlayers[pl]->pPlayerBuffs[k].IsBuffExpiredToTime(
                pParty->GetPlayingTime());
        }

        if (pPlayers[pl]->pPlayerBuffs[PLAYER_BUFF_HASTE].Expired()) {
            pPlayers[pl]->SetCondition(Condition_Weak, 0);
        }
    }

    for (uint i = 0; i < 20; ++i) {
        if (pParty->pPartyBuffs[i].IsBuffExpiredToTime(
                pParty->GetPlayingTime()) == 1)
            viewparams->bRedrawGameUI = true;
    }

    if (pParty->pPartyBuffs[PARTY_BUFF_HASTE].Expired()) {
        for (uint i = 0; i < 4; ++i)
            pParty->pPlayers[i].SetCondition(Condition_Weak, 0);
    }

    for (uint i = 0; i < 2; ++i) {  // Проверка в сознании ли перс сделавший закл
                                    // на полёт и хождение по воде
        SpellBuff *pBuf = &pParty->pPartyBuffs[Party_Spec_Motion_status_ids[i]];
        if (!pBuf->expire_time) continue;

        if (!(pBuf->uFlags & 1)) {
            if (!pPlayers[pBuf->uCaster]->CanAct()) {
                pBuf->Reset();
                if (Party_Spec_Motion_status_ids[i] == PARTY_BUFF_FLY)
                    pParty->bFlying = false;
            }
        }
    }

    if (!party_condition_flag) {
        if (current_screen_type != CURRENT_SCREEN::SCREEN_REST) {
            for (uint pl = 1; pl <= 4; pl++) {
                if (pPlayers[pl]->conditions_times[Condition_Sleep].Valid()) {
                    pPlayers[pl]->conditions_times[Condition_Sleep].Reset();
                    party_condition_flag = 1;
                    break;
                }
            }
            if (!party_condition_flag || _5C35C0_force_party_death)
                uGameState = GAME_STATE_PARTY_DIED;
        }
    }

    if (uActiveCharacter) {  // выбор следующего после пропускающего ход
        if (current_screen_type != CURRENT_SCREEN::SCREEN_REST) {
            if (pPlayers[uActiveCharacter]->conditions_times[Condition_Sleep] ||
                pPlayers[uActiveCharacter]
                    ->conditions_times[Condition_Paralyzed] ||
                pPlayers[uActiveCharacter]
                    ->conditions_times[Condition_Unconcious] ||
                pPlayers[uActiveCharacter]->conditions_times[Condition_Dead] ||
                pPlayers[uActiveCharacter]
                    ->conditions_times[Condition_Pertified] ||
                pPlayers[uActiveCharacter]
                    ->conditions_times[Condition_Eradicated]) {
                viewparams->bRedrawGameUI = true;
                uActiveCharacter = pParty->GetNextActiveCharacter();
            }
        }
    }
}

//----- (00493938) --------------------------------------------------------
void _493938_regenerate() {  // immolation
    int current_time;                     // edi@1
    int last_reg_time;                    // qax@1
    int v4;                               // eax@2
    int v5;                               // edi@5
    int v9;                               // edi@15
    // signed int v10;                       // eax@15
    int numberOfActorsAffected;           // ebx@20
    unsigned int v14;                     // esi@21
    signed int v19;                       // eax@21
    bool recovery_HP;                     // ebx@25
    signed int v25;                       // eax@33
    signed int v31;                       // ecx@53
    int actorsAffectedByImmolation[100];  // [sp+4h] [bp-22Ch]@20
    SpriteObject a1;                      // [sp+194h] [bp-9Ch]@15
    Vec3_int_ a3;                         // [sp+204h] [bp-2Ch]@15
    bool has_dragon_flag;                 // [sp+210h] [bp-20h]@22
    bool lich_jar_flag;                   // [sp+214h] [bp-1Ch]@25
    bool zombie_flag;                     // [sp+218h] [bp-18h]@25
    bool decrease_HP;                     // [sp+21Ch] [bp-14h]@25
    bool lich_flag;                       // [sp+220h] [bp-10h]@25
    int v49;                              // [sp+224h] [bp-Ch]@24
    bool recovery_SP;                     // [sp+228h] [bp-8h]@25
    bool redraw_flag;                     // [sp+22Ch] [bp-4h]@2

    current_time = pParty->GetPlayingTime().GetMinutesFraction();
    last_reg_time = pParty->last_regenerated.GetMinutesFraction();

    if (current_time == last_reg_time) return;

    int testmin = last_reg_time + 5;
    if (testmin >= 60) {  // hour tickover boundaries
        testmin -= 60;
        if (current_time >= 55) current_time -= 60;
    }

    if (current_time >= testmin) {
        redraw_flag = false;
        v4 = (current_time - last_reg_time) / 5;

        // chance to flight break due to a curse
        if (pParty->FlyActive()) {
            if (pParty->bFlying) {
                if (!(pParty->pPartyBuffs[PARTY_BUFF_FLY].uFlags & 1)) {
                    v5 = v4 * pParty->pPartyBuffs[PARTY_BUFF_FLY].uPower;

                    auto v6 =
                        &pParty
                             ->pPlayers[pParty->pPartyBuffs[PARTY_BUFF_FLY]
                                            .uCaster -
                                        1]
                             .conditions_times[Condition_Cursed];
                    if (v6->value < v5) {
                        v6 = 0;
                        pParty->uFlags &= 0xFFFFFFBF;
                        pParty->bFlying = false;
                        redraw_flag = true;
                    }
                }
            }
        }

        // chance to waterwalk drowning due to a curse
        if (pParty->WaterWalkActive()) {
            if (pParty->uFlags & PARTY_FLAGS_1_STANDING_ON_WATER) {
                if (!(pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uFlags &
                      1)) {  // taking on water
                    auto v8 =
                        &pParty
                             ->pPlayers[pParty
                                            ->pPartyBuffs[PARTY_BUFF_WATER_WALK]
                                            .uCaster -
                                        1]
                             .conditions_times[Condition_Cursed];
                    v8->value -= v4;
                    if (v8->value <= 0) {
                        v8->value = 0;
                        pParty->uFlags &= ~PARTY_FLAGS_1_STANDING_ON_WATER;
                        redraw_flag = true;
                    }
                }
            }
        }

        if (pParty->ImmolationActive()) {  // Жертва
            a3.z = 0;
            a3.y = 0;
            a3.x = 0;
            a1.containing_item.Reset();
            a1.spell_level = pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].uPower;
            a1.spell_skill = pParty->ImmolationSkillLevel();
            a1.uType = SPRITE_SPELL_FIRE_IMMOLATION;
            a1.spell_id = SPELL_FIRE_IMMOLATION;
            a1.uObjectDescID = pObjectList->ObjectIDByItemID(spell_sprite_mapping[8].uSpriteType);
            a1.field_60_distance_related_prolly_lod = 0;
            a1.uAttributes = 0;
            a1.uSectorID = 0;
            a1.uSpriteFrameID = 0;
            a1.spell_caster_pid = PID(OBJECT_Player, pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].uCaster);
            a1.uFacing = 0;
            a1.uSoundID = 0;
            numberOfActorsAffected = pParty->_46A89E_immolation_effect(actorsAffectedByImmolation, 100, 307);
            for (v9 = 0; v9 < numberOfActorsAffected; ++v9) {
                v14 = actorsAffectedByImmolation[v9];
                a1.vPosition.x = pActors[v14].vPosition.x;
                a1.vPosition.y = pActors[v14].vPosition.y;
                a1.vPosition.z = pActors[v14].vPosition.z;
                a1.spell_target_pid = PID(OBJECT_Actor, v14);
                v19 = a1.Create(0, 0, 0, 0);
                Actor::DamageMonsterFromParty(PID(OBJECT_Item, v19), v14, &a3);
            }
        }

        has_dragon_flag = false;
        if (PartyHasDragon()) has_dragon_flag = true;

        for (v49 = 0; v49 < 4; v49++) {
            recovery_HP = false;
            recovery_SP = false;
            decrease_HP = false;
            lich_flag = false;
            lich_jar_flag = false;
            zombie_flag = false;

            for (int v22 = 0; (signed int)v22 < 16; v22++) {
                if (pParty->pPlayers[v49].HasItemEquipped(
                        (ITEM_EQUIP_TYPE)v22)) {
                    uint _idx = pParty->pPlayers[v49].pEquipment.pIndices[v22];
                    if (pParty->pPlayers[v49]
                            .pInventoryItemList[_idx - 1]
                            .uItemID > 134) {
                        if (pParty->pPlayers[v49]
                                .pInventoryItemList[_idx - 1]
                                .uItemID == ITEM_RELIC_ETHRICS_STAFF)
                            decrease_HP = true;
                        if (pParty->pPlayers[v49]
                                .pInventoryItemList[_idx - 1]
                                .uItemID == ITEM_ARTIFACT_HERMES_SANDALS) {
                            recovery_HP = true;
                            recovery_SP = true;
                        }
                        if (pParty->pPlayers[v49]
                                .pInventoryItemList[_idx - 1]
                                .uItemID == ITEM_ARTIFACT_MINDS_EYE)
                            recovery_SP = true;
                        if (pParty->pPlayers[v49]
                                .pInventoryItemList[_idx - 1]
                                .uItemID == ITEM_ARTIFACT_HEROS_BELT)
                            recovery_HP = true;
                    } else {
                        v25 = pParty->pPlayers[v49]
                                  .pInventoryItemList[_idx - 1]
                                  .special_enchantment;
                        if (v25 == 37  // of Regeneration("Regenerate 1hp/x
                                       // while walking, etc")
                            || v25 == 44  // of Life("HP (+10), Regen hpts")
                            || v25 == 50  // of The Phoenix("Fire Res (+30),
                                          // Regen hpts") &&
                            ||
                            v25 == 54)  // of The Troll("End (+15), Regen hpts")
                            recovery_HP = true;
                        if (v25 == 38  // of Mana("Regenerate 1sp/x while
                                       // walking, etc")
                            ||
                            v25 == 47  // of The Eclipse("SP (+10), Regen spts")
                            ||
                            v25 ==
                                55)  // of The Unicorn("Luck (+15), Regen spts")
                            recovery_SP = true;
                        if (v25 == 66) {  // of Plenty("Regenerate 1 hp/x and 1
                                          // sp/x while walking, etc.")
                            recovery_HP = true;
                            recovery_SP = true;
                        }
                    }

                    if (recovery_HP &&
                        !pParty->pPlayers[v49]
                             .conditions_times[Condition_Dead] &&
                        !pParty->pPlayers[v49]
                             .conditions_times[Condition_Eradicated]) {
                        if (pParty->pPlayers[v49].sHealth <
                            pParty->pPlayers[v49].GetMaxHealth()) {
                            ++pParty->pPlayers[v49].sHealth;
                        }
                        if (pParty->pPlayers[v49]
                                .conditions_times[Condition_Unconcious] &&
                            pParty->pPlayers[v49].sHealth > 0) {
                            pParty->pPlayers[v49]
                                .conditions_times[Condition_Unconcious]
                                .Reset();
                        }
                        redraw_flag = true;
                    }

                    if (recovery_SP &&
                        !pParty->pPlayers[v49]
                             .conditions_times[Condition_Dead] &&
                        !pParty->pPlayers[v49]
                             .conditions_times[Condition_Eradicated]) {
                        if (pParty->pPlayers[v49].sMana <
                            pParty->pPlayers[v49].GetMaxMana())
                            ++pParty->pPlayers[v49].sMana;
                        redraw_flag = true;
                    }

                    if (decrease_HP &&
                        !pParty->pPlayers[v49]
                             .conditions_times[Condition_Dead] &&
                        !pParty->pPlayers[v49]
                             .conditions_times[Condition_Eradicated]) {
                        --pParty->pPlayers[v49].sHealth;
                        if (!(pParty->pPlayers[v49]
                                  .conditions_times[Condition_Unconcious]) &&
                            pParty->pPlayers[v49].sHealth < 0) {
                            pParty->pPlayers[v49]
                                .conditions_times[Condition_Unconcious] =
                                pParty->GetPlayingTime();
                        }
                        if (pParty->pPlayers[v49].sHealth < 1) {
                            if (pParty->pPlayers[v49].sHealth +
                                        pParty->pPlayers[v49].uEndurance +
                                        pParty->pPlayers[v49].GetItemsBonus(
                                            CHARACTER_ATTRIBUTE_ENDURANCE) >=
                                    1 ||
                                pParty->pPlayers[v49]
                                    .pPlayerBuffs[PLAYER_BUFF_PRESERVATION]
                                    .expire_time) {
                                pParty->pPlayers[v49]
                                    .conditions_times[Condition_Unconcious] =
                                    pParty->GetPlayingTime();
                            } else if (!pParty->pPlayers[v49]
                                            .conditions_times[Condition_Dead]) {
                                pParty->pPlayers[v49]
                                    .conditions_times[Condition_Dead] =
                                    pParty->GetPlayingTime();
                            }
                        }
                        redraw_flag = true;
                    }
                }
            }

            // regeneration
            if (pParty->pPlayers[v49]
                    .pPlayerBuffs[PLAYER_BUFF_REGENERATION]
                    .expire_time &&
                !pParty->pPlayers[v49].conditions_times[Condition_Dead] &&
                !pParty->pPlayers[v49].conditions_times[Condition_Eradicated]) {
                pParty->pPlayers[v49].sHealth +=
                    5 * pParty->pPlayers[v49]
                            .pPlayerBuffs[PLAYER_BUFF_REGENERATION]
                            .uPower;
                if (pParty->pPlayers[v49].sHealth >
                    pParty->pPlayers[v49].GetMaxHealth()) {
                    pParty->pPlayers[v49].sHealth =
                        pParty->pPlayers[v49].GetMaxHealth();
                }
                if (pParty->pPlayers[v49]
                        .conditions_times[Condition_Unconcious] &&
                    pParty->pPlayers[v49].sHealth > 0) {
                    pParty->pPlayers[v49]
                        .conditions_times[Condition_Unconcious]
                        .Reset();
                }
                redraw_flag = true;
            }

            // for warlock
            if (has_dragon_flag &&
                pParty->pPlayers[v49].classType == PLAYER_CLASS_WARLOCK) {
                if (pParty->pPlayers[v49].sMana <
                    pParty->pPlayers[v49].GetMaxMana()) {
                    ++pParty->pPlayers[v49].sMana;
                }
                redraw_flag = true;
            }

            // for lich
            if (pParty->pPlayers[v49].classType == PLAYER_CLASS_LICH) {
                for (v31 = 0; v31 < 126; ++v31) {
                    if (pParty->pPlayers[v49].pInventoryItemList[v31].uItemID ==
                        ITEM_LICH_JAR_FULL)
                        lich_jar_flag = true;
                }
                lich_flag = true;
            }

            if (lich_flag &&
                !pParty->pPlayers[v49].conditions_times[Condition_Dead] &&
                !pParty->pPlayers[v49].conditions_times[Condition_Eradicated]) {
                if (pParty->pPlayers[v49].sHealth >
                    pParty->pPlayers[v49].GetMaxHealth() / 2) {
                    pParty->pPlayers[v49].sHealth =
                        pParty->pPlayers[v49].sHealth - 2;
                }
                if (pParty->pPlayers[v49].sMana >
                    pParty->pPlayers[v49].GetMaxMana() / 2) {
                    pParty->pPlayers[v49].sMana =
                        pParty->pPlayers[v49].sMana - 2;
                }
            }

            if (lich_jar_flag) {
                if (pParty->pPlayers[v49].sMana <
                    pParty->pPlayers[v49].GetMaxMana()) {
                    ++pParty->pPlayers[v49].sMana;
                }
            }

            // for zombie
            if (pParty->pPlayers[v49].conditions_times[Condition_Zombie]) {
                zombie_flag = true;
            }
            if (zombie_flag &&
                !pParty->pPlayers[v49].conditions_times[Condition_Dead] &&
                !pParty->pPlayers[v49].conditions_times[Condition_Eradicated]) {
                if (pParty->pPlayers[v49].sHealth >
                    pParty->pPlayers[v49].GetMaxHealth() / 2) {
                    pParty->pPlayers[v49].sHealth =
                        pParty->pPlayers[v49].sHealth - 1;
                }
                if (pParty->pPlayers[v49].sMana > 0) {
                    pParty->pPlayers[v49].sMana =
                        pParty->pPlayers[v49].sMana - 1;
                }
            }
        }
        pParty->last_regenerated = pParty->GetPlayingTime();
        if (!viewparams->bRedrawGameUI) viewparams->bRedrawGameUI = redraw_flag;
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
        logger->Warning("pLevelStrOffsets: deserialization warning");
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
            if (RemoveQuotes(&pLevelStr[pLevelStrOffsets[i]]) !=
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
            test_event =
                (_evt_raw *)&pLevelEVT[pLevelEVT_Index[i].uEventOffsetInEVT];
            if (test_event->_e_type == EVENT_OnMapLeave) {
                EventProcessor(pLevelEVT_Index[i].uEventID, 0, 1,
                               pLevelEVT_Index[i].event_sequence_num);
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

    for (uint i = 0; i < uLevelEVT_NumEvents; ++i) {
        EventIndex pEvent = pLevelEVT_Index[i];

        _evt_raw *_evt = (_evt_raw *)(&pLevelEVT[pEvent.uEventOffsetInEVT]);

        //        if (_evt->_e_type == EVENT_PlaySound)
        //            pSoundList->LoadSound(EVT_DWORD(_evt->v5), 0);
        //        else
        if (_evt->_e_type == EVENT_OnMapReload) {
            EventProcessor(pEvent.uEventID, 0, 0, pEvent.event_sequence_num);
        } else if (_evt->_e_type == EVENT_OnTimer ||
                 _evt->_e_type == EVENT_OnLongTimer) {
            // v3 = &MapsLongTimersList[MapsLongTimers_count];
            v20 = pOutdoor->loc_time.last_visit;
            if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
                v20 = pIndoor->stru1.last_visit;

            MapsLongTimersList[MapsLongTimers_count].timer_evt_type =
                _evt->_e_type;
            MapsLongTimersList[MapsLongTimers_count].timer_evt_ID =
                pEvent.uEventID;
            MapsLongTimersList[MapsLongTimers_count].timer_evt_seq_num =
                pEvent.event_sequence_num;

            MapsLongTimersList[MapsLongTimers_count].YearsInterval = _evt->v5;
            MapsLongTimersList[MapsLongTimers_count].MonthsInterval = _evt->v6;
            MapsLongTimersList[MapsLongTimers_count].WeeksInterval = _evt->v7;
            MapsLongTimersList[MapsLongTimers_count].HoursInterval = _evt->v8;
            MapsLongTimersList[MapsLongTimers_count].MinutesInterval = _evt->v9;
            MapsLongTimersList[MapsLongTimers_count].SecondsInterval =
                _evt->v10;

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

void Level_LoadEvtAndStr(const char *pLevelName) {
    char pContainerName[120];  // [sp+8h] [bp-98h]@1

    sprintf(pContainerName, "%s.evt", pLevelName);
    uLevelEVT_Size = LoadEventsToBuffer(pContainerName, pLevelEVT.data(), 9216);

    sprintf(pContainerName, "%s.str", pLevelName);
    uLevelStrFileSize =
        LoadEventsToBuffer(pContainerName, pLevelStr.data(), 9216);
    if (uLevelStrFileSize) LoadLevel_InitializeLevelStr();
}

void sub_4452BB() {
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
           current_screen_type == CURRENT_SCREEN::SCREEN_E ||
           current_screen_type == CURRENT_SCREEN::SCREEN_CHANGE_LOCATION ||
           current_screen_type == CURRENT_SCREEN::SCREEN_INPUT_BLV ||
           current_screen_type == CURRENT_SCREEN::SCREEN_CASTING;
}

void Transition_StopSound_Autosave(const char *pMapName,
                                   MapStartPoint start_point) {
    pAudioPlayer->StopChannels(-1, -1);

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

    __int64 v13 = (pParty->GetPlayingTime() - _5773B8_event_timer).value / 128;
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
