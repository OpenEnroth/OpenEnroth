#include "Engine/Spells/Spells.h"

#include <map>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/LOD.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/SpellFxRenderer.h"

#include "../OurMath.h"
#include "../Party.h"

#include "Media/Audio/AudioPlayer.h"

using EngineIoc = Engine_::IocContainer;

SpellFxRenderer *spell_fx_renderer = EngineIoc::ResolveSpellFxRenderer();

std::array<TownPortalData, 6> TownPortalList =  // 4ECBB8
    {{{Vec3_int_(-5121, 2107, 1), 1536, 0, 21, 0},
      {Vec3_int_(-15148, -10240, 1473), 0, 0, 4, 0},
      {Vec3_int_(-10519, 5375, 753), 512, 0, 3, 0},
      {Vec3_int_(3114, -11055, 513), 0, 0, 10, 0},
      {Vec3_int_(-158, 7624, 1), 512, 0, 7, 0},
      {Vec3_int_(-1837, -4247, 65), 65, 0, 8, 0}}};

struct SpellStats *pSpellStats = nullptr;

std::array<stru324_spell_id_to_sprite_mapping, 103>
    spell_sprite_mapping =  // 4E3ACC
    {{{SPRITE_SPELL_0, 0},
      {SPRITE_SPELL_FIRE_TORCH_LIGHT, 0},
      {SPRITE_SPELL_FIRE_FIRE_BOLT, 0},
      {SPRITE_SPELL_FIRE_PROTECTION_FROM_FIRE, 0},
      {SPRITE_SPELL_FIRE_FIRE_AURA, 0},
      {SPRITE_SPELL_FIRE_HASTE, 0},
      {SPRITE_SPELL_FIRE_FIREBALL, 0},
      {SPRITE_SPELL_FIRE_FIRE_SPIKE, 0},
      {SPRITE_SPELL_FIRE_IMMOLATION, 0},
      {SPRITE_SPELL_FIRE_METEOR_SHOWER, 0},
      {SPRITE_SPELL_FIRE_INFERNO, 0},
      {SPRITE_SPELL_FIRE_INCINERATE, 0},

      {SPRITE_SPELL_AIR_WIZARD_EYE, 0},
      {SPRITE_SPELL_AIR_FEATHER_FALL, 0},
      {SPRITE_SPELL_AIR_PROTECTION_FROM_AIR, 0},
      {SPRITE_SPELL_AIR_SPARKS, 0},
      {SPRITE_SPELL_AIR_JUMP, 0},
      {SPRITE_SPELL_AIR_SHIELD, 0},
      {SPRITE_SPELL_AIR_LIGHNING_BOLT, 0},
      {SPRITE_SPELL_AIR_INVISIBILITY, 0},
      {SPRITE_SPELL_AIR_IMPLOSION, 0},
      {SPRITE_SPELL_AIR_FLY, 0},
      {SPRITE_SPELL_AIR_STARBURST, 0},

      {SPRITE_SPELL_WATER_AWAKEN, 0},
      {SPRITE_SPELL_WATER_POISON_SPRAY, 0},
      {SPRITE_SPELL_WATER_PROTECTION_FROM_WATER, 0},
      {SPRITE_SPELL_WATER_ICE_BOLT, 0},
      {SPRITE_SPELL_WATER_WATER_WALK, 0},
      {SPRITE_SPELL_WATER_RECHARGE_ITEM, 0},
      {SPRITE_SPELL_WATER_ACID_BURST, 0},
      {SPRITE_SPELL_WATER_ENCHANT_ITEM, 0},
      {SPRITE_SPELL_WATER_TOWN_PORTAL, 0},
      {SPRITE_SPELL_WATER_ICE_BLAST, 0},
      {SPRITE_SPELL_WATER_LLOYDS_BEACON, 0},

      {SPRITE_SPELL_EARTH_STUN, 0},
      {SPRITE_SPELL_EARTH_SLOW, 0},
      {SPRITE_SPELL_EARTH_PROTECTION_FROM_EARTH, 0},
      {SPRITE_SPELL_EARTH_DEADLY_SWARM, 0},
      {SPRITE_SPELL_EARTH_STONESKIN, 0},
      {SPRITE_SPELL_EARTH_BLADES, 0},
      {SPRITE_SPELL_EARTH_STONE_TO_FLESH, 0},
      {SPRITE_SPELL_EARTH_ROCK_BLAST, 0},
      {SPRITE_SPELL_EARTH_TELEKINESIS, 0},
      {SPRITE_SPELL_EARTH_DEATH_BLOSSOM, 0},
      {SPRITE_SPELL_EARTH_MASS_DISTORTION, 0},

      {SPRITE_SPELL_SPIRIT_DETECT_LIFE, 0},
      {SPRITE_SPELL_SPIRIT_BLESS, 0},
      {SPRITE_SPELL_SPIRIT_FATE, 0},
      {SPRITE_SPELL_SPIRIT_TURN_UNDEAD, 0},
      {SPRITE_SPELL_SPIRIT_REMOVE_CURSE, 0},
      {SPRITE_SPELL_SPIRIT_PRESERVATION, 0},
      {SPRITE_SPELL_SPIRIT_HEROISM, 0},
      {SPRITE_SPELL_SPIRIT_SPIRIT_LASH, 0},
      {SPRITE_SPELL_SPIRIT_RAISE_DEAD, 0},
      {SPRITE_SPELL_SPIRIT_SHARED_LIFE, 0},
      {SPRITE_SPELL_SPIRIT_RESSURECTION, 0},

      {SPRITE_SPELL_MIND_REMOVE_FEAR, 0},
      {SPRITE_SPELL_MIND_MIND_BLAST, 0},
      {SPRITE_SPELL_MIND_PROTECTION_FROM_MIND, 0},
      {SPRITE_SPELL_MIND_TELEPATHY, 0},
      {SPRITE_SPELL_MIND_CHARM, 0},
      {SPRITE_SPELL_MIND_CURE_PARALYSIS, 0},
      {SPRITE_SPELL_MIND_BERSERK, 0},
      {SPRITE_SPELL_MIND_MASS_FEAR, 0},
      {SPRITE_SPELL_MIND_CURE_INSANITY, 0},
      {SPRITE_SPELL_MIND_PSYCHIC_SHOCK, 0},
      {SPRITE_SPELL_MIND_ENSLAVE, 0},

      {SPRITE_SPELL_BODY_CURE_WEAKNESS, 0},
      {SPRITE_SPELL_BODY_FIRST_AID, 0},
      {SPRITE_SPELL_BODY_PROTECTION_FROM_BODY, 0},
      {SPRITE_SPELL_BODY_HARM, 0},
      {SPRITE_SPELL_BODY_REGENERATION, 0},
      {SPRITE_SPELL_BODY_CURE_POISON, 0},
      {SPRITE_SPELL_BODY_HAMMERHANDS, 0},
      {SPRITE_SPELL_BODY_CURE_DISEASE, 0},
      {SPRITE_SPELL_BODY_PROTECTION_FROM_MAGIC, 0},
      {SPRITE_SPELL_BODY_FLYING_FIST, 0},
      {SPRITE_SPELL_BODY_POWER_CURE, 0},

      {SPRITE_SPELL_LIGHT_LIGHT_BOLT, 0},
      {SPRITE_SPELL_LIGHT_DESTROY_UNDEAD, 0},
      {SPRITE_SPELL_LIGHT_DISPEL_MAGIC, 0},
      {SPRITE_SPELL_LIGHT_PARALYZE, 0},
      {SPRITE_SPELL_LIGHT_SUMMON_ELEMENTAL, 0},
      {SPRITE_SPELL_LIGHT_DAY_OF_THE_GODS, 0},
      {SPRITE_SPELL_LIGHT_PRISMATIC_LIGHT, 0},
      {SPRITE_SPELL_LIGHT_DAY_OF_PROTECTION, 0},
      {SPRITE_SPELL_LIGHT_HOUR_OF_POWER, 0},
      {SPRITE_SPELL_LIGHT_SUNRAY, 0},
      {SPRITE_SPELL_LIGHT_DIVINE_INTERVENTION, 0},

      {SPRITE_SPELL_DARK_REANIMATE, 0},
      {SPRITE_SPELL_DARK_TOXIC_CLOUD, 0},
      {SPRITE_SPELL_DARK_VAMPIRIC_WEAPON, 0},
      {SPRITE_SPELL_DARK_SHRINKING_RAY, 0},
      {SPRITE_SPELL_DARK_SHARPMETAL, 0},
      {SPRITE_SPELL_DARK_CONTROL_UNDEAD, 0},
      {SPRITE_SPELL_DARK_PAIN_REFLECTION, 0},
      {SPRITE_SPELL_DARK_SACRIFICE, 0},
      {SPRITE_SPELL_DARK_DRAGON_BREATH, 0},
      {SPRITE_SPELL_DARK_ARMAGEDDON, 0},
      {SPRITE_SPELL_DARK_SOULDRINKER, 0},

      { SPRITE_ARROW_PROJECTILE, 0},
      { SPRITE_ARROW_PROJECTILE, 0},
      { SPRITE_BLASTER_PROJECTILE, 0}}};

SpellData::SpellData(__int16 innormalMana, __int16 inExpertLevelMana,
                     __int16 inMasterLevelMana, __int16 inMagisterLevelMana,
                     __int16 inNormalLevelRecovery,
                     __int16 inExpertLevelRecovery,
                     __int16 inMasterLevelRecovery,
                     __int16 inMagisterLevelRecovery, __int8 inbaseDamage,
                     __int8 inbonusSkillDamage, __int16 instats)
    : uNormalLevelMana(innormalMana),
      uExpertLevelMana(inExpertLevelMana),
      uMasterLevelMana(inMasterLevelMana),
      uMagisterLevelMana(inMagisterLevelMana),
      uNormalLevelRecovery(inNormalLevelRecovery),
      uExpertLevelRecovery(inExpertLevelRecovery),
      uMasterLevelRecovery(inMasterLevelRecovery),
      uMagisterLevelRecovery(inMagisterLevelRecovery),
      baseDamage(inbaseDamage),
      bonusSkillDamage(inbonusSkillDamage),
      stats(instats) {}

// 9 spellbook pages  11 spells per page 9*11 =99 +1 zero struct at 0. It
// counted from 1!
std::array<SpellData, 100> pSpellDatas = {
    {SpellData(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
     SpellData(1, 1, 1, 1, 60, 60, 60, 40, 0, 0, 0),  // 0 fire
     SpellData(2, 2, 2, 2, 110, 110, 100, 90, 3, 3, 0),
     SpellData(3, 3, 3, 3, 120, 120, 120, 120, 0, 0, 0),
     SpellData(4, 4, 4, 4, 120, 120, 120, 120, 0, 0, 0),
     SpellData(5, 5, 5, 5, 120, 120, 120, 120, 0, 0, 0),
     SpellData(8, 8, 8, 8, 100, 100, 90, 80, 0, 6, 0),
     SpellData(10, 10, 10, 10, 150, 150, 150, 150, 0, 6, 0),
     SpellData(15, 15, 15, 15, 120, 120, 120, 120, 0, 6, 0),
     SpellData(20, 20, 20, 20, 100, 100, 100, 90, 8, 1, 0),
     SpellData(25, 25, 25, 25, 100, 100, 100, 90, 12, 1, 0),
     SpellData(30, 30, 30, 30, 90, 90, 90, 90, 15, 15, 0),

     SpellData(1, 1, 1, 0, 60, 60, 60, 60, 0, 0, 0),  // 1 air
     SpellData(2, 2, 2, 2, 120, 120, 120, 100, 0, 0, 0),
     SpellData(3, 3, 3, 3, 120, 120, 120, 120, 0, 0, 0),
     SpellData(4, 4, 4, 4, 110, 100, 90, 80, 2, 1, 0),
     SpellData(5, 5, 5, 5, 90, 90, 70, 50, 0, 0, 0),
     SpellData(8, 8, 8, 8, 120, 120, 120, 120, 0, 0, 0),
     SpellData(10, 10, 10, 10, 100, 100, 90, 70, 0, 8, 0),
     SpellData(15, 15, 15, 15, 200, 200, 200, 200, 0, 0, 0),
     SpellData(20, 20, 20, 20, 100, 100, 100, 90, 10, 10, 0),
     SpellData(25, 25, 25, 25, 250, 250, 250, 250, 0, 0, 0),
     SpellData(30, 30, 30, 30, 90, 90, 90, 90, 20, 1, 0),

     SpellData(1, 1, 1, 1, 60, 60, 60, 20, 0, 0, 0),  // 2 water
     SpellData(2, 2, 2, 2, 110, 100, 90, 70, 2, 2, 0),
     SpellData(3, 3, 3, 3, 120, 120, 120, 120, 0, 0, 0),
     SpellData(4, 4, 4, 4, 110, 100, 90, 80, 0, 4, 0),
     SpellData(5, 5, 5, 5, 150, 150, 150, 150, 0, 0, 0),
     SpellData(8, 8, 8, 8, 200, 200, 200, 200, 0, 0, 0),
     SpellData(10, 10, 10, 10, 100, 100, 90, 80, 9, 9, 0),
     SpellData(15, 15, 15, 15, 140, 140, 140, 140, 0, 0, 0),
     SpellData(20, 20, 20, 20, 200, 200, 200, 200, 0, 0, 0),
     SpellData(25, 25, 25, 25, 80, 80, 80, 80, 12, 3, 0),
     SpellData(30, 30, 30, 30, 250, 250, 250, 250, 0, 0, 0),

     SpellData(1, 1, 1, 1, 80, 80, 80, 80, 0, 0, 0),  // 3 earth
     SpellData(2, 2, 2, 2, 100, 100, 100, 100, 0, 0, 0),
     SpellData(3, 3, 3, 3, 120, 120, 120, 120, 0, 0, 0),
     SpellData(4, 4, 4, 4, 110, 100, 90, 80, 5, 3, 0),
     SpellData(5, 5, 5, 5, 120, 120, 120, 120, 0, 0, 0),
     SpellData(8, 8, 8, 8, 100, 100, 90, 80, 0, 9, 0),
     SpellData(10, 10, 10, 10, 140, 140, 140, 140, 0, 0, 0),
     SpellData(15, 15, 15, 15, 90, 90, 90, 80, 0, 8, 0),
     SpellData(20, 20, 20, 20, 150, 150, 150, 150, 0, 0, 0),
     SpellData(25, 25, 25, 25, 100, 100, 100, 90, 20, 1, 0),
     SpellData(30, 30, 30, 30, 90, 90, 90, 90, 25, 0, 0),

     SpellData(1, 1, 1, 1, 100, 100, 100, 100, 0, 0, 0),  // 4 spirit
     SpellData(2, 2, 2, 2, 100, 100, 100, 100, 0, 0, 0),
     SpellData(3, 3, 3, 3, 90, 90, 90, 90, 0, 0, 0),
     SpellData(4, 4, 4, 4, 120, 120, 120, 120, 0, 0, 0),
     SpellData(5, 5, 5, 5, 120, 120, 120, 120, 0, 0, 0),
     SpellData(8, 8, 8, 8, 120, 120, 120, 120, 0, 0, 0),
     SpellData(10, 10, 10, 10, 120, 120, 120, 120, 0, 0, 0),
     SpellData(15, 15, 15, 15, 100, 100, 100, 100, 10, 8, 0),
     SpellData(20, 20, 20, 20, 240, 240, 240, 240, 0, 0, 0),
     SpellData(25, 25, 25, 25, 150, 150, 150, 150, 0, 0, 0),
     SpellData(30, 30, 30, 30, 1000, 1000, 1000, 1000, 0, 0, 0),

     SpellData(1, 1, 1, 1, 120, 120, 120, 120, 0, 0, 0),  // 5 mind
     SpellData(2, 2, 2, 2, 110, 110, 110, 110, 3, 3, 0),
     SpellData(3, 3, 3, 3, 120, 120, 120, 120, 0, 0, 0),
     SpellData(4, 4, 4, 4, 110, 100, 90, 80, 0, 0, 0),
     SpellData(5, 5, 5, 5, 100, 100, 100, 100, 0, 0, 0),
     SpellData(8, 8, 8, 8, 120, 120, 120, 120, 0, 0, 0),
     SpellData(10, 10, 10, 10, 120, 120, 120, 120, 0, 0, 0),
     SpellData(15, 15, 15, 15, 80, 80, 80, 80, 0, 0, 0),
     SpellData(20, 20, 20, 20, 120, 120, 120, 120, 0, 0, 0),
     SpellData(25, 25, 25, 25, 110, 110, 110, 100, 12, 12, 0),
     SpellData(30, 30, 30, 30, 120, 120, 120, 120, 0, 0, 0),

     SpellData(1, 1, 1, 1, 120, 120, 120, 120, 0, 0, 0),  // 6 body
     SpellData(2, 2, 2, 2, 100, 100, 100, 100, 0, 0, 0),
     SpellData(3, 3, 3, 3, 120, 120, 120, 120, 0, 0, 0),
     SpellData(4, 4, 4, 4, 110, 100, 90, 80, 8, 2, 0),
     SpellData(5, 5, 5, 5, 110, 110, 110, 110, 0, 0, 0),
     SpellData(8, 8, 8, 8, 120, 120, 120, 120, 0, 0, 0),
     SpellData(10, 10, 10, 10, 120, 120, 120, 120, 0, 0, 0),
     SpellData(15, 15, 15, 15, 120, 120, 120, 120, 0, 0, 0),
     SpellData(20, 20, 20, 20, 120, 120, 120, 120, 0, 0, 0),
     SpellData(25, 25, 25, 25, 110, 110, 110, 100, 30, 5, 0),
     SpellData(30, 30, 30, 30, 100, 100, 100, 100, 0, 0, 0),

     SpellData(5, 5, 5, 5, 110, 100, 90, 80, 0, 4, 0),  // 7 light
     SpellData(10, 10, 10, 10, 120, 110, 100, 90, 16, 16, 0),
     SpellData(15, 15, 15, 15, 120, 110, 100, 90, 0, 0, 0),
     SpellData(20, 20, 20, 20, 160, 140, 120, 100, 0, 0, 0),
     SpellData(25, 25, 25, 25, 140, 140, 140, 140, 0, 0, 0),
     SpellData(30, 30, 30, 30, 500, 500, 500, 500, 0, 0, 0),
     SpellData(35, 35, 35, 35, 135, 135, 120, 100, 25, 1, 0),
     SpellData(40, 40, 40, 40, 500, 500, 500, 500, 0, 0, 0),
     SpellData(45, 45, 45, 45, 250, 250, 250, 250, 0, 0, 0),
     SpellData(50, 50, 50, 50, 150, 150, 150, 135, 20, 20, 0),
     SpellData(55, 55, 55, 55, 300, 300, 300, 300, 0, 0, 0),

     SpellData(10, 10, 10, 10, 140, 140, 140, 140, 0, 0, 0),  // 8 dark
     SpellData(15, 15, 15, 15, 120, 110, 100, 90, 25, 10, 0),
     SpellData(20, 20, 20, 20, 120, 100, 90, 120, 0, 0, 0),
     SpellData(25, 25, 25, 25, 120, 120, 120, 120, 0, 0, 0),
     SpellData(30, 30, 30, 30, 90, 90, 80, 70, 6, 6, 0),
     SpellData(35, 35, 35, 35, 120, 120, 100, 80, 0, 0, 0),
     SpellData(40, 40, 40, 40, 110, 110, 110, 110, 0, 0, 0),
     SpellData(45, 45, 45, 45, 200, 200, 200, 150, 0, 0, 0),
     SpellData(50, 50, 50, 50, 120, 120, 120, 100, 0, 25, 0),
     SpellData(55, 55, 55, 55, 250, 250, 250, 250, 50, 1, 0),
     SpellData(60, 60, 60, 60, 300, 300, 300, 300, 25, 8, 0)}};

std::array<SPELL_TYPE, 25> wand_spell_ids = {{
    // 135 Wand of Fire               136 Wand of Sparks             137 Wand of
    // Poison             138 Wand of Stunning           139 Wand of Harm
    SPELL_FIRE_FIRE_BOLT, SPELL_AIR_SPARKS, SPELL_WATER_POISON_SPRAY,
    SPELL_EARTH_STUN, SPELL_BODY_HARM,
    // 140 Fairy Wand of Light        141 Fairy Wand of Ice          142 Fairy
    // Wand of Lashing      143 Fairy Wand of Mind         144 Fairy Wand of
    // Swarms
    SPELL_LIGHT_LIGHT_BOLT, SPELL_WATER_ICE_BOLT, SPELL_SPIRIT_SPIRIT_LASH,
    SPELL_MIND_MIND_BLAST, SPELL_EARTH_DEADLY_SWARM,
    // 145 Alacorn Wand of Fireballs  146 Alacorn Wand of Acid       147 Alacorn
    // Wand of Lightning  148 Alacorn Wand of Blades     149 Alacorn Wand of
    // Charms
    SPELL_FIRE_FIREBALL, SPELL_WATER_ACID_BURST, SPELL_AIR_LIGHNING_BOLT,
    SPELL_EARTH_BLADES, SPELL_MIND_CHARM,
    // 150 Arcane Wand of Blasting    151 Arcane Wand of The Fist    152 Arcane
    // Wand of Rocks       153 Arcane Wand of Paralyzing  154 Arcane Wand of
    // Clouds
    SPELL_WATER_ICE_BLAST, SPELL_BODY_FLYING_FIST, SPELL_EARTH_ROCK_BLAST,
    SPELL_LIGHT_PARALYZE, SPELL_DARK_TOXIC_CLOUD,
    // 155 Mystic Wand of Implosion   156 Mystic Wand of Distortion  157 Mystic
    // Wand of Sharpmetal  158 Mystic Wand of Shrinking   159 Mystic Wand of
    // Incineration
    SPELL_AIR_IMPLOSION, SPELL_EARTH_MASS_DISTORTION, SPELL_DARK_SHARPMETAL,
    SPELL_DARK_SHRINKING_RAY, SPELL_FIRE_INCINERATE
}};

std::array<std::array<struct SpellBookIconPos, 12>, 9> pIconPos = {{
    {{{0,   0},   {17,  13},  {115, 2},   {217, 15},  {299, 6},   {28,  125},
      {130, 133}, {294, 114}, {11,  232}, {134, 233}, {237, 171}, {296, 231}}},

    {{{0,   0},   {19,  9},   {117, 3},   {206, 13},  {285, 7},   {16,  123},
      {113, 101}, {201, 118}, {317, 110}, {11,  230}, {149, 236}, {296, 234}}},

    {{{0,  0},   {17,  9},   {140, 0},   {210, 34},  {293, 5},   {15,  98},
      {78, 121}, {175, 136}, {301, 115}, {15,  226}, {154, 225}, {272, 220}}},

    {{{0,   0},   {7,   9},   {156, 2},   {277, 9},   {11,  117}, {111, 82},
      {180, 102}, {303, 108}, {10,  229}, {120, 221}, {201, 217}, {296, 225}}},

    {{{0,   0},   {18,  8},   {89,  15},  {192, 14},  {292, 7},   {22,  129},
      {125, 146}, {217, 136}, {305, 115}, {22,  226}, {174, 237}, {290, 231}}},

    {{{0,   0},  {18,  12},  {148, 9},   {292, 7},   {17,  122}, {121, 99},
      {220, 87}, {293, 112}, {13,  236}, {128, 213}, {220, 223}, {315, 223}}},

    {{{0,   0},   {23,  14},  {127, 8},   {204, 0},   {306, 8},   {14,  115},
      {122, 132}, {200, 116}, {293, 122}, {20,  228}, {154, 228}, {294, 239}}},

    {{{0,   0},  {19,  14},  {124, 10},  {283, 12},  {8,   105}, {113, 89},
      {190, 82}, {298, 108}, {18,  181}, {101, 204}, {204, 203}, {285, 218}}},

    {{{0,   0},   {18,  17},  {110, 16},  {201, 15},  {307, 15},  {18,  148},
      {125, 166}, {201, 123}, {275, 120}, {28,  235}, {217, 222}, {324, 216}}}
}};

//----- (00458585) --------------------------------------------------------
void SpellBuff::Reset() {
    uSkill = 0;
    uPower = 0;
    expire_time.Reset();
    uCaster = 0;
    uFlags = 0;
    if (uOverlayID) {
        pOtherOverlayList->pOverlays[uOverlayID - 1].Reset();
        pOtherOverlayList->bRedraw = true;
        uOverlayID = 0;
    }
}

//----- (004585CA) --------------------------------------------------------
bool SpellBuff::IsBuffExpiredToTime(GameTime time) {
    if (this->expire_time && (this->expire_time < time)) {
        expire_time.Reset();
        uPower = 0;
        uSkill = 0;
        uOverlayID = 0;
        return true;
    }
    return false;
}

//----- (004584E0) --------------------------------------------------------
bool SpellBuff::Apply(GameTime expire_time, unsigned __int16 uSkillLevel,
                      unsigned __int16 uPower, int uOverlayID,
                      unsigned __int8 caster) {
    if (this->expire_time && (expire_time < this->expire_time)) {
        return false;
    }

    this->uSkill = uSkillLevel;
    this->uPower = uPower;
    this->expire_time = expire_time;
    if (this->uOverlayID && this->uOverlayID != uOverlayID) {
        pOtherOverlayList->pOverlays[this->uOverlayID - 1].Reset();
        pOtherOverlayList->bRedraw = true;
        this->uOverlayID = 0;
    }
    this->uOverlayID = uOverlayID;
    this->uCaster = caster;

    return true;
}

//----- (0045384A) --------------------------------------------------------
void SpellStats::Initialize() {
    std::map<String, SPELL_SCHOOL, decltype(iless_functor)> spellSchoolMaps(iless_functor);
    spellSchoolMaps["fire"] = SPELL_SCHOOL_FIRE;
    spellSchoolMaps["air"] = SPELL_SCHOOL_AIR;
    spellSchoolMaps["water"] = SPELL_SCHOOL_WATER;
    spellSchoolMaps["earth"] = SPELL_SCHOOL_EARTH;
    spellSchoolMaps["spirit"] = SPELL_SCHOOL_SPIRIT;
    spellSchoolMaps["mind"] = SPELL_SCHOOL_MIND;
    spellSchoolMaps["body"] = SPELL_SCHOOL_BODY;
    spellSchoolMaps["light"] = SPELL_SCHOOL_LIGHT;
    spellSchoolMaps["dark"] = SPELL_SCHOOL_DARK;
    spellSchoolMaps["magic"] = SPELL_SCHOOL_MAGIC;

    char *test_string;

    free(pSpellsTXT_Raw);
    pSpellsTXT_Raw = (char *)pEvents_LOD->LoadCompressedTexture("spells.txt");

    strtok(pSpellsTXT_Raw, "\r");
    for (int i = 1; i < 100; ++i) {
        if (((i % 11) - 1) == 0) strtok(NULL, "\r");
        test_string = strtok(NULL, "\r") + 1;

        extern std::vector<char *> Tokenize(char *input, const char separator);
        auto tokens = Tokenize(test_string, '\t');

        pInfos[i].pName = RemoveQuotes(tokens[2]);
        auto findResult = spellSchoolMaps.find(tokens[3]);
        pInfos[i].uSchool = findResult == spellSchoolMaps.end()
                                ? SPELL_SCHOOL_NONE
                                : findResult->second;
        pInfos[i].pShortName = RemoveQuotes(tokens[4]);
        pInfos[i].pDescription = RemoveQuotes(tokens[5]);
        pInfos[i].pBasicSkillDesc = RemoveQuotes(tokens[6]);
        pInfos[i].pExpertSkillDesc = RemoveQuotes(tokens[7]);
        pInfos[i].pMasterSkillDesc = RemoveQuotes(tokens[8]);
        pInfos[i].pGrandmasterSkillDesc = RemoveQuotes(tokens[9]);
        pSpellDatas[i].stats |= strchr(tokens[10], 'm') || strchr(tokens[10], 'M') ? 1 : 0;
        pSpellDatas[i].stats |= strchr(tokens[10], 'e') || strchr(tokens[10], 'E') ? 2 : 0;
        pSpellDatas[i].stats |= strchr(tokens[10], 'c') || strchr(tokens[10], 'C') ? 4 : 0;
        pSpellDatas[i].stats |= strchr(tokens[10], 'x') || strchr(tokens[10], 'X') ? 8 : 0;
    }
}

void EventCastSpell(int uSpellID, int uSkillLevel, int uSkill, int fromx,
                    int fromy, int fromz, int tox, int toy, int toz) {
    unsigned int skillMasteryPlusOne = uSkillLevel + 1;
    Assert(skillMasteryPlusOne > 0 && skillMasteryPlusOne <= 4,
           "Invalid mastery level");

    int64_t x_coord_delta = 0;
    int64_t y_coord_delta = 0;
    int64_t z_coord_delta = 0;
    if (tox || toy || toz) {
        x_coord_delta = tox - fromx;
        y_coord_delta = toy - fromy;
        z_coord_delta = toz - fromz;
    } else {
        x_coord_delta = pParty->vPosition.x - fromx;
        y_coord_delta = pParty->vPosition.y - fromy;
        z_coord_delta = (pParty->vPosition.z + pParty->sEyelevel) - fromz;
    }

    int yaw = 0;
    int pitch = 0;
    double distance_to_target = sqrt((long double)(
        x_coord_delta * x_coord_delta + y_coord_delta * y_coord_delta +
        z_coord_delta * z_coord_delta));
    if (distance_to_target <= 1.0) {
        distance_to_target = 1;
    } else {
        int64_t ySquared = y_coord_delta * y_coord_delta;
        int64_t xSquared = x_coord_delta * x_coord_delta;
        int xy_distance = (int)sqrt((long double)(xSquared + ySquared));
        yaw = TrigLUT->Atan2((int)x_coord_delta, (int)y_coord_delta);
        pitch = TrigLUT->Atan2(xy_distance, (int)z_coord_delta);
    }

    SpriteObject spell_sprites;

    switch (uSpellID) {
        case SPELL_FIRE_FIRE_BOLT:
        case SPELL_FIRE_FIREBALL:
        case SPELL_AIR_LIGHNING_BOLT:
        case SPELL_WATER_ICE_BOLT:
        case SPELL_WATER_ACID_BURST:
        case SPELL_WATER_ICE_BLAST:
        case SPELL_EARTH_BLADES:
        case SPELL_EARTH_ROCK_BLAST:
        case SPELL_WATER_POISON_SPRAY:
        case SPELL_AIR_SPARKS:
        case SPELL_EARTH_DEATH_BLOSSOM:
            spell_sprites.uType = spell_sprite_mapping[uSpellID].uSpriteType;
            spell_sprites.containing_item.Reset();
            spell_sprites.spell_id = uSpellID;
            spell_sprites.spell_level = uSkill;
            spell_sprites.spell_skill = skillMasteryPlusOne;
            spell_sprites.uObjectDescID = pObjectList->ObjectIDByItemID(spell_sprites.uType);
            spell_sprites.vPosition.x = fromx;
            spell_sprites.vPosition.y = fromy;
            spell_sprites.vPosition.z = fromz;
            spell_sprites.uAttributes = 16;
            spell_sprites.uSectorID = pIndoor->GetSector(fromx, fromy, fromz);
            spell_sprites.field_60_distance_related_prolly_lod = distance_to_target;
            spell_sprites.uSpriteFrameID = 0;
            spell_sprites.spell_caster_pid = 8000 | OBJECT_Item;
            spell_sprites.uSoundID = 0;
            break;
    }

    int spell_length = 0;
    GameTime spell_expire_time;
    int spell_power;
    int launch_angle;
    int launch_speed;
    int spell_num_objects = 0;
    int spell_spray_arc;
    int spell_spray_angles;
    int spriteid;

    switch (uSpellID) {
        case SPELL_FIRE_FIRE_BOLT:
        case SPELL_FIRE_FIREBALL:
        case SPELL_AIR_LIGHNING_BOLT:
        case SPELL_WATER_ICE_BOLT:
        case SPELL_WATER_ACID_BURST:
        case SPELL_WATER_ICE_BLAST:
        case SPELL_EARTH_BLADES:
        case SPELL_EARTH_ROCK_BLAST:
            // v20 = yaw;
            spell_sprites.spell_target_pid = 0;
            spell_sprites.uFacing = yaw;
            spell_sprites.uSoundID = 0;
            launch_speed =
                pObjectList
                    ->pObjects[(signed __int16)spell_sprites.uObjectDescID]
                    .uSpeed;
            spriteid = spell_sprites.Create(yaw, pitch, launch_speed, 0);
            //    pAudioPlayer->PlaySound((SoundID)word_4EE088_sound_ids[uSpellID],
            //    0, 0, fromx, fromy, 0, 0, 0);
            pAudioPlayer->PlaySpellSound(uSpellID, PID(OBJECT_Item, spriteid));
            return;
        case SPELL_WATER_POISON_SPRAY:
            spell_num_objects = (skillMasteryPlusOne * 2) - 1;
            spell_sprites.spell_target_pid = 0;
            spell_sprites.uFacing = yaw;
            if (spell_num_objects == 1) {
                launch_speed =
                    pObjectList
                        ->pObjects[(signed __int16)spell_sprites.uObjectDescID]
                        .uSpeed;
                spriteid = spell_sprites.Create(yaw, pitch, launch_speed, 0);
            } else {
                spell_spray_arc =
                    (signed int)(60 * TrigLUT->uIntegerDoublePi) / 360;
                spell_spray_angles = spell_spray_arc / (spell_num_objects - 1);
                for (int i = spell_spray_arc / -2; i <= spell_spray_arc / 2;
                     i += spell_spray_angles) {
                    spell_sprites.uFacing = i + yaw;
                    spriteid = spell_sprites.Create(
                        i + yaw, pitch,
                        pObjectList->pObjects[spell_sprites.uObjectDescID]
                            .uSpeed,
                        0);
                }
            }
            //    pAudioPlayer->PlaySound((SoundID)word_4EE088_sound_ids[uSpellID],
            //    0, 0, fromx, fromy, 0, 0, 0);
            pAudioPlayer->PlaySpellSound(uSpellID, PID(OBJECT_Item, spriteid));
            return;
        case SPELL_AIR_SPARKS:
            spell_num_objects = (skillMasteryPlusOne * 2) + 1;
            spell_spray_arc =
                (signed int)(60 * TrigLUT->uIntegerDoublePi) / 360;
            spell_spray_angles = spell_spray_arc / (spell_num_objects - 1);
            spell_sprites.spell_target_pid = 4;
            for (int i = spell_spray_arc / -2; i <= spell_spray_arc / 2;
                 i += spell_spray_angles) {
                spell_sprites.uFacing = i + yaw;
                spriteid = spell_sprites.Create(
                    i + yaw, pitch,
                    pObjectList->pObjects[spell_sprites.uObjectDescID].uSpeed,
                    0);
            }
            //    pAudioPlayer->PlaySound((SoundID)word_4EE088_sound_ids[uSpellID],
            //    0, 0, fromx, fromy, 0, 0, 0);
            pAudioPlayer->PlaySpellSound(uSpellID, PID(OBJECT_Item, spriteid));
            return;
        case SPELL_EARTH_DEATH_BLOSSOM:
            if (uCurrentlyLoadedLevelType == LEVEL_Indoor) return;
            spell_sprites.spell_target_pid = 4;
            launch_speed =
                pObjectList->pObjects[spell_sprites.uObjectDescID].uSpeed;
            launch_angle = TrigLUT->uIntegerHalfPi / 2;
            spriteid = spell_sprites.Create(yaw, launch_angle, launch_speed, 0);
            //    pAudioPlayer->PlaySound((SoundID)word_4EE088_sound_ids[uSpellID],
            //    0, 0, fromx, fromy, 0, 0, 0);
            pAudioPlayer->PlaySpellSound(uSpellID, PID(OBJECT_Item, spriteid));
            return;

        case SPELL_FIRE_HASTE:
            if (skillMasteryPlusOne > 0) {
                if (skillMasteryPlusOne <= 2)
                    spell_length = 60 * (uSkill + 60);
                else if (skillMasteryPlusOne == 3)
                    spell_length = 180 * (uSkill + 20);
                else if (skillMasteryPlusOne == 4)
                    spell_length = 240 * (uSkill + 15);
            }
            for (uint i = 0; i < 4; ++i)
                if (pParty->pPlayers[i].IsWeak()) return;
            pParty->pPartyBuffs[PARTY_BUFF_HASTE].Apply(
                GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spell_length)),
                skillMasteryPlusOne, 0, 0, 0);
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 0);
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 1);
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 2);
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 3);
            //    pAudioPlayer->PlaySound((SoundID)word_4EE088_sound_ids[uSpellID],
            //    0, 0, fromx, fromy, 0, 0, 0);  // звук алтаря
            pAudioPlayer->PlaySpellSound(uSpellID, 0);
            return;
        case SPELL_AIR_SHIELD:
        case SPELL_EARTH_STONESKIN:
        case SPELL_SPIRIT_HEROISM:
            switch (skillMasteryPlusOne) {
                case 1:
                case 2:
                    spell_length = 300 * (uSkill + 12);
                    break;
                case 3:
                    spell_length = 900 * (uSkill + 4);
                    break;
                case 4:
                    spell_length = 3600 * (uSkill + 1);
                    break;
            }
            switch (uSpellID) {
                case SPELL_AIR_SHIELD:
                    spell_num_objects = 0;
                    uSkill = 14;
                    break;
                case SPELL_EARTH_STONESKIN:
                    spell_num_objects = uSkill + 5;
                    uSkill = 15;
                    break;
                case SPELL_SPIRIT_HEROISM:
                    spell_num_objects = uSkill + 5;
                    uSkill = 9;
                    break;
            }
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 0);
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 1);
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 2);
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 3);
            spell_expire_time =
                GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spell_length));
            pParty->pPartyBuffs[uSkill].Apply(spell_expire_time,
                                              skillMasteryPlusOne,
                                              spell_num_objects, 0, 0);
            //    pAudioPlayer->PlaySound((SoundID)word_4EE088_sound_ids[uSpellID],
            //    0, 0, fromx, fromy, 0, 0, 0);
            pAudioPlayer->PlaySpellSound(uSpellID, 0);
            return;
        case SPELL_FIRE_IMMOLATION:
            if (skillMasteryPlusOne == 4)
                spell_length = 600 * uSkill;
            else
                spell_length = 60 * uSkill;
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 0);
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 1);
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 2);
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 3);

            spell_expire_time =
                GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spell_length));
            pParty->pPartyBuffs[PARTY_BUFF_IMMOLATION].Apply(
                spell_expire_time, skillMasteryPlusOne, uSkill, 0, 0);
            //    pAudioPlayer->PlaySound((SoundID)word_4EE088_sound_ids[uSpellID],
            //    0, 0, fromx, fromy, 0, 0, 0);
            pAudioPlayer->PlaySpellSound(uSpellID, 0);
            return;
        case SPELL_FIRE_PROTECTION_FROM_FIRE:
        case SPELL_AIR_PROTECTION_FROM_AIR:
        case SPELL_WATER_PROTECTION_FROM_WATER:
        case SPELL_EARTH_PROTECTION_FROM_EARTH:
        case SPELL_MIND_PROTECTION_FROM_MIND:
        case SPELL_BODY_PROTECTION_FROM_BODY:
            spell_length = 3600 * uSkill;
            spell_num_objects = uSkill * skillMasteryPlusOne;

            switch (uSpellID) {
                case SPELL_FIRE_PROTECTION_FROM_FIRE:
                    uSkill = PARTY_BUFF_RESIST_FIRE;
                    break;
                case SPELL_AIR_PROTECTION_FROM_AIR:
                    uSkill = PARTY_BUFF_RESIST_AIR;
                    break;
                case SPELL_WATER_PROTECTION_FROM_WATER:
                    uSkill = PARTY_BUFF_RESIST_WATER;
                    break;
                case SPELL_EARTH_PROTECTION_FROM_EARTH:
                    uSkill = PARTY_BUFF_RESIST_EARTH;
                    break;
                case SPELL_MIND_PROTECTION_FROM_MIND:
                    uSkill = PARTY_BUFF_RESIST_MIND;
                    break;
                case SPELL_BODY_PROTECTION_FROM_BODY:
                    uSkill = PARTY_BUFF_RESIST_BODY;
                    break;
            }
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 0);
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 1);
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 2);
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 3);
            pParty->pPartyBuffs[uSkill].Apply(
                GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spell_length)),
                skillMasteryPlusOne, spell_num_objects, 0, 0);
            //    pAudioPlayer->PlaySound((SoundID)word_4EE088_sound_ids[uSpellID],
            //    0, 0, fromx, fromy, 0, 0, 0);
            pAudioPlayer->PlaySpellSound(uSpellID, 0);
            return;
        case SPELL_LIGHT_DAY_OF_THE_GODS:
            switch (skillMasteryPlusOne) {
                case 2:
                    spell_length = 10800 * uSkill;
                    spell_power = 3 * uSkill + 10;
                    break;
                case 3:
                    spell_length = 18000 * uSkill;
                    spell_power = 5 * uSkill + 10;
                    break;
                case 4:
                    spell_length = 14400 * uSkill;
                    spell_power = 4 * uSkill + 10;
                    break;
            }
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 0);
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 1);
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 2);
            spell_fx_renderer->SetPlayerBuffAnim(uSpellID, 3);

            spell_expire_time =
                GameTime(pParty->GetPlayingTime() + GameTime::FromSeconds(spell_length));
            pParty->pPartyBuffs[PARTY_BUFF_DAY_OF_GODS].Apply(
                spell_expire_time, skillMasteryPlusOne, spell_power, 0, 0);
            //    pAudioPlayer->PlaySound((SoundID)word_4EE088_sound_ids[uSpellID],
            //    0, 0, fromx, fromy, 0, 0, 0);
            pAudioPlayer->PlaySpellSound(uSpellID, 0);
            return;
        default:
            return;
    }
}

//----- (00427769) --------------------------------------------------------
bool sub_427769_isSpellQuickCastableOnShiftClick(unsigned int uSpellID) {
    return (pSpellDatas[uSpellID].stats & 0xC) != 0;
}
//----- (0043AFE3) --------------------------------------------------------
int _43AFE3_calc_spell_damage(int spellId, int spellLevel,
                              signed int skillMastery, int currentHp) {
    int result;       // eax@1
    unsigned int v5;  // [sp-4h] [bp-8h]@9

    result = 0;
    if (spellId == SPELL_FIRE_FIRE_SPIKE) {
        switch (skillMastery) {
            case 1:
            case 2:
                v5 = 6;
                break;
            case 3:
                v5 = 8;
                break;
            case 4:
                v5 = 10;
                break;
            default:
                return 0;
        }
        result = GetDiceResult(spellLevel, v5);
    } else if (spellId == SPELL_EARTH_MASS_DISTORTION) {
        result = currentHp *
            (pSpellDatas[SPELL_EARTH_MASS_DISTORTION].baseDamage +
                2 * spellLevel) /
            100;
    } else {
        result =
            pSpellDatas[spellId].baseDamage +
            GetDiceResult(spellLevel, pSpellDatas[spellId].bonusSkillDamage);
    }

    return result;
}
