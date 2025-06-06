#include "NPCTopics.h"

#include <utility>
#include <string>
#include <vector>

#include "Engine/ArenaEnumFunctions.h"
#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Objects/MonsterEnumFunctions.h"
#include "Engine/Party.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Evt/Processor.h"
#include "Engine/Random/Random.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"
#include "Utility/String/Ascii.h"

int membershipOrTrainingApproved;
int topicEventId; // event id of currently viewed scripted NPC event
DialogueId guildMembershipNPCTopicId;

int gold_transaction_amount;

static constexpr std::array<Vec2f, 20> pMonsterArenaPlacements = {{
    Vec2f(1524, 8332),    Vec2f(2186, 8844),
    Vec2f(3219, 9339),    Vec2f(4500, 9339),
    Vec2f(5323, 9004),    Vec2f(0x177D, 0x2098),
    Vec2f(0x50B, 0x1E15), Vec2f(0x18FF, 0x1E15),
    Vec2f(0x50B, 0xD69),  Vec2f(0x18FF, 0x1B15),
    Vec2f(0x50B, 0x1021), Vec2f(0x18FF, 0x1848),
    Vec2f(0x50B, 0x12D7), Vec2f(0x18FF, 0x15A3),
    Vec2f(0x50B, 0x14DB), Vec2f(0x18FF, 0x12D7),
    Vec2f(0x50B, 0x1848), Vec2f(0x18FF, 0x1021),
    Vec2f(0x50B, 0x1B15), Vec2f(0x18FF, 0xD69),
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

static constexpr std::array<std::pair<QuestBit, ItemId>, 27> _4F0882_evt_VAR_PlayerItemInHands_vals = {{
    {QBIT_212, ITEM_QUEST_VASE},
    {QBIT_213, ITEM_SPECIAL_LADY_CARMINES_DAGGER},
    {QBIT_214, ITEM_MESSAGE_SCROLL_OF_WAVES},
    {QBIT_215, ITEM_MESSAGE_CIPHER},
    {QBIT_216, ITEM_QUEST_WORN_BELT},
    {QBIT_217, ITEM_QUEST_HEART_OF_THE_WOOD},
    {QBIT_218, ITEM_MESSAGE_MAP_TO_EVENMORN_ISLAND},
    {QBIT_219, ITEM_QUEST_GOLEM_HEAD},
    {QBIT_220, ITEM_QUEST_ABBEY_NORMAL_GOLEM_HEAD},
    {QBIT_221, ITEM_QUEST_GOLEM_RIGHT_ARM},
    {QBIT_222, ITEM_QUEST_GOLEM_LEFT_ARM},
    {QBIT_223, ITEM_QUEST_GOLEM_RIGHT_LEG},
    {QBIT_224, ITEM_QUEST_GOLEM_LEFT_LEG},
    {QBIT_225, ITEM_QUEST_GOLEM_CHEST},
    {QBIT_226, ITEM_SPELLBOOK_DIVINE_INTERVENTION},
    {QBIT_227, ITEM_QUEST_DRAGON_EGG},
    {QBIT_228, ITEM_QUEST_ZOKARR_IVS_SKULL},
    {QBIT_229, ITEM_QUEST_LICH_JAR_EMPTY},
    {QBIT_230, ITEM_QUEST_ELIXIR},
    {QBIT_231, ITEM_QUEST_CASE_OF_SOUL_JARS},
    {QBIT_232, ITEM_QUEST_ALTAR_PIECE_1},
    {QBIT_233, ITEM_QUEST_ALTAR_PIECE_2},
    {QBIT_234, ITEM_QUEST_CONTROL_CUBE},
    {QBIT_235, ITEM_QUEST_WETSUIT},
    {QBIT_236, ITEM_QUEST_OSCILLATION_OVERTHRUSTER},
    {QBIT_237, ITEM_QUEST_LICH_JAR_FULL},
    {QBIT_241, ITEM_SPECIAL_THE_PERFECT_BOW}
}};

DialogueId arenaMainDialogue() {
    if (pParty->arenaState == ARENA_STATE_INITIAL)
        return DIALOGUE_ARENA_WELCOME;

    if (pParty->arenaState == ARENA_STATE_WON)
        return DIALOGUE_ARENA_ALREADY_WON;

    assert(pParty->arenaState == ARENA_STATE_FIGHTING);

    int killedMonsters = 0;
    for (Actor &actor : pActors) {
        if (actor.aiState == Dead ||
            actor.aiState == Removed ||
            actor.aiState == Disabled ||
            (actor.summonerId && actor.summonerId.type() == OBJECT_Character)) {
            killedMonsters++;
        }
    }

    if (killedMonsters >= pActors.size() || pActors.size() <= 0) {
        pParty->uNumArenaWins[pParty->arenaLevel]++;
        for (Character &player : pParty->pCharacters) {
            player.SetVariable(VAR_Award, awardTypeForArenaLevel(pParty->arenaLevel));
        }
        pParty->partyFindsGold(gold_transaction_amount, GOLD_RECEIVE_SHARE);
        pAudioPlayer->playUISound(SOUND_51heroism03);
        pParty->arenaState = ARENA_STATE_WON;
        pParty->arenaLevel = ARENA_LEVEL_INVALID;
        return DIALOGUE_ARENA_REWARD;
    } else {
        pParty->pos = Vec3f(3849, 5770, 1);
        pParty->velocity = Vec3f();
        pParty->uFallStartZ = 1;
        pParty->_viewYaw = 512;
        pParty->_viewPitch = 0;
        pAudioPlayer->playUISound(SOUND_51heroism03);
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        return DIALOGUE_NULL;
    }
}

/**
 * @offset 0x4BC109
 */
void prepareArenaFight(ArenaLevel level) {
    pParty->arenaState = ARENA_STATE_FIGHTING;
    pParty->arenaLevel = level;

    GUIWindow window = *pDialogueWindow;
    window.uFrameWidth = game_viewport_width;
    window.uFrameZ = 452;
    int textHeight = assets->pFontArrus->CalcTextHeight(localization->GetString(LSTR_PLEASE_WAIT_WHILE_I_SUMMON_THE_MONSTERS), window.uFrameWidth, 13) + 7;

    // TODO(pskelton): This doesnt work properly and we dont want draw calls here
    render->BeginScene3D();
    if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        pIndoor->Draw();
    } else if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        pOutdoor->Draw();
    }
    render->DrawBillboards_And_MaybeRenderSpecialEffects_And_EndScene();
     render->BeginScene2D();
    render->DrawTextureCustomHeight(8 / 640.0f, (352 - textHeight) / 480.0f, ui_leather_mm7, textHeight);
    render->DrawTextureNew(8 / 640.0f, (347 - textHeight) / 480.0f, _591428_endcap);
    std::string text = assets->pFontArrus->FitTextInAWindow(localization->GetString(LSTR_PLEASE_WAIT_WHILE_I_SUMMON_THE_MONSTERS), window.uFrameWidth, 13);
    pDialogueWindow->DrawText(assets->pFontArrus.get(), {13, 354 - textHeight}, colorTable.White, text);
    render->Present();

    pParty->pos = Vec3f(3849, 5770, 1); // TODO(pskelton) :: extract this common teleport to func
    pParty->velocity = Vec3f();
    pParty->uFallStartZ = 1;
    pParty->_viewYaw = 512;
    pParty->_viewPitch = 0;
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);

    int characterMaxLevel = 0;
    for (Character &character : pParty->pCharacters) {
        if (characterMaxLevel < character.GetActualLevel()) {
            characterMaxLevel = character.GetActualLevel();
        }
    }

    int monsterMaxLevel = characterMaxLevel;
    int monsterMinLevel = characterMaxLevel / 2;

    switch(level) {
    case ARENA_LEVEL_PAGE:
        monsterMaxLevel = characterMaxLevel;
        break;
    case ARENA_LEVEL_SQUIRE:
        monsterMaxLevel = characterMaxLevel * 1.5;
        break;
    case ARENA_LEVEL_KNIGHT:
    case ARENA_LEVEL_LORD:
        monsterMaxLevel = characterMaxLevel * 2;
        break;
    default:
        assert(false);
    }

    if (monsterMinLevel < 2)
        monsterMinLevel = 2;
    if (monsterMinLevel > 100)
        monsterMinLevel = 100;

    if (monsterMaxLevel > 100)
        monsterMaxLevel = 100;
    if (monsterMaxLevel < 2)
        monsterMaxLevel = 2;

    std::vector<MonsterId> candidateIds;
    for (MonsterId i : allArenaMonsters()) {
        if (pMonsterStats->infos[i].level >= monsterMinLevel && pMonsterStats->infos[i].level <= monsterMaxLevel) {
            candidateIds.push_back(i);
        }
    }
    assert(!candidateIds.empty());

    int maxIdsNum = 6;
    if (candidateIds.size() < 6) {
        maxIdsNum = candidateIds.size();
    }

    std::vector<MonsterId> monsterIds;
    for (int i = 0; i < maxIdsNum; i++) {
        monsterIds.push_back(grng->randomSample(candidateIds));
    }

    int baseReward = 0, monstersNum = 0;

    if (level == ARENA_LEVEL_PAGE) {
        baseReward = 50;
        monstersNum = grng->random(3) + 6; // [6:8] monsters
    } else if (level == ARENA_LEVEL_SQUIRE) {
        baseReward = 100;
        monstersNum = grng->random(7) + 6; // [6:12] monsters
    } else if (level == ARENA_LEVEL_KNIGHT) {
        baseReward = 200;
        monstersNum = grng->random(11) + 10; // [10:19] monsters
    } else if (level == ARENA_LEVEL_LORD) {
        baseReward = 500;
        monstersNum = 20;
    }

    gold_transaction_amount = characterMaxLevel * baseReward;

    for (int i = 0; i < monstersNum; ++i) {
        Vec2f pos = pMonsterArenaPlacements[i];
        Actor::Arena_summon_actor(grng->randomSample(monsterIds), Vec3f(pos.x, pos.y, 1));
    }
    pAudioPlayer->playUISound(SOUND_51heroism03);
}

/**
 * @offset 0x004B1ECE.
 *
 * @brief Oracle's 'I lost it!' dialog option
 */
void oracleDialogue() {
    Item *item = nullptr;
    ItemId item_id = ITEM_NULL;

    // display "You never had it" if nothing missing will be found
    current_npc_text = pNPCTopics[667].pText;

    // only items with special subquest in range 212-237 and also 241 are recoverable
    for (auto pair : _4F0882_evt_VAR_PlayerItemInHands_vals) {
        QuestBit quest_id = pair.first;
        if (pParty->_questBits[quest_id]) {
            ItemId search_item_id = pair.second;
            if (!pParty->hasItem(search_item_id) && pParty->pPickedItem.itemId != search_item_id) {
                item_id = search_item_id;
                break;
            }
        }
    }

    // missing item found
    if (item_id != ITEM_NULL) {
        pParty->pCharacters[0].AddVariable(VAR_PlayerItemInHands, std::to_underlying(item_id));
        // TODO(captainurist): what if fmt throws?
        current_npc_text = fmt::sprintf(pNPCTopics[666].pText, // "Here's %s that you lost. Be careful" // NOLINT: this is not ::sprintf.
                                        fmt::format("{::}{}\f00000", colorTable.Sunflower.tag(),
                                                    pItemTable->items[item_id].unidentifiedName));
    }

    // missing item is lich jar and we need to bind soul vessel to lich class character
    // TODO(Nik-RE-dev): this code is walking only through inventory, but item was added to hand, so it will not bind new item if it was acquired
    //                   rather this code will bind jars that already present in inventory to liches that currently do not have binded jars
    if (item_id == ITEM_QUEST_LICH_JAR_FULL) {
        for (int i = 0; i < pParty->pCharacters.size(); i++) {
            if (pParty->pCharacters[i].classType == CLASS_LICH) {
                bool have_vessels_soul = false;
                for (Character &player : pParty->pCharacters) {
                    for (int idx = 0; idx < Character::INVENTORY_SLOT_COUNT; idx++) {
                        if (player.pInventoryItemList[idx].itemId == ITEM_QUEST_LICH_JAR_FULL) {
                            if (player.pInventoryItemList[idx].lichJarCharacterIndex == -1) {
                                item = &player.pInventoryItemList[idx];
                            }
                            if (player.pInventoryItemList[idx].lichJarCharacterIndex == i) {
                                have_vessels_soul = true;
                            }
                        }
                    }
                }

                if (item && !have_vessels_soul) {
                    item->lichJarCharacterIndex = i;
                    break;
                }
            }
        }
    }
}

/**
 * @offset 0x4B29F2
 */
const std::string &joinGuildOptionString() {
    GuildId guild_id = static_cast<GuildId>(topicEventId - 400);
    static const int dialogue_base = 110;
    AwardType guildMembershipAwardBit = static_cast<AwardType>(Award_Membership_ElementalGuilds + std::to_underlying(guild_id));

    membershipOrTrainingApproved = false;
    gold_transaction_amount = priceForMembership[guild_id];

    // TODO(pskelton): check this behaviour
    if (!pParty->hasActiveCharacter())
        pParty->setActiveToFirstCanAct();  // avoid nzi

    if (pParty->activeCharacter().CanAct()) {
        if (pParty->activeCharacter()._achievedAwardsBits[guildMembershipAwardBit]) {
            return pNPCTopics[dialogue_base + 13].pText;
        } else {
            if (gold_transaction_amount <= pParty->GetGold()) {
                membershipOrTrainingApproved = true;
                return pNPCTopics[dialogue_base + std::to_underlying(guild_id)].pText;
            } else {
                return pNPCTopics[dialogue_base + 14].pText;
            }
        }
    } else {
        return pNPCTopics[dialogue_base + 12].pText;
    }
}

/**
 * @offset 0x4B254D
 */
std::string masteryTeacherOptionString() {
    int teacherLevel = (topicEventId - 200) % 3;
    CharacterSkillType skillBeingTaught = static_cast<CharacterSkillType>((topicEventId - 200) / 3);
    Character *activePlayer = &pParty->activeCharacter();
    CharacterClass pClassType = activePlayer->classType;
    CharacterSkillMastery currClassMaxMastery = skillMaxMasteryPerClass[pClassType][skillBeingTaught];
    CharacterSkillMastery masteryLevelBeingTaught = static_cast<CharacterSkillMastery>(teacherLevel + 2);

    membershipOrTrainingApproved = false;

    if (currClassMaxMastery < masteryLevelBeingTaught) {
        if (skillMaxMasteryPerClass[getTier2Class(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught) {
            return localization->FormatString(LSTR_YOU_HAVE_TO_BE_PROMOTED_TO_S_TO_LEARN, localization->GetClassName(getTier2Class(pClassType)));
        } else if (skillMaxMasteryPerClass[getTier3LightClass(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught &&
                skillMaxMasteryPerClass[getTier3DarkClass(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught) {
            return localization->FormatString(LSTR_YOU_HAVE_TO_BE_PROMOTED_TO_S_OR_S_TO,
                    localization->GetClassName(getTier3LightClass(pClassType)),
                    localization->GetClassName(getTier3DarkClass(pClassType)));
        } else if (skillMaxMasteryPerClass[getTier3LightClass(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught) {
            return localization->FormatString(LSTR_YOU_HAVE_TO_BE_PROMOTED_TO_S_TO_LEARN, localization->GetClassName(getTier3LightClass(pClassType)));
        } else if (skillMaxMasteryPerClass[getTier3DarkClass(pClassType)][skillBeingTaught] >= masteryLevelBeingTaught) {
            return localization->FormatString(LSTR_YOU_HAVE_TO_BE_PROMOTED_TO_S_TO_LEARN, localization->GetClassName(getTier3DarkClass(pClassType)));
        } else {
            return localization->FormatString(LSTR_THIS_SKILL_LEVEL_CAN_NOT_BE_LEARNED_BY, localization->GetClassName(pClassType));
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

    bool canLearn = true;

    if (masteryLevelBeingTaught == CHARACTER_SKILL_MASTERY_EXPERT) {
        canLearn = skillLevel >= 4;
        gold_transaction_amount = expertSkillMasteryCost[skillBeingTaught];
    }

    if (masteryLevelBeingTaught == CHARACTER_SKILL_MASTERY_MASTER) {
        switch (skillBeingTaught) {
          case CHARACTER_SKILL_LIGHT:
            canLearn = pParty->_questBits[QBIT_114];
            break;
          case CHARACTER_SKILL_DARK:
            canLearn = pParty->_questBits[QBIT_110];
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
        canLearn = canLearn && (skillLevel >= 7) && (skillMastery == CHARACTER_SKILL_MASTERY_EXPERT);
        gold_transaction_amount = masterSkillMasteryCost[skillBeingTaught];
    }

    if (masteryLevelBeingTaught == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
        switch (skillBeingTaught) {
          case CHARACTER_SKILL_LIGHT:
            canLearn = activePlayer->isClass(CLASS_ARCHAMGE) || activePlayer->isClass(CLASS_PRIEST_OF_SUN);
            break;
          case CHARACTER_SKILL_DARK:
            canLearn = activePlayer->isClass(CLASS_LICH) || activePlayer->isClass(CLASS_PRIEST_OF_MOON);
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
        canLearn = canLearn && (skillLevel >= 10) && (skillMastery == CHARACTER_SKILL_MASTERY_MASTER);
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

    membershipOrTrainingApproved = true;

    return localization->FormatString(LSTR_BECOME_S_IN_S_FOR_LU_GOLD, localization->MasteryNameLong(masteryLevelBeingTaught),
                                      localization->GetSkillName(skillBeingTaught), gold_transaction_amount);
}

std::string npcDialogueOptionString(DialogueId topic, NPCData *npcData) {
    switch (topic) {
      case DIALOGUE_SCRIPTED_LINE_1:
        return pNPCTopics[npcData->dialogue_1_evt_id].pTopic;
      case DIALOGUE_SCRIPTED_LINE_2:
        return pNPCTopics[npcData->dialogue_2_evt_id].pTopic;
      case DIALOGUE_SCRIPTED_LINE_3:
        return pNPCTopics[npcData->dialogue_3_evt_id].pTopic;
      case DIALOGUE_SCRIPTED_LINE_4:
        return pNPCTopics[npcData->dialogue_4_evt_id].pTopic;
      case DIALOGUE_SCRIPTED_LINE_5:
        return pNPCTopics[npcData->dialogue_5_evt_id].pTopic;
      case DIALOGUE_SCRIPTED_LINE_6:
        return pNPCTopics[npcData->dialogue_6_evt_id].pTopic;
      case DIALOGUE_HIRE_FIRE:
        if (npcData->Hired()) {
            return localization->FormatString(LSTR_DISMISS_S, npcData->name);
        } else {
            return localization->GetString(LSTR_HIRE);
        }
      case DIALOGUE_13_hiring_related:
        if (npcData->Hired()) {
            return localization->FormatString(LSTR_DISMISS_S, npcData->name);
        } else {
            return localization->GetString(LSTR_JOIN);
        }
      case DIALOGUE_PROFESSION_DETAILS:
        return localization->GetString(LSTR_MORE_INFORMATION);
      case DIALOGUE_MASTERY_TEACHER_LEARN:
        return masteryTeacherOptionString();
      case DIALOGUE_MAGIC_GUILD_JOIN:
        return joinGuildOptionString();
      case DIALOGUE_ARENA_SELECT_LORD:
        return localization->GetString(LSTR_ARENA_DIFFICULTY_LORD);
      case DIALOGUE_ARENA_SELECT_KNIGHT:
        return localization->GetString(LSTR_ARENA_DIFFICULTY_KNIGHT);
      case DIALOGUE_ARENA_SELECT_SQUIRE:
        return localization->GetString(LSTR_ARENA_DIFFICULTY_SQUIRE);
      case DIALOGUE_ARENA_SELECT_PAGE:
        return localization->GetString(LSTR_ARENA_DIFFICULTY_PAGE);
      case DIALOGUE_USE_HIRED_NPC_ABILITY:
        return GetProfessionActionText(npcData->profession);
      default:
        return "";
    }
}

std::vector<DialogueId> prepareScriptedNPCDialogueTopics(NPCData *npcData) {
    std::vector<DialogueId> optionList;

    if (npcData->is_joinable) {
        optionList.push_back(DIALOGUE_13_hiring_related);
    }

    // TODO(Nik-RE-dev): place NPC events in array
#define ADD_NPC_SCRIPTED_DIALOGUE(EVENT_ID, MSG_PARAM) \
    if (EVENT_ID) { \
        if (optionList.size() < 4) { \
            int res = npcDialogueEventProcessor(EVENT_ID); \
            if (res == 1 || res == 2) { \
                optionList.push_back(MSG_PARAM); \
            } \
        } \
    }

    ADD_NPC_SCRIPTED_DIALOGUE(npcData->dialogue_1_evt_id, DIALOGUE_SCRIPTED_LINE_1);
    ADD_NPC_SCRIPTED_DIALOGUE(npcData->dialogue_2_evt_id, DIALOGUE_SCRIPTED_LINE_2);
    ADD_NPC_SCRIPTED_DIALOGUE(npcData->dialogue_3_evt_id, DIALOGUE_SCRIPTED_LINE_3);
    ADD_NPC_SCRIPTED_DIALOGUE(npcData->dialogue_4_evt_id, DIALOGUE_SCRIPTED_LINE_4);
    ADD_NPC_SCRIPTED_DIALOGUE(npcData->dialogue_5_evt_id, DIALOGUE_SCRIPTED_LINE_5);
    ADD_NPC_SCRIPTED_DIALOGUE(npcData->dialogue_6_evt_id, DIALOGUE_SCRIPTED_LINE_6);

#undef ADD_NPC_SCRIPTED_DIALOGUE

    return optionList;
}

DialogueId handleScriptedNPCTopicSelection(DialogueId topic, NPCData *npcData) {
    int eventId;

    if (topic == DIALOGUE_SCRIPTED_LINE_1) {
        eventId = npcData->dialogue_1_evt_id;
    } else if (topic == DIALOGUE_SCRIPTED_LINE_2) {
        eventId = npcData->dialogue_2_evt_id;
    } else if (topic == DIALOGUE_SCRIPTED_LINE_3) {
        eventId = npcData->dialogue_3_evt_id;
    } else if (topic == DIALOGUE_SCRIPTED_LINE_4) {
        eventId = npcData->dialogue_4_evt_id;
    } else if (topic == DIALOGUE_SCRIPTED_LINE_5) {
        eventId = npcData->dialogue_5_evt_id;
    } else {
        assert(topic == DIALOGUE_SCRIPTED_LINE_6);
        eventId = npcData->dialogue_6_evt_id;
    }


    if (eventId == 311) {
        // Original code also listed this event which presumably opened bounty dialogue but MM7
        // use event 311 for some teleport in Bracada
        assert(false);
        return DIALOGUE_MAIN;
    }

    if (eventId == 139) {
        oracleDialogue();
    } else if (eventId == 399) {
        return arenaMainDialogue();
    } else if (eventId >= 400 && eventId <= 410) {
        guildMembershipNPCTopicId = topic;
        current_npc_text = pNPCTopics[eventId - 301].pText;
        topicEventId = eventId;
        return DIALOGUE_MAGIC_GUILD_OFFER;
    } else if (eventId >= 200 && eventId <= 310) {
        current_npc_text = pNPCTopics[eventId + 168].pText;
        topicEventId = eventId;
        return DIALOGUE_MASTERY_TEACHER_OFFER;
    } else {
        activeLevelDecoration = (LevelDecoration *)1;
        current_npc_text.clear();
        eventProcessor(eventId, Pid(), 1);
        activeLevelDecoration = nullptr;
    }

    return DIALOGUE_MAIN;
}

std::vector<DialogueId> listNPCDialogueOptions(DialogueId topic) {
    switch (topic) {
      case DIALOGUE_MAGIC_GUILD_OFFER:
        return {DIALOGUE_MAGIC_GUILD_JOIN};
      case DIALOGUE_MASTERY_TEACHER_OFFER:
        return {DIALOGUE_MASTERY_TEACHER_LEARN};
      case DIALOGUE_ARENA_WELCOME:
        return {DIALOGUE_ARENA_SELECT_PAGE, DIALOGUE_ARENA_SELECT_SQUIRE, DIALOGUE_ARENA_SELECT_KNIGHT, DIALOGUE_ARENA_SELECT_LORD};
      default:
        return {};
    }
}


void selectSpecialNPCTopicSelection(DialogueId topic, NPCData* npcData) {
    if (topic == DIALOGUE_MASTERY_TEACHER_LEARN) {
        if (membershipOrTrainingApproved) {
            if (pParty->hasActiveCharacter()) {
                uint8_t teacherLevel = (topicEventId - 200) % 3;
                CharacterSkillType skillBeingTaught = static_cast<CharacterSkillType>((topicEventId - 200) / 3);
                CharacterSkillMastery newMastery = static_cast<CharacterSkillMastery>(teacherLevel + 2);
                CombinedSkillValue skillValue = CombinedSkillValue::increaseMastery(pParty->activeCharacter().getSkillValue(skillBeingTaught), newMastery);
                pParty->activeCharacter().setSkillValue(skillBeingTaught, skillValue);
                pParty->activeCharacter().playReaction(SPEECH_SKILL_MASTERY_INC);
                pParty->TakeGold(gold_transaction_amount);
                engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
            }
        }
    } else if (topic == DIALOGUE_MAGIC_GUILD_JOIN) {
        if (membershipOrTrainingApproved) {
            AwardType guildMembershipAwardBit = static_cast<AwardType>(Award_Membership_ElementalGuilds + topicEventId - 400);
            pParty->TakeGold(gold_transaction_amount, true);
            for (Character &player : pParty->pCharacters) {
                player.SetVariable(VAR_Award, guildMembershipAwardBit);
            }

            switch (guildMembershipNPCTopicId) {
              case DIALOGUE_SCRIPTED_LINE_1:
                if (npcData->dialogue_1_evt_id >= 400 && npcData->dialogue_1_evt_id <= 416)
                    npcData->dialogue_1_evt_id = 0;
                break;
              case DIALOGUE_SCRIPTED_LINE_2:
                if (npcData->dialogue_2_evt_id >= 400 && npcData->dialogue_2_evt_id <= 416)
                    npcData->dialogue_2_evt_id = 0;
                break;
              case DIALOGUE_SCRIPTED_LINE_3:
                if (npcData->dialogue_3_evt_id >= 400 && npcData->dialogue_3_evt_id <= 416)
                    npcData->dialogue_3_evt_id = 0;
                break;
              case DIALOGUE_SCRIPTED_LINE_4:
                if (npcData->dialogue_4_evt_id >= 400 && npcData->dialogue_4_evt_id <= 416)
                    npcData->dialogue_4_evt_id = 0;
                break;
              case DIALOGUE_SCRIPTED_LINE_5:
                if (npcData->dialogue_5_evt_id >= 400 && npcData->dialogue_5_evt_id <= 416)
                    npcData->dialogue_5_evt_id = 0;
                break;
              case DIALOGUE_SCRIPTED_LINE_6:
                if (npcData->dialogue_6_evt_id >= 400 && npcData->dialogue_6_evt_id <= 416)
                    npcData->dialogue_6_evt_id = 0;
                break;
              default:
                break;
            }

            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
            if (pParty->hasActiveCharacter()) {
                pParty->activeCharacter().playReaction(SPEECH_JOINED_GUILD);
            }
        }
    } else if (topic == DIALOGUE_PROFESSION_DETAILS) {
        dialogue_show_profession_details = ~dialogue_show_profession_details;
    } else if (topic >= DIALOGUE_ARENA_SELECT_PAGE && topic <= DIALOGUE_ARENA_SELECT_LORD) {
        prepareArenaFight(arenaLevelForDialogue(topic));
    } else if (topic == DIALOGUE_USE_HIRED_NPC_ABILITY) {
        int hirelingId;
        for (hirelingId = 0; hirelingId < pParty->pHirelings.size(); hirelingId++) {
            if (ascii::noCaseEquals(pParty->pHirelings[hirelingId].name, npcData->name)) { // TODO(captainurist): #unicode
                break;
            }
        }
        assert(hirelingId < pParty->pHirelings.size());
        if (UseNPCSkill(npcData->profession, hirelingId) == 0) {
            if (npcData->profession != GateMaster) {
                npcData->bHasUsedTheAbility = 1;
            }
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        } else {
            engine->_statusBar->setEvent(LSTR_YOUR_PACKS_ARE_ALREADY_FULL);
        }
    } else if (topic == DIALOGUE_HIRE_FIRE) {
        if (npcData->Hired()) {
            if (pNPCStats->uNumNewNPCs > 0) {
                for (int i = 0; i < pNPCStats->uNumNewNPCs; ++i) {
                    if (pNPCStats->pNPCData[i].Hired() && npcData->name == pNPCStats->pNPCData[i].name) {
                        pNPCStats->pNPCData[i].uFlags &= ~NPC_HIRED;
                    }
                }
            }
            if (ascii::noCaseEquals(pParty->pHirelings[0].name, npcData->name)) { // TODO(captainurist): #unicode
                pParty->pHirelings[0] = NPCData();
            } else if (ascii::noCaseEquals(pParty->pHirelings[1].name, npcData->name)) { // TODO(captainurist): #unicode
                pParty->pHirelings[1] = NPCData();
            }
            pParty->hirelingScrollPosition = 0;
            pParty->CountHirelings();
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
            return;
        }
        if (!pParty->pHirelings[0].name.empty() && !pParty->pHirelings[1].name.empty()) {
            engine->_statusBar->setEvent(LSTR_I_CANNOT_JOIN_YOU_YOURE_PARTY_IS_FULL);
        } else {
            if (npcData->profession != Burglar) {
                // burglars have no hiring price
                if (pParty->GetGold() < pNPCStats->pProfessions[npcData->profession].uHirePrice) {
                    engine->_statusBar->setEvent(LSTR_YOU_DONT_HAVE_ENOUGH_GOLD);
                    dialogue_show_profession_details = false;
                    //uDialogueType = DIALOGUE_13_hiring_related;
                    if (pParty->hasActiveCharacter()) {
                        pParty->activeCharacter().playReaction(SPEECH_NOT_ENOUGH_GOLD);
                    }
                    return;
                }
                pParty->TakeGold(pNPCStats->pProfessions[npcData->profession].uHirePrice);
            }
            npcData->uFlags |= NPC_HIRED;
            if (!pParty->pHirelings[0].name.empty()) {
                pParty->pHirelings[1] = *npcData;
                pParty->pHireling2Name = npcData->name;
            } else {
                pParty->pHirelings[0] = *npcData;
                pParty->pHireling1Name = npcData->name;
            }
            pParty->hirelingScrollPosition = 0;
            pParty->CountHirelings();
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
            if (pParty->hasActiveCharacter()) {
                pParty->activeCharacter().playReaction(SPEECH_HIRE_NPC);
            }
        }
    }
}
