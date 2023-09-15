#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <string>

#include "Engine/Objects/Items.h"
#include "Engine/Tables/NPCTable.h"
#include "Engine/Objects/Character.h"
#include "Engine/Time.h"
#include "GUI/UI/UIHouseEnums.h"
#include "Media/Audio/SoundEnums.h"
#include "Utility/IndexedBitset.h"

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

/*  208 */
struct PartyTimeStruct {
    IndexedArray<GameTime, HOUSE_FIRST_TOWNHALL, HOUSE_LAST_TOWNHALL> bountyHuntNextGenTime; // Size was 10 originally.
    IndexedArray<GameTime, HOUSE_FIRST_SHOP, HOUSE_LAST_SHOP> shopNextRefreshTime;
    IndexedArray<GameTime, HOUSE_FIRST_MAGIC_GUILD, HOUSE_LAST_MAGIC_GUILD> guildNextRefreshTime;
    IndexedArray<GameTime, HOUSE_FIRST_SHOP, HOUSE_LAST_SHOP> shopBanTimes;
    std::array<GameTime, 10> CounterEventValues;  // (0xACD314h in Silvo's binary)
    std::array<GameTime, 29> HistoryEventTimes;  // (0xACD364h in Silvo's binary)
    std::array<GameTime, 20> _s_times;  // 5d8 440h+8*51     //(0xACD44Ch in Silvo's binary)
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
    bool _497FC5_check_party_perception_against_level();

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
        return uFlags & PARTY_FLAGS_1_ALERT_RED;
    }
    inline void SetRedAlert() { uFlags |= PARTY_FLAGS_1_ALERT_RED; }
    inline bool GetYellowAlert() const {
        return uFlags & PARTY_FLAGS_1_ALERT_YELLOW;
    }
    inline void SetYellowAlert() { uFlags |= PARTY_FLAGS_1_ALERT_YELLOW; }

    inline bool GetRedOrYellowAlert() const {
        return uFlags & PARTY_FLAGS_1_ALERT_RED_OR_YELLOW;
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
     * @param character     Pointer to character class.
     * @return              ID of character.
     */
    int getCharacterIdInParty(Character *character) {
        for (int i = 0; i < pCharacters.size(); i++) {
            if (&pCharacters[i] == character) {
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

    void setDelayedReaction(CharacterSpeech speech, int id) {
        if (!_delayedReactionTimer) {
            _delayedReactionTimer = Timer::Second * 2;
            _delayedReactionSpeech = speech;
            _delayedReactionCharacterId = id;
        }
    }

    void updateDelayedReaction() {
        if (_delayedReactionTimer) {
            _delayedReactionTimer -= pMiscTimer->uTimeElapsed;
            if (_delayedReactionTimer <= 0) {
                if (pCharacters[_delayedReactionCharacterId].CanAct()) {
                    pCharacters[_delayedReactionCharacterId].playReaction(_delayedReactionSpeech);
                }
                _delayedReactionTimer = 0;
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
    GameTime playing_time;  // uint64_t uTimePlayed;
    GameTime last_regenerated; // Timestamp when HP/MP regeneration was checked last time (using 5 minutes granularity)
    PartyTimeStruct PartyTimes;
    Vec3i pos;
    Vec3i speed; // Party speed, negative z => falling, positive z => jumping.
                 // Note that only z component is preserved between frames, x & y are fully determined by the keys pressed.
    int _viewYaw; // View yaw in polar coordinates, 0 is positive X, 512 (pi/2) is positive Y.
    int _viewPitch; // View pitch in polar coordinates, 0 is horizontal, positive is looking up, negative is looking down.
    Vec3i lastPos; // TODO(captainurist): drop
    int _viewPrevYaw;
    int _viewPrevPitch;
    int lastEyeLevel; // TODO(captainurist): drop
    int sPartySavedFlightZ;  // this saves the Z position when flying without bob mods
    int floor_face_id;  // face we are standing at
    SoundID currentWalkingSound; // previously was 'walk_sound_timer'
    int _6FC_water_lava_timer;
    int uFallStartZ;
    unsigned int bFlying;
    uint8_t hirelingScrollPosition;
    char cNonHireFollowers;  // number of non hireling party guests
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
    unsigned int uNumDeaths;
    int uNumPrisonTerms;
    unsigned int uNumBountiesCollected;
    IndexedArray<int16_t, HOUSE_FIRST_TOWNHALL, HOUSE_LAST_TOWNHALL> monster_id_for_hunting;
    IndexedArray<bool, HOUSE_FIRST_TOWNHALL, HOUSE_LAST_TOWNHALL> monster_for_hunting_killed;
    unsigned char days_played_without_rest;
    IndexedBitset<QBIT_FIRST, QBIT_LAST> _questBits;
    std::array<uint8_t, 16> pArcomageWins;
    int8_t field_7B5_in_arena_quest; // 0, DIALOGUE_ARENA_SELECT_PAGE..DIALOGUE_ARENA_SELECT_CHAMPION, or -1 for win
    std::array<char, 4> uNumArenaWins; // 0=page, 1=squire, 2=knight, 3=lord
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
    ItemGen pPickedItem;
    PARTY_FLAGS_1 uFlags;
    IndexedArray<std::array<ItemGen, 12>, HOUSE_FIRST_SHOP, HOUSE_LAST_SHOP> standartItemsInShops;
    IndexedArray<std::array<ItemGen, 12>, HOUSE_FIRST_SHOP, HOUSE_LAST_SHOP> specialItemsInShops;
    IndexedArray<std::array<ItemGen, 12>, HOUSE_FIRST_MAGIC_GUILD, HOUSE_LAST_MAGIC_GUILD> spellBooksInGuilds;
    std::string pHireling1Name;
    std::string pHireling2Name;
    int armageddon_timer;
    int armageddonDamage;
    // Keeps track of how many impulses have been applied to actors during armeggeddon
    // Stops actors being yeeted high in the air at high fps
    int armageddonForceCount{ 0 };
    std::array<int, 4> pTurnBasedCharacterRecoveryTimes;
    std::array<int, 53> InTheShopFlags;
    int uFine;
    float TorchLightLastIntensity;

    unsigned int _roundingDt{ 0 };  // keeps track of rounding remainder for recovery

    int _delayedReactionTimer;
    CharacterSpeech _delayedReactionSpeech;
    int _delayedReactionCharacterId;

    /**
     * @return                          1-based index of currently active character. Zero means that there is no
     *                                  active character.
     */
    inline unsigned int activeCharacterIndex() const {
        assert(hasActiveCharacter());
        return _activeCharacter;
    }

    /**
     * @param id                        1-based index of currently active character. Pass zero to make no one active.
     */
    inline void setActiveCharacterIndex(unsigned int id) {
        assert(id >= 0 && id <= pCharacters.size());
        _activeCharacter = id;
    }
    inline bool hasActiveCharacter() const {
        return _activeCharacter > 0;
    }
    inline Character &activeCharacter() {
        assert(hasActiveCharacter());
        return pCharacters[_activeCharacter - 1];
    }

 private:
     // TODO(pskelton): change to signed int - make 0 based with -1 for none??
     unsigned int _activeCharacter;  // which character is active - 1 based; 0 for none
};

extern Party *pParty;  // idb

extern struct ActionQueue *pPartyActionQueue;

bool TestPartyQuestBit(QuestBit bit);

/**
 * Perform resting without healing.
 *
 * @param restTime      Resting time.
 * @offset 0x4938D1
 */
void Rest(GameTime restTime);

/**
 * Perform resting with healing.
 *
 * @param restTime      Resting time.
 * @offset 0x4B1BDB
 */
void restAndHeal(GameTime restTime);

/**
 * @offset 0x444D80
 */
int getTravelTime();
