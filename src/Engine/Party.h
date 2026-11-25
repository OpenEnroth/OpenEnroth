#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <algorithm>

#include "Engine/Objects/Item.h"
#include "Engine/Tables/NPCTable.h"
#include "Engine/Objects/Character.h"
#include "Engine/PartyMM8Roster.h"
#include "Engine/Timer.h"

#include "Engine/Data/SoundEnums.h"

#include "Core/Time/Time.h"

#include "Library/Geometry/Point.h"

#include "Utility/IndexedBitset.h"

#include "ArenaEnums.h"
#include "PartyEnums.h"

class RandomEngine;

struct ActionQueue {
    inline ActionQueue() : uNumActions(0) {}

    void Add(PartyAction action);
    void Reset();
    PartyAction Next();

    unsigned int uNumActions = 0;
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
    unsigned int fame();

    /**
     * @offset 0x49137D
     */
    void createDefaultParty();
    void Reset();
    void ResetPosMiscAndSpellBuffs();

    /**
     * @offset 0x493244
     */
    bool hasItem(ItemId uItemID);

    /**
     * @param item                      Item to hold in mouse.
     * @param offset                    Item's bitmap offset relative to cursor position. Always non-positive.
     * @offset 0x4936E1
     */
    void setHoldingItem(const Item &item, Pointi offset = {});

    Item takeHoldingItem();

    /**
     * Makes the first character that can act the active one.
     */
    void setActiveToFirstCanAct();

    /**
     * Picks the active character for the next action. Keeps the current one if it can act and has recovered,
     * otherwise picks the next character that has, or no one if none has.
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
     * @return                          Number of party members that are actually part of the
     *                                  current game, as opposed to `pCharacters.size()`, which is
     *                                  a fixed 5 regardless of game version. Under `GAME_TYPE_MM8`
     *                                  a fresh game is a solo (1-champion) start. Slots
     *                                  1-4 hold real recruited companions only once `recruitRoster()`
     *                                  fills them (unrecruited slots are inert placeholders, see
     *                                  `Character::rosterId`) and must not be drawn/counted as
     *                                  active party members. MM7 is unaffected: it always returns
     *                                  the fixed 4 (index 4 is MM8-only, always inert for MM7).
     */
    int activeCharacterCount() const;

    /**
     * MM8 only. Puts the character built from `roster.txt` row `rosterId` into the first free
     * active party slot (`pCharacters[1..4]`), or into the Adventurer's Inn (`mm8Roster`) if the
     * active party is already full (5 members).
     *
     * @return                          Whether the character joined the active party (false means
     *                                  it went to the Inn instead).
     */
    bool recruitRoster(int rosterId);

    /**
     * MM8 only. Puts the heroes the guides place at the Dagger Wound Adventurer's Inn from the
     * start of a new game on the Inn bench (`mm8Roster`), so the active party is unchanged.
     */
    void seatStartingInnRecruits();

    /**
     * MM8 only. A script that sets a roster row's own "QuestBit" (400 + row, e.g. 430 for row 30,
     * set by a statue's event in Murmurwoods) makes that hero wait at the
     * Adventurer's Inn, as in the original game. A no-op if no row owns the bit or the
     * hero is already in the party or at the Inn.
     */
    void seatRosterOfQuestBit(QuestBit bit);

    /**
     * Test-only: a party that cannot die. Members that join later (`recruitRoster`/`activateFromInn`/`swapWithInn`)
     * get the same treatment, with counters starting at join time. 0 / false = off. Not saved.
     */
    int survivableHp = 0;
    bool survivableConditions = false;
    /** Test-only: every party melee/ranged/spell hit on an actor lands and deals exactly this much, ignoring immunity. 0 = off. Not saved. */
    int survivableDamage = 0;
    void armSurvivable(Character &character) const;

    /**
     * MM8 only. Moves the roster companion with the given `rosterId` from the active party to the
     * Adventurer's Inn, compacting the remaining active slots. A no-op (returns false) if `rosterId`
     * isn't currently in the active party, if `rosterId <= 0` (the champion, slot 0, is never
     * roster-backed and can't be dismissed this way), or if `bTurnBasedModeOn` is set (the original refuses
     * a dismiss in turn-based mode, showing `LSTR_YOU_CANNOT_DISMISS_A_CHARACTER_IN_TURN_BASED_MODE`).
     *
     * @return                          Whether a party member was actually dismissed.
     */
    bool dismissRoster(int rosterId);

    /**
     * MM8 only. Adventurer's Inn engine op: moves the roster companion with the given `rosterId`
     * from the Inn bench into the first free active party slot. A no-op (returns false, changes
     * nothing) if `rosterId` isn't currently on the bench, or if the active party is already full
     * (5 members) - use `swapWithInn()` instead in that case.
     *
     * @return                          Whether the companion actually joined the active party.
     */
    bool activateFromInn(int rosterId);

    /**
     * MM8 only. Adventurer's Inn engine op: exchanges the bench companion `benchRosterId` with the
     * active companion `activeRosterId` in one atomic move, so it works even with a full (5-member)
     * party. A no-op (returns false, changes nothing) if `benchRosterId` isn't on the bench, if
     * `activeRosterId` isn't currently in the active party, or if `activeRosterId == 0` (the original never
     * lets the champion be swapped out via the Inn).
     *
     * @return                          Whether the swap actually happened.
     */
    bool swapWithInn(int benchRosterId, int activeRosterId);

    /**
     * MM8 only.
     *
     * @return                          Whether the roster companion with the given `rosterId` is
     *                                  currently in the active party (as opposed to the Inn, or not
     *                                  recruited at all).
     */
    bool isRosterInParty(int rosterId) const;

    /**
     * MM8 only. Forces every `pCharacters` slot at index >= `activeCharacterCount()` (an
     * unrecruited roster placeholder) into `CONDITION_DEAD` - the "inert by construction"
     * invariant: `Character::CanAct()`/`IsDead()` alone is then enough to keep a
     * placeholder out of anything that reads raw `pCharacters` without going through
     * `activeCharacterCount()`. A no-op under
     * MM7 (fixed party of 4, no placeholder slots exist) and for any slot already dead. Idempotent;
     * call after anything that can change which slots count as active or repopulate a slot:
     * `Reset()`, `recruitRoster()`, `dismissRoster()`, and after loading a save.
     */
    void enforcePlaceholderInertness();

    /**
     * @offset 0x48C6F6
     */
    bool addItemToParty(Item *pItem, bool isSilent = false);

    /**
     * @offset 0x43AD34
     */
    void yell();
    int CountHirelings();

    void GivePartyExp(int pEXPNum);

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
    inline Mastery wizardEyeSkillLevel() const {
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
    inline Mastery ImmolationSkillLevel() const {
        return pPartyBuffs[PARTY_BUFF_IMMOLATION].skillMastery;
    }
    inline bool LevitateActive() const {
        return pPartyBuffs[PARTY_BUFF_LEVITATE].Active();
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
     * @param itemId                    Item type to check, e.g. `ITEM_ARTIFACT_LADYS_ESCORT`.
     * @return                          Whether the provided item is worn by at least one member of the party.
     */
    bool wearsItem(ItemId itemId) const {
        for (const Character &character : pCharacters)
            if (character.wearsItem(itemId))
                return true;
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

    void setDelayedReaction(SpeechId speech, int id) {
        if (!_delayedReactionTimer) {
            _delayedReactionTimer = Duration::fromRealtimeSeconds(2);
            _delayedReactionSpeech = speech;
            _delayedReactionCharacterId = id;
        }
    }

    void updateDelayedReaction() {
        if (_delayedReactionTimer) {
            _delayedReactionTimer = std::max(0_ticks, _delayedReactionTimer - animTimer->dt());
            if (!_delayedReactionTimer && pCharacters[_delayedReactionCharacterId].CanAct())
                pCharacters[_delayedReactionCharacterId].playReaction(_delayedReactionSpeech);
        }
    }

    int height = 0; // Party height, 192 by default.
    int eyeLevel = 0; // Party eye level, 160 by default.
    int radius = 0; // Party radius, 37 by default.
    int _yawGranularity = 0;
    int walkSpeed = 0; // Party walk speed, units per real time second.
    int _yawRotationSpeed = 0;  // deg/s
    int jump_strength = 0; // jump strength, higher value => higher jumps, default 5.
    Time playing_time;  // uint64_t uTimePlayed;
    Time last_regenerated; // Timestamp when HP/MP regeneration was done last time.
    PartyTimeStruct PartyTimes;
    Vec3f pos;
    Vec3f velocity; // Party speed, negative z => falling, positive z => jumping.
                 // Note that only z component is preserved between frames, x & y are fully determined by the keys pressed.
    int _viewYaw = 0; // View yaw in polar coordinates, 0 is positive X, 512 (pi/2) is positive Y.
    int _viewPitch = 0; // View pitch in polar coordinates, 0 is horizontal, positive is looking up, negative is looking down.
    int sPartySavedFlightZ = 0;  // this saves the Z position when flying without bob mods
    int floor_face_id = -1;  // Face the party is standing on, -1 for none.
    SoundId currentWalkingSound = SOUND_Invalid; // previously was 'walk_sound_timer'
    Time _6FC_water_lava_timer;
    int uFallStartZ = 0;
    unsigned int bFlying = 0;
    int hirelingScrollPosition = 0;
    char cNonHireFollowers = 0;  // number of non hireling party guests

    // TODO(captainurist): #time replace with GetCurrentCivilTime().
    int uCurrentYear = 0;
    int uCurrentMonth = 0;
    int uCurrentMonthWeek = 0;
    int uCurrentDayOfMonth = 0;  // unsigned int uDaysPlayed;
    int uCurrentHour = 0;
    int uCurrentMinute = 0;
    int uCurrentTimeSecond = 0;

    int uNumFoodRations = 0;
    int uNumGold = 0;
    int uNumGoldInBank = 0;
    int uNumDeaths = 0;
    int uNumPrisonTerms = 0;
    int uNumBountiesCollected = 0;
    IndexedArray<MonsterId, HOUSE_FIRST_TOWN_HALL, HOUSE_LAST_TOWN_HALL> monster_id_for_hunting;
    IndexedArray<bool, HOUSE_FIRST_TOWN_HALL, HOUSE_LAST_TOWN_HALL> monster_for_hunting_killed;
    unsigned char days_played_without_rest = 0;
    IndexedBitset<QBIT_FIRST, QBIT_LAST> _questBits;
    std::array<bool, 13> pArcomageWins; // Indexed by arcomageTavernIndex().
    ArenaState arenaState = ARENA_STATE_INITIAL;
    ArenaLevel arenaLevel = ARENA_LEVEL_INVALID;
    IndexedArray<uint8_t, ARENA_LEVEL_FIRST_VALID, ARENA_LEVEL_LAST_VALID> uNumArenaWins;
    IndexedArray<bool, ITEM_FIRST_SPAWNABLE_ARTIFACT, ITEM_LAST_SPAWNABLE_ARTIFACT> isArtifactGenerated;  // 7ba
    IndexedBitset<1, 208> _autonoteBits;
    int uNumArcomageWins = 0;
    int uNumArcomageLoses = 0;
    bool bTurnBasedModeOn = false;
    PARTY_FLAGS_2 uFlags2 = {};
    PartyAlignment alignment = PartyAlignment_Neutral;
    IndexedArray<SpellBuff, PARTY_BUFF_FIRST, PARTY_BUFF_LAST> pPartyBuffs;
    // 5 slots: MM8's champion and up to 4 recruits. MM7 uses the first 4, see `activeCharacterCount()`.
    std::array<Character, 5> pCharacters;
    std::array<NPCData, 2> pHirelings;
    // MM8's Adventurer's Inn bench: companions recruited but not currently in the active party (see
    // `PartyMM8Roster`), keyed by `Character::rosterId`. Unused under MM7, which keeps its fixed-4
    // roster in `pCharacters`/`pHirelings` above untouched.
    PartyMM8Roster mm8Roster;
    std::array<NPCSacrificeStatus, 2> pHirelingsSacrifice;
    Item pPickedItem;
    PartyFlags uFlags;
    IndexedArray<std::array<Item, 12>, HOUSE_FIRST_SHOP, HOUSE_LAST_SHOP> standartItemsInShops;
    IndexedArray<std::array<Item, 12>, HOUSE_FIRST_SHOP, HOUSE_LAST_SHOP> specialItemsInShops;
    IndexedArray<std::array<Item, 12>, HOUSE_FIRST_MAGIC_GUILD, HOUSE_LAST_MAGIC_GUILD> spellBooksInGuilds;
    std::string pHireling1Name;
    std::string pHireling2Name;
    Duration armageddon_timer;
    int armageddonDamage = 0;
    // Keeps track of how many impulses have been applied to actors during armeggeddon
    // Stops actors being yeeted high in the air at high fps
    int armageddonForceCount{ 0 };
    std::array<Duration, 5> pTurnBasedCharacterRecoveryTimes;
    IndexedArray<int, HOUSE_FIRST_SHOP, HOUSE_LAST_SHOP> InTheShopFlags;
    int uFine = 0;
    float TorchLightLastIntensity = 0.0f;

    Duration _roundingDt;  // keeps track of rounding remainder for recovery

    Duration _delayedReactionTimer;
    SpeechId _delayedReactionSpeech = SPEECH_NONE;
    int _delayedReactionCharacterId = -1;

    std::array<bool, 5> playerAlreadyPicked = {{}};  // Was at offset 0xAE3368 in vanilla, we moved it into Party in OE.

    /**
     * @return                          0-based index of the active character. Asserts that there is one, so check
     *                                  `hasActiveCharacter()` first.
     */
    int activeCharacterIndex() const {
        assert(hasActiveCharacter());
        return _activeCharacterIndex;
    }

    /**
     * @param index                     0-based index of the character to make active. Pass `-1` to make no one active.
     */
    void setActiveCharacterIndex(int index) {
        assert(index >= -1 && index < std::ssize(pCharacters));
        _activeCharacterIndex = index;
    }
    bool hasActiveCharacter() const {
        return _activeCharacterIndex >= 0;
    }
    Character &activeCharacter() {
        assert(hasActiveCharacter());
        return pCharacters[_activeCharacterIndex];
    }

 private:
    int _activeCharacterIndex = -1;
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
