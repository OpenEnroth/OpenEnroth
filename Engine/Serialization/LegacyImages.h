#pragma once

#include "Engine/Graphics/ParticleEngine.h"

#include "Engine/Objects/Actor.h"

#include "Engine/Spells/Spells.h"

#include "Engine/VectorTypes.h"

#include "GUI/GUIFont.h"

/*
 *  Party, timers, NPCs and other stuff is binary dumped into resources/savegame
 * files, which limits ability to alter these structures without breaking
 * compatibility. This module is used to serialzie and deserialize native game
 * structures intro their current representations without breaking compatibility
 * with original files.
 */

/*   42 */
#pragma pack(push, 1)
struct SpriteFrame_MM6 {
    SpriteFrame_MM6();

    char pIconName[12];
    char pTextureName[12];    // c
    int16_t pHwSpriteIDs[8];  // 18h
    int32_t scale;            // 28h
    int uFlags;               // 2c
    int16_t uGlowRadius;      // 30
    int16_t uPaletteID;       // 32
    int16_t uPaletteIndex;
    int16_t uAnimTime;
};

struct SpriteFrame_MM7 : public SpriteFrame_MM6 {
    SpriteFrame_MM7();

    int16_t uAnimLength;
    int16_t _pad;
};
#pragma pack(pop)

/*   93 */
#pragma pack(push, 1)
struct BLVFace_MM7 {  // 60h
    struct Plane_float_ pFacePlane;
    struct Plane_int_ pFacePlane_old;
    int zCalc1;
    int zCalc2;
    int zCalc3;
    unsigned int uAttributes;
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
    struct BBox_short_ pBounding;
    uint8_t uPolygonType;
    uint8_t uNumVertices;
    char field_5E;
    char field_5F;
};
#pragma pack(pop)

/*   48 */
#pragma pack(push, 1)
struct TileDesc_MM7 {  // 26
    char pTileName[16];
    uint16_t uTileID;
    uint16_t uBitmapID;
    uint16_t tileset;
    uint16_t uSection;
    uint16_t uAttributes;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct TextureFrame_MM7 {
    char pTextureName[12];
    int16_t uTextureID;
    int16_t uAnimTime;
    int16_t uAnimLength;
    int16_t uFlags;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct NPCData_Image_MM7 {
    NPCData_Image_MM7();

    void Serialize(struct NPCData *item);
    void Deserialize(struct NPCData *item);

    /* 00 */ int32_t pName;  // char *pName;
    /* 04 */ unsigned int uPortraitID;
    /* 08 */ unsigned int
        uFlags;  // & 0x80    no greeting on dialogue start; looks like hired
    /* 0C */ int fame;
    /* 10 */ int rep;
    /* 14 */ unsigned int Location2D;
    /* 18 */ unsigned int uProfession;
    /* 1C */ int greet;
    /* 20 */ int joins;
    /* 24 */ int field_24;
    /* 28 */ unsigned int evt_A;
    /* 2C */ unsigned int evt_B;
    /* 30 */ unsigned int evt_C;
    /* 34 */ unsigned int evt_D;
    /* 38 */ unsigned int evt_E;
    /* 3C */ unsigned int evt_F;
    /* 40 */ unsigned int uSex;
    /* 44 */ int bHasUsedTheAbility;
    /* 48 */ int news_topic;
    /* 4C */
};

struct ItemGen_Image_MM7 {
    ItemGen_Image_MM7();

    void Serialize(struct ItemGen *item);
    void Deserialize(struct ItemGen *item);

    /* 00 */ int uItemID;
    /* 04 */ int uEnchantmentType;
    /* 08 */ int m_enchantmentStrength;
    /* 0C */ int special_enchantment;
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
    // 67  Adds 5 points of Body damage and +2 Disarm skill.
    // 68  Adds 6-8 points of Cold damage and +5 Armor Class.
    // 71  Prevents drowning damage.
    // 72  Prevents falling damage.
    /* 10 */ int uNumCharges;
    /* 14 */ unsigned int uAttributes;
    /* 18 */ uint8_t uBodyAnchor;
    /* 19 */ char uMaxCharges;
    /* 1A */ char uHolderPlayer;
    /* 1B */ char field_1B;
    /* 1C */ uint64_t uExpireTime;
};

struct SpellBuff_Image_MM7 {
    SpellBuff_Image_MM7();

    void Serialize(struct SpellBuff *item);
    void Deserialize(struct SpellBuff *item);

    /* 00 */ int64_t uExpireTime;
    /* 08 */ uint16_t uPower;
    /* 0A */ uint16_t uSkill;
    /* 0C */ uint16_t uOverlayID;
    /* 0E */ uint8_t uCaster;
    /* 0F */ uint8_t uFlags;
    /* 10 */
};

struct PlayerSpellbookChapter_Image_MM7 {
    PlayerSpellbookChapter_Image_MM7();

    /* 00 */ char bIsSpellAvailable[11];
    /* 0B */
};
struct PlayerSpells_Image_MM7 {
    PlayerSpells_Image_MM7();

    union {
        struct {
            /* 00 */ PlayerSpellbookChapter_Image_MM7 pChapters[9];
            /* 63 */ char _pad1;
            /* 64 */
        };
        struct {
            /* 00 */ char bHaveSpell[99];
            /* 63 */ char _pad2;
            /* 64 */
        };
    };
};

union PlayerEquipment_Image_MM7 {
    PlayerEquipment_Image_MM7();

    union {
        struct {
            /* 00 */ unsigned int uShield;
            /* 04 */ unsigned int uMainHand;
            /* 08 */ unsigned int uBow;
            /* 0C */ unsigned int uArmor;
            /* 10 */ unsigned int uHelm;
            /* 14 */ unsigned int uBelt;
            /* 18 */ unsigned int uCloak;
            /* 1C */ unsigned int uGlove;
            /* 20 */ unsigned int uBoot;
            /* 24 */ unsigned int uAmulet;
            /* 28 */ unsigned int uRings[6];
            /* 40 */
        };
        unsigned int pIndices[16];
    };
};

struct LloydBeacon_Image_MM7 {
    LloydBeacon_Image_MM7();

    /* 00 */ uint64_t uBeaconTime;
    /* 08 */ int32_t PartyPos_X;
    /* 0C */ int32_t PartyPos_Y;
    /* 10 */ int32_t PartyPos_Z;
    /* 14 */ int16_t PartyRot_X;
    /* 16 */ int16_t PartyRot_Y;
    /* 18 */ uint16_t unknown;
    /* 1A */ uint16_t SaveFileID;
    /* 1C */
};

struct Player_Image_MM7 {
    Player_Image_MM7();

    void Serialize(struct Player *);
    void Deserialize(struct Player *);

    /* 0000 */ int64_t pConditions[20];
    /* 00A0 */ uint64_t uExperience;
    /* 00A8 */ char pName[16];
    /* 00B8 */ unsigned char uSex;
    /* 00B9 */ unsigned char classType;
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
    /* 00E0 */ int field_E0;
    /* 00E4 */ int field_E4;
    /* 00E8 */ int field_E8;
    /* 00EC */ int field_EC;
    /* 00F0 */ int field_F0;
    /* 00F4 */ int field_F4;
    /* 00F8 */ int field_F8;
    /* 00FC */ int field_FC;
    /* 0100 */ int field_100;
    /* 0104 */ int field_104;
    /* 0108 */ union {
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
        };
        uint16_t pActiveSkills[37];
    };
    /* 0152 */ unsigned char _achieved_awards_bits[64];
    /* 0192 */ PlayerSpells_Image_MM7 spellbook;
    /* 01F6 */ char _1F6_pad[2];
    /* 01F8 */ int pure_luck_used;
    /* 01FC */ int pure_speed_used;
    /* 0200 */ int pure_intellect_used;
    /* 0204 */ int pure_endurance_used;
    /* 0208 */ int pure_willpower_used;
    /* 020C */ int pure_accuracy_used;
    /* 0210 */ int pure_might_used;
    /* 0214 */ union {
        struct {
            ItemGen_Image_MM7 pInventoryItemList[126];
            ItemGen_Image_MM7 pEquippedItems[12];
        };
        struct {
            ItemGen_Image_MM7 pOwnItems[138];
        };
    };
    /* 157C */ int pInventoryMatrix[126];
    /* 1774 */ int16_t sResFireBase;
    /* 1776 */ int16_t sResAirBase;
    /* 1778 */ int16_t sResWaterBase;
    /* 177A */ int16_t sResEarthBase;
    /* 177C */ int16_t field_177C;
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
    /* 1792 */ int16_t field_1792;
    /* 1794 */ int16_t sResMagicBonus;
    /* 1796 */ int16_t sResSpiritBonus;
    /* 1798 */ int16_t sResMindBonus;
    /* 179A */ int16_t sResBodyBonus;
    /* 179C */ int16_t sResLightBonus;
    /* 179E */ int16_t sResDarkBonus;
    /* 17A0 */ SpellBuff_Image_MM7 pPlayerBuffs[24];
    /* 1920 */ unsigned int uVoiceID;
    /* 1924 */ int uPrevVoiceID;
    /* 1928 */ int uPrevFace;
    /* 192C */ int field_192C;
    /* 1930 */ int field_1930;
    /* 1934 */ uint16_t uTimeToRecovery;
    /* 1936 */ char field_1936;
    /* 1937 */ char field_1937;
    /* 1938 */ unsigned int uSkillPoints;
    /* 193C */ int sHealth;
    /* 1940 */ int sMana;
    /* 1944 */ unsigned int uBirthYear;
    /* 1948 */ PlayerEquipment_Image_MM7 pEquipment;
    /* 1988 */ int field_1988[49];
    /* 1A4C */ char field_1A4C;
    /* 1A4D */ char field_1A4D;
    /* 1A4E */ char lastOpenedSpellbookPage;
    /* 1A4F */ uint8_t uQuickSpell;
    /* 1A50 */ char playerEventBits[64];
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
    /* 1AA4 */ int _expression21_animtime;
    /* 1AA8 */ int _expression21_frameset;
    /* 1AAC */ LloydBeacon_Image_MM7 pInstalledBeacons[5];
    /* 1B38 */ char uNumDivineInterventionCastsThisDay;
    /* 1B39 */ char uNumArmageddonCasts;
    /* 1B3A */ char uNumFireSpikeCasts;
    /* 1B3B */ char field_1B3B;
    /* 1B3C */
};

struct PartyTimeStruct_Image_MM7 {
    PartyTimeStruct_Image_MM7();

    /* 000 */ int64_t bountyHunting_next_generation_time[10];
    /* 050 */ int64_t Shops_next_generation_time[85];  // field_50
    /* 2F8 */ int64_t _shop_ban_times[53];
    /* 4A0 */ uint64_t CounterEventValues[10];               // (0xACD314h in Silvo's binary)
    /* 4F0 */ int64_t HistoryEventTimes[29];  // (0xACD364h in Silvo's binary)
    /* 5D8 */ uint64_t _s_times[20];  // 5d8 440h+8*51     //(0xACD44Ch in Silvo's binary)
    /* 678 */
};

struct Party_Image_MM7 {
    Party_Image_MM7();

    void Serialize(struct Party *);
    void Deserialize(struct Party *);

    /* 00000 */ int field_0;
    /* 00004 */ unsigned int uPartyHeight;
    /* 00008 */ unsigned int uDefaultPartyHeight;
    /* 0000C */ int sEyelevel;
    /* 00010 */ unsigned int uDefaultEyelevel;
    /* 00014 */ int field_14_radius;
    /* 00018 */ int y_rotation_granularity;
    /* 0001C */ unsigned int uWalkSpeed;
    /* 00020 */ int y_rotation_speed;  // deg/s
    /* 00024 */ int field_24;
    /* 00028 */ int field_28;
    /* 0002C */ uint64_t uTimePlayed;
    /* 00034 */ int64_t uLastRegenerationTime;
    /* 0003C */ PartyTimeStruct_Image_MM7 PartyTimes;
    /* 006B4 */ Vec3_int_ vPosition;
    /* 006C0 */ int sRotationZ;
    /* 006C4 */ int sRotationX;
    /* 006C8 */ Vec3_int_ vPrevPosition;
    /* 006D4 */ int sPrevRotationY;
    /* 006D8 */ int sPrevRotationX;
    /* 006DC */ int sPrevEyelevel;
    /* 006E0 */ int field_6E0;
    /* 006E4 */ int field_6E4;
    /* 006E8 */ int uFallSpeed;
    /* 006EC */ int field_6EC;
    /* 006F0 */ int field_6F0;
    /* 006F4 */ int floor_face_pid;  // face we are standing at
    /* 006F8 */ int walk_sound_timer;
    /* 006FC */ int _6FC_water_lava_timer;
    /* 00700 */ int uFallStartY;
    /* 00704 */ unsigned int bFlying;
    /* 00708 */ char field_708;
    /* 00709 */ uint8_t hirelingScrollPosition;
    /* 0070A */ char field_70A;
    /* 0070B */ char field_70B;
    /* 0070C */ unsigned int uCurrentYear;
    /* 00710 */ unsigned int uCurrentMonth;
    /* 00714 */ unsigned int uCurrentMonthWeek;
    /* 00718 */ unsigned int uCurrentDayOfMonth;
    /* 0071C */ unsigned int uCurrentHour;
    /* 00720 */ unsigned int uCurrentMinute;
    /* 00724 */ unsigned int uCurrentTimeSecond;
    /* 00728 */ unsigned int uNumFoodRations;
    /* 0072C */ int field_72C;
    /* 00730 */ int field_730;
    /* 00734 */ unsigned int uNumGold;
    /* 00738 */ unsigned int uNumGoldInBank;
    /* 0073C */ unsigned int uNumDeaths;
    /* 00740 */ int field_740;
    /* 00744 */ int uNumPrisonTerms;
    /* 00748 */ unsigned int uNumBountiesCollected;
    /* 0074C */ int field_74C;
    /* 00750 */ int16_t monster_id_for_hunting[5];
    /* 0075A */ int16_t monster_for_hunting_killed[5];
    /* 00764 */ unsigned char days_played_without_rest;
    /* 00765 */ uint8_t _quest_bits[64];
    /* 007A5 */ uint8_t pArcomageWins[16];
    /* 007B5 */ char field_7B5_in_arena_quest;
    /* 007B6 */ char uNumArenaPageWins;
    /* 007B7 */ char uNumArenaSquireWins;
    /* 007B8 */ char uNumArenaKnightWins;
    /* 007B9 */ char uNumArenaLordWins;
    /* 007BA */ char pIsArtifactFound[29];  // 7ba
    /* 007D7 */ char field_7d7[39];
    /* 007FE */ unsigned char _autonote_bits[26];
    /* 00818 */ char field_818[60];
    /* 00854 */ char field_854[32];
    /* 00874 */ int uNumArcomageWins;
    /* 00878 */ int uNumArcomageLoses;
    /* 0087C */ unsigned int bTurnBasedModeOn;
    /* 00880 */ int field_880;
    /* 00884 */ int uFlags2;
    /* 00888 */ unsigned int alignment;
    /* 0088C */ SpellBuff_Image_MM7 pPartyBuffs[20];
    /* 00954 */ Player_Image_MM7 pPlayers[4];
    /* 07644 */ NPCData_Image_MM7 pHirelings[2];
    /* 07754 */ ItemGen_Image_MM7 pPickedItem;
    /* 07778 */ unsigned int uFlags;
    /* 0777C */ ItemGen_Image_MM7 StandartItemsInShops[53][12];
    /* 0D0EC */ ItemGen_Image_MM7 SpecialItemsInShops[53][12];
    /* 12A5C */ ItemGen_Image_MM7 SpellBooksInGuilds[32][12];
    /* 1605C */ char field_1605C[24];
    /* 16074 */ char pHireling1Name[100];
    /* 160D8 */ char pHireling2Name[100];
    /* 1613C */ int armageddon_timer;
    /* 16140 */ int armageddonDamage;
    /* 16144 */ int pTurnBasedPlayerRecoveryTimes[4];
    /* 16154 */ int InTheShopFlags[53];
    /* 16228 */ int uFine;
    /* 1622C */ float flt_TorchlightColorR;
    /* 16230 */ float flt_TorchlightColorG;
    /* 16234 */ float flt_TorchlightColorB;
    /* 16238 */
};

struct Timer_Image_MM7 {
    Timer_Image_MM7();

    void Serialize(struct Timer *);
    void Deserialize(struct Timer *);

    /* 00 */ uint32_t bReady;
    /* 04 */ uint32_t bPaused;
    /* 08 */ int32_t bTackGameTime;
    /* 0C */ uint32_t uStartTime;
    /* 10 */ uint32_t uStopTime;
    /* 14 */ int32_t uGameTimeStart;
    /* 18 */ int32_t field_18;
    /* 1C */ uint32_t uTimeElapsed;
    /* 20 */ int32_t dt_in_some_format;
    /* 24 */ uint32_t uTotalGameTimeElapsed;
    /* 28 */
};

/*  282 */
struct OtherOverlay_Image_MM7 {
    OtherOverlay_Image_MM7();

    /* 00 */ int16_t field_0;
    /* 02 */ int16_t field_2;
    /* 04 */ int16_t sprite_frame_time;
    /* 06 */ int16_t field_6;
    /* 08 */ int16_t screen_space_x;
    /* 0A */ int16_t screen_space_y;
    /* 0C */ int16_t field_C;
    /* 0E */ int16_t field_E;
    /* 10 */ int field_10;
    /* 14 */
};

/*   63 */
struct OtherOverlayList_Image_MM7 {
    OtherOverlayList_Image_MM7();

    void Serialize(struct OtherOverlayList *);
    void Deserialize(struct OtherOverlayList *);

    /* 000 */ OtherOverlay_Image_MM7 pOverlays[50];
    /* 3E8 */ int field_3E8;
    /* 3EC */ int bRedraw;
    /* 3F0 */
};
#pragma pack(pop)

/*   44 */
#pragma pack(push, 1)
struct IconFrame_MM7 {
    IconFrame_MM7();

    void Serialize(class Icon *);
    void Deserialize(class Icon *);

    /* 000 */ char pAnimationName[12];
    /* 00C */ char pTextureName[12];
    /* 018 */ int16_t uAnimTime;
    /* 01A */ int16_t uAnimLength;
    /* 01C */ int16_t uFlags;  // 0x01 - more icons in this animation
    /* 01E */ uint16_t uTextureID;
};
#pragma pack(pop)

/*   76 */
#pragma pack(push, 1)
struct UIAnimation_MM7 {
    UIAnimation_MM7();

    void Serialize(class UIAnimation *);
    void Deserialize(class UIAnimation *);

    /* 000 */ uint16_t uIconID;
    /* 002 */ int16_t field_2;
    /* 004 */ int16_t uAnimTime;
    /* 006 */ int16_t uAnimLength;
    /* 008 */ int16_t x;
    /* 00A */ int16_t y;
    /* 00C */ char field_C;
};
#pragma pack(pop)

/*  187 */
#pragma pack(push, 1)
struct MonsterInfo_MM7 {
    MonsterInfo_MM7();

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
    uint16_t bQuestMonster;
    uint16_t uSpellSkillAndMastery1;
    uint16_t uSpellSkillAndMastery2;
    int16_t field_3C_some_special_attack;
    int16_t field_3E;
    unsigned int uHP;
    unsigned int uAC;
    unsigned int uExp;
    unsigned int uBaseSpeed;
    signed int uRecoveryTime;
    unsigned int uAttackPreference;
};
#pragma pack(pop)

/*   66 */
#pragma pack(push, 1)
struct Actor_MM7 {
    Actor_MM7();

    void Serialize(class Actor *);
    void Deserialize(class Actor *);

    char pActorName[32];
    int16_t sNPC_ID;
    int16_t field_22;
    unsigned int uAttributes;
    int16_t sCurrentHP;
    char field_2A[2];
    struct MonsterInfo_MM7 pMonsterInfo;
    int16_t word_000084_range_attack;
    int16_t word_000086_some_monster_id;  // base monster class monsterlist id
    uint16_t uActorRadius;
    uint16_t uActorHeight;
    uint16_t uMovementSpeed;
    struct Vec3_short_ vPosition;
    struct Vec3_short_ vVelocity;
    uint16_t uYawAngle;
    uint16_t uPitchAngle;
    int16_t uSectorID;
    uint16_t uCurrentActionLength;
    struct Vec3_short_ vInitialPosition;
    struct Vec3_short_ vGuardingPosition;
    uint16_t uTetherDistance;
    int16_t uAIState;
    uint16_t uCurrentActionAnimation;
    uint16_t uCarriedItemID;
    char field_B6;
    char field_B7;
    unsigned int uCurrentActionTime;
    uint16_t pSpriteIDs[8];
    uint16_t pSoundSampleIDs[4];  // 1 die     3 bored
    struct SpellBuff pActorBuffs[22];
    struct ItemGen ActorHasItems[4];
    unsigned int uGroup;
    unsigned int uAlly;
    struct ActorJob pScheduledJobs[8];
    unsigned int uSummonerID;
    unsigned int uLastCharacterIDToHit;
    int dword_000334_unique_name;
    char field_338[12];
};
#pragma pack(pop)

/*   95 */
#pragma pack(push, 1)
struct BLVSector_MM7 {  // 0x74
    BLVSector_MM7();

    void Serialize(class BLVSector *);
    void Deserialize(class BLVSector *);

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
    BBox_short_ pBounding;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct FontData_MM7 {
    FontData_MM7();

    void Serialize(class FontData *);
    void Deserialize(class FontData *, size_t size);

    uint8_t cFirstChar;  // 0
    uint8_t cLastChar;   // 1
    uint8_t field_2;
    uint8_t field_3;
    uint8_t field_4;
    uint16_t uFontHeight;  // 5-6
    uint8_t field_7;
    uint32_t palletes_count;
    uint32_t pFontPalettes[5];
    GUICharMetric pMetrics[256];
    uint32_t font_pixels_offset[256];
    uint8_t pFontData[0];  // array of font pixels
};
#pragma pack(pop)
