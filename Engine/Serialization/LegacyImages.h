#pragma once
/*
 *  Party, timers, NPCs and other stuff is binary dumped into resources/savegame files, which limits ability to alter
 *      these structures without breaking compatibility.
 *  This module is used to serialzie and deserialize native game structures intro their current representations without
 *      breaking compatibility with original files.
*/


#pragma pack(push, 1)
struct NPCData_Image_MM7
{
    NPCData_Image_MM7();

    void Serialize(struct NPCData *item);
    void Deserialize(struct NPCData *item);

    /* 00 */ char *pName;
    /* 04 */ unsigned int uPortraitID;
    /* 08 */ unsigned int uFlags;  // & 0x80    no greeting on dialogue start; looks like hired
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

struct ItemGen_Image_MM7
{
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
    /* 18 */ unsigned __int8 uBodyAnchor;
    /* 19 */ char uMaxCharges;
    /* 1A */ char uHolderPlayer;
    /* 1B */ char field_1B;
    /* 1C */ unsigned __int64 uExpireTime;
};

struct SpellBuff_Image_MM7
{
    SpellBuff_Image_MM7();

    void Serialize(struct SpellBuff *item);
    void Deserialize(struct SpellBuff *item);

    /* 00 */ signed __int64 uExpireTime;
    /* 08 */ unsigned __int16 uPower;
    /* 0A */ unsigned __int16 uSkill;
    /* 0C */ unsigned __int16 uOverlayID;
    /* 0E */ unsigned __int8 uCaster;
    /* 0F */ unsigned __int8 uFlags;
    /* 10 */
};

struct PlayerSpellbookChapter_Image_MM7
{
    PlayerSpellbookChapter_Image_MM7();

    /* 00 */ char bIsSpellAvailable[11];
    /* 0B */
};
struct PlayerSpells_Image_MM7
{
    PlayerSpells_Image_MM7();

    union
    {
        struct
        {
            /* 00 */ PlayerSpellbookChapter_Image_MM7 pFireSpellbook;
            /* 0B */ PlayerSpellbookChapter_Image_MM7 pAirSpellbook;
            /* 16 */ PlayerSpellbookChapter_Image_MM7 pWaterSpellbook;
            /* 21 */ PlayerSpellbookChapter_Image_MM7 pEarthSpellbook;
            /* 2C */ PlayerSpellbookChapter_Image_MM7 pSpiritSpellbook;
            /* 37 */ PlayerSpellbookChapter_Image_MM7 pMindSpellbook;
            /* 42 */ PlayerSpellbookChapter_Image_MM7 pBodySpellbook;
            /* 4D */ PlayerSpellbookChapter_Image_MM7 pLightSpellbook;
            /* 58 */ PlayerSpellbookChapter_Image_MM7 pDarkSpellbook;
            /* 63 */ char _pad;
            /* 64 */
        };
        struct
        {
            /* 00 */ PlayerSpellbookChapter_Image_MM7 pChapters[9];
            /* 63 */ char _pad;
            /* 64 */
        };
        struct
        {
            /* 00 */ char bHaveSpell[99];
            /* 63 */ char _pad;
            /* 64 */
        };
    };
};

union PlayerEquipment_Image_MM7
{
    PlayerEquipment_Image_MM7();

    union
    {
        struct
        {
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

struct LloydBeacon_Image_MM7
{
    LloydBeacon_Image_MM7();

    /* 00 */ unsigned __int64 uBeaconTime;
    /* 08 */ int PartyPos_X;
    /* 0C */ int PartyPos_Y;
    /* 10 */ int PartyPos_Z;
    /* 14 */ __int16 PartyRot_X;
    /* 16 */ __int16 PartyRot_Y;
    /* 18 */ int SaveFileID;
    /* 1C */
};

struct Player_Image_MM7
{
    Player_Image_MM7();

    void Serialize(struct Player *);
    void Deserialize(struct Player *);

    /* 0000 */ __int64 pConditions[20];
    /* 00A0 */ unsigned __int64 uExperience;
    /* 00A8 */ char pName[16];
    /* 00B8 */ unsigned char uSex;
    /* 00B9 */ unsigned char classType;
    /* 00BA */ unsigned __int8 uCurrentFace;
    /* 00BB */ char field_BB;
    /* 00BC */ unsigned __int16 uMight;
    /* 00BE */ unsigned __int16 uMightBonus;
    /* 00C0 */ unsigned __int16 uIntelligence;
    /* 00C2 */ unsigned __int16 uIntelligenceBonus;
    /* 00C4 */ unsigned __int16 uWillpower;
    /* 00C6 */ unsigned __int16 uWillpowerBonus;
    /* 00C8 */ unsigned __int16 uEndurance;
    /* 00CA */ unsigned __int16 uEnduranceBonus;
    /* 00CC */ unsigned __int16 uSpeed;
    /* 00CE */ unsigned __int16 uSpeedBonus;
    /* 00D0 */ unsigned __int16 uAccuracy;
    /* 00D2 */ unsigned __int16 uAccuracyBonus;
    /* 00D4 */ unsigned __int16 uLuck;
    /* 00D6 */ unsigned __int16 uLuckBonus;
    /* 00D8 */ __int16 sACModifier;
    /* 00DA */ unsigned __int16 uLevel;
    /* 00DC */ __int16 sLevelModifier;
    /* 00DE */ __int16 sAgeModifier;
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
    /* 0108 */ union
    {
        struct
        {
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
        unsigned __int16 pActiveSkills[37];
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
    /* 0214 */ union
    {
        struct
        {
            ItemGen_Image_MM7 pInventoryItemList[126];
            ItemGen_Image_MM7 pEquippedItems[12];
        };
        struct
        {
            ItemGen_Image_MM7 pOwnItems[138];
        };
    };
    /* 157C */ int pInventoryMatrix[126];
    /* 1774 */ __int16 sResFireBase;
    /* 1776 */ __int16 sResAirBase;
    /* 1778 */ __int16 sResWaterBase;
    /* 177A */ __int16 sResEarthBase;
    /* 177C */ __int16 field_177C;
    /* 177E */ __int16 sResMagicBase;
    /* 1780 */ __int16 sResSpiritBase;
    /* 1782 */ __int16 sResMindBase;
    /* 1784 */ __int16 sResBodyBase;
    /* 1786 */ __int16 sResLightBase;
    /* 1788 */ __int16 sResDarkBase;
    /* 178A */ __int16 sResFireBonus;
    /* 178C */ __int16 sResAirBonus;
    /* 178E */ __int16 sResWaterBonus;
    /* 1790 */ __int16 sResEarthBonus;
    /* 1792 */ __int16 field_1792;
    /* 1794 */ __int16 sResMagicBonus;
    /* 1796 */ __int16 sResSpiritBonus;
    /* 1798 */ __int16 sResMindBonus;
    /* 179A */ __int16 sResBodyBonus;
    /* 179C */ __int16 sResLightBonus;
    /* 179E */ __int16 sResDarkBonus;
    /* 17A0 */ SpellBuff_Image_MM7 pPlayerBuffs[24];
    /* 1920 */ unsigned int uVoiceID;
    /* 1924 */ int uPrevVoiceID;
    /* 1928 */ int uPrevFace;
    /* 192C */ int field_192C;
    /* 1930 */ int field_1930;
    /* 1934 */ unsigned __int16 uTimeToRecovery;
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
    /* 1A4F */ unsigned __int8 uQuickSpell;
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
    /* 1A9C */ unsigned __int16 expression;
    /* 1A9E */ unsigned __int16 uExpressionTimePassed;
    /* 1AA0 */ unsigned __int16 uExpressionTimeLength;
    /* 1AA2 */ __int16 field_1AA2;
    /* 1AA4 */ int _expression21_animtime;
    /* 1AA8 */ int _expression21_frameset;
    /* 1AAC */ LloydBeacon_Image_MM7 pInstalledBeacons[5];
    /* 1B38 */ char uNumDivineInterventionCastsThisDay;
    /* 1B39 */ char uNumArmageddonCasts;
    /* 1B3A */ char uNumFireSpikeCasts;
    /* 1B3B */ char field_1B3B;
    /* 1B3C */
};


struct PartyTimeStruct_Image_MM7
{
    PartyTimeStruct_Image_MM7();

    /* 000 */ __int64 bountyHunting_next_generation_time[10];
    /* 050 */ __int64 Shops_next_generation_time[85];//field_50
    /* 2F8 */ __int64 _shop_ban_times[53];
    /* 4A0 */ unsigned __int64 CounterEventValues[10];  // (0xACD314h in Silvo's binary)
    /* 4F0 */ __int64 HistoryEventTimes[29];   // (0xACD364h in Silvo's binary)
    /* 5D8 */ unsigned __int64 _s_times[20]; //5d8 440h+8*51     //(0xACD44Ch in Silvo's binary)
    /* 678 */
};

struct Party_Image_MM7
{
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
    /* 0002C */ unsigned __int64 uTimePlayed;
    /* 00034 */ __int64 uLastRegenerationTime;
    /* 0003C */ PartyTimeStruct_Image_MM7 PartyTimes;
    /* 006B4 */ Vec3_int_ vPosition;
    /* 006C0 */ int sRotationY;
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
    /* 006F4 */ int floor_face_pid; // face we are standing at
    /* 006F8 */ int walk_sound_timer;
    /* 006FC */ int _6FC_water_lava_timer;
    /* 00700 */ int uFallStartY;
    /* 00704 */ unsigned int bFlying;
    /* 00708 */ char field_708;
    /* 00709 */ unsigned __int8 hirelingScrollPosition;
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
    /* 00750 */ __int16 monster_id_for_hunting[5];
    /* 0075A */ __int16 monster_for_hunting_killed[5];
    /* 00764 */ unsigned char days_played_without_rest;
    /* 00765 */ unsigned __int8 _quest_bits[64];
    /* 007A5 */ unsigned __int8 pArcomageWins[16];
    /* 007B5 */ char field_7B5_in_arena_quest;
    /* 007B6 */ char uNumArenaPageWins;
    /* 007B7 */ char uNumArenaSquireWins;
    /* 007B8 */ char uNumArenaKnightWins;
    /* 007B9 */ char uNumArenaLordWins;
    /* 007BA */ char pIsArtifactFound[29];  //7ba
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



struct Timer_Image_MM7
{
    Timer_Image_MM7();

    void Serialize(struct Timer *);
    void Deserialize(struct Timer *);

    /* 00 */ unsigned int bReady;
    /* 04 */ unsigned int bPaused;
    /* 08 */ int bTackGameTime;
    /* 0C */ unsigned int uStartTime;
    /* 10 */ unsigned int uStopTime;
    /* 14 */ int uGameTimeStart;
    /* 18 */ int field_18;
    /* 1C */ unsigned int uTimeElapsed;
    /* 20 */ int dt_in_some_format;
    /* 24 */ unsigned int uTotalGameTimeElapsed;
    /* 28 */
};




/*  282 */
struct OtherOverlay_Image_MM7
{
    OtherOverlay_Image_MM7();

    /* 00 */ __int16 field_0;
    /* 02 */ __int16 field_2;
    /* 04 */ __int16 field_4;
    /* 06 */ __int16 field_6;
    /* 08 */ __int16 field_8;
    /* 0A */ __int16 field_A;
    /* 0C */ __int16 field_C;
    /* 0E */ __int16 field_E;
    /* 10 */ int field_10;
    /* 14 */
};

/*   63 */
struct OtherOverlayList_Image_MM7
{
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
struct IconFrame_MM7
{
    IconFrame_MM7();

    void Serialize(class Icon *);
    void Deserialize(class Icon *);

    /* 000 */ char pAnimationName[12];
    /* 00C */ char pTextureName[12];
    /* 018 */ __int16 uAnimTime;
    /* 01A */ __int16 uAnimLength;
    /* 01C */ __int16 uFlags; // 0x01 - more icons in this animation
    /* 01E */ unsigned __int16 uTextureID;
};
#pragma pack(pop)


/*   76 */
#pragma pack(push, 1)
struct UIAnimation_MM7
{
    UIAnimation_MM7();

    void Serialize(class UIAnimation *);
    void Deserialize(class UIAnimation *);

    /* 000 */ unsigned __int16 uIconID;
    /* 002 */ __int16 field_2;
    /* 004 */ __int16 uAnimTime;
    /* 006 */ __int16 uAnimLength;
    /* 008 */ __int16 x;
    /* 00A */ __int16 y;
    /* 00C */ char field_C;
};
#pragma pack(pop)
