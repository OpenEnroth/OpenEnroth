#include "GUI/UI/UIArena.h"

#include "Engine/Engine.h"
#include "Engine/Events.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Localization.h"
#include "Engine/LOD.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Party.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"
#include "GUI/UI/UIHouses.h"

#include "Media/Audio/AudioPlayer.h"


std::array<Vec2_int_, 20> pMonsterArenaPlacements = {{
    Vec2_int_(1524, 8332),    Vec2_int_(2186, 8844),
    Vec2_int_(3219, 9339),    Vec2_int_(4500, 9339),
    Vec2_int_(5323, 9004),    Vec2_int_(0x177D, 0x2098),
    Vec2_int_(0x50B, 0x1E15), Vec2_int_(0x18FF, 0x1E15),
    Vec2_int_(0x50B, 0xD69),  Vec2_int_(0x18FF, 0x1B15),
    Vec2_int_(0x50B, 0x1021), Vec2_int_(0x18FF, 0x1848),
    Vec2_int_(0x50B, 0x12D7), Vec2_int_(0x18FF, 0x15A3),
    Vec2_int_(0x50B, 0x14DB), Vec2_int_(0x18FF, 0x12D7),
    Vec2_int_(0x50B, 0x1848), Vec2_int_(0x18FF, 0x1021),
    Vec2_int_(0x50B, 0x1B15), Vec2_int_(0x18FF, 0xD69),
}};

void Arena_SelectionFightLevel() {
    // GUIButton *v5;  // eax@18
    // GUIButton *v6;  // esi@19

    if (pParty->field_7B5_in_arena_quest) {
        if (pParty->field_7B5_in_arena_quest == -1) {
            uDialogueType = DIALOGUE_ARENA_ALREADY_WON;
        } else {
            int v0 = 0;
            for (size_t i = 0; i < uNumActors; i++) {
                if (pActors[i].uAIState == Dead ||
                    pActors[i].uAIState == Removed ||
                    pActors[i].uAIState == Disabled ||
                    pActors[i].uSummonerID != 0 &&
                        PID_TYPE(pActors[i].uSummonerID) == OBJECT_Player)
                    ++v0;
            }
            if (v0 >= (signed int)uNumActors || (signed int)uNumActors <= 0) {
                uDialogueType = DIALOGUE_ARENA_REWARD;
                ++*((char *)&pParty->monster_for_hunting_killed[3] +
                    (unsigned __int8)pParty->field_7B5_in_arena_quest +
                    1);  // Ritor1:I dont know
                for (uint i = 0; i < 4; i++)
                    pParty->pPlayers[i].SetVariable(
                        VAR_Award,
                        (unsigned __int8)pParty->field_7B5_in_arena_quest + 3);
                pParty->PartyFindsGold(gold_transaction_amount, 0);
                pAudioPlayer->PlaySound(SOUND_51heroism03, 0, 0, -1, 0, 0);
                pParty->field_7B5_in_arena_quest = -1;
            } else {
                uDialogueType = DIALOGUE_ARENA_WELCOME;
                pParty->vPosition.x = 3849;
                pParty->vPosition.y = 5770;
                pParty->vPosition.z = 1;
                pParty->uFallStartY = 1;
                pParty->sRotationZ = 512;
                pParty->sRotationX = 0;
                pParty->uFallSpeed = 0;
                pAudioPlayer->PlaySound(SOUND_51heroism03, 0, 0, -1, 0, 0);
            }
        }
    } else {
        uDialogueType = DIALOGUE_ARENA_WELCOME;
        pDialogueWindow->DeleteButtons();
        pBtn_ExitCancel = pDialogueWindow->CreateButton(
            471, 445, 0xA9u, 0x23u, 1, 0, UIMSG_Escape, 0, GameKey::None,
            localization->GetString(79),  // Close, Закрыть
            {{ui_exit_cancel_button_background}});
        pDialogueWindow->CreateButton(480, 160, 0x8Cu, 0x1Eu, 1, 0, UIMSG_SelectNPCDialogueOption, 85);
        pDialogueWindow->CreateButton(480, 190, 0x8Cu, 0x1Eu, 1, 0, UIMSG_SelectNPCDialogueOption, 86);
        pDialogueWindow->CreateButton(480, 220, 0x8Cu, 0x1Eu, 1, 0, UIMSG_SelectNPCDialogueOption, 87);
        pDialogueWindow->CreateButton(480, 250, 0x8Cu, 0x1Eu, 1, 0, UIMSG_SelectNPCDialogueOption, 88);
        pDialogueWindow->_41D08F_set_keyboard_control_group(4, 1, 0, 1);
    }
}

//----- (004BC109) --------------------------------------------------------
void ArenaFight() {
    int v0;                  // edi@1
    int v3;                  // eax@10
    signed int v4;           // esi@10
    signed int v6;           // ebx@34
    signed int v13;          // eax@49
    int v14;                 // esi@49
    int v15;                 // edx@50
    int v16;                 // eax@51
    signed int v17;          // ecx@51
    int v18;                 // edx@53
    int i;                   // edi@55
    signed int v22;          // [sp-4h] [bp-144h]@51
    __int16 v23[100];        // [sp+Ch] [bp-134h]@39
    __int16 monster_ids[6];  // [sp+128h] [bp-18h]@56
    int v26;                 // [sp+134h] [bp-Ch]@1
    int num_monsters;        // [sp+13Ch] [bp-4h]@17

    v26 = 0;
    pParty->field_7B5_in_arena_quest = uDialogueType;
    GUIWindow window = *pDialogueWindow;
    window.uFrameWidth = game_viewport_width;
    window.uFrameZ = 452;
    v0 = pFontArrus->CalcTextHeight(localization->GetString(575),
                                    window.uFrameWidth, 13) +
         7;  // Пожалуйста, подождите пока я вызываю существ. Удачи.
    render->BeginSceneD3D();

    if (uCurrentlyLoadedLevelType == LEVEL_Indoor)
        pIndoor->Draw();
    else if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
        pOutdoor->Draw();

    render->DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();
    render->BeginScene();
    // if ( render->pRenderD3D )
    render->FillRectFast(
        pViewport->uViewportTL_X, pViewport->uViewportTL_Y,
        pViewport->uViewportBR_X - pViewport->uViewportTL_X,
        pViewport->uViewportBR_Y - pViewport->uViewportTL_Y + 1, 0x7FF);

    render->DrawTextureCustomHeight(8 / 640.0f, (352 - v0) / 480.0f,
                                    ui_leather_mm7, v0);

    render->DrawTextureAlphaNew(8 / 640.0f, (347 - v0) / 480.0f,
                                _591428_endcap);
    String v1 = pFontArrus->FitTextInAWindow(
        localization->GetString(575), window.uFrameWidth,
        0xDu);  // Пожалуйста, подождите пока я вызываю существ. Удачи.
    pDialogueWindow->DrawText(pFontArrus, 13, 354 - v0, 0, v1, 0, 0, 0);
    render->EndScene();
    render->Present();
    pParty->vPosition.x = 3849;
    pParty->vPosition.y = 5770;
    pParty->vPosition.z = 1;
    pParty->uFallStartY = 1;
    pParty->sRotationZ = 512;
    pParty->sRotationX = 0;
    pParty->uFallSpeed = 0;
    /*if ( (signed int)pMessageQueue_50CBD0->uNumMessages < 40 )
    {
    pMessageQueue_50CBD0->pMessages[pMessageQueue_50CBD0->uNumMessages].eType =
    UIMSG_Escape;
    pMessageQueue_50CBD0->pMessages[pMessageQueue_50CBD0->uNumMessages].param =
    1;
    *(&pMessageQueue_50CBD0->uNumMessages + 3 *
    pMessageQueue_50CBD0->uNumMessages + 3) = 0;
    ++pMessageQueue_50CBD0->uNumMessages;
    }*/
    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
    // v2 = pParty->pPlayers.data();
    for (uint i = 0; i < 4; i++) {
        v3 = pParty->pPlayers[i].GetActualLevel();
        v4 = v26;
        if (v3 > v26) {
            v26 = pParty->pPlayers[i].GetActualLevel();
            v4 = pParty->pPlayers[i].GetActualLevel();
        }
        // ++v2;
    }
    // while ( (signed int)v2 < (signed int)pParty->pHirelings.data() );
    if (uDialogueType == DIALOGUE_ARENA_SELECT_PAGE) {
        num_monsters = v4;
        v4 /= 2;
    } else if (uDialogueType == DIALOGUE_ARENA_SELECT_SQUIRE) {
        // v5 = (signed __int64)((double)v26 * 1.5);
        num_monsters = (int)((double)v26 * 1.5);
        v4 /= 2;
    } else if (uDialogueType == DIALOGUE_ARENA_SELECT_KNIGHT) {
        // LODWORD(v5) = 2 * v4;
        num_monsters = 2 * v4;
        v4 /= 2;
    } else if (uDialogueType == DIALOGUE_ARENA_SELECT_CHAMPION) {
        num_monsters = 2 * v4;
        v4 /= 2;
    }
    if (v4 < 1) v4 = 1;
    if (v4 > 100) v4 = 100;
    if (num_monsters > 100) num_monsters = 100;
    if (v4 < 2) v4 = 2;
    if (num_monsters < 2) num_monsters = 2;
    v6 = 0;
    // v27 = 1;
    // v7 = (char *)&pMonsterStats->pInfos[1].uLevel;
    for (unsigned int i = 1; i <= 258; i++) {
        if (pMonsterStats->pInfos[i].uAIType != 1) {  // if ( v7[8] != 1 )
            if (!MonsterStats::BelongsToSupertype(
                    pMonsterStats->pInfos[i].uID,
                    MONSTER_SUPERTYPE_8)) {  //! MonsterStats::BelongsToSupertype(*((short
                                             //! *)v7 + 22), MONSTER_SUPERTYPE_8)
                // v8 = (unsigned __int8)pMonsterStats->pInfos[i].uLevel;
                if (pMonsterStats->pInfos[i].uLevel >= v4) {
                    if (pMonsterStats->pInfos[i].uLevel <= num_monsters)
                        v23[v6++] = i;
                }
            }
        }
        // ++v27;
        // v7 += 88;
    }
    // while ( (signed int)v7 <= (signed int)&pMonsterStats->pInfos[258].uLevel
    // );
    num_monsters = 6;
    if (v6 < 6) num_monsters = v6;
    // v9 = 0;
    if (num_monsters > 0) {
        for (uint i = 0; i < num_monsters; i++) {
            // v10 = rand();
            // ++v9;
            // v12 = __OFSUB__(v9, num_monsters);
            // v11 = v9 - num_monsters < 0;
            // *((short *)&window.pControlsTail + v9 + 1) = v23[rand() % v6];
            monster_ids[i] = v23[rand() % v6];
        }
        // while ( v11 ^ v12 );
    }

    if (uDialogueType == DIALOGUE_ARENA_SELECT_PAGE) {
        v16 = rand();
        v17 = 3;
        v22 = 50;
        v18 = v16 % v17;
        v13 = v22;
        v14 = v18 + 6;
    } else if (uDialogueType == DIALOGUE_ARENA_SELECT_SQUIRE) {
        v16 = rand();
        v17 = 7;
        v22 = 100;
        v18 = v16 % v17;
        v13 = v22;
        v14 = v18 + 6;
    } else if (uDialogueType == DIALOGUE_ARENA_SELECT_KNIGHT) {
        v15 = rand() % 11;
        v13 = 200;
        v14 = v15 + 10;
    } else {
        if (uDialogueType == DIALOGUE_ARENA_SELECT_CHAMPION) {
            v13 = 500;
            v14 = 20;
        }
        // else
        //{
        // v14 = v27;
        // v13 = gold_transaction_amount;
        //}
    }
    gold_transaction_amount = v26 * v13;
    for (i = 0; i < v14; ++i)
        Actor::Arena_summon_actor(monster_ids[rand() % num_monsters],
                                  pMonsterArenaPlacements[i].x,
                                  pMonsterArenaPlacements[i].y, 1);
    pAudioPlayer->PlaySound(SOUND_51heroism03, 0, 0, -1, 0, 0);
}
