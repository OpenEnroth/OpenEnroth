#include "Engine/Objects/Monsters.h"

#include "Engine/Engine.h"

#include "Platform/Api.h"

#include "../LOD.h"
#include "../Tables/FrameTableInc.h"


struct MonsterStats *pMonsterStats;
struct MonsterList *pMonsterList;

unsigned int ParseSpellType(struct FrameTableTxtLine *tbl, int *next_token);

int ParseAttackType(const char *damage_type_str);
void ParseDamage(char *damage_str, unsigned __int8 *dice_rolls,
                 unsigned __int8 *dice_sides, unsigned __int8 *dmg_bonus);
int ParseMissleAttackType(const char *missle_attack_str);
int ParseSpecialAttack(const char *spec_att_str);

//----- (004548E2) --------------------------------------------------------
unsigned int ParseSpellType(struct FrameTableTxtLine *tbl, int *next_token) {
    if (!tbl->pProperties[0]) {
        ++*next_token;
        return 0;
    }
    if (!_stricmp(tbl->pProperties[0], "Dispel")) {  // dispel magic
        ++*next_token;
        return 80;
    } else if (!_stricmp(tbl->pProperties[0], "Day")) {  // day of protection
        *next_token += 2;
        return 85;
    } else if (!_stricmp(tbl->pProperties[0], "Hour")) {  // hour  of power
        *next_token += 2;
        return 86;
    } else if (!_stricmp(tbl->pProperties[0], "Shield")) {
        return 17;
    } else if (!_stricmp(tbl->pProperties[0], "Spirit")) {
        ++*next_token;
        return 52;
    } else if (!_stricmp(tbl->pProperties[0], "Power")) {  // power cure
        ++*next_token;
        return 77;
    } else if (!_stricmp(tbl->pProperties[0], "Meteor")) {  // meteot shower
        ++*next_token;
        return 9;
    } else if (!_stricmp(tbl->pProperties[0], "Lightning")) {  // Lightning bolt
        ++*next_token;
        return 18;
    } else if (!_stricmp(tbl->pProperties[0], "Implosion")) {
        return 20;
    } else if (!_stricmp(tbl->pProperties[0], "Stone")) {
        ++*next_token;
        return 38;
    } else if (!_stricmp(tbl->pProperties[0], "Haste")) {
        return 5;
    } else if (!_stricmp(tbl->pProperties[0], "Heroism")) {
        return 51;
    } else if (!_stricmp(tbl->pProperties[0], "Pain")) {  // pain reflection
        ++*next_token;
        return 95;
    } else if (!_stricmp(tbl->pProperties[0], "Sparks")) {
        return 15;
    } else if (!_stricmp(tbl->pProperties[0], "Light")) {
        ++*next_token;
        return 78;
    } else if (!_stricmp(tbl->pProperties[0], "Toxic")) {  // toxic cloud
        ++*next_token;
        return 90;
    } else if (!_stricmp(tbl->pProperties[0], "ShrapMetal")) {
        return 93;
    } else if (!_stricmp(tbl->pProperties[0], "Paralyze")) {
        return 81;
    } else if (!_stricmp(tbl->pProperties[0], "Fireball")) {
        return 6;
    } else if (!_stricmp(tbl->pProperties[0], "Incinerate")) {
        return 11;
    } else if (!_stricmp(tbl->pProperties[0], "Fire")) {
        ++*next_token;
        return 2;
    } else if (!_stricmp(tbl->pProperties[0], "Rock")) {
        ++*next_token;
        return 41;
    } else if (!_stricmp(tbl->pProperties[0], "Mass")) {
        ++*next_token;
        return 44;
    } else if (!_stricmp(tbl->pProperties[0], "Ice")) {
        ++*next_token;
        return 26;
    } else if (!_stricmp(tbl->pProperties[0], "Acid")) {
        ++*next_token;
        return 29;
    } else if (!_stricmp(tbl->pProperties[0], "Bless")) {
        return 46;
    } else if (!_stricmp(tbl->pProperties[0], "Dragon")) {
        ++*next_token;
        return 97;
    } else if (!_stricmp(tbl->pProperties[0], "Reanimate")) {
        return 89;
    } else if (!_stricmp(tbl->pProperties[0], "Summon")) {
        ++*next_token;
        return 82;
    } else if (!_stricmp(tbl->pProperties[0], "Fate")) {
        return 47;
    } else if (!_stricmp(tbl->pProperties[0], "Harm")) {
        return 70;
    } else if (!_stricmp(tbl->pProperties[0], "Mind")) {
        ++*next_token;
        return 57;
    } else if (!_stricmp(tbl->pProperties[0], "Blades")) {
        return 39;
    } else if (!_stricmp(tbl->pProperties[0], "Psychic")) {
        ++*next_token;
        return 65;
    } else if (!_stricmp(tbl->pProperties[0], "Hammerhands")) {
        return 73;
    } else {
        logger->Warning(
            "Unknown monster spell %s", tbl->pProperties[0]);
        ++*next_token;
        return 0;
    }
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
void ParseDamage(char *damage_str, unsigned __int8 *dice_rolls,
                 unsigned __int8 *dice_sides, unsigned __int8 *dmg_bonus) {
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
    if (!_stricmp(missle_attack_str, "ARROW"))
        return 1;
    else if (!_stricmp(missle_attack_str, "ARROWF"))
        return 2;
    else if (!_stricmp(missle_attack_str, "FIRE"))
        return 3;
    else if (!_stricmp(missle_attack_str, "AIR"))
        return 4;
    else if (!_stricmp(missle_attack_str, "WATER"))
        return 5;
    else if (!_stricmp(missle_attack_str, "EARTH"))
        return 6;
    else if (!_stricmp(missle_attack_str, "SPIRIT"))
        return 7;
    else if (!_stricmp(missle_attack_str, "MIND"))
        return 8;
    else if (!_stricmp(missle_attack_str, "BODY"))
        return 9;
    else if (!_stricmp(missle_attack_str, "LIGHT"))
        return 10;
    else if (!_stricmp(missle_attack_str, "DARK"))
        return 11;
    else if (!_stricmp(missle_attack_str, "ENER"))
        return 13;
    else
        return 0;
}

int ParseSpecialAttack(char *spec_att_str) {
    _strlwr(spec_att_str);
    if (strstr(spec_att_str, "curse"))
        return 1;
    else if (strstr(spec_att_str, "weak"))
        return 2;
    else if (strstr(spec_att_str, "asleep"))
        return 3;
    else if (strstr(spec_att_str, "afraid"))
        return 23;
    else if (strstr(spec_att_str, "drunk"))
        return 4;
    else if (strstr(spec_att_str, "insane"))
        return 5;
    else if (strstr(spec_att_str, "poison weak"))
        return 6;
    else if (strstr(spec_att_str, "poison medium"))
        return 7;
    else if (strstr(spec_att_str, "poison severe"))
        return 8;
    else if (strstr(spec_att_str, "disease weak"))
        return 9;
    else if (strstr(spec_att_str, "disease medium"))
        return 10;
    else if (strstr(spec_att_str, "disease severe"))
        return 11;
    else if (strstr(spec_att_str, "paralyze"))
        return 12;
    else if (strstr(spec_att_str, "uncon"))
        return 13;
    else if (strstr(spec_att_str, "dead"))
        return 14;
    else if (strstr(spec_att_str, "stone"))
        return 15;
    else if (strstr(spec_att_str, "errad"))
        return 16;
    else if (strstr(spec_att_str, "brkitem"))
        return 17;
    else if (strstr(spec_att_str, "brkarmor"))
        return 18;
    else if (strstr(spec_att_str, "brkweapon"))
        return 19;
    else if (strstr(spec_att_str, "steal"))
        return 20;
    else if (strstr(spec_att_str, "age"))
        return 21;
    else if (strstr(spec_att_str, "drainsp"))
        return 22;
    else
        return 0;
}

//----- (004598FC) --------------------------------------------------------
bool MonsterList::FromFileTxt(const char *Args) {
    MonsterList *v2;        // ebx@1
    FILE *v3;               // eax@1
    unsigned int v4;        // esi@3
    void *v5;               // eax@9
    FILE *v6;               // ST14_4@11
    char *i;                // eax@11
    signed int v8;          // esi@12
    unsigned __int16 v9;    // ax@16
    const char *v10;        // ST18_4@16
    unsigned __int16 v11;   // ax@16
    const char *v12;        // ST14_4@16
    unsigned __int16 v13;   // ax@16
    const char *v14;        // ST10_4@16
    __int16 v15;            // ax@16
    const char *v16;        // ST0C_4@16
    int v17;                // esi@16
    unsigned __int8 v18;    // al@16
    signed int v19;         // esi@16
    unsigned __int16 v20;   // ax@17
    int v21;                // ecx@17
    char Buf;               // [sp+4h] [bp-304h]@3
    FrameTableTxtLine v24;  // [sp+1F8h] [bp-110h]@4
    FrameTableTxtLine v25;  // [sp+274h] [bp-94h]@4
    int v26;                // [sp+2F0h] [bp-18h]@16
    char v27;               // [sp+2F4h] [bp-14h]@14
    char v28;               // [sp+2F5h] [bp-13h]@14
    char v29;               // [sp+2F6h] [bp-12h]@14
    char v30;               // [sp+2F7h] [bp-11h]@14
    char v31;               // [sp+2F8h] [bp-10h]@14
    char v32;               // [sp+2F9h] [bp-Fh]@14
    char v33;               // [sp+2FAh] [bp-Eh]@14
    char v34;               // [sp+2FBh] [bp-Dh]@14
    char v35;               // [sp+2FCh] [bp-Ch]@14
    char v36;               // [sp+2FDh] [bp-Bh]@14
    char v37;               // [sp+2FEh] [bp-Ah]@14
    char v38;               // [sp+2FFh] [bp-9h]@14
    char v39;               // [sp+300h] [bp-8h]@14
    char v40;               // [sp+301h] [bp-7h]@14
    char v41;               // [sp+302h] [bp-6h]@14
    char v42;               // [sp+303h] [bp-5h]@14
    FILE *File;             // [sp+304h] [bp-4h]@1
    unsigned int Argsa;     // [sp+310h] [bp+8h]@3
    int Argsb;              // [sp+310h] [bp+8h]@16

    v2 = this;
    v3 = fcaseopen(Args, "r");
    File = v3;
    if (!v3) Error("MonsterRaceListStruct::load - Unable to open file: %s.");

    v4 = 0;
    Argsa = 0;
    if (fgets(&Buf, 490, v3)) {
        do {
            *strchr(&Buf, 10) = 0;
            memcpy(&v25, frame_table_txt_parser(&Buf, &v24), sizeof(v25));
            if (v25.uPropCount && *v25.pProperties[0] != 47) ++Argsa;
        } while (fgets(&Buf, 490, File));
        v4 = Argsa;
    }
    v2->uNumMonsters = v4;
    v5 = malloc(sizeof(MonsterDesc) * v4);
    v2->pMonsters = (MonsterDesc *)v5;
    if (!v5) Error("MonsterRaceListStruct::load - Out of Memory!");

    v6 = File;
    v2->uNumMonsters = 0;
    fseek(v6, 0, 0);
    for (i = fgets(&Buf, 490, File); i; i = fgets(&Buf, 490, File)) {
        *strchr(&Buf, 10) = 0;
        memcpy(&v25, frame_table_txt_parser(&Buf, &v24), sizeof(v25));
        v8 = 0;
        if (v25.uPropCount && *v25.pProperties[0] != 47) {
            strcpy(v2->pMonsters[v2->uNumMonsters].pMonsterName,
                   v25.pProperties[0]);
            v35 = 0;
            v36 = 1;
            v37 = 7;
            v38 = 2;
            v39 = 3;
            v40 = 4;
            v41 = 5;
            v42 = 6;
            v27 = 1;
            v28 = 2;
            v29 = 3;
            v30 = 4;
            v31 = 4;
            v32 = 5;
            v33 = 6;
            v34 = 7;
            do {
                strcpy(v2->pMonsters[v2->uNumMonsters]
                           .pSpriteNames[(unsigned __int8)*(&v35 + v8)],
                       v25.pProperties[(unsigned __int8)*(&v27 + v8)]);
                ++v8;
            } while (v8 < 8);
            v9 = atoi(v25.pProperties[8]);
            v10 = v25.pProperties[9];
            v2->pMonsters[v2->uNumMonsters].uMonsterHeight = v9;
            v11 = atoi(v10);
            v12 = v25.pProperties[10];
            v2->pMonsters[v2->uNumMonsters].uMovementSpeed = v11;
            v13 = atoi(v12);
            v14 = v25.pProperties[11];
            v2->pMonsters[v2->uNumMonsters].uMonsterRadius = v13;
            v15 = atoi(v14);
            v16 = v25.pProperties[12];
            v2->pMonsters[v2->uNumMonsters].uToHitRadius = v15;
            v17 = (unsigned __int8)atoi(v16);
            Argsb = atoi(v25.pProperties[13]) & 0xFF;
            v26 = atoi(v25.pProperties[14]) & 0xFF;
            v18 = atoi(v25.pProperties[15]);
            v2->pMonsters[v2->uNumMonsters].sTintColor =
                v18 | ((v26 | ((Argsb | (v17 << 8)) << 8)) << 8);
            v19 = 0;
            do {
                v20 = atoi(v25.pProperties[v19 + 16]);
                v21 = v19++;
                v2->pMonsters[v2->uNumMonsters].pSoundSampleIDs[v21] = v20;
            } while (v19 < 4);
            ++v2->uNumMonsters;
        }
    }
    fclose(File);
    return 1;
}

//----- (004598AF) --------------------------------------------------------
void MonsterList::FromFile(void *data_mm6, void *data_mm7, void *data_mm8) {
    uint num_mm6_monsters = data_mm6 ? *(int *)data_mm6 : 0,
         num_mm7_monsters = data_mm7 ? *(int *)data_mm7 : 0,
         num_mm8_monsters = data_mm8 ? *(int *)data_mm8 : 0;

    uNumMonsters = num_mm6_monsters + num_mm7_monsters + num_mm8_monsters;
    Assert(uNumMonsters);
    Assert(!num_mm8_monsters);

    pMonsters = (MonsterDesc *)malloc(sizeof(MonsterDesc) * uNumMonsters);
    memcpy(pMonsters, (char *)data_mm7 + 4,
           num_mm7_monsters * sizeof(MonsterDesc));
    for (uint i = 0; i < num_mm6_monsters; ++i) {
        auto src = (MonsterDesc_mm6 *)((char *)data_mm6 + 4) + i;
        MonsterDesc *dst = &pMonsters[num_mm7_monsters + i];

        dst->uMonsterHeight = src->uMonsterHeight;
        dst->uMonsterRadius = src->uMonsterRadius;
        dst->uMovementSpeed = src->uMovementSpeed;
        dst->uToHitRadius = src->uToHitRadius;
        dst->sTintColor = -1;
        memcpy(dst->pSoundSampleIDs, src->pSoundSampleIDs,
               sizeof(src->pSoundSampleIDs));
        memcpy(dst->pMonsterName, src->pMonsterName, sizeof(src->pMonsterName));
        memcpy(dst->pSpriteNames, src->pSpriteNames, sizeof(src->pSpriteNames));
    }
    memcpy(pMonsters + num_mm6_monsters + num_mm7_monsters,
           (char *)data_mm8 + 4, num_mm8_monsters * sizeof(MonsterDesc));
}

//----- (00459860) --------------------------------------------------------
void MonsterList::ToFile() {
    MonsterList *v1;  // esi@1
    FILE *v2;         // eax@1
    FILE *v3;         // edi@1

    v1 = this;
    v2 = fcaseopen("data/dmonlist.bin", "wb");
    v3 = v2;
    if (!v2) Error("Unable to save dmonlist.bin!");
    fwrite(v1, 4u, 1u, v2);
    fwrite(v1->pMonsters, 0x98u, v1->uNumMonsters, v3);
    fclose(v3);
}

//----- (004563FF) --------------------------------------------------------
signed int MonsterStats::FindMonsterByTextureName(
    const char *monster_textr_name) {
    for (int i = 1; i < uNumMonsters; ++i) {
        if ((pInfos[i].pName) &&
            (!_stricmp(pInfos[i].pPictureName, monster_textr_name)))
            return i;
    }
    return -1;
}

//----- (00454F4E) --------------------------------------------------------
void MonsterStats::InitializePlacements() {
    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;
    //  int item_counter;

    pMonsterPlacementTXT_Raw = (char *)pEvents_LOD->LoadCompressedTexture("placemon.txt");
    strtok(pMonsterPlacementTXT_Raw, "\r");
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
                    pPlaceStrings[i] = RemoveQuotes(test_string);
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
void MonsterStats::Initialize() {
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
    String str;

    free(pMonstersTXT_Raw);
    pMonstersTXT_Raw = (char *)pEvents_LOD->LoadCompressedTexture("monsters.txt");
    strtok(pMonstersTXT_Raw, "\r");
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
                        pInfos[curr_rec_num].pName = RemoveQuotes(test_string);
                        break;
                    case 2:
                        pInfos[curr_rec_num].pPictureName =
                            RemoveQuotes(test_string);
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
                        pInfos[curr_rec_num].uTreasureLevel = 0;
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
                                test_string[str_pos + 1] - '0';
                            item_name = &test_string[str_pos + 2];
                            if (*item_name) {
                                if (!_stricmp(item_name, "WEAPON"))
                                    pInfos[curr_rec_num].uTreasureType = 20;
                                else if (!_stricmp(item_name, "ARMOR"))
                                    pInfos[curr_rec_num].uTreasureType = 21;
                                else if (!_stricmp(item_name, "MISC"))
                                    pInfos[curr_rec_num].uTreasureType = 22;
                                else if (!_stricmp(item_name, "SWORD"))
                                    pInfos[curr_rec_num].uTreasureType = 23;
                                else if (!_stricmp(item_name, "DAGGER"))
                                    pInfos[curr_rec_num].uTreasureType = 24;
                                else if (!_stricmp(item_name, "AXE"))
                                    pInfos[curr_rec_num].uTreasureType = 25;
                                else if (!_stricmp(item_name, "SPEAR"))
                                    pInfos[curr_rec_num].uTreasureType = 26;
                                else if (!_stricmp(item_name, "BOW"))
                                    pInfos[curr_rec_num].uTreasureType = 27;
                                else if (!_stricmp(item_name, "MACE"))
                                    pInfos[curr_rec_num].uTreasureType = 28;
                                else if (!_stricmp(item_name, "CLUB"))
                                    pInfos[curr_rec_num].uTreasureType = 29;
                                else if (!_stricmp(item_name, "STAFF"))
                                    pInfos[curr_rec_num].uTreasureType = 30;
                                else if (!_stricmp(item_name, "LEATHER"))
                                    pInfos[curr_rec_num].uTreasureType = 31;
                                else if (!_stricmp(item_name, "CHAIN"))
                                    pInfos[curr_rec_num].uTreasureType = 32;
                                else if (!_stricmp(item_name, "PLATE"))
                                    pInfos[curr_rec_num].uTreasureType = 33;
                                else if (!_stricmp(item_name, "SHIELD"))
                                    pInfos[curr_rec_num].uTreasureType = 34;
                                else if (!_stricmp(item_name, "HELM"))
                                    pInfos[curr_rec_num].uTreasureType = 35;
                                else if (!_stricmp(item_name, "BELT"))
                                    pInfos[curr_rec_num].uTreasureType = 36;
                                else if (!_stricmp(item_name, "CAPE"))
                                    pInfos[curr_rec_num].uTreasureType = 37;
                                else if (!_stricmp(item_name, "GAUNTLETS"))
                                    pInfos[curr_rec_num].uTreasureType = 38;
                                else if (!_stricmp(item_name, "BOOTS"))
                                    pInfos[curr_rec_num].uTreasureType = 39;
                                else if (!_stricmp(item_name, "RING"))
                                    pInfos[curr_rec_num].uTreasureType = 40;
                                else if (!_stricmp(item_name, "AMULET"))
                                    pInfos[curr_rec_num].uTreasureType = 41;
                                else if (!_stricmp(item_name, "WAND"))
                                    pInfos[curr_rec_num].uTreasureType = 42;
                                else if (!_stricmp(item_name, "SCROLL"))
                                    pInfos[curr_rec_num].uTreasureType = 43;
                                else if (!_stricmp(item_name, "GEM"))
                                    pInfos[curr_rec_num].uTreasureType = 46;
                            }
                        }
                    } break;
                    case 8: {
                        pInfos[curr_rec_num].bQuestMonster = 0;
                        if (atoi(test_string))
                            pInfos[curr_rec_num].bQuestMonster = 1;
                    } break;
                    case 9: {
                        pInfos[curr_rec_num].uFlying = false;
                        if (_strnicmp(test_string, "n", 1))
                            pInfos[curr_rec_num].uFlying = true;
                    } break;
                    case 10: {
                        switch (tolower(test_string[0])) {
                            case 's':
                                pInfos[curr_rec_num].uMovementType =
                                    MONSTER_MOVEMENT_TYPE_SHORT;  // short
                                if (tolower(test_string[1]) != 'h')
                                    pInfos[curr_rec_num].uMovementType =
                                        MONSTER_MOVEMENT_TYPE_STAIONARY;  // stationary
                                break;  // short
                            case 'l':
                                pInfos[curr_rec_num].uMovementType =
                                    MONSTER_MOVEMENT_TYPE_LONG;
                                break;  // long
                            case 'm':
                                pInfos[curr_rec_num].uMovementType =
                                    MONSTER_MOVEMENT_TYPE_MEDIUM;
                                break;  // med
                            case 'g':
                                pInfos[curr_rec_num].uMovementType =
                                    MONSTER_MOVEMENT_TYPE_GLOBAL;
                                break;  // global?
                            default:
                                pInfos[curr_rec_num].uMovementType =
                                    MONSTER_MOVEMENT_TYPE_FREE;  // free
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
                        char type_flag;
                        strcpy(parse_str, test_string);
                        parse_str[0] = ' ';
                        parse_str[strlen(parse_str) - 1] = ' ';
                        frame_table_txt_parser(parse_str, &parsed_field);
                        if (parsed_field.uPropCount > 2) {
                            param_num = 1;
                            pInfos[curr_rec_num].uSpell1ID =
                                ParseSpellType(&parsed_field, &param_num);
                            type_flag = *parsed_field.pProperties[param_num];
                            pInfos[curr_rec_num].uSpellSkillAndMastery1 =
                                atoi(parsed_field.pProperties[param_num + 1]) &
                                0x003F;
                            switch (type_flag) {
                                case 'E':
                                    pInfos[curr_rec_num]
                                        .uSpellSkillAndMastery1 |= 0x0040;
                                    break;
                                case 'M':
                                    pInfos[curr_rec_num]
                                        .uSpellSkillAndMastery1 |= 0x0080;
                                    break;
                                case 'G':
                                    pInfos[curr_rec_num]
                                        .uSpellSkillAndMastery1 |= 0x0100;
                                    break;
                            }
                        } else {
                            pInfos[curr_rec_num].uSpell1ID = 0;
                            pInfos[curr_rec_num].uSpellSkillAndMastery1 = 0;
                        }
                    } break;
                    case 26:
                        pInfos[curr_rec_num].uSpell2UseChance =
                            atoi(test_string);
                        break;
                    case 27: {
                        int param_num;
                        char type_flag;
                        strcpy(parse_str, test_string);
                        parse_str[0] = ' ';
                        parse_str[strlen(parse_str) - 1] = ' ';
                        frame_table_txt_parser(parse_str, &parsed_field);
                        if (parsed_field.uPropCount > 2) {
                            param_num = 1;
                            pInfos[curr_rec_num].uSpell2ID =
                                ParseSpellType(&parsed_field, &param_num);
                            type_flag = *parsed_field.pProperties[param_num];
                            pInfos[curr_rec_num].uSpellSkillAndMastery2 =
                                atoi(parsed_field.pProperties[param_num + 1]) &
                                0x003F;
                            switch (type_flag) {
                                case 'E':
                                    pInfos[curr_rec_num]
                                        .uSpellSkillAndMastery2 |= 0x0040;
                                    break;
                                case 'M':
                                    pInfos[curr_rec_num]
                                        .uSpellSkillAndMastery2 |= 0x0080;
                                    break;
                                case 'G':
                                    pInfos[curr_rec_num]
                                        .uSpellSkillAndMastery2 |= 0x0100;
                                    break;
                            }
                        } else {
                            pInfos[curr_rec_num].uSpell2ID = 0;
                            pInfos[curr_rec_num].uSpellSkillAndMastery2 = 0;
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
                                if (!_stricmp(parsed_field.pProperties[0],
                                              "shot")) {
                                    pInfos[curr_rec_num].uSpecialAbilityType =
                                        1;
                                    pInfos[curr_rec_num]
                                        .uSpecialAbilityDamageDiceBonus = atoi(
                                        (char *)(parsed_field.pProperties[1] +
                                                 1));
                                } else if (!_stricmp(
                                               parsed_field.pProperties[0],
                                               "summon")) {
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
                                        if (pMonsterList->uNumMonsters) {
                                            pInfos[curr_rec_num]
                                                .field_3C_some_special_attack =
                                                pMonsterList
                                                    ->GetMonsterIDByName(
                                                        str.c_str()) +
                                                1;
                                            if (pInfos[curr_rec_num]
                                                    .field_3C_some_special_attack ==
                                                -1) {
                                                logger->Warning(
                                                        "Can't create random "
                                                        "monster: '%s' See "
                                                        "MapStats!",
                                                        str.c_str());
                                            }
                                        }
                                        pInfos[curr_rec_num]
                                            .uSpecialAbilityDamageDiceSides = 0;
                                        if (!_stricmp(
                                                parsed_field.pProperties[1],
                                                "ground"))
                                            pInfos[curr_rec_num]
                                                .uSpecialAbilityDamageDiceSides =
                                                1;
                                        if (pInfos[curr_rec_num]
                                                .field_3C_some_special_attack ==
                                            -1)
                                            pInfos[curr_rec_num]
                                                .uSpecialAbilityType = 0;
                                    }
                                } else if (!_stricmp(
                                               parsed_field.pProperties[0],
                                               "explode")) {
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
signed __int16 MonsterList::GetMonsterIDByName(const char *pMonsterName) {
    if (!pMonsterName) return -1;
    for (signed __int16 i = 0; i <= uNumMonsters; ++i) {
        if ((!_stricmp(pMonsters[i].pMonsterName, pMonsterName))) return i;
    }
    Error("Monster not found: %s", pMonsterName);
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
