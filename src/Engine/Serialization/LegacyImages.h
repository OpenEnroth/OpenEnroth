#pragma once

#include "Engine/Spells/Spells.h"
#include "Engine/Objects/Items.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/SpriteObject.h"
#include "GUI/GUIFont.h"

#include "Utility/Geometry/Plane.h"
#include "Utility/Geometry/BBox.h"

/*
 * Party, timers, NPCs and other stuff is binary dumped into resources/savegame
 * files, which limits ability to alter these structures without breaking
 * compatibility. This module is used to serialize and deserialize native game
 * structures intro their current representations without breaking compatibility
 * with original files.
 */

struct BLVDoor;
struct BLVFace;
struct BLVSector;
struct NPCData;
struct ItemGen;
struct SpellBuff;
struct Player;
class UIAnimation;
class Icon;
struct OtherOverlayList;
struct Timer;
struct Party;
struct MonsterDesc;
struct ODMFace;
class SpriteFrame;

#pragma pack(push, 1)

struct SpriteFrame_MM6 {
    std::array<char, 12> pIconName;
    std::array<char, 12> pTextureName;    // c
    std::array<int16_t, 8> pHwSpriteIDs;  // 18h
    int32_t scale;            // 28h
    int32_t uFlags;               // 2c
    int16_t uGlowRadius;      // 30
    int16_t uPaletteID;       // 32
    int16_t uPaletteIndex;
    int16_t uAnimTime;
};
static_assert(sizeof(SpriteFrame_MM6) == 56);


struct SpriteFrame_MM7 : public SpriteFrame_MM6 {
    int16_t uAnimLength;
    int16_t _pad;
};
static_assert(sizeof(SpriteFrame_MM7) == 60);

void Deserialize(const SpriteFrame_MM7 &src, SpriteFrame *dst);


struct BLVFace_MM7 {
    Planef pFacePlane;
    Planei pFacePlane_old;
    int32_t zCalc1;
    int32_t zCalc2;
    int32_t zCalc3;
    uint32_t uAttributes;
    int32_t pVertexIDs;
    int32_t pXInterceptDisplacements;
    int32_t pYInterceptDisplacements;
    int32_t pZInterceptDisplacements;
    int32_t pVertexUIDs;
    int32_t pVertexVIDs;
    uint16_t uFaceExtraID;
    uint16_t uBitmapID;
    uint16_t uSectorID;
    int16_t uBackSectorID;
    BBoxs pBounding;
    uint8_t uPolygonType;
    uint8_t uNumVertices;
    char field_5E;
    char field_5F;
};
static_assert(sizeof(BLVFace_MM7) == 0x60);

void Deserialize(const BLVFace_MM7 &src, BLVFace *dst);


struct TileDesc_MM7 {
    std::array<char, 16> pTileName;
    uint16_t uTileID;
    uint16_t uBitmapID;
    uint16_t tileset;
    uint16_t uSection;
    uint16_t uAttributes;
};
static_assert(sizeof(TileDesc_MM7) == 26);


struct TextureFrame_MM7 {
    std::array<char, 12> pTextureName;
    int16_t uTextureID;
    int16_t uAnimTime;
    int16_t uAnimLength;
    int16_t uFlags;
};
static_assert(sizeof(TextureFrame_MM7) == 20);


struct NPCData_MM7 {
    /* 00 */ int32_t pName;  // char *pName;
    /* 04 */ uint32_t uPortraitID;
    /* 08 */ uint32_t uFlags;  // & 0x80    no greeting on dialogue start; looks like hired
    /* 0C */ int32_t fame;
    /* 10 */ int32_t rep;
    /* 14 */ uint32_t Location2D;
    /* 18 */ uint32_t uProfession;
    /* 1C */ int32_t greet;
    /* 20 */ int32_t joins;
    /* 24 */ int32_t field_24;
    /* 28 */ uint32_t evt_A;
    /* 2C */ uint32_t evt_B;
    /* 30 */ uint32_t evt_C;
    /* 34 */ uint32_t evt_D;
    /* 38 */ uint32_t evt_E;
    /* 3C */ uint32_t evt_F;
    /* 40 */ uint32_t uSex;
    /* 44 */ int32_t bHasUsedTheAbility;
    /* 48 */ int32_t news_topic;
    /* 4C */
};
static_assert(sizeof(NPCData_MM7) == 0x4C);

void Serialize(const NPCData &src, NPCData_MM7 *dst);
void Deserialize(const NPCData_MM7 &src, NPCData *dst);


struct ItemGen_MM7 {
    /* 00 */ int32_t uItemID;
    /* 04 */ int32_t uEnchantmentType;
    /* 08 */ int32_t m_enchantmentStrength;
    /* 0C */ int32_t special_enchantment;
    /* 10 */ int32_t uNumCharges;
    /* 14 */ uint32_t uAttributes;
    /* 18 */ uint8_t uBodyAnchor;
    /* 19 */ uint8_t uMaxCharges;
    /* 1A */ uint8_t uHolderPlayer;
    /* 1B */ uint8_t placedInChest; // unknown unused 8-bit field, was repurposed
    /* 1C */ uint64_t uExpireTime;
};
static_assert(sizeof(ItemGen_MM7) == 0x24);

void Serialize(const ItemGen &src, ItemGen_MM7 *dst);
void Deserialize(const ItemGen_MM7 &src, ItemGen *dst);


struct SpellBuff_MM7 {
    /* 00 */ int64_t uExpireTime;
    /* 08 */ uint16_t uPower;
    /* 0A */ uint16_t uSkillMastery;
    /* 0C */ uint16_t uOverlayID;
    /* 0E */ uint8_t uCaster;
    /* 0F */ uint8_t uFlags;
    /* 10 */
};
static_assert(sizeof(SpellBuff_MM7) == 0x10);

void Serialize(const SpellBuff &src, SpellBuff_MM7 *dst);
void Deserialize(const SpellBuff_MM7 &src, SpellBuff *dst);


struct PlayerSpellbookChapter_MM7 {
    /* 00 */ std::array<char, 11> bIsSpellAvailable;
    /* 0B */
};
static_assert(sizeof(PlayerSpellbookChapter_MM7) == 0xB);


struct PlayerSpells_MM7 {
    union {
        struct {
            /* 00 */ std::array<PlayerSpellbookChapter_MM7, 9> pChapters;
            /* 63 */ char _pad1;
            /* 64 */
        };
        struct {
            /* 00 */ std::array<char, 99> bHaveSpell;
            /* 63 */ char _pad2;
            /* 64 */
        };
    };
};
static_assert(sizeof(PlayerSpells_MM7) == 0x64);


union PlayerEquipment_MM7 {
    union {
        struct {
            /* 00 */ uint32_t uOffHand;
            /* 04 */ uint32_t uMainHand;
            /* 08 */ uint32_t uBow;
            /* 0C */ uint32_t uArmor;
            /* 10 */ uint32_t uHelm;
            /* 14 */ uint32_t uBelt;
            /* 18 */ uint32_t uCloak;
            /* 1C */ uint32_t uGlove;
            /* 20 */ uint32_t uBoot;
            /* 24 */ uint32_t uAmulet;
            /* 28 */ std::array<uint32_t, 6> uRings;
            /* 40 */
        };
        std::array<uint32_t, 16> pIndices;
    };
};
static_assert(sizeof(PlayerEquipment_MM7) == 0x40);


struct LloydBeacon_MM7 {
    /* 00 */ uint64_t uBeaconTime;
    /* 08 */ int32_t PartyPos_X;
    /* 0C */ int32_t PartyPos_Y;
    /* 10 */ int32_t PartyPos_Z;
    /* 14 */ int16_t _partyViewYaw;
    /* 16 */ int16_t _partyViewPitch;
    /* 18 */ uint16_t unknown;
    /* 1A */ uint16_t SaveFileID;
    /* 1C */
};
static_assert(sizeof(LloydBeacon_MM7) == 0x1C);


struct Player_MM7 {
    /* 0000 */ std::array<int64_t, 20> pConditions;
    /* 00A0 */ uint64_t uExperience;
    /* 00A8 */ std::array<char, 16> pName;
    /* 00B8 */ uint8_t uSex;
    /* 00B9 */ uint8_t classType;
    /* 00BA */ uint8_t uCurrentFace;
    /* 00BB */ char field_BB;
    /* 00BC */ uint16_t uMight;
    /* 00BE */ uint16_t uMightBonus;
    /* 00C0 */ uint16_t uIntelligence;
    /* 00C2 */ uint16_t uIntelligenceBonus;
    /* 00C4 */ uint16_t uWillpower;
    /* 00C6 */ uint16_t uWillpowerBonus;
    /* 00C8 */ uint16_t uEndurance;
    /* 00CA */ uint16_t uEnduranceBonus;
    /* 00CC */ uint16_t uSpeed;
    /* 00CE */ uint16_t uSpeedBonus;
    /* 00D0 */ uint16_t uAccuracy;
    /* 00D2 */ uint16_t uAccuracyBonus;
    /* 00D4 */ uint16_t uLuck;
    /* 00D6 */ uint16_t uLuckBonus;
    /* 00D8 */ int16_t sACModifier;
    /* 00DA */ uint16_t uLevel;
    /* 00DC */ int16_t sLevelModifier;
    /* 00DE */ int16_t sAgeModifier;
    /* 00E0 */ int32_t field_E0;
    /* 00E4 */ int32_t field_E4;
    /* 00E8 */ int32_t field_E8;
    /* 00EC */ int32_t field_EC;
    /* 00F0 */ int32_t field_F0;
    /* 00F4 */ int32_t field_F4;
    /* 00F8 */ int32_t field_F8;
    /* 00FC */ int32_t field_FC;
    /* 0100 */ int32_t field_100;
    /* 0104 */ int32_t field_104;
    /* 0108 */ std::array<uint16_t, 37> pActiveSkills;
    /* 0152 */ std::array<uint8_t, 64> _achieved_awards_bits;
    /* 0192 */ PlayerSpells_MM7 spellbook;
    /* 01F6 */ std::array<char, 2> _1F6_pad;
    /* 01F8 */ int32_t pure_luck_used;
    /* 01FC */ int32_t pure_speed_used;
    /* 0200 */ int32_t pure_intellect_used;
    /* 0204 */ int32_t pure_endurance_used;
    /* 0208 */ int32_t pure_willpower_used;
    /* 020C */ int32_t pure_accuracy_used;
    /* 0210 */ int32_t pure_might_used;
    /* 0214 */ union {
        struct {
            std::array<ItemGen_MM7, 126> pInventoryItemList;
            std::array<ItemGen_MM7, 12> pEquippedItems;
        };
        struct {
            std::array<ItemGen_MM7, 138> pOwnItems;
        };
    };
    /* 157C */ std::array<int32_t, 126> pInventoryMatrix;
    /* 1774 */ int16_t sResFireBase;
    /* 1776 */ int16_t sResAirBase;
    /* 1778 */ int16_t sResWaterBase;
    /* 177A */ int16_t sResEarthBase;
    /* 177C */ int16_t sResPhysicalBase;
    /* 177E */ int16_t sResMagicBase;
    /* 1780 */ int16_t sResSpiritBase;
    /* 1782 */ int16_t sResMindBase;
    /* 1784 */ int16_t sResBodyBase;
    /* 1786 */ int16_t sResLightBase;
    /* 1788 */ int16_t sResDarkBase;
    /* 178A */ int16_t sResFireBonus;
    /* 178C */ int16_t sResAirBonus;
    /* 178E */ int16_t sResWaterBonus;
    /* 1790 */ int16_t sResEarthBonus;
    /* 1792 */ int16_t sResPhysicalBonus;
    /* 1794 */ int16_t sResMagicBonus;
    /* 1796 */ int16_t sResSpiritBonus;
    /* 1798 */ int16_t sResMindBonus;
    /* 179A */ int16_t sResBodyBonus;
    /* 179C */ int16_t sResLightBonus;
    /* 179E */ int16_t sResDarkBonus;
    /* 17A0 */ std::array<SpellBuff_MM7, 24> pPlayerBuffs;
    /* 1920 */ uint32_t uVoiceID;
    /* 1924 */ int32_t uPrevVoiceID;
    /* 1928 */ int32_t uPrevFace;
    /* 192C */ int32_t field_192C;
    /* 1930 */ int32_t field_1930;
    /* 1934 */ uint16_t uTimeToRecovery;
    /* 1936 */ char field_1936;
    /* 1937 */ char field_1937;
    /* 1938 */ uint32_t uSkillPoints;
    /* 193C */ int32_t sHealth;
    /* 1940 */ int32_t sMana;
    /* 1944 */ uint32_t uBirthYear;
    /* 1948 */ PlayerEquipment_MM7 pEquipment;
    /* 1988 */ std::array<int32_t, 49> field_1988;
    /* 1A4C */ char field_1A4C;
    /* 1A4D */ char field_1A4D;
    /* 1A4E */ char lastOpenedSpellbookPage;
    /* 1A4F */ uint8_t uQuickSpell;
    /* 1A50 */ std::array<char, 64> playerEventBits;
    /* 1A90 */ char _some_attack_bonus;
    /* 1A91 */ char field_1A91;
    /* 1A92 */ char _melee_dmg_bonus;
    /* 1A93 */ char field_1A93;
    /* 1A94 */ char _ranged_atk_bonus;
    /* 1A95 */ char field_1A95;
    /* 1A96 */ char _ranged_dmg_bonus;
    /* 1A97 */ char field_1A97;
    /* 1A98 */ char uFullHealthBonus;
    /* 1A99 */ char _health_related;
    /* 1A9A */ char uFullManaBonus;
    /* 1A9B */ char _mana_related;
    /* 1A9C */ uint16_t expression;
    /* 1A9E */ uint16_t uExpressionTimePassed;
    /* 1AA0 */ uint16_t uExpressionTimeLength;
    /* 1AA2 */ int16_t field_1AA2;
    /* 1AA4 */ int32_t _expression21_animtime;
    /* 1AA8 */ int32_t _expression21_frameset;
    /* 1AAC */ std::array<LloydBeacon_MM7, 5> pInstalledBeacons;
    /* 1B38 */ char uNumDivineInterventionCastsThisDay;
    /* 1B39 */ char uNumArmageddonCasts;
    /* 1B3A */ char uNumFireSpikeCasts;
    /* 1B3B */ char field_1B3B;
    /* 1B3C */
};
static_assert(sizeof(Player_MM7) == 0x1B3C);

void Serialize(const Player &src, Player_MM7 *dst);
void Deserialize(const Player_MM7 &src, Player *dst);


struct PartyTimeStruct_MM7 {
    /* 000 */ std::array<int64_t, 10> bountyHunting_next_generation_time;
    /* 050 */ std::array<int64_t, 85> Shops_next_generation_time;  // field_50
    /* 2F8 */ std::array<int64_t, 53> _shop_ban_times;
    /* 4A0 */ std::array<uint64_t, 10> CounterEventValues;               // (0xACD314h in Silvo's binary)
    /* 4F0 */ std::array<int64_t, 29> HistoryEventTimes;  // (0xACD364h in Silvo's binary)
    /* 5D8 */ std::array<uint64_t, 20> _s_times;  // 5d8 440h+8*51     //(0xACD44Ch in Silvo's binary)
    /* 678 */
};
static_assert(sizeof(PartyTimeStruct_MM7) == 0x678);


struct Party_MM7 {
    /* 00000 */ int32_t field_0;
    /* 00004 */ uint32_t uPartyHeight;
    /* 00008 */ uint32_t uDefaultPartyHeight;
    /* 0000C */ int32_t sEyelevel;
    /* 00010 */ uint32_t uDefaultEyelevel;
    /* 00014 */ int32_t radius;
    /* 00018 */ int32_t _yawGranularity;
    /* 0001C */ uint32_t uWalkSpeed;
    /* 00020 */ int32_t _yawRotationSpeed;  // deg/s
    /* 00024 */ int32_t jump_strength;
    /* 00028 */ int32_t field_28;
    /* 0002C */ uint64_t uTimePlayed;
    /* 00034 */ int64_t uLastRegenerationTime;
    /* 0003C */ PartyTimeStruct_MM7 PartyTimes;
    /* 006B4 */ Vec3i vPosition;
    /* 006C0 */ int32_t _viewYaw;
    /* 006C4 */ int32_t _viewPitch;
    /* 006C8 */ Vec3i vPrevPosition;
    /* 006D4 */ int32_t _viewPrevYaw;
    /* 006D8 */ int32_t _viewPrevPitch;
    /* 006DC */ int32_t sPrevEyelevel;
    /* 006E0 */ int32_t field_6E0;
    /* 006E4 */ int32_t field_6E4;
    /* 006E8 */ int32_t uFallSpeed;
    /* 006EC */ int32_t field_6EC;
    /* 006F0 */ int32_t field_6F0;
    /* 006F4 */ int32_t floor_face_pid;  // face we are standing at
    /* 006F8 */ int32_t walk_sound_timer;
    /* 006FC */ int32_t _6FC_water_lava_timer;
    /* 00700 */ int32_t uFallStartZ;
    /* 00704 */ uint32_t bFlying;
    /* 00708 */ char field_708;
    /* 00709 */ uint8_t hirelingScrollPosition;
    /* 0070A */ char field_70A;
    /* 0070B */ char field_70B;
    /* 0070C */ uint32_t uCurrentYear;
    /* 00710 */ uint32_t uCurrentMonth;
    /* 00714 */ uint32_t uCurrentMonthWeek;
    /* 00718 */ uint32_t uCurrentDayOfMonth;
    /* 0071C */ uint32_t uCurrentHour;
    /* 00720 */ uint32_t uCurrentMinute;
    /* 00724 */ uint32_t uCurrentTimeSecond;
    /* 00728 */ uint32_t uNumFoodRations;
    /* 0072C */ int32_t field_72C;
    /* 00730 */ int32_t field_730;
    /* 00734 */ uint32_t uNumGold;
    /* 00738 */ uint32_t uNumGoldInBank;
    /* 0073C */ uint32_t uNumDeaths;
    /* 00740 */ int32_t field_740;
    /* 00744 */ int32_t uNumPrisonTerms;
    /* 00748 */ uint32_t uNumBountiesCollected;
    /* 0074C */ int field_74C;
    /* 00750 */ std::array<int16_t, 5> monster_id_for_hunting;
    /* 0075A */ std::array<int16_t, 5> monster_for_hunting_killed;
    /* 00764 */ uint8_t days_played_without_rest;
    /* 00765 */ std::array<uint8_t, 64> _quest_bits;
    /* 007A5 */ std::array<uint8_t, 16> pArcomageWins;
    /* 007B5 */ char field_7B5_in_arena_quest;
    /* 007B6 */ std::array<char, 4> uNumArenaWins;
    /* 007BA */ std::array<char, 29> pIsArtifactFound;  // 7ba
    /* 007D7 */ std::array<char, 39> field_7d7;
    /* 007FE */ std::array<uint8_t, 26> _autonote_bits;
    /* 00818 */ std::array<char, 60> field_818;
    /* 00854 */ std::array<char, 32> field_854;
    /* 00874 */ int32_t uNumArcomageWins;
    /* 00878 */ int32_t uNumArcomageLoses;
    /* 0087C */ uint32_t bTurnBasedModeOn;
    /* 00880 */ int32_t field_880;
    /* 00884 */ int32_t uFlags2;
    /* 00888 */ uint32_t alignment;
    /* 0088C */ std::array<SpellBuff_MM7, 20> pPartyBuffs;
    /* 00954 */ std::array<Player_MM7, 4> pPlayers;
    /* 07644 */ std::array<NPCData_MM7, 2> pHirelings;
    /* 07754 */ ItemGen_MM7 pPickedItem;
    /* 07778 */ uint32_t uFlags;
    /* 0777C */ std::array<std::array<ItemGen_MM7, 12>, 53> StandartItemsInShops;
    /* 0D0EC */ std::array<std::array<ItemGen_MM7, 12>, 53> SpecialItemsInShops;
    /* 12A5C */ std::array<std::array<ItemGen_MM7, 12>, 32> SpellBooksInGuilds;
    /* 1605C */ std::array<char, 24> field_1605C;
    /* 16074 */ std::array<char, 100> pHireling1Name;
    /* 160D8 */ std::array<char, 100> pHireling2Name;
    /* 1613C */ int32_t armageddon_timer;
    /* 16140 */ int32_t armageddonDamage;
    /* 16144 */ std::array<int32_t, 4> pTurnBasedPlayerRecoveryTimes;
    /* 16154 */ std::array<int32_t, 53> InTheShopFlags;
    /* 16228 */ int32_t uFine;
    /* 1622C */ float flt_TorchlightColorR;
    /* 16230 */ float flt_TorchlightColorG;
    /* 16234 */ float flt_TorchlightColorB;
    /* 16238 */
};
static_assert(sizeof(Party_MM7) == 0x16238);

void Serialize(const Party &src, Party_MM7 *dst);
void Deserialize(const Party_MM7 &src, Party *dst);

struct Timer_MM7 {
    /* 00 */ uint32_t bReady;
    /* 04 */ uint32_t bPaused;
    /* 08 */ int32_t bTackGameTime;
    /* 0C */ uint32_t uStartTime;
    /* 10 */ uint32_t uStopTime;
    /* 14 */ int32_t uGameTimeStart;
    /* 18 */ int32_t field_18;
    /* 1C */ uint32_t uTimeElapsed;
    /* 20 */ int32_t dt_fixpoint;
    /* 24 */ uint32_t uTotalGameTimeElapsed;
    /* 28 */
};
static_assert(sizeof(Timer_MM7) == 0x28);

void Serialize(const Timer &src, Timer_MM7 *dst);
void Deserialize(const Timer_MM7 &src, Timer *dst);


struct OtherOverlay_MM7 {
    /* 00 */ int16_t field_0;
    /* 02 */ int16_t field_2;
    /* 04 */ int16_t sprite_frame_time;
    /* 06 */ int16_t field_6;
    /* 08 */ int16_t screen_space_x;
    /* 0A */ int16_t screen_space_y;
    /* 0C */ int16_t field_C;
    /* 0E */ int16_t field_E;
    /* 10 */ int32_t field_10;
    /* 14 */
};
static_assert(sizeof(OtherOverlay_MM7) == 0x14);


struct OtherOverlayList_MM7 {
    /* 000 */ std::array<OtherOverlay_MM7, 50> pOverlays;
    /* 3E8 */ int32_t field_3E8;
    /* 3EC */ int32_t bRedraw;
    /* 3F0 */
};
static_assert(sizeof(OtherOverlayList_MM7) == 0x3F0);

void Serialize(const OtherOverlayList &src, OtherOverlayList_MM7 *dst);
void Deserialize(const OtherOverlayList_MM7 &src, OtherOverlayList *dst);


struct IconFrame_MM7 {
    /* 000 */ std::array<char, 12> pAnimationName;
    /* 00C */ std::array<char, 12> pTextureName;
    /* 018 */ int16_t uAnimTime;
    /* 01A */ int16_t uAnimLength;
    /* 01C */ int16_t uFlags;  // 0x01 - more icons in this animation
    /* 01E */ uint16_t uTextureID;
};
static_assert(sizeof(IconFrame_MM7) == 0x20);

void Serialize(const Icon &src, IconFrame_MM7 *dst);
void Deserialize(const IconFrame_MM7 &src, Icon *dst);


struct UIAnimation_MM7 {
    /* 000 */ uint16_t uIconID;
    /* 002 */ int16_t field_2;
    /* 004 */ int16_t uAnimTime;
    /* 006 */ int16_t uAnimLength;
    /* 008 */ int16_t x;
    /* 00A */ int16_t y;
    /* 00C */ char field_C;
};
static_assert(sizeof(UIAnimation_MM7) == 0xD);

void Serialize(const UIAnimation &src, UIAnimation_MM7 *dst);
void Deserialize(const UIAnimation_MM7 &src, UIAnimation *dst);


struct MonsterInfo_MM7 {
    int32_t pName;
    int32_t pPictureName;
    uint8_t uLevel;
    uint8_t uTreasureDropChance;
    uint8_t uTreasureDiceRolls;
    uint8_t uTreasureDiceSides;
    uint8_t uTreasureLevel;
    uint8_t uTreasureType;
    uint8_t uFlying;
    uint8_t uMovementType;
    uint8_t uAIType;
    uint8_t uHostilityType;
    char field_12;
    uint8_t uSpecialAttackType;
    uint8_t uSpecialAttackLevel;
    uint8_t uAttack1Type;
    uint8_t uAttack1DamageDiceRolls;
    uint8_t uAttack1DamageDiceSides;
    uint8_t uAttack1DamageBonus;
    uint8_t uMissleAttack1Type;
    uint8_t uAttack2Chance;
    uint8_t uAttack2Type;
    uint8_t uAttack2DamageDiceRolls;
    uint8_t uAttack2DamageDiceSides;
    uint8_t uAttack2DamageBonus;
    uint8_t uMissleAttack2Type;
    uint8_t uSpell1UseChance;
    uint8_t uSpell1ID;
    uint8_t uSpell2UseChance;
    uint8_t uSpell2ID;
    uint8_t uResFire;
    uint8_t uResAir;
    uint8_t uResWater;
    uint8_t uResEarth;
    uint8_t uResMind;
    uint8_t uResSpirit;
    uint8_t uResBody;
    uint8_t uResLight;
    uint8_t uResDark;
    uint8_t uResPhysical;
    uint8_t uSpecialAbilityType;
    uint8_t uSpecialAbilityDamageDiceRolls;
    uint8_t uSpecialAbilityDamageDiceSides;
    uint8_t uSpecialAbilityDamageDiceBonus;
    uint8_t uNumCharactersAttackedPerSpecialAbility;
    char field_33;
    uint16_t uID;
    uint16_t bBloodSplatOnDeath;
    uint16_t uSpellSkillAndMastery1;
    uint16_t uSpellSkillAndMastery2;
    int16_t field_3C_some_special_attack;
    int16_t field_3E;
    uint32_t uHP;
    uint32_t uAC;
    uint32_t uExp;
    uint32_t uBaseSpeed;
    int32_t uRecoveryTime;
    uint32_t uAttackPreference;
};
static_assert(sizeof(MonsterInfo_MM7) == 0x58);


struct MonsterDesc_MM6 {
    uint16_t uMonsterHeight;
    uint16_t uMonsterRadius;
    uint16_t uMovementSpeed;
    int16_t uToHitRadius;
    std::array<uint16_t, 4> pSoundSampleIDs;
    std::array<char, 32> pMonsterName;
    std::array<std::array<char, 10>, 10> pSpriteNames;
};
static_assert(sizeof(MonsterDesc_MM6) == 148);

void Deserialize(const MonsterDesc_MM6 &src, MonsterDesc *dst);


struct MonsterDesc_MM7 {
    uint16_t uMonsterHeight;
    uint16_t uMonsterRadius;
    uint16_t uMovementSpeed;
    int16_t uToHitRadius;
    uint32_t sTintColor;
    std::array<uint16_t, 4> pSoundSampleIDs;
    std::array<char, 32> pMonsterName;
    std::array<std::array<char, 10>, 10> pSpriteNames;
};
static_assert(sizeof(MonsterDesc_MM7) == 152);

void Serialize(const MonsterDesc &src, MonsterDesc_MM7 *dst);
void Deserialize(const MonsterDesc_MM7 &src, MonsterDesc *dst);


struct Actor_MM7 {
    std::array<char, 32> pActorName;
    int16_t sNPC_ID;
    int16_t field_22;
    uint32_t uAttributes;
    int16_t sCurrentHP;
    std::array<char, 2> field_2A;
    MonsterInfo_MM7 pMonsterInfo;
    int16_t word_000084_range_attack;
    int16_t word_000086_some_monster_id;  // base monster class monsterlist id
    uint16_t uActorRadius;
    uint16_t uActorHeight;
    uint16_t uMovementSpeed;
    Vec3s vPosition;
    Vec3s vVelocity;
    uint16_t uYawAngle;
    uint16_t uPitchAngle;
    int16_t uSectorID;
    uint16_t uCurrentActionLength;
    Vec3s vInitialPosition;
    Vec3s vGuardingPosition;
    uint16_t uTetherDistance;
    int16_t uAIState;
    uint16_t uCurrentActionAnimation;
    uint16_t uCarriedItemID;
    char field_B6;
    char field_B7;
    uint32_t uCurrentActionTime;
    std::array<uint16_t, 8> pSpriteIDs;
    std::array<uint16_t, 4> pSoundSampleIDs;  // 1 die     3 bored
    std::array<SpellBuff_MM7, 22> pActorBuffs;
    std::array<ItemGen_MM7, 4> ActorHasItems;
    uint32_t uGroup;
    uint32_t uAlly;
    std::array<ActorJob, 8> pScheduledJobs;
    uint32_t uSummonerID;
    uint32_t uLastCharacterIDToHit;
    int32_t dword_000334_unique_name;
    std::array<char, 12> field_338;
};
static_assert(sizeof(Actor_MM7) == 0x344);

void Serialize(const Actor &src, Actor_MM7 *dst);
void Deserialize(const Actor_MM7 &src, Actor *dst);


struct BLVDoor_MM7 {
    uint32_t uAttributes;
    uint32_t uDoorID;
    uint32_t uTimeSinceTriggered;
    Vec3i vDirection;
    uint32_t uMoveLength;
    uint32_t uOpenSpeed;
    uint32_t uCloseSpeed;
    uint32_t pVertexIDs;
    uint32_t pFaceIDs;
    uint32_t pSectorIDs;
    int32_t pDeltaUs;
    int32_t pDeltaVs;
    uint32_t pXOffsets;
    uint32_t pYOffsets;
    uint32_t pZOffsets;
    uint16_t uNumVertices;
    uint16_t uNumFaces;
    uint16_t uNumSectors;
    uint16_t uNumOffsets;
    uint16_t uState;
    int16_t field_4E;
};
static_assert(sizeof(BLVDoor_MM7) == 0x50);

void Serialize(const BLVDoor &src, BLVDoor_MM7 *dst);
void Deserialize(const BLVDoor_MM7 &src, BLVDoor *dst);


struct BLVSector_MM7 {
    int32_t field_0;
    uint16_t uNumFloors;
    int16_t field_6;
    uint32_t pFloors;
    uint16_t uNumWalls;
    int16_t field_E;
    uint32_t pWalls;
    uint16_t uNumCeilings;
    int16_t field_16;
    uint32_t pCeilings;
    uint16_t uNumFluids;
    int16_t field_1E;
    uint32_t pFluids;
    int16_t uNumPortals;
    int16_t field_26;
    uint32_t pPortals;
    uint16_t uNumFaces;
    uint16_t uNumNonBSPFaces;
    uint32_t pFaceIDs;
    uint16_t uNumCylinderFaces;
    int16_t field_36;
    int32_t pCylinderFaces;
    uint16_t uNumCogs;
    int16_t field_3E;
    uint32_t pCogs;
    uint16_t uNumDecorations;
    int16_t field_46;
    uint32_t pDecorationIDs;
    uint16_t uNumMarkers;
    int16_t field_4E;
    uint32_t pMarkers;
    uint16_t uNumLights;
    int16_t field_56;
    uint32_t pLights;
    int16_t uWaterLevel;
    int16_t uMistLevel;
    int16_t uLightDistanceMultiplier;
    int16_t uMinAmbientLightLevel;
    int16_t uFirstBSPNode;
    int16_t exit_tag;
    BBoxs pBounding;
};
static_assert(sizeof(BLVSector_MM7) == 0x74);

void Serialize(const BLVSector &src, BLVSector_MM7 *dst);
void Deserialize(const BLVSector_MM7 &src, BLVSector *dst);


struct FontData_MM7 {
    uint8_t cFirstChar;  // 0
    uint8_t cLastChar;   // 1
    uint8_t field_2;
    uint8_t field_3;
    uint8_t field_4;
    uint16_t uFontHeight;  // 5-6
    uint8_t field_7;
    uint32_t palletes_count;
    std::array<uint32_t, 5> pFontPalettes;
    std::array<GUICharMetric, 256> pMetrics;
    std::array<uint32_t, 256> font_pixels_offset;
    uint8_t pFontData[0];  // array of font pixels
};
static_assert(sizeof(FontData_MM7) == 0x1020);

void Serialize(const FontData &src, FontData_MM7 *dst);
void Deserialize(const FontData_MM7 &src, size_t size, FontData *dst);


struct ODMFace_MM7 {
    Planei pFacePlane;
    int32_t zCalc1;
    int32_t zCalc2;
    int32_t zCalc3;
    uint32_t uAttributes;
    std::array<uint16_t, 20> pVertexIDs;
    std::array<int16_t, 20> pTextureUIDs;
    std::array<int16_t, 20> pTextureVIDs;
    std::array<int16_t, 20> pXInterceptDisplacements;
    std::array<int16_t, 20> pYInterceptDisplacements;
    std::array<int16_t, 20> pZInterceptDisplacements;
    int16_t uTextureID;
    int16_t sTextureDeltaU;
    int16_t sTextureDeltaV;
    BBoxs pBoundingBox;
    int16_t sCogNumber;
    int16_t sCogTriggeredID;
    int16_t sCogTriggerType;
    char field_128;
    char field_129;
    uint8_t uGradientVertex1;
    uint8_t uGradientVertex2;
    uint8_t uGradientVertex3;
    uint8_t uGradientVertex4;
    uint8_t uNumVertices;
    uint8_t uPolygonType;
    uint8_t uShadeType;
    uint8_t bVisible;
    char field_132;
    char field_133;
};
static_assert(sizeof(ODMFace_MM7) == 308);

void Deserialize(const ODMFace_MM7 &src, ODMFace *dst);


struct SpawnPoint_MM6 {
    Vec3i vPosition;
    uint16_t uRadius;
    uint16_t uKind;
    uint16_t uIndex;
    uint16_t uAttributes;
};
static_assert(sizeof(SpawnPoint_MM6) == 20);

struct SpawnPoint_MM7 {
    Vec3i vPosition;
    uint16_t uRadius;
    uint16_t uKind;
    uint16_t uIndex;
    uint16_t uAttributes;
    unsigned int uGroup;
};
static_assert(sizeof(SpawnPoint_MM7) == 24);

void Deserialize(const SpawnPoint_MM7 &src, SpawnPoint *dst);


struct SpriteObject_MM7 {
    uint16_t uType;
    uint16_t uObjectDescID;
    Vec3i vPosition;
    Vec3s vVelocity;
    uint16_t uFacing;
    uint16_t uSoundID;
    uint16_t uAttributes;
    int16_t uSectorID;
    uint16_t uSpriteFrameID;
    int16_t tempLifetime;
    int16_t field_22_glow_radius_multiplier;
    ItemGen_MM7 containing_item;
    int uSpellID;
    int spell_level;
    int32_t spell_skill;
    int field_54;
    int spell_caster_pid;
    int spell_target_pid;
    char field_60_distance_related_prolly_lod;
    char field_61;
    char field_62[2];
    Vec3i initialPosition;
};

static_assert(sizeof(SpriteObject_MM7) == 0x70);

void Serialize(const SpriteObject &src, SpriteObject_MM7 *dst);
void Deserialize(const SpriteObject_MM7 &src, SpriteObject *dst);

#pragma pack(pop)
