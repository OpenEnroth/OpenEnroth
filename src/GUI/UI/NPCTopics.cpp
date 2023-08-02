#include "GUI/UI/NPCTopics.h"

#include <utility>

#include "Engine/Engine.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Localization.h"
#include "Engine/LOD.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Party.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Events/Processor.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIHouses.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Random/Random.h"

// TODO(Nik-RE-dev): refactor and remove
AwardType dword_F8B1AC_award_bit_number;
CharacterSkillType dword_F8B1AC_skill_being_taught; // Address the same as above --- splitting a union into two variables.

static constexpr std::array<Vec2i, 20> pMonsterArenaPlacements = {{
    Vec2i(1524, 8332),    Vec2i(2186, 8844),
    Vec2i(3219, 9339),    Vec2i(4500, 9339),
    Vec2i(5323, 9004),    Vec2i(0x177D, 0x2098),
    Vec2i(0x50B, 0x1E15), Vec2i(0x18FF, 0x1E15),
    Vec2i(0x50B, 0xD69),  Vec2i(0x18FF, 0x1B15),
    Vec2i(0x50B, 0x1021), Vec2i(0x18FF, 0x1848),
    Vec2i(0x50B, 0x12D7), Vec2i(0x18FF, 0x15A3),
    Vec2i(0x50B, 0x14DB), Vec2i(0x18FF, 0x12D7),
    Vec2i(0x50B, 0x1848), Vec2i(0x18FF, 0x1021),
    Vec2i(0x50B, 0x1B15), Vec2i(0x18FF, 0xD69),
}};

static constexpr IndexedArray<int, GUILD_FIRST, GUILD_LAST> priceForMembership = {{
    {GUILD_OF_ELEMENTS, 100},
    {GUILD_OF_SELF,     100},
    {GUILD_OF_AIR,      50},
    {GUILD_OF_EARTH,    50},
    {GUILD_OF_FIRE,     50},
    {GUILD_OF_WATER,    50},
    {GUILD_OF_BODY,     50},
    {GUILD_OF_MIND,     50},
    {GUILD_OF_SPIRIT,   50},
    {GUILD_OF_LIGHT,    1000},
    {GUILD_OF_DARK,     1000}
}};

static constexpr IndexedArray<int, CHARACTER_SKILL_FIRST, CHARACTER_SKILL_LAST> expertSkillMasteryCost = {{
    {CHARACTER_SKILL_STAFF,        2000},
    {CHARACTER_SKILL_SWORD,        2000},
    {CHARACTER_SKILL_DAGGER,       2000},
    {CHARACTER_SKILL_AXE,          2000},
    {CHARACTER_SKILL_SPEAR,        2000},
    {CHARACTER_SKILL_BOW,          2000},
    {CHARACTER_SKILL_MACE,         2000},
    {CHARACTER_SKILL_BLASTER,      0},
    {CHARACTER_SKILL_SHIELD,       1000},
    {CHARACTER_SKILL_LEATHER,      1000},
    {CHARACTER_SKILL_CHAIN,        1000},
    {CHARACTER_SKILL_PLATE,        1000},
    {CHARACTER_SKILL_FIRE,         1000},
    {CHARACTER_SKILL_AIR,          1000},
    {CHARACTER_SKILL_WATER,        1000},
    {CHARACTER_SKILL_EARTH,        1000},
    {CHARACTER_SKILL_SPIRIT,       1000},
    {CHARACTER_SKILL_MIND,         1000},
    {CHARACTER_SKILL_BODY,         1000},
    {CHARACTER_SKILL_LIGHT,        2000},
    {CHARACTER_SKILL_DARK,         2000},
    {CHARACTER_SKILL_ITEM_ID,      500},
    {CHARACTER_SKILL_MERCHANT,     2000},
    {CHARACTER_SKILL_REPAIR,       500},
    {CHARACTER_SKILL_BODYBUILDING, 500},
    {CHARACTER_SKILL_MEDITATION,   500},
    {CHARACTER_SKILL_PERCEPTION,   500},
    {CHARACTER_SKILL_DIPLOMACY,    0}, // not used
    {CHARACTER_SKILL_THIEVERY,     0}, // not used
    {CHARACTER_SKILL_TRAP_DISARM,  500},
    {CHARACTER_SKILL_DODGE,        2000},
    {CHARACTER_SKILL_UNARMED,      2000},
    {CHARACTER_SKILL_MONSTER_ID,   500},
    {CHARACTER_SKILL_ARMSMASTER,   2000},
    {CHARACTER_SKILL_STEALING,     500},
    {CHARACTER_SKILL_ALCHEMY,      500},
    {CHARACTER_SKILL_LEARNING,     2000},
    {CHARACTER_SKILL_CLUB,         500},
    {CHARACTER_SKILL_MISC,         0} // hidden, not used
}};

static constexpr IndexedArray<int, CHARACTER_SKILL_FIRST, CHARACTER_SKILL_LAST> masterSkillMasteryCost = {{
    {CHARACTER_SKILL_STAFF,        5000},
    {CHARACTER_SKILL_SWORD,        5000},
    {CHARACTER_SKILL_DAGGER,       5000},
    {CHARACTER_SKILL_AXE,          5000},
    {CHARACTER_SKILL_SPEAR,        5000},
    {CHARACTER_SKILL_BOW,          5000},
    {CHARACTER_SKILL_MACE,         5000},
    {CHARACTER_SKILL_BLASTER,      0},
    {CHARACTER_SKILL_SHIELD,       3000},
    {CHARACTER_SKILL_LEATHER,      3000},
    {CHARACTER_SKILL_CHAIN,        3000},
    {CHARACTER_SKILL_PLATE,        3000},
    {CHARACTER_SKILL_FIRE,         4000},
    {CHARACTER_SKILL_AIR,          4000},
    {CHARACTER_SKILL_WATER,        4000},
    {CHARACTER_SKILL_EARTH,        4000},
    {CHARACTER_SKILL_SPIRIT,       4000},
    {CHARACTER_SKILL_MIND,         4000},
    {CHARACTER_SKILL_BODY,         4000},
    {CHARACTER_SKILL_LIGHT,        5000},
    {CHARACTER_SKILL_DARK,         5000},
    {CHARACTER_SKILL_ITEM_ID,      2500},
    {CHARACTER_SKILL_MERCHANT,     5000},
    {CHARACTER_SKILL_REPAIR,       2500},
    {CHARACTER_SKILL_BODYBUILDING, 2500},
    {CHARACTER_SKILL_MEDITATION,   2500},
    {CHARACTER_SKILL_PERCEPTION,   2500},
    {CHARACTER_SKILL_DIPLOMACY,    0}, // not used
    {CHARACTER_SKILL_THIEVERY,     0}, // not used
    {CHARACTER_SKILL_TRAP_DISARM,  2500},
    {CHARACTER_SKILL_DODGE,        5000},
    {CHARACTER_SKILL_UNARMED,      5000},
    {CHARACTER_SKILL_MONSTER_ID,   2500},
    {CHARACTER_SKILL_ARMSMASTER,   5000},
    {CHARACTER_SKILL_STEALING,     2500},
    {CHARACTER_SKILL_ALCHEMY,      2500},
    {CHARACTER_SKILL_LEARNING,     5000},
    {CHARACTER_SKILL_CLUB,         2500},
    {CHARACTER_SKILL_MISC,         0} // hidden, not used
}};

static constexpr IndexedArray<int, CHARACTER_SKILL_FIRST, CHARACTER_SKILL_LAST> grandmasterSkillMasteryCost = {{
    {CHARACTER_SKILL_STAFF,        8000},
    {CHARACTER_SKILL_SWORD,        8000},
    {CHARACTER_SKILL_DAGGER,       8000},
    {CHARACTER_SKILL_AXE,          8000},
    {CHARACTER_SKILL_SPEAR,        8000},
    {CHARACTER_SKILL_BOW,          8000},
    {CHARACTER_SKILL_MACE,         8000},
    {CHARACTER_SKILL_BLASTER,      0},
    {CHARACTER_SKILL_SHIELD,       7000},
    {CHARACTER_SKILL_LEATHER,      7000},
    {CHARACTER_SKILL_CHAIN,        7000},
    {CHARACTER_SKILL_PLATE,        7000},
    {CHARACTER_SKILL_FIRE,         8000},
    {CHARACTER_SKILL_AIR,          8000},
    {CHARACTER_SKILL_WATER,        8000},
    {CHARACTER_SKILL_EARTH,        8000},
    {CHARACTER_SKILL_SPIRIT,       8000},
    {CHARACTER_SKILL_MIND,         8000},
    {CHARACTER_SKILL_BODY,         8000},
    {CHARACTER_SKILL_LIGHT,        8000},
    {CHARACTER_SKILL_DARK,         8000},
    {CHARACTER_SKILL_ITEM_ID,      6000},
    {CHARACTER_SKILL_MERCHANT,     8000},
    {CHARACTER_SKILL_REPAIR,       6000},
    {CHARACTER_SKILL_BODYBUILDING, 6000},
    {CHARACTER_SKILL_MEDITATION,   6000},
    {CHARACTER_SKILL_PERCEPTION,   6000},
    {CHARACTER_SKILL_DIPLOMACY,    0}, // not used
    {CHARACTER_SKILL_THIEVERY,     0}, // not used
    {CHARACTER_SKILL_TRAP_DISARM,  6000},
    {CHARACTER_SKILL_DODGE,        8000},
    {CHARACTER_SKILL_UNARMED,      8000},
    {CHARACTER_SKILL_MONSTER_ID,   6000},
    {CHARACTER_SKILL_ARMSMASTER,   8000},
    {CHARACTER_SKILL_STEALING,     6000},
    {CHARACTER_SKILL_ALCHEMY,      6000},
    {CHARACTER_SKILL_LEARNING,     8000},
    {CHARACTER_SKILL_CLUB,         6000},
    {CHARACTER_SKILL_MISC,         0} // hidden, not used
}};

static constexpr std::array<std::pair<int16_t, ITEM_TYPE>, 27> _4F0882_evt_VAR_PlayerItemInHands_vals = {{
    {0x0D4, ITEM_QUEST_VASE},
    {0x0D5, ITEM_RARE_LADY_CARMINES_DAGGER},
    {0x0D6, ITEM_MESSAGE_SCROLL_OF_WAVES},
    {0x0D7, ITEM_MESSAGE_CIPHER},
    {0x0D8, ITEM_QUEST_WORN_BELT},
    {0x0D9, ITEM_QUEST_HEART_OF_THE_WOOD},
    {0x0DA, ITEM_MESSAGE_MAP_TO_EVENMORN_ISLAND},
    {0x0DB, ITEM_QUEST_GOLEM_HEAD},
    {0x0DC, ITEM_QUEST_ABBEY_NORMAL_GOLEM_HEAD},
    {0x0DD, ITEM_QUEST_GOLEM_RIGHT_ARM},
    {0x0DE, ITEM_QUEST_GOLEM_LEFT_ARM},
    {0x0DF, ITEM_QUEST_GOLEM_RIGHT_LEG},
    {0x0E0, ITEM_QUEST_GOLEM_LEFT_LEG},
    {0x0E1, ITEM_QUEST_GOLEM_CHEST},
    {0x0E2, ITEM_SPELLBOOK_DIVINE_INTERVENTION},
    {0x0E3, ITEM_QUEST_DRAGON_EGG},
    {0x0E4, ITEM_QUEST_ZOKARR_IVS_SKULL},
    {0x0E5, ITEM_QUEST_LICH_JAR_EMPTY},
    {0x0E6, ITEM_QUEST_ELIXIR},
    {0x0E7, ITEM_QUEST_CASE_OF_SOUL_JARS},
    {0x0E8, ITEM_QUEST_ALTAR_PIECE_1},
    {0x0E9, ITEM_QUEST_ALTAR_PIECE_2},
    {0x0EA, ITEM_QUEST_CONTROL_CUBE},
    {0x0EB, ITEM_QUEST_WETSUIT},
    {0x0EC, ITEM_QUEST_OSCILLATION_OVERTHRUSTER},
    {0x0ED, ITEM_QUEST_LICH_JAR_FULL},
    {0x0F1, ITEM_RARE_THE_PERFECT_BOW}
}};

void Arena_SelectionFightLevel() {
    // GUIButton *v5;  // eax@18
    // GUIButton *v6;  // esi@19

    if (pParty->field_7B5_in_arena_quest) {
        if (pParty->field_7B5_in_arena_quest == -1) {
            uDialogueType = DIALOGUE_ARENA_ALREADY_WON;
        } else {
            int v0 = 0;
            for (size_t i = 0; i < pActors.size(); i++) {
                if (pActors[i].aiState == Dead ||
                    pActors[i].aiState == Removed ||
                    pActors[i].aiState == Disabled ||
                    pActors[i].summonerId && pActors[i].summonerId.type() == OBJECT_Character)
                    ++v0;
            }
            if (v0 >= (signed int)pActors.size() || (signed int)pActors.size() <= 0) {
                uDialogueType = DIALOGUE_ARENA_REWARD;
                pParty->uNumArenaWins[pParty->field_7B5_in_arena_quest - DIALOGUE_ARENA_SELECT_PAGE]++;
                for (Character &player : pParty->pCharacters) {
                    player.SetVariable(VAR_Award, (uint8_t)pParty->field_7B5_in_arena_quest + 3);
                }
                pParty->partyFindsGold(gold_transaction_amount, GOLD_RECEIVE_SHARE);
                pAudioPlayer->playUISound(SOUND_51heroism03);
                pParty->field_7B5_in_arena_quest = -1;
            } else {
                uDialogueType = DIALOGUE_ARENA_WELCOME;
                pParty->pos = Vec3i(3849, 5770, 1);
                pParty->speed = Vec3i();
                pParty->uFallStartZ = 1;
                pParty->_viewYaw = 512;
                pParty->_viewPitch = 0;
                pAudioPlayer->playUISound(SOUND_51heroism03);
            }
        }
    } else {
        uDialogueType = DIALOGUE_ARENA_WELCOME;
        pDialogueWindow->DeleteButtons();
        pBtn_ExitCancel = pDialogueWindow->CreateButton({471, 445}, {0xA9u, 0x23u}, 1, 0,
            UIMSG_Escape, 0, Io::InputAction::Invalid, localization->GetString(LSTR_DIALOGUE_EXIT), {ui_exit_cancel_button_background});
        pDialogueWindow->CreateButton({480, 160}, {0x8Cu, 0x1Eu}, 1, 0, UIMSG_SelectNPCDialogueOption, DIALOGUE_ARENA_SELECT_PAGE);
        pDialogueWindow->CreateButton({480, 190}, {0x8Cu, 0x1Eu}, 1, 0, UIMSG_SelectNPCDialogueOption, DIALOGUE_ARENA_SELECT_SQUIRE);
        pDialogueWindow->CreateButton({480, 220}, {0x8Cu, 0x1Eu}, 1, 0, UIMSG_SelectNPCDialogueOption, DIALOGUE_ARENA_SELECT_KNIGHT);
        pDialogueWindow->CreateButton({480, 250}, {0x8Cu, 0x1Eu}, 1, 0, UIMSG_SelectNPCDialogueOption, DIALOGUE_ARENA_SELECT_CHAMPION);
        pDialogueWindow->_41D08F_set_keyboard_control_group(4, 1, 0, 1);
    }
}

void ArenaFight() {
    const int LAST_ARENA_FIGHTER_TYPE = 258;
    int v0;                  // edi@1
    int v3;                  // eax@10
    signed int v4;           // esi@10
    signed int v6;           // ebx@34
    signed int v13 = 0;          // eax@49
    int v14 = 0;                 // esi@49
    int v15;                 // edx@50
    signed int v17;          // ecx@51
    int v18;                 // edx@53
    int i;                   // edi@55
    signed int v22;          // [sp-4h] [bp-144h]@51
    int16_t v23[LAST_ARENA_FIGHTER_TYPE] {};        // [sp+Ch] [bp-134h]@39
    int16_t monster_ids[6] {};  // [sp+128h] [bp-18h]@56
    int v26;                 // [sp+134h] [bp-Ch]@1
    int num_monsters = 0;        // [sp+13Ch] [bp-4h]@17

    v26 = 0;
    pParty->field_7B5_in_arena_quest = uDialogueType;
    GUIWindow window = *pDialogueWindow;
    window.uFrameWidth = game_viewport_width;
    window.uFrameZ = 452;
    v0 = pFontArrus->CalcTextHeight(
        localization->GetString(LSTR_PLEASE_WAIT_WHILE_I_SUMMON),
        window.uFrameWidth, 13) + 7;
    render->BeginScene3D();

    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR)
        pIndoor->Draw();
    else if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR)
        pOutdoor->Draw();

    render->DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();
    render->BeginScene2D();

    render->DrawTextureCustomHeight(8 / 640.0f, (352 - v0) / 480.0f,
                                    ui_leather_mm7, v0);

    render->DrawTextureNew(8 / 640.0f, (347 - v0) / 480.0f,
                                _591428_endcap);
    std::string v1 = pFontArrus->FitTextInAWindow(
        localization->GetString(LSTR_PLEASE_WAIT_WHILE_I_SUMMON), window.uFrameWidth,
        13);
    pDialogueWindow->DrawText(pFontArrus, {13, 354 - v0}, colorTable.White, v1);
    render->Present();
    pParty->pos = Vec3i(3849, 5770, 1);
    pParty->speed = Vec3i();
    pParty->uFallStartZ = 1;
    pParty->_viewYaw = 512;
    pParty->_viewPitch = 0;
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
    // v2 = pParty->pCharacters.data();
    for (uint i = 0; i < 4; i++) {
        v3 = pParty->pCharacters[i].GetActualLevel();
        v4 = v26;
        if (v3 > v26) {
            v26 = pParty->pCharacters[i].GetActualLevel();
            v4 = pParty->pCharacters[i].GetActualLevel();
        }
        // ++v2;
    }
    // while ( (signed int)v2 < (signed int)pParty->pHirelings.data() );
    if (uDialogueType == DIALOGUE_ARENA_SELECT_PAGE) {
        num_monsters = v4;
        v4 /= 2;
    } else if (uDialogueType == DIALOGUE_ARENA_SELECT_SQUIRE) {
        // v5 = (int64_t)((double)v26 * 1.5);
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
    for (unsigned int i = 1; i <= LAST_ARENA_FIGHTER_TYPE; i++) {
        if (pMonsterStats->pInfos[i].uAIType != 1) {  // if ( v7[8] != 1 )
            if (!MonsterStats::BelongsToSupertype(
                    pMonsterStats->pInfos[i].uID,
                    MONSTER_SUPERTYPE_8)) {  //! MonsterStats::BelongsToSupertype(*((short
                                             //! *)v7 + 22), MONSTER_SUPERTYPE_8)
                // v8 = (uint8_t)pMonsterStats->pInfos[i].uLevel;
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
            monster_ids[i] = v23[grng->random(v6)];
        }
        // while ( v11 ^ v12 );
    }

    if (uDialogueType == DIALOGUE_ARENA_SELECT_PAGE) {
        v17 = 3;
        v22 = 50;
        v18 = grng->random(v17);
        v13 = v22;
        v14 = v18 + 6;
    } else if (uDialogueType == DIALOGUE_ARENA_SELECT_SQUIRE) {
        v17 = 7;
        v22 = 100;
        v18 = grng->random(v17);
        v13 = v22;
        v14 = v18 + 6;
    } else if (uDialogueType == DIALOGUE_ARENA_SELECT_KNIGHT) {
        v15 = grng->random(11);
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
        Actor::Arena_summon_actor(monster_ids[grng->random(num_monsters)],
                                  pMonsterArenaPlacements[i].x,
                                  pMonsterArenaPlacements[i].y, 1);
    pAudioPlayer->playUISound(SOUND_51heroism03);
}

void DrawJoinGuildWindow(GUILD_ID guild_id) {
    uDialogueType = DIALOGUE_81_join_guild;
    current_npc_text = pNPCTopics[guild_id + 99].pText;
    GetJoinGuildDialogueOption(guild_id);
    pDialogueWindow->Release();
    pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, {render->GetRenderDimensions().w, 350}, guild_id);
    pBtn_ExitCancel = pDialogueWindow->CreateButton({471, 445}, {169, 35}, 1, 0, UIMSG_Escape, 0, Io::InputAction::Invalid,
        localization->GetString(LSTR_CANCEL), { ui_exit_cancel_button_background }
    );
    pDialogueWindow->CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_HouseScreenClick, 0, Io::InputAction::Invalid, "");
    pDialogueWindow->CreateButton({480, 160}, {140, 30}, 1, 0, UIMSG_SelectHouseNPCDialogueOption, DIALOGUE_82_join_guild, Io::InputAction::Invalid,
        localization->GetString(LSTR_JOIN));
    pDialogueWindow->_41D08F_set_keyboard_control_group(1, 1, 0, 2);
    dialog_menu_id = DIALOGUE_OTHER;
}

void _4B3FE5_training_dialogue(int eventId) {
    uDialogueType = DIALOGUE_SKILL_TRAINER;
    current_npc_text = std::string(pNPCTopics[eventId + 168].pText);
    _4B254D_SkillMasteryTeacher(eventId);  // checks whether the facility can be used
    pDialogueWindow->Release();
    pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, {render->GetRenderDimensions().w, 350}, eventId);
    pBtn_ExitCancel = pDialogueWindow->CreateButton({471, 445}, {169, 35}, 1, 0, UIMSG_Escape, 0, Io::InputAction::Invalid,
        localization->GetString(LSTR_CANCEL), { ui_exit_cancel_button_background }
    );
    pDialogueWindow->CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_HouseScreenClick, 0, Io::InputAction::Invalid, "");
    pDialogueWindow->CreateButton({480, 160}, {0x8Cu, 0x1Eu}, 1, 0, UIMSG_SelectHouseNPCDialogueOption, DIALOGUE_79_mastery_teacher, Io::InputAction::Invalid,
        guild_membership_approved ? localization->GetString(LSTR_LEARN) : "");
    pDialogueWindow->_41D08F_set_keyboard_control_group(1, 1, 0, 2);
    dialog_menu_id = DIALOGUE_OTHER;
}

void OracleDialogue() {
    ItemGen *item = nullptr;
    ITEM_TYPE item_id = ITEM_NULL;

    // display "You never had it" if nothing missing will be found
    current_npc_text = pNPCTopics[667].pText;

    // only items with special subquest in range 212-237 and also 241 are recoverable
    for (auto pair : _4F0882_evt_VAR_PlayerItemInHands_vals) {
        int quest_id = pair.first;
        if (pParty->_questBits[quest_id]) {
            ITEM_TYPE search_item_id = pair.second;
            if (!pParty->hasItem(search_item_id) && pParty->pPickedItem.uItemID != search_item_id) {
                item_id = search_item_id;
                break;
            }
        }
    }

    // missing item found
    if (item_id != ITEM_NULL) {
        pParty->pCharacters[0].AddVariable(VAR_PlayerItemInHands, std::to_underlying(item_id));
        // TODO(captainurist): what if fmt throws?
        current_npc_text = fmt::sprintf(pNPCTopics[666].pText, // "Here's %s that you lost. Be careful"
                                        fmt::format("{::}{}\f00000", colorTable.Jonquil.tag(),
                                                    pItemTable->pItems[item_id].pUnidentifiedName));
    }

    // missing item is lich jar and we need to bind soul vessel to lich class character
    // TODO(Nik-RE-dev): this code is walking only through inventory, but item was added to hand, so it will not bind new item if it was acquired
    //                   rather this code will bind jars that already present in inventory to liches that currently do not have binded jars
    if (item_id == ITEM_QUEST_LICH_JAR_FULL) {
        for (int i = 0; i < pParty->pCharacters.size(); i++) {
            if (pParty->pCharacters[i].classType == CHARACTER_CLASS_LICH) {
                bool have_vessels_soul = false;
                for (Character &player : pParty->pCharacters) {
                    for (int idx = 0; idx < Character::INVENTORY_SLOT_COUNT; idx++) {
                        if (player.pInventoryItemList[idx].uItemID == ITEM_QUEST_LICH_JAR_FULL) {
                            if (player.pInventoryItemList[idx].uHolderPlayer == -1) {
                                item = &player.pInventoryItemList[idx];
                            }
                            if (player.pInventoryItemList[idx].uHolderPlayer == i) {
                                have_vessels_soul = true;
                            }
                        }
                    }
                }

                if (item && !have_vessels_soul) {
                    item->uHolderPlayer = i;
                    break;
                }
            }
        }
    }
}

std::string _4B254D_SkillMasteryTeacher(int trainerInfo) {
    uint8_t teacherLevel = (trainerInfo - 200) % 3;
    CharacterSkillType skillBeingTaught = static_cast<CharacterSkillType>((trainerInfo - 200) / 3);
    Character *activePlayer = &pParty->activeCharacter();
    CharacterClassType pClassType = activePlayer->classType;
    CharacterSkillMastery currClassMaxMastery = skillMaxMasteryPerClass[pClassType][skillBeingTaught];
    CharacterSkillMastery masteryLevelBeingTaught = dword_F8B1B0_MasteryBeingTaught = static_cast<CharacterSkillMastery>(teacherLevel + 2);
    guild_membership_approved = false;

    if (currClassMaxMastery < masteryLevelBeingTaught) {
        if (skillMaxMasteryPerClass[getTier2Class(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught) {
            return localization->FormatString(LSTR_FMT_HAVE_TO_BE_PROMOTED, localization->GetClassName(getTier2Class(pClassType)));
        } else if (skillMaxMasteryPerClass[getTier3LightClass(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught &&
                skillMaxMasteryPerClass[getTier3DarkClass(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught) {
            return localization->FormatString(LSTR_FMT_HAVE_TO_BE_PROMOTED_2,
                    localization->GetClassName(getTier3LightClass(pClassType)),
                    localization->GetClassName(getTier3DarkClass(pClassType)));
        } else if (skillMaxMasteryPerClass[getTier3LightClass(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught) {
            return localization->FormatString(LSTR_FMT_HAVE_TO_BE_PROMOTED, localization->GetClassName(getTier3LightClass(pClassType)));
        } else if (skillMaxMasteryPerClass[getTier3DarkClass(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught) {
            return localization->FormatString(LSTR_FMT_HAVE_TO_BE_PROMOTED, localization->GetClassName(getTier3DarkClass(pClassType)));
        } else {
            return localization->FormatString(LSTR_FMT_SKILL_CANT_BE_LEARNED, localization->GetClassName(pClassType));
        }
    }

    // Not in your condition!
    if (!activePlayer->CanAct()) {
        return std::string(pNPCTopics[122].pText);
    }

    // You must know the skill before you can become an expert in it!
    int skillLevel = activePlayer->getSkillValue(skillBeingTaught).level();
    if (!skillLevel) {
        return std::string(pNPCTopics[131].pText);
    }

    // You are already have this mastery in this skill.
    CharacterSkillMastery skillMastery = activePlayer->getSkillValue(skillBeingTaught).mastery();
    if (std::to_underlying(skillMastery) > teacherLevel + 1) {
        return std::string(pNPCTopics[teacherLevel + 128].pText);
    }

    dword_F8B1AC_skill_being_taught = skillBeingTaught;

    bool canLearn = true;

    if (masteryLevelBeingTaught == CHARACTER_SKILL_MASTERY_EXPERT) {
        canLearn = skillLevel >= 4;
        gold_transaction_amount = expertSkillMasteryCost[skillBeingTaught];
    }

    if (masteryLevelBeingTaught == CHARACTER_SKILL_MASTERY_MASTER) {
        switch (skillBeingTaught) {
          case CHARACTER_SKILL_LIGHT:
            canLearn = pParty->_questBits[114];
            break;
          case CHARACTER_SKILL_DARK:
            canLearn = pParty->_questBits[110];
            break;
          case CHARACTER_SKILL_MERCHANT:
            canLearn = activePlayer->GetBasePersonality() >= 50;
            break;
          case CHARACTER_SKILL_BODYBUILDING:
            canLearn = activePlayer->GetBaseEndurance() >= 50;
            break;
          case CHARACTER_SKILL_LEARNING:
            canLearn = activePlayer->GetBaseIntelligence() >= 50;
            break;
          default:
            break;
        }
        canLearn = canLearn && (skillLevel >= 7);
        gold_transaction_amount = masterSkillMasteryCost[skillBeingTaught];
    }

    if (masteryLevelBeingTaught == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
        switch (skillBeingTaught) {
          case CHARACTER_SKILL_LIGHT:
            canLearn = activePlayer->isClass(CHARACTER_CLASS_ARCHAMGE) || activePlayer->isClass(CHARACTER_CLASS_PRIEST_OF_SUN);
            break;
          case CHARACTER_SKILL_DARK:
            canLearn = activePlayer->isClass(CHARACTER_CLASS_LICH) || activePlayer->isClass(CHARACTER_CLASS_PRIEST_OF_MOON);
            break;
          case CHARACTER_SKILL_DODGE:
            canLearn = activePlayer->pActiveSkills[CHARACTER_SKILL_UNARMED].level() >= 10;
            break;
          case CHARACTER_SKILL_UNARMED:
            canLearn = activePlayer->pActiveSkills[CHARACTER_SKILL_DODGE].level() >= 10;
            break;
          default:
            break;
        }
        canLearn = canLearn && (skillLevel >= 10);
        gold_transaction_amount = grandmasterSkillMasteryCost[skillBeingTaught];
    }

    // You don't meet the requirements, and cannot be taught until you do.
    if (!canLearn) {
        return std::string(pNPCTopics[127].pText);
    }

    // You don't have enough gold!
    if (gold_transaction_amount > pParty->GetGold()) {
        return std::string(pNPCTopics[124].pText);
    }

    guild_membership_approved = true;

    return localization->FormatString(
        LSTR_FMT_BECOME_S_IN_S_FOR_D_GOLD,
        localization->MasteryNameLong(masteryLevelBeingTaught),
        localization->GetSkillName(skillBeingTaught),
        gold_transaction_amount
    );
}

const std::string &GetJoinGuildDialogueOption(GUILD_ID guild_id) {
    static const int dialogue_base = 110;
    guild_membership_approved = false;
    dword_F8B1AC_award_bit_number = static_cast<AwardType>(Award_Membership_ElementalGuilds + std::to_underlying(guild_id));
    gold_transaction_amount = priceForMembership[guild_id];

    // TODO(pskelton): check this behaviour
    if (!pParty->hasActiveCharacter())
        pParty->setActiveToFirstCanAct();  // avoid nzi

    if (pParty->activeCharacter().CanAct()) {
        if (pParty->activeCharacter()._achievedAwardsBits[dword_F8B1AC_award_bit_number]) {
            return pNPCTopics[dialogue_base + 13].pText;
        } else {
            if (gold_transaction_amount <= pParty->GetGold()) {
                guild_membership_approved = true;
                return pNPCTopics[dialogue_base + guild_id].pText;
            } else {
                return pNPCTopics[dialogue_base + 14].pText;
            }
        }
    } else {
        return pNPCTopics[dialogue_base + 12].pText;
    }
}

void NPCHireableDialogPrepare() {
    int v0 = 0;
    NPCData *v1 = houseNpcs[currentHouseNpc].npc;

    pDialogueWindow->Release();
    pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, {render->GetRenderDimensions().w, 350}, 0);
    pBtn_ExitCancel = pDialogueWindow->CreateButton({471, 445}, {169, 35}, 1, 0,
        UIMSG_Escape, 0, Io::InputAction::Invalid, localization->GetString(LSTR_CANCEL), {ui_exit_cancel_button_background}
    );
    pDialogueWindow->CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_HouseScreenClick, 0);
    if (!pNPCStats->pProfessions[v1->profession].pBenefits.empty()) {
        pDialogueWindow->CreateButton({480, 160}, {140, 30}, 1, 0,
            UIMSG_SelectHouseNPCDialogueOption, DIALOGUE_PROFESSION_DETAILS, Io::InputAction::Invalid, localization->GetString(LSTR_MORE_INFORMATION)
        );
        v0 = 1;
    }
    pDialogueWindow->CreateButton({480, 30 * v0 + 160}, {140, 30}, 1, 0,
        UIMSG_SelectHouseNPCDialogueOption, DIALOGUE_HIRE_FIRE, Io::InputAction::Invalid, localization->GetString(LSTR_HIRE));
    pDialogueWindow->_41D08F_set_keyboard_control_group(v0 + 1, 1, 0, 2);
    dialog_menu_id = DIALOGUE_OTHER;
}

void handleScriptedNPCTopicSelection(DIALOGUE_TYPE topic, int eventId) {
    if (eventId == 311) {
        // Original code also listed this event which presumably opened bounty dialogue but MM7
        // use event 311 for some teleport in Bracada
        __debugbreak();
        return;
    }

    if (eventId == 139) {
        OracleDialogue();
    } else if (eventId == 399) {
        Arena_SelectionFightLevel();
    } else if (eventId >= 400 && eventId <= 410) {
        _dword_F8B1D8_last_npc_topic_menu = topic;
        DrawJoinGuildWindow((GUILD_ID)(eventId - 400));
    } else if (eventId >= 200 && eventId <= 310) {
        _4B3FE5_training_dialogue(eventId);
    } else {
        activeLevelDecoration = (LevelDecoration *)1;
        current_npc_text.clear();
        eventProcessor(eventId, Pid(), 1);
        activeLevelDecoration = nullptr;
    }
}
