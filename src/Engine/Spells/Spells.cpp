#include "Engine/Spells/Spells.h"

#include <cstring>
#include <algorithm>
#include <map>
#include <string>

#include "Engine/Party.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Random/Random.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/SpellFxRenderer.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/Spells/SpellEnumFunctions.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/Math/TrigLut.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Split.h"
#include "Utility/MapAccess.h"

SpellFxRenderer *spell_fx_renderer = EngineIocContainer::ResolveSpellFxRenderer();

SpellStats *pSpellStats = nullptr;

/**
 * @offset 0x4E3ACC
 */
const IndexedArray<SpriteId, SPELL_FIRST_WITH_SPRITE, SPELL_LAST_WITH_SPRITE> SpellSpriteMapping = {
    {SPELL_FIRE_TORCH_LIGHT, SPRITE_SPELL_FIRE_TORCH_LIGHT},
    {SPELL_FIRE_FIRE_BOLT, SPRITE_SPELL_FIRE_FIRE_BOLT},
    {SPELL_FIRE_PROTECTION_FROM_FIRE, SPRITE_SPELL_FIRE_PROTECTION_FROM_FIRE},
    {SPELL_FIRE_FIRE_AURA, SPRITE_SPELL_FIRE_FIRE_AURA},
    {SPELL_FIRE_HASTE, SPRITE_SPELL_FIRE_HASTE},
    {SPELL_FIRE_FIREBALL, SPRITE_SPELL_FIRE_FIREBALL},
    {SPELL_FIRE_FIRE_SPIKE, SPRITE_SPELL_FIRE_FIRE_SPIKE},
    {SPELL_FIRE_IMMOLATION, SPRITE_SPELL_FIRE_IMMOLATION},
    {SPELL_FIRE_METEOR_SHOWER, SPRITE_SPELL_FIRE_METEOR_SHOWER},
    {SPELL_FIRE_INFERNO, SPRITE_SPELL_FIRE_INFERNO},
    {SPELL_FIRE_INCINERATE, SPRITE_SPELL_FIRE_INCINERATE},

    {SPELL_AIR_WIZARD_EYE, SPRITE_SPELL_AIR_WIZARD_EYE},
    {SPELL_AIR_FEATHER_FALL, SPRITE_SPELL_AIR_FEATHER_FALL},
    {SPELL_AIR_PROTECTION_FROM_AIR, SPRITE_SPELL_AIR_PROTECTION_FROM_AIR},
    {SPELL_AIR_SPARKS, SPRITE_SPELL_AIR_SPARKS},
    {SPELL_AIR_JUMP, SPRITE_SPELL_AIR_JUMP},
    {SPELL_AIR_SHIELD, SPRITE_SPELL_AIR_SHIELD},
    {SPELL_AIR_LIGHTNING_BOLT, SPRITE_SPELL_AIR_LIGHTNING_BOLT},
    {SPELL_AIR_INVISIBILITY, SPRITE_SPELL_AIR_INVISIBILITY},
    {SPELL_AIR_IMPLOSION, SPRITE_SPELL_AIR_IMPLOSION},
    {SPELL_AIR_FLY, SPRITE_SPELL_AIR_FLY},
    {SPELL_AIR_STARBURST, SPRITE_SPELL_AIR_STARBURST},

    {SPELL_WATER_AWAKEN, SPRITE_SPELL_WATER_AWAKEN},
    {SPELL_WATER_POISON_SPRAY, SPRITE_SPELL_WATER_POISON_SPRAY},
    {SPELL_WATER_PROTECTION_FROM_WATER, SPRITE_SPELL_WATER_PROTECTION_FROM_WATER},
    {SPELL_WATER_ICE_BOLT, SPRITE_SPELL_WATER_ICE_BOLT},
    {SPELL_WATER_WATER_WALK, SPRITE_SPELL_WATER_WATER_WALK},
    {SPELL_WATER_RECHARGE_ITEM, SPRITE_SPELL_WATER_RECHARGE_ITEM},
    {SPELL_WATER_ACID_BURST, SPRITE_SPELL_WATER_ACID_BURST},
    {SPELL_WATER_ENCHANT_ITEM, SPRITE_SPELL_WATER_ENCHANT_ITEM},
    {SPELL_WATER_TOWN_PORTAL, SPRITE_SPELL_WATER_TOWN_PORTAL},
    {SPELL_WATER_ICE_BLAST, SPRITE_SPELL_WATER_ICE_BLAST},
    {SPELL_WATER_LLOYDS_BEACON, SPRITE_SPELL_WATER_LLOYDS_BEACON},

    {SPELL_EARTH_STUN, SPRITE_SPELL_EARTH_STUN},
    {SPELL_EARTH_SLOW, SPRITE_SPELL_EARTH_SLOW},
    {SPELL_EARTH_PROTECTION_FROM_EARTH, SPRITE_SPELL_EARTH_PROTECTION_FROM_EARTH},
    {SPELL_EARTH_DEADLY_SWARM, SPRITE_SPELL_EARTH_DEADLY_SWARM},
    {SPELL_EARTH_STONESKIN, SPRITE_SPELL_EARTH_STONESKIN},
    {SPELL_EARTH_BLADES, SPRITE_SPELL_EARTH_BLADES},
    {SPELL_EARTH_STONE_TO_FLESH, SPRITE_SPELL_EARTH_STONE_TO_FLESH},
    {SPELL_EARTH_ROCK_BLAST, SPRITE_SPELL_EARTH_ROCK_BLAST},
    {SPELL_EARTH_TELEKINESIS, SPRITE_SPELL_EARTH_TELEKINESIS},
    {SPELL_EARTH_DEATH_BLOSSOM, SPRITE_SPELL_EARTH_DEATH_BLOSSOM},
    {SPELL_EARTH_MASS_DISTORTION, SPRITE_SPELL_EARTH_MASS_DISTORTION},

    {SPELL_SPIRIT_DETECT_LIFE, SPRITE_SPELL_SPIRIT_DETECT_LIFE},
    {SPELL_SPIRIT_BLESS, SPRITE_SPELL_SPIRIT_BLESS},
    {SPELL_SPIRIT_FATE, SPRITE_SPELL_SPIRIT_FATE},
    {SPELL_SPIRIT_TURN_UNDEAD, SPRITE_SPELL_SPIRIT_TURN_UNDEAD},
    {SPELL_SPIRIT_REMOVE_CURSE, SPRITE_SPELL_SPIRIT_REMOVE_CURSE},
    {SPELL_SPIRIT_PRESERVATION, SPRITE_SPELL_SPIRIT_PRESERVATION},
    {SPELL_SPIRIT_HEROISM, SPRITE_SPELL_SPIRIT_HEROISM},
    {SPELL_SPIRIT_SPIRIT_LASH, SPRITE_SPELL_SPIRIT_SPIRIT_LASH},
    {SPELL_SPIRIT_RAISE_DEAD, SPRITE_SPELL_SPIRIT_RAISE_DEAD},
    {SPELL_SPIRIT_SHARED_LIFE, SPRITE_SPELL_SPIRIT_SHARED_LIFE},
    {SPELL_SPIRIT_RESSURECTION, SPRITE_SPELL_SPIRIT_RESSURECTION},

    {SPELL_MIND_REMOVE_FEAR, SPRITE_SPELL_MIND_REMOVE_FEAR},
    {SPELL_MIND_MIND_BLAST, SPRITE_SPELL_MIND_MIND_BLAST},
    {SPELL_MIND_PROTECTION_FROM_MIND, SPRITE_SPELL_MIND_PROTECTION_FROM_MIND},
    {SPELL_MIND_TELEPATHY, SPRITE_SPELL_MIND_TELEPATHY},
    {SPELL_MIND_CHARM, SPRITE_SPELL_MIND_CHARM},
    {SPELL_MIND_CURE_PARALYSIS, SPRITE_SPELL_MIND_CURE_PARALYSIS},
    {SPELL_MIND_BERSERK, SPRITE_SPELL_MIND_BERSERK},
    {SPELL_MIND_MASS_FEAR, SPRITE_SPELL_MIND_MASS_FEAR},
    {SPELL_MIND_CURE_INSANITY, SPRITE_SPELL_MIND_CURE_INSANITY},
    {SPELL_MIND_PSYCHIC_SHOCK, SPRITE_SPELL_MIND_PSYCHIC_SHOCK},
    {SPELL_MIND_ENSLAVE, SPRITE_SPELL_MIND_ENSLAVE},

    {SPELL_BODY_CURE_WEAKNESS, SPRITE_SPELL_BODY_CURE_WEAKNESS},
    {SPELL_BODY_FIRST_AID, SPRITE_SPELL_BODY_FIRST_AID},
    {SPELL_BODY_PROTECTION_FROM_BODY, SPRITE_SPELL_BODY_PROTECTION_FROM_BODY},
    {SPELL_BODY_HARM, SPRITE_SPELL_BODY_HARM},
    {SPELL_BODY_REGENERATION, SPRITE_SPELL_BODY_REGENERATION},
    {SPELL_BODY_CURE_POISON, SPRITE_SPELL_BODY_CURE_POISON},
    {SPELL_BODY_HAMMERHANDS, SPRITE_SPELL_BODY_HAMMERHANDS},
    {SPELL_BODY_CURE_DISEASE, SPRITE_SPELL_BODY_CURE_DISEASE},
    {SPELL_BODY_PROTECTION_FROM_MAGIC, SPRITE_SPELL_BODY_PROTECTION_FROM_MAGIC},
    {SPELL_BODY_FLYING_FIST, SPRITE_SPELL_BODY_FLYING_FIST},
    {SPELL_BODY_POWER_CURE, SPRITE_SPELL_BODY_POWER_CURE},

    {SPELL_LIGHT_LIGHT_BOLT, SPRITE_SPELL_LIGHT_LIGHT_BOLT},
    {SPELL_LIGHT_DESTROY_UNDEAD, SPRITE_SPELL_LIGHT_DESTROY_UNDEAD},
    {SPELL_LIGHT_DISPEL_MAGIC, SPRITE_SPELL_LIGHT_DISPEL_MAGIC},
    {SPELL_LIGHT_PARALYZE, SPRITE_SPELL_LIGHT_PARALYZE},
    {SPELL_LIGHT_SUMMON_ELEMENTAL, SPRITE_SPELL_LIGHT_SUMMON_ELEMENTAL},
    {SPELL_LIGHT_DAY_OF_THE_GODS, SPRITE_SPELL_LIGHT_DAY_OF_THE_GODS},
    {SPELL_LIGHT_PRISMATIC_LIGHT, SPRITE_SPELL_LIGHT_PRISMATIC_LIGHT},
    {SPELL_LIGHT_DAY_OF_PROTECTION, SPRITE_SPELL_LIGHT_DAY_OF_PROTECTION},
    {SPELL_LIGHT_HOUR_OF_POWER, SPRITE_SPELL_LIGHT_HOUR_OF_POWER},
    {SPELL_LIGHT_SUNRAY, SPRITE_SPELL_LIGHT_SUNRAY},
    {SPELL_LIGHT_DIVINE_INTERVENTION, SPRITE_SPELL_LIGHT_DIVINE_INTERVENTION},

    {SPELL_DARK_REANIMATE, SPRITE_SPELL_DARK_REANIMATE},
    {SPELL_DARK_TOXIC_CLOUD, SPRITE_SPELL_DARK_TOXIC_CLOUD},
    {SPELL_DARK_VAMPIRIC_WEAPON, SPRITE_SPELL_DARK_VAMPIRIC_WEAPON},
    {SPELL_DARK_SHRINKING_RAY, SPRITE_SPELL_DARK_SHRINKING_RAY},
    {SPELL_DARK_SHARPMETAL, SPRITE_SPELL_DARK_SHARPMETAL},
    {SPELL_DARK_CONTROL_UNDEAD, SPRITE_SPELL_DARK_CONTROL_UNDEAD},
    {SPELL_DARK_PAIN_REFLECTION, SPRITE_SPELL_DARK_PAIN_REFLECTION},
    {SPELL_DARK_SACRIFICE, SPRITE_SPELL_DARK_SACRIFICE},
    {SPELL_DARK_DRAGON_BREATH, SPRITE_SPELL_DARK_DRAGON_BREATH},
    {SPELL_DARK_ARMAGEDDON, SPRITE_SPELL_DARK_ARMAGEDDON},
    {SPELL_DARK_SOULDRINKER, SPRITE_SPELL_DARK_SOULDRINKER},

    {SPELL_BOW_ARROW, SPRITE_ARROW_PROJECTILE},
    {SPELL_101, SPRITE_ARROW_PROJECTILE},
    {SPELL_LASER_PROJECTILE, SPRITE_BLASTER_PROJECTILE}};

SpellData::SpellData(int16_t inNormalMana,
                     int16_t inExpertLevelMana,
                     int16_t inMasterLevelMana,
                     int16_t inMagisterLevelMana,
                     int16_t inNormalLevelRecovery,
                     int16_t inExpertLevelRecovery,
                     int16_t inMasterLevelRecovery,
                     int16_t inMagisterLevelRecovery,
                     int8_t inBaseDamage,
                     int8_t inBonusSkillDamage,
                     SpellFlags inFlags,
                     CharacterSkillMastery inSkillMastery) {
    mana_per_skill[CHARACTER_SKILL_MASTERY_NOVICE] = inNormalMana;
    mana_per_skill[CHARACTER_SKILL_MASTERY_EXPERT] = inExpertLevelMana;
    mana_per_skill[CHARACTER_SKILL_MASTERY_MASTER] = inMasterLevelMana;
    mana_per_skill[CHARACTER_SKILL_MASTERY_GRANDMASTER] = inMagisterLevelMana;
    recovery_per_skill[CHARACTER_SKILL_MASTERY_NOVICE] = Duration::fromTicks(inNormalLevelRecovery);
    recovery_per_skill[CHARACTER_SKILL_MASTERY_EXPERT] = Duration::fromTicks(inExpertLevelRecovery);
    recovery_per_skill[CHARACTER_SKILL_MASTERY_MASTER] = Duration::fromTicks(inMasterLevelRecovery);
    recovery_per_skill[CHARACTER_SKILL_MASTERY_GRANDMASTER] = Duration::fromTicks(inMagisterLevelRecovery);
    baseDamage = inBaseDamage;
    bonusSkillDamage = inBonusSkillDamage;
    flags = inFlags;
    skillMastery = inSkillMastery;
}

/**
 * Description of spells.
 *
 *                                                 Mana Novice
 *                                                 |   Mana Expert
 *                                                 |   |   Mana Master
 *                                                 |   |   |   Mana Grandmaster
 *                                                 |   |   |   |     Recovery Novice
 *                                                 |   |   |   |     |     Recovery Expert
 *                                                 |   |   |   |     |     |     Recovery Master
 *                                                 |   |   |   |     |     |     |     Recovery Grandmaster
 *                                                 |   |   |   |     |     |     |     |   Base Damage
 *                                                 |   |   |   |     |     |     |     |   |   Bonus Skill Damage
 *                                                 |   |   |   |     |     |     |     |   |   |  Flags
 *                                                 |   |   |   |     |     |     |     |   |   |  |  Required skill mastery
 *                                                 |   |   |   |     |     |     |     |   |   |  |  |
 */
IndexedArray<SpellData, SPELL_FIRST_REGULAR, SPELL_LAST_REGULAR> pSpellDatas = {
    {SPELL_FIRE_TORCH_LIGHT,            SpellData( 1,  1,  1,  1,   60,   60,   60,   40,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_FIRE_FIRE_BOLT,              SpellData( 2,  2,  2,  2,  110,  110,  100,   90,  0,  3, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_FIRE_PROTECTION_FROM_FIRE,   SpellData( 3,  3,  3,  3,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_FIRE_FIRE_AURA,              SpellData( 4,  4,  4,  4,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_FIRE_HASTE,                  SpellData( 5,  5,  5,  5,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_FIRE_FIREBALL,               SpellData( 8 , 8,  8,  8,  100,  100,   90,   80,  0,  6, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_FIRE_FIRE_SPIKE,             SpellData(10, 10, 10, 10,  150,  150,  150,  150,  0,  6, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_FIRE_IMMOLATION,             SpellData(15, 15, 15, 15,  120,  120,  120,  120,  0,  6, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_FIRE_METEOR_SHOWER,          SpellData(20, 20, 20, 20,  100,  100,  100,   90,  8,  1, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_FIRE_INFERNO,                SpellData(25, 25, 25, 25,  100,  100,  100,   90, 12,  1, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_FIRE_INCINERATE,             SpellData(30, 30, 30, 30,   90,   90,   90,   90, 15, 15, 0, CHARACTER_SKILL_MASTERY_GRANDMASTER)},

    {SPELL_AIR_WIZARD_EYE,              SpellData( 1,  1,  1,  0,   60,   60,   60,   60,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_AIR_FEATHER_FALL,            SpellData( 2,  2,  2,  2,  120,  120,  120,  100,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_AIR_PROTECTION_FROM_AIR,     SpellData( 3,  3,  3,  3,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_AIR_SPARKS,                  SpellData( 4,  4,  4,  4,  110,  100,   90,   80,  2,  1, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_AIR_JUMP,                    SpellData( 5,  5,  5,  5,   90,   90,   70,   50,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_AIR_SHIELD,                  SpellData( 8,  8,  8,  8,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_AIR_LIGHTNING_BOLT,          SpellData(10, 10, 10, 10,  100,  100,   90,   70,  0,  8, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_AIR_INVISIBILITY,            SpellData(15, 15, 15, 15,  200,  200,  200,  200,  0,  0, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_AIR_IMPLOSION,               SpellData(20, 20, 20, 20,  100,  100,  100,   90, 10, 10, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_AIR_FLY,                     SpellData(25, 25, 25, 25,  250,  250,  250,  250,  0,  0, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_AIR_STARBURST,               SpellData(30, 30, 30, 30,   90,   90,   90,   90, 20,  1, 0, CHARACTER_SKILL_MASTERY_GRANDMASTER)},

    {SPELL_WATER_AWAKEN,                SpellData( 1,  1,  1,  1,   60,   60,   60,   20,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_WATER_POISON_SPRAY,          SpellData( 2,  2,  2,  2,  110,  100,   90,   70,  2,  2, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_WATER_PROTECTION_FROM_WATER, SpellData( 3,  3,  3,  3,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_WATER_ICE_BOLT,              SpellData( 4,  4,  4,  4,  110,  100,   90,   80,  0,  4, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_WATER_WATER_WALK,            SpellData( 5,  5,  5,  5,  150,  150,  150,  150,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_WATER_RECHARGE_ITEM,         SpellData( 8,  8,  8,  8,  200,  200,  200,  200,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_WATER_ACID_BURST,            SpellData(10, 10, 10, 10,  100,  100,   90,   80,  9,  9, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_WATER_ENCHANT_ITEM,          SpellData(15, 15, 15, 15,  140,  140,  140,  140,  0,  0, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_WATER_TOWN_PORTAL,           SpellData(20, 20, 20, 20,  200,  200,  200,  200,  0,  0, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_WATER_ICE_BLAST,             SpellData(25, 25, 25, 25,   80,   80,   80,   80, 12,  3, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_WATER_LLOYDS_BEACON,         SpellData(30, 30, 30, 30,  250,  250,  250,  250,  0,  0, 0, CHARACTER_SKILL_MASTERY_GRANDMASTER)},

    {SPELL_EARTH_STUN,                  SpellData( 1,  1,  1,  1,   80,   80,   80,   80,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_EARTH_SLOW,                  SpellData( 2,  2,  2,  2,  100,  100,  100,  100,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_EARTH_PROTECTION_FROM_EARTH, SpellData( 3,  3,  3,  3,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_EARTH_DEADLY_SWARM,          SpellData( 4,  4,  4,  4,  110,  100,   90,   80,  5,  3, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_EARTH_STONESKIN,             SpellData( 5,  5,  5,  5,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_EARTH_BLADES,                SpellData( 8,  8,  8,  8,  100,  100,   90,   80,  0,  9, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_EARTH_STONE_TO_FLESH,        SpellData(10, 10, 10, 10,  140,  140,  140,  140,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_EARTH_ROCK_BLAST,            SpellData(15, 15, 15, 15,   90,   90,   90,   80,  0,  8, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_EARTH_TELEKINESIS,           SpellData(20, 20, 20, 20,  150,  150,  150,  150,  0,  0, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_EARTH_DEATH_BLOSSOM,         SpellData(25, 25, 25, 25,  100,  100,  100,   90, 20,  1, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_EARTH_MASS_DISTORTION,       SpellData(30, 30, 30, 30,   90,   90,   90,   90, 25,  0, 0, CHARACTER_SKILL_MASTERY_GRANDMASTER)},

    {SPELL_SPIRIT_DETECT_LIFE,          SpellData( 1,  1,  1,  1,  100,  100,  100,  100,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_SPIRIT_BLESS,                SpellData( 2,  2,  2,  2,  100,  100,  100,  100,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_SPIRIT_FATE,                 SpellData( 3,  3,  3,  3,   90,   90,   90,   90,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_SPIRIT_TURN_UNDEAD,          SpellData( 4,  4,  4,  4,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_SPIRIT_REMOVE_CURSE,         SpellData( 5,  5,  5,  5,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_SPIRIT_PRESERVATION,         SpellData( 8,  8,  8,  8,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_SPIRIT_HEROISM,              SpellData(10, 10, 10, 10,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_SPIRIT_SPIRIT_LASH,          SpellData(15, 15, 15, 15,  100,  100,  100,  100, 10,  8, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_SPIRIT_RAISE_DEAD,           SpellData(20, 20, 20, 20,  240,  240,  240,  240,  0,  0, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_SPIRIT_SHARED_LIFE,          SpellData(25, 25, 25, 25,  150,  150,  150,  150,  0,  0, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_SPIRIT_RESSURECTION,         SpellData(30, 30, 30, 30, 1000, 1000, 1000, 1000,  0,  0, 0, CHARACTER_SKILL_MASTERY_GRANDMASTER)},

    {SPELL_MIND_REMOVE_FEAR,            SpellData( 1,  1,  1,  1,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_MIND_MIND_BLAST,             SpellData( 2,  2,  2,  2,  110,  110,  110,  110,  3,  3, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_MIND_PROTECTION_FROM_MIND,   SpellData( 3,  3,  3,  3,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_MIND_TELEPATHY,              SpellData( 4,  4,  4,  4,  110,  100,   90,   80,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_MIND_CHARM,                  SpellData( 5,  5,  5,  5,  100,  100,  100,  100,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_MIND_CURE_PARALYSIS,         SpellData( 8,  8,  8,  8,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_MIND_BERSERK,                SpellData(10, 10, 10, 10,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_MIND_MASS_FEAR,              SpellData(15, 15, 15, 15,   80,   80,   80,   80,  0,  0, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_MIND_CURE_INSANITY,          SpellData(20, 20, 20, 20,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_MIND_PSYCHIC_SHOCK,          SpellData(25, 25, 25, 25,  110,  110,  110,  100, 12,  1, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_MIND_ENSLAVE,                SpellData(30, 30, 30, 30,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_GRANDMASTER)},

    {SPELL_BODY_CURE_WEAKNESS,          SpellData( 1,  1,  1,  1,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_BODY_FIRST_AID,              SpellData( 2,  2,  2,  2,  100,  100,  100,  100,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_BODY_PROTECTION_FROM_BODY,   SpellData( 3,  3,  3,  3,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_BODY_HARM,                   SpellData( 4,  4,  4,  4,  110,  100,   90,   80,  8,  2, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_BODY_REGENERATION,           SpellData( 5,  5,  5,  5,  110,  110,  110,  110,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_BODY_CURE_POISON,            SpellData( 8,  8,  8,  8,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_BODY_HAMMERHANDS,            SpellData(10, 10, 10, 10,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_BODY_CURE_DISEASE,           SpellData(15, 15, 15, 15,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_BODY_PROTECTION_FROM_MAGIC,  SpellData(20, 20, 20, 20,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_BODY_FLYING_FIST,            SpellData(25, 25, 25, 25,  110,  110,  110,  100, 30,  5, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_BODY_POWER_CURE,             SpellData(30, 30, 30, 30,  100,  100,  100,  100,  0,  0, 0, CHARACTER_SKILL_MASTERY_GRANDMASTER)},

    {SPELL_LIGHT_LIGHT_BOLT,            SpellData( 5,  5,  5,  5,  110,  100,   90,   80,  0,  4, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_LIGHT_DESTROY_UNDEAD,        SpellData(10, 10, 10, 10,  120,  110,  100,   90, 16, 16, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_LIGHT_DISPEL_MAGIC,          SpellData(15, 15, 15, 15,  120,  110,  100,   90,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_LIGHT_PARALYZE,              SpellData(20, 20, 20, 20,  160,  140,  120,  100,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_LIGHT_SUMMON_ELEMENTAL,      SpellData(25, 25, 25, 25,  140,  140,  140,  140,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_LIGHT_DAY_OF_THE_GODS,       SpellData(30, 30, 30, 30,  500,  500,  500,  500,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_LIGHT_PRISMATIC_LIGHT,       SpellData(35, 35, 35, 35,  135,  135,  120,  100, 25,  1, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_LIGHT_DAY_OF_PROTECTION,     SpellData(40, 40, 40, 40,  500,  500,  500,  500,  0,  0, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_LIGHT_HOUR_OF_POWER,         SpellData(45, 45, 45, 45,  250,  250,  250,  250,  0,  0, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_LIGHT_SUNRAY,                SpellData(50, 50, 50, 50,  150,  150,  150,  135, 20, 20, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_LIGHT_DIVINE_INTERVENTION,   SpellData(55, 55, 55, 55,  300,  300,  300,  300,  0,  0, 0, CHARACTER_SKILL_MASTERY_GRANDMASTER)},

    {SPELL_DARK_REANIMATE,              SpellData(10, 10, 10, 10,  140,  140,  140,  140,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_DARK_TOXIC_CLOUD,            SpellData(15, 15, 15, 15,  120,  110,  100,   90, 25, 10, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_DARK_VAMPIRIC_WEAPON,        SpellData(20, 20, 20, 20,  120,  100,   90,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_DARK_SHRINKING_RAY,          SpellData(25, 25, 25, 25,  120,  120,  120,  120,  0,  0, 0, CHARACTER_SKILL_MASTERY_NOVICE)},
    {SPELL_DARK_SHARPMETAL,             SpellData(30, 30, 30, 30,   90,   90,   80,   70,  6,  6, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_DARK_CONTROL_UNDEAD,         SpellData(35, 35, 35, 35,  120,  120,  100,   80,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_DARK_PAIN_REFLECTION,        SpellData(40, 40, 40, 40,  110,  110,  110,  110,  0,  0, 0, CHARACTER_SKILL_MASTERY_EXPERT)},
    {SPELL_DARK_SACRIFICE,              SpellData(45, 45, 45, 45,  200,  200,  200,  150,  0,  0, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_DARK_DRAGON_BREATH,          SpellData(50, 50, 50, 50,  120,  120,  120,  100,  0, 25, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_DARK_ARMAGEDDON,             SpellData(55, 55, 55, 55,  250,  250,  250,  250, 50,  1, 0, CHARACTER_SKILL_MASTERY_MASTER)},
    {SPELL_DARK_SOULDRINKER,            SpellData(60, 60, 60, 60,  300,  300,  300,  300, 25,  8, 0, CHARACTER_SKILL_MASTERY_GRANDMASTER)}
};

IndexedArray<std::array<struct SpellBookIconPos, 12>, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> pIconPos = {
    {MAGIC_SCHOOL_FIRE, {{{0,   0},   {17,  13},  {115, 2},   {217, 15},  {299, 6},   {28,  125},
                          {130, 133}, {294, 114}, {11,  232}, {134, 233}, {237, 171}, {296, 231}}}},

    {MAGIC_SCHOOL_AIR, {{{0,   0},   {19,  9},   {117, 3},   {206, 13},  {285, 7},   {16,  123},
                         {113, 101}, {201, 118}, {317, 110}, {11,  230}, {149, 236}, {296, 234}}}},

    {MAGIC_SCHOOL_WATER, {{{0,  0},   {17,  9},   {140, 0},   {210, 34},  {293, 5},   {15,  98},
                           {78, 121}, {175, 136}, {301, 115}, {15,  226}, {154, 225}, {272, 220}}}},

    {MAGIC_SCHOOL_EARTH, {{{0,   0},   {7,   9},   {156, 2},   {277, 9},   {11,  117}, {111, 82},
                           {180, 102}, {303, 108}, {10,  229}, {120, 221}, {201, 217}, {296, 225}}}},

    {MAGIC_SCHOOL_SPIRIT, {{{0,   0},   {18,  8},   {89,  15},  {192, 14},  {292, 7},   {22,  129},
                            {125, 146}, {217, 136}, {305, 115}, {22,  226}, {174, 237}, {290, 231}}}},

    {MAGIC_SCHOOL_MIND, {{{0,   0},  {18,  12},  {148, 9},   {292, 7},   {17,  122}, {121, 99},
                          {220, 87}, {293, 112}, {13,  236}, {128, 213}, {220, 223}, {315, 223}}}},

    {MAGIC_SCHOOL_BODY, {{{0,   0},   {23,  14},  {127, 8},   {204, 0},   {306, 8},   {14,  115},
                          {122, 132}, {200, 116}, {293, 122}, {20,  228}, {154, 228}, {294, 239}}}},

    {MAGIC_SCHOOL_LIGHT, {{{0,   0},  {19,  14},  {124, 10},  {283, 12},  {8,   105}, {113, 89},
                           {190, 82}, {298, 108}, {18,  181}, {101, 204}, {204, 203}, {285, 218}}}},

    {MAGIC_SCHOOL_DARK, {{{0,   0},   {18,  17},  {110, 16},  {201, 15},  {307, 15},  {18,  148},
                          {125, 166}, {201, 123}, {275, 120}, {28,  235}, {217, 222}, {324, 216}}}}
};

// TODO: use SoundID not uint16_t
// TODO(captainurist): Originally the array was two elements shorter, last two zeros are my addition. Can we drop elements for non-regular spells?
const IndexedArray<uint16_t, SPELL_FIRST_WITH_SPRITE, SPELL_LAST_WITH_SPRITE> SpellSoundIds = {{
    {SPELL_FIRE_TORCH_LIGHT, 10000},
    {SPELL_FIRE_FIRE_BOLT, 10010},
    {SPELL_FIRE_PROTECTION_FROM_FIRE, 10020},
    {SPELL_FIRE_FIRE_AURA, 10030},
    {SPELL_FIRE_HASTE, 10040},
    {SPELL_FIRE_FIREBALL, 10050},
    {SPELL_FIRE_FIRE_SPIKE, 10060},
    {SPELL_FIRE_IMMOLATION, 10070},
    {SPELL_FIRE_METEOR_SHOWER, 10080},
    {SPELL_FIRE_INFERNO, 10090},
    {SPELL_FIRE_INCINERATE, 10100},
    {SPELL_AIR_WIZARD_EYE, 11000},
    {SPELL_AIR_FEATHER_FALL, 11010},
    {SPELL_AIR_PROTECTION_FROM_AIR, 11020},
    {SPELL_AIR_SPARKS, 11030},
    {SPELL_AIR_JUMP, 11040},
    {SPELL_AIR_SHIELD, 11050},
    {SPELL_AIR_LIGHTNING_BOLT, 11060},
    {SPELL_AIR_INVISIBILITY, 11070},
    {SPELL_AIR_IMPLOSION, 11080},
    {SPELL_AIR_FLY, 11090},
    {SPELL_AIR_STARBURST, 11100},
    {SPELL_WATER_AWAKEN, 12000},
    {SPELL_WATER_POISON_SPRAY, 12010},
    {SPELL_WATER_PROTECTION_FROM_WATER, 12020},
    {SPELL_WATER_ICE_BOLT, 12030},
    {SPELL_WATER_WATER_WALK, 12040},
    {SPELL_WATER_RECHARGE_ITEM, 12050},
    {SPELL_WATER_ACID_BURST, 12060},
    {SPELL_WATER_ENCHANT_ITEM, 12070},
    {SPELL_WATER_TOWN_PORTAL, 12080},
    {SPELL_WATER_ICE_BLAST, 12090},
    {SPELL_WATER_LLOYDS_BEACON, 12100},
    {SPELL_EARTH_STUN, 13000},
    {SPELL_EARTH_SLOW, 13010},
    {SPELL_EARTH_PROTECTION_FROM_EARTH, 13020},
    {SPELL_EARTH_DEADLY_SWARM, 13030},
    {SPELL_EARTH_STONESKIN, 13040},
    {SPELL_EARTH_BLADES, 13050},
    {SPELL_EARTH_STONE_TO_FLESH, 13060},
    {SPELL_EARTH_ROCK_BLAST, 13070},
    {SPELL_EARTH_TELEKINESIS, 13080},
    {SPELL_EARTH_DEATH_BLOSSOM, 13090},
    {SPELL_EARTH_MASS_DISTORTION, 13100},
    {SPELL_SPIRIT_DETECT_LIFE, 14000},
    {SPELL_SPIRIT_BLESS, 14010},
    {SPELL_SPIRIT_FATE, 14020},
    {SPELL_SPIRIT_TURN_UNDEAD, 14030},
    {SPELL_SPIRIT_REMOVE_CURSE, 14040},
    {SPELL_SPIRIT_PRESERVATION, 14050},
    {SPELL_SPIRIT_HEROISM, 14060},
    {SPELL_SPIRIT_SPIRIT_LASH, 14070},
    {SPELL_SPIRIT_RAISE_DEAD, 14080},
    {SPELL_SPIRIT_SHARED_LIFE, 14090},
    {SPELL_SPIRIT_RESSURECTION, 14100},
    {SPELL_MIND_REMOVE_FEAR, 15000},
    {SPELL_MIND_MIND_BLAST, 15010},
    {SPELL_MIND_PROTECTION_FROM_MIND, 15020},
    {SPELL_MIND_TELEPATHY, 15030},
    {SPELL_MIND_CHARM, 15040},
    {SPELL_MIND_CURE_PARALYSIS, 15050},
    {SPELL_MIND_BERSERK, 15060},
    {SPELL_MIND_MASS_FEAR, 15070},
    {SPELL_MIND_CURE_INSANITY, 15080},
    {SPELL_MIND_PSYCHIC_SHOCK, 15090},
    {SPELL_MIND_ENSLAVE, 15100},
    {SPELL_BODY_CURE_WEAKNESS, 16000},
    {SPELL_BODY_FIRST_AID, 16010},
    {SPELL_BODY_PROTECTION_FROM_BODY, 16020},
    {SPELL_BODY_HARM, 16030},
    {SPELL_BODY_REGENERATION, 16040},
    {SPELL_BODY_CURE_POISON, 16050},
    {SPELL_BODY_HAMMERHANDS, 16060},
    {SPELL_BODY_CURE_DISEASE, 16070},
    {SPELL_BODY_PROTECTION_FROM_MAGIC, 16080},
    {SPELL_BODY_FLYING_FIST, 16090},
    {SPELL_BODY_POWER_CURE, 16100},
    {SPELL_LIGHT_LIGHT_BOLT, 17000},
    {SPELL_LIGHT_DESTROY_UNDEAD, 17010},
    {SPELL_LIGHT_DISPEL_MAGIC, 17020},
    {SPELL_LIGHT_PARALYZE, 17030},
    {SPELL_LIGHT_SUMMON_ELEMENTAL, 17040},
    {SPELL_LIGHT_DAY_OF_THE_GODS, 17050},
    {SPELL_LIGHT_PRISMATIC_LIGHT, 17060},
    {SPELL_LIGHT_DAY_OF_PROTECTION, 17070},
    {SPELL_LIGHT_HOUR_OF_POWER, 17080},
    {SPELL_LIGHT_SUNRAY, 17090},
    {SPELL_LIGHT_DIVINE_INTERVENTION, 17100},
    {SPELL_DARK_REANIMATE, 18000},
    {SPELL_DARK_TOXIC_CLOUD, 18010},
    {SPELL_DARK_VAMPIRIC_WEAPON, 18020},
    {SPELL_DARK_SHRINKING_RAY, 18030},
    {SPELL_DARK_SHARPMETAL, 18040},
    {SPELL_DARK_CONTROL_UNDEAD, 18050},
    {SPELL_DARK_PAIN_REFLECTION, 18060},
    {SPELL_DARK_SACRIFICE, 18070},
    {SPELL_DARK_DRAGON_BREATH, 18080},
    {SPELL_DARK_ARMAGEDDON, 18090},
    {SPELL_DARK_SOULDRINKER, 18100},
    {SPELL_BOW_ARROW, 00001},
    {SPELL_101, 00000},
    {SPELL_LASER_PROJECTILE, 00000}
}};

void SpellBuff::Reset() {
    skillMastery = CHARACTER_SKILL_MASTERY_NONE;
    power = 0;
    expireTime = Time();
    caster = 0;
    isGMBuff = false;
    if (overlayID) {
        pActiveOverlayList->pOverlays[overlayID - 1].Reset();
        overlayID = 0;
    }
}

bool SpellBuff::IsBuffExpiredToTime(Time time) {
    if (this->expireTime && (this->expireTime < time)) {
        expireTime.SetExpired();
        power = 0;
        skillMastery = CHARACTER_SKILL_MASTERY_NONE;
        overlayID = 0;
        return true;
    }
    return false;
}

bool SpellBuff::Apply(Time expire_time, CharacterSkillMastery uSkillMastery,
                      int uPower, int uOverlayID,
                      uint8_t caster) {
    // For bug catching
    assert(uSkillMastery >= CHARACTER_SKILL_MASTERY_NOVICE && uSkillMastery <= CHARACTER_SKILL_MASTERY_GRANDMASTER);

    if (this->expireTime && (expire_time < this->expireTime)) {
        return false;
    }

    this->skillMastery = uSkillMastery;
    this->power = uPower;
    this->expireTime = expire_time;
    if (this->overlayID && this->overlayID != uOverlayID) {
        pActiveOverlayList->pOverlays[this->overlayID - 1].Reset();
        this->overlayID = 0;
    }
    this->overlayID = uOverlayID;
    this->caster = caster;

    return true;
}

void SpellStats::Initialize(const Blob &spells) {
    std::map<std::string, DamageType, ascii::NoCaseLess> spellSchoolMaps; // TODO(captainurist): #enum, use enum serialization
    spellSchoolMaps["fire"] = DAMAGE_FIRE;
    spellSchoolMaps["air"] = DAMAGE_AIR;
    spellSchoolMaps["water"] = DAMAGE_WATER;
    spellSchoolMaps["earth"] = DAMAGE_EARTH;
    spellSchoolMaps["spirit"] = DAMAGE_SPIRIT;
    spellSchoolMaps["mind"] = DAMAGE_MIND;
    spellSchoolMaps["body"] = DAMAGE_BODY;
    spellSchoolMaps["light"] = DAMAGE_LIGHT;
    spellSchoolMaps["dark"] = DAMAGE_DARK;
    spellSchoolMaps["magic"] = DAMAGE_MAGIC;

    char *test_string;

    std::string txtRaw(spells.string_view());

    strtok(txtRaw.data(), "\r");
    for (SpellId uSpellID : allRegularSpells()) {
        if (((std::to_underlying(uSpellID) % 11) - 1) == 0) {
            strtok(NULL, "\r");
        }
        test_string = strtok(NULL, "\r") + 1;

        auto tokens = tokenize(test_string, '\t');

        pInfos[uSpellID].name = removeQuotes(tokens[2]);
        pInfos[uSpellID].damageType = valueOr(spellSchoolMaps, tokens[3], DAMAGE_PHYSICAL);
        pInfos[uSpellID].pShortName = removeQuotes(tokens[4]);
        pInfos[uSpellID].pDescription = removeQuotes(tokens[5]);
        pInfos[uSpellID].pBasicSkillDesc = removeQuotes(tokens[6]);
        pInfos[uSpellID].pExpertSkillDesc = removeQuotes(tokens[7]);
        pInfos[uSpellID].pMasterSkillDesc = removeQuotes(tokens[8]);
        pInfos[uSpellID].pGrandmasterSkillDesc = removeQuotes(tokens[9]);
        pSpellDatas[uSpellID].flags |= strchr(tokens[10], 'm') || strchr(tokens[10], 'M') ? SPELL_CASTABLE_BY_MONSTER : SpellFlag();
        pSpellDatas[uSpellID].flags |= strchr(tokens[10], 'e') || strchr(tokens[10], 'E') ? SPELL_CASTABLE_BY_EVENT : SpellFlag();
        pSpellDatas[uSpellID].flags |= strchr(tokens[10], 'c') || strchr(tokens[10], 'C') ? SPELL_SHIFT_CLICK_CASTABLE : SpellFlag();
        pSpellDatas[uSpellID].flags |= strchr(tokens[10], 'x') || strchr(tokens[10], 'X') ? SPELL_FLAG_8 : SpellFlag();
    }
}

void eventCastSpell(SpellId uSpellID, CharacterSkillMastery skillMastery, int skillLevel, Vec3f from, Vec3f to) {
    // For bug catching
    assert(skillMastery >= CHARACTER_SKILL_MASTERY_NOVICE && skillMastery <= CHARACTER_SKILL_MASTERY_GRANDMASTER);

    Vec3f coord_delta;
    if (to.lengthSqr() > 1.0f) {
        coord_delta = to - from;
    } else {
        coord_delta = pParty->pos - from + Vec3f(0, 0, pParty->eyeLevel);
    }

    int yaw = 0;
    int pitch = 0;
    float distance_to_target = coord_delta.length();
    if (distance_to_target <= 1.0) {
        distance_to_target = 1;
    } else {
        int64_t ySquared = coord_delta.y * coord_delta.y;
        int64_t xSquared = coord_delta.x * coord_delta.x;
        int xy_distance = (int)std::sqrt((double)(xSquared + ySquared));
        yaw = TrigLUT.atan2(coord_delta.x, coord_delta.y);
        pitch = TrigLUT.atan2(xy_distance, coord_delta.z);
    }

    SpriteObject spell_sprites;

    switch (uSpellID) {
        case SPELL_FIRE_FIRE_BOLT:
        case SPELL_FIRE_FIREBALL:
        case SPELL_AIR_LIGHTNING_BOLT:
        case SPELL_WATER_ICE_BOLT:
        case SPELL_WATER_ACID_BURST:
        case SPELL_WATER_ICE_BLAST:
        case SPELL_EARTH_BLADES:
        case SPELL_EARTH_ROCK_BLAST:
        case SPELL_WATER_POISON_SPRAY:
        case SPELL_AIR_SPARKS:
        case SPELL_EARTH_DEATH_BLOSSOM:
            spell_sprites.uType = SpellSpriteMapping[uSpellID];
            spell_sprites.containing_item.Reset();
            spell_sprites.uSpellID = uSpellID;
            spell_sprites.spell_level = skillLevel;
            spell_sprites.spell_skill = skillMastery;
            spell_sprites.uObjectDescID = pObjectList->ObjectIDByItemID(spell_sprites.uType);
            spell_sprites.vPosition = from;
            spell_sprites.uAttributes = SPRITE_IGNORE_RANGE;
            spell_sprites.uSectorID = pIndoor->GetSector(from);
            spell_sprites.field_60_distance_related_prolly_lod = distance_to_target;
            spell_sprites.timeSinceCreated = 0_ticks;
            spell_sprites.spell_caster_pid = Pid(OBJECT_Item, 1000); // 8000 | OBJECT_Item;
            spell_sprites.uSoundID = 0;
            break;
        default:
            break;
    }

    Duration spell_length;
    int spell_power = 0;
    int launch_angle;
    int launch_speed;
    int spell_num_objects;
    int spell_spray_arc;
    int spell_spray_angles;
    int spriteid;
    PartyBuff buff_id;

    switch (uSpellID) {
        case SPELL_FIRE_FIRE_BOLT:
        case SPELL_FIRE_FIREBALL:
        case SPELL_AIR_LIGHTNING_BOLT:
        case SPELL_WATER_ICE_BOLT:
        case SPELL_WATER_ACID_BURST:
        case SPELL_WATER_ICE_BLAST:
        case SPELL_EARTH_BLADES:
        case SPELL_EARTH_ROCK_BLAST:
            // v20 = yaw;
            spell_sprites.spell_target_pid = Pid();
            spell_sprites.uFacing = yaw;
            spell_sprites.uSoundID = 0;
            launch_speed = pObjectList->pObjects[(int16_t)spell_sprites.uObjectDescID].uSpeed;
            spriteid = spell_sprites.Create(yaw, pitch, launch_speed, 0);
            //    pAudioPlayer->PlaySound(word_4EE088_sound_ids[uSpellID],
            //    0, 0, fromx, fromy, 0, 0, 0);
            pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_PID, Pid(OBJECT_Item, spriteid));
            break;
        case SPELL_WATER_POISON_SPRAY:
            spell_num_objects = (std::to_underlying(skillMastery) * 2) - 1;
            spell_sprites.spell_target_pid = Pid();
            spell_sprites.uFacing = yaw;
            if (spell_num_objects == 1) {
                launch_speed = pObjectList->pObjects[(int16_t)spell_sprites.uObjectDescID].uSpeed;
                spriteid = spell_sprites.Create(yaw, pitch, launch_speed, 0);
            } else {
                spell_spray_arc = (signed int)(60 * TrigLUT.uIntegerDoublePi) / 360;
                spell_spray_angles = spell_spray_arc / (spell_num_objects - 1);
                for (int i = spell_spray_arc / -2; i <= spell_spray_arc / 2; i += spell_spray_angles) {
                    spell_sprites.uFacing = i + yaw;
                    spriteid = spell_sprites.Create(i + yaw, pitch, pObjectList->pObjects[spell_sprites.uObjectDescID].uSpeed, 0);
                }
            }
            //    pAudioPlayer->PlaySound(word_4EE088_sound_ids[uSpellID],
            //    0, 0, fromx, fromy, 0, 0, 0);
            pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_PID, Pid(OBJECT_Item, spriteid));
            break;
        case SPELL_AIR_SPARKS:
            spell_num_objects = (std::to_underlying(skillMastery) * 2) + 1;
            spell_spray_arc = (signed int)(60 * TrigLUT.uIntegerDoublePi) / 360;
            spell_spray_angles = spell_spray_arc / (spell_num_objects - 1);
            spell_sprites.spell_target_pid = Pid::character(0);
            for (int i = spell_spray_arc / -2; i <= spell_spray_arc / 2; i += spell_spray_angles) {
                spell_sprites.uFacing = i + yaw;
                spriteid = spell_sprites.Create(i + yaw, pitch, pObjectList->pObjects[spell_sprites.uObjectDescID].uSpeed, 0);
            }
            //    pAudioPlayer->PlaySound(word_4EE088_sound_ids[uSpellID],
            //    0, 0, fromx, fromy, 0, 0, 0);
            pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_PID, Pid(OBJECT_Item, spriteid));
            break;
        case SPELL_EARTH_DEATH_BLOSSOM:
            if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
                return;
            }
            spell_sprites.spell_target_pid = Pid::character(0);
            launch_speed = pObjectList->pObjects[spell_sprites.uObjectDescID].uSpeed;
            launch_angle = TrigLUT.uIntegerHalfPi / 2;
            spriteid = spell_sprites.Create(yaw, launch_angle, launch_speed, 0);
            //    pAudioPlayer->PlaySound(word_4EE088_sound_ids[uSpellID],
            //    0, 0, fromx, fromy, 0, 0, 0);
            pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_PID, Pid(OBJECT_Item, spriteid));
            break;

        case SPELL_FIRE_HASTE:
            if (skillMastery >= CHARACTER_SKILL_MASTERY_NOVICE) {
                if (skillMastery <= CHARACTER_SKILL_MASTERY_EXPERT) {
                    spell_length = Duration::fromHours(1) + Duration::fromMinutes(skillLevel);
                } else if (skillMastery == CHARACTER_SKILL_MASTERY_MASTER) {
                    spell_length = Duration::fromHours(1) + Duration::fromMinutes(3 * skillLevel);
                } else if (skillMastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                    spell_length = Duration::fromHours(1) + Duration::fromMinutes(4 * skillLevel);
                }
            }
            for (Character &player : pParty->pCharacters) {
                if (player.IsWeak()) {
                    return;
                }
            }
            pParty->pPartyBuffs[PARTY_BUFF_HASTE].Apply(pParty->GetPlayingTime() + spell_length, skillMastery, 0, 0, 0);
            spell_fx_renderer->SetPartyBuffAnim(uSpellID);
            //    pAudioPlayer->PlaySound(word_4EE088_sound_ids[uSpellID],
            //    0, 0, fromx, fromy, 0, 0, 0);  // звук алтаря
            //    Pid was 0
            pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_EXCLUSIVE);
            break;
        case SPELL_AIR_SHIELD:
        case SPELL_EARTH_STONESKIN:
        case SPELL_SPIRIT_HEROISM:
            switch (skillMastery) {
                case CHARACTER_SKILL_MASTERY_NOVICE:
                case CHARACTER_SKILL_MASTERY_EXPERT:
                    spell_length = Duration::fromHours(1) + Duration::fromMinutes(5 * skillLevel);
                    break;
                case CHARACTER_SKILL_MASTERY_MASTER:
                    spell_length = Duration::fromHours(1) + Duration::fromMinutes(15 * skillLevel);
                    break;
                case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                    spell_length = Duration::fromHours(skillLevel + 1);
                    break;
                default:
                    assert(false);
                    break;
            }
            if (uSpellID == SPELL_AIR_SHIELD) {
                spell_power = 0;
                buff_id = PARTY_BUFF_SHIELD;
            } else if (uSpellID == SPELL_EARTH_STONESKIN) {
                spell_power = skillLevel + 5;
                buff_id = PARTY_BUFF_STONE_SKIN;
            } else {
                assert(uSpellID == SPELL_SPIRIT_HEROISM);
                spell_power = skillLevel + 5;
                buff_id = PARTY_BUFF_HEROISM;
            }
            spell_fx_renderer->SetPartyBuffAnim(uSpellID);
            pParty->pPartyBuffs[buff_id].Apply(pParty->GetPlayingTime() + spell_length, skillMastery, spell_power, 0, 0);
            //    pAudioPlayer->PlaySound(word_4EE088_sound_ids[uSpellID],
            //    0, 0, fromx, fromy, 0, 0, 0);
            //    Pid was 0
            pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_EXCLUSIVE);
            break;
        case SPELL_FIRE_IMMOLATION:
            if (skillMastery == CHARACTER_SKILL_MASTERY_GRANDMASTER) {
                spell_length = Duration::fromMinutes(10 * skillLevel);
            } else {
                spell_length = Duration::fromMinutes(skillLevel);
            }
            spell_fx_renderer->SetPartyBuffAnim(uSpellID);
            pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].Apply(pParty->GetPlayingTime() + spell_length, skillMastery, skillLevel, 0, 0);
            //    pAudioPlayer->PlaySound(word_4EE088_sound_ids[uSpellID],
            //    0, 0, fromx, fromy, 0, 0, 0);
            //    Pid was 0
            pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_EXCLUSIVE);
            break;
        case SPELL_FIRE_PROTECTION_FROM_FIRE:
        case SPELL_AIR_PROTECTION_FROM_AIR:
        case SPELL_WATER_PROTECTION_FROM_WATER:
        case SPELL_EARTH_PROTECTION_FROM_EARTH:
        case SPELL_MIND_PROTECTION_FROM_MIND:
        case SPELL_BODY_PROTECTION_FROM_BODY:
            spell_length = Duration::fromHours(skillLevel);
            spell_power = skillLevel * std::to_underlying(skillMastery);

            if (uSpellID == SPELL_FIRE_PROTECTION_FROM_FIRE) {
                buff_id = PARTY_BUFF_RESIST_FIRE;
            } else if (uSpellID == SPELL_AIR_PROTECTION_FROM_AIR) {
                buff_id = PARTY_BUFF_RESIST_AIR;
            } else if (uSpellID == SPELL_WATER_PROTECTION_FROM_WATER) {
                buff_id = PARTY_BUFF_RESIST_WATER;
            } else if (uSpellID == SPELL_EARTH_PROTECTION_FROM_EARTH) {
                buff_id = PARTY_BUFF_RESIST_EARTH;
            } else if (uSpellID == SPELL_MIND_PROTECTION_FROM_MIND) {
                buff_id = PARTY_BUFF_RESIST_MIND;
            } else {
                assert(uSpellID == SPELL_BODY_PROTECTION_FROM_BODY);
                buff_id = PARTY_BUFF_RESIST_BODY;
            }

            spell_fx_renderer->SetPartyBuffAnim(uSpellID);
            pParty->pPartyBuffs[buff_id].Apply(pParty->GetPlayingTime() + spell_length, skillMastery, spell_power, 0, 0);
            //    pAudioPlayer->PlaySound(word_4EE088_sound_ids[uSpellID],
            //    0, 0, fromx, fromy, 0, 0, 0);
            //    Pid was 0
            pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_EXCLUSIVE);
            break;
        case SPELL_LIGHT_DAY_OF_THE_GODS:
            // Spell lengths for master and grandmaster were mixed up
            switch (skillMastery) {
                case CHARACTER_SKILL_MASTERY_EXPERT:
                    spell_length = Duration::fromHours(3 * skillLevel);
                    spell_power = 3 * skillLevel + 10;
                    break;
                case CHARACTER_SKILL_MASTERY_MASTER:
                    spell_length = Duration::fromHours(4 * skillLevel);
                    spell_power = 5 * skillLevel + 10;
                    break;
                case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                    spell_length = Duration::fromHours(5 * skillLevel);
                    spell_power = 4 * skillLevel + 10;
                    break;
                default:
                    break;
            }
            spell_fx_renderer->SetPartyBuffAnim(uSpellID);

            pParty->pPartyBuffs[PARTY_BUFF_DAY_OF_GODS].Apply(pParty->GetPlayingTime() + spell_length, skillMastery, spell_power, 0, 0);
            //    pAudioPlayer->PlaySound(word_4EE088_sound_ids[uSpellID],
            //    0, 0, fromx, fromy, 0, 0, 0);
            //    Pid was 0
            pAudioPlayer->playSpellSound(uSpellID, false, SOUND_MODE_EXCLUSIVE);
            break;
        default:
            break;
    }
}

bool IsSpellQuickCastableOnShiftClick(SpellId uSpellID) {
    return pSpellDatas[uSpellID].flags & (SPELL_SHIFT_CLICK_CASTABLE | SPELL_FLAG_8);
}

int CalcSpellDamage(SpellId uSpellID, int spellLevel, CharacterSkillMastery skillMastery, int currentHp) {
    int result;       // eax@1
    unsigned int diceSides;  // [sp-4h] [bp-8h]@9

    result = 0;
    if (uSpellID == SPELL_FIRE_FIRE_SPIKE) {
        switch (skillMastery) {
            case CHARACTER_SKILL_MASTERY_NOVICE:
            case CHARACTER_SKILL_MASTERY_EXPERT:
                diceSides = 6;
                break;
            case CHARACTER_SKILL_MASTERY_MASTER:
                diceSides = 8;
                break;
            case CHARACTER_SKILL_MASTERY_GRANDMASTER:
                diceSides = 10;
                break;
            default:
                return 0;
        }
        result = grng->randomDice(spellLevel, diceSides);
    } else if (uSpellID == SPELL_EARTH_MASS_DISTORTION) {
        result = currentHp * (pSpellDatas[SPELL_EARTH_MASS_DISTORTION].baseDamage + 2 * spellLevel) / 100;
    } else {
        result = pSpellDatas[uSpellID].baseDamage + grng->randomDice(spellLevel, pSpellDatas[uSpellID].bonusSkillDamage);
    }

    return result;
}

void armageddonProgress() {
    assert(uCurrentlyLoadedLevelType == LEVEL_OUTDOOR && pParty->armageddon_timer > 0_ticks);

    if (pParty->armageddon_timer > 417_ticks) {
        pParty->armageddon_timer = 0_ticks;
        return; // TODO(captainurist): wtf? Looks like a quick hack for some bug.
    }

    if (pTurnEngine->pending_actions) {
        --pTurnEngine->pending_actions;
    }

    pParty->_viewYaw = TrigLUT.uDoublePiMask & (pParty->_viewYaw + grng->randomInSegment(-8, 8)); // Was RandomInSegment(-8, 7)
    pParty->_viewPitch = std::clamp(pParty->_viewPitch + grng->randomInSegment(-8, 8), -128, 128); // Was RandomInSegment(-8, 7)
    pParty->armageddon_timer = std::max(0_ticks, pParty->armageddon_timer - pEventTimer->dt()); // Was pMiscTimer

    // TODO(pskelton): ignore if pEventTimer->uTimeElapsed is zero?
    // TODO(captainurist): See the logic in Outdoor.cpp, right now the force is applied in fixed amounts per frame,
    // while it should be applied in amounts relative to frame time --- basically, armageddon should provide some
    // acceleration, and then this acceleration should be applied to actors over a brief period of time.
    --pParty->armageddonForceCount;
    if (pParty->armageddon_timer) {
        return; // Deal damage only when timer gets to 0.
    }

    int outgoingDamage = pParty->armageddonDamage + 50;

    for (Actor &actor : pActors) {
        if (!actor.CanAct()) {
            continue; // TODO(captainurist): paralyzed & summoned actors should receive damage too!
        }

        int incomingDamage = actor.CalcMagicalDamageToActor(DAMAGE_MAGIC, outgoingDamage);
        if (incomingDamage > 0) {
            actor.currentHP -= incomingDamage;

            if (actor.currentHP >= 0) {
                Actor::AI_Stun(actor.id, Pid::character(0), 0);
            } else {
                Actor::Die(actor.id);
                if (actor.monsterInfo.exp) {
                    pParty->GivePartyExp(pMonsterStats->infos[actor.monsterInfo.id].exp);
                }
            }
        }
    }

    for (Character &player : pParty->pCharacters) {
        if (!player.conditions.HasAny({CONDITION_DEAD, CONDITION_PETRIFIED, CONDITION_ERADICATED})) {
            player.receiveDamage(outgoingDamage, DAMAGE_MAGIC);
        }
    }
}
