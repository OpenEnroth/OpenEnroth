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

#include "GUI/GUIButton.h"
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
                logger->warning("NPC id exceeds MAX_DATA!");
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
            (pNPCStats->pNPCData[i].uFlags & NPC_HIRED)) {
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
    case Healer:
    case ExpertHealer:
    case MasterHealer:
    case Cook:
    case Chef:
    case WindMaster:
    case WaterMaster:
    case GateMaster:
    case Acolyte:
    case Piper:
    case FallenWizard:
        return pNPCStats->pProfessions[prof].pActionText;
    default:
        return pNPCTopics[407].pTopic;
    }
}

//----- (004BB756) --------------------------------------------------------
int UseNPCSkill(NpcProfession profession, int id) {
    switch (profession) {
        case Healer: {
            for (Character &player : pParty->pCharacters) {
                player.health = player.GetMaxHealth();
                player.playReaction(SPEECH_TEMPLE_HEAL);
            }
            pAudioPlayer->playExclusiveSound(SOUND_heal);
        } break;

        case ExpertHealer: {
            for (Character &player : pParty->pCharacters) {
                player.health = player.GetMaxHealth();
                for (Condition condition : standardConditionsExcludeDead) {
                    player.conditions.Reset(condition);
                }
                player.playReaction(SPEECH_TEMPLE_HEAL);
            }
            pAudioPlayer->playExclusiveSound(SOUND_heal);
        } break;

        case MasterHealer: {
            for (Character &player : pParty->pCharacters) {
                player.health = player.GetMaxHealth();
                for (Condition condition : standardConditionsIncludeDead) {
                    // Master healer heals all except Eradicated and zombie
                    if (condition != CONDITION_ERADICATED) {
                        player.conditions.Reset(condition);
                    }
                }
                player.playReaction(SPEECH_TEMPLE_HEAL);
            }
            pAudioPlayer->playExclusiveSound(SOUND_heal);
        } break;

        case Cook: {
            // Was 13
            if (pParty->GetFood() >= 14) {
                return 1;
            }

            pParty->GiveFood(1);
        } break;

        case Chef: {
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

        case WindMaster: {
            if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                engine->_statusBar->setEvent(LSTR_CANT_FLY_INDOORS);
                pAudioPlayer->playUISound(SOUND_fizzle);
            } else {
                // Spell power was changed to 0 because it does not have meaning for this buff
                pParty->pPartyBuffs[PARTY_BUFF_FLY]
                    .Apply(pParty->GetPlayingTime() + Duration::fromHours(2), CHARACTER_SKILL_MASTERY_MASTER, 0, 0, 0);
                // Mark buff as GM because NPC buff does not drain mana
                pParty->pPartyBuffs[PARTY_BUFF_FLY].isGMBuff = true;
                pAudioPlayer->playSpellSound(SPELL_AIR_FLY, false, SOUND_MODE_UI);
            }
        } break;

        case WaterMaster: {
            pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK]
                .Apply(pParty->GetPlayingTime() + Duration::fromHours(3), CHARACTER_SKILL_MASTERY_MASTER, 0, 0, 0);
            // Mark buff as GM because NPC buff does not drain mana
            pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].isGMBuff = true;
            pAudioPlayer->playSpellSound(SPELL_WATER_WATER_WALK, false, SOUND_MODE_UI);
        } break;

        case GateMaster: {
            engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
            engine->_messageQueue->addMessageNextFrame(UIMSG_OnCastTownPortal, Pid(OBJECT_Character, pParty->pCharacters.size() + id).packed(), 0);
        } break;

        case Acolyte:
            pushNPCSpell(SPELL_SPIRIT_BLESS);
            break;
        case Piper:
            pushNPCSpell(SPELL_SPIRIT_HEROISM);
            break;
        case FallenWizard:
            pushNPCSpell(SPELL_LIGHT_HOUR_OF_POWER);
            break;

        case Teacher:
        case Instructor:
        case Armsmaster:
        case Weaponsmaster:
        case Apprentice:
        case Mystic:
        case Spellmaster:
        case Trader:
        case Merchant:
        case Scout:
        case Herbalist:
        case Apothecary:
        case Tinker:
        case Locksmith:
        case Fool:
        case ChimneySweep:
        case Porter:
        case QuarterMaster:
        case Factor:
        case Banker:
        case Horseman:
        case Bard:
        case Enchanter:
        case Cartographer:
        case Explorer:
        case Pirate:
        case Squire:
        case Psychic:
        case Gypsy:
        case Diplomat:
        case Duper:
        case Burglar:
        case Acolyte2:
        case Initiate:
        case Prelate:
        case Monk:
        case Sage:
        case Hunter:
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
