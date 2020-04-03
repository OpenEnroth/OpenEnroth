#pragma once

#include <vector>

#include "Engine/Engine.h"

#include "Engine/Events.h"
#include "Engine/Conditions.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Objects/Items.h"
#include "Engine/Spells/Spells.h"

enum PLAYER_BUFFS {
    PLAYER_BUFF_RESIST_AIR = 0,
    PLAYER_BUFF_BLESS = 1,
    PLAYER_BUFF_RESIST_BODY = 2,
    PLAYER_BUFF_RESIST_EARTH = 3,
    PLAYER_BUFF_FATE = 4,
    PLAYER_BUFF_RESIST_FIRE = 5,
    PLAYER_BUFF_HAMMERHANDS = 6,
    PLAYER_BUFF_HASTE = 7,
    PLAYER_BUFF_HEROISM = 8,
    PLAYER_BUFF_RESIST_MIND = 9,
    PLAYER_BUFF_PAIN_REFLECTION = 10,
    PLAYER_BUFF_PRESERVATION = 11,
    PLAYER_BUFF_REGENERATION = 12,
    PLAYER_BUFF_SHIELD = 13,
    PLAYER_BUFF_STONESKIN = 14,
    PLAYER_BUFF_ACCURACY = 15,
    PLAYER_BUFF_ENDURANCE = 16,
    PLAYER_BUFF_INTELLIGENCE = 17,
    PLAYER_BUFF_LUCK = 18,
    PLAYER_BUFF_STRENGTH = 19,
    PLAYER_BUFF_WILLPOWER = 20,
    PLAYER_BUFF_SPEED = 21,
    PLAYER_BUFF_RESIST_WATER = 22,
    PLAYER_BUFF_WATER_WALK = 23
};

#define PLAYER_GUILD_BITS__FINED 1
#define PLAYER_GUILD_BITS__SPIRIT_MEMBERSHIP 58
#define PLAYER_GUILD_BITS__ARCOMAGE_WIN 103
#define PLAYER_GUILD_BITS__ARCOMAGE_LOSE 104

/*  301 */
enum PlayerSpeech {
    SPEECH_1 = 1,
    SPEECH_2 = 2,
    SPEECH_3 = 3,
    SPEECH_4 = 4,
    SPEECH_5 = 5,
    SPEECH_6 = 6,
    SPEECH_7 = 7,
    SPEECH_8 = 8,
    SPEECH_9 = 9,
    SPEECH_10 = 10,
    SPEECH_11 = 11,
    SPEECH_12 = 12,
    SPEECH_CantRestHere = 13,
    SPEECH_14 = 14,
    SPEECH_NoRoom = 15,  // when placing to inventory
    SPEECH_DO_POTION_FINE = 16,
    SPEECH_17 = 17,
    SPEECH_18 = 18,
    SPEECH_19 = 19,
    SPEECH_20 = 20,
    SPEECH_21 = 21,
    SPEECH_GoodDay = 22,  // greets on dialogue start
    SPEECH_GoodEvening = 23,
    SPEECH_24 = 24,  // damage - owww
    SPEECH_25 = 25,  // weak
    SPEECH_26 = 26,  // fear
    SPEECH_27 = 27,  // poisoned
    SPEECH_28 = 28,  // diseased
    SPEECH_29 = 29,  // insane
    SPEECH_30 = 30,  // cursed
    SPEECH_31 = 31,  // drunk
    SPEECH_32 = 32,  // uncnocious
    SPEECH_33 = 33,  // dead
    SPEECH_34 = 34,  // petrified
    SPEECH_35 = 35,  // eradicated
    SPEECH_36 = 36,
    SPEECH_37 = 37,
    SPEECH_NotEnoughGold = 38,
    SPEECH_39 = 39,  // cant use that
    SPEECH_40 = 40,
    SPEECH_41 = 41,
    SPEECH_42 = 42,
    SPEECH_43 = 43,
    SPEECH_44 = 44,
    SPEECH_45 = 45,
    SPEECH_46 = 46,
    SPEECH_47 = 47,
    SPEECH_48 = 48,
    SPEECH_49 = 49,
    SPEECH_50 = 50,
    SPEECH_51 = 51,
    SPEECH_52 = 52,  // ATTACK MISSED??
    SPEECH_53 = 53,
    SPEECH_54 = 54,
    SPEECH_55 = 55,
    SPEECH_56 = 56,
    SPEECH_57 = 57,
    SPEECH_58 = 58,
    SPEECH_59 = 59,
    SPEECH_60 = 60,
    SPEECH_61 = 61,
    SPEECH_62 = 62,
    SPEECH_63 = 63,
    SPEECH_64 = 64,
    SPEECH_Yell = 65,
    SPEECH_Falling_scream = 66,
    SPEECH_67 = 67,
    SPEECH_68 = 68,
    SPEECH_69 = 69,
    SPEECH_70 = 70,
    SPEECH_CarriageReady = 71,  // travelling by carriage
    SPEECH_SetSail = 72,        // travelling by sea
    SPEECH_73 = 73,
    SPEECH_74 = 74,
    SPEECH_75 = 75,
    SPEECH_76 = 76,
    SPEECH_77 = 77,
    SPEECH_78 = 78,
    SPEECH_79 = 79,
    SPEECH_80 = 80,
    SPEECH_81 = 81,
    SPEECH_82 = 82,
    SPEECH_83 = 83,
    SPEECH_84 = 84,
    SPEECH_85 = 85,
    SPEECH_86 = 86,
    SPEECH_87 = 87,
    SPEECH_88 = 88,
    SPEECH_89 = 89,
    SPEECH_90 = 90,
    SPEECH_91 = 91,
    SPEECH_92 = 92,
    SPEECH_93 = 93,
    SPEECH_94 = 94,
    SPEECH_95 = 95,
    SPEECH_96 = 96,
    SPEECH_97 = 97,
    SPEECH_98 = 98,
    SPEECH_99 = 99,  // zombie
    SPEECH_100 = 100,
    SPEECH_101 = 101,
    SPEECH_PickMe = 102,
    SPEECH_103 = 103,
    SPEECH_IDENTIFY_MONSTER_WEAKER = 104,
    SPEECH_IDENTIFY_MONSTER_STRONGER = 105,
    SPEECH_IDENTIFY_MONSTER_106 = 106,
    SPEECH_107 = 107,  // its just you and me now
    SPEECH_108 = 108,
    SPEECH_109 = 109,
    SPEECH_110 = 110,
};

/*  339 */
enum CHARACTER_RACE {
    CHARACTER_RACE_HUMAN = 0,
    CHARACTER_RACE_ELF = 1,
    CHARACTER_RACE_GOBLIN = 2,
    CHARACTER_RACE_DWARF = 3,
};

#pragma warning(push)
#pragma warning(disable : 4341)
/*  328 */
enum PLAYER_SKILL_TYPE : signed __int8 {
    PLAYER_SKILL_STAFF = 0,
    PLAYER_SKILL_SWORD = 1,
    PLAYER_SKILL_DAGGER = 2,
    PLAYER_SKILL_AXE = 3,
    PLAYER_SKILL_SPEAR = 4,
    PLAYER_SKILL_BOW = 5,
    PLAYER_SKILL_MACE = 6,
    PLAYER_SKILL_BLASTER = 7,
    PLAYER_SKILL_SHIELD = 8,
    PLAYER_SKILL_LEATHER = 9,
    PLAYER_SKILL_CHAIN = 10,
    PLAYER_SKILL_PLATE = 11,
    PLAYER_SKILL_FIRE = 12,
    PLAYER_SKILL_AIR = 13,
    PLAYER_SKILL_WATER = 14,
    PLAYER_SKILL_EARTH = 15,
    PLAYER_SKILL_SPIRIT = 16,
    PLAYER_SKILL_MIND = 17,
    PLAYER_SKILL_BODY = 18,
    PLAYER_SKILL_LIGHT = 19,
    PLAYER_SKILL_DARK = 20,
    PLAYER_SKILL_ITEM_ID = 21,
    PLAYER_SKILL_MERCHANT = 22,
    PLAYER_SKILL_REPAIR = 23,
    PLAYER_SKILL_BODYBUILDING = 24,
    PLAYER_SKILL_MEDITATION = 25,
    PLAYER_SKILL_PERCEPTION = 26,
    PLAYER_SKILL_DIPLOMACY = 27,
    PLAYER_SKILL_TIEVERY = 28,
    PLAYER_SKILL_TRAP_DISARM = 29,
    PLAYER_SKILL_DODGE = 30,
    PLAYER_SKILL_UNARMED = 31,
    PLAYER_SKILL_MONSTER_ID = 32,
    PLAYER_SKILL_ARMSMASTER = 33,
    PLAYER_SKILL_STEALING = 34,
    PLAYER_SKILL_ALCHEMY = 35,
    PLAYER_SKILL_LEARNING = 36,
    PLAYER_SKILL_CLUB = 37,
    PLAYER_SKILL_MISC = 38,
    PLAYER_SKILL_INVALID = -1
};
#pragma warning(pop)

/*  329 */
enum PLAYER_CLASS_TYPE : unsigned __int8 {
    PLAYER_CLASS_KNIGHT = 0,
    PLAYER_CLASS_CHEVALIER = 1,
    PLAYER_CLASS_CHAMPION = 2,
    PLAYER_CLASS_BLACK_KNIGHT = 3,
    PLAYER_CLASS_THEIF = 4,
    PLAYER_CLASS_ROGUE = 5,
    PLAYER_CLASS_SPY = 6,
    PLAYER_CLASS_ASSASSIN = 7,
    PLAYER_CLASS_MONK = 8,
    PLAYER_CLASS_INITIATE = 9,
    PLAYER_CLASS_MASTER = 10,
    PLAYER_CLASS_NINJA = 11,
    PLAYER_CLASS_PALADIN = 12,
    PLAYER_CLASS_CRUSADER = 13,
    PLAYER_CLASS_HERO = 14,
    PLAYER_CLASS_VILLIAN = 15,
    PLAYER_CLASS_ARCHER = 16,
    PLAYER_CLASS_WARRIOR_MAGE = 17,
    PLAYER_CLASS_MASTER_ARCHER = 18,
    PLAYER_CLASS_SNIPER = 19,
    PLAYER_CLASS_RANGER = 20,
    PLAYER_CLASS_HUNTER = 21,
    PLAYER_CLASS_RANGER_LORD = 22,
    PLAYER_CLASS_BOUNTY_HUNTER = 23,
    PLAYER_CLASS_CLERIC = 24,
    PLAYER_CLASS_PRIEST = 25,
    PLAYER_CLASS_PRIEST_OF_SUN = 26,
    PLAYER_CLASS_PRIEST_OF_MOON = 27,
    PLAYER_CLASS_DRUID = 28,
    PLAYER_CLASS_GREAT_DRUID = 29,
    PLAYER_CLASS_ARCH_DRUID = 30,
    PLAYER_CLASS_WARLOCK = 31,
    PLAYER_CLASS_SORCERER = 32,
    PLAYER_CLASS_WIZARD = 33,
    PLAYER_CLASS_ARCHMAGE = 34,
    PLAYER_CLASS_LICH = 35
};

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
        if (image != nullptr) {
            image->Release();
        }
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
            unsigned int uShield;
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
        std::array<unsigned int, 16> pIndices;
    };
};
#pragma pack(pop)

enum CHARACTER_EXPRESSION_ID : unsigned __int16 {
    CHARACTER_EXPRESSION_INVALID = 0,
    CHARACTER_EXPRESSION_1 = 1,
    CHARACTER_EXPRESSION_CURSED = 2,
    CHARACTER_EXPRESSION_WEAK = 3,
    CHARACTER_EXPRESSION_SLEEP = 4,
    CHARACTER_EXPRESSION_FEAR = 5,
    CHARACTER_EXPRESSION_DRUNK = 6,
    CHARACTER_EXPRESSION_INSANE = 7,
    CHARACTER_EXPRESSION_POISONED = 8,
    CHARACTER_EXPRESSION_DISEASED = 9,
    CHARACTER_EXPRESSION_PARALYZED = 10,
    CHARACTER_EXPRESSION_UNCONCIOUS = 11,
    CHARACTER_EXPRESSION_PERTIFIED = 12,
    CHARACTER_EXPRESSION_13 = 13,
    CHARACTER_EXPRESSION_14 = 14,
    CHARACTER_EXPRESSION_15 = 15,
    CHARACTER_EXPRESSION_16 = 16,
    CHARACTER_EXPRESSION_17 = 17,
    CHARACTER_EXPRESSION_18 = 18,
    CHARACTER_EXPRESSION_19 = 19,
    CHARACTER_EXPRESSION_20 = 20,
    CHARACTER_EXPRESSION_21 = 21,
    CHARACTER_EXPRESSION_22 = 22,
    CHARACTER_EXPRESSION_23 = 23,
    CHARACTER_EXPRESSION_24 = 24,
    CHARACTER_EXPRESSION_25 = 25,
    CHARACTER_EXPRESSION_26 = 26,
    CHARACTER_EXPRESSION_27 = 27,
    CHARACTER_EXPRESSION_28 = 28,
    CHARACTER_EXPRESSION_29 = 29,
    CHARACTER_EXPRESSION_30 = 30,
    CHARACTER_EXPRESSION_31 = 31,
    CHARACTER_EXPRESSION_32 = 32,
    CHARACTER_EXPRESSION_33 = 33,
    CHARACTER_EXPRESSION_DMGRECVD_MINOR = 34,
    CHARACTER_EXPRESSION_DMGRECVD_MODERATE = 35,
    CHARACTER_EXPRESSION_DMGRECVD_MAJOR = 36,
    CHARACTER_EXPRESSION_37 = 37,
    CHARACTER_EXPRESSION_38 = 38,
    CHARACTER_EXPRESSION_39 = 39,

    // ?

    CHARACTER_EXPRESSION_SCARED = 46,  // like falling

    CHARACTER_EXPRESSION_54 = 54,
    CHARACTER_EXPRESSION_55 = 55,
    CHARACTER_EXPRESSION_56 = 56,
    CHARACTER_EXPRESSION_57 = 57,
    CHARACTER_EXPRESSION_FALLING = 58,

    // ?

    CHARACTER_EXPRESSION_DEAD = 98,
    CHARACTER_EXPRESSION_ERADICATED = 99,
};

enum PLAYER_SEX : uint8_t { SEX_MALE = 0, SEX_FEMALE = 1 };

struct Player {
    static const unsigned int INVETORYSLOTSWIDTH = 14;
    static const unsigned int INVETORYSLOTSHEIGHT = 9;

    Player();

    void SetVariable(enum VariableType var, signed int a3);
    void AddVariable(enum VariableType var, signed int val);
    void SubtractVariable(enum VariableType VarNum, signed int pValue);
    bool CompareVariable(enum VariableType VarNum, signed int pValue);
    void UseItem_DrinkPotion_etc(signed int a2, int a3);
    bool AddItem(struct ItemGen* pItem);
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
    String GetMeleeDamageString();
    String GetRangedDamageString();
    bool CanTrainToNextLevel();
    unsigned int GetExperienceDisplayColor();
    int CalculateIncommingDamage(DAMAGE_TYPE dmg_type, int amount);
    ITEM_EQUIP_TYPE GetEquippedItemEquipType(ITEM_EQUIP_TYPE uEquipSlot);
    PLAYER_SKILL_TYPE GetEquippedItemSkillType(ITEM_EQUIP_TYPE uEquipSlot);
    bool IsUnarmed();
    bool HasItemEquipped(ITEM_EQUIP_TYPE uEquipIndex);
    bool HasEnchantedItemEquipped(int uEnchantment);
    bool WearsItem(int item_id, ITEM_EQUIP_TYPE equip_type);
    int StealFromShop(ItemGen* itemToSteal, int extraStealDifficulty,
                      int reputation, int extraStealFine, int* fineIfFailed);
    int StealFromActor(unsigned int uActorID, int _steal_perm, int reputation);
    void Heal(int amount);
    int ReceiveDamage(signed int amount, DAMAGE_TYPE dmg_type);
    int ReceiveSpecialAttackEffect(int attType, struct Actor* pActor);
    unsigned int GetSpellSchool(unsigned int uSpellID);
    int GetAttackRecoveryTime(bool bRangedAttack);

    int GetHealth() const { return this->sHealth; }
    int GetMaxHealth();
    int GetMana() const { return this->sMana; }
    int GetMaxMana();

    int GetBaseAC();
    int GetActualAC();
    unsigned int GetBaseAge();
    unsigned int GetActualAge();
    int GetBaseResistance(enum CHARACTER_ATTRIBUTE_TYPE a2);
    int GetActualResistance(enum CHARACTER_ATTRIBUTE_TYPE a2);
    void SetRecoveryTime(signed int sRecoveryTime);
    void RandomizeName();
    unsigned int GetMajorConditionIdx();
    int GetParameterBonus(int player_parameter);
    int GetSpecialItemBonus(ITEM_ENCHANTMENT enchantment);
    int GetItemsBonus(enum CHARACTER_ATTRIBUTE_TYPE attr, bool a3 = false);
    int GetMagicalBonus(enum CHARACTER_ATTRIBUTE_TYPE a2);
    int GetActualSkillLevel(PLAYER_SKILL_TYPE uSkillType);
    int GetActualSkillMastery(PLAYER_SKILL_TYPE uSkillType);
    int GetSkillBonus(enum CHARACTER_ATTRIBUTE_TYPE a2);
    enum CHARACTER_RACE GetRace() const;
    String GetRaceName() const;
    PLAYER_SEX GetSexByVoice();
    void SetInitialStats();
    void SetSexByVoice();
    void Reset(PLAYER_CLASS_TYPE classType);
    PLAYER_SKILL_TYPE GetSkillIdxByOrder(signed int order);
    void DecreaseAttribute(int eAttribute);
    void IncreaseAttribute(int eAttribute);
    void Zero();
    unsigned int GetStatColor(int uStat);
    bool DiscardConditionIfLastsLongerThan(unsigned int uCondition,
                                           GameTime time);
    int SelectPhrasesTransaction(ItemGen* pItem, int building_type,
                                 int BuildID_2Events, int a5);
    int GetBodybuilding();
    int GetMeditation();
    bool CanIdentify(ItemGen* pItem);
    bool CanRepair(ItemGen* pItem);
    int GetMerchant();
    int GetPerception();
    int GetDisarmTrap();
    char GetLearningPercent();
    bool CanFitItem(unsigned int uSlot, unsigned int uItemID);
    int FindFreeInventoryListSlot();
    int CreateItemInInventory(unsigned int uSlot, unsigned int uItemID);
    int HasSkill(unsigned int uSkillType);
    void WearItem(unsigned int uItemID);
    int AddItem(int uSlot, unsigned int uItemID);
    int AddItem2(int uSlot, ItemGen* Src);
    int CreateItemInInventory2(unsigned int index, ItemGen* Src);
    void PutItemArInventoryIndex(int uItemID, int itemListPos, int uSlot);
    void RemoveItemAtInventoryIndex(unsigned int uSlot);
    bool CanAct();
    bool CanSteal();
    bool CanEquip_RaceAndAlignmentCheck(unsigned int uItemID);
    void SetCondition(unsigned int uConditionIdx, int blockable);
    bool ProfessionOrGuildFlagsCorrect(unsigned int uClass, int a3);
    void PlaySound(PlayerSpeech speech, int a3);
    void PlayEmotion(CHARACTER_EXPRESSION_ID expression, int duration);
    void ItemsPotionDmgBreak(int enchant_count);
    unsigned int GetItemListAtInventoryIndex(int inout_item_cell);
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
    int GetConditionDaysPassed(unsigned int uCondition);
    bool NothingOrJustBlastersEquipped();
    void SalesProcess(unsigned int inventory_idnx, int item_index,
                      int _2devent_idx);  // 0x4BE2DD
    bool Recover(int dt);
    bool CanCastSpell(unsigned int uRequiredMana);
    void PlayAwardSound();
    void EquipBody(ITEM_EQUIP_TYPE uEquipType);
    bool HasUnderwaterSuitEquipped();
    bool HasItem(unsigned int uItemID, bool checkHeldItem);
    void OnInventoryLeftClick();

    bool PlayerHitOrMiss(Actor* pActor, int distancemod, int skillmod);

    unsigned int GetMultiplierForSkillLevel(PLAYER_SKILL_TYPE uSkillType,
                                            int mult1, int mult2, int mult3,
                                            int mult4);
    int CalculateMeleeDmgToEnemyWithWeapon(ItemGen* weapon,
                                           unsigned int uTargetActorID,
                                           bool addOneDice);
    bool WearsItemAnyWhere(int item_id);
    float GetArmorRecoveryMultiplierFromSkillLevel(
        unsigned char armour_skill_type, float param2, float param3,
        float param4, float param5);
    void SetSkillByEvent(unsigned __int16 Player::*skillToSet,
                         unsigned __int16 skillValue);
    void AddSkillByEvent(unsigned __int16 Player::*skillToSet,
                         unsigned __int16 addSkillValue);
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
    ItemGen* GetNthEquippedIndexItem(int index);
    ItemGen* GetItem(unsigned int PlayerEquipment::*itemPos);
    int GetPlayerIndex();

    static void _42ECB5_PlayerAttacksActor();
    static void _42FA66_do_explosive_impact(int xpos, int ypos, int zpos,
                                            int a4, __int16 a5,
                                            signed int actchar);
    void CleanupBeacons();
    bool SetBeacon(size_t index, size_t power);

    std::array<GameTime, 20> conditions_times;
    unsigned __int64 uExperience;
    char pName[16];
    PLAYER_SEX uSex;
    PLAYER_CLASS_TYPE classType;
    unsigned __int8 uCurrentFace;
    char field_BB;
    unsigned __int16 uMight;
    unsigned __int16 uMightBonus;
    unsigned __int16 uIntelligence;
    unsigned __int16 uIntelligenceBonus;
    unsigned __int16 uWillpower;
    unsigned __int16 uWillpowerBonus;
    unsigned __int16 uEndurance;
    unsigned __int16 uEnduranceBonus;
    unsigned __int16 uSpeed;
    unsigned __int16 uSpeedBonus;
    unsigned __int16 uAccuracy;
    unsigned __int16 uAccuracyBonus;
    unsigned __int16 uLuck;
    unsigned __int16 uLuckBonus;
    __int16 sACModifier;
    unsigned __int16 uLevel;
    __int16 sLevelModifier;
    __int16 sAgeModifier;
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
            unsigned __int16 skillStaff;
            unsigned __int16 skillSword;
            unsigned __int16 skillDagger;
            unsigned __int16 skillAxe;
            unsigned __int16 skillSpear;
            unsigned __int16 skillBow;
            unsigned __int16 skillMace;
            unsigned __int16 skillBlaster;
            unsigned __int16 skillShield;
            unsigned __int16 skillLeather;
            unsigned __int16 skillChain;
            unsigned __int16 skillPlate;
            unsigned __int16 skillFire;
            unsigned __int16 skillAir;
            unsigned __int16 skillWater;
            unsigned __int16 skillEarth;
            unsigned __int16 skillSpirit;
            unsigned __int16 skillMind;
            unsigned __int16 skillBody;
            unsigned __int16 skillLight;
            unsigned __int16 skillDark;
            unsigned __int16 skillItemId;
            unsigned __int16 skillMerchant;
            unsigned __int16 skillRepair;
            unsigned __int16 skillBodybuilding;
            unsigned __int16 skillMeditation;
            unsigned __int16 skillPerception;
            unsigned __int16 skillDiplomacy;
            unsigned __int16 skillThievery;
            unsigned __int16 skillDisarmTrap;
            unsigned __int16 skillDodge;
            unsigned __int16 skillUnarmed;
            unsigned __int16 skillMonsterId;
            unsigned __int16 skillArmsmaster;
            unsigned __int16 skillStealing;
            unsigned __int16 skillAlchemy;
            unsigned __int16 skillLearning;
        };
        std::array<unsigned __int16, 37>
            pActiveSkills;  // this encodes level and mastery using bitwise
                            // comparison   ( & 0x3F for level) ( & 0x1C0
                            // mastery 1-4)
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
            ItemGen pInventoryItemList[126];
            ItemGen pEquippedItems[12];
        };
        std::array<ItemGen, 138> pOwnItems;
    };

    std::array<int, 126> pInventoryMatrix;
    __int16 sResFireBase;
    __int16 sResAirBase;
    __int16 sResWaterBase;
    __int16 sResEarthBase;
    __int16 field_177C;
    __int16 sResMagicBase;
    __int16 sResSpiritBase;
    __int16 sResMindBase;
    __int16 sResBodyBase;
    __int16 sResLightBase;
    __int16 sResDarkBase;
    __int16 sResFireBonus;
    __int16 sResAirBonus;
    __int16 sResWaterBonus;
    __int16 sResEarthBonus;
    __int16 field_1792;
    __int16 sResMagicBonus;
    __int16 sResSpiritBonus;
    __int16 sResMindBonus;
    __int16 sResBodyBonus;
    __int16 sResLightBonus;
    __int16 sResDarkBonus;
    std::array<SpellBuff, 24> pPlayerBuffs;
    unsigned int uVoiceID;
    int uPrevVoiceID;
    int uPrevFace;
    int field_192C;
    int field_1930;
    unsigned __int16 uTimeToRecovery;
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
    unsigned __int8 uQuickSpell;
    char playerEventBits[64];
    char _some_attack_bonus;
    char field_1A91;
    char _melee_dmg_bonus;
    char field_1A93;
    char _ranged_atk_bonus;
    char field_1A95;
    char _ranged_dmg_bonus;
    char field_1A97;
    char uFullHealthBonus;
    char _health_related;
    char uFullManaBonus;
    char _mana_related;
    CHARACTER_EXPRESSION_ID expression;
    unsigned __int16 uExpressionTimePassed;
    unsigned __int16 uExpressionTimeLength;
    __int16 field_1AA2;
    int _expression21_animtime;
    int _expression21_frameset;
    std::vector<LloydBeacon> vBeacons;
    char uNumDivineInterventionCastsThisDay;
    char uNumArmageddonCasts;
    char uNumFireSpikeCasts;
    char field_1B3B;
};

void DamagePlayerFromMonster(unsigned int uObjID, int a2,
                             struct Vec3_int_* pPos, signed int a4);
bool IsDwarfPresentInParty(bool b);
bool ShouldLoadTexturesForRaceAndGender(unsigned int _this);
int PlayerCreation_GetUnspentAttributePointCount();
int CycleCharacter(bool backwards);
unsigned int SkillToMastery(unsigned int skill_value);

extern NZIArray<struct Player*, 5> pPlayers;

extern enum PlayerSpeech PlayerSpeechID;
