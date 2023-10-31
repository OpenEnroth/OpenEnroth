#include "Monsters.h"

#include <string>
#include <utility>

#include "Engine/ErrorHandling.h"

#include "../Tables/FrameTableInc.h"

#include "Library/Logger/Logger.h"
#include "Library/Serialization/Serialization.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String.h"
#include "Utility/Exception.h"

struct MonsterStats *pMonsterStats;
struct MonsterList *pMonsterList;

void ParseDamage(char *damage_str, uint8_t *dice_rolls,
                 uint8_t *dice_sides, uint8_t *dmg_bonus);
int ParseMissleAttackType(const char *missle_attack_str);
int ParseSpecialAttack(const char *spec_att_str);

//----- (004548E2) --------------------------------------------------------
SpellId ParseSpellType(struct FrameTableTxtLine *tbl, int *next_token) {
    if (!tbl->pProperties[0]) {
        ++*next_token;
        return SPELL_NONE;
    }
    if (iequals(tbl->pProperties[0], "Dispel")) {  // dispel magic
        ++*next_token;
        return SPELL_LIGHT_DISPEL_MAGIC;
    } else if (iequals(tbl->pProperties[0], "Day")) {  // day of protection
        *next_token += 2;
        return SPELL_LIGHT_DAY_OF_PROTECTION;
    } else if (iequals(tbl->pProperties[0], "Hour")) {  // hour  of power
        *next_token += 2;
        return SPELL_LIGHT_HOUR_OF_POWER;
    } else if (iequals(tbl->pProperties[0], "Shield")) {
        return SPELL_AIR_SHIELD;
    } else if (iequals(tbl->pProperties[0], "Spirit")) {
        ++*next_token;
        return SPELL_SPIRIT_SPIRIT_LASH;
    } else if (iequals(tbl->pProperties[0], "Power")) {  // power cure
        ++*next_token;
        return SPELL_BODY_POWER_CURE;
    } else if (iequals(tbl->pProperties[0], "Meteor")) {  // meteot shower
        ++*next_token;
        return SPELL_FIRE_METEOR_SHOWER;
    } else if (iequals(tbl->pProperties[0], "Lightning")) {  // Lightning bolt
        ++*next_token;
        return SPELL_AIR_LIGHTNING_BOLT;
    } else if (iequals(tbl->pProperties[0], "Implosion")) {
        return SPELL_AIR_IMPLOSION;
    } else if (iequals(tbl->pProperties[0], "Stone")) {
        ++*next_token;
        return SPELL_EARTH_STONESKIN;
    } else if (iequals(tbl->pProperties[0], "Haste")) {
        return SPELL_FIRE_HASTE;
    } else if (iequals(tbl->pProperties[0], "Heroism")) {
        return SPELL_SPIRIT_HEROISM;
    } else if (iequals(tbl->pProperties[0], "Pain")) {  // pain reflection
        ++*next_token;
        return SPELL_DARK_PAIN_REFLECTION;
    } else if (iequals(tbl->pProperties[0], "Sparks")) {
        return SPELL_AIR_SPARKS;
    } else if (iequals(tbl->pProperties[0], "Light")) {
        ++*next_token;
        return SPELL_LIGHT_LIGHT_BOLT;
    } else if (iequals(tbl->pProperties[0], "Toxic")) {  // toxic cloud
        ++*next_token;
        return SPELL_DARK_TOXIC_CLOUD;
    } else if (iequals(tbl->pProperties[0], "ShrapMetal")) {
        return SPELL_DARK_SHARPMETAL;
    } else if (iequals(tbl->pProperties[0], "Paralyze")) {
        return SPELL_LIGHT_PARALYZE;
    } else if (iequals(tbl->pProperties[0], "Fireball")) {
        return SPELL_FIRE_FIREBALL;
    } else if (iequals(tbl->pProperties[0], "Incinerate")) {
        return SPELL_FIRE_INCINERATE;
    } else if (iequals(tbl->pProperties[0], "Fire")) {
        ++*next_token;
        return SPELL_FIRE_FIRE_BOLT;
    } else if (iequals(tbl->pProperties[0], "Rock")) {
        ++*next_token;
        return SPELL_EARTH_ROCK_BLAST;
    } else if (iequals(tbl->pProperties[0], "Mass")) {
        ++*next_token;
        return SPELL_EARTH_MASS_DISTORTION;
    } else if (iequals(tbl->pProperties[0], "Ice")) {
        ++*next_token;
        return SPELL_WATER_ICE_BOLT;
    } else if (iequals(tbl->pProperties[0], "Acid")) {
        ++*next_token;
        return SPELL_WATER_ACID_BURST;
    } else if (iequals(tbl->pProperties[0], "Bless")) {
        return SPELL_SPIRIT_BLESS;
    } else if (iequals(tbl->pProperties[0], "Dragon")) {
        ++*next_token;
        return SPELL_DARK_DRAGON_BREATH;
    } else if (iequals(tbl->pProperties[0], "Reanimate")) {
        return SPELL_DARK_REANIMATE;
    } else if (iequals(tbl->pProperties[0], "Summon")) {
        ++*next_token;
        return SPELL_LIGHT_SUMMON_ELEMENTAL;
    } else if (iequals(tbl->pProperties[0], "Fate")) {
        return SPELL_SPIRIT_FATE;
    } else if (iequals(tbl->pProperties[0], "Harm")) {
        return SPELL_BODY_HARM;
    } else if (iequals(tbl->pProperties[0], "Mind")) {
        ++*next_token;
        return SPELL_MIND_MIND_BLAST;
    } else if (iequals(tbl->pProperties[0], "Blades")) {
        return SPELL_EARTH_BLADES;
    } else if (iequals(tbl->pProperties[0], "Psychic")) {
        ++*next_token;
        return SPELL_MIND_PSYCHIC_SHOCK;
    } else if (iequals(tbl->pProperties[0], "Hammerhands")) {
        return SPELL_BODY_HAMMERHANDS;
    } else {
        logger->warning("Unknown monster spell {}", tbl->pProperties[0]);
        ++*next_token;
        return SPELL_NONE;
    }
}

CombinedSkillValue ParseSkillValue(std::string_view skillString, std::string_view masteryString) {
    int skill;
    if (!tryDeserialize(skillString, &skill))
        return CombinedSkillValue::none(); // TODO(captainurist): this does happen, investigate.

    CharacterSkillMastery mastery;
    if (masteryString == "N") {
        mastery = CHARACTER_SKILL_MASTERY_NOVICE;
    } else if (masteryString == "E") {
        mastery = CHARACTER_SKILL_MASTERY_EXPERT;
    } else if (masteryString == "M") {
        mastery = CHARACTER_SKILL_MASTERY_MASTER;
    } else if (masteryString == "G") {
        mastery = CHARACTER_SKILL_MASTERY_GRANDMASTER;
    } else {
        throw Exception("Invalid character skill mastery string '{}'", masteryString);
    }

    return CombinedSkillValue(skill, mastery);
}

//----- (00454CB4) --------------------------------------------------------
static DamageType ParseAttackType(const char *damage_type_str) {
    switch (tolower(*damage_type_str)) {
        case 'f':
            return DAMAGE_FIRE;  // fire
        case 'a':
            return DAMAGE_AIR;  // air
        case 'w':
            return DAMAGE_WATER;  // water
        case 'e':
            return DAMAGE_EARTH;  // earth

        case 's':
            return DAMAGE_SPIRIT;  // spirit
        case 'm':
            return DAMAGE_MIND;  // mind

            // TODO(captainurist): where is DAMAGE_BODY?

        case 'l':
            return DAMAGE_LIGHT;  // light
        case 'd':
            return DAMAGE_DARK;  // dark
    }
    return DAMAGE_PHYSICAL;  // phis
}

//----- (00454D7D) --------------------------------------------------------
void ParseDamage(char *damage_str, uint8_t *dice_rolls,
                 uint8_t *dice_sides, uint8_t *dmg_bonus) {
    int str_len = 0;
    int str_pos = 0;
    bool dice_flag = false;

    *dice_rolls = 0;
    *dice_sides = 1;
    *dmg_bonus = 0;

    str_len = strlen(damage_str);
    if (str_len <= 0) return;
    for (str_pos = 0; str_pos < str_len; ++str_pos) {
        if (tolower(damage_str[str_pos]) == 'd') {
            damage_str[str_pos] = '\0';
            *dice_rolls = atoi(damage_str);
            *dice_sides = atoi(&damage_str[str_pos + 1]);
            dice_flag = true;
            damage_str[str_pos] = 'd';
        } else if (tolower(damage_str[str_pos]) == '+') {
            *dmg_bonus = atoi(&damage_str[str_pos + 1]);
        }
    }
    if (!dice_flag) {
        if ((*damage_str >= '0') && (*damage_str <= '9')) {
            *dice_rolls = atoi(damage_str);
            *dice_sides = 1;
        }
    }
}

//----- (00454E3A) --------------------------------------------------------
int ParseMissleAttackType(const char *missle_attack_str) {
    if (iequals(missle_attack_str, "ARROW"))
        return 1;
    else if (iequals(missle_attack_str, "ARROWF"))
        return 2;
    else if (iequals(missle_attack_str, "FIRE"))
        return 3;
    else if (iequals(missle_attack_str, "AIR"))
        return 4;
    else if (iequals(missle_attack_str, "WATER"))
        return 5;
    else if (iequals(missle_attack_str, "EARTH"))
        return 6;
    else if (iequals(missle_attack_str, "SPIRIT"))
        return 7;
    else if (iequals(missle_attack_str, "MIND"))
        return 8;
    else if (iequals(missle_attack_str, "BODY"))
        return 9;
    else if (iequals(missle_attack_str, "LIGHT"))
        return 10;
    else if (iequals(missle_attack_str, "DARK"))
        return 11;
    else if (iequals(missle_attack_str, "ENER"))
        return 13;
    else
        return 0;
}

int ParseSpecialAttack(char *spec_att_str) {
    std::string tmp = toLower(spec_att_str);

    // TODO(captainurist): we're getting strings like "Disease1" here, and they are not handled by the code below.

    if (tmp.starts_with("curse"))
        return 1;
    else if (tmp.starts_with("weak"))
        return 2;
    else if (tmp.starts_with("asleep"))
        return 3;
    else if (tmp.starts_with("afraid"))
        return 23;
    else if (tmp.starts_with("drunk"))
        return 4;
    else if (tmp.starts_with("insane"))
        return 5;
    else if (tmp.starts_with("poison weak"))
        return 6;
    else if (tmp.starts_with("poison medium"))
        return 7;
    else if (tmp.starts_with("poison severe"))
        return 8;
    else if (tmp.starts_with("disease weak"))
        return 9;
    else if (tmp.starts_with("disease medium"))
        return 10;
    else if (tmp.starts_with("disease severe"))
        return 11;
    else if (tmp.starts_with("paralyze"))
        return 12;
    else if (tmp.starts_with("uncon"))
        return 13;
    else if (tmp.starts_with("dead"))
        return 14;
    else if (tmp.starts_with("stone"))
        return 15;
    else if (tmp.starts_with("errad"))
        return 16;
    else if (tmp.starts_with("brkitem"))
        return 17;
    else if (tmp.starts_with("brkarmor"))
        return 18;
    else if (tmp.starts_with("brkweapon"))
        return 19;
    else if (tmp.starts_with("steal"))
        return 20;
    else if (tmp.starts_with("age"))
        return 21;
    else if (tmp.starts_with("drainsp"))
        return 22;
    else
        return 0;
}

//----- (004563FF) --------------------------------------------------------
MonsterId MonsterStats::FindMonsterByTextureName(const std::string &monster_textr_name) {
    for (MonsterId i : pInfos.indices()) {
        if (!pInfos[i].pName.empty() && iequals(pInfos[i].pPictureName, monster_textr_name))
            return i;
    }
    return MONSTER_INVALID;
}

//----- (00454F4E) --------------------------------------------------------
void MonsterStats::InitializePlacements(const Blob &placements) {
    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;
    //  int item_counter;

    std::string txtRaw(placements.string_view());
    strtok(txtRaw.data(), "\r");
    for (i = 1; i < 31; ++i) {
        test_string = strtok(NULL, "\r") + 1;
        break_loop = false;
        decode_step = 0;
        do {
            c = *(unsigned char *)test_string;
            temp_str_len = 0;
            while ((c != '\t') && (c > 0)) {
                ++temp_str_len;
                c = test_string[temp_str_len];
            }
            tmp_pos = test_string + temp_str_len;
            if (*tmp_pos == 0) break_loop = true;
            *tmp_pos = 0;
            if (temp_str_len) {
                if (decode_step == 1)
                    pUniqueNames[i] = removeQuotes(test_string);
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 3) && !break_loop);
    }
}

//----- (0045501E) --------------------------------------------------------
void MonsterStats::Initialize(const Blob &monsters) {
    int i;  // ,j;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;
    //    int item_counter;
    MonsterId curr_rec_num;
    char parse_str[64];
    // char Src[120];
    FrameTableTxtLine parsed_field;
    std::string str;

    std::string txtRaw(monsters.string_view());
    strtok(txtRaw.data(), "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    curr_rec_num = MONSTER_INVALID;
    for (i = 0; i < 264; ++i) { // TODO(captainurist): get rid of magic numbers in txt deserialization.
        test_string = strtok(NULL, "\r") + 1;
        break_loop = false;
        decode_step = 0;
        do {
            c = *(unsigned char *)test_string;
            temp_str_len = 0;
            while ((c != '\t') && (c > 0)) {
                ++temp_str_len;
                c = test_string[temp_str_len];
            }
            tmp_pos = test_string + temp_str_len;
            if (*tmp_pos == 0) break_loop = true;
            *tmp_pos = 0;
            if (temp_str_len) {
                switch (decode_step) {
                    case 0:
                        curr_rec_num = static_cast<MonsterId>(atoi(test_string));
                        pInfos[curr_rec_num].uID = curr_rec_num;
                        break;
                    case 1:
                        pInfos[curr_rec_num].pName = removeQuotes(test_string);
                        break;
                    case 2:
                        pInfos[curr_rec_num].pPictureName = removeQuotes(test_string);
                        break;
                    case 3:
                        pInfos[curr_rec_num].uLevel = atoi(test_string);
                        break;
                    case 4: {
                        int str_len = 0;
                        int str_pos = 0;
                        pInfos[curr_rec_num].uHP = 0;
                        if (test_string[0] == '"') test_string[0] = ' ';
                        str_len = strlen(test_string);
                        if (str_len == 0) break;
                        while ((test_string[str_pos] != ',') &&
                               (str_pos < str_len))
                            ++str_pos;
                        if (str_len == str_pos) {
                            pInfos[curr_rec_num].uHP = atoi(test_string);
                        } else {
                            test_string[str_pos] = '\0';
                            pInfos[curr_rec_num].uHP = 1000 * atoi(test_string);
                            pInfos[curr_rec_num].uHP +=
                                atoi(&test_string[str_pos + 1]);
                            test_string[str_pos] = ',';
                        }
                    } break;
                    case 5:
                        pInfos[curr_rec_num].uAC = atoi(test_string);
                        break;
                    case 6: {
                        int str_len = 0;
                        int str_pos = 0;
                        pInfos[curr_rec_num].uExp = 0;
                        if (test_string[0] == '"') test_string[0] = ' ';
                        str_len = strlen(test_string);
                        if (str_len == 0) break;
                        while ((test_string[str_pos] != ',') &&
                               (str_pos < str_len))
                            ++str_pos;
                        if (str_len == str_pos) {
                            pInfos[curr_rec_num].uExp = atoi(test_string);
                        } else {
                            test_string[str_pos] = '\0';
                            pInfos[curr_rec_num].uExp =
                                1000 * atoi(test_string);
                            pInfos[curr_rec_num].uExp +=
                                atoi(&test_string[str_pos + 1]);
                            test_string[str_pos] = ',';
                        }
                    } break;
                    case 7: {
                        int str_len = 0;
                        int str_pos = 0;
                        bool chance_flag = false;
                        bool dice_flag = false;
                        bool item_type_flag = false;
                        char *item_name;
                        pInfos[curr_rec_num].uTreasureDropChance = 0;
                        pInfos[curr_rec_num].uTreasureDiceRolls = 0;
                        pInfos[curr_rec_num].uTreasureDiceSides = 0;
                        pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_ANY;
                        pInfos[curr_rec_num].uTreasureLevel = ITEM_TREASURE_LEVEL_INVALID;
                        if (test_string[0] == '"') test_string[0] = ' ';
                        str_len = strlen(test_string);
                        do {
                            switch (tolower(test_string[str_pos])) {
                                case '%':
                                    chance_flag = true;
                                    break;
                                case 'd':
                                    dice_flag = true;
                                    break;
                                case 'l':
                                    item_type_flag = true;
                                    break;
                            }
                            ++str_pos;
                        } while (str_pos < str_len);
                        if (chance_flag) {
                            pInfos[curr_rec_num].uTreasureDropChance =
                                atoi(test_string);
                        } else {
                            if ((!dice_flag) && (!item_type_flag)) break;
                            pInfos[curr_rec_num].uTreasureDropChance = 100;
                        }
                        if (dice_flag) {
                            str_pos = 0;
                            dice_flag = false;
                            do {
                                switch (tolower(test_string[str_pos])) {
                                    case '%':
                                        pInfos[curr_rec_num]
                                            .uTreasureDiceRolls =
                                            atoi(&test_string[str_pos + 1]);
                                        dice_flag = true;
                                        break;
                                    case 'd':
                                        if (!dice_flag)
                                            pInfos[curr_rec_num]
                                                .uTreasureDiceRolls =
                                                atoi(test_string);
                                        pInfos[curr_rec_num]
                                            .uTreasureDiceSides =
                                            atoi(&test_string[str_pos + 1]);
                                        str_pos = str_len;
                                        break;
                                }
                                ++str_pos;
                            } while (str_pos < str_len);
                        }
                        if (item_type_flag) {
                            str_pos = 0;
                            do {
                                if (tolower(test_string[str_pos]) == 'l') break;
                                ++str_pos;
                            } while (str_pos < str_len);

                            pInfos[curr_rec_num].uTreasureLevel =
                                ItemTreasureLevel(test_string[str_pos + 1] - '0');
                            item_name = &test_string[str_pos + 2];
                            if (*item_name) {
                                if (iequals(item_name, "WEAPON"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_WEAPON;
                                else if (iequals(item_name, "ARMOR"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_ARMOR;
                                else if (iequals(item_name, "MISC"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_MICS;
                                else if (iequals(item_name, "SWORD"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_SWORD;
                                else if (iequals(item_name, "DAGGER"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_DAGGER;
                                else if (iequals(item_name, "AXE"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_AXE;
                                else if (iequals(item_name, "SPEAR"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_SPEAR;
                                else if (iequals(item_name, "BOW"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_BOW;
                                else if (iequals(item_name, "MACE"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_MACE;
                                else if (iequals(item_name, "CLUB"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_CLUB;
                                else if (iequals(item_name, "STAFF"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_STAFF;
                                else if (iequals(item_name, "LEATHER"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_LEATHER_ARMOR;
                                else if (iequals(item_name, "CHAIN"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_CHAIN_ARMOR;
                                else if (iequals(item_name, "PLATE"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_PLATE_ARMOR;
                                else if (iequals(item_name, "SHIELD"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_SHIELD;
                                else if (iequals(item_name, "HELM"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_HELMET;
                                else if (iequals(item_name, "BELT"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_BELT;
                                else if (iequals(item_name, "CAPE"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_CLOAK;
                                else if (iequals(item_name, "GAUNTLETS"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_GAUNTLETS;
                                else if (iequals(item_name, "BOOTS"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_BOOTS;
                                else if (iequals(item_name, "RING"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_RING;
                                else if (iequals(item_name, "AMULET"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_AMULET;
                                else if (iequals(item_name, "WAND"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_WAND;
                                else if (iequals(item_name, "SCROLL"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_SPELL_SCROLL;
                                else if (iequals(item_name, "GEM"))
                                    pInfos[curr_rec_num].uTreasureType = RANDOM_ITEM_GEM;
                            }
                        }
                    } break;
                    case 8: {
                        pInfos[curr_rec_num].bBloodSplatOnDeath = false;
                        if (atoi(test_string))
                            pInfos[curr_rec_num].bBloodSplatOnDeath = true;
                    } break;
                    case 9: {
                        pInfos[curr_rec_num].uFlying = false;
                        if (!iequals(test_string, "n")) // "Y"/"N"
                            pInfos[curr_rec_num].uFlying = true;
                    } break;
                    case 10: {
                        switch (tolower(test_string[0])) {
                            case 's':
                                pInfos[curr_rec_num].uMovementType = MONSTER_MOVEMENT_TYPE_SHORT;  // short
                                if (tolower(test_string[1]) != 'h')
                                    pInfos[curr_rec_num].uMovementType = MONSTER_MOVEMENT_TYPE_STATIONARY;  // stationary
                                break;  // short
                            case 'l':
                                pInfos[curr_rec_num].uMovementType = MONSTER_MOVEMENT_TYPE_LONG;
                                break;  // long
                            case 'm':
                                pInfos[curr_rec_num].uMovementType = MONSTER_MOVEMENT_TYPE_MEDIUM;
                                break;  // med
                            case 'g':
                                pInfos[curr_rec_num].uMovementType = MONSTER_MOVEMENT_TYPE_GLOBAL;
                                break;  // global?
                            default:
                                pInfos[curr_rec_num].uMovementType = MONSTER_MOVEMENT_TYPE_FREE;  // free
                        }
                    } break;
                    case 11: {
                        switch (tolower(test_string[0])) {
                            case 's':
                                pInfos[curr_rec_num].uAIType = MONSTER_AI_SUICIDE;
                                break;
                            case 'w':
                                pInfos[curr_rec_num].uAIType = MONSTER_AI_WIMP;
                                break;
                            case 'n':
                                pInfos[curr_rec_num].uAIType = MONSTER_AI_NORMAL;
                                break;
                            default:
                                pInfos[curr_rec_num].uAIType = MONSTER_AI_AGGRESSIVE;
                        }
                    } break;
                    case 12:
                        pInfos[curr_rec_num].uHostilityType =
                            (MonsterHostility)atoi(test_string);
                        break;
                    case 13:
                        pInfos[curr_rec_num].uBaseSpeed = atoi(test_string);
                        break;
                    case 14:
                        pInfos[curr_rec_num].uRecoveryTime = atoi(test_string);
                        break;
                    case 15: {
                        int str_len = 0;
                        int str_pos = 0;
                        pInfos[curr_rec_num].uAttackPreferences = 0;
                        pInfos[curr_rec_num]
                            .uNumCharactersAttackedPerSpecialAbility = 0;
                        str_len = strlen(test_string);
                        for (str_pos = 0; str_pos < str_len; ++str_pos) {
                            switch (tolower(test_string[str_pos])) {
                                case '0':
                                    // TODO(captainurist): '0' means archer? Why???
                                    pInfos[curr_rec_num].uAttackPreferences |=
                                        ATTACK_PREFERENCE_ARCHER;
                                    break;
                                case '2':
                                    pInfos[curr_rec_num]
                                        .uNumCharactersAttackedPerSpecialAbility =
                                        2;
                                    break;
                                case '3':
                                    pInfos[curr_rec_num]
                                        .uNumCharactersAttackedPerSpecialAbility =
                                        3;
                                    break;
                                case '4':
                                    pInfos[curr_rec_num]
                                        .uNumCharactersAttackedPerSpecialAbility =
                                        4;
                                    break;
                                case 'c':
                                    pInfos[curr_rec_num].uAttackPreferences |=
                                        ATTACK_PREFERENCE_CLERIC;
                                    break;
                                case 'd':
                                    pInfos[curr_rec_num].uAttackPreferences |=
                                        ATTACK_PREFERENCE_DRUID;
                                    break;
                                case 'e':
                                    pInfos[curr_rec_num].uAttackPreferences |=
                                        ATTACK_PREFERENCE_ELF;
                                    break;
                                case 'f':
                                    pInfos[curr_rec_num].uAttackPreferences |=
                                        ATTACK_PREFERENCE_FEMALE;
                                    break;
                                case 'h':
                                    pInfos[curr_rec_num].uAttackPreferences |=
                                        ATTACK_PREFERENCE_HUMAN;
                                    break;
                                case 'k':
                                    pInfos[curr_rec_num].uAttackPreferences |=
                                        ATTACK_PREFERENCE_KNIGHT;
                                    break;
                                case 'm':
                                    pInfos[curr_rec_num].uAttackPreferences |=
                                        ATTACK_PREFERENCE_MONK;
                                    break;
                                case 'o':
                                    // TODO(captainurist): both 'f' and 'o' are ATTACK_PREFERENCE_FEMALE?
                                    pInfos[curr_rec_num].uAttackPreferences |=
                                        ATTACK_PREFERENCE_FEMALE;
                                    break;
                                case 'p':
                                    pInfos[curr_rec_num].uAttackPreferences |=
                                        ATTACK_PREFERENCE_PALADIN;
                                    break;
                                case 'r':
                                    pInfos[curr_rec_num].uAttackPreferences |=
                                        ATTACK_PREFERENCE_RANGER;
                                    break;
                                case 's':
                                    pInfos[curr_rec_num].uAttackPreferences |=
                                        ATTACK_PREFERENCE_SORCERER;
                                    break;
                                case 't':
                                    pInfos[curr_rec_num].uAttackPreferences |=
                                        ATTACK_PREFERENCE_THIEF;
                                    break;
                                case 'w':
                                    pInfos[curr_rec_num].uAttackPreferences |=
                                        ATTACK_PREFERENCE_DWARF;
                                    break;
                                case 'x':
                                    pInfos[curr_rec_num].uAttackPreferences |=
                                        ATTACK_PREFERENCE_MALE;
                                    break;
                            }
                        }
                    } break;
                    case 16: {
                        int str_len = 0;
                        int str_pos = 0;
                        pInfos[curr_rec_num].uSpecialAttackLevel = 1;
                        pInfos[curr_rec_num].uSpecialAttackType =
                            (SPECIAL_ATTACK_TYPE)0;
                        str_len = strlen(test_string);
                        if (str_len > 1) {
                            for (str_pos = 0; str_pos < str_len; ++str_pos) {
                                if (tolower(test_string[str_pos]) == 'x') {
                                    test_string[str_pos] = '\0';
                                    pInfos[curr_rec_num].uSpecialAttackLevel =
                                        atoi(&test_string[str_pos + 1]);
                                    test_string[str_pos] = 'x';
                                    break;
                                }
                            }
                            pInfos[curr_rec_num].uSpecialAttackType =
                                (SPECIAL_ATTACK_TYPE)ParseSpecialAttack(
                                    test_string);
                        }
                    } break;
                    case 17:
                        pInfos[curr_rec_num].uAttack1Type = ParseAttackType(test_string);
                        break;
                    case 18: {
                        ParseDamage(
                            test_string,
                            &pInfos[curr_rec_num].uAttack1DamageDiceRolls,
                            &pInfos[curr_rec_num].uAttack1DamageDiceSides,
                            &pInfos[curr_rec_num].uAttack1DamageBonus);
                    } break;
                    case 19:
                        pInfos[curr_rec_num].uMissleAttack1Type =
                            ParseMissleAttackType(test_string);
                        break;
                    case 20:
                        pInfos[curr_rec_num].uAttack2Chance = atoi(test_string);
                        break;
                    case 21:
                        pInfos[curr_rec_num].uAttack2Type =
                            ParseAttackType(test_string);
                        break;
                    case 22: {
                        ParseDamage(
                            test_string,
                            &pInfos[curr_rec_num].uAttack2DamageDiceRolls,
                            &pInfos[curr_rec_num].uAttack2DamageDiceSides,
                            &pInfos[curr_rec_num].uAttack2DamageBonus);
                    } break;
                    case 23:
                        pInfos[curr_rec_num].uMissleAttack2Type =
                            ParseMissleAttackType(test_string);
                        break;
                    case 24:
                        pInfos[curr_rec_num].uSpell1UseChance =
                            atoi(test_string);
                        break;
                    case 25: {
                        int param_num;
                        strcpy(parse_str, test_string);
                        parse_str[0] = ' ';
                        parse_str[strlen(parse_str) - 1] = ' ';
                        frame_table_txt_parser(parse_str, &parsed_field);
                        if (parsed_field.uPropCount > 2) {
                            param_num = 1;
                            pInfos[curr_rec_num].uSpell1ID =
                                ParseSpellType(&parsed_field, &param_num);
                            pInfos[curr_rec_num].uSpellSkillAndMastery1 =
                                ParseSkillValue(parsed_field.pProperties[param_num + 1], parsed_field.pProperties[param_num]);
                        } else {
                            pInfos[curr_rec_num].uSpell1ID = SPELL_NONE;
                            pInfos[curr_rec_num].uSpellSkillAndMastery1 = CombinedSkillValue::none();
                        }
                    } break;
                    case 26:
                        pInfos[curr_rec_num].uSpell2UseChance =
                            atoi(test_string);
                        break;
                    case 27: {
                        int param_num;
                        strcpy(parse_str, test_string);
                        parse_str[0] = ' ';
                        parse_str[strlen(parse_str) - 1] = ' ';
                        frame_table_txt_parser(parse_str, &parsed_field);
                        if (parsed_field.uPropCount > 2) {
                            param_num = 1;
                            pInfos[curr_rec_num].uSpell2ID =
                                ParseSpellType(&parsed_field, &param_num);
                            pInfos[curr_rec_num].uSpellSkillAndMastery2 =
                                ParseSkillValue(parsed_field.pProperties[param_num + 1], parsed_field.pProperties[param_num]);
                        } else {
                            pInfos[curr_rec_num].uSpell2ID = SPELL_NONE;
                            pInfos[curr_rec_num].uSpellSkillAndMastery2 = CombinedSkillValue::none();
                        }
                    } break;
                    case 28: {
                        if (tolower(test_string[0]) == 'i')
                            pInfos[curr_rec_num].uResFire = 200;
                        else
                            pInfos[curr_rec_num].uResFire = atoi(test_string);
                    } break;
                    case 29: {
                        if (tolower(test_string[0]) == 'i')
                            pInfos[curr_rec_num].uResAir = 200;
                        else
                            pInfos[curr_rec_num].uResAir = atoi(test_string);
                    } break;
                    case 30: {
                        if (tolower(test_string[0]) == 'i')
                            pInfos[curr_rec_num].uResWater = 200;
                        else
                            pInfos[curr_rec_num].uResWater = atoi(test_string);
                    } break;
                    case 31: {
                        if (tolower(test_string[0]) == 'i')
                            pInfos[curr_rec_num].uResEarth = 200;
                        else
                            pInfos[curr_rec_num].uResEarth = atoi(test_string);
                    } break;
                    case 32: {
                        if (tolower(test_string[0]) == 'i')
                            pInfos[curr_rec_num].uResMind = 200;
                        else
                            pInfos[curr_rec_num].uResMind = atoi(test_string);
                    } break;
                    case 33: {
                        if (tolower(test_string[0]) == 'i')
                            pInfos[curr_rec_num].uResSpirit = 200;
                        else
                            pInfos[curr_rec_num].uResSpirit = atoi(test_string);
                    } break;
                    case 34: {
                        if (tolower(test_string[0]) == 'i')
                            pInfos[curr_rec_num].uResBody = 200;
                        else
                            pInfos[curr_rec_num].uResBody = atoi(test_string);
                    } break;
                    case 35: {
                        if (tolower(test_string[0]) == 'i')
                            pInfos[curr_rec_num].uResLight = 200;
                        else
                            pInfos[curr_rec_num].uResLight = atoi(test_string);
                    } break;
                    case 36: {
                        if (tolower(test_string[0]) == 'i')
                            pInfos[curr_rec_num].uResDark = 200;
                        else
                            pInfos[curr_rec_num].uResDark = atoi(test_string);
                    } break;
                    case 37: {
                        if (tolower(test_string[0]) == 'i')
                            pInfos[curr_rec_num].uResPhysical = 200;
                        else
                            pInfos[curr_rec_num].uResPhysical =
                                atoi(test_string);
                    } break;
                    case 38: {
                        //                    int param_num;
                        //                    char type_flag;
                        pInfos[curr_rec_num].uSpecialAbilityType = MONSTER_SPECIAL_ABILITY_NONE;
                        pInfos[curr_rec_num].uSpecialAbilityDamageDiceBonus = 0;
                        strcpy(parse_str, test_string);
                        parse_str[0] = ' ';
                        parse_str[strlen(parse_str) - 1] = ' ';
                        frame_table_txt_parser(parse_str, &parsed_field);
                        if (parsed_field.uPropCount) {
                            //      v74 = v94.field_0;
                            if (parsed_field.uPropCount < 10) {
                                if (iequals(parsed_field.pProperties[0], "shot")) {
                                    pInfos[curr_rec_num].uSpecialAbilityType = MONSTER_SPECIAL_ABILITY_SHOT;
                                    pInfos[curr_rec_num]
                                        .uSpecialAbilityDamageDiceBonus = atoi(
                                        (char *)(parsed_field.pProperties[1] +
                                                 1));
                                } else if (iequals(parsed_field.pProperties[0], "summon")) {
                                    pInfos[curr_rec_num].uSpecialAbilityType = MONSTER_SPECIAL_ABILITY_SUMMON;
                                    if (parsed_field.uPropCount > 1) {
                                        str = parsed_field.pProperties[2];
                                        if (parsed_field.uPropCount > 2) {
                                            int prop_cnt = 3;
                                            if (parsed_field.uPropCount > 3) {
                                                do {
                                                    str += " ";
                                                    char test_char =
                                                        parsed_field.pProperties
                                                            [prop_cnt][0];
                                                    str +=
                                                        parsed_field.pProperties
                                                            [prop_cnt];
                                                    if (prop_cnt ==
                                                        (parsed_field
                                                             .uPropCount -
                                                         1)) {
                                                        switch (tolower(
                                                            test_char)) {
                                                            case 'a':
                                                                pInfos[curr_rec_num]
                                                                    .uSpecialAbilityDamageDiceRolls =
                                                                    1;
                                                                break;
                                                            case 'b':
                                                                pInfos[curr_rec_num]
                                                                    .uSpecialAbilityDamageDiceRolls =
                                                                    2;
                                                                break;
                                                            case 'c':
                                                                pInfos[curr_rec_num]
                                                                    .uSpecialAbilityDamageDiceRolls =
                                                                    3;
                                                                break;
                                                            default:
                                                                pInfos[curr_rec_num]
                                                                    .uSpecialAbilityDamageDiceRolls =
                                                                    0;
                                                        }
                                                    }
                                                    ++prop_cnt;
                                                } while (
                                                    prop_cnt <
                                                    parsed_field.uPropCount);
                                            }
                                        } else {
                                            pInfos[curr_rec_num]
                                                .uSpecialAbilityDamageDiceRolls =
                                                0;
                                        }
                                        if (!pMonsterList->pMonsters.empty()) {
                                            pInfos[curr_rec_num].field_3C_some_special_attack =
                                                std::to_underlying(pMonsterList->GetMonsterIDByName(str));
                                        }
                                        pInfos[curr_rec_num]
                                            .uSpecialAbilityDamageDiceSides = 0;
                                        if (iequals(parsed_field.pProperties[1], "ground"))
                                            pInfos[curr_rec_num]
                                                .uSpecialAbilityDamageDiceSides =
                                                1;
                                        if (pInfos[curr_rec_num]
                                                .field_3C_some_special_attack ==
                                            -1)
                                            pInfos[curr_rec_num].uSpecialAbilityType = MONSTER_SPECIAL_ABILITY_NONE;
                                    }
                                } else if (iequals(parsed_field.pProperties[0], "explode")) {
                                    pInfos[curr_rec_num].uSpecialAbilityType = MONSTER_SPECIAL_ABILITY_EXPLODE;
                                    ParseDamage(
                                        (char *)parsed_field.pProperties[1],
                                        &pInfos[curr_rec_num]
                                             .uSpecialAbilityDamageDiceRolls,
                                        &pInfos[curr_rec_num]
                                             .uSpecialAbilityDamageDiceSides,
                                        &pInfos[curr_rec_num]
                                             .uSpecialAbilityDamageDiceBonus);
                                    pInfos[curr_rec_num]
                                        .field_3C_some_special_attack =
                                        std::to_underlying(ParseAttackType(test_string));
                                }
                            }
                        }
                    } break;
                }
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 39) && !break_loop);
    }
}

//----- (0044FA08) --------------------------------------------------------
MonsterId MonsterList::GetMonsterIDByName(const std::string &pMonsterName) {
    for (MonsterId i : pMonsters.indices()) {
        if (iequals(pMonsters[i].pMonsterName, pMonsterName))
            return i;
    }
    Error("Monster not found: %s", pMonsterName.c_str());
}
