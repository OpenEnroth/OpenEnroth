#pragma once

#include <array>

enum NPCProf {
    Smith = 1,       // GM Weapon Repair;
    Armorer = 2,     // GM Armor Repair;
    Alchemist = 3,   // GM Potion Repair;
    Scholar = 4,     // GM Item ID;               Learning: +5
    Guide = 5,       // Travel by foot: -1 day;
    Tracker = 6,     // Travel by foot: -2 days;
    Pathfinder = 7,  // Travel by foot: -3 days;
    Sailor = 8,      // Travel by sea: -2 days;
    Navigator = 9,   // Travel by sea: -3 days;
    Healer = 10,
    ExpertHealer = 11,
    MasterHealer = 12,
    Teacher = 13,        // Learning: +10;
    Instructor = 14,     // Learning: +15;
    Armsmaster = 15,     // Armsmaster: +2;
    Weaponsmaster = 16,  // Armsmaster: +3;
    Apprentice = 17,    // Fire: +2;         Air: +2;    Water: +2;   Earth: +2;
    Mystic = 18,        // Fire: +3;         Air: +3;    Water: +3;   Earth: +3;
    Spellmaster = 19,   // Fire: +4;         Air: +4;    Water: +4;   Earth: +4;
    Trader = 20,        // Merchant: +4;
    Merchant = 21,      // Merchant: +6;
    Scout = 22,         // Perception: +6;
    Herbalist = 23,     // Alchemy: +4;
    Apothecary = 24,    // Alchemy: +8;
    Tinker = 25,        // Traps: +4;
    Locksmith = 26,     // Traps: +6;
    Fool = 27,          // Luck: +5;
    ChimneySweep = 28,  // Luck: +20;
    Porter = 29,        // Food for rest: -1;
    QuarterMaster = 30,  // Food for rest: -2;
    Factor = 31,         // Gold finds: +10%;
    Banker = 32,         // Gold finds: +20%;
    Cook = 33,
    Chef = 34,
    Horseman = 35,  // Travel by foot: -2 days;
    Bard = 36,
    Enchanter = 37,     // Resist All: +20;
    Cartographer = 38,  // Wizard Eye level 2;
    WindMaster = 39,
    WaterMaster = 40,
    GateMaster = 41,
    Acolyte = 42,
    Piper = 43,
    Explorer = 44,  // Travel by foot -1 day;     Travel by sea: -1 day;
    Pirate = 45,    // Travel by sea: -2 days;    Gold finds: +10%; Reputation:
                  // +5;
    Squire = 46,
    Psychic = 47,  // Perception: +5;            Luck: +10;
    Gypsy = 48,    // Food for rest: -1;         Merchant: +3; Reputation: +5;
    Diplomat = 49,
    Duper = 50,    // Merchant: +8;              Reputation: +5;
    Burglar = 51,  // Traps: +8;                 Stealing: +8; Reputation: +5;
    FallenWizard = 52,  // Reputation: +5;
    Acolyte2 =
        53,  // Spirit: +2;                Mind: +2;              Body: +2;
    Initiate =
        54,  // Spirit: +3;                Mind: +3;              Body: +3;
    Prelate =
        55,      // Spirit: +4;                Mind: +4;              Body: +4;
    Monk = 56,   // Unarmed: +2;               Dodge: +2;
    Sage = 57,   // Monster ID: +6
    Hunter = 58  // Monster ID: +6
};

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
    unsigned int Location2D;   // 14
    unsigned int uProfession;  // 18
    int greet;                 // 1c
    int joins;                 // 20
    int field_24;
    unsigned int evt_A;      // 28
    unsigned int evt_B;      // 2c evtb
    unsigned int evt_C;      //  30 evtc
    unsigned int evt_D;      // 34
    unsigned int evt_E;      // 38
    unsigned int evt_F;      // 3c
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
bool CheckHiredNPCSpeciality(unsigned int uProfession);

int UseNPCSkill(NPCProf profession);
const char *ContractSelectText(int pEventCode);
void NPCHireableDialogPrepare();
void _4B4224_UpdateNPCTopics(int _this);
const char *GetProfessionActionText(int a1);
struct NPCData *GetNPCData(signed int npcid);
struct NPCData *GetNewNPCData(signed int npcid, int *npc_indx);
int GetGreetType(signed int SpeakingNPC_ID);
int NPC_EventProcessor(int npc_event_id, int entry_line = 0);
