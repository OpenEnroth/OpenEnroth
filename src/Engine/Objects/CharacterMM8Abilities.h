#pragma once

#include "Core/Time/Duration.h"
#include "Engine/Objects/CharacterEnums.h"

class RandomEngine;

/**
 * MM8 race innate abilities that have no MM7 analogue. These are meant to be called from a single
 * `gameType() == GAME_TYPE_MM8 && character->GetRace() == RACE_MM8_...`-guarded call site added to the
 * relevant existing combat/tick code (e.g. `Actor::DamageMonsterFromParty` for melee, `RegeneratePartyHealthMana`
 * for periodic effects, or `Character::GetBaseResistance` for passive stat bonuses), rather than by editing
 * that code's MM7 paths: one function (or function pair) per race here, one guarded call site at the relevant hook.
 * The Dragon's Flight has no formula to extract, its guarded hook is directly in
 * `_494035_timed_effects__water_walking_damage__etc`, Engine.cpp.
 *
 * Vampire "Lifedrain" (MM8 manual, Vampire Abilities): 3 + 1-3 per skill point damage on a hit, and a third of
 * the damage dealt heals the Vampire. Modeled here at the ability's untrained baseline (skill value 1), since
 * the "Vampire Ability" skill itself isn't wired into the engine yet (see `mm8StartingSkillAffinity`'s doc
 * comment in CharacterEnumFunctions.cpp). Scaling this with skill investment is not implemented.
 *
 * Neither the manual nor any of MM8's `Data/Tables` `*.txt` files describe a sun/daylight penalty for Vampires.
 */
int mm8VampireLifedrainDamage(RandomEngine &rng);

/**
 * @param lifedrainDamage               The Lifedrain damage actually dealt to the target (post-resistance),
 *                                       as returned by `mm8VampireLifedrainDamage` after the caller applies
 *                                       target resistance.
 * @return                              HP the Vampire heals from that hit - 1/3 of the damage dealt, per the
 *                                       MM8 manual.
 */
int mm8VampireLifedrainHeal(int lifedrainDamage);

/**
 * Troll "Regeneration": per the MM8 manual/`skilldes.txt`, Trolls start with the Regeneration skill (the
 * "Class Starting Skills" table's "F" - forced/predetermined - column), which heals hp over time, faster at
 * higher skill levels. Neither source gives an exact per-skill-point rate, so - same as Vampire Lifedrain -
 * this is modeled at the ability's untrained baseline (skill value 1), with the Regeneration rate the game
 * uses for that value (`CHARACTER_BUFF_REGENERATION`'s `5 * power` in `Engine.cpp`, i.e. 5 hp
 * per 5-minute tick per point).
 *
 * @return                              HP a Troll innately regains per 5-minute tick, meant to be added to
 *                                       `RegenData::hpRegen` in `Engine.cpp`'s regen-tick loop.
 */
int mm8TrollRegenerationHpPerTick();

/**
 * MM8 race innate starting resistances, sourced from the MM8 manual's "Starting Resistance" table (p.20):
 * Dark Elf gets +5 Fire/Air/Water/Earth, Minotaur gets +5 Spirit/Mind/Body, Troll gets +5 Water/Body. (The
 * same table also lists Vampire and Dragon values, which are not applied here.)
 *
 * @param race                          An MM8 race, as tested by `isMm8Race`.
 * @param resistance                    One of `ATTRIBUTE_RESIST_FIRE`/`_AIR`/`_WATER`/`_EARTH`/`_MIND`/
 *                                       `_BODY`/`_SPIRIT`.
 * @return                              The race's innate starting bonus for that resistance, or 0 if none.
 */
int mm8RaceStartingResistanceBonus(Race race, Attribute resistance);

/**
 * MM8 Dragon "Flame Blast" breath-weapon damage. Per the manual (Dragon Abilities, Expert level) it does
 * 10 + 1-10 per skill point fire damage. Modeled at the untrained baseline (skill value 1), same convention
 * and same reason as `mm8VampireLifedrainDamage` - the "Dragon Ability" skill isn't wired into the engine
 * yet. The manual's class description treats breath as an innate trait, so - like Lifedrain - this applies
 * unconditionally, not behind the (unimplemented) skill/mastery progression.
 *
 * This is a different, MM8-only mechanic from the `SPELL_DARK_DRAGON_BREATH` monster spell
 * (enemy dragon-type monsters attacking the party).
 */
int mm8DragonBreathDamage(RandomEngine &rng);

/**
 * MM8 Vampire "Levitate" duration, from spells.txt row 112.
 *
 * @param mastery                       Mastery of the Vampire skill.
 * @param skillLevel                    Skill points in the Vampire skill.
 * @return                              How long the party levitates.
 */
Duration mm8VampireLevitateDuration(Mastery mastery, int skillLevel);
