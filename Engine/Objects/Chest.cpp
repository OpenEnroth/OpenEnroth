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
void Chest::ChestUI_WritePointedObjectStatusString() {

	Point pt = pMouse->GetCursorPos();
	unsigned int pX=pt.x;
	unsigned int pY=pt.y;

	int chestheight = pChestHeightsByType[pChests[(int)pGUIWindow_CurrentMenu->par1C].uChestBitmapID];
	int chestwidth = pChestWidthsByType[pChests[(int)pGUIWindow_CurrentMenu->par1C].uChestBitmapID];

	int inventoryYCoord = (pY - 34) / 32;	//use pchestoffsets??
	int inventoryXCoord = (pX - 42) / 32;
	int invMatrixIndex = inventoryXCoord + (chestheight * inventoryYCoord);

	if (inventoryYCoord >= 0 && inventoryYCoord < chestheight && inventoryXCoord >= 0 && inventoryXCoord < chestwidth) {

			int chestindex = pChests[(int)pGUIWindow_CurrentMenu->par1C].pInventoryIndices[invMatrixIndex];
			if (chestindex < 0) {
				invMatrixIndex = (-(chestindex + 1));
				chestindex = pChests[(int)pGUIWindow_CurrentMenu->par1C].pInventoryIndices[invMatrixIndex];
			}

			if (chestindex) {
				int itemindex = chestindex - 1;
				ItemGen* item = &pChests[(int)pGUIWindow_CurrentMenu->par1C].igChestItems[itemindex];
				GameUI_StatusBar_Set(item->GetDisplayName());
			}
	}
}



//----- (0041FE71) --------------------------------------------------------
bool Chest::CanPlaceItemAt(signed int test_cell_position, int item_id, signed int uChestID) {
	
    int chest_cell_heght = pChestHeightsByType[pChests[uChestID].uChestBitmapID];
    int chest_cell_width = pChestWidthsByType[pChests[uChestID].uChestBitmapID];

	auto img = assets->GetImage_16BitColorKey(pItemsTable->pItems[item_id].pIconName, 0x7FF);
	unsigned int slot_width = GetSizeInInventorySlots(img->GetWidth());
	unsigned int slot_height = GetSizeInInventorySlots(img->GetHeight());

    if (img) {
        img->Release();
        img = nullptr;
    }

	Assert(slot_height > 0 && slot_width > 0, "Items should have nonzero dimensions");
    if ((slot_width + test_cell_position % chest_cell_width <= chest_cell_width) && (slot_height + test_cell_position / chest_cell_width <= chest_cell_heght)) {

		for (unsigned int x = 0; x < slot_width; x++) {
			for (unsigned int y = 0; y < slot_height; y++) {
				if (pChests[uChestID].pInventoryIndices[y * chest_cell_width + x + test_cell_position] != 0) {
					return false;
				}
			}
		}
		return true;
    }
    return false;
}

//----- (0041FF64) --------------------------------------------------------
int Chest::CountChestItems(signed int uChestID) { // this returns first free slot rather than an actual count

    signed int item_count=0;
    int max_items = pChestWidthsByType[pChests[uChestID].uChestBitmapID] * pChestHeightsByType[pChests[uChestID].uChestBitmapID];

    if (max_items <= 0)
        item_count = -1;
    else {

        while (pChests[uChestID].igChestItems[item_count].uItemID) {

            ++item_count;
            if (item_count >= max_items) {
                item_count = -1;
                break;
            }
        }
    }
    return item_count;
}

//----- (0041FFA2) --------------------------------------------------------
int Chest::PutItemInChest(int position, ItemGen *put_item, signed int uChestID) {

	signed int item_in_chest_count = CountChestItems(uChestID);
	signed int test_pos = 0;

	int max_size = pChestWidthsByType[pChests[uChestID].uChestBitmapID] * pChestHeightsByType[pChests[uChestID].uChestBitmapID];
	int chest_width = pChestWidthsByType[pChests[uChestID].uChestBitmapID];

	if (item_in_chest_count == -1)
		return 0;

	if (position != -1) {
		if (CanPlaceItemAt(position, put_item->uItemID, uChestID)) {
			test_pos = position;
		}
		else
			position = -1; // try another position?? is this the right behavior
	}

	if (position == -1) { // no position specified

		for (int _i = 0; _i < max_size; _i++) {
			if (Chest::CanPlaceItemAt(_i, put_item->uItemID, pChestWindow->par1C)) {
				test_pos = _i; // found somewhere to place item
				break;
			}
		}

		if (test_pos == max_size) { // limits check no room
			if (uActiveCharacter)
				pPlayers[uActiveCharacter]->PlaySound(SPEECH_NoRoom, 0);
			return 0;
		}

	}

	auto texture = assets->GetImage_16BitColorKey(put_item->GetIconName(), 0x7FF);
	unsigned int slot_width = GetSizeInInventorySlots(texture->GetWidth());
	unsigned int slot_height = GetSizeInInventorySlots(texture->GetHeight());

	if (texture) {
		texture->Release();
		texture = nullptr;
	}

	Assert(slot_height > 0 && slot_width > 0, "Items should have nonzero dimensions");

		// set inventory indices - memset was eratic??
		for (unsigned int x = 0; x < slot_width; x++) {
			for (unsigned int y = 0; y < slot_height; y++) {
				pChests[uChestID].pInventoryIndices[y * chest_width + x + test_pos] = (-1 - test_pos);
			}
		}

        pChests[uChestID].pInventoryIndices[test_pos] = item_in_chest_count + 1;
        memcpy(&pChests[uChestID].igChestItems[item_in_chest_count], put_item, sizeof(ItemGen));
       
    return (test_pos + 1);
}

//----- (0042013E) --------------------------------------------------------
void Chest::PlaceItemAt(unsigned int put_cell_pos, unsigned int item_at_cell, signed int uChestID) //only used for setup?
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
void Chest::PlaceItems(signed int uChestID) // only sued for setup
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
void RemoveItemAtChestIndex(int index) {

	int chestindex = pChests[(int)pGUIWindow_CurrentMenu->par1C].pInventoryIndices[index];
	ItemGen *item_in_slot = &pChests[(int)pChestWindow->ptr_1C].igChestItems[chestindex - 1];

	auto img = assets->GetImage_16BitColorKey(item_in_slot->GetIconName(), 0x7FF);
	unsigned int slot_width = GetSizeInInventorySlots(img->GetWidth());
	unsigned int slot_height = GetSizeInInventorySlots(img->GetHeight());

	int chestwidth = pChestWidthsByType[pChests[(int)pChestWindow->ptr_1C].uChestBitmapID];

	item_in_slot->Reset();

	if (slot_width > 0) {
		// blank inventory indices - memset was eratic??
		for (unsigned int x = 0; x < slot_width; x++) {
			for (unsigned int y = 0; y < slot_height; y++) {
				pChests[(int)pGUIWindow_CurrentMenu->par1C].pInventoryIndices[y * chestwidth + x + index] = 0;				
			}
		}
	}
}

//----- (00420E01) --------------------------------------------------------
void Chest::OnChestLeftClick() {

	unsigned int pX;
	unsigned int pY;

	int chestheight = pChestHeightsByType[pChests[(int)pGUIWindow_CurrentMenu->par1C].uChestBitmapID];
	int chestwidth = pChestWidthsByType[pChests[(int)pGUIWindow_CurrentMenu->par1C].uChestBitmapID];

	pMouse->GetClickPos(&pX, &pY);
	int inventoryYCoord = (pY - 34) / 32;	//use pchestoffsets??
	int inventoryXCoord = (pX - 42) / 32;
	int invMatrixIndex = inventoryXCoord + (chestheight * inventoryYCoord);

	if (inventoryYCoord >= 0 && inventoryYCoord < chestheight && inventoryXCoord >= 0 && inventoryXCoord < chestwidth) {

		if (pParty->pPickedItem.uItemID) { // item held

			if (Chest::PutItemInChest(invMatrixIndex, &pParty->pPickedItem, pGUIWindow_CurrentMenu->par1C))
				pMouse->RemoveHoldingItem();

		}
		else {

			int chestindex = pChests[(int)pGUIWindow_CurrentMenu->par1C].pInventoryIndices[invMatrixIndex];
			if (chestindex < 0) {
				invMatrixIndex = (-(chestindex + 1));
				chestindex = pChests[(int)pGUIWindow_CurrentMenu->par1C].pInventoryIndices[invMatrixIndex];
			}

			if (chestindex) {

				int itemindex = chestindex - 1;

				if (pChests[(int)pGUIWindow_CurrentMenu->par1C].igChestItems[itemindex].GetItemEquipType() == EQUIP_GOLD) {
					pParty->PartyFindsGold(pChests[(int)pGUIWindow_CurrentMenu->par1C].igChestItems[itemindex].special_enchantment, 0);
					viewparams->bRedrawGameUI = 1;
				}
				else {
					pParty->SetHoldingItem(&pChests[(int)pGUIWindow_CurrentMenu->par1C].igChestItems[itemindex]);
				}

				RemoveItemAtChestIndex(invMatrixIndex);
			}
		}
	}
}