#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <utility>

#include "Engine/Objects/NPCEnums.h"
#include "Engine/Objects/ActorEnums.h"
#include "Engine/Objects/CombinedSkillValue.h"
#include "Engine/Objects/Items.h"
#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Objects/MonsterEnums.h"
#include "Engine/Spells/SpellEnums.h"
#include "Engine/Spells/SpellBuff.h"
#include "Engine/Tables/BuildingTable.h"
#include "Engine/Events/EventEnums.h"
#include "Engine/Pid.h"

#include "GUI/GUIEnums.h"

#include "Library/Color/Color.h"
#include "Library/Geometry/Vec.h"
#include "Library/Snapshots/RawSnapshots.h"

#include "Utility/IndexedArray.h"
#include "Utility/IndexedBitset.h"

class Actor;
class GraphicsImage;

enum class StealResult {
    STEAL_BUSTED = 0, // Failed to steal & was caught.
    STEAL_NOTHING = 1, // Either failed to steal, or there was nothing to steal.
    STEAL_SUCCESS = 2, // Stolen successfully.
};
using enum StealResult;

struct LloydBeacon {
    ~LloydBeacon() {
        // if (image != nullptr) {
        //    image->Release();
        // }
        // image release moved to install beacon to avoid de-refernce
        image = nullptr;
    }

    Time uBeaconTime = Time();
    Vec3f _partyPos;
    int16_t _partyViewYaw = 0;
    int16_t _partyViewPitch = 0;
    uint16_t unknown = 0;
    MapId mapId = MAP_INVALID;
    GraphicsImage *image = nullptr;
};

struct RawCharacterConditions {
    /** Game time when condition has started. Zero means that the character doesn't have a condition. */
    IndexedArray<Time, CONDITION_FIRST, CONDITION_LAST> _times;
};

class CharacterConditions : private RawCharacterConditions {
    MM_DECLARE_RAW_PRIVATE_BASE(RawCharacterConditions)
 public: // NOLINT: no idea why linter is triggering here.
    [[nodiscard]] bool Has(Condition condition) const {
        return _times[condition].isValid();
    }

    [[nodiscard]] bool HasAny(std::initializer_list<Condition> conditions) const {
        for (Condition condition : conditions)
            if (Has(condition))
                return true;
        return false;
    }

    [[nodiscard]] bool HasNone(std::initializer_list<Condition> conditions) const {
        return !HasAny(conditions);
    }

    void Reset(Condition condition) {
        _times[condition] = Time();
    }

    void ResetAll() {
        for (Time &time : _times)
            time = Time();
    }

    void Set(Condition condition, Time time) {
        _times[condition] = time;
    }

    [[nodiscard]] Time Get(Condition condition) const {
        return _times[condition];
    }
};

class Character {
 public:
    static constexpr unsigned int INVENTORY_SLOTS_WIDTH = 14;
    static constexpr unsigned int INVENTORY_SLOTS_HEIGHT = 9;

    // Maximum number of items the character inventory can hold.
    static constexpr unsigned int INVENTORY_SLOT_COUNT = INVENTORY_SLOTS_WIDTH * INVENTORY_SLOTS_HEIGHT;

    Character();
    void Zero();

    bool matchesAttackPreference(MonsterAttackPreference preference) const;

    void SetVariable(VariableType var, signed int a3);
    void AddVariable(VariableType var, signed int val);
    void SubtractVariable(VariableType VarNum, signed int pValue);
    bool CompareVariable(VariableType VarNum, signed int pValue);

    /**
     * Use item on character.
     * This includes potion drinking, spell scroll/books usages and so on.
     *
     * @param targetCharacter     Character that uses the item.
     * @param isPortraitClick     true if item used on character portrait, false if on character doll in inventory screen.
     * @offset 0x4680ED
     */
    void useItem(int targetCharacter, bool isPortraitClick);

    int GetBaseMight() const;
    int GetBaseIntelligence() const;
    int GetBasePersonality() const;
    int GetBaseEndurance() const;
    int GetBaseAccuracy() const;
    int GetBaseSpeed() const;
    int GetBaseLuck() const;
    int GetBaseStat(CharacterAttributeType stat) const;

    int GetBaseLevel() const;
    int GetActualLevel() const;

    int GetActualMight() const;
    int GetActualIntelligence() const;
    int GetActualPersonality() const;
    int GetActualEndurance() const;
    int GetActualAccuracy() const;
    int GetActualSpeed() const;
    int GetActualLuck() const;
    int GetActualStat(CharacterAttributeType stat) const;

    int GetActualAttack(bool onlyMainHandDmg) const;
    int GetMeleeDamageMinimal() const;
    int GetMeleeDamageMaximal() const;
    int CalculateMeleeDamageTo(bool ignoreSkillBonus, bool ignoreOffhand,
                               MonsterId uTargetActorID);
    int GetRangedAttack();
    int GetRangedDamageMin();
    int GetRangedDamageMax();
    int CalculateRangedDamageTo(MonsterId uMonsterInfoID);
    std::string GetMeleeDamageString();
    std::string GetRangedDamageString();
    bool CanTrainToNextLevel();
    Color GetExperienceDisplayColor();
    int CalculateIncommingDamage(DamageType dmg_type, int amount);
    ItemType GetEquippedItemEquipType(ItemSlot uEquipSlot) const;
    CharacterSkillType GetEquippedItemSkillType(ItemSlot uEquipSlot) const;
    bool IsUnarmed() const;
    bool HasItemEquipped(ItemSlot uEquipIndex) const;
    bool HasEnchantedItemEquipped(ItemEnchantment uEnchantment) const;
    bool WearsItem(ItemId item_id, ItemSlot equip_type) const;
    int StealFromShop(ItemGen *itemToSteal, int extraStealDifficulty,
                      int reputation, int extraStealFine, int *fineIfFailed);
    StealResult StealFromActor(unsigned int uActorID, int _steal_perm, int reputation);
    void Heal(int amount);

    /**
     * @offset 0x48DC1E
     */
    int receiveDamage(signed int amount, DamageType dmg_type);
    int ReceiveSpecialAttackEffect(SpecialAttackType attType, Actor *pActor);

    // TODO(captainurist): move closer to Spells data.
    DamageType GetSpellDamageType(SpellId uSpellID) const;
    Duration GetAttackRecoveryTime(bool attackUsesBow) const;

    int GetHealth() const { return this->health; }
    int GetMaxHealth() const;
    int GetMana() const { return this->mana; }
    int GetMaxMana() const;

    int GetBaseAC() const;
    int GetActualAC() const;
    unsigned int GetBaseAge() const;
    unsigned int GetActualAge() const;
    int GetBaseResistance(CharacterAttributeType a2) const;
    int GetActualResistance(CharacterAttributeType resistance) const;
    void SetRecoveryTime(Duration sRecoveryTime);
    void RandomizeName();
    Condition GetMajorConditionIdx() const;
    int GetParameterBonus(int character_parameter) const;
    int GetSpecialItemBonus(ItemEnchantment enchantment) const;
    int GetItemsBonus(CharacterAttributeType attr, bool getOnlyMainHandDmg = false) const;
    int GetMagicalBonus(CharacterAttributeType a2) const;
    int actualSkillLevel(CharacterSkillType skill) const;
    CombinedSkillValue getActualSkillValue(CharacterSkillType skill) const;
    int GetSkillBonus(CharacterAttributeType a2) const;
    Race GetRace() const;
    std::string GetRaceName() const;
    CharacterSex GetSexByVoice() const;
    void SetInitialStats();
    void SetSexByVoice();
    void ChangeClass(CharacterClass classType);
    CharacterSkillType GetSkillIdxByOrder(signed int order);
    void DecreaseAttribute(CharacterAttributeType eAttribute);
    void IncreaseAttribute(CharacterAttributeType eAttribute);
    void resetTempBonuses();
    Color GetStatColor(CharacterAttributeType uStat) const;
    bool DiscardConditionIfLastsLongerThan(Condition uCondition, Time time);
    MerchantPhrase SelectPhrasesTransaction(ItemGen *pItem, BuildingType building_type, HouseId houseId, ShopScreen ShopMenuType);
    int GetBodybuilding() const;
    int GetMeditation() const;
    bool CanIdentify(ItemGen *pItem) const;
    bool CanRepair(ItemGen *pItem) const;
    int GetPerception() const;
    int GetDisarmTrap() const;

    /**
     * Get percentage bonus for character EXP received.
     *
     * @offset 0x491317
     */
    char getLearningPercent() const;

    /**
     * @offset 0x492528
     */
    bool canFitItem(unsigned int uSlot, ItemId uItemID) const;

    /**
     * @offset 0x4925E6
     */
    int findFreeInventoryListSlot() const;
    int CreateItemInInventory(unsigned int uSlot, ItemId uItemID);
    bool HasSkill(CharacterSkillType skill) const;
    void WearItem(ItemId uItemID);
    int AddItem(int uSlot, ItemId uItemID);
    int AddItem2(int uSlot, ItemGen *Src);
    int CreateItemInInventory2(unsigned int index, ItemGen *Src);
    void PutItemArInventoryIndex(ItemId uItemID, int itemListPos, int uSlot);
    void RemoveItemAtInventoryIndex(unsigned int uSlot);
    bool CanAct() const;
    bool CanSteal() const;
    bool CanEquip_RaceAndAlignmentCheck(ItemId uItemID) const;
    void SetCondition(Condition condition, int blockable);

    /**
     * @offset 0x49327B
     */
    bool isClass(CharacterClass class_type, bool check_honorary = true) const;

    /**
     * @offset 0x4948B1
     */
    void playReaction(CharacterSpeech speech, int a3 = 0);

    /**
     * @offset 0x494A25
     */
    void playEmotion(CharacterExpressionID expression, Duration duration);
    void ItemsPotionDmgBreak(int enchant_count);
    unsigned int GetItemListAtInventoryIndex(int inout_item_cell);
    unsigned int GetItemMainInventoryIndex(int inout_item_cell);
    ItemGen *GetItemAtInventoryIndex(int inout_item_cell);
    int GetConditionDaysPassed(Condition condition) const;
    bool NothingOrJustBlastersEquipped() const;
    void SalesProcess(unsigned int inventory_idnx, int item_index, HouseId houseId);  // 0x4BE2DD
    bool Recover(Duration dt);
    bool CanCastSpell(unsigned int uRequiredMana);
    void SpendMana(unsigned int uRequiredMana);
    void PlayAwardSound();
    void EquipBody(ItemType uEquipType);

    /**
     * @offset 0x43EE77
     */
    bool hasUnderwaterSuitEquipped();

    /**
     * @offset 0x43EE15
     */
    bool hasItem(ItemId uItemID, bool checkHeldItem);
    void OnInventoryLeftClick();

    bool characterHitOrMiss(Actor *pActor, int distancemod, int skillmod);

    unsigned int GetMultiplierForSkillLevel(CharacterSkillType uSkillType, int mult1, int mult2, int mult3, int mult4) const;
    int CalculateMeleeDmgToEnemyWithWeapon(ItemGen *weapon,
                                           MonsterId uTargetActorID,
                                           bool addOneDice);
    bool wearsItemAnywhere(ItemId item_id) const;
    float GetArmorRecoveryMultiplierFromSkillLevel(CharacterSkillType armour_skill_type, float param2, float param3, float param4, float param5) const;
    void SetSkillReaction();
    void PlayAwardSound_Anim();
    void PlayAwardSound_Anim_Face(CharacterSpeech speech);
    void PlayAwardSound_Anim97();
    void PlayAwardSound_Anim97_Face(CharacterSpeech speech);
    void AddSkillByEvent(CharacterSkillType, uint16_t addSkillValue);
    void PlayAwardSound_AnimSubtract();
    void PlayAwardSound_AnimSubtract_Face(CharacterSpeech speech);
    void SubtractSkillByEvent(CharacterSkillType skill, uint16_t subSkillValue);

    bool IsWeak() const;
    bool IsDead() const;
    bool IsEradicated() const;
    bool IsZombie() const;
    bool IsCursed() const;
    bool IsPetrified() const;
    bool IsUnconcious() const;
    bool IsAsleep() const;
    bool IsParalyzed() const;
    bool IsDrunk() const;

    void SetCondWeakWithBlockCheck(int blockable);
    void SetCondInsaneWithBlockCheck(int blockable);
    void SetCondDeadWithBlockCheck(int blockable);
    void SetCondUnconsciousWithBlockCheck(int blockable);

    inline bool IsRaceHuman() const { return GetRace() == RACE_HUMAN; }
    inline bool IsRaceDwarf() const { return GetRace() == RACE_DWARF; }
    inline bool IsRaceElf() const { return GetRace() == RACE_ELF; }
    inline bool IsRaceGoblin() const { return GetRace() == RACE_GOBLIN; }

    inline bool IsMale() const { return GetSexByVoice() == SEX_MALE; }
    inline bool IsFemale() const { return !IsMale(); }

    ItemGen *GetMainHandItem();
    ItemGen *GetOffHandItem();
    ItemGen *GetBowItem();
    ItemGen *GetArmorItem();
    ItemGen *GetHelmItem();
    ItemGen *GetBeltItem();
    ItemGen *GetCloakItem();
    ItemGen *GetGloveItem();
    ItemGen *GetBootItem();
    ItemGen *GetAmuletItem();
    ItemGen *GetNthRingItem(int ringNum);
    ItemGen *GetItem(ItemSlot index);

    const ItemGen *GetMainHandItem() const;
    const ItemGen *GetOffHandItem() const;
    const ItemGen *GetBowItem() const;
    const ItemGen *GetArmorItem() const;
    const ItemGen *GetHelmItem() const;
    const ItemGen *GetBeltItem() const;
    const ItemGen *GetCloakItem() const;
    const ItemGen *GetGloveItem() const;
    const ItemGen *GetBootItem() const;
    const ItemGen *GetAmuletItem() const;
    const ItemGen *GetNthRingItem(int ringNum) const;
    const ItemGen *GetItem(ItemSlot index) const;

    // TODO(Nik-RE-dev): use getCharacterIdInParty directly where this function is called.
    /**
     * @return                          0-based index of this character in the party.
     */
    int getCharacterIndex();

    static void _42ECB5_CharacterAttacksActor();
    static void _42FA66_do_explosive_impact(Vec3f pos, int a4, int16_t a5, int actchar);
    void cleanupBeacons();
    bool setBeacon(int index, Duration duration);

    // TODO(captainurist): check all usages, most should be using getActualSkillValue.
    CombinedSkillValue getSkillValue(CharacterSkillType skill) const;
    void setSkillValue(CharacterSkillType skill, const CombinedSkillValue &value);

    void setXP(int xp);

    CharacterConditions conditions;
    uint64_t experience;
    std::string name;
    CharacterSex uSex;
    CharacterClass classType;
    uint8_t uCurrentFace;
    IndexedArray<int, CHARACTER_ATTRIBUTE_FIRST_STAT, CHARACTER_ATTRIBUTE_LAST_STAT> _stats;
    IndexedArray<int, CHARACTER_ATTRIBUTE_FIRST_STAT, CHARACTER_ATTRIBUTE_LAST_STAT> _statBonuses;
    int16_t sACModifier;
    uint16_t uLevel;
    int16_t sLevelModifier;
    int16_t sAgeModifier;
    IndexedArray<CombinedSkillValue, CHARACTER_SKILL_FIRST, CHARACTER_SKILL_LAST> pActiveSkills;
    IndexedBitset<1, 512> _achievedAwardsBits;
    IndexedArray<bool, SPELL_FIRST_REGULAR, SPELL_LAST_REGULAR> bHaveSpell;
    IndexedArray<bool, CHARACTER_ATTRIBUTE_FIRST_STAT, CHARACTER_ATTRIBUTE_LAST_STAT> _pureStatPotionUsed;
    std::array<ItemGen, INVENTORY_SLOT_COUNT> pInventoryItemList;
    std::array<int, INVENTORY_SLOT_COUNT> pInventoryMatrix; // 0 => empty cell
                                                            // positive => subtract 1 to get an index into pInventoryItemList.
                                                            // negative => negate & subtract 1 to get a real index into pInventoryMatrix.
    int16_t sResFireBase;
    int16_t sResAirBase;
    int16_t sResWaterBase;
    int16_t sResEarthBase;
    int16_t sResPhysicalBase;
    int16_t sResMagicBase;
    int16_t sResSpiritBase;
    int16_t sResMindBase;
    int16_t sResBodyBase;
    int16_t sResLightBase;
    int16_t sResDarkBase;
    int16_t sResFireBonus;
    int16_t sResAirBonus;
    int16_t sResWaterBonus;
    int16_t sResEarthBonus;
    int16_t sResPhysicalBonus;
    int16_t sResMagicBonus;
    int16_t sResSpiritBonus;
    int16_t sResMindBonus;
    int16_t sResBodyBonus;
    int16_t sResLightBonus;
    int16_t sResDarkBonus;
    IndexedArray<SpellBuff, CHARACTER_BUFF_FIRST, CHARACTER_BUFF_LAST> pCharacterBuffs;
    unsigned int uVoiceID;
    int uPrevVoiceID;
    int uPrevFace;
    Duration timeToRecovery;
    unsigned int uSkillPoints;
    int health;
    int mana;
    unsigned int uBirthYear;
    IndexedArray<unsigned int, ITEM_SLOT_FIRST_VALID, ITEM_SLOT_LAST_VALID> pEquipment; // 0 => empty,
                                                                                        // non-zero => subtract 1 to get an index into pInventoryItemList.
    MagicSchool lastOpenedSpellbookPage;
    SpellId uQuickSpell;
    IndexedBitset<1, 512> _characterEventBits;
    char _some_attack_bonus;
    char _melee_dmg_bonus;
    char _ranged_atk_bonus;
    char _ranged_dmg_bonus;
    char uFullHealthBonus;
    char _health_related;
    char uFullManaBonus;
    char _mana_related;
    CharacterExpressionID expression;
    Duration uExpressionTimePassed;
    Duration uExpressionTimeLength;
    int16_t uExpressionImageIndex;
    Duration _expression21_animtime;
    int _expression21_frameset;
    std::vector<LloydBeacon> vBeacons;
    char uNumDivineInterventionCastsThisDay;
    char uNumArmageddonCasts;
    char uNumFireSpikeCasts;
};

void DamageCharacterFromMonster(Pid uObjID, ActorAbility dmgSource, signed int a4);
bool IsDwarfPresentInParty(bool b);
bool ShouldLoadTexturesForRaceAndGender(unsigned int _this);
int CharacterCreation_GetUnspentAttributePointCount();

/**
 * @offset 0x49387A
 */
int cycleCharacter(bool backwards);

