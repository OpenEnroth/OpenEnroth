#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>

#include "Engine/Engine.h"
#include "Engine/Time.h"
#include "Engine/Party.h"
#include "Engine/LOD.h"

#include "Engine/Graphics/IRender.h"

#include "Engine/Tables/FrameTableInc.h"
#include "Engine/Tables/IconFrameTable.h"

#include "Engine/TurnEngine/TurnEngine.h"

#include "GUI/GUIWindow.h"

#include "Overlays.h"
#include "Sprites.h"





struct OtherOverlayList *pOtherOverlayList = new OtherOverlayList; // idb
struct OverlayList *pOverlayList = new OverlayList;






// inlined
//----- (mm6c::0045BD50) --------------------------------------------------
void OtherOverlayList::Reset()
{
  for (uint i = 0; i < 50; ++i)
    pOverlays[i].Reset();
}

//----- (004418B1) --------------------------------------------------------
int OtherOverlayList::_4418B1(int a2, int a3, int a4, int a5)
{
  return 0;
}

//----- (004418B6) --------------------------------------------------------
int OtherOverlayList::_4418B6(int uOverlayID, __int16 a3, int a4, int a5, __int16 a6)
{
  signed int v9; // esi@6
  __int16 v11; // dx@11

  for ( uint i = 0; i < 50; ++i )
  {
    if ( this->pOverlays[i].field_6 <= 0 )
    {
      this->pOverlays[i].field_0 = 0;
      this->pOverlays[i].screen_space_y = 0;
      this->pOverlays[i].screen_space_x = 0;
      this->pOverlays[i].field_C = a3;
      v9 = 0;
      for ( v9; v9 < (signed int)pOverlayList->uNumOverlays; ++v9 )
      {
        if ( uOverlayID == pOverlayList->pOverlays[v9].uOverlayID )
          break;
      }
      this->pOverlays[i].field_2 = v9;
      this->pOverlays[i].sprite_frame_time = 0;
      if ( a4 )
        v11 = a4;
      else
        v11 = 8 * pSpriteFrameTable->pSpriteSFrames[pOverlayList->pOverlays[v9].uSpriteFramesetID].uAnimLength;
      this->pOverlays[i].field_6 = v11;
      this->pOverlays[i].field_10 = a5;
      this->pOverlays[i].field_E = a6;
      bRedraw = true;
      return true;
    }
  }
  return 0;
}

//----- (00441964) --------------------------------------------------------
void OtherOverlayList::DrawTurnBasedIcon(int a2)
{
    Icon *frame; // eax@12
    unsigned int v5; // [sp-8h] [bp-Ch]@4

    if (current_screen_type != SCREEN_GAME || !pParty->bTurnBasedModeOn)
        return;

    if (pTurnEngine->turn_stage == TE_MOVEMENT)//все персы отстрелялись(сжатый кулак)
        frame = pIconsFrameTable->GetFrame(pIconIDs_Turn[5 - pTurnEngine->uActionPointsLeft / 26], pEventTimer->uStartTime);
    else  if (pTurnEngine->turn_stage == TE_WAIT)
    {
        if (dword_50C998_turnbased_icon_1A)
            v5 = uIconID_TurnStart;//анимация руки(запуск пошагового режима)
        else
            v5 = uIconID_TurnHour; //группа ожидает(часы)
        frame = pIconsFrameTable->GetFrame(v5, dword_50C994);
    }
    else if (pTurnEngine->turn_stage == TE_ATTACK)//группа атакует(ладонь)
        frame = pIconsFrameTable->GetFrame(uIconID_TurnStop, pEventTimer->uStartTime);
    //if ( render->pRenderD3D )
    render->DrawTextureAlphaNew(394 / 640.0f, 288 / 480.0f, frame->GetTexture());
    /*else
      render->DrawTextureIndexedAlpha(0x18Au, 0x120u, v7);*/
    if (dword_50C994 < dword_50C998_turnbased_icon_1A)
    {
        dword_50C994 += pEventTimer->uTimeElapsed;
        if ((signed int)dword_50C994 >= dword_50C998_turnbased_icon_1A)
            dword_50C998_turnbased_icon_1A = 0;
    }
}


//----- (00458D97) --------------------------------------------------------
void OverlayList::InitializeSprites()
{
  for (uint i = 0; i < uNumOverlays; ++i)
    pSpriteFrameTable->InitializeSprite(pOverlays[i].uSpriteFramesetID);
}

//----- (00458DBC) --------------------------------------------------------
void OverlayList::ToFile()
{
  FILE *v2; // eax@1
  //FILE *v3; // edi@1

  v2 = fopen("data\\doverlay.bin", "wb");
  //v3 = v2;
  if ( !v2 )
    Error("Unable to save doverlay.bin!");
  fwrite(this, 4, 1, v2);
  fwrite(this->pOverlays, 8, this->uNumOverlays, v2);
  fclose(v2);
}

//----- (00458E08) --------------------------------------------------------
void OverlayList::FromFile(void *data_mm6, void *data_mm7, void *data_mm8)
{
  uint num_mm6_overlays = data_mm6 ? *(int *)data_mm6 : 0,
       num_mm7_overlays = data_mm7 ? *(int *)data_mm7 : 0,
       num_mm8_overlays = data_mm8 ? *(int *)data_mm8 : 0;

  uNumOverlays = num_mm6_overlays + num_mm7_overlays + num_mm8_overlays;
  Assert(uNumOverlays);
  Assert(!num_mm8_overlays);

  pOverlays = (OverlayDesc *)malloc(uNumOverlays * sizeof(OverlayDesc));
  memcpy(pOverlays,                                       (char *)data_mm7 + 4, num_mm7_overlays * sizeof(OverlayDesc));
  memcpy(pOverlays + num_mm7_overlays,                    (char *)data_mm6 + 4, num_mm6_overlays * sizeof(OverlayDesc));
  memcpy(pOverlays + num_mm6_overlays + num_mm7_overlays, (char *)data_mm8 + 4, num_mm8_overlays * sizeof(OverlayDesc));
}

//----- (00458E4F) --------------------------------------------------------
bool OverlayList::FromFileTxt(const char *Args)
{
  __int32 v3; // edi@1
  FILE *v4; // eax@1
  unsigned int v5; // esi@3
  void *v7; // eax@9
  //FILE *v8; // ST0C_4@11
  char *i; // eax@11
  char Buf; // [sp+10h] [bp-2F0h]@3
  FrameTableTxtLine v18; // [sp+204h] [bp-FCh]@4
  FrameTableTxtLine v19; // [sp+280h] [bp-80h]@4
  FILE *File; // [sp+2FCh] [bp-4h]@1
  unsigned int Argsa; // [sp+308h] [bp+8h]@3

  free(this->pOverlays);
  v3 = 0;
  this->pOverlays = nullptr;
  this->uNumOverlays = 0;
  v4 = fopen(Args, "r");
  File = v4;
  if ( !v4 )
    Error("ObjectDescriptionList::load - Unable to open file: %s.");

  v5 = 0;
  Argsa = 0;
  if ( fgets(&Buf, 490, v4) )
  {
    do
    {
      *strchr(&Buf, 10) = 0;
      memcpy(&v19, txt_file_frametable_parser(&Buf, &v18), sizeof(v19));
      if ( v19.uPropCount && *v19.pProperties[0] != 47 )
        ++Argsa;
    }
    while ( fgets(&Buf, 490, File) );
    v5 = Argsa;
    v3 = 0;
  }
  this->uNumOverlays = v5;
  v7 = malloc(8 * v5);
  this->pOverlays = (OverlayDesc *)v7;
  if ( v7 == (void *)v3 )
    Error("OverlayDescriptionList::load - Out of Memory!");

  memset(v7, v3, 8 * this->uNumOverlays);
  //v8 = File;
  this->uNumOverlays = v3;
  fseek(File, v3, v3);
  for ( i = fgets(&Buf, 490, File); i; i = fgets(&Buf, 490, File) )
  {
    *strchr(&Buf, 10) = 0;
    memcpy(&v19, txt_file_frametable_parser(&Buf, &v18), sizeof(v19));
    if ( v19.uPropCount && *v19.pProperties[0] != 47 )
    {
      this->pOverlays[this->uNumOverlays].uOverlayID = atoi(v19.pProperties[0]);
      if ( _stricmp(v19.pProperties[1], "center") )
      {
        if ( !_stricmp(v19.pProperties[1], "transparent") )
          this->pOverlays[this->uNumOverlays].uOverlayType = 2;
        else
          this->pOverlays[this->uNumOverlays].uOverlayType = 1;
      }
      else
        this->pOverlays[this->uNumOverlays].uOverlayType = 0;
      this->pOverlays[this->uNumOverlays++].uSpriteFramesetID = pSpriteFrameTable->FastFindSprite((char *)v19.pProperties[2]);
    }
  }
  fclose(File);
  return 1;
}

//----- (0045855F) --------------------------------------------------------
void OtherOverlay::Reset()
{
  this->field_0 = 0;
  this->field_2 = 0;
  this->sprite_frame_time = 0;
  this->field_6 = 0;
  this->screen_space_x = 0;
  this->screen_space_y = 0;
  this->field_C = 0;
  this->field_E = 0;
  this->field_10 = 65536;
}

//----- (004584B8) --------------------------------------------------------
OtherOverlay::OtherOverlay()
{
    this->Reset();
}