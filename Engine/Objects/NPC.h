#pragma once

#include <array>
#include <cstdint>

#include "Engine/Objects/NPCProf.h"


struct NPCTopic {
    const char *pTopic;
    const char *pText;
};

extern std::array<NPCTopic, 789> pNPCTopics;

/*  136 */
#pragma pack(push, 1)
struct NPCData {  // 4Ch
    inline bool Hired() { return (uFlags & 0x80) != 0; }

    char *pName;               // 0
    unsigned int uPortraitID;  // 4
    unsigned int uFlags;  // 8    // & 0x80    no greeting on dialogue start;
                          // looks like hired
    int fame;                  // c
    int rep;                   // 10
    unsigned int Location2D;   // 14  house_id
    NPCProf profession;  // 18
    int greet;                 // 1c
    int is_joinable;           // 20
    int field_24;
    unsigned int dialogue_1_evt_id;  // dialogue options that are defined by script
    unsigned int dialogue_2_evt_id;  // = 0  == unused
    unsigned int dialogue_3_evt_id;  // can also be idx in pNPCTopics
    unsigned int dialogue_4_evt_id;  // and absolutely crazy stuff when it's in party hierlings (npc2)
    unsigned int dialogue_5_evt_id;
    unsigned int dialogue_6_evt_id;
    unsigned int uSex;       // 40
    int bHasUsedTheAbility;  // 44
    int news_topic;          // 48
};
#pragma pack(pop)

/*  138 */
#pragma pack(push, 1)
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
#pragma pack(pop)

/*  139 */
#pragma pack(push, 1)
struct NPCProfessionChance {
    unsigned int uTotalprofChance;     // summ
    char professionChancePerArea[60];  // prof position
};
#pragma pack(pop)

/*  140 */
#pragma pack(push, 1)
struct NPCGreeting {
    union {
        struct {
            char *pGreeting1;  // at first meet
            char *pGreeting2;  // at latest meets
        };
        char *pGreetings[2];
    };
};
#pragma pack(pop)

/*  137 */
#pragma pack(push, 1)
struct NPCStats {
    inline NPCStats()
        : pNPCTopicTXT_Raw(nullptr),
          pNPCTextTXT_Raw(nullptr),
          pNPCDistTXT_Raw(nullptr) {
        uNumNPCNames[0] = uNumNPCNames[1] = 0;
    }

    void InitializeNPCText();
    void InitializeNPCData();
    void Initialize();
    void Release();
    void InitializeAdditionalNPCs(NPCData *pNPCDataBuff, int npc_uid,
                                  int uLocation2D, int uMapId);
    void OnLoadSetNPC_Names();
    char *sub_495366_MispronounceName(uint8_t firstLetter,
                                      uint8_t genderId);

    NPCData pNPCData[501]{};     // 0 - 94BCh count from 1
    NPCData pNewNPCData[501]{};  // 94BCh- 12978h count from 1
    char *pNPCNames[540][2];
    NPCProfession pProfessions[59]{};  // count from 1
    NPCData pAdditionalNPC[100]{};
    char *pCatchPhrases[52];   // 15CA4h
    char *pNPCUnicNames[500];  // from first batch
    NPCProfessionChance
        pProfessionChance[77];  // 16544h profession chance in each area
    int field_17884 = 0;
    int field_17888 = 0;
    NPCGreeting pNPCGreetings[206];
    uint16_t pGroups[51];
    uint16_t pGroups_copy[51];
    unsigned int uNewlNPCBufPos;
    unsigned int uNumNewNPCs;
    int field_17FC8 = 0;
    unsigned int uNumNPCProfessions;
    unsigned int uNumNPCNames[2];  // 0 male 1 female
    char *pNPCDataTXT_Raw;
    char *pNPCNamesTXT_Raw;
    char *pNPCProfTXT_Raw;
    char *pNPCNewsTXT_Raw;
    char *pNPCTopicTXT_Raw;
    char *pNPCTextTXT_Raw;
    char *pNPCDistTXT_Raw;
    char *pNPCGreetTXT_Raw;
    char *pNCPGroupTXT_Raw;

    static int dword_AE336C_LastMispronouncedNameFirstLetter;
    static int dword_AE3370_LastMispronouncedNameResult;
};
#pragma pack(pop)

extern int pDialogueNPCCount;
extern std::array<class Image *, 6> pDialogueNPCPortraits;
extern int uNumDialogueNPCPortraits;
extern struct NPCStats *pNPCStats;

bool PartyHasDragon();
bool CheckHiredNPCSpeciality(NPCProf prof);

int UseNPCSkill(NPCProf profession);
void NPCHireableDialogPrepare();
void _4B4224_UpdateNPCTopics(int _this);
const char *GetProfessionActionText(NPCProf prof);
struct NPCData *GetNPCData(signed int npcid);
struct NPCData *GetNewNPCData(signed int npcid, int *npc_indx);
int GetGreetType(signed int SpeakingNPC_ID);
int NPCDialogueEventProcessor(int npc_event_id, int entry_line = 0);
