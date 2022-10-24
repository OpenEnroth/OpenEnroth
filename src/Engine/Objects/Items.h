#pragma once
#include <array>
#include <map>
#include <string>

#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/PlayerEnums.h"
#include "Engine/Time.h"

struct Player;

/*   64 */
#pragma pack(push, 1)
struct ItemGen {  // 0x24
    static void PopulateSpecialBonusMap();
    static void PopulateRegularBonusMap();
    static void PopulateArtifactBonusMap();
    static void ClearItemBonusMaps();

    void GetItemBonusArtifact(Player* owner, CHARACTER_ATTRIBUTE_TYPE attrToGet, int* bonusSum);
    void GetItemBonusSpecialEnchantment(Player* owner, CHARACTER_ATTRIBUTE_TYPE attrToGet, int* additiveBonus, int* halfSkillBonus);

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

    bool IsRegularEnchanmentForAttribute(CHARACTER_ATTRIBUTE_TYPE attrToGet);

    inline bool IsBroken() const { return uAttributes & ITEM_BROKEN; }
    inline void SetBroken() { uAttributes |= ITEM_BROKEN; }
    inline bool IsIdentified() const { return uAttributes & ITEM_IDENTIFIED; }
    inline void SetIdentified() { uAttributes |= ITEM_IDENTIFIED; }
    inline bool IsStolen() const { return uAttributes & ITEM_STOLEN; }
    inline void SetStolen() { uAttributes |= ITEM_STOLEN; }

    bool GenerateArtifact();
    unsigned int GetValue();
    std::string GetDisplayName();
    std::string GetIdentifiedName();
    void UpdateTempBonus(GameTime time);
    void Reset();
    int _439DF3_get_additional_damage(DAMAGE_TYPE *a2, bool *vampiyr);

    ITEM_EQUIP_TYPE GetItemEquipType();
    unsigned char GetPlayerSkillType();
    char *GetIconName();
    uint8_t GetDamageDice();
    uint8_t GetDamageRoll();
    uint8_t GetDamageMod();
    bool MerchandiseTest(int _2da_idx);

    int32_t uItemID = ITEM_NULL;        // 0
    int32_t uEnchantmentType = 0;       // 4
    int32_t m_enchantmentStrength = 0;  // 8
    ITEM_ENCHANTMENT special_enchantment = ITEM_ENCHANTMENT_NULL;  // 0c
                              // 25  +5 levels
                              // 16  Drain Hit Points from target.
                              // 35  Increases chance of disarming.
                              // 39  Double damage vs Demons.
                              // 40  Double damage vs Dragons
                              // 45  +5 Speed and Accuracy
                              // 56  +5 Might and Endurance.
                              // 57  +5 Intellect and Personality.
                              // 58  Increased Value.
                              // 60  +3 Unarmed and Dodging skills
                              // 61  +3 Stealing and Disarm skills.
                              // 59  Increased Weapon speed.
                              // 63  Double Damage vs. Elves.
                              // 64  Double Damage vs. Undead.
                              // 67  Adds 5 points of Body damage and +2 Disarm
                              // skill. 68  Adds 6-8 points of Cold damage and
                              // +5 Armor Class. 71  Prevents drowning damage.
                              // 72  Prevents falling damage.
    int32_t uNumCharges = 0;           // 10
    ITEM_FLAGS uAttributes = 0;          // 14
    uint8_t uBodyAnchor = 0;           // 18
    uint8_t uMaxCharges = 0;           // 19
    uint8_t uHolderPlayer = 0;         // 1A
    char field_1B = 0;                 // 1B
    GameTime uExpireTime;        // uint64_t uExpireTime; //1C
};
#pragma pack(pop)

/*  175 */
#pragma pack(push, 1)
struct ItemDesc {  // 30h
    // Item # |Pic File|Name|Value|Equip Stat|Skill Group|Mod1|Mod2|material|
    /// ID/Rep/St|Not identified name|Sprite Index|VarA|VarB|Equip X|Equip
    /// Y|Notes
    char *pIconName;              // 0 4
    char *pName;                  // 4 8
    char *pUnidentifiedName;      // 8 c
    char *pDescription;           // 0c 10
    uint32_t uValue;          // 10 14
    uint16_t uSpriteID;   // 14 18
    int16_t field_1A;             // 16
    int16_t uEquipX;       // 18  1c
    int16_t uEquipY;       // 1a  1e
    ITEM_EQUIP_TYPE uEquipType;   // 1c 20
    uint8_t uSkillType;   // 1d 21
    uint8_t uDamageDice;  // 1e 22
    uint8_t uDamageRoll;  // 1f 23
    uint8_t uDamageMod;   // 20 24
    ITEM_MATERIAL uMaterial;    // 21 25
    char _additional_value;       // 22 26 // TODO(captainurist): actually ITEM_ENCHANTMENT
    char _bonus_type;             // 23  27
    char _bonus_strength;         // 24 28
    char field_25;                // 25  29
    char field_26;                // 26   2A
    char field_27;                // 27   2b
    union {
        uint8_t uChanceByTreasureLvl[6];
        struct {
            uint8_t uChanceByTreasureLvl1;  // 28  2c
            uint8_t uChanceByTreasureLvl2;  // 29  2d
            uint8_t uChanceByTreasureLvl3;  // 2A   2e
            uint8_t uChanceByTreasureLvl4;  // 2B  2f
            uint8_t uChanceByTreasureLvl5;  // 2C  30
            uint8_t uChanceByTreasureLvl6;  // 2D  32
        };
    };
    unsigned char uItemID_Rep_St;  // 2e 32
    char field_2f;
};
#pragma pack(pop)

/*
+10 to all Resistances.	1
        +10 to all Seven Statistics.	2
        Explosive Impact!	3
        Adds 3-4 points of Cold damage.	4
        Adds 6-8 points of Cold damage.	5
        Adds 9-12 points of Cold damage.	6
        Adds 2-5 points of Electrical damage.	7
        Adds 4-10 points of Electrical damage.	8
        Adds 6-15 points of Electrical damage.	9
        Adds 1-6 points of Fire damage.	10
        Adds 2-12 points of Fire damage.	11
        Adds 3-18 points of Fire damage.	12
        Adds 5 points of Body damage.	13
        Adds 8 points of Body damage.	14
        Adds 12 points of Body damage.	15
        Drain Hit Points from target.	16
        Increases rate of Recovery.	17
        Wearer resistant to Diseases.	18
        Wearer resistant to Insanity.	19
        Wearer resistant to Paralysis.	20
        Wearer resistant to Poison.	21
        Wearer resistant to Sleep.	22
        Wearer resistant to Stone.	23
        Increased Knockback.	24
        +5 Level.	25
        Increases effect of all Air spells.	26
        Increases effect of all Body spells.	27
        Increases effect of all Dark spells.	28
        Increases effect of all Earth spells.	29
        Increases effect of all Fire spells.	30
        Increases effect of all Light spells.	31
        Increases effect of all Mind spells.	32
        Increases effect of all Spirit spells.	33
        Increases effect of all Water spells.	34
        Increases chance of Disarming.	35
        Half damage from all missile attacks.	36
        Regenerate Hit points over time.	37
        Regenerate Spell points over time.	38
        Double damage vs Demons.	39
        Double damage vs Dragons	40
        Drain Hit Points from target and Increased Weapon speed.	41
        +1 to Seven Stats, HP, SP, Armor, Resistances.	42
        +10 to Endurance, Armor, Hit points.	43
        +10 Hit points and Regenerate Hit points over time.	44
        +5 Speed and Accuracy.	45
        Adds 10-20 points of Fire damage and +25 Might.	46
        +10 Spell points and Regenerate Spell points over time.	47
        +15 Endurance and +5 Armor.	48
        +10 Intellect and Luck.	49
        +30 Fire Resistance and Regenerate Hit points over time.	50
        +10 Spell points, Speed, Intellect.	51
        +10 Endurance and Accuracy.	52
        +10 Might and Personality.	53
        +15 Endurance and Regenerate Hit points over time.	54
        +15 Luck and Regenerate Spell points over time.	55
        +5 Might and Endurance.	56
        +5 Intellect and Personality.	57
        Increased Value.	58
        Increased Weapon speed.	59
        +3 Unarmed and Dodging skills.	60
        +3 Stealing and Disarm skills.	61
        +3 ID Item and ID Monster skills.	62
        Double Damage vs. Elves.	63
        Double Damage vs. Undead.	64
        Double Damage vs. Titans.	65
        Regenerate Spell points and Hit points over time.	66
        Adds 5 points of Body damage and +2 Disarm skill.	67
        Adds 6-8 points of Cold damage and +5 Armor Class.	68
        +20 Air Resistance and Shielding.	69
        +10 Water Resistance and +2 Alchemy skill.	70
        Prevents damage from drowning.	71
        Prevents damage from falling.	72
*/

/*  391 */
#pragma pack(push, 1)
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
#pragma pack(pop)


class EquipemntPair {
 public:
    ITEM_TYPE m_ItemId;
    ITEM_EQUIP_TYPE m_EquipSlot;
    EquipemntPair(ITEM_TYPE type, ITEM_EQUIP_TYPE slot) {
        m_ItemId = type;
        m_EquipSlot = slot;
    }
    EquipemntPair() {
        m_ItemId = (ITEM_TYPE)0;
        m_EquipSlot = (ITEM_EQUIP_TYPE)0;
    }
};

int GetItemTextureFilename(char *pOut, signed int item_id, int index,
                           int shoulder);

extern ItemGen* ptr_50C9A4_ItemToEnchant;
