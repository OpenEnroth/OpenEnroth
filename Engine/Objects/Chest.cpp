#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <crtdbg.h>
#include <stdio.h>

#include "Engine/Engine.h"
#include "Engine/Time.h"

#include "IO/Mouse.h"

#include "GUI/UI/Chest.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

#include "Items.h"
#include "Chest.h"
#include "../Tables/FrameTableInc.h"
#include "../LOD.h"
#include "../MapInfo.h"
#include "Actor.h"
#include "../Graphics/Outdoor.h"
#include "../Graphics/DecorationList.h"
#include "../Party.h"
#include "../OurMath.h"
#include "ObjectList.h"
#include "SpriteObject.h"
#include "../Graphics/Viewport.h"
#include "../Graphics/Level/Decoration.h"



size_t uNumChests; // idb
struct ChestList *pChestList;
std::array<Chest, 20> pChests;

extern int pChestWidthsByType[8];
extern int pChestHeightsByType[8];



//----- (0042041E) --------------------------------------------------------
bool Chest::Open(signed int uChestID)
{
    unsigned int pMapID; // eax@8
    int pRandom; // edx@16
    int v6; // eax@16
    ODMFace *pODMFace; // eax@19
    BLVFace *pBLVFace; // eax@20
    int pObjectX; // ebx@21
    int pObjectZ; // edi@21
    double dir_x; // st7@23
    double dir_y; // st6@23
    double length_vector; // st7@23
    int pDepth; // ecx@26
    Vec3_int_ v; // ST4C_12@28
    bool flag_shout; // edi@28
    SPRITE_OBJECT_TYPE pSpriteID[4]; // [sp+84h] [bp-40h]@16
    Vec3_int_ pOut; // [sp+A0h] [bp-24h]@28
    int pObjectY; // [sp+B0h] [bp-14h]@21
    int sRotX; // [sp+B4h] [bp-10h]@23
    float dir_z; // [sp+BCh] [bp-8h]@23
    int sRotY; // [sp+C0h] [bp-4h]@8
    SpriteObject pSpellObject; // [sp+14h] [bp-B0h]@28

    assert(uChestID < 20);
    if ((uChestID < 0) && (uChestID >= 20))
        return false;
    Chest* chest = &pChests[uChestID];

    if (!chest->Initialized())
        Chest::PlaceItems(uChestID);

    if (!uActiveCharacter)
        return false;
    flag_shout = false;
    pMapID = pMapStats->GetMapInfo(pCurrentMapName);
    if (chest->Trapped() && pMapID)
    {
        if (pPlayers[uActiveCharacter]->GetDisarmTrap() < 2 * pMapStats->pInfos[pMapID].LockX5)
        {
            pSpriteID[0] = SPRITE_811;
            pSpriteID[1] = SPRITE_812;
            pSpriteID[2] = SPRITE_813;
            pSpriteID[3] = SPRITE_814;
            pRandom = rand() % 4;
            v6 = PID_ID(EvtTargetObj);
            if (PID_TYPE(EvtTargetObj) == OBJECT_Decoration)
            {
                pObjectX = pLevelDecorations[v6].vPosition.x;
                pObjectY = pLevelDecorations[v6].vPosition.y;
                pObjectZ = pLevelDecorations[v6].vPosition.z + (pDecorationList->pDecorations[pLevelDecorations[v6].uDecorationDescID].uDecorationHeight / 2);
            }
            if (PID_TYPE(EvtTargetObj) == OBJECT_BModel)
            {
                if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
                {
                    pODMFace = &pOutdoor->pBModels[EvtTargetObj >> 9].pFaces[(EvtTargetObj >> 3) & 0x3F];
                    pObjectX = (pODMFace->pBoundingBox.x1 + pODMFace->pBoundingBox.x2) / 2;
                    pObjectY = (pODMFace->pBoundingBox.y1 + pODMFace->pBoundingBox.y2) / 2;
                    pObjectZ = (pODMFace->pBoundingBox.z1 + pODMFace->pBoundingBox.z2) / 2;
                }
                else//Indoor
                {
                    pBLVFace = &pIndoor->pFaces[v6];
                    pObjectX = (pBLVFace->pBounding.x1 + pBLVFace->pBounding.x2) / 2;
                    pObjectY = (pBLVFace->pBounding.y1 + pBLVFace->pBounding.y2) / 2;
                    pObjectZ = (pBLVFace->pBounding.z1 + pBLVFace->pBounding.z2) / 2;
                }
            }
            dir_x = (double)pParty->vPosition.x - (double)pObjectX;
            dir_y = (double)pParty->vPosition.y - (double)pObjectY;
            dir_z = ((double)pParty->sEyelevel + (double)pParty->vPosition.z) - (double)pObjectZ;
            length_vector = sqrt((dir_x * dir_x) + (dir_y * dir_y) + (dir_z * dir_z));
            if (length_vector <= 1.0)
            {
                *(float *)&sRotX = 0.0;
                *(float *)&sRotY = 0.0;
            }
            else
            {
                sRotY = (signed __int64)sqrt(dir_x * dir_x + dir_y * dir_y);
                sRotX = stru_5C6E00->Atan2((signed __int64)dir_x, (signed __int64)dir_y);
                sRotY = stru_5C6E00->Atan2(dir_y * dir_y, (signed __int64)dir_z);
            }
            pDepth = 256;
            if (length_vector < 256.0)
                pDepth = (signed __int64)length_vector / 4;
            v.x = pObjectX;
            v.y = pObjectY;
            v.z = pObjectZ;
            Vec3_int_::Rotate(pDepth, sRotX, sRotY, v, &pOut.x, &pOut.z, &pOut.y);
            SpriteObject::sub_42F7EB_DropItemAt(pSpriteID[pRandom], pOut.x, pOut.z, pOut.y, 0, 1, 0, 48, 0);

            pSpellObject.containing_item.Reset();
            pSpellObject.spell_skill = 0;
            pSpellObject.spell_level = 0;
            pSpellObject.spell_id = 0;
            pSpellObject.field_54 = 0;
            pSpellObject.uType = pSpriteID[pRandom];
            pSpellObject.uObjectDescID = 0;
            if (pObjectList->uNumObjects)
            {
                for (uint i = 0; i < (signed int)pObjectList->uNumObjects; ++i)
                {
                    if (pSpriteID[pRandom] == pObjectList->pObjects[i].uObjectID)
                        pSpellObject.uObjectDescID = i;
                }
            }
            pSpellObject.vPosition.y = pOut.z;
            pSpellObject.vPosition.x = pOut.x;
            pSpellObject.vPosition.z = pOut.y;
            pSpellObject.uSoundID = 0;
            pSpellObject.uAttributes = 48;
            pSpellObject.uSectorID = pIndoor->GetSector(pOut.x, pOut.z, pOut.y);
            pSpellObject.uSpriteFrameID = 0;
            pSpellObject.spell_caster_pid = 0;
            pSpellObject.spell_target_pid = 0;
            pSpellObject.uFacing = 0;
            pSpellObject.Create(0, 0, 0, 0);
            pAudioPlayer->PlaySound(SOUND_fireBall, 0, 0, -1, 0, 0, 0, 0);
            pSpellObject.ExplosionTraps();
            chest->uFlags &= 0xFEu;
            if (uActiveCharacter && !_A750D8_player_speech_timer && !OpenedTelekinesis)
            {
                _A750D8_player_speech_timer = 256i64;
                PlayerSpeechID = SPEECH_5;
                uSpeakingCharacter = uActiveCharacter;
            }
            OpenedTelekinesis = false;
            return false;
        }
        chest->uFlags &= 0xFEu;
        flag_shout = true;
    }
    pAudioPlayer->StopChannels(-1, -1);
    pAudioPlayer->PlaySound(SOUND_openchest0101, 0, 0, -1, 0, 0, 0, 0);
    if (flag_shout == true)
    {
        if (!OpenedTelekinesis)
            pPlayers[uActiveCharacter]->PlaySound(SPEECH_4, 0);
    }
    OpenedTelekinesis = false;
    pChestWindow = pGUIWindow_CurrentMenu = new GUIWindow_Chest(uChestID);
    return true;
}

//----- (0042038D) --------------------------------------------------------
void Chest::ChestUI_WritePointedObjectStatusString()
{
    int v1; // ecx@2
    POINT cursor; // [sp+8h] [bp-8h]@1

    pMouse->GetCursorPos(&cursor);
    if (cursor.y < 350)
    {
        v1 = render->pActiveZBuffer[cursor.x + pSRZBufferLineOffsets[cursor.y]];
        if (v1 != 0 && v1 != -65536)
        {
            if (v1)
            {
                ItemGen* item = &pChests[pChestWindow->par1C].igChestItems[pChests[pChestWindow->par1C].pInventoryIndices[(v1 & 0xFFFF) - 1] - 1];
                GameUI_StatusBar_Set(item->GetDisplayName());
            }
        }
    }
}



//----- (0041FE71) --------------------------------------------------------
bool Chest::CanPlaceItemAt(signed int test_cell_position, int item_id, signed int uChestID)
{
    //    int v3; // eax@1
        //unsigned int item_texture_id; // eax@1
        //Image *item_texture; // ecx@1
    signed int v6; // eax@1
//    signed int v7; // edi@3
    signed int v8; // eax@3
    int texture_cell_width; // edi@3
    int texture_cell_height; // ebx@5
    int _row; // esi@9
    int _cell_rows; // edx@10
    int _column; // ecx@11
//    char *v14; // eax@12
    int chest_cell_heght; // [sp+Ch] [bp-Ch]@1
//    signed int v17; // [sp+10h] [bp-8h]@1
    signed int chest_cell_width; // [sp+14h] [bp-4h]@1

    chest_cell_heght = pChestHeightsByType[pChests[uChestID].uChestBitmapID];
    chest_cell_width = pChestWidthsByType[pChests[uChestID].uChestBitmapID];

    auto item_texture = assets->GetImage_16BitColorKey(pItemsTable->pItems[item_id].pIconName, 0x7FF);
    v6 = item_texture->GetWidth();
    if (v6 < 14)
        v6 = 14;
    texture_cell_width = ((v6 - 14) >> 5) + 1;
    v8 = item_texture->GetHeight();
    if (v8 < 14)
        v8 = 14;
    texture_cell_height = ((v8 - 14) >> 5) + 1;

    if (item_texture)
    {
        item_texture->Release();
        item_texture = nullptr;
    }

    if ((texture_cell_width + test_cell_position % chest_cell_width <= chest_cell_width) &&
        (texture_cell_height + test_cell_position / chest_cell_width <= chest_cell_heght))
    { //we not put over borders
        _row = 0;
        if (texture_cell_height <= 0)
            return true;
        _cell_rows = 0;
        while (1)
        {
            _column = 0;
            if (texture_cell_width > 0)
            {
                while (pChests[uChestID].pInventoryIndices[test_cell_position + _cell_rows + _column] == 0)
                {
                    ++_column;
                    if (_column >= texture_cell_width)
                        break;
                }
                if (pChests[uChestID].pInventoryIndices[test_cell_position + _cell_rows + _column] != 0)
                    return false;
            }
            _cell_rows += chest_cell_width;
            ++_row;
            if (_row >= texture_cell_height)
                return true;
        }

    }
    return false;
}

//----- (0041FF64) --------------------------------------------------------
int Chest::CountChestItems(signed int uChestID)
{
    signed int item_count; // eax@1
    int max_items; // edx@1
    item_count = 0;
    max_items = pChestWidthsByType[pChests[uChestID].uChestBitmapID] * pChestHeightsByType[pChests[uChestID].uChestBitmapID];
    if (max_items <= 0)
        item_count = -1;
    else
    {
        while (pChests[uChestID].igChestItems[item_count].uItemID)
        {
            ++item_count;
            if (item_count >= max_items)
            {
                item_count = -1;
                break;
            }
        }
    }
    return item_count;
}

//----- (0041FFA2) --------------------------------------------------------
int Chest::PutItemInChest(int position, ItemGen *put_item, signed int uChestID)
{//(rus: положить предмет в ящик)
    ItemGen *v4; // edi@1
    int v5; // esi@1
    int result; // eax@11
    //unsigned int v7; // eax@12
    int v8; // edx@12
    //Texture_MM7 *texture; // ecx@12
    signed int v10; // eax@12
    signed int v11; // edi@14
    unsigned int v12; // esi@14
    int v13; // edi@16
    void *v14; // edi@21
    int v15; // edi@21
    int i; // ecx@21
    ItemGen *Src; // [sp+Ch] [bp-18h]@1
    signed int item_in_chest_count; // [sp+10h] [bp-14h]@2
    int v19; // [sp+14h] [bp-10h]@1
    int v20; // [sp+18h] [bp-Ch]@19
    signed int v21; // [sp+1Ch] [bp-8h]@1
  //  signed int v22; // [sp+20h] [bp-4h]@3
    int v23; // [sp+20h] [bp-4h]@19

    v21 = 0;
    v4 = put_item;
    v5 = pChestWidthsByType[pChests[uChestID].uChestBitmapID] * pChestHeightsByType[pChests[uChestID].uChestBitmapID];
    Src = put_item;
    v19 = pChestWidthsByType[pChests[uChestID].uChestBitmapID];
    if (position == -1)
    {
        item_in_chest_count = CountChestItems(uChestID);
        if (item_in_chest_count == -1)
            return 0;
        for (int _i = 0; _i < v5; _i++)
        {
            if (Chest::CanPlaceItemAt(_i, v4->uItemID, pChestWindow->par1C))
                v21 = _i;
        }
        if (v21 == v5)
        {
            if (uActiveCharacter)
                pPlayers[uActiveCharacter]->PlaySound(SPEECH_NoRoom, 0);
            return 0;
        }
        HIWORD(v8) = 0;

        auto texture = assets->GetImage_16BitColorKey(v4->GetIconName(), 0x7FF);

        v10 = texture->GetWidth();
        if (v10 < 14)
            v10 = 14;
        v12 = ((v10 - 14) >> 5) + 1;
        v11 = texture->GetHeight();
        if (v11 < 14)
            v11 = 14;
        v13 = ((v11 - 14) >> 5) + 1;

        if (v13 > 0)
        {
            v23 = 0;
            v20 = v13;
            do
            {
                if ((signed int)v12 > 0)
                {
                    v14 = &pChests[uChestID].pInventoryIndices[v21 + v23];
                    LOWORD(v8) = -1 - v21;
                    v8 <<= 16;
                    LOWORD(v8) = -1 - v21;
                    memset32(v14, v8, v12 >> 1);
                    v15 = (int)((char *)v14 + 4 * (v12 >> 1));
                    for (i = v12 & 1; i; --i)
                    {
                        *(short *)v15 = v8;
                        v15 += 2;
                    }
                }
                v23 += v19;
                --v20;
            } while (v20);
        }
        pChests[uChestID].pInventoryIndices[v21] = item_in_chest_count + 1;
        memcpy(&pChests[uChestID].igChestItems[item_in_chest_count], put_item, sizeof(ItemGen));
        result = v21 + 1;
    }
    else
        result = 1;
    return result;
}

//----- (0042013E) --------------------------------------------------------
void Chest::PlaceItemAt(unsigned int put_cell_pos, unsigned int item_at_cell, signed int uChestID)
{
    int uItemID; // edi@1
    int v6; // edx@4
    signed int v9; // eax@5
    signed int v10; // edi@7
    unsigned int texture_cell_width; // ebx@7
    int textute_cell_height; // edi@9
    int chest_cell_row_pos; // edx@12
    int chest_cell_width; // [sp+10h] [bp-Ch]@11

    uItemID = pChests[uChestID].igChestItems[item_at_cell].uItemID;
    pItemsTable->SetSpecialBonus(&pChests[uChestID].igChestItems[item_at_cell]);
    if (uItemID >= 135 && uItemID <= 159 && !pChests[uChestID].igChestItems[item_at_cell].uNumCharges)
    {
        v6 = rand() % 21 + 10;
        pChests[uChestID].igChestItems[item_at_cell].uNumCharges = v6;
        pChests[uChestID].igChestItems[item_at_cell].uMaxCharges = v6;
    }

    auto img = assets->GetImage_16BitAlpha(pItemsTable->pItems[uItemID].pIconName);

    v9 = img->GetWidth();
    if (v9 < 14)
        v9 = 14;
    texture_cell_width = ((v9 - 14) >> 5) + 1;
    v10 = img->GetHeight();
    if (v10 < 14)
        v10 = 14;
    textute_cell_height = ((v10 - 14) >> 5) + 1;

    chest_cell_width = pChestWidthsByType[pChests[uChestID].uChestBitmapID];
    chest_cell_row_pos = 0;
    for (int i = 0; i < textute_cell_height; ++i)
    {
        for (int j = 0; j < texture_cell_width; ++j)
            pChests[uChestID].pInventoryIndices[put_cell_pos + chest_cell_row_pos + j] = (signed __int16)-(put_cell_pos + 1);
        chest_cell_row_pos += chest_cell_width;
    }
    pChests[uChestID].pInventoryIndices[put_cell_pos] = item_at_cell + 1;
}

//----- (00420284) --------------------------------------------------------
void Chest::PlaceItems(signed int uChestID)
{
    int uChestArea; // edi@1
    int random_chest_pos; // eax@2
    int test_position; // ebx@11
    char chest_cells_map[144]; // [sp+Ch] [bp-A0h]@1
    int chest_item_id; // [sp+9Ch] [bp-10h]@10
    unsigned int items_counter; // [sp+A4h] [bp-8h]@8

    render->ClearZBuffer(0, 479);
    uChestArea = pChestWidthsByType[pChests[uChestID].uChestBitmapID] * pChestHeightsByType[pChests[uChestID].uChestBitmapID];
    memset(chest_cells_map, 0, 144);
    //fill cell map at random positions
    for (items_counter = 0; items_counter < uChestArea; ++items_counter)
    {
        //get random position in chest
        do
            random_chest_pos = (unsigned __int8)rand();
        while (random_chest_pos >= uChestArea);
        //if this pos occupied move to next
        while (chest_cells_map[random_chest_pos])
        {
            ++random_chest_pos;
            if (random_chest_pos == uChestArea)
                random_chest_pos = 0;
        }
        chest_cells_map[random_chest_pos] = items_counter;
    }
    items_counter = 0;

    for (items_counter = 0; items_counter < uChestArea; ++items_counter)
    {
        chest_item_id = pChests[uChestID].igChestItems[items_counter].uItemID;
        if (chest_item_id)
        {
            test_position = 0;
            while (!Chest::CanPlaceItemAt((unsigned __int8)chest_cells_map[test_position], chest_item_id, uChestID))
            {
                ++test_position;
                if (test_position >= uChestArea)
                    break;
            }
            if (test_position < uChestArea)
            {
                Chest::PlaceItemAt((unsigned __int8)chest_cells_map[test_position], items_counter, uChestID);
                if (pChests[uChestID].uFlags & CHEST_OPENED)
                    pChests[uChestID].igChestItems[items_counter].SetIdentified();
            }
        }
    }
    pChests[uChestID].SetInitialized(true);
}

//----- (00448A17) --------------------------------------------------------
void Chest::ToggleFlag(signed int uChestID, unsigned __int16 uFlag, unsigned int bToggle)
{
    if (uChestID >= 0 && uChestID <= 19)
    {
        if (bToggle)
            pChests[uChestID].uFlags |= uFlag;
        else
            pChests[uChestID].uFlags &= ~uFlag;
    }
}

//----- (00458B03) --------------------------------------------------------
void ChestList::ToFile()
{
    FILE *v2; // eax@1
    FILE *v3; // edi@1

    v2 = fopen("data\\dchest.bin", "wb");
    v3 = v2;
    if (!v2)
        Error("Unable to save dchest.bin!");

    fwrite(this, 4, 1, v2);
    fwrite(this->pChests, 36, this->uNumChests, v3);
    fclose(v3);
}


//----- (00458B4F) --------------------------------------------------------
void ChestList::FromFile(void *data_mm6, void *data_mm7, void *data_mm8)
{
    uint num_mm6_chests = data_mm6 ? *(int *)data_mm6 : 0,
         num_mm7_chests = data_mm7 ? *(int *)data_mm7 : 0,
         num_mm8_chests = data_mm8 ? *(int *)data_mm8 : 0;

    uNumChests = num_mm6_chests + num_mm7_chests + num_mm8_chests;
    assert(uNumChests);
    assert(!num_mm8_chests);

    pChests = (ChestDesc *)malloc(uNumChests * sizeof(ChestDesc));
    memcpy(pChests,                                   (char *)data_mm7 + 4, num_mm7_chests * sizeof(ChestDesc));
    memcpy(pChests + num_mm7_chests,                  (char *)data_mm6 + 4, num_mm6_chests * sizeof(ChestDesc));
    memcpy(pChests + num_mm6_chests + num_mm7_chests, (char *)data_mm8 + 4, num_mm8_chests * sizeof(ChestDesc));
}


//----- (00458B9C) --------------------------------------------------------
int ChestList::FromFileTxt(const char *Args)
{
    //ChestList *v2; // ebx@1
    __int32 v3; // edi@1
    FILE *v4; // eax@1
    unsigned int v5; // esi@3
    const void *v6; // ST18_4@9
    void *v7; // eax@9
    FILE *v8; // ST0C_4@11
    char *i; // eax@11
    char v10; // al@14
    const char *v11; // ST14_4@14
    char v12; // al@14
    const char *v13; // ST10_4@14
    char Buf; // [sp+8h] [bp-2F0h]@3
    FrameTableTxtLine v16; // [sp+1FCh] [bp-FCh]@4
    FrameTableTxtLine v17; // [sp+278h] [bp-80h]@4
    FILE *File; // [sp+2F4h] [bp-4h]@1
    unsigned int Argsa; // [sp+300h] [bp+8h]@3

    //v2 = this;
    free(this->pChests);
    v3 = 0;
    this->pChests = 0;
    this->uNumChests = 0;
    v4 = fopen(Args, "r");
    File = v4;
    if (!v4)
        Error("ChestDescriptionList::load - Unable to open file: %s.");

    v5 = 0;
    Argsa = 0;
    if (fgets(&Buf, 490, v4))
    {
        do
        {
            *strchr(&Buf, 10) = 0;
            memcpy(&v17, txt_file_frametable_parser(&Buf, &v16), sizeof(v17));
            if (v17.uPropCount && *v17.pProperties[0] != 47)
                ++Argsa;
        } while (fgets(&Buf, 490, File));
        v5 = Argsa;
        v3 = 0;
    }
    v6 = this->pChests;
    this->uNumChests = v5;
    v7 = malloc(36 * v5);
    this->pChests = (ChestDesc *)v7;
    if (v7 == (void *)v3)
        Error("ChestDescriptionList::load - Out of Memory!");

    memset(v7, v3, 36 * this->uNumChests);
    v8 = File;
    this->uNumChests = v3;
    fseek(v8, v3, v3);
    for (i = fgets(&Buf, 490, File); i; i = fgets(&Buf, 490, File))
    {
        *strchr(&Buf, 10) = 0;
        memcpy(&v17, txt_file_frametable_parser(&Buf, &v16), sizeof(v17));
        if (v17.uPropCount && *v17.pProperties[0] != 47)
        {
            strcpy(this->pChests[this->uNumChests].pName, v17.pProperties[0]);
            v10 = atoi(v17.pProperties[1]);
            v11 = v17.pProperties[2];
            this->pChests[this->uNumChests].uWidth = v10;
            v12 = atoi(v11);
            v13 = v17.pProperties[3];
            this->pChests[this->uNumChests].uHeight = v12;
            this->pChests[this->uNumChests++].uTextureID = atoi(v13);
        }
    }
    fclose(File);
    return 1;
}

//----- (00420B13) --------------------------------------------------------
void sub_420B13(int a1, int a2)
{	//Give item from chest(rus: Взять предмет из ящика)
    signed int v6; // eax@1
    signed int v7; // edi@3
    signed int v8; // eax@3
    int v9; // edi@3
    int v10; // eax@5
    int v11; // esi@8
    unsigned int v12; // ecx@10
    void *v13; // edi@10
    unsigned __int8 v14; // cf@10
    int v15; // edi@10
    int i; // ecx@10
    int v17; // [sp+Ch] [bp-14h]@1
    int v18; // [sp+10h] [bp-10h]@3
    int v21; // [sp+1Ch] [bp-4h]@5
    int v22; // [sp+1Ch] [bp-4h]@8

    v17 = pChestWidthsByType[pChests[(int)pChestWindow->ptr_1C].uChestBitmapID];

    auto img = assets->GetImage_16BitColorKey(pChests[(int)pChestWindow->ptr_1C].igChestItems[a1].GetIconName(), 0x7FF);

    v6 = img->GetWidth();
    if (v6 < 14)
        v6 = 14;
    v7 = v6 - 14;
    v9 = (v7 >> 5) + 1;
    v18 = v9;

    v8 = img->GetHeight();
    if (v8 < 14)
        v8 = 14;
    v10 = ((v8 - 14) >> 5) + 1;
    v21 = v10;

    if (v10 > 0)
    {
        v11 = 0;
        for (v22 = v10; v22; --v22)
        {
            if (v9 > 0)
            {
                v12 = v9;
                v13 = &pChests[(int)pChestWindow->ptr_1C].pInventoryIndices[a2 + v11];
                v14 = v12 & 1;
                v12 >>= 1;
                memset(&pChests[(int)pChestWindow->ptr_1C].pInventoryIndices[a2 + v11], 0, 4 * v12);
                v15 = (int)((char *)v13 + 4 * v12);
                for (i = v14; i; --i)
                {
                    *(short *)v15 = 0;
                    v15 += 2;
                }
                v9 = v18;
            }
            v11 += v17;
        }
    }
    pChests[(int)pChestWindow->ptr_1C].igChestItems[a1].Reset();
}

//----- (00420E01) --------------------------------------------------------
void Chest::OnChestLeftClick()
{
    int v2; // eax@2
    int v3; // ebx@4
    int v4; // esi@6
    POINT cursor; // [sp+84h] [bp-8h]@2

    SpriteObject v6; // [sp+Ch] [bp-80h]@1
    if (pParty->pPickedItem.uItemID)
    {
        if (Chest::PutItemInChest(-1, &pParty->pPickedItem, pGUIWindow_CurrentMenu->par1C))
            pMouse->RemoveHoldingItem();
    }
    else
    {
        pMouse->GetCursorPos(&cursor);
        v2 = render->pActiveZBuffer[cursor.x + pSRZBufferLineOffsets[cursor.y]] & 0xFFFF;
        if (v2)
        {
            if (v2)
                v3 = v2 - 1;
            else
                v3 = -1;
            v4 = pChests[(int)pGUIWindow_CurrentMenu->par1C].pInventoryIndices[v3] - 1;
            if (pChests[(int)pGUIWindow_CurrentMenu->par1C].igChestItems[v4].GetItemEquipType() == EQUIP_GOLD)
            {
                pParty->PartyFindsGold(pChests[(int)pGUIWindow_CurrentMenu->par1C].igChestItems[v4].special_enchantment, 0);
                viewparams->bRedrawGameUI = 1;
            }
            else
                pParty->SetHoldingItem(&pChests[(int)pGUIWindow_CurrentMenu->par1C].igChestItems[v4]);
            sub_420B13(v4, v3);
        }
    }
}