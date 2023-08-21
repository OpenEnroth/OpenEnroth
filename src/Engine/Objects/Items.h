#pragma once

#include <stdint.h>
#include <string>

#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Time.h"
#include "Engine/MapEnums.h"
#include "Utility/IndexedArray.h"
#include "GUI/UI/UIHouseEnums.h"
#include "Utility/Flags.h"
#include "Utility/Segment.h"

class Character;
enum class MAP_TREASURE_LEVEL : int8_t;

struct ItemGen {  // 0x24
    static void PopulateSpecialBonusMap();
    static void PopulateRegularBonusMap();
    static void PopulateArtifactBonusMap();
    static void ClearItemBonusMaps();

    void GetItemBonusArtifact(const Character *owner, CharacterAttributeType attrToGet, int *bonusSum) const;
    void GetItemBonusSpecialEnchantment(const Character *owner, CharacterAttributeType attrToGet, int *additiveBonus, int *halfSkillBonus) const;

    inline void ResetEnchantAnimation() { uAttributes &= ~ITEM_ENCHANT_ANIMATION_MASK; }
    inline bool ItemEnchanted() const {
        return uAttributes & ITEM_ENCHANT_ANIMATION_MASK;
    }
    inline bool AuraEffectRed() const {
        return (uAttributes & ITEM_ENCHANT_ANIMATION_MASK) == ITEM_AURA_EFFECT_RED;
    }
    inline bool AuraEffectBlue() const {
        return (uAttributes & ITEM_ENCHANT_ANIMATION_MASK) == ITEM_AURA_EFFECT_BLUE;
    }
    inline bool AuraEffectGreen() const {
        return (uAttributes & ITEM_ENCHANT_ANIMATION_MASK) == ITEM_AURA_EFFECT_GREEN;
    }
    inline bool AuraEffectPurple() const {
        return (uAttributes & ITEM_ENCHANT_ANIMATION_MASK) == ITEM_AURA_EFFECT_PURPLE;
    }

    bool IsRegularEnchanmentForAttribute(CharacterAttributeType attrToGet);

    inline bool IsBroken() const { return uAttributes & ITEM_BROKEN; }
    inline void SetBroken() { uAttributes |= ITEM_BROKEN; }
    inline bool IsIdentified() const { return uAttributes & ITEM_IDENTIFIED; }
    inline void SetIdentified() { uAttributes |= ITEM_IDENTIFIED; }
    inline bool IsStolen() const { return uAttributes & ITEM_STOLEN; }
    inline void SetStolen() { uAttributes |= ITEM_STOLEN; }

    bool GenerateArtifact();
    unsigned int GetValue() const;
    std::string GetDisplayName();
    std::string GetIdentifiedName();
    void UpdateTempBonus(GameTime time);
    void Reset();
    int _439DF3_get_additional_damage(DAMAGE_TYPE *a2, bool *vampiyr);

    ITEM_EQUIP_TYPE GetItemEquipType() const;
    CharacterSkillType GetPlayerSkillType() const;
    const std::string& GetIconName() const;
    uint8_t GetDamageDice() const;
    uint8_t GetDamageRoll() const;
    uint8_t GetDamageMod() const;
    bool MerchandiseTest(HOUSE_ID houseId);

    bool isGold() const {
        return GetItemEquipType() == EQUIP_GOLD;
    }
    bool isShield() const {
        return GetItemEquipType() == EQUIP_SHIELD;
    }
    bool isWand() const {
        return GetItemEquipType() == EQUIP_WAND;
    }
    bool isPotion() const {
        return GetItemEquipType() == EQUIP_POTION;
    }
    bool isBook() const {
        return GetItemEquipType() == EQUIP_BOOK;
    }
    bool isReagent() const {
        return GetItemEquipType() == EQUIP_REAGENT;
    }
    bool isSpellScroll() const {
        return GetItemEquipType() == EQUIP_SPELL_SCROLL;
    }
    bool isMessageScroll() const {
        return GetItemEquipType() == EQUIP_MESSAGE_SCROLL;
    }

    bool isMeleeWeapon() const {
        return GetItemEquipType() == EQUIP_SINGLE_HANDED || GetItemEquipType() == EQUIP_TWO_HANDED;
    }
    bool isWeapon() const {
        return ::isWeapon(GetItemEquipType());
    }
    bool isArmor() const {
        return ::isArmor(GetItemEquipType());
    }
    bool isPassiveEquipment() const {
        return ::isPassiveEquipment(GetItemEquipType());
    }


    ITEM_TYPE uItemID = ITEM_NULL;        // 0
    // TODO(captainurist): this is actually CharacterAttributeType plus one (because 0 means no enchantment),
    // with values in [0..24], i.e. up to & including CHARACTER_ATTRIBUTE_SKILL_UNARMED
    int32_t uEnchantmentType = ITEM_ENCHANTMENT_NULL;       // 4 // For potion it's potion strength.
    int32_t m_enchantmentStrength = 0;  // 8
    ITEM_ENCHANTMENT special_enchantment = ITEM_ENCHANTMENT_NULL;  // 0c // For gold it's amount
    int32_t uNumCharges = 0;           // 10
    ItemFlags uAttributes = 0;          // 14
    ITEM_SLOT uBodyAnchor = ITEM_SLOT_INVALID; // 18
    uint8_t uMaxCharges = 0;           // 19
    int8_t uHolderPlayer = -1;        // 1A
    bool placedInChest = false;        // 1B (was unused, repurposed)
    GameTime uExpireTime;        // uint64_t uExpireTime; //1C
};

struct ItemDesc {  // 30h
    // Item # |Pic File|Name|Value|Equip Stat|Skill Group|Mod1|Mod2|material|
    /// ID/Rep/St|Not identified name|Sprite Index|VarA|VarB|Equip X|Equip
    /// Y|Notes
    std::string iconName = "";              // 0 4
    std::string name = "";                  // 4 8
    std::string pUnidentifiedName = "";      // 8 c
    std::string pDescription = "";           // 0c 10
    uint32_t uValue = 0;          // 10 14
    uint16_t uSpriteID = 0;   // 14 18
    int16_t field_1A = 0;             // 16
    int16_t uEquipX = 0;       // 18  1c
    int16_t uEquipY = 0;       // 1a  1e
    ITEM_EQUIP_TYPE uEquipType = EQUIP_NONE;   // 1c 20
    CharacterSkillType uSkillType = CHARACTER_SKILL_MISC;   // 1d 21
    uint8_t uDamageDice = 0;  // 1e 22
    uint8_t uDamageRoll = 0;  // 1f 23
    uint8_t uDamageMod = 0;   // 20 24
    ITEM_MATERIAL uMaterial = MATERIAL_COMMON;    // 21 25
    char _additional_value = 0;       // 22 26 // TODO(captainurist): actually ITEM_ENCHANTMENT?
    char _bonus_type = 0;             // 23  27 // TODO(captainurist): actually ITEM_ENCHANTMENT??
    char _bonus_strength = 0;         // 24 28
    char field_25 = 0;                // 25  29
    char field_26 = 0;                // 26   2A
    char field_27 = 0;                // 27   2b
    IndexedArray<uint8_t, ITEM_TREASURE_LEVEL_FIRST_RANDOM, ITEM_TREASURE_LEVEL_LAST_RANDOM> uChanceByTreasureLvl = {{}};
    unsigned char uItemID_Rep_St = 0;  // 2e 32
    char field_2f = 0;
};

struct SummonedItem {
    //----- (00493F79) --------------------------------------------------------
    void Initialize(GameTime duration) {
        this->field_0_expire_second = duration.GetSecondsFraction();
        this->field_4_expire_minute = duration.GetMinutesFraction();
        this->field_8_expire_hour = duration.GetHoursOfDay();
        this->field_10_expire_week = duration.GetWeeksOfMonth();
        this->field_C_expire_day = duration.GetDaysOfMonth();
        this->field_14_exprie_month = duration.GetMonths();
        this->field_18_expire_year = duration.GetYears() + game_starting_year;
    }

    int field_0_expire_second = 0;
    int field_4_expire_minute = 0;
    int field_8_expire_hour = 0;
    int field_C_expire_day = 0;
    int field_10_expire_week = 0;
    int field_14_exprie_month = 0;
    int field_18_expire_year = 0;
};

std::string GetItemTextureFilename(ITEM_TYPE item_id, int index, int shoulder);

Segment<ITEM_TREASURE_LEVEL> RemapTreasureLevel(ITEM_TREASURE_LEVEL itemTreasureLevel, MAP_TREASURE_LEVEL mapTreasureLevel);

extern ItemGen *ptr_50C9A4_ItemToEnchant;
