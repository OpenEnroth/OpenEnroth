#include "Localization.h"

#include <cstring>
#include <string>
#include <vector>

#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Engine.h"
#include "Engine/GameResourceManager.h"

#include "Utility/String/Transformations.h"
#include "Utility/String/Split.h"

Localization *localization = nullptr;

const std::string &Localization::GetString(LstrId index) const {
    return this->localization_strings[index];
}

std::string Localization::SkillValueShortString(CombinedSkillValue skillValue) const {
    if (skillValue.mastery() == CHARACTER_SKILL_MASTERY_NONE)
        return {};

    return fmt::sprintf(this->skill_value_short_templates[skillValue.mastery()], skillValue.level()); // NOLINT: not std::sprintf.
}

//----- (00452C49) --------------------------------------------------------
bool Localization::Initialize() {
    char *tmp_pos;     // eax@3
    int step;          // ebp@4
    unsigned char c;   // dl@5
    int temp_str_len;  // ecx@5
    bool string_end;   // [sp+14h] [bp-4h]@4

    this->localization_raw = engine->_gameResourceManager->getEventsFile("global.txt").string_view();
    if (this->localization_raw.empty()) {
        return false;
    }

    strtok(this->localization_raw.data(), "\r");
    strtok(NULL, "\r");

    for (LstrId i : Segment(LSTR_FIRST_MM7, LSTR_LAST_MM7)) {
        char *test_string = strtok(NULL, "\r") + 1;
        step = 0;
        string_end = false;
        do {
            c = *(unsigned char *)test_string;
            temp_str_len = 0;
            if (c != '\t') {
                do {
                    if (!c) break;
                    c = *(test_string + temp_str_len + 1);
                    temp_str_len++;
                } while (c != '\t');
            }
            tmp_pos = test_string + temp_str_len;
            if (*tmp_pos == 0) string_end = true;

            *tmp_pos = 0;
            if (temp_str_len == 0) {
                string_end = true;
            } else {
                if (step == 1)
                    this->localization_strings[i] = removeQuotes(test_string);
            }
            ++step;
            test_string = tmp_pos + 1;
        } while (step <= 2 && !string_end);
    }

    // TODO(captainurist): should be moved to localization files eventually
    this->localization_strings[LSTR_FMT_S_STOLE_D_ITEM] = "%s stole %s!";
    this->localization_strings[LSTR_FMT_RECOVERY_TIME_D] = "Recovery time: %d";
    this->localization_strings[LSTR_FMT_S_U_OUT_OF_U] = "%s: %lu out of %lu";
    this->localization_strings[LSTR_NOBODY_IS_IN_CONDITION] = "Nobody is in a condition to do anything!";
    this->localization_strings[LSTR_KEY_CONFLICT] = "Please resolve all key conflicts!";
    this->localization_strings[LSTR_RECOVERY_TIME_NA] = "Recovery time: N/A";
    this->localization_strings[LSTR_WAND_ALREADY_CHARGED] = "Wand already charged!";
    this->localization_strings[LSTR_ENERGY] = "Energy";
    this->localization_strings[LSTR_IMMOLATION_DAMAGE] = "Immolation deals %d damage to %d target(s)";
    this->localization_strings[LSTR_REMAINING_POWER] = "Remaining power: %d";
    this->localization_strings[LSTR_PLAYER_IS_NOT_ACTIVE] = "That player is not active";

    this->special_attack_names[SPECIAL_ATTACK_CURSE] = "Curse";
    this->special_attack_names[SPECIAL_ATTACK_WEAK] = "Weaken";
    this->special_attack_names[SPECIAL_ATTACK_SLEEP] = "Sleep";
    this->special_attack_names[SPECIAL_ATTACK_DRUNK] = "Drunk"; // "Inebriate"?
    this->special_attack_names[SPECIAL_ATTACK_INSANE] = "Insanity";
    this->special_attack_names[SPECIAL_ATTACK_POISON_WEAK] = "Poison";
    this->special_attack_names[SPECIAL_ATTACK_POISON_MEDIUM] = "Poison";
    this->special_attack_names[SPECIAL_ATTACK_POISON_SEVERE] = "Poison";
    this->special_attack_names[SPECIAL_ATTACK_DISEASE_WEAK] = "Disease";
    this->special_attack_names[SPECIAL_ATTACK_DISEASE_MEDIUM] = "Disease";
    this->special_attack_names[SPECIAL_ATTACK_DISEASE_SEVERE] = "Disease";
    this->special_attack_names[SPECIAL_ATTACK_PARALYZED] = "Paralyze";
    this->special_attack_names[SPECIAL_ATTACK_UNCONSCIOUS] = "Knockout";
    this->special_attack_names[SPECIAL_ATTACK_DEAD] = "Death";
    this->special_attack_names[SPECIAL_ATTACK_PETRIFIED] = "Petrify";
    this->special_attack_names[SPECIAL_ATTACK_ERADICATED] = "Eradicate";
    this->special_attack_names[SPECIAL_ATTACK_BREAK_ANY] = "Break Item";
    this->special_attack_names[SPECIAL_ATTACK_BREAK_ARMOR] = "Break Armor";
    this->special_attack_names[SPECIAL_ATTACK_BREAK_WEAPON] = "Break Weapon";
    this->special_attack_names[SPECIAL_ATTACK_STEAL] = "Steal";
    this->special_attack_names[SPECIAL_ATTACK_AGING] = "Aging";
    this->special_attack_names[SPECIAL_ATTACK_MANA_DRAIN] = "SP Drain";
    this->special_attack_names[SPECIAL_ATTACK_FEAR] = "Fear";

    this->monster_special_ability_names[MONSTER_SPECIAL_ABILITY_SHOT] = "Multi-shot";
    this->monster_special_ability_names[MONSTER_SPECIAL_ABILITY_SUMMON] = "Summoner";
    this->monster_special_ability_names[MONSTER_SPECIAL_ABILITY_EXPLODE] = "Explodes";

    this->skill_value_short_templates[CHARACTER_SKILL_MASTERY_NOVICE] = "%d";
    this->skill_value_short_templates[CHARACTER_SKILL_MASTERY_EXPERT] = "%dE";
    this->skill_value_short_templates[CHARACTER_SKILL_MASTERY_MASTER] = "%dM";
    this->skill_value_short_templates[CHARACTER_SKILL_MASTERY_GRANDMASTER] = "%dG";

    InitializeMm6ItemCategories();

    InitializeMonthNames();
    InitializeDayNames();
    InitializeMoonPhaseNames();

    InitializeClassNames();
    InitializeAttributeNames();
    InitializeSkillNames();
    InitializeCharacterConditionNames();

    InitializeSpellSchoolNames();
    InitializeSpellNames();

    InitializeNpcProfessionNames();

    return true;
}

void Localization::InitializeSpellNames() {
    this->party_buff_names[PARTY_BUFF_RESIST_AIR]               = this->localization_strings[LSTR_AIR_RES];
    this->party_buff_names[PARTY_BUFF_RESIST_BODY]              = this->localization_strings[LSTR_BODY_RES];
    this->party_buff_names[PARTY_BUFF_DAY_OF_GODS]              = this->localization_strings[LSTR_DAY_OF_THE_GODS];
    this->party_buff_names[PARTY_BUFF_DETECT_LIFE]              = this->localization_strings[LSTR_DETECT_LIFE];
    this->party_buff_names[PARTY_BUFF_RESIST_EARTH]             = this->localization_strings[LSTR_EARTH_RES];
    this->party_buff_names[PARTY_BUFF_FEATHER_FALL]             = this->localization_strings[LSTR_FEATHER_FALL];
    this->party_buff_names[PARTY_BUFF_RESIST_FIRE]              = this->localization_strings[LSTR_FIRE_RES];
    this->party_buff_names[PARTY_BUFF_FLY]                      = this->localization_strings[LSTR_FLY];
    this->party_buff_names[PARTY_BUFF_HASTE]                    = this->localization_strings[LSTR_HASTE];
    this->party_buff_names[PARTY_BUFF_HEROISM]                  = this->localization_strings[LSTR_HEROISM];
    this->party_buff_names[PARTY_BUFF_IMMOLATION]               = this->localization_strings[LSTR_IMMOLATION];
    this->party_buff_names[PARTY_BUFF_INVISIBILITY]             = this->localization_strings[LSTR_INVISIBILITY];
    this->party_buff_names[PARTY_BUFF_RESIST_MIND]              = this->localization_strings[LSTR_MIND_RES];
    this->party_buff_names[PARTY_BUFF_PROTECTION_FROM_MAGIC]    = this->localization_strings[LSTR_PROT_MAGIC];
    this->party_buff_names[PARTY_BUFF_SHIELD]                   = this->localization_strings[LSTR_SHIELD];
    this->party_buff_names[PARTY_BUFF_STONE_SKIN]               = this->localization_strings[LSTR_STONESKIN];
    this->party_buff_names[PARTY_BUFF_TORCHLIGHT]               = this->localization_strings[LSTR_TORCH_LIGHT];
    this->party_buff_names[PARTY_BUFF_RESIST_WATER]             = this->localization_strings[LSTR_WATER_RES];
    this->party_buff_names[PARTY_BUFF_WATER_WALK]               = this->localization_strings[LSTR_WATER_WALK];
    this->party_buff_names[PARTY_BUFF_WIZARD_EYE]               = this->localization_strings[LSTR_WIZARD_EYE];

    this->character_buff_names[CHARACTER_BUFF_RESIST_AIR]       = this->localization_strings[LSTR_AIR_RES];
    this->character_buff_names[CHARACTER_BUFF_BLESS]            = this->localization_strings[LSTR_BLESS];
    this->character_buff_names[CHARACTER_BUFF_RESIST_BODY]      = this->localization_strings[LSTR_BODY_RES];
    this->character_buff_names[CHARACTER_BUFF_RESIST_EARTH]     = this->localization_strings[LSTR_EARTH_RES];
    this->character_buff_names[CHARACTER_BUFF_FATE]             = this->localization_strings[LSTR_FATE];
    this->character_buff_names[CHARACTER_BUFF_RESIST_FIRE]      = this->localization_strings[LSTR_FIRE_RES];
    this->character_buff_names[CHARACTER_BUFF_HAMMERHANDS]      = this->localization_strings[LSTR_HAMMERHANDS];
    this->character_buff_names[CHARACTER_BUFF_HASTE]            = this->localization_strings[LSTR_HASTE];
    this->character_buff_names[CHARACTER_BUFF_HEROISM]          = this->localization_strings[LSTR_HEROISM];
    this->character_buff_names[CHARACTER_BUFF_RESIST_MIND]      = this->localization_strings[LSTR_MIND_RES];
    this->character_buff_names[CHARACTER_BUFF_PAIN_REFLECTION]  = this->localization_strings[LSTR_PAIN_REFLECTION];
    this->character_buff_names[CHARACTER_BUFF_PRESERVATION]     = this->localization_strings[LSTR_PRESERVATION];
    this->character_buff_names[CHARACTER_BUFF_REGENERATION]     = this->localization_strings[LSTR_REGENERATION];
    this->character_buff_names[CHARACTER_BUFF_SHIELD]           = this->localization_strings[LSTR_SHIELD];
    this->character_buff_names[CHARACTER_BUFF_STONESKIN]        = this->localization_strings[LSTR_STONESKIN];
    this->character_buff_names[CHARACTER_BUFF_ACCURACY]         = this->localization_strings[LSTR_TEMP_ACCURACY];
    this->character_buff_names[CHARACTER_BUFF_ENDURANCE]        = this->localization_strings[LSTR_TEMP_ENDURANCE];
    this->character_buff_names[CHARACTER_BUFF_INTELLIGENCE]     = this->localization_strings[LSTR_TEMP_INTELLIGENCE];
    this->character_buff_names[CHARACTER_BUFF_LUCK]             = this->localization_strings[LSTR_TEMP_LUCK];
    this->character_buff_names[CHARACTER_BUFF_STRENGTH]         = this->localization_strings[LSTR_TEMP_MIGHT];
    this->character_buff_names[CHARACTER_BUFF_PERSONALITY]      = this->localization_strings[LSTR_TEMP_PERSONALITY];
    this->character_buff_names[CHARACTER_BUFF_SPEED]            = this->localization_strings[LSTR_TEMP_SPEED];
    this->character_buff_names[CHARACTER_BUFF_RESIST_WATER]     = this->localization_strings[LSTR_WATER_RES];
    this->character_buff_names[CHARACTER_BUFF_WATER_WALK]       = this->localization_strings[LSTR_WATER_BREATHING];

    this->actor_buff_names[ACTOR_BUFF_CHARM]                    = this->localization_strings[LSTR_CHARMED];
    this->actor_buff_names[ACTOR_BUFF_SUMMONED]                 = this->localization_strings[LSTR_SUMMONED];
    this->actor_buff_names[ACTOR_BUFF_SHRINK]                   = this->localization_strings[LSTR_SHRUNK];
    this->actor_buff_names[ACTOR_BUFF_AFRAID]                   = this->localization_strings[LSTR_AFRAID];
    this->actor_buff_names[ACTOR_BUFF_STONED]                   = this->localization_strings[LSTR_STONED];
    this->actor_buff_names[ACTOR_BUFF_PARALYZED]                = this->localization_strings[LSTR_PARALYZED];
    this->actor_buff_names[ACTOR_BUFF_SLOWED]                   = this->localization_strings[LSTR_SLOWED];
    this->actor_buff_names[ACTOR_BUFF_BERSERK]                  = this->localization_strings[LSTR_BERSERK];
    this->actor_buff_names[ACTOR_BUFF_SOMETHING_THAT_HALVES_AC] = {};
    this->actor_buff_names[ACTOR_BUFF_MASS_DISTORTION]          = {};
    this->actor_buff_names[ACTOR_BUFF_FATE]                     = this->localization_strings[LSTR_FATE];
    this->actor_buff_names[ACTOR_BUFF_ENSLAVED]                 = this->localization_strings[LSTR_ENSLAVED];
    this->actor_buff_names[ACTOR_BUFF_DAY_OF_PROTECTION]        = this->localization_strings[LSTR_DAY_OF_PROTECTION];
    this->actor_buff_names[ACTOR_BUFF_HOUR_OF_POWER]            = this->localization_strings[LSTR_HOUR_OF_POWER];
    this->actor_buff_names[ACTOR_BUFF_SHIELD]                   = this->localization_strings[LSTR_SHIELD];
    this->actor_buff_names[ACTOR_BUFF_STONESKIN]                = this->localization_strings[LSTR_STONESKIN];
    this->actor_buff_names[ACTOR_BUFF_BLESS]                    = this->localization_strings[LSTR_BLESS];
    this->actor_buff_names[ACTOR_BUFF_HEROISM]                  = this->localization_strings[LSTR_HEROISM];
    this->actor_buff_names[ACTOR_BUFF_HASTE]                    = this->localization_strings[LSTR_HASTE];
    this->actor_buff_names[ACTOR_BUFF_PAIN_REFLECTION]          = this->localization_strings[LSTR_PAIN_REFLECTION];
    this->actor_buff_names[ACTOR_BUFF_HAMMERHANDS]              = this->localization_strings[LSTR_HAMMERHANDS];
}

void Localization::InitializeNpcProfessionNames() {
    this->npc_profession_names[NoProfession]    = this->localization_strings[LSTR_NONE];
    this->npc_profession_names[Smith]           = this->localization_strings[LSTR_SMITH];
    this->npc_profession_names[Armorer]         = this->localization_strings[LSTR_ARMORER];
    this->npc_profession_names[Alchemist]       = this->localization_strings[LSTR_ALCHEMIST];
    this->npc_profession_names[Scholar]         = this->localization_strings[LSTR_SCHOLAR];
    this->npc_profession_names[Guide]           = this->localization_strings[LSTR_GUIDE];
    this->npc_profession_names[Tracker]         = this->localization_strings[LSTR_TRACKER];
    this->npc_profession_names[Pathfinder]      = this->localization_strings[LSTR_PATHFINDER];
    this->npc_profession_names[Sailor]          = this->localization_strings[LSTR_SAILOR];
    this->npc_profession_names[Navigator]       = this->localization_strings[LSTR_NAVIGATOR];
    this->npc_profession_names[Healer]          = this->localization_strings[LSTR_HEALER];
    this->npc_profession_names[ExpertHealer]    = this->localization_strings[LSTR_EXPERT_HEALER];
    this->npc_profession_names[MasterHealer]    = this->localization_strings[LSTR_MASTER_HEALER];
    this->npc_profession_names[Teacher]         = this->localization_strings[LSTR_TEACHER];
    this->npc_profession_names[Instructor]      = this->localization_strings[LSTR_INSTRUCTOR];
    this->npc_profession_names[Armsmaster]      = this->localization_strings[LSTR_ARMS_MASTER];
    this->npc_profession_names[Weaponsmaster]   = this->localization_strings[LSTR_WEAPONS_MASTER];
    this->npc_profession_names[Apprentice]      = this->localization_strings[LSTR_APPRENTICE];
    this->npc_profession_names[Mystic]          = this->localization_strings[LSTR_MYSTIC];
    this->npc_profession_names[Spellmaster]     = this->localization_strings[LSTR_SPELL_MASTER];
    this->npc_profession_names[Trader]          = this->localization_strings[LSTR_TRADER];
    this->npc_profession_names[Merchant]        = this->localization_strings[LSTR_MERCHANT];
    this->npc_profession_names[Scout]           = this->localization_strings[LSTR_SCOUT];
    this->npc_profession_names[Herbalist]       = this->localization_strings[LSTR_HERBALIST];
    this->npc_profession_names[Apothecary]      = this->localization_strings[LSTR_APOTHECARY];
    this->npc_profession_names[Tinker]          = this->localization_strings[LSTR_TINKER];
    this->npc_profession_names[Locksmith]       = this->localization_strings[LSTR_LOCKSMITH];
    this->npc_profession_names[Fool]            = this->localization_strings[LSTR_FOOL];
    this->npc_profession_names[ChimneySweep]    = this->localization_strings[LSTR_CHIMNEY_SWEEP];
    this->npc_profession_names[Porter]          = this->localization_strings[LSTR_PORTER];
    this->npc_profession_names[QuarterMaster]   = this->localization_strings[LSTR_QUARTER_MASTER];
    this->npc_profession_names[Factor]          = this->localization_strings[LSTR_FACTOR];
    this->npc_profession_names[Banker]          = this->localization_strings[LSTR_BANKER];
    this->npc_profession_names[Cook]            = this->localization_strings[LSTR_COOK];
    this->npc_profession_names[Chef]            = this->localization_strings[LSTR_CHEF];
    this->npc_profession_names[Horseman]        = this->localization_strings[LSTR_HORSEMAN];
    this->npc_profession_names[Bard]            = this->localization_strings[LSTR_BARD];
    this->npc_profession_names[Enchanter]       = this->localization_strings[LSTR_ENCHANTER];
    this->npc_profession_names[Cartographer]    = this->localization_strings[LSTR_CARTOGRAPHER];
    this->npc_profession_names[WindMaster]      = this->localization_strings[LSTR_WIND_MASTER];
    this->npc_profession_names[WaterMaster]     = this->localization_strings[LSTR_WATER_MASTER];
    this->npc_profession_names[GateMaster]      = this->localization_strings[LSTR_GATE_MASTER];
    this->npc_profession_names[Acolyte]         = this->localization_strings[LSTR_CHAPLAIN]; // TODO(captainurist): why not LSTR_ACOLYTE?
    this->npc_profession_names[Piper]           = this->localization_strings[LSTR_PIPER];
    this->npc_profession_names[Explorer]        = this->localization_strings[LSTR_EXPLORER];
    this->npc_profession_names[Pirate]          = this->localization_strings[LSTR_PIRATE];
    this->npc_profession_names[Squire]          = this->localization_strings[LSTR_SQUIRE];
    this->npc_profession_names[Psychic]         = this->localization_strings[LSTR_PSYCHIC];
    this->npc_profession_names[Gypsy]           = this->localization_strings[LSTR_GYPSY];
    this->npc_profession_names[Diplomat]        = this->localization_strings[LSTR_DIPLOMAT];
    this->npc_profession_names[Duper]           = this->localization_strings[LSTR_DUPER];
    this->npc_profession_names[Burglar]         = this->localization_strings[LSTR_BURGLAR];
    this->npc_profession_names[FallenWizard]    = this->localization_strings[LSTR_FALLEN_WIZARD];
    this->npc_profession_names[Acolyte2]        = this->localization_strings[LSTR_ACOLYTE];
    this->npc_profession_names[Initiate]        = this->localization_strings[LSTR_INITIATE];
    this->npc_profession_names[Prelate]         = this->localization_strings[LSTR_PRELATE];
    this->npc_profession_names[Monk]            = this->localization_strings[LSTR_MONK];
    this->npc_profession_names[Sage]            = this->localization_strings[LSTR_SAGE];
    this->npc_profession_names[Hunter]          = this->localization_strings[LSTR_HUNTER];
}

void Localization::InitializeCharacterConditionNames() {
    this->character_conditions[CONDITION_CURSED]            = this->localization_strings[LSTR_CURSED];
    this->character_conditions[CONDITION_WEAK]              = this->localization_strings[LSTR_WEAK];
    this->character_conditions[CONDITION_SLEEP]             = this->localization_strings[LSTR_ASLEEP];
    this->character_conditions[CONDITION_FEAR]              = this->localization_strings[LSTR_AFRAID];
    this->character_conditions[CONDITION_DRUNK]             = this->localization_strings[LSTR_DRUNK];
    this->character_conditions[CONDITION_INSANE]            = this->localization_strings[LSTR_INSANE];
    this->character_conditions[CONDITION_POISON_WEAK]       = this->localization_strings[LSTR_POISON];
    this->character_conditions[CONDITION_DISEASE_WEAK]      = this->localization_strings[LSTR_DISEASED];
    this->character_conditions[CONDITION_POISON_MEDIUM]     = this->localization_strings[LSTR_POISON];
    this->character_conditions[CONDITION_DISEASE_MEDIUM]    = this->localization_strings[LSTR_DISEASED];
    this->character_conditions[CONDITION_POISON_SEVERE]     = this->localization_strings[LSTR_POISON];
    this->character_conditions[CONDITION_DISEASE_SEVERE]    = this->localization_strings[LSTR_DISEASED];
    this->character_conditions[CONDITION_PARALYZED]         = this->localization_strings[LSTR_PARALYZED];
    this->character_conditions[CONDITION_UNCONSCIOUS]       = this->localization_strings[LSTR_UNCONSCIOUS];
    this->character_conditions[CONDITION_DEAD]              = this->localization_strings[LSTR_DEAD];
    this->character_conditions[CONDITION_PETRIFIED]         = this->localization_strings[LSTR_STONED];
    this->character_conditions[CONDITION_ERADICATED]        = this->localization_strings[LSTR_ERADICATED];
    this->character_conditions[CONDITION_ZOMBIE]            = this->localization_strings[LSTR_ZOMBIE];
    this->character_conditions[CONDITION_GOOD]              = this->localization_strings[LSTR_GOOD];
}

void Localization::InitializeSkillNames() {
    this->skill_names[CHARACTER_SKILL_STAFF]       = this->localization_strings[LSTR_STAFF];
    this->skill_names[CHARACTER_SKILL_SWORD]       = this->localization_strings[LSTR_SWORD];
    this->skill_names[CHARACTER_SKILL_DAGGER]      = this->localization_strings[LSTR_DAGGER];
    this->skill_names[CHARACTER_SKILL_AXE]         = this->localization_strings[LSTR_AXE];
    this->skill_names[CHARACTER_SKILL_SPEAR]       = this->localization_strings[LSTR_SPEAR];
    this->skill_names[CHARACTER_SKILL_BOW]         = this->localization_strings[LSTR_BOW];
    this->skill_names[CHARACTER_SKILL_MACE]        = this->localization_strings[LSTR_MACE];
    this->skill_names[CHARACTER_SKILL_BLASTER]     = this->localization_strings[LSTR_BLASTER];
    this->skill_names[CHARACTER_SKILL_SHIELD]      = this->localization_strings[LSTR_SHIELD];
    this->skill_names[CHARACTER_SKILL_LEATHER]     = this->localization_strings[LSTR_LEATHER];
    this->skill_names[CHARACTER_SKILL_CHAIN]       = this->localization_strings[LSTR_CHAIN];
    this->skill_names[CHARACTER_SKILL_PLATE]       = this->localization_strings[LSTR_PLATE];
    this->skill_names[CHARACTER_SKILL_FIRE]        = this->localization_strings[LSTR_FIRE_MAGIC];
    this->skill_names[CHARACTER_SKILL_AIR]         = this->localization_strings[LSTR_AIR_MAGIC];
    this->skill_names[CHARACTER_SKILL_WATER]       = this->localization_strings[LSTR_WATER_MAGIC];
    this->skill_names[CHARACTER_SKILL_EARTH]       = this->localization_strings[LSTR_EARTH_MAGIC];
    this->skill_names[CHARACTER_SKILL_SPIRIT]      = this->localization_strings[LSTR_SPIRIT_MAGIC];
    this->skill_names[CHARACTER_SKILL_MIND]        = this->localization_strings[LSTR_MIND_MAGIC];
    this->skill_names[CHARACTER_SKILL_BODY]        = this->localization_strings[LSTR_BODY_MAGIC];
    this->skill_names[CHARACTER_SKILL_LIGHT]       = this->localization_strings[LSTR_LIGHT_MAGIC];
    this->skill_names[CHARACTER_SKILL_DARK]        = this->localization_strings[LSTR_DARK_MAGIC];
    this->skill_names[CHARACTER_SKILL_ITEM_ID]     = this->localization_strings[LSTR_IDENTIFY_ITEM];
    this->skill_names[CHARACTER_SKILL_MERCHANT]    = this->localization_strings[LSTR_MERCHANT];
    this->skill_names[CHARACTER_SKILL_REPAIR]      = this->localization_strings[LSTR_REPAIR_ITEM];
    this->skill_names[CHARACTER_SKILL_BODYBUILDING] = this->localization_strings[LSTR_BODY_BUILDING];
    this->skill_names[CHARACTER_SKILL_MEDITATION]  = this->localization_strings[LSTR_MEDITATION];
    this->skill_names[CHARACTER_SKILL_PERCEPTION]  = this->localization_strings[LSTR_PERCEPTION];
    this->skill_names[CHARACTER_SKILL_DIPLOMACY]   = this->localization_strings[LSTR_DIPLOMACY];
    this->skill_names[CHARACTER_SKILL_THIEVERY]    = this->localization_strings[LSTR_THIEVERY];
    this->skill_names[CHARACTER_SKILL_TRAP_DISARM] = this->localization_strings[LSTR_DISARM_TRAP];
    this->skill_names[CHARACTER_SKILL_DODGE]       = this->localization_strings[LSTR_DODGING];
    this->skill_names[CHARACTER_SKILL_UNARMED]     = this->localization_strings[LSTR_UNARMED];
    this->skill_names[CHARACTER_SKILL_MONSTER_ID]  = this->localization_strings[LSTR_IDENTIFY_MONSTER];
    this->skill_names[CHARACTER_SKILL_ARMSMASTER]  = this->localization_strings[LSTR_ARMSMASTER];
    this->skill_names[CHARACTER_SKILL_STEALING]    = this->localization_strings[LSTR_STEALING];
    this->skill_names[CHARACTER_SKILL_ALCHEMY]     = this->localization_strings[LSTR_ALCHEMY];
    this->skill_names[CHARACTER_SKILL_LEARNING]    = this->localization_strings[LSTR_LEARNING];
    this->skill_names[CHARACTER_SKILL_INVALID]     = this->localization_strings[LSTR_NONE]; // "None", used during character creation.

    // TODO(captainurist): Not currently used anywhere
    // this->skill_names[CHARACTER_SKILL_CLUB]        = this->localization_strings[LSTR_CLUB];
    // this->skill_descriptions[CHARACTER_SKILL_CLUB] = "Everyone is able to wield a club without any prior training and bonk anything with it. "
    //    "But there is not much room to improve finesse or mastery for such a rudimentary weapon though. "
    //    "So don't expect to become thwonking killer and devastating anyone beyond weaklings.";

    skill_desc_raw = engine->_gameResourceManager->getEventsFile("skilldes.txt").string_view();
    strtok(skill_desc_raw.data(), "\r");
    for (CharacterSkillType i : allVisibleSkills()) {
        char *test_string = strtok(NULL, "\r") + 1;

        if (test_string != NULL && strlen(test_string) > 0) {
            auto tokens = tokenize(test_string, '\t');
            assert(tokens.size() >= 6 && "Invalid number of tokens");

            this->skill_descriptions[i] = removeQuotes(tokens[1]);
            this->skill_descriptions_normal[i] = removeQuotes(tokens[2]);
            this->skill_descriptions_expert[i] = removeQuotes(tokens[3]);
            this->skill_descriptions_master[i] = removeQuotes(tokens[4]);
            this->skill_descriptions_grand[i] = removeQuotes(tokens[5]);
        }
    }
}

void Localization::InitializeClassNames() {
    this->class_names[CLASS_KNIGHT] = this->localization_strings[LSTR_KNIGHT];
    this->class_names[CLASS_CAVALIER] = this->localization_strings[LSTR_CAVALIER];
    this->class_names[CLASS_CHAMPION] = this->localization_strings[LSTR_CHAMPION];
    this->class_names[CLASS_BLACK_KNIGHT] = this->localization_strings[LSTR_BLACK_KNIGHT];

    this->class_names[CLASS_THIEF] = this->localization_strings[LSTR_THIEF];
    this->class_names[CLASS_ROGUE] = this->localization_strings[LSTR_ROGUE];
    this->class_names[CLASS_SPY] = this->localization_strings[LSTR_SPY];
    this->class_names[CLASS_ASSASSIN] = this->localization_strings[LSTR_ASSASSIN];

    this->class_names[CLASS_MONK] = this->localization_strings[LSTR_MONK];
    this->class_names[CLASS_INITIATE] = this->localization_strings[LSTR_INITIATE];
    this->class_names[CLASS_MASTER] = this->localization_strings[LSTR_MASTER];
    this->class_names[CLASS_NINJA] = this->localization_strings[LSTR_NINJA];

    this->class_names[CLASS_PALADIN] = this->localization_strings[LSTR_PALADIN];
    this->class_names[CLASS_CRUSADER] = this->localization_strings[LSTR_CRUSADER];
    this->class_names[CLASS_HERO] = this->localization_strings[LSTR_HERO];
    this->class_names[CLASS_VILLIAN] = this->localization_strings[LSTR_VILLAIN];

    this->class_names[CLASS_ARCHER] = this->localization_strings[LSTR_ARCHER];
    this->class_names[CLASS_WARRIOR_MAGE] = this->localization_strings[LSTR_WARRIOR_MAGE];
    this->class_names[CLASS_MASTER_ARCHER] = this->localization_strings[LSTR_MASTER_ARCHER];
    this->class_names[CLASS_SNIPER] = this->localization_strings[LSTR_SNIPER];

    this->class_names[CLASS_RANGER] = this->localization_strings[LSTR_RANGER];
    this->class_names[CLASS_HUNTER] = this->localization_strings[LSTR_HUNTER];
    this->class_names[CLASS_RANGER_LORD] = this->localization_strings[LSTR_RANGER_LORD];
    this->class_names[CLASS_BOUNTY_HUNTER] = this->localization_strings[LSTR_BOUNTY_HUNTER];

    this->class_names[CLASS_CLERIC] = this->localization_strings[LSTR_CLERIC];
    this->class_names[CLASS_PRIEST] = this->localization_strings[LSTR_PRIEST];
    this->class_names[CLASS_PRIEST_OF_SUN] = this->localization_strings[LSTR_PRIEST_OF_LIGHT];
    this->class_names[CLASS_PRIEST_OF_MOON] = this->localization_strings[LSTR_PRIEST_OF_DARK];

    this->class_names[CLASS_DRUID] = this->localization_strings[LSTR_DRUID];
    this->class_names[CLASS_GREAT_DRUID] = this->localization_strings[LSTR_GREATER_DRUID];
    this->class_names[CLASS_ARCH_DRUID] = this->localization_strings[LSTR_ARCH_DRUID];
    this->class_names[CLASS_WARLOCK] = this->localization_strings[LSTR_WARLOCK];

    this->class_names[CLASS_SORCERER] = this->localization_strings[LSTR_SORCERER];
    this->class_names[CLASS_WIZARD] = this->localization_strings[LSTR_WIZARD];
    this->class_names[CLASS_ARCHAMGE] = this->localization_strings[LSTR_ARCHMAGE];
    this->class_names[CLASS_LICH] = this->localization_strings[LSTR_LICH];

    this->class_desc_raw = engine->_gameResourceManager->getEventsFile("class.txt").string_view();
    strtok(this->class_desc_raw.data(), "\r");
    for (CharacterClass i : class_desciptions.indices()) {
        char *test_string = strtok(NULL, "\r") + 1;
        auto tokens = tokenize(test_string, '\t');
        assert(tokens.size() == 3 && "Invalid number of tokens");
        class_desciptions[i] = removeQuotes(tokens[1]);
    }
}

//----- (00452B95) --------------------------------------------------------
void Localization::InitializeMm6ItemCategories() {
    this->mm6_item_categories[0] = this->localization_strings[LSTR_CLUB];
    this->mm6_item_categories[1] = this->localization_strings[LSTR_STAFF];
    this->mm6_item_categories[2] = this->localization_strings[LSTR_SWORD];
    this->mm6_item_categories[3] = this->localization_strings[LSTR_DAGGER];
    this->mm6_item_categories[4] = this->localization_strings[LSTR_AXE];
    this->mm6_item_categories[5] = this->localization_strings[LSTR_SPEAR];
    this->mm6_item_categories[6] = this->localization_strings[LSTR_BOW];
    this->mm6_item_categories[7] = this->localization_strings[LSTR_MACE];
    this->mm6_item_categories[8] = this->localization_strings[LSTR_BLASTER];
    this->mm6_item_categories[9] = this->localization_strings[LSTR_SHIELD];
    this->mm6_item_categories[10] = this->localization_strings[LSTR_LEATHER];
    this->mm6_item_categories[11] = this->localization_strings[LSTR_CHAIN];
    this->mm6_item_categories[12] = this->localization_strings[LSTR_PLATE];
    this->mm6_item_categories[13] = this->localization_strings[LSTR_MISC];
}

//----- (00413FF1) --------------------------------------------------------
void Localization::InitializeMonthNames() {
    this->month_names[0] = this->localization_strings[LSTR_JANUARY];
    this->month_names[1] = this->localization_strings[LSTR_FEBRUARY];
    this->month_names[2] = this->localization_strings[LSTR_MARCH];
    this->month_names[3] = this->localization_strings[LSTR_APRIL];
    this->month_names[4] = this->localization_strings[LSTR_MAY];
    this->month_names[5] = this->localization_strings[LSTR_JUNE];
    this->month_names[6] = this->localization_strings[LSTR_JULY];
    this->month_names[7] = this->localization_strings[LSTR_AUGUST];
    this->month_names[8] = this->localization_strings[LSTR_SEPTEMBER];
    this->month_names[9] = this->localization_strings[LSTR_OCTOBER];
    this->month_names[10] = this->localization_strings[LSTR_NOVEMBER];
    this->month_names[11] = this->localization_strings[LSTR_DECEMBER];
}

//----- (0041406F) --------------------------------------------------------
void Localization::InitializeDayNames() {
    this->day_names[0] = this->localization_strings[LSTR_MONDAY];
    this->day_names[1] = this->localization_strings[LSTR_TUESDAY];
    this->day_names[2] = this->localization_strings[LSTR_WEDNESDAY];
    this->day_names[3] = this->localization_strings[LSTR_THURSDAY];
    this->day_names[4] = this->localization_strings[LSTR_FRIDAY];
    this->day_names[5] = this->localization_strings[LSTR_SATURDAY];
    this->day_names[6] = this->localization_strings[LSTR_SUNDAY];
}

//----- (004140BB) --------------------------------------------------------
void Localization::InitializeSpellSchoolNames() {
    this->spell_school_names[MAGIC_SCHOOL_FIRE]   = this->localization_strings[LSTR_FIRE];
    this->spell_school_names[MAGIC_SCHOOL_AIR]    = this->localization_strings[LSTR_AIR];
    this->spell_school_names[MAGIC_SCHOOL_WATER]  = this->localization_strings[LSTR_WATER];
    this->spell_school_names[MAGIC_SCHOOL_EARTH]  = this->localization_strings[LSTR_EARTH];
    this->spell_school_names[MAGIC_SCHOOL_SPIRIT] = this->localization_strings[LSTR_SPIRIT];
    this->spell_school_names[MAGIC_SCHOOL_MIND]   = this->localization_strings[LSTR_MIND];
    this->spell_school_names[MAGIC_SCHOOL_BODY]   = this->localization_strings[LSTR_BODY];
    this->spell_school_names[MAGIC_SCHOOL_LIGHT]  = this->localization_strings[LSTR_LIGHT];
    this->spell_school_names[MAGIC_SCHOOL_DARK]   = this->localization_strings[LSTR_DARK];
}

//----- (0041411B) --------------------------------------------------------
void Localization::InitializeAttributeNames() {
    this->attribute_names[ATTRIBUTE_MIGHT]        = this->localization_strings[LSTR_MIGHT];
    this->attribute_names[ATTRIBUTE_INTELLIGENCE] = this->localization_strings[LSTR_INTELLECT];
    this->attribute_names[ATTRIBUTE_PERSONALITY]  = this->localization_strings[LSTR_PERSONALITY];
    this->attribute_names[ATTRIBUTE_ENDURANCE]    = this->localization_strings[LSTR_ENDURANCE];
    this->attribute_names[ATTRIBUTE_ACCURACY]     = this->localization_strings[LSTR_ACCURACY];
    this->attribute_names[ATTRIBUTE_SPEED]        = this->localization_strings[LSTR_SPEED];
    this->attribute_names[ATTRIBUTE_LUCK]         = this->localization_strings[LSTR_LUCK];

    this->attribute_desc_raw = engine->_gameResourceManager->getEventsFile("stats.txt").string_view();
    strtok(this->attribute_desc_raw.data(), "\r");
    for (int i = 0; i < 26; ++i) {
        char *test_string = strtok(NULL, "\r") + 1;
        std::vector<std::string_view> tokens = split(test_string, '\t');
        assert(tokens.size() == 2 && "Invalid number of tokens");
        switch (i) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
                this->attribute_descriptions[static_cast<CharacterAttribute>(i)] = removeQuotes(tokens[1]);
                break;
            case 7:
                this->hp_description = removeQuotes(tokens[1]);
                break;
            case 8:
                this->armour_class_description = removeQuotes(tokens[1]);
                break;
            case 9:
                this->sp_description = removeQuotes(tokens[1]);
                break;
            case 10:
                this->character_condition_description = removeQuotes(tokens[1]);
                break;
            case 11:
                this->fast_spell_description = removeQuotes(tokens[1]);
                break;
            case 12:
                this->age_description = removeQuotes(tokens[1]);
                break;
            case 13:
                this->level_description = removeQuotes(tokens[1]);
                break;
            case 14:
                this->exp_description = removeQuotes(tokens[1]);
                break;
            case 15:
                this->melee_attack_description = removeQuotes(tokens[1]);
                break;
            case 16:
                this->melee_damage_description = removeQuotes(tokens[1]);
                break;
            case 17:
                this->ranged_attack_description = removeQuotes(tokens[1]);
                break;
            case 18:
                this->ranged_damage_description = removeQuotes(tokens[1]);
                break;
            case 19:
                this->fire_res_description = removeQuotes(tokens[1]);
                break;
            case 20:
                this->air_res_description = removeQuotes(tokens[1]);
                break;
            case 21:
                this->water_res_description = removeQuotes(tokens[1]);
                break;
            case 22:
                this->earth_res_description = removeQuotes(tokens[1]);
                break;
            case 23:
                this->mind_res_description = removeQuotes(tokens[1]);
                break;
            case 24:
                this->body_res_description = removeQuotes(tokens[1]);
                break;
            case 25:
                this->skill_points_description = removeQuotes(tokens[1]);
                break;
        }
    }
}

//----- (00410AF5) --------------------------------------------------------
void Localization::InitializeMoonPhaseNames() {
    this->moon_phase_names[0] = this->localization_strings[LSTR_NEW];
    this->moon_phase_names[1] = this->localization_strings[LSTR_QUARTER];
    this->moon_phase_names[2] = this->localization_strings[LSTR_HALF];
    this->moon_phase_names[3] = this->localization_strings[LSTR_THREE_QUARTER];
    this->moon_phase_names[4] = this->localization_strings[LSTR_FULL];
}

/*
enum GLOBAL_LOCALIZ_INDEX
{
    LOCSTR_AC = 0,		//"AC"
    LOCSTR_ACCURACY = 1,		//"Accuracy"
    LOCSTR_BLK_KNIGHT = 2,		//"Black Knight"
    LOCSTR_SPY = 3,		// "Spy"

    LOCSTR_TOWN_PORTAL = 10, // "Town Portal"

    LOCSTR_ASSASIN = 13,	// "Assassin"

    LOCSTR_AUTOSAVE = 16,	// "Autosave"

    LOCSTR_ATTACK = 18, // "Attack"
    LOCSTR_AVAILABLE = 19, //"Available"

    LOCSTR_MONK = 21, //"Monk"

    LOCSTR_AVARDS_FOR = 23, //"Awards for"


    LOCSTR_RANGER = 31, //"Ranger"

    LOCSTR_CANCEL = 34, //"Cancel"


    LOCSTR_SELECT_TGT = 39,	 ///"Select Target"

    LOCSTR_CLASS = 41,	// "Class"

    LOCSTR_CONDITION = 47,	//"Condition"

    LOCSTR_CREATE_PARTY = 51,   //"C R E A T E   P A R T Y"

    LOCSTR_DAWN = 55, //"Dawn"
    LOCSTR_DAY = 56,	//"Day"
    LOCSTR_DAYS = 57,	/// "Days"

    LOCSTR_PRESS_ESCAPE = 61,	// "Press Escape"

    LOCSTR_TROUBLE_LOAD = 63,//"Might and Magic VII is having trouble loading
files.
                             // Please re-install to fix this problem. Note:
Re-installing will not destroy your save games." LOCSTR_DETAIL_TOGGLE = 64,
//"Detail Toggle"

    LOCSTR_DMG = 66,	 /// "Dmg"

    LOCSTR_EMPTY = 72, // "Empty"

    LOCSTR_EXIT = 79,	// "Exit"
    LOCSTR_EXIT_BLDNG = 80,   // "Exit Building"

    LOCSTR_EXPIRIENCE = 83, //"Experience"

    LOCSTR_GRAND = 96,	// "Grand"

    LOCSTR_HP = 107,	// "HP"
    LOCSTR_HIT_POINTS = 108,	// "Hit Points"
    LOCSTR_HOUR = 109,	//"Hour"
    LOCSTR_HOURS = 110, //"Hours"

    LOCSTR_ROGUE = 114,	 //"Rogue"

    LOCSTR_LEVEL = 131,   // "Level"

    LOCSTR_LOADING = 135,  //"Loading"

    LOCSTR_MIGHT = 144,	 //"Might"

    LOCSTR_NAME = 149,	// "Name"

    LOCSTR_YOU_TO_TRAIN = 147, // "You are eligible to train to %u."

    LOCSTR_STAY_IN_AREA = 156,  //"Stay in this Area"
    LOCSTR_3DO_COPYRHT = 157,	  // ""© 1999 The 3DO Company.

    LOCSTR_PLEASE_WAIT = 165,	// "Please Wait"

    LOCSTR_REPUTATION = 180,	 //"Reputation"

    LOCSTR_REST_HEAL_8 = 183, //"Rest & Heal 8 Hours"

    LOCSTR_TIME_ERATHIA = 186, // "Time in Erathia"

    LOCSTR_SAVING = 190,  // "Saving"

    LOCSTR_SKILL_POINTS = 207, /// "Skill Points"

    LOCSTR_KNIGHT = 253,  //"Knight"
    LOCSTR_CAVALIER = 254,  //"Cavalier"
    LOCSTR_CHAMPION = 255,  //"Champion"
    LOCSTR_CLERIC = 256,  //"Cleric"
    LOCSTR_PRIEST = 257,  //"Priest"

    LOCSTR_SORCERER = 259,  // "Sorcerer"

    LOCSTR_PALADIN = 262, // "Paladin"

    LOCSTR_ARCHER = 265,   /// "Archer"

    LOCSTR_DRUID = 268, // "Druid"

    LOCSTR_THIEF = 307, //"Thief"

    LOCSTR_SET_BEACON = 375, // "Set Beacon"

    LOCSTR_HATED = 379,// "Hated"

    LOCSTR_UNFRENDLY = 392,	 //"Unfriendly"

    LOCSTR_NEITRAL = 399,  // "Neutral"

    LOCSTR_FRENDLY = 402, // "Friendly"

    LOCSTR_D_DAYS_TO_S = 404,   //"%d days to %s"
    LOCSTR_TRAVEL_COST = 405,    //"Travel Cost %d gold"

    LOCSTR_ENTER_S = 411, // "Enter %s"

    LOCSTR_IS_IN_NO_COND = 427,	// "%s is in no condition to %s"

    LOCSTR_S_THE_S = 429,    //"%s the %s"

    LOCSTR_NORMAL = 431,	//"Normal"
    LOCSTR_MASTER = 432,	//"Master"
    LOCSTR_EXPERT = 433,	//"Expert"
    LOCSTR_LIKED = 434, //"Liked"

    LOCSTR_ACTIVE_SPELL = 450,	// "Active Spells: %s"

    LOCSTR_READING = 505, ///"Reading..."

    LOCSTR_NOTHING_HERE = 521, // "Nothing here"

    LOCSTR_SP_COST = 522, //"SP Cost"
    LOCSTR_RECALL_BEACON = 523, // "Recall Beacon"

    LOCSTR_TIME = 526,	//"Time"

    LOCSTR_NEED_MORE_EXP = 538, //"You need %d more experience to train to level
%d"

    LOCSTR_IDENT_ITEM = 541, // "Identify Items"

    LOCSTR_DUSK = 566,   //"Dusk"
    LOCSTR_NIGHT = 567,  // "Night"

    LOCSTR_NO_SAVING = 583,	//"No saving in the Arena"

    LOCSTR_AUTOSAVE_MM7 = 613, // "AutoSave.MM7"

    LOCSTR_BONUS = 623, //"Bonus"

    LOCSTR_GAME_SAVED = 656,	// "Game Saved!"

    MAX_LOC_STRINGS = 677

};*/

/*

+		[4]	0x04102dab "Afraid"	unsigned char *
+		[5]	0x04102db5 "Age"	unsigned char *
+		[6]	0x04102dbc "Air"	unsigned char *
+		[7]	0x04102dc3 "Alchemist"	unsigned char *
+		[8]	0x04102dd0 "Obelisk Notes"	unsigned char *
+		[9]	0x04102de1 "Congratulations!"	unsigned char *

+		[11]	0x04102e06 "Armor"	unsigned char *
+		[12]	0x04102e10 "Armor Class"	unsigned char *

+		[14]	0x04102e2d "Asleep"	unsigned char *
+		[15]	0x04102e38 "Assertion failed at %d in %s"
unsigned char *

+		[17]	0x04102e66 "Exp."	unsigned char *


+		[20]	0x04102e88 "Available Skills"	unsigned char *

+		[22]	0x04102ea6 "Awards"	unsigned char *

+		[24]	0x04102ec0 "Fire Res"	unsigned char *
+		[25]	0x04102ecd "Balance"	unsigned char *
+		[26]	0x04102ed9 "Initiate"	unsigned char *
+		[27]	0x04102ee6 "Ninja"	unsigned char *
+		[28]	0x04102ef0 "Villain"	unsigned char *
+		[29]	0x04102efc "Body"	unsigned char *
+		[30]	0x04102f05 "Bonus"	unsigned char *

+		[32]	0x04102f1a "Broken Item"	unsigned char *
+		[33]	0x04102f2a "Ranger Lord"	unsigned char *
+		[34]	0x04102f3a "Cancel"	unsigned char *
+		[35]	0x04102f45 "Town Portal to %s"	unsigned char *
+		[36]	0x04102f5b "%s can not be used that way"
unsigned char * +		[37]	0x04102f7b "Total Time:"
unsigned char * +		[38]	0x04102f8b "Cast Spell"	unsigned char *

+		[0x28]	0x04102fac "Bounty Hunter"	unsigned char *

+		[0x2a]	0x04102fc8 "Clerk"	unsigned char *
+		[0x2b]	0x04102fd2 "Cold"	unsigned char *
+		[0x2c]	0x04102fdb "Priest of Light"	unsigned char *
+		[45]	0x04102fef "Cond"	unsigned char *
+		[46]	0x04102ff8 "Priest of Dark"	unsigned char *

+		[48]	0x04103019 "Warlock"	unsigned char *
+		[49]	0x04103025 "Lich"	unsigned char *
+		[50]	0x0410302e "Dodging"	unsigned char *

+		[52]	0x04103056 "Cursed"	unsigned char *
+		[53]	0x04103061 "Damage"	unsigned char *
+		[0x36]	0x0410306c "Dark"	unsigned char *



+		[58]	0x0410308f "Dead"	unsigned char *
+		[59]	0x04103098 "Internal Error"	unsigned char *
+		[60]	0x041030ab "Deposit"	unsigned char *

+		[62]	0x041030c8 "Might and Magic VII requires your desktop to
be in 16bit (32k or 65k) Color mode in order to operate in a window." unsigned
char *


+		[65]	0x041031e6 "Diseased"	unsigned char *

+		[0x43]	0x041031fb "%s does not have the skill"	unsigned char *
+		[68]	0x0410321a "Donate"	unsigned char *
+		[69]	0x04103225 "Drunk"	unsigned char *
+		[70]	0x0410322f "Earth"	unsigned char *
+		[71]	0x04103239 "Electricity"	unsigned char *

+		[73]	0x04103253 "Enter"	unsigned char *
+		[74]	0x0410325d "End Conversation"	unsigned char *
+		[75]	0x04103272 "Endurance"	unsigned char *
+		[0x4c]	0x04103280 "Eradicated"	unsigned char *
+		[0x4d]	0x0410328f "Unarmed"	unsigned char *
+		[0x4e]	0x0410329b "Calendar"	unsigned char *
+		[79]	0x041032a8 "Exit"	unsigned char *

+		[81]	0x041032c3 "Exit Rest"	unsigned char *
+		[82]	0x041032d1 "Are you sure?  Click again to quit" unsigned
char *

+		[84]	0x04103307 "Fame"	unsigned char *
+		[85]	0x04103310 "Potion Notes"	unsigned char *
+		[86]	0x04103321 "Fill Packs to %d days for %d gold" unsigned
char * +		[87]	0x04103347 "Fire"	unsigned char * +
[88]	0x04103350 "Identify Monster"	unsigned char * +		[89]
0x04103365 "Armsmaster"	unsigned char * +		[90]	0x04103374
"Stealing"	unsigned char * +		[91]	0x04103381 "Friday"
unsigned char * +		[92]	0x0410338c "Full"	unsigned char *
+		[93]	0x04103395 "Game Options"	unsigned char *
+		[94]	0x041033a6 "Your score: %lu"	unsigned char *
+		[95]	0x041033ba "Alchemy"	unsigned char *



+		[97]	0x041033d0 "Gold"	unsigned char *
+		[98]	0x041033d9 "Good"	unsigned char *
+		[99]	0x041033e2 "Human"	unsigned char *
+		[100]	0x041033ed "Guard"	unsigned char *
+		[101]	0x041033f8 "Dwarf"	unsigned char *
+		[102]	0x04103403 "Half"	unsigned char *
+		[103]	0x0410340d "Elf"	unsigned char *
+		[104]	0x04103416 "Heal"	unsigned char *
+		[105]	0x04103420 "Healer"	unsigned char *
+		[106]	0x0410342c "Goblin"	unsigned char *

+		[108]	0x04103440 "Hit Points"	unsigned char *

+		[111]	0x04103465 "Grand"	unsigned char *
+		[112]	0x04103470 "How Much?"	unsigned char *
+		[113]	0x0410347f "Identify"	unsigned char *

+		[115]	0x04103498 "Instructor"	unsigned char *
+		[116]	0x041034a8 "Intellect"	unsigned char *
+		[117]	0x041034b7 "Insane"	unsigned char *
+		[118]	0x041034c3 "We hope that you've enjoyed playing Might and Magic VII as much as we did making it. We have saved this screen as MM7_WIN.PCX in your MM7 directory. You can print it out as proof of your accomplishment."	unsigned char *
+		[119]	0x04103593 "Master Archer"	unsigned char *
+		[120]	0x041035a6 "Inventory"	unsigned char *
+		[121]	0x041035b5 "Permanent"	unsigned char *
+		[122]	0x041035c4 "Join"	unsigned char *
+		[123]	0x041035ce "Miscellaneous Notes"	unsigned char *
+		[124]	0x041035e7 "Sniper"	unsigned char *
+		[125]	0x041035f3 "+2 Skill Points!"	unsigned char *
+		[126]	0x04103609 "Do you wish to leave %s?"	unsigned char *
+		[127]	0x04103627 "No Text!"	unsigned char *
+		[128]	0x04103635 "It will take %d days to travel to %s."
unsigned char * +		[129]	0x04103660 "%s the Level %u %s" unsigned
char * +		[130]	0x04103678 "Player"	unsigned char *

+		[132]	0x0410368f "Years"	unsigned char *
+		[133]	0x0410369a "Light"	unsigned char *
+		[134]	0x041036a5 "Buy Standard"	unsigned char *

+		[136]	0x041036c4 "Luck"	unsigned char *
+		[137]	0x041036ce "Fountain Notes"	unsigned char *
+		[138]	0x041036e2 "Magic"	unsigned char *
+		[139]	0x041036ed "Maps"	unsigned char *
+		[140]	0x041036f7 "Your packs are already full!"
unsigned char * +		[141]	0x04103719 "Seer Notes"	unsigned char *
+		[142]	0x04103729 "Mind"	unsigned char *
+		[143]	0x04103733 "Misc"	unsigned char *

+		[145]	0x04103748 "Monday"	unsigned char *
+		[146]	0x04103754 "Month"	unsigned char *

+		[148]	0x04103785 "Months"	unsigned char *

+		[150]	0x0410379b "New"	unsigned char *
+		[151]	0x041037a4 "Congratulations Adventurer."
unsigned char * +		[152]	0x041037c5 "Buy Special"
unsigned char * +		[153]	0x041037d6 "None"	unsigned char *
+		[154]	0x041037e0 "Auto Notes"	unsigned char *
+		[155]	0x041037f0 "You don't have enough gold"	unsigned char *

+		[157]	0x04103827 ""© 1999 The 3DO Company.  All Rights
Reserved.  Might and Magic, Blood and Honor, New World Computing, 3DO, and their
respective logos, are trademarks and/or service marks of The 3DO Company in the
U.S. and other countries.  All other trademarks belong to their respective
owners.  New World Computing is a division of The 3DO Company.""	unsigned
char * +		[158]	0x0410397d "Bootleg Bay East"	unsigned char *
+		[159]	0x04103993 "Display Inventory"	unsigned char *
+		[160]	0x041039aa "Learn Skills"	unsigned char *
+		[161]	0x041039bc "Steal"	unsigned char *
+		[162]	0x041039c7 "Paralyzed"	unsigned char *
+		[163]	0x041039d6 "Personality"	unsigned char *
+		[164]	0x041039e7 "%s hits %s for %lu damage"	unsigned char *

+		[166]	0x04103a17 "Poison"	unsigned char *
+		[167]	0x04103a23 "- The Might and Magic VII Development Team."
unsigned char * +		[168]	0x04103a54 "Points"	unsigned char *
+		[169]	0x04103a60 "Three Quarter"	unsigned char *
+		[170]	0x04103a73 "QSpell"	unsigned char *
+		[171]	0x04103a7f "Quarter"	unsigned char *
+		[172]	0x04103a8c "Quick Spell"	unsigned char *
+		[173]	0x04103a9d "Quick Reference"	unsigned char *
+		[174]	0x04103ab2 "Current Quests"	unsigned char *
+		[175]	0x04103ac6 "%s inflicts %lu points killing %s" unsigned
char *
+		[176]	0x04103aed "Might and Magic VII has detected an internal
error and will be forced to close.  Would you like us to autosave your game
before closing?"	unsigned char * +		[177]	0x04103b7b
"Standard"	unsigned char * +		[178]	0x04103b89 "Rent Room
for %d gold"	unsigned char * +		[179]	0x04103ba4 "Repair"
unsigned char *

+		[181]	0x04103bc0 "Steal %24"	unsigned char *
+		[182]	0x04103bcf "Rest"	unsigned char *

+		[184]	0x04103bf2 "Reinstall Necessary"	unsigned char *
+		[185]	0x04103c0b "Steal item"	unsigned char *

+		[187]	0x04103c30 "Stolen"	unsigned char *
+		[188]	0x04103c3c "Saturday"	unsigned char *
+		[189]	0x04103c4a "%s shoots %s for %lu points"
unsigned char *

+		[191]	0x04103c77 "You've been banned from this shop!" unsigned
char * +		[192]	0x04103c9f "Scroll Up"	unsigned char * +
[193]	0x04103cae "Scroll Down"	unsigned char * +		[194]
0x04103cbf "Water Res"	unsigned char * +		[195]	0x04103cce
"Select the Item to Buy"	unsigned char * +		[196]
0x04103cea "Select the Special Item to Buy"	unsigned char * +
[197]	0x04103d0e "Select the Item to Identify"	unsigned char * +
[198]	0x04103d2f "Select the Item to Repair"	unsigned char * +
[199]	0x04103d4e "Select the Item to Sell"	unsigned char * +
[200]	0x04103d6b "Sell"	unsigned char * +		[201]
0x04103d75 "Are you sure?  Click again to start a New Game"	unsigned char *
+		[202]	0x04103da9 "Air Res"	unsigned char *
+		[203]	0x04103db6 "Shoot"	unsigned char *
+		[204]	0x04103dc1 "Body Res"	unsigned char *
+		[205]	0x04103dcf "Skills"	unsigned char *
+		[206]	0x04103ddb "Skills for"	unsigned char *

+		[208]	0x04103dfd "Earth Res"	unsigned char *
+		[209]	0x04103e0c "SP"	unsigned char *
+		[210]	0x04103e14 "Special"	unsigned char *
+		[211]	0x04103e21 "Speed"	unsigned char *
+		[212]	0x04103e2c "Spell Points"	unsigned char *
+		[213]	0x04103e3e "Mind Res"	unsigned char *
+		[214]	0x04103e4c "Spirit"	unsigned char *
+		[215]	0x04103e58 "Detect Life"	unsigned char *
+		[216]	0x04103e69 "Stats"	unsigned char *
+		[217]	0x04103e74 "Invisibility"	unsigned char *
+		[218]	0x04103e86 "Immolation"	unsigned char *
+		[219]	0x04103e96 "Day of the Gods"	unsigned char *
+		[220]	0x04103eab "Stoned"	unsigned char *
+		[221]	0x04103eb7 "Fate"	unsigned char *
+		[222]	0x04103ec1 "Sunday"	unsigned char *
+		[223]	0x00000000 <Bad Ptr>	unsigned char *
+		[224]	0x00000000 <Bad Ptr>	unsigned char *
+		[225]	0x04103ed9 "Grandmaster"	unsigned char *
+		[226]	0x04103eea ""Gamma controls the relative ""brightness""
of the game.  May vary depending on your monitor.""	unsigned char * +
[227]	0x04103f4e "Thursday"	unsigned char * +		[228]
0x04103f5c "Hammerhands"	unsigned char * +		[229]
0x04103f6d "Pain Reflection"	unsigned char * +		[230]
0x04103f82 "Tuesday"	unsigned char * +		[231]	0x04103f8f
"Unconscious"	unsigned char * +		[232]	0x04103fa0 "Not
Identified"	unsigned char * +		[233]	0x04103fb4
"Preservation"	unsigned char * +		[234]	0x04103fc6
"Regeneration"	unsigned char * +		[235]	0x04103fd8 "Temp
Accuracy"	unsigned char * +		[236]	0x04103feb "Wait without
healing"	unsigned char * +		[237]	0x04104005 "Wait until
Dawn"	unsigned char * +		[238]	0x0410401a "Wait 5 Minutes"
unsigned char * +		[239]	0x0410402e "Wait 1 Hour"
unsigned char * +		[240]	0x0410403f "Water"	unsigned char *
+		[241]	0x0410404a "Weak"	unsigned char *
+		[242]	0x04104054 "Weapons"	unsigned char *
+		[243]	0x04104061 "Wednesday"	unsigned char *
+		[244]	0x04104070 "Withdraw"	unsigned char *
+		[245]	0x0410407e "Year"	unsigned char *
+		[246]	0x04104088 "Temp Endurance"	unsigned char *
+		[247]	0x0410409c "Temp Intelligence"	unsigned char *
+		[248]	0x041040b3 "Temp Luck"	unsigned char *
+		[249]	0x041040c2 "Temp Personality"	unsigned char *
+		[250]	0x041040d8 "You need %s"	unsigned char *
+		[251]	0x041040e9 "Zoom In"	unsigned char *
+		[252]	0x041040f6 "Zoom Out"	unsigned char *


+		[258]	0x04104144 "Temp Speed"	unsigned char *

+		[260]	0x04104162 "Wizard"	unsigned char *
+		[261]	0x0410416e "Archmage"	unsigned char *

+		[263]	0x04104189 "Crusader"	unsigned char *
+		[264]	0x04104197 "Hero"	unsigned char *

+		[266]	0x041041ad "DISCARD A CARD"	unsigned char *
+		[267]	0x041041c1 "Warrior Mage"	unsigned char *

+		[269]	0x041041de "Greater Druid"	unsigned char *
+		[270]	0x041041f1 "Arch Druid"	unsigned char *
+		[271]	0x04104201 "Staff"	unsigned char *
+		[272]	0x0410420c "Sword"	unsigned char *
+		[273]	0x04104217 "Dagger"	unsigned char *
+		[274]	0x04104223 "Axe"	unsigned char *
+		[275]	0x0410422c "Spear"	unsigned char *
+		[276]	0x04104237 "Bow"	unsigned char *
+		[277]	0x04104240 "Mace"	unsigned char *
+		[278]	0x0410424a "Blaster"	unsigned char *
+		[279]	0x04104257 "Shield"	unsigned char *
+		[280]	0x04104263 "Leather"	unsigned char *
+		[281]	0x04104270 "Chain"	unsigned char *
+		[282]	0x0410427b "Plate"	unsigned char *
+		[283]	0x04104286 "Fire Magic"	unsigned char *
+		[284]	0x04104296 "Air Magic"	unsigned char *
+		[285]	0x041042a5 "Water Magic"	unsigned char *
+		[286]	0x041042b6 "Earth Magic"	unsigned char *
+		[287]	0x041042c7 "Light Magic"	unsigned char *
+		[288]	0x041042d8 "Dark Magic"	unsigned char *
+		[289]	0x041042e8 "Spirit Magic"	unsigned char *
+		[290]	0x041042fa "Mind Magic"	unsigned char *
+		[291]	0x0410430a "Body Magic"	unsigned char *
+		[292]	0x0410431a "Identify Item"	unsigned char *
+		[293]	0x0410432d "Merchant"	unsigned char *
+		[294]	0x0410433b "Repair Item"	unsigned char *
+		[295]	0x0410434c "Body Building"	unsigned char *
+		[296]	0x0410435f "Meditation"	unsigned char *
+		[297]	0x0410436f "Perception"	unsigned char *
+		[298]	0x0410437f "Diplomacy"	unsigned char *
+		[299]	0x0410438e "Thievery"	unsigned char *
+		[300]	0x0410439c "Disarm Trap"	unsigned char *
+		[301]	0x041043ad "Learning"	unsigned char *
+		[302]	0x041043bb "%s stole %d gold!"	unsigned char *
+		[303]	0x041043d2 "Noble"	unsigned char *
+		[304]	0x041043dd "Official"	unsigned char *
+		[305]	0x041043eb "Peasant"	unsigned char *
+		[306]	0x041043f8 "Scholar"	unsigned char *

+		[308]	0x04104410 "Smith"	unsigned char *
+		[309]	0x0410441b "Armorer"	unsigned char *
+		[310]	0x04104428 "Guide"	unsigned char *
+		[311]	0x04104433 "Tracker"	unsigned char *
+		[312]	0x04104440 "Pathfinder"	unsigned char *
+		[313]	0x04104450 "Sailor"	unsigned char *
+		[314]	0x0410445c "Navigator"	unsigned char *
+		[315]	0x0410446b "Expert Healer"	unsigned char *
+		[316]	0x0410447e "Master Healer"	unsigned char *
+		[317]	0x04104491 "Teacher"	unsigned char *
+		[318]	0x0410449e "Arms Master"	unsigned char *
+		[319]	0x041044af "Weapons Master"	unsigned char *
+		[320]	0x041044c3 "Apprentice"	unsigned char *
+		[321]	0x041044d3 "Mystic"	unsigned char *
+		[322]	0x041044df "Spell Master"	unsigned char *
+		[323]	0x041044f1 "Trader"	unsigned char *
+		[324]	0x041044fd "Scout"	unsigned char *
+		[325]	0x04104508 "Counselor"	unsigned char *
+		[326]	0x04104517 "Barrister"	unsigned char *
+		[327]	0x04104526 "Tinker"	unsigned char *
+		[328]	0x04104532 "Locksmith"	unsigned char *
+		[329]	0x04104541 "Fool"	unsigned char *
+		[330]	0x0410454b "Chimney Sweep"	unsigned char *
+		[331]	0x0410455e "Porter"	unsigned char *
+		[332]	0x0410456a "Quarter Master"	unsigned char *
+		[333]	0x0410457e "Factor"	unsigned char *
+		[334]	0x0410458a "Banker"	unsigned char *
+		[335]	0x04104596 "Cook"	unsigned char *
+		[336]	0x041045a0 "Chef"	unsigned char *
+		[337]	0x041045aa "Horseman"	unsigned char *
+		[338]	0x041045b8 "Bard"	unsigned char *
+		[339]	0x041045c2 "Enchanter"	unsigned char *
+		[340]	0x041045d1 "Cartographer"	unsigned char *
+		[341]	0x041045e3 "Wind Master"	unsigned char *
+		[342]	0x041045f4 "Water Master"	unsigned char *
+		[343]	0x04104606 "Gate Master"	unsigned char *
+		[344]	0x04104617 "Acolyte"	unsigned char *
+		[345]	0x04104624 "Piper"	unsigned char *
+		[346]	0x0410462f "Explorer"	unsigned char *
+		[347]	0x0410463d "Pirate"	unsigned char *
+		[348]	0x04104649 "Squire"	unsigned char *
+		[349]	0x04104655 "Psychic"	unsigned char *
+		[350]	0x04104662 "Gypsy"	unsigned char *
+		[351]	0x0410466d "Negotiator"	unsigned char *
+		[352]	0x0410467d "Duper"	unsigned char *
+		[353]	0x04104688 "Burglar"	unsigned char *
+		[354]	0x04104695 "Serf"	unsigned char *
+		[355]	0x0410469f "Tailor"	unsigned char *
+		[356]	0x041046ab "Laborer"	unsigned char *
+		[357]	0x041046b8 "Farmer"	unsigned char *
+		[358]	0x041046c4 "Cooper"	unsigned char *
+		[359]	0x041046d0 "Potter"	unsigned char *
+		[360]	0x041046dc "Weaver"	unsigned char *
+		[361]	0x041046e8 "Cobbler"	unsigned char *
+		[362]	0x041046f5 "Ditch Digger"	unsigned char *
+		[363]	0x04104707 "Miller"	unsigned char *
+		[364]	0x04104713 "Carpenter"	unsigned char *
+		[365]	0x04104722 "Stone Cutter"	unsigned char *
+		[366]	0x04104734 "Jester"	unsigned char *
+		[367]	0x04104740 "Trapper"	unsigned char *
+		[368]	0x0410474d "Beggar"	unsigned char *
+		[369]	0x04104759 "Rustler"	unsigned char *
+		[370]	0x04104766 "Hunter"	unsigned char *
+		[371]	0x04104772 "Scribe"	unsigned char *
+		[372]	0x0410477e "Missionary"	unsigned char *
+		[373]	0x0410478e "Follower of Baa"	unsigned char *
+		[374]	0x041047a3 "Gambler"	unsigned char *

+		[376]	0x041047c0 "%s was caught stealing!"	unsigned char *
+		[377]	0x041047dd "%s failed to steal anything!"
unsigned char * +		[378]	0x041047ff ""%s %d, %d""
unsigned char *

+		[380]	0x0410481b "You already know the %s spell"
unsigned char * +		[381]	0x0410483e "You don't have the skill to
learn %s"	unsigned char * +		[382]	0x04104868 "That player
is %s"	unsigned char * +		[383]	0x0410487f "his"
unsigned char * +		[384]	0x04104888 "her"	unsigned char *
+		[385]	0x04104891 "sir"	unsigned char *
+		[386]	0x0410489a "Sir"	unsigned char *
+		[387]	0x041048a3 "lady"	unsigned char *
+		[388]	0x041048ad "Lord"	unsigned char *
+		[389]	0x041048b7 "Lady"	unsigned char *
+		[390]	0x041048c1 "brother"	unsigned char *
+		[391]	0x041048ce "sister"	unsigned char *

+		[393]	0x041048ea "daughter"	unsigned char *
+		[394]	0x041048f8 "Unknown"	unsigned char *
+		[395]	0x04104905 "morning"	unsigned char *
+		[396]	0x04104912 "day"	unsigned char *
+		[397]	0x0410491b "evening"	unsigned char *
+		[398]	0x04104928 "Have a Drink"	unsigned char *

+		[400]	0x04104947 "Buy Spells"	unsigned char *
+		[401]	0x04104957 "Skill Cost: %lu"	unsigned char *

+		[403]	0x0410497a "You already know the %s skill"
unsigned char *

+		[406]	0x041049c9 "Hire"	unsigned char *
+		[407]	0x041049d3 "More Information"	unsigned char *
+		[408]	0x041049e9 "Dismiss %s"	unsigned char *
+		[409]	0x041049f9 "Do you wish to leave %s?"	unsigned char *
+		[410]	0x04104a17 "Leave %s"	unsigned char *

+		[412]	0x04104a33 "Create Party cannot be completed unless you
have assigned all characters 2 extra skills and have spent all of your bonus
points."	unsigned char * +		[413]	0x04104ab9 "You can't
spend more than 50 points."	unsigned char * +		[414]
0x04104ae3 "This place is open from %d%s to %d%s"	unsigned char * +
[415]	0x04104b0d "January"	unsigned char * +		[416]
0x04104b1a "February"	unsigned char * +		[417]	0x04104b28
"March"	unsigned char * +		[418]	0x04104b33 "April"
unsigned char * +		[419]	0x04104b3e "May"	unsigned char *
+		[420]	0x04104b47 "June"	unsigned char *
+		[421]	0x04104b51 "July"	unsigned char *
+		[422]	0x04104b5b "August"	unsigned char *
+		[423]	0x04104b67 "September"	unsigned char *
+		[424]	0x04104b76 "October"	unsigned char *
+		[425]	0x04104b83 "November"	unsigned char *
+		[426]	0x04104b91 "December"	unsigned char *

+		[428]	0x04104bc0 "Spell failed"	unsigned char *

+		[430]	0x04104be1 "%s is now Level %lu and has earned %lu Skill
Points!"	unsigned char *


+		[435]	0x04104c4a "Converse with %s"	unsigned char *
+		[436]	0x04104c60 "Minutes"	unsigned char *
+		[437]	0x04104c6d "Minute"	unsigned char *
+		[438]	0x04104c79 "Seconds"	unsigned char *
+		[439]	0x04104c86 "Second"	unsigned char *
+		[440]	0x04104c92 "Heroism"	unsigned char *
+		[441]	0x04104c9f "Haste"	unsigned char *
+		[442]	0x04104caa "Stoneskin"	unsigned char *
+		[443]	0x04104cb9 "Bless"	unsigned char *
+		[444]	0x04104cc4 "Ooops!"	unsigned char *
+		[445]	0x04104cd0 "Lucky Day"	unsigned char *
+		[446]	0x04104cdf "Identify Failed"	unsigned char *
+		[447]	0x04104cf4 "Precision"	unsigned char *
+		[448]	0x04104d03 "Repair Failed"	unsigned char *
+		[449]	0x04104d16 "Power"	unsigned char *
+		[450]	0x04104d21 "Active Spells: %s"	unsigned char *
+		[451]	0x04104d38 "Active Party Spells"	unsigned char *
+		[452]	0x04104d51 "Torch Light"	unsigned char *
+		[453]	0x04104d62 "Wizard Eye"	unsigned char *
+		[454]	0x04104d72 "Feather Fall"	unsigned char *
+		[455]	0x04104d84 "Fly"	unsigned char *
+		[456]	0x04104d8d "Water Walk"	unsigned char *
+		[457]	0x04104d9d "Guardian"	unsigned char *
+		[458]	0x04104dab "Prot Fire"	unsigned char *
+		[459]	0x04104dba "Prot Elec"	unsigned char *
+		[460]	0x04104dc9 "Prot Cold"	unsigned char *
+		[461]	0x04104dd8 "Prot Poison"	unsigned char *
+		[462]	0x04104de9 "Prot Magic"	unsigned char *
+		[463]	0x04104df9 "Type: %s"	unsigned char *
+		[464]	0x04104e07 "Charges"	unsigned char *
+		[465]	0x04104e14 "Value"	unsigned char *
+		[466]	0x04104e1f "You found %lu gold (followers take %lu)!"
unsigned char * +		[467]	0x04104e4d "You found %lu gold!"
unsigned char * +		[468]	0x04104e66 "Clicking here will spend %d
Skill Points"	unsigned char * +		[469]	0x04104e94 "You need %d
more Skill Points to advance here"	unsigned char * +		[470]
0x04104ec7 "Get %s"	unsigned char * +		[471]	0x04104ed3 "You
found an item (%s)!"	unsigned char * +		[472]	0x04104ef0 "am"
unsigned char * +		[473]	0x04104ef8 "pm"	unsigned char * +
[474]	0x04104f00 "Recall to %s"	unsigned char * +		[475]
0x04104f12 "Set %s over %s"	unsigned char * +		[476]
0x04104f26 "Set to %s"	unsigned char * +		[477]	0x04104f35 "You
are already resting!"	unsigned char * +		[478]	0x04104f53 "You
can't rest in turn-based mode!"	unsigned char * +		[479]
0x04104f7b "You can't rest here!"	unsigned char * +		[480]
0x04104f95 "There are hostile enemies near!"	unsigned char * +
[481]	0x04104fba "Encounter!"	unsigned char * +		[482]
0x04104fca "You don't have enough food to rest"	unsigned char * +
[483]	0x04104ff2 "Set %s as the Ready Spell"	unsigned char * +
[484]	0x04105011 "Select a spell then click here to set a QuickSpell" unsigned
char * +		[485]	0x04105049 "Cast %s"	unsigned char * +
[486]	0x04105056 "Select %s"	unsigned char * +		[487]
0x04105065 "You have already mastered this skill!"	unsigned char * +
[488]	0x04105090 "You don't have enough skill points!"	unsigned char *
+		[489]	0x041050b9 ""You have %d total gold, %d in the Bank""
unsigned char * +		[490]	0x041050e7 "You found %d gold and an
item (%s)!"	unsigned char * +		[491]	0x04105110 "Can't cast
Meteor Shower indoors!"	unsigned char * +		[492]	0x04105137
"Can't cast Inferno outdoors!"	unsigned char * +		[493]
0x04105159 "Can't cast Jump while airborne! "	unsigned char * +
[494]	0x0410517f "Can not cast Fly indoors!"	unsigned char * +
[495]	0x0410519e "Can't cast Starburst indoors!"	unsigned char * +
[496]	0x041051c1 "No valid target exists! "	unsigned char * +
[497]	0x041051df "Can't cast Prismatic Light outdoors!"	unsigned char *
+		[498]	0x04105209 "Herbalist"	unsigned char *
+		[499]	0x04105218 "Can't cast Armageddon indoors!" unsigned
char * +		[500]	0x0410523c "You have %lu gold"	unsigned char *
+		[501]	0x04105253 "You have %lu food"	unsigned char *
+		[502]	0x0410526a "You find %lu food"	unsigned char *
+		[503]	0x04105281 "You lose %lu gold"	unsigned char *
+		[504]	0x04105298 "You lose %lu food"	unsigned char *

+		[506]	0x041052bf "Roderick"	unsigned char *
+		[507]	0x041052cd "Alexis"	unsigned char *
+		[508]	0x041052d9 "Serena"	unsigned char *
+		[509]	0x041052e5 "Zoltan"	unsigned char *

+		[510]	0x041052f1 "Saintly"	unsigned char *
+		[511]	0x041052fe "Angelic"	unsigned char *
+		[512]	0x0410530b "Glorious"	unsigned char *
+		[513]	0x04105319 "Honorable"	unsigned char *
+		[514]	0x04105328 "Respectable"	unsigned char *
+		[515]	0x04105339 "Average"	unsigned char *
+		[516]	0x04105346 "Bad"	unsigned char *
+		[517]	0x0410534f "Vile"	unsigned char *
+		[518]	0x04105359 "Despicable"	unsigned char *
+		[519]	0x04105369 "Monstrous"	unsigned char *
+		[520]	0x04105378 "Notorious"	unsigned char *



+		[524]	0x041053b9 "Once again you've cheated death! …" unsigned
char * +		[525]	0x041053e1 "Apothecary"	unsigned char *

+		[527]	0x041053fb "Thank You!"	unsigned char *
+		[528]	0x0410540b "I can offer you nothing further." unsigned
char *
+		[529]	0x04105431 ""Sorry, but we are unable to train you.""
unsigned char * +		[530]	0x0410545f "Moon"	unsigned char *
+		[531]	0x04105469 "Location"	unsigned char *
+		[532]	0x04105477 "Please try back in "	unsigned char *
+		[533]	0x04105490 ""I cannot join you, you're party is full""
unsigned char * +		[534]	0x041054bf "Become %s in %s for %lu
gold"	unsigned char * +		[535]	0x041054e1 "Learn"
unsigned char * +		[536]	0x041054ec ""With your skills, you
should be working here as a teacher.""	unsigned char * +		[537]
0x0410552e "Train to level %d for %d gold"	unsigned char *

+		[539]	0x04105587 "Buy Items"	unsigned char *
+		[540]	0x04105596 "Sell Items"	unsigned char *

+		[542]	0x041055ba "Repair Items"	unsigned char *
+		[543]	0x041055cc "Special Items"	unsigned char *
+		[544]	0x041055df "Seek knowledge elsewhere %s the %s" unsigned
char * +		[545]	0x04105607 "Castle Ironfist"	unsigned char *
+		[546]	0x0410561c "New Sorpigal"	unsigned char *
+		[547]	0x0410562e "Free Haven"	unsigned char *
+		[548]	0x0410563e "Arena"	unsigned char *
+		[549]	0x04105649 "Blackshire"	unsigned char *
+		[550]	0x04105659 "Kriegspire"	unsigned char *
+		[551]	0x04105669 "White Cap"	unsigned char *
+		[552]	0x04105678 "Silver Cove"	unsigned char *
+		[553]	0x04105689 "Darkmoor"	unsigned char *
+		[554]	0x04105697 "Mist"	unsigned char *
+		[555]	0x041056a1 "Bootleg Bay West"	unsigned char *
+		[556]	0x041056b7 "Volcano"	unsigned char *
+		[557]	0x041056c4 "Hermit's Isle"	unsigned char *
+		[558]	0x041056d7 "Child"	unsigned char *
+		[559]	0x041056e2 "Island North"	unsigned char *
+		[560]	0x041056f4 "Island South"	unsigned char *
+		[561]	0x04105706 ""Sorry, come back another day"" unsigned
char * +		[562]	0x0410572a "do anything"	unsigned char *
+		[563]	0x0410573b "Pack is Full!"	unsigned char *
+		[564]	0x0410574e "Hic..."	unsigned char *
+		[565]	0x0410575a "Have a Drink first..."	unsigned char *

+		[568]	0x0410578a "Club"	unsigned char *
+		[569]	0x04105794 "Done!"	unsigned char *
+		[570]	0x0410579f "Good as New!"	unsigned char *
+		[571]	0x041057b1 "Ready Spell: %s"	unsigned char *
+		[572]	0x041057c6 "Scroll Left"	unsigned char *
+		[573]	0x041057d7 "Scroll Right"	unsigned char *
+		[574]	0x041057e9 ""Welcome to the Arena of Life and Death.
Remember, you are only allowed one arena combat per visit.  To fight an arena
battle, select the option that best describes your abilities and return to me-
if you survive…""	unsigned char * +		[575]	0x041058c6
"Please wait while I summon the monsters.  Good luck."	unsigned char * +
[576]	0x04105900 "Congratulations on your win… here's your stuff… %u gold."
unsigned char * +		[577]	0x0410593e "Get back in there you
wimps…"	unsigned char * +		[578]	0x04105960 "Page"
unsigned char * +		[579]	0x0410596a "Squire"	unsigned char *
+		[580]	0x04105976 "Knight"	unsigned char *
+		[581]	0x04105982 "Lord"	unsigned char *
+		[582]	0x0410598c "You already won this trip to the Arena…"
unsigned char *

+		[584]	0x041059d5 "Click here to remove your Quick Spell"
unsigned char * +		[585]	0x04105a00 "Item is not of high enough quality"	unsigned char * +		[586]	0x04105a28 "Not enough
spell points"	unsigned char * +		[587]	0x04105a45 "Attack
Bonus"	unsigned char * +		[588]	0x04105a57 "Attack Damage"
unsigned char * +		[589]	0x04105a6a "Shoot Bonus"
unsigned char * +		[590]	0x04105a7b "Shoot Damage"
unsigned char * +		[591]	0x04105a8d "Charmed"	unsigned char *
+		[592]	0x04105a9a "Shrunk"	unsigned char *
+		[593]	0x04105aa6 "Slowed"	unsigned char *
+		[594]	0x04105ab2 "Feebleminded"	unsigned char *
+		[595]	0x04105ac4 "Wand"	unsigned char *
+		[596]	0x04105ace "Chaplain"	unsigned char *
+		[597]	0x04105adc "Diplomat"	unsigned char *
+		[598]	0x04105aea "Fallen Wizard"	unsigned char *
+		[599]	0x04105afd "Prelate"	unsigned char *
+		[600]	0x04105b0a "Sage"	unsigned char *
+		[601]	0x04105b14 "Zombie"	unsigned char *
+		[602]	0x04105b20 "History"	unsigned char *
+		[603]	0x04105b2d "Pay Fine"	unsigned char *
+		[604]	0x04105b3b "Bounty Hunt"	unsigned char *
+		[605]	0x04105b4c "Current Fine"	unsigned char *
+		[606]	0x04105b5e "Pay"	unsigned char *
+		[607]	0x04105b67 "Enslaved"	unsigned char *
+		[608]	0x04105b75 "Berserk"	unsigned char *
+		[609]	0x04105b82 "Hour of Power"	unsigned char *
+		[610]	0x04105b95 "Day of Protection"	unsigned char *
+		[611]	0x04105bac "Play ArcoMage"	unsigned char *
+		[612]	0x04105bbf "Save game corrupted!  Code=%d"
unsigned char *

+		[614]	0x04105bf4 "New Game"	unsigned char *
+		[615]	0x04105c02 "Save Game"	unsigned char *
+		[616]	0x04105c11 "Load Game"	unsigned char *
+		[617]	0x04105c20 ""Sound, Keyboard, Game Options…"" unsigned
char * +		[618]	0x04105c46 "Quit"	unsigned char * +
[619]	0x04105c50 "Return to Game"	unsigned char * +		[620]
0x04105c64 "Rules"	unsigned char * +		[621]	0x04105c6f
"Play"	unsigned char * +		[622]	0x04105c79 "Victory Conditions"
unsigned char *

+		[624]	0x04105c9c "Physical"	unsigned char *
+		[625]	0x04105caa "Immune"	unsigned char *
+		[626]	0x04105cb6 "Resistances"	unsigned char *
+		[627]	0x04105cc7 "Resistant"	unsigned char *
+		[628]	0x04105cd6 "Spell"	unsigned char *
+		[629]	0x04105ce1 "Spells"	unsigned char *
+		[630]	0x04105ced "?"	unsigned char *
+		[631]	0x04105cf4 "Effects"	unsigned char *
+		[632]	0x04105d01 "This skill level can not be learned by the
%s class."	unsigned char * +		[633]	0x04105d3b "You have to
be promoted to %s to learn this skill level."	unsigned char * +
[634]	0x04105d79 "You have to be promoted to %s or %s to learn this skill
level."	unsigned char * +		[635]	0x04105dbd "%s stuns %s"
unsigned char * +		[636]	0x04105dce "%s paralyzes %s" unsigned
char * +		[637]	0x04105de3 "%s evades damage"	unsigned char *
+		[638]	0x04105df9 "There are hostile creatures nearby!"
unsigned char * +		[639]	0x04105e22 "A tie!"	unsigned char *
+		[640]	0x04105e2e "You won!"	unsigned char *
+		[641]	0x04105e3c "You lost!"	unsigned char *
+		[642]	0x04105e4b "Error"	unsigned char *
+		[643]	0x04105e56 "by a Tower Building Victory!"
unsigned char * +		[644]	0x04105e78 "by Wall Building due to a
Tower Building Tie!"	unsigned char * +		[645]	0x04105eab "by a
Tower Destruction Victory!"	unsigned char * +		[646]
0x04105ed0 "by a Resource Victory!"	unsigned char * +		[647]
0x04105eec "by a Resource Victory due to a Tower & Wall Building Tie!" unsigned
char *
+		[648]	0x04105f2b "This character can't summon any more
monsters!"	unsigned char * +		[649]	0x04105f5f "Summoned"
unsigned char * +		[650]	0x04105f6d "Current Hit Points" unsigned
char * +		[651]	0x04105f85 "Hardened"	unsigned char * +
[652]	0x04105f93 "You can not do that while you are underwater!"
unsigned char * +		[653]	0x04105fc6 "Food"	unsigned char *
+		[654]	0x04105fd0 "%s's Jar"	unsigned char *
+		[655]	0x04105fde "%s' Jar"	unsigned char *

+		[657]	0x04105ffc "Water Breathing"	unsigned char *
+		[658]	0x04106011 "Collect Prize"	unsigned char *
+		[659]	0x04106024 "x: %d  y: %d"	unsigned char *
+		[660]	0x04106036 "You are drowning!"	unsigned char *
+		[661]	0x0410604d "You are burning!"	unsigned char *
+		[662]	0x04106063 "Instructors"	unsigned char *
+		[663]	0x04106074 "It will take %d day to cross to %s."
unsigned char * +		[664]	0x0410609d "Click here to accept this
party and continue to the game."	unsigned char * +		[665]
0x041060dc "Ok Button"	unsigned char * +		[666]	0x041060eb
"Clears all party stats and skills."	unsigned char * +		[667]
0x04106113 "Clear Button"	unsigned char * +		[668]
0x04106125 "Subtract"	unsigned char * +		[669]	0x04106133
""Subtracts a point from the highlighted skill, returning it to the bonus pool""
unsigned char * +		[670]	0x04106187 "Add"	unsigned char *
+		[671]	0x04106190 ""Adds a point from the highlighted skill,
taking it from the bonus pool""	unsigned char * +		[672]
0x041061de ""For your numerous crimes and evil deeds, you have been sentenced to
one year in prison.""	unsigned char * +		[673]	0x0410623d "You
have an outstanding fine of %lu gold.  Pay your fine now or be sentenced to jail
for 1 year?"	unsigned char * +		[674]	0x041062a3 "Temp Might"
unsigned char * +		[675]	0x041062b3 ""Splendid job!  With the
activation of the Gate, a thousand worlds lie at your feet.  Perhaps on one of
them you will find the Ancients themselves, and return with the fruits their
great civilization has to offer your world and your kingdom.""	unsigned char *
+		[676]	0x041063ab ""Brilliant!  The completion of the Heavenly
Forge has provided enough Ancient weapons to crush all resistance to your plans.
Soon the world will bow to your every whim!  Still, you can't help but wonder
what was beyond the Gate the other side was trying so hard to build."" unsigned
char *
*/
