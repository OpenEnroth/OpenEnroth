#include "Engine/Graphics/Viewport.h"

#include "Engine/Engine.h"
#include "Engine/Evt/Processor.h"
#include "Engine/Objects/DecorationList.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Spells/Spells.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/TurnEngine/TurnEngine.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIBranchlessDialogue.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

//----- (004C0262) --------------------------------------------------------
void Viewport::SetScreen(int sTL_X, int sTL_Y, int sBR_X, int sBR_Y) {
    unsigned int tl_x;  // edx@1
    unsigned int br_x;  // esi@1
    unsigned int tl_y;  // edi@3
    unsigned int br_y;  // eax@3

    tl_x = sTL_X;
    br_x = sBR_X;
    if (sTL_X > sBR_X) {
        br_x = sTL_X;  // swap x's
        tl_x = sBR_X;
    }
    tl_y = sTL_Y;
    br_y = sBR_Y;
    if (sTL_Y > sBR_Y) {
        br_y = sTL_Y;  // swap y's
        tl_y = sBR_Y;
    }
    this->uScreen_TL_X = tl_x;
    this->uScreen_TL_Y = tl_y;
    this->uScreen_BR_X = br_x;
    this->uScreen_BR_Y = br_y;
    this->uScreenWidth = br_x - tl_x + 1;
    this->uScreenHeight = br_y - tl_y + 1;
    this->uScreenCenterX = (signed int)(br_x + tl_x) / 2;
    // if ( render->pRenderD3D == 0 )
    //    this->uScreenCenterY = this->uScreen_BR_Y - fixpoint_mul(field_30,
    //    uScreenHeight);
    // else
    this->uScreenCenterY = (br_y + tl_y) / 2;
    SetViewport(this->uScreen_TL_X, this->uScreen_TL_Y, this->uScreen_BR_X,
                this->uScreen_BR_Y);
}

//----- (004C02F8) --------------------------------------------------------
void Viewport::ResetScreen() {
    SetScreen(uScreen_TL_X, uScreen_TL_Y, uScreen_BR_X, uScreen_BR_Y);
}

bool Viewport::Contains(unsigned int x, unsigned int y) {
    return ((int)x >= uViewportTL_X && (int)x <= uViewportBR_X &&
            (int)y >= uViewportTL_Y && (int)y <= uViewportBR_Y);
}

void Viewport::SetViewport(int sTL_X, int sTL_Y, int sBR_X, int sBR_Y) {
    int tl_x;
    int tl_y;
    int br_x;
    int br_y;

    tl_x = sTL_X;
    if (sTL_X < this->uScreen_TL_X) tl_x = this->uScreen_TL_X;
    tl_y = sTL_Y;
    if (sTL_Y < this->uScreen_TL_Y) tl_y = this->uScreen_TL_Y;
    br_x = sBR_X;
    if (sBR_X > this->uScreen_BR_X) br_x = this->uScreen_BR_X;
    br_y = sBR_Y;
    if (sBR_Y > this->uScreen_BR_Y) br_y = this->uScreen_BR_Y;
    this->uViewportTL_Y = tl_y;
    this->uViewportTL_X = tl_x;
    this->uViewportBR_X = br_x;
    this->uViewportBR_Y = br_y;
}

//----- (00443219) --------------------------------------------------------
void ViewingParams::MapViewUp() {
    this->sViewCenterY += 512;
    AdjustPosition();
}

//----- (00443225) --------------------------------------------------------
void ViewingParams::MapViewLeft() {
    this->sViewCenterX -= 512;
    AdjustPosition();
}

//----- (00443231) --------------------------------------------------------
void ViewingParams::MapViewDown() {
    this->sViewCenterY -= 512;
    AdjustPosition();
}

//----- (0044323D) --------------------------------------------------------
void ViewingParams::MapViewRight() {
    this->sViewCenterX += 512;
    AdjustPosition();
}

//----- (00443249) --------------------------------------------------------
void ViewingParams::CenterOnPartyZoomOut() {
    this->uMapBookMapZoom /= 2;
    if (this->uMapBookMapZoom < 384) this->uMapBookMapZoom = 384;

    this->sViewCenterX = pParty->pos.x;
    this->sViewCenterY = pParty->pos.y;
    AdjustPosition();
}

//----- (00443291) --------------------------------------------------------
void ViewingParams::CenterOnPartyZoomIn() {
    int MaxZoom;

    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        MaxZoom = 1536;
    } else {
        assert(uCurrentlyLoadedLevelType == LEVEL_INDOOR);
        MaxZoom = 3072;
    }

    this->uMapBookMapZoom *= 2;
    if (this->uMapBookMapZoom > MaxZoom) this->uMapBookMapZoom = MaxZoom;

    this->sViewCenterX = pParty->pos.x;
    this->sViewCenterY = pParty->pos.y;
    AdjustPosition();
}

//----- (004432E7) --------------------------------------------------------
void ViewingParams::AdjustPosition() {
    ViewingParams *v1;  // esi@1
    int v2;             // ebx@1
    signed int v3;      // edx@1
    int v4;             // ecx@1
    int v5;             // edi@3
    int v6;             // eax@3
    int v7;             // eax@5

    v1 = this;
    v2 = this->indoor_center_y;
    v3 = 88 >> (this->uMapBookMapZoom / 384);
    v4 = (44 - v3) << 9;
    if (v1->sViewCenterY > v2 + v4) v1->sViewCenterY = v2 + v4;

    v5 = v1->indoor_center_x;
    v6 = (v3 - 44) << 9;
    if (v1->sViewCenterX < v5 + v6) v1->sViewCenterX = v5 + v6;

    v7 = v2 + v6;
    if (v1->sViewCenterY < v7) v1->sViewCenterY = v7;

    if (v1->sViewCenterX > v5 + v4) v1->sViewCenterX = v5 + v4;
}

//----- (00443343) --------------------------------------------------------
void ViewingParams::InitGrayPalette() {
    for (unsigned short i = 0; i < 256; ++i) pPalette[i] = Color(i, i, i);
}

//----- (00443365) --------------------------------------------------------
void ViewingParams::_443365() {
    Vec3f *v3;  // eax@4
    Vec3f *v6;  // eax@12
    int minimum_y;    // [sp+10h] [bp-10h]@2
    int maximum_y;    // [sp+14h] [bp-Ch]@2
    int minimum_x;    // [sp+18h] [bp-8h]@2
    int maximum_x;    // [sp+1Ch] [bp-4h]@2

    InitGrayPalette();
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        minimum_x = 0x40000000;
        minimum_y = 0x40000000;

        maximum_x = -0x40000000;
        maximum_y = -0x40000000;
        for (int i = 0; i < pIndoor->pMapOutlines.size(); ++i) {
            v3 = &pIndoor->pVertices[pIndoor->pMapOutlines[i].uVertex1ID];

            if (v3->x < minimum_x) minimum_x = v3->x;
            if (v3->x > maximum_x) maximum_x = v3->x;
            if (v3->y < minimum_y) minimum_y = v3->x;
            if (v3->y > maximum_y) maximum_y = v3->x;

            v6 = &pIndoor->pVertices[pIndoor->pMapOutlines[i].uVertex2ID];

            if (v6->x < minimum_x) minimum_x = v3->x;
            if (v6->x > maximum_x) maximum_x = v3->x;

            if (v6->y < minimum_y) minimum_y = v3->y;
            if (v6->y > maximum_y) maximum_y = v3->y;
        }

        uMinimapZoom = 1024;
        indoor_center_x = (signed int)(minimum_x + maximum_x) / 2;
        indoor_center_y = (signed int)(minimum_y + maximum_y) / 2;
    } else {
        indoor_center_x = 0;
        indoor_center_y = 0;
        uMinimapZoom = 512;
    }
    uMapBookMapZoom = 384;
}

void ItemInteraction(unsigned int item_id) {
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
    SpriteObject::OnInteraction(item_id);
}

bool CanInteractWithActor(unsigned int id) {
    return pActors[id].GetActorsRelation(0) == HOSTILITY_FRIENDLY && pActors[id].ActorFriend() && pActors[id].CanAct();
}

void InteractWithActor(unsigned int id) {
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

void DecorationInteraction(unsigned int id, Pid pid) {
    if (pLevelDecorations[id].uEventID) {
        eventProcessor(pLevelDecorations[id].uEventID, pid, 1);
        pLevelDecorations[id].uFlags |= LEVEL_DECORATION_VISIBLE_ON_MAP;
    } else {
        if (pLevelDecorations[id].IsInteractive()) {
            activeLevelDecoration = &pLevelDecorations[id];
            eventProcessor(engine->_persistentVariables.decorVars[pLevelDecorations[id].eventVarId] + 380, Pid(), 1);
            activeLevelDecoration = nullptr;
        }
    }
}

void Engine::onGameViewportClick() {
    int16_t clickable_distance = 512;

    // bug fix - stops you entering shops while dialog still open.
    // was SCREEN_NPC_DIALOGUE
    if (current_screen_type != SCREEN_GAME) {
        return;
    }

    auto pidAndDepth = engine->PickMouseNormal();
    Pid pid = pidAndDepth.pid;
    int16_t distance = pidAndDepth.depth;
    bool in_range = distance < clickable_distance;
    // else
    //  v0 = render->pActiveZBuffer[v1->x + pSRZBufferLineOffsets[v1->y]];

    if (pid.type() == OBJECT_Item) {
        int item_id = pid.id();
        // v21 = (signed int)(uint16_t)v0 >> 3;
        if (pSpriteObjects[item_id].IsUnpickable() || item_id >= 1000 || !pSpriteObjects[item_id].uObjectDescID || !in_range) {
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
            if (CanInteractWithActor(mon_id)) {
                if (in_range) {
                    if (pParty->hasActiveCharacter()) {
                        InteractWithActor(mon_id);
                    } else {
                        // Do not interact with actors with no active character
                        engine->_statusBar->setEvent(LSTR_NOBODY_IS_IN_CONDITION);
                    }
                } else {
                    pParty->dropHeldItem();
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
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_CastQuickSpell, 0, 0);
        } else if (pParty->pPickedItem.itemId != ITEM_NULL) {
            pParty->dropHeldItem();
        } else {
            pAudioPlayer->playUISound(SOUND_error);
        }
    } else if (pid.type() == OBJECT_Decoration) {
        int id = pid.id();
        if (distance - pDecorationList->GetDecoration(pLevelDecorations[id].uDecorationDescID)->uRadius < clickable_distance) {
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
            if (!pIndoor->pFaces[pid.id()].Clickable()) {
                if (pParty->pPickedItem.itemId == ITEM_NULL) {
                    engine->_statusBar->nothingHere();
                } else {
                    pParty->dropHeldItem();
                }
                return;
            } else {
                eventId = pIndoor->pFaceExtras[pIndoor->pFaces[pid.id()].uFaceExtraID].uEventID;
            }
        } else if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
            const ODMFace &model = pOutdoor->face(pid);
            if (!model.Clickable()) {
                if (pParty->pPickedItem.itemId == ITEM_NULL) {
                    engine->_statusBar->nothingHere();
                } else {
                    pParty->dropHeldItem();
                }
                return;
            } else {
                eventId = model.sCogTriggeredID;
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
