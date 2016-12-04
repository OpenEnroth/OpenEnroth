#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS
#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"

#include "GUIProgressBar.h"
#include "Engine/LOD.h"
#include "Engine/Party.h"
#include "Engine/Graphics/Render.h"
#include "Engine/Tables/IconFrameTable.h"




struct GUIProgressBar *pGameLoadingUI_ProgressBar = new GUIProgressBar;




//----- (00Initialize) --------------------------------------------------------
bool GUIProgressBar::Initialize(Type type)
{
  //GUIProgressBar *v2; // esi@1
  signed int v4; // eax@7
  int v5; // ecx@8
  //int v6; // edi@8
  int v7; // edx@14
  //const char *v8; // [sp-8h] [bp-84h]@20
  //unsigned int v9; // [sp-4h] [bp-80h]@20
  char Str1[64]; // [sp+4h] [bp-78h]@16

  switch (type)
  {
    case TYPE_None:
      return true;

    case TYPE_Box:
    case TYPE_Fullscreen:
      break;

    default:
      Error("Invalid GUIProgressBar type: %u", type);
  }

  //v2 = this;
  if (loading_bg)
    return false;

  uType = type;

  v4 = 1;
  if (uType == TYPE_Fullscreen)
  {
    v5 = 0;
    //v6 = (int)&field_10;
    do
    {
      if ( field_10[v4] == 1 )
        ++v5;
      ++v4;
    }
    while ( v4 <= 5 );
    if ( v5 == 5 )
      memset(field_10, 0, 8);
    v7 = rand() % 5 + 1;
    if ( field_10[v7] == 1 )
    {
      do
        v7 = rand() % 5 + 1;
      while ( field_10[v7] == 1 );
    }
    sprintf(Str1, "loading%d.pcx", v7);

    wchar_t image_name[1024];
    swprintf(image_name, L"loading%d.pcx", v7);

    loading_bg = assets->GetImage_PCXFromIconsLOD(image_name);
    //pLoadingBg.Load(Str1, 2);
    uProgressCurrent = 0;
    uX = 122;
    uY = 151;
    uWidth = 449;
    uHeight = 56;
    uProgressMax = 26;

    //pIcons_LOD->PlacementLoadTexture(&pLoadingProgress, "loadprog", 2u);
    progressbar_loading = assets->GetImage_16BitColorKey("loadprog", 0x7FF);
    Draw();
    return true;
  }

  switch (pParty->alignment)
  {
    case PartyAlignment_Good:    progressbar_dungeon = assets->GetImage_16BitColorKey("bardata-b", 0x7FF); break;
    case PartyAlignment_Neutral: progressbar_dungeon = assets->GetImage_16BitColorKey("bardata", 0x7FF); break;
    case PartyAlignment_Evil:    progressbar_dungeon = assets->GetImage_16BitColorKey("bardata-c", 0x7FF); break;
    //case PartyAlignment_Good:    pIcons_LOD->PlacementLoadTexture(&pBardata, "bardata-b", 2); break;
    //case PartyAlignment_Neutral: pIcons_LOD->PlacementLoadTexture(&pBardata, "bardata", 2); break;
    //case PartyAlignment_Evil:    pIcons_LOD->PlacementLoadTexture(&pBardata, "bardata-c", 2); break;
    default: Error("Invalid alignment type: %u", pParty->alignment);
  }

  uProgressCurrent = 0;
  uProgressMax = 26;
  Draw();
  return true;
}

//----- (004435BB) --------------------------------------------------------
void GUIProgressBar::Reset(unsigned __int8 uMaxProgress)
{
  field_9 = 0;
  uProgressCurrent = 0;
  uProgressMax = uMaxProgress;
}

//----- (004435CD) --------------------------------------------------------
void GUIProgressBar::Progress()
{
  ++this->uProgressCurrent;
  if ( this->uProgressCurrent > this->uProgressMax )
    this->uProgressCurrent = this->uProgressMax;
  Draw();
}

//----- (004435E2) --------------------------------------------------------
void GUIProgressBar::Release()
{
  int v3; // edi@7

  if (loading_bg)
  {
      loading_bg->Release();
      loading_bg = nullptr;
  }

  if ( this->uType == 1 )
  {
    if ( this->uProgressCurrent != this->uProgressMax )
    {
      this->uProgressCurrent = this->uProgressMax - 1;
      Progress();
    }

    if (progressbar_loading)
    {
        progressbar_loading->Release();
        progressbar_loading = nullptr;
    }
    //v3 = (int)&this->pLoadingProgress.paletted_pixels;
    //free(this->pLoadingProgress.paletted_pixels);
    //free(this->pLoadingProgress.pPalette16);
    //this->pLoadingProgress.pPalette16 = 0;
    //*(int *)v3 = 0;
  }
  else
  {
    //if ( !this->pBardata.paletted_pixels)
    //  return;
    //free(this->pBardata.paletted_pixels);
    //v3 = (int)&this->pBardata.pPalette16;
    //free(this->pBardata.pPalette16);
    //this->pBardata.paletted_pixels = 0;
    //*(int *)v3 = 0;
      if (progressbar_dungeon)
      {
          progressbar_dungeon->Release();
          progressbar_dungeon = nullptr;
      }
  }
}

//----- (00443670) --------------------------------------------------------
void GUIProgressBar::Draw()
{
  pRenderer->BeginScene();
  if (uType != TYPE_Fullscreen)
  {
    //if (pBardata.paletted_pixels)
    {
      pRenderer->Sub01();

      pRenderer->DrawTextureAlphaNew(80/640.0f, 122/480.0f, progressbar_dungeon);
      pRenderer->DrawTextureAlphaNew(100/640.0f, 146/480.0f, pIconsFrameTable->GetFrame(uIconID_TurnHour, 0)->texture);
      //pRenderer->FillRectFast(174, 164, floorf(((double)(113 * uProgressCurrent) / (double)uProgressMax) + 0.5f),//COERCE_UNSIGNED_INT64(v4 + 6.7553994e15),
        //16, pRenderer->uTargetRMask);
      pRenderer->FillRectFast(174, 164, floorf(((double)(113 * uProgressCurrent) / (double)uProgressMax) + 0.5f),//COERCE_UNSIGNED_INT64(v4 + 6.7553994e15),
        16, 0xF800);
      pRenderer->EndScene();
      pRenderer->Present();
      return;
    }
    pRenderer->EndScene();
    return;
  }

  //if (!pLoadingBg.pPixels)
  if (!loading_bg)
  {
      pRenderer->EndScene();
  }
  else
  {
      pRenderer->DrawTextureNew(0, 0, loading_bg);
      //pRenderer->SetRasterClipRect(0, 0, 639, 479);
      pRenderer->SetUIClipRect(172, 459, 15 * (signed int)(signed __int64)((double)(300 * uProgressCurrent) / (double)uProgressMax) / 15 + 172, 471);
      pRenderer->DrawTextureAlphaNew(172/640.0f, 459/480.0f, progressbar_loading);
      pRenderer->ResetUIClipRect();
      pRenderer->EndScene();
      pRenderer->Present();
  }
}