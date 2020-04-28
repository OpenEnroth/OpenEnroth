#pragma once

#include <array>
#include <map>

#include "Engine/Engine.h"
#include "Engine/Time.h"
#include "Engine/NZIArray.h"

enum DAMAGE_TYPE : unsigned int {
    DMGT_FIRE = 0,
    DMGT_ELECTR = 1,
    DMGT_COLD = 2,
    DMGT_EARTH = 3,
    DMGT_PHISYCAL = 4,
    DMGT_MAGICAL = 5,
    DMGT_SPIRIT = 6,
    DMGT_MIND = 7,
    DMGT_BODY = 8,
    DMGT_LIGHT = 9,
    DMGT_DARK = 10
};

/*  338 */
enum ITEM_FLAGS : unsigned int {
    ITEM_IDENTIFIED = 0x1,
    ITEM_BROKEN = 0x2,
    ITEM_TEMP_BONUS = 0x8,
    ITEM_AURA_EFFECT_RED = 0x10,
    ITEM_AURA_EFFECT_BLUE = 0x20,
    ITEM_AURA_EFFECT_GREEN = 0x40,
    ITEM_AURA_EFFECT_PURPLE = 0x80,
    ITEM_ENCHANT_ANIMATION = 0xF0,
    ITEM_STOLEN = 0x100,
    ITEM_HARDENED = 0x200,
};

enum ITEM_ENCHANTMENT : unsigned int {
    ITEM_ENCHANTMENT_NULL = 0,

    ITEM_ENCHANTMENT_OF_COLD = 4,          // Adds 3-4 points of cold damage
    ITEM_ENCHANTMENT_OF_FROST = 5,         // Adds 6-8 points of cold damage
    ITEM_ENCHANTMENT_OF_ICE = 6,           // Adds 9-12 points of cold damage
    ITEM_ENCHANTMENT_OF_SPARKS = 7,        // Adds 2-5 points of electrical damage
    ITEM_ENCHANTMENT_OF_LIGHTNING = 8,     // Adds 4-10 points of electrical damage
    ITEM_ENCHANTMENT_OF_THUNDERBOLTS = 9,  // Adds 6-15 points of electrical damage
    ITEM_ENCHANTMENT_OF_FIRE = 10,         // Adds 1-6 points of fire damage
    ITEM_ENCHANTMENT_OF_FLAME = 11,        // Adds 2-12 points of fire damage
    ITEM_ENCHANTMENT_OF_INFERNOS = 12,     // Adds 3-18 points of fire damage
    ITEM_ENCHANTMENT_OF_POISON = 13,       // Adds 5 points of body damage
    ITEM_ENCHANTMENT_OF_VENOM = 14,        // Adds 8 points of body damage
    ITEM_ENCHANTMENT_OF_ACID = 15,         // Adds 12 points of body damage
    ITEM_ENCHANTMENT_VAMPIRIC = 16,        // 20% of damage dealt given to wielder
    ITEM_ENCHANTMENT_OF_RECOVERY = 17,     // Increases rate of Recovery

    ITEM_ENCHANTMENT_OF_FORCE = 24,        // Increases Knockback

    ITEM_ENCHANTMENT_40 = 40,
    ITEM_ENCHANTMENT_OF_DARKNESS = 41,     // Vampiric and Swift

    ITEM_ENCHANTMENT_OF_DRAGON =
        46,  // Adds 10-20 points of fire damage and +25 Might
};

enum ITEM_MATERIAL {
    MATERIAL_COMMON = 0,
    MATERIAL_ARTEFACT = 1,
    MATERIAL_RELIC = 2,
    MATERIAL_SPECIAL = 3
};

/*  330 */
enum ITEM_TYPE {
    ITEM_NULL = 0,
    ITEM_LONGSWORD_1 = 0x1,
    ITEM_DAGGER_1 = 15,
    ITEM_AXE_1 = 23,
    ITEM_SPEAR_1 = 31,
    ITEM_CROSSBOW_1 = 47,
    ITEM_MACE_1 = 50,
    ITEM_STAFF_1 = 61,
    ITEM_BLASTER = 64,
    ITEM_LASER_RIFLE = 65,
    ITEM_LEATHER_1 = 66,
    ITEM_CHAINMAIL_1 = 71,
    ITEM_PLATE_1 = 76,
    ITEM_BUCKLER_1 = 84,
    ITEM_92 = 92,
    ITEM_GAUNTLETS_1 = 110,
    ITEM_BOOTS_1 = 115,
    ITEM_WAND_FIRE = 135,
    ITEM_WAND_STUN = 138,
    ITEM_WAND_INCENERATION = 159,
    ITEM_160 = 160,
    ITEM_161 = 161,
    ITEM_162 = 162,
    ITEM_GOLD_SMALL = 197,
    ITEM_GOLD_MEDIUM = 198,
    ITEM_GOLD_LARGE = 199,
    ITEM_REAGENT_WIDOWSWEEP_BERRIES = 200,
    ITEM_REAGENT_CRUSHED_ROSE_PETALS = 201,
    ITEM_TROLL_BLOOD = 202,
    ITEM_TROLL_RUBY = 203,
    ITEM_DRAGON_EYE = 204,
    ITEM_PHIMA_ROOT = 205,
    ITEM_METEORITE_FRAGMENT = 206,
    ITEM_HARPY_FEATHER = 207,
    ITEM_MOONSTONE = 208,
    ITEM_ELVISH_TOADSTOOL = 209,
    ITEM_POPPYSNAPS = 210,
    ITEM_FAE_DUST = 211,
    ITEM_SULFUR = 212,
    ITEM_GARNET = 213,
    ITEM_DEVIL_ICHOR = 214,
    ITEM_MUSHROOM = 215,
    ITEM_OBSIDIAN = 216,
    ITEM_OOZE_ENDOPLASM_VIAL = 217,
    ITEM_MERCURY = 218,
    ITEM_REAGENT_PHILOSOPHERS_STONE = 219,
    ITEM_POTION_BOTTLE = 220,
    ITEM_POTION_CATALYST = 221,
    ITEM_POTION_CURE_WOUNDS = 222,
    ITEM_POTION_MAGIC_POTION = 223,
    ITEM_POTION_CURE_WEAKNESS = 224,
    ITEM_POTION_CURE_DISEASE = 225,
    ITEM_POTION_AWAKEN = 227,
    ITEM_POTION_HASTE = 228,
    ITEM_POTION_RECHARGE_ITEM = 233,
    ITEM_POTION_HARDEN_ITEM = 236,
    ITEM_POTION_CURE_INSANITY = 239,
    ITEM_POTION_MIGHT_BOOST = 240,
    ITEM_POTION_ACCURACY_BOOST = 245,
    ITEM_POTION_FLAMING_POTION = 246,
    ITEM_POTION_SWIFT_POTION = 250,
    ITEM_POTION_BODY_RESISTANE = 261,
    ITEM_POTION_STONE_TO_FLESH = 262,
    ITEM_POTION_SLAYING_POTION = 263,
    ITEM_POTION_REJUVENATION = 271,
    ITEM_SPELLBOOK_TORCHLIGHT = 400,
    ITEM_SPELLBOOK_FIRE_STRIKE = 401,
    ITEM_SPELLBOOK_AIR_FEATHER_FALL = 412,
    ITEM_SPELLBOOK_WATER_POISON_SPRAY = 423,
    ITEM_SPELLBOOK_EARTH_SLOW = 434,
    ITEM_SPELLBOOK_SPIRIT_BLESS = 445,
    ITEM_SPELLBOOK_MIND_REMOVE_FEAR = 455,
    ITEM_SPELLBOOK_MIND_MIND_BLAST = 456,
    ITEM_SPELLBOOK_BODY_FIRST_AID = 467,
    ITEM_SPELLBOOK_BODY_HEAL = 470,
    ITEM_SPELLBOOK_BODY_BREAK_POISON = 471,
    ITEM_SPELLBOOK_LIGHT_LIGHT_BOLT = 477,
    ITEM_SPELLBOOK_LIGHT_SUN_BURST = 486,
    ITEM_SPELLBOOK_LIGHT_DIVINE_INTERVENTION = 487,
    ITEM_ARTIFACT_PUCK = 500,  // 0x1F4,
    ITEM_ARTIFACT_IRON_FEATHER = 501,
    ITEM_ARTIFACT_WALLACE = 502,
    ITEM_ARTIFACT_CORSAIR = 503,
    ITEM_ARTIFACT_GOVERNORS_ARMOR = 504,  // 1F8
    ITEM_ARTIFACT_YORUBA = 505,           // 1F9
    ITEM_ARTIFACT_SPLITTER = 506,         // 1FA
    ITEM_ARTIFACT_GHOULSBANE = 507,       // 1FA
    ITEM_ARTIFACT_GIBBET = 508,           // 1FA
    ITEM_ARTIFACT_CHARELE = 509,          // 1FA
    ITEM_ARTEFACT_ULLYSES = 510,
    ITEM_ARTEFACT_HANDS_OF_THE_MASTER = 511,
    ITEM_ARTIFACT_LEAGUE_BOOTS = 512,  // 200
    ITEM_ARTIFACT_RULERS_RING = 513,
    ITEM_RELIC_MASH = 514,
    ITEM_RELIC_ETHRICS_STAFF = 515,   // 204
    ITEM_RELIC_HARECS_LEATHER = 516,  // 204
    ITEM_RELIC_OLD_NICK = 517,
    ITEM_RELIC_AMUCK = 518,
    ITEM_RELIC_GLORY_SHIELD = 519,
    ITEM_RELIC_KELEBRIM = 520,         // 208
    ITEM_RELIC_TALEDONS_HELM = 521,    // 209
    ITEM_RELIC_SCHOLARS_CAP = 522,     // 20A
    ITEM_RELIC_PHYNAXIAN_CROWN = 523,  // 20B
    ITEM_RILIC_TITANS_BELT = 524,      // 20C
    ITEM_RELIC_TWILIGHT = 525,         // 20D
    ITEM_RELIC_ANIA_SELVING = 526,
    ITEM_RELIC_JUSTICE = 527,
    ITEM_RELIC_MEKORIGS_HAMMER = 528,
    ITEM_ARTIFACT_HERMES_SANDALS = 529,
    ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP = 530,  // 212
    ITEM_ARTIFACT_ELFBANE = 531,             // 212
    ITEM_ARTIFACT_MINDS_EYE = 532,           // 214
    ITEM_ELVEN_CHAINMAIL = 533,              // 215
    ITEM_FORGE_GAUNTLETS = 534,
    ITEM_ARTIFACT_HEROS_BELT = 535,  // 217
    ITEM_ARTIFACT_LADYS_ESCORT = 536,
    ITEM_RARE_CLANKERS_AMULET = 537,
    ITEM_RARE_LIETENANTS_CUTLASS = 538,
    ITEM_RARE_MEDUSAS_MIRROR = 539,
    ITEM_RARE_LADY_CARMINES_DAGGER = 540,
    ITEM_RARE_VILLAINS_BLADE = 541,
    ITEM_RARE_PERFECT_BOW = 542,
    ITEM_RARE_PERFECT_BOW_FIXED = 543,
    ITEM_RARE_SHADOWS_MASK = 544,       // 220
    ITEM_RARE_GHOST_RING = 545,         // 220
    ITEM_RARE_FAERIE_RING = 546,        // 220
    ITEM_RARE_SUN_CLOAK = 547,          // 223
    ITEM_RARE_MOON_CLOAK = 548,         // 224
    ITEM_RARE_ZOKKARS_AXE = 549,        // 224
    ITEM_RARE_VAMPIRES_CAPE = 550,      // 226
    ITEM_RARE_MINOTAURS_AXE = 551,      // 226
    ITEM_RARE_GROGNARDS_CUTLASS = 552,  // 226
    ITEM_LICH_JAR_FULL = 601,
    ITEM_WETSUIT = 604,
    ITEM_LICH_JAR_EMPTY = 615,
    ITEM_GENIE_LAMP = 616,

    ITEM_RED_APPLE = 630,

    ITEM_LUTE = 632,
    ITEM_FAERIE_PIPES = 633,
    ITEM_GRYPHONHEARTS_TRUMPET = 634,

    ITEM_HORSESHOE = 646,

    ITEM_TEMPLE_IN_A_BOTTLE = 650,

    ITEM_RECIPE_REJUVENATION = 740,
    ITEM_RECIPE_BODY_RESISTANCE = 771,
};

/*  331 */
enum ITEM_EQUIP_TYPE : uint8_t {
    EQUIP_SINGLE_HANDED = 0,
    EQUIP_TWO_HANDED = 1,
    EQUIP_BOW = 2,
    EQUIP_ARMOUR = 3,
    EQUIP_SHIELD = 4,
    EQUIP_HELMET = 5,
    EQUIP_BELT = 6,
    EQUIP_CLOAK = 7,
    EQUIP_GAUNTLETS = 8,
    EQUIP_BOOTS = 9,
    EQUIP_RING = 10,
    EQUIP_AMULET = 11,
    EQUIP_WAND = 12,
    EQUIP_REAGENT = 13,
    EQUIP_POTION = 14,
    EQUIP_SPELL_SCROLL = 15,
    EQUIP_BOOK = 16,
    EQIUP_ANY = 16,
    EQUIP_MESSAGE_SCROLL = 17,
    EQUIP_GOLD = 18,
    EQUIP_GEM = 19,
    EQUIP_NONE = 20
};

struct Player;

typedef struct CEnchantment {
    CEnchantment(int bonus, unsigned __int16 Player::*skillPtr = nullptr)
        : statPtr(skillPtr), statBonus(bonus) {}

    uint16_t Player::*statPtr;
    int statBonus;
} CEnchantment;

/*   64 */
#pragma pack(push, 1)
struct ItemGen {  // 0x24
    static void AddToMap(
        std::map<int, std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment *> *> &maptoadd,
        int enchId, CHARACTER_ATTRIBUTE_TYPE attrId, int bonusValue = 0,
        uint16_t Player::*skillPtr = nullptr);

    static std::map<int, std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment *> *> regularBonusMap;
    static std::map<int, std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment *> *> specialBonusMap;
    static std::map<int, std::map<CHARACTER_ATTRIBUTE_TYPE, CEnchantment *> *> artifactBonusMap;

    static void PopulateSpecialBonusMap();
    static void PopulateRegularBonusMap();
    static void PopulateArtifactBonusMap();
    static void ClearItemBonusMaps();

    inline void ResetEnchantAnimation() { uAttributes &= 0xFFFFFF0F; }
    inline bool ItemEnchanted() const {
        return (uAttributes & ITEM_ENCHANT_ANIMATION) != 0;
    }
    inline bool AuraEffectRed() const {
        return (uAttributes & ITEM_ENCHANT_ANIMATION) == ITEM_AURA_EFFECT_RED;
    }
    inline bool AuraEffectBlue() const {
        return (uAttributes & ITEM_ENCHANT_ANIMATION) == ITEM_AURA_EFFECT_BLUE;
    }
    inline bool AuraEffectGreen() const {
        return (uAttributes & ITEM_ENCHANT_ANIMATION) == ITEM_AURA_EFFECT_GREEN;
    }
    inline bool AuraEffectPurple() const {
        return (uAttributes & ITEM_ENCHANT_ANIMATION) ==
               ITEM_AURA_EFFECT_PURPLE;
    }

    void GetItemBonusSpecialEnchantment(Player *owner,
                                        CHARACTER_ATTRIBUTE_TYPE attrToGet,
                                        int *additiveBonus,
                                        int *halfSkillBonus);
    void GetItemBonusArtifact(Player *owner, CHARACTER_ATTRIBUTE_TYPE attrToGet,
                              int *bonusSum);
    bool IsRegularEnchanmentForAttribute(CHARACTER_ATTRIBUTE_TYPE attrToGet);

    inline bool IsBroken() { return (uAttributes & ITEM_BROKEN) != 0; }
    inline void SetBroken() { uAttributes |= ITEM_BROKEN; }
    inline bool IsIdentified() { return (uAttributes & ITEM_IDENTIFIED) != 0; }
    inline void SetIdentified() { uAttributes |= ITEM_IDENTIFIED; }
    inline bool IsStolen() { return (uAttributes & ITEM_STOLEN) != 0; }
    inline void SetStolen() { uAttributes |= ITEM_STOLEN; }

    bool GenerateArtifact();
    unsigned int GetValue();
    String GetDisplayName();
    String GetIdentifiedName();
    void UpdateTempBonus(GameTime time);
    void Reset();
    int _439DF3_get_additional_damage(DAMAGE_TYPE *a2, bool *vampiyr);

    ITEM_EQUIP_TYPE GetItemEquipType();
    unsigned char GetPlayerSkillType();
    char *GetIconName();
    unsigned __int8 GetDamageDice();
    unsigned __int8 GetDamageRoll();
    unsigned __int8 GetDamageMod();
    bool MerchandiseTest(int _2da_idx);
    int uItemID;                // 0
    int uEnchantmentType;       // 4
    int m_enchantmentStrength;  // 8
    ITEM_ENCHANTMENT
        special_enchantment;  // 25  +5 levels //0c
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
    int uNumCharges;              // 10
    unsigned int uAttributes;     // 14
    unsigned __int8 uBodyAnchor;  // 18
    char uMaxCharges;             // 19
    char uHolderPlayer;           // 1A
    char field_1B;                // 1B
    GameTime uExpireTime;        // unsigned __int64 uExpireTime; //1C
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
    unsigned int uValue;          // 10 14
    unsigned __int16 uSpriteID;   // 14 18
    __int16 field_1A;             // 16
    signed __int16 uEquipX;       // 18  1c
    signed __int16 uEquipY;       // 1a  1e
    ITEM_EQUIP_TYPE uEquipType;   // 1c 20
    unsigned __int8 uSkillType;   // 1d 21
    unsigned __int8 uDamageDice;  // 1e 22
    unsigned __int8 uDamageRoll;  // 1f 23
    unsigned __int8 uDamageMod;   // 20 24
    unsigned __int8 uMaterial;    // 21 25
    char _additional_value;       // 22 26
    char _bonus_type;             // 23  27
    char _bonus_strength;         // 24 28
    char field_25;                // 25  29
    char field_26;                // 26   2A
    char field_27;                // 27   2b
    union {
        unsigned __int8 uChanceByTreasureLvl[6];
        struct {
            unsigned __int8 uChanceByTreasureLvl1;  // 28  2c
            unsigned __int8 uChanceByTreasureLvl2;  // 29  2d
            unsigned __int8 uChanceByTreasureLvl3;  // 2A   2e
            unsigned __int8 uChanceByTreasureLvl4;  // 2B  2f
            unsigned __int8 uChanceByTreasureLvl5;  // 2C  30
            unsigned __int8 uChanceByTreasureLvl6;  // 2D  32
        };
    };
    unsigned char uItemID_Rep_St;  // 2e 32
    char field_2f;
};
#pragma pack(pop)

/*  177 */
#pragma pack(push, 1)
struct ItemEnchantment {  // Bonus|Sta|Of
                          // Name|Arm|Shld|Helm|Belt|Cape|Gaunt|Boot|Ring|Amul
    char *pBonusStat;
    char *pOfName;
    /*	union{
                    struct {
                            unsigned char to_arm;
                            unsigned char to_shld;
                            unsigned char to_helm;
                            unsigned char to_belt;
                            unsigned char to_cape;
                            unsigned char to_gaunt;
                            unsigned char to_boot;
                            unsigned char to_ring;
                            unsigned char to_amul;
                            }; */
    unsigned char to_item[12];
    // };
};
#pragma pack(pop)

/*  178 */
#pragma pack(push, 1)
struct ItemSpecialEnchantment {  // 1Ch
                                 // Bonus Stat|Name
    // Add|W1|W2|Miss|Arm|Shld|Helm|Belt|Cape|Gaunt|Boot|Ring|Amul|Value|Lvl|Description
    // fo special Bonuses and values

    char *pBonusStatement;   // 0
    char *pNameAdd;          // 4
    char to_item_apply[12];  // 8
    int iValue;              // 14
    int iTreasureLevel;      // 18
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BonusRange {
    unsigned int minR;
    unsigned int maxR;
};
#pragma pack(pop)

/*  176 */
#pragma pack(push, 1)
struct ItemsTable {
    void Initialize();
    void LoadPotions();
    void LoadPotionNotes();
    void GenerateItem(int treasure_level, unsigned int uTreasureType,
                      ItemGen *pItem);
    void SetSpecialBonus(ItemGen *pItem);
    bool IsMaterialSpecial(ItemGen *pItem);
    bool IsMaterialNonCommon(ItemGen *pItem);
    void Release();

    int uAllItemsCount;
    NZIArray<ItemDesc, 800> pItems;                   // 4-9604h
    ItemEnchantment pEnchantments[24];                // 9604h
    ItemSpecialEnchantment pSpecialEnchantments[72];  // 97E4h -9FC4h
    char field_9FC4[5000];
    char field_B348[5000];
    char field_C6D0[5000];
    char field_DA58[5000];
    char field_EDE0[384];
    unsigned __int16 potion_data[50][50];  // 77B2h*2=EF64h  -102ECh
    unsigned __int16 potion_note[50][50];  // 8176h*2=102ECh -11674h
    char *pItemsTXT_Raw;                   // 11674h
    char *pRndItemsTXT_Raw;
    char *pStdItemsTXT_Raw;                     // 1167Ch
    char *pSpcItemsTXT_Raw;                     // 11680h
    unsigned int uChanceByTreasureLvlSumm[6];   // 11684
    unsigned int uBonusChanceStandart[6];       // 1169c
    unsigned int uBonusChanceSpecial[6];        // 116B4
    unsigned int uBonusChanceWpSpecial[6];      // 116cc -116e4
    unsigned int pEnchantmentsSumm[9];          // 116E4h -11708h
    BonusRange bonus_ranges[6];                 // 45C2h*4 =11708h
    unsigned int pSpecialEnchantmentsSumm[24];  // 11738h
    unsigned int pSpecialEnchantments_count;    // 11798h
    char field_1179C;
    char field_1179D;
    char field_1179E;
    char field_1179F;
};
#pragma pack(pop)

extern std::array<const char, 5> uItemsAmountPerShopType;
extern ItemGen *ptr_50C9A4_ItemToEnchant;

extern struct ItemsTable *pItemsTable;

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

    int field_0_expire_second;
    int field_4_expire_minute;
    int field_8_expire_hour;
    int field_C_expire_day;
    int field_10_expire_week;
    int field_14_exprie_month;
    int field_18_expire_year;
};
#pragma pack(pop)

int GetItemTextureFilename(char *pOut, signed int item_id, int index,
                           int shoulder);
