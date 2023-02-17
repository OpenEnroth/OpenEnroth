#pragma once
#include <array>

#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/PlayerEnums.h"
#include "Engine/Objects/SpriteObjectType.h"
#include "Engine/Time.h"

#include "Utility/IndexedArray.h"
#include "Utility/Geometry/Vec.h"

/*  360 */
enum SPELL_TYPE {
    SPELL_0 = 0,

    SPELL_FIRE_TORCH_LIGHT = 1,
    SPELL_FIRE_FIRE_BOLT = 2,
    SPELL_FIRE_PROTECTION_FROM_FIRE = 3,
    SPELL_FIRE_FIRE_AURA = 4,
    SPELL_FIRE_HASTE = 5,
    SPELL_FIRE_FIREBALL = 6,
    SPELL_FIRE_FIRE_SPIKE = 7,
    SPELL_FIRE_IMMOLATION = 8,
    SPELL_FIRE_METEOR_SHOWER = 9,
    SPELL_FIRE_INFERNO = 10,
    SPELL_FIRE_INCINERATE = 11,

    SPELL_AIR_WIZARD_EYE = 12,
    SPELL_AIR_FEATHER_FALL = 13,
    SPELL_AIR_PROTECTION_FROM_AIR = 14,
    SPELL_AIR_SPARKS = 15,
    SPELL_AIR_JUMP = 16,
    SPELL_AIR_SHIELD = 17,
    SPELL_AIR_LIGHNING_BOLT = 18,
    SPELL_AIR_INVISIBILITY = 19,
    SPELL_AIR_IMPLOSION = 20,
    SPELL_AIR_FLY = 21,
    SPELL_AIR_STARBURST = 22,

    SPELL_WATER_AWAKEN = 23,
    SPELL_WATER_POISON_SPRAY = 24,
    SPELL_WATER_PROTECTION_FROM_WATER = 25,
    SPELL_WATER_ICE_BOLT = 26,
    SPELL_WATER_WATER_WALK = 27,
    SPELL_WATER_RECHARGE_ITEM = 28,
    SPELL_WATER_ACID_BURST = 29,
    SPELL_WATER_ENCHANT_ITEM = 30,
    SPELL_WATER_TOWN_PORTAL = 31,
    SPELL_WATER_ICE_BLAST = 32,
    SPELL_WATER_LLOYDS_BEACON = 33,

    SPELL_EARTH_STUN = 34,
    SPELL_EARTH_SLOW = 35,
    SPELL_EARTH_PROTECTION_FROM_EARTH = 36,
    SPELL_EARTH_DEADLY_SWARM = 37,
    SPELL_EARTH_STONESKIN = 38,
    SPELL_EARTH_BLADES = 39,
    SPELL_EARTH_STONE_TO_FLESH = 40,
    SPELL_EARTH_ROCK_BLAST = 41,
    SPELL_EARTH_TELEKINESIS = 42,
    SPELL_EARTH_DEATH_BLOSSOM = 43,
    SPELL_EARTH_MASS_DISTORTION = 44,

    SPELL_SPIRIT_DETECT_LIFE = 45,
    SPELL_SPIRIT_BLESS = 46,
    SPELL_SPIRIT_FATE = 47,
    SPELL_SPIRIT_TURN_UNDEAD = 48,
    SPELL_SPIRIT_REMOVE_CURSE = 49,
    SPELL_SPIRIT_PRESERVATION = 50,
    SPELL_SPIRIT_HEROISM = 51,
    SPELL_SPIRIT_SPIRIT_LASH = 52,
    SPELL_SPIRIT_RAISE_DEAD = 53,
    SPELL_SPIRIT_SHARED_LIFE = 54,
    SPELL_SPIRIT_RESSURECTION = 55,

    SPELL_MIND_REMOVE_FEAR = 56,
    SPELL_MIND_MIND_BLAST = 57,
    SPELL_MIND_PROTECTION_FROM_MIND = 58,
    SPELL_MIND_TELEPATHY = 59,
    SPELL_MIND_CHARM = 60,
    SPELL_MIND_CURE_PARALYSIS = 61,
    SPELL_MIND_BERSERK = 62,
    SPELL_MIND_MASS_FEAR = 63,
    SPELL_MIND_CURE_INSANITY = 64,
    SPELL_MIND_PSYCHIC_SHOCK = 65,
    SPELL_MIND_ENSLAVE = 66,

    SPELL_BODY_CURE_WEAKNESS = 67,
    SPELL_BODY_FIRST_AID = 68,
    SPELL_BODY_PROTECTION_FROM_BODY = 69,
    SPELL_BODY_HARM = 70,
    SPELL_BODY_REGENERATION = 71,
    SPELL_BODY_CURE_POISON = 72,
    SPELL_BODY_HAMMERHANDS = 73,
    SPELL_BODY_CURE_DISEASE = 74,
    SPELL_BODY_PROTECTION_FROM_MAGIC = 75,
    SPELL_BODY_FLYING_FIST = 76,
    SPELL_BODY_POWER_CURE = 77,

    SPELL_LIGHT_LIGHT_BOLT = 78,
    SPELL_LIGHT_DESTROY_UNDEAD = 79,
    SPELL_LIGHT_DISPEL_MAGIC = 80,
    SPELL_LIGHT_PARALYZE = 81,
    SPELL_LIGHT_SUMMON_ELEMENTAL = 82,
    SPELL_LIGHT_DAY_OF_THE_GODS = 83,
    SPELL_LIGHT_PRISMATIC_LIGHT = 84,
    SPELL_LIGHT_DAY_OF_PROTECTION = 85,
    SPELL_LIGHT_HOUR_OF_POWER = 86,
    SPELL_LIGHT_SUNRAY = 87,
    SPELL_LIGHT_DIVINE_INTERVENTION = 88,

    SPELL_DARK_REANIMATE = 89,
    SPELL_DARK_TOXIC_CLOUD = 90,
    SPELL_DARK_VAMPIRIC_WEAPON = 91,
    SPELL_DARK_SHRINKING_RAY = 92,
    SPELL_DARK_SHARPMETAL = 93,
    SPELL_DARK_CONTROL_UNDEAD = 94,
    SPELL_DARK_PAIN_REFLECTION = 95,
    SPELL_DARK_SACRIFICE = 96,
    SPELL_DARK_DRAGON_BREATH = 97,
    SPELL_DARK_ARMAGEDDON = 98,
    SPELL_DARK_SOULDRINKER = 99,

    SPELL_BOW_ARROW = 100,
    SPELL_101 = 101,
    SPELL_LASER_PROJECTILE = 102,

    BECOME_MAGIC_GUILD_MEMBER = 150,
    SPELL_QUEST_COMPLETED = 151,
    SPELL_152 = 152,
    SPELL_DISEASE = 153
};

enum SPELL_SCHOOL : int {
    SPELL_SCHOOL_FIRE = 0,
    SPELL_SCHOOL_AIR = 1,
    SPELL_SCHOOL_WATER = 2,
    SPELL_SCHOOL_EARTH = 3,
    SPELL_SCHOOL_NONE = 4,
    SPELL_SCHOOL_MAGIC = 5,
    SPELL_SCHOOL_SPIRIT = 6,
    SPELL_SCHOOL_MIND = 7,
    SPELL_SCHOOL_BODY = 8,
    SPELL_SCHOOL_LIGHT = 9,
    SPELL_SCHOOL_DARK = 10
};

/*   68 */
#pragma pack(push, 1)
struct SpellBuff {
    bool Apply(GameTime time, PLAYER_SKILL_MASTERY uSkillMastery,
                      PLAYER_SKILL_LEVEL uPower, int uOverlayID, uint8_t caster);
    void Reset();
    bool IsBuffExpiredToTime(GameTime time);

    //----- (0042EB31) --------------------------------------------------------
    bool Active() const { return this->expire_time.value > 0; }
    bool Expired() const { return this->expire_time.value < 0; }

    GameTime expire_time;
    uint16_t uPower = 0; // Spell power, semantics are spell-specific.
    PLAYER_SKILL_MASTERY uSkillMastery = PLAYER_SKILL_MASTERY_NONE; // 1-4, normal to grandmaster.
    uint16_t uOverlayID = 0;
    uint8_t uCaster = 0;
    uint8_t uFlags = 0; // 0x1 => cast at grandmaster.
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SpellInfo {
    char *pName;
    char *pShortName;
    char *pDescription;
    char *pBasicSkillDesc;
    char *pExpertSkillDesc;
    char *pMasterSkillDesc;
    char *pGrandmasterSkillDesc;
    SPELL_SCHOOL uSchool;
    int field_20;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SpellStats {
    void Initialize();

    SpellInfo pInfos[100];
};
#pragma pack(pop)

/*  364 */
#pragma pack(push, 1)
struct stru324_spell_id_to_sprite_mapping {  // stru324_spell
    SPRITE_OBJECT_TYPE uSpriteType;
    int16_t field_2;
};
#pragma pack(pop)

/*  151 */
#pragma pack(push, 1)
class SpellData {
 public:
    SpellData(int16_t innormalMana, int16_t inExpertLevelMana,
              int16_t inMasterLevelMana, int16_t inMagisterLevelMana,
              int16_t inNormalLevelRecovery, int16_t inExpertLevelRecovery,
              int16_t inMasterLevelRecovery, int16_t inMagisterLevelRecovery,
              int8_t inbaseDamage, int8_t inbonusSkillDamage, int16_t instats);
    union {
        uint16_t mana_per_skill[4];
        struct {
            uint16_t uNormalLevelMana;
            uint16_t uExpertLevelMana;
            uint16_t uMasterLevelMana;
            uint16_t uMagisterLevelMana;
        };
    };
    union {
        uint16_t recovery_per_skill[4];
        struct {
            uint16_t uNormalLevelRecovery;
            uint16_t uExpertLevelRecovery;
            uint16_t uMasterLevelRecovery;
            uint16_t uMagisterLevelRecovery;
        };
    };
    int8_t baseDamage;
    int8_t bonusSkillDamage;
    int16_t stats;
    // char field_12;
    // char field_13;
    // int16_t field_14;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SpellBookIconPos {
    int32_t Xpos;
    int32_t Ypos;
};

#pragma pack(pop)

/*  154 */
#pragma pack(push, 1)
struct TownPortalData {
    Vec3i pos;
    int16_t rot_y;
    int16_t rot_x;
    uint16_t uMapInfoID;
    int16_t field_12;
};
#pragma pack(pop)
extern std::array<TownPortalData, 6> TownPortalList;  // 4ECBB8

extern struct SpellStats *pSpellStats;

extern std::array<std::array<struct SpellBookIconPos, 12>, 9> pIconPos;

extern std::array<stru324_spell_id_to_sprite_mapping, 103>
    spell_sprite_mapping;  // 4E3ACC
extern std::array<SpellData, 100> pSpellDatas;
extern IndexedArray<SPELL_TYPE, ITEM_FIRST_WAND, ITEM_LAST_WAND> wand_spell_ids;
extern std::array<uint16_t, 101> SpellSoundIds;

int _43AFE3_calc_spell_damage(int spellId, PLAYER_SKILL_LEVEL spellLevel, PLAYER_SKILL_MASTERY skillMastery, int currentHp);
bool sub_427769_isSpellQuickCastableOnShiftClick(unsigned int uSpellID);
void EventCastSpell(int uSpellID, PLAYER_SKILL_MASTERY skillMastery, PLAYER_SKILL_LEVEL skillLevel, int fromx,
                    int fromy, int fromz, int tox, int toy, int toz);  // sub_448DF8

void armageddonProgress();
