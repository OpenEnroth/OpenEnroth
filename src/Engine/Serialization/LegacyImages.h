#pragma once

#include <array>

#include "Utility/Geometry/Plane.h"
#include "Utility/Geometry/BBox.h"

#include "Library/Binary/BinarySerialization.h"

#include "MultiStageSerialization.h"
#include "CommonImages.h"

/*
 * Party, timers, NPCs and other stuff is binary dumped into resources/savegame
 * files, which limits ability to alter these structures without breaking
 * compatibility. This module is used to serialize and deserialize native game
 * structures intro their current representations without breaking compatibility
 * with original files.
 */

class Actor;
class Icon;
class SpriteFrame;
class TextureFrame;
class SoundInfo;
class TileDesc;
class UIAnimation;
struct ActorJob;
struct BLVDoor;
struct BLVFace;
struct BLVFaceExtra;
struct BLVLight;
struct BLVMapOutline;
struct BLVSector;
struct BSPNode;
struct Chest;
struct ChestDesc;
struct DecorationDesc;
struct FontData;
struct GUICharMetric;
struct ItemGen;
struct LevelDecoration;
struct LocationInfo;
struct LocationTime;
struct PersistentVariables;
struct MonsterDesc;
struct NPCData;
struct ObjectDesc;
struct ODMFace;
struct OutdoorLocationTileType;
struct OverlayDesc;
struct ActiveOverlay;
struct ActiveOverlayList;
struct Party;
struct Player;
struct PlayerFrame;
struct SaveGameHeader;
struct SpawnPoint;
struct SpellBuff;
struct SpriteObject;
struct Timer;

static_assert(sizeof(Vec3s) == 6);
static_assert(sizeof(Vec3i) == 12);
static_assert(sizeof(Vec3f) == 12);
static_assert(sizeof(Planei) == 16);
static_assert(sizeof(Planef) == 16);
static_assert(sizeof(BBoxs) == 12);
MM_DECLARE_MEMCOPY_SERIALIZABLE(Vec3s)
MM_DECLARE_MEMCOPY_SERIALIZABLE(Vec3i)
MM_DECLARE_MEMCOPY_SERIALIZABLE(Vec3f)
MM_DECLARE_MEMCOPY_SERIALIZABLE(Planei)
MM_DECLARE_MEMCOPY_SERIALIZABLE(Planef)
MM_DECLARE_MEMCOPY_SERIALIZABLE(BBoxs)

#pragma pack(push, 1)

struct SpriteFrame_MM6 {
    std::array<char, 12> iconName;
    std::array<char, 12> textureName;    // c
    std::array<int16_t, 8> hwSpriteIds;  // 18h
    int32_t scale;            // 28h
    int32_t flags;               // 2c
    int16_t glowRadius;      // 30
    int16_t paletteId;       // 32
    int16_t paletteIndex;
    int16_t animTime;
};
static_assert(sizeof(SpriteFrame_MM6) == 56);
MM_DECLARE_MEMCOPY_SERIALIZABLE(SpriteFrame_MM6)


struct SpriteFrame_MM7 : public SpriteFrame_MM6 {
    int16_t animLength;
    int16_t _pad;
};
static_assert(sizeof(SpriteFrame_MM7) == 60);
MM_DECLARE_MEMCOPY_SERIALIZABLE(SpriteFrame_MM7)

void deserialize(const SpriteFrame_MM7 &src, SpriteFrame *dst);


struct BLVFace_MM7 {
    Planef facePlane;
    Planei facePlaneOld;
    int32_t zCalc1;
    int32_t zCalc2;
    int32_t zCalc3;
    uint32_t attributes;
    int32_t vertexIDs;
    int32_t xInterceptDisplacements;
    int32_t yInterceptDisplacements;
    int32_t zInterceptDisplacements;
    int32_t vertexUIds;
    int32_t vertexVIds;
    uint16_t faceExtraId;
    uint16_t bitmapId;
    uint16_t sectorId;
    int16_t backSectorId;
    BBoxs bounding;
    uint8_t polygonType;
    uint8_t numVertices;
    int16_t _pad;
};
static_assert(sizeof(BLVFace_MM7) == 0x60);
MM_DECLARE_MEMCOPY_SERIALIZABLE(BLVFace_MM7)

void deserialize(const BLVFace_MM7 &src, BLVFace *dst);


struct TileDesc_MM7 {
    std::array<char, 16> tileName;
    uint16_t tileId;
    uint16_t bitmapId;
    uint16_t tileSet;
    uint16_t section;
    uint16_t attributes;
};
static_assert(sizeof(TileDesc_MM7) == 26);
MM_DECLARE_MEMCOPY_SERIALIZABLE(TileDesc_MM7)

void deserialize(const TileDesc_MM7 &src, TileDesc *dst);


struct TextureFrame_MM7 {
    std::array<char, 12> textureName;
    int16_t textureID;
    int16_t animTime;
    int16_t animLength;
    int16_t flags;
};
static_assert(sizeof(TextureFrame_MM7) == 20);
MM_DECLARE_MEMCOPY_SERIALIZABLE(TextureFrame_MM7)

void deserialize(const TextureFrame_MM7 &src, TextureFrame *dst);


struct NPCData_MM7 {
    /* 00 */ int32_t name;  // char *pName;
    /* 04 */ uint32_t portraitId;
    /* 08 */ uint32_t flags;  // & 0x80    no greeting on dialogue start; looks like hired
    /* 0C */ int32_t fame;
    /* 10 */ int32_t rep;
    /* 14 */ uint32_t location2d;
    /* 18 */ uint32_t profession;
    /* 1C */ int32_t greet;
    /* 20 */ int32_t joins;
    /* 24 */ int32_t field_24;
    /* 28 */ uint32_t evt_A;
    /* 2C */ uint32_t evt_B;
    /* 30 */ uint32_t evt_C;
    /* 34 */ uint32_t evt_D;
    /* 38 */ uint32_t evt_E;
    /* 3C */ uint32_t evt_F;
    /* 40 */ uint32_t sex;
    /* 44 */ int32_t hasUsedAbility;
    /* 48 */ int32_t newsTopic;
    /* 4C */
};
static_assert(sizeof(NPCData_MM7) == 0x4C);
MM_DECLARE_MEMCOPY_SERIALIZABLE(NPCData_MM7)

void serialize(const NPCData &src, NPCData_MM7 *dst);
void deserialize(const NPCData_MM7 &src, NPCData *dst);


struct ItemGen_MM7 {
    /* 00 */ int32_t itemID;
    /* 04 */ int32_t enchantmentType;
    /* 08 */ int32_t enchantmentStrength;
    /* 0C */ int32_t specialEnchantment;
    /* 10 */ int32_t numCharges;
    /* 14 */ uint32_t attributes;
    /* 18 */ uint8_t bodyAnchor;
    /* 19 */ uint8_t maxCharges;
    /* 1A */ uint8_t holderPlayer;
    /* 1B */ uint8_t placedInChest; // unknown unused 8-bit field, was repurposed
    /* 1C */ uint64_t expireTime;
};
static_assert(sizeof(ItemGen_MM7) == 0x24);
MM_DECLARE_MEMCOPY_SERIALIZABLE(ItemGen_MM7)

void serialize(const ItemGen &src, ItemGen_MM7 *dst);
void deserialize(const ItemGen_MM7 &src, ItemGen *dst);


struct SpellBuff_MM7 {
    /* 00 */ int64_t expireTime;
    /* 08 */ uint16_t power;
    /* 0A */ uint16_t skillMastery;
    /* 0C */ uint16_t overlayId;
    /* 0E */ uint8_t caster;
    /* 0F */ uint8_t flags;
    /* 10 */
};
static_assert(sizeof(SpellBuff_MM7) == 0x10);
MM_DECLARE_MEMCOPY_SERIALIZABLE(SpellBuff_MM7)

void serialize(const SpellBuff &src, SpellBuff_MM7 *dst);
void deserialize(const SpellBuff_MM7 &src, SpellBuff *dst);


struct PlayerSpellbookChapter_MM7 {
    /* 00 */ std::array<char, 11> isSpellAvailable;
    /* 0B */
};
static_assert(sizeof(PlayerSpellbookChapter_MM7) == 0xB);
MM_DECLARE_MEMCOPY_SERIALIZABLE(PlayerSpellbookChapter_MM7)


struct PlayerSpells_MM7 {
    union {
        struct {
            /* 00 */ std::array<PlayerSpellbookChapter_MM7, 9> chapters;
            /* 63 */ char _pad1;
            /* 64 */
        };
        struct {
            /* 00 */ std::array<char, 99> haveSpell;
            /* 63 */ char _pad2;
            /* 64 */
        };
    };
};
static_assert(sizeof(PlayerSpells_MM7) == 0x64);
MM_DECLARE_MEMCOPY_SERIALIZABLE(PlayerSpells_MM7)


union PlayerEquipment_MM7 {
    std::array<uint32_t, 16> indices;
};
static_assert(sizeof(PlayerEquipment_MM7) == 0x40);
MM_DECLARE_MEMCOPY_SERIALIZABLE(PlayerEquipment_MM7)


struct LloydBeacon_MM7 {
    /* 00 */ uint64_t beaconTime;
    /* 08 */ int32_t partyPosX;
    /* 0C */ int32_t partyPosY;
    /* 10 */ int32_t partyPosZ;
    /* 14 */ int16_t partyViewYaw;
    /* 16 */ int16_t partyViewPitch;
    /* 18 */ uint16_t field_18;
    /* 1A */ uint16_t saveFileId;
    /* 1C */
};
static_assert(sizeof(LloydBeacon_MM7) == 0x1C);
MM_DECLARE_MEMCOPY_SERIALIZABLE(LloydBeacon_MM7)


struct Player_MM7 {
    /* 0000 */ std::array<int64_t, 20> conditions;
    /* 00A0 */ uint64_t experience;
    /* 00A8 */ std::array<char, 16> name;
    /* 00B8 */ uint8_t sex;
    /* 00B9 */ uint8_t classType;
    /* 00BA */ uint8_t currentFace;
    /* 00BB */ uint8_t _pad;
    /* 00BC */ uint16_t might;
    /* 00BE */ uint16_t mightBonus;
    /* 00C0 */ uint16_t intelligence;
    /* 00C2 */ uint16_t intelligenceBonus;
    /* 00C4 */ uint16_t willpower;
    /* 00C6 */ uint16_t willpowerBonus;
    /* 00C8 */ uint16_t endurance;
    /* 00CA */ uint16_t enduranceBonus;
    /* 00CC */ uint16_t speed;
    /* 00CE */ uint16_t speedBonus;
    /* 00D0 */ uint16_t accuracy;
    /* 00D2 */ uint16_t accuracyBonus;
    /* 00D4 */ uint16_t luck;
    /* 00D6 */ uint16_t luckBonus;
    /* 00D8 */ int16_t acModifier;
    /* 00DA */ uint16_t level;
    /* 00DC */ int16_t levelModifier;
    /* 00DE */ int16_t ageModifier;
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
    /* 0108 */ std::array<uint16_t, 37> activeSkills;
    /* 0152 */ std::array<uint8_t, 64> achievedAwardsBits;
    /* 0192 */ PlayerSpells_MM7 spellbook;
    /* 01F6 */ uint16_t _pad2;
    /* 01F8 */ int32_t pureLuckUsed;
    /* 01FC */ int32_t pureSpeedUsed;
    /* 0200 */ int32_t pureIntellectUsed;
    /* 0204 */ int32_t pureEnduranceUsed;
    /* 0208 */ int32_t pureWillpowerUsed;
    /* 020C */ int32_t pureAccuracyUsed;
    /* 0210 */ int32_t pureMightUsed;
    /* 0214 */ std::array<ItemGen_MM7, 138> ownItems;
    /* 157C */ std::array<int32_t, 126> inventoryMatrix;
    /* 1774 */ int16_t resFireBase;
    /* 1776 */ int16_t resAirBase;
    /* 1778 */ int16_t resWaterBase;
    /* 177A */ int16_t resEarthBase;
    /* 177C */ int16_t resPhysicalBase;
    /* 177E */ int16_t resMagicBase;
    /* 1780 */ int16_t resSpiritBase;
    /* 1782 */ int16_t resMindBase;
    /* 1784 */ int16_t resBodyBase;
    /* 1786 */ int16_t resLightBase;
    /* 1788 */ int16_t resDarkBase;
    /* 178A */ int16_t resFireBonus;
    /* 178C */ int16_t resAirBonus;
    /* 178E */ int16_t resWaterBonus;
    /* 1790 */ int16_t resEarthBonus;
    /* 1792 */ int16_t resPhysicalBonus;
    /* 1794 */ int16_t resMagicBonus;
    /* 1796 */ int16_t resSpiritBonus;
    /* 1798 */ int16_t resMindBonus;
    /* 179A */ int16_t resBodyBonus;
    /* 179C */ int16_t resLightBonus;
    /* 179E */ int16_t resDarkBonus;
    /* 17A0 */ std::array<SpellBuff_MM7, 24> playerBuffs;
    /* 1920 */ uint32_t voiceId;
    /* 1924 */ int32_t prevVoiceId;
    /* 1928 */ int32_t prevFace;
    /* 192C */ int32_t field_192C;
    /* 1930 */ int32_t field_1930;
    /* 1934 */ uint16_t timeToRecovery;
    /* 1936 */ char field_1936;
    /* 1937 */ char field_1937;
    /* 1938 */ uint32_t skillPoints;
    /* 193C */ int32_t health;
    /* 1940 */ int32_t mana;
    /* 1944 */ uint32_t birthYear;
    /* 1948 */ PlayerEquipment_MM7 equipment;
    /* 1988 */ std::array<int32_t, 49> field_1988;
    /* 1A4C */ char field_1A4C;
    /* 1A4D */ char field_1A4D;
    /* 1A4E */ char lastOpenedSpellbookPage;
    /* 1A4F */ uint8_t quickSpell;
    /* 1A50 */ std::array<uint8_t, 64> playerEventBits;
    /* 1A90 */ char someAttackBonus;
    /* 1A91 */ char field_1A91;
    /* 1A92 */ char meleeDmgBonus;
    /* 1A93 */ char field_1A93;
    /* 1A94 */ char rangedAttackBonus;
    /* 1A95 */ char field_1A95;
    /* 1A96 */ char rangedDmgBonus;
    /* 1A97 */ char field_1A97;
    /* 1A98 */ char fullHealthBonus;
    /* 1A99 */ char healthRelated;
    /* 1A9A */ char fullManaBonus;
    /* 1A9B */ char manaRelated;
    /* 1A9C */ uint16_t expression;
    /* 1A9E */ uint16_t expressionTimePassed;
    /* 1AA0 */ uint16_t expressionTimeLength;
    /* 1AA2 */ int16_t field_1AA2;
    /* 1AA4 */ int32_t _expression21_animtime;
    /* 1AA8 */ int32_t _expression21_frameset;
    /* 1AAC */ std::array<LloydBeacon_MM7, 5> installedBeacons;
    /* 1B38 */ char numDivineInterventionCasts;
    /* 1B39 */ char numArmageddonCasts;
    /* 1B3A */ char numFireSpikeCasts;
    /* 1B3B */ char field_1B3B;
    /* 1B3C */
};
static_assert(sizeof(Player_MM7) == 0x1B3C);
MM_DECLARE_MEMCOPY_SERIALIZABLE(Player_MM7)

void serialize(const Player &src, Player_MM7 *dst);
void deserialize(const Player_MM7 &src, Player *dst);


struct PartyTimeStruct_MM7 {
    /* 000 */ std::array<int64_t, 10> bountyHuntingNextGenerationTime;
    /* 050 */ std::array<int64_t, 85> shopsNextGenerationTime;
    /* 2F8 */ std::array<int64_t, 53> shopBanTimes;
    /* 4A0 */ std::array<int64_t, 10> counterEventValues;
    /* 4F0 */ std::array<int64_t, 29> historyEventTimes;
    /* 5D8 */ std::array<int64_t, 20> someOtherTimes;
    /* 678 */
};
static_assert(sizeof(PartyTimeStruct_MM7) == 0x678);
MM_DECLARE_MEMCOPY_SERIALIZABLE(PartyTimeStruct_MM7)


struct Party_MM7 {
    /* 00000 */ int32_t field_0;
    /* 00004 */ uint32_t partyHeight;
    /* 00008 */ uint32_t defaultPartyHeight;
    /* 0000C */ int32_t eyeLevel;
    /* 00010 */ uint32_t defaultEyeLevel;
    /* 00014 */ int32_t radius;
    /* 00018 */ int32_t yawGranularity;
    /* 0001C */ uint32_t walkSpeed;
    /* 00020 */ int32_t yawRotationSpeed;  // deg/s
    /* 00024 */ int32_t jumpStrength;
    /* 00028 */ int32_t field_28;
    /* 0002C */ uint64_t timePlayed;
    /* 00034 */ int64_t lastRegenerationTime;
    /* 0003C */ PartyTimeStruct_MM7 partyTimes;
    /* 006B4 */ Vec3i position;
    /* 006C0 */ int32_t viewYaw;
    /* 006C4 */ int32_t viewPitch;
    /* 006C8 */ Vec3i prevPosition;
    /* 006D4 */ int32_t viewPrevYaw;
    /* 006D8 */ int32_t viewPrevPitch;
    /* 006DC */ int32_t prevEyeLevel;
    /* 006E0 */ int32_t field_6E0;
    /* 006E4 */ int32_t field_6E4;
    /* 006E8 */ int32_t fallSpeed;
    /* 006EC */ int32_t field_6EC;
    /* 006F0 */ int32_t field_6F0;
    /* 006F4 */ int32_t floorFacePid;  // face we are standing at
    /* 006F8 */ int32_t walkSoundTimer;
    /* 006FC */ int32_t waterLavaTimer;
    /* 00700 */ int32_t fallStartZ;
    /* 00704 */ uint32_t flying;
    /* 00708 */ char field_708;
    /* 00709 */ uint8_t hirelingScrollPosition;
    /* 0070A */ char field_70A;
    /* 0070B */ char field_70B;
    /* 0070C */ uint32_t currentYear;
    /* 00710 */ uint32_t currentMonth;
    /* 00714 */ uint32_t currentMonthWeek;
    /* 00718 */ uint32_t currentDayOfMonth;
    /* 0071C */ uint32_t currentHour;
    /* 00720 */ uint32_t currentMinute;
    /* 00724 */ uint32_t currentTimeSecond;
    /* 00728 */ uint32_t numFoodRations;
    /* 0072C */ int32_t field_72C;
    /* 00730 */ int32_t field_730;
    /* 00734 */ uint32_t numGold;
    /* 00738 */ uint32_t numGoldInBank;
    /* 0073C */ uint32_t numDeaths;
    /* 00740 */ int32_t field_740;
    /* 00744 */ int32_t numPrisonTerms;
    /* 00748 */ uint32_t numBountiesCollected;
    /* 0074C */ int field_74C;
    /* 00750 */ std::array<int16_t, 5> monsterIdForHunting;
    /* 0075A */ std::array<int16_t, 5> monsterForHuntingKilled;
    /* 00764 */ uint8_t daysPlayedWithoutRest;
    /* 00765 */ std::array<uint8_t, 64> questBits;
    /* 007A5 */ std::array<uint8_t, 16> arcomageWins;
    /* 007B5 */ char field_7B5_in_arena_quest;
    /* 007B6 */ std::array<char, 4> numArenaWins;
    /* 007BA */ std::array<bool, 29> isArtifactFound;  // 7ba
    /* 007D7 */ std::array<char, 39> field_7d7;
    /* 007FE */ std::array<uint8_t, 26> autonoteBits;
    /* 00818 */ std::array<char, 60> field_818;
    /* 00854 */ std::array<char, 32> field_854;
    /* 00874 */ int32_t numArcomageWins;
    /* 00878 */ int32_t numArcomageLoses;
    /* 0087C */ uint32_t turnBasedModeOn;
    /* 00880 */ int32_t field_880;
    /* 00884 */ int32_t flags2;
    /* 00888 */ uint32_t alignment;
    /* 0088C */ std::array<SpellBuff_MM7, 20> partyBuffs;
    /* 00954 */ std::array<Player_MM7, 4> players;
    /* 07644 */ std::array<NPCData_MM7, 2> hirelings;
    /* 07754 */ ItemGen_MM7 pickedItem;
    /* 07778 */ uint32_t flags;
    /* 0777C */ std::array<std::array<ItemGen_MM7, 12>, 53> standartItemsInShops;
    /* 0D0EC */ std::array<std::array<ItemGen_MM7, 12>, 53> specialItemsInShops;
    /* 12A5C */ std::array<std::array<ItemGen_MM7, 12>, 32> spellBooksInGuilds;
    /* 1605C */ std::array<char, 24> field_1605C;
    /* 16074 */ std::array<char, 100> hireling1Name;
    /* 160D8 */ std::array<char, 100> hireling2Name;
    /* 1613C */ int32_t armageddonTimer;
    /* 16140 */ int32_t armageddonDamage;
    /* 16144 */ std::array<int32_t, 4> turnBasedPlayerRecoveryTimes;
    /* 16154 */ std::array<int32_t, 53> inTheShopFlags;
    /* 16228 */ int32_t fine;
    /* 1622C */ float torchlightColorR;
    /* 16230 */ float torchlightColorG;
    /* 16234 */ float torchlightColorB;
    /* 16238 */
};
static_assert(sizeof(Party_MM7) == 0x16238);
MM_DECLARE_MEMCOPY_SERIALIZABLE(Party_MM7)

void serialize(const Party &src, Party_MM7 *dst);
void deserialize(const Party_MM7 &src, Party *dst);

struct Timer_MM7 {
    /* 00 */ uint32_t ready;
    /* 04 */ uint32_t paused;
    /* 08 */ int32_t tackGameTime;
    /* 0C */ uint32_t startTime;
    /* 10 */ uint32_t stopTime;
    /* 14 */ int32_t gameTimeStart;
    /* 18 */ int32_t field_18;
    /* 1C */ uint32_t timeElapsed;
    /* 20 */ int32_t dtFixpoint;
    /* 24 */ uint32_t totalGameTimeElapsed;
    /* 28 */
};
static_assert(sizeof(Timer_MM7) == 0x28);
MM_DECLARE_MEMCOPY_SERIALIZABLE(Timer_MM7)

void serialize(const Timer &src, Timer_MM7 *dst);
void deserialize(const Timer_MM7 &src, Timer *dst);


struct ActiveOverlay_MM7 {
    /* 00 */ int16_t field_0;
    /* 02 */ int16_t indexToOverlayList;
    /* 04 */ int16_t spriteFrameTime;
    /* 06 */ int16_t animLength;
    /* 08 */ int16_t screenSpaceX;
    /* 0A */ int16_t screenSpaceY;
    /* 0C */ int16_t pid;
    /* 0E */ int16_t projSize;
    /* 10 */ int32_t fpDamageMod;
    /* 14 */
};
static_assert(sizeof(ActiveOverlay_MM7) == 0x14);
MM_DECLARE_MEMCOPY_SERIALIZABLE(ActiveOverlay_MM7)

void serialize(const ActiveOverlay &src, ActiveOverlay_MM7 *dst);
void deserialize(const ActiveOverlay_MM7 &src, ActiveOverlay *dst);


struct ActiveOverlayList_MM7 {
    /* 000 */ std::array<ActiveOverlay_MM7, 50> overlays;
    /* 3E8 */ int32_t field_3E8;
    /* 3EC */ int32_t redraw;
    /* 3F0 */
};
static_assert(sizeof(ActiveOverlayList_MM7) == 0x3F0);
MM_DECLARE_MEMCOPY_SERIALIZABLE(ActiveOverlayList_MM7)

void serialize(const ActiveOverlayList &src, ActiveOverlayList_MM7 *dst);
void deserialize(const ActiveOverlayList_MM7 &src, ActiveOverlayList *dst);


struct IconFrame_MM7 {
    std::array<char, 12> animationName;
    std::array<char, 12> textureName;
    int16_t animTime;
    int16_t animLength;
    int16_t flags;  // 0x01 - more icons in this animation
    uint16_t textureId;
};
static_assert(sizeof(IconFrame_MM7) == 0x20);
MM_DECLARE_MEMCOPY_SERIALIZABLE(IconFrame_MM7)

void serialize(const Icon &src, IconFrame_MM7 *dst);
void deserialize(const IconFrame_MM7 &src, Icon *dst);


struct UIAnimation_MM7 {
    /* 000 */ uint16_t iconId;
    /* 002 */ int16_t field_2;
    /* 004 */ int16_t animTime;
    /* 006 */ int16_t animLength;
    /* 008 */ int16_t x;
    /* 00A */ int16_t y;
    /* 00C */ char field_C;
};
static_assert(sizeof(UIAnimation_MM7) == 0xD);
MM_DECLARE_MEMCOPY_SERIALIZABLE(UIAnimation_MM7)

void serialize(const UIAnimation &src, UIAnimation_MM7 *dst);
void deserialize(const UIAnimation_MM7 &src, UIAnimation *dst);


struct MonsterInfo_MM7 {
    int32_t name;
    int32_t pictureName;
    uint8_t level;
    uint8_t treasureDropChance;
    uint8_t treasureDiceRolls;
    uint8_t treasureDiceSides;
    uint8_t treasureLevel;
    uint8_t treasureType;
    uint8_t flying;
    uint8_t movementType;
    uint8_t aiType;
    uint8_t hostilityType;
    char field_12;
    uint8_t specialAttackType;
    uint8_t specialAttackLevel;
    uint8_t attack1Type;
    uint8_t attack1DamageDiceRolls;
    uint8_t attack1DamageDiceSides;
    uint8_t attack1DamageBonus;
    uint8_t missileAttack1Type;
    uint8_t attack2Chance;
    uint8_t attack2Type;
    uint8_t attack2DamageDiceRolls;
    uint8_t attack2DamageDiceSides;
    uint8_t attack2DamageBonus;
    uint8_t missileAttack2Type;
    uint8_t spell1UseChance;
    uint8_t spell1Id;
    uint8_t spell2UseChance;
    uint8_t spell2Id;
    uint8_t resFire;
    uint8_t resAir;
    uint8_t resWater;
    uint8_t resEarth;
    uint8_t resMind;
    uint8_t resSpirit;
    uint8_t resBody;
    uint8_t resLight;
    uint8_t resDark;
    uint8_t resPhysical;
    uint8_t specialAbilityType;
    uint8_t specialAbilityDamageDiceRolls;
    uint8_t specialAbilityDamageDiceSides;
    uint8_t specialAbilityDamageDiceBonus;
    uint8_t numCharactersAttackedPerSpecialAbility;
    char field_33;
    uint16_t id;
    uint16_t bloodSplatOnDeath;
    uint16_t spellSkillAndMastery1;
    uint16_t spellSkillAndMastery2;
    int16_t field_3C_some_special_attack;
    int16_t field_3E;
    uint32_t hp;
    uint32_t ac;
    uint32_t exp;
    uint32_t baseSpeed;
    int32_t recoveryTime;
    uint32_t attackPreference;
};
static_assert(sizeof(MonsterInfo_MM7) == 0x58);
MM_DECLARE_MEMCOPY_SERIALIZABLE(MonsterInfo_MM7)


struct MonsterDesc_MM6 {
    uint16_t monsterHeight;
    uint16_t monsterRadius;
    uint16_t movementSpeed;
    int16_t toHitRadius;
    std::array<uint16_t, 4> soundSampleIds;
    std::array<char, 32> monsterName;
    std::array<std::array<char, 10>, 10> spriteNames;
};
static_assert(sizeof(MonsterDesc_MM6) == 148);
MM_DECLARE_MEMCOPY_SERIALIZABLE(MonsterDesc_MM6)

void deserialize(const MonsterDesc_MM6 &src, MonsterDesc *dst);


struct MonsterDesc_MM7 {
    uint16_t monsterHeight;
    uint16_t monsterRadius;
    uint16_t movementSpeed;
    int16_t toHitRadius;
    uint32_t tintColor;
    std::array<uint16_t, 4> soundSampleIds;
    std::array<char, 32> monsterName;
    std::array<std::array<char, 10>, 10> spriteNames;
};
static_assert(sizeof(MonsterDesc_MM7) == 152);
MM_DECLARE_MEMCOPY_SERIALIZABLE(MonsterDesc_MM7)

void serialize(const MonsterDesc &src, MonsterDesc_MM7 *dst);
void deserialize(const MonsterDesc_MM7 &src, MonsterDesc *dst);


struct ActorJob_MM7 {
    Vec3s pos;
    uint16_t attributes = 0;
    uint8_t action = 0;
    uint8_t hour = 0;
    uint8_t day = 0;
    uint8_t month = 0;
};
static_assert(sizeof(ActorJob_MM7) == 12);
MM_DECLARE_MEMCOPY_SERIALIZABLE(ActorJob_MM7)

void serialize(const ActorJob &src, ActorJob_MM7 *dst);
void deserialize(const ActorJob_MM7 &src, ActorJob *dst);


struct Actor_MM7 {
    std::array<char, 32> pActorName;
    int16_t sNPC_ID;
    int16_t field_22;
    uint32_t uAttributes;
    int16_t sCurrentHP;
    std::array<char, 2> padding_unused;
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
    std::array<ActorJob_MM7, 8> pScheduledJobs;
    uint32_t uSummonerID;
    uint32_t uLastCharacterIDToHit;
    int32_t dword_000334_unique_name;
    std::array<char, 12> field_338;
};
static_assert(sizeof(Actor_MM7) == 0x344);
MM_DECLARE_MEMCOPY_SERIALIZABLE(Actor_MM7)

void serialize(const Actor &src, Actor_MM7 *dst);
void deserialize(const Actor_MM7 &src, Actor *dst);


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
MM_DECLARE_MEMCOPY_SERIALIZABLE(BLVDoor_MM7)

void serialize(const BLVDoor &src, BLVDoor_MM7 *dst);
void deserialize(const BLVDoor_MM7 &src, BLVDoor *dst);


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
MM_DECLARE_MEMCOPY_SERIALIZABLE(BLVSector_MM7)

void serialize(const BLVSector &src, BLVSector_MM7 *dst);
void deserialize(const BLVSector_MM7 &src, BLVSector *dst);


struct GUICharMetric_MM7 {
    int32_t uLeftSpacing;
    int32_t uWidth;
    int32_t uRightSpacing;
};
static_assert(sizeof(GUICharMetric_MM7) == 12);
MM_DECLARE_MEMCOPY_SERIALIZABLE(GUICharMetric_MM7)

void serialize(const GUICharMetric &src, GUICharMetric_MM7 *dst);
void deserialize(const GUICharMetric_MM7 &src, GUICharMetric *dst);


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
    std::array<GUICharMetric_MM7, 256> pMetrics;
    std::array<uint32_t, 256> font_pixels_offset;
    uint8_t pFontData[0];  // array of font pixels
};
static_assert(sizeof(FontData_MM7) == 0x1020);
MM_DECLARE_MEMCOPY_SERIALIZABLE(FontData_MM7)

void serialize(const FontData &src, FontData_MM7 *dst);
void deserialize(const FontData_MM7 &src, size_t size, FontData *dst);


struct ODMFace_MM7 {
    Planei facePlane;
    int32_t zCalc1;
    int32_t zCalc2;
    int32_t zCalc3;
    uint32_t attributes;
    std::array<int16_t, 20> pVertexIDs;
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
MM_DECLARE_MEMCOPY_SERIALIZABLE(ODMFace_MM7)

void deserialize(const ODMFace_MM7 &src, ODMFace *dst);


struct SpawnPoint_MM6 {
    Vec3i vPosition;
    uint16_t uRadius;
    uint16_t uKind;
    uint16_t uIndex;
    uint16_t uAttributes;
};
static_assert(sizeof(SpawnPoint_MM6) == 20);
MM_DECLARE_MEMCOPY_SERIALIZABLE(SpawnPoint_MM6)

struct SpawnPoint_MM7 {
    Vec3i vPosition;
    uint16_t uRadius;
    uint16_t uKind;
    uint16_t uIndex;
    uint16_t uAttributes;
    unsigned int uGroup;
};
static_assert(sizeof(SpawnPoint_MM7) == 24);
MM_DECLARE_MEMCOPY_SERIALIZABLE(SpawnPoint_MM7)

void deserialize(const SpawnPoint_MM7 &src, SpawnPoint *dst);


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
MM_DECLARE_MEMCOPY_SERIALIZABLE(SpriteObject_MM7)

void serialize(const SpriteObject &src, SpriteObject_MM7 *dst);
void deserialize(const SpriteObject_MM7 &src, SpriteObject *dst);


struct ChestDesc_MM7 {
    std::array<char, 32> pName;
    uint8_t uWidth;
    uint8_t uHeight;
    int16_t uTextureID;
};
static_assert(sizeof(ChestDesc_MM7) == 36);
MM_DECLARE_MEMCOPY_SERIALIZABLE(ChestDesc_MM7)

void deserialize(const ChestDesc_MM7 &src, ChestDesc *dst);


struct DecorationDesc_MM6 {
    std::array<char, 32> pName;
    std::array<char, 32> field_20;
    int16_t uType;
    uint16_t uDecorationHeight;
    int16_t uRadius;
    int16_t uLightRadius;
    uint16_t uSpriteID;
    uint16_t uFlags;
    int16_t uSoundID;
    int16_t _pad;
};
static_assert(sizeof(DecorationDesc_MM6) == 80);
MM_DECLARE_MEMCOPY_SERIALIZABLE(DecorationDesc_MM6)

struct DecorationDesc_MM7 : public DecorationDesc_MM6 {
    uint8_t uColoredLightRed;
    uint8_t uColoredLightGreen;
    uint8_t uColoredLightBlue;
    uint8_t _pad;
};
static_assert(sizeof(DecorationDesc_MM7) == 84);
MM_DECLARE_MEMCOPY_SERIALIZABLE(DecorationDesc_MM7)

void deserialize(const DecorationDesc_MM6 &src, DecorationDesc *dst);
void deserialize(const DecorationDesc_MM7 &src, DecorationDesc *dst);


struct Chest_MM7 {
    uint16_t uChestBitmapID;
    uint16_t uFlags;
    std::array<ItemGen_MM7, 140> igChestItems;
    std::array<int16_t, 140> pInventoryIndices;
};
static_assert(sizeof(Chest_MM7) == 5324);
MM_DECLARE_MEMCOPY_SERIALIZABLE(Chest_MM7)

void serialize(const Chest &src, Chest_MM7 *dst);
void deserialize(const Chest_MM7 &src, Chest *dst);


struct BLVLight_MM6 {
    Vec3s vPosition;
    int16_t uRadius;
    int16_t uAttributes;
    uint16_t uBrightness;
};
static_assert(sizeof(BLVLight_MM6) == 12);
MM_DECLARE_MEMCOPY_SERIALIZABLE(BLVLight_MM6)

struct BLVLight_MM7 {
    Vec3s vPosition;
    int16_t uRadius;
    char uRed;
    char uGreen;
    char uBlue;
    char uType;
    int16_t uAtributes;
    int16_t uBrightness;
};
static_assert(sizeof(BLVLight_MM7) == 16);
MM_DECLARE_MEMCOPY_SERIALIZABLE(BLVLight_MM7)

void deserialize(const BLVLight_MM7 &src, BLVLight *dst);


struct OverlayDesc_MM7 {
    uint16_t uOverlayID;
    uint16_t uOverlayType;
    uint16_t uSpriteFramesetID;
    int16_t spriteFramesetGroup;
};
static_assert(sizeof(OverlayDesc_MM7) == 8);
MM_DECLARE_MEMCOPY_SERIALIZABLE(OverlayDesc_MM7)

void deserialize(const OverlayDesc_MM7 &src, OverlayDesc *dst);


struct PlayerFrame_MM7 {
    uint16_t expression;
    uint16_t uTextureID;
    int16_t uAnimTime;
    int16_t uAnimLength;
    int16_t uFlags;
};
static_assert(sizeof(PlayerFrame_MM7) == 10);
MM_DECLARE_MEMCOPY_SERIALIZABLE(PlayerFrame_MM7)

void deserialize(const PlayerFrame_MM7 &src, PlayerFrame *dst);


struct LevelDecoration_MM7 {
    uint16_t uDecorationDescID;
    uint16_t uFlags;
    Vec3i vPosition;
    int32_t _yawAngle;
    uint16_t uCog;
    uint16_t uEventID;
    uint16_t uTriggerRange;
    int16_t field_1A;
    int16_t eventVarId;
    int16_t field_1E;
};
static_assert(sizeof(LevelDecoration_MM7) == 32);
MM_DECLARE_MEMCOPY_SERIALIZABLE(LevelDecoration_MM7)

void deserialize(const LevelDecoration_MM7 &src, LevelDecoration *dst);


struct BLVFaceExtra_MM7 {
    int16_t field_0;
    int16_t field_2;
    int16_t field_4;
    int16_t field_6;
    int16_t field_8;
    int16_t field_A;
    int16_t face_id;
    uint16_t uAdditionalBitmapID;
    int16_t field_10;
    int16_t field_12;
    int16_t sTextureDeltaU;
    int16_t sTextureDeltaV;
    int16_t sCogNumber;
    uint16_t uEventID;
    int16_t field_1C;
    int16_t field_1E;
    int16_t field_20;
    int16_t field_22;
};
static_assert(sizeof(BLVFaceExtra_MM7) == 36);
MM_DECLARE_MEMCOPY_SERIALIZABLE(BLVFaceExtra_MM7)

void deserialize(const BLVFaceExtra_MM7 &src, BLVFaceExtra *dst);


struct BSPNode_MM7 {
    int16_t uFront;
    int16_t uBack;
    int16_t uBSPFaceIDOffset;
    int16_t uNumBSPFaces;
};
static_assert(sizeof(BSPNode_MM7) == 8);
MM_DECLARE_MEMCOPY_SERIALIZABLE(BSPNode_MM7)

void deserialize(const BSPNode_MM7 &src, BSPNode *dst);


struct BLVMapOutline_MM7 {
    uint16_t uVertex1ID;
    uint16_t uVertex2ID;
    uint16_t uFace1ID;
    uint16_t uFace2ID;
    int16_t sZ;
    uint16_t uFlags;
};
static_assert(sizeof(BLVMapOutline_MM7) == 12);
MM_DECLARE_MEMCOPY_SERIALIZABLE(BLVMapOutline_MM7)

void deserialize(const BLVMapOutline_MM7 &src, BLVMapOutline *dst);


struct ObjectDesc_MM6 {
    std::array<char, 32> field_0;
    int16_t uObjectID;
    int16_t uRadius;
    int16_t uHeight;
    int16_t uFlags;
    uint16_t uSpriteID;
    int16_t uLifetime;
    uint16_t uParticleTrailColor;
    int16_t uSpeed;
    uint8_t uParticleTrailColorR;
    uint8_t uParticleTrailColorG;
    uint8_t uParticleTrailColorB;
    char _pad;
};
static_assert(sizeof(ObjectDesc_MM6) == 52);
MM_DECLARE_MEMCOPY_SERIALIZABLE(ObjectDesc_MM6)

struct ObjectDesc_MM7 {
    std::array<char, 32> field_0;
    int16_t uObjectID;
    int16_t uRadius;
    int16_t uHeight;
    uint16_t uFlags;
    uint16_t uSpriteID;
    int16_t uLifetime;
    uint32_t uParticleTrailColor;
    int16_t uSpeed;
    uint8_t uParticleTrailColorR;
    uint8_t uParticleTrailColorG;
    uint8_t uParticleTrailColorB;
    std::array<char, 3> _pad;
};
static_assert(sizeof(ObjectDesc_MM7) == 56);
MM_DECLARE_MEMCOPY_SERIALIZABLE(ObjectDesc_MM7)

void deserialize(const ObjectDesc_MM6 &src, ObjectDesc *dst);
void deserialize(const ObjectDesc_MM7 &src, ObjectDesc *dst);


struct BSPModelData_MM7 {
    std::array<char, 32> pModelName;
    std::array<char, 32> pModelName2;
    int32_t field_40;
    uint32_t uNumVertices;
    uint32_t ppVertices;
    uint32_t uNumFaces;
    uint32_t uNumConvexFaces;
    uint32_t ppFaces;
    uint32_t ppFacesOrdering;
    uint32_t uNumNodes;
    uint32_t ppNodes;
    uint32_t uNumDecorations;
    int32_t sCenterX;
    int32_t sCenterY;
    Vec3i vPosition;
    int32_t sMinX;
    int32_t sMinY;
    int32_t sMinZ;
    int32_t sMaxX;
    int32_t sMaxY;
    int32_t sMaxZ;
    int32_t sSomeOtherMinX;
    int32_t sSomeOtherMinY;
    int32_t sSomeOtherMinZ;
    int32_t sSomeOtherMaxX;
    int32_t sSomeOtherMaxY;
    int32_t sSomeOtherMaxZ;
    Vec3i vBoundingCenter;
    int32_t sBoundingRadius;
};
static_assert(sizeof(BSPModelData_MM7) == 188);
MM_DECLARE_MEMCOPY_SERIALIZABLE(BSPModelData_MM7)
// Note: serialization code is in CompositeImages.h

struct LocationTime_MM7 {
    int64_t last_visit;
    std::array<char, 12> sky_texture_name;
    int32_t day_attrib;
    int32_t day_fogrange_1;
    int32_t day_fogrange_2;
    std::array<char, 24> field_2F4;
};
static_assert(sizeof(LocationTime_MM7) == 0x38);
MM_DECLARE_MEMCOPY_SERIALIZABLE(LocationTime_MM7)

void serialize(const LocationTime &src, LocationTime_MM7 *dst);
void deserialize(const LocationTime_MM7 &src, LocationTime *dst);


struct SoundInfo_MM6 {
    std::array<char, 32> pSoundName;
    uint32_t uSoundID;
    uint32_t eType;
    uint32_t uFlags;
    std::array<uint32_t, 17> pSoundDataID;
};
static_assert(sizeof(SoundInfo_MM6) == 112);
MM_DECLARE_MEMCOPY_SERIALIZABLE(SoundInfo_MM6)

struct SoundInfo_MM7 : public SoundInfo_MM6 {
    uint32_t p3DSoundID;
    uint32_t bDecompressed;
};
static_assert(sizeof(SoundInfo_MM7) == 120);
MM_DECLARE_MEMCOPY_SERIALIZABLE(SoundInfo_MM7)

void deserialize(const SoundInfo_MM6 &src, SoundInfo *dst);
void deserialize(const SoundInfo_MM7 &src, SoundInfo *dst);


struct LocationInfo_MM7 {
    int32_t respawnCount;
    int32_t lastRespawnDay;
    int32_t reputation;
    int32_t alertStatus;
};
static_assert(sizeof(LocationInfo_MM7) == 16);
MM_DECLARE_MEMCOPY_SERIALIZABLE(LocationInfo_MM7)

void serialize(const LocationInfo &src, LocationInfo_MM7 *dst);
void deserialize(const LocationInfo_MM7 &src, LocationInfo *dst);


struct LocationHeader_MM7 {
    LocationInfo_MM7 info;
    uint32_t totalFacesCount; // Outdoor: total number of faces in all bmodels in the level. Indoor: total number of faces in the level.
    uint32_t decorationCount;
    uint32_t bmodelCount;
    int32_t field_1C;
    int32_t field_20;
    int32_t field_24;
};
static_assert(sizeof(LocationHeader_MM7) == 40);
MM_DECLARE_MEMCOPY_SERIALIZABLE(LocationHeader_MM7)
// LocationHeader_MM7 is only used during deserialization and doesn't have a runtime equivalent,
// so no deserialize() overloads for it.


// TODO(captainurist): PersistentVariables_MM7
struct MapEventVariables_MM7 {
    std::array<unsigned char, 75> mapVars;
    std::array<unsigned char, 125> decorVars;
};
static_assert(sizeof(MapEventVariables_MM7) == 0xC8);
MM_DECLARE_MEMCOPY_SERIALIZABLE(MapEventVariables_MM7)

void serialize(const PersistentVariables &src, MapEventVariables_MM7 *dst);
void deserialize(const MapEventVariables_MM7 &src, PersistentVariables *dst);


struct BLVHeader_MM7 {
    std::array<char, 104> field_0;
    unsigned int uFaces_fdata_Size;
    unsigned int uSector_rdata_Size;
    unsigned int uSector_lrdata_Size;
    unsigned int uDoors_ddata_Size;
    std::array<char, 16> field_78;
};
static_assert(sizeof(BLVHeader_MM7) == 136);
MM_DECLARE_MEMCOPY_SERIALIZABLE(BLVHeader_MM7)
// BLVHeader_MM7 is only used during deserialization and doesn't have a runtime equivalent,
// so no deserialize() overloads for it.


struct OutdoorLocationTileType_MM7 {
    uint16_t tileset;
    uint16_t tileId;
};
static_assert(sizeof(OutdoorLocationTileType_MM7) == 4);
MM_DECLARE_MEMCOPY_SERIALIZABLE(OutdoorLocationTileType_MM7)

void deserialize(const OutdoorLocationTileType_MM7 &src, OutdoorLocationTileType *dst);


struct SaveGameHeader_MM7 {
    std::array<char, 20> name;
    std::array<char, 20> locationName;
    int64_t playingTime;
    std::array<char, 52> field_30;
};
static_assert(sizeof(SaveGameHeader_MM7) == 0x64);
MM_DECLARE_MEMCOPY_SERIALIZABLE(SaveGameHeader_MM7)

void serialize(const SaveGameHeader &src, SaveGameHeader_MM7 *dst);
void deserialize(const SaveGameHeader_MM7 &src, SaveGameHeader *dst);


#pragma pack(pop)
