#include "Interaction.h"

#include <cassert>

#include "Engine/Engine.h"
#include "Engine/Evt/Processor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Party.h"
#include "Engine/Tables/ItemTable.h"

#include "GUI/GUIMessageQueue.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIBranchlessDialogue.h"
#include "GUI/UI/UIStatusBar.h"

#include "Library/Logger/Logger.h"

void ItemInteraction(int item_id) {
    if (pItemTable->items[pSpriteObjects[item_id].containing_item.itemId].type == ITEM_TYPE_GOLD) {
        pParty->partyFindsGold(pSpriteObjects[item_id].containing_item.goldAmount, GOLD_RECEIVE_SHARE);
    } else {
        if (pParty->pPickedItem.itemId != ITEM_NULL) {
            return;
        }

        engine->_statusBar->setEvent(LSTR_YOU_FOUND_AN_ITEM_S, pItemTable->items[pSpriteObjects[item_id].containing_item.itemId].unidentifiedName);

        // TODO: WTF? 184 / 185 qbits are associated with Tatalia's Mercenery Guild Harmondale raids. Are these about castle's tapestries ?
        if (pSpriteObjects[item_id].containing_item.itemId == ITEM_ARTIFACT_SPLITTER) {
            pParty->_questBits.set(QBIT_SPLITTER_FOUND);
        }
        if (pSpriteObjects[item_id].containing_item.itemId == ITEM_SPELLBOOK_REMOVE_FEAR) {
            pParty->_questBits.set(QBIT_REMOVE_FEAR_FOUND);
        }
        if (!pParty->addItemToParty(&pSpriteObjects[item_id].containing_item)) {
            pParty->setHoldingItem(pSpriteObjects[item_id].containing_item);
        }
    }
    SpriteObject::Remove(item_id);
}

bool CanInteractWithActor(int id) {
    return pActors[id].GetActorsRelation(0) == HOSTILITY_FRIENDLY && pActors[id].ActorFriend() && pActors[id].CanAct();
}

void InteractWithActor(int id) {
    assert(CanInteractWithActor(id));

    Actor::AI_FaceObject(id, Pid::character(0), 0);
    if (pActors[id].npcId) {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_StartNPCDialogue, id, 0);
    } else {
        if (pNPCStats->pGroups[pActors[id].group]) {
            if (!pNPCStats->pCatchPhrases[pNPCStats->pGroups[pActors[id].group]].empty()) {
                branchless_dialogue_str = pNPCStats->pCatchPhrases[pNPCStats->pGroups[pActors[id].group]];
                startBranchlessDialogue(0, 0, EVENT_Invalid);
            }
        }
    }
}

void DecorationInteraction(int id, Pid pid) {
    if (pLevelDecorations[id].uEventID) {
        eventProcessor(pLevelDecorations[id].uEventID, pid, 1);
        pLevelDecorations[id].uFlags |= LEVEL_DECORATION_VISIBLE_ON_MAP;
    } else {
        if (pLevelDecorations[id].IsInteractive()) {
            activeLevelDecoration = &pLevelDecorations[id];
            eventProcessor(engine->_persistentVariables.decorVars[pLevelDecorations[id].eventVarId] + 380, Pid(), 1); // 380 is the MM7 dispatch base, see EVENT_ChangeEvent.
            activeLevelDecoration = nullptr;
        }
    }
}

//----- (0046A334) --------------------------------------------------------
void DoInteractionWithTopmostZObject(Pid pid) {
    auto id = pid.id();
    auto type = pid.type();

    // was SCREEN_BRANCHLESS_NPC_DIALOG
    if (current_screen_type != SCREEN_GAME) {
        return;
    }

    switch (type) {
        case OBJECT_Sprite: {  // take the item
            if (pSpriteObjects[id].IsUnpickable() || id >= pSpriteObjects.size() || !pSpriteObjects[id].uObjectDescID) {
                return;
            }

            ItemInteraction(id);
            break;
        }

        case OBJECT_Actor:
            if (pActors[id].aiState == Dying || pActors[id].aiState == Summoned)
                return;
            if (pActors[id].aiState == Dead) {
                pActors[id].LootActor();
            } else {
                if (CanInteractWithActor(id)) {
                    if (pParty->hasActiveCharacter()) {
                        InteractWithActor(id);
                    } else {
                        engine->_statusBar->setEvent(LSTR_NOBODY_IS_IN_CONDITION);
                    }
                }
            }
            break;

        case OBJECT_Decoration:
            if (pParty->hasActiveCharacter()) {
                DecorationInteraction(id, pid);
            } else {
                engine->_statusBar->setEvent(LSTR_NOBODY_IS_IN_CONDITION);
            }
            break;

        case OBJECT_Face: {
            const BLVFace *face = nullptr;
            if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
                int bmodel_id = id >> 6;
                if (bmodel_id >= pOutdoor->pBModels.size())
                    return;
                face = &pOutdoor->pBModels[bmodel_id].faces[id & 0x3F];
            } else {
                face = &pIndoor->faces[id];
            }

            if (!face->Clickable()) {
                engine->_statusBar->nothingHere();
                return;
            }
            if (face->attributes & FACE_EVENT_IS_HINT || face->eventId == 0)
                return;

            if (pParty->hasActiveCharacter()) {
                eventProcessor(face->eventId, pid, 1);
            } else {
                engine->_statusBar->setEvent(LSTR_NOBODY_IS_IN_CONDITION);
            }
            break;
        }

        default:
            MM_WARNING("Warning: Invalid ID reached!");
            break;
    }
}
