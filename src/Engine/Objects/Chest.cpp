#include "Engine/Objects/Chest.h"

#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Items.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/MapInfo.h"

#include "GUI/UI/UIChest.h"
#include "GUI/UI/UIStatusBar.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Random/Random.h"
#include "Library/Logger/Logger.h"

#include "Utility/Math/TrigLut.h"

ChestList *pChestList;
std::vector<Chest> vChests;

bool Chest::open(int uChestID, Pid objectPid) {
    ODMFace *pODMFace;
    BLVFace *pBLVFace;
    Vec3i objectPos;
    double dir_x;
    double dir_y;
    double length_vector;
    int pDepth;
    bool flag_shout;
    SPRITE_OBJECT_TYPE pSpriteID[4];
    Vec3i pOut;
    int yawAngle{};
    int pitchAngle{};
    SpriteObject pSpellObject;

    assert(uChestID < 20);
    if ((uChestID < 0) && (uChestID >= 20))
        return false;
    Chest *chest = &vChests[uChestID];

    if (!chest->Initialized() || engine->config->gameplay.ChestTryPlaceItems.value() == 1)
        Chest::PlaceItems(uChestID);

    if (!pParty->hasActiveCharacter())
        return false;
    flag_shout = false;
    MAP_TYPE pMapID = pMapStats->GetMapInfo(pCurrentMapName);
    if (chest->Trapped() && pMapID != MAP_INVALID) {
        if (pParty->activeCharacter().GetDisarmTrap() < 2 * pMapStats->pInfos[pMapID].LockX5) {
            pSpriteID[0] = SPRITE_TRAP_FIRE;
            pSpriteID[1] = SPRITE_TRAP_LIGHTNING;
            pSpriteID[2] = SPRITE_TRAP_COLD;
            pSpriteID[3] = SPRITE_TRAP_BODY;
            int pRandom = grng->random(4); // Not sure if this should be grng or vrng, so we'd rather err on the side of safety.
            int objId = PID_ID(objectPid);
            if (PID_TYPE(objectPid) == OBJECT_Decoration) {
                objectPos = pLevelDecorations[objId].vPosition +
                    Vec3i(0, 0, pDecorationList->GetDecoration(pLevelDecorations[objId].uDecorationDescID)->uDecorationHeight / 2);
            }
            if (PID_TYPE(objectPid) == OBJECT_Face) {
                // TODO(pskelton): trap explosion moves depending on what face is clicked
                if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
                    pODMFace = &pOutdoor->pBModels[objectPid >> 9].pFaces[(objectPid >> 3) & 0x3F];
                    objectPos = pODMFace->pBoundingBox.center();
                } else {  // Indoor
                    pBLVFace = &pIndoor->pFaces[objId];
                    objectPos = pBLVFace->pBounding.center();
                }
            }

            dir_x = (double)pParty->pos.x - (double)objectPos.x;
            dir_y = (double)pParty->pos.y - (double)objectPos.y;
            length_vector = std::sqrt(dir_x * dir_x + dir_y * dir_y);
            if (length_vector <= 1.0) {
                yawAngle = 0;
                pitchAngle = 0;
            } else {
                // sprite should be rotated towards the party and moved slightly forward
                yawAngle = TrigLUT.atan2((int64_t) dir_y, (int64_t) dir_x);
                pitchAngle = 256;
            }

            pDepth = 96;
            if (length_vector < pDepth) {
                pDepth = length_vector;
            }
            pOut = objectPos + Vec3i::fromPolar(pDepth, yawAngle, pitchAngle);

            pSpellObject.containing_item.Reset();
            pSpellObject.spell_skill = CHARACTER_SKILL_MASTERY_NONE;
            pSpellObject.spell_level = 0;
            pSpellObject.uSpellID = SPELL_NONE;
            pSpellObject.field_54 = 0;
            pSpellObject.uType = pSpriteID[pRandom];
            pSpellObject.uObjectDescID = pObjectList->ObjectIDByItemID(pSpellObject.uType);

            // adjust height to account for different sprite sizes and offset
            SpriteFrame *frame = pSpellObject.getSpriteFrame();
            if (frame->uFlags & 0x20) {
                // centering
                pOut += Vec3i(0, 0, frame->hw_sprites[0]->texture->height() / 4);
            } else {
                pOut -= Vec3i(0, 0, (frame->hw_sprites[0]->texture->height() - 64) / 2);
            }
            pSpellObject.vPosition = pOut;

            pSpellObject.uSoundID = 0;
            pSpellObject.uAttributes = SPRITE_IGNORE_RANGE | SPRITE_NO_Z_BUFFER;
            pSpellObject.uSectorID = pIndoor->GetSector(pOut);
            pSpellObject.uSpriteFrameID = 0;
            pSpellObject.spell_caster_pid = Pid();
            pSpellObject.spell_target_pid = 0;
            pSpellObject.uFacing = 0;
            pSpellObject.Create(0, 0, 0, 0);
            // TODO(Nik-RE-dev): chest is originator in this case
            pAudioPlayer->playSound(SOUND_fireBall, 0);
            pSpellObject.explosionTraps();
            chest->uFlags &= ~CHEST_TRAPPED;
            if (pParty->hasActiveCharacter() && !OpenedTelekinesis) {
                pParty->setDelayedReaction(SPEECH_TRAP_EXPLODED, pParty->activeCharacterIndex() - 1);
            }
            OpenedTelekinesis = false;
            return false;
        }
        chest->uFlags &= ~CHEST_TRAPPED;
        flag_shout = true;
    }
    pAudioPlayer->playUISound(SOUND_openchest0101);
    if (flag_shout == true) {
        if (!OpenedTelekinesis) {
            pParty->activeCharacter().playReaction(SPEECH_TRAP_DISARMED);
        }
    }
    OpenedTelekinesis = false;
    /*pChestWindow =*/ pGUIWindow_CurrentMenu = new GUIWindow_Chest(uChestID);
    return true;
}

bool Chest::ChestUI_WritePointedObjectStatusString() {
    Pointi pt = mouse->GetCursorPos();
    unsigned int pX = pt.x;
    unsigned int pY = pt.y;

    Chest *chest = &vChests[pGUIWindow_CurrentMenu->wData.val];

    int chestheight = pChestHeightsByType[chest->uChestBitmapID];
    int chestwidth = pChestWidthsByType[chest->uChestBitmapID];

    int inventoryYCoord = (pY - (pChestPixelOffsetY[chest->uChestBitmapID])) / 32;
    int inventoryXCoord = (pX - (pChestPixelOffsetX[chest->uChestBitmapID])) / 32;
    int invMatrixIndex = inventoryXCoord + (chestheight * inventoryYCoord);

    if (inventoryYCoord >= 0 && inventoryYCoord < chestheight &&
        inventoryXCoord >= 0 && inventoryXCoord < chestwidth) {
        int chestindex = chest->pInventoryIndices[invMatrixIndex];
        if (chestindex < 0) {
            invMatrixIndex = (-(chestindex + 1));
            chestindex = chest->pInventoryIndices[invMatrixIndex];
        }

        if (chestindex) {
            int itemindex = chestindex - 1;
            ItemGen *item = &chest->igChestItems[itemindex];

            ///////////////////////////////////////////////
            // normal picking

            GameUI_StatusBar_Set(item->GetDisplayName());
            uLastPointedObjectID = 1;
            return 1;

            ////////////////////////////////////////////////////

            // per pixel transparency check tests
            /*

            auto img = assets->GetImage_16BitColorKey(item->GetIconName(),
            colorTable.TealMask);

            int imgwidth = img->GetWidth();
            int imgheight = img->GetHeight();
            auto pixels = (int32_t*)img->GetPixels(IMAGE_FORMAT_A8R8G8B8);

            Assert(pixels != nullptr, "Cannot get pixels");

            if (imgwidth < 14)
                    imgwidth = 14;

            int v12 = imgwidth - 14;
            v12 = v12 & 0xFFFFFFE0;
             int v13 = v12 + 32;

            if (imgheight < 14)
                    imgheight = 14;

            int chest_offs_x =
            42;//pChestPixelOffsetX[(int)pGUIWindow_CurrentMenu->par1C].uChestBitmapID];
            int chest_offs_y = 34; //
            pChestPixelOffsetY[(int)pGUIWindow_CurrentMenu->par1C].uChestBitmapID];

            int imgX = chest_offs_x + 32 * (invMatrixIndex % chestwidth) +
            ((signed int)(v13 - imgwidth) / 2);

            int imgY = chest_offs_y + 32 * (invMatrixIndex / chestheight) +
                    ((signed int)(((imgheight - 14) & 0xFFFFFFE0) + 32 -
            imgheight) / 2);

            int pix_chk_x = pX-imgX;
            int pix_chk_y = pY-imgY;

            if (pix_chk_x > 0 && pix_chk_x <= imgwidth && pix_chk_y > 0 &&
            pix_chk_y <= imgheight) {

                    pixels += pix_chk_x + pix_chk_y*imgwidth;

                    if (*pixels & 0xFF000000) {
                            GameUI_StatusBar_Set(item->GetDisplayName());
                            uLastPointedObjectID = 1;
                            return 1;
                    }
            }

            */
        }
    }
    return 0;
}

bool Chest::CanPlaceItemAt(int test_cell_position, ITEM_TYPE item_id, int uChestID) {
    int chest_cell_heght = pChestHeightsByType[vChests[uChestID].uChestBitmapID];
    int chest_cell_width = pChestWidthsByType[vChests[uChestID].uChestBitmapID];

    auto img = assets->getImage_ColorKey(pItemTable->pItems[item_id].iconName);
    unsigned int slot_width = GetSizeInInventorySlots(img->width());
    unsigned int slot_height = GetSizeInInventorySlots(img->height());

    Assert(slot_height > 0 && slot_width > 0, "Items should have nonzero dimensions");
    if ((slot_width + test_cell_position % chest_cell_width <= chest_cell_width) &&
        (slot_height + test_cell_position / chest_cell_width <= chest_cell_heght)) {
        for (unsigned int x = 0; x < slot_width; x++) {
            for (unsigned int y = 0; y < slot_height; y++) {
                if (vChests[uChestID].pInventoryIndices[y * chest_cell_width + x + test_cell_position] != 0) {
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}

int Chest::CountChestItems(int uChestID) {
    // this returns first free slot rather than an actual count
    int item_count = 0;
    int max_items = pChestWidthsByType[vChests[uChestID].uChestBitmapID] *
                    pChestHeightsByType[vChests[uChestID].uChestBitmapID];

    if (max_items <= 0) {
        item_count = -1;
    } else {
        while (vChests[uChestID].igChestItems[item_count].uItemID != ITEM_NULL) {
            ++item_count;
            if (item_count >= max_items) {
                item_count = -1;
                break;
            }
        }
    }
    return item_count;
}

int Chest::PutItemInChest(int position, ItemGen *put_item, int uChestID) {
    int item_in_chest_count = CountChestItems(uChestID);
    int test_pos = 0;

    int max_size = pChestWidthsByType[vChests[uChestID].uChestBitmapID] *
                   pChestHeightsByType[vChests[uChestID].uChestBitmapID];
    int chest_width = pChestWidthsByType[vChests[uChestID].uChestBitmapID];

    if (item_in_chest_count == -1) return 0;

    if (position != -1) {
        if (CanPlaceItemAt(position, put_item->uItemID, uChestID)) {
            test_pos = position;
        } else {
            position = -1;  // try another position?? is this the right behavior
        }
    }

    if (position == -1) {  // no position specified
        for (int _i = 0; _i < max_size; _i++) {
            if (Chest::CanPlaceItemAt(_i, put_item->uItemID, pGUIWindow_CurrentMenu->wData.val)) {
                test_pos = _i;  // found somewhere to place item
                break;
            }
        }

        if (test_pos == max_size) {  // limits check no room
            if (pParty->hasActiveCharacter()) {
                pParty->activeCharacter().playReaction(SPEECH_NO_ROOM);
            }
            return 0;
        }
    }

    GraphicsImage *texture = assets->getImage_ColorKey(put_item->GetIconName());
    unsigned int slot_width = GetSizeInInventorySlots(texture->width());
    unsigned int slot_height = GetSizeInInventorySlots(texture->height());

    Assert(slot_height > 0 && slot_width > 0, "Items should have nonzero dimensions");

    // set inventory indices - memset was eratic??
    for (unsigned int x = 0; x < slot_width; x++) {
        for (unsigned int y = 0; y < slot_height; y++) {
            vChests[uChestID].pInventoryIndices[y * chest_width + x + test_pos] = (-1 - test_pos);
        }
    }

    vChests[uChestID].pInventoryIndices[test_pos] = item_in_chest_count + 1;
    vChests[uChestID].igChestItems[item_in_chest_count] = *put_item;

    return (test_pos + 1);
}

void Chest::PlaceItemAt(unsigned int put_cell_pos, unsigned int item_at_cell, int uChestID) {  // only used for setup?
    ITEM_TYPE uItemID = vChests[uChestID].igChestItems[item_at_cell].uItemID;
    pItemTable->SetSpecialBonus(&vChests[uChestID].igChestItems[item_at_cell]);
    if (isWand(uItemID) && !vChests[uChestID].igChestItems[item_at_cell].uNumCharges) {
        int v6 = grng->random(21) + 10;
        vChests[uChestID].igChestItems[item_at_cell].uNumCharges = v6;
        vChests[uChestID].igChestItems[item_at_cell].uMaxCharges = v6;
    }

    auto img = assets->getImage_Alpha(pItemTable->pItems[uItemID].iconName);

    int v9 = img->width();
    if (v9 < 14) v9 = 14;
    unsigned int texture_cell_width = ((v9 - 14) >> 5) + 1;
    int v10 = img->height();
    if (v10 < 14) v10 = 14;
    int textute_cell_height = ((v10 - 14) >> 5) + 1;

    int chest_cell_width = pChestWidthsByType[vChests[uChestID].uChestBitmapID];
    int chest_cell_row_pos = 0;
    for (int i = 0; i < textute_cell_height; ++i) {
        for (int j = 0; j < texture_cell_width; ++j)
            vChests[uChestID].pInventoryIndices[put_cell_pos + chest_cell_row_pos + j] = (int16_t)-(put_cell_pos + 1);
        chest_cell_row_pos += chest_cell_width;
    }
    vChests[uChestID].pInventoryIndices[put_cell_pos] = item_at_cell + 1;
}

void Chest::PlaceItems(int uChestID) {  // only sued for setup
    char chest_cells_map[144];   // [sp+Ch] [bp-A0h]@1

    render->ClearZBuffer();
    int uChestArea = pChestWidthsByType[vChests[uChestID].uChestBitmapID] * pChestHeightsByType[vChests[uChestID].uChestBitmapID];
    memset(chest_cells_map, 0, 144);
    // fill cell map at random positions
    for (int items_counter = 0; items_counter < uChestArea; ++items_counter) {
        // get random position in chest
        int random_chest_pos = 0;
        do {
            random_chest_pos = grng->random(256);
        } while (random_chest_pos >= uChestArea);
        // if this pos occupied move to next
        while (chest_cells_map[random_chest_pos]) {
            ++random_chest_pos;
            if (random_chest_pos == uChestArea) random_chest_pos = 0;
        }
        chest_cells_map[random_chest_pos] = items_counter;
    }

    for (int items_counter = 0; items_counter < uChestArea; ++items_counter) {
        ITEM_TYPE chest_item_id = vChests[uChestID].igChestItems[items_counter].uItemID;
        assert(chest_item_id >= ITEM_NULL && "Checking that generated items are valid");
        if (chest_item_id != ITEM_NULL && !vChests[uChestID].igChestItems[items_counter].placedInChest) {
            int test_position = 0;
            while (!Chest::CanPlaceItemAt((uint8_t)chest_cells_map[test_position], chest_item_id, uChestID)) {
                ++test_position;
                if (test_position >= uChestArea) break;
            }
            if (test_position < uChestArea) {
                Chest::PlaceItemAt((uint8_t)chest_cells_map[test_position], items_counter, uChestID);
                vChests[uChestID].igChestItems[items_counter].placedInChest = true;
                if (vChests[uChestID].uFlags & CHEST_OPENED) {
                    vChests[uChestID].igChestItems[items_counter].SetIdentified();
                }
            } else {
                logger->verbose("Cannot place item with id {} in the chest!", std::to_underlying(chest_item_id));
            }
        }
    }
    vChests[uChestID].SetInitialized(true);
}

void Chest::toggleFlag(int uChestID, ChestFlag uFlag, bool bValue) {
    if (uChestID >= 0 && uChestID <= 19) {
        if (bValue)
            vChests[uChestID].uFlags |= uFlag;
        else
            vChests[uChestID].uFlags &= ~uFlag;
    }
}

void RemoveItemAtChestIndex(int index) {
    Chest *chest = &vChests[pGUIWindow_CurrentMenu->wData.val];

    int chestindex = chest->pInventoryIndices[index];
    ItemGen *item_in_slot = &chest->igChestItems[chestindex - 1];

    auto img = assets->getImage_ColorKey(item_in_slot->GetIconName());
    unsigned int slot_width = GetSizeInInventorySlots(img->width());
    unsigned int slot_height = GetSizeInInventorySlots(img->height());

    int chestwidth = pChestWidthsByType[chest->uChestBitmapID];

    item_in_slot->Reset();

    if (slot_width > 0) {
        // blank inventory indices - memset was eratic??
        for (unsigned int x = 0; x < slot_width; x++) {
            for (unsigned int y = 0; y < slot_height; y++) {
                chest->pInventoryIndices[y * chestwidth + x + index] = 0;
            }
        }
    }
}

void Chest::OnChestLeftClick() {
    int uChestID = pGUIWindow_CurrentMenu->wData.val;
    Chest *chest = &vChests[uChestID];

    int chestheight = pChestHeightsByType[chest->uChestBitmapID];
    int chestwidth = pChestWidthsByType[chest->uChestBitmapID];

    int pX;
    int pY;
    mouse->GetClickPos(&pX, &pY);
    int inventoryYCoord = (pY - (pChestPixelOffsetY[chest->uChestBitmapID])) / 32;
    int inventoryXCoord = (pX - (pChestPixelOffsetX[chest->uChestBitmapID])) / 32;

    int invMatrixIndex = inventoryXCoord + (chestheight * inventoryYCoord);

    if (inventoryYCoord >= 0 && inventoryYCoord < chestheight &&
        inventoryXCoord >= 0 && inventoryXCoord < chestwidth) {
        if (pParty->pPickedItem.uItemID != ITEM_NULL) {  // item held
            if (Chest::PutItemInChest(invMatrixIndex, &pParty->pPickedItem, uChestID)) {
                mouse->RemoveHoldingItem();
            }
        } else {
            int chestindex = chest->pInventoryIndices[invMatrixIndex];
            if (chestindex < 0) {
                invMatrixIndex = (-(chestindex + 1));
                chestindex = chest->pInventoryIndices[invMatrixIndex];
            }

            if (chestindex > 0) {
                int itemindex = chestindex - 1;

                if (chest->igChestItems[itemindex].isGold()) {
                    pParty->partyFindsGold(chest->igChestItems[itemindex].special_enchantment, GOLD_RECEIVE_SHARE);
                } else {
                    pParty->setHoldingItem(&chest->igChestItems[itemindex]);
                }

                RemoveItemAtChestIndex(invMatrixIndex);
                if (engine->config->gameplay.ChestTryPlaceItems.value() == 2)
                    Chest::PlaceItems(uChestID);
            }
        }
    }
}

void Chest::GrabItem(bool all) {  // new fucntion to grab items from chest using spacebar
    if (pParty->pPickedItem.uItemID != ITEM_NULL || !pParty->hasActiveCharacter()) {
        return;
    }

    int InventSlot;
    int grabcount = 0;
    int goldcount = 0;
    int goldamount = 0;

    Chest *chest = &vChests[pGUIWindow_CurrentMenu->wData.val];

    // loop through chest pInvetoryIndices
    for (int loop = 0; loop < 140; loop++) {
        int chestindex = chest->pInventoryIndices[loop];
        if (chestindex <= 0) continue;  // no item here

        int itemindex = chestindex - 1;
        ItemGen chestitem = chest->igChestItems[itemindex];
        if (chestitem.isGold()) {
            pParty->partyFindsGold(chestitem.special_enchantment, GOLD_RECEIVE_SHARE);
            goldamount += chestitem.special_enchantment;
            goldcount++;
        } else {  // this should add item to invetory of active char - if that fails set as holding item and break
            if (pParty->hasActiveCharacter() && (InventSlot = pParty->activeCharacter().AddItem(-1, chestitem.uItemID)) != 0) {  // can place
                memcpy(&pParty->activeCharacter().pInventoryItemList[InventSlot - 1], &chestitem, 0x24u);
                grabcount++;
                GameUI_SetStatusBar(LSTR_FMT_YOU_FOUND_ITEM, pItemTable->pItems[chestitem.uItemID].pUnidentifiedName);
            } else {  // no room so set as holding item
                pParty->setHoldingItem(&chestitem);
                RemoveItemAtChestIndex(loop);
                pParty->activeCharacter().playReaction(SPEECH_NO_ROOM);
                break;
            }
        }
        RemoveItemAtChestIndex(loop);
        if (all == false)  // only grab 1 item
            break;
    }

    if (grabcount > 1 || goldcount > 1) {  // found items
        GameUI_SetStatusBar(fmt::format("You found {} item(s) and {} Gold!", grabcount, goldamount));
    }
    if (grabcount == 0 && goldcount == 0) {
        GameUI_SetStatusBar(LSTR_NOTHING_HERE);
    }
}

void GenerateItemsInChest() {
    MAP_TYPE mapType = pMapStats->GetMapInfo(pCurrentMapName);
    MapInfo *currMapInfo = &pMapStats->pInfos[mapType];
    for (int i = 0; i < 20; ++i) {
        for (int j = 0; j < 140; ++j) {
            ItemGen *currItem = &vChests[i].igChestItems[j];
            if (isRandomItem(currItem->uItemID)) {
                currItem->placedInChest = false;
                int additionaItemCount = grng->random(5);  // additional items in chect
                additionaItemCount++;  // + 1 because it's the item at pChests[i].igChestItems[j] and the additional ones
                ITEM_TREASURE_LEVEL resultTreasureLevel = grng->randomSample(
                    RemapTreasureLevel(randomItemTreasureLevel(currItem->uItemID), currMapInfo->Treasure_prob));
                if (resultTreasureLevel != ITEM_TREASURE_LEVEL_GUARANTEED_ARTIFACT) {
                    for (int k = 0; k < additionaItemCount; k++) {
                        int whatToGenerateProb = grng->random(100);
                        if (whatToGenerateProb < 20) {
                            currItem->Reset();
                        } else if (whatToGenerateProb < 60) {  // generate gold
                            int goldAmount = 0;
                            currItem->Reset();
                            // TODO(captainurist): merge with the other implementation?
                            switch (resultTreasureLevel) {
                            case ITEM_TREASURE_LEVEL_1:
                                goldAmount = grng->random(51) + 50;
                                currItem->uItemID = ITEM_GOLD_SMALL;
                                break;
                            case ITEM_TREASURE_LEVEL_2:
                                goldAmount = grng->random(101) + 100;
                                currItem->uItemID = ITEM_GOLD_SMALL;
                                break;
                            case ITEM_TREASURE_LEVEL_3:
                                goldAmount = grng->random(301) + 200;
                                currItem->uItemID = ITEM_GOLD_MEDIUM;
                                break;
                            case ITEM_TREASURE_LEVEL_4:
                                goldAmount = grng->random(501) + 500;
                                currItem->uItemID = ITEM_GOLD_MEDIUM;
                                break;
                            case ITEM_TREASURE_LEVEL_5:
                                goldAmount = grng->random(1001) + 1000;
                                currItem->uItemID = ITEM_GOLD_LARGE;
                                break;
                            case ITEM_TREASURE_LEVEL_6:
                                goldAmount = grng->random(3001) + 2000;
                                currItem->uItemID = ITEM_GOLD_LARGE;
                                break;
                            default:
                                assert(false);
                                break;
                            }
                            currItem->SetIdentified();
                            currItem->special_enchantment = (ITEM_ENCHANTMENT)goldAmount;
                        } else {
                            pItemTable->generateItem(resultTreasureLevel, 0, currItem);
                        }

                        for (int m = 0; m < 140; m++) {
                            if (vChests[i].igChestItems[m].uItemID == ITEM_NULL) {
                                currItem = &vChests[i].igChestItems[m];
                                break;
                            }
                        }
                    }
                } else {
                    currItem->GenerateArtifact();
                }
            }
        }
    }
}
