#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <algorithm>

#include "Engine/Objects/Items.h"
#include "Engine/Tables/NPCTable.h"
#include "Engine/Objects/Character.h"
#include "Engine/Time/Time.h"
#include "Engine/Time/Timer.h"
#include "GUI/UI/UIHouseEnums.h"
#include "Media/Audio/SoundEnums.h"
#include "Utility/IndexedBitset.h"

#include "ArenaEnums.h"
#include "PartyEnums.h"

class RandomEngine;

struct ActionQueue {
    inline ActionQueue() : uNumActions(0) {}

    void Add(PartyAction action);
    void Reset();
    PartyAction Next();

    unsigned int uNumActions;
    PartyAction pActions[30]{};
};

struct PartyTimeStruct {
    IndexedArray<Time, HOUSE_FIRST_TOWN_HALL, HOUSE_LAST_TOWN_HALL> bountyHuntNextGenTime; // Size was 10 originally.
    IndexedArray<Time, HOUSE_FIRST_SHOP, HOUSE_LAST_SHOP> shopNextRefreshTime;
    IndexedArray<Time, HOUSE_FIRST_MAGIC_GUILD, HOUSE_LAST_MAGIC_GUILD> guildNextRefreshTime;
    IndexedArray<Time, HOUSE_FIRST_SHOP, HOUSE_LAST_SHOP> shopBanTimes;
    std::array<Time, 10> CounterEventValues;  // (0xACD314h in Silvo's binary)
    std::array<Time, 29> HistoryEventTimes;  // (0xACD364h in Silvo's binary)
    std::array<Time, 20> _s_times;  // 5d8 440h+8*51     //(0xACD44Ch in Silvo's binary)
};

struct Party {
    Party() : playing_time(), last_regenerated() {
        Zero();
    }

    void Zero();

    void resetCharacterEmotions();

    /**
     * @offset 0x4909F4
     */
    void updateCharactersAndHirelingsEmotions();

    /**
     * @offset 0x490D02
     */
    void restAndHeal();

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
    bool hasItem(ItemId uItemID);

    /**
     * @offset 0x4936E1
     */
    void setHoldingItem(ItemGen *pItem);

    /**
    * Sets _activeCharacter to the first character that can act
    * Added to fix some nzi access problems
    */
    void setActiveToFirstCanAct();
    /**
    * Sets _activeCharacter to the first active (recoverd) character
    */
    void switchToNextActiveCharacter();

    /**
     * @offset 0x497FC5
     */
    bool checkPartyPerceptionAgainstCurrentMap();

    /**
     * @return                          Total number of characters who can act.
     */
    int canActCount() const;

    /**
     * @offset 0x48C6F6
     */
    bool addItemToParty(ItemGen *pItem, bool isSilent = false);

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
    void placeHeldItemInInventoryOrDrop();
    void dropHeldItem();

    int GetGold() const;
    void SetGold(int amount, bool silent = false);
    void AddGold(int amount);
    void TakeGold(int amount, bool silent = false);

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
        return pPartyBuffs[PARTY_BUFF_WIZARD_EYE].Active();
    }
    inline CharacterSkillMastery wizardEyeSkillLevel() const {
        return pPartyBuffs[PARTY_BUFF_WIZARD_EYE].skillMastery;
    }
    inline bool TorchlightActive() const {
        return pPartyBuffs[PARTY_BUFF_TORCHLIGHT].Active();
    }
    inline bool FlyActive() const {
        return pPartyBuffs[PARTY_BUFF_FLY].Active();
    }
    inline bool WaterWalkActive() const {
        return pPartyBuffs[PARTY_BUFF_WATER_WALK].Active();
    }
    inline bool ImmolationActive() const {
        return pPartyBuffs[PARTY_BUFF_IMMOLATION].Active();
    }
    inline CharacterSkillMastery ImmolationSkillLevel() const {
        return pPartyBuffs[PARTY_BUFF_IMMOLATION].skillMastery;
    }
    inline bool FeatherFallActive() const {
        return pPartyBuffs[PARTY_BUFF_FEATHER_FALL].Active();
    }
    inline bool Invisible() const {
        return pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active();
    }

    inline bool GetRedAlert() const {
        return uFlags & PARTY_FLAG_ALERT_RED;
    }
    inline void SetRedAlert() { uFlags |= PARTY_FLAG_ALERT_RED; }
    inline bool GetYellowAlert() const {
        return uFlags & PARTY_FLAG_ALERT_YELLOW;
    }
    inline void SetYellowAlert() { uFlags |= PARTY_FLAG_ALERT_YELLOW; }

    inline bool GetRedOrYellowAlert() const {
        return uFlags & PARTY_FLAG_ALERT_RED_OR_YELLOW;
    }

    inline bool isAirborne() const {
        return uFlags & PARTY_FLAG_AIRBORNE;
    }

    inline void setAirborne(bool new_state) {
        if (new_state) {
            uFlags |= PARTY_FLAG_AIRBORNE;
        } else {
            uFlags &= ~PARTY_FLAG_AIRBORNE;
        }
    }

    /**
     * @param item_id                   Item type to check, e.g. `ITEM_ARTIFACT_LADYS_ESCORT`.
     * @return                          Whether the provided item is worn by at least one member of the party.
     */
    bool wearsItemAnywhere(ItemId item_id) const {
        for (const Character &character : pCharacters) {
            if (character.wearsItemAnywhere(item_id)) {
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
    int getRandomActiveCharacterId(RandomEngine *rng) const;

    /**
     * Return id of character that is represented by given pointer.
     *
     * @param character     `Character` pointer.
     * @return              0-based index of the given character in party.
     */
    int getCharacterIdInParty(Character *character) {
        assert(character >= pCharacters.data() && character < pCharacters.data() + pCharacters.size());
        return character - pCharacters.data();
    }

    Time &GetPlayingTime() { return this->playing_time; }

    bool isPartyEvil();
    bool isPartyGood();

    /**
     * @offset 0x46A89E
     */
    size_t immolationAffectedActors(int *affected, size_t affectedArrSize, size_t effectRange);

    void setDelayedReaction(CharacterSpeech speech, int id) {
        if (!_delayedReactionTimer) {
            _delayedReactionTimer = Duration::fromRealtimeSeconds(2);
            _delayedReactionSpeech = speech;
            _delayedReactionCharacterId = id;
        }
    }

    void updateDelayedReaction() {
        if (_delayedReactionTimer) {
            _delayedReactionTimer = std::max(0_ticks, _delayedReactionTimer - pMiscTimer->dt());
            if (!_delayedReactionTimer && pCharacters[_delayedReactionCharacterId].CanAct()) {
                pCharacters[_delayedReactionCharacterId].playReaction(_delayedReactionSpeech);
            }
        }
    }

    int height; // Party height, 192 by default.
    int eyeLevel; // Party eye level, 160 by default.
    int radius; // Party radius, 37 by default.
    int _yawGranularity;
    int walkSpeed; // Party walk speed, units per real time second.
    int _yawRotationSpeed;  // deg/s
    int jump_strength; // jump strength, higher value => higher jumps, default 5.
    Time playing_time;  // uint64_t uTimePlayed;
    Time last_regenerated; // Timestamp when HP/MP regeneration was done last time.
    PartyTimeStruct PartyTimes;
    Vec3f pos;
    Vec3f velocity; // Party speed, negative z => falling, positive z => jumping.
                 // Note that only z component is preserved between frames, x & y are fully determined by the keys pressed.
    int _viewYaw; // View yaw in polar coordinates, 0 is positive X, 512 (pi/2) is positive Y.
    int _viewPitch; // View pitch in polar coordinates, 0 is horizontal, positive is looking up, negative is looking down.
    Vec3f lastPos; // TODO(captainurist): drop
    int _viewPrevYaw;
    int _viewPrevPitch;
    int lastEyeLevel; // TODO(captainurist): drop
    int sPartySavedFlightZ;  // this saves the Z position when flying without bob mods
    int floor_face_id;  // face we are standing at
    SoundId currentWalkingSound; // previously was 'walk_sound_timer'
    Time _6FC_water_lava_timer;
    int uFallStartZ;
    unsigned int bFlying;
    int hirelingScrollPosition;
    char cNonHireFollowers;  // number of non hireling party guests

    // TODO(captainurist): #time drop all of these?
    unsigned int uCurrentYear;
    unsigned int uCurrentMonth;
    unsigned int uCurrentMonthWeek;
    unsigned int uCurrentDayOfMonth;  // unsigned int uDaysPlayed;
    unsigned int uCurrentHour;
    unsigned int uCurrentMinute;
    unsigned int uCurrentTimeSecond;

    unsigned int uNumFoodRations;
    unsigned int uNumGold;
    unsigned int uNumGoldInBank;
    int uNumDeaths;
    int uNumPrisonTerms;
    unsigned int uNumBountiesCollected;
    IndexedArray<MonsterId, HOUSE_FIRST_TOWN_HALL, HOUSE_LAST_TOWN_HALL> monster_id_for_hunting;
    IndexedArray<bool, HOUSE_FIRST_TOWN_HALL, HOUSE_LAST_TOWN_HALL> monster_for_hunting_killed;
    unsigned char days_played_without_rest;
    IndexedBitset<QBIT_FIRST, QBIT_LAST> _questBits;
    IndexedArray<bool, HOUSE_FIRST_ARCOMAGE_TAVERN, HOUSE_LAST_ARCOMAGE_TAVERN> pArcomageWins;
    ArenaState arenaState = ARENA_STATE_INITIAL;
    ArenaLevel arenaLevel = ARENA_LEVEL_INVALID;
    IndexedArray<uint8_t, ARENA_LEVEL_FIRST_VALID, ARENA_LEVEL_LAST_VALID> uNumArenaWins;
    IndexedArray<bool, ITEM_FIRST_SPAWNABLE_ARTIFACT, ITEM_LAST_SPAWNABLE_ARTIFACT> pIsArtifactFound;  // 7ba
    IndexedBitset<1, 208> _autonoteBits;
    int uNumArcomageWins;
    int uNumArcomageLoses;
    bool bTurnBasedModeOn;
    PARTY_FLAGS_2 uFlags2;
    PartyAlignment alignment;
    IndexedArray<SpellBuff, PARTY_BUFF_FIRST, PARTY_BUFF_LAST> pPartyBuffs;
    std::array<Character, 4> pCharacters;
    std::array<NPCData, 2> pHirelings;
    std::array<NPCSacrificeStatus, 2> pHirelingsSacrifice;
    ItemGen pPickedItem;
    PartyFlags uFlags;
    IndexedArray<std::array<ItemGen, 12>, HOUSE_FIRST_SHOP, HOUSE_LAST_SHOP> standartItemsInShops;
    IndexedArray<std::array<ItemGen, 12>, HOUSE_FIRST_SHOP, HOUSE_LAST_SHOP> specialItemsInShops;
    IndexedArray<std::array<ItemGen, 12>, HOUSE_FIRST_MAGIC_GUILD, HOUSE_LAST_MAGIC_GUILD> spellBooksInGuilds;
    std::string pHireling1Name;
    std::string pHireling2Name;
    Duration armageddon_timer;
    int armageddonDamage;
    // Keeps track of how many impulses have been applied to actors during armeggeddon
    // Stops actors being yeeted high in the air at high fps
    int armageddonForceCount{ 0 };
    std::array<Duration, 4> pTurnBasedCharacterRecoveryTimes;
    IndexedArray<int, HOUSE_FIRST_SHOP, HOUSE_LAST_SHOP> InTheShopFlags;
    int uFine = 0;
    float TorchLightLastIntensity = 0.0f;

    Duration _roundingDt;  // keeps track of rounding remainder for recovery

    Duration _delayedReactionTimer;
    CharacterSpeech _delayedReactionSpeech = SPEECH_NONE;
    int _delayedReactionCharacterId = -1;

    std::array<bool, 4> playerAlreadyPicked = {{}};  // Was at offset 0xAE3368 in vanilla, we moved it into Party in OE.

    /**
     * @return                          1-based index of currently active character. Zero means that there is no
     *                                  active character.
     */
    int activeCharacterIndex() const {
        assert(hasActiveCharacter());
        return _activeCharacter;
    }

    /**
     * @param id                        1-based index of currently active character. Pass zero to make no one active.
     */
    void setActiveCharacterIndex(int id) {
        assert(id >= 0 && id <= pCharacters.size());
        _activeCharacter = id;
    }
    bool hasActiveCharacter() const {
        return _activeCharacter > 0;
    }
    Character &activeCharacter() {
        assert(hasActiveCharacter());
        return pCharacters[_activeCharacter - 1];
    }

 private:
     // TODO(pskelton): make 0-based with -1 for none??
     int _activeCharacter = 0;  // which character is active - 1 based; 0 for none
};

extern Party *pParty;  // idb

extern ActionQueue *pPartyActionQueue;

bool TestPartyQuestBit(QuestBit bit);

/**
 * Perform resting without healing.
 *
 * @param restTime      Resting time.
 * @offset 0x4938D1
 */
void Rest(Duration restTime);

/**
 * Perform resting with healing.
 *
 * @param restTime      Resting time.
 * @offset 0x4B1BDB
 */
void restAndHeal(Duration restTime);

/**
 * @offset 0x444D80
 */
int getTravelTime();
