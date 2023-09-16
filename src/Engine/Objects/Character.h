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
#include "Engine/Spells/SpellEnums.h"
#include "Engine/Spells/SpellBuff.h"
#include "Engine/Tables/BuildingTable.h"
#include "Engine/Events/EventEnums.h"
#include "Engine/ErrorHandling.h"
#include "Engine/Pid.h"

#include "Library/Color/Color.h"

#include "Utility/Geometry/Vec.h"
#include "Utility/IndexedArray.h"
#include "Utility/IndexedBitset.h"

class Actor;
class GraphicsImage;

struct LloydBeacon {
    ~LloydBeacon() {
        // if (image != nullptr) {
        //    image->Release();
        // }
        // image release moved to install beacon to avoid de-refernce
        image = nullptr;
    }

    GameTime uBeaconTime = GameTime(0);
    Vec3i _partyPos = Vec3i(0, 0, 0);
    int16_t _partyViewYaw = 0;
    int16_t _partyViewPitch = 0;
    uint16_t unknown = 0;
    MapId mapId = MAP_INVALID;
    GraphicsImage *image = nullptr;
};

struct CharacterSpellbookChapter {
    std::array<char, 11> bIsSpellAvailable;
};

struct CharacterSpells {
    union {
        struct {
            CharacterSpellbookChapter pFireSpellbook;
            CharacterSpellbookChapter pAirSpellbook;
            CharacterSpellbookChapter pWaterSpellbook;
            CharacterSpellbookChapter pEarthSpellbook;
            CharacterSpellbookChapter pSpiritSpellbook;
            CharacterSpellbookChapter pMindSpellbook;
            CharacterSpellbookChapter pBodySpellbook;
            CharacterSpellbookChapter pLightSpellbook;
            CharacterSpellbookChapter pDarkSpellbook;
            char _pad_0;
        };
        struct {
            std::array<CharacterSpellbookChapter, 9> pChapters;
            char _pad_1;
        };
        struct {
            std::array<char, 99> bHaveSpell;
            char _pad_2;
        };
    };
};

union CharacterEquipment {
    union {
        struct {
            unsigned int uOffHand;
            unsigned int uMainHand;
            unsigned int uBow;
            unsigned int uArmor;
            unsigned int uHelm;
            unsigned int uBelt;
            unsigned int uCloak;
            unsigned int uGlove;
            unsigned int uBoot;
            unsigned int uAmulet;
            std::array<unsigned int, 6> uRings;
            // unsigned int field_2C;
            // unsigned int field_30;
            // unsigned int field_34;
            // unsigned int field_38;
            // unsigned int field_3C;
        };
        IndexedArray<unsigned int, ITEM_SLOT_FIRST_VALID, ITEM_SLOT_LAST_VALID> pIndices;
    };

    CharacterEquipment() : pIndices() {}
};


// TODO(captainurist): ENUM!
#define STEAL_BUSTED   0
#define STEAL_NOTHING  1
#define STEAL_SUCCESS  2


class CharacterConditions {
 public:
    [[nodiscard]] bool Has(Condition condition) const {
        return this->times_[std::to_underlying(condition)].Valid();
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
        this->times_[std::to_underlying(condition)].Reset();
    }

    void ResetAll() {
        for(size_t i = 0; i < times_.size(); i++)
            times_[i].Reset();
    }

    void Set(Condition condition, GameTime time) {
        this->times_[std::to_underlying(condition)] = time;
    }

    [[nodiscard]] GameTime Get(Condition condition) const {
        return this->times_[std::to_underlying(condition)];
    }

 private:
    /** Game time when condition has started. */
    std::array<GameTime, 20> times_;
};

class Character {
 public:
    static constexpr unsigned int INVENTORY_SLOTS_WIDTH = 14;
    static constexpr unsigned int INVENTORY_SLOTS_HEIGHT = 9;

    // Maximum number of items the character inventory can hold
    static constexpr unsigned int INVENTORY_SLOT_COUNT = INVENTORY_SLOTS_WIDTH*INVENTORY_SLOTS_HEIGHT;
    static constexpr unsigned int ADDITIONAL_SLOT_COUNT = 12; // TODO: investigate, these look unused
    static constexpr unsigned int TOTAL_ITEM_SLOT_COUNT = INVENTORY_SLOT_COUNT + ADDITIONAL_SLOT_COUNT;

    Character();

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
    bool AddItem(ItemGen *pItem);
    int GetActualAttribute(CharacterAttributeType attrId,
                           unsigned short Character::*attrValue,
                           unsigned short Character::*attrBonus) const;
    int GetBaseMight() const;
    int GetBaseIntelligence() const;
    int GetBasePersonality() const;
    int GetBaseEndurance() const;
    int GetBaseAccuracy() const;
    int GetBaseSpeed() const;
    int GetBaseLuck() const;
    int GetBaseLevel() const;
    int GetActualLevel() const;
    int GetActualMight() const;
    int GetActualIntelligence() const;
    int GetActualPersonality() const;
    int GetActualEndurance() const;
    int GetActualAccuracy() const;
    int GetActualSpeed() const;
    int GetActualLuck() const;
    int GetActualAttack(bool onlyMainHandDmg) const;
    int GetMeleeDamageMinimal() const;
    int GetMeleeDamageMaximal() const;
    int CalculateMeleeDamageTo(bool ignoreSkillBonus, bool ignoreOffhand,
                               unsigned int uTargetActorID);
    int GetRangedAttack();
    int GetRangedDamageMin();
    int GetRangedDamageMax();
    int CalculateRangedDamageTo(int uMonsterInfoID);
    std::string GetMeleeDamageString();
    std::string GetRangedDamageString();
    bool CanTrainToNextLevel();
    Color GetExperienceDisplayColor();
    int CalculateIncommingDamage(DAMAGE_TYPE dmg_type, int amount);
    ITEM_EQUIP_TYPE GetEquippedItemEquipType(ItemSlot uEquipSlot) const;
    CharacterSkillType GetEquippedItemSkillType(ItemSlot uEquipSlot) const;
    bool IsUnarmed() const;
    bool HasItemEquipped(ItemSlot uEquipIndex) const;
    bool HasEnchantedItemEquipped(ITEM_ENCHANTMENT uEnchantment) const;
    bool WearsItem(ItemId item_id, ItemSlot equip_type) const;
    int StealFromShop(ItemGen *itemToSteal, int extraStealDifficulty,
                      int reputation, int extraStealFine, int *fineIfFailed);
    int StealFromActor(unsigned int uActorID, int _steal_perm, int reputation);
    void Heal(int amount);

    /**
     * @offset 0x48DC1E
     */
    int receiveDamage(signed int amount, DAMAGE_TYPE dmg_type);
    int ReceiveSpecialAttackEffect(int attType, Actor *pActor);

    // TODO(captainurist): move closer to Spells data.
    DAMAGE_TYPE GetSpellDamageType(SPELL_TYPE uSpellID) const;
    int GetAttackRecoveryTime(bool bRangedAttack) const;

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
    void SetRecoveryTime(signed int sRecoveryTime);
    void RandomizeName();
    Condition GetMajorConditionIdx() const;
    int GetParameterBonus(int character_parameter) const;
    int GetSpecialItemBonus(ITEM_ENCHANTMENT enchantment) const;
    int GetItemsBonus(CharacterAttributeType attr, bool getOnlyMainHandDmg = false) const;
    int GetMagicalBonus(CharacterAttributeType a2) const;
    int actualSkillLevel(CharacterSkillType skill) const;
    CombinedSkillValue getActualSkillValue(CharacterSkillType skill) const;
    int GetSkillBonus(CharacterAttributeType a2) const;
    CharacterRace GetRace() const;
    std::string GetRaceName() const;
    CharacterSex GetSexByVoice() const;
    void SetInitialStats();
    void SetSexByVoice();
    void Reset(CharacterClassType classType);
    CharacterSkillType GetSkillIdxByOrder(signed int order);
    void DecreaseAttribute(CharacterAttributeType eAttribute);
    void IncreaseAttribute(CharacterAttributeType eAttribute);
    void resetTempBonuses();
    Color GetStatColor(CharacterAttributeType uStat) const;
    bool DiscardConditionIfLastsLongerThan(Condition uCondition, GameTime time);
    MerchantPhrase SelectPhrasesTransaction(ItemGen *pItem, BuildingType building_type, HOUSE_ID houseId, int ShopMenuType);
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
    int HasSkill(CharacterSkillType skill) const;
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
    bool isClass(CharacterClassType class_type, bool check_honorary = true) const;

    /**
     * @offset 0x4948B1
     */
    void playReaction(CharacterSpeech speech, int a3 = 0);

    /**
     * @offset 0x494A25
     */
    void playEmotion(CharacterExpressionID expression, int duration);
    void ItemsPotionDmgBreak(int enchant_count);
    unsigned int GetItemListAtInventoryIndex(int inout_item_cell);
    unsigned int GetItemMainInventoryIndex(int inout_item_cell);
    struct ItemGen *GetItemAtInventoryIndex(int inout_item_cell);
    int GetConditionDaysPassed(Condition condition) const;
    bool NothingOrJustBlastersEquipped() const;
    void SalesProcess(unsigned int inventory_idnx, int item_index, HOUSE_ID houseId);  // 0x4BE2DD
    bool Recover(GameTime dt);
    bool CanCastSpell(unsigned int uRequiredMana);
    void SpendMana(unsigned int uRequiredMana);
    void PlayAwardSound();
    void EquipBody(ITEM_EQUIP_TYPE uEquipType);

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
                                           unsigned int uTargetActorID,
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
    bool IsPertified() const;
    bool IsUnconcious() const;
    bool IsAsleep() const;
    bool IsParalyzed() const;
    bool IsDrunk() const;

    void SetCursed(GameTime time);
    void SetWeak(GameTime time);
    void SetAsleep(GameTime time);
    void SetAfraid(GameTime time);
    void SetDrunk(GameTime time);
    void SetInsane(GameTime time);
    void SetPoisonWeak(GameTime time);
    void SetDiseaseWeak(GameTime time);
    void SetPoisonMedium(GameTime time);
    void SetDiseaseMedium(GameTime time);
    void SetPoisonSevere(GameTime time);
    void SetDiseaseSevere(GameTime time);
    void SetParalyzed(GameTime time);
    void SetUnconcious(GameTime time);
    void SetDead(GameTime time);
    void SetPertified(GameTime time);
    void SetEradicated(GameTime time);
    void SetZombie(GameTime time);

    void SetCondWeakWithBlockCheck(int blockable);
    void SetCondInsaneWithBlockCheck(int blockable);
    void SetCondDeadWithBlockCheck(int blockable);
    void SetCondUnconsciousWithBlockCheck(int blockable);

    inline bool IsRaceHuman() const { return GetRace() == CHARACTER_RACE_HUMAN; }
    inline bool IsRaceDwarf() const { return GetRace() == CHARACTER_RACE_DWARF; }
    inline bool IsRaceElf() const { return GetRace() == CHARACTER_RACE_ELF; }
    inline bool IsRaceGoblin() const { return GetRace() == CHARACTER_RACE_GOBLIN; }

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
    ItemGen *GetNthEquippedIndexItem(ItemSlot index);
    ItemGen *GetItem(unsigned int CharacterEquipment::*itemPos);

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
    const ItemGen *GetNthEquippedIndexItem(ItemSlot index) const;
    const ItemGen *GetItem(unsigned int CharacterEquipment::*itemPos) const;

    // TODO(Nik-RE-dev): use getCharacterIdInParty directly where this function is called.
    int getCharacterIndex();

    static void _42ECB5_CharacterAttacksActor();
    static void _42FA66_do_explosive_impact(Vec3i pos, int a4, int16_t a5, signed int actchar);
    void cleanupBeacons();
    bool setBeacon(int index, GameTime duration);

    // TODO(captainurist): check all usages, most should be using getActualSkillValue.
    CombinedSkillValue getSkillValue(CharacterSkillType skill) const;
    void setSkillValue(CharacterSkillType skill, const CombinedSkillValue &value);

    CharacterConditions conditions;
    uint64_t experience;
    std::string name;
    CharacterSex uSex;
    CharacterClassType classType;
    uint8_t uCurrentFace;
    uint16_t uMight;
    uint16_t uMightBonus;
    uint16_t uIntelligence;
    uint16_t uIntelligenceBonus;
    uint16_t uPersonality;
    uint16_t uPersonalityBonus;
    uint16_t uEndurance;
    uint16_t uEnduranceBonus;
    uint16_t uSpeed;
    uint16_t uSpeedBonus;
    uint16_t uAccuracy;
    uint16_t uAccuracyBonus;
    uint16_t uLuck;
    uint16_t uLuckBonus;
    int16_t sACModifier;
    uint16_t uLevel;
    int16_t sLevelModifier;
    int16_t sAgeModifier;
    int field_E0;
    int field_E4;
    int field_E8;
    int field_EC;
    int field_F0;
    int field_F4;
    int field_F8;
    int field_FC;
    int field_100;
    int field_104;
    IndexedArray<CombinedSkillValue, CHARACTER_SKILL_FIRST, CHARACTER_SKILL_LAST> pActiveSkills;
    IndexedBitset<1, 512> _achievedAwardsBits;
    CharacterSpells spellbook;
    int pure_luck_used;
    int pure_speed_used;
    int pure_intellect_used;
    int pure_endurance_used;
    int pure_personality_used;
    int pure_accuracy_used;
    int pure_might_used;
    union {  // 214h
        struct {
            // TODO(captainurist): gcc doesn't let us turn these into std::array. And we probably need to drop the 2nd
            //                     one anyway. Investigate.
            ItemGen pInventoryItemList[INVENTORY_SLOT_COUNT];
            ItemGen pEquippedItems[ADDITIONAL_SLOT_COUNT];
        };
        std::array<ItemGen, TOTAL_ITEM_SLOT_COUNT> pOwnItems;
    };

    std::array<int, INVENTORY_SLOT_COUNT> pInventoryMatrix;
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
    uint16_t timeToRecovery;
    unsigned int uSkillPoints;
    int health;
    int mana;
    unsigned int uBirthYear;
    CharacterEquipment pEquipment;
    char lastOpenedSpellbookPage;
    SPELL_TYPE uQuickSpell;
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
    uint16_t uExpressionTimePassed;
    uint16_t uExpressionTimeLength;
    int16_t uExpressionImageIndex;
    int _expression21_animtime;
    int _expression21_frameset;
    std::vector<LloydBeacon> vBeacons;
    char uNumDivineInterventionCastsThisDay;
    char uNumArmageddonCasts;
    char uNumFireSpikeCasts;
};

inline CharacterExpressionID expressionForCondition(Condition condition) {
    switch (condition) {
      case CONDITION_DEAD:
        return CHARACTER_EXPRESSION_DEAD;
      case CONDITION_PETRIFIED:
        return CHARACTER_EXPRESSION_PETRIFIED;
      case CONDITION_ERADICATED:
        return CHARACTER_EXPRESSION_ERADICATED;
      case CONDITION_CURSED:
        return CHARACTER_EXPRESSION_CURSED;
      case CONDITION_WEAK:
        return CHARACTER_EXPRESSION_WEAK;
      case CONDITION_SLEEP:
        return CHARACTER_EXPRESSION_SLEEP;
      case CONDITION_FEAR:
        return CHARACTER_EXPRESSION_FEAR;
      case CONDITION_DRUNK:
        return CHARACTER_EXPRESSION_DRUNK;
      case CONDITION_INSANE:
        return CHARACTER_EXPRESSION_INSANE;
      case CONDITION_POISON_WEAK:
      case CONDITION_POISON_MEDIUM:
      case CONDITION_POISON_SEVERE:
        return CHARACTER_EXPRESSION_POISONED;
      case CONDITION_DISEASE_WEAK:
      case CONDITION_DISEASE_MEDIUM:
      case CONDITION_DISEASE_SEVERE:
        return CHARACTER_EXPRESSION_DISEASED;
      case CONDITION_PARALYZED:
        return CHARACTER_EXPRESSION_PARALYZED;
      case CONDITION_UNCONSCIOUS:
        return CHARACTER_EXPRESSION_UNCONCIOUS;
      default:
        Error("Invalid condition: %u", condition);
    }

    return CHARACTER_EXPRESSION_NORMAL;
}

void DamageCharacterFromMonster(Pid uObjID, ABILITY_INDEX dmgSource, Vec3i *pPos, signed int a4);
bool IsDwarfPresentInParty(bool b);
bool ShouldLoadTexturesForRaceAndGender(unsigned int _this);
int CharacterCreation_GetUnspentAttributePointCount();

/**
 * @offset 0x49387A
 */
int cycleCharacter(bool backwards);

