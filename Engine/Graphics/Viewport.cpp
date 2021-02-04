#include "Engine/Graphics/Viewport.h"

#include "Engine/Engine.h"
#include "Engine/Events.h"
#include "Engine/Graphics/DecorationList.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Localization.h"
#include "Engine/LOD.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/OurMath.h"
#include "Engine/Party.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/stru123.h"

#include "GUI/GUIWindow.h"
#include "GUI/UI/UIDialogue.h"
#include "GUI/UI/UIStatusBar.h"

#include "Io/Mouse.h"


//----- (004C0262) --------------------------------------------------------
void Viewport::SetScreen(signed int sTL_X, signed int sTL_Y, signed int sBR_X,
                         signed int sBR_Y) {
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
void Viewport::SetFOV(float field_of_view) {
    this->field_of_view = field_of_view * 65536.0f;
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

    this->sViewCenterX = pParty->vPosition.x;
    this->sViewCenterY = pParty->vPosition.y;
    AdjustPosition();
}

//----- (00443291) --------------------------------------------------------
void ViewingParams::CenterOnPartyZoomIn() {
    int MaxZoom;

    if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        MaxZoom = 1536;
    else if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        MaxZoom = 3072;
    else
        assert(false);

    this->uMapBookMapZoom *= 2;
    if (this->uMapBookMapZoom > MaxZoom) this->uMapBookMapZoom = MaxZoom;

    this->sViewCenterX = pParty->vPosition.x;
    this->sViewCenterY = pParty->vPosition.y;
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
    for (unsigned short i = 0; i < 256; ++i) pPalette[i] = Color16(i, i, i);
}

//----- (00443365) --------------------------------------------------------
void ViewingParams::_443365() {
    Vec3_short_ *v3;  // eax@4
    Vec3_short_ *v6;  // eax@12
    int minimum_y;    // [sp+10h] [bp-10h]@2
    int maximum_y;    // [sp+14h] [bp-Ch]@2
    int minimum_x;    // [sp+18h] [bp-8h]@2
    int maximum_x;    // [sp+1Ch] [bp-4h]@2

    InitGrayPalette();
    if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
        minimum_x = 0x40000000;
        minimum_y = 0x40000000;

        maximum_x = -0x40000000;
        maximum_y = -0x40000000;
        for (int i = 0; i < pIndoor->pMapOutlines->uNumOutlines; ++i) {
            v3 = &pIndoor
                      ->pVertices[pIndoor->pMapOutlines->pOutlines[i].uFace1ID];

            if (v3->x < minimum_x) minimum_x = v3->x;
            if (v3->x > maximum_x) maximum_x = v3->x;
            if (v3->y < minimum_y) minimum_y = v3->x;
            if (v3->y > maximum_y) maximum_y = v3->x;

            v6 = &pIndoor
                      ->pVertices[pIndoor->pMapOutlines->pOutlines[i].uFace2ID];

            if (v6->x < minimum_x) minimum_x = v3->x;
            if (v6->x > maximum_x) maximum_x = v3->x;

            if (v6->y < minimum_y) minimum_y = v3->y;
            if (v6->y > maximum_y) maximum_y = v3->y;
        }

        uMinimapZoom = 1024;
        indoor_center_x = (signed int)(minimum_x + maximum_x) / 2;
        field_28 = 10;
        indoor_center_y = (signed int)(minimum_y + maximum_y) / 2;
    } else {
        indoor_center_x = 0;
        indoor_center_y = 0;
        uMinimapZoom = 512;
        field_28 = 9;
    }
    uMapBookMapZoom = 384;
}

void ItemInteraction(unsigned int item_id) {
    if (pItemsTable->pItems[pSpriteObjects[item_id].containing_item.uItemID]
            .uEquipType == EQUIP_GOLD) {
        pParty->PartyFindsGold(
            pSpriteObjects[item_id].containing_item.special_enchantment, 0);
        viewparams->bRedrawGameUI = 1;
    } else {
        if (pParty->pPickedItem.uItemID) return;

        GameUI_StatusBar_OnEvent(localization->FormatString(
            471,
            pItemsTable->pItems[pSpriteObjects[item_id].containing_item.uItemID]
                .pUnidentifiedName));  // You found an item (%s)!

        if (pSpriteObjects[item_id].containing_item.uItemID ==
            ITEM_ARTIFACT_SPLITTER)
            _449B7E_toggle_bit(pParty->_quest_bits, 184, 1);
        if (pSpriteObjects[item_id].containing_item.uItemID ==
            ITEM_SPELLBOOK_MIND_REMOVE_FEAR)
            _449B7E_toggle_bit(pParty->_quest_bits, 185, 1);
        if (!pParty->AddItemToParty(&pSpriteObjects[item_id].containing_item))
            pParty->SetHoldingItem(&pSpriteObjects[item_id].containing_item);
    }
    SpriteObject::OnInteraction(item_id);
}

bool ActorInteraction(unsigned int id) {
    if (!pActors[id].GetActorsRelation(0) && pActors[id].ActorFriend() &&
        pActors[id].CanAct()) {
        Actor::AI_FaceObject(id, 4, 0, 0);
        if (pActors[id].sNPC_ID) {
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_StartNPCDialogue, id, 0);
        } else {
            if (pNPCStats->pGroups_copy[pActors[id].uGroup]) {
                if (pNPCStats->pCatchPhrases
                        [pNPCStats->pGroups_copy[pActors[id].uGroup]]) {
                    pParty->uFlags |= PARTY_FLAGS_1_0002;
                    branchless_dialogue_str = pNPCStats->pCatchPhrases[pNPCStats->pGroups_copy[pActors[id].uGroup]];
                    sub_4451A8_press_any_key(0, 0, 0);
                }
            }
        }
        return true;
    }
    return false;
}

void DecorationInteraction(unsigned int id, unsigned int pid) {
    if (pLevelDecorations[id].uEventID) {
        EventProcessor(pLevelDecorations[id].uEventID, pid, 1);
        pLevelDecorations[id].uFlags |= LEVEL_DECORATION_VISIBLE_ON_MAP;
    } else {
        if (pLevelDecorations[id].IsInteractive()) {
            activeLevelDecoration = &pLevelDecorations[id];
            EventProcessor(
                stru_5E4C90_MapPersistVars
                        ._decor_events[pLevelDecorations[id]._idx_in_stru123 -
                                       75] +
                    380,
                0, 1);
            activeLevelDecoration = nullptr;
        }
    }
}

void Engine::DropHeldItem() {
    if (!pParty->pPickedItem.uItemID) return;

    SpriteObject a1;
    a1.uType = (SPRITE_OBJECT_TYPE)pItemsTable->pItems[pParty->pPickedItem.uItemID].uSpriteID;
    a1.uObjectDescID = pObjectList->ObjectIDByItemID(a1.uType);
    a1.vPosition.y = pParty->vPosition.y;
    a1.spell_caster_pid = OBJECT_Player;
    a1.vPosition.x = pParty->vPosition.x;
    a1.vPosition.z = pParty->sEyelevel + pParty->vPosition.z;
    a1.uSoundID = 0;
    a1.uFacing = 0;
    a1.uAttributes = 8;
    a1.uSectorID = pIndoor->GetSector(pParty->vPosition.x, pParty->vPosition.y,
                                      pParty->sEyelevel + pParty->vPosition.z);
    a1.uSpriteFrameID = 0;
    memcpy(&a1.containing_item, &pParty->pPickedItem, 0x24u);

    // extern int UnprojectX(int);
    // v9 = UnprojectX(v1->x);
    a1.Create(pParty->sRotationZ, 184, 200, 0);  //+ UnprojectX(v1->x), 184, 200, 0);

    mouse->RemoveHoldingItem();
}

//----- (0042213C) --------------------------------------------------------
void Engine::OnGameViewportClick() {
    int pEventID;     // ecx@21
    SpriteObject a1;  // [sp+Ch] [bp-80h]@1

    int16_t clickable_distance = 512;

    // bug fix - stops you entering shops while dialog still open.
    if (current_screen_type == CURRENT_SCREEN::SCREEN_NPC_DIALOGUE)
        return;

    auto pidAndDepth = vis->get_picked_object_zbuf_val();
    uint16_t pid = pidAndDepth.object_pid;
    int16_t distance = pidAndDepth.depth;
    bool in_range = distance < clickable_distance;
    // else
    //  v0 = render->pActiveZBuffer[v1->x + pSRZBufferLineOffsets[v1->y]];

    if (PID_TYPE(pid) == OBJECT_Item) {
        int item_id = PID_ID(pid);
        // v21 = (signed int)(unsigned __int16)v0 >> 3;
        if (pSpriteObjects[item_id].IsUnpickable() ||
            item_id >= 1000 || !pSpriteObjects[item_id].uObjectDescID || !in_range) {
            if (pParty->pPickedItem.uItemID) {
                DropHeldItem();
            }
        } else {
            ItemInteraction(item_id);
        }
    } else if (PID_TYPE(pid) == OBJECT_Actor) {
        int mon_id = PID_ID(pid);
        // a2.y = v16;
        if (pActors[mon_id].uAIState == Dead) {
            if (in_range)
                pActors[mon_id].LootActor();
            else if (pParty->pPickedItem.uItemID)
                DropHeldItem();
        } else if (!keyboardInputHandler->IsCastOnClickToggled()) {
            if (!in_range) {
                if (pParty->pPickedItem.uItemID) DropHeldItem();
            } else if (!ActorInteraction(mon_id)) {
                if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT) {
                    pTurnEngine->field_18 |= TE_FLAG_8_finished;
                } else {
                    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Attack, 0, 0);
                }
            }
        } else if (pParty->bTurnBasedModeOn && pTurnEngine->turn_stage == TE_MOVEMENT) {
            pParty->uFlags |= PARTY_FLAGS_1_FALLING;
        } else if (uActiveCharacter != 0 && sub_427769_isSpellQuickCastableOnShiftClick(pPlayers[uActiveCharacter]->uQuickSpell)) {
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_CastQuickSpell, 0, 0);
        }
    } else if (PID_TYPE(pid) == OBJECT_Decoration) {
        int id = PID_ID(pid);
        if (distance - pDecorationList->GetDecoration(pLevelDecorations[id].uDecorationDescID)->uRadius >= clickable_distance) {
            if (pParty->pPickedItem.uItemID) {
                DropHeldItem();
            }
        } else {
            DecorationInteraction(id, pid);
        }
    } else if (PID_TYPE(pid) == OBJECT_BModel && in_range) {
        if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
            if (!pIndoor->pFaces[PID_ID(pid)].Clickable()) {
                if (!pParty->pPickedItem.uItemID) {
                    GameUI_StatusBar_NothingHere();
                    if (!pParty->pPickedItem.uItemID)
                        return;
                } else {
                    DropHeldItem();
                }
            } else {
                pEventID = pIndoor->pFaceExtras[pIndoor->pFaces[PID_ID(pid)].uFaceExtraID].uEventID;
                EventProcessor(pEventID, pid, 1);
            }
        } else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor) {
            if (!pOutdoor->pBModels[(signed int)(pid) >> 9].pFaces[PID_ID(pid) & 0x3F].Clickable()) {
                if (!pParty->pPickedItem.uItemID) {
                    GameUI_StatusBar_NothingHere();
                    if (!pParty->pPickedItem.uItemID)
                        return;
                } else {
                    DropHeldItem();
                }
            } else {
                pEventID = pOutdoor->pBModels[(signed int)(pid) >> 9]
                               .pFaces[PID_ID(pid) & 0x3F]
                               .sCogTriggeredID;
                EventProcessor(pEventID, pid, 1);
            }
        }
    } else if (pParty->pPickedItem.uItemID) {
        DropHeldItem();
    }
}
