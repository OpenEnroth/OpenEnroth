#include "Interaction.h"

#include "Engine/Engine.h"
#include "Engine/Evt/Processor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Party.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Tables/DecorationTable.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/TurnEngine/TurnEngine.h"

#include "GUI/GUIMessageQueue.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIBranchlessDialogue.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

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

void Engine::onGameViewportClick() {
    int clickable_distance = engine->config->gameplay.MouseInteractionDepth.value();

    // bug fix - stops you entering shops while dialog still open.
    // was SCREEN_NPC_DIALOGUE
    if (current_screen_type != SCREEN_GAME) {
        return;
    }

    auto pidAndDepth = engine->PickMouseForTargeting();
    Pid pid = pidAndDepth.pid;
    int distance = pidAndDepth.depth;
    bool in_range = distance < clickable_distance;

    if (pid.type() == OBJECT_Sprite) {
        int item_id = pid.id();
        if (pSpriteObjects[item_id].IsUnpickable() || !pSpriteObjects[item_id].uObjectDescID || !in_range) {
            pParty->dropHeldItem();
        } else {
            ItemInteraction(item_id);
        }
    } else if (pid.type() == OBJECT_Actor) {
        int mon_id = pid.id();

        if (pActors[mon_id].aiState == Dead) {
            if (in_range) {
                pActors[mon_id].LootActor();
            } else {
                pParty->dropHeldItem();
            }
        } else if (!keyboardInputHandler->IsCastOnClickToggled()) {
            if (pActors[mon_id].GetActorsRelation(nullptr) == HOSTILITY_FRIENDLY && pActors[mon_id].ActorFriend()) {
                if (!in_range) {
                    pParty->dropHeldItem();
                } else if (pActors[mon_id].CanAct()) {
                    if (pParty->hasActiveCharacter()) {
                        InteractWithActor(mon_id);
                    } else {
                        // Do not interact with actors with no active character
                        engine->_statusBar->setEvent(LSTR_NOBODY_IS_IN_CONDITION);
                    }
                }
            } else {
                if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT) {
                    pTurnEngine->flags |= TE_FLAG_8_finished;
                } else {
                    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Attack, 0, 0);
                }
            }
        } else if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT) {
            pParty->setAirborne(true);
        } else if (pParty->hasActiveCharacter() &&
                   pParty->activeCharacter().uQuickSpell != SPELL_NONE &&
                   IsSpellQuickCastableOnShiftClick(pParty->activeCharacter().uQuickSpell)) {
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_CastQuickSpellAtActor, mon_id, 0);
        } else if (pParty->pPickedItem.itemId != ITEM_NULL) {
            pParty->dropHeldItem();
        } else if (!pParty->hasActiveCharacter()) {
            engine->_statusBar->setEvent(LSTR_NOBODY_IS_IN_CONDITION);
            pAudioPlayer->playUISound(SOUND_error);
        } else {
            engine->_statusBar->setEvent(LSTR_SET_A_QUICK_SPELL);
            pAudioPlayer->playUISound(SOUND_error);
        }
    } else if (pid.type() == OBJECT_Decoration) {
        int id = pid.id();
        if (distance - pDecorationTable->decoration(pLevelDecorations[id].uDecorationDescID)->uRadius < clickable_distance) {
            if (pParty->hasActiveCharacter()) {
                // Do not interact with decoration with no active character
                DecorationInteraction(id, pid);
            } else {
                engine->_statusBar->setEvent(LSTR_NOBODY_IS_IN_CONDITION);
            }
        } else {
            pParty->dropHeldItem();
        }
    } else if (pid.type() == OBJECT_Face && in_range) {
        int eventId = 0;

        if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
            if (!pIndoor->faces[pid.id()].Clickable()) {
                if (pParty->pPickedItem.itemId == ITEM_NULL) {
                    engine->_statusBar->nothingHere();
                } else {
                    pParty->dropHeldItem();
                }
                return;
            } else {
                eventId = pIndoor->faces[pid.id()].eventId;
            }
        } else if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
            const BLVFace &model = pOutdoor->face(pid);
            if (!model.Clickable()) {
                if (pParty->pPickedItem.itemId == ITEM_NULL) {
                    engine->_statusBar->nothingHere();
                } else {
                    pParty->dropHeldItem();
                }
                return;
            } else {
                eventId = model.eventId;
            }
        }

        if (pParty->hasActiveCharacter()) {
            eventProcessor(eventId, pid, 1);
        } else {
            // Do not interact with faces with no active character
            engine->_statusBar->setEvent(LSTR_NOBODY_IS_IN_CONDITION);
        }
    } else {
        pParty->dropHeldItem();
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

        case OBJECT_Face:
            if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
                int bmodel_id = id >> 6;
                int face_id = id & 0x3F;

                if (bmodel_id >= pOutdoor->pBModels.size()) {
                    return;
                }

                BLVFace &model = pOutdoor->pBModels[bmodel_id].faces[face_id];

                if (model.attributes & FACE_EVENT_IS_HINT || model.eventId == 0) {
                    return;
                }

                if (pParty->hasActiveCharacter()) {
                    eventProcessor(pOutdoor->pBModels[bmodel_id].faces[face_id].eventId, pid, 1);
                } else {
                    engine->_statusBar->setEvent(LSTR_NOBODY_IS_IN_CONDITION);
                }
            } else {
                if (!(pIndoor->faces[id].attributes & FACE_CLICKABLE)) {
                    engine->_statusBar->nothingHere();
                    return;
                }
                if (pIndoor->faces[id].attributes & FACE_EVENT_IS_HINT || !pIndoor->faces[id].eventId) {
                    return;
                }

                if (pParty->hasActiveCharacter()) {
                    eventProcessor((int16_t)pIndoor->faces[id].eventId, pid, 1);
                } else {
                    engine->_statusBar->setEvent(LSTR_NOBODY_IS_IN_CONDITION);
                }
            }
            break;

        default:
            MM_WARNING("Warning: Invalid ID reached!");
            break;
    }
}
