#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <crtdbg.h>
#include <direct.h>

#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/Time.h"
#include "Engine/Party.h"
#include "Engine/LOD.h"
#include "Engine/Events.h"
#include "Engine/OurMath.h"
#include "Engine/stru123.h"
#include "Engine/LuaVM.h"
#include "Engine/MMT.h"
#include "Engine/SaveLoad.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Weather.h"
#include "Engine/Graphics/LightmapBuilder.h"
#include "Engine/Graphics/DecalBuilder.h"
#include "Engine/Graphics/ParticleEngine.h"
#include "Engine/Graphics/GammaControl.h"
#include "Engine/Graphics/stru9.h"
#include "Engine/Graphics/stru10.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/Lights.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/RenderD3D11.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Tables/FrameTableInc.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/Chest.h"

#include "Arcomage\Arcomage.h"

#include "IO/Mouse.h"
#include "IO/Keyboard.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIProgressBar.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIShops.h"
#include "GUI/UI/UIPartyCreation.h"
#include "GUI/UI/UIStatusBar.h"

#include "GUI/NewUI/MainMenu.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/Video/Bink_Smacker.h"

#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Tables/PlayerFrameTable.h"
#include "Engine/Tables/StorylineTextTable.h"
#include "Engine/Tables/FactionTable.h"
#include "Engine/MapsLongTimer.h"

#include "Game/Game.h"
#include "Game/MainMenu.h"
#include "Game/MainMenuLoad.h"

#include "stru6.h"





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
render->DrawTextureAlphaNew((64 + torchA.icon->texture->GetWidth())/ 640.0f, 48 / 480.0f, icon->texture);

icon = pIconsFrameTable->GetFrame(torchC.icon->id, GetTickCount() / 2);
render->DrawTextureAlphaNew((64 + torchA.icon->texture->GetWidth() + torchB.icon->texture->GetWidth()) / 640.0f, 48 / 480.0f, icon->texture);

*/


Engine *pEngine = nullptr;







//----- (00466C40) --------------------------------------------------------
const wchar_t *MENU_STATE_to_string(MENU_STATE m)
{
    switch (m)
    {
    case -1:                return L"-1";
    case MENU_MAIN:         return L"MENU_MAIN";
    case MENU_NEWGAME:      return L"MENU_NEWGAME";
    case MENU_CREDITS:      return L"MENU_CREDITS";
    case MENU_SAVELOAD:     return L"MENU_SAVELOAD";
    case MENU_EXIT_GAME:    return L"MENU_EXIT_GAME";
    case MENU_5:            return L"MENU_5";
    case MENU_CREATEPARTY:  return L"MENU_CREATEPARTY";
    case MENU_NAMEPANELESC: return L"MENU_NAMEPANELESC";
    case MENU_CREDITSPROC:  return L"MENU_CREDITSPROC";
    case MENU_LoadingProcInMainMenu: return L"MENU_LoadingProcInMainMenu";
    case MENU_DebugBLVLevel:         return L"MENU_DebugBLVLevel";
    case MENU_CREDITSCLOSE: return L"MENU_CREDITSCLOSE";
    case MENU_MMT_MAIN_MENU: return L"MENU_MMT_MAIN_MENU";
    default:                return L"unk";
    };
};


void SetCurrentMenuID(MENU_STATE uMenu)
{
    sCurrentMenuID = uMenu;
    Log::Warning(L"CurrentMenu = %s \n", MENU_STATE_to_string(uMenu));
}

//----- (00466CA0) --------------------------------------------------------
MENU_STATE GetCurrentMenuID()
{
    return sCurrentMenuID;
}



//----- (00464761) --------------------------------------------------------
void Engine_DeinitializeAndTerminate(int exitCode)
{
    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
    ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
    pEngine->Deinitialize();
    render->Release();
    delete window;
    //if ( !DestroyWindow(hWnd) )
    //  GetLastError();
    exit(exitCode);
}











//----- (00435694) --------------------------------------------------------
void Engine::ToggleFlags2(unsigned int uFlag)
{
    unsigned int v2; // eax@1

    v2 = this->uFlags2;
    if (v2 & uFlag)
        this->uFlags2 = v2 & ~uFlag;
    else
        this->uFlags2 = uFlag | v2;
}

//----- (0044103C) --------------------------------------------------------
void Engine::Draw()
{
    int v4; // edi@26

    uFlags2 &= ~0x02;
    if (pParty->_497FC5_check_party_perception_against_level())
        uFlags2 |= 2;

    pIndoorCameraD3D->sRotationX = pParty->sRotationX;
    pIndoorCameraD3D->sRotationY = pParty->sRotationY;
    pIndoorCameraD3D->vPartyPos.x = pParty->vPosition.x - pParty->y_rotation_granularity * cosf(2 * pi_double * pParty->sRotationY / 2048.0);
    pIndoorCameraD3D->vPartyPos.y = pParty->vPosition.y - pParty->y_rotation_granularity * sinf(2 * pi_double * pParty->sRotationY / 2048.0);
    pIndoorCameraD3D->vPartyPos.z = pParty->vPosition.z + pParty->sEyelevel;//193, but real 353

    //pIndoorCamera->Initialize2();
    pIndoorCameraD3D->CalculateRotations(pParty->sRotationX, pParty->sRotationY);
    pIndoorCameraD3D->CreateWorldMatrixAndSomeStuff();
    pIndoorCameraD3D->_4374E8_ProllyBuildFrustrum();

    if (pMovie_Track)
    {
        /*if ( !render->pRenderD3D )
        {
        render->BeginSceneD3D();
        pMouse->DrawCursorToTarget();
        render->DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();
        }*/
    }
    else
    {
        if (pParty->vPosition.x != pParty->vPrevPosition.x || pParty->sRotationY != pParty->sPrevRotationY || pParty->vPosition.y != pParty->vPrevPosition.y
            || pParty->sRotationX != pParty->sPrevRotationX || pParty->vPosition.z != pParty->vPrevPosition.z || pParty->sEyelevel != pParty->sPrevEyelevel)
            pParty->uFlags |= 2u;
        pParty->vPrevPosition.x = pParty->vPosition.x;
        pParty->vPrevPosition.y = pParty->vPosition.y;
        pParty->vPrevPosition.z = pParty->vPosition.z;
        //v0 = &render;
        pParty->sPrevRotationY = pParty->sRotationY;
        pParty->sPrevRotationX = pParty->sRotationX;

        pParty->sPrevEyelevel = pParty->sEyelevel;
        render->BeginSceneD3D();

        //if ( !render->pRenderD3D )
        //pMouse->DrawCursorToTarget();
        if (!PauseGameDrawing() || viewparams->field_48 == 1)
        {
            //if ( render->pRenderD3D )
            {
                float v2 = (double)(((signed int)pMiscTimer->uTotalGameTimeElapsed >> 2) & 0x1F) * 0.032258064 * 6.0;
                //v3 = v2 + 6.7553994e15;
                //render->field_1036A8_bitmapid = LODWORD(v3);
                render->hd_water_current_frame = floorf(v2 + 0.5f);
            }

            if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
                pIndoor->Draw();
            else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
                pOutdoor->Draw();
            else
                Error("Invalid level type: %u", uCurrentlyLoadedLevelType);

            //if (render->pRenderD3D)
            {
                pDecalBuilder->DrawBloodsplats();
                pEngine->pLightmapBuilder->DrawLightmapsType(2);
            }
        }
        render->DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();
    }

    //DEBUG: force redraw gui
    viewparams->bRedrawGameUI = true;


    render->BeginScene();
    //if (render->pRenderD3D)
    pMouse->DrawCursorToTarget();
    if (pOtherOverlayList->bRedraw)
        viewparams->bRedrawGameUI = true;
    v4 = viewparams->bRedrawGameUI;
    GameUI_StatusBar_DrawForced();
    if (!viewparams->bRedrawGameUI)
        GameUI_DrawRightPanelItems();
    else
    {
        GameUI_DrawRightPanelFrames();
        GameUI_StatusBar_Draw();
        viewparams->bRedrawGameUI = false;
    }
    if (!pMovie_Track)//!pVideoPlayer->pSmackerMovie)
    {
        GameUI_DrawMinimap(488, 16, 625, 133, viewparams->uMinimapZoom, true);//redraw = pParty->uFlags & 2);
        if (v4)
        {
            if (!PauseGameDrawing() /*&& render->pRenderD3D*/) // clear game viewport with transparent color
                render->FillRectFast(pViewport->uViewportTL_X, pViewport->uViewportTL_Y, pViewport->uViewportBR_X - pViewport->uViewportTL_X,
                pViewport->uViewportBR_Y - pViewport->uViewportTL_Y + 1,
                0x7FF);
            viewparams->field_48 = 0;
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
    if (_44100D_should_alter_right_panel())
        GameUI_DrawRightPanel();
    if (!pMovie_Track)
    {
        pStru6Instance->DrawPlayerBuffAnims();
        pOtherOverlayList->DrawTurnBasedIcon(v4);
        GameUI_DrawTorchlightAndWizardEye();
    }


    static bool render_framerate = false;
    static float framerate = 0.0f;
    static uint frames_this_second = 0;
    static uint last_frame_time = GetTickCount();
    static uint framerate_time_elapsed = 0;

    if (current_screen_type == SCREEN_GAME && uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        pWeather->Draw();//Ritor1: my include

    //while(GetTickCount() - last_frame_time < 33 );//FPS control 
    uint frame_dt = GetTickCount() - last_frame_time;
    last_frame_time = GetTickCount();

    framerate_time_elapsed += frame_dt;
    if (framerate_time_elapsed >= 1000)
    {
        framerate = frames_this_second *  (1000.0f / framerate_time_elapsed);

        framerate_time_elapsed = 0;
        frames_this_second = 0;
        render_framerate = true;
    }

    ++frames_this_second;

    if (debug_information)
    {
        if (render_framerate)
        {
            pPrimaryWindow->DrawText(pFontArrus, 494, 0, Color16(0, 0, 0), StringPrintf("FPS: % .4f", framerate), 0, 0, 0);
        }

        if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        {
            int sector_id = pIndoor->GetSector(pParty->vPosition.x, pParty->vPosition.y, pParty->vPosition.z);
            pPrimaryWindow->DrawText(pFontArrus, 16, 16, Color16(255, 255, 255), StringPrintf("Party Sector ID:        %u/%u\n", sector_id, pIndoor->uNumSectors), 0, 0, Color16(255, 255, 255));
        }
        pPrimaryWindow->DrawText(pFontArrus, 16, 16 + 16, Color16(255, 255, 255), StringPrintf("Party Position:         % d % d % d", pParty->vPosition.x, pParty->vPosition.y, pParty->vPosition.z), 0, 0, Color16(255, 255, 255));

        String floor_level_str;
        if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        {
            uint uFaceID;
            int sector_id = pIndoor->GetSector(pParty->vPosition.x, pParty->vPosition.y, pParty->vPosition.z);
            int floor_level = BLV_GetFloorLevel(pParty->vPosition.x, pParty->vPosition.y, pParty->vPosition.z + 40, sector_id, &uFaceID);
            floor_level_str = StringPrintf("BLV_GetFloorLevel: %d   face_id %d\n", floor_level, uFaceID);
        }
        else
        {
            int on_water, _a6;
            int floor_level = ODM_GetFloorLevel(pParty->vPosition.x, pParty->vPosition.y, pParty->vPosition.z, 0, &on_water, &_a6, false);
            floor_level_str = StringPrintf("ODM_GetFloorLevel: %d   on_water: %s    a6 = %d\n", floor_level, on_water ? "true" : "false", _a6);
        }
        pPrimaryWindow->DrawText(pFontArrus, 16, 16 + 16 + 16, Color16(255, 255, 255), floor_level_str, 0, 0, Color16(255, 255, 255));
    }

    GUI_UpdateWindows();
    pParty->UpdatePlayersAndHirelingsEmotions();

    _unused_5B5924_is_travel_ui_drawn = false;
    if (v4)
        pMouse->bRedraw = true;
    pMouse->ReadCursorWithItem();
    pMouse->DrawCursor();
    pMouse->Activate();
    render->EndScene();
    render->Present();
    pParty->uFlags &= ~2;
}

//----- (0047A815) --------------------------------------------------------
void Engine::DrawParticles()
{
    pParticleEngine->Draw();
}


//----- (0044F192) --------------------------------------------------------
void Engine::PrepareBloodsplats()
{
    for (uint i = 0; i < uNumBloodsplats; ++i)
    {
        pBloodsplatContainer->AddBloodsplat(pBloodsplats[i].x, pBloodsplats[i].y, pBloodsplats[i].z,
            pBloodsplats[i].radius, pBloodsplats[i].r, pBloodsplats[i].g, pBloodsplats[i].b);
    }
}


//----- (0044F120) --------------------------------------------------------
void Engine::PushStationaryLights(int a2)
{
    Game__StationaryLight* pLight;

    for (int i = 0; i < uNumStationaryLights; ++i)
    {
        pLight = &pStationaryLights[i];
        pStationaryLightsStack->AddLight(pLight->vPosition.x, pLight->vPosition.y, pLight->vPosition.z,
            pLight->flt_18, pLight->vRGBColor.x, pLight->vRGBColor.y, pLight->vRGBColor.z, _4E94D0_light_type);
    }
}
// 4E94D0: using guessed type char _4E94D0_light_type;

//----- (0044F0FD) --------------------------------------------------------
void Engine::_44F0FD()
{
    ToggleFlags(0x40u);

    if (!(uFlags & 0x40))
    {
        uNumBloodsplats = 0;
        field_E0C = 0;
    }
}

//----- (0044F0D8) --------------------------------------------------------
void Engine::ToggleFlags(uint uMask)
{
    if (uFlags & uMask)
        uFlags &= ~uMask;
    else
        uFlags |= uMask;
}


//----- (0044F07B) --------------------------------------------------------
bool Engine::_44F07B()
{
    if (!pKeyboardInstance->IsKeyBeingHeld(VK_SHIFT) && !pKeyboardInstance->IsKeyBeingHeld(VK_LSHIFT) &&
        !pKeyboardInstance->IsKeyBeingHeld(VK_LSHIFT) || (pKeyboardInstance->WasKeyPressed(VK_F11) == 0 &&
        pKeyboardInstance->WasKeyPressed(VK_F11)))
        return true;
    return false;
}

//----- (0044EEA7) --------------------------------------------------------
bool Engine::_44EEA7()
{
    //Game *v1; // esi@1
    //double v2; // st7@2
    float depth; // ST00_4@9
    //bool result; // eax@9
    //unsigned int v5; // eax@14
    __int64 v6; // kr00_8@21
    //unsigned int y; // [sp+4h] [bp-24h]@2
    //unsigned int x; // [sp+8h] [bp-20h]@2
    Vis_SelectionFilter *v10; // [sp+10h] [bp-18h]@2
    Vis_SelectionFilter *v11; // [sp+14h] [bp-14h]@2
    POINT cursor; // [sp+20h] [bp-8h]@1

    //v1 = this;
    ++qword_5C6DF0;
    pParticleEngine->UpdateParticles();
    pMouseInstance->GetCursorPos(&cursor);

    //x = cursor.y;
    //y = cursor.x;
    if (sub_4637E0_is_there_popup_onscreen())
    {
        v11 = &vis_face_filter;
        v10 = &vis_sprite_filter_2;
        depth = pIndoorCameraD3D->GetPickDepth();
    }
    else
    {
        if (uFlags2 & GAME_FLAGS_2_TARGETING_MODE)
        {
            v11 = &vis_face_filter;
            v10 = &vis_sprite_filter_1;
        }
        else
        {
            v11 = &vis_face_filter;
            v10 = &vis_sprite_filter_4;
        }
        depth = 5120.0;
    }
    //depth = v2;

    PickMouse(depth, cursor.x, cursor.y, false, v10, v11);
    pLightmapBuilder->StationaryLightsCount = 0;
    pLightmapBuilder->MobileLightsCount = 0;
    pDecalBuilder->DecalsCount = 0;
    pDecalBuilder->curent_decal_id = 0;
    if (!_44F07B())
        return false;

    if (uFlags & GAME_FLAGS_1_DRAW_BLV_DEBUGS)
        pStru10Instance->bDoNotDrawPortalFrustum = false;
    if ( /*render->pRenderD3D &&*/ uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        render->uFogColor = GetLevelFogColor() & 0xFFFFFF;
    if (uFlags & 0x0400)
        uFlags2 |= 0x01;
    /*if ( !render->pRenderD3D && uCurrentlyLoadedLevelType == LEVEL_Outdoor && pMobileLightsStack->uNumLightsActive )
    {
    uFlags2 |= 0x01;
    field_E10 = qword_5C6DF0;
    }*/
    v6 = qword_5C6DF0 - field_E10;
    if (qword_5C6DF0 - field_E10 == 1)
        uFlags2 |= v6;
    if (uNumStationaryLights_in_pStationaryLightsStack != pStationaryLightsStack->uNumLightsActive)
    {
        uFlags2 |= 1;
        uNumStationaryLights_in_pStationaryLightsStack = pStationaryLightsStack->uNumLightsActive;
    }
    return true;
}


//----- (0044EDE4) --------------------------------------------------------
bool Engine::AlterGamma_BLV(BLVFace *pFace, signed int *pColor)
{
    if (uFlags2 & GAME_FLAGS_2_SATURATE_LIGHTMAPS &&
        pFace->uAttributes & FACE_CAN_SATURATE_COLOR)
    {
        *pColor = ReplaceHSV(*pColor, 1.0, fSaturation, -1.0);
        return true;
    }
    else
        return false;
}

//----- (0044EE30) --------------------------------------------------------
bool Engine::AlterGamma_ODM(ODMFace *pFace, signed int *pColor)
{
    if (uFlags2 & GAME_FLAGS_2_SATURATE_LIGHTMAPS &&
        pFace->uAttributes & FACE_CAN_SATURATE_COLOR)
    {
        *pColor = ReplaceHSV(*pColor, 1.0, fSaturation, -1.0);
        return true;
    }
    else
        return false;
}


//----- (004645FA) --------------------------------------------------------
void Engine::Deinitialize()
{
    WriteWindowsRegistryInt("startinwindow", 1);//render->bWindowMode);
    //if (render->bWindowMode)
    {
        WriteWindowsRegistryInt("window X", window->GetX());
        WriteWindowsRegistryInt("window Y", window->GetY());
    }
    WriteWindowsRegistryInt("valAlwaysRun", bAlwaysRun);
    pItemsTable->Release();
    pNPCStats->Release();

    if (pMouse)
        pMouse->Deactivate();

    delete render;
    pAudioPlayer->Release();//error
    pNew_LOD->FreeSubIndexAndIO();
    pGames_LOD->FreeSubIndexAndIO();
    ClipCursor(0);
    Engine::Destroy();
    delete pEventTimer;
}

//----- (0044EE7C) --------------------------------------------------------
bool Engine::draw_debug_outlines()
{
    if (/*uFlags & 0x04*/ debug_lights)
    {
        pLightmapBuilder->DrawDebugOutlines(-1);
        pDecalBuilder->DrawDecalDebugOutlines();
    }
    return true;
}

//----- (0044EC23) --------------------------------------------------------
int Engine::_44EC23(struct Polygon *a2, int *a3, signed int a4)
{
    double v4; // st7@4
    //double v5; // ST00_8@4
    signed int v6; // eax@5
    //double v7; // ST00_8@6
    signed int result; // eax@8
    //double v9; // ST00_8@9
    //double v10; // ST00_8@11
    float a2a; // [sp+14h] [bp+8h]@4
    float a3a; // [sp+18h] [bp+Ch]@4
    float a3b; // [sp+18h] [bp+Ch]@6
    float a4a; // [sp+1Ch] [bp+10h]@9
    float a4b; // [sp+1Ch] [bp+10h]@11

    if (this->uFlags2 & 2 && a2->field_59 == 5 && a2->pODMFace->uAttributes & 2)
    {
        v4 = (double)a4;
        a2a = v4;
        *a3 |= 2u;
        a3a = (1.0 - this->fSaturation) * v4;
        //v5 = a3a + 6.7553994e15;
        //if ( SLODWORD(v5) >= 0 )
        if (floorf(a3a + 0.5f) >= 0)
        {
            a3b = (1.0 - this->fSaturation) * a2a;
            //v7 = a3b + 6.7553994e15;
            //v6 = LODWORD(v7);
            v6 = floorf(a3b + 0.5f);
        }
        else
            v6 = 0;
        if (a4 >= v6)
        {
            a4a = (1.0 - fSaturation) * a2a;
            //v9 = a4a + 6.7553994e15;
            //if ( SLODWORD(v9) >= 0 )
            if (floorf(a4a + 0.5f) >= 0)
            {
                a4b = (1.0 - fSaturation) * a2a;
                //v10 = a4b + 6.7553994e15;
                //result = LODWORD(v10);
                result = floorf(a4b + 0.5f);
            }
            else
                result = 0;
        }
        else
            result = a4;
    }
    else
        result = -1;
    return result;
}



//----- (00465C8B) --------------------------------------------------------
Engine *Engine::Create()
{
    return new Engine;
}

//----- (00465CF3) --------------------------------------------------------
void Engine::Destroy()
{
    delete pEngine;
    pEngine = nullptr;
}

//----- (0044ED0A) --------------------------------------------------------
signed int Engine::_44ED0A(BLVFace *a2, int *a3, signed int a4)
{
    double v4; // st7@3
    //double v5; // ST00_8@3
    signed int v6; // eax@4
    //double v7; // ST00_8@5
    signed int result; // eax@7
    //double v9; // ST00_8@8
    //double v10; // ST00_8@10
    float v11; // [sp+14h] [bp+8h]@3
    float v12; // [sp+18h] [bp+Ch]@3
    float v13; // [sp+18h] [bp+Ch]@5
    float v14; // [sp+1Ch] [bp+10h]@8
    float v15; // [sp+1Ch] [bp+10h]@10

    if (this->uFlags2 & 2 && a2->uAttributes & 2)
    {
        v4 = (double)a4;
        v11 = v4;
        *a3 |= 2u;
        v12 = (1.0 - this->fSaturation) * v4;
        //v5 = v12 + 6.7553994e15;
        if (floorf(v12 + 0.5f)/* SLODWORD(v5)*/ >= 0)
        {
            v13 = (1.0 - this->fSaturation) * v11;
            //v7 = v13 + 6.7553994e15;
            //v6 = LODWORD(v7);
            v6 = floorf(v13 + 0.5f);
        }
        else
            v6 = 0;
        if (a4 >= v6)
        {
            v14 = (1.0 - fSaturation) * v11;
            //v9 = v14 + 6.7553994e15;
            if (floorf(v14 + 0.5f)/* SLODWORD(v9)*/ >= 0)
            {
                v15 = (1.0 - fSaturation) * v11;
                //v10 = v15 + 6.7553994e15;
                //result = LODWORD(v10);
                result = floorf(v15 + 0.5f);
            }
            else
                result = 0;
        }
        else
            result = a4;
    }
    else
        result = -1;
    return result;
}


//----- (0044E4B7) --------------------------------------------------------
Engine::Engine()
{
    uNumStationaryLights = 0;
    uNumBloodsplats = 0;
    field_E0C = 0;
    field_E10 = 0;
    uNumStationaryLights_in_pStationaryLightsStack = 0;
    uFlags = 0;
    uFlags2 = 0;

    //pThreadWardInstance = new ThreadWard;
    pThreadWardInstance = nullptr;
    pParticleEngine = new ParticleEngine;
    pMouse = pMouseInstance = new Mouse;
    pLightmapBuilder = new LightmapBuilder;
    pVisInstance = new Vis;
    pStru6Instance = new stru6;
    pIndoorCameraD3D = new IndoorCameraD3D;
    pStru9Instance = new stru9;
    pStru10Instance = new stru10;
    //pStru11Instance = new stru11;
    pStru11Instance = nullptr;
    //pStru12Instance = new stru12(pStru11Instance);
    pStru12Instance = nullptr;
    //pCShow = new CShow;
    pCShow = nullptr;
    pKeyboardInstance = new Keyboard;
    //pGammaController = new GammaController;

    uFlags |= 0x0800;
    uFlags2 |= 0x24;

    _44F0FD();
}

//----- (0044E7F3) --------------------------------------------------------
Engine::~Engine()
{
    //delete pGammaController;
    delete pKeyboardInstance;
    /*delete pCShow;
    delete pStru12Instance;
    delete pStru11Instance;*/
    delete pStru10Instance;
    delete pStru9Instance;
    delete pIndoorCameraD3D;
    delete pStru6Instance;
    delete pVisInstance;
    delete pLightmapBuilder;
    delete pMouseInstance;
    delete pParticleEngine;
    //delete pThreadWardInstance;
}

//----- (0044EA5E) --------------------------------------------------------
bool Engine::PickMouse(float fPickDepth, unsigned int uMouseX, unsigned int uMouseY, bool bOutline, Vis_SelectionFilter *sprite_filter, Vis_SelectionFilter *face_filter)
{
    /*if (current_screen_type != SCREEN_GAME|| !render->pRenderD3D)
    return false;*/

    if (!pVisInstance)
    {
        MessageBoxW(nullptr, L"The 'Vis' object pointer has not been instatiated, but CGame::Pick() is trying to call through it.", nullptr, 0);
        return false;
    }

    if (uMouseX >= (signed int)pViewport->uScreen_TL_X &&
        uMouseX <= (signed int)pViewport->uScreen_BR_X &&
        uMouseY >= (signed int)pViewport->uScreen_TL_Y &&
        uMouseY <= (signed int)pViewport->uScreen_BR_Y)
    {
        pVisInstance->PickMouse(fPickDepth, uMouseX, uMouseY, sprite_filter, face_filter);

        if (bOutline)
            OutlineSelection();
    }

    return true;
}
// 4E28F8: using guessed type int current_screen_type;

//----- (0044EB12) --------------------------------------------------------
bool Engine::PickKeyboard(bool bOutline, Vis_SelectionFilter *sprite_filter, Vis_SelectionFilter *face_filter)
{
    if (current_screen_type == SCREEN_GAME && pVisInstance /*&& render->pRenderD3D*/)
    {
        bool r = pVisInstance->PickKeyboard(&pVisInstance->default_list, sprite_filter, face_filter);

        if (bOutline)
            OutlineSelection();
        return r;
    }
    return false;
}
/*
Result::Code Game::PickKeyboard(bool bOutline, struct unnamed_F93E6C *a3, struct unnamed_F93E6C *a4)
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
void Engine::OutlineSelection()
{
    if (!pVisInstance)
        return;

    if (!pVisInstance->default_list.uNumPointers)
        return;

    Vis_ObjectInfo* object_info = pVisInstance->default_list.object_pointers[0];
    if (object_info)
        switch (object_info->object_type)
    {
        case VisObjectType_Sprite:
        {
            Log::Warning(L"Sprite outline currently unsupported");
            return;
        }

        case VisObjectType_Face:
        {
            if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
            {
                ODMFace* face = (ODMFace *)object_info->object;
                if (face->uAttributes & FACE_OUTLINED)
                    face->uAttributes &= ~FACE_OUTLINED;
                else
                    face->uAttributes |= FACE_OUTLINED;
            }
            else if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
            {
                BLVFace* face = (BLVFace *)object_info->object;
                if (face->uAttributes & FACE_OUTLINED)
                    face->uAttributes &= ~FACE_OUTLINED;
                else
                    face->uAttributes |= FACE_OUTLINED;
            }
            else
                Error("Invalid level type", uCurrentlyLoadedLevelType);
        }
            break;

        default:
        {
            MessageBoxW(nullptr, L"Undefined CObjectInfo type requested in CGame::outline_selection()", nullptr, 0);
            ExitProcess(0);
        }
    }
}





//----- (0042FBDD) --------------------------------------------------------
void  sub_42FBDD()
{
    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0, 0, 0, 0);
    render->DrawTextureAlphaNew(pBtn_YES->uX/640.0f, pBtn_YES->uY/480.0f, pBtn_YES->pTextures[0]);
    render->Present();
}

//----- (0042FC15) --------------------------------------------------------
void CloseWindowBackground()
{
    pAudioPlayer->PlaySound(SOUND_StartMainChoice02, -2, 0, -1, 0, 0, 0, 0);
    render->DrawTextureAlphaNew(pBtn_ExitCancel->uX/640.0f, pBtn_ExitCancel->uY/480.0f, pBtn_ExitCancel->pTextures[0]);
    render->Present();
}


//----- (0046BDC0) --------------------------------------------------------
void  UpdateUserInput_and_MapSpecificStuff()
{
    if (dword_6BE364_game_settings_1 & GAME_SETTINGS_0080_SKIP_USER_INPUT_THIS_FRAME)
    {
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
void PrepareWorld(unsigned int _0_box_loading_1_fullscreen)
{
    //if ( render->pRenderD3D )
    pEngine->pVisInstance->_4C1A02();
    pEventTimer->Pause();
    pMiscTimer->Pause();
    pParty->uFlags = 2;
    CastSpellInfoHelpers::_427D48();
    ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
    DoPrepareWorld(0, (_0_box_loading_1_fullscreen == 0) + 1);
    pMiscTimer->Resume();
    pEventTimer->Resume();
}

//----- (00464866) --------------------------------------------------------
void DoPrepareWorld(unsigned int bLoading, int _1_fullscreen_loading_2_box)
{
    char *v3; // eax@1
    unsigned int v5; // eax@3
    char Str1[20]; // [sp+Ch] [bp-18h]@1

    //v9 = bLoading;
    ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
    pDecalBuilder->Reset(0);
    pGameLoadingUI_ProgressBar->Initialize(_1_fullscreen_loading_2_box == 1 ? GUIProgressBar::TYPE_Fullscreen :
        GUIProgressBar::TYPE_Box);
    strcpy(Str1, pCurrentMapName);
    v3 = strtok(Str1, ".");
    strcpy(Str1, v3);
    Level_LoadEvtAndStr(Str1);
    LoadLevel_InitializeLevelEvt();
    strcpy(Str1, pCurrentMapName);
    _strrev(Str1);
    strtok(Str1, ".");
    _strrev(Str1);

    for (uint i = 0; i < 1000; ++i)
        pSpriteObjects[i].uObjectDescID = 0;

    v5 = pMapStats->GetMapInfo(pCurrentMapName);
    bUnderwater = false;
    uLevelMapStatsID = v5;
    pEngine->uFlags2 &= 0xFFFFFFF7u;
    if (!_stricmp(pCurrentMapName, "out15.odm"))
    {
        bUnderwater = true;
        pEngine->uFlags2 |= GAME_FLAGS_2_ALTER_GRAVITY;
    }
    pParty->floor_face_pid = 0;
    if (_stricmp(Str1, "blv"))
        PrepareToLoadODM(bLoading, 0);
    else
        PrepareToLoadBLV(bLoading);
    pAudioPlayer->SetMapEAX();
    _461103_load_level_sub();
    if (!_stricmp(pCurrentMapName, "d11.blv") || !_stricmp(pCurrentMapName, "d10.blv"))
    {
        //spawning grounds & walls of mist - no loot & exp from monsters

        for (uint i = 0; i < uNumActors; ++i)
        {
            pActors[i].pMonsterInfo.uTreasureType = 0;
            pActors[i].pMonsterInfo.uTreasureDiceRolls = 0;
            pActors[i].pMonsterInfo.uExp = 0;
        }
    }
    bDialogueUI_InitializeActor_NPC_ID = 0;
    OnMapLoad();
    pGameLoadingUI_ProgressBar->Progress();
    memset(&render->pBillboardRenderListD3D, 0, sizeof(render->pBillboardRenderListD3D));
    pGameLoadingUI_ProgressBar->Release();
    _flushall();
}

void IntegrityTest()
{
    static_assert(sizeof(MovieHeader) == 44, "Wrong type size");
    static_assert(sizeof(SoundDesc_mm6) == 112, "Wrong type size");
    static_assert(sizeof(SoundDesc) == 120, "Wrong type size");
    static_assert(sizeof(OverlayDesc) == 8, "Wrong type size");
    static_assert(sizeof(ChestDesc) == 36, "Wrong type size");
    static_assert(sizeof(ObjectDesc_mm6) == 52, "Wrong type size");
    static_assert(sizeof(ObjectDesc) == 56, "Wrong type size");
    static_assert(sizeof(DecorationDesc) == 84, "Wrong type size");
    static_assert(sizeof(PlayerFrame) == 10, "Wrong type size");
    static_assert(sizeof(TextureFrame) == 20, "Wrong type size");
    static_assert(sizeof(SpriteFrame) == 60, "Wrong type size");
    static_assert(sizeof(RenderVertexSoft) == 0x30, "Wrong type size");
    static_assert(sizeof(RenderBillboard) == 0x34, "Wrong type size");
    //static_assert(sizeof(RGBTexture) == 0x28, "Wrong type size");
    //static_assert(sizeof(LODFile_IconsBitmaps) == 0x11BB8 + 4, "Wrong type size"); // + virtual dtor ptr
    static_assert(sizeof(AudioPlayer) == 0xC84, "Wrong type size");
    static_assert(sizeof(SoundDesc) == 0x78, "Wrong type size");
    static_assert(sizeof(stru339_spell_sound) == 0xAFD8, "Wrong type size");
    //static_assert(sizeof(VideoPlayer) == 0x108 + 4, "Wrong type size");
    static_assert(sizeof(MovieHeader) == 0x2C, "Wrong type size");
    static_assert(sizeof(DecorationDesc) == 0x54, "Wrong type size");
    static_assert(sizeof(ObjectDesc) == 0x38, "Wrong type size");
    static_assert(sizeof(OverlayDesc) == 0x8, "Wrong type size");
    static_assert(sizeof(ChestDesc) == 0x24, "Wrong type size");
    static_assert(sizeof(TileDesc) == 0x1A, "Wrong type size");
    static_assert(sizeof(MonsterDesc_mm6) == 148, "Wrong type size");
    static_assert(sizeof(MonsterDesc) == 152, "Wrong type size");
    static_assert(sizeof(Timer) == 0x28, "Wrong type size");
    static_assert(sizeof(OtherOverlay) == 0x14, "Wrong type size");
    static_assert(sizeof(ItemGen) == 0x24, "Wrong type size");
    static_assert(sizeof(SpriteObject) == 0x70, "Wrong type size");
    static_assert(sizeof(ItemDesc) == 0x30, "Wrong type size");
    static_assert(sizeof(ItemsTable) == 0x117A0, "Wrong type size");
    static_assert(sizeof(Chest) == 0x14CC, "Wrong type size");
    static_assert(sizeof(MapInfo) == 0x44, "Wrong type size");
    static_assert(sizeof(SpellInfo) == 0x24, "Wrong type size");
    static_assert(sizeof(SpellData) == 0x14, "Wrong type size");
    static_assert(sizeof(SpellBuff) == 0x10, "Wrong type size");
    static_assert(sizeof(AIDirection) == 0x1C, "Wrong type size");
    static_assert(sizeof(ActorJob) == 0xC, "Wrong type size");
    static_assert(sizeof(Actor) == 0x344, "Wrong type size");
    static_assert(sizeof(LevelDecoration) == 0x20, "Wrong type size");
    static_assert(sizeof(KeyboardActionMapping) == 0x20C, "Wrong type size");
    //static_assert(sizeof(UIAnimation) == 0xD, "Wrong type size");
    //static_assert(sizeof(SpawnPointMM7) == 0x18, "Wrong type size");
    static_assert(sizeof(ODMFace) == 0x134, "Wrong type size");
    static_assert(sizeof(BSPNode) == 0x8, "Wrong type size");
    static_assert(sizeof(BSPModel) == 0xBC, "Wrong type size");
    //static_assert(sizeof(OutdoorLocation) == 0x1C28C, "Wrong type size");
    static_assert(sizeof(BLVFace) == 0x60, "Wrong type size");
    static_assert(sizeof(BLVFaceExtra) == 0x24, "Wrong type size");
    static_assert(sizeof(BLVSector) == 0x74, "Wrong type size");
    static_assert(sizeof(BLVLightMM7) == 0x10, "Wrong type size");
    static_assert(sizeof(BLVDoor) == 0x50, "Wrong type size");
    //static_assert(sizeof(IndoorLocation) == 0x690, "Wrong type size");
    //static_assert(sizeof(ODMRenderParams) == 0x74, "Wrong type size");
    static_assert(sizeof(Mouse) == 0x114, "Wrong type size");
    static_assert(sizeof(Particle_sw) == 0x68, "Wrong type size");
    static_assert(sizeof(Particle) == 0x68, "Wrong type size");
    static_assert(sizeof(ParticleEngine) == 0xE430, "Wrong type size");
    static_assert(sizeof(Lightmap) == 0xC1C, "Wrong type size");
    static_assert(sizeof(LightmapBuilder) == 0x3CBC38, "Wrong type size");
    static_assert(sizeof(Vis_SelectionList) == 0x2008, "Wrong type size");
    static_assert(sizeof(Vis) == 0x20D0, "Wrong type size");
    static_assert(sizeof(PlayerBuffAnim) == 0x10, "Wrong type size");
    static_assert(sizeof(ProjectileAnim) == 0x1C, "Wrong type size");
    static_assert(sizeof(stru6) == 0x5F8, "Wrong type size");
    static_assert(sizeof(IndoorCameraD3D_Vec3) == 0x10, "Wrong type size");
    static_assert(sizeof(IndoorCameraD3D_Vec4) == 0x18, "Wrong type size"); //should be 14 (10 vec3 + 4 vdtor)  but 18 coz of his +4 from own vdtor, but it is odd since vdtor already present from vec3
    //static_assert(sizeof(IndoorCameraD3D) == 0x1A1384, "Wrong type size");
    static_assert(sizeof(StationaryLight) == 0xC, "Wrong type size");
    static_assert(sizeof(LightsStack_StationaryLight_) == 0x12C8, "Wrong type size");
    static_assert(sizeof(MobileLight) == 0x12, "Wrong type size");
    static_assert(sizeof(LightsStack_MobileLight_) == 0x1C28, "Wrong type size");
    static_assert(sizeof(Engine) == 0xE78, "Wrong type size");
    static_assert(sizeof(stru141_actor_collision_object) == 0xA8, "Wrong type size");
    static_assert(sizeof(ActionQueue) == 0x7C, "Wrong type size");
    static_assert(sizeof(NPCData) == 0x4C, "Wrong type size");
    static_assert(sizeof(NPCStats) == 0x17FFC, "Wrong type size");
    static_assert(sizeof(BspRenderer) == 0x53740, "Wrong type size");
    static_assert(sizeof(PaletteManager) == 0x267AF0, "Wrong type size");
    static_assert(sizeof(ViewingParams) == 0x26C, "Wrong type size");
    //static_assert(sizeof(IndoorCamera) == 0x50, "Wrong type size");
    static_assert(sizeof(Bloodsplat) == 0x28, "Wrong type size");
    static_assert(sizeof(BloodsplatContainer) == 0xA0C, "Wrong type size");
    static_assert(sizeof(TrailParticle) == 0x18, "Wrong type size");
    static_assert(sizeof(EventIndex) == 0xC, "Wrong type size");
    static_assert(sizeof(_2devent) == 0x34, "Wrong type size");
    static_assert(sizeof(MapsLongTimer) == 0x20, "Wrong type size");
    static_assert(sizeof(SavegameHeader) == 0x64, "Wrong type size");
    static_assert(sizeof(SavegameList) == 0x3138, "Wrong type size");
    static_assert(sizeof(StorylineText) == 0x160, "Wrong type size");
    static_assert(sizeof(FactionTable) == 0x1EF1, "Wrong type size");
    static_assert(sizeof(Decal) == 0xC20, "Wrong type size");
    static_assert(sizeof(DecalBuilder) == 0x30C038, "Wrong type size");
    static_assert(sizeof(MonsterInfo) == 0x58, "Wrong type size");
    static_assert(sizeof(MonsterStats) == 0x5BA0, "Wrong type size");
    static_assert(sizeof(RenderD3D) == 0x148, "Wrong type size");
    //  static_assert(sizeof(Render) == 0x129844, "Wrong type size");
    static_assert(sizeof(Player) == 0x1B3C, "Wrong type size");
    static_assert(sizeof(PartyTimeStruct) == 0x678, "Wrong type size");
    static_assert(sizeof(Party) == 0x16238, "Wrong type size");
    //static_assert(sizeof(GUIButton) == 0xBC, "Wrong type size");
    //static_assert(sizeof(GUIWindow) == 0x54, "Wrong type size");
    //static_assert(sizeof(GUIProgressBar) == 0x1B8, "Wrong type size");
    static_assert(sizeof(GUIFont) == 0x1020, "Wrong type size");
    // static_assert(sizeof(stru262_TurnBased) == 0x40, "Wrong type size");
    //static_assert(sizeof(ArcomageGame) == 0xFB, "Wrong type size");
    static_assert(sizeof(CastSpellInfo) == 0x14, "Wrong type size");
    static_assert(sizeof(ArcomageCard) == 0x6C, "Wrong type size");
    static_assert(sizeof(LightsData) == 0x3FC, "Wrong type size");
    static_assert(sizeof(TravelInfo) == 0x20, "Wrong type size");
    static_assert(sizeof(stru336) == 0x798, "Wrong type size");
    static_assert(sizeof(Vec3_short_) == 6, "Wrong type size");
    static_assert(sizeof(BLVFace) == 96, "Wrong type size");
    static_assert(sizeof(BLVFaceExtra) == 36, "Wrong type size");
    static_assert(sizeof(BLVSector) == 116, "Wrong type size");
    static_assert(sizeof(LevelDecoration) == 32, "Wrong type size");
    static_assert(sizeof(BLVLightMM7) == 16, "Wrong type size");
    static_assert(sizeof(BSPNode) == 8, "Wrong type size");
    //static_assert(sizeof(SpawnPointMM7) == 24, "Wrong type size");
    static_assert(sizeof(DDM_DLV_Header) == 40, "Wrong type size");
    static_assert(sizeof(Actor) == 836, "Wrong type size");
    static_assert(sizeof(SpriteObject) == 112, "Wrong type size");
    static_assert(sizeof(Chest) == 5324, "Wrong type size");
    static_assert(sizeof(stru123) == 0xC8, "Wrong type size");
    static_assert(sizeof(BLVMapOutline) == 12, "Wrong type size");
    static_assert(sizeof(LODSprite) == 0x28, "Wrong type size");
}


//----- (004647AB) --------------------------------------------------------
void FinalInitialization()
{
    pViewport->SetScreen(viewparams->uSomeX, viewparams->uSomeY, viewparams->uSomeZ, viewparams->uSomeW);
    pViewport->SetFOV(flt_6BE3A0 * 65536.0f);

    //pIndoorCamera = new IndoorCamera;
    //pIndoorCamera->Initialize(65, viewparams->uScreen_BttmR_X - viewparams->uScreen_topL_X + 1,
    //                              viewparams->uScreen_BttmR_Y - viewparams->uScreen_topL_Y + 1);

    InitializeTurnBasedAnimations(&stru_50C198);
    pBitmaps_LOD->_inlined_sub1();
    pSprites_LOD->_inlined_sub1();
    pIcons_LOD->_inlined_sub1();
}
// 6BE3A0: using guessed type float flt_6BE3A0;



//----- (00464E17) --------------------------------------------------------
bool CheckMM7CD(char c)
{
    char DstBuf[256] = { 0 };
    char strCommand[256] = { 0 }; // [sp+10Ch] [bp-118h]@1
    char Filename[20] = { 0 }; // [sp+20Ch] [bp-18h]@1

    wchar_t pMagicPath[1024];
    swprintf(pMagicPath, wcslen(L"%C:\\anims\\magic7.vid"), L"%C:\\anims\\magic7.vid", c);
    if (GetFileAttributesW(pMagicPath) == -1)
        return false;

    //Open CD audio
    wsprintfA(strCommand, "open %c: type cdaudio alias CD", c);
    if (!mciSendStringA(strCommand, DstBuf, 255, 0))
    {
        wsprintfA(strCommand, "info CD UPC wait");
        mciSendStringA(strCommand, DstBuf, 255, 0);
        wsprintfA(strCommand, "close CD");
        mciSendStringA(strCommand, DstBuf, 255, 0);
    }

    memcpy(Filename, "X:\\anims\\magic7.vid", sizeof(Filename));
    *Filename = c;

    FILE* f = fopen(Filename, "rb");
    if (!f)
        return false;

    if (!fseek(f, 0, SEEK_END))
    {
        if (!fseek(f, -100, SEEK_CUR))
            fread(DstBuf, 1, 0x64u, f);

        fclose(f);
        return true;
    }
    fclose(f);
    return false;
}

//----- (00464F1B) --------------------------------------------------------
signed int __stdcall InsertMM7CDDialogFunc(HWND hDlg, int a2, __int16 a3, int a4)
{
    char v4; // zf@3
    int v6; // eax@10
    int v7; // eax@11
    int v8; // eax@12
    int v9; // eax@13
    BOOL(__stdcall *v10)(HWND, int, LPCSTR); // edi@15
    const CHAR *v11; // [sp-Ch] [bp-Ch]@15
    INT_PTR v12; // [sp-4h] [bp-4h]@5

    if (a2 == 272)
    {
        hInsertCDWindow = hDlg;
        v6 = (GetUserDefaultLangID() & 0x3FF) - 7;
        if (v6)
        {
            v7 = v6 - 3;
            if (v7)
            {
                v8 = v7 - 2;
                if (v8)
                {
                    v9 = v8 - 4;
                    if (v9)
                    {
                        if (v9 != 5)
                            return 0;
                        SetWindowTextA(hDlg, "Wloz CD-ROM numer 2");
                        v10 = SetDlgItemTextA;
                        SetDlgItemTextA(hDlg, 1010, "Wloz CD-ROM numer 2 Might and Magic® VII.");
                        v11 = "Odwolaj";
                    }
                    else
                    {
                        SetWindowTextA(hDlg, "Inserire il secondo CD");
                        v10 = SetDlgItemTextA;
                        SetDlgItemTextA(hDlg, 1010, "Inserire il secondo CD di Might and Magic® VII.");
                        v11 = "Annulla";
                    }
                }
                else
                {
                    SetWindowTextA(hDlg, "Insйrez le CD 2");
                    v10 = SetDlgItemTextA;
                    SetDlgItemTextA(hDlg, 1010, "Insйrez Might & Magic® VII CD 2.");
                    v11 = "Supprimer";
                }
            }
            else
            {
                SetWindowTextA(hDlg, "Por favor, inserte disco 2");
                v10 = SetDlgItemTextA;
                SetDlgItemTextA(hDlg, 1010, "Por favor, inserte disco 2 de Might & Magic® VII.");
                v11 = "Cancelar";
            }
        }
        else
        {
            SetWindowTextA(hDlg, "Bitte CD 2 einlegen");
            v10 = SetDlgItemTextA;
            SetDlgItemTextA(hDlg, 1010, "Bitte CD 2 von Might and Magic® VII einlegen.");
            v11 = "Abbrechen";
        }
        v10(hDlg, 2, v11);
        return 0;
    }
    if (a2 == 273)
    {
        if (a3 == 2)
        {
            v12 = 0;
            EndDialog(hDlg, v12);
            return 1;
        }
        v4 = a3 == 1;
    }
    else
    {
        v4 = a2 == 1025;
    }
    if (v4)
    {
        v12 = 1;
        EndDialog(hDlg, v12);
        return 1;
    }
    return 0;
}

//----- (00465061) --------------------------------------------------------
bool FindMM7CD(HWND hWnd, char *pCDDrive)
{
    char drive[4] = { 'X', ':', '\\', 0 };

    bool bGotCDFromRegistry = false;

    HKEY hSoftware = nullptr,
        hNWC = nullptr,
        hMM7 = nullptr,
        hVersion = nullptr;
    if (!RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE", 0, KEY_READ | KEY_WOW64_32KEY, &hSoftware))
    {
        if (!RegOpenKeyExA(hSoftware, "New World Computing", 0, KEY_READ | KEY_WOW64_32KEY, &hNWC))
        {
            if (!RegOpenKeyExA(hNWC, "Might and Magic VII", 0, KEY_READ | KEY_WOW64_32KEY, &hMM7))
            {
                if (!RegOpenKeyExA(hMM7, "1.0", 0, KEY_READ | KEY_WOW64_32KEY, &hVersion))
                {
                    DWORD cbData = 3;
                    if (!RegQueryValueExA(hVersion, "CDDrive", 0, 0, (BYTE *)drive, &cbData))
                        bGotCDFromRegistry = true;
                }
                RegCloseKey(hVersion);
            }
            RegCloseKey(hMM7);
        }
        RegCloseKey(hNWC);
    }
    RegCloseKey(hSoftware);

    if (bGotCDFromRegistry)
        if (CheckMM7CD(*drive))
        {
        cMM7GameCDDriveLetter = *drive;
        return true;
        }

    while (true)
    {
        for (uint i = 0; i < 26; ++i)
        {
            drive[0] = 'A' + i;

            if (GetDriveTypeA(drive) == DRIVE_CDROM)
                if (CheckMM7CD(*drive))
                {
                cMM7GameCDDriveLetter = *drive;
                WriteWindowsRegistryString("CDDrive", drive);
                return true;
                }
        }

        if (DialogBoxParamA(GetModuleHandleW(nullptr), "InsertCD", hWnd, (DLGPROC)InsertMM7CDDialogFunc, 0))
            continue;
        return false;
    }
}

//----- (004651F4) --------------------------------------------------------
bool MM7_Initialize(int game_width, int game_height, const char *mm7_path)
{
    wchar_t pCurrentDir[1024];
    _wgetcwd(pCurrentDir, 1024);

    wchar_t pMM6IniFile[1024];
    wsprintfW(pMM6IniFile, L"%s\\mm6.ini", pCurrentDir);

    bCanLoadFromCD = GetPrivateProfileIntW(L"settings", L"use_cd", 1, pMM6IniFile);
    if (bNoCD)
        bCanLoadFromCD = false;
    if (bCanLoadFromCD)
    {
        Log::Warning(L"Checking for CD...");
        if (!FindMM7CD(nullptr, &cMM7GameCDDriveLetter))
            return false;
        Log::Warning(L"...done.");
    }


    srand(GetTickCount());

    pEventTimer = Timer::Create();
    pEventTimer->Initialize();
    window = OSWindow::Create(L"Might and Magic® Trilogy", game_width, game_height);//Create  game window

    bool use_d3d11 = false;
    if (use_d3d11)
        render = RenderD3D11::Create();
    else
        render = Render::Create();//Create DirectX
    if (!render)
    {
        Log::Warning(L"Render creation failed");
        return false;
    }
    else
    {
        //bool bWindowMode = ReadWindowsRegistryInt("startinwindow", false);
        //uint uDefaultDevice = ReadWindowsRegistryInt("D3D Device", 1);

        if (!render->Initialize(window/*, bColoredLights, uLevelOfDetail, bTinting*/))
        {
            Log::Warning(L"Render failed to initialize");
            return false;
        }
    }

    game_starting_year = 1168;

    pParty = new Party;
    memset(&pParty->pHirelings, 0, sizeof(pParty->pHirelings));
    pParty->uWalkSpeed = GetPrivateProfileIntW(L"debug", L"walkspeed", 384, pMM6IniFile);
    pParty->uDefaultEyelevel = GetPrivateProfileIntW(L"party", L"eyelevel", 160, pMM6IniFile);
    pParty->sEyelevel = pParty->uDefaultEyelevel;
    pParty->uDefaultPartyHeight = GetPrivateProfileIntW(L"party", L"height", 192, pMM6IniFile);
    pParty->uPartyHeight = pParty->uDefaultPartyHeight;

    MM6_Initialize(pMM6IniFile);

    pKeyActionMap = new KeyboardActionMapping;

    OnTimer(1);
    GameUI_StatusBar_Update(true);
    pEngine = Engine::Create();
    pMouse = pEngine->pMouseInstance;

    char filename[2048];

    pIcons_LOD = new LODFile_IconsBitmaps;
    sprintf(filename, "%s\\data\\icons.lod", mm7_path);
    if (!pIcons_LOD->Load(filename, "icons"))
    {
        MessageBoxW(nullptr, L"Some files are missing\n\nPlease Reinstall.",
            L"Files Missing", MB_ICONEXCLAMATION);
        return false;
    }
    pIcons_LOD->_011BA4_debug_paletted_pixels_uncompressed = false;

    pEvents_LOD = new LODFile_IconsBitmaps;
    sprintf(filename, "%s\\data\\events.lod", mm7_path);
    if (!pEvents_LOD->Load(filename, "icons"))
    {
        MessageBoxW(nullptr, L"Some files are missing\n\nPlease Reinstall.",
            L"Files Missing", MB_ICONEXCLAMATION);
        return false;
    }

    localization = new Localization();
    localization->Initialize();

    pBitmaps_LOD = new LODFile_IconsBitmaps;
    sprintf(filename, "%s\\data\\bitmaps.lod", mm7_path);
    if (!pBitmaps_LOD->Load(filename, "bitmaps"))
    {
        MessageBoxA(nullptr, localization->GetString(63), localization->GetString(184), MB_ICONEXCLAMATION);
        return false;
    }

    pSprites_LOD = new LODFile_Sprites;
    sprintf(filename, "%s\\data\\sprites.lod", mm7_path);
    if (!pSprites_LOD->LoadSprites(filename))
    {
        MessageBoxA(nullptr, localization->GetString(63), localization->GetString(184), MB_ICONEXCLAMATION);
        return false;
    }


#if 0
    if (_access("../MM_VI/data/icons.lod", 0) == 0)
    {
        pIcons_LOD_mm6 = new LODFile_IconsBitmaps;
        if (!pIcons_LOD_mm6->Load("../MM_VI/data/icons.lod", "icons"))
        {
            delete pIcons_LOD_mm6;
            pIcons_LOD_mm6 = nullptr;
            Log::Warning(L"Unable to load mm6:icons.lod");
        }
    }
    else
        Log::Warning(L"Unable to find mm6:icons.lod");

    if (_access("../MM_VI/data/bitmaps.lod", 0) == 0)
    {
        pBitmaps_LOD_mm6 = new LODFile_IconsBitmaps;
        if (!pBitmaps_LOD_mm6->Load("../MM_VI/data/bitmaps.lod", "bitmaps"))
        {
            delete pBitmaps_LOD_mm6;
            pBitmaps_LOD_mm6 = nullptr;
            Log::Warning(L"Unable to load mm6:bitmaps.lod");
        }
    }
    else
        Log::Warning(L"Unable to find mm6:bitmaps.lod");

    auto mm6_sprite_container_name = bUseLoResSprites ? "../MM_VI/data/spriteLO.lod"
        : "../MM_VI/data/sprites.lod";
    if (_access(mm6_sprite_container_name, 0) == 0)
    {
        pSprites_LOD_mm6 = new LODFile_Sprites;
        if (!pSprites_LOD_mm6->LoadSprites(mm6_sprite_container_name))
        {
            delete pSprites_LOD_mm6;
            pSprites_LOD_mm6 = nullptr;
            Log::Warning(L"Unable to load mm6:sprites.lod");
        }
    }
    else
        Log::Warning(L"Unable to find mm6:sprites.lod");


    if (_access("../mm8/data/icons.lod", 0) == 0)
    {
        pIcons_LOD_mm8 = new LODFile_IconsBitmaps;
        if (!pIcons_LOD_mm8->Load("../mm8/data/icons.lod", "icons"))
        {
            delete pIcons_LOD_mm8;
            pIcons_LOD_mm8 = nullptr;
            Log::Warning(L"Unable to load mm8:icons.lod");
        }
    }
    else
        Log::Warning(L"Unable to find mm8:icons.lod");


    if (_access("../mm8/data/bitmaps.lod", 0) == 0)
    {
        pBitmaps_LOD_mm8 = new LODFile_IconsBitmaps;
        if (!pBitmaps_LOD_mm8->Load("../mm8/data/bitmaps.lod", "bitmaps"))
        {
            delete pBitmaps_LOD_mm8;
            pBitmaps_LOD_mm8 = nullptr;
            Log::Warning(L"Unable to load mm8:bitmaps.lod");
        }
    }
    else
        Log::Warning(L"Unable to find mm8:bitmaps.lod");


    if (_access("../mm8/data/sprites.lod", 0) == 0)
    {
        pSprites_LOD_mm8 = new LODFile_Sprites;
        if (!pSprites_LOD_mm8->LoadSprites("../mm8/data/sprites.lod"))
        {
            delete pSprites_LOD_mm8;
            pSprites_LOD_mm8 = nullptr;
            Log::Warning(L"Unable to load mm8:sprites.lod");
        }
    }
    else
        Log::Warning(L"Unable to find mm8:sprites.lod");
#endif

    {
        void *sft_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadRaw("dsft.bin", 1) : nullptr,
            *sft_mm8 = nullptr;
        void *sft_mm7 = pEvents_LOD->LoadRaw("dsft.bin", 1);
        pSpriteFrameTable = new SpriteFrameTable;
        pSpriteFrameTable->FromFile(sft_mm6, sft_mm7, sft_mm8);
        free(sft_mm6);
        free(sft_mm7);
        free(sft_mm8);

        void *tft_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadRaw("dtft.bin", 1) : nullptr,
            *tft_mm8 = nullptr;
        void *tft_mm7 = pEvents_LOD->LoadRaw("dtft.bin", 1);
        pTextureFrameTable = new TextureFrameTable;
        pTextureFrameTable->FromFile(tft_mm6, tft_mm7, tft_mm8);
        free(tft_mm6);
        free(tft_mm7);
        free(tft_mm8);

        void *tiles_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadRaw("dtile.bin", 1) : nullptr,
            *tiles_mm8 = nullptr;
        void *tiles_mm7 = pEvents_LOD->LoadRaw("dtile.bin", 1);
        pTileTable = new TileTable;
        pTileTable->FromFile(tiles_mm6, tiles_mm7, tiles_mm8);
        free(tiles_mm6);
        free(tiles_mm7);
        free(tiles_mm8);

        void *pft_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadRaw("dpft.bin", 1) : nullptr,
            *pft_mm8 = nullptr;
        void *pft_mm7 = pEvents_LOD->LoadRaw("dpft.bin", 1);
        pPlayerFrameTable = new PlayerFrameTable;
        pPlayerFrameTable->FromFile(pft_mm6, pft_mm7, pft_mm8);
        free(pft_mm6);
        free(pft_mm7);
        free(pft_mm8);

        void *ift_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadRaw("dift.bin", 1) : nullptr,
            *ift_mm8 = nullptr;
        void *ift_mm7 = pEvents_LOD->LoadRaw("dift.bin", 1);
        pIconsFrameTable = new IconFrameTable;
        pIconsFrameTable->FromFile(ift_mm6, ift_mm7, ift_mm8);
        free(ift_mm6);
        free(ift_mm7);
        free(ift_mm8);

        void *decs_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadRaw("ddeclist.bin", 1) : nullptr,
            *decs_mm8 = nullptr;
        void *decs_mm7 = pEvents_LOD->LoadRaw("ddeclist.bin", 1);
        pDecorationList = new DecorationList;
        pDecorationList->FromFile(decs_mm6, decs_mm7, decs_mm8);
        free(decs_mm6);
        free(decs_mm7);
        free(decs_mm8);

        void *objs_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadRaw("dobjlist.bin", 1) : nullptr,
            *objs_mm8 = nullptr;
        void *objs_mm7 = pEvents_LOD->LoadRaw("dobjlist.bin", 1);
        pObjectList = new ObjectList;
        pObjectList->FromFile(objs_mm6, objs_mm7, objs_mm8);
        free(objs_mm6);
        free(objs_mm7);
        free(objs_mm8);

        void *mons_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadRaw("dmonlist.bin", 1) : nullptr,
            *mons_mm8 = nullptr;
        void *mons_mm7 = pEvents_LOD->LoadRaw("dmonlist.bin", 1);
        pMonsterList = new MonsterList;
        pMonsterList->FromFile(mons_mm6, mons_mm7, mons_mm8);
        free(mons_mm6);
        free(mons_mm7);
        free(mons_mm8);

        void *chests_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadRaw("dchest.bin", 1) : nullptr,
            *chests_mm8 = nullptr;
        void *chests_mm7 = pEvents_LOD->LoadRaw("dchest.bin", 1);
        pChestList = new ChestList;
        pChestList->FromFile(chests_mm6, chests_mm7, chests_mm8);
        free(chests_mm6);
        free(chests_mm7);
        free(chests_mm8);

        void *overlays_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadRaw("doverlay.bin", 1) : nullptr,
            *overlays_mm8 = nullptr;
        void *overlays_mm7 = pEvents_LOD->LoadRaw("doverlay.bin", 1);
        pOverlayList = new OverlayList;
        pOverlayList->FromFile(overlays_mm6, overlays_mm7, overlays_mm8);
        free(overlays_mm6);
        free(overlays_mm7);
        free(overlays_mm8);

        void *sounds_mm6 = pIcons_LOD_mm6 ? pIcons_LOD_mm6->LoadRaw("dsounds.bin", 1) : nullptr,
            *sounds_mm8 = nullptr;
        void *sounds_mm7 = pEvents_LOD->LoadRaw("dsounds.bin", 1);
        pSoundList = new SoundList;
        pSoundList->FromFile(sounds_mm6, sounds_mm7, sounds_mm8);
        free(sounds_mm6);
        free(sounds_mm7);
        free(sounds_mm8);
    }




    if (dword_6BE368_debug_settings_2 & DEBUG_SETTINGS_RUN_IN_WIDOW)
    {
        //window->SetWindowedMode(game_width, game_height);
        render->SwitchToWindow();
    }
    else
    {
        __debugbreak(); // Nomad
        window->SetFullscreenMode();
        render->InitializeFullscreen();
    }

    uSoundVolumeMultiplier = min(9, ReadWindowsRegistryInt("soundflag", 9));
    uMusicVolimeMultiplier = min(9, ReadWindowsRegistryInt("musicflag", 9));
    uVoicesVolumeMultiplier = min(9, ReadWindowsRegistryInt("CharVoices", 9));
    bShowDamage = ReadWindowsRegistryInt("ShowDamage", 1) != 0;

    uGammaPos = min(4, ReadWindowsRegistryInt("GammaPos", 4));
    //pEngine->pGammaController->Initialize(uGammaPos * 0.1 + 0.6);

    if (ReadWindowsRegistryInt("Bloodsplats", 1))
        pEngine->uFlags2 |= GAME_FLAGS_2_DRAW_BLOODSPLATS;
    else
        pEngine->uFlags2 &= ~GAME_FLAGS_2_DRAW_BLOODSPLATS;

    uTurnSpeed = ReadWindowsRegistryInt("TurnDelta", 3);

    if (!bNoSound)
        pAudioPlayer->Initialize();

    pMediaPlayer = new Media::MPlayer();
    pMediaPlayer->Initialize(window);

    dword_6BE364_game_settings_1 |= GAME_SETTINGS_4000;

    switch (uTurnSpeed)
    {
    case 0: // undefined turn option
        __debugbreak(); // really shouldn't use this mode
        uTurnSpeed = 64; //(unsigned int)uCPUSpeed < 199/*MHz*/ ? 128 : 64; // adjust turn speed to estimated fps
        break;

    case 1:             // 16x
        Log::Warning(L"x16 Turn Speed"); // really shouldn't use this mode
        uTurnSpeed = 128;
        break;

    case 2:             // 32x
        Log::Warning(L"x32 Turn Speed"); // really shouldn't use this mode
        uTurnSpeed = 64;
        break;

    case 3:             // smooth
        uTurnSpeed = 0;
        break;
    }

    return true;
}

//----- (00465D0B) --------------------------------------------------------
void SecondaryInitialization()
{
    pMouse->Initialize(window);

    pItemsTable = new ItemsTable;
    pItemsTable->Initialize();

    //pBitmaps_LOD->can_load_hardware_sprites = 1;
    //pBitmaps_LOD->SetupPalettes(render->uTargetRBits, render->uTargetGBits, render->uTargetBBits);
    pBitmaps_LOD->SetupPalettes(5, 6, 5);
    //pIcons_LOD->SetupPalettes(render->uTargetRBits, render->uTargetGBits, render->uTargetBBits);
    pIcons_LOD->SetupPalettes(5, 6, 5);
    //pPaletteManager->SetColorChannelInfo(render->uTargetRBits, render->uTargetGBits, render->uTargetBBits);
    pPaletteManager->SetColorChannelInfo(5, 6, 5);

    pPaletteManager->SetMistColor(128, 128, 128);
    pPaletteManager->RecalculateAll();
    //pSprites_LOD->can_load_hardware_sprites = 1;
    pObjectList->InitializeSprites();
    pOverlayList->InitializeSprites();

    if (!bNoSound)
        pSoundList->Initialize();


    for (uint i = 0; i < 4; ++i)
    {
        static const char *pUIAnimNames[4] =
        {
            "glow03", "glow05",
            "torchA", "wizeyeA"
        };
        static unsigned short _4E98D0[4][4] =
        {
            { 479, 0, 329, 0 },
            { 585, 0, 332, 0 },
            { 468, 0, 0, 0 },
            { 606, 0, 0, 0 }
        };

        //pUIAnims[i]->uIconID = pIconsFrameTable->FindIcon(pUIAnimNames[i]);
        pUIAnims[i]->icon = pIconsFrameTable->GetIcon(pUIAnimNames[i]);
        pIconsFrameTable->InitializeAnimation(pUIAnims[i]->icon->id);

        pUIAnims[i]->uAnimLength = 0;
        pUIAnims[i]->uAnimTime = 0;
        pUIAnims[i]->x = _4E98D0[i][0];
        pUIAnims[i]->y = _4E98D0[i][2];
    }



    for (unsigned int i = 0; i < pObjectList->uNumObjects; ++i)
    {
        pObjectList->pObjects[i].uParticleTrailColor = pObjectList->pObjects[i].uParticleTrailColorB |
            ((unsigned int)pObjectList->pObjects[i].uParticleTrailColorG << 8) |
            ((unsigned int)pObjectList->pObjects[i].uParticleTrailColorR << 16);
    }

    MainMenuUI_Create();
    pEngine->pStru6Instance->LoadAnimations();

    for (uint i = 0; i < 7; ++i)
    {
        char container_name[64];
        sprintf(container_name, "HDWTR%03u", i);
        render->pHDWaterBitmapIDs[i] = pBitmaps_LOD->LoadTexture(container_name);
    }

    pNPCStats = new NPCStats;
    memset(pNPCStats->pNPCData, 0, 0x94BCu);
    pNPCStats->Initialize();

    Initialize_GlobalEVT();
    pBitmaps_LOD->_inlined_sub0();
    pSprites_LOD->_inlined_sub0();
    pPaletteManager->LockAll();

    _mkdir("Saves");
    for (uint i = 0; i < 5; ++i)
        for (uint j = 0; j < 6; ++j)
        {
            remove(
                StringPrintf("data\\lloyd%d%d.pcx", i, j).c_str()
            );
        }

    Initialize_GamesLOD_NewLOD();
    _576E2C_current_minimap_zoom = 512;
    dword_576E28 = 9;
}

int max_flight_height = 4000;    //maximum altitude
bool use_MMT = false;
bool use_music_folder = false;
bool for_refactoring = false;
bool all_spells = true;
bool bNoMargareth = false;

void ParseCommandLine(const wchar_t *cmd)
{
    //if (wcsstr(pCmdLine, L"-usedefs"))
    //  bDebugResouces = 1;
    if (wcsstr(cmd, L"-window"))
        dword_6BE368_debug_settings_2 |= DEBUG_SETTINGS_RUN_IN_WIDOW;

    if (wcsstr(cmd, L"-nointro"))
        bNoIntro = true;//dword_6BE364_game_settings_1 |= 4;
    if (wcsstr(cmd, L"-nologo"))
        bNoLogo = true;//dword_6BE364_game_settings_1 |= 8;
    if (wcsstr(cmd, L"-nosound"))
        bNoSound = true; //dword_6BE364_game_settings_1 |= 0x10;

    bWalkSound = ReadWindowsRegistryInt("WalkSound", 1) != 0;
    if (wcsstr(cmd, L"-nowalksound"))
        bWalkSound = false;//dword_6BE364_game_settings_1 |= 0x20;
    if (wcsstr(cmd, L"-novideo"))
    {
        dword_6BE364_game_settings_1 |= GAME_SETTINGS_NO_HOUSE_ANIM;
        bNoVideo = true;
    }
    if (wcsstr(cmd, L"-nocd"))
        bNoCD = true;
    if (wcsstr(cmd, L"-nomarg"))
        bNoMargareth = true;
}


bool GameLoop()
{
    while (1)
    {
        if (uGameState == GAME_FINISHED || GetCurrentMenuID() == MENU_EXIT_GAME)
        {
            pEngine->Deinitialize();
            return false;
        }
        else if (GetCurrentMenuID() == MENU_SAVELOAD)
        {
            MainMenuLoad_Loop();
            if (GetCurrentMenuID() == MENU_LoadingProcInMainMenu)
            {
                uGameState = GAME_STATE_PLAYING;
                Game_Loop();
            }
            break;
        }
        else if (GetCurrentMenuID() == MENU_NEWGAME)
        {
            pOtherOverlayList->Reset();
            if (!PartyCreationUI_Loop())
            {
                break;
            }

            pParty->pPickedItem.uItemID = 0;

            strcpy(pCurrentMapName, pStartingMapName);
            bFlashQuestBook = true;
            pMediaPlayer->PlayFullscreenMovie(MOVIE_Emerald, true);
            SaveNewGame();
            if (bNoMargareth)
                _449B7E_toggle_bit(pParty->_quest_bits, PARTY_QUEST_EMERALD_MARGARETH_OFF, 1);
            Game_Loop();
            if (uGameState == GAME_STATE_NEWGAME_OUT_GAMEMENU)
            {
                SetCurrentMenuID(MENU_NEWGAME);
                uGameState = GAME_STATE_PLAYING;
                continue;
            }
            else if (uGameState == GAME_STATE_GAME_QUITTING_TO_MAIN_MENU)
                break;
            assert(false && "Invalid game state");
        }
        else if (GetCurrentMenuID() == MENU_CREDITS)
        {
            if (use_music_folder)
                alSourceStop(mSourceID);
            else
            {
                if (pAudioPlayer->hAILRedbook)
                    AIL_redbook_stop(pAudioPlayer->hAILRedbook);
            }
            MainMenuUI_Credits_Loop();
            break;
        }
        else if (GetCurrentMenuID() == MENU_5 || GetCurrentMenuID() == MENU_LoadingProcInMainMenu)
        {
            uGameState = GAME_STATE_PLAYING;
            Game_Loop();
        }
        else if (GetCurrentMenuID() == MENU_DebugBLVLevel)
        {
            pMouse->ChangeActivation(0);
            pParty->Reset();
            pParty->CreateDefaultParty(true);

            __debugbreak();
            /*extern void CreateDefaultBLVLevel();
            CreateDefaultBLVLevel();

            OPENFILENAMEA ofn;
            if ( !GetOpenFileNameA((LPOPENFILENAMEA)&ofn) )
            {
            pMouse->ChangeActivation(1);
            break;
            }
            _chdir("..\\");
            strcpy(pCurrentMapName, ofn.lpstrFileTitle);*/
            pMouse->ChangeActivation(1);
            Game_Loop();
        }
        if (uGameState == GAME_STATE_LOADING_GAME)
        {
            SetCurrentMenuID(MENU_5);
            uGameState = GAME_STATE_PLAYING;
            continue;
        }
        if (uGameState == GAME_STATE_NEWGAME_OUT_GAMEMENU)
        {
            SetCurrentMenuID(MENU_NEWGAME);
            uGameState = GAME_STATE_PLAYING;
            continue;
        }
        if (uGameState == GAME_STATE_GAME_QUITTING_TO_MAIN_MENU)// from the loaded game
        {
            pAudioPlayer->StopChannels(-1, -1);
            uGameState = GAME_STATE_PLAYING;
            break;
        }
    }

    return true;
}

//----- (00462C94) --------------------------------------------------------
bool MM_Main(const wchar_t *pCmdLine, const char *mm7_path)
{
    IntegrityTest();

    lua = new LuaVM;
    lua->Initialize();

    if (pCmdLine && *pCmdLine)
        ParseCommandLine(pCmdLine);

    if (!MM7_Initialize(640, 480, mm7_path))
    {
        Log::Warning(L"MM init: failed");
        pEngine->Deinitialize();
        return false;
    }

    pEventTimer->Pause();

    GUIWindow::InitializeGUI();

    ShowLogoVideo();

    dword_6BE364_game_settings_1 |= GAME_SETTINGS_4000;

    Log::Warning(L"MM: entering main loop");
    while (1)
    {
        MainMenu_Loop();
        uGameState = GAME_STATE_PLAYING;

        if (!GameLoop())
        {
            break;
        }
    }

    pEngine->Deinitialize();
    return true;
}



//----- (00466082) --------------------------------------------------------
void MM6_Initialize(const wchar_t *pIniFilename)
{
    size_t v2; // eax@31
    size_t v3; // ebx@32
    size_t v4; // edi@36
    char pDefaultGroundTexture[16]; // [sp+FCh] [bp-8Ch]@32
    unsigned int v9; // [sp+184h] [bp-4h]@28

    //_getcwd(v5, 120);
    //sprintfex(pIniFilename, "%s\\mm6.ini", v5);
    viewparams = new ViewingParams;
    game_viewport_x = viewparams->uScreen_topL_X = GetPrivateProfileIntW(L"screen", L"vx1", 8, pIniFilename);
    game_viewport_y = viewparams->uScreen_topL_Y = GetPrivateProfileIntW(L"screen", L"vy1", 8, pIniFilename);
    game_viewport_z = viewparams->uScreen_BttmR_X = GetPrivateProfileIntW(L"screen", L"vx2", 468, pIniFilename);
    game_viewport_w = viewparams->uScreen_BttmR_Y = GetPrivateProfileIntW(L"screen", L"vy2", 351, pIniFilename);
    game_viewport_width = game_viewport_z - game_viewport_x;
    game_viewport_height = game_viewport_w - game_viewport_y + 1;


    pAudioPlayer = new AudioPlayer;
    pAudioPlayer->uMixerChannels = GetPrivateProfileIntW(L"settings", L"mixerchannels", 16, pIniFilename);
    if (pAudioPlayer->uMixerChannels > 16)
        pAudioPlayer->uMixerChannels = 16;


    if (GetPrivateProfileIntW(L"debug", L"nomonster", 0, pIniFilename))
        dword_6BE368_debug_settings_2 |= DEBUG_SETTINGS_NO_ACTORS;
    if (ReadWindowsRegistryInt("startinwindow", 0))
        dword_6BE368_debug_settings_2 |= DEBUG_SETTINGS_RUN_IN_WIDOW;
    if (GetPrivateProfileIntW(L"debug", L"showFR", 0, pIniFilename))
        dword_6BE368_debug_settings_2 |= DEBUG_SETTINGS_0002_SHOW_FR;
    if (GetPrivateProfileIntW(L"debug", L"nodamage", 0, pIniFilename))
        dword_6BE368_debug_settings_2 |= DEBUG_SETTINGS_NO_DAMAGE;
    if (GetPrivateProfileIntW(L"debug", L"nodecoration", 0, pIniFilename))
        dword_6BE368_debug_settings_2 |= DEBUG_SETTINGS_NO_DECORATIONS;

    wchar_t pStartingMapNameW[1024];
    GetPrivateProfileStringW(L"file", L"startmap", L"out01.odm", pStartingMapNameW, 0x20u, pIniFilename);
    sprintf(pStartingMapName, "%S", pStartingMapNameW);

    v9 = 0;
    if (strlen(pStartingMapName))
    {
        do
        {
            if (pStartingMapName[v9] == ' ')
                pStartingMapName[v9] = 0;
            ++v9;
            v2 = strlen(pStartingMapName);
        } while (v9 < v2);
    }

    pODMRenderParams = new ODMRenderParams;
    pODMRenderParams->outdoor_no_mist = GetPrivateProfileIntW(L"debug", L"noMist", 0, pIniFilename);
    pODMRenderParams->bNoSky = GetPrivateProfileIntW(L"outdoor", L"nosky", 0, pIniFilename);
    pODMRenderParams->bDoNotRenderDecorations = GetPrivateProfileIntW(L"render", L"nodecorations", 0, pIniFilename);
    pODMRenderParams->outdoor_no_wavy_water = GetPrivateProfileIntW(L"outdoor", L"nowavywater", 0, pIniFilename);
    //outdoor_grid_band_1 = GetPrivateProfileIntW(L"outdoor", L"gridband1", 10, pIniFilename);
    //outdoor_grid_band_2 = GetPrivateProfileIntW(L"outdoor", L"gridband2", 15, pIniFilename);
    //outdoor_grid_band_3 = GetPrivateProfileIntW(L"outdoor", L"gridband3", 25, pIniFilename);
    pODMRenderParams->terrain_gamma = GetPrivateProfileIntW(L"outdoor", L"ter_gamma", 0, pIniFilename);
    pODMRenderParams->building_gamme = GetPrivateProfileIntW(L"outdoor", L"bld_gamma", 0, pIniFilename);
    pODMRenderParams->shading_dist_shade = GetPrivateProfileIntW(L"shading", L"dist_shade", 2048, pIniFilename);
    pODMRenderParams->shading_dist_shademist = GetPrivateProfileIntW(L"shading", L"dist_shademist", 4096, pIniFilename);

    pODMRenderParams->shading_dist_mist = GetPrivateProfileIntW(L"shading", L"dist_mist", 0x2000, pIniFilename);//drawing dist 0x2000

    wchar_t pDefaultSkyTextureW[1024];
    GetPrivateProfileStringW(L"textures", L"sky", L"plansky1", pDefaultSkyTextureW, 0x10u, pIniFilename);
    sprintf(pDefaultSkyTexture.data(), "%S", pDefaultSkyTextureW);

    wchar_t pDefaultGroundTextureW[1024];
    GetPrivateProfileStringW(L"textures", L"default", L"dirt", pDefaultGroundTextureW, 0x10u, pIniFilename);
    sprintf(pDefaultGroundTexture, "%S", pDefaultGroundTextureW);

    wchar_t pFloat[1024];
    GetPrivateProfileStringW(L"debug", L"recmod1", L"1.0", pFloat, 0x10u, pIniFilename);
    swscanf(pFloat, L"%f", &flt_6BE3A4_debug_recmod1);

    GetPrivateProfileStringW(L"debug", L"recmod2", L"1.0", pFloat, 0x10u, pIniFilename);
    swscanf(pFloat, L"%f", &flt_6BE3A8_debug_recmod2);

    flt_6BE3AC_debug_recmod1_x_1_6 = flt_6BE3A4_debug_recmod1 * 1.666666666666667;

    v3 = 0;
    if (strlen(pDefaultSkyTexture.data()))
    {
        do
        {
            if (pDefaultSkyTexture[v3] == ' ')
                pDefaultSkyTexture[v3] = 0;
            ++v3;
        } while (v3 < strlen(pDefaultSkyTexture.data()));
    }
    v4 = 0;
    if (strlen(pDefaultGroundTexture))
    {
        do
        {
            if (pDefaultGroundTexture[v4] == ' ')
                pDefaultGroundTexture[v4] = 0;
            ++v4;
        } while (v4 < strlen(pDefaultGroundTexture));
    }

    MM7Initialization();
}

//----- (004666D5) --------------------------------------------------------
void MM7Initialization()
{
    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
    {
        /*if (byte_6BE388_graphicsmode == 0)
        {
        outdoor_grid_band_1 = 10;
        outdoor_grid_band_2 = 15;
        outdoor_grid_band_3 = 20;
        pODMRenderParams->shading_dist_mist = 8192;
        pODMRenderParams->bNoSky = false;
        LOBYTE(viewparams->field_20) = 0;
        }*/
        pODMRenderParams->shading_dist_shade = 2048;
        pODMRenderParams->terrain_gamma = 0;
        pODMRenderParams->building_gamme = 0;
        pODMRenderParams->shading_dist_shademist = 4096;
        pODMRenderParams->outdoor_no_wavy_water = 0;
        //_47F4D3_initialize_terrain_bezier_stuff(outdoor_grid_band_1, outdoor_grid_band_2, outdoor_grid_band_3);
        {
            pODMRenderParams->outdoor_grid_band_3 = 25;//outdoor_grid_band_3;
            pODMRenderParams->uPickDepth = 25 * 512;//outdoor_grid_band_3 * 512;
        }
    }
    else
        LOBYTE(viewparams->field_20) = 0;

    pParty->uFlags |= 2;
    viewparams->uSomeY = viewparams->uScreen_topL_Y;
    viewparams->uSomeX = viewparams->uScreen_topL_X;
    viewparams->uSomeZ = viewparams->uScreen_BttmR_X;
    viewparams->uSomeW = viewparams->uScreen_BttmR_Y;

    pViewport->SetScreen(viewparams->uScreen_topL_X, viewparams->uScreen_topL_Y, viewparams->uScreen_BttmR_X, viewparams->uScreen_BttmR_Y);
    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        pODMRenderParams->Initialize();
}

//----- (004610AA) --------------------------------------------------------
void PrepareToLoadODM(unsigned int bLoading, ODMRenderParams *a2)
{
    pGameLoadingUI_ProgressBar->Reset(27);
    pSoundList->_4A9D79(0);
    uCurrentlyLoadedLevelType = LEVEL_Outdoor;
    ODM_LoadAndInitialize(pCurrentMapName, a2);
    if (!bLoading)
        TeleportToStartingPoint(uLevel_StartingPointType);
    viewparams->_443365();
    PlayLevelMusic();
}


//----- (00464479) --------------------------------------------------------
void ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows()
{
    if (pMouse)
        pMouse->SetCursorBitmap("MICON1");

    pPaletteManager->ResetNonLocked();
    pBitmaps_LOD->ReleaseAll2();
    pSprites_LOD->DeleteSomeOtherSprites();
    pIcons_LOD->ReleaseAll2();

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        pIndoor->Release();
    else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        pOutdoor->Release();

    pAudioPlayer->StopChannels(-1, -1);
    pSoundList->_4A9D79(0);
    uCurrentlyLoadedLevelType = LEVEL_null;
    pSpriteFrameTable->ResetSomeSpriteFlags();
    pParty->armageddon_timer = 0;

    while (uNumVisibleWindows > 0)
    {
        pWindowList[pVisibleWindowsIdxs[uNumVisibleWindows] - 1]->Release();
        delete pWindowList[pVisibleWindowsIdxs[uNumVisibleWindows] - 1];
        pWindowList[pVisibleWindowsIdxs[uNumVisibleWindows] - 1] = nullptr;

        uNumVisibleWindows--;
    }
}


//----- (00450218) --------------------------------------------------------
void GenerateItemsInChest()
{
    unsigned int mapType; // eax@1
    MapInfo *currMapInfo; // esi@1
    ItemGen *currItem; // ebx@2
    int additionaItemCount; // ebp@4
    int treasureLevelBot; // edi@4
    int treasureLevelTop; // esi@4
    signed int treasureLevelRange; // esi@4
    int resultTreasureLevel; // edx@4
    int goldAmount; // esi@8
    int v11; // ebp@25
    int v12; // esi@25
    signed int whatToGenerateProb; // [sp+10h] [bp-18h]@1

    mapType = pMapStats->GetMapInfo(pCurrentMapName);
    currMapInfo = &pMapStats->pInfos[mapType];
    for (int i = 1; i < 20; ++i)
    {
        for (int j = 0; j < 140; ++j)
        {

            currItem = &pChests[i].igChestItems[j];
            if (currItem->uItemID < 0)
            {
                additionaItemCount = rand() % 5; //additional items in chect
                treasureLevelBot = byte_4E8168[abs(currItem->uItemID) - 1][2 * currMapInfo->Treasure_prob];
                treasureLevelTop = byte_4E8168[abs(currItem->uItemID) - 1][2 * currMapInfo->Treasure_prob + 1];
                treasureLevelRange = treasureLevelTop - treasureLevelBot + 1;
                resultTreasureLevel = treasureLevelBot + rand() % treasureLevelRange;  //treasure level 
                if (resultTreasureLevel < 7)
                {
                    v11 = 0;
                    do
                    {
                        whatToGenerateProb = rand() % 100;
                        if (whatToGenerateProb < 20)
                        {
                            currItem->Reset();
                        }
                        else if (whatToGenerateProb < 60) //generate gold
                        {
                            goldAmount = 0;
                            currItem->Reset();
                            switch (resultTreasureLevel)
                            {
                            case 1:
                                goldAmount = rand() % 51 + 50;
                                currItem->uItemID = ITEM_GOLD_SMALL;
                                break;
                            case 2:
                                goldAmount = rand() % 101 + 100;
                                currItem->uItemID = ITEM_GOLD_SMALL;
                                break;
                            case 3:
                                goldAmount = rand() % 301 + 200;
                                currItem->uItemID = ITEM_GOLD_MEDIUM;
                                break;
                            case 4:
                                goldAmount = rand() % 501 + 500;
                                currItem->uItemID = ITEM_GOLD_MEDIUM;
                                break;
                            case 5:
                                goldAmount = rand() % 1001 + 1000;
                                currItem->uItemID = ITEM_GOLD_LARGE;
                                break;
                            case 6:
                                goldAmount = rand() % 3001 + 2000;
                                currItem->uItemID = ITEM_GOLD_LARGE;
                                break;
                            }
                            currItem->SetIdentified();
                            currItem->special_enchantment = (ITEM_ENCHANTMENT)goldAmount;
                        }
                        else
                        {
                            pItemsTable->GenerateItem(resultTreasureLevel, 0, currItem);
                        }
                        v12 = 0;
                        while (!(pChests[i].igChestItems[v12].uItemID == ITEM_NULL) && (v12 < 140))
                        {
                            ++v12;
                        }
                        if (v12 >= 140)
                            break;
                        currItem = &pChests[i].igChestItems[v12];
                        v11++;
                    } while (v11 < additionaItemCount + 1); // + 1 because it's the item at pChests[i].igChestItems[j] and the additional ones
                }
                else
                    currItem->GenerateArtifact();
            }
        }
    }
}

//----- (00461103) --------------------------------------------------------
void _461103_load_level_sub()
{
    int v4; // edx@8
    signed int v6; // esi@14
    signed int v8; // ecx@16
    int v12; // esi@25
    int v13; // eax@26
    __int16 v14; // ax@41
    signed int v17; // [sp+14h] [bp-48h]@3
    signed int v18; // [sp+14h] [bp-48h]@23
    int v19; // [sp+18h] [bp-44h]@1
    signed int v20; // [sp+18h] [bp-44h]@14
    int v21[16]; // [sp+1Ch] [bp-40h]@17

    if (no_actors)
        uNumActors = 0;

    GenerateItemsInChest();
    pGameLoadingUI_ProgressBar->Progress();
    pParty->uFlags |= 2;
    pParty->field_7B5_in_arena_quest = 0;
    dword_5C6DF8 = 1;
    pNPCStats->uNewlNPCBufPos = 0;
    v19 = pMapStats->GetMapInfo(pCurrentMapName);

    //v15 = 0;
    for (uint i = 0; i < uNumActors; ++i)
        //if ( (signed int)uNumActors > 0 )
    {
        //Actor* pActor = &pActors[i];
        //v2 = (char *)&pActors[0].uNPC_ID;
        //do
        //{
        //v3 = pActors[i].pMonsterInfo.uID;
        v17 = 0;
        if (pActors[i].pMonsterInfo.uID >= 115 && pActors[i].pMonsterInfo.uID <= 186
            || pActors[i].pMonsterInfo.uID >= 232 && pActors[i].pMonsterInfo.uID <= 249)
            v17 = 1;
        //v1 = 0;
        v4 = (pActors[i].pMonsterInfo.uID - 1) % 3;
        if (2 == v4)
        {
            if (pActors[i].sNPC_ID && pActors[i].sNPC_ID < 5000)
                continue;
        }
        else
        {
            if (v4 != 1)
            {
                if (v4 == 0 && pActors[i].sNPC_ID == 0)
                    pActors[i].sNPC_ID = 0;
                continue;
            }
        }
        if (pActors[i].sNPC_ID > 0 && pActors[i].sNPC_ID < 5000)
            continue;
        if (v17)
        {
            pNPCStats->InitializeAdditionalNPCs(&pNPCStats->pAdditionalNPC[pNPCStats->uNewlNPCBufPos], pActors[i].pMonsterInfo.uID, 0, v19);
            v14 = LOWORD(pNPCStats->uNewlNPCBufPos) + 5000;
            ++pNPCStats->uNewlNPCBufPos;
            pActors[i].sNPC_ID = v14;
            continue;
        }
        pActors[i].sNPC_ID = 0;
        //++v15;
        //v2 += 836;
        //}
        //while ( v15 < (signed int)uNumActors );
    }

    pGameLoadingUI_ProgressBar->Progress();

    //v5 = uNumActors;
    v6 = 0;
    v20 = 0;
    //v16 = v1;

    for (uint i = 0; i < uNumActors; ++i)
    {
        //v7 = (char *)&pActors[0].pMonsterInfo;
        //do
        //{
        for (v8 = 0; v8 < v6; ++v8)
        {
            if (v21[v8] == pActors[i].pMonsterInfo.uID - 1)
                break;
        }

        if (v8 == v6)
        {
            v21[v6++] = pActors[i].pMonsterInfo.uID - 1;
            v20 = v6;
            if (v6 == 16)
                break;
        }
        //++v16;
        //v7 += 836;
        //}
        //while ( v16 < (signed int)v5 );
    }

    pGameLoadingUI_ProgressBar->Progress();

    for (int _v0 = 0; _v0 < v6; ++_v0)
    {
        for (v18 = 4; v18; --v18)
            pSoundList->LoadSound(pMonsterList->pMonsters[v21[_v0]].pSoundSampleIDs[4 - v18], 0);
        v12 = 0;
        do
            v13 = pSoundList->LoadSound(v12++ + word_4EE088_sound_ids[pMonsterStats->pInfos[v21[_v0] + 1].uSpell1ID], 1);
        while (v13);
    }
    //v0 = pGameLoadingUI_ProgressBar;
    //v1 = 0;

    pGameLoadingUI_ProgressBar->Progress();

    if (dword_6BE368_debug_settings_2 & DEBUG_SETTINGS_NO_ACTORS)
        uNumActors = 0;
    if (dword_6BE368_debug_settings_2 & DEBUG_SETTINGS_NO_DECORATIONS)
        uNumLevelDecorations = 0;
    init_event_triggers();

    pGameLoadingUI_ProgressBar->Progress();

    pIndoorCameraD3D->vPartyPos.x = 0;
    pIndoorCameraD3D->vPartyPos.y = 0;
    pIndoorCameraD3D->vPartyPos.z = 100;
    pIndoorCameraD3D->sRotationX = 0;
    pIndoorCameraD3D->sRotationY = 0;
    viewparams->bRedrawGameUI = true;
    uLevel_StartingPointType = MapStartPoint_Party;
    pSprites_LOD->_461397();
    pPaletteManager->LockTestAll();
    if (pParty->pPickedItem.uItemID != 0)
        pMouse->SetCursorBitmapFromItemID(pParty->pPickedItem.uItemID);
}

//----- (0042F3D6) --------------------------------------------------------
void InitializeTurnBasedAnimations(void *_this)
{
    for (unsigned int i = 0; i < pIconIDs_Turn.size(); ++i)
    {
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
unsigned int GetGravityStrength()
{
    int v0; // eax@1

    v0 = ~LOBYTE(pEngine->uFlags2) & 8;
    LOBYTE(v0) = v0 | 2;
    return (unsigned int)v0 >> 1;
}

//----- (00448B45) --------------------------------------------------------
void GameUI_StatusBar_Update(bool force_hide)
{
    if (force_hide || game_ui_status_bar_event_string_time_left && GetTickCount() >= game_ui_status_bar_event_string_time_left)
    {
        game_ui_status_bar_event_string_time_left = 0;
    }
}

//----- (0044861E) --------------------------------------------------------
void sub_44861E_set_texture(unsigned int uFaceCog, const char *pFilename)
{
    unsigned int texture; // eax@2

    if (uFaceCog)
    {
        texture = pBitmaps_LOD->LoadTexture(pFilename);
        if (texture != -1 ? (int)&pBitmaps_LOD->pTextures[texture] : 0)
        {
            pBitmaps_LOD->pTextures[texture].palette_id2 = pPaletteManager->LoadPalette(pBitmaps_LOD->pTextures[texture].palette_id1);

            if (uCurrentlyLoadedLevelType == 1)
            {
                if ((signed int)pIndoor->uNumFaceExtras > 1)
                {
                    for (uint i = 1; i < (signed int)pIndoor->uNumFaceExtras; ++i)
                    {
                        if (pIndoor->pFaceExtras[i].sCogNumber == uFaceCog)
                        {
                            if (pIndoor->pFaces[pIndoor->pFaceExtras[i].field_C].uAttributes & FACE_TEXTURE_FRAME)
                            {
                                pIndoor->pFaces[pIndoor->pFaceExtras[i].field_C].uBitmapID = pTextureFrameTable->FindTextureByName(pFilename);
                                if (pIndoor->pFaces[pIndoor->pFaceExtras[i].field_C].uBitmapID)
                                {
                                    pTextureFrameTable->LoadAnimationSequenceAndPalettes(pIndoor->pFaces[pIndoor->pFaceExtras[i].field_C].uBitmapID);
                                }
                                else
                                {
                                    pIndoor->pFaces[pIndoor->pFaceExtras[i].field_C].uBitmapID = texture;
                                    pIndoor->pFaces[pIndoor->pFaceExtras[i].field_C].uAttributes &= 0xFFFFBFFF;
                                }
                            }
                            else
                            {
                                pIndoor->pFaces[pIndoor->pFaceExtras[i].field_C].uBitmapID = texture;
                            }
                        }
                    }
                }
                pParty->uFlags |= 2;
            }
            else
            {
                for (uint j = 0; j < (unsigned int)pOutdoor->uNumBModels; ++j)
                {
                    for (uint i = 0; i < pOutdoor->pBModels[j].uNumFaces; ++i)
                    {
                        if (pOutdoor->pBModels[j].pFaces[i].sCogNumber == uFaceCog)
                        {
                            if (pOutdoor->pBModels[j].pFaces[i].uAttributes & FACE_TEXTURE_FRAME)
                            {
                                pOutdoor->pBModels[j].pFaces[i].uTextureID = pTextureFrameTable->FindTextureByName(pFilename);
                                if (pOutdoor->pBModels[j].pFaces[i].uTextureID)
                                    pTextureFrameTable->LoadAnimationSequenceAndPalettes(pOutdoor->pBModels[j].pFaces[i].uTextureID);
                                else
                                {
                                    pOutdoor->pBModels[j].pFaces[i].uTextureID = texture;
                                    pOutdoor->pBModels[j].pFaces[i].uAttributes &= 0xFFFFBFFF;
                                }
                            }
                            else
                                pOutdoor->pBModels[j].pFaces[i].uTextureID = texture;
                        }
                    }
                }
            }
            pParty->uFlags |= 2;
        }
    }
}

//----- (0044892E) --------------------------------------------------------
void sub_44892E_set_faces_bit(int sCogNumber, int bit, int on)
{
    if (sCogNumber)
    {
        if (uCurrentlyLoadedLevelType == 1)
        {
            for (uint i = 1; i < (unsigned int)pIndoor->uNumFaceExtras; ++i)
            {
                if (pIndoor->pFaceExtras[i].sCogNumber == sCogNumber)
                {
                    if (on)
                        pIndoor->pFaces[pIndoor->pFaceExtras[i].field_C].uAttributes |= bit;
                    else
                        pIndoor->pFaces[pIndoor->pFaceExtras[i].field_C].uAttributes &= ~bit;
                }
            }
            pParty->uFlags |= 2;
        }
        else
        {
            for (uint j = 0; j < (unsigned int)pOutdoor->uNumBModels; ++j)
            {
                for (uint i = 0; i < (unsigned int)pOutdoor->pBModels[j].uNumFaces; ++i)
                {
                    if (pOutdoor->pBModels[j].pFaces[i].sCogNumber == sCogNumber)
                    {
                        if (on)
                            pOutdoor->pBModels[j].pFaces[i].uAttributes |= bit;
                        else
                            pOutdoor->pBModels[j].pFaces[i].uAttributes &= ~bit;
                    }
                }
            }
        }
        pParty->uFlags |= 2;
    }
}

//----- (0044882F) --------------------------------------------------------
void SetDecorationSprite(uint16_t uCog, bool bHide, const char *pFileName)
{
    for (size_t i = 0; i < uNumLevelDecorations; i++)
    {
        if (pLevelDecorations[i].uCog == uCog)
        {
            if (pFileName && strcmp(pFileName, "0"))
            {
                pLevelDecorations[i].uDecorationDescID = pDecorationList->GetDecorIdByName(pFileName);
                pDecorationList->InitializeDecorationSprite(pLevelDecorations[i].uDecorationDescID);
            }

            if (bHide)
                pLevelDecorations[i].uFlags &= ~LEVEL_DECORATION_INVISIBLE;
            else
                pLevelDecorations[i].uFlags |= LEVEL_DECORATION_INVISIBLE;

            pParty->uFlags |= 2;
        }
    }
}

//----- (004356FF) --------------------------------------------------------
void back_to_game()
{
    dword_507BF0_is_there_popup_onscreen = 0;
    dword_4E455C = 1;

    extern int no_rightlick_in_inventory;
    no_rightlick_in_inventory = false;

    if (pGUIWindow_ScrollWindow)
        free_book_subwindow();
    if (!current_screen_type && !pGUIWindow_CastTargetedSpell)
        pEventTimer->Resume();
    viewparams->bRedrawGameUI = 1;
}

//----- (00494035) --------------------------------------------------------
void _494035_timed_effects__water_walking_damage__etc()
{
    unsigned int v4; // edi@1
    int v24; // ecx@60
    int v26; // ecx@64
    int v28; // ecx@68
    int v30; // ecx@72
    int v32; // ecx@76
    int v34; // ecx@80
    int v36; // ecx@84
    int v38; // ecx@88
    int v40; // ecx@92
    int v42; // ecx@96
    signed int a2a; // [sp+18h] [bp-18h]@47
    signed int old_day; // [sp+1Ch] [bp-14h]@47
    signed int old_hour;

    old_day = pParty->uCurrentDayOfMonth;
    old_hour = pParty->uCurrentHour;

    pParty->GetPlayingTime().value += pEventTimer->uTimeElapsed;

    pParty->uCurrentTimeSecond = pParty->GetPlayingTime().GetSecondsFraction();
    pParty->uCurrentMinute = pParty->GetPlayingTime().GetMinutesFraction();
    pParty->uCurrentHour = pParty->GetPlayingTime().GetHoursOfDay();
    pParty->uCurrentMonthWeek = pParty->GetPlayingTime().GetDays() / 7 & 3;
    pParty->uCurrentDayOfMonth = pParty->GetPlayingTime().GetDays() % 28;
    pParty->uCurrentMonth = pParty->GetPlayingTime().GetMonths();
    pParty->uCurrentYear = pParty->GetPlayingTime().GetMonths() / 12 + game_starting_year;
    if (pParty->uCurrentHour >= 3 && (old_hour < 3 || pParty->uCurrentDayOfMonth > old_day)) // new day dawns
    {
        pParty->pHirelings[0].bHasUsedTheAbility = false;
        pParty->pHirelings[1].bHasUsedTheAbility = false;

        for (uint i = 0; i < pNPCStats->uNumNewNPCs; ++i)
            pNPCStats->pNewNPCData[i].bHasUsedTheAbility = false;

        ++pParty->days_played_without_rest;
        if (pParty->days_played_without_rest > 1)
        {
            for (uint i = 0; i < 4; ++i)
                pParty->pPlayers[i].SetCondWeakWithBlockCheck(0);

            if (pParty->uNumFoodRations)
                Party::TakeFood(1);
            else
                for (uint i = 0; i < 4; ++i)
                    pParty->pPlayers[i].sHealth = pParty->pPlayers[i].sHealth / (pParty->days_played_without_rest + 1) + 1;

            if (pParty->days_played_without_rest > 3)
                for (uint i = 0; i < 4; ++i)
                {
                pParty->pPlayers[i].Zero();
                if (!pParty->pPlayers[i].IsPertified() && !pParty->pPlayers[i].IsEradicated()
                    && !pParty->pPlayers[i].IsDead())
                {
                    if (rand() % 100 < 5 * pParty->days_played_without_rest)
                        pParty->pPlayers[i].SetCondDeadWithBlockCheck(0);
                    if (rand() % 100 < 10 * pParty->days_played_without_rest)
                        pParty->pPlayers[i].SetCondInsaneWithBlockCheck(0);
                }
                }
        }
        if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
            pOutdoor->SetFog();

        for (uint i = 0; i < 4; ++i)
            pParty->pPlayers[i].uNumDivineInterventionCastsThisDay = 0;
    }

    // water damage
    if (pParty->uFlags & 4 && pParty->_6FC_water_lava_timer < pParty->GetPlayingTime().value)
    {
        pParty->_6FC_water_lava_timer = pParty->GetPlayingTime().value + 128;
        viewparams->bRedrawGameUI = true;
        for (uint pl = 1; pl <= 4; ++pl)
        {
            if (pPlayers[pl]->WearsItem(ITEM_RELIC_HARECS_LEATHER, EQUIP_ARMOUR)
                || pPlayers[pl]->HasEnchantedItemEquipped(71)
                || pPlayers[pl]->pPlayerBuffs[PLAYER_BUFF_WATER_WALK].expire_time)
                pPlayers[pl]->PlayEmotion(CHARACTER_EXPRESSION_37, 0);
            else
            {
                if (!pPlayers[pl]->HasUnderwaterSuitEquipped())
                {
                    pPlayers[pl]->ReceiveDamage((signed __int64)pPlayers[pl]->GetMaxHealth() * 0.1, DMGT_FIRE);
                    if (pParty->uFlags & 4)
                    {
                        GameUI_StatusBar_OnEvent_128ms(localization->GetString(660)); // You're drowning!
                    }
                }
                else
                    pPlayers[pl]->PlayEmotion(CHARACTER_EXPRESSION_37, 0);
            }
        }
    }

    // lava damage
    if (pParty->uFlags & 0x200 && pParty->_6FC_water_lava_timer < pParty->GetPlayingTime().value)
    {
        viewparams->bRedrawGameUI = true;
        pParty->_6FC_water_lava_timer = pParty->GetPlayingTime().value + 128;

        for (uint pl = 1; pl <= 4; pl++)
        {
            pPlayers[pl]->ReceiveDamage((signed __int64)pPlayers[pl]->GetMaxHealth() * 0.1, DMGT_FIRE);
            if (pParty->uFlags & 0x200)
            {
                GameUI_StatusBar_OnEvent_128ms(localization->GetString(661)); // On fire!
            }
        }
    }
    _493938_regenerate();
    uint party_condition_flag = 4;
    a2a = pEventTimer->uTimeElapsed;
    if (pParty->uFlags2 & PARTY_FLAGS_2_RUNNING)//замедление восстановления при беге
    {
        a2a *= 0.5f;
        if (a2a < 1)
            a2a = 1;
    }

    for (uint pl = 1; pl <= 4; pl++)
    {
        if (pPlayers[pl]->uTimeToRecovery)
            pPlayers[pl]->Recover(a2a);//восстановление активности
        if (pPlayers[pl]->GetItemsBonus(CHARACTER_ATTRIBUTE_ENDURANCE) + pPlayers[pl]->sHealth + pPlayers[pl]->uEndurance >= 1
            || pPlayers[pl]->pPlayerBuffs[PLAYER_BUFF_PRESERVATION].Active())
        {
            if (pPlayers[pl]->sHealth < 1)
                pPlayers[pl]->SetCondition(Condition_Unconcious, 0);
        }
        else
            pPlayers[pl]->SetCondition(Condition_Dead, 0);
        if (pPlayers[pl]->field_E0)
        {
            v24 = pPlayers[pl]->field_E0 - pEventTimer->uTimeElapsed;
            if (v24 > 0)
                pPlayers[pl]->field_E0 = v24;
            else
            {
                pPlayers[pl]->field_E0 = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->field_E4)
        {
            v26 = pPlayers[pl]->field_E4 - pEventTimer->uTimeElapsed;
            if (v26 > 0)
                pPlayers[pl]->field_E4 = v26;
            else
            {
                pPlayers[pl]->field_E4 = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->field_E8)
        {
            v28 = pPlayers[pl]->field_E8 - pEventTimer->uTimeElapsed;
            if (v28 > 0)
                pPlayers[pl]->field_E8 = v28;
            else
            {
                pPlayers[pl]->field_E8 = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->field_EC)
        {
            v30 = pPlayers[pl]->field_EC - pEventTimer->uTimeElapsed;
            if (v30 > 0)
                pPlayers[pl]->field_EC = v30;
            else
            {
                pPlayers[pl]->field_EC = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->field_F0)
        {
            v32 = pPlayers[pl]->field_F0 - pEventTimer->uTimeElapsed;
            if (v32 > 0)
                pPlayers[pl]->field_F0 = v32;
            else
            {
                pPlayers[pl]->field_F0 = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->field_F4)
        {
            v34 = pPlayers[pl]->field_F4 - pEventTimer->uTimeElapsed;
            if (v34 > 0)
                pPlayers[pl]->field_F4 = v34;
            else
            {
                pPlayers[pl]->field_F4 = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->field_F8)
        {
            v36 = pPlayers[pl]->field_F8 - pEventTimer->uTimeElapsed;
            if (v36 > 0)
                pPlayers[pl]->field_F8 = v36;
            else
            {
                pPlayers[pl]->field_F8 = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->field_FC)
        {
            v38 = pPlayers[pl]->field_FC - pEventTimer->uTimeElapsed;
            if (v38 > 0)
                pPlayers[pl]->field_FC = v38;
            else
            {
                pPlayers[pl]->field_FC = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->field_100)
        {
            v40 = pPlayers[pl]->field_100 - pEventTimer->uTimeElapsed;
            if (v40 > 0)
                pPlayers[pl]->field_100 = v40;
            else
            {
                pPlayers[pl]->field_100 = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->field_104)
        {
            v42 = pPlayers[pl]->field_104 - pEventTimer->uTimeElapsed;
            if (v42 > 0)
                pPlayers[pl]->field_104 = v42;
            else
            {
                pPlayers[pl]->field_104 = 0;
                viewparams->bRedrawGameUI = true;
            }
        }
        if (pPlayers[pl]->conditions_times[Condition_Sleep].Valid() || pPlayers[pl]->conditions_times[Condition_Paralyzed].Valid()
            || pPlayers[pl]->conditions_times[Condition_Unconcious].Valid() || pPlayers[pl]->conditions_times[Condition_Dead].Valid()
            || pPlayers[pl]->conditions_times[Condition_Pertified].Valid() || pPlayers[pl]->conditions_times[Condition_Eradicated].Valid())
        {
            --party_condition_flag;
        }

        for (uint k = 0; k < 24; ++k)
        {
            pPlayers[pl]->pPlayerBuffs[k].IsBuffExpiredToTime(pParty->GetPlayingTime());
        }

        if (pPlayers[pl]->pPlayerBuffs[PLAYER_BUFF_HASTE].Expired())
        {
            pPlayers[pl]->SetCondition(Condition_Weak, 0);
        }
    }

    for (uint i = 0; i < 20; ++i)
    {
        if (pParty->pPartyBuffs[i].IsBuffExpiredToTime(pParty->GetPlayingTime()) == 1)
            viewparams->bRedrawGameUI = true;
    }

    if (pParty->pPartyBuffs[PARTY_BUFF_HASTE].Expired())
    {
        for (uint i = 0; i < 4; ++i)
            pParty->pPlayers[i].SetCondition(Condition_Weak, 0);
    }

    for (uint i = 0; i < 2; ++i)//Проверка в сознании ли перс сделавший закл на полёт и хождение по воде
    {
        SpellBuff* pBuf = &pParty->pPartyBuffs[Party_Spec_Motion_status_ids[i]];
        if (!pBuf->expire_time)
            continue;

        if (!(pBuf->uFlags & 1))
        {
            if (!pPlayers[pBuf->uCaster]->CanAct())
            {
                pBuf->Reset();
                if (Party_Spec_Motion_status_ids[i] == PARTY_BUFF_FLY)
                    pParty->bFlying = false;
            }
        }
    }

    if (!party_condition_flag)
    {
        if (current_screen_type != SCREEN_REST)
        {
            for (uint pl = 1; pl <= 4; pl++)
            {
                if (pPlayers[pl]->conditions_times[Condition_Sleep].Valid())
                {
                    pPlayers[pl]->conditions_times[Condition_Sleep].Reset();
                    party_condition_flag = 1;
                    break;
                }
            }
            if (!party_condition_flag || _5C35C0_force_party_death)
                uGameState = GAME_STATE_PARTY_DIED;
        }
    }

    if (uActiveCharacter)//выбор следующего после пропускающего ход
    {
        if (current_screen_type != SCREEN_REST)
        {
            if (pPlayers[uActiveCharacter]->conditions_times[Condition_Sleep]
                || pPlayers[uActiveCharacter]->conditions_times[Condition_Paralyzed]
                || pPlayers[uActiveCharacter]->conditions_times[Condition_Unconcious]
                || pPlayers[uActiveCharacter]->conditions_times[Condition_Dead]
                || pPlayers[uActiveCharacter]->conditions_times[Condition_Pertified]
                || pPlayers[uActiveCharacter]->conditions_times[Condition_Eradicated]
            )
            {
                viewparams->bRedrawGameUI = true;
                uActiveCharacter = pParty->GetNextActiveCharacter();
            }
        }
    }
}

//----- (00493938) --------------------------------------------------------
void _493938_regenerate()
{
    int current_time; // edi@1
    int last_reg_time; // qax@1
    int v4; // eax@2
    int v5; // edi@5
    int v9; // edi@15
    signed int v10; // eax@15
    int numberOfActorsAffected; // ebx@20
    unsigned int v14; // esi@21
    signed int v19; // eax@21
    bool recovery_HP; // ebx@25
    signed int v25; // eax@33
    signed int v31; // ecx@53
    int actorsAffectedByImmolation[100]; // [sp+4h] [bp-22Ch]@20
    SpriteObject a1; // [sp+194h] [bp-9Ch]@15
    Vec3_int_ a3; // [sp+204h] [bp-2Ch]@15
    bool has_dragon_flag; // [sp+210h] [bp-20h]@22
    bool lich_jar_flag; // [sp+214h] [bp-1Ch]@25
    bool zombie_flag; // [sp+218h] [bp-18h]@25
    bool decrease_HP; // [sp+21Ch] [bp-14h]@25
    bool lich_flag; // [sp+220h] [bp-10h]@25
    int v49; // [sp+224h] [bp-Ch]@24
    bool recovery_SP; // [sp+228h] [bp-8h]@25
    bool redraw_flag; // [sp+22Ch] [bp-4h]@2

    current_time = pParty->GetPlayingTime().GetMinutesFraction();
    last_reg_time = pParty->last_regenerated.GetMinutesFraction();
    if (current_time >= (signed int)last_reg_time + 5)
    {
        redraw_flag = false;
        v4 = (current_time - last_reg_time) / 5;

        // chance to flight break due to a curse
        if (pParty->FlyActive())
        {
            if (pParty->bFlying)
            {
                if (!(pParty->pPartyBuffs[PARTY_BUFF_FLY].uFlags & 1))
                {
                    v5 = v4 * pParty->pPartyBuffs[PARTY_BUFF_FLY].uPower;

                    auto v6 = &pParty->pPlayers[pParty->pPartyBuffs[PARTY_BUFF_FLY].uCaster - 1].conditions_times[Condition_Cursed];
                    if (v6->value < v5)
                    {
                        v6 = 0;
                        pParty->uFlags &= 0xFFFFFFBF;
                        pParty->bFlying = false;
                        redraw_flag = true;
                    }
                }
            }
        }

        // chance to waterwalk drowning due to a curse
        if (pParty->WaterWalkActive())
        {
            if (pParty->uFlags & PARTY_FLAGS_1_STANDING_ON_WATER)
            {
                if (!(pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uFlags & 1))
                { // taking on water
                    auto v8 = &pParty->pPlayers[pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].uCaster - 1].conditions_times[Condition_Cursed];
                    v8->value -= v4;
                    if (v8->value <= 0)
                    {
                        v8->value = 0;
                        pParty->uFlags &= ~PARTY_FLAGS_1_STANDING_ON_WATER;
                        redraw_flag = true;
                    }
                }
            }
        }

        if (pParty->ImmolationActive())//Жертва
        {
            a3.z = 0;
            a3.y = 0;
            a3.x = 0;
            a1.containing_item.Reset();
            a1.spell_level = pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].uPower;
            a1.spell_skill = pParty->ImmolationSkillLevel();
            v10 = 0;
            a1.uType = SPRITE_SPELL_FIRE_IMMOLATION;
            a1.spell_id = SPELL_FIRE_IMMOLATION;
            v10 = 0;
            for (uint i = 0; i > pObjectList->uNumObjects; i++)
            {
                if (pObjectList->pObjects[i].uObjectID == spell_sprite_mapping[8].uSpriteType)
                    v10 = i;
            }
            a1.uObjectDescID = v10;
            a1.field_60_distance_related_prolly_lod = 0;
            a1.uAttributes = 0;
            a1.uSectorID = 0;
            a1.uSpriteFrameID = 0;
            a1.spell_caster_pid = PID(OBJECT_Player, pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].uCaster);
            a1.uFacing = 0;
            a1.uSoundID = 0;
            numberOfActorsAffected = pParty->_46A89E_immolation_effect(actorsAffectedByImmolation, 100, 307);
            for (v9 = 0; v9 < numberOfActorsAffected; ++v9)
            {
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
        if (PartyHasDragon())
            has_dragon_flag = true;

        for (v49 = 0; v49 < 4; v49++)
        {
            recovery_HP = false;
            recovery_SP = false;
            decrease_HP = false;
            lich_flag = false;
            lich_jar_flag = false;
            zombie_flag = false;

            for (int v22 = 0; (signed int)v22 < 16; v22++)
            {
                if (pParty->pPlayers[v49].HasItemEquipped((ITEM_EQUIP_TYPE)v22))
                {
                    uint _idx = pParty->pPlayers[v49].pEquipment.pIndices[v22];
                    if (pParty->pPlayers[v49].pInventoryItemList[_idx - 1].uItemID > 134)
                    {
                        if (pParty->pPlayers[v49].pInventoryItemList[_idx - 1].uItemID == ITEM_RELIC_ETHRICS_STAFF)
                            decrease_HP = true;
                        if (pParty->pPlayers[v49].pInventoryItemList[_idx - 1].uItemID == ITEM_ARTIFACT_HERMES_SANDALS)
                        {
                            recovery_HP = true;
                            recovery_SP = true;
                        }
                        if (pParty->pPlayers[v49].pInventoryItemList[_idx - 1].uItemID == ITEM_ARTIFACT_MINDS_EYE)
                            recovery_SP = true;
                        if (pParty->pPlayers[v49].pInventoryItemList[_idx - 1].uItemID == ITEM_ARTIFACT_HEROS_BELT)
                            recovery_HP = true;
                    }
                    else
                    {
                        v25 = pParty->pPlayers[v49].pInventoryItemList[_idx - 1].special_enchantment;
                        if (v25 == 37 //of Regeneration("Regenerate 1hp/x while walking, etc")
                            || v25 == 44 //of Life("HP (+10), Regen hpts")
                            || v25 == 50 //of The Phoenix("Fire Res (+30), Regen hpts") && 
                            || v25 == 54)// of The Troll("End (+15), Regen hpts")
                            recovery_HP = true;
                        if (v25 == 38 //of Mana("Regenerate 1sp/x while walking, etc")
                            || v25 == 47 //of The Eclipse("SP (+10), Regen spts")
                            || v25 == 55)//of The Unicorn("Luck (+15), Regen spts")
                            recovery_SP = true;
                        if (v25 == 66)// of Plenty("Regenerate 1 hp/x and 1 sp/x while walking, etc.")
                        {
                            recovery_HP = true;
                            recovery_SP = true;
                        }
                    }

                    if (recovery_HP &&
                        !pParty->pPlayers[v49].conditions_times[Condition_Dead] &&
                        !pParty->pPlayers[v49].conditions_times[Condition_Eradicated])
                    {
                        if (pParty->pPlayers[v49].sHealth < pParty->pPlayers[v49].GetMaxHealth())
                        {
                            ++pParty->pPlayers[v49].sHealth;
                        }
                        if (pParty->pPlayers[v49].conditions_times[Condition_Unconcious] && pParty->pPlayers[v49].sHealth > 0)
                        {
                            pParty->pPlayers[v49].conditions_times[Condition_Unconcious].Reset();
                        }
                        redraw_flag = true;
                    }

                    if (recovery_SP &&
                        !pParty->pPlayers[v49].conditions_times[Condition_Dead] &&
                        !pParty->pPlayers[v49].conditions_times[Condition_Eradicated])
                    {
                        if (pParty->pPlayers[v49].sMana < pParty->pPlayers[v49].GetMaxMana())
                            ++pParty->pPlayers[v49].sMana;
                        redraw_flag = true;
                    }

                    if (decrease_HP &&
                        !pParty->pPlayers[v49].conditions_times[Condition_Dead] &&
                        !pParty->pPlayers[v49].conditions_times[Condition_Eradicated])
                    {
                        --pParty->pPlayers[v49].sHealth;
                        if (!(pParty->pPlayers[v49].conditions_times[Condition_Unconcious]) && pParty->pPlayers[v49].sHealth < 0)
                        {
                            pParty->pPlayers[v49].conditions_times[Condition_Unconcious] = pParty->GetPlayingTime();
                        }
                        if (pParty->pPlayers[v49].sHealth < 1)
                        {
                            if (pParty->pPlayers[v49].sHealth + pParty->pPlayers[v49].uEndurance + pParty->pPlayers[v49].GetItemsBonus(CHARACTER_ATTRIBUTE_ENDURANCE) >= 1
                                || pParty->pPlayers[v49].pPlayerBuffs[PLAYER_BUFF_PRESERVATION].expire_time)
                            {
                                pParty->pPlayers[v49].conditions_times[Condition_Unconcious] = pParty->GetPlayingTime();
                            }
                            else if (!pParty->pPlayers[v49].conditions_times[Condition_Dead])
                            {
                                pParty->pPlayers[v49].conditions_times[Condition_Dead] = pParty->GetPlayingTime();
                            }
                        }
                        redraw_flag = true;
                    }
                }
            }

            // regeneration
            if (pParty->pPlayers[v49].pPlayerBuffs[PLAYER_BUFF_REGENERATION].expire_time
                && !pParty->pPlayers[v49].conditions_times[Condition_Dead]
                && !pParty->pPlayers[v49].conditions_times[Condition_Eradicated])
            {
                pParty->pPlayers[v49].sHealth += 5 * pParty->pPlayers[v49].pPlayerBuffs[PLAYER_BUFF_REGENERATION].uPower;
                if (pParty->pPlayers[v49].sHealth > pParty->pPlayers[v49].GetMaxHealth())
                {
                    pParty->pPlayers[v49].sHealth = pParty->pPlayers[v49].GetMaxHealth();
                }
                if (pParty->pPlayers[v49].conditions_times[Condition_Unconcious] && pParty->pPlayers[v49].sHealth > 0)
                {
                    pParty->pPlayers[v49].conditions_times[Condition_Unconcious].Reset();
                }
                redraw_flag = true;
            }

            //for warlock
            if (has_dragon_flag && pParty->pPlayers[v49].classType == PLAYER_CLASS_WARLOCK)
            {
                if (pParty->pPlayers[v49].sMana < pParty->pPlayers[v49].GetMaxMana())
                {
                    ++pParty->pPlayers[v49].sMana;
                }
                redraw_flag = true;
            }

            //for lich
            if (pParty->pPlayers[v49].classType == PLAYER_CLASS_LICH)
            {
                for (v31 = 0; v31 < 126; ++v31)
                {
                    if (pParty->pPlayers[v49].pInventoryItemList[v31].uItemID == ITEM_LICH_JAR_FULL)
                        lich_jar_flag = true;
                }
                lich_flag = true;
            }

            if (lich_flag && !pParty->pPlayers[v49].conditions_times[Condition_Dead]
                && !pParty->pPlayers[v49].conditions_times[Condition_Eradicated])
            {
                if (pParty->pPlayers[v49].sHealth > pParty->pPlayers[v49].GetMaxHealth() / 2)
                {
                    pParty->pPlayers[v49].sHealth = pParty->pPlayers[v49].sHealth - 2;
                }
                if (pParty->pPlayers[v49].sMana > pParty->pPlayers[v49].GetMaxMana() / 2)
                {
                    pParty->pPlayers[v49].sMana = pParty->pPlayers[v49].sMana - 2;
                }
            }

            if (lich_jar_flag)
            {
                if (pParty->pPlayers[v49].sMana < pParty->pPlayers[v49].GetMaxMana())
                {
                    ++pParty->pPlayers[v49].sMana;
                }
            }

            //for zombie
            if (pParty->pPlayers[v49].conditions_times[Condition_Zombie])
            {
                zombie_flag = true;
            }
            if (zombie_flag && !pParty->pPlayers[v49].conditions_times[Condition_Dead]
                && !pParty->pPlayers[v49].conditions_times[Condition_Eradicated])
            {
                if (pParty->pPlayers[v49].sHealth > pParty->pPlayers[v49].GetMaxHealth() / 2)
                {
                    pParty->pPlayers[v49].sHealth = pParty->pPlayers[v49].sHealth - 1;
                }
                if (pParty->pPlayers[v49].sMana > 0)
                {
                    pParty->pPlayers[v49].sMana = pParty->pPlayers[v49].sMana - 1;
                }
            }
        }
        pParty->last_regenerated = pParty->GetPlayingTime();
        if (!viewparams->bRedrawGameUI)
            viewparams->bRedrawGameUI = redraw_flag;
    }
}

//----- (00491E3A) --------------------------------------------------------
void sub_491E3A()
{
    signed int v1; // esi@3
    unsigned int v3; // eax@7
    unsigned int v4; // edx@8
    int v6; // edi@17

    //__debugbreak();//Ritor1
    for (uint pl = 0; pl < 4; pl++)
    {
        if (SoundSetAction[24][0])
        {
            v3 = 0;
            for (v1 = 0; v1 < (signed int)pSoundList->sNumSounds; ++v1)
            {
                int ps = 2 * (SoundSetAction[24][0] + 50 * pParty->pPlayers[pl].uVoiceID) + 4998;//6728
                if (pSoundList->pSL_Sounds[v1].uSoundID == 2 * (SoundSetAction[24][0] + 50 * pParty->pPlayers[pl].uVoiceID) + 4998)
                    v3 = v1;
            }
            pSoundList->UnloadSound(v3, 1);
            for (v4 = 0; (signed int)v4 < (signed int)pSoundList->sNumSounds; ++v4)
            {
                if (pSoundList->pSL_Sounds[v4].uSoundID == 2 * (SoundSetAction[24][0] + 50 * pParty->pPlayers[pl].uVoiceID) + 4999)
                    pSoundList->UnloadSound(v4, 1);
            }
        }
    }
    v6 = pIcons_LOD->uNumLoadedFiles - 1;
    if (v6 >= pIcons_LOD->pFacesLock)
    {
        do
        {
            pIcons_LOD->pTextures[v6].Release();
            if (pIcons_LOD->pHardwareTextures)
            {
                if (pIcons_LOD->pHardwareTextures[v6])
                {
                    pIcons_LOD->pHardwareTextures[v6]->Release();
                    pIcons_LOD->pHardwareTextures[v6] = 0;
                }
            }
            if (pIcons_LOD->pHardwareSurfaces)
            {
                if (pIcons_LOD->pHardwareSurfaces[v6])
                {
                    pIcons_LOD->pHardwareSurfaces[v6]->Release();
                    pIcons_LOD->pHardwareSurfaces[v6] = 0;
                }
            }
            --v6;
        } while (v6 >= pIcons_LOD->pFacesLock);
    }
    pIcons_LOD->uNumLoadedFiles = pIcons_LOD->pFacesLock;
    pIcons_LOD->pFacesLock = 0;
}

//----- (00494820) --------------------------------------------------------
unsigned int _494820_training_time(unsigned int a1)
{
    signed int v1; // eax@1

    v1 = 5;
    if (a1 % 24 >= 5)
        v1 = 29;
    return v1 - a1 % 24;
}

//----- (00494836) --------------------------------------------------------
int stru339_spell_sound::AddPartySpellSound(int uSoundID, int a6)
{
    int v3; // esi@1
    int result; // eax@1
    //stru339_spell_sound *v5; // ebx@1
    //int *v6; // edi@2
    unsigned int v7; // eax@3
    int v8; // [sp+Ch] [bp-8h]@3
    int v9; // [sp+10h] [bp-4h]@2
    int a2a; // [sp+1Ch] [bp+8h]@1
    //return 0;
    v3 = 0;
    result = word_4EE088_sound_ids[uSoundID];
    //v5 = this;
    a2a = word_4EE088_sound_ids[uSoundID];
    if (word_4EE088_sound_ids[uSoundID])
    {
        //v6 = this->pSoundsOffsets;
        for (v9 = 0; v9 < 2; ++v9)
        {
            v7 = a2a++;
            result = pSoundList->LoadSound(v7, (char *)this + v3, 44744 - v3, &v8, a6);
            if (!result)
                break;
            a6 += 4;
            result = v8 + 256;
            this->pSoundsOffsets[v9] = v3;
            v3 += result;
            this->pSoundsSizes[v9] = v8 + 256;
            //++v6;
        }
    }
    return result;
}

//----- (00443E31) --------------------------------------------------------
void LoadLevel_InitializeLevelStr()
{

    //  char Args[100]; 
    int string_num;
    int max_string_length;
    //  int current_string_length;
    int prev_string_offset;

    if (sizeof(pLevelStrOffsets) != 2000)
        Log::Warning(L"pLevelStrOffsets: deserialization warning");
    memset(pLevelStrOffsets.data(), 0, 2000);

    max_string_length = 0;
    string_num = 1;
    prev_string_offset = 0;
    pLevelStrOffsets[0] = 0;
    for (uint i = 0; i < uLevelStrFileSize; ++i)
    {
        if (!pLevelStr[i])
        {
            pLevelStrOffsets[string_num] = i + 1;
            ++string_num;
            if (i - prev_string_offset > max_string_length)
                max_string_length = i - prev_string_offset;
            prev_string_offset = i;
        }
    }

    uLevelStrNumStrings = string_num - 1;
    if (max_string_length > 800)
        Error("MAX_EVENT_TEXT_LENGTH needs to be increased to %lu", max_string_length + 1);

    if (uLevelStrNumStrings > 0)
    {
        for (uint i = 0; i <uLevelStrNumStrings; ++i)
        {
            if (RemoveQuotes(&pLevelStr[pLevelStrOffsets[i]]) != &pLevelStr[pLevelStrOffsets[i]])
                ++pLevelStrOffsets[i];
        }
    }
}

//----- (00443F95) --------------------------------------------------------
void OnMapLeave()
{
    _evt_raw *test_event;
    if (uLevelEVT_NumEvents > 0)
    {
        for (uint i = 0; i < uLevelEVT_NumEvents; ++i)
        {
            test_event = (_evt_raw*)&pLevelEVT[pLevelEVT_Index[i].uEventOffsetInEVT];
            if (test_event->_e_type == EVENT_OnMapLeave)
            {
                EventProcessor(pLevelEVT_Index[i].uEventID, 0, 1, pLevelEVT_Index[i].event_sequence_num);
            }
        }
    }
}

//----- (00443FDC) --------------------------------------------------------
void OnMapLoad()
{
    int v6; // eax@9
    int hours; // ebx@26
    GameTime v18; // [sp+Ch] [bp-44h]@12
    unsigned int seconds; // [sp+14h] [bp-3Ch]@26
    GameTime v20; // [sp+1Ch] [bp-34h]@7
    unsigned int minutes; // [sp+2Ch] [bp-24h]@26
    unsigned int years; // [sp+34h] [bp-1Ch]@26
    unsigned int weeks; // [sp+38h] [bp-18h]@26
    unsigned int days; // [sp+3Ch] [bp-14h]@26
    unsigned int months; // [sp+40h] [bp-10h]@26

    for (uint i = 0; i < uLevelEVT_NumEvents; ++i)
    {
        EventIndex pEvent = pLevelEVT_Index[i];

        _evt_raw* _evt = (_evt_raw *)(&pLevelEVT[pEvent.uEventOffsetInEVT]);

        if (_evt->_e_type == EVENT_PlaySound)
            pSoundList->LoadSound(EVT_DWORD(_evt->v5), 0);
        else if (_evt->_e_type == EVENT_OnMapReload)
            EventProcessor(pEvent.uEventID, 0, 0, pEvent.event_sequence_num);
        else if (_evt->_e_type == EVENT_OnTimer || _evt->_e_type == EVENT_OnLongTimer)
        {
            //v3 = &MapsLongTimersList[MapsLongTimers_count];
            v20 = pOutdoor->loc_time.last_visit;
            if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
                v20 = pIndoor->stru1.last_visit;

            MapsLongTimersList[MapsLongTimers_count].timer_evt_type = _evt->_e_type;
            MapsLongTimersList[MapsLongTimers_count].timer_evt_ID = pEvent.uEventID;
            MapsLongTimersList[MapsLongTimers_count].timer_evt_seq_num = pEvent.event_sequence_num;

            MapsLongTimersList[MapsLongTimers_count].YearsInterval = _evt->v5;
            MapsLongTimersList[MapsLongTimers_count].MonthsInterval = _evt->v6;
            MapsLongTimersList[MapsLongTimers_count].WeeksInterval = _evt->v7;
            MapsLongTimersList[MapsLongTimers_count].HoursInterval = _evt->v8;
            MapsLongTimersList[MapsLongTimers_count].MinutesInterval = _evt->v9;
            MapsLongTimersList[MapsLongTimers_count].SecondsInterval = _evt->v10;


            v6 = ((unsigned short)_evt->v12 << 8) + _evt->v11;

            MapsLongTimersList[MapsLongTimers_count].time_left_to_fire = ((unsigned short)_evt->v12 << 8) + _evt->v11;
            MapsLongTimersList[MapsLongTimers_count].IntervalHalfMins = ((unsigned short)_evt->v12 << 8) + _evt->v11;
            if (MapsLongTimersList[MapsLongTimers_count].timer_evt_type == EVENT_OnLongTimer && !(short)v6)
            {
                if (v20)
                    v18 = pParty->GetPlayingTime() - v20;
                else
                    v18 = 0;

                if (v18.GetYears() != 0 && MapsLongTimersList[MapsLongTimers_count].YearsInterval ||
                    v18.GetMonths() != 0 && MapsLongTimersList[MapsLongTimers_count].MonthsInterval != 0 ||
                    v18.GetWeeks() != 0 && MapsLongTimersList[MapsLongTimers_count].WeeksInterval != 0 ||
                    v18.GetDays() != 0 || !v20)
                {
                    ++MapsLongTimers_count;
                    MapsLongTimersList[MapsLongTimers_count].NextStartTime = 0;
                    continue;
                }
            }
            else
            {
                seconds = pParty->GetPlayingTime().GetSecondsFraction();
                minutes = pParty->GetPlayingTime().GetMinutesFraction();
                hours = pParty->GetPlayingTime().GetHoursOfDay();
                days = pParty->GetPlayingTime().GetDaysOfWeek();
                weeks = pParty->GetPlayingTime().GetWeeksOfMonth();
                months = pParty->GetPlayingTime().GetMonthsOfYear();
                years = pParty->GetPlayingTime().GetYears();

                if (MapsLongTimersList[MapsLongTimers_count].YearsInterval)
                    ++years;
                else if (MapsLongTimersList[MapsLongTimers_count].MonthsInterval)
                    ++months;
                else if (MapsLongTimersList[MapsLongTimers_count].WeeksInterval)
                    ++weeks;
                else
                {
                    ++days;
                    hours = MapsLongTimersList[MapsLongTimers_count].HoursInterval;
                    minutes = MapsLongTimersList[MapsLongTimers_count].MinutesInterval;
                    seconds = MapsLongTimersList[MapsLongTimers_count].SecondsInterval;
                }
                MapsLongTimersList[MapsLongTimers_count].NextStartTime = GameTime(seconds, minutes, hours, days, weeks, months, years);
                ++MapsLongTimers_count;
            }
        }
    }
}

//----- (00444360) --------------------------------------------------------
void Level_LoadEvtAndStr(const char *pLevelName)
{
    char pContainerName[120]; // [sp+8h] [bp-98h]@1

    sprintf(pContainerName, "%s.evt", pLevelName);
    uLevelEVT_Size = LoadEventsToBuffer(pContainerName, pLevelEVT.data(), 9216);

    sprintf(pContainerName, "%s.str", pLevelName);
    uLevelStrFileSize = LoadEventsToBuffer(pContainerName, pLevelStr.data(), 9216);
    if (uLevelStrFileSize)
        LoadLevel_InitializeLevelStr();
}

//----- (004452BB) --------------------------------------------------------
void sub_4452BB()
{
    pGUIWindow2->Release();
    pGUIWindow2 = 0;
    activeLevelDecoration = _591094_decoration;
    EventProcessor(dword_5C3418, 0, 1, dword_5C341C);
    activeLevelDecoration = nullptr;
    pEventTimer->Resume();
}

//----- (0044100D) --------------------------------------------------------
bool _44100D_should_alter_right_panel()
{
    return current_screen_type == SCREEN_NPC_DIALOGUE || current_screen_type == SCREEN_CHARACTERS ||
        current_screen_type == SCREEN_HOUSE || current_screen_type == SCREEN_E ||
        current_screen_type == SCREEN_CHANGE_LOCATION || current_screen_type == SCREEN_INPUT_BLV || current_screen_type == SCREEN_CASTING;
}

//----- (0044987B) --------------------------------------------------------
void Transition_StopSound_Autosave(const char *pMapName, MapStartPoint start_point)
{
    pAudioPlayer->StopChannels(-1, -1);
    pGameLoadingUI_ProgressBar->Initialize(GUIProgressBar::TYPE_None);
    if (_stricmp(pCurrentMapName, pMapName))
        SaveGame(1, 0);

    uGameState = GAME_STATE_CHANGE_LOCATION;
    strcpy(pCurrentMapName, pMapName);
    uLevel_StartingPointType = start_point;
}

//----- (004451A8) --------------------------------------------------------
void sub_4451A8_press_any_key(int a1, int a2, int a4)
{
    if (!pGUIWindow2)
    {
        if (pParty->uFlags & 2)
            pEngine->Draw();
        pAudioPlayer->StopChannels(-1, -1);
        pMiscTimer->Pause();
        pEventTimer->Pause();
        dword_5C3418 = a1;
        dword_5C341C = a2;
        _591094_decoration = activeLevelDecoration;
        pGUIWindow2 = new GUIWindow_GenericDialogue(0, 0, window->GetWidth(), window->GetHeight(), a4, 0);
        pGUIWindow2->CreateButton(61, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 1, '1', "", 0);
        pGUIWindow2->CreateButton(177, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 2, '2', "", 0);
        pGUIWindow2->CreateButton(292, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 3, '3', "", 0);
        pGUIWindow2->CreateButton(407, 424, 31, 40, 2, 94, UIMSG_SelectCharacter, 4, '4', "", 0);
    }
}




//----- (00448B67) --------------------------------------------------------
void OnTimer(int)
{
    if (pEventTimer->bPaused)
        return;

    __int64 v13 = (pParty->GetPlayingTime() - _5773B8_event_timer).value / 128;
    if (!v13)
        return;

    _5773B8_event_timer = pParty->GetPlayingTime();

    for (uint i = 0; i < MapsLongTimers_count; ++i)
    {
        //v4 = (char *)&array_5B5928_timers[0].field_C;
        MapsLongTimer* timer = &MapsLongTimersList[i];
        //while ( 1 )
        //{
        //v5 = *(short *)v4;
        if (timer->time_left_to_fire)
        {
            if (v13 < timer->time_left_to_fire)
                timer->time_left_to_fire -= v13;
            else
            {
                timer->time_left_to_fire = timer->IntervalHalfMins;
                EventProcessor(timer->timer_evt_ID, 0, 1, timer->timer_evt_seq_num);
            }
        }
        else
        {
            if (timer->NextStartTime < pParty->GetPlayingTime())
            {
                uint next_trigger_time = 1 * 60 * 60 * 24; // 1 day
                if (timer->YearsInterval)
                    next_trigger_time = 336 * 60 * 60 * 24; // 1 year
                else if (timer->MonthsInterval)
                    next_trigger_time = 28 * 60 * 60 * 24; // 1 month
                else if (timer->WeeksInterval)
                    next_trigger_time = 7 * 60 * 60 * 24; // 1 week

                timer->NextStartTime.value += (next_trigger_time * 128) / 3.0f;
                if (timer->NextStartTime < pParty->GetPlayingTime()) // make sure in wont fire several times in a row if big time interval has lapsed
                    timer->NextStartTime = pParty->GetPlayingTime();

                EventProcessor(timer->timer_evt_ID, 0, 1, timer->timer_evt_seq_num);
            }
        }
    }
}







//----- (0044C28F) --------------------------------------------------------
bool TeleportToNWCDungeon()
{
    if (!_stricmp("nwc.blv", pCurrentMapName))
        return false;

    _5B65A8_npcdata_uflags_or_other = 0;
    _5B65AC_npcdata_fame_or_other = 0;
    _5B65B0_npcdata_rep_or_other = 0;
    _5B65B4_npcdata_loword_house_or_other = 0;
    _5B65B8_npcdata_hiword_house_or_other = 0;
    dword_5B65BC = 0;
    dword_5B65C0 = 0;

    pGameLoadingUI_ProgressBar->uType = GUIProgressBar::TYPE_Fullscreen;
    Transition_StopSound_Autosave("nwc.blv", MapStartPoint_Party);
    current_screen_type = SCREEN_GAME;
    return true;
}

//----- (00401000) --------------------------------------------------------
void mm7__vector_constructor(void *a1, int objSize, int numObjs, int(*constructor)(int))
{
    void *v4; // esi@2

    if (numObjs > 0)
    {
        v4 = a1;
        for (int i = numObjs; i; --i)
        {
            constructor((int)v4);
            v4 = (char *)v4 + objSize;
        }
    }
}



