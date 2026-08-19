#include "NPCTable.h"

#include <array>
#include <string>
#include <vector>

#include "Engine/MapEnumFunctions.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Objects/MonsterEnumFunctions.h"
#include "Engine/Objects/NPCEnumFunctions.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Random/Random.h"

#include "Library/Tsv/TsvReader.h"
#include "Library/Serialization/Serialization.h"

#include "Utility/Memory/Blob.h"

std::array<NPCTopic, 789> pNPCTopics;
NPCStats *pNPCStats = nullptr;

int NPCStats::dword_AE336C_LastMispronouncedNameFirstLetter = -1;
int NPCStats::dword_AE3370_LastMispronouncedNameResult = -1;

//----- (00476977) --------------------------------------------------------
void NPCStats::InitializeNPCText(const Blob &npcText) {
    // npctext.txt table structure: index | text (localized) | dev notes | npc name (localized, not used).
    for (TsvLine line : TsvReader(npcText).drop(1).skip(&TsvLine::isBlank)) {
        int i = line[0].as<int>() - 1; // File indices are 1-based, array is 0-based.
        pNPCTopics[i].pText = line[1];
    }
}

void NPCStats::InitializeNPCTopics(const Blob &npcTopics) {
    // npctopic.txt table structure: index | topic (localized) | ??? (not used) | dev notes | text index (not used) |
    //                               npc name (not localized, not used) | npc index (not used).
    for (TsvLine line : TsvReader(npcTopics).drop(1).skip(&TsvLine::isBlank)) {
        int i = line[0].as<int>();
        pNPCTopics[i].pTopic = line[1];
    }
}

void NPCStats::InitializeNPCDist(const Blob &npcDist) {
    // npcdist.txt table structure: profession (localized, not used) | area profession chance values...
    for (auto [line, prof] : TsvReader(npcDist).drop(2).skip(&TsvLine::isBlank).zip(allNpcProfessions()))
        for (auto [token, map] : line.drop(1).zip(allMaps()))
            pProfessionChance[map].chanceByProfession[prof] = token.as<int>();

    for (MapId map : allMaps())
        for (NpcProfession prof : allNpcProfessions())
            pProfessionChance[map].total += pProfessionChance[map].chanceByProfession[prof];
}

//----- (00476CB5) --------------------------------------------------------
void NPCStats::InitializeNPCData(const Blob &npcData) {
    // npcdata.txt table structure: index | name (localized) | portrait id | groups (not used) | house | profession |
    //                              greeting index | can join (y/n) | event ids 1-6 | dev notes |
    //                              map id (optional, not used).
    for (TsvLine line : TsvReader(npcData).drop(2).skip(&TsvLine::isBlank).take(500)) {
        int i = line[0].as<int>(); // File indices are 1-based.
        pNPCUnicNames[i] = line[1];
        pOriginalNPCData[i].name = pNPCUnicNames[i];
        pOriginalNPCData[i].portraitId = line[2].as<int>();
        pOriginalNPCData[i].house = static_cast<HouseId>(line[6].as<int>());
        pOriginalNPCData[i].profession = static_cast<NpcProfession>(line[7].as<int>());
        pOriginalNPCData[i].greetingIndex = line[8].as<int>();
        pOriginalNPCData[i].canJoin = line[9].starts_with('y') ? 1 : 0;
        pOriginalNPCData[i].dialogue_1_evt_id = line[10].as<int>();
        pOriginalNPCData[i].dialogue_2_evt_id = line[11].as<int>();
        pOriginalNPCData[i].dialogue_3_evt_id = line[12].as<int>();
        pOriginalNPCData[i].dialogue_4_evt_id = line[13].as<int>();
        pOriginalNPCData[i].dialogue_5_evt_id = line[14].as<int>();
        pOriginalNPCData[i].dialogue_6_evt_id = line[15].as<int>();
    }
    uNumNewNPCs = 501;
}

void NPCStats::InitializeNPCGreets(const Blob &npcGreets) {
    // npcgreet.txt table structure: index | greeting 1 (localized) | greeting 2 (localized) |
    //                               notes (not localized, not used) | owner (not localized, not used).
    for (TsvLine line : TsvReader(npcGreets).drop(1).skip(&TsvLine::isBlank)) {
        if (line[0].empty())
            continue; // Trailing orphan row with no index column.

        int i = line[0].as<int>(); // File indices are 1-based.
        pNPCGreetings[i].pGreeting1 = line[1];
        pNPCGreetings[i].pGreeting2 = line[2];
    }
}

void NPCStats::InitializeNPCGroups(const Blob &npcGroups) {
    // npcgroup.txt table structure: group index | news index | dev notes.
    for (TsvLine line : TsvReader(npcGroups).drop(1).skip(&TsvLine::isBlank)) {
        int i = line[0].as<int>(); // File indices are 0-based.
        pOriginalGroups[i] = line[1].as<int>();
    }
}

void NPCStats::InitializeNPCNews(const Blob &npcNews) {
    // npcnews.txt table structure: index | text (localized) | dev notes.
    for (TsvLine line : TsvReader(npcNews).drop(1).skip(&TsvLine::isBlank)) {
        int i = line[0].as<int>(); // File indices are 0-based.
        pCatchPhrases[i] = line[1];
    }
}

//----- (0047702F) --------------------------------------------------------
void NPCStats::Initialize(ResourceManager *resourceManager) {
    pOriginalNPCData.fill(NPCData());
    InitializeNPCData(resourceManager->eventsData("npcdata.txt"));
    InitializeNPCGreets(resourceManager->eventsData("npcgreet.txt"));
    InitializeNPCGroups(resourceManager->eventsData("npcgroup.txt"));
    InitializeNPCNews(resourceManager->eventsData("npcnews.txt"));
    InitializeNPCText(resourceManager->eventsData("npctext.txt"));
    InitializeNPCTopics(resourceManager->eventsData("npctopic.txt"));
    InitializeNPCDist(resourceManager->eventsData("npcdist.txt"));
    InitializeNPCNames(resourceManager->eventsData("npcnames.txt"));
    InitializeNPCProfs(resourceManager->eventsData("npcprof.txt"));
}

void NPCStats::InitializeNPCNames(const Blob &npcNames) {
    // npcnames.txt table structure: male name (localized) | female name (localized).
    // Female column runs out before the male column.
    uNewlNPCBufPos = 0;
    pNPCNames.fill({});
    for (TsvLine line : TsvReader(npcNames).drop(1).skip(&TsvLine::isBlank)) {
        if (!line[0].empty())
            pNPCNames[SEX_MALE].emplace_back(line[0]);
        if (!line[1].empty())
            pNPCNames[SEX_FEMALE].emplace_back(line[1]);
    }
}

void NPCStats::InitializeNPCProfs(const Blob &npcProfs) {
    // npcprof.txt table structure: profession id | profession name (localized, not used) | hire price |
    //                              action text (localized) | benefit description (localized) |
    //                              join text (localized) | dismiss text (localized).
    for (TsvLine line : TsvReader(npcProfs).drop(4).skip(&TsvLine::isBlank)) {
        if (line[0].empty())
            continue; // Trailing pure-tab orphan rows past the last entry.

        NpcProfession prof = static_cast<NpcProfession>(line[0].as<int>());
        pProfessions[prof].uHirePrice = line[2].as<int>();
        pProfessions[prof].pActionText = line[3];
        pProfessions[prof].pBenefits = line[4];
        pProfessions[prof].pJoinText = line[5];
        pProfessions[prof].pDismissText = line[6];
    }
    uNumNPCProfessions = 59;
}

//----- (0047732C) --------------------------------------------------------
void NPCStats::InitializeAdditionalNPCs(NPCData *pNPCDataBuff, MonsterId npc_uid,
                                        HouseId uLocation2D, MapId uMapId) {
    int rep_gen;
    int uGeneratedPortret;    // ecx@23
    int test_prof_summ;       // ecx@37
    int max_prof_cap;         // edx@37
                              // signed int result; // eax@39
    Race uRace;                // [sp+Ch] [bp-Ch]@1
    bool break_gen;           // [sp+10h] [bp-8h]@1
    signed int gen_attempts;  // [sp+14h] [bp-4h]@1
    int uPortretMin;          // [sp+24h] [bp+Ch]@1
    int uPortretMax;

    MonsterType monsterType = monsterTypeForMonsterId(npc_uid);
    Sex uNPCSex = sexForMonsterType(monsterType);
    uRace = raceForMonsterType(monsterType);
    pNPCDataBuff->sex = uNPCSex;
    pNPCDataBuff->name = grng->randomSample(pNPCNames[uNPCSex]);

    gen_attempts = 0;
    break_gen = false;

    do {
        switch (uRace) {
            case RACE_HUMAN:
                if (uNPCSex == SEX_MALE) {
                    uPortretMin = 2;
                    uPortretMax = 100;
                } else {
                    uPortretMin = 201;
                    uPortretMax = 250;
                }
            case RACE_ELF:
                if (uNPCSex == SEX_MALE) {
                    uPortretMin = 400;
                    uPortretMax = 430;
                } else {
                    uPortretMin = 460;
                    uPortretMax = 490;
                }
                break;
            case RACE_GOBLIN:
                if (uNPCSex == SEX_MALE) {
                    uPortretMin = 500;
                    uPortretMax = 520;
                } else {
                    uPortretMin = 530;
                    uPortretMax = 550;
                }
                break;
            case RACE_DWARF:
                if (uNPCSex == SEX_MALE) {
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

    pNPCDataBuff->portraitId = uGeneratedPortret;
    pNPCDataBuff->flags = 0;
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

    max_prof_cap = grng->random(pProfessionChance[uMapId].total);
    test_prof_summ = 0;
    pNPCDataBuff->profession = NPC_PROFESSION_LAST;
    for (NpcProfession i : allNpcProfessions()) {
        test_prof_summ += pProfessionChance[uMapId].chanceByProfession[i];
        if (test_prof_summ > max_prof_cap) {
            pNPCDataBuff->profession = i;
            break;
        }
    }
    pNPCDataBuff->house = uLocation2D;
    pNPCDataBuff->field_24 = 1;
    pNPCDataBuff->canJoin = 1;
    pNPCDataBuff->dialogue_1_evt_id = 0;
    pNPCDataBuff->dialogue_2_evt_id = 0;
    pNPCDataBuff->dialogue_3_evt_id = 0;
    pNPCDataBuff->dialogue_4_evt_id = 0;
    pNPCDataBuff->dialogue_5_evt_id = 0;
    pNPCDataBuff->dialogue_6_evt_id = 0;
}

//----- (00495366) --------------------------------------------------------
const std::string &NPCStats::sub_495366_MispronounceName(char firstLetter, Sex gender) {
    int pickedName;

    // TODO(captainurist): Caching in kinda wrong? Revisit when working on #mm6.
    //                     See "O Ho! %13! Er, %13. I think. Whatever..."

    if (firstLetter == dword_AE336C_LastMispronouncedNameFirstLetter) {
        pickedName = dword_AE3370_LastMispronouncedNameResult;
    } else {
        dword_AE336C_LastMispronouncedNameFirstLetter = firstLetter;
        const std::vector<std::string> &names = this->pNPCNames[gender];

        std::vector<int> matches;
        for (int i = 0; i < static_cast<int>(names.size()); ++i)
            if (tolower(names[i][0]) == tolower(firstLetter))
                matches.push_back(i);

        if (!matches.empty())
            pickedName = vrng->randomSample(matches);
        else
            pickedName = vrng->random(names.size()); // No name with this letter - pick any.
    }
    dword_AE3370_LastMispronouncedNameResult = pickedName;
    return this->pNPCNames[gender][pickedName];
}
