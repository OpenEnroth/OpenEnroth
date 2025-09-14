#include "Engine/Objects/Chest.h"

#include <numeric>
#include <unordered_map>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/Objects/DecorationList.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/Random/Random.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Item.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/MapInfo.h"
#include "Engine/Tables/ChestTable.h"

#include "GUI/UI/UIChest.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/ItemGrid.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Logger/Logger.h"

#include "Utility/Math/TrigLut.h"

std::vector<Chest> vChests;

bool Chest::open(int uChestID, Pid objectPid) {
    double dir_x;
    double dir_y;
    double length_vector;
    int pDepth;
    bool flag_shout;
    SpriteId pSpriteID[4];
    Vec3f pOut;
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
    if (chest->Trapped() && engine->_currentLoadedMapId != MAP_INVALID) {
        if (pParty->activeCharacter().GetDisarmTrap() < 2 * pMapStats->pInfos[engine->_currentLoadedMapId].disarmDifficulty) {
            pSpriteID[0] = SPRITE_TRAP_FIRE;
            pSpriteID[1] = SPRITE_TRAP_LIGHTNING;
            pSpriteID[2] = SPRITE_TRAP_COLD;
            pSpriteID[3] = SPRITE_TRAP_BODY;
            int pRandom = grng->random(4); // Not sure if this should be grng or vrng, so we'd rather err on the side of safety.
            int objId = objectPid.id();

            Vec3f objectPos;
            if (chest->position) {
                objectPos = *chest->position;
            } else if (objectPid.type() == OBJECT_Decoration) {
                objectPos = pLevelDecorations[objId].vPosition +
                    Vec3f(0, 0, pDecorationList->GetDecoration(pLevelDecorations[objId].uDecorationDescID)->uDecorationHeight / 2);
            } else if (objectPid.type() == OBJECT_Face) {
                if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
                    objectPos = pOutdoor->face(objectPid).pBoundingBox.center();
                } else {
                    objectPos = pIndoor->pFaces[objId].pBounding.center();
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
                yawAngle = TrigLUT.atan2(dir_x, dir_y);
                pitchAngle = 256;
            }

            pDepth = 96;
            if (length_vector < pDepth) {
                pDepth = length_vector;
            }
            pOut = objectPos + Vec3f::fromPolar(pDepth, yawAngle, pitchAngle);

            pSpellObject.containing_item.Reset();
            pSpellObject.spell_skill = MASTERY_NONE;
            pSpellObject.spell_level = 0;
            pSpellObject.uSpellID = SPELL_NONE;
            pSpellObject.field_54 = 0;
            pSpellObject.uType = pSpriteID[pRandom];
            pSpellObject.uObjectDescID = pObjectList->ObjectIDByItemID(pSpellObject.uType);

            // adjust height to account for different sprite sizes and offset
            SpriteFrame *frame = pSpellObject.getSpriteFrame();
            if (frame->uFlags & 0x20) {
                // centering
                pOut += Vec3f(0, 0, frame->hw_sprites[0]->texture->height() / 4);
            } else {
                pOut -= Vec3f(0, 0, (frame->hw_sprites[0]->texture->height() - 64) / 2);
            }
            pSpellObject.vPosition = pOut;

            pSpellObject.uSoundID = 0;
            pSpellObject.uAttributes = SPRITE_IGNORE_RANGE | SPRITE_NO_Z_BUFFER;
            pSpellObject.uSectorID = pIndoor->GetSector(pSpellObject.vPosition);
            pSpellObject.timeSinceCreated = 0_ticks;
            pSpellObject.spell_caster_pid = Pid();
            pSpellObject.spell_target_pid = Pid();
            pSpellObject.uFacing = 0;
            pSpellObject.Create(0, 0, 0, 0);
            // TODO(Nik-RE-dev): chest is originator in this case
            pAudioPlayer->playSound(SOUND_fireBall, SOUND_MODE_UI);
            pSpellObject.explosionTraps();
            chest->flags &= ~CHEST_TRAPPED;
            if (pParty->hasActiveCharacter() && !OpenedTelekinesis) {
                pParty->setDelayedReaction(SPEECH_TRAP_EXPLODED, pParty->activeCharacterIndex() - 1);
            }
            OpenedTelekinesis = false;
            return false;
        }
        chest->flags &= ~CHEST_TRAPPED;
        flag_shout = true;
    }
    pAudioPlayer->playUISound(SOUND_openchest0101);
    if (flag_shout == true) {
        if (!OpenedTelekinesis) {
            pParty->activeCharacter().playReaction(SPEECH_TRAP_DISARMED);
        }
    }
    OpenedTelekinesis = false;
    pGUIWindow_CurrentMenu = pGUIWindow_CurrentChest = new GUIWindow_Chest(uChestID);
    return true;
}

bool Chest::ChestUI_WritePointedObjectStatusString() {
    Chest *chest = &vChests[pGUIWindow_CurrentChest->chestId()];

    Pointi mousePos = mouse->position();
    // TODO(captainurist): need to use mouse->pickedItemOffset here?
    Pointi inventoryPos = mapToInventoryGrid(mousePos, chestTable[chest->chestTypeId].inventoryOffset);

    InventoryEntry entry = chest->inventory.entry(inventoryPos);
    if (entry) {
        engine->_statusBar->setPermanent(entry->GetDisplayName());
        uLastPointedObjectID = Pid::dummy();
        return 1;
    } else {
        return 0;
    }
}

void Chest::PlaceItems(int uChestID) { // only used for setup
    Chest &chest = vChests[uChestID];

    char chest_cells_map[144];   // [sp+Ch] [bp-A0h]@1

    int uChestArea = chestTable[chest.chestTypeId].size.w * chestTable[chest.chestTypeId].size.h;
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

    Sizei chestSize = chest.inventory.gridSize();
    for (InventoryEntry entry : chest.inventory.entries()) {
        if (entry.zone() != INVENTORY_ZONE_STASH)
            continue;

        assert(!isRandomItem(entry->itemId) && "Checking that generated items are valid");

        Sizei itemSize = entry->inventorySize();
        for (int i = 0; i < uChestArea; i++) {
            Pointi pos(chest_cells_map[i] % chestSize.w, chest_cells_map[i] / chestSize.w);

            if (chest.inventory.canAdd(pos, itemSize)) {
                // TODO(captainurist): this is a weird place to call postGenerate, and we won't call it for items that
                //                     end up buried. But it's here b/c we don't want to break the traces, at least now.
                //                     Redo this properly and write a small test.
                entry->postGenerate(ITEM_SOURCE_CHEST);
                if (chest.flags & CHEST_OPENED)
                    entry->SetIdentified();
                entry = chest.inventory.add(pos, chest.inventory.take(entry));
                break;
            }
        }

        if (entry.zone() == INVENTORY_ZONE_STASH)
            logger->trace("Cannot place item with id {} in the chest!", std::to_underlying(entry->itemId));
    }

    chest.SetInitialized(true);
}

void Chest::toggleFlag(int uChestID, ChestFlag uFlag, bool bValue) {
    if (uChestID >= 0 && uChestID <= 19) {
        if (bValue)
            vChests[uChestID].flags |= uFlag;
        else
            vChests[uChestID].flags &= ~uFlag;
    }
}

void Chest::OnChestLeftClick() {
    int uChestID = pGUIWindow_CurrentChest->chestId();
    Chest *chest = &vChests[uChestID];

    Pointi mousePos = mouse->position();
    Pointi inventoryPos = mapToInventoryGrid(mousePos + mouse->pickedItemOffset, chestTable[chest->chestTypeId].inventoryOffset, &pParty->pPickedItem);

    if (pParty->pPickedItem.itemId != ITEM_NULL) {  // item held
        std::optional<Pointi> pos;
        if (chest->inventory.canAdd(inventoryPos, pParty->pPickedItem)) {
            pos = inventoryPos;
        } else {
            pos = chest->inventory.findSpace(pParty->pPickedItem);
        }

        if (pos) {
            chest->inventory.add(*pos, pParty->takeHoldingItem());
        }
    } else {
        if (InventoryEntry entry = chest->inventory.entry(inventoryPos)) {
            Item item = chest->inventory.take(entry);

            if (item.isGold()) {
                pParty->partyFindsGold(item.goldAmount, GOLD_RECEIVE_SHARE);
            } else {
                Pointi offset = mousePos + mouse->pickedItemOffset - chestTable[chest->chestTypeId].inventoryOffset - (inventoryPos * 32);

                GraphicsImage *tex = assets->getImage_Alpha(item.GetIconName());
                offset -= Pointi(itemOffset(tex->width()), itemOffset(tex->height()));

                pParty->setHoldingItem(item, -offset);
            }

            if (engine->config->gameplay.ChestTryPlaceItems.value() == 2)
                Chest::PlaceItems(uChestID);
        }
    }
}

void Chest::GrabItem(bool all) {  // new function to grab items from chest using spacebar
    if (pParty->pPickedItem.itemId != ITEM_NULL || !pParty->hasActiveCharacter()) {
        return;
    }

    int InventSlot;
    int grabcount = 0;
    int goldcount = 0;
    int goldamount = 0;

    int chestId = pGUIWindow_CurrentChest->chestId();
    Chest *chest = &vChests[chestId];

    for (InventoryEntry entry : chest->inventory.entries()) {
        if (entry->isGold()) {
            pParty->partyFindsGold(entry->goldAmount, GOLD_RECEIVE_SHARE);
            goldamount += entry->goldAmount;
            goldcount++;
        } else {  // this should add item to invetory of active char - if that fails set as holding item and break
            std::optional<Pointi> pos;
            if (pParty->hasActiveCharacter())
                pos = pParty->activeCharacter().inventory.findSpace(*entry);

            if (pos) {  // can place
                pParty->activeCharacter().inventory.add(*pos, *entry);
                grabcount++;
                engine->_statusBar->setEvent(LSTR_YOU_FOUND_AN_ITEM_S, pItemTable->items[entry->itemId].unidentifiedName);
            } else {  // no room so set as holding item
                pParty->setHoldingItem(*entry);
                chest->inventory.take(entry);
                pParty->activeCharacter().playReaction(SPEECH_NO_ROOM);
                break;
            }
        }
        chest->inventory.take(entry);
        if (all == false)  // only grab 1 item
            break;
    }

    if (engine->config->gameplay.ChestTryPlaceItems.value() == 2)
        Chest::PlaceItems(chestId);

    if (grabcount > 1 || goldcount > 1) {  // found items
        engine->_statusBar->setEvent(fmt::format("You found {} item(s) and {} Gold!", grabcount, goldamount));
    }
    if (grabcount == 0 && goldcount == 0) {
        engine->_statusBar->setEvent(LSTR_NOTHING_HERE);
    }
}

void GenerateItemsInChest() {
    MapInfo *currMapInfo = &pMapStats->pInfos[engine->_currentLoadedMapId];
    for (int i = 0; i < 20; ++i) {
        for (InventoryEntry entry : vChests[i].inventory.entries()) {
            if (!isRandomItem(entry->itemId))
                continue;

            int itemCount = grng->randomInSegment(1, 5); // TODO(captainurist): move down & retrace.
            ItemTreasureLevel resultTreasureLevel = grng->randomSample(
                RemapTreasureLevel(randomItemTreasureLevel(entry->itemId), currMapInfo->mapTreasureLevel));

            if (resultTreasureLevel == ITEM_TREASURE_LEVEL_7) {
                // TODO(captainurist): GenerateArtifact calls Reset on failure, this messes up inventory state. Rewrite properly.
                Item item;
                if (item.GenerateArtifact()) {
                    *entry = item;
                } else {
                    vChests[i].inventory.take(entry);
                }
                continue;
            }

            for (int k = 0; k < itemCount; k++) {
                Item item;
                int whatToGenerateProb = grng->random(100);
                if (whatToGenerateProb < 20) {
                    // Do nothing.
                } else if (whatToGenerateProb < 60) {  // generate gold
                    item.generateGold(resultTreasureLevel);
                } else {
                    pItemTable->generateItem(resultTreasureLevel, RANDOM_ITEM_ANY, &item);
                }
                if (item.itemId != ITEM_NULL) {
                    if (entry) {
                        *entry = item;
                        entry = {};
                    } else {
                        vChests[i].inventory.stash(item);
                    }
                }
            }
            if (entry) // We didn't generate anything.
                vChests[i].inventory.take(entry);
        }
    }
}

void UpdateChestPositions() {
    std::unordered_map<int, std::vector<Vec3f>> pointsByChestId;

    auto processEvent = [&](int eventId, const Vec3f &position) {
        // Can there be two EVENT_OpenChest in a single script, with different chests? If no, then we can
        // break out of the loop below early. If yes... Well. This should work.
        if (engine->_localEventMap.hasEvent(eventId))
            for (const EvtInstruction &event : engine->_localEventMap.function(eventId))
                if (event.opcode == EVENT_OpenChest)
                    pointsByChestId[event.data.chest_id].push_back(position);
    };

    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        for (const BSPModel &model : pOutdoor->pBModels)
            for (const ODMFace &face : model.pFaces)
                if (face.sCogTriggeredID)
                    processEvent(face.sCogTriggeredID, face.pBoundingBox.center());
    } else {
        for (const BLVFace &face : pIndoor->pFaces)
            if (face.uFaceExtraID)
                if (int eventId = pIndoor->pFaceExtras[face.uFaceExtraID].uEventID)
                    processEvent(eventId, face.pBounding.center());
    }

    for (const auto &[chestId, points] : pointsByChestId) {
        Vec3f center = std::accumulate(points.begin(), points.end(), Vec3f()) / points.size();

        bool isChestLike = true;
        for (const Vec3f &point : points) {
            if ((point - center).lengthSqr() > 256 * 256) {
                // Wormhole chest detected. 256 is half the size of the ODM tile.
                isChestLike = false;
                break;
            }
        }

        if (isChestLike)
            vChests[chestId].position = center;
    }
}
