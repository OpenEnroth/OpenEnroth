#pragma once

#include <array>

#include "Library/Geometry/Vec.h"
#include "Library/Geometry/Plane.h"
#include "Library/Geometry/BBox.h"

#include "Library/Binary/BinarySerialization.h"

/**
 * @file
 *
 * Snapshots in this file are representations of different engine entities, as stored in the game binary files.
 *
 * All structs here can be directly memory-mapped / memcpy'd from the game files. This layer is mainly needed so that
 * the engine has 100% freedom to change data layout without breaking backward compatibility.
 */

class Actor;
class Character;
class Icon;
class Pid;
struct RawCharacterConditions;
struct RawTimer;
class SoundInfo;
class SpriteFrame;
class TextureFrame;
class TileDesc;
class UIAnimation;
struct ActiveOverlay;
struct ActiveOverlayList;
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
struct FontHeader;
struct GUICharMetric;
struct ItemGen;
struct LevelDecoration;
struct LocationInfo;
struct LocationTime;
struct MonsterDesc;
struct NPCData;
struct ODMFace;
struct ObjectDesc;
struct OutdoorLocationTileType;
struct OverlayDesc;
struct Party;
struct PersistentVariables;
struct PlayerFrame;
struct SaveGameHeader;
struct SpawnPoint;
struct SpellBuff;
struct SpriteObject;

static_assert(sizeof(Vec3s) == 6);
static_assert(sizeof(Vec3i) == 12);
static_assert(sizeof(Vec3f) == 12);
MM_DECLARE_MEMCOPY_SERIALIZABLE(Vec3s)
MM_DECLARE_MEMCOPY_SERIALIZABLE(Vec3i)
MM_DECLARE_MEMCOPY_SERIALIZABLE(Vec3f)


void snapshot(const Pid &src, uint16_t *dst);
void reconstruct(uint16_t src, Pid *dst);

void snapshot(const Vec3i &src, Vec3s *dst);
void reconstruct(const Vec3s &src, Vec3i *dst);

void snapshot(const Vec3f &src, Vec3s *dst);
void reconstruct(const Vec3s &src, Vec3f *dst);

void snapshot(const Vec3f &src, Vec3i *dst);
void reconstruct(const Vec3i &src, Vec3f *dst);

#pragma pack(push, 1)

struct BBoxs_MM7 {
    int16_t x1;
    int16_t x2;
    int16_t y1;
    int16_t y2;
    int16_t z1;
    int16_t z2;
};
static_assert(sizeof(BBoxs_MM7) == 12);
MM_DECLARE_MEMCOPY_SERIALIZABLE(BBoxs_MM7)

void snapshot(const BBoxi &src, BBoxs_MM7 *dst);
void reconstruct(const BBoxs_MM7 &src, BBoxi *dst);

void snapshot(const BBoxf &src, BBoxs_MM7 *dst);
void reconstruct(const BBoxs_MM7 &src, BBoxf *dst);

struct Planef_MM7 {
    Vec3f normal;
    float dist;
};
static_assert(sizeof(Planef_MM7) == 16);
MM_DECLARE_MEMCOPY_SERIALIZABLE(Planef_MM7)

void reconstruct(const Planef_MM7 &src, Planef *dst);


struct Planei_MM7 {
    Vec3i normal; // Fixpoint normal.
    int32_t dist; // Fixpoint -dot(normal, origin). Plane equation is dot(normal, x) + dist = 0.
};
static_assert(sizeof(Planei_MM7) == 16);
MM_DECLARE_MEMCOPY_SERIALIZABLE(Planei_MM7)

void reconstruct(const Planei_MM7 &src, Planef *dst);


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

void reconstruct(const SpriteFrame_MM7 &src, SpriteFrame *dst);


struct BLVFace_MM7 {
    Planef_MM7 facePlane;
    Planei_MM7 facePlaneOld;
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
    BBoxs_MM7 bounding;
    uint8_t polygonType;
    uint8_t numVertices;
    int16_t _pad;
};
static_assert(sizeof(BLVFace_MM7) == 0x60);
MM_DECLARE_MEMCOPY_SERIALIZABLE(BLVFace_MM7)

void reconstruct(const BLVFace_MM7 &src, BLVFace *dst);


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

void reconstruct(const TileDesc_MM7 &src, TileDesc *dst);


struct TextureFrame_MM7 {
    std::array<char, 12> textureName;
    int16_t textureID;
    int16_t animTime; // Total animation time, set only on the 1st frame, in 1/16th of a real-time second.
    int16_t animLength; // Frame duration, in 1/16th of a real-time second.
    int16_t flags;
};
static_assert(sizeof(TextureFrame_MM7) == 20);
MM_DECLARE_MEMCOPY_SERIALIZABLE(TextureFrame_MM7)

void reconstruct(const TextureFrame_MM7 &src, TextureFrame *dst);


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

void snapshot(const NPCData &src, NPCData_MM7 *dst);
void reconstruct(const NPCData_MM7 &src, NPCData *dst);


struct ItemGen_MM7 {
    /* 00 */ int32_t itemID;
    /* 04 */ int32_t attributeEnchantmentOrPotionPower; // Potion power for potions, attribute index + 1 for attribute enchantments.
    /* 08 */ int32_t enchantmentStrength;
    /* 0C */ int32_t specialEnchantmentOrGoldAmount; // Gold amount for gold, otherwise special enchantment.
    /* 10 */ int32_t numCharges;
    /* 14 */ uint32_t attributes;
    /* 18 */ uint8_t bodyAnchor;
    /* 19 */ uint8_t maxCharges;
    /* 1A */ uint8_t holderPlayer;
    /* 1B */ uint8_t placedInChest; // unknown unused 8-bit field, was repurposed
    /* 1C */ int64_t expireTime;
};
static_assert(sizeof(ItemGen_MM7) == 0x24);
MM_DECLARE_MEMCOPY_SERIALIZABLE(ItemGen_MM7)

void snapshot(const ItemGen &src, ItemGen_MM7 *dst);
void reconstruct(const ItemGen_MM7 &src, ItemGen *dst);


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

void snapshot(const SpellBuff &src, SpellBuff_MM7 *dst);
void reconstruct(const SpellBuff_MM7 &src, SpellBuff *dst);


struct LloydBeacon_MM7 {
    int64_t beaconTime;
    int32_t partyPosX;
    int32_t partyPosY;
    int32_t partyPosZ;
    int16_t partyViewYaw;
    int16_t partyViewPitch;
    uint16_t field_18;
    uint16_t mapIndexInGamesLod; // This one is convoluted - sane people would have stored a map id here,
                                 // but what we have here instead is the index of the map in games.lod.
};
static_assert(sizeof(LloydBeacon_MM7) == 0x1C);
MM_DECLARE_MEMCOPY_SERIALIZABLE(LloydBeacon_MM7)


struct CharacterConditions_MM7 {
    std::array<int64_t, 19> times;
    int64_t unused; // Conditions array was originally 20 elements long, but there's only 19 conditions in the game.
};
static_assert(sizeof(CharacterConditions_MM7) == 0xA0);
MM_DECLARE_MEMCOPY_SERIALIZABLE(CharacterConditions_MM7)

void snapshot(const RawCharacterConditions &src, CharacterConditions_MM7 *dst);
void reconstruct(const CharacterConditions_MM7 &src, RawCharacterConditions *dst);


struct Player_MM7 {
    /* 0000 */ CharacterConditions_MM7 conditions;
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
    /* 00C4 */ uint16_t personality;
    /* 00C6 */ uint16_t personalityBonus;
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
    /* 00E0 */ int32_t timer_E0; // These look like some timers. In the original binary they were decremented by ticks
                                 // elapsed on each frame, but weren't used anywhere. Not used in OE.
    /* 00E4 */ int32_t timer_E4;
    /* 00E8 */ int32_t timer_E8;
    /* 00EC */ int32_t timer_EC;
    /* 00F0 */ int32_t timer_F0;
    /* 00F4 */ int32_t timer_F4;
    /* 00F8 */ int32_t timer_F8;
    /* 00FC */ int32_t timer_FC;
    /* 0100 */ int32_t timer_100;
    /* 0104 */ int32_t timer_104;
    /* 0108 */ std::array<uint16_t, 37> activeSkills;
    /* 0152 */ std::array<uint8_t, 64> achievedAwardsBits;
    /* 0192 */ std::array<bool, 99> haveSpell;
    /* .... */ std::array<char, 3> _pad2;
    /* 01F8 */ int32_t pureLuckUsed;
    /* 01FC */ int32_t pureSpeedUsed;
    /* 0200 */ int32_t pureIntellectUsed;
    /* 0204 */ int32_t pureEnduranceUsed;
    /* 0208 */ int32_t purePersonalityUsed;
    /* 020C */ int32_t pureAccuracyUsed;
    /* 0210 */ int32_t pureMightUsed;
    /* 0214 */ std::array<ItemGen_MM7, 126> inventoryItems;
    /* .... */ std::array<ItemGen_MM7, 12> unusedItems;
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
    /* 1934 */ int16_t timeToRecovery; // Time left for the character to recover, in game ticks. Will overflow if
                                       // recovery is around 1 in-game hour, which never happens.
    /* 1936 */ char field_1936;
    /* 1937 */ char field_1937;
    /* 1938 */ uint32_t skillPoints;
    /* 193C */ int32_t health;
    /* 1940 */ int32_t mana;
    /* 1944 */ uint32_t birthYear;
    /* 1948 */ std::array<uint32_t, 16> equipment;
    /* 1988 */ std::array<int32_t, 49> field_1988; // field_1988[27] was set to 1 in party creation when character
                                                   // name was changed. We just set everything to zero.
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

void snapshot(const Character &src, Player_MM7 *dst);
void reconstruct(const Player_MM7 &src, Character *dst);


struct PartyTimeStruct_MM7 {
    /* 000 */ std::array<int64_t, 5> bountyHuntingNextGenerationTime;
              std::array<int64_t, 5> bountyHuntingNextGenerationTimeUnused; // Only first five elements are actually used, these are always zero.
    // Originally was one array
    /* 050 */ int64_t shopsNextGenerationTime0; // shop generation time for house id 0 which is invalid value
              std::array<int64_t, 52> shopsNextGenerationTime;
              std::array<int64_t, 32> guildsNextGenerationTime;
              int64_t shopBanTime0;
    /* 2F8 */ std::array<int64_t, 52> shopBanTimes;
    /* 4A0 */ std::array<int64_t, 10> counterEventValues;
    /* 4F0 */ std::array<int64_t, 29> historyEventTimes;
    /* 5D8 */ std::array<int64_t, 20> someOtherTimes;
    /* 678 */
};
static_assert(sizeof(PartyTimeStruct_MM7) == 0x678);
MM_DECLARE_MEMCOPY_SERIALIZABLE(PartyTimeStruct_MM7)


struct Party_MM7 {
    /* 00000 */ int32_t field_0; // Was set to 25 in Party::Reset & Party::Zero, not used for anything.
    /* 00004 */ uint32_t partyHeight;
    /* 00008 */ uint32_t defaultPartyHeight; // Unused in OE.
    /* 0000C */ int32_t eyeLevel;
    /* 00010 */ uint32_t defaultEyeLevel; // Unused in OE.
    /* 00014 */ int32_t radius;
    /* 00018 */ int32_t yawGranularity;
    /* 0001C */ uint32_t walkSpeed;
    /* 00020 */ int32_t yawRotationSpeed;  // deg/s
    /* 00024 */ int32_t jumpStrength;
    /* 00028 */ int32_t field_28;
    /* 0002C */ int64_t timePlayed;
    /* 00034 */ int64_t lastRegenerationTime;
    /* 0003C */ PartyTimeStruct_MM7 partyTimes;
    /* 006B4 */ Vec3i position;
    /* 006C0 */ int32_t viewYaw;
    /* 006C4 */ int32_t viewPitch;
    /* 006C8 */ Vec3i prevPosition;
    /* 006D4 */ int32_t viewPrevYaw;
    /* 006D8 */ int32_t viewPrevPitch;
    /* 006DC */ int32_t prevEyeLevel;
    /* 006E0 */ int32_t field_6E0; // Party old x/y?
    /* 006E4 */ int32_t field_6E4; // Party old x/y?
    /* 006E8 */ int32_t fallSpeed;
    /* 006EC */ int32_t field_6EC;
    /* 006F0 */ int32_t savedFlightZ;
    /* 006F4 */ int32_t floorFacePidUnused; // Face the party is standing at. Face id indoors, face pid outdoors.
                                            // Always set to zero on level loading, so in OE we are just saving 0 and
                                            // not using it when loading a savegame.
    /* 006F8 */ int32_t walkSoundTimerUnused; // This was removed in OE and we're just saving 0 in this field.
    /* 006FC */ int32_t waterLavaTimer; // Next game time when water/lava damage should be processed. This value will
                                        // overflow after ~16 in-game years, and then the lava logic will trigger on
                                        // the 1st frame after loading the game. We are OK with that.
    /* 00700 */ int32_t fallStartZ;
    /* 00704 */ uint32_t flying;
    /* 00708 */ char field_708; // Was set to 15 in Party::Reset & Party::Zero, not used for anything.
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
    /* 007A5 */ std::array<bool, 13> arcomageWins;
                std::array<bool, 3> arcomageWinsUnused; // Original array was 16 elements long, but we only have 13 taverns.
    /* 007B5 */ int8_t field_7B5_in_arena_quest; // -1 for a win, 0 for initial state, otherwise dialogue id for the
                                                 // arena level being fought.
    /* 007B6 */ std::array<uint8_t, 4> numArenaWins;
    /* 007BA */ std::array<bool, 29> isArtifactFound;  // 7ba
    /* 007D7 */ std::array<char, 39> field_7d7;
    /* 007FE */ std::array<uint8_t, 26> autonoteBits;
    /* 00818 */ std::array<char, 60> field_818;
    /* 00854 */ std::array<char, 32> randomNumbersUnused; // Array of random numbers, was filled during party creation
                                                          // and not used for anything. Probably a remnant of the old
                                                          // party creation code that randomized stats?
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
    /* 0777C */ std::array<ItemGen_MM7, 12> standartItemsInShop0;
                std::array<std::array<ItemGen_MM7, 12>, 52> standartItemsInShops;
    /* 0D0EC */ std::array<ItemGen_MM7, 12> specialItemsInShop0;
                std::array<std::array<ItemGen_MM7, 12>, 52> specialItemsInShops;
    /* 12A5C */ std::array<std::array<ItemGen_MM7, 12>, 32> spellBooksInGuilds;
    /* 1605C */ std::array<char, 24> field_1605C;
    /* 16074 */ std::array<char, 100> hireling1Name;
    /* 160D8 */ std::array<char, 100> hireling2Name;
    /* 1613C */ int32_t armageddonTimer;
    /* 16140 */ int32_t armageddonDamage;
    /* 16144 */ std::array<int32_t, 4> turnBasedPlayerRecoveryTimes;
                int32_t inTheShopFlag0; // Unused flag for HOUSE_INVALID.
    /* 16154 */ std::array<int32_t, 52> inTheShopFlags;
    /* 16228 */ int32_t fine;

    // Not sure why torchlight color is even stored in savegames in vanilla. Testing it on old savegames, it seems
    // it's always set to zero. But maybe savegames made in hardware mode differ from ones made in software mode,
    // and hardware mode actually sets torchlight color? Not sure how to check.
    // Anyway, reading torchlight color from a savegame makes very little sense, so we don't.
    /* 1622C */ std::array<float, 3> torchLightColorRgbUnused;
    /* 16238 */
};
static_assert(sizeof(Party_MM7) == 0x16238);
MM_DECLARE_MEMCOPY_SERIALIZABLE(Party_MM7)

void snapshot(const Party &src, Party_MM7 *dst);
void reconstruct(const Party_MM7 &src, Party *dst);

struct Timer_MM7 {
    /** Not used by the engine, was set to true for event timer & to false for misc timer.
     * Misc timer is never serialized, so we set it to true unconditionally. */
    uint32_t ready;

    /** Actually a bool. Whether the timer is paused. */
    uint32_t paused;

    /** Actually a bool. Means the timer is in turn-based mode. */
    int32_t turnBased;

    /** OS tick count, converted to game ticks, at the time of the last frame. */
    uint32_t lastFrameTime;

    /** OS tick count, converted to game ticks, when the timer was paused. Not used anywhere by the engine,
     * so we just set it to 0. */
    uint32_t pauseTime;

    /** OS tick count, converted to ticks, when the turn-based mode was enabled for this timer. Not used anywhere
     * by the engine, so we just set it to 0. */
    int32_t turnBasedTime;

    int32_t field_18;

    /** Game ticks since the last frame. */
    uint32_t timeElapsed;

    /** Time delta since the last frame in fixpoint realtime seconds. Not used in OE. */
    int32_t dtFixpoint;

    /** Total game ticks elapsed. */
    uint32_t totalGameTimeElapsed;
};
static_assert(sizeof(Timer_MM7) == 0x28);
MM_DECLARE_MEMCOPY_SERIALIZABLE(Timer_MM7)

void snapshot(const RawTimer &src, Timer_MM7 *dst);
void reconstruct(const Timer_MM7 &src, RawTimer *dst);


struct ActiveOverlay_MM7 {
    /* 00 */ int16_t field_0;
    /* 02 */ int16_t indexToOverlayList;
    /* 04 */ int16_t spriteFrameTime;
    /* 06 */ int16_t animLength;
    /* 08 */ int16_t screenSpaceX;
    /* 0A */ int16_t screenSpaceY;
    /* 0C */ uint16_t pid;
    /* 0E */ int16_t projSize;
    /* 10 */ int32_t fpDamageMod;
    /* 14 */
};
static_assert(sizeof(ActiveOverlay_MM7) == 0x14);
MM_DECLARE_MEMCOPY_SERIALIZABLE(ActiveOverlay_MM7)

void snapshot(const ActiveOverlay &src, ActiveOverlay_MM7 *dst);
void reconstruct(const ActiveOverlay_MM7 &src, ActiveOverlay *dst);


struct ActiveOverlayList_MM7 {
    /* 000 */ std::array<ActiveOverlay_MM7, 50> overlays;
    /* 3E8 */ int32_t field_3E8;
    /* 3EC */ int32_t redraw;
    /* 3F0 */
};
static_assert(sizeof(ActiveOverlayList_MM7) == 0x3F0);
MM_DECLARE_MEMCOPY_SERIALIZABLE(ActiveOverlayList_MM7)

void snapshot(const ActiveOverlayList &src, ActiveOverlayList_MM7 *dst);
void reconstruct(const ActiveOverlayList_MM7 &src, ActiveOverlayList *dst);


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

void snapshot(const Icon &src, IconFrame_MM7 *dst);
void reconstruct(const IconFrame_MM7 &src, Icon *dst);


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

void snapshot(const UIAnimation &src, UIAnimation_MM7 *dst);
void reconstruct(const UIAnimation_MM7 &src, UIAnimation *dst);


struct MonsterInfo_MM7 {
    int32_t name;
    int32_t textureName;
    uint8_t level;
    uint8_t treasureDropChance;
    uint8_t goldDiceRolls;
    uint8_t goldDiceSides;
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
    uint8_t attack1MissileType;
    uint8_t attack2Chance;
    uint8_t attack2Type;
    uint8_t attack2DamageDiceRolls;
    uint8_t attack2DamageDiceSides;
    uint8_t attack2DamageBonus;
    uint8_t attack2MissileType;
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
    char _pad;
    uint16_t id;
    uint16_t bloodSplatOnDeath;
    uint16_t spell1SkillMastery;
    uint16_t spell2SkillMastery;
    int16_t field_3C_some_special_attack;
    int16_t field_3E;
    uint32_t hp;
    uint32_t ac;
    uint32_t exp;
    uint32_t baseSpeed;
    int32_t recoveryTime;
    uint32_t attackPreferences;
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
    std::array<std::array<char, 10>, 8> spriteNames;
    std::array<std::array<char, 10>, 2> spriteNamesUnused;
};
static_assert(sizeof(MonsterDesc_MM6) == 148);
MM_DECLARE_MEMCOPY_SERIALIZABLE(MonsterDesc_MM6)

void reconstruct(const MonsterDesc_MM6 &src, MonsterDesc *dst);


struct MonsterDesc_MM7 {
    uint16_t monsterHeight;
    uint16_t monsterRadius;
    uint16_t movementSpeed;
    int16_t toHitRadius;
    uint32_t tintColor;
    std::array<uint16_t, 4> soundSampleIds;
    std::array<char, 32> monsterName;
    std::array<std::array<char, 10>, 8> spriteNames;
    std::array<std::array<char, 10>, 2> spriteNamesUnused;
};
static_assert(sizeof(MonsterDesc_MM7) == 152);
MM_DECLARE_MEMCOPY_SERIALIZABLE(MonsterDesc_MM7)

void snapshot(const MonsterDesc &src, MonsterDesc_MM7 *dst);
void reconstruct(const MonsterDesc_MM7 &src, MonsterDesc *dst);


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

void snapshot(const ActorJob &src, ActorJob_MM7 *dst);
void reconstruct(const ActorJob_MM7 &src, ActorJob *dst);


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
    uint16_t _pad;
    uint32_t uCurrentActionTime;
    std::array<uint16_t, 8> pSpriteIDs;
    std::array<uint16_t, 4> pSoundSampleIDs;  // 1 die     3 bored
    SpellBuff_MM7 actorBuffZeroUnused; // An artifact of the original memory layout, zero is ACTOR_BUFF_NONE.
                                       // It's not used for anything in vanilla and simply dropped in OE.
    std::array<SpellBuff_MM7, 21> pActorBuffs;
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

void snapshot(const Actor &src, Actor_MM7 *dst);
void reconstruct(const Actor_MM7 &src, Actor *dst);


struct BLVDoor_MM7 {
    uint32_t uAttributes;
    uint32_t uDoorID;
    uint32_t uTimeSinceTriggered;
    Vec3i vDirection;
    uint32_t uMoveLength;
    uint32_t uCloseSpeed;
    uint32_t uOpenSpeed;
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
    int16_t _pad;
};
static_assert(sizeof(BLVDoor_MM7) == 0x50);
MM_DECLARE_MEMCOPY_SERIALIZABLE(BLVDoor_MM7)

void snapshot(const BLVDoor &src, BLVDoor_MM7 *dst);
void reconstruct(const BLVDoor_MM7 &src, BLVDoor *dst);


struct BLVSector_MM7 {
    int32_t field_0;
    uint16_t uNumFloors;
    int16_t _pad0;
    uint32_t pFloors;
    uint16_t uNumWalls;
    int16_t _pad1;
    uint32_t pWalls;
    uint16_t uNumCeilings;
    int16_t _pad2;
    uint32_t pCeilings;
    uint16_t uNumFluids;
    int16_t _pad3;
    uint32_t pFluids;
    int16_t uNumPortals;
    int16_t _pad4;
    uint32_t pPortals;
    uint16_t uNumFaces;
    uint16_t uNumNonBSPFaces;
    uint32_t pFaceIDs;
    uint16_t uNumCylinderFaces;
    int16_t _pad5;
    int32_t pCylinderFaces;
    uint16_t uNumCogs;
    int16_t _pad6;
    uint32_t pCogs;
    uint16_t uNumDecorations;
    int16_t _pad7;
    uint32_t pDecorationIDs;
    uint16_t uNumMarkers;
    int16_t _pad8;
    uint32_t pMarkers;
    uint16_t uNumLights;
    int16_t _pad9;
    uint32_t pLights;
    int16_t uWaterLevel;
    int16_t uMistLevel;
    int16_t uLightDistanceMultiplier;
    int16_t uMinAmbientLightLevel;
    int16_t uFirstBSPNode;
    int16_t exit_tag;
    BBoxs_MM7 pBounding;
};
static_assert(sizeof(BLVSector_MM7) == 0x74);
MM_DECLARE_MEMCOPY_SERIALIZABLE(BLVSector_MM7)

void snapshot(const BLVSector &src, BLVSector_MM7 *dst);
void reconstruct(const BLVSector_MM7 &src, BLVSector *dst);


struct GUICharMetric_MM7 {
    int32_t uLeftSpacing;
    int32_t uWidth;
    int32_t uRightSpacing;
};
static_assert(sizeof(GUICharMetric_MM7) == 12);
MM_DECLARE_MEMCOPY_SERIALIZABLE(GUICharMetric_MM7)

void snapshot(const GUICharMetric &src, GUICharMetric_MM7 *dst);
void reconstruct(const GUICharMetric_MM7 &src, GUICharMetric *dst);


struct FontHeader_MM7 {
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
    // array of font pixels follows in the serialized representation.
};
static_assert(sizeof(FontHeader_MM7) == 0x1020);
MM_DECLARE_MEMCOPY_SERIALIZABLE(FontHeader_MM7)

void reconstruct(const FontHeader_MM7 &src, FontHeader *dst);


struct ODMFace_MM7 {
    Planei_MM7 facePlane;
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
    BBoxs_MM7 pBoundingBox;
    int16_t sCogNumber;
    int16_t sCogTriggeredID;
    int16_t sCogTriggerType;
    char field_128;
    char field_129;
    uint8_t uGradientVertex1; // Not sure what these are, unused in OpenEnroth.
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

void reconstruct(const ODMFace_MM7 &src, ODMFace *dst);


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

void reconstruct(const SpawnPoint_MM7 &src, SpawnPoint *dst);


struct SpriteObject_MM7 {
    uint16_t uType;
    uint16_t uObjectDescID;
    Vec3i vPosition;
    Vec3s vVelocity;
    uint16_t uFacing;
    uint16_t uSoundID;
    uint16_t uAttributes;
    int16_t uSectorID;
    uint16_t uTimeSinceCreated;
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
    char spellCasterAbility;
    uint16_t _pad;
    Vec3i initialPosition;
};

static_assert(sizeof(SpriteObject_MM7) == 0x70);
MM_DECLARE_MEMCOPY_SERIALIZABLE(SpriteObject_MM7)

void snapshot(const SpriteObject &src, SpriteObject_MM7 *dst);
void reconstruct(const SpriteObject_MM7 &src, SpriteObject *dst);


struct ChestDesc_MM7 {
    std::array<char, 32> pName;
    uint8_t uWidth;
    uint8_t uHeight;
    int16_t uTextureID;
};
static_assert(sizeof(ChestDesc_MM7) == 36);
MM_DECLARE_MEMCOPY_SERIALIZABLE(ChestDesc_MM7)

void reconstruct(const ChestDesc_MM7 &src, ChestDesc *dst);


struct DecorationDesc_MM6 {
    std::array<char, 32> name;
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

void reconstruct(const DecorationDesc_MM6 &src, DecorationDesc *dst);
void reconstruct(const DecorationDesc_MM7 &src, DecorationDesc *dst);


struct Chest_MM7 {
    uint16_t uChestBitmapID;
    uint16_t uFlags;
    std::array<ItemGen_MM7, 140> igChestItems;
    std::array<int16_t, 140> pInventoryIndices;
};
static_assert(sizeof(Chest_MM7) == 5324);
MM_DECLARE_MEMCOPY_SERIALIZABLE(Chest_MM7)

void snapshot(const Chest &src, Chest_MM7 *dst);
void reconstruct(const Chest_MM7 &src, Chest *dst);


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

void reconstruct(const BLVLight_MM7 &src, BLVLight *dst);


struct OverlayDesc_MM7 {
    uint16_t uOverlayID;
    uint16_t uOverlayType;
    uint16_t uSpriteFramesetID;
    int16_t spriteFramesetGroup;
};
static_assert(sizeof(OverlayDesc_MM7) == 8);
MM_DECLARE_MEMCOPY_SERIALIZABLE(OverlayDesc_MM7)

void reconstruct(const OverlayDesc_MM7 &src, OverlayDesc *dst);


struct PlayerFrame_MM7 {
    uint16_t expression;
    uint16_t uTextureID;
    int16_t uAnimTime;
    int16_t uAnimLength;
    int16_t uFlags;
};
static_assert(sizeof(PlayerFrame_MM7) == 10);
MM_DECLARE_MEMCOPY_SERIALIZABLE(PlayerFrame_MM7)

void reconstruct(const PlayerFrame_MM7 &src, PlayerFrame *dst);


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

void reconstruct(const LevelDecoration_MM7 &src, LevelDecoration *dst);


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

void reconstruct(const BLVFaceExtra_MM7 &src, BLVFaceExtra *dst);


struct BSPNode_MM7 {
    int16_t uFront;
    int16_t uBack;
    int16_t uBSPFaceIDOffset;
    int16_t uNumBSPFaces;
};
static_assert(sizeof(BSPNode_MM7) == 8);
MM_DECLARE_MEMCOPY_SERIALIZABLE(BSPNode_MM7)

void reconstruct(const BSPNode_MM7 &src, BSPNode *dst);


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

void reconstruct(const BLVMapOutline_MM7 &src, BLVMapOutline *dst);


struct ObjectDesc_MM6 {
    std::array<char, 32> name;
    int16_t uObjectID;
    int16_t uRadius;
    int16_t uHeight;
    int16_t uFlags;
    uint16_t uSpriteID;
    int16_t uLifetime;
    uint16_t uParticleTrailColor16;
    int16_t uSpeed;
    uint8_t uParticleTrailColorR;
    uint8_t uParticleTrailColorG;
    uint8_t uParticleTrailColorB;
    char _pad;
};
static_assert(sizeof(ObjectDesc_MM6) == 52);
MM_DECLARE_MEMCOPY_SERIALIZABLE(ObjectDesc_MM6)

struct ObjectDesc_MM7 {
    std::array<char, 32> name;
    int16_t uObjectID;
    int16_t uRadius;
    int16_t uHeight;
    uint16_t uFlags;
    uint16_t uSpriteID;
    int16_t uLifetime;
    uint32_t uParticleTrailColor32;
    int16_t uSpeed;
    uint8_t uParticleTrailColorR;
    uint8_t uParticleTrailColorG;
    uint8_t uParticleTrailColorB;
    std::array<char, 3> _pad;
};
static_assert(sizeof(ObjectDesc_MM7) == 56);
MM_DECLARE_MEMCOPY_SERIALIZABLE(ObjectDesc_MM7)

void reconstruct(const ObjectDesc_MM6 &src, ObjectDesc *dst);
void reconstruct(const ObjectDesc_MM7 &src, ObjectDesc *dst);


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

void snapshot(const LocationTime &src, LocationTime_MM7 *dst);
void reconstruct(const LocationTime_MM7 &src, LocationTime *dst);


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

void reconstruct(const SoundInfo_MM6 &src, SoundInfo *dst);
void reconstruct(const SoundInfo_MM7 &src, SoundInfo *dst);


struct LocationInfo_MM7 {
    int32_t respawnCount;
    int32_t lastRespawnDay;
    int32_t reputation;
    int32_t alertStatus;
};
static_assert(sizeof(LocationInfo_MM7) == 16);
MM_DECLARE_MEMCOPY_SERIALIZABLE(LocationInfo_MM7)

void snapshot(const LocationInfo &src, LocationInfo_MM7 *dst);
void reconstruct(const LocationInfo_MM7 &src, LocationInfo *dst);


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
// so no reconstruct() overloads for it.


struct PersistentVariables_MM7 {
    std::array<unsigned char, 75> mapVars;
    std::array<unsigned char, 125> decorVars;
};
static_assert(sizeof(PersistentVariables_MM7) == 0xC8);
MM_DECLARE_MEMCOPY_SERIALIZABLE(PersistentVariables_MM7)

void snapshot(const PersistentVariables &src, PersistentVariables_MM7 *dst);
void reconstruct(const PersistentVariables_MM7 &src, PersistentVariables *dst);


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
// so no reconstruct() overloads for it.


struct OutdoorLocationTileType_MM7 {
    uint16_t tileset;
    uint16_t tileId;
};
static_assert(sizeof(OutdoorLocationTileType_MM7) == 4);
MM_DECLARE_MEMCOPY_SERIALIZABLE(OutdoorLocationTileType_MM7)

void reconstruct(const OutdoorLocationTileType_MM7 &src, OutdoorLocationTileType *dst);


struct SaveGameHeader_MM7 {
    std::array<char, 20> name;
    std::array<char, 20> locationName;
    int64_t playingTime;
    std::array<char, 52> field_30;
};
static_assert(sizeof(SaveGameHeader_MM7) == 0x64);
MM_DECLARE_MEMCOPY_SERIALIZABLE(SaveGameHeader_MM7)

void snapshot(const SaveGameHeader &src, SaveGameHeader_MM7 *dst);
void reconstruct(const SaveGameHeader_MM7 &src, SaveGameHeader *dst);


#pragma pack(pop)
