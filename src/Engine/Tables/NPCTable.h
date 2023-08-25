#pragma once

#include <stdint.h>
#include <string>
#include <array>

#include "Engine/Objects/NPCEnums.h"
#include "Engine/MapEnums.h"
#include "Utility/IndexedArray.h"
#include "Utility/Flags.h"

class Blob;
class GameResourceManager;
enum class MAP_TYPE : uint32_t;

// TODO(Nik-RE-dev): It seems that two greet flags are used purely because it's modification is performed
//                   before greeting string is constructed. It is also ensures that NPC in multi-NPC houses
//                   always greet you with first line until you leave the house.
//                   Ideally there should be only one flag.
enum class NpcFlag : uint32_t {
    NPC_GREETED_FIRST = 0x01, // NPC has been greeted first time
    NPC_GREETED_SECOND = 0x02, // NPC has been greeted second time
    NPC_HIRED = 0x80 // NPC is hired
};
using enum NpcFlag;
MM_DECLARE_FLAGS(NpcFlags, NpcFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(NpcFlags)

struct NPCTopic {
    std::string pTopic;
    std::string pText;
};

struct NPCData {  // 4Ch
    inline bool Hired() { return uFlags & NPC_HIRED; }

    std::string pName;               // 0
    unsigned int uPortraitID = 0;  // 4
    NpcFlags uFlags = 0;  // 8
    int fame = 0;                  // c
    int rep = 0;                   // 10
    unsigned int Location2D = 0;   // 14  house_id
    NPCProf profession = NoProfession;  // 18
    int greet = 0;                 // 1c
    int is_joinable = 0;           // 20
    int field_24 = 0;
    unsigned int dialogue_1_evt_id = 0;  // dialogue options that are defined by script
    unsigned int dialogue_2_evt_id = 0;  // = 0  == unused
    unsigned int dialogue_3_evt_id = 0;  // can also be idx in pNPCTopics
    unsigned int dialogue_4_evt_id = 0;  // and absolutely crazy stuff when it's in party hierlings (npc2)
    unsigned int dialogue_5_evt_id = 0;
    unsigned int dialogue_6_evt_id = 0;
    unsigned int uSex = 0;       // 40
    int bHasUsedTheAbility = 0;  // 44
    int news_topic = 0;          // 48
};

struct NPCProfession {
    unsigned int uHirePrice{};
    std::string pBenefits{};
    std::string pActionText{};
    std::string pJoinText{};
    std::string pDismissText{};
};

struct NPCProfessionChance {
    unsigned int uTotalprofChance{};     // summ
    char professionChancePerArea[60]{};  // prof position
};

struct NPCGreeting {
    std::string pGreeting1;  // at first meet
    std::string pGreeting2;  // at latest meets
};

struct NPCStats {
    inline NPCStats() {
        uNumNPCNames[0] = uNumNPCNames[1] = 0;
    }

    void Initialize(GameResourceManager *resourceManager);
    void InitializeNPCNames(const Blob &npcNames);
    void InitializeNPCProfs(const Blob &npcProfs);
    void InitializeNPCText(const Blob &npcText);
    void InitializeNPCTopics(const Blob &npcTopics);
    void InitializeNPCDist(const Blob &npcDist);
    void InitializeNPCData(const Blob &npcData);
    void InitializeNPCGreets(const Blob &npcGreets);
    void InitializeNPCGroups(const Blob &npcGroups);
    void InitializeNPCNews(const Blob &npcNews);
    void InitializeAdditionalNPCs(NPCData *pNPCDataBuff, int npc_uid,
                                  int uLocation2D, MapId uMapId);
    /**
     * @offset 0x476C60
     */
    void setNPCNamesOnLoad();
    const std::string &sub_495366_MispronounceName(uint8_t firstLetter, uint8_t genderId);

    std::array<NPCData, 501> pNPCData;     // 0 - 94BCh count from 1
    std::array<NPCData, 501> pNewNPCData;  // 94BCh- 12978h count from 1
    std::array<std::array<std::string, 2>, 540> pNPCNames{};
    IndexedArray<NPCProfession, NPC_PROFESSION_FIRST, NPC_PROFESSION_LAST> pProfessions = {{}};  // count from 1
    std::array<NPCData, 100> pAdditionalNPC = {{}};
    std::array<std::string, 52> pCatchPhrases{};   // 15CA4h
    std::array<std::string, 500> pNPCUnicNames{};  // from first batch
    NPCProfessionChance pProfessionChance[77];  // 16544h profession chance in each area // TODO(captainurist): IndexedArray
    int field_17884 = 0;
    int field_17888 = 0;
    NPCGreeting pNPCGreetings[206];
    std::array<uint16_t, 51> pGroups = {{}};
    std::array<uint16_t, 51> pGroups_copy = {{}};
    unsigned int uNewlNPCBufPos{};
    unsigned int uNumNewNPCs{};
    int field_17FC8 = 0;
    unsigned int uNumNPCProfessions{};
    unsigned int uNumNPCNames[2]{};  // 0 male 1 female

    static int dword_AE336C_LastMispronouncedNameFirstLetter;
    static int dword_AE3370_LastMispronouncedNameResult;
};

extern std::array<NPCTopic, 789> pNPCTopics;
extern struct NPCStats *pNPCStats;
