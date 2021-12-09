#pragma once

#include <array>
#include <cstdint>

#include "Engine/Objects/Items.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Objects/Player.h"
#include "Engine/Time.h"

#define PARTY_AUTONOTES_BIT__EMERALD_FIRE_FOUNTAIN 2

// reference "quests.txt" and pQuestTable
enum PARTY_QUEST_BITS : uint16_t {
    QBIT_EMERALD_ISLAND_RED_POTION_ACTIVE = 1,
    QBIT_EMERALD_ISLAND_SEASHELL_ACTIVE = 2,
    QBIT_EMERALD_ISLAND_LONGBOW_ACTIVE = 3,
    QBIT_EMERALD_ISLAND_PLATE_ACTIVE = 4,
    QBIT_EMERALD_ISLAND_LUTE_ACTIVE = 5,
    QBIT_EMERALD_ISLAND_HAT_ACTIVE = 6,

    QBIT_EMERALD_ISLAND_MARGARETH_OFF = 17,

    QBIT_EVENMORN_MAP_FOUND = 64,

    QBIT_HARMONDALE_REBUILT = 98,
    QBIT_LIGHT_PATH = 99,
    QBIT_DARK_PATH = 100,

    QBIT_ESCAPED_EMERALD_ISLE = 136,

    QBIT_OBELISK_IN_HARMONDALE_FOUND = 164,
    QBIT_OBELISK_IN_ERATHIA_FOUND = 165,
    QBIT_OBELISK_IN_TULAREAN_FOREST_FOUND = 166,
    QBIT_OBELISK_IN_DEYJA_FOUND = 167,
    QBIT_OBELISK_IN_BRACADA_DESERT_FOUND = 168,
    QBIT_OBELISK_IN_CELESTE_FOUND = 169,
    QBIT_OBELISK_IN_THE_PIT_FOUND = 170,
    QBIT_OBELISK_IN_EVENMORN_ISLAND_FOUND = 171,
    QBIT_OBELISK_IN_MOUNT_NIGHON_FOUND = 172,
    QBIT_OBELISK_IN_BARROW_DOWNS_FOUND = 173,
    QBIT_OBELISK_IN_LAND_OF_THE_GIANTS_FOUND = 174,
    QBIT_OBELISK_IN_TATALIA_FOUND = 175,
    QBIT_OBELISK_IN_AVLEE_FOUND = 176,
    QBIT_OBELISK_IN_STONE_CITY_FOUND = 177,

    QBIT_OBELISK_TREASURE_FOUND = 178,

    QBIT_SPLITTER_FOUND = 184,

    QBIT_FOUNTAIN_IN_HARMONDALE_ACTIVATED = 206,
    QBIT_FOUNTAIN_IN_MOUNT_NIGHON_ACTIVATED = 207,
    QBIT_FOUNTAIN_IN_PIERPONT_ACTIVATED = 208,
    QBIT_FOUNTAIN_IN_CELESTIA_ACTIVATED = 209,
    QBIT_FOUNTAIN_IN_THE_PIT_ACTIVATED = 210,
    QBIT_FOUNTAIN_IN_EVENMORN_ISLE_ACTIVATED = 211,

    QBIT_ARCOMAGE_CHAMPION = 238,  // won all arcomage games
    QBIT_DIVINE_INTERVENTION_RETRIEVED = 239,
};

/*  355 */
enum PARTY_FLAGS_1 : int32_t {
    PARTY_FLAGS_1_ForceRedraw = 0x0002,
    PARTY_FLAGS_1_WATER_DAMAGE = 0x0004,
    PARTY_FLAGS_1_AIRBORNE = 0x0008,
    PARTY_FLAGS_1_ALERT_RED = 0x0010,
    PARTY_FLAGS_1_ALERT_YELLOW = 0x0020,
    PARTY_FLAGS_1_ALERT_RED_OR_YELLOW = 0x0030,
    PARTY_FLAGS_1_STANDING_ON_WATER = 0x0080,
    PARTY_FLAGS_1_LANDING = 0x0100,
    PARTY_FLAGS_1_BURNING = 0x0200
};
enum PARTY_FLAGS_2 : int32_t {
    PARTY_FLAGS_2_RUNNING = 0x2,
};

/*  347 */
enum PARTY_BUFF_INDEX {
    PARTY_BUFF_RESIST_AIR = 0,
    PARTY_BUFF_RESIST_BODY = 1,
    PARTY_BUFF_DAY_OF_GODS = 2,
    PARTY_BUFF_DETECT_LIFE = 3,
    PARTY_BUFF_RESIST_EARTH = 4,
    PARTY_BUFF_FEATHER_FALL = 5,
    PARTY_BUFF_RESIST_FIRE = 6,
    PARTY_BUFF_FLY = 7,
    PARTY_BUFF_HASTE = 8,
    PARTY_BUFF_HEROISM = 9,
    PARTY_BUFF_IMMOLATION = 10,
    PARTY_BUFF_INVISIBILITY = 11,
    PARTY_BUFF_RESIST_MIND = 12,
    PARTY_BUFF_PROTECTION_FROM_MAGIC = 13,
    PARTY_BUFF_SHIELD = 14,
    PARTY_BUFF_STONE_SKIN = 15,
    PARTY_BUFF_TORCHLIGHT = 16,
    PARTY_BUFF_RESIST_WATER = 17,
    PARTY_BUFF_WATER_WALK = 18,
    PARTY_BUFF_WIZARD_EYE = 19,
};

/*  300 */
enum PartyAction : uint32_t {
    PARTY_INVALID = 0,
    PARTY_TurnLeft = 1,
    PARTY_TurnRight = 2,
    PARTY_StrafeLeft = 3,
    PARTY_StrafeRight = 4,
    PARTY_WalkForward = 5,
    PARTY_WalkBackward = 6,
    PARTY_LookUp = 7,
    PARTY_LookDown = 8,
    PARTY_CenterView = 9,
    PARTY_unkA = 10,
    PARTY_unkB = 11,
    PARTY_Jump = 12,
    PARTY_FlyUp = 13,
    PARTY_FlyDown = 14,
    PARTY_Land = 15,
    PARTY_RunForward = 16,
    PARTY_RunBackward = 17,
    PARTY_FastTurnLeft = 18,
    PARTY_FastTurnRight = 19,

    PARTY_dword = 0xFFFFFFFF
};

/*  135 */
#pragma pack(push, 1)
struct ActionQueue {
    inline ActionQueue() : uNumActions(0) {}

    void Add(PartyAction action);
    void Reset();
    PartyAction Next();

    unsigned int uNumActions;
    PartyAction pActions[30]{};
};
#pragma pack(pop)

enum class PartyAlignment : uint32_t {
    PartyAlignment_Good = 0,
    PartyAlignment_Neutral = 1,
    PartyAlignment_Evil = 2
};

/*  208 */
#pragma pack(push, 1)
struct PartyTimeStruct {
    std::array<GameTime, 10> bountyHunting_next_generation_time;
    std::array<GameTime, 85> Shops_next_generation_time;  // field_50
    std::array<GameTime, 53> _shop_ban_times;
    std::array<GameTime, 10> CounterEventValues;  // (0xACD314h in Silvo's binary)
    std::array<GameTime, 29> HistoryEventTimes;  // (0xACD364h in Silvo's binary)
    std::array<GameTime, 20> _s_times;  // 5d8 440h+8*51     //(0xACD44Ch in Silvo's binary)
};
#pragma pack(pop)

struct Party {
    Party() : playing_time(), last_regenerated() {
        Zero();
    }

    void Zero();
    void UpdatePlayersAndHirelingsEmotions();
    void RestAndHeal();
    unsigned int GetPartyFame();
    void CreateDefaultParty(bool bDebugGiveItems = false);
    void Reset();
    void ResetPosMiscAndSpellBuffs();
    bool HasItem(unsigned int uItemID);
    void SetHoldingItem(ItemGen *pItem);
    int GetFirstCanAct();  // added to fix some nzi access problems
    int GetNextActiveCharacter();
    bool _497FC5_check_party_perception_against_level();
    bool AddItemToParty(ItemGen *pItem);
    void Yell();
    void CountHirelings();

    void GivePartyExp(unsigned int pEXPNum);
    int GetPartyReputation();

    void PartyFindsGold(
        unsigned int uNumGold,
        int _1_dont_share_with_followers___2_the_same_but_without_a_message__else_normal);
    void PickedItem_PlaceInInventory_or_Drop();

    int GetGold() const;
    static void SetGold(int amount);
    static void AddGold(int amount);
    static void TakeGold(int amount);

    int GetFood() const;
    static void SetFood(int amount);
    static void TakeFood(int amount);
    static void GiveFood(int amount);

    static void Sleep8Hours();


    inline bool WizardEyeActive() const {
        return pPartyBuffs[PARTY_BUFF_WIZARD_EYE].expire_time.value > 0;
    }
    inline int WizardEyeSkillLevel() const {
        return pPartyBuffs[PARTY_BUFF_WIZARD_EYE].uSkill;
    }
    inline bool TorchlightActive() const {
        return pPartyBuffs[PARTY_BUFF_TORCHLIGHT].expire_time.value > 0;
    }
    inline bool FlyActive() const {
        return pPartyBuffs[PARTY_BUFF_FLY].expire_time.value > 0;
    }
    inline bool WaterWalkActive() const {
        return pPartyBuffs[PARTY_BUFF_WATER_WALK].expire_time.value > 0;
    }
    inline bool ImmolationActive() const {
        return pPartyBuffs[PARTY_BUFF_IMMOLATION].expire_time.value > 0;
    }
    inline int ImmolationSkillLevel() const {
        return pPartyBuffs[PARTY_BUFF_IMMOLATION].uSkill;
    }
    inline bool FeatherFallActive() const {
        return pPartyBuffs[PARTY_BUFF_FEATHER_FALL].expire_time.value > 0;
    }
    inline bool Invisible() const {
        return pPartyBuffs[PARTY_BUFF_INVISIBILITY].expire_time.value > 0;
    }

    inline bool GetRedAlert() const {
        return (uFlags & PARTY_FLAGS_1_ALERT_RED) != 0;
    }
    inline void SetRedAlert() { uFlags |= PARTY_FLAGS_1_ALERT_RED; }
    inline bool GetYellowAlert() const {
        return (uFlags & PARTY_FLAGS_1_ALERT_YELLOW) != 0;
    }
    inline void SetYellowAlert() { uFlags |= PARTY_FLAGS_1_ALERT_YELLOW; }

    inline bool GetRedOrYellowAlert() {
        return (uFlags & PARTY_FLAGS_1_ALERT_RED_OR_YELLOW) != 0;
    }

    inline bool IsAirborne() const {
        return uFlags & PARTY_FLAGS_1_AIRBORNE;
    }

    inline void SetAirborne(bool new_state) {
        if (new_state) {
            uFlags |= PARTY_FLAGS_1_AIRBORNE;
        } else {
            uFlags &= ~PARTY_FLAGS_1_AIRBORNE;
        }
    }

    GameTime &GetPlayingTime() { return this->playing_time; }

    bool IsPartyEvil();
    bool IsPartyGood();
    int _46A89E_immolation_effect(int *affected, int affectedArrSize,
                                  int effectRange);
    int field_0;
    unsigned int uPartyHeight;
    unsigned int uDefaultPartyHeight;
    int sEyelevel;
    unsigned int uDefaultEyelevel;
    int field_14_radius;
    int y_rotation_granularity;
    unsigned int uWalkSpeed;
    int y_rotation_speed;  // deg/s
    int field_24;
    int field_28;
    GameTime playing_time;  // unsigned __int64 uTimePlayed;
    GameTime last_regenerated;
    PartyTimeStruct PartyTimes;
    Vec3_int_ vPosition;
    int sRotationZ;
    int sRotationX;
    Vec3_int_ vPrevPosition;
    int sPrevRotationY;
    int sPrevRotationX;
    int sPrevEyelevel;
    int field_6E0;
    int field_6E4;
    int uFallSpeed;
    int field_6EC;
    int field_6F0;
    int floor_face_pid;  // face we are standing at
    int walk_sound_timer;
    int _6FC_water_lava_timer;
    int uFallStartY;
    unsigned int bFlying;
    char field_708;
    unsigned __int8 hirelingScrollPosition;
    char cNonHireFollowers;  // number of non hireling party guests
    char field_70B;
    unsigned int uCurrentYear;
    unsigned int uCurrentMonth;
    unsigned int uCurrentMonthWeek;
    unsigned int uCurrentDayOfMonth;  // unsigned int uDaysPlayed;
    unsigned int uCurrentHour;
    unsigned int uCurrentMinute;
    unsigned int uCurrentTimeSecond;
    unsigned int uNumFoodRations;
    int field_72C;
    int field_730;
    unsigned int uNumGold;
    unsigned int uNumGoldInBank;
    unsigned int uNumDeaths;
    int field_740;
    int uNumPrisonTerms;
    unsigned int uNumBountiesCollected;
    int field_74C;
    std::array<__int16, 5> monster_id_for_hunting;
    std::array<__int16, 5> monster_for_hunting_killed;
    unsigned char days_played_without_rest;
    unsigned __int8 _quest_bits[64];
    std::array<unsigned __int8, 16> pArcomageWins;
    char field_7B5_in_arena_quest;
    char uNumArenaPageWins;
    char uNumArenaSquireWins;
    char uNumArenaKnightWins;
    char uNumArenaLordWins;
    std::array<char, 29> pIsArtifactFound;  // 7ba
    std::array<char, 39> field_7d7;
    unsigned char _autonote_bits[26];
    std::array<char, 60> field_818;
    std::array<char, 32> field_854;
    int uNumArcomageWins;
    int uNumArcomageLoses;
    bool bTurnBasedModeOn;
    int field_880;
    int uFlags2;
    PartyAlignment alignment;
    std::array<SpellBuff, 20> pPartyBuffs;
    std::array<Player, 4> pPlayers;
    std::array<NPCData, 2> pHirelings;
    ItemGen pPickedItem;
    unsigned int uFlags;
    std::array<std::array<ItemGen, 12>, 53> StandartItemsInShops;
    std::array<std::array<ItemGen, 12>, 53> SpecialItemsInShops;  // D0EC
    std::array<std::array<ItemGen, 12>, 32> SpellBooksInGuilds;
    std::array<char, 24> field_1605C;
    char pHireling1Name[100];
    char pHireling2Name[100];
    int armageddon_timer;
    int armageddonDamage;
    std::array<int, 4> pTurnBasedPlayerRecoveryTimes;
    std::array<int, 53> InTheShopFlags;
    int uFine;
    float flt_TorchlightColorR;
    float flt_TorchlightColorG;
    float flt_TorchlightColorB;
    float TorchLightLastIntensity;
};

extern Party *pParty;  // idb

extern struct ActionQueue *pPartyActionQueue;

bool TestPartyQuestBit(PARTY_QUEST_BITS bit);
void Rest(unsigned int uHoursToSleep);
void RestAndHeal(int uNumMinutes);  // idb
int GetTravelTime();

bool _449B57_test_bit(uint8_t *a1, int16_t a2);
void _449B7E_toggle_bit(unsigned char *pArray, int16_t a2, uint16_t bToggle);  // idb
