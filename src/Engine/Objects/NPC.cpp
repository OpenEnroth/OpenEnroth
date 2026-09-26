#include "Engine/Objects/NPC.h"

#include <string>

#include "Engine/Engine.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Party.h"
#include "Engine/MapEnumFunctions.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Tables/NPCTable.h"

#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIStatusBar.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Logger/Logger.h"

bool CheckPortretAgainstSex(int portret_num, int sex);

// All conditions for alive character excluding zombie
static const Segment<Condition> standardConditionsExcludeDead = {CONDITION_CURSED, CONDITION_UNCONSCIOUS};

// All conditions including dead character ones, but still excluding zombie
static const Segment<Condition> standardConditionsIncludeDead = {CONDITION_CURSED, CONDITION_ERADICATED};

NPCData *getNPCData(int npcId) {
    if (npcId >= 0) {
        if (npcId < 5000) {
            if (npcId >= 501) {
                MM_WARNING("NPC id exceeds MAX_DATA!");
            }
            return &pNPCStats->pNPCData[npcId];
        } else {
            return &pNPCStats->pAdditionalNPC[npcId - 5000];
        }
    } else {
        FlatHirelings buf;
        buf.Prepare();

        return buf.Get(std::abs(npcId) - 1);
    }
}

//----- (00476387) --------------------------------------------------------
bool PartyHasDragon() { return pNPCStats->pNPCData[57].Hired(); }

//----- (00476395) --------------------------------------------------------
// 0x26 Wizard eye at skill level 2
bool CheckHiredNPCSpeciality(NpcProfession prof) {
    if (isHirelingsBlockedOnMap(engine->_currentLoadedMapId))
        return false;

    for (unsigned i = 0; i < pNPCStats->uNumNewNPCs; ++i) {
        if (pNPCStats->pNPCData[i].profession == prof &&
            (pNPCStats->pNPCData[i].flags & NPC_HIRED)) {
            return true;
        }
    }
    return pParty->pHirelings[0].profession == prof
        || pParty->pHirelings[1].profession == prof;
}

NpcType getNPCType(int npcId) {
    if (npcId >= 0) {
        if (npcId < 5000) {
            return NPC_TYPE_QUEST;
        }
        return NPC_TYPE_HIREABLE;
    }

    FlatHirelings buf;
    buf.Prepare();

    return buf.IsFollower(std::abs(npcId) - 1) ? NPC_TYPE_QUEST : NPC_TYPE_HIREABLE;
}

//----- (00445308) --------------------------------------------------------
const std::string &GetProfessionActionText(NpcProfession prof) {
    switch (prof) {
    case NPC_PROFESSION_HEALER:
    case NPC_PROFESSION_EXPERT_HEALER:
    case NPC_PROFESSION_MASTER_HEALER:
    case NPC_PROFESSION_COOK:
    case NPC_PROFESSION_CHEF:
    case NPC_PROFESSION_WIND_MASTER:
    case NPC_PROFESSION_WATER_MASTER:
    case NPC_PROFESSION_GATE_MASTER:
    case NPC_PROFESSION_CHAPLAIN:
    case NPC_PROFESSION_PIPER:
    case NPC_PROFESSION_FALLEN_WIZARD:
        return pNPCStats->pProfessions[prof].pActionText;
    default:
        // TODO(captainurist): This looks broken.
        // pNPCTopics[407].pTopic = "Mind Guild Membership"
        // pNPCTopics[407].pText = "With Expert Air Magic you can learn all of the Expert spells for this element...."
        // Double broken!
        return pNPCTopics[407].pTopic;
    }
}

//----- (004BB756) --------------------------------------------------------
int UseNPCSkill(NpcProfession profession, int id) {
    switch (profession) {
        case NPC_PROFESSION_HEALER: {
            for (Character &player : pParty->pCharacters) {
                player.health = player.GetMaxHealth();
                player.playReaction(SPEECH_TEMPLE_HEAL);
            }
            pAudioPlayer->playExclusiveSound(SOUND_heal);
        } break;

        case NPC_PROFESSION_EXPERT_HEALER: {
            for (Character &player : pParty->pCharacters) {
                player.health = player.GetMaxHealth();
                for (Condition condition : standardConditionsExcludeDead) {
                    player.conditions.reset(condition);
                }
                player.playReaction(SPEECH_TEMPLE_HEAL);
            }
            pAudioPlayer->playExclusiveSound(SOUND_heal);
        } break;

        case NPC_PROFESSION_MASTER_HEALER: {
            for (Character &player : pParty->pCharacters) {
                player.health = player.GetMaxHealth();
                for (Condition condition : standardConditionsIncludeDead) {
                    // Master healer heals all except Eradicated and zombie
                    if (condition != CONDITION_ERADICATED) {
                        player.conditions.reset(condition);
                    }
                }
                player.playReaction(SPEECH_TEMPLE_HEAL);
            }
            pAudioPlayer->playExclusiveSound(SOUND_heal);
        } break;

        case NPC_PROFESSION_COOK: {
            // Was 13
            if (pParty->GetFood() >= 14) {
                return 1;
            }

            pParty->GiveFood(1);
        } break;

        case NPC_PROFESSION_CHEF: {
            // Was 13
            if (pParty->GetFood() >= 14) {
                return 1;
            }

            if (pParty->GetFood() == 13) {
                pParty->GiveFood(1);
            } else {
                pParty->GiveFood(2);
            }
        } break;

        case NPC_PROFESSION_WIND_MASTER: {
            if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                engine->_statusBar->setEvent(LSTR_CAN_NOT_CAST_FLY_INDOORS);
                pAudioPlayer->playUISound(SOUND_fizzle);
            } else {
                // Spell power was changed to 0 because it does not have meaning for this buff
                pParty->pPartyBuffs[PARTY_BUFF_FLY]
                    .Apply(pParty->GetPlayingTime() + Duration::fromHours(2), MASTERY_MASTER, 0, 0, -1);
                // Mark buff as GM because NPC buff does not drain mana
                pParty->pPartyBuffs[PARTY_BUFF_FLY].isGM = true;
                pAudioPlayer->playSpellSound(SPELL_AIR_FLY, false, SOUND_MODE_UI);
            }
        } break;

        case NPC_PROFESSION_WATER_MASTER: {
            pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK]
                .Apply(pParty->GetPlayingTime() + Duration::fromHours(3), MASTERY_MASTER, 0, 0, -1);
            // Mark buff as GM because NPC buff does not drain mana
            pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].isGM = true;
            pAudioPlayer->playSpellSound(SPELL_WATER_WATER_WALK, false, SOUND_MODE_UI);
        } break;

        case NPC_PROFESSION_GATE_MASTER: {
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
            engine->_messageQueue->addMessageNextFrame(UIMSG_OnCastTownPortal, Pid(OBJECT_Character, pParty->pCharacters.size() + id).packed(), 0);
        } break;

        case NPC_PROFESSION_CHAPLAIN:
            pushNPCSpell(SPELL_SPIRIT_BLESS);
            break;
        case NPC_PROFESSION_PIPER:
            pushNPCSpell(SPELL_SPIRIT_HEROISM);
            break;
        case NPC_PROFESSION_FALLEN_WIZARD:
            pushNPCSpell(SPELL_LIGHT_HOUR_OF_POWER);
            break;

        case NPC_PROFESSION_TEACHER:
        case NPC_PROFESSION_INSTRUCTOR:
        case NPC_PROFESSION_ARMS_MASTER:
        case NPC_PROFESSION_WEAPONS_MASTER:
        case NPC_PROFESSION_APPRENTICE:
        case NPC_PROFESSION_MYSTIC:
        case NPC_PROFESSION_SPELL_MASTER:
        case NPC_PROFESSION_TRADER:
        case NPC_PROFESSION_MERCHANT:
        case NPC_PROFESSION_SCOUT:
        case NPC_PROFESSION_HERBALIST:
        case NPC_PROFESSION_APOTHECARY:
        case NPC_PROFESSION_TINKER:
        case NPC_PROFESSION_LOCKSMITH:
        case NPC_PROFESSION_FOOL:
        case NPC_PROFESSION_CHIMNEY_SWEEP:
        case NPC_PROFESSION_PORTER:
        case NPC_PROFESSION_QUARTER_MASTER:
        case NPC_PROFESSION_FACTOR:
        case NPC_PROFESSION_BANKER:
        case NPC_PROFESSION_HORSEMAN:
        case NPC_PROFESSION_BARD:
        case NPC_PROFESSION_ENCHANTER:
        case NPC_PROFESSION_CARTOGRAPHER:
        case NPC_PROFESSION_EXPLORER:
        case NPC_PROFESSION_PIRATE:
        case NPC_PROFESSION_SQUIRE:
        case NPC_PROFESSION_PSYCHIC:
        case NPC_PROFESSION_GYPSY:
        case NPC_PROFESSION_DIPLOMAT:
        case NPC_PROFESSION_DUPER:
        case NPC_PROFESSION_BURGLAR:
        case NPC_PROFESSION_ACOLYTE:
        case NPC_PROFESSION_INITIATE:
        case NPC_PROFESSION_PRELATE:
        case NPC_PROFESSION_MONK:
        case NPC_PROFESSION_SAGE:
        case NPC_PROFESSION_HUNTER:
            break;

        default:
            assert(false && "Invalid enum value");
    }
    return 0;
}

void FlatHirelings::Prepare() {
    count = 0;

    for (size_t i = 0; i < 2; ++i)
        if (!pParty->pHirelings[i].name.empty())
            ids[count++] = i;

    for (size_t i = 0; i < pNPCStats->uNumNewNPCs; ++i) {
        NPCData *npc = &pNPCStats->pNPCData[i];
        if (npc->Hired()) {
            assert(!npc->name.empty()); // Important for the checks below.

            if (npc->name != pParty->pHirelings[0].name && npc->name != pParty->pHirelings[1].name) {
                assert(i + 2 < 256); // Won't fit into uint8_t otherwise.
                ids[count++] = i + 2;
            }
        }
    }
}

bool FlatHirelings::IsFollower(size_t index) const {
    assert(index < count);

    return ids[index] >= 2;
}

NPCData *FlatHirelings::Get(size_t index) const {
    assert(index < count);

    uint8_t id = ids[index];

    if (id < 2)
        return &pParty->pHirelings[id];
    else
        return &pNPCStats->pNPCData[id - 2];
}

NPCSacrificeStatus *FlatHirelings::GetSacrificeStatus(size_t index) const {
    assert(index < count);

    uint8_t id = ids[index];
    if (id < 2)
        return &pParty->pHirelingsSacrifice[id];
    else
        return nullptr;
}

void setNPCNamesOnLoad() {
    for (unsigned int i = 1; i < pNPCStats->uNumNewNPCs; ++i)
        pNPCStats->pNPCData[i].name = pNPCStats->pNPCUnicNames[i];

    if (!pParty->pHirelings[0].name.empty())
        pParty->pHirelings[0].name = pParty->pHireling1Name;
    if (!pParty->pHirelings[1].name.empty())
        pParty->pHirelings[1].name = pParty->pHireling2Name;
}
