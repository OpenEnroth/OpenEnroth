#include "NPCTable.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <memory>

#include "Engine/Objects/NPC.h"
#include "Engine/Party.h"
#include "Engine/GameResourceManager.h"
#include "Library/Random/Random.h"
#include "Utility/String.h"
#include "Library/Random/RandomEngine.h"
#include "Utility/Memory/Blob.h"
#include "Utility/Segment.h"
#include "Utility/Workaround/ToUnderlying.h"

enum class MAP_TYPE : uint32_t;

std::array<NPCTopic, 789> pNPCTopics;
struct NPCStats *pNPCStats = nullptr;

int NPCStats::dword_AE336C_LastMispronouncedNameFirstLetter = -1;
int NPCStats::dword_AE3370_LastMispronouncedNameResult = -1;

//----- (00476977) --------------------------------------------------------
void NPCStats::InitializeNPCText(const Blob &npcText) {
    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    std::string txtRaw(npcText.string_view());
    strtok(txtRaw.data(), "\r");

    for (i = 0; i < 789; ++i) {
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
                    pNPCTopics[i].pText = removeQuotes(test_string);
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 2) && !break_loop);
    }
}

void NPCStats::InitializeNPCTopics(const Blob &npcTopics) {
    std::string txtRaw(npcTopics.string_view());
    strtok(txtRaw.data(), "\r");

    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    for (int i = 1; i <= 579; ++i) {  // NPC topics count limit
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
                    pNPCTopics[i].pTopic = removeQuotes(test_string);
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 2) && !break_loop);
    }
}

void NPCStats::InitializeNPCDist(const Blob &npcDist) {
    std::string txtRaw(npcDist.string_view());
    strtok(txtRaw.data(), "\r");
    strtok(NULL, "\r");

    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    for (int i = 1; i < 59; ++i) {
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
                if ((decode_step > 0) && (decode_step < 77)) {
                    pProfessionChance[decode_step].professionChancePerArea[i] = atoi(test_string);
                } else if (decode_step == 0) {
                    pProfessionChance[0].professionChancePerArea[i] = 10;
                }
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 78) && !break_loop);
    }

    for (int i = 0; i < 77; ++i) {
        pProfessionChance[i].uTotalprofChance = 0;
        for (int ii = 1; ii < 59; ++ii) {
            pProfessionChance[i].uTotalprofChance += pProfessionChance[i].professionChancePerArea[ii];
        }
        pProfessionChance[i].professionChancePerArea[0] = 0;
        pProfessionChance[i].professionChancePerArea[59] = 0;
    }
}

// TODO(Nik-RE-dev): move out of table back to Engine/Objects/NPC.cpp
void NPCStats::setNPCNamesOnLoad() {
    for (unsigned int i = 1; i < uNumNewNPCs; ++i)
        pNewNPCData[i].pName = pNPCUnicNames[i - 1];

    if (!pParty->pHirelings[0].pName.empty())
        pParty->pHirelings[0].pName = pParty->pHireling1Name;
    if (!pParty->pHirelings[1].pName.empty())
        pParty->pHirelings[1].pName = pParty->pHireling2Name;
}

//----- (00476CB5) --------------------------------------------------------
void NPCStats::InitializeNPCData(const Blob &npcData) {
    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    std::string txtRaw(npcData.string_view());
    strtok(txtRaw.data(), "\r");
    strtok(NULL, "\r");

    for (i = 0; i < 500; ++i) {
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
            if (temp_str_len) {  // i+1
                switch (decode_step) {
                    case 1:
                        pNPCUnicNames[i] = removeQuotes(test_string);
                        pNPCData[i + 1].pName = pNPCUnicNames[i];
                        break;
                    case 2:
                        pNPCData[i + 1].uPortraitID = atoi(test_string);
                        break;
                    case 6:
                        pNPCData[i + 1].Location2D = atoi(test_string);
                        break;
                    case 7:
                        pNPCData[i + 1].profession = (NPCProf)atoi(test_string);
                        break;
                    case 8:
                        pNPCData[i + 1].greet = atoi(test_string);
                        break;
                    case 9:
                        pNPCData[i + 1].is_joinable = (*test_string == 'y') ? 1 : 0;
                        break;
                    case 10:
                        pNPCData[i + 1].dialogue_1_evt_id = atoi(test_string);
                        break;
                    case 11:
                        pNPCData[i + 1].dialogue_2_evt_id = atoi(test_string);
                        break;
                    case 12:
                        pNPCData[i + 1].dialogue_3_evt_id = atoi(test_string);
                        break;
                    case 13:
                        pNPCData[i + 1].dialogue_4_evt_id = atoi(test_string);
                        break;
                    case 14:
                        pNPCData[i + 1].dialogue_5_evt_id = atoi(test_string);
                        break;
                    case 15:
                        pNPCData[i + 1].dialogue_6_evt_id = atoi(test_string);
                        break;
                }
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 16) && !break_loop);
    }
    uNumNewNPCs = 501;
}

void NPCStats::InitializeNPCGreets(const Blob &npcGreets) {
    std::string txtRaw(npcGreets.string_view());
    strtok(txtRaw.data(), "\r");

    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    for (int i = 1; i <= 205; ++i) {
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
            if (temp_str_len) {  // i+1
                switch (decode_step) {
                    case 1:
                        pNPCGreetings[i].pGreeting1 = removeQuotes(test_string);
                        break;
                    case 2:
                        pNPCGreetings[i].pGreeting2 = removeQuotes(test_string);
                        break;
                }
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 3) && !break_loop);
    }
}

void NPCStats::InitializeNPCGroups(const Blob &npcGroups) {
    std::string txtRaw(npcGroups.string_view());
    strtok(txtRaw.data(), "\r");

    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    for (int i = 0; i < 51; ++i) {
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
            if (temp_str_len) {  // i+1
                if (decode_step == 1) {
                    pGroups[i] = atoi(test_string);
                }
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 2) && !break_loop);
    }
}

void NPCStats::InitializeNPCNews(const Blob &npcNews) {
    std::string txtRaw(npcNews.string_view());
    strtok(txtRaw.data(), "\r");

    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    for (int i = 0; i < 51; ++i) {
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
            if (temp_str_len) {  // i+1
                if (decode_step == 1)
                    pCatchPhrases[i] = removeQuotes(test_string);
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 2) && !break_loop);
    }
}

//----- (0047702F) --------------------------------------------------------
void NPCStats::Initialize(GameResourceManager *resourceManager) {
    pNPCData.fill(NPCData());
    InitializeNPCData(resourceManager->getEventsFile("npcdata.txt"));
    InitializeNPCGreets(resourceManager->getEventsFile("npcgreet.txt"));
    InitializeNPCGroups(resourceManager->getEventsFile("npcgroup.txt"));
    InitializeNPCNews(resourceManager->getEventsFile("npcnews.txt"));
    InitializeNPCText(resourceManager->getEventsFile("npctext.txt"));
    InitializeNPCTopics(resourceManager->getEventsFile("npctopic.txt"));
    InitializeNPCDist(resourceManager->getEventsFile("npcdist.txt"));
    InitializeNPCNames(resourceManager->getEventsFile("npcnames.txt"));
    InitializeNPCProfs(resourceManager->getEventsFile("npcprof.txt"));
}

void NPCStats::InitializeNPCNames(const Blob &npcNames) {
    std::string txtRaw(npcNames.string_view());
    strtok(txtRaw.data(), "\r");

    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos = nullptr;
    int decode_step;

    uNewlNPCBufPos = 0;

    for (i = 0; i < 540; ++i) {
        test_string = strtok(NULL, "\r") + 1;
        break_loop = false;
        decode_step = 0;
        do {
            c = *(unsigned char *)test_string;
            temp_str_len = 0;
            if (c == '\t') {
                if ((decode_step == 1) && (!uNumNPCNames[1]))
                    uNumNPCNames[1] = i;
            } else {
                while ((c != '\n') && (c != '\t') && (c > 0)) {
                    ++temp_str_len;
                    c = test_string[temp_str_len];
                }
                tmp_pos = test_string + temp_str_len;
                if (*tmp_pos == 0) break_loop = true;

                if (temp_str_len) {
                    *tmp_pos = 0;
                    if (decode_step == 0)
                        pNPCNames[i][0] = removeQuotes(test_string);
                    else if (decode_step == 1)
                        pNPCNames[i][1] = removeQuotes(test_string);
                } else {
                    if ((decode_step == 1) && (!uNumNPCNames[1]))
                        uNumNPCNames[1] = i;
                }
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 2) && !break_loop);
    }
    uNumNPCNames[0] = i;
}

void NPCStats::InitializeNPCProfs(const Blob &npcProfs) {
    std::string txtRaw(npcProfs.string_view());
    strtok(txtRaw.data(), "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");

    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos = nullptr;
    int decode_step;

    for (NPCProf i : Segment(NPC_PROFESSION_FIRST_VALID, NPC_PROFESSION_LAST_VALID)) {
        test_string = strtok(NULL, "\r") + 1;
        break_loop = false;
        decode_step = 0;
        do {
            // while (*test_string == '\t')  // some steps are separated by
            // multiple \t's
            // ++test_string;

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
                    case 2:
                        pProfessions[i].uHirePrice = atoi(test_string);
                        break;
                    case 3:
                        pProfessions[i].pActionText = removeQuotes(test_string);
                        break;
                    case 4:
                        pProfessions[i].pBenefits = removeQuotes(test_string);
                        break;
                    case 5:
                        pProfessions[i].pJoinText = removeQuotes(test_string);
                        break;
                    case 6:
                        pProfessions[i].pDismissText = removeQuotes(test_string);
                }
            } else {
                if (!decode_step) break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 7) && !break_loop);
    }
    uNumNPCProfessions = 59;
}

//----- (0047732C) --------------------------------------------------------
void NPCStats::InitializeAdditionalNPCs(NPCData *pNPCDataBuff, int npc_uid,
                                        int uLocation2D, MapId uMapId) {
    int rep_gen;
    int uNPCSex;              // esi@1
    int uGeneratedPortret;    // ecx@23
    int test_prof_summ;       // ecx@37
    int gen_profession;       // eax@37
    int max_prof_cap;         // edx@37
                              // signed int result; // eax@39
    int uRace;                // [sp+Ch] [bp-Ch]@1
    bool break_gen;           // [sp+10h] [bp-8h]@1
    signed int gen_attempts;  // [sp+14h] [bp-4h]@1
    int uPortretMin;          // [sp+24h] [bp+Ch]@1
    int uPortretMax;

    static const uint8_t NPCSexGenTable[86] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0,
        1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0};
    static const uint8_t NPCRaceGenTable[86] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3,
        1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0, 0};

    uint8_t seed = (uint8_t)((double)(npc_uid - 1) / 3.0);
    uNPCSex = NPCSexGenTable[seed];
    uRace = NPCRaceGenTable[seed];
    pNPCDataBuff->uSex = uNPCSex;
    pNPCDataBuff->pName = pNPCNames[grng->random(uNumNPCNames[uNPCSex])][uNPCSex];

    gen_attempts = 0;
    break_gen = false;

    do {
        switch (uRace) {
            case 0:
                if (uNPCSex == 0) {
                    uPortretMin = 2;
                    uPortretMax = 100;
                } else {
                    uPortretMin = 201;
                    uPortretMax = 250;
                }
            case 1:
                if (uNPCSex == 0) {
                    uPortretMin = 400;
                    uPortretMax = 430;
                } else {
                    uPortretMin = 460;
                    uPortretMax = 490;
                }
                break;
            case 2:
                if (uNPCSex == 0) {
                    uPortretMin = 500;
                    uPortretMax = 520;
                } else {
                    uPortretMin = 530;
                    uPortretMax = 550;
                }
                break;
            case 3:
                if (uNPCSex == 0) {
                    uPortretMin = 300;
                    uPortretMax = 330;
                } else {
                    uPortretMin = 360;
                    uPortretMax = 387;
                }

                break;
        }

        uGeneratedPortret =
            uPortretMin + grng->random(uPortretMax - uPortretMin + 1);
        if (CheckPortretAgainstSex(uGeneratedPortret, uNPCSex))
            break_gen = true;
        ++gen_attempts;
        if (gen_attempts >= 4) {
            uGeneratedPortret = uPortretMin;
            break_gen = true;
        }
    } while (!break_gen);

    pNPCDataBuff->uPortraitID = uGeneratedPortret;
    pNPCDataBuff->uFlags = 0;
    pNPCDataBuff->fame = 0;
    // generate reputation
    rep_gen = grng->random(100) + 1;

    if (rep_gen >= 60) {
        if (rep_gen >= 90) {
            if (rep_gen >= 95) {
                if (rep_gen >= 98)
                    pNPCDataBuff->rep = -600;
                else
                    pNPCDataBuff->rep = 400;
            } else {
                pNPCDataBuff->rep = -300;
            }
        } else {
            pNPCDataBuff->rep = 200;
        }
    } else {
        pNPCDataBuff->rep = 0;
    }

    max_prof_cap = grng->random(pProfessionChance[std::to_underlying(uMapId)].uTotalprofChance) + 1;
    test_prof_summ = 0;
    gen_profession = 0;

    if (max_prof_cap > 0) {
        do {
            test_prof_summ += pProfessionChance[std::to_underlying(uMapId)]
                .professionChancePerArea[gen_profession++];
        } while (test_prof_summ < max_prof_cap);
    }
    pNPCDataBuff->profession = (NPCProf)(gen_profession - 1);
    pNPCDataBuff->Location2D = uLocation2D;
    pNPCDataBuff->field_24 = 1;
    pNPCDataBuff->is_joinable = 1;
    pNPCDataBuff->dialogue_1_evt_id = 0;
    pNPCDataBuff->dialogue_2_evt_id = 0;
    pNPCDataBuff->dialogue_3_evt_id = 0;
    pNPCDataBuff->dialogue_4_evt_id = 0;
    pNPCDataBuff->dialogue_5_evt_id = 0;
    pNPCDataBuff->dialogue_6_evt_id = 0;
}

//----- (00495366) --------------------------------------------------------
const std::string &NPCStats::sub_495366_MispronounceName(uint8_t firstLetter, uint8_t genderId) {
    int pickedName;  // edx@2

    if (firstLetter == dword_AE336C_LastMispronouncedNameFirstLetter) {
        pickedName = dword_AE3370_LastMispronouncedNameResult;
    } else {
        dword_AE336C_LastMispronouncedNameFirstLetter = firstLetter;
        if (this->uNumNPCNames[genderId] == 0) {
            pickedName = vrng->random(this->uNumNPCNames[(genderId + 1) % 2]);
                     // originally without " + 1) % 2", but
                     // that would yield a div by zero
        } else {
            int rangeBottom = 0;
            int rangeTop = 0;
            for (int i = 0; i < this->uNumNPCNames[genderId]; ++i) {
                if (tolower(this->pNPCNames[i][genderId][0])) {
                    if (rangeBottom)
                        rangeTop = i;
                    else
                        rangeBottom = i;
                }
            }
            if (rangeTop != 0)
                pickedName = rangeBottom + vrng->random(rangeTop - rangeBottom);
            else
                pickedName = vrng->random(this->uNumNPCNames[genderId]);
        }
    }
    dword_AE3370_LastMispronouncedNameResult = pickedName;
    return this->pNPCNames[pickedName][genderId];
}
