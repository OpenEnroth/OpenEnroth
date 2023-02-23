#pragma once

#include <vector>
#include <string>
#include <utility>

#include "Engine/Engine.h"
#include "Engine/Events.h"
#include "Engine/Events2D.h"
#include "Engine/Conditions.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Items.h"
#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/PlayerEnums.h"
#include "Engine/Spells/Spells.h"

#include "Utility/IndexedArray.h"

struct LloydBeacon {
    LloydBeacon() {
        uBeaconTime = GameTime(0);
        PartyPos_X = 0;
        PartyPos_Y = 0;
        PartyPos_Z = 0;
        PartyRot_X = 0;
        PartyRot_Y = 0;
        unknown = 0;
        SaveFileID = 0;
        image = nullptr;
    }

    ~LloydBeacon() {
        // if (image != nullptr) {
        //    image->Release();
        // }
        // image release moved to install beacon to avoid de-refernce
        image = nullptr;
    }

    GameTime uBeaconTime;
    int32_t PartyPos_X;
    int32_t PartyPos_Y;
    int32_t PartyPos_Z;
    int16_t PartyRot_X;
    int16_t PartyRot_Y;
    uint16_t unknown;
    uint16_t SaveFileID;
    Image *image;
};

#pragma pack(push, 1)
struct PlayerSpellbookChapter {
    std::array<char, 11> bIsSpellAvailable;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PlayerSpells {
    union {
        struct {
            PlayerSpellbookChapter pFireSpellbook;
            PlayerSpellbookChapter pAirSpellbook;
            PlayerSpellbookChapter pWaterSpellbook;
            PlayerSpellbookChapter pEarthSpellbook;
            PlayerSpellbookChapter pSpiritSpellbook;
            PlayerSpellbookChapter pMindSpellbook;
            PlayerSpellbookChapter pBodySpellbook;
            PlayerSpellbookChapter pLightSpellbook;
            PlayerSpellbookChapter pDarkSpellbook;
            char _pad_0;
        };
        struct {
            std::array<PlayerSpellbookChapter, 9> pChapters;
            char _pad_1;
        };
        struct {
            std::array<char, 99> bHaveSpell;
            char _pad_2;
        };
    };
};
#pragma pack(pop)

#pragma pack(push, 1)
union PlayerEquipment {
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

    PlayerEquipment() : pIndices() {}
};
#pragma pack(pop)


// TODO(captainurist): ENUM!
#define STEAL_BUSTED   0
#define STEAL_NOTHING  1
#define STEAL_SUCCESS  2


class PlayerConditions {
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


struct Player {
    static constexpr unsigned int INVENTORY_SLOTS_WIDTH = 14;
    static constexpr unsigned int INVENTORY_SLOTS_HEIGHT = 9;

    // Maximum number of items the player inventory can hold
    static constexpr unsigned int INVENTORY_SLOT_COUNT = INVENTORY_SLOTS_WIDTH*INVENTORY_SLOTS_HEIGHT;
    static constexpr unsigned int ADDITIONAL_SLOT_COUNT = 12; // TODO: investigate, these look unused
    static constexpr unsigned int TOTAL_ITEM_SLOT_COUNT = INVENTORY_SLOT_COUNT + ADDITIONAL_SLOT_COUNT;

    Player();

    void SetVariable(VariableType var, signed int a3);
    void AddVariable(VariableType var, signed int val);
    void SubtractVariable(VariableType VarNum, signed int pValue);
    bool CompareVariable(VariableType VarNum, signed int pValue);
    void UseItem_DrinkPotion_etc(signed int a2, int a3);
    bool AddItem(ItemGen* pItem);
    int GetActualAttribute(CHARACTER_ATTRIBUTE_TYPE attrId,
                           unsigned short Player::*attrValue,
                           unsigned short Player::*attrBonus);
    int GetBaseStrength();
    int GetBaseIntelligence();
    int GetBaseWillpower();
    int GetBaseEndurance();
    int GetBaseAccuracy();
    int GetBaseSpeed();
    int GetBaseLuck();
    int GetBaseLevel();
    int GetActualLevel();
    int GetActualMight();
    int GetActualIntelligence();
    int GetActualWillpower();
    int GetActualEndurance();
    int GetActualAccuracy();
    int GetActualSpeed();
    int GetActualLuck();
    int GetActualAttack(bool onlyMainHandDmg);
    int GetMeleeDamageMinimal();
    int GetMeleeDamageMaximal();
    int CalculateMeleeDamageTo(bool ignoreSkillBonus, bool ignoreOffhand,
                               unsigned int uTargetActorID);
    int GetRangedAttack();
    int GetRangedDamageMin();
    int GetRangedDamageMax();
    int CalculateRangedDamageTo(int uMonsterInfoID);
    std::string GetMeleeDamageString();
    std::string GetRangedDamageString();
    bool CanTrainToNextLevel();
    unsigned int GetExperienceDisplayColor();
    int CalculateIncommingDamage(DAMAGE_TYPE dmg_type, int amount);
    ITEM_EQUIP_TYPE GetEquippedItemEquipType(ITEM_SLOT uEquipSlot);
    PLAYER_SKILL_TYPE GetEquippedItemSkillType(ITEM_SLOT uEquipSlot);
    bool IsUnarmed();
    bool HasItemEquipped(ITEM_SLOT uEquipIndex) const;
    bool HasEnchantedItemEquipped(int uEnchantment);
    bool WearsItem(ITEM_TYPE item_id, ITEM_SLOT equip_type) const;
    int StealFromShop(ItemGen* itemToSteal, int extraStealDifficulty,
                      int reputation, int extraStealFine, int* fineIfFailed);
    int StealFromActor(unsigned int uActorID, int _steal_perm, int reputation);
    void Heal(int amount);
    int ReceiveDamage(signed int amount, DAMAGE_TYPE dmg_type);
    int ReceiveSpecialAttackEffect(int attType, Actor* pActor);
    unsigned int GetSpellSchool(SPELL_TYPE uSpellID);
    int GetAttackRecoveryTime(bool bRangedAttack);

    int GetHealth() const { return this->sHealth; }
    int GetMaxHealth();
    int GetMana() const { return this->sMana; }
    int GetMaxMana();

    int GetBaseAC();
    int GetActualAC();
    unsigned int GetBaseAge();
    unsigned int GetActualAge();
    int GetBaseResistance(CHARACTER_ATTRIBUTE_TYPE a2);
    int GetActualResistance(CHARACTER_ATTRIBUTE_TYPE a2);
    void SetRecoveryTime(signed int sRecoveryTime);
    void RandomizeName();
    Condition GetMajorConditionIdx();
    int GetParameterBonus(int player_parameter);
    int GetSpecialItemBonus(ITEM_ENCHANTMENT enchantment);
    int GetItemsBonus(CHARACTER_ATTRIBUTE_TYPE attr, bool a3 = false);
    int GetMagicalBonus(CHARACTER_ATTRIBUTE_TYPE a2);
    PLAYER_SKILL_LEVEL GetActualSkillLevel(PLAYER_SKILL_TYPE uSkillType);
    PLAYER_SKILL_MASTERY GetActualSkillMastery(PLAYER_SKILL_TYPE uSkillType);
    int GetSkillBonus(CHARACTER_ATTRIBUTE_TYPE a2);
    CHARACTER_RACE GetRace() const;
    std::string GetRaceName() const;
    PLAYER_SEX GetSexByVoice();
    void SetInitialStats();
    void SetSexByVoice();
    void Reset(PLAYER_CLASS_TYPE classType);
    PLAYER_SKILL_TYPE GetSkillIdxByOrder(signed int order);
    void DecreaseAttribute(int eAttribute);
    void IncreaseAttribute(int eAttribute);
    void Zero();
    unsigned int GetStatColor(int uStat);
    bool DiscardConditionIfLastsLongerThan(Condition uCondition,
                                           GameTime time);
    MERCHANT_PHRASE SelectPhrasesTransaction(ItemGen* pItem, BuildingType building_type, int BuildID_2Events, int ShopMenuType);
    int GetBodybuilding();
    int GetMeditation();
    bool CanIdentify(ItemGen* pItem);
    bool CanRepair(ItemGen* pItem);
    int GetMerchant();
    int GetPerception();
    int GetDisarmTrap();
    char GetLearningPercent();
    bool CanFitItem(unsigned int uSlot, ITEM_TYPE uItemID);
    int FindFreeInventoryListSlot();
    int CreateItemInInventory(unsigned int uSlot, ITEM_TYPE uItemID);
    int HasSkill(PLAYER_SKILL_TYPE uSkillType);
    void WearItem(ITEM_TYPE uItemID);
    int AddItem(int uSlot, ITEM_TYPE uItemID);
    int AddItem2(int uSlot, ItemGen* Src);
    int CreateItemInInventory2(unsigned int index, ItemGen* Src);
    void PutItemArInventoryIndex(ITEM_TYPE uItemID, int itemListPos, int uSlot);
    void RemoveItemAtInventoryIndex(unsigned int uSlot);
    bool CanAct();
    bool CanSteal();
    bool CanEquip_RaceAndAlignmentCheck(ITEM_TYPE uItemID);
    void SetCondition(Condition uConditionIdx, int blockable);
    bool IsClass(PLAYER_CLASS_TYPE class_type, bool check_honorary = true);
    void PlaySound(PlayerSpeech speech, int a3);
    void PlayEmotion(CHARACTER_EXPRESSION_ID expression, int duration);
    void ItemsPotionDmgBreak(int enchant_count);
    unsigned int GetItemListAtInventoryIndex(int inout_item_cell);
    unsigned int GetItemMainInventoryIndex(int inout_item_cell);
    struct ItemGen* GetItemAtInventoryIndex(int inout_item_cell);
    bool IsPlayerHealableByTemple();
    int GetBaseIdentifyPrice(float price_multiplier);
    int GetBaseRepairPrice(int uRealValue, float price_multiplier);
    int GetBaseBuyingPrice(int uRealValue, float price_multiplier);
    int GetBaseSellingPrice(int uRealValue, float price_multiplier);
    int GetPriceRepair(int uRealValue, float price_multiplier);
    int GetPriceIdentification(float price_multiplier);
    int GetBuyingPrice(unsigned int uRealValue, float price_multiplier);
    int GetPriceSell(ItemGen itemx, float price_multiplier);
    int GetTempleHealCostModifier(float price_multi);
    int GetConditionDaysPassed(Condition uCondition);
    bool NothingOrJustBlastersEquipped();
    void SalesProcess(unsigned int inventory_idnx, int item_index,
                      int _2devent_idx);  // 0x4BE2DD
    bool Recover(GameTime dt);
    bool CanCastSpell(unsigned int uRequiredMana);
    void SpendMana(unsigned int uRequiredMana);
    void PlayAwardSound();
    void EquipBody(ITEM_EQUIP_TYPE uEquipType);
    bool HasUnderwaterSuitEquipped();
    bool HasItem(ITEM_TYPE uItemID, bool checkHeldItem);
    void OnInventoryLeftClick();

    bool PlayerHitOrMiss(Actor* pActor, int distancemod, PLAYER_SKILL_LEVEL skillmod);

    unsigned int GetMultiplierForSkillLevel(PLAYER_SKILL_TYPE uSkillType, int mult1, int mult2, int mult3, int mult4);
    int CalculateMeleeDmgToEnemyWithWeapon(ItemGen* weapon,
                                           unsigned int uTargetActorID,
                                           bool addOneDice);
    bool WearsItemAnywhere(ITEM_TYPE item_id) const;
    float GetArmorRecoveryMultiplierFromSkillLevel(PLAYER_SKILL_TYPE armour_skill_type, float param2, float param3, float param4, float param5);
    void SetSkillByEvent(uint16_t Player::*skillToSet,
                         uint16_t skillValue);
    void AddSkillByEvent(uint16_t Player::*skillToSet,
                         uint16_t addSkillValue);
    void PlayAwardSound_Anim();
    void PlayAwardSound_Anim_Face(PlayerSpeech speech);
    void PlayAwardSound_Anim97();
    void PlayAwardSound_Anim97_Face(PlayerSpeech speech);
    void PlayAwardSound_Anim98();
    void PlayAwardSound_Anim98_Face(PlayerSpeech speech);

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

    inline bool IsRaceHuman() { return GetRace() == CHARACTER_RACE_HUMAN; }
    inline bool IsRaceDwarf() { return GetRace() == CHARACTER_RACE_DWARF; }
    inline bool IsRaceElf() { return GetRace() == CHARACTER_RACE_ELF; }
    inline bool IsRaceGoblin() { return GetRace() == CHARACTER_RACE_GOBLIN; }

    inline bool IsMale() { return GetSexByVoice() == SEX_MALE; }
    inline bool IsFemale() { return !IsMale(); }

    ItemGen* GetMainHandItem();
    ItemGen* GetOffHandItem();
    ItemGen* GetBowItem();
    ItemGen* GetArmorItem();
    ItemGen* GetHelmItem();
    ItemGen* GetBeltItem();
    ItemGen* GetCloakItem();
    ItemGen* GetGloveItem();
    ItemGen* GetBootItem();
    ItemGen* GetAmuletItem();
    ItemGen* GetNthRingItem(int ringNum);
    const ItemGen* GetNthEquippedIndexItem(ITEM_SLOT index) const;
    ItemGen *GetNthEquippedIndexItem(ITEM_SLOT index);
    ItemGen* GetItem(unsigned int PlayerEquipment::*itemPos);
    int GetPlayerIndex();

    static void _42ECB5_PlayerAttacksActor();
    static void _42FA66_do_explosive_impact(int xpos, int ypos, int zpos,
                                            int a4, int16_t a5,
                                            signed int actchar);
    void CleanupBeacons();
    bool SetBeacon(size_t index, size_t power);

    PLAYER_SKILL_LEVEL GetSkillLevel(PLAYER_SKILL_TYPE skill);
    PLAYER_SKILL_MASTERY GetSkillMastery(PLAYER_SKILL_TYPE skill);
    void SetSkillLevel(PLAYER_SKILL_TYPE skill, PLAYER_SKILL_LEVEL level);
    void SetSkillMastery(PLAYER_SKILL_TYPE skill, PLAYER_SKILL_MASTERY mastery);

    PlayerConditions conditions;
    uint64_t uExperience;
    std::string pName;
    PLAYER_SEX uSex;
    PLAYER_CLASS_TYPE classType;
    uint8_t uCurrentFace;
    char field_BB;
    uint16_t uMight;
    uint16_t uMightBonus;
    uint16_t uIntelligence;
    uint16_t uIntelligenceBonus;
    uint16_t uWillpower;
    uint16_t uWillpowerBonus;
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
    union {
        struct {
            uint16_t skillStaff;
            uint16_t skillSword;
            uint16_t skillDagger;
            uint16_t skillAxe;
            uint16_t skillSpear;
            uint16_t skillBow;
            uint16_t skillMace;
            uint16_t skillBlaster;
            uint16_t skillShield;
            uint16_t skillLeather;
            uint16_t skillChain;
            uint16_t skillPlate;
            uint16_t skillFire;
            uint16_t skillAir;
            uint16_t skillWater;
            uint16_t skillEarth;
            uint16_t skillSpirit;
            uint16_t skillMind;
            uint16_t skillBody;
            uint16_t skillLight;
            uint16_t skillDark;
            uint16_t skillItemId;
            uint16_t skillMerchant;
            uint16_t skillRepair;
            uint16_t skillBodybuilding;
            uint16_t skillMeditation;
            uint16_t skillPerception;
            uint16_t skillDiplomacy;
            uint16_t skillThievery;
            uint16_t skillDisarmTrap;
            uint16_t skillDodge;
            uint16_t skillUnarmed;
            uint16_t skillMonsterId;
            uint16_t skillArmsmaster;
            uint16_t skillStealing;
            uint16_t skillAlchemy;
            uint16_t skillLearning;
            uint16_t skillClub;
        };
        IndexedArray<PLAYER_SKILL, PLAYER_SKILL_FIRST, PLAYER_SKILL_LAST> pActiveSkills;
    };
    unsigned char _achieved_awards_bits[64];
    PlayerSpells spellbook;
    char _1F6_padding[2];
    int pure_luck_used;
    int pure_speed_used;
    int pure_intellect_used;
    int pure_endurance_used;
    int pure_willpower_used;
    int pure_accuracy_used;
    int pure_might_used;
    union {  // 214h
        struct {
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
    std::array<SpellBuff, 24> pPlayerBuffs;
    unsigned int uVoiceID;
    int uPrevVoiceID;
    int uPrevFace;
    int field_192C;
    int field_1930;
    uint16_t uTimeToRecovery;
    char field_1936;
    char field_1937;
    unsigned int uSkillPoints;
    int sHealth;
    int sMana;
    unsigned int uBirthYear;
    PlayerEquipment pEquipment;
    int field_1988[49];
    char field_1A4C;
    char field_1A4D;
    char lastOpenedSpellbookPage;
    SPELL_TYPE uQuickSpell;
    char playerEventBits[64];
    char _some_attack_bonus;
    char field_1A91;
    char _melee_dmg_bonus;
    char field_1A93;
    char _ranged_atk_bonus;
    char field_1A95;
    char _ranged_dmg_bonus;
    char field_1A97_set0_unused;
    char uFullHealthBonus;
    char _health_related;
    char uFullManaBonus;
    char _mana_related;
    CHARACTER_EXPRESSION_ID expression;
    uint16_t uExpressionTimePassed;
    uint16_t uExpressionTimeLength;
    int16_t uExpressionImageIndex;
    int _expression21_animtime;
    int _expression21_frameset;
    std::vector<LloydBeacon> vBeacons;
    char uNumDivineInterventionCastsThisDay;
    char uNumArmageddonCasts;
    char uNumFireSpikeCasts;
    char field_1B3B_set0_unused;
};

void DamagePlayerFromMonster(unsigned int uObjID, ABILITY_INDEX dmgSource, Vec3i* pPos, signed int a4);
bool IsDwarfPresentInParty(bool b);
bool ShouldLoadTexturesForRaceAndGender(unsigned int _this);
int PlayerCreation_GetUnspentAttributePointCount();
int CycleCharacter(bool backwards);
extern IndexedArray<Player *, 1, 4> pPlayers;

extern enum PlayerSpeech PlayerSpeechID;
