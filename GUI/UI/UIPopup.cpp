#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <crtdbg.h>

#include "Engine/Engine.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Party.h"
#include "Engine/LOD.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Graphics/PaletteManager.h"
#include "Engine/Timer.h"
#include "Engine/texts.h"
#include "Engine/Events.h"

#include "IO/Mouse.h"

#include "GUI/UI/Books/MapBook.h"
#include "GUI/GUIFont.h"

#include "Media/Audio/AudioPlayer.h"

#include "UIPopup.h"
#include "UIShops.h"

static char static_sub_417BB5_out_string[1200]; // static to a file, not sub actually


struct Image *parchment = nullptr;
struct Image *messagebox_corner_x = nullptr; // 5076AC
struct Image *messagebox_corner_y = nullptr; // 5076B4
struct Image *messagebox_corner_z = nullptr; // 5076A8
struct Image *messagebox_corner_w = nullptr; // 5076B0
struct Image *messagebox_border_top = nullptr; // 507698
struct Image *messagebox_border_bottom = nullptr; // 5076A4
struct Image *messagebox_border_left = nullptr; // 50769C
struct Image *messagebox_border_right = nullptr; // 5076A0

//----- (004179BC) --------------------------------------------------------
void CharacterUI_DrawTooltip(const char *Title, const char *content)
{
  GUIWindow popup_window; // [sp+Ch] [bp-5Ch]@1
  POINT v6; // [sp+60h] [bp-8h]@1

  memset(&popup_window, 0, 0x54u);
  popup_window.uFrameWidth = 384;
  popup_window.uFrameHeight = 256;
  popup_window.uFrameX = 128;
  popup_window.uFrameY = pMouse->GetCursorPos(&v6)->y + 30;
  popup_window.uFrameHeight = pFontSmallnum->CalcTextHeight(content, &popup_window, 24, 0) + 2 * LOBYTE(pFontLucida->uFontHeight) + 24;
  popup_window.uFrameZ = popup_window.uFrameX + popup_window.uFrameWidth - 1;
  popup_window.uFrameW = popup_window.uFrameY + popup_window.uFrameHeight - 1;
  popup_window.DrawMessageBox(0);

  popup_window.uFrameX += 12;
  popup_window.uFrameWidth -= 24;
  popup_window.uFrameY += 12;
  popup_window.uFrameHeight -= 12;
  popup_window.uFrameZ = popup_window.uFrameX + popup_window.uFrameWidth - 1;
  popup_window.uFrameW = popup_window.uFrameY + popup_window.uFrameHeight - 1;
  sprintf(pTmpBuf.data(), "\f%05d%s\f00000\n", ui_character_tooltip_header_default_color, Title);
  popup_window.DrawTitleText(pFontCreate, 0, 0, 0, pTmpBuf.data(), 3);
  popup_window.DrawText(pFontSmallnum, 1, LOBYTE(pFontLucida->uFontHeight), 0, content, 0, popup_window.uFrameY + popup_window.uFrameHeight, 0);
}

//----- (004151D9) --------------------------------------------------------
void DrawPopupWindow(unsigned int uX, unsigned int uY, unsigned int uWidth, unsigned int uHeight)
{
  unsigned int uNumTiles; // [sp+2Ch] [bp-Ch]@6
  int coord_x; // [sp+2Ch] [bp-Ch]@3
  int coord_y; // [sp+34h] [bp-4h]@5

  if (!parchment)
      return;


    pRenderer->SetUIClipRect(uX, uY, uX + uWidth, uY + uHeight);

    unsigned int parchment_width = parchment->GetWidth();
    unsigned int parchment_height = parchment->GetHeight();

    uNumTiles = uWidth / parchment_width;
    if ( uWidth % parchment_width)
      ++uNumTiles;
    coord_y = uY;
    for ( uint j = 0; j <= uHeight / parchment_height; j++ )
    {
      coord_x = uX - parchment_width;
      for ( uint i = uNumTiles + 1; i; --i )
      {
        coord_x += parchment_width;
        pRenderer->DrawTextureNew(coord_x / 640.0f, coord_y / 480.0f, parchment);
      }
      coord_y += parchment_height;
    }


    pRenderer->DrawTextureAlphaNew(
        uX / 640.0f,
        uY / 480.0f,
        messagebox_corner_x
    );
    pRenderer->DrawTextureAlphaNew(
        uX / 640.0f,
        (uY + uHeight - messagebox_corner_y->GetHeight()) / 480.0f,
        messagebox_corner_y
    );
    pRenderer->DrawTextureAlphaNew(
        (uX + uWidth - messagebox_corner_z->GetWidth()) / 640.0f,
        uY / 480.0f,
        messagebox_corner_z
    );
    pRenderer->DrawTextureAlphaNew(
        (uX + uWidth - messagebox_corner_z->GetWidth()) / 640.0f,
        (uY + uHeight - messagebox_corner_y->GetHeight()) / 480.0f,
        messagebox_corner_w
    );


    if (uWidth > messagebox_corner_x->GetWidth() + messagebox_corner_z->GetWidth())
    {
      pRenderer->SetUIClipRect(
          uX + messagebox_corner_x->GetWidth(),
          uY,
          uX + uWidth - messagebox_corner_z->GetWidth(),
          uY + uHeight
      );

      // horizontal borders
      for (
          unsigned int x = uX + messagebox_corner_x->GetWidth();
          x < uX + uWidth - messagebox_corner_x->GetWidth();
          x += messagebox_border_top->GetWidth()
      )
      {
          pRenderer->DrawTextureAlphaNew(
              x / 640.0f,
              uY / 480.0f,
              messagebox_border_top
          );
          pRenderer->DrawTextureAlphaNew(
              x / 640.0f,
              (uY + uHeight - messagebox_border_bottom->GetHeight()) / 480.0f,
              messagebox_border_bottom
          );
      }
    }

    // vertical borders
    if ( uHeight > messagebox_corner_x->GetHeight() + messagebox_corner_y->GetHeight())
    {
      pRenderer->SetUIClipRect(
          uX,
          uY + messagebox_corner_x->GetHeight(),
          uX + uWidth,
          uY + uHeight - messagebox_corner_y->GetHeight()
      );

      for (
          unsigned int y = uY + messagebox_corner_x->GetHeight();
          y < uY + uHeight - messagebox_corner_y->GetHeight();
          y += messagebox_border_top->GetHeight()
      )
      {
          pRenderer->DrawTextureAlphaNew(
              uX / 640.0f,
              y / 480.0f,
              messagebox_border_left
          );
          pRenderer->DrawTextureAlphaNew(
              (uX + uWidth - messagebox_border_right->GetWidth() - 1) / 640.0f,
              y / 480.0f,
              messagebox_border_right
          );
      }
    }
    pRenderer->ResetUIClipRect();
}

//----- (0041D895) --------------------------------------------------------
void GameUI_DrawItemInfo( struct ItemGen* inspect_item )
{
  unsigned int v2; // eax@3
  const char *v28; // edi@69
  int v34; // esi@81
  const char *pText; // [sp-14h] [bp-28Ch]@110
  char out_text[300]; // [sp+8h] [bp-270h]@40
  char v65[120]; // [sp+134h] [bp-144h]@92
  stru351_summoned_item v67;
  int v77; // [sp+200h] [bp-78h]@12
  int v78; // [sp+204h] [bp-74h]@5
  GUIWindow iteminfo_window; // [sp+208h] [bp-70h]@2
  POINT a2; // [sp+25Ch] [bp-1Ch]@2
  int v81; // [sp+264h] [bp-14h]@5
  PlayerSpeech v83; // [sp+26Ch] [bp-Ch]@18
  char* v84;
  int v85;
  char *Str; // [sp+270h] [bp-8h]@65

  int r_mask = 0xF800;
  int g_mask = 0x7E0;
  int b_mask = 0x1F;

  if (!inspect_item->uItemID)
    return;

  auto inspect_item_image = assets->GetImage_16BitColorKey(inspect_item->GetIconName(), 0x7FF);

  iteminfo_window.Hint = nullptr;
  iteminfo_window.uFrameWidth = 384;
  iteminfo_window.uFrameHeight = 180;
  iteminfo_window.uFrameY = 40;
  if ( pMouse->GetCursorPos(&a2)->x <= 320 )
    v2 = pMouse->GetCursorPos(&a2)->x + 30;
  else
    v2 = pMouse->GetCursorPos(&a2)->x - iteminfo_window.uFrameWidth - 30;
  iteminfo_window.uFrameX = v2;
  v78 = 100 - inspect_item_image->GetWidth();
  v81 = 144 - inspect_item_image->GetHeight();
  if ( v78 > 0 )
    v78 = v78 / 2;
  if ( v81 <= 0 )
    v81 = 0;
  else
    v81 = v81 / 2;
  if ( !pItemsTable->pItems[inspect_item->uItemID].uItemID_Rep_St )
    inspect_item->SetIdentified();
  v77 = 0;
  if (inspect_item->GetItemEquipType() == EQUIP_GOLD)
    v77 = inspect_item->special_enchantment;
  if ( uActiveCharacter )
  {
  //try to identify
    if (!inspect_item->IsIdentified())
    {
      if ( pPlayers[uActiveCharacter]->CanIdentify(inspect_item) == 1 )
        inspect_item->SetIdentified();
      v83 = SPEECH_9;
      if ( !inspect_item->IsIdentified() )
        ShowStatusBarString(pGlobalTXT_LocalizationStrings[446], 2);//"Identify Failed"
      else
      {
        v83 = SPEECH_8;
        if ( inspect_item->GetValue() < 100 * (pPlayers[uActiveCharacter]->uLevel + 5) )
          v83 = SPEECH_7;
      }
      if ( dword_4E455C )
      {
        pPlayers[uActiveCharacter]->PlaySound((PlayerSpeech)(int)v83, 0);
        dword_4E455C = 0;
      }
    }
    inspect_item->UpdateTempBonus(pParty->uTimePlayed);
    if (inspect_item->IsBroken())
    {
      if ( pPlayers[uActiveCharacter]->CanRepair(inspect_item) == 1 )
        inspect_item->uAttributes = inspect_item->uAttributes & 0xFFFFFFFD | 1;
      v83 = SPEECH_11;
      if ( !inspect_item->IsBroken() )
        v83 = SPEECH_10;
      else
        ShowStatusBarString(pGlobalTXT_LocalizationStrings[448], 2);//"Repair Failed"
      if ( dword_4E455C )
      {
        pPlayers[uActiveCharacter]->PlaySound(v83, 0);
        dword_4E455C = 0;
      }
    }
  }
  if (inspect_item->IsBroken())
  {
    iteminfo_window.DrawMessageBox(0);
    pRenderer->SetUIClipRect(iteminfo_window.uFrameX + 12, iteminfo_window.uFrameY + 12,
    iteminfo_window.uFrameX + iteminfo_window.uFrameWidth - 12, 
    iteminfo_window.uFrameY + iteminfo_window.uFrameHeight - 12);
    iteminfo_window.uFrameWidth -= 24;
    iteminfo_window.uFrameHeight -= 12;
    iteminfo_window.uFrameZ = iteminfo_window.uFrameX + iteminfo_window.uFrameWidth - 1;
    iteminfo_window.uFrameW = iteminfo_window.uFrameY + iteminfo_window.uFrameHeight - 1;

    pRenderer->DrawTransparentRedShade(
        (iteminfo_window.uFrameX + v78) / 640.0f,
        (v81 + iteminfo_window.uFrameY + 30) / 480.0f,
        //pIcons_LOD->LoadTexturePtr(inspect_item->GetIconName(), TEXTURE_16BIT_PALETTE));
        inspect_item_image);
    if ( inspect_item->IsIdentified())
      pText = (char *)inspect_item->GetIdentifiedName();
    else
      pText = pItemsTable->pItems[inspect_item->uItemID].pUnidentifiedName;
    iteminfo_window.DrawTitleText(pFontArrus, 0, 0xCu, Color16(0xFFu, 0xFFu, 0x9Bu), pText, 3);
    iteminfo_window.DrawTitleText(pFontArrus, 0x64u, ((signed int)iteminfo_window.uFrameHeight >> 1) - pFontArrus->CalcTextHeight(pGlobalTXT_LocalizationStrings[32], &iteminfo_window, 0, 0) / 2,
                   Color16(0xFFu, 0x19u, 0x19u), pGlobalTXT_LocalizationStrings[32], 3); //"Broken Item"
    pRenderer->ResetUIClipRect();

    if (inspect_item_image)
    {
        inspect_item_image->Release();
        inspect_item_image = nullptr;
    }

    return;
  }
  if (!inspect_item->IsIdentified())
  {
    iteminfo_window.DrawMessageBox(0);
    pRenderer->SetUIClipRect(iteminfo_window.uFrameX + 12,  iteminfo_window.uFrameY + 12,
          iteminfo_window.uFrameX + iteminfo_window.uFrameWidth - 12,  iteminfo_window.uFrameY + iteminfo_window.uFrameHeight - 12);
    iteminfo_window.uFrameWidth -= 24;
    iteminfo_window.uFrameHeight -= 12;
    iteminfo_window.uFrameZ = iteminfo_window.uFrameX + iteminfo_window.uFrameWidth - 1;
    iteminfo_window.uFrameW = iteminfo_window.uFrameY + iteminfo_window.uFrameHeight - 1;
    pRenderer->DrawTextureAlphaNew(
        (iteminfo_window.uFrameX + v78)/640.0f,
        (v81 + iteminfo_window.uFrameY + 30)/480.0f,
        inspect_item_image);
    iteminfo_window.DrawTitleText(pFontArrus, 0, 0xCu, Color16(0xFFu, 0xFFu, 0x9Bu), pItemsTable->pItems[inspect_item->uItemID].pUnidentifiedName, 3);
    iteminfo_window.DrawTitleText(pFontArrus, 0x64u, ((signed int)iteminfo_window.uFrameHeight >> 1) - pFontArrus->CalcTextHeight(pGlobalTXT_LocalizationStrings[232], &iteminfo_window, 0, 0) / 2,
                        Color16(0xFFu, 0x19u, 0x19u), pGlobalTXT_LocalizationStrings[232], 3);//"Not Identified"
    pRenderer->ResetUIClipRect();

    if (inspect_item_image)
    {
        inspect_item_image->Release();
        inspect_item_image = nullptr;
    }
    return;
  }
  sprintfex(out_text, pGlobalTXT_LocalizationStrings[463], pItemsTable->pItems[inspect_item->uItemID].pUnidentifiedName); //"Type: %s"
  out_text[100] = 0;
  out_text[200] = 0;
  switch (inspect_item->GetItemEquipType())
  {
    case EQUIP_SINGLE_HANDED:
    case EQUIP_TWO_HANDED:
      sprintfex(out_text + 100, "%s: +%d   %s: %dd%d", pGlobalTXT_LocalizationStrings[LOCSTR_ATTACK],
          (int)inspect_item->GetDamageMod(), pGlobalTXT_LocalizationStrings[53],
          (int)inspect_item->GetDamageDice(), (int)inspect_item->GetDamageRoll()); //"Damage"
      if (inspect_item->GetDamageMod())
      {
        char mod[16];
        sprintf(mod, "+%d", (int)inspect_item->GetDamageMod());
        strcat(out_text + 100, mod);
      }
      break;

    case EQUIP_BOW:
      sprintfex(out_text + 100, "%s: +%d   %s: %dd%d", pGlobalTXT_LocalizationStrings[203], //"Shoot"
          (int)inspect_item->GetDamageMod(), pGlobalTXT_LocalizationStrings[53], //"Damage"
          (int)inspect_item->GetDamageDice(), (int)inspect_item->GetDamageRoll());
      if (inspect_item->GetDamageMod())
      {
        char mod[16];
        sprintf(mod, "+%d", (int)inspect_item->GetDamageMod());
        strcat(out_text + 100, mod);
      }
      break;

    case EQUIP_ARMOUR:
    case EQUIP_SHIELD:
    case EQUIP_HELMET:
    case EQUIP_BELT:
    case EQUIP_CLOAK:
    case EQUIP_GAUNTLETS:
    case EQUIP_BOOTS:
    case EQUIP_RING:
    case EQUIP_AMULET:
      if (inspect_item->GetDamageDice()) //"Armor"	
          sprintfex(out_text + 100, "%s: +%d", pGlobalTXT_LocalizationStrings[11],
                    inspect_item->GetDamageDice() + inspect_item->GetDamageMod());
      break;
  }

  if ( !v77 )
  {
    if (inspect_item->GetItemEquipType() ==EQUIP_POTION)  //this is CORRECT! do not move to switch!
    {
      if ( inspect_item->uEnchantmentType )
           sprintf(out_text + 200,  "%s: %d",pGlobalTXT_LocalizationStrings[449] , inspect_item->uEnchantmentType); //"Power"
    }
    else if (inspect_item->GetItemEquipType() == EQUIP_REAGENT)
      sprintf(out_text + 200, "%s: %d", pGlobalTXT_LocalizationStrings[449], inspect_item->GetDamageDice()); //"Power"
    else if ( inspect_item->uEnchantmentType )
      sprintf(out_text + 200, "%s: %s +%d", pGlobalTXT_LocalizationStrings[210], pItemsTable->pEnchantments[inspect_item->uEnchantmentType-1].pBonusStat, inspect_item->m_enchantmentStrength); //"Special"
    else  if ( inspect_item->special_enchantment )
      sprintf(out_text + 200, "%s: %s", pGlobalTXT_LocalizationStrings[210], pItemsTable->pSpecialEnchantments[inspect_item->special_enchantment-1].pBonusStatement);
    else if ( inspect_item->uNumCharges )
      sprintf(out_text + 200, "%s: %lu", pGlobalTXT_LocalizationStrings[464], inspect_item->uNumCharges); //"Charges"
  }
  iteminfo_window.uFrameWidth -= 12;
  iteminfo_window.uFrameZ = iteminfo_window.uFrameX + iteminfo_window.uFrameWidth - 1;
  iteminfo_window.uFrameW = iteminfo_window.uFrameY + iteminfo_window.uFrameHeight - 1;
  Str = (char *)(3 * (LOBYTE(pFontArrus->uFontHeight) + 8));
  v84 = &out_text[0];
  for ( uint i = 1; i <= 3; i++ )
  {
    if ( *v84 )
      Str += pFontComic->CalcTextHeight(v84, &iteminfo_window, 100, 0) + 3;
    v84 += 100;
  }
  v28 = pItemsTable->pItems[inspect_item->uItemID].pDescription;
  if ( *v28 )
    Str += pFontSmallnum->CalcTextHeight(pItemsTable->pItems[inspect_item->uItemID].pDescription, &iteminfo_window, 100, 0);
  iteminfo_window.uFrameHeight = pIcons_LOD->LoadTexturePtr(inspect_item->GetIconName(),
                                     TEXTURE_16BIT_PALETTE)->uTextureHeight + v81 + 54;
  if ( (signed int)Str > (signed int)iteminfo_window.uFrameHeight )
    iteminfo_window.uFrameHeight = (unsigned int)Str;
  if ( inspect_item->uAttributes & ITEM_TEMP_BONUS && (inspect_item->special_enchantment || inspect_item->uEnchantmentType) )
    iteminfo_window.uFrameHeight += LOBYTE(pFontComic->uFontHeight);
  v85 = 0;
  if ( pFontArrus->uFontHeight )
  {
    iteminfo_window.uFrameWidth -= 24;
    if ( pFontArrus->CalcTextHeight(inspect_item->GetIdentifiedName(), &iteminfo_window, 0, 0) / (signed int)pFontArrus->uFontHeight )
      v85 = pFontArrus->uFontHeight;
    iteminfo_window.uFrameWidth += 24;
  }
  iteminfo_window.uFrameWidth += 12;
  iteminfo_window.uFrameHeight += (unsigned int)v85;
  iteminfo_window.uFrameW = iteminfo_window.uFrameY + iteminfo_window.uFrameHeight - 1;
  iteminfo_window.uFrameZ = iteminfo_window.uFrameX + iteminfo_window.uFrameWidth - 1;
  iteminfo_window.DrawMessageBox(0);
  pRenderer->SetUIClipRect(iteminfo_window.uFrameX + 12, iteminfo_window.uFrameY + 12,
         iteminfo_window.uFrameX + iteminfo_window.uFrameWidth - 12, iteminfo_window.uFrameY + iteminfo_window.uFrameHeight - 12);
  iteminfo_window.uFrameWidth -= 12;
  iteminfo_window.uFrameHeight -= 12;
  iteminfo_window.uFrameZ = iteminfo_window.uFrameX + iteminfo_window.uFrameWidth - 1;
  iteminfo_window.uFrameW = iteminfo_window.uFrameY + iteminfo_window.uFrameHeight - 1;
  pRenderer->DrawTextureAlphaNew(
      (iteminfo_window.uFrameX + v78)/640.0f,
      (iteminfo_window.uFrameY + (signed int)(iteminfo_window.uFrameHeight - inspect_item_image->GetHeight()) / 2)/480.0f,
      inspect_item_image
  );

  v34 = (int)(v85 + 35);
  Str = out_text;
  for ( uint i = 1; i <= 3; i++ )
  {
    if ( *Str )
    {
      iteminfo_window.DrawText(pFontComic, 100, v34, 0, Str, 0, 0, 0);
      v34 += pFontComic->CalcTextHeight(Str, &iteminfo_window, 100, 0) + 3;
    }
    Str += 100;
  }
  v28 = pItemsTable->pItems[inspect_item->uItemID].pDescription;
  if ( *v28 )
    iteminfo_window.DrawText(pFontSmallnum, 100, v34, 0, v28, 0, 0, 0);
  iteminfo_window.uFrameX += 12;
  iteminfo_window.uFrameWidth -= 24;
  iteminfo_window.DrawTitleText(pFontArrus, 0, 0xCu, Color16(0xFFu, 0xFFu, 0x9Bu), inspect_item->GetIdentifiedName(), 3);
  iteminfo_window.uFrameWidth += 24;
  iteminfo_window.uFrameX -= 12;
  if ( v77 )
  {
    sprintf(pTmpBuf.data(), "%s: %lu", pGlobalTXT_LocalizationStrings[465], v77);//"Value"
    iteminfo_window.DrawText(pFontComic, 100, iteminfo_window.uFrameHeight - LOBYTE(pFontComic->uFontHeight), 0, pTmpBuf.data(), 0, 0, 0);
    pRenderer->ResetUIClipRect();
  }
  else
  {
    if ( (inspect_item->uAttributes & ITEM_TEMP_BONUS) && (inspect_item->special_enchantment || inspect_item->uEnchantmentType) )
    {
      init_summoned_item(&v67, inspect_item->uExpireTime - pParty->uTimePlayed);
      strcpy(pTmpBuf.data(), "Duration:");
      Str = (char *)(v67.field_18_expire_year - game_starting_year);
      if (v67.field_18_expire_year != 1168 )
      {
        sprintf(v65, " %d:yr", v67.field_18_expire_year - game_starting_year);
        strcat(pTmpBuf.data(), v65);
      }
      if ( (((v67.field_14_exprie_month || Str) && 
           ((sprintf(v65, " %d:mo", v67.field_14_exprie_month), strcat(pTmpBuf.data(), v65), v67.field_14_exprie_month) || Str) 
         || v67.field_C_expire_day)
         && ((sprintf(v65, " %d:dy", v67.field_C_expire_day), strcat(pTmpBuf.data(), v65), v67.field_14_exprie_month) || Str || 
             v67.field_C_expire_day)
         || v67.field_8_expire_hour)
         && ((sprintf(v65, " %d:hr", v67.field_8_expire_hour), strcat(pTmpBuf.data(), v65), v67.field_14_exprie_month) || Str || 
             v67.field_C_expire_day || v67.field_8_expire_hour)
         || v67.field_4_expire_minute )
      {
        sprintf(v65, " %d:mn", v67.field_4_expire_minute);
        strcat(pTmpBuf.data(), v65);
      }
      iteminfo_window.DrawText(pFontComic, 100, iteminfo_window.uFrameHeight - 2 * LOBYTE(pFontComic->uFontHeight), 0, pTmpBuf.data(), 0, 0, 0);
    }
    sprintf(pTmpBuf.data(), "%s: %lu", pGlobalTXT_LocalizationStrings[465], inspect_item->GetValue());
    iteminfo_window.DrawText(pFontComic, 100, iteminfo_window.uFrameHeight - LOBYTE(pFontComic->uFontHeight), 0, pTmpBuf.data(), 0, 0, 0);
    if ( inspect_item->uAttributes & ITEM_STOLEN )
      pText = pGlobalTXT_LocalizationStrings[187]; //"Stolen"
    else
    {
      if ( !(inspect_item->uAttributes & ITEM_HARDENED) )
      {
        pRenderer->ResetUIClipRect();
        if ( !areWeLoadingTexture )
        {
          pIcons_LOD->LoadTexturePtr(inspect_item->GetIconName(), TEXTURE_16BIT_PALETTE)->Release();
          pIcons_LOD->SyncLoadedFilesCount();
        }
        return;
      }
      pText = pGlobalTXT_LocalizationStrings[651]; //"Hardened"
    }
    LOWORD(inspect_item->uAttributes) = r_mask;
    iteminfo_window.DrawText(pFontComic, pFontComic->GetLineWidth(pTmpBuf.data()) + 132,
          iteminfo_window.uFrameHeight - LOBYTE(pFontComic->uFontHeight), inspect_item->uAttributes, pText, 0, 0, 0);
    pRenderer->ResetUIClipRect();
  }
  if ( !areWeLoadingTexture )
  {
    pIcons_LOD->LoadTexturePtr(inspect_item->GetIconName(), TEXTURE_16BIT_PALETTE)->Release();
    pIcons_LOD->SyncLoadedFilesCount();
  }
  return;
}

//----- (0041E360) --------------------------------------------------------
void MonsterPopup_Draw(unsigned int uActorID, GUIWindow *pWindow)
{
  unsigned __int16 v9; // dx@4
  SpriteFrame *v10; // edi@17
  unsigned int v18; // ecx@19
  unsigned int v19; // eax@21
  int skill_points; // edi@61
  unsigned int skill_level; // eax@61
  int pTextHeight; // edi@90
  PlayerSpeech speech; // [sp-8h] [bp-1F4h]@79
  DDBLTFX Dst; // [sp+Ch] [bp-1E0h]@18
  DDSURFACEDESC2 pDesc; // [sp+70h] [bp-17Ch]@18
  RECT dest_rect; // [sp+ECh] [bp-100h]@26
  const char *string_name[10]; // [sp+FCh] [bp-F0h]@145
  const char *content[11]; // [sp+124h] [bp-C8h]@127
  unsigned char resistances[11]; // [sp+124h] [bp-C8h]@127
  RenderBillboardTransform_local0 v106; // [sp+150h] [bp-9Ch]@3
  unsigned int v107; // [sp+1A0h] [bp-4Ch]@18
  bool for_effects; // [sp+1C0h] [bp-2Ch]@3
  bool normal_level; // [sp+1D0h] [bp-1Ch]@18
  bool expert_level; // [sp+1C4h] [bp-28h]@18
  bool master_level; // [sp+1C8h] [bp-24h]@18
  bool grandmaster_level; // [sp+1B4h] [bp-38h]@3
  const char *pText; // [sp+1D4h] [bp-18h]@18
  int pTextColorID; // [sp+1E4h] [bp-8h]@18
  int v115;

  bool monster_full_informations = false;
  static Actor pMonsterInfoUI_Doll;
  if ( !uActiveCharacter ) //
    uActiveCharacter = 1;

  /*if ( !(bMonsterInfoUI_bDollInitialized & 1) )
  {
    bMonsterInfoUI_bDollInitialized |= 1u;
    Actor::Actor(&pMonsterInfoUI_Doll);
    atexit(nullsub_3);
  }*/
  v106.sParentBillboardID = -1;
  v115 = monster_popup_y_offsets[((signed __int16)pActors[uActorID].pMonsterInfo.uID - 1) / 3] - 40;
  if ( pActors[uActorID].pMonsterInfo.uID == pMonsterInfoUI_Doll.pMonsterInfo.uID )
    v9 = pMonsterInfoUI_Doll.uCurrentActionLength;
  else
  {
    memcpy(&pMonsterInfoUI_Doll, &pActors[uActorID], sizeof(pMonsterInfoUI_Doll));
    pMonsterInfoUI_Doll.uCurrentActionAnimation = ANIM_Bored;
    pMonsterInfoUI_Doll.uCurrentActionTime = 0;
    v9 = rand() % 256 + 128;
    pMonsterInfoUI_Doll.uCurrentActionLength = v9;
  }

  if ( (signed int)pMonsterInfoUI_Doll.uCurrentActionTime > (signed __int16)v9 )
  {
    pMonsterInfoUI_Doll.uCurrentActionTime = 0;
    if ( pMonsterInfoUI_Doll.uCurrentActionAnimation == ANIM_Bored || pMonsterInfoUI_Doll.uCurrentActionAnimation == ANIM_AtkMelee)
    {
      pMonsterInfoUI_Doll.uCurrentActionAnimation = ANIM_Standing;
      pMonsterInfoUI_Doll.uCurrentActionLength = rand() % 128 + 128;
    }
    else
    {
      //rand();
      pMonsterInfoUI_Doll.uCurrentActionAnimation = ANIM_Bored;
      if ( (pMonsterInfoUI_Doll.pMonsterInfo.uID < 115 || pMonsterInfoUI_Doll.pMonsterInfo.uID > 186) &&
           (pMonsterInfoUI_Doll.pMonsterInfo.uID < 232 || pMonsterInfoUI_Doll.pMonsterInfo.uID > 249) && rand() % 30 < 100 )
        pMonsterInfoUI_Doll.uCurrentActionAnimation = ANIM_AtkMelee;
      pMonsterInfoUI_Doll.uCurrentActionLength = 8 * pSpriteFrameTable->pSpriteSFrames[pActors[uActorID].pSpriteIDs[(signed __int16)pMonsterInfoUI_Doll.uCurrentActionAnimation]].uAnimLength;
    }
  }
  v10 = pSpriteFrameTable->GetFrame( pActors[uActorID].pSpriteIDs[pMonsterInfoUI_Doll.uCurrentActionAnimation], pMonsterInfoUI_Doll.uCurrentActionTime);
  v106.pTarget = pRenderer->pTargetSurface;
  v106.pTargetZ = pRenderer->pActiveZBuffer;
  v106.uTargetPitch = pRenderer->uTargetSurfacePitch;
  v106.uViewportX = pWindow->uFrameX + 13;
  v106.uViewportY = pWindow->uFrameY + 52;
  v106.uViewportW = (pWindow->uFrameY + 52) + 128;
  v106.uViewportZ = v106.uViewportX + 128;
  v106.uScreenSpaceX = (signed int)(v106.uViewportX + 128 + v106.uViewportX) / 2;
  v106._screenspace_x_scaler_packedfloat = 65536;
  v106._screenspace_y_scaler_packedfloat = 65536;
  v106.uScreenSpaceY = v115 + (pWindow->uFrameY + 52) + pSprites_LOD->pSpriteHeaders[v10->pHwSpriteIDs[0]].uHeight;
  v106.pPalette = PaletteManager::Get_Dark_or_Red_LUT(v10->uPaletteIndex, 0, 1);
  v106.sZValue = 0;
  v106.uFlags = 0;
  pRenderer->SetRasterClipRect(0, 0, window->GetWidth() - 1, window->GetHeight() - 1);
  pRenderer->RasterLine2D(v106.uViewportX - 1, v106.uViewportY - 1, v106.uViewportX + 129, v106.uViewportY - 1, Color16(0xE1u, 255, 0x9Bu));//горизонтальная верхняя линия
  pRenderer->RasterLine2D(v106.uViewportX - 1, v106.uViewportW + 1, v106.uViewportX - 1, v106.uViewportY - 1, Color16(0xE1u, 255, 0x9Bu));//горизонтальная нижняя линия
  pRenderer->RasterLine2D(v106.uViewportX + 129, v106.uViewportW + 1, v106.uViewportX - 1, v106.uViewportW + 1, Color16(0xE1u, 255, 0x9Bu));//левая вертикальная линия
  pRenderer->RasterLine2D(v106.uViewportX + 129, v106.uViewportY - 1, v106.uViewportX + 129, v106.uViewportW + 1, Color16(0xE1u, 255, 0x9Bu));//правая вертикальная линия
  //if ( pRenderer->pRenderD3D )
  {
    v106.uScreenSpaceY = v115 + v106.uViewportY + pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].uBufferHeight;

    memset(&Dst, 0, sizeof(Dst));
    Dst.dwSize = sizeof(Dst);
    Dst.dwFillColor = 0;

    memset(&pDesc, 0, sizeof(pDesc));
    pDesc.dwSize = sizeof(pDesc);

    pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].pTextureSurface->GetSurfaceDesc(&pDesc);
    v107 = 0;
    uint i = 0;
    int dst_x = v106.uScreenSpaceX + pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].uAreaX - pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].uBufferWidth / 2;
    int dst_y = v106.uScreenSpaceY + pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].uAreaY - pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].uBufferHeight;
    uint dst_z = v106.uScreenSpaceX + pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].uAreaX + pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].uAreaWidth + pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].uBufferWidth / 2 - pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].uBufferWidth;
    uint dst_w = v106.uScreenSpaceY + pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].uAreaY + pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].uAreaHeight - pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].uBufferHeight;
    if (dst_x < v106.uViewportX)
    {
      v18 = v106.uViewportX - dst_x;
      dst_x = v106.uViewportX;
      v107 = v18;
    }
    if (dst_y < v106.uViewportY)
    {
      v19 = v106.uViewportY - dst_y;
      dst_y = v106.uViewportY;
      i = v19;
    }
    if (dst_z > v106.uViewportZ)
      dst_z = v106.uViewportZ;
    if (dst_w > v106.uViewportW)
      dst_w = v106.uViewportW;
    pRenderer->FillRectFast(v106.uViewportX, v106.uViewportY, v106.uViewportZ - v106.uViewportX, v106.uViewportW - v106.uViewportY, 0x7FF);
    pRenderer->FillRectFast(v106.uViewportX, v106.uViewportY, v106.uViewportZ - v106.uViewportX, v106.uViewportW - v106.uViewportY, 0x7FF);
    dest_rect.left = v106.uViewportX;
    dest_rect.top = v106.uViewportY;
    dest_rect.right = v106.uViewportZ;
    dest_rect.bottom = v106.uViewportW;

    ErrD3D(pRenderer->pBackBuffer4->Blt(&dest_rect, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &Dst));
    /*if ( pRenderer->uTargetGBits == 5 )
    {
      __debugbreak(); // no monster popup for r5g5b5 will be
      memset(&pDesc, 0, 0x7Cu);
      pDesc.dwSize = 124;
      if ( pRenderer->LockSurface_DDraw4(pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].pTextureSurface, &pDesc, DDLOCK_WAIT))
      {
        v20 = (char *)dst_y;
        v110 = pDesc.lpSurface;
        if (dst_y < dst_w)
        {
          v21 = dst_x;
          //v22 = &pRenderer->pTargetSurface[dst_y * pRenderer->uTargetSurfacePitch + dst_x];
          ushort* _v22_2 = v22;
          v23 = i - dst_y;
          v115 = i - dst_y;
          while ( 1 )
          {
            dst_y = v21;
            if ( v21 < dst_z )
            {
              v25 = v107 - v21;
              v109 = (int)&v20[v23];
              for ( i = v107 - v21; ; v25 = i )
              {
                v108 = (unsigned __int16 *)((pDesc.lPitch >> 1) * pDesc.dwHeight * v109 / pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].uAreaHeight);
                v26 = (char *)v108 + pDesc.dwWidth * (v25 + dst_y++) / pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].uAreaWidth;
                *v22 = *((short *)v110 + (int)v26);
                ++v22;
                if ( dst_y >= dst_z )
                  break;
              }
              v23 = v115;
            }
            v22 = (unsigned __int16 *)((char *)_v22_2 + 2 * pRenderer->uTargetSurfacePitch);
            _v22_2 = v22;
            v20 = (char *)(dst_y + 1);
            v28 = __OFSUB__(dst_y + 1, dst_w);
            v27 = (signed int)(dst_y++ + 1 - dst_w) < 0;
            if ( !(v27 ^ v28) )
              break;
            v21 = dst_x;
          }
        }
        pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].pTextureSurface->Unlock(0);
      }
    }
    else*/
    {
      memset(&pDesc, 0, sizeof(pDesc));
      pDesc.dwSize = sizeof(pDesc);
      if ( pRenderer->LockSurface_DDraw4(pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].pTextureSurface, &pDesc, DDLOCK_WAIT) )
      {
        ushort* src = (unsigned __int16 *)pDesc.lpSurface;
        uint num_top_scanlines_above_frame_y = i - dst_y;
        for (uint y = dst_y; y < dst_w; ++y)
        {
          //ushort* dst = &pRenderer->pTargetSurface[y * pRenderer->uTargetSurfacePitch + dst_x];

          uint src_y = num_top_scanlines_above_frame_y + y;
          for (uint x = dst_x; x < dst_z; ++x)
          {
            uint src_x  = v107 - dst_x + x; // num scanlines left to frame_x  + current x

            uint idx = pDesc.dwHeight * src_y / pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].uAreaHeight * (pDesc.lPitch / sizeof(short)) +
                       pDesc.dwWidth  * src_x / pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].uAreaWidth;
            uint b = src[idx] & 0x1F;
            //*dst++ = b | 2 * (src[idx] & 0xFFE0);
            pRenderer->WritePixel16(x, y, b | 2 * (src[idx] & 0xFFE0));
          }
        }
        pSprites_LOD->pHardwareSprites[v10->pHwSpriteIDs[0]].pTextureSurface->Unlock(NULL);
      }
    }
  }
  /*else
  {
    pRenderer->FillRectFast(v106.uViewportX, v106.uViewportY, v106.uViewportZ - v106.uViewportX, v106.uViewportW - v106.uViewportY, 0);
    if ( v10->pHwSpriteIDs[0] >= 0 )
      pSprites_LOD->pSpriteHeaders[v10->pHwSpriteIDs[0]].DrawSprite_sw(&v106, 0);
  }*/
//name and profession
  if ( pActors[uActorID].sNPC_ID )
  {
    if (GetNPCData(pActors[uActorID].sNPC_ID)->uProfession)
      sprintfex(pTmpBuf.data(), pGlobalTXT_LocalizationStrings[429], GetNPCData(pActors[uActorID].sNPC_ID)->pName, aNPCProfessionNames[GetNPCData(pActors[uActorID].sNPC_ID)->uProfession]); // "%s the %s"   /   ^Pi[%s] %s
    else
      strncpy(pTmpBuf.data(), GetNPCData(pActors[uActorID].sNPC_ID)->pName, 2000);
  }
  else
  {
    if ( pActors[uActorID].dword_000334_unique_name )
      strncpy(pTmpBuf.data(), pMonsterStats->pPlaceStrings[pActors[uActorID].dword_000334_unique_name], 2000);
    else
      strncpy(pTmpBuf.data(), pMonsterStats->pInfos[pActors[uActorID].pMonsterInfo.uID].pName, 2000);
  }
  pWindow->DrawTitleText(pFontComic, 0, 0xCu, Color16(0xFFu, 0xFFu, 0x9Bu), pTmpBuf.data(), 3);
  //health bar
  Actor::DrawHealthBar(&pActors[uActorID], pWindow);

  normal_level = 0;
  expert_level = 0;
  master_level = 0;
  grandmaster_level = 0;
  for_effects = 0;
  pMonsterInfoUI_Doll.uCurrentActionTime += pMiscTimer->uTimeElapsed;
  if ( pPlayers[uActiveCharacter]->GetActualSkillLevel(PLAYER_SKILL_MONSTER_ID) )
  {
    skill_points = (unsigned __int8)pPlayers[uActiveCharacter]->GetActualSkillLevel(PLAYER_SKILL_MONSTER_ID) & 0x3F;
    skill_level = SkillToMastery(pPlayers[uActiveCharacter]->GetActualSkillLevel(PLAYER_SKILL_MONSTER_ID)) - 1;
    if ( skill_level == 0 )//(normal)
    {
      if ( skill_points + 10 >= pActors[uActorID].pMonsterInfo.uLevel )
        normal_level = 1;
    }
    else if ( skill_level == 1 )//(expert)
    {
      if ( 2 * skill_points + 10 >= pActors[uActorID].pMonsterInfo.uLevel )
      {
        normal_level = 1;
        expert_level = 1;
      }
    }
    else if ( skill_level  == 2 )//(master)
    {
      if ( 3 * skill_points + 10 >= pActors[uActorID].pMonsterInfo.uLevel )
      {
        normal_level = 1;
        expert_level = 1;
        master_level = 1;
      }
    }
    else if ( skill_level == 3 )//grandmaster
    {
      normal_level = 1;
      expert_level = 1;
      master_level = 1;
      grandmaster_level = 1;
    }
  }
  if ( pActors[uActorID].uAIState != Dead
    && pActors[uActorID].uAIState != Dying
    && !dword_507BF0_is_there_popup_onscreen && pPlayers[uActiveCharacter]->GetActualSkillLevel(PLAYER_SKILL_MONSTER_ID) )
  {
    if ( normal_level | expert_level | master_level | grandmaster_level )
    {
      if ( pActors[uActorID].pMonsterInfo.uLevel >= pPlayers[uActiveCharacter]->uLevel - 5 )
        speech = SPEECH_IDENTIFY_MONSTER_STRONGER;
      else
        speech = SPEECH_IDENTIFY_MONSTER_WEAKER;
    }
    else
      speech = SPEECH_IDENTIFY_MONSTER_106;
    pPlayers[uActiveCharacter]->PlaySound(speech, 0);
  }

  if ( (signed int)SkillToMastery(pParty->pPlayers[uActiveCharacter - 1].GetActualSkillLevel(PLAYER_SKILL_MONSTER_ID)) >= 3 )
    for_effects = 1;

  if ( monster_full_informations == true )
  {
    normal_level = 1;//
    expert_level = 1;//
    master_level = 1;//
    grandmaster_level = 1;//
    for_effects = 1;
  }

  pWindow->DrawText(pFontSmallnum, 12, 196, Color16(0xE1u, 255, 0x9Bu), pGlobalTXT_LocalizationStrings[631], 0, 0, 0);//Effects
  if ( !for_effects && false)
    pWindow->DrawText(pFontSmallnum, 28, LOBYTE(pFontSmallnum->uFontHeight) + 193, Color16(0xE1u, 255, 0x9Bu), pGlobalTXT_LocalizationStrings[630], 0, 0, 0);//?
  else
  {
    pText = "";
    pTextHeight = LOBYTE(pFontSmallnum->uFontHeight) + 193;
    for ( uint i = 1; i <= 21; ++i )
    {
      if ( pActors[uActorID].pActorBuffs[i].uExpireTime > 0 )
      {
        switch ( i )
        {
          case ACTOR_BUFF_CHARM:
            pTextColorID = 60;
            pText = pGlobalTXT_LocalizationStrings[591];//Charmed
            break;
          case ACTOR_BUFF_SUMMONED:
            pTextColorID = 82;
            pText = pGlobalTXT_LocalizationStrings[649];//Summoned
            break;
          case ACTOR_BUFF_SHRINK:
            pTextColorID = 92;
            pText = pGlobalTXT_LocalizationStrings[592];//Shrunk
            break;
          case ACTOR_BUFF_AFRAID:
            pTextColorID = 63;
            pText = pGlobalTXT_LocalizationStrings[4];//Afraid
            break;
          case ACTOR_BUFF_STONED:
            pText = pGlobalTXT_LocalizationStrings[220];//Stoned
            pTextColorID = 81;
            break;
          case ACTOR_BUFF_PARALYZED:
            pText = pGlobalTXT_LocalizationStrings[162];//Paralyzed
            pTextColorID = 81;
            break;
          case ACTOR_BUFF_SLOWED:
            pText = pGlobalTXT_LocalizationStrings[593];//Slowed
            pTextColorID = 35;
            break;
          case ACTOR_BUFF_BERSERK:
            pText = pGlobalTXT_LocalizationStrings[608];//Berserk
            pTextColorID = 62;
            break;
          case ACTOR_BUFF_SOMETHING_THAT_HALVES_AC:
          case ACTOR_BUFF_MASS_DISTORTION:
            pText = "";
            pTextColorID = 0;
            continue;
          case ACTOR_BUFF_FATE:
            pTextColorID = 47;
            pText = pGlobalTXT_LocalizationStrings[221];//Fate
            break;
          case ACTOR_BUFF_ENSLAVED:
            pTextColorID = 66;
            pText = pGlobalTXT_LocalizationStrings[607];//Enslaved
            break;
          case ACTOR_BUFF_DAY_OF_PROTECTION:
            pTextColorID = 85;
            pText = pGlobalTXT_LocalizationStrings[610];//Day of Protection
            break;
          case ACTOR_BUFF_HOUR_OF_POWER:
            pTextColorID = 86;
            pText = pGlobalTXT_LocalizationStrings[609];//Hour of Power
            break;
          case ACTOR_BUFF_SHIELD:
            pTextColorID = 17;
            pText = pGlobalTXT_LocalizationStrings[279];//Shield
            break;
          case ACTOR_BUFF_STONESKIN:
            pTextColorID = 38;
            pText = pGlobalTXT_LocalizationStrings[442];//Stoneskin
            break;
          case ACTOR_BUFF_BLESS:
            pTextColorID = 46;
            pText = pGlobalTXT_LocalizationStrings[443];//Bless
            break;
          case ACTOR_BUFF_HEROISM:
            pTextColorID = 51;
            pText = pGlobalTXT_LocalizationStrings[440];//Heroism
            break;
          case ACTOR_BUFF_HASTE:
            pTextColorID = 5;
            pText = pGlobalTXT_LocalizationStrings[441];//Haste
            break;
          case ACTOR_BUFF_PAIN_REFLECTION:
            pTextColorID = 95;
            pText = pGlobalTXT_LocalizationStrings[229];//Pain Reflection
            break;
          case ACTOR_BUFF_PAIN_HAMMERHANDS:
            pTextColorID = 73;
            pText = pGlobalTXT_LocalizationStrings[228];//Hammerhands
            break;
          default:
            pText = "";
            break;
        }
        if ( _stricmp(pText, "" ))
        {
          pWindow->DrawText(pFontSmallnum, 28, pTextHeight, GetSpellColor(pTextColorID), pText, 0, 0, 0);
          pTextHeight = pTextHeight + *(char *)((int)pFontSmallnum + 5) - 3;
        }
      }
    }
    if ( !_stricmp(pText,"" ))
      pWindow->DrawText(pFontSmallnum, 28, pTextHeight, Color16(0xE1u, 255, 0x9Bu), pGlobalTXT_LocalizationStrings[153], 0, 0, 0);//Нет
  }

  if ( normal_level )
  {
    sprintf(pTmpBuf.data(), "%s\f%05u\t100%d\n", pGlobalTXT_LocalizationStrings[108], 0, pActors[uActorID].pMonsterInfo.uHP);
    pWindow->DrawText(pFontSmallnum, 150, (int)v106.uViewportY, Color16(0xE1u, 255, 0x9Bu), pTmpBuf.data(), 0, 0, 0);
    pTextHeight = v106.uViewportY + LOBYTE(pFontSmallnum->uFontHeight) - 3;
    sprintf(pTmpBuf.data(), "%s\f%05u\t100%d\n", pGlobalTXT_LocalizationStrings[12], 0, pActors[uActorID].pMonsterInfo.uAC);//Armor Class
  }
  else
  {
    sprintf(pTmpBuf.data(), "%s\f%05u\t100%s\n", pGlobalTXT_LocalizationStrings[108], 0, pGlobalTXT_LocalizationStrings[630]);//?   - [630] actually displays a question mark
    pWindow->DrawText(pFontSmallnum, 150, (int)v106.uViewportY, Color16(0xE1u, 255, 0x9Bu), pTmpBuf.data(), 0, 0, 0);
    pTextHeight = v106.uViewportY + LOBYTE(pFontSmallnum->uFontHeight) - 3;
    sprintf(pTmpBuf.data(), "%s\f%05u\t100%s\n", pGlobalTXT_LocalizationStrings[12], 0, pGlobalTXT_LocalizationStrings[630]);//?   - [630] actually displays a question mark
  }
  pWindow->DrawText(pFontSmallnum, 150, pTextHeight, Color16(0xE1u, 255, 0x9Bu), pTmpBuf.data(), 0, 0, 0);
  pTextHeight = pTextHeight + LOBYTE(pFontSmallnum->uFontHeight) - 6 + LOBYTE(pFontSmallnum->uFontHeight);

  content[0] = pGlobalTXT_LocalizationStrings[87];
  content[1] = pGlobalTXT_LocalizationStrings[6];
  content[2] = pGlobalTXT_LocalizationStrings[240];
  content[3] = pGlobalTXT_LocalizationStrings[70];
  content[4] = pGlobalTXT_LocalizationStrings[624];
  content[5] = pGlobalTXT_LocalizationStrings[138];
  content[6] = pGlobalTXT_LocalizationStrings[214];
  content[7] = pGlobalTXT_LocalizationStrings[142];
  content[8] = pGlobalTXT_LocalizationStrings[29];
  content[9] = pGlobalTXT_LocalizationStrings[133];
  content[10] = pGlobalTXT_LocalizationStrings[54];

  if ( expert_level )
  {
    sprintf(pTmpBuf.data(), "%s\f%05u\t080%s\n", pGlobalTXT_LocalizationStrings[18], 0, content[pActors[uActorID].pMonsterInfo.uAttack1Type]);//Attack
    pWindow->DrawText(pFontSmallnum, 150, (int)pTextHeight, Color16(0xE1u, 255, 0x9Bu), pTmpBuf.data(), 0, 0, 0);
    pTextHeight = pTextHeight + LOBYTE(pFontSmallnum->uFontHeight) - 3;
    if ( pActors[uActorID].pMonsterInfo.uAttack1DamageBonus )
      sprintf(pTmpBuf.data(), "%s\f%05u\t080%dd%d+%d\n", pGlobalTXT_LocalizationStrings[53],
        0, pActors[uActorID].pMonsterInfo.uAttack1DamageDiceRolls, pActors[uActorID].pMonsterInfo.uAttack1DamageDiceSides, pActors[uActorID].pMonsterInfo.uAttack1DamageBonus);
    else
      sprintf(pTmpBuf.data(), "%s\f%05u\t080%dd%d\n", pGlobalTXT_LocalizationStrings[53],
        0, pActors[uActorID].pMonsterInfo.uAttack1DamageDiceRolls, pActors[uActorID].pMonsterInfo.uAttack1DamageDiceSides);
  }
  else
  {
    sprintf(pTmpBuf.data(), "%s\f%05u\t080%s\n", pGlobalTXT_LocalizationStrings[18], 0, pGlobalTXT_LocalizationStrings[630]);
    pWindow->DrawText(pFontSmallnum, 150, (int)pTextHeight, Color16(0xE1u, 255, 0x9Bu), pTmpBuf.data(), 0, 0, 0);
    pTextHeight = pTextHeight + LOBYTE(pFontSmallnum->uFontHeight) - 3;
    sprintf(pTmpBuf.data(), "%s\f%05u\t080%s\n", pGlobalTXT_LocalizationStrings[53], 0, pGlobalTXT_LocalizationStrings[630]);
  }
  pWindow->DrawText(pFontSmallnum, 150, pTextHeight, Color16(0xE1u, 255, 0x9Bu), pTmpBuf.data(), 0, 0, 0);
  pTextHeight = pTextHeight + LOBYTE(pFontSmallnum->uFontHeight) - 6 + LOBYTE(pFontSmallnum->uFontHeight);

  if ( !master_level )
  {
    sprintf(pTmpBuf.data(), "%s\f%05u\t080%s\n", pGlobalTXT_LocalizationStrings[628], 0, pGlobalTXT_LocalizationStrings[630]);//"Spell" "?"
    pWindow->DrawText(pFontSmallnum, 150, (int)pTextHeight, Color16(0xE1u, 255, 0x9Bu), pTmpBuf.data(), 0, 0, 0);
    pTextHeight = pTextHeight + LOBYTE(pFontSmallnum->uFontHeight) - 3;
  }
  else
  {
    pText = pGlobalTXT_LocalizationStrings[628];//Spell
    if ( pActors[uActorID].pMonsterInfo.uSpell1ID && pActors[uActorID].pMonsterInfo.uSpell2ID )
      pText = pGlobalTXT_LocalizationStrings[629];//Spells
    if ( pActors[uActorID].pMonsterInfo.uSpell1ID )
    {
      sprintf(pTmpBuf.data(), "%s\f%05u\t070%s\n", pText, 0, pSpellStats->pInfos[pActors[uActorID].pMonsterInfo.uSpell1ID].pShortName);//"%s\f%05u\t060%s\n"
      pWindow->DrawText(pFontSmallnum, 150, (int)pTextHeight, Color16(0xE1u, 255, 0x9Bu), pTmpBuf.data(), 0, 0, 0);
      pTextHeight = pTextHeight + LOBYTE(pFontSmallnum->uFontHeight) - 3;
    }
    if ( pActors[uActorID].pMonsterInfo.uSpell2ID )
    {
      sprintf(pTmpBuf.data(), "\f%05u\t070%s\n", 0, pSpellStats->pInfos[pActors[uActorID].pMonsterInfo.uSpell2ID].pShortName);//"%s\f%05u\t060%s\n"
      pWindow->DrawText(pFontSmallnum, 150, (int)pTextHeight, Color16(0xE1u, 255, 0x9Bu), pTmpBuf.data(), 0, 0, 0);
      pTextHeight = pTextHeight + LOBYTE(pFontSmallnum->uFontHeight) - 3;
    }
    if ( !pActors[uActorID].pMonsterInfo.uSpell1ID && !pActors[uActorID].pMonsterInfo.uSpell2ID )
    {
      sprintf(pTmpBuf.data(), "%s\f%05u\t070%s\n", pGlobalTXT_LocalizationStrings[628], 0, pGlobalTXT_LocalizationStrings[153]);//"%s\f%05u\t060%s\n"
      pWindow->DrawText(pFontSmallnum, 150, (int)pTextHeight, Color16(0xE1u, 255, 0x9Bu), pTmpBuf.data(), 0, 0, 0);
      pTextHeight = pTextHeight + LOBYTE(pFontSmallnum->uFontHeight) - 3;
    }
  }
  pTextHeight = pTextHeight + LOBYTE(pFontSmallnum->uFontHeight) - 3;
  pWindow->DrawText(pFontSmallnum, 150, pTextHeight, Color16(0xE1u, 255, 0x9Bu), pGlobalTXT_LocalizationStrings[626], 0, 0, 0);//Immune
  pTextHeight = pTextHeight + LOBYTE(pFontSmallnum->uFontHeight) - 3;

  string_name[0] = pGlobalTXT_LocalizationStrings[87];//Fire
  string_name[1] = pGlobalTXT_LocalizationStrings[6];//Air
  string_name[2] = pGlobalTXT_LocalizationStrings[240];
  string_name[3] = pGlobalTXT_LocalizationStrings[70];
  string_name[4] = pGlobalTXT_LocalizationStrings[142];
  string_name[5] = pGlobalTXT_LocalizationStrings[214];
  string_name[6] = pGlobalTXT_LocalizationStrings[29];
  string_name[7] = pGlobalTXT_LocalizationStrings[133];
  string_name[8] = pGlobalTXT_LocalizationStrings[54];
  string_name[9] = pGlobalTXT_LocalizationStrings[624];

  resistances[0] = pActors[uActorID].pMonsterInfo.uResFire;
  resistances[1] = pActors[uActorID].pMonsterInfo.uResAir;
  resistances[2] = pActors[uActorID].pMonsterInfo.uResWater;
  resistances[3] = pActors[uActorID].pMonsterInfo.uResEarth;
  resistances[4] = pActors[uActorID].pMonsterInfo.uResMind;
  resistances[5] = pActors[uActorID].pMonsterInfo.uResSpirit;
  resistances[6] = pActors[uActorID].pMonsterInfo.uResBody;
  resistances[7] = pActors[uActorID].pMonsterInfo.uResLight;
  resistances[8] = pActors[uActorID].pMonsterInfo.uResPhysical;
  resistances[9] = pActors[uActorID].pMonsterInfo.uResDark;

  if ( grandmaster_level )
  {
    for ( uint i = 0; i < 10; i++ )
    {
      if ( resistances[i] == 200 )
      {
        pText = pGlobalTXT_LocalizationStrings[625];//Immune
      }
      else
      {
        if ( resistances[i] )
          pText = pGlobalTXT_LocalizationStrings[627];//Resistant
        else
          pText = pGlobalTXT_LocalizationStrings[153];//None
      }
      sprintf(pTmpBuf.data(), "%s\f%05u\t070%s\n", string_name[i], 0, pText);
      pWindow->DrawText(pFontSmallnum, 170, pTextHeight, Color16(0xE1u, 255, 0x9Bu), pTmpBuf.data(), 0, 0, 0);
      pTextHeight = pTextHeight + LOBYTE(pFontSmallnum->uFontHeight) - 3;
    }
  }
  else
  {
    for ( uint i = 0; i < 10; ++i )
    {
      sprintf(pTmpBuf.data(), "%s\f%05u\t070%s\n", string_name[i], 0, pGlobalTXT_LocalizationStrings[630]); // "?"
      pWindow->DrawText(pFontSmallnum, 170, pTextHeight, Color16(0xE1u, 255, 0x9Bu), pTmpBuf.data(), 0, 0, 0);
      pTextHeight = pTextHeight + LOBYTE(pFontSmallnum->uFontHeight) - 3;
    }
  }
  //cast spell: Detect life
  if ( (signed __int64)pParty->pPartyBuffs[PARTY_BUFF_DETECT_LIFE].uExpireTime > 0 )
  {
    sprintf(pTmpBuf.data(), "%s: %d", pGlobalTXT_LocalizationStrings[650], pActors[uActorID].sCurrentHP);//Current Hit Points
    pFontSmallnum->GetLineWidth(pTmpBuf.data());
    pWindow->DrawTitleText(pFontSmallnum, 0, pWindow->uFrameHeight - LOBYTE(pFontSmallnum->uFontHeight) - 12, 0, pTmpBuf.data(), 3);
  }
}

//----- (00417BB5) --------------------------------------------------------
const char *CharacterUI_GetSkillDescText(unsigned int uPlayerID, PLAYER_SKILL_TYPE uPlayerSkillType)
{
  char a2[1200]; // [sp+Ch] [bp-538h]@7
  char Source[120]; // [sp+4BCh] [bp-88h]@7
  int v35; // [sp+53Ch] [bp-8h]@1

  v35 = pFontSmallnum->GetLineWidth(pGlobalTXT_LocalizationStrings[431]);// Normal
  if ( pFontSmallnum->GetLineWidth(pGlobalTXT_LocalizationStrings[433]) > (signed int)v35 )
    v35 = pFontSmallnum->GetLineWidth(pGlobalTXT_LocalizationStrings[433]);// Expert
  if ( pFontSmallnum->GetLineWidth(pGlobalTXT_LocalizationStrings[432]) > (signed int)v35 )
    v35 = pFontSmallnum->GetLineWidth(pGlobalTXT_LocalizationStrings[432]);// Master
  if ( pFontSmallnum->GetLineWidth(pGlobalTXT_LocalizationStrings[96]) > (signed int)v35 )
    v35 = pFontSmallnum->GetLineWidth(pGlobalTXT_LocalizationStrings[96]);// Grand

  a2[0] = 0;
  Source[0] = 0;
  strcpy(a2, "%s\n\n");
  sprintf(Source, "\f%05d", GetSkillColor(pParty->pPlayers[uPlayerID].classType, uPlayerSkillType, 1));
  strcat(a2, Source);
  strcat(a2, "%s\t%03d:\t%03d%s\t000\n");
  sprintf(Source, "\f%05d", GetSkillColor(pParty->pPlayers[uPlayerID].classType, uPlayerSkillType, 2));
  strcat(a2, Source);
  strcat(a2, "%s\t%03d:\t%03d%s\t000\n");
  sprintf(Source, "\f%05d", GetSkillColor(pParty->pPlayers[uPlayerID].classType, uPlayerSkillType, 3));
  strcat(a2, Source);
  strcat(a2, "%s\t%03d:\t%03d%s\t000\n");
  sprintf(Source, "\f%05d", GetSkillColor(pParty->pPlayers[uPlayerID].classType, uPlayerSkillType, 4));
  strcat(a2, Source);
  strcat(a2, "%s\t%03d:\t%03d%s\t000\n");
  if ( (pParty->pPlayers[uPlayerID].pActiveSkills[uPlayerSkillType] & 0x3F) == (pParty->pPlayers[uPlayerID].GetActualSkillLevel(uPlayerSkillType) & 0x3F) )
  {
    sprintf(static_sub_417BB5_out_string, a2, pSkillDesc[uPlayerSkillType],
            pGlobalTXT_LocalizationStrings[431], v35 + 3, v35 + 5, pNormalSkillDesc[uPlayerSkillType],     // Normal
            pGlobalTXT_LocalizationStrings[433], v35 + 3, v35 + 5, pExpertSkillDesc[uPlayerSkillType],     // Expert
            pGlobalTXT_LocalizationStrings[432], v35 + 3, v35 + 5, pMasterSkillDesc[uPlayerSkillType],     // Master
            pGlobalTXT_LocalizationStrings[96],  v35 + 3, v35 + 5, pGrandSkillDesc[uPlayerSkillType]);      // Grand
  }
  else
  {
    sprintf(Source, "\f%05d", Color16(0xFFu, 0xFFu, 0xFFu));
    strcat(a2, Source);
    strcat(a2, "%s: +%d");
    sprintf(static_sub_417BB5_out_string, a2, pSkillDesc[uPlayerSkillType],
            pGlobalTXT_LocalizationStrings[431], v35 + 3, v35 + 5, pNormalSkillDesc[uPlayerSkillType],
            pGlobalTXT_LocalizationStrings[433], v35 + 3, v35 + 5, pExpertSkillDesc[uPlayerSkillType],
            pGlobalTXT_LocalizationStrings[432], v35 + 3, v35 + 5, pMasterSkillDesc[uPlayerSkillType],
            pGlobalTXT_LocalizationStrings[96],  v35 + 3, v35 + 5, pGrandSkillDesc[uPlayerSkillType],
            pGlobalTXT_LocalizationStrings[623], //Bonus
            (pParty->pPlayers[uPlayerID].GetActualSkillLevel(uPlayerSkillType) & 0x3F) - (pParty->pPlayers[uPlayerID].pActiveSkills[uPlayerSkillType] & 0x3F));
  }
  return static_sub_417BB5_out_string;
}

//----- (00417FE5) --------------------------------------------------------
void CharacterUI_SkillsTab_ShowHint()
{
  GUIButton *pButton; // esi@6
  unsigned int pX; // [sp+4h] [bp-8h]@1
  unsigned int pY; // [sp+8h] [bp-4h]@1

  pMouse->GetClickPos(&pX, &pY);
  if ( (signed int)pX < 24 || (signed int)pX > 455 || (signed int)pY < 18 || (signed int)pY > 36 )
  {
    for ( pButton = pGUIWindow_CurrentMenu->pControlsHead; pButton; pButton = pButton->pNext )
    {
      if ( pButton->msg == UIMSG_SkillUp
        && (signed int)pX >= (signed int)pButton->uX
        && (signed int)pX <= (signed int)pButton->uZ
        && (signed int)pY >= (signed int)pButton->uY
        && (signed int)pY <= (signed int)pButton->uW )
      {
        CharacterUI_DrawTooltip(pSkillNames[pButton->msg_param], CharacterUI_GetSkillDescText(uActiveCharacter - 1, (PLAYER_SKILL_TYPE)pButton->msg_param));
      }
    }
  }
  else
    CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[207], pSkillPointsAttributeDescription);//Очки навыков
}

//----- (00418083) --------------------------------------------------------
void  CharacterUI_StatsTab_ShowHint()
{
  int pStringNum; // edi@1
  signed int pTextColor; // eax@15
  const char *pHourWord; // ecx@17
  const char *pDayWord; // eax@20
  int v15; // ebx@28
  POINT a2; // [sp+Ch] [bp-24h]@1
  int pHour; // [sp+14h] [bp-1Ch]@15
  unsigned int pDay; // [sp+24h] [bp-Ch]@15

  pMouse->GetCursorPos(&a2);
  for ( pStringNum = 0; pStringNum < stat_string_coord.size(); ++pStringNum )
  {
    if (a2.x >= stat_string_coord[pStringNum].x && a2.x <= stat_string_coord[pStringNum].x + stat_string_coord[pStringNum].width )
    {
      if (a2.y >= stat_string_coord[pStringNum].y && a2.y <= stat_string_coord[pStringNum].y + stat_string_coord[pStringNum].height )
        break;
    }
  }

  switch ( pStringNum )
  {
    case 0:// Attributes
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      if ( aAttributeNames[pStringNum] && pAttributeDescriptions[pStringNum] )
        CharacterUI_DrawTooltip(aAttributeNames[pStringNum], pAttributeDescriptions[pStringNum]);
      break;
    case 7:// Health Points
      if ( pGlobalTXT_LocalizationStrings[108] && pHealthPointsAttributeDescription )
        CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[108], pHealthPointsAttributeDescription);
      break;
    case 8:// Spell Points
      if ( pGlobalTXT_LocalizationStrings[212] && pSpellPointsAttributeDescription )
        CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[212], pSpellPointsAttributeDescription);
      break;
    case 9:// Armor Class
      if ( pGlobalTXT_LocalizationStrings[12] && pArmourClassAttributeDescription )
        CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[12], pArmourClassAttributeDescription);
      break;
    case 10:// Player Condition
      strcpy(pTmpBuf2.data(), pPlayerConditionAttributeDescription);
      strcat(pTmpBuf2.data(), "\n");
      extern std::array<unsigned int, 18> pConditionImportancyTable;
      for ( uint i = 0; i < 18; ++i )
      {
        if ( pPlayers[uActiveCharacter]->pConditions[pConditionImportancyTable[i]] )
        {
          strcat(pTmpBuf2.data(), " \n");
          pHour = pParty->uTimePlayed - pPlayers[uActiveCharacter]->pConditions[pConditionImportancyTable[i]];
          pHour = (unsigned int)((pHour * 0.234375) / 60 / 60);
          pDay = (unsigned int)pHour / 24;
          pHour %= 24i64;
          pTextColor = GetConditionDrawColor(pConditionImportancyTable[i]);
          sprintfex(pTmpBuf.data(), format_4E2DE8, pTextColor, aCharacterConditionNames[pConditionImportancyTable[i]]);
          strcat(pTmpBuf2.data(), pTmpBuf.data());
          if ( pHour && pHour <= 1 )
            pHourWord = pGlobalTXT_LocalizationStrings[109];
          else
            pHourWord = pGlobalTXT_LocalizationStrings[110];
          if ( !pDay || (pDayWord = pGlobalTXT_LocalizationStrings[56], pDay > 1) )
            pDayWord = pGlobalTXT_LocalizationStrings[57];
          sprintfex(pTmpBuf.data(), "%lu %s, %lu %s", pDay, pDayWord, pHour, pHourWord);
          strcat(pTmpBuf2.data(), pTmpBuf.data());
        }
      }
      if ( pGlobalTXT_LocalizationStrings[47] && pTmpBuf2.data() )
        CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[47], pTmpBuf2.data());
      break;
    case 11:// Fast Spell
      if ( pGlobalTXT_LocalizationStrings[172] && pFastSpellAttributeDescription )
        CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[172], pFastSpellAttributeDescription);
      break;
    case 12:// Player Age
      if ( pGlobalTXT_LocalizationStrings[5] && pPlayerAgeAttributeDescription )
        CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[5], pPlayerAgeAttributeDescription);
      break;
    case 13:// Player Level
      if ( pGlobalTXT_LocalizationStrings[131] && pPlayerLevelAttributeDescription )
        CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[131], pPlayerLevelAttributeDescription);
      break;
    case 14://Experience
      v15 = pPlayers[uActiveCharacter]->uLevel;
      do
      {
        if ( (signed __int64)pPlayers[uActiveCharacter]->uExperience < (unsigned int)GetExperienceRequiredForLevel(v15) )
          break;
        ++v15;
      }
      while ( v15 <= 10000 );
      pTmpBuf[0] = 0;
      pTmpBuf2[0] = 0;
      if ( v15 > pPlayers[uActiveCharacter]->uLevel )
        sprintf(pTmpBuf.data(), pGlobalTXT_LocalizationStrings[147], v15);
      sprintfex(pTmpBuf2.data(), pGlobalTXT_LocalizationStrings[538], GetExperienceRequiredForLevel(v15) - LODWORD(pPlayers[uActiveCharacter]->uExperience), v15 + 1);
      strcat(pTmpBuf.data(), "\n");
      strcat(pTmpBuf.data(), pTmpBuf2.data());
      sprintf(pTmpBuf2.data(), "%s\n \n%s", pPlayerExperienceAttributeDescription, pTmpBuf.data());
      if ( pGlobalTXT_LocalizationStrings[83] && pTmpBuf2.data() )
        CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[83], pTmpBuf2.data());
      break;
    case 15:// Attack Bonus
      if ( pGlobalTXT_LocalizationStrings[587] && pAttackBonusAttributeDescription )
        CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[587], pAttackBonusAttributeDescription);
      break;
    case 16:// Attack Damage
      if ( pGlobalTXT_LocalizationStrings[588] && pAttackDamageAttributeDescription )
        CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[588], pAttackDamageAttributeDescription);
      break;
    case 17:// Missle Bonus
      if ( pGlobalTXT_LocalizationStrings[589] && pMissleBonusAttributeDescription )
        CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[589], pMissleBonusAttributeDescription);
      break;
    case 18:// Missle Damage
      if ( pGlobalTXT_LocalizationStrings[590] && pMissleDamageAttributeDescription )
        CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[590], pMissleDamageAttributeDescription);
      break;
    case 19:// Fire Resistance
      if ( pGlobalTXT_LocalizationStrings[87] && pFireResistanceAttributeDescription )
        CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[87], pFireResistanceAttributeDescription);
      break;
    case 20:// Air Resistance
      if ( pGlobalTXT_LocalizationStrings[6] && pAirResistanceAttributeDescription )
        CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[6], pAirResistanceAttributeDescription);
      break;
    case 21:// Water Resistance
      if ( pGlobalTXT_LocalizationStrings[240] && pWaterResistanceAttributeDescription )
        CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[240], pWaterResistanceAttributeDescription);
      break;
    case 22:// Earth Resistance
      if ( pGlobalTXT_LocalizationStrings[70] && pEarthResistanceAttributeDescription )
        CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[70], pEarthResistanceAttributeDescription);
      break;
    case 23:// Mind Resistance
      if ( pGlobalTXT_LocalizationStrings[142] && pMindResistanceAttributeDescription )
        CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[142], pMindResistanceAttributeDescription);
      break;
    case 24:// Body Resistance
      if ( pGlobalTXT_LocalizationStrings[29] && pBodyResistanceAttributeDescription )
        CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[29], pBodyResistanceAttributeDescription);
      break;
    case 25: // Skill Points
      if ( pGlobalTXT_LocalizationStrings[207] && pSkillPointsAttributeDescription )
        CharacterUI_DrawTooltip(pGlobalTXT_LocalizationStrings[207], pSkillPointsAttributeDescription);
      break;
    case 26: // Class description
    {
      if (pClassDescriptions[pPlayers[uActiveCharacter]->classType] && pClassNames[pPlayers[uActiveCharacter]->classType])
        CharacterUI_DrawTooltip(pClassNames[pPlayers[uActiveCharacter]->classType], pClassDescriptions[pPlayers[uActiveCharacter]->classType]);
    }
    break;

    default:
      break;
  }
}

//----- (00410B28) --------------------------------------------------------
void  DrawSpellDescriptionPopup(int spell_index)
{
  SpellInfo *spell; // esi@1
  unsigned int v3; // eax@2
  LONG v5; // ecx@4
  GUIWindow spell_info_window; // [sp+Ch] [bp-68h]@4
  POINT mouse; // [sp+64h] [bp-10h]@1

  spell = &pSpellStats->pInfos[spell_index + 11 * pPlayers[uActiveCharacter]->lastOpenedSpellbookPage + 1];
  if ( pMouse->GetCursorPos(&mouse)->y <= 250 )
    v3 = pMouse->GetCursorPos(&mouse)->y + 30;
  else
    v3 = 30;
  spell_info_window.uFrameY = v3;
  spell_info_window.uFrameWidth = 328;
  spell_info_window.uFrameHeight = 68;
  spell_info_window.uFrameX = 90;
  spell_info_window.uFrameZ = 417;
  spell_info_window.uFrameW = v3 + 67;
  spell_info_window.Hint = nullptr;
  v5 = pFontSmallnum->GetLineWidth(pGlobalTXT_LocalizationStrings[LOCSTR_NORMAL]);
  if ( pFontSmallnum->GetLineWidth(pGlobalTXT_LocalizationStrings[LOCSTR_MASTER]) > v5 )
    v5 = pFontSmallnum->GetLineWidth(pGlobalTXT_LocalizationStrings[LOCSTR_MASTER]);
  if ( pFontSmallnum->GetLineWidth(pGlobalTXT_LocalizationStrings[LOCSTR_EXPERT]) > v5 )
    v5 = pFontSmallnum->GetLineWidth(pGlobalTXT_LocalizationStrings[LOCSTR_EXPERT]);
  if ( pFontSmallnum->GetLineWidth(pGlobalTXT_LocalizationStrings[LOCSTR_GRAND]) > v5 )
    v5 = pFontSmallnum->GetLineWidth(pGlobalTXT_LocalizationStrings[LOCSTR_GRAND]);
  sprintf(pTmpBuf2.data(),
            "%s\n\n%s\t%03d:\t%03d%s\t000\n%s\t%03d:\t%03d%s\t000\n%s\t%03d:\t%03d%s\t000\n%s\t%03d:\t%03d%s",
            spell->pDescription,
            pGlobalTXT_LocalizationStrings[LOCSTR_NORMAL], v5 + 3, v5 + 10, spell->pBasicSkillDesc,
            pGlobalTXT_LocalizationStrings[LOCSTR_EXPERT], v5 + 3, v5 + 10, spell->pExpertSkillDesc,
            pGlobalTXT_LocalizationStrings[LOCSTR_MASTER], v5 + 3, v5 + 10, spell->pMasterSkillDesc,
            pGlobalTXT_LocalizationStrings[LOCSTR_GRAND], v5 + 3,  v5 + 10, spell->pGrandmasterSkillDesc);
  spell_info_window.uFrameHeight += pFontSmallnum->CalcTextHeight(pTmpBuf2.data(), &spell_info_window, 0, 0);
  if ( (signed int)spell_info_window.uFrameHeight < 150 )
    spell_info_window.uFrameHeight = 150;
  spell_info_window.uFrameWidth = game_viewport_width;
  spell_info_window.DrawMessageBox(0);
  spell_info_window.uFrameWidth -= 12;
  spell_info_window.uFrameHeight -= 12;
  spell_info_window.uFrameZ = spell_info_window.uFrameX + spell_info_window.uFrameWidth - 1;
  spell_info_window.uFrameW = spell_info_window.uFrameHeight + spell_info_window.uFrameY - 1;
  spell_info_window.DrawTitleText(pFontArrus, 0x78u, 0xCu, Color16(0xFFu, 0xFFu, 0x9Bu), spell->pName, 3);
  spell_info_window.DrawText(pFontSmallnum, 120, 44, 0, pTmpBuf2.data(), 0, 0, 0);
  spell_info_window.uFrameWidth = 108;
  spell_info_window.uFrameZ = spell_info_window.uFrameX + 107;
  int skill_level = SkillToMastery(pPlayers[uActiveCharacter]->pActiveSkills[pPlayers[uActiveCharacter]->lastOpenedSpellbookPage + 12]);
  spell_info_window.DrawTitleText(pFontComic, 12, 75, 0, pSkillNames[pPlayers[uActiveCharacter]->lastOpenedSpellbookPage + 12], 3);
  sprintf( pTmpBuf.data(),  "%s\n%d",    pGlobalTXT_LocalizationStrings[LOCSTR_SP_COST],
       pSpellDatas[spell_index + 11 * pPlayers[uActiveCharacter]->lastOpenedSpellbookPage + 1].mana_per_skill[skill_level - 1]);
  spell_info_window.DrawTitleText(pFontComic, 12, spell_info_window.uFrameHeight - LOBYTE(pFontComic->uFontHeight) - 16, 0, pTmpBuf.data(), 3);
  dword_507B00_spell_info_to_draw_in_popup = 0;
}


//----- (00416D62) --------------------------------------------------------
void UI_OnMouseRightClick(Vec2_int_ *_this)
{
  int v5; // esi@62
  GUIButton *pButton; // esi@84
  const char *pStr; // edi@85
  const char *pHint; // edx@113
  GUIWindow popup_window; // [sp+4h] [bp-74h]@32
  struct tagPOINT Point; // [sp+60h] [bp-18h]@6
  unsigned int pX; // [sp+70h] [bp-8h]@3
  unsigned int pY; // [sp+74h] [bp-4h]@3

  if ( current_screen_type == SCREEN_VIDEO || GetCurrentMenuID() == MENU_MAIN )
    return;
  if ( _this )
  {
    pX = _this->x;
    pY = _this->y;
  }
  else
  {
    pMouse->GetClickPos(&pX, &pY);
  }
  //if ( pRenderer->bWindowMode )
  {
    GetCursorPos(&Point);
    ScreenToClient(window->GetApiHandle(), &Point);
    if ( Point.x < 1 || Point.y < 1 || Point.x > 638 || Point.y > 478 )
    {
      back_to_game();
      return;
    }
  }
  if ( pParty->pPickedItem.uItemID )//нажатие на портрет перса правой кнопкой мыши с раствором
  {
    for ( uint i = 0; i < 4; ++i )
    {
      if ( (signed int)pX > RightClickPortraitXmin[i] && (signed int)pX < RightClickPortraitXmax[i]
        && (signed int)pY > 375 && (signed int)pY < 466 )
      {
        pPlayers[uActiveCharacter]->UseItem_DrinkPotion_etc(i + 1, 1);
        return;
      }
    }
  }

  pEventTimer->Pause();
  switch(current_screen_type)
  {
    case SCREEN_CASTING:
    {
      Inventory_ItemPopupAndAlchemy();
      break;
    }
    case SCREEN_CHEST:
    {
      if ( !pPlayers[uActiveCharacter]->CanAct() )
      {
        sprintf(pTmpBuf.data(), pGlobalTXT_LocalizationStrings[427], pPlayers[uActiveCharacter]->pName, pGlobalTXT_LocalizationStrings[541]);//%s не в состоянии %s Опознать предметы
        popup_window.Hint = pTmpBuf.data();
        popup_window.uFrameWidth = 384;
        popup_window.uFrameHeight = 180;
        popup_window.uFrameY = 40;
        if ( (signed int)pX <= 320 )
          popup_window.uFrameX = pX + 30;
        else
          popup_window.uFrameX = pX - 414;
        popup_window.DrawMessageBox(0);
      }
      else
      {
        if ( pRenderer->pActiveZBuffer[pX + pSRZBufferLineOffsets[pY]] & 0xFFFF )
          GameUI_DrawItemInfo(&pChests[pChestWindow->par1C].igChestItems[pChests[pChestWindow->par1C].pInventoryIndices[(pRenderer->pActiveZBuffer[pX + pSRZBufferLineOffsets[pY]] & 0xFFFF) - 1] - 1]);
      }
      break;
    }
    case SCREEN_GAME://In the main menu displays a pop-up window(В главном меню показывает всплывающее окно)
    {
      if (GetCurrentMenuID() > 0)
        break;
      if ( (signed int)pY > (signed int)pViewport->uViewportBR_Y )
      {
        popup_window.ptr_1C = (void *)((signed int)pX / 118);
        if ( (signed int)pX / 118 < 4 )//portaits zone
        {
          popup_window.Hint = nullptr;
          popup_window.uFrameWidth = 400;
          popup_window.uFrameHeight = 200;
          popup_window.uFrameX = 38;
          popup_window.uFrameY = 60;
          pAudioPlayer->StopChannels(-1, -1);
          GameUI_CharacterQuickRecord_Draw(&popup_window, pPlayers[(int)popup_window.ptr_1C + 1]);
        }
      }
      else if ( (signed int)pX > (signed int)pViewport->uViewportBR_X )
      {
        if ( (signed int)pY >= 130 )
        {
          if ( (signed int)pX >= 476 && (signed int)pX <= 636 && (signed int)pY >= 240 && (signed int)pY <= 300 )//buff_tooltip zone
          {
            popup_window.Hint = nullptr;
            popup_window.uFrameWidth = 400;
            popup_window.uFrameHeight = 200;
            popup_window.uFrameX = 38;
            popup_window.uFrameY = 60;
            pAudioPlayer->StopChannels(-1, -1);
            popup_window._41D73D_draw_buff_tooltip();
          }
          else if ( (signed int)pX < 485 || (signed int)pX > 548 || (signed int)pY < 156 || (signed int)pY > 229 )//NPC zone
          {
            if (!( (signed int)pX < 566 || (signed int)pX > 629 || (signed int)pY < 156 || (signed int)pY > 229 ))
            {
              pAudioPlayer->StopChannels(-1, -1);
              GameUI_DrawNPCPopup((void *)1);//NPC 2
            }
          }
          else
          {
            pAudioPlayer->StopChannels(-1, -1);
            GameUI_DrawNPCPopup(0);//NPC 1
          }
        }
        else//minimap zone
        {
          popup_window.Hint = (char *)GameUI_GetMinimapHintText();
          popup_window.uFrameWidth = 256;
          popup_window.uFrameX = 130;
          popup_window.uFrameY = 140;
          popup_window.uFrameHeight = 64;
          pAudioPlayer->StopChannels(-1, -1);
          popup_window.DrawMessageBox(0);
        }
      }
      else//game zone
      {
        popup_window.Hint = nullptr;
        popup_window.uFrameWidth = 320;
        popup_window.uFrameHeight = 320;
        popup_window.uFrameX = pX - 350;
        if ( (signed int)pX <= 320 )
          popup_window.uFrameX = pX + 30;
        popup_window.uFrameY = 40;
        //if ( pRenderer->pRenderD3D )
          v5 = pEngine->pVisInstance->get_picked_object_zbuf_val();
        /*else
          v5 = pRenderer->pActiveZBuffer[pX + pSRZBufferLineOffsets[pY]];*/
        if (PID_TYPE((unsigned __int16)v5) == OBJECT_Actor)
        {
          /*if ( pRenderer->uNumSceneBegins )
          {
            popup_window.DrawMessageBox(1);
            MonsterPopup_Draw(PID_ID((unsigned __int16)v5), &popup_window);
          }
          else*/
          {
            pRenderer->BeginScene();
            popup_window.DrawMessageBox(1);
            MonsterPopup_Draw(PID_ID((unsigned __int16)v5), &popup_window);
            pRenderer->EndScene();
          }
        }
        if (PID_TYPE((unsigned __int16)v5) == OBJECT_Item)
        {
          if ( !(pObjectList->pObjects[pSpriteObjects[PID_ID((unsigned __int16)v5)].uObjectDescID].uFlags & 0x10 ) )
          {
              GameUI_DrawItemInfo(&pSpriteObjects[PID_ID((unsigned __int16)v5)].containing_item);
          }
        }
      }
      break;
    }
    case SCREEN_BOOKS:
    {
      if ( !dword_506364
        || (signed int)pX < (signed int)pViewport->uViewportTL_X || (signed int)pX > (signed int)pViewport->uViewportBR_X
        || (signed int)pY < (signed int)pViewport->uViewportTL_Y || (signed int)pY > (signed int)pViewport->uViewportBR_Y
        || ((popup_window.Hint = (char *)GetMapBookHintText()) == 0) )
        break;
      popup_window.uFrameWidth = (pFontArrus->GetLineWidth(popup_window.Hint) + 32) + 0.5f;
      popup_window.uFrameX = pX + 5;
      popup_window.uFrameY = pY + 5;
      popup_window.uFrameHeight = 64;
      pAudioPlayer->StopChannels(-1, -1);
      popup_window.DrawMessageBox(0);
      break;
    }
    case SCREEN_CHARACTERS:
    case SCREEN_E:
    case SCREEN_CHEST_INVENTORY:
    {
      if ( (signed int)pX > 467 && current_screen_type != SCREEN_E )
        Inventory_ItemPopupAndAlchemy();
      else if ( (signed int)pY >= 345 )
        break;
      else if (current_character_screen_window == WINDOW_CharacterWindow_Stats)//2DEvent - CharacerScreenStats
        CharacterUI_StatsTab_ShowHint();
      else if (current_character_screen_window == WINDOW_CharacterWindow_Skills)//2DEvent - CharacerScreenSkills
        CharacterUI_SkillsTab_ShowHint();
      else if (current_character_screen_window == WINDOW_CharacterWindow_Inventory)//2DEvent - CharacerScreenInventory
          Inventory_ItemPopupAndAlchemy();
      break;
    }
    case SCREEN_SPELL_BOOK:
    {
      if ( dword_507B00_spell_info_to_draw_in_popup )
        DrawSpellDescriptionPopup(dword_507B00_spell_info_to_draw_in_popup - 1);
      break;
    }
    case SCREEN_HOUSE:
    {
      if ( (signed int)pY < 345 && (signed int)pX < 469 )
        ShowPopupShopItem();
      break;
    }
    case SCREEN_PARTY_CREATION:
    {
      popup_window.Hint = nullptr;
      pStr = 0;
      for ( pButton = pGUIWindow_CurrentMenu->pControlsHead; pButton; pButton = pButton->pNext)
      {
        if ( pButton->uButtonType == 1 && pButton->uButtonType != 3 && (signed int)pX > (signed int)pButton->uX && (signed int)pX < (signed int)pButton->uZ
             && (signed int)pY > (signed int)pButton->uY && (signed int)pY < (signed int)pButton->uW )
        {
          switch ( pButton->msg )
          {
            case UIMSG_0: //stats info
              popup_window.Hint = pAttributeDescriptions[(signed int)pButton->msg_param % 7];
              pStr = aAttributeNames[(signed int)pButton->msg_param % 7];
              break;
            case UIMSG_PlayerCreationClickPlus: //Plus button info 
              pStr = pGlobalTXT_LocalizationStrings[670];//Добавить
              popup_window.Hint = pGlobalTXT_LocalizationStrings[671];//"Добавляет очко к выделенному навыку, забирая его из накопителя очков"
              break;
            case UIMSG_PlayerCreationClickMinus: //Minus button info
              pStr = pGlobalTXT_LocalizationStrings[668];//Вычесть
              popup_window.Hint = pGlobalTXT_LocalizationStrings[669];//"Вычитает очко из выделенного навыка, возвращая его в накопитель очков"
              break;
            case UIMSG_PlayerCreationSelectActiveSkill: //Available skill button info
              pStr = pSkillNames[pParty->pPlayers[uPlayerCreationUI_SelectedCharacter].GetSkillIdxByOrder(pButton->msg_param + 4)];
              popup_window.Hint = pSkillDesc[pParty->pPlayers[uPlayerCreationUI_SelectedCharacter].GetSkillIdxByOrder(pButton->msg_param + 4)];
              break;
            case UIMSG_PlayerCreationSelectClass: //Available Class Info
              popup_window.Hint = pClassDescriptions[pButton->msg_param];
              pStr = pClassNames[pButton->msg_param];
              break;
            case UIMSG_PlayerCreationClickOK: //OK Info
              popup_window.Hint = pGlobalTXT_LocalizationStrings[664];//Щелкните здесь для утверждения состава отряда и продолжения игры.
              pStr = pGlobalTXT_LocalizationStrings[665];//Кнопка ОК
              break;
            case UIMSG_PlayerCreationClickReset: //Clear info
              popup_window.Hint = pGlobalTXT_LocalizationStrings[666];//Сбрасывает все параметры и навыки отряда.
              pStr = pGlobalTXT_LocalizationStrings[667];//Кнопка Очистить
              break;
            case UIMSG_PlayerCreation_SelectAttribute: // Character info
              pStr = pParty->pPlayers[pButton->msg_param].pName;
              popup_window.Hint = pClassDescriptions[pParty->pPlayers[pButton->msg_param].classType];
              break;
          }
          if ( pButton->msg > UIMSG_44 && pButton->msg <= UIMSG_PlayerCreationRemoveDownSkill ) //Sellected skills info
          {
            pY = 0;
            if ( (signed int)pParty->pPlayers[pButton->msg_param].GetSkillIdxByOrder(pButton->msg - UIMSG_48) < 37 )
            {
              strcpy(pTmpBuf2.data(), CharacterUI_GetSkillDescText(pButton->msg_param, (PLAYER_SKILL_TYPE)pParty->pPlayers[pButton->msg_param].GetSkillIdxByOrder(pButton->msg - UIMSG_48)));
              popup_window.Hint = pTmpBuf2.data();
              pStr = pSkillNames[pParty->pPlayers[pButton->msg_param].GetSkillIdxByOrder(pButton->msg - UIMSG_48)];
            }
          }
        }
      }
      if ( popup_window.Hint )
      {
        pHint = popup_window.Hint;
        popup_window.Hint = nullptr;
        popup_window.uFrameWidth = 384;
        popup_window.uFrameHeight = 256;
        popup_window.uFrameX = 128;
        popup_window.uFrameY = 40;
        popup_window.uFrameHeight = pFontSmallnum->CalcTextHeight(pHint, &popup_window, 24, 0) + 2 * LOBYTE(pFontLucida->uFontHeight) + 24;
        popup_window.uFrameZ = popup_window.uFrameX + popup_window.uFrameWidth - 1;
        popup_window.uFrameW = popup_window.uFrameY + popup_window.uFrameHeight - 1;
        popup_window.DrawMessageBox(0);
        popup_window.uFrameX += 12;
        popup_window.uFrameWidth -= 24;
        popup_window.uFrameY += 12;
        popup_window.uFrameHeight -= 12;
        popup_window.uFrameZ = popup_window.uFrameX + popup_window.uFrameWidth - 1;
        popup_window.uFrameW = popup_window.uFrameY + popup_window.uFrameHeight - 1;
        sprintf(pTmpBuf.data(), "\f%05d%s\f00000\n", Color16(0xFF, 0xFF, 0x9B), pStr);
        popup_window.DrawTitleText(pFontCreate, 0, 0, 0, pTmpBuf.data(), 3);
        popup_window.DrawText(pFontSmallnum, 1, pFontLucida->uFontHeight, 0, pHint, 0, 0, 0);
      }
      break;
    }
    default:
      break;
  }
  dword_507BF0_is_there_popup_onscreen = 1;
  viewparams->bRedrawGameUI = 1;
}
int no_rightlick_in_inventory = false; // 0050CDCC
//----- (00416196) --------------------------------------------------------
void Inventory_ItemPopupAndAlchemy()
{
  int potion1_id; // edx@25
  unsigned int potion2_id; // edi@25
  signed int potionID; // edx@27
  unsigned int pOut_y; // edx@57
  double v31; // st7@112
  Vec3_int_ v39; // [sp-18h] [bp-A8h]@83
  GUIWindow message_window; // [sp+Ch] [bp-84h]@137
  POINT cursor; // [sp+78h] [bp-18h]@2
  unsigned int damage_level; // [sp+8Ch] [bp-4h]@23

  if (no_rightlick_in_inventory)
    return;

  pMouse->GetCursorPos(&cursor);
  int item_pid = (pRenderer->pActiveZBuffer[cursor.x + pSRZBufferLineOffsets[cursor.y]] & 0xFFFF) - 1;
  if (item_pid == -1) //added here to avoid crash
    return;
  ItemGen* item = &pPlayers[uActiveCharacter]->pInventoryItemList[item_pid];

  if (cursor.x <= 13 || cursor.x >= 462)//items out of inventory(вещи вне инвентаря)
  {
    GameUI_DrawItemInfo(item);
    return;
  }

  if (!item_pid)
  {
    int inventory_mouse_x = cursor.x - 14;
    int inventory_mouse_y = cursor.y - 17;

    int mouse_cell_x = inventory_mouse_x / 32;
    int mouse_cell_y = inventory_mouse_y / 32;

    if (mouse_cell_x + mouse_cell_y < 0)
      return;

    int inventory_idx = mouse_cell_x + 14 * mouse_cell_y;
    if (inventory_idx > 126)
      return;

    if (pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex(&inventory_idx) == 0)
      return;

    item_pid = pPlayers[uActiveCharacter]->GetItemIDAtInventoryIndex(&inventory_idx);
  }
//check character condition(проверка состояния персонажа)
  if (!pPlayers[uActiveCharacter]->CanAct())
  {
    sprintfex(pTmpBuf.data(), pGlobalTXT_LocalizationStrings[427], pPlayers[uActiveCharacter]->pName, pGlobalTXT_LocalizationStrings[541]);//%s не в состоянии %s Опознать предметы
    message_window.Hint = pTmpBuf.data();
    message_window.uFrameWidth = 384;
    message_window.uFrameHeight = 180;
    if (cursor.x <= 320 )
      message_window.uFrameX = cursor.x + 30;
    else
      message_window.uFrameX = cursor.x - 414;
    message_window.uFrameY = 40;
    message_window.DrawMessageBox(0);
    return;
  }

  int alchemy_skill_points = (int8_t)pPlayers[uActiveCharacter]->GetActualSkillLevel(PLAYER_SKILL_ALCHEMY) & 0x3F;
  int alchemy_skill_level = SkillToMastery(pPlayers[uActiveCharacter]->GetActualSkillLevel(PLAYER_SKILL_ALCHEMY));

// for potion bottle(простая бутылка)
  if (pParty->pPickedItem.uItemID == ITEM_POTION_BOTTLE)
  {
    GameUI_DrawItemInfo(item);
    return;
  }
//for recharge potion(зелье перезарядка)
  if (pParty->pPickedItem.uItemID == ITEM_POTION_RECHARGE_ITEM)
  {
    if (item->uItemID < ITEM_POTION_BOTTLE || item->uItemID > ITEM_POTION_REJUVENATION)// all potions
    {
      if (item->GetItemEquipType() != EQUIP_WAND) // can recharge only wands
      {
        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0, 0, 0);
        return;
      }

      v31 = (70.0 - (double)pParty->pPickedItem.uEnchantmentType) * 0.01;
      if ( v31 < 0.0 )
        v31 = 0.0;
      item->uMaxCharges = (signed __int64)((double)item->uMaxCharges - v31 * (double)item->uMaxCharges);
      item->uNumCharges = (signed __int64)((double)item->uMaxCharges - v31 * (double)item->uMaxCharges);

      pMouse->RemoveHoldingItem();
      no_rightlick_in_inventory = 1;
      return;
    }
    GameUI_DrawItemInfo(item);
    return;
  }
// for harden potion(зелье закалка)
  else if (pParty->pPickedItem.uItemID == ITEM_POTION_HARDEN_ITEM)
  {
    if (item->uItemID < ITEM_POTION_BOTTLE || item->uItemID > ITEM_POTION_REJUVENATION) // bottle and all potions
    {
      if (item->IsBroken() ||                         // cant harden broken items
          item->uItemID >= ITEM_ARTIFACT_PUCK ||      // cant harden artifacts
          item->GetItemEquipType() < EQUIP_SINGLE_HANDED ||
          item->GetItemEquipType() > EQUIP_WAND)
      {
        pMouse->RemoveHoldingItem();
        no_rightlick_in_inventory = true;
        return;
      }

      item->uAttributes |= ITEM_AURA_EFFECT_RED | ITEM_HARDENED;

      _50C9A8_item_enchantment_timer = 256;
      pMouse->RemoveHoldingItem();
      no_rightlick_in_inventory = true;
      return;
    }
    GameUI_DrawItemInfo(item);
    return;
  }
// several potions(несколько зелий)
  else if (pParty->pPickedItem.uItemID >= ITEM_POTION_FLAMING_POTION && pParty->pPickedItem.uItemID <= ITEM_POTION_SWIFT_POTION ||
           pParty->pPickedItem.uItemID == ITEM_POTION_SLAYING_POTION)
  {
    if ( item->uItemID < ITEM_POTION_BOTTLE || item->uItemID > ITEM_POTION_REJUVENATION) // all potions
    {
      if (item->uItemID >= ITEM_BLASTER && item->uItemID <= ITEM_LASER_RIFLE ||
          item->uItemID >= ITEM_ARTIFACT_PUCK ||
          item->IsBroken() ||
          item->special_enchantment ||
          item->uEnchantmentType ||
          item->GetItemEquipType() >= EQUIP_ARMOUR)  // only melee weapons and bows
      {
        pMouse->RemoveHoldingItem();
        no_rightlick_in_inventory = true;
        return;
      }
      
      item->UpdateTempBonus(pParty->uTimePlayed);
      if (pParty->pPickedItem.uItemID == ITEM_POTION_SLAYING_POTION)
      {
          item->special_enchantment = ITEM_ENCHANTMENT_40; // of Slaying
        v31 = (double)(1800 * pParty->pPickedItem.uEnchantmentType * 128);
      }
      else
      {
          static ITEM_ENCHANTMENT _4E2904_enchantment_by_potion_lut[] =
          {
              (ITEM_ENCHANTMENT)164, (ITEM_ENCHANTMENT)93, (ITEM_ENCHANTMENT)22,
              (ITEM_ENCHANTMENT)164, (ITEM_ENCHANTMENT)93, (ITEM_ENCHANTMENT)22,
              ITEM_ENCHANTMENT_OF_FLAME,
              ITEM_ENCHANTMENT_OF_FROST,
              ITEM_ENCHANTMENT_OF_POISON,
              ITEM_ENCHANTMENT_OF_SPARKS,
              (ITEM_ENCHANTMENT)59
          };
        item->special_enchantment = _4E2904_enchantment_by_potion_lut[pParty->pPickedItem.uItemID - 240];
        v31 = (double)(1800 * pParty->pPickedItem.uEnchantmentType * 128);
      }

      item->uExpireTime = pParty->uTimePlayed + v31 * 0.033333335;
      item->uAttributes = alchemy_skill_level | 0x18;

      _50C9A8_item_enchantment_timer = 256;
      pMouse->RemoveHoldingItem();
      no_rightlick_in_inventory = true;
      return;
    }
    GameUI_DrawItemInfo(item);
    return;
  }
  // use reagents(применение реагентов)
  if (pParty->pPickedItem.uItemID >= ITEM_REAGENT_WIDOWSWEEP_BERRIES && pParty->pPickedItem.uItemID <= ITEM_REAGENT_PHILOSOPHERS_STONE &&
      pPlayers[uActiveCharacter]->pInventoryItemList[item_pid].uItemID == ITEM_POTION_BOTTLE)
  {
    pPlayers[uActiveCharacter]->pInventoryItemList[item_pid].uEnchantmentType = alchemy_skill_points + pParty->pPickedItem.GetDamageDice();
    switch ( pParty->pPickedItem.uItemID )
    {
    case ITEM_REAGENT_WIDOWSWEEP_BERRIES:
    case ITEM_REAGENT_CRUSHED_ROSE_PETALS:
    case ITEM_TROLL_BLOOD:
    case ITEM_TROLL_RUBY:
    case ITEM_DRAGON_EYE:
          pPlayers[uActiveCharacter]->pInventoryItemList[item_pid].uItemID = ITEM_POTION_CURE_WOUNDS;
        break;

    case ITEM_PHIMA_ROOT:
    case ITEM_METEORITE_FRAGMENT:
    case ITEM_HARPY_FEATHER:
    case ITEM_MOONSTONE:
    case ITEM_ELVISH_TOADSTOOL:
          pPlayers[uActiveCharacter]->pInventoryItemList[item_pid].uItemID = ITEM_POTION_MAGIC_POTION;
        break;

    case ITEM_POPPYSNAPS:
    case ITEM_FAE_DUST:
    case ITEM_SULFUR:
    case ITEM_GARNET:
    case ITEM_DEVIL_ICHOR:
          pPlayers[uActiveCharacter]->pInventoryItemList[item_pid].uItemID = ITEM_POTION_CURE_WEAKNESS;
        break;

    case ITEM_MUSHROOM:
    case ITEM_OBSIDIAN:
    case ITEM_OOZE_ENDOPLASM_VIAL:
    case ITEM_MERCURY:
    case ITEM_REAGENT_PHILOSOPHERS_STONE:
          pPlayers[uActiveCharacter]->pInventoryItemList[item_pid].uItemID = ITEM_POTION_CATALYST;
        break;
      default:
        break;
    }
    pMouse->RemoveHoldingItem();
    no_rightlick_in_inventory = 1;
    if ( dword_4E455C )
    {
      pPlayers[uActiveCharacter]->PlaySound(SPEECH_DO_POTION_FINE, 0);
      dword_4E455C = 0;
    }
    return;
  }
//potions mixing(смешивание двух зелий)
  if (pParty->pPickedItem.uItemID >= ITEM_POTION_CATALYST && pParty->pPickedItem.uItemID <= ITEM_POTION_REJUVENATION &&
      pPlayers[uActiveCharacter]->pInventoryItemList[item_pid].uItemID >= ITEM_POTION_CATALYST &&
      pPlayers[uActiveCharacter]->pInventoryItemList[item_pid].uItemID <= ITEM_POTION_REJUVENATION)
  {
    potion1_id = item->uItemID - ITEM_POTION_CURE_WOUNDS;
    potion2_id = pParty->pPickedItem.uItemID - ITEM_POTION_CURE_WOUNDS;

    if ( pParty->pPickedItem.uItemID == ITEM_POTION_CATALYST || item->uItemID == ITEM_POTION_CATALYST )
      potionID = 5;
    else
      potionID = pItemsTable->potion_data[potion2_id][potion1_id];
    damage_level = 0;
    if ( alchemy_skill_points )
    {
      if ( potionID < ITEM_POTION_CURE_DISEASE || potionID > ITEM_POTION_AWAKEN )//< 225 >227
      {
        if ( potionID >= ITEM_POTION_HASTE && potionID <= ITEM_POTION_CURE_INSANITY && alchemy_skill_level == 1)//228 >= potionID <= 239
          damage_level = 2;
        if ( potionID >= ITEM_POTION_MIGHT_BOOST && potionID <= ITEM_POTION_BODY_RESISTANE && alchemy_skill_level <= 2)//240 >= potionID <= 261
          damage_level = 3;
        if ( potionID >= ITEM_POTION_STONE_TO_FLESH && alchemy_skill_level <= 3 )// 262 < potionID
          damage_level = 4;
      }
    }
    else//no skill(нет навыка)
    {
      if ( potionID >= ITEM_POTION_CURE_DISEASE && potionID <= ITEM_POTION_AWAKEN )//225 <= v16 <= 227
        damage_level = 1;
      if ( potionID >= ITEM_POTION_HASTE && potionID <= ITEM_POTION_CURE_INSANITY )//228 <= v16 <= 239
        damage_level = 2;
      if ( potionID >= ITEM_POTION_MIGHT_BOOST && potionID <= ITEM_POTION_BODY_RESISTANE )//240 <= v16 <= 261
        damage_level = 3;
      if ( potionID >= ITEM_POTION_STONE_TO_FLESH )//262 <= v16
        damage_level = 4;
    }
    
    int pOut_x = item_pid + 1;
    for ( uint i = 0; i < 126; ++i )
    {
      if ( pPlayers[uActiveCharacter]->pInventoryMatrix[i] == pOut_x )
      {
        pOut_y = i;
        break;
      }
    }
    if ( !potionID )
    {
      GameUI_DrawItemInfo(item);
      return;
    }

    if (damage_level > 0)
    {
        pPlayers[uActiveCharacter]->RemoveItemAtInventoryIndex(pOut_y);

        if (damage_level == 1)
        {
            pPlayers[uActiveCharacter]->ReceiveDamage(rand() % 11 + 10, DMGT_FIRE);
        }
        else if (damage_level == 2)
        {
            pPlayers[uActiveCharacter]->ReceiveDamage(rand() % 71 + 30, DMGT_FIRE);
            pPlayers[uActiveCharacter]->ItemsEnchant(1);
        }
        else if (damage_level == 3)
        {
            pPlayers[uActiveCharacter]->ReceiveDamage(rand() % 201 + 50, DMGT_FIRE);
            pPlayers[uActiveCharacter]->ItemsEnchant(5);        
        }
        else if (damage_level >= 4)
        {
            pPlayers[uActiveCharacter]->SetCondition(Condition_Eradicated, 0);
            pPlayers[uActiveCharacter]->ItemsEnchant(0);
        }

        pAudioPlayer->PlaySound(SOUND_fireBall, 0, 0, -1, 0, 0, 0, 0);
        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);
        v39.z = pParty->vPosition.z + pParty->sEyelevel;
        v39.x = pParty->vPosition.x;
        v39.y = pParty->vPosition.y;

        int rot_x, rot_y, rot_z;
        Vec3_int_::Rotate(64, pParty->sRotationY, pParty->sRotationX, v39, &rot_x, &rot_y, &rot_z);
        SpriteObject::sub_42F7EB_DropItemAt(SPRITE_SPELL_FIRE_FIREBALL_IMPACT, rot_x, rot_y, rot_z, 0, 1, 0, 0, 0);
        if (dword_4E455C)
        {
            if (pPlayers[uActiveCharacter]->CanAct())
                pPlayers[uActiveCharacter]->PlaySound(SPEECH_17, 0);
            ShowStatusBarString(pGlobalTXT_LocalizationStrings[444], 2);//Ой!
            dword_4E455C = 0;
        }
        pMouse->RemoveHoldingItem();
        no_rightlick_in_inventory = 1;
        return;
    }
    else //if ( damage_level == 0 )
    {
      if ( alchemy_skill_points )
      {
          if (pPlayers[uActiveCharacter]->pInventoryItemList[item_pid].uItemID == ITEM_POTION_CATALYST || pParty->pPickedItem.uItemID == ITEM_POTION_CATALYST)
        {
            if (pPlayers[uActiveCharacter]->pInventoryItemList[item_pid].uItemID == ITEM_POTION_CATALYST)
            pPlayers[uActiveCharacter]->pInventoryItemList[item_pid].uItemID = pParty->pPickedItem.uItemID;
          if (pParty->pPickedItem.uItemID == ITEM_POTION_CATALYST)
            pPlayers[uActiveCharacter]->pInventoryItemList[item_pid].uEnchantmentType = pParty->pPickedItem.uEnchantmentType;
        }
        else
        {
          pPlayers[uActiveCharacter]->pInventoryItemList[item_pid].uItemID = potionID;
          pPlayers[uActiveCharacter]->pInventoryItemList[item_pid].uEnchantmentType = (pParty->pPickedItem.uEnchantmentType
                                            + pPlayers[uActiveCharacter]->pInventoryItemList[item_pid].uEnchantmentType) / 2;
          pPlayers[uActiveCharacter]->SetVariable(VAR_AutoNotes, pItemsTable->potion_note[potion1_id][potion2_id]);
        }
          int bottle = pPlayers[uActiveCharacter]->AddItem(-1, ITEM_POTION_BOTTLE);
        if ( bottle )
          pPlayers[uActiveCharacter]->pOwnItems[bottle - 1].uAttributes = ITEM_IDENTIFIED;
        if ( !(pItemsTable->pItems[pPlayers[uActiveCharacter]->pInventoryItemList[item_pid].uItemID].uItemID_Rep_St) )
          pPlayers[uActiveCharacter]->pInventoryItemList[item_pid].uAttributes |= 1;
        if ( !dword_4E455C )
        {
          pMouse->RemoveHoldingItem();
          no_rightlick_in_inventory = 1;
          return;
        }
        pPlayers[uActiveCharacter]->PlaySound(SPEECH_DO_POTION_FINE, 0);
        dword_4E455C = 0;
        pMouse->RemoveHoldingItem();
        no_rightlick_in_inventory = 1;
        return;
      }
      GameUI_DrawItemInfo(item);
      return;
    }
  }
  GameUI_DrawItemInfo(item);
  return;
}


//----- (0045828B) --------------------------------------------------------
unsigned int __fastcall GetSpellColor(signed int a1)
{
  if ( a1 == 0 )
    return Color16(0, 0, 0);
  if ( a1 < 12 )
    return Color16(255, 85, 0);
  if ( a1 < 23 )
    return Color16(150, 212, 255);
  if ( a1 < 34 )
    return Color16(0, 128, 255);
  if ( a1 < 45 )
    return Color16(128, 128, 128);
  if ( a1 < 56 )
    return Color16(225, 225, 225);
  if ( a1 < 67 )
    return Color16(235, 15, 255);
  if ( a1 < 78 )
    return Color16(255, 128, 0);
  if ( a1 < 89 )
    return Color16(255, 255, 155);
  if ( a1 < 100 )
    return Color16(192, 192, 240);
  else
    __debugbreak();
}

//----- (004B46F8) --------------------------------------------------------
__int64 GetExperienceRequiredForLevel(int level)
{
	__int64 v1; // eax@1
	int i; // edx@1

	v1 = 0;
	for (i = 0; i < level; ++i)
		v1 += i + 1;
	return 1000 * v1;
}
