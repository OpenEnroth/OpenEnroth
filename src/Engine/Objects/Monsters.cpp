#include "Engine/Objects/Monsters.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <utility>
#include <string_view>

#include "Engine/ErrorHandling.h"
#include "Engine/EngineIocContainer.h"
#include "../Tables/FrameTableInc.h"
#include "Library/Logger/Logger.h"
#include "Utility/Memory/Blob.h"
#include "Utility/String.h"
#include "Utility/Exception.h"
#include "Engine/Objects/ActorEnums.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Objects/ItemEnums.h"
#include "Library/Serialization/StandardSerialization.h"
#include "fmt/core.h"

struct MonsterStats *pMonsterStats;
struct MonsterList *pMonsterList;

int ParseAttackType(const char *damage_type_str);
void ParseDamage(char *damage_str, uint8_t *dice_rolls,
                 uint8_t *dice_sides, uint8_t *dmg_bonus);
int ParseMissleAttackType(const char *missle_attack_str);
int ParseSpecialAttack(const char *spec_att_str);

//----- (004548E2) --------------------------------------------------------
SPELL_TYPE ParseSpellType(struct FrameTableTxtLine *tbl, int *next_token) {
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
int ParseAttackType(const char *damage_type_str) {
    switch (tolower(*damage_type_str)) {
        case 'f':
            return 0;  // fire
        case 'a':
            return 1;  // air
        case 'w':
            return 2;  // water
        case 'e':
            return 3;  // earth

        case 's':
            return 6;  // spirit
        case 'm':
            return 7;  // mind
            // m ?? 8
        case 'l':
            return 9;  // light
        case 'd':
            return 10;  // dark
            // d?? 11
    }
    return 4;  // phis
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

//----- (004598FC) --------------------------------------------------------
bool MonsterList::FromFileTxt(const char *Args) {
    FILE *v3;               // eax@1
    unsigned int v4;        // esi@3
    void *v5;               // eax@9
    FILE *v6;               // ST14_4@11
    char *i;                // eax@11
    signed int v8;          // esi@12
    uint16_t v9;    // ax@16
    const char *v10;        // ST18_4@16
    uint16_t v11;   // ax@16
    const char *v12;        // ST14_4@16
    uint16_t v13;   // ax@16
    const char *v14;        // ST10_4@16
    int16_t v15;            // ax@16
    const char *v16;        // ST0C_4@16
    int v17;                // esi@16
    uint8_t v18;    // al@16
    signed int v19;         // esi@16
    uint16_t v20;   // ax@17
    int v21;                // ecx@17
    char Buf[490];           // [sp+4h] [bp-304h]@3
    FrameTableTxtLine v24;  // [sp+1F8h] [bp-110h]@4
    FrameTableTxtLine v25;  // [sp+274h] [bp-94h]@4
    int v26;                // [sp+2F0h] [bp-18h]@16
    FILE *File;             // [sp+304h] [bp-4h]@1
    unsigned int Argsa;     // [sp+310h] [bp+8h]@3
    int Argsb;              // [sp+310h] [bp+8h]@16

    v3 = fopen(Args, "r");
    File = v3;
    if (!v3) Error("MonsterRaceListStruct::load - Unable to open file: %s.");

    v4 = 0;
    Argsa = 0;
    if (fgets(Buf, sizeof(Buf), v3)) {
        do {
            *strchr(Buf, 10) = 0;
            memcpy(&v25, frame_table_txt_parser(Buf, &v24), sizeof(v25));
            if (v25.uPropCount && *v25.pProperties[0] != 47) ++Argsa;
        } while (fgets(Buf, sizeof(Buf), File));
        v4 = Argsa;
    }

    this->pMonsters.clear();
    v6 = File;
    fseek(v6, 0, 0);
    for (i = fgets(Buf, sizeof(Buf), File); i; i = fgets(Buf, sizeof(Buf), File)) {
        *strchr(Buf, 10) = 0;
        memcpy(&v25, frame_table_txt_parser(Buf, &v24), sizeof(v25));
        v8 = 0;
        if (v25.uPropCount && *v25.pProperties[0] != 47) {
            MonsterDesc &monster = this->pMonsters.emplace_back();

            monster.pMonsterName = v25.pProperties[0];

            constexpr std::array<std::pair<ActorAnimation, int>, 8> mapping = {{
                {ANIM_Standing, 1},
                {ANIM_Walking, 2},
                {ANIM_Bored, 3},
                {ANIM_AtkMelee, 4},
                {ANIM_AtkRanged, 4},
                {ANIM_GotHit, 5},
                {ANIM_Dying, 6},
                {ANIM_Dead, 7},
            }};

            do {
                monster.pSpriteNames[mapping[v8].first] = v25.pProperties[mapping[v8].second];
                ++v8;
            } while (v8 < 8);
            v9 = atoi(v25.pProperties[8]);
            v10 = v25.pProperties[9];
            monster.uMonsterHeight = v9;
            v11 = atoi(v10);
            v12 = v25.pProperties[10];
            monster.uMovementSpeed = v11;
            v13 = atoi(v12);
            v14 = v25.pProperties[11];
            monster.uMonsterRadius = v13;
            v15 = atoi(v14);
            v16 = v25.pProperties[12];
            monster.uToHitRadius = v15;
            v17 = (uint8_t)atoi(v16);
            Argsb = atoi(v25.pProperties[13]) & 0xFF;
            v26 = atoi(v25.pProperties[14]) & 0xFF;
            v18 = atoi(v25.pProperties[15]);
            monster.sTintColor = Color(v18, v26, Argsb, v17);
            v19 = 0;
            do {
                v20 = atoi(v25.pProperties[v19 + 16]);
                v21 = v19++;
                monster.pSoundSampleIDs[v21] = v20;
            } while (v19 < 4);
        }
    }
    fclose(File);
    return 1;
}

//----- (004563FF) --------------------------------------------------------
signed int MonsterStats::FindMonsterByTextureName(const std::string &monster_textr_name) {
    for (int i = 1; i < uNumMonsters; ++i) {
        if (!pInfos[i].pName.empty() && iequals(pInfos[i].pPictureName, monster_textr_name))
            return i;
    }
    return -1;
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
                    pPlaceStrings[i] = removeQuotes(test_string);
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 3) && !break_loop);
    }
    uNumPlacements = 31;
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
    int curr_rec_num;
    char parse_str[64];
    // char Src[120];
    FrameTableTxtLine parsed_field;
    std::string str;

    std::string txtRaw(monsters.string_view());
    strtok(txtRaw.data(), "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    uNumMonsters = 265;
    curr_rec_num = 0;
    for (i = 0; i < uNumMonsters - 1; ++i) {
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
                        curr_rec_num = atoi(test_string);
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
                        pInfos[curr_rec_num].uTreasureType = 0;
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
                                ITEM_TREASURE_LEVEL(test_string[str_pos + 1] - '0');
                            item_name = &test_string[str_pos + 2];
                            if (*item_name) {
                                if (iequals(item_name, "WEAPON"))
                                    pInfos[curr_rec_num].uTreasureType = 20;
                                else if (iequals(item_name, "ARMOR"))
                                    pInfos[curr_rec_num].uTreasureType = 21;
                                else if (iequals(item_name, "MISC"))
                                    pInfos[curr_rec_num].uTreasureType = 22;
                                else if (iequals(item_name, "SWORD"))
                                    pInfos[curr_rec_num].uTreasureType = 23;
                                else if (iequals(item_name, "DAGGER"))
                                    pInfos[curr_rec_num].uTreasureType = 24;
                                else if (iequals(item_name, "AXE"))
                                    pInfos[curr_rec_num].uTreasureType = 25;
                                else if (iequals(item_name, "SPEAR"))
                                    pInfos[curr_rec_num].uTreasureType = 26;
                                else if (iequals(item_name, "BOW"))
                                    pInfos[curr_rec_num].uTreasureType = 27;
                                else if (iequals(item_name, "MACE"))
                                    pInfos[curr_rec_num].uTreasureType = 28;
                                else if (iequals(item_name, "CLUB"))
                                    pInfos[curr_rec_num].uTreasureType = 29;
                                else if (iequals(item_name, "STAFF"))
                                    pInfos[curr_rec_num].uTreasureType = 30;
                                else if (iequals(item_name, "LEATHER"))
                                    pInfos[curr_rec_num].uTreasureType = 31;
                                else if (iequals(item_name, "CHAIN"))
                                    pInfos[curr_rec_num].uTreasureType = 32;
                                else if (iequals(item_name, "PLATE"))
                                    pInfos[curr_rec_num].uTreasureType = 33;
                                else if (iequals(item_name, "SHIELD"))
                                    pInfos[curr_rec_num].uTreasureType = 34;
                                else if (iequals(item_name, "HELM"))
                                    pInfos[curr_rec_num].uTreasureType = 35;
                                else if (iequals(item_name, "BELT"))
                                    pInfos[curr_rec_num].uTreasureType = 36;
                                else if (iequals(item_name, "CAPE"))
                                    pInfos[curr_rec_num].uTreasureType = 37;
                                else if (iequals(item_name, "GAUNTLETS"))
                                    pInfos[curr_rec_num].uTreasureType = 38;
                                else if (iequals(item_name, "BOOTS"))
                                    pInfos[curr_rec_num].uTreasureType = 39;
                                else if (iequals(item_name, "RING"))
                                    pInfos[curr_rec_num].uTreasureType = 40;
                                else if (iequals(item_name, "AMULET"))
                                    pInfos[curr_rec_num].uTreasureType = 41;
                                else if (iequals(item_name, "WAND"))
                                    pInfos[curr_rec_num].uTreasureType = 42;
                                else if (iequals(item_name, "SCROLL"))
                                    pInfos[curr_rec_num].uTreasureType = 43;
                                else if (iequals(item_name, "GEM"))
                                    pInfos[curr_rec_num].uTreasureType = 46;
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
                                pInfos[curr_rec_num].uAIType = 0;
                                break;  // suicide
                            case 'w':
                                pInfos[curr_rec_num].uAIType = 1;
                                break;  // wimp
                            case 'n':
                                pInfos[curr_rec_num].uAIType = 2;
                                break;  // normal
                            default:
                                pInfos[curr_rec_num].uAIType = 3;  // Agress
                        }
                    } break;
                    case 12:
                        pInfos[curr_rec_num].uHostilityType =
                            (MonsterInfo::HostilityRadius)atoi(test_string);
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
                        pInfos[curr_rec_num].uAttackPreference = 0;
                        pInfos[curr_rec_num]
                            .uNumCharactersAttackedPerSpecialAbility = 0;
                        str_len = strlen(test_string);
                        for (str_pos = 0; str_pos < str_len; ++str_pos) {
                            switch (tolower(test_string[str_pos])) {
                                case '0':
                                    pInfos[curr_rec_num].uAttackPreference |=
                                        0x0004;
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
                                    pInfos[curr_rec_num].uAttackPreference |=
                                        0x0010;
                                    break;
                                case 'd':
                                    pInfos[curr_rec_num].uAttackPreference |=
                                        0x0008;
                                    break;
                                case 'e':
                                    pInfos[curr_rec_num].uAttackPreference |=
                                        0x1000;
                                    break;
                                case 'f':
                                    pInfos[curr_rec_num].uAttackPreference |=
                                        0x0400;
                                    break;
                                case 'h':
                                    pInfos[curr_rec_num].uAttackPreference |=
                                        0x0800;
                                    break;
                                case 'k':
                                    pInfos[curr_rec_num].uAttackPreference |=
                                        0x0001;
                                    break;
                                case 'm':
                                    pInfos[curr_rec_num].uAttackPreference |=
                                        0x0100;
                                    break;
                                case 'o':
                                    pInfos[curr_rec_num].uAttackPreference |=
                                        0x0400;
                                    break;
                                case 'p':
                                    pInfos[curr_rec_num].uAttackPreference |=
                                        0x0002;
                                    break;
                                case 'r':
                                    pInfos[curr_rec_num].uAttackPreference |=
                                        0x0040;
                                    break;
                                case 's':
                                    pInfos[curr_rec_num].uAttackPreference |=
                                        0x0020;
                                    break;
                                case 't':
                                    pInfos[curr_rec_num].uAttackPreference |=
                                        0x0080;
                                    break;
                                case 'w':
                                    pInfos[curr_rec_num].uAttackPreference |=
                                        0x2000;
                                    break;
                                case 'x':
                                    pInfos[curr_rec_num].uAttackPreference |=
                                        0x0200;
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
                        pInfos[curr_rec_num].uAttack1Type =
                            ParseAttackType(test_string);
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
                        pInfos[curr_rec_num].uSpecialAbilityType = 0;
                        pInfos[curr_rec_num].uSpecialAbilityDamageDiceBonus = 0;
                        strcpy(parse_str, test_string);
                        parse_str[0] = ' ';
                        parse_str[strlen(parse_str) - 1] = ' ';
                        frame_table_txt_parser(parse_str, &parsed_field);
                        if (parsed_field.uPropCount) {
                            //      v74 = v94.field_0;
                            if (parsed_field.uPropCount < 10) {
                                if (iequals(parsed_field.pProperties[0], "shot")) {
                                    pInfos[curr_rec_num].uSpecialAbilityType =
                                        1;
                                    pInfos[curr_rec_num]
                                        .uSpecialAbilityDamageDiceBonus = atoi(
                                        (char *)(parsed_field.pProperties[1] +
                                                 1));
                                } else if (iequals(parsed_field.pProperties[0], "summon")) {
                                    pInfos[curr_rec_num].uSpecialAbilityType =
                                        2;
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
                                            pInfos[curr_rec_num]
                                                .field_3C_some_special_attack =
                                                pMonsterList
                                                    ->GetMonsterIDByName(str) +
                                                1;
                                            if (pInfos[curr_rec_num]
                                                    .field_3C_some_special_attack ==
                                                -1) {
                                                logger->warning("Can't create random monster: '{}'. See MapStats!", str);
                                            }
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
                                            pInfos[curr_rec_num]
                                                .uSpecialAbilityType = 0;
                                    }
                                } else if (iequals(parsed_field.pProperties[0], "explode")) {
                                    pInfos[curr_rec_num].uSpecialAbilityType =
                                        3;
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
                                        ParseAttackType(test_string);
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
    uNumMonsters = i;
}

//----- (0044FA08) --------------------------------------------------------
int16_t MonsterList::GetMonsterIDByName(const std::string &pMonsterName) {
    for (int16_t i = 0; i < pMonsters.size(); ++i) {
        if (iequals(pMonsters[i].pMonsterName, pMonsterName))
            return i;
    }
    Error("Monster not found: %s", pMonsterName.c_str());
}
//----- (00438BDF) --------------------------------------------------------
bool MonsterStats::BelongsToSupertype(unsigned int uMonsterInfoID,
                                      enum MONSTER_SUPERTYPE eSupertype) {
    switch (eSupertype) {
        case MONSTER_SUPERTYPE_UNDEAD:
            if ((signed int)uMonsterInfoID >= MONSTER_GHOST_1 &&
                    (signed int)uMonsterInfoID <= MONSTER_GHOST_3  // 70<=id<=72
                || (signed int)uMonsterInfoID >= MONSTER_LICH_1 &&
                       (signed int)uMonsterInfoID <= MONSTER_LICH_3  // 91-93
                ||
                (signed int)uMonsterInfoID >= MONSTER_SKELETON_1 &&
                    (signed int)uMonsterInfoID <= MONSTER_SKELETON_3  // 199-201
                ||
                (signed int)uMonsterInfoID >= MONSTER_VAMPIRE_1 &&
                    (signed int)uMonsterInfoID <= MONSTER_VAMPIRE_3  // 217-219
                || (signed int)uMonsterInfoID >= MONSTER_WIGHT_1 &&
                       (signed int)uMonsterInfoID <= MONSTER_WIGHT_3  // 223-225
                ||
                (signed int)uMonsterInfoID >= MONSTER_ZOMBIE_1 &&
                    (signed int)uMonsterInfoID <= MONSTER_ZOMBIE_3  // 229-231
                ||
                (signed int)uMonsterInfoID >= MONSTER_GHOUL_1 &&
                    (signed int)uMonsterInfoID <= MONSTER_GHOUL_3)  // 256-258
                return true;
            return false;
        case MONSTER_SUPERTYPE_KREEGAN:
            if ((signed int)uMonsterInfoID >= MONSTER_DEVIL_1 &&
                (signed int)uMonsterInfoID <= MONSTER_DEVIL_3)  // 22-24
                return true;
            return false;
        case MONSTER_SUPERTYPE_ELF:
            if ((signed int)uMonsterInfoID >= MONSTER_PEASANT_ELF_FEMALE_1_1 &&
                    (signed int)uMonsterInfoID <=
                        MONSTER_PEASANT_ELF_MALE_3_3  // 133 - 150
                ||
                (signed int)uMonsterInfoID >= MONSTER_ELF_ARCHER_1 &&
                    (signed int)uMonsterInfoID <= MONSTER_ELF_ARCHER_3  // 49-51
                || (signed int)uMonsterInfoID >= MONSTER_ELF_SPEARMAN_1 &&
                       (signed int)uMonsterInfoID <=
                           MONSTER_ELF_SPEARMAN_3)  // 52-54
                return true;
            return false;
        case MONSTER_SUPERTYPE_DRAGON:
            if ((signed int)uMonsterInfoID >= MONSTER_DRAGON_1 &&
                (signed int)uMonsterInfoID <= MONSTER_DRAGON_3)  // 25-27
                return true;
            return false;
        case MONSTER_SUPERTYPE_WATER_ELEMENTAL:
            if ((signed int)uMonsterInfoID >= MONSTER_ELEMENTAL_WATER_1 &&
                (signed int)uMonsterInfoID <=
                    MONSTER_ELEMENTAL_WATER_3)  // 46-48
                return true;
            return false;
        case MONSTER_SUPERTYPE_TREANT:
            if ((signed int)uMonsterInfoID >= MONSTER_TREANT_1 &&
                (signed int)uMonsterInfoID <= MONSTER_TREANT_3)  // 253-255
                return true;
            return false;
        case MONSTER_SUPERTYPE_TITAN:
            if ((signed int)uMonsterInfoID >= MONSTER_TITAN_1 &&
                (signed int)uMonsterInfoID <= MONSTER_TITAN_3)  // 211-213
                return true;
            return false;
        default:
            return false;
    }
    return false;
}
