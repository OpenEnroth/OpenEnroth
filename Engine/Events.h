#pragma once
#include <array>

/*  167 */
#pragma pack(push, 1)
struct EventIndex {
    int event_id;
    // event_step previously event_sequence_num
    int event_step;  // most event have several steps (commands), each of steps is stored separately
    unsigned int uEventOffsetInEVT;
};
#pragma pack(pop)

// raw event struct
// header
//  size_event 1 header+data
//  event id  2 3
// event_sequence_num
// event data
#pragma pack(push, 1)
struct _evt_raw {
    unsigned char _e_size;
    unsigned char v1;
    unsigned char v2;
    unsigned char v3;

    unsigned char _e_type;
    unsigned char v5;
    unsigned char v6;
    unsigned char v7;
    unsigned char v8;
    unsigned char v9;
    unsigned char v10;
    unsigned char v11;
    unsigned char v12;
    unsigned char v13;
    unsigned char v14;
    unsigned char v15;
    unsigned char v16;
    unsigned char v17;
    unsigned char v18;
    unsigned char v19;
    unsigned char v20;
    unsigned char v21;
    unsigned char v22;
    unsigned char v23;
    unsigned char v24;
    unsigned char v25;
    unsigned char v26;
    unsigned char v27;
    unsigned char v28;
    unsigned char v29;
    unsigned char v30;
    unsigned char v31;
    unsigned char v32;
    unsigned char tail[128];
};
#pragma pack(pop)

/*  310 */
enum EventType {
    EVENT_Exit = 0x1,
    EVENT_SpeakInHouse = 0x2,
    EVENT_PlaySound = 0x3,
    EVENT_MouseOver = 0x4,
    EVENT_LocationName = 0x5,
    EVENT_MoveToMap = 0x6,
    EVENT_OpenChest = 0x7,
    EVENT_ShowFace = 0x8,
    EVENT_ReceiveDamage = 0x9,
    EVENT_SetSnow = 0xA,
    EVENT_SetTexture = 0xB,
    EVENT_SetSprite = 0xD,
    EVENT_Compare = 0xE,
    EVENT_ChangeDoorState = 0xF,
    EVENT_Add = 0x10,
    EVENT_Substract = 0x11,
    EVENT_Set = 0x12,
    EVENT_SummonMonsters = 0x13,
    EVENT_CastSpell = 0x15,
    EVENT_SpeakNPC = 0x16,
    EVENT_SetFacesBit = 0x17,
    EVENT_ToggleActorFlag = 0x18,
    EVENT_RandomGoTo = 0x19,
    EVENT_InputString = 0x1A,
    EVENT_StatusText = 0x1D,
    EVENT_ShowMessage = 0x1E,
    EVENT_OnTimer = 0x1F,
    EVENT_ToggleIndoorLight = 0x20,
    EVENT_PressAnyKey = 0x21,
    EVENT_SummonItem = 0x22,
    EVENT_ForPartyMember = 0x23,
    EVENT_Jmp = 0x24,
    EVENT_OnMapReload = 0x25,
    EVENT_OnLongTimer = 0x26,
    EVENT_SetNPCTopic = 0x27,
    EVENT_MoveNPC = 0x28,
    EVENT_GiveItem = 0x29,
    EVENT_ChangeEvent = 0x2A,
    EVENT_CheckSkill = 0x2B,
    EVENT_OnCanShowDialogItemCmp = 44,
    EVENT_EndCanShowDialogItem = 45,
    EVENT_SetCanShowDialogItem = 46,
    EVENT_SetNPCGroupNews = 0x2F,
    EVENT_SetActorGroup = 0x30,
    EVENT_NPCSetItem = 0x31,
    EVENT_SetNPCGreeting = 0x32,
    EVENT_IsActorAlive = 0x33,
    EVENT_IsActorAssasinated = 52,
    EVENT_OnMapLeave = 0x35,
    EVENT_ChangeGroup = 0x36,
    EVENT_ChangeGroupAlly = 0x37,
    EVENT_CheckSeason = 0x38,
    EVENT_ToggleActorGroupFlag = 0x39,
    EVENT_ToggleChestFlag = 0x3A,
    EVENT_CharacterAnimation = 0x3B,
    EVENT_SetActorItem = 0x3C,
    EVENT_OnDateTimer = 0x3D,
    EVENT_EnableDateTimer = 0x3E,
    EVENT_StopAnimation = 0x3F,
    EVENT_CheckItemsCount = 0x40,
    EVENT_RemoveItems = 0x41,
    EVENT_SpecialJump = 0x42,
    EVENT_IsTotalBountyHuntingAwardInRange = 0x43,
    EVENT_IsNPCInParty = 0x44,
};

/*  311 */
enum VariableType {
    VAR_Sex = 0x1,
    VAR_Class = 0x2,
    VAR_CurrentHP = 0x3,
    VAR_MaxHP = 0x4,
    VAR_CurrentSP = 0x5,
    VAR_MaxSP = 0x6,
    VAR_ActualAC = 0x7,
    VAR_ACModifier = 0x8,
    VAR_BaseLevel = 0x9,
    VAR_LevelModifier = 0xA,
    VAR_Age = 0xB,
    VAR_Award = 0xC,
    VAR_Experience = 0xD,
    VAR_Race = 0xE,
    VAR_QBits_QuestsDone = 0x10,
    VAR_PlayerItemInHands = 0x11,
    VAR_Hour = 0x12,
    VAR_DayOfYear = 0x13,
    VAR_DayOfWeek = 0x14,
    VAR_FixedGold = 0x15,
    VAR_RandomGold = 0x16,
    VAR_FixedFood = 0x17,
    VAR_RandomFood = 0x18,
    VAR_MightBonus = 0x19,
    VAR_IntellectBonus = 0x1A,
    VAR_PersonalityBonus = 0x1B,
    VAR_EnduranceBonus = 0x1C,
    VAR_SpeedBonus = 0x1D,
    VAR_AccuracyBonus = 0x1E,
    VAR_LuckBonus = 0x1F,
    VAR_BaseMight = 0x20,
    VAR_BaseIntellect = 0x21,
    VAR_BasePersonality = 0x22,
    VAR_BaseEndurance = 0x23,
    VAR_BaseSpeed = 0x24,
    VAR_BaseAccuracy = 0x25,
    VAR_BaseLuck = 0x26,
    VAR_ActualMight = 0x27,
    VAR_ActualIntellect = 0x28,
    VAR_ActualPersonality = 0x29,
    VAR_ActualEndurance = 0x2A,
    VAR_ActualSpeed = 0x2B,
    VAR_ActualAccuracy = 0x2C,
    VAR_ActualLuck = 0x2D,
    VAR_FireResistance = 0x2E,
    VAR_AirResistance = 0x2F,
    VAR_WaterResistance = 0x30,
    VAR_EarthResistance = 0x31,
    VAR_SpiritResistance = 0x32,
    VAR_MindResistance = 0x33,
    VAR_BodyResistance = 0x34,
    VAR_LightResistance = 0x35,
    VAR_DarkResistance = 0x36,
    VAR_PhysicalResistance = 0x37,
    VAR_MagicResistance = 0x38,
    VAR_FireResistanceBonus = 0x39,
    VAR_AirResistanceBonus = 0x3A,
    VAR_WaterResistanceBonus = 0x3B,
    VAR_EarthResistanceBonus = 0x3C,
    VAR_SpiritResistanceBonus = 0x3D,
    VAR_MindResistanceBonus = 0x3E,
    VAR_BodyResistanceBonus = 0x3F,
    VAR_LightResistanceBonus = 0x40,
    VAR_DarkResistanceBonus = 0x41,
    VAR_PhysicalResistanceBonus = 0x42,
    VAR_MagicResistanceBonus = 0x43,
    VAR_StaffSkill = 0x44,
    VAR_SwordSkill = 0x45,
    VAR_DaggerSkill = 0x46,
    VAR_AxeSkill = 0x47,
    VAR_SpearSkill = 0x48,
    VAR_BowSkill = 0x49,
    VAR_MaceSkill = 0x4A,
    VAR_BlasterSkill = 0x4B,
    VAR_ShieldSkill = 0x4C,
    VAR_LeatherSkill = 0x4D,
    VAR_SkillChain = 0x4E,
    VAR_PlateSkill = 0x4F,
    VAR_FireSkill = 0x50,
    VAR_AirSkill = 0x51,
    VAR_WaterSkill = 0x52,
    VAR_EarthSkill = 0x53,
    VAR_SpiritSkill = 0x54,
    VAR_MindSkill = 0x55,
    VAR_BodySkill = 0x56,
    VAR_LightSkill = 0x57,
    VAR_DarkSkill = 0x58,
    VAR_IdentifyItemSkill = 0x59,
    VAR_MerchantSkill = 0x5A,
    VAR_RepairSkill = 0x5B,
    VAR_BodybuildingSkill = 0x5C,
    VAR_MeditationSkill = 0x5D,
    VAR_PerceptionSkill = 0x5E,
    VAR_DiplomacySkill = 0x5F,
    VAR_ThieverySkill = 0x60,
    VAR_DisarmTrapSkill = 0x61,
    VAR_DodgeSkill = 0x62,
    VAR_UnarmedSkill = 0x63,
    VAR_IdentifyMonsterSkill = 0x64,
    VAR_ArmsmasterSkill = 0x65,
    VAR_StealingSkill = 0x66,
    VAR_AlchemySkill = 0x67,
    VAR_LearningSkill = 0x68,
    VAR_Cursed = 0x69,
    VAR_Weak = 0x6A,
    VAR_Asleep = 0x6B,
    VAR_Afraid = 0x6C,
    VAR_Drunk = 0x6D,
    VAR_Insane = 0x6E,
    VAR_PoisonedGreen = 0x6F,
    VAR_DiseasedGreen = 0x70,
    VAR_PoisonedYellow = 0x71,
    VAR_DiseasedYellow = 0x72,
    VAR_PoisonedRed = 0x73,
    VAR_DiseasedRed = 0x74,
    VAR_Paralyzed = 0x75,
    VAR_Unconsious = 0x76,
    VAR_Dead = 0x77,
    VAR_Stoned = 0x78,
    VAR_Eradicated = 0x79,
    VAR_MajorCondition = 0x7A,
    VAR_MapPersistentVariable_0 = 123,
    VAR_MapPersistentVariable_74 = 0xC5,
    VAR_MapPersistentVariable_75 = 0xC6,
    VAR_MapPersistentVariable_99 = 0xDE,
    VAR_AutoNotes = 0xDF,
    VAR_IsMightMoreThanBase = 0xE0,
    VAR_IsIntellectMoreThanBase = 0xE1,
    VAR_IsPersonalityMoreThanBase = 0xE2,
    VAR_IsEnduranceMoreThanBase = 0xE3,
    VAR_IsSpeedMoreThanBase = 0xE4,
    VAR_IsAccuracyMoreThanBase = 0xE5,
    VAR_IsLuckMoreThanBase = 0xE6,
    VAR_PlayerBits = 0xE7,
    VAR_NPCs2 = 0xE8,
    VAR_IsFlying = 0xF0,
    VAR_HiredNPCHasSpeciality = 0xF1,
    VAR_CircusPrises = 0xF2,
    VAR_NumSkillPoints = 0xF3,
    VAR_MonthIs = 0xF4,
    VAR_Counter1 = 0xF5,
    VAR_Counter2 = 0xF6,
    VAR_Counter3 = 0xF7,
    VAR_Counter4 = 0xF8,
    VAR_Counter5 = 0xF9,
    VAR_Counter6 = 0xFa,
    VAR_Counter7 = 0xFB,
    VAR_Counter8 = 0xFC,
    VAR_Counter9 = 0xFD,
    VAR_Counter10 = 0xFE,
    VAR_UnknownTimeEvent0 = 0xFF,
    VAR_UnknownTimeEvent19 = 0x112,
    VAR_ReputationInCurrentLocation = 0x113,
    VAR_History_0 = 0x114,
    VAR_History_28 = 0x130,
    VAR_Unknown1 = 0x131,
    VAR_GoldInBank = 0x132,
    VAR_NumDeaths = 0x133,
    VAR_NumBounties = 0x134,
    VAR_PrisonTerms = 0x135,
    VAR_ArenaWinsPage = 0x136,
    VAR_ArenaWinsSquire = 0x137,
    VAR_ArenaWinsKnight = 0x138,
    VAR_ArenaWinsLord = 0x139,
    VAR_Invisible = 0x13A,
    VAR_ItemEquipped = 0x13B,
};

#define ENUM_TO_STRING_CASE(val) case val: return #val;
inline const char *ToString(VariableType variable) {
    switch (variable) {
        ENUM_TO_STRING_CASE(VAR_Sex)
        ENUM_TO_STRING_CASE(VAR_Class)
        ENUM_TO_STRING_CASE(VAR_CurrentHP)
        ENUM_TO_STRING_CASE(VAR_MaxHP)
        ENUM_TO_STRING_CASE(VAR_CurrentSP)
        ENUM_TO_STRING_CASE(VAR_MaxSP)
        ENUM_TO_STRING_CASE(VAR_ActualAC)
        ENUM_TO_STRING_CASE(VAR_ACModifier)
        ENUM_TO_STRING_CASE(VAR_BaseLevel)
        ENUM_TO_STRING_CASE(VAR_LevelModifier)
        ENUM_TO_STRING_CASE(VAR_Age)
        ENUM_TO_STRING_CASE(VAR_Award)
        ENUM_TO_STRING_CASE(VAR_Experience)
        ENUM_TO_STRING_CASE(VAR_Race)
        ENUM_TO_STRING_CASE(VAR_QBits_QuestsDone)
        ENUM_TO_STRING_CASE(VAR_PlayerItemInHands)
        ENUM_TO_STRING_CASE(VAR_Hour)
        ENUM_TO_STRING_CASE(VAR_DayOfYear)
        ENUM_TO_STRING_CASE(VAR_DayOfWeek)
        ENUM_TO_STRING_CASE(VAR_FixedGold)
        ENUM_TO_STRING_CASE(VAR_RandomGold)
        ENUM_TO_STRING_CASE(VAR_FixedFood)
        ENUM_TO_STRING_CASE(VAR_RandomFood)
        ENUM_TO_STRING_CASE(VAR_MightBonus)
        ENUM_TO_STRING_CASE(VAR_IntellectBonus)
        ENUM_TO_STRING_CASE(VAR_PersonalityBonus)
        ENUM_TO_STRING_CASE(VAR_EnduranceBonus)
        ENUM_TO_STRING_CASE(VAR_SpeedBonus)
        ENUM_TO_STRING_CASE(VAR_AccuracyBonus)
        ENUM_TO_STRING_CASE(VAR_LuckBonus)
        ENUM_TO_STRING_CASE(VAR_BaseMight)
        ENUM_TO_STRING_CASE(VAR_BaseIntellect)
        ENUM_TO_STRING_CASE(VAR_BasePersonality)
        ENUM_TO_STRING_CASE(VAR_BaseEndurance)
        ENUM_TO_STRING_CASE(VAR_BaseSpeed)
        ENUM_TO_STRING_CASE(VAR_BaseAccuracy)
        ENUM_TO_STRING_CASE(VAR_BaseLuck)
        ENUM_TO_STRING_CASE(VAR_ActualMight)
        ENUM_TO_STRING_CASE(VAR_ActualIntellect)
        ENUM_TO_STRING_CASE(VAR_ActualPersonality)
        ENUM_TO_STRING_CASE(VAR_ActualEndurance)
        ENUM_TO_STRING_CASE(VAR_ActualSpeed)
        ENUM_TO_STRING_CASE(VAR_ActualAccuracy)
        ENUM_TO_STRING_CASE(VAR_ActualLuck)
        ENUM_TO_STRING_CASE(VAR_FireResistance)
        ENUM_TO_STRING_CASE(VAR_AirResistance)
        ENUM_TO_STRING_CASE(VAR_WaterResistance)
        ENUM_TO_STRING_CASE(VAR_EarthResistance)
        ENUM_TO_STRING_CASE(VAR_SpiritResistance)
        ENUM_TO_STRING_CASE(VAR_MindResistance)
        ENUM_TO_STRING_CASE(VAR_BodyResistance)
        ENUM_TO_STRING_CASE(VAR_LightResistance)
        ENUM_TO_STRING_CASE(VAR_DarkResistance)
        ENUM_TO_STRING_CASE(VAR_PhysicalResistance)
        ENUM_TO_STRING_CASE(VAR_MagicResistance)
        ENUM_TO_STRING_CASE(VAR_FireResistanceBonus)
        ENUM_TO_STRING_CASE(VAR_AirResistanceBonus)
        ENUM_TO_STRING_CASE(VAR_WaterResistanceBonus)
        ENUM_TO_STRING_CASE(VAR_EarthResistanceBonus)
        ENUM_TO_STRING_CASE(VAR_SpiritResistanceBonus)
        ENUM_TO_STRING_CASE(VAR_MindResistanceBonus)
        ENUM_TO_STRING_CASE(VAR_BodyResistanceBonus)
        ENUM_TO_STRING_CASE(VAR_LightResistanceBonus)
        ENUM_TO_STRING_CASE(VAR_DarkResistanceBonus)
        ENUM_TO_STRING_CASE(VAR_PhysicalResistanceBonus)
        ENUM_TO_STRING_CASE(VAR_MagicResistanceBonus)
        ENUM_TO_STRING_CASE(VAR_StaffSkill)
        ENUM_TO_STRING_CASE(VAR_SwordSkill)
        ENUM_TO_STRING_CASE(VAR_DaggerSkill)
        ENUM_TO_STRING_CASE(VAR_AxeSkill)
        ENUM_TO_STRING_CASE(VAR_SpearSkill)
        ENUM_TO_STRING_CASE(VAR_BowSkill)
        ENUM_TO_STRING_CASE(VAR_MaceSkill)
        ENUM_TO_STRING_CASE(VAR_BlasterSkill)
        ENUM_TO_STRING_CASE(VAR_ShieldSkill)
        ENUM_TO_STRING_CASE(VAR_LeatherSkill)
        ENUM_TO_STRING_CASE(VAR_SkillChain)
        ENUM_TO_STRING_CASE(VAR_PlateSkill)
        ENUM_TO_STRING_CASE(VAR_FireSkill)
        ENUM_TO_STRING_CASE(VAR_AirSkill)
        ENUM_TO_STRING_CASE(VAR_WaterSkill)
        ENUM_TO_STRING_CASE(VAR_EarthSkill)
        ENUM_TO_STRING_CASE(VAR_SpiritSkill)
        ENUM_TO_STRING_CASE(VAR_MindSkill)
        ENUM_TO_STRING_CASE(VAR_BodySkill)
        ENUM_TO_STRING_CASE(VAR_LightSkill)
        ENUM_TO_STRING_CASE(VAR_DarkSkill)
        ENUM_TO_STRING_CASE(VAR_IdentifyItemSkill)
        ENUM_TO_STRING_CASE(VAR_MerchantSkill)
        ENUM_TO_STRING_CASE(VAR_RepairSkill)
        ENUM_TO_STRING_CASE(VAR_BodybuildingSkill)
        ENUM_TO_STRING_CASE(VAR_MeditationSkill)
        ENUM_TO_STRING_CASE(VAR_PerceptionSkill)
        ENUM_TO_STRING_CASE(VAR_DiplomacySkill)
        ENUM_TO_STRING_CASE(VAR_ThieverySkill)
        ENUM_TO_STRING_CASE(VAR_DisarmTrapSkill)
        ENUM_TO_STRING_CASE(VAR_DodgeSkill)
        ENUM_TO_STRING_CASE(VAR_UnarmedSkill)
        ENUM_TO_STRING_CASE(VAR_IdentifyMonsterSkill)
        ENUM_TO_STRING_CASE(VAR_ArmsmasterSkill)
        ENUM_TO_STRING_CASE(VAR_StealingSkill)
        ENUM_TO_STRING_CASE(VAR_AlchemySkill)
        ENUM_TO_STRING_CASE(VAR_LearningSkill)
        ENUM_TO_STRING_CASE(VAR_Cursed)
        ENUM_TO_STRING_CASE(VAR_Weak)
        ENUM_TO_STRING_CASE(VAR_Asleep)
        ENUM_TO_STRING_CASE(VAR_Afraid)
        ENUM_TO_STRING_CASE(VAR_Drunk)
        ENUM_TO_STRING_CASE(VAR_Insane)
        ENUM_TO_STRING_CASE(VAR_PoisonedGreen)
        ENUM_TO_STRING_CASE(VAR_DiseasedGreen)
        ENUM_TO_STRING_CASE(VAR_PoisonedYellow)
        ENUM_TO_STRING_CASE(VAR_DiseasedYellow)
        ENUM_TO_STRING_CASE(VAR_PoisonedRed)
        ENUM_TO_STRING_CASE(VAR_DiseasedRed)
        ENUM_TO_STRING_CASE(VAR_Paralyzed)
        ENUM_TO_STRING_CASE(VAR_Unconsious)
        ENUM_TO_STRING_CASE(VAR_Dead)
        ENUM_TO_STRING_CASE(VAR_Stoned)
        ENUM_TO_STRING_CASE(VAR_Eradicated)
        ENUM_TO_STRING_CASE(VAR_MajorCondition)
        ENUM_TO_STRING_CASE(VAR_MapPersistentVariable_0)
        ENUM_TO_STRING_CASE(VAR_MapPersistentVariable_74)
        ENUM_TO_STRING_CASE(VAR_MapPersistentVariable_75)
        ENUM_TO_STRING_CASE(VAR_MapPersistentVariable_99)
        ENUM_TO_STRING_CASE(VAR_AutoNotes)
        ENUM_TO_STRING_CASE(VAR_IsMightMoreThanBase)
        ENUM_TO_STRING_CASE(VAR_IsIntellectMoreThanBase)
        ENUM_TO_STRING_CASE(VAR_IsPersonalityMoreThanBase)
        ENUM_TO_STRING_CASE(VAR_IsEnduranceMoreThanBase)
        ENUM_TO_STRING_CASE(VAR_IsSpeedMoreThanBase)
        ENUM_TO_STRING_CASE(VAR_IsAccuracyMoreThanBase)
        ENUM_TO_STRING_CASE(VAR_IsLuckMoreThanBase)
        ENUM_TO_STRING_CASE(VAR_PlayerBits)
        ENUM_TO_STRING_CASE(VAR_NPCs2)
        ENUM_TO_STRING_CASE(VAR_IsFlying)
        ENUM_TO_STRING_CASE(VAR_HiredNPCHasSpeciality)
        ENUM_TO_STRING_CASE(VAR_CircusPrises)
        ENUM_TO_STRING_CASE(VAR_NumSkillPoints)
        ENUM_TO_STRING_CASE(VAR_MonthIs)
        ENUM_TO_STRING_CASE(VAR_Counter1)
        ENUM_TO_STRING_CASE(VAR_Counter2)
        ENUM_TO_STRING_CASE(VAR_Counter3)
        ENUM_TO_STRING_CASE(VAR_Counter4)
        ENUM_TO_STRING_CASE(VAR_Counter5)
        ENUM_TO_STRING_CASE(VAR_Counter6)
        ENUM_TO_STRING_CASE(VAR_Counter7)
        ENUM_TO_STRING_CASE(VAR_Counter8)
        ENUM_TO_STRING_CASE(VAR_Counter9)
        ENUM_TO_STRING_CASE(VAR_Counter10)
        ENUM_TO_STRING_CASE(VAR_UnknownTimeEvent0)
        ENUM_TO_STRING_CASE(VAR_UnknownTimeEvent19)
        ENUM_TO_STRING_CASE(VAR_ReputationInCurrentLocation)
        ENUM_TO_STRING_CASE(VAR_History_0)
        ENUM_TO_STRING_CASE(VAR_History_28)
        ENUM_TO_STRING_CASE(VAR_Unknown1)
        ENUM_TO_STRING_CASE(VAR_GoldInBank)
        ENUM_TO_STRING_CASE(VAR_NumDeaths)
        ENUM_TO_STRING_CASE(VAR_NumBounties)
        ENUM_TO_STRING_CASE(VAR_PrisonTerms)
        ENUM_TO_STRING_CASE(VAR_ArenaWinsPage)
        ENUM_TO_STRING_CASE(VAR_ArenaWinsSquire)
        ENUM_TO_STRING_CASE(VAR_ArenaWinsKnight)
        ENUM_TO_STRING_CASE(VAR_ArenaWinsLord)
        ENUM_TO_STRING_CASE(VAR_Invisible)
        ENUM_TO_STRING_CASE(VAR_ItemEquipped)
    }

    return nullptr;
}

#define EVT_WORD(x) *(unsigned __int16 *)&x
#define EVT_DWORD(x) *(unsigned int *)&x
#define EVT_STRING(x) (unsigned char *)&x
#define EVT_BYTE(x) (unsigned char)x

extern std::array<EventIndex, 4400> pSomeOtherEVT_Events;
extern signed int uSomeOtherEVT_NumEvents;
extern char *pSomeOtherEVT;
extern std::array<EventIndex, 4400> pSomeEVT_Events;
extern signed int uSomeEVT_NumEvents;
extern char *pSomeEVT;

extern unsigned int uGlobalEVT_NumEvents;
extern unsigned int uGlobalEVT_Size;
extern std::array<char, 46080> pGlobalEVT;
extern std::array<EventIndex, 4400> pGlobalEVT_Index;

extern std::array<unsigned int, 500> pLevelStrOffsets;
extern unsigned int uLevelStrNumStrings;
extern unsigned int uLevelStrFileSize;
extern signed int uLevelEVT_NumEvents;
extern unsigned int uLevelEVT_Size;
extern std::array<char, 9216> pLevelStr;
extern std::array<char, 9216> pLevelEVT;
extern std::array<EventIndex, 4400> pLevelEVT_Index;

unsigned int LoadEventsToBuffer(const char *pContainerName, char *pBuffer,
                                unsigned int uBufferSize);
void Initialize_GlobalEVT();
void LoadLevel_InitializeLevelEvt();
void EventProcessor(int uEventID, int a2, int a3, int entry_line = 0);
char *GetEventHintString(unsigned int uEventID);  // idb

/*  312 */
#pragma pack(push, 1)
struct ByteArray {
    char f[65535];
};
#pragma pack(pop)

void init_event_triggers();
void check_event_triggers();
bool sub_4465DF_check_season(int a1);
void sub_448CF4_spawn_monsters(int16_t typeindex, int16_t level, int count,
                               int x, int y, int z, int group,
                               unsigned int uUniqueName);
