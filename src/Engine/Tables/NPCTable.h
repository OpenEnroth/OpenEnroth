#pragma once

#include <string>
#include <array>

#include "Engine/Objects/NPCEnums.h"

#include "Utility/IndexedArray.h"

struct NPCTopic {
    const char *pTopic;
    const char *pText;
};

struct NPCData {  // 4Ch
    inline bool Hired() { return (uFlags & 0x80) != 0; }

    std::string pName;               // 0
    unsigned int uPortraitID = 0;  // 4
    unsigned int uFlags = 0;  // 8    // & 0x80    no greeting on dialogue start;
                          // looks like hired
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
    inline NPCProfession()
        : uHirePrice(0),
          pBenefits(nullptr),
          pActionText(nullptr),
          pJoinText(nullptr),
          pDismissText(nullptr) {}

    unsigned int uHirePrice;
    char *pBenefits;
    char *pActionText;
    char *pJoinText;
    char *pDismissText;
};

struct NPCProfessionChance {
    unsigned int uTotalprofChance{};     // summ
    char professionChancePerArea[60]{};  // prof position
};

struct NPCGreeting {
    union {
        struct {
            char *pGreeting1;  // at first meet
            char *pGreeting2;  // at latest meets
        };
        char *pGreetings[2];
    };
};

struct NPCStats {
    inline NPCStats() {
        uNumNPCNames[0] = uNumNPCNames[1] = 0;
    }

    void InitializeNPCText();
    void InitializeNPCData();
    void Initialize();
    void Release();
    void InitializeAdditionalNPCs(NPCData *pNPCDataBuff, int npc_uid,
                                  int uLocation2D, int uMapId);
    /**
     * @offset 0x476C60
     */
    void setNPCNamesOnLoad();
    char *sub_495366_MispronounceName(uint8_t firstLetter,
                                      uint8_t genderId);

    std::array<NPCData, 501> pNPCData;     // 0 - 94BCh count from 1
    std::array<NPCData, 501> pNewNPCData;  // 94BCh- 12978h count from 1
    char *pNPCNames[540][2];
    IndexedArray<NPCProfession, NPC_PROFESSION_FIRST, NPC_PROFESSION_LAST> pProfessions = {{}};  // count from 1
    std::array<NPCData, 100> pAdditionalNPC;
    char *pCatchPhrases[52]{};   // 15CA4h
    char *pNPCUnicNames[500]{};  // from first batch
    NPCProfessionChance pProfessionChance[77];  // 16544h profession chance in each area
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
    std::string pNPCDataTXT_Raw;
    std::string pNPCNamesTXT_Raw;
    std::string pNPCProfTXT_Raw;
    std::string pNPCNewsTXT_Raw;
    std::string pNPCTopicTXT_Raw;
    std::string pNPCTextTXT_Raw;
    std::string pNPCDistTXT_Raw;
    std::string pNPCGreetTXT_Raw;
    std::string pNCPGroupTXT_Raw;

    static int dword_AE336C_LastMispronouncedNameFirstLetter;
    static int dword_AE3370_LastMispronouncedNameResult;
};

extern std::array<NPCTopic, 789> pNPCTopics;
extern struct NPCStats *pNPCStats;
