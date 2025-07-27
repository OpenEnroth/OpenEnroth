#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <utility>

#include "Inventory.h"
#include "Engine/Data/HouseEnums.h"
#include "Engine/Objects/NPCEnums.h"
#include "Engine/Objects/ActorEnums.h"
#include "Engine/Objects/CombinedSkillValue.h"
#include "Engine/Objects/Item.h"
#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Objects/MonsterEnums.h"
#include "Engine/Spells/SpellEnums.h"
#include "Engine/Spells/SpellBuff.h"
#include "Engine/Evt/EvtEnums.h"
#include "Engine/Pid.h"

#include "GUI/GUIEnums.h"

#include "Library/Color/Color.h"
#include "Library/Geometry/Vec.h"

#include "Utility/IndexedArray.h"
#include "Utility/IndexedBitset.h"

#include "TalkAnimation.h"
#include "CharacterConditions.h"

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

// HP/SP regeneration from items and spell
// TODO(pskelton): maybe expand so we can handle different strength enchantments
struct RegenData {
    int hpRegen = 0; // From all sources except for regeneration buff, hp / 5 ticks.
    int hpSpellRegen = 0; // From regeneration buff, hp / 5 ticks.
    int spRegen = 0; // From all sources, mp / 5 ticks.
};

class Character {
 public:
    Character();
    void Zero();

    bool matchesAttackPreference(MonsterAttackPreference preference) const;

    void SetVariable(EvtVariable var, signed int a3);
    void AddVariable(EvtVariable var, signed int val);
    void SubtractVariable(EvtVariable VarNum, signed int pValue);
    bool CompareVariable(EvtVariable VarNum, signed int pValue);

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
    int GetBaseStat(Attribute stat) const;

    int GetBaseLevel() const;
    int GetActualLevel() const;

    int GetActualMight() const;
    int GetActualIntelligence() const;
    int GetActualPersonality() const;
    int GetActualEndurance() const;
    int GetActualAccuracy() const;
    int GetActualSpeed() const;
    int GetActualLuck() const;
    int GetActualStat(Attribute stat) const;

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
    bool IsUnarmed() const;
    bool HasItemEquipped(ItemSlot uEquipIndex) const;
    bool HasEnchantedItemEquipped(ItemEnchantment uEnchantment) const;
    bool WearsItem(ItemId item_id, ItemSlot equip_type) const;
    int StealFromShop(Item *itemToSteal, int extraStealDifficulty,
                      int reputation, int extraStealFine, int *fineIfFailed);
    StealResult StealFromActor(unsigned int uActorID, int _steal_perm, int reputation);
    void Heal(int amount);

    /**
     * @offset 0x48DC1E
     */
    int receiveDamage(signed int amount, DamageType dmg_type);
    int ReceiveSpecialAttackEffect(MonsterSpecialAttack attType, Actor *pActor);

    Duration GetAttackRecoveryTime(bool attackUsesBow) const;

    int GetHealth() const { return this->health; }
    int GetMaxHealth() const;
    int GetMana() const { return this->mana; }
    int GetMaxMana() const;

    int GetBaseAC() const;
    int GetActualAC() const;
    unsigned int GetBaseAge() const;
    unsigned int GetActualAge() const;
    int GetBaseResistance(Attribute a2) const;
    int GetActualResistance(Attribute resistance) const;
    void SetRecoveryTime(Duration sRecoveryTime);
    void RandomizeName();
    Condition GetMajorConditionIdx() const;
    int GetParameterBonus(int character_parameter) const;
    int GetSpecialItemBonus(ItemEnchantment enchantment) const;
    int GetItemsBonus(Attribute attr, bool getOnlyMainHandDmg = false) const;
    int GetMagicalBonus(Attribute a2) const;
    int actualSkillLevel(Skill skill) const;
    CombinedSkillValue getActualSkillValue(Skill skill) const;
    int GetSkillBonus(Attribute a2) const;
    Race GetRace() const;
    std::string GetRaceName() const;
    Sex GetSexByVoice() const;
    void SetInitialStats();
    void SetSexByVoice();
    void ChangeClass(Class classType);
    Skill GetSkillIdxByOrder(signed int order);
    void DecreaseAttribute(Attribute eAttribute);
    void IncreaseAttribute(Attribute eAttribute);
    void resetTempBonuses();
    Color GetStatColor(Attribute uStat) const;
    bool DiscardConditionIfLastsLongerThan(Condition uCondition, Time time);
    MerchantPhrase SelectPhrasesTransaction(Item *pItem, HouseType building_type, HouseId houseId, ShopScreen ShopMenuType);
    int GetBodybuilding() const;
    int GetMeditation() const;
    bool CanIdentify(const Item &item) const;
    bool CanRepair(const Item &item) const;
    int GetPerception() const;
    int GetDisarmTrap() const;

    /**
     * Get percentage bonus for character EXP received.
     *
     * @offset 0x491317
     */
    int getLearningPercent() const;

    bool HasSkill(Skill skill) const;
    bool CanAct() const;
    bool CanSteal() const;
    bool CanEquip_RaceAndAlignmentCheck(ItemId uItemID) const;
    void SetCondition(Condition condition, int blockable);

    /**
     * @offset 0x49327B
     */
    bool isClass(Class class_type, bool check_honorary = true) const;

    /**
     * @offset 0x4948B1
     */
    void playReaction(SpeechId speech, int a3 = 0);

    /**
     * @offset 0x494A25
     */
    void playEmotion(PortraitId newPortrait, Duration duration);
    void ItemsPotionDmgBreak(int count);
    int GetConditionDaysPassed(Condition condition) const;
    bool NothingOrJustBlastersEquipped() const;
    void SalesProcess(InventoryEntry entry, HouseId houseId);  // 0x4BE2DD
    bool Recover(Duration dt);
    bool CanCastSpell(unsigned int uRequiredMana);
    void SpendMana(unsigned int uRequiredMana);
    void PlayAwardSound();
    void EquipBody(ItemType uEquipType);

    /**
     * @offset 0x43EE77
     */
    bool hasUnderwaterSuitEquipped();

    void OnInventoryLeftClick();

    bool characterHitOrMiss(Actor *pActor, int distancemod, int skillmod);

    unsigned int GetMultiplierForSkillLevel(Skill uSkillType, int mult1, int mult2, int mult3, int mult4) const;
    int CalculateMeleeDmgToEnemyWithWeapon(Item *weapon,
                                           MonsterId uTargetActorID,
                                           bool addOneDice);
    bool wearsItemAnywhere(ItemId item_id) const;
    float GetArmorRecoveryMultiplierFromSkillLevel(Skill armour_skill_type, float param2, float param3, float param4, float param5) const;
    void SetSkillReaction();
    void PlayAwardSound_Anim();
    void PlayAwardSound_Anim_Face(SpeechId speech);
    void PlayAwardSound_Anim97();
    void PlayAwardSound_Anim97_Face(SpeechId speech);
    void AddSkillByEvent(Skill, uint16_t addSkillValue);
    void PlayAwardSound_AnimSubtract();
    void PlayAwardSound_AnimSubtract_Face(SpeechId speech);
    void SubtractSkillByEvent(Skill skill, uint16_t subSkillValue);

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

    Item *GetMainHandItem();
    Item *GetOffHandItem();
    Item *GetBowItem();
    Item *GetArmorItem();
    Item *GetHelmItem();
    Item *GetBeltItem();
    Item *GetCloakItem();
    Item *GetGloveItem();
    Item *GetBootItem();
    Item *GetAmuletItem();
    Item *GetNthRingItem(int ringNum);
    Item *GetItem(ItemSlot index);

    const Item *GetMainHandItem() const;
    const Item *GetOffHandItem() const;
    const Item *GetBowItem() const;
    const Item *GetArmorItem() const;
    const Item *GetHelmItem() const;
    const Item *GetBeltItem() const;
    const Item *GetCloakItem() const;
    const Item *GetGloveItem() const;
    const Item *GetBootItem() const;
    const Item *GetAmuletItem() const;
    const Item *GetNthRingItem(int ringNum) const;
    const Item *GetItem(ItemSlot index) const;

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
    CombinedSkillValue getSkillValue(Skill skill) const;
    void setSkillValue(Skill skill, const CombinedSkillValue &value);

    void setXP(int xp);

    void tickRegeneration(int tick5, const RegenData &rData, bool stacking);

    CharacterConditions conditions;
    uint64_t experience;
    std::string name;
    Sex uSex;
    Class classType;
    uint8_t uCurrentFace;
    IndexedArray<int, ATTRIBUTE_FIRST_STAT, ATTRIBUTE_LAST_STAT> _stats;
    IndexedArray<int, ATTRIBUTE_FIRST_STAT, ATTRIBUTE_LAST_STAT> _statBonuses;
    int16_t sACModifier;
    uint16_t uLevel;
    int16_t sLevelModifier;
    int16_t sAgeModifier;
    IndexedArray<CombinedSkillValue, SKILL_FIRST, SKILL_LAST> pActiveSkills;
    IndexedBitset<1, 512> _achievedAwardsBits;
    IndexedArray<bool, SPELL_FIRST_REGULAR, SPELL_LAST_REGULAR> bHaveSpell;
    IndexedArray<bool, ATTRIBUTE_FIRST_STAT, ATTRIBUTE_LAST_STAT> _pureStatPotionUsed;
    CharacterInventory inventory;
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
    PortraitId portrait;
    Duration portraitTimePassed;
    Duration portraitTimeLength;
    int16_t portraitImageIndex;
    TalkAnimation talkAnimation;
    std::array<std::optional<LloydBeacon>, 5> vBeacons;
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
