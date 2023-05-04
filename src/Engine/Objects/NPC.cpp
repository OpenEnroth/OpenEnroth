#include "Engine/Objects/NPC.h"

#include "Engine/Autonotes.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Awards.h"
#include "Engine/Events/Processor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Localization.h"
#include "Engine/LOD.h"
#include "Engine/MapInfo.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Party.h"
#include "Engine/Spells/CastSpellInfo.h"

#include "GUI/GUIButton.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UITransition.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Random/Random.h"



int pDialogueNPCCount;
std::array<class Image *, 6> pDialogueNPCPortraits;
int uNumDialogueNPCPortraits;
struct NPCStats *pNPCStats = nullptr;

int NPCStats::dword_AE336C_LastMispronouncedNameFirstLetter = -1;
int NPCStats::dword_AE3370_LastMispronouncedNameResult = -1;

void InitializeAwards();
void InitializeScrolls();
void InitializeMerchants();
void InitializeTransitions();
void InitializeAutonotes();
void InitializeQuests();
bool CheckPortretAgainstSex(int portret_num, int sex);

// All conditions for alive character excluding zombie
static const Segment<Condition> standardConditionsExcludeDead = {Condition_Cursed, Condition_Unconscious};

// All conditions including dead character ones, but still excluding zombie
static const Segment<Condition> standardConditionsIncludeDead = {Condition_Cursed, Condition_Eradicated};

//----- (004459F9) --------------------------------------------------------
NPCData *GetNPCData(signed int npcid) {
    NPCData *result;

    if (npcid >= 0) {
        if (npcid < 5000) {
            if (npcid >= 501) {
                logger->warning("NPC id exceeds MAX_DATA!");
            }
            return &pNPCStats->pNewNPCData[npcid];  // - 1];
        }
        return &pNPCStats->pAdditionalNPC[npcid - 5000];
    }

    if (npcid >= 5000) return &pNPCStats->pAdditionalNPC[npcid - 5000];
    if (sDialogue_SpeakingActorNPC_ID >= 0) {
        result = 0;
    } else {
        FlatHirelings buf;
        buf.Prepare();

        result = buf.Get(abs(sDialogue_SpeakingActorNPC_ID) - 1);
    }
    return result;
}

//----- (00445B2C) --------------------------------------------------------
struct NPCData *GetNewNPCData(signed int npcid, int *npc_indx) {
    NPCData *result;

    if (npcid >= 0) {
        if (npcid < 5000) {
            if (npcid >= 501) {
                logger->warning("NPC id exceeds MAX_DATA!");
            }
            *npc_indx = npcid;
            return &pNPCStats->pNewNPCData[npcid];
        }
        *npc_indx = npcid - 5000;
        return &pNPCStats->pAdditionalNPC[npcid - 5000];
    }
    if (npcid >= 5000) {
        *npc_indx = npcid - 5000;
        return &pNPCStats->pAdditionalNPC[npcid - 5000];
    }
    if (sDialogue_SpeakingActorNPC_ID >= 0) {
        *npc_indx = 0;
        result = nullptr;
    } else {
        FlatHirelings buf;
        buf.Prepare();

        result = buf.Get(abs(sDialogue_SpeakingActorNPC_ID) - 1);
    }
    return result;
}

//----- (00476977) --------------------------------------------------------
void NPCStats::InitializeNPCText() {
    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    pNPCTextTXT_Raw = pEvents_LOD->LoadCompressedTexture("npctext.txt").string_view();
    strtok(pNPCTextTXT_Raw.data(), "\r");

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
    pNPCTopicTXT_Raw = pEvents_LOD->LoadCompressedTexture("npctopic.txt").string_view();
    strtok(pNPCTopicTXT_Raw.data(), "\r");

    for (i = 1; i <= 579; ++i) {  // NPC topics count limit
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

    pNPCDistTXT_Raw = pEvents_LOD->LoadCompressedTexture("npcdist.txt").string_view();
    strtok(pNPCDistTXT_Raw.data(), "\r");
    strtok(NULL, "\r");

    for (i = 1; i < 59; ++i) {
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
                    pProfessionChance[decode_step].professionChancePerArea[i] =
                        atoi(test_string);
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

    for (i = 0; i < 77; ++i) {
        pProfessionChance[i].uTotalprofChance = 0;
        for (int ii = 1; ii < 59; ++ii) {
            pProfessionChance[i].uTotalprofChance +=
                pProfessionChance[i].professionChancePerArea[ii];
        }
        pProfessionChance[i].professionChancePerArea[0] = 0;
        pProfessionChance[i].professionChancePerArea[59] = 0;
    }

    pNPCDistTXT_Raw.clear();
}

//----- (00476C60) --------------------------------------------------------
void NPCStats::OnLoadSetNPC_Names() {
    for (unsigned int i = 1; i < uNumNewNPCs; ++i)
        pNewNPCData[i].pName = pNPCUnicNames[i - 1];

    if (!pParty->pHirelings[0].pName.empty())
        pParty->pHirelings[0].pName = pParty->pHireling1Name;
    if (!pParty->pHirelings[1].pName.empty())
        pParty->pHirelings[1].pName = pParty->pHireling2Name;
}

//----- (00476CB5) --------------------------------------------------------
void NPCStats::InitializeNPCData() {
    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    pNPCDataTXT_Raw = pEvents_LOD->LoadCompressedTexture("npcdata.txt").string_view();
    strtok(pNPCDataTXT_Raw.data(), "\r");
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
    pNPCGreetTXT_Raw = pEvents_LOD->LoadCompressedTexture("npcgreet.txt").string_view();
    strtok(pNPCGreetTXT_Raw.data(), "\r");
    for (i = 1; i <= 205; ++i) {
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
                        pNPCGreetings[i].pGreetings[0] =
                            removeQuotes(test_string);
                        break;
                    case 2:
                        pNPCGreetings[i].pGreetings[1] =
                            removeQuotes(test_string);
                        break;
                }
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 3) && !break_loop);
    }

    pNCPGroupTXT_Raw = pEvents_LOD->LoadCompressedTexture("npcgroup.txt").string_view();
    strtok(pNCPGroupTXT_Raw.data(), "\r");

    for (i = 0; i < 51; ++i) {
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

    pNPCNewsTXT_Raw = pEvents_LOD->LoadCompressedTexture("npcnews.txt").string_view();
    strtok(pNPCNewsTXT_Raw.data(), "\r");

    for (i = 0; i < 51; ++i) {
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
void NPCStats::Initialize() {
    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos = nullptr;
    int decode_step;

    InitializeNPCData();
    InitializeNPCText();
    InitializeQuests();
    InitializeAutonotes();
    InitializeAwards();
    InitializeTransitions();
    InitializeMerchants();
    InitializeScrolls();

    pNPCNamesTXT_Raw = pEvents_LOD->LoadCompressedTexture("npcnames.txt").string_view();
    strtok(pNPCNamesTXT_Raw.data(), "\r");

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

    pNPCProfTXT_Raw = pEvents_LOD->LoadCompressedTexture("npcprof.txt").string_view();
    strtok(pNPCProfTXT_Raw.data(), "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");
    strtok(NULL, "\r");

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
                        pProfessions[i].pDismissText =
                            removeQuotes(test_string);
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

//----- (00477266) --------------------------------------------------------
void NPCStats::Release() {
    pNPCTopicTXT_Raw.clear();
    pNPCTextTXT_Raw.clear();
    pNPCNewsTXT_Raw.clear();
    pNPCProfTXT_Raw.clear();
    pNPCNamesTXT_Raw.clear();
    pNPCDataTXT_Raw.clear();
    pNPCDistTXT_Raw.clear();
    pNPCGreetTXT_Raw.clear();
    pNCPGroupTXT_Raw.clear();
}

//----- (0047730C) --------------------------------------------------------
bool CheckPortretAgainstSex(int a1, int) { return true; }
// 47730C: using guessed type int __stdcall const_1(int);

//----- (0047732C) --------------------------------------------------------
void NPCStats::InitializeAdditionalNPCs(NPCData *pNPCDataBuff, int npc_uid,
                                        int uLocation2D, int uMapId) {
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

    max_prof_cap = grng->random(pProfessionChance[uMapId].uTotalprofChance) + 1;
    test_prof_summ = 0;
    gen_profession = 0;

    if (max_prof_cap > 0) {
        do {
            test_prof_summ += pProfessionChance[uMapId]
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
char *NPCStats::sub_495366_MispronounceName(uint8_t firstLetter,
                                            uint8_t genderId) {
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
            for (uint i = 0; i < this->uNumNPCNames[genderId]; ++i) {
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

//----- (00476387) --------------------------------------------------------
bool PartyHasDragon() { return pNPCStats->pNewNPCData[57].Hired(); }

//----- (00476395) --------------------------------------------------------
// 0x26 Wizard eye at skill level 2
bool CheckHiredNPCSpeciality(NPCProf prof) {
    if (bNoNPCHiring == 1) return false;

    for (uint i = 0; i < pNPCStats->uNumNewNPCs; ++i) {
        if (pNPCStats->pNewNPCData[i].profession == prof &&
            (pNPCStats->pNewNPCData[i].uFlags & 0x80)) {
            return true;
        }
    }
    return pParty->pHirelings[0].profession == prof
        || pParty->pHirelings[1].profession == prof;
}

//----- (004763E0) --------------------------------------------------------
void InitializeAwards() {
    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    pAwardsTXT_Raw = pEvents_LOD->LoadCompressedTexture("awards.txt").string_view();
    strtok(pAwardsTXT_Raw.data(), "\r");

    for (i = 1; i < 105; ++i) {
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
                    pAwards[i].pText = removeQuotes(test_string);
                else if (decode_step == 2)
                    pAwards[i].uPriority = atoi(test_string);
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 3) && !break_loop);
    }
}
// 7241C8: using guessed type int dword_7241C8;

//----- (004764C2) --------------------------------------------------------
void InitializeScrolls() {
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    pScrollsTXT_Raw = pEvents_LOD->LoadCompressedTexture("scroll.txt").string_view();
    strtok(pScrollsTXT_Raw.data(), "\r");
    for (ITEM_TYPE i : pScrolls.indices()) {
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
                if (decode_step == 1) pScrolls[i] = removeQuotes(test_string);
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 2) && !break_loop);
    }
}

//----- (00476590) --------------------------------------------------------
void InitializeMerchants() {
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    pMerchantsTXT_Raw = pEvents_LOD->LoadCompressedTexture("merchant.txt").string_view();
    strtok(pMerchantsTXT_Raw.data(), "\r");

    for (MERCHANT_PHRASE i : MerchantPhrases()) {
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
                    case 1:
                        pMerchantsBuyPhrases[i] = removeQuotes(test_string);
                        break;
                    case 2:
                        pMerchantsSellPhrases[i] = removeQuotes(test_string);
                        break;
                    case 3:
                        pMerchantsRepairPhrases[i] = removeQuotes(test_string);
                        break;
                    case 4:
                        pMerchantsIdentifyPhrases[i] = removeQuotes(test_string);
                        break;
                }
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 5) && !break_loop);
    }
}

//----- (00476682) --------------------------------------------------------
void InitializeTransitions() {
    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    pTransitionsTXT_Raw = pEvents_LOD->LoadCompressedTexture("trans.txt").string_view();
    strtok(pTransitionsTXT_Raw.data(), "\r");

    for (i = 0; i < 464; ++i) {
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
                    pTransitionStrings[i + 1] = removeQuotes(test_string);
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 2) && !break_loop);
    }
}

//----- (00476750) --------------------------------------------------------
void InitializeAutonotes() {
    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    pAutonoteTXT_Raw = pEvents_LOD->LoadCompressedTexture("autonote.txt").string_view();
    strtok(pAutonoteTXT_Raw.data(), "\r");

    for (i = 0; i < 195; ++i) {
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
                    case 1:
                        pAutonoteTxt[i + 1].pText = removeQuotes(test_string);
                        break;
                    case 2: {
                        if (iequals(test_string, "potion")) {
                            pAutonoteTxt[i + 1].eType = AUTONOTE_POTION_RECEPIE;
                            break;
                        }
                        if (iequals(test_string, "stat")) {
                            pAutonoteTxt[i + 1].eType = AUTONOTE_STAT_HINT;
                            break;
                        }
                        if (iequals(test_string, "seer")) {
                            pAutonoteTxt[i + 1].eType = AUTONOTE_SEER;
                            break;
                        }
                        if (iequals(test_string, "obelisk")) {
                            pAutonoteTxt[i + 1].eType = AUTONOTE_OBELISK;
                            break;
                        }
                        if (iequals(test_string, "teacher")) {
                            pAutonoteTxt[i + 1].eType = AUTONOTE_TEACHER;
                            break;
                        }
                        pAutonoteTxt[i + 1].eType = AUTONOTE_MISC;
                        break;
                    }
                }
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 3) && !break_loop);
    }
}

//----- (004768A9) --------------------------------------------------------
void InitializeQuests() {
    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    pQuestsTXT_Raw = pEvents_LOD->LoadCompressedTexture("quests.txt").string_view();
    strtok(pQuestsTXT_Raw.data(), "\r");
    memset(pQuestTable.data(), 0, sizeof(pQuestTable));
    for (i = 0; i < 512; ++i) {
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
                    pQuestTable[i + 1] = removeQuotes(test_string);
            } else {
                break_loop = true;
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 2) && !break_loop);
    }
}

//----- (004B40E6) --------------------------------------------------------
void NPCHireableDialogPrepare() {
    signed int v0;  // ebx@1
    NPCData *v1;    // edi@1

    v0 = 0;
    v1 = HouseNPCData[pDialogueNPCCount + -(dword_591080 != 0)];  //- 1
    pDialogueWindow->Release();
    pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, {render->GetRenderDimensions().w, 350}, 0);
    pBtn_ExitCancel = pDialogueWindow->CreateButton({471, 445}, {169, 35}, 1, 0,
        UIMSG_Escape, 0, InputAction::Invalid, localization->GetString(LSTR_CANCEL), {ui_exit_cancel_button_background}
    );
    pDialogueWindow->CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_BuyInShop_Identify_Repair, 0);
    if (pNPCStats->pProfessions[v1->profession].pBenefits) {
        pDialogueWindow->CreateButton({480, 160}, {140, 30}, 1, 0,
            UIMSG_ClickNPCTopic, DIALOGUE_PROFESSION_DETAILS, InputAction::Invalid, localization->GetString(LSTR_MORE_INFORMATION)
        );
        v0 = 1;
    }
    pDialogueWindow->CreateButton({480, 30 * v0 + 160}, {140, 30}, 1, 0,
        UIMSG_ClickNPCTopic, DIALOGUE_HIRE_FIRE, InputAction::Invalid, localization->GetString(LSTR_HIRE));
    pDialogueWindow->_41D08F_set_keyboard_control_group(v0 + 1, 1, 0, 2);
    dialog_menu_id = DIALOGUE_OTHER;
}

//----- (004B4224) --------------------------------------------------------
void _4B4224_UpdateNPCTopics(int _this) {
    int num_menu_buttons;  // ebx@1
    int i;                 // ebp@5
                           // signed int v4; // ebp@9
    NPCData *v17;          // [sp+10h] [bp-4h]@4

    num_menu_buttons = 0;
    pDialogueNPCCount = (_this + 1);
    Sizei renDims = render->GetRenderDimensions();
    if (_this + 1 == uNumDialogueNPCPortraits && uHouse_ExitPic) {
        pDialogueWindow->Release();
        pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, renDims, 0);
        transition_button_label = localization->FormatString(LSTR_FMT_ENTER_S, pMapStats->pInfos[uHouse_ExitPic].pName.c_str()
        );
        pBtn_ExitCancel = pDialogueWindow->CreateButton({566, 445}, {75, 33}, 1, 0,
            UIMSG_Escape, 0, InputAction::No, localization->GetString(LSTR_CANCEL), {ui_buttdesc2});
        pBtn_YES = pDialogueWindow->CreateButton({486, 445}, {75, 33}, 1, 0,
            UIMSG_BF, 1, InputAction::Yes, transition_button_label.c_str(), {ui_buttyes2});
        pDialogueWindow->CreateButton({pNPCPortraits_x[0][0], pNPCPortraits_y[0][0]}, {63, 73}, 1, 0,
            UIMSG_BF, 1, InputAction::EventTrigger, transition_button_label.c_str());
        pDialogueWindow->CreateButton({8, 8}, {460, 344}, 1, 0,
            UIMSG_BF, 1, InputAction::Yes, transition_button_label.c_str()
        );
    } else {
        v17 = HouseNPCData[_this + 1 - ((dword_591080 != 0) ? 1 : 0)];  //+ 1
        if (dialog_menu_id == DIALOGUE_OTHER) {
            pDialogueWindow->Release();
        } else {
            for (i = 0; i < uNumDialogueNPCPortraits; ++i)
                HouseNPCPortraitsButtonsList[i]->Release();
        }
        pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, {renDims.w, 345}, 0);
        pBtn_ExitCancel = pDialogueWindow->CreateButton({471, 445}, {169, 35}, 1, 0,
            UIMSG_Escape, 0, InputAction::Invalid, localization->GetString(LSTR_END_CONVERSATION), {ui_exit_cancel_button_background}
        );
        pDialogueWindow->CreateButton({8, 8}, {450, 320}, 1, 0, UIMSG_BuyInShop_Identify_Repair, 0);
        if (pDialogueNPCCount == 1 && dword_591080) {
            InitializaDialogueOptions(in_current_building_type);
        } else {
            if (v17->is_joinable) {
                num_menu_buttons = 1;
                pDialogueWindow->CreateButton({480, 160}, {140, 30}, 1, 0, UIMSG_ClickNPCTopic, DIALOGUE_13_hiring_related);
            }

            #define AddScriptedDialogueLine(EVENT_ID, MSG_PARAM) \
                if (EVENT_ID) { \
                    if (num_menu_buttons < 4) { \
                        int res = npcDialogueEventProcessor(EVENT_ID); \
                        if (res == 1 || res == 2) \
                            pDialogueWindow->CreateButton({480, 160 + 30 * num_menu_buttons++}, {140, 30}, 1, 0, UIMSG_ClickNPCTopic, MSG_PARAM); \
                    } \
                }

            AddScriptedDialogueLine(v17->dialogue_1_evt_id, DIALOGUE_SCRIPTED_LINE_1);
            AddScriptedDialogueLine(v17->dialogue_2_evt_id, DIALOGUE_SCRIPTED_LINE_2);
            AddScriptedDialogueLine(v17->dialogue_3_evt_id, DIALOGUE_SCRIPTED_LINE_3);
            AddScriptedDialogueLine(v17->dialogue_4_evt_id, DIALOGUE_SCRIPTED_LINE_4);
            AddScriptedDialogueLine(v17->dialogue_5_evt_id, DIALOGUE_SCRIPTED_LINE_5);
            AddScriptedDialogueLine(v17->dialogue_6_evt_id, DIALOGUE_SCRIPTED_LINE_6);

            pDialogueWindow->_41D08F_set_keyboard_control_group(num_menu_buttons, 1, 0, 2);
            dword_F8B1E0 = pDialogueWindow->pNumPresenceButton;
        }
        dialog_menu_id = DIALOGUE_MAIN;
    }
}

//----- (00445C8B) --------------------------------------------------------
int GetGreetType(signed int SpeakingNPC_ID) {
    if (SpeakingNPC_ID >= 0) {
        if (SpeakingNPC_ID < 5000) return 1;  // QuestNPC_greet
        return 2;                             // HiredNPC_greet
    }

    if (SpeakingNPC_ID >= 5000) return 2;

    int v3 = abs((int)sDialogue_SpeakingActorNPC_ID) - 1;

    FlatHirelings buf;
    buf.Prepare();

    return buf.IsFollower(v3) ? 1 : 2;
}

//----- (00445308) --------------------------------------------------------
const char *GetProfessionActionText(NPCProf prof) {
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
int UseNPCSkill(NPCProf profession, int id) {
    switch (profession) {
        case Healer: {
            for (Player &player : pParty->pPlayers) {
                player.health = player.GetMaxHealth();
            }
        } break;

        case ExpertHealer: {
            for (Player &player : pParty->pPlayers) {
                player.health = player.GetMaxHealth();

                for (Condition condition : standardConditionsExcludeDead) {
                    player.conditions.Reset(condition);
                }
            }
        } break;

        case MasterHealer: {
            for (Player &player : pParty->pPlayers) {
                player.health = player.GetMaxHealth();

                for (Condition condition : standardConditionsIncludeDead) {
                    // Master healer heals all except Eradicated and zombie
                    if (condition != Condition_Eradicated) {
                        player.conditions.Reset(condition);
                    }
                }
            }
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
            if (uCurrentlyLoadedLevelType == LEVEL_Indoor) {
                GameUI_SetStatusBar(LSTR_CANT_FLY_INDOORS);
                pAudioPlayer->playUISound(SOUND_fizzle);
            } else {
                int v19 = pOtherOverlayList->_4418B1(10008, 203, 0, 65536);
                // Spell power was changed to 0 because it does not have meaning for this buff
                pParty->pPartyBuffs[PARTY_BUFF_FLY]
                    .Apply(pParty->GetPlayingTime() + GameTime::FromHours(2), PLAYER_SKILL_MASTERY_MASTER, 0, v19, 0);
                // Mark buff as GM because NPC buff does not drain mana
                pParty->pPartyBuffs[PARTY_BUFF_FLY].isGMBuff = true;
                pAudioPlayer->playSpellSound(SPELL_AIR_FLY, 0, false);
            }
        } break;

        case WaterMaster: {
            int v20 = pOtherOverlayList->_4418B1(10005, 201, 0, 65536);
            pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK]
                .Apply(pParty->GetPlayingTime() + GameTime::FromHours(3), PLAYER_SKILL_MASTERY_MASTER, 0, v20, 0);
            // Mark buff as GM because NPC buff does not drain mana
            pParty->pPartyBuffs[PARTY_BUFF_WATER_WALK].isGMBuff = true;
            pAudioPlayer->playSpellSound(SPELL_WATER_WATER_WALK, 0, false);
        } break;

        case GateMaster: {
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
            pNextFrameMessageQueue->AddGUIMessage(UIMSG_OnCastTownPortal, PID(OBJECT_Player, pParty->pPlayers.size() + id), 0);
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
        if (!pParty->pHirelings[i].pName.empty())
            ids[count++] = i;

    for (size_t i = 0; i < pNPCStats->uNumNewNPCs; ++i) {
        NPCData *npc = &pNPCStats->pNewNPCData[i];
        if (npc->Hired()) {
            Assert(!npc->pName.empty()); // Important for the checks below.

            if (npc->pName != pParty->pHirelings[0].pName && npc->pName != pParty->pHirelings[1].pName) {
                Assert(i + 2 < 256); // Won't fit into uint8_t otherwise.
                ids[count++] = i + 2;
            }
        }
    }
}

bool FlatHirelings::IsFollower(size_t index) const {
    Assert(index < count);

    return ids[index] >= 2;
}

NPCData *FlatHirelings::Get(size_t index) const {
    Assert(index < count);

    uint8_t id = ids[index];

    if (id < 2)
        return &pParty->pHirelings[id];
    else
        return &pNPCStats->pNewNPCData[id - 2];
}
