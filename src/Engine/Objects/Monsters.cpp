#include "Monsters.h"

#include <cstring>
#include <string>
#include <utility>

#include "Engine/Tables/FrameTableInc.h"

#include "Library/Logger/Logger.h"
#include "Library/Serialization/Serialization.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"
#include "Utility/Exception.h"
#include "Utility/String/Transformations.h"

MonsterStats *pMonsterStats;
MonsterList *pMonsterList;

void ParseDamage(char *damage_str, uint8_t *dice_rolls,
                 uint8_t *dice_sides, uint8_t *dmg_bonus);
int ParseMissleAttackType(const char *missle_attack_str);
int ParseSpecialAttack(const char *spec_att_str);

//----- (004548E2) --------------------------------------------------------
SpellId ParseSpellType(FrameTableTxtLine *tbl, int *next_token) {
    if (!tbl->pProperties[0]) {
        ++*next_token;
        return SPELL_NONE;
    }
    if (ascii::noCaseEquals(tbl->pProperties[0], "Dispel")) {  // dispel magic
        ++*next_token;
        return SPELL_LIGHT_DISPEL_MAGIC;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Day")) {  // day of protection
        *next_token += 2;
        return SPELL_LIGHT_DAY_OF_PROTECTION;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Hour")) {  // hour  of power
        *next_token += 2;
        return SPELL_LIGHT_HOUR_OF_POWER;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Shield")) {
        return SPELL_AIR_SHIELD;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Spirit")) {
        ++*next_token;
        return SPELL_SPIRIT_SPIRIT_LASH;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Power")) {  // power cure
        ++*next_token;
        return SPELL_BODY_POWER_CURE;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Meteor")) {  // meteot shower
        ++*next_token;
        return SPELL_FIRE_METEOR_SHOWER;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Lightning")) {  // Lightning bolt
        ++*next_token;
        return SPELL_AIR_LIGHTNING_BOLT;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Implosion")) {
        return SPELL_AIR_IMPLOSION;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Stone")) {
        ++*next_token;
        return SPELL_EARTH_STONESKIN;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Haste")) {
        return SPELL_FIRE_HASTE;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Heroism")) {
        return SPELL_SPIRIT_HEROISM;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Pain")) {  // pain reflection
        ++*next_token;
        return SPELL_DARK_PAIN_REFLECTION;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Sparks")) {
        return SPELL_AIR_SPARKS;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Light")) {
        ++*next_token;
        return SPELL_LIGHT_LIGHT_BOLT;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Toxic")) {  // toxic cloud
        ++*next_token;
        return SPELL_DARK_TOXIC_CLOUD;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "ShrapMetal")) {
        return SPELL_DARK_SHARPMETAL;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Paralyze")) {
        return SPELL_LIGHT_PARALYZE;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Fireball")) {
        return SPELL_FIRE_FIREBALL;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Incinerate")) {
        return SPELL_FIRE_INCINERATE;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Fire")) {
        ++*next_token;
        return SPELL_FIRE_FIRE_BOLT;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Rock")) {
        ++*next_token;
        return SPELL_EARTH_ROCK_BLAST;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Mass")) {
        ++*next_token;
        return SPELL_EARTH_MASS_DISTORTION;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Ice")) {
        ++*next_token;
        return SPELL_WATER_ICE_BOLT;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Acid")) {
        ++*next_token;
        return SPELL_WATER_ACID_BURST;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Bless")) {
        return SPELL_SPIRIT_BLESS;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Dragon")) {
        ++*next_token;
        return SPELL_DARK_DRAGON_BREATH;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Reanimate")) {
        return SPELL_DARK_REANIMATE;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Summon")) {
        ++*next_token;
        return SPELL_LIGHT_SUMMON_ELEMENTAL;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Fate")) {
        return SPELL_SPIRIT_FATE;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Harm")) {
        return SPELL_BODY_HARM;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Mind")) {
        ++*next_token;
        return SPELL_MIND_MIND_BLAST;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Blades")) {
        return SPELL_EARTH_BLADES;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Psychic")) {
        ++*next_token;
        return SPELL_MIND_PSYCHIC_SHOCK;
    } else if (ascii::noCaseEquals(tbl->pProperties[0], "Hammerhands")) {
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
    // TODO(captainurist): #enum
    if (ascii::noCaseEquals(missle_attack_str, "ARROW"))
        return 1;
    else if (ascii::noCaseEquals(missle_attack_str, "ARROWF"))
        return 2;
    else if (ascii::noCaseEquals(missle_attack_str, "FIRE"))
        return 3;
    else if (ascii::noCaseEquals(missle_attack_str, "AIR"))
        return 4;
    else if (ascii::noCaseEquals(missle_attack_str, "WATER"))
        return 5;
    else if (ascii::noCaseEquals(missle_attack_str, "EARTH"))
        return 6;
    else if (ascii::noCaseEquals(missle_attack_str, "SPIRIT"))
        return 7;
    else if (ascii::noCaseEquals(missle_attack_str, "MIND"))
        return 8;
    else if (ascii::noCaseEquals(missle_attack_str, "BODY"))
        return 9;
    else if (ascii::noCaseEquals(missle_attack_str, "LIGHT"))
        return 10;
    else if (ascii::noCaseEquals(missle_attack_str, "DARK"))
        return 11;
    else if (ascii::noCaseEquals(missle_attack_str, "ENER"))
        return 13;
    else
        return 0;
}

int ParseSpecialAttack(char *spec_att_str) {
    std::string tmp = ascii::toLower(spec_att_str);

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
MonsterId MonsterStats::FindMonsterByTextureName(std::string_view monster_textr_name) {
    for (MonsterId i : infos.indices()) {
        if (!infos[i].name.empty() && ascii::noCaseEquals(infos[i].textureName, monster_textr_name))
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
                    uniqueNames[i] = removeQuotes(test_string);
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
                        infos[curr_rec_num].id = curr_rec_num;
                        break;
                    case 1:
                        infos[curr_rec_num].name = removeQuotes(test_string);
                        break;
                    case 2:
                        infos[curr_rec_num].textureName = removeQuotes(test_string);
                        break;
                    case 3:
                        infos[curr_rec_num].level = atoi(test_string);
                        break;
                    case 4: {
                        int str_len = 0;
                        int str_pos = 0;
                        infos[curr_rec_num].hp = 0;
                        if (test_string[0] == '"') test_string[0] = ' ';
                        str_len = strlen(test_string);
                        if (str_len == 0) break;
                        while ((test_string[str_pos] != ',') &&
                               (str_pos < str_len))
                            ++str_pos;
                        if (str_len == str_pos) {
                            infos[curr_rec_num].hp = atoi(test_string);
                        } else {
                            test_string[str_pos] = '\0';
                            infos[curr_rec_num].hp = 1000 * atoi(test_string);
                            infos[curr_rec_num].hp +=
                                atoi(&test_string[str_pos + 1]);
                            test_string[str_pos] = ',';
                        }
                    } break;
                    case 5:
                        infos[curr_rec_num].ac = atoi(test_string);
                        break;
                    case 6: {
                        int str_len = 0;
                        int str_pos = 0;
                        infos[curr_rec_num].exp = 0;
                        if (test_string[0] == '"') test_string[0] = ' ';
                        str_len = strlen(test_string);
                        if (str_len == 0) break;
                        while ((test_string[str_pos] != ',') &&
                               (str_pos < str_len))
                            ++str_pos;
                        if (str_len == str_pos) {
                            infos[curr_rec_num].exp = atoi(test_string);
                        } else {
                            test_string[str_pos] = '\0';
                            infos[curr_rec_num].exp =
                                1000 * atoi(test_string);
                            infos[curr_rec_num].exp +=
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
                        infos[curr_rec_num].treasureDropChance = 0;
                        infos[curr_rec_num].goldDiceRolls = 0;
                        infos[curr_rec_num].goldDiceSides = 0;
                        infos[curr_rec_num].treasureType = RANDOM_ITEM_ANY;
                        infos[curr_rec_num].treasureLevel = ITEM_TREASURE_LEVEL_INVALID;
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
                            infos[curr_rec_num].treasureDropChance =
                                atoi(test_string);
                        } else {
                            if ((!dice_flag) && (!item_type_flag)) break;
                            infos[curr_rec_num].treasureDropChance = 100;
                        }
                        if (dice_flag) {
                            str_pos = 0;
                            dice_flag = false;
                            do {
                                switch (tolower(test_string[str_pos])) {
                                    case '%':
                                        infos[curr_rec_num]
                                            .goldDiceRolls =
                                            atoi(&test_string[str_pos + 1]);
                                        dice_flag = true;
                                        break;
                                    case 'd':
                                        if (!dice_flag)
                                            infos[curr_rec_num]
                                                .goldDiceRolls =
                                                atoi(test_string);
                                        infos[curr_rec_num]
                                            .goldDiceSides =
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

                            infos[curr_rec_num].treasureLevel =
                                ItemTreasureLevel(test_string[str_pos + 1] - '0');
                            item_name = &test_string[str_pos + 2];
                            if (*item_name) {
                                if (ascii::noCaseEquals(item_name, "WEAPON"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_WEAPON;
                                else if (ascii::noCaseEquals(item_name, "ARMOR"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_ARMOR;
                                else if (ascii::noCaseEquals(item_name, "MISC"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_MICS;
                                else if (ascii::noCaseEquals(item_name, "SWORD"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_SWORD;
                                else if (ascii::noCaseEquals(item_name, "DAGGER"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_DAGGER;
                                else if (ascii::noCaseEquals(item_name, "AXE"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_AXE;
                                else if (ascii::noCaseEquals(item_name, "SPEAR"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_SPEAR;
                                else if (ascii::noCaseEquals(item_name, "BOW"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_BOW;
                                else if (ascii::noCaseEquals(item_name, "MACE"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_MACE;
                                else if (ascii::noCaseEquals(item_name, "CLUB"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_CLUB;
                                else if (ascii::noCaseEquals(item_name, "STAFF"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_STAFF;
                                else if (ascii::noCaseEquals(item_name, "LEATHER"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_LEATHER_ARMOR;
                                else if (ascii::noCaseEquals(item_name, "CHAIN"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_CHAIN_ARMOR;
                                else if (ascii::noCaseEquals(item_name, "PLATE"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_PLATE_ARMOR;
                                else if (ascii::noCaseEquals(item_name, "SHIELD"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_SHIELD;
                                else if (ascii::noCaseEquals(item_name, "HELM"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_HELMET;
                                else if (ascii::noCaseEquals(item_name, "BELT"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_BELT;
                                else if (ascii::noCaseEquals(item_name, "CAPE"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_CLOAK;
                                else if (ascii::noCaseEquals(item_name, "GAUNTLETS"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_GAUNTLETS;
                                else if (ascii::noCaseEquals(item_name, "BOOTS"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_BOOTS;
                                else if (ascii::noCaseEquals(item_name, "RING"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_RING;
                                else if (ascii::noCaseEquals(item_name, "AMULET"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_AMULET;
                                else if (ascii::noCaseEquals(item_name, "WAND"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_WAND;
                                else if (ascii::noCaseEquals(item_name, "SCROLL"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_SPELL_SCROLL;
                                else if (ascii::noCaseEquals(item_name, "GEM"))
                                    infos[curr_rec_num].treasureType = RANDOM_ITEM_GEM;
                            }
                        }
                    } break;
                    case 8: {
                        infos[curr_rec_num].bloodSplatOnDeath = false;
                        if (atoi(test_string))
                            infos[curr_rec_num].bloodSplatOnDeath = true;
                    } break;
                    case 9: {
                        infos[curr_rec_num].flying = false;
                        if (!ascii::noCaseEquals(test_string, "n")) // "Y"/"N"
                            infos[curr_rec_num].flying = true;
                    } break;
                    case 10: {
                        switch (tolower(test_string[0])) {
                            case 's':
                                infos[curr_rec_num].movementType = MONSTER_MOVEMENT_TYPE_SHORT;  // short
                                if (tolower(test_string[1]) != 'h')
                                    infos[curr_rec_num].movementType = MONSTER_MOVEMENT_TYPE_STATIONARY;  // stationary
                                break;  // short
                            case 'l':
                                infos[curr_rec_num].movementType = MONSTER_MOVEMENT_TYPE_LONG;
                                break;  // long
                            case 'm':
                                infos[curr_rec_num].movementType = MONSTER_MOVEMENT_TYPE_MEDIUM;
                                break;  // med
                            case 'g':
                                infos[curr_rec_num].movementType = MONSTER_MOVEMENT_TYPE_GLOBAL;
                                break;  // global?
                            default:
                                infos[curr_rec_num].movementType = MONSTER_MOVEMENT_TYPE_FREE;  // free
                        }
                    } break;
                    case 11: {
                        switch (tolower(test_string[0])) {
                            case 's':
                                infos[curr_rec_num].aiType = MONSTER_AI_SUICIDE;
                                break;
                            case 'w':
                                infos[curr_rec_num].aiType = MONSTER_AI_WIMP;
                                break;
                            case 'n':
                                infos[curr_rec_num].aiType = MONSTER_AI_NORMAL;
                                break;
                            default:
                                infos[curr_rec_num].aiType = MONSTER_AI_AGGRESSIVE;
                        }
                    } break;
                    case 12:
                        infos[curr_rec_num].hostilityType =
                            (MonsterHostility)atoi(test_string);
                        break;
                    case 13:
                        infos[curr_rec_num].baseSpeed = atoi(test_string);
                        break;
                    case 14:
                        infos[curr_rec_num].recoveryTime = Duration::fromTicks(atoi(test_string));
                        break;
                    case 15: {
                        int str_len = 0;
                        int str_pos = 0;
                        infos[curr_rec_num].attackPreferences = 0;
                        infos[curr_rec_num]
                            .numCharactersAttackedPerSpecialAbility = 0;
                        str_len = strlen(test_string);
                        for (str_pos = 0; str_pos < str_len; ++str_pos) {
                            switch (tolower(test_string[str_pos])) {
                                case '0':
                                    // TODO(captainurist): '0' means archer? Why???
                                    infos[curr_rec_num].attackPreferences |=
                                        ATTACK_PREFERENCE_ARCHER;
                                    break;
                                case '2':
                                    infos[curr_rec_num]
                                        .numCharactersAttackedPerSpecialAbility =
                                        2;
                                    break;
                                case '3':
                                    infos[curr_rec_num]
                                        .numCharactersAttackedPerSpecialAbility =
                                        3;
                                    break;
                                case '4':
                                    infos[curr_rec_num]
                                        .numCharactersAttackedPerSpecialAbility =
                                        4;
                                    break;
                                case 'c':
                                    infos[curr_rec_num].attackPreferences |=
                                        ATTACK_PREFERENCE_CLERIC;
                                    break;
                                case 'd':
                                    infos[curr_rec_num].attackPreferences |=
                                        ATTACK_PREFERENCE_DRUID;
                                    break;
                                case 'e':
                                    infos[curr_rec_num].attackPreferences |=
                                        ATTACK_PREFERENCE_ELF;
                                    break;
                                case 'f':
                                    infos[curr_rec_num].attackPreferences |=
                                        ATTACK_PREFERENCE_FEMALE;
                                    break;
                                case 'h':
                                    infos[curr_rec_num].attackPreferences |=
                                        ATTACK_PREFERENCE_HUMAN;
                                    break;
                                case 'k':
                                    infos[curr_rec_num].attackPreferences |=
                                        ATTACK_PREFERENCE_KNIGHT;
                                    break;
                                case 'm':
                                    infos[curr_rec_num].attackPreferences |=
                                        ATTACK_PREFERENCE_MONK;
                                    break;
                                case 'o':
                                    // TODO(captainurist): both 'f' and 'o' are ATTACK_PREFERENCE_FEMALE?
                                    infos[curr_rec_num].attackPreferences |=
                                        ATTACK_PREFERENCE_FEMALE;
                                    break;
                                case 'p':
                                    infos[curr_rec_num].attackPreferences |=
                                        ATTACK_PREFERENCE_PALADIN;
                                    break;
                                case 'r':
                                    infos[curr_rec_num].attackPreferences |=
                                        ATTACK_PREFERENCE_RANGER;
                                    break;
                                case 's':
                                    infos[curr_rec_num].attackPreferences |=
                                        ATTACK_PREFERENCE_SORCERER;
                                    break;
                                case 't':
                                    infos[curr_rec_num].attackPreferences |=
                                        ATTACK_PREFERENCE_THIEF;
                                    break;
                                case 'w':
                                    infos[curr_rec_num].attackPreferences |=
                                        ATTACK_PREFERENCE_DWARF;
                                    break;
                                case 'x':
                                    infos[curr_rec_num].attackPreferences |=
                                        ATTACK_PREFERENCE_MALE;
                                    break;
                            }
                        }
                    } break;
                    case 16: {
                        int str_len = 0;
                        int str_pos = 0;
                        infos[curr_rec_num].specialAttackLevel = 1;
                        infos[curr_rec_num].specialAttackType =
                            (SpecialAttackType)0;
                        str_len = strlen(test_string);
                        if (str_len > 1) {
                            for (str_pos = 0; str_pos < str_len; ++str_pos) {
                                if (tolower(test_string[str_pos]) == 'x') {
                                    test_string[str_pos] = '\0';
                                    infos[curr_rec_num].specialAttackLevel =
                                        atoi(&test_string[str_pos + 1]);
                                    test_string[str_pos] = 'x';
                                    break;
                                }
                            }
                            infos[curr_rec_num].specialAttackType =
                                (SpecialAttackType)ParseSpecialAttack(
                                    test_string);
                        }
                    } break;
                    case 17:
                        infos[curr_rec_num].attack1Type = ParseAttackType(test_string);
                        break;
                    case 18: {
                        ParseDamage(
                            test_string,
                            &infos[curr_rec_num].attack1DamageDiceRolls,
                            &infos[curr_rec_num].attack1DamageDiceSides,
                            &infos[curr_rec_num].attack1DamageBonus);
                    } break;
                    case 19:
                        infos[curr_rec_num].attack1MissileType =
                            ParseMissleAttackType(test_string);
                        break;
                    case 20:
                        infos[curr_rec_num].attack2Chance = atoi(test_string);
                        break;
                    case 21:
                        infos[curr_rec_num].attack2Type =
                            ParseAttackType(test_string);
                        break;
                    case 22: {
                        ParseDamage(
                            test_string,
                            &infos[curr_rec_num].attack2DamageDiceRolls,
                            &infos[curr_rec_num].attack2DamageDiceSides,
                            &infos[curr_rec_num].attack2DamageBonus);
                    } break;
                    case 23:
                        infos[curr_rec_num].attack2MissileType =
                            ParseMissleAttackType(test_string);
                        break;
                    case 24:
                        infos[curr_rec_num].spell1UseChance =
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
                            infos[curr_rec_num].spell1Id =
                                ParseSpellType(&parsed_field, &param_num);
                            infos[curr_rec_num].spell1SkillMastery =
                                ParseSkillValue(parsed_field.pProperties[param_num + 1], parsed_field.pProperties[param_num]);
                        } else {
                            infos[curr_rec_num].spell1Id = SPELL_NONE;
                            infos[curr_rec_num].spell1SkillMastery = CombinedSkillValue::none();
                        }
                    } break;
                    case 26:
                        infos[curr_rec_num].spell2UseChance =
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
                            infos[curr_rec_num].spell2Id =
                                ParseSpellType(&parsed_field, &param_num);
                            infos[curr_rec_num].spell2SkillMastery =
                                ParseSkillValue(parsed_field.pProperties[param_num + 1], parsed_field.pProperties[param_num]);
                        } else {
                            infos[curr_rec_num].spell2Id = SPELL_NONE;
                            infos[curr_rec_num].spell2SkillMastery = CombinedSkillValue::none();
                        }
                    } break;
                    case 28: {
                        if (tolower(test_string[0]) == 'i')
                            infos[curr_rec_num].resFire = 200;
                        else
                            infos[curr_rec_num].resFire = atoi(test_string);
                    } break;
                    case 29: {
                        if (tolower(test_string[0]) == 'i')
                            infos[curr_rec_num].resAir = 200;
                        else
                            infos[curr_rec_num].resAir = atoi(test_string);
                    } break;
                    case 30: {
                        if (tolower(test_string[0]) == 'i')
                            infos[curr_rec_num].resWater = 200;
                        else
                            infos[curr_rec_num].resWater = atoi(test_string);
                    } break;
                    case 31: {
                        if (tolower(test_string[0]) == 'i')
                            infos[curr_rec_num].resEarth = 200;
                        else
                            infos[curr_rec_num].resEarth = atoi(test_string);
                    } break;
                    case 32: {
                        if (tolower(test_string[0]) == 'i')
                            infos[curr_rec_num].resMind = 200;
                        else
                            infos[curr_rec_num].resMind = atoi(test_string);
                    } break;
                    case 33: {
                        if (tolower(test_string[0]) == 'i')
                            infos[curr_rec_num].resSpirit = 200;
                        else
                            infos[curr_rec_num].resSpirit = atoi(test_string);
                    } break;
                    case 34: {
                        if (tolower(test_string[0]) == 'i')
                            infos[curr_rec_num].resBody = 200;
                        else
                            infos[curr_rec_num].resBody = atoi(test_string);
                    } break;
                    case 35: {
                        if (tolower(test_string[0]) == 'i')
                            infos[curr_rec_num].resLight = 200;
                        else
                            infos[curr_rec_num].resLight = atoi(test_string);
                    } break;
                    case 36: {
                        if (tolower(test_string[0]) == 'i')
                            infos[curr_rec_num].resDark = 200;
                        else
                            infos[curr_rec_num].resDark = atoi(test_string);
                    } break;
                    case 37: {
                        if (tolower(test_string[0]) == 'i')
                            infos[curr_rec_num].resPhysical = 200;
                        else
                            infos[curr_rec_num].resPhysical =
                                atoi(test_string);
                    } break;
                    case 38: {
                        //                    int param_num;
                        //                    char type_flag;
                        infos[curr_rec_num].specialAbilityType = MONSTER_SPECIAL_ABILITY_NONE;
                        infos[curr_rec_num].specialAbilityDamageDiceBonus = 0;
                        strcpy(parse_str, test_string);
                        parse_str[0] = ' ';
                        parse_str[strlen(parse_str) - 1] = ' ';
                        frame_table_txt_parser(parse_str, &parsed_field);
                        if (parsed_field.uPropCount) {
                            //      v74 = v94.field_0;
                            if (parsed_field.uPropCount < 10) {
                                if (ascii::noCaseEquals(parsed_field.pProperties[0], "shot")) {
                                    infos[curr_rec_num].specialAbilityType = MONSTER_SPECIAL_ABILITY_SHOT;
                                    infos[curr_rec_num]
                                        .specialAbilityDamageDiceBonus = atoi(
                                        (char *)(parsed_field.pProperties[1] +
                                                 1));
                                } else if (ascii::noCaseEquals(parsed_field.pProperties[0], "summon")) {
                                    infos[curr_rec_num].specialAbilityType = MONSTER_SPECIAL_ABILITY_SUMMON;
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
                                                                infos[curr_rec_num]
                                                                    .specialAbilityDamageDiceRolls =
                                                                    1;
                                                                break;
                                                            case 'b':
                                                                infos[curr_rec_num]
                                                                    .specialAbilityDamageDiceRolls =
                                                                    2;
                                                                break;
                                                            case 'c':
                                                                infos[curr_rec_num]
                                                                    .specialAbilityDamageDiceRolls =
                                                                    3;
                                                                break;
                                                            default:
                                                                infos[curr_rec_num]
                                                                    .specialAbilityDamageDiceRolls =
                                                                    0;
                                                        }
                                                    }
                                                    ++prop_cnt;
                                                } while (
                                                    prop_cnt <
                                                    parsed_field.uPropCount);
                                            }
                                        } else {
                                            infos[curr_rec_num]
                                                .specialAbilityDamageDiceRolls =
                                                0;
                                        }
                                        if (!pMonsterList->monsters.empty()) {
                                            infos[curr_rec_num].field_3C_some_special_attack =
                                                std::to_underlying(pMonsterList->GetMonsterIDByName(str));
                                        }
                                        infos[curr_rec_num]
                                            .specialAbilityDamageDiceSides = 0;
                                        if (ascii::noCaseEquals(parsed_field.pProperties[1], "ground"))
                                            infos[curr_rec_num]
                                                .specialAbilityDamageDiceSides =
                                                1;
                                        if (infos[curr_rec_num]
                                                .field_3C_some_special_attack ==
                                            -1)
                                            infos[curr_rec_num].specialAbilityType = MONSTER_SPECIAL_ABILITY_NONE;
                                    }
                                } else if (ascii::noCaseEquals(parsed_field.pProperties[0], "explode")) {
                                    infos[curr_rec_num].specialAbilityType = MONSTER_SPECIAL_ABILITY_EXPLODE;
                                    ParseDamage(
                                        (char *)parsed_field.pProperties[1],
                                        &infos[curr_rec_num]
                                             .specialAbilityDamageDiceRolls,
                                        &infos[curr_rec_num]
                                             .specialAbilityDamageDiceSides,
                                        &infos[curr_rec_num]
                                             .specialAbilityDamageDiceBonus);
                                    infos[curr_rec_num]
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
MonsterId MonsterList::GetMonsterIDByName(std::string_view pMonsterName) {
    for (MonsterId i : monsters.indices()) {
        if (ascii::noCaseEquals(monsters[i].monsterName, pMonsterName))
            return i;
    }
    logger->error("Monster not found: {}", pMonsterName);
    return MONSTER_INVALID;
}
