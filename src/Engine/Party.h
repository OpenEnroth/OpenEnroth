#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <string>

#include "Engine/Objects/Items.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Objects/Player.h"
#include "Engine/Time.h"
#include "GUI/UI/UIHouseEnums.h"
#include "Library/Random/Random.h"

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

enum class PartyAlignment: int32_t {
    PartyAlignment_Good = 0,
    PartyAlignment_Neutral = 1,
    PartyAlignment_Evil = 2
};
using enum PartyAlignment;

/**
 * Controls gold receiving mechanics.
 */
enum class GoldReceivePolicy {
    GOLD_RECEIVE_SHARE,          // default behaviour: receive gold and share it with hirelings
    GOLD_RECEIVE_NOSHARE_MSG,    // receive gold without sharing, displays status message
    GOLD_RECEIVE_NOSHARE_SILENT  // receive gold without sharing and status message
};
using enum GoldReceivePolicy;

/*  208 */
struct PartyTimeStruct {
    IndexedArray<GameTime, HOUSE_FIRST_TOWNHALL, HOUSE_LAST_TOWNHALL> bountyHunting_next_generation_time; // Size was 10 originally.
    std::array<GameTime, 85> Shops_next_generation_time;  // field_50
    std::array<GameTime, 53> _shop_ban_times;
    std::array<GameTime, 10> CounterEventValues;  // (0xACD314h in Silvo's binary)
    std::array<GameTime, 29> HistoryEventTimes;  // (0xACD364h in Silvo's binary)
    std::array<GameTime, 20> _s_times;  // 5d8 440h+8*51     //(0xACD44Ch in Silvo's binary)
};

struct Party {
    Party() : playing_time(), last_regenerated() {
        Zero();
    }

    void Zero();
    void UpdatePlayersAndHirelingsEmotions();
    void RestAndHeal();

    /**
     * @offset 0x49135E
     */
    unsigned int getPartyFame();

    /**
     * @offset 0x49137D
     */
    void createDefaultParty(bool bDebugGiveItems = false);
    void Reset();
    void ResetPosMiscAndSpellBuffs();

    /**
     * @offset 0x493244
     */
    bool hasItem(ITEM_TYPE uItemID);
    void SetHoldingItem(ItemGen *pItem);

    /**
    * Sets _activeCharacter to the first character that can act
    * Added to fix some nzi access problems
    */
    void setActiveToFirstCanAct();
    /**
    * Sets _activeCharacter to the first active (recoverd) character
    */
    void switchToNextActiveCharacter();
    bool _497FC5_check_party_perception_against_level();
    bool AddItemToParty(ItemGen *pItem);

    /**
     * @offset 0x43AD34
     */
    void yell();
    int CountHirelings();

    void GivePartyExp(unsigned int pEXPNum);
    int GetPartyReputation();

    /**
     * @offset 0x420C05
     */
    void partyFindsGold(int amount, GoldReceivePolicy policy);
    void PickedItem_PlaceInInventory_or_Drop();

    int GetGold() const;
    void SetGold(int amount);
    void AddGold(int amount);
    void TakeGold(int amount);

    int GetFood() const;
    void SetFood(int amount);
    void TakeFood(int amount);
    void GiveFood(int amount);

    int GetBankGold() const;
    void SetBankGold(int amount);
    void AddBankGold(int amount);
    void TakeBankGold(int amount);

    int GetFine() const;
    void SetFine(int amount);
    void AddFine(int amount);
    void TakeFine(int amount);

    /**
     * Perform resting activity within current frame.
     * Used to simulate party resting through time.
     *
     * @offset 0x41F5BE
     */
    static void restOneFrame();

    /**
     * New function - applies fall damage with modifiers to all party members 
     * @param distance                    Fall distance
     */
    void giveFallDamage(int distance);

    inline bool wizardEyeActive() const {
        return pPartyBuffs[PARTY_BUFF_WIZARD_EYE].expire_time.value > 0;
    }
    inline PLAYER_SKILL_MASTERY wizardEyeSkillLevel() const {
        return pPartyBuffs[PARTY_BUFF_WIZARD_EYE].uSkillMastery;
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
    inline PLAYER_SKILL_MASTERY ImmolationSkillLevel() const {
        return pPartyBuffs[PARTY_BUFF_IMMOLATION].uSkillMastery;
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

    inline bool GetRedOrYellowAlert() const {
        return (uFlags & PARTY_FLAGS_1_ALERT_RED_OR_YELLOW) != 0;
    }

    inline bool isAirborne() const {
        return uFlags & PARTY_FLAGS_1_AIRBORNE;
    }

    inline void setAirborne(bool new_state) {
        if (new_state) {
            uFlags |= PARTY_FLAGS_1_AIRBORNE;
        } else {
            uFlags &= ~PARTY_FLAGS_1_AIRBORNE;
        }
    }

    /**
     * @param item_id                   Item type to check, e.g. `ITEM_ARTIFACT_LADYS_ESCORT`.
     * @return                          Whether the provided item is worn by at least one member of the party.
     */
    bool wearsItemAnywhere(ITEM_TYPE item_id) const {
        for (const Player &player : pPlayers) {
            if (player.wearsItemAnywhere(item_id)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Return id of random character that can still act.
     *
     * @param rng         Random generator used.
     * @return            ID of character or -1 if none of the character cat act.
     */
    int getRandomActiveCharacterId(RandomEngine *rng) const {
        std::vector<int> activeCharacters = {};
        for (int i = 0; i < pPlayers.size(); i++) {
            if (pPlayers[i].CanAct()) {
                activeCharacters.push_back(i);
            }
        }
        if (!activeCharacters.empty()) {
            return activeCharacters[rng->random(activeCharacters.size())];
        }
        return -1;
    }

    /**
     * Return id of character that is represented by given pointer.
     *
     * @param player      Pointer to player class.
     * @return            ID of character.
     */
    int getCharacterIdInParty(Player *player) {
        for (int i = 0; i < pPlayers.size(); i++) {
            if (&pPlayers[i] == player) {
                return i;
            }
        }
        assert(false && "Character not found.");
        return -1;
    }

    GameTime &GetPlayingTime() { return this->playing_time; }

    bool isPartyEvil();
    bool isPartyGood();

    /**
     * @offset 0x46A89E
     */
    size_t immolationAffectedActors(int *affected, size_t affectedArrSize, size_t effectRange);

    /**
     * @brief Returns strongest effect of given skill among all conscious players.
     * @param skillType Skill type. Can be only ID item/repair item/merchant currently.
     * @param param Parameter passed to player method.
     *
     * \todo Return also which player contributed strongest skill
    */
    int getSharedSkillStrongestEffect(PLAYER_SKILL_TYPE skillType, std::any param = std::any());
    /**
     * @brief If config option is enabled, returns highest skill effect among all conscious players. Otherwise returns effect of player with given index.
     * @param skill Skill type. Can be only ID item/repair item/merchant currently.
     * @param playerIndex Index of player whose skill would be tested normally. Can be -1 to get currently active character.
     * @param param Parameter passed to player method.
     * 
     * \todo Return also which player contributed strongest skill
     */
    int getOptionallySharedSkillStrongestEffect(PLAYER_SKILL_TYPE skillType,
                                           int playerIndex = -1, std::any param = std::any());

    int field_0_set25_unused;
    int uPartyHeight;
    int uDefaultPartyHeight;
    int sEyelevel;
    int uDefaultEyelevel;
    int radius; // party radius, 37 by default.
    int _yawGranularity;
    int uWalkSpeed;
    int _yawRotationSpeed;  // deg/s
    int jump_strength; // jump strength, higher value => higher jumps, default 5.
    int field_28_set0_unused;
    GameTime playing_time;  // uint64_t uTimePlayed;
    GameTime last_regenerated; // Timestamp when HP/MP regeneration was checked last time (using 5 minutes granularity)
    PartyTimeStruct PartyTimes;
    Vec3i vPosition;
    int _viewYaw;
    int _viewPitch;
    Vec3i vPrevPosition;
    int _viewPrevYaw;
    int _viewPrevPitch;
    int sPrevEyelevel;
    int field_6E0_set0_unused; // party old x/y ?
    int field_6E4_set0_unused; // party old x/y ?
    int uFallSpeed; // party vertical speed, negative => falling, positive => jumping
    int field_6EC_set0_unused;
    int sPartySavedFlightZ;  // this saves the Z position when flying without bob mods
    int floor_face_pid;  // face we are standing at
    int walk_sound_timer;
    int _6FC_water_lava_timer;
    int uFallStartZ;
    unsigned int bFlying;
    char field_708_set15_unused;
    uint8_t hirelingScrollPosition;
    char cNonHireFollowers;  // number of non hireling party guests
    char field_70B_set0_unused;
    unsigned int uCurrentYear;
    unsigned int uCurrentMonth;
    unsigned int uCurrentMonthWeek;
    unsigned int uCurrentDayOfMonth;  // unsigned int uDaysPlayed;
    unsigned int uCurrentHour;
    unsigned int uCurrentMinute;
    unsigned int uCurrentTimeSecond;
    unsigned int uNumFoodRations;
    int field_72C_set0_unused;
    int field_730_set0_unused;
    unsigned int uNumGold;
    unsigned int uNumGoldInBank;
    unsigned int uNumDeaths;
    int field_740_set0_unused;
    int uNumPrisonTerms;
    unsigned int uNumBountiesCollected;
    int field_74C_set0_unused;
    IndexedArray<int16_t, HOUSE_FIRST_TOWNHALL, HOUSE_LAST_TOWNHALL> monster_id_for_hunting;
    IndexedArray<int16_t, HOUSE_FIRST_TOWNHALL, HOUSE_LAST_TOWNHALL> monster_for_hunting_killed; // TODO(captainurist): bool
    unsigned char days_played_without_rest;
    uint8_t _quest_bits[64];
    std::array<uint8_t, 16> pArcomageWins;
    char field_7B5_in_arena_quest; // 0, DIALOGUE_ARENA_SELECT_PAGE..DIALOGUE_ARENA_SELECT_CHAMPION, or -1 for win
    std::array<char, 4> uNumArenaWins; // 0=page, 1=squire, 2=knight, 3=lord
    IndexedArray<bool, ITEM_FIRST_SPAWNABLE_ARTIFACT, ITEM_LAST_SPAWNABLE_ARTIFACT> pIsArtifactFound;  // 7ba
    std::array<char, 39> field_7d7_set0_unused;
    unsigned char _autonote_bits[26];
    std::array<char, 60> field_818_set0_unused;
    std::array<char, 32> random_order_num_unused;
    int uNumArcomageWins;
    int uNumArcomageLoses;
    bool bTurnBasedModeOn;
    int field_880_set0_unused;
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
    std::array<char, 24> field_1605C_set0_unused;
    std::string pHireling1Name;
    std::string pHireling2Name;
    int armageddon_timer;
    int armageddonDamage;
    std::array<int, 4> pTurnBasedPlayerRecoveryTimes;
    std::array<int, 53> InTheShopFlags;
    int uFine;
    float flt_TorchlightColorR;
    float flt_TorchlightColorG;
    float flt_TorchlightColorB;
    float TorchLightLastIntensity;

    uint _roundingDt{ 0 };  // keeps track of rounding remainder for recovery

    inline uint getActiveCharacter() const {
        assert(hasActiveCharacter());
        return _activeCharacter;
    }
    inline void setActiveCharacter(uint id) {
        assert(id >= 0 && id <= pPlayers.size());
        _activeCharacter = id;
    }
    inline bool hasActiveCharacter() const {
        return _activeCharacter > 0;
    }
    // TODO(pskelton): function for returning ref pPlayers[pParty->getActiveCharacter()]

 private:
     // TODO(pskelton): rename activePlayer
     // TODO(pskelton): change to signed int - make 0 based with -1 for none??
     unsigned int _activeCharacter;  // which character is active - 1 based; 0 for none
};

extern Party *pParty;  // idb

extern struct ActionQueue *pPartyActionQueue;

bool TestPartyQuestBit(PARTY_QUEST_BITS bit);

/**
 * Perform resting without healing.
 *
 * @param restTime      Resting time.
 * @offset 0x4938D1
 */
void Rest(GameTime restTime);
void RestAndHeal(int uNumMinutes);  // idb

/**
 * @offset 0x444D80
 */
int getTravelTime();

bool _449B57_test_bit(uint8_t *a1, int16_t a2);
void _449B7E_toggle_bit(unsigned char *pArray, int16_t a2, uint16_t bToggle);  // idb
