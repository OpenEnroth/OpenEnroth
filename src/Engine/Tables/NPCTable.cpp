#include "NPCTable.h"

#include <array>
#include <cassert>
#include <string>
#include <vector>

#include "Engine/GameType.h"
#include "Engine/MapEnumFunctions.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Objects/MonsterEnumFunctions.h"
#include "Engine/Objects/NPCEnumFunctions.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Random/Random.h"

#include "Library/Logger/Logger.h"
#include "Library/Serialization/Serialization.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

std::array<NPCTopic, 1048> pNPCTopics;
NPCStats *pNPCStats = nullptr;

int NPCStats::dword_AE336C_LastMispronouncedNameFirstLetter = -1;
int NPCStats::dword_AE3370_LastMispronouncedNameResult = -1;

//----- (00476977) --------------------------------------------------------
void NPCStats::InitializeNPCText(const Blob &npcText) {
    // npctext.txt table structure: index | text (localized) | dev notes | npc name (localized, not used).
    for (std::string_view line : split(npcText.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t');
        int i = fromString<int>(tokens[0]) - 1; // File indices are 1-based, array is 0-based.
        // `pNPCTopics` is sized to fit MM8's npctext.txt full index range (up to 1047), not just its 942 non-empty rows.
        if (i < 0 || static_cast<size_t>(i) >= pNPCTopics.size()) {
            MM_WARNING("NPCTable: dropping NPC text #{}, doesn't fit in a {}-slot table", i + 1, pNPCTopics.size());
            continue;
        }
        pNPCTopics[i].pText = unquote(tokens[1]);
    }
}

void NPCStats::InitializeNPCTopics(const Blob &npcTopics) {
    // npctopic.txt table structure: index | topic (localized) | ??? (not used) | dev notes | text index (not used) |
    //                               npc name (not localized, not used) | npc index (not used).
    for (std::string_view line : split(npcTopics.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t');
        int i = fromString<int>(tokens[0]);
        // `pNPCTopics` is sized to fit npctext.txt's full index range (1048). npctopic.txt only needs 750.
        if (i < 0 || static_cast<size_t>(i) >= pNPCTopics.size()) {
            MM_WARNING("NPCTable: dropping NPC topic #{}, doesn't fit in a {}-slot table", i, pNPCTopics.size());
            continue;
        }
        pNPCTopics[i].pTopic = unquote(tokens[1]);
    }
}

void NPCStats::InitializeNPCDist(const Blob &npcDist) {
    // npcdist.txt table structure: profession (localized, not used) | area profession chance values...
    for (auto [line, prof] : split(npcDist.str()).by("\r\n").drop(2).skip("").zip(allNpcProfessions()))
        for (auto [token, map] : split(line).by('\t').drop(1).zip(allMaps()))
            pProfessionChance[map].chanceByProfession[prof] = fromString<int>(token);

    for (MapId map : allMaps())
        for (NpcProfession prof : allNpcProfessions())
            pProfessionChance[map].total += pProfessionChance[map].chanceByProfession[prof];
}

//----- (00476CB5) --------------------------------------------------------
void NPCStats::InitializeNPCData(const Blob &npcData) {
    // npcdata.txt table structure: index | name (localized) | portrait id | groups (not used) | house | profession |
    //                              greeting index | can join (y/n) | event ids 1-6 | dev notes |
    //                              map id (optional, not used).
    for (std::string_view line : split(npcData.str()).by("\r\n").drop(2).skip("")) {
        std::array<std::string_view, 16> tokens = split(line).by('\t');
        int i = fromString<int>(tokens[0]); // File indices are 1-based.
        if (i <= 0 || static_cast<size_t>(i) >= pOriginalNPCData.size()) {
            // `pOriginalNPCData` is sized to fit all of MM8's real (non-filler) NPCs, up to index 527.
            // Past that are MM8's filler rows (528-1000 in npcdata.txt), all literally named "Boob" with every
            // other field zeroed - not real content, so dropping them is correct and shouldn't warn.
            // Still warn if a row past 527 ever isn't the placeholder, so genuinely new content doesn't get
            // silently lost by a future MM8 data revision.
            if (i > 0 && unquote(tokens[1]) != "Boob")
                MM_WARNING("NPCTable: dropping NPC #{}, doesn't fit in a {}-slot table", i, pOriginalNPCData.size());
            continue;
        }
        pNPCUnicNames[i] = unquote(tokens[1]);
        pOriginalNPCData[i].name = pNPCUnicNames[i];
        pOriginalNPCData[i].portraitId = fromString<int>(tokens[2]);
        pOriginalNPCData[i].house = static_cast<HouseId>(fromString<int>(tokens[6]));
        pOriginalNPCData[i].profession = static_cast<NpcProfession>(fromString<int>(tokens[7]));
        pOriginalNPCData[i].greetingIndex = fromString<int>(tokens[8]);
        pOriginalNPCData[i].canJoin = tokens[9][0] == 'y' ? 1 : 0;
        pOriginalNPCData[i].dialogue_1_evt_id = fromString<int>(tokens[10]);
        pOriginalNPCData[i].dialogue_2_evt_id = fromString<int>(tokens[11]);
        pOriginalNPCData[i].dialogue_3_evt_id = fromString<int>(tokens[12]);
        pOriginalNPCData[i].dialogue_4_evt_id = fromString<int>(tokens[13]);
        pOriginalNPCData[i].dialogue_5_evt_id = fromString<int>(tokens[14]);
        pOriginalNPCData[i].dialogue_6_evt_id = fromString<int>(tokens[15]);
    }
    uNumNewNPCs = static_cast<int>(pOriginalNPCData.size()); // == 528, kept in sync with the array size above.
}

void NPCStats::InitializeNPCGreets(const Blob &npcGreets) {
    // npcgreet.txt table structure: index | greeting 1 (localized) | greeting 2 (localized) |
    //                               notes (not localized, not used) | owner (not localized, not used).
    for (std::string_view line : split(npcGreets.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 3> tokens = split(line).by('\t');
        if (tokens[0].empty())
            continue; // Trailing orphan row with no index column.

        int i = fromString<int>(tokens[0]); // File indices are 1-based.
        pNPCGreetings[i].pGreeting1 = unquote(tokens[1]);
        pNPCGreetings[i].pGreeting2 = unquote(tokens[2]);
    }
}

void NPCStats::InitializeNPCGroups(const Blob &npcGroups) {
    // npcgroup.txt table structure: group index | news index | dev notes.
    for (std::string_view line : split(npcGroups.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t');
        int i = fromString<int>(tokens[0]); // File indices are 0-based.
        pOriginalGroups[i] = fromString<int>(tokens[1]);
    }
}

void NPCStats::InitializeNPCNews(const Blob &npcNews) {
    // npcnews.txt table structure: index | text (localized) | dev notes.
    for (std::string_view line : split(npcNews.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t');
        int i = fromString<int>(tokens[0]); // File indices are 0-based.
        pCatchPhrases[i] = unquote(tokens[1]);
    }
}

//----- (0047702F) --------------------------------------------------------
void NPCStats::Initialize(ResourceManager *resourceManager) {
    pOriginalNPCData.fill(NPCData());
    InitializeNPCData(resourceManager->eventsText("npcdata.txt"));
    InitializeNPCGreets(resourceManager->eventsText("npcgreet.txt"));
    InitializeNPCGroups(resourceManager->eventsText("npcgroup.txt"));
    InitializeNPCNews(resourceManager->eventsText("npcnews.txt"));
    InitializeNPCText(resourceManager->eventsText("npctext.txt"));
    InitializeNPCTopics(resourceManager->eventsText("npctopic.txt"));
    InitializeNPCDist(resourceManager->eventsText("npcdist.txt"));
    // MM8's `EnglishT.lod` doesn't ship `npcnames.txt` - the equivalent data was renamed/restructured
    // into `pcnames.txt` (per-race/class name columns instead of MM7's flat male|female pair), see
    // `InitializeNPCNamesMM8`.
    if (gameType() == GAME_TYPE_MM8)
        InitializeNPCNamesMM8(resourceManager->eventsText("pcnames.txt"));
    else
        InitializeNPCNames(resourceManager->eventsText("npcnames.txt"));
    InitializeNPCProfs(resourceManager->eventsText("npcprof.txt"));
}

void NPCStats::InitializeNPCNames(const Blob &npcNames) {
    // npcnames.txt table structure: male name (localized) | female name (localized).
    // Female column runs out before the male column.
    uNewlNPCBufPos = 0;
    pNPCNames.fill({});
    for (std::string_view line : split(npcNames.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t');
        if (!tokens[0].empty())
            pNPCNames[SEX_MALE].emplace_back(unquote(tokens[0]));
        if (!tokens[1].empty())
            pNPCNames[SEX_FEMALE].emplace_back(unquote(tokens[1]));
    }
}

void NPCStats::InitializeNPCNamesMM8(const Blob &pcNames) {
    // pcnames.txt table structure: 8 name-pool columns,
    // one per race/class combo - Knight/Cleric Male | Knight/Cleric Female | Dark Elf Peasant Male |
    // Dark Elf Peasant Female | Troll/Minotaur | Vampire/Necro Female | Vampire/Necro Male | Dragon - followed by
    // a 9th "To make excel work right" column that's just a constant "*end" marker on every row (spreadsheet
    // artifact, not data). Troll/Minotaur and Dragon have no separate Male/Female columns, so their names are
    // added to both sexes' pools. Each of the 8 columns is independently variable-length: once a column's list
    // of names runs out, the row where it ends carries a literal "*end" in that column, and only rows after
    // that go empty. Other columns keep going regardless. The file ends with a sentinel row whose first column
    // is "*end".
    static constexpr std::array<bool, 8> kColumnIsMale = {true, false, true, false, true, false, true, true};
    static constexpr std::array<bool, 8> kColumnIsFemale = {false, true, false, true, true, true, false, true};

    pNPCNames.fill({});
    for (std::string_view line : split(pcNames.str()).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 9> tokens = split(line).by('\t');
        if (tokens[0] == "*end")
            break; // Sentinel end-of-table row.

        for (size_t i = 0; i < kColumnIsMale.size(); i++) {
            if (tokens[i].empty() || tokens[i] == "*end")
                continue; // Empty cell, or this column's own per-column end-of-list marker.
            if (kColumnIsMale[i])
                pNPCNames[SEX_MALE].emplace_back(unquote(tokens[i]));
            if (kColumnIsFemale[i])
                pNPCNames[SEX_FEMALE].emplace_back(unquote(tokens[i]));
        }
    }
}

void NPCStats::InitializeNPCProfs(const Blob &npcProfs) {
    // npcprof.txt table structure: profession id | profession name (localized, not used) | hire price |
    //                              action text (localized) | benefit description (localized) |
    //                              join text (localized) | dismiss text (localized).
    for (std::string_view line : split(npcProfs.str()).by("\r\n").drop(4)) {
        std::array<std::string_view, 7> tokens = split(line).by('\t');
        if (tokens[0].empty())
            continue; // Trailing pure-tab orphan rows past the last entry.

        NpcProfession prof = static_cast<NpcProfession>(fromString<int>(tokens[0]));
        pProfessions[prof].uHirePrice = fromString<int>(tokens[2]);
        pProfessions[prof].pActionText = unquote(tokens[3]);
        // TODO(captainurist): MM7 says Hunter increases the identify monster skill "by four", but vanilla MM7 and OE
        //                     give +6. Fix the text as a data patch.
        pProfessions[prof].pBenefits = unquote(tokens[4]);
        pProfessions[prof].pJoinText = unquote(tokens[5]);
        pProfessions[prof].pDismissText = unquote(tokens[6]);
    }
    uNumNPCProfessions = 59;
}

namespace {

// MM7's portrait ranges in `InitializeAdditionalNPCs` don't exist in MM8 (its portrait ids start at 800).
// MM8 resolves the portrait from the monster id, with the same id ranges as `isMm8PeasantMonsterId`.
//
// MM8's `npcdata.txt` (`EnglishT.lod`) reserves rows 516-527 as
// "generic species" portrait templates (name literally "<Species> Peasant"/
// "<Species> Guard", every other column zero) - these are the ids used below. No
// "Vampire Peasant" or "Human Peasant" template row exists for id ranges 49-51/166-168. Those two
// use a stand-in portrait.
int mm8PeasantPortraitIdForMonsterId(MonsterId monsterId) {
    int id = std::to_underlying(monsterId);
    if (id >= 1 && id <= 3) return 800;      // Lizardman Peasant/Farmer/Villager -> "Lizardman Peasant" template.
    if (id >= 19 && id <= 24) return 933;    // Dark Elf Peasant Male/Female A/B/C -> "Dark Elf Peasant" template.
    if (id >= 28 && id <= 30) return 1009;   // Ogre Peasant -> "Ogre Peasant" template.
    if (id >= 49 && id <= 51) return 1181;   // Peasant (Vampire Female Peasant) -> no template row; stand-in is
                                              // a real vampire portrait ("Grand Master Vampire" trainer).
    if (id >= 61 && id <= 63) return 977;    // Troll Peasant -> "Troll Peasant" template.
    if (id >= 67 && id <= 69) return 1044;   // Minotaur Peasant -> "Minotaur Peasant" template.
    if (id >= 166 && id <= 168) return 1013; // Human Peasant -> no template row; stand-in is a plain real
                                              // human portrait.
    return 0; // Unknown/not-yet-catalogued MM8 peasant species - caller falls back to a real default.
}

} // namespace

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

    if (gameType() == GAME_TYPE_MM8) {
        uGeneratedPortret = mm8PeasantPortraitIdForMonsterId(npc_uid);
        if (uGeneratedPortret == 0)
            uGeneratedPortret = 1013; // Real, human-passing default - never the placeholder glyph.
    } else {
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
                    break;
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
                default:
                    // MM8 races: `raceForMonsterType` only ever returns MM7 races, so this is unreachable for now.
                    assert(false);
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
    }

    pNPCDataBuff->portraitId = uGeneratedPortret;
    pNPCDataBuff->flags = 0;
    pNPCDataBuff->fame = 0;
    // generate reputation
    rep_gen = grng->random(100) + 1;

    if (rep_gen >= 60) {
        if (rep_gen >= 90) {
            if (rep_gen >= 95) {
                if (rep_gen >= 98) {
                    pNPCDataBuff->rep = -600;
                } else {
                    pNPCDataBuff->rep = 400;
                }
            } else {
                pNPCDataBuff->rep = -300;
            }
        } else {
            pNPCDataBuff->rep = 200;
        }
    } else {
        pNPCDataBuff->rep = 0;
    }

    pNPCDataBuff->profession = NPC_PROFESSION_LAST;
    // MM8's `EnglishT.lod` doesn't ship `npcdist.txt`, so `pProfessionChance[uMapId].total`
    // stays 0 for MM8 maps, and `grng->random()` asserts on a non-positive bound.
    if (pProfessionChance[uMapId].total > 0) {
        max_prof_cap = grng->random(pProfessionChance[uMapId].total);
        test_prof_summ = 0;
        for (NpcProfession i : allNpcProfessions()) {
            test_prof_summ += pProfessionChance[uMapId].chanceByProfession[i];
            if (test_prof_summ > max_prof_cap) {
                pNPCDataBuff->profession = i;
                break;
            }
        }
    }
    pNPCDataBuff->house = uLocation2D;
    pNPCDataBuff->field_24 = 1;
    // MM8 crowd peasants are not hireable. Their dialogue shows only the catch-phrase, no "More Information"/"Hire"
    // panel. MM7's random hireling NPCs are recruitable.
    pNPCDataBuff->canJoin = gameType() != GAME_TYPE_MM8;
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

        if (!matches.empty()) {
            pickedName = vrng->randomSample(matches);
        } else {
            pickedName = vrng->random(names.size()); // No name with this letter - pick any.
        }
    }
    dword_AE3370_LastMispronouncedNameResult = pickedName;
    return this->pNPCNames[gender][pickedName];
}
