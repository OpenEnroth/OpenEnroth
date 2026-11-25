#include "MonsterEnumFunctions.h"

#include <cassert>
#include <string>
#include <utility>
#include <vector>

#include "Utility/IndexedArray.h"

struct MonsterData {
    Sex sex = SEX_MALE;
    Race race = RACE_HUMAN;
    bool appearsInArena = false;

    constexpr MonsterData() = default;
    constexpr MonsterData(Sex sex, Race race, bool appearsInArena) : sex(sex), race(race), appearsInArena(appearsInArena) {}
};

// TODO(captainurist): a bit weird that all the monsters belong to RACE_HUMAN.
static constexpr IndexedArray<MonsterData, MONSTER_TYPE_FIRST, MONSTER_TYPE_LAST> dataByMonsterType = {
    // type                                  sex            race            appearsInArena
    {MONSTER_TYPE_ANGEL,                    {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_ARCHER,                   {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_BAT,                      {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_BEHEMOTH,                 {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_BEHOLDER,                 {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_CLERIC_MOON,              {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_CLERIC_SUN,               {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_DEVIL,                    {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_DRAGON,                   {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_DRAGONFLY,                {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_DWARF,                    {SEX_MALE,      RACE_DWARF,     true}},
    {MONSTER_TYPE_ELEMENTAL_AIR,            {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_ELEMENTAL_EARTH,          {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_ELEMENTAL_FIRE,           {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_ELEMENTAL_LIGHT,          {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_ELEMENTAL_WATER,          {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_ELF_ARCHER,               {SEX_MALE,      RACE_ELF,       true}},
    {MONSTER_TYPE_ELF_SPEARMAN,             {SEX_MALE,      RACE_ELF,       true}},
    {MONSTER_TYPE_FIGHTER_CHAIN,            {SEX_FEMALE,    RACE_HUMAN,     true}},
    {MONSTER_TYPE_FIGHTER_LEATHER,          {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_FIGHTER_PLATE,            {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_GARGOYLE,                 {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_GENIE,                    {SEX_FEMALE,    RACE_HUMAN,     true}},
    {MONSTER_TYPE_GHOST,                    {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_GOBLIN,                   {SEX_MALE,      RACE_GOBLIN,    true}}, // Was RACE_HUMAN in original binary.
    {MONSTER_TYPE_GOG,                      {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_GOLEM,                    {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_GRIFFIN,                  {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_HARPY,                    {SEX_FEMALE,    RACE_HUMAN,     true}},
    {MONSTER_TYPE_HYDRA,                    {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_LICH,                     {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_MAGE,                     {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_MANTICORE,                {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_MEDUSA,                   {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_MINOTAUR,                 {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_MONK,                     {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_NECROMANCER,              {SEX_FEMALE,    RACE_HUMAN,     true}},
    {MONSTER_TYPE_OOZE,                     {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_PEASANT_DWARF_FEMALE_A,   {SEX_FEMALE,    RACE_DWARF,     false}},
    {MONSTER_TYPE_PEASANT_DWARF_FEMALE_B,   {SEX_FEMALE,    RACE_DWARF,     false}},
    {MONSTER_TYPE_PEASANT_DWARF_FEMALE_C,   {SEX_FEMALE,    RACE_DWARF,     false}},
    {MONSTER_TYPE_PEASANT_DWARF_MALE_A,     {SEX_MALE,      RACE_DWARF,     false}},
    {MONSTER_TYPE_PEASANT_DWARF_MALE_B,     {SEX_MALE,      RACE_DWARF,     false}},
    {MONSTER_TYPE_PEASANT_DWARF_MALE_C,     {SEX_MALE,      RACE_DWARF,     false}},
    {MONSTER_TYPE_PEASANT_ELF_FEMALE_A,     {SEX_FEMALE,    RACE_ELF,       false}},
    {MONSTER_TYPE_PEASANT_ELF_FEMALE_B,     {SEX_FEMALE,    RACE_ELF,       false}},
    {MONSTER_TYPE_PEASANT_ELF_FEMALE_C,     {SEX_FEMALE,    RACE_ELF,       false}},
    {MONSTER_TYPE_PEASANT_ELF_MALE_A,       {SEX_MALE,      RACE_ELF,       false}},
    {MONSTER_TYPE_PEASANT_ELF_MALE_B,       {SEX_MALE,      RACE_ELF,       false}},
    {MONSTER_TYPE_PEASANT_ELF_MALE_C,       {SEX_MALE,      RACE_ELF,       false}},
    {MONSTER_TYPE_PEASANT_HUMAN1_FEMALE_A,  {SEX_FEMALE,    RACE_HUMAN,     false}},
    {MONSTER_TYPE_PEASANT_HUMAN1_FEMALE_B,  {SEX_FEMALE,    RACE_HUMAN,     false}},
    {MONSTER_TYPE_PEASANT_HUMAN1_FEMALE_C,  {SEX_FEMALE,    RACE_HUMAN,     false}},
    {MONSTER_TYPE_PEASANT_HUMAN1_MALE_A,    {SEX_MALE,      RACE_HUMAN,     false}},
    {MONSTER_TYPE_PEASANT_HUMAN1_MALE_B,    {SEX_MALE,      RACE_HUMAN,     false}},
    {MONSTER_TYPE_PEASANT_HUMAN1_MALE_C,    {SEX_MALE,      RACE_HUMAN,     false}},
    {MONSTER_TYPE_PEASANT_HUMAN2_MALE_A,    {SEX_MALE,      RACE_HUMAN,     false}},
    {MONSTER_TYPE_PEASANT_HUMAN2_MALE_B,    {SEX_MALE,      RACE_HUMAN,     false}},
    {MONSTER_TYPE_PEASANT_HUMAN2_MALE_C,    {SEX_MALE,      RACE_HUMAN,     false}},
    {MONSTER_TYPE_PEASANT_HUMAN2_FEMALE_A,  {SEX_FEMALE,    RACE_HUMAN,     false}},
    {MONSTER_TYPE_PEASANT_HUMAN2_FEMALE_B,  {SEX_FEMALE,    RACE_HUMAN,     false}},
    {MONSTER_TYPE_PEASANT_HUMAN2_FEMALE_C,  {SEX_FEMALE,    RACE_HUMAN,     false}},
    {MONSTER_TYPE_RAT,                      {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_ROBOT,                    {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_ROC,                      {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_SEA_MONSTER,              {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_SKELETON_WARRIOR,         {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_SPIDER,                   {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_SWORDSMAN,                {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_THIEF,                    {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_TITAN,                    {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_TROGLODYTE,               {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_VAMPIRE,                  {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_WARLOCK,                  {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_WIGHT,                    {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_WYVERN,                   {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_ZOMBIE,                   {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_PEASANT_GOBLIN_FEMALE_A,  {SEX_FEMALE,    RACE_GOBLIN,    false}},
    {MONSTER_TYPE_PEASANT_GOBLIN_FEMALE_B,  {SEX_FEMALE,    RACE_GOBLIN,    false}},
    {MONSTER_TYPE_PEASANT_GOBLIN_FEMALE_C,  {SEX_FEMALE,    RACE_GOBLIN,    false}},
    {MONSTER_TYPE_PEASANT_GOBLIN_MALE_A,    {SEX_MALE,      RACE_GOBLIN,    false}},
    {MONSTER_TYPE_PEASANT_GOBLIN_MALE_B,    {SEX_MALE,      RACE_GOBLIN,    false}},
    {MONSTER_TYPE_PEASANT_GOBLIN_MALE_C,    {SEX_MALE,      RACE_GOBLIN,    false}},
    {MONSTER_TYPE_TROLL,                    {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_TREANT,                   {SEX_MALE,      RACE_HUMAN,     true}},
    {MONSTER_TYPE_GHOUL,                    {SEX_MALE,      RACE_HUMAN,     true}},

    // OE addition, these weren't in the original data tables:
    {MONSTER_TYPE_BLASTERGUY,               {SEX_MALE,      RACE_HUMAN,     false}},
    {MONSTER_TYPE_ULTRA_DRAGON,             {SEX_MALE,      RACE_HUMAN,     false}},
    {MONSTER_TYPE_UNUSED_CAT,               {SEX_MALE,      RACE_HUMAN,     false}},
    {MONSTER_TYPE_UNUSED_CHICKEN,           {SEX_MALE,      RACE_HUMAN,     false}},
    {MONSTER_TYPE_UNUSED_DOG,               {SEX_MALE,      RACE_HUMAN,     false}},
    {MONSTER_TYPE_UNUSED_RAT,               {SEX_MALE,      RACE_HUMAN,     false}},
};

static const std::vector<MonsterId> arenaMonsters = [] {
    // Original code in `prepareArenaFight` was iterating over all monsters and checking that:
    // - AI type is not MONSTER_AI_WIMP, effectively throwing out all peasants.
    // - Supertype is not MONSTER_SUPERTYPE_NOT_ARENA (which is not set for any of the MM7 monsters).
    // - Monster id is below MONSTER_BLASTERGUY_A, e.g. it's not a special monster.
    // And in this case the monster was considered a candidate for spawning in the arena.
    //
    // In OE we have simply added a flag into the monster data table above.
    std::vector<MonsterId> result;
    for (MonsterType monsterType : dataByMonsterType.indices())
        if (dataByMonsterType[monsterType].appearsInArena)
            for (MonsterId monsterId : monsterIdsForMonsterType(monsterType))
                result.push_back(monsterId);
    return result;
}();

std::span<const MonsterId> allArenaMonsters() {
    return arenaMonsters;
}

Sex sexForMonsterType(MonsterType monsterType) {
    return dataByMonsterType[monsterType].sex;
}

Race raceForMonsterType(MonsterType monsterType) {
    return dataByMonsterType[monsterType].race;
}

struct BountyHuntableMask : IndexedArray<bool, HOUSE_FIRST_TOWN_HALL, HOUSE_LAST_TOWN_HALL> {
    constexpr BountyHuntableMask() {
        fill(false);
    }

    constexpr BountyHuntableMask(std::initializer_list<HouseId> townHalls) {
        fill(false);
        for (const HouseId townHall : townHalls)
            (*this)[townHall] = true;
    }
};

// TODO(captainurist): Tbh the table still makes little sense. Why are Angels bounty-huntable in Celeste?
/**
 * Table of monster types that can be targeted in bounty hunts in each of the game's town halls.
 *
 * This is autogenerated code. It was, however, generated from code that was subsequently deleted, and then the data
 * here was edited. Feel free to edit.
 *
 * @see runBountyHuntCodeGen
 */
static constexpr IndexedArray<BountyHuntableMask, MONSTER_TYPE_FIRST, MONSTER_TYPE_LAST> bountyHuntableMaskByMonsterType = {
    {MONSTER_TYPE_ANGEL,                   {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_ARCHER,                  {HOUSE_TOWN_HALL_HARMONDALE,                          HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_BAT,                     {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_BEHEMOTH,                {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_BEHOLDER,                {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_CLERIC_MOON,             {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE                     }},
    {MONSTER_TYPE_CLERIC_SUN,              {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST,                          HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_DEVIL,                   {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_DRAGON,                  {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_DRAGONFLY,               {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA,                                  HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_DWARF,                   {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_ELEMENTAL_AIR,           {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_ELEMENTAL_EARTH,         {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_ELEMENTAL_FIRE,          {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_ELEMENTAL_LIGHT,         {                                                                                                                                  }},
    {MONSTER_TYPE_ELEMENTAL_WATER,         {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_ELF_ARCHER,              {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA,                                  HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_ELF_SPEARMAN,            {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA,                                  HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_FIGHTER_CHAIN,           {HOUSE_TOWN_HALL_HARMONDALE,                          HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_FIGHTER_LEATHER,         {HOUSE_TOWN_HALL_HARMONDALE,                          HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_FIGHTER_PLATE,           {HOUSE_TOWN_HALL_HARMONDALE,                          HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_GARGOYLE,                {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_GENIE,                   {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST,                          HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_GHOST,                   {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE                     }},
    {MONSTER_TYPE_GOBLIN,                  {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE                     }},
    {MONSTER_TYPE_GOG,                     {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_GOLEM,                   {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST,                          HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_GRIFFIN,                 {HOUSE_TOWN_HALL_HARMONDALE,                          HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_HARPY,                   {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_HYDRA,                   {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_LICH,                    {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE                     }},
    {MONSTER_TYPE_MAGE,                    {                            HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST,                          HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_MANTICORE,               {                                                                                                                                  }},
    {MONSTER_TYPE_MEDUSA,                  {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_MINOTAUR,                {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_MONK,                    {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST,                          HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_NECROMANCER,             {                            HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE                     }},
    {MONSTER_TYPE_OOZE,                    {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_PEASANT_DWARF_FEMALE_A,  {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_DWARF_FEMALE_B,  {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_DWARF_FEMALE_C,  {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_DWARF_MALE_A,    {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_DWARF_MALE_B,    {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_DWARF_MALE_C,    {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_ELF_FEMALE_A,    {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_ELF_FEMALE_B,    {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_ELF_FEMALE_C,    {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_ELF_MALE_A,      {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_ELF_MALE_B,      {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_ELF_MALE_C,      {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_HUMAN1_FEMALE_A, {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_HUMAN1_FEMALE_B, {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_HUMAN1_FEMALE_C, {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_HUMAN1_MALE_A,   {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_HUMAN1_MALE_B,   {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_HUMAN1_MALE_C,   {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_HUMAN2_MALE_A,   {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_HUMAN2_MALE_B,   {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_HUMAN2_MALE_C,   {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_HUMAN2_FEMALE_A, {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_HUMAN2_FEMALE_B, {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_HUMAN2_FEMALE_C, {                                                                                                                                  }},
    {MONSTER_TYPE_RAT,                     {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_ROBOT,                   {                                                                                                                                  }},
    {MONSTER_TYPE_ROC,                     {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST,                          HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_SEA_MONSTER,             {                                                                                                                                  }},
    {MONSTER_TYPE_SKELETON_WARRIOR,        {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE                     }},
    {MONSTER_TYPE_SPIDER,                  {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_SWORDSMAN,               {                                                     HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_THIEF,                   {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_TITAN,                   {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_TROGLODYTE,              {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_VAMPIRE,                 {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE                     }},
    {MONSTER_TYPE_WARLOCK,                 {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_WIGHT,                   {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE                     }},
    {MONSTER_TYPE_WYVERN,                  {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_ZOMBIE,                  {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE                     }},

    // OE fix: in original binary female goblin peasant could become a bounty hunt target in Harmondale.
    {MONSTER_TYPE_PEASANT_GOBLIN_FEMALE_A, {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_GOBLIN_FEMALE_B, {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_GOBLIN_FEMALE_C, {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_GOBLIN_MALE_A,   {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_GOBLIN_MALE_B,   {                                                                                                                                  }},
    {MONSTER_TYPE_PEASANT_GOBLIN_MALE_C,   {                                                                                                                                  }},
    {MONSTER_TYPE_TROLL,                   {                            HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE, HOUSE_TOWN_HALL_PIT}},
    {MONSTER_TYPE_TREANT,                  {                                                                                                                                  }},
    {MONSTER_TYPE_GHOUL,                   {HOUSE_TOWN_HALL_HARMONDALE, HOUSE_TOWN_HALL_ERATHIA, HOUSE_TOWN_HALL_TULAREAN_FOREST, HOUSE_TOWN_HALL_CELESTE                     }},
    {MONSTER_TYPE_BLASTERGUY,              {                                                                                                                                  }},
    {MONSTER_TYPE_ULTRA_DRAGON,            {                                                                                                                                  }},
    {MONSTER_TYPE_UNUSED_CAT,              {                                                                                                                                  }},
    {MONSTER_TYPE_UNUSED_CHICKEN,          {                                                                                                                                  }},
    {MONSTER_TYPE_UNUSED_DOG,              {                                                                                                                                  }},
    {MONSTER_TYPE_UNUSED_RAT,              {                                                                                                                                  }},
};

bool isBountyHuntable(MonsterType monsterType, HouseId townHall) {
    return bountyHuntableMaskByMonsterType[monsterType][townHall];
}

ItemId itemDropForMonsterType(MonsterType monsterType) {
    switch (monsterType) {
    case MONSTER_TYPE_HARPY: return ITEM_REAGENT_HARPY_FEATHER;
    case MONSTER_TYPE_OOZE: return ITEM_REAGENT_VIAL_OF_OOZE_ENDOPLASM;
    case MONSTER_TYPE_TROLL: return ITEM_REAGENT_VIAL_OF_TROLL_BLOOD;
    case MONSTER_TYPE_DEVIL: return ITEM_REAGENT_VIAL_OF_DEVIL_ICHOR;
    case MONSTER_TYPE_DRAGON: return ITEM_REAGENT_DRAGONS_EYE;
    default: return ITEM_NULL;
    }
}

MonsterSupertype supertypeForMonsterType(MonsterType monsterType) {
    switch (monsterType) {
    case MONSTER_TYPE_GHOST:
    case MONSTER_TYPE_LICH:
    case MONSTER_TYPE_SKELETON_WARRIOR:
    case MONSTER_TYPE_VAMPIRE:
    case MONSTER_TYPE_WIGHT:
    case MONSTER_TYPE_ZOMBIE:
    case MONSTER_TYPE_GHOUL:
        return MONSTER_SUPERTYPE_UNDEAD;
    case MONSTER_TYPE_DEVIL:
        return MONSTER_SUPERTYPE_KREEGAN;
    case MONSTER_TYPE_PEASANT_ELF_FEMALE_A:
    case MONSTER_TYPE_PEASANT_ELF_FEMALE_B:
    case MONSTER_TYPE_PEASANT_ELF_FEMALE_C:
    case MONSTER_TYPE_PEASANT_ELF_MALE_A:
    case MONSTER_TYPE_PEASANT_ELF_MALE_B:
    case MONSTER_TYPE_PEASANT_ELF_MALE_C:
    case MONSTER_TYPE_ELF_ARCHER:
    case MONSTER_TYPE_ELF_SPEARMAN:
        return MONSTER_SUPERTYPE_ELF;

    case MONSTER_TYPE_DRAGON:
    case MONSTER_TYPE_ULTRA_DRAGON: // OE addition: In vanilla "Dragon Slaying" items didn't do x2 damage vs Mega dragon.
        return MONSTER_SUPERTYPE_DRAGON;

    // TODO(captainurist): This needs some reworking it seems. Water elemental supertype is about water walking,
    //                     treant supertype is about being a tree that can't move. The rest are about "of X slaying".
    case MONSTER_TYPE_ELEMENTAL_WATER:
        return MONSTER_SUPERTYPE_WATER_ELEMENTAL;
    case MONSTER_TYPE_TREANT:
        return MONSTER_SUPERTYPE_TREANT;
    case MONSTER_TYPE_TITAN:
        return MONSTER_SUPERTYPE_TITAN;
    default:
        return MONSTER_SUPERTYPE_NONE;
    }
}

// MM8's `monsters.txt` (`data/EnglishT.lod`) keeps MM7's 3-tier-per-species
// chunking (ids 1..198, 66 species in a row), but completely reshuffles which species sits at which id, so
// `monsterTypeForMonsterId`'s MM7 row-position arithmetic doesn't carry over. This table gives the
// `MonsterType` only for the MM8 species that have an MM7 counterpart of the same name. Every other MM8
// species (e.g. Lizardman, Couatl, Ogre, Cyclops, Pirate - none of which exist as an MM7 `MonsterType`) is
// `MONSTER_TYPE_UNUSED_RAT`, an inert placeholder with no bounty/drop/arena behavior.
//
// This is a semantic classification only (item drop / sex / race / bounty-hunt / arena / peasant).
// Do NOT use it for hostility resolution - `hostile.txt`'s row/column position for a species
// is unrelated to its MM7 `MonsterType` ordinal. See `mm8HostilityMonsterTypeForMonsterId` below.
MonsterType mm8MonsterTypeForMonsterId(MonsterId monsterId) {
    int id = std::to_underlying(monsterId);
    if (id >= 46 && id <= 48) return MONSTER_TYPE_CLERIC_SUN;        // Acolyte/Cleric/Priest of the Sun
    if (id >= 52 && id <= 54) return MONSTER_TYPE_VAMPIRE;           // Vampire Minion/Vampire/Greater Vampire
    if (id >= 55 && id <= 57) return MONSTER_TYPE_NECROMANCER;       // Dark Path Journeyman/Necromancer/Master Necromancer
    if (id >= 58 && id <= 60) return MONSTER_TYPE_TROLL;             // Troll Clubman/Warrior/Chieftain
    if (id >= 64 && id <= 66) return MONSTER_TYPE_MINOTAUR;          // Minotaur Guard/Warrior/Battleleader
    if (id >= 70 && id <= 72) return MONSTER_TYPE_DRAGON;            // Dragon/Dragon Flightleader/Great Wyrm
    if (id >= 73 && id <= 75) return MONSTER_TYPE_ELEMENTAL_FIRE;    // Lesser/regular/Greater Fire Elemental
    if (id >= 76 && id <= 78) return MONSTER_TYPE_ELEMENTAL_WATER;   // Lesser/regular/Greater Water Elemental
    if (id >= 79 && id <= 81) return MONSTER_TYPE_ELEMENTAL_EARTH;   // Lesser/regular/Greater Earth Elemental
    if (id >= 82 && id <= 84) return MONSTER_TYPE_ELEMENTAL_AIR;     // Lesser/regular/Greater Air Elemental
    if (id >= 121 && id <= 123) return MONSTER_TYPE_WYVERN;          // Wyvern/Horned Wyvern/Ancient Wyvern
    if (id >= 160 && id <= 162) return MONSTER_TYPE_GOG;             // Smoke/Ember/Fire Gog
    return MONSTER_TYPE_UNUSED_RAT;
}

// MM8 `hostile.txt` (`data/EnglishT.lod`) column index per species, in its header row's order.
// This is the value `HostilityTable::Initialize` keys `relations[col][row]` by, and is
// independent of `mm8MonsterTypeForMonsterId`'s MM7-semantic value above - use this function ONLY for
// `Actor::hostilityGroup` / `HostilityTable::relations` lookups.
MonsterType mm8HostilityMonsterTypeForMonsterId(MonsterId monsterId) {
    // MM8's hostile.txt has 66 species rows in exactly the order of monsters.txt's 66 species
    // (3 tiers each, ids 1..198), so the row is (id-1)/3+1.
    int id = std::to_underlying(monsterId);
    if (id >= 1 && id <= 198)
        return static_cast<MonsterType>((id - 1) / 3 + 1);
    return MONSTER_TYPE_UNUSED_RAT;
}

// `spcitems.txt` ordinal 39 ("Ogre Slaying" under MM8, "Demon Slaying" under MM7) and ordinal 63
// ("Elemental Slaying" under MM8, "Elf Slaying" under MM7) need a classification of which MM8 monsters
// they double-damage against. From the same `monsters.txt` as `mm8MonsterTypeForMonsterId`:
// "Ogre Slaying" covers the Ogre Peasant/Warrior, Troll, Troll Peasant, Cyclops and Ogre Mage kinds;
// "Elemental Slaying" covers the four classic elementals plus Efreeti/Thunderbird/Salamander/Phoenix/
// Juggernaut (none of which contain "Elemental" in their name, so name-substring matching would miss them).
MonsterSupertype mm8SupertypeForMonsterId(MonsterId monsterId) {
    static constexpr std::pair<int, int> ogreKindIdRanges[] = {
        {28, 30},   // Ogre Peasant
        {31, 33},   // Ogre Brawler/Warrior/Warleader
        {58, 60},   // Troll Clubman/Warrior/Chieftain
        {61, 63},   // Troll Peasant
        {91, 93},   // Cyclops/Cyclops Hunter/Cyclops Warrior
        {106, 108}, // Ogre Mage Apprentice/Ogre Mage/Ogre Magi
    };
    static constexpr std::pair<int, int> elementalKindIdRanges[] = {
        {73, 75},   // Fire Elemental
        {76, 78},   // Water Elemental
        {79, 81},   // Earth Elemental
        {82, 84},   // Air Elemental
        {103, 105}, // Efreeti Soldier/Knight/Lord
        {115, 117}, // Thunderbird (Mist Hawk/Storm Eagle/Thunderbird)
        {127, 129}, // Salamander (Fire Newt/Fire Lizard/Salamander)
        {130, 132}, // Phoenix (Fire Bird/Phoenix/Grand Phoenix)
        {157, 159}, // Juggernaut (Charger/Juggernaut/Demolisher)
    };

    int id = std::to_underlying(monsterId);
    for (const auto &[first, last] : ogreKindIdRanges)
        if (id >= first && id <= last)
            return MONSTER_SUPERTYPE_MM8_OGRE;
    for (const auto &[first, last] : elementalKindIdRanges)
        if (id >= first && id <= last)
            return MONSTER_SUPERTYPE_MM8_ELEMENTAL;
    return MONSTER_SUPERTYPE_NONE;
}

// See the header comment on `isMm8PeasantMonsterId`. These are the id ranges of
// each species' "peasant" tier in MM8's `monsters.txt`. Human Peasant (166-168) is included here too even
// though it already resolves via `isPeasant(MonsterType)`.
bool isMm8PeasantMonsterId(MonsterId monsterId) {
    static constexpr std::pair<int, int> peasantIdRanges[] = {
        {1, 3},     // Lizardman Peasant/Farmer/Villager
        {19, 24},   // Dark Elf Peasant (Male A/B/C, Female A/B/C)
        {28, 30},   // Ogre Peasant
        {49, 51},   // Peasant (Vampire Female Peasant)
        {61, 63},   // Troll Peasant
        {67, 69},   // Minotaur Peasant
        {166, 168}, // Human Peasant
    };

    int id = std::to_underlying(monsterId);
    for (const auto &[first, last] : peasantIdRanges)
        if (id >= first && id <= last)
            return true;
    return false;
}

std::span<const MonsterAttackPreference> allMonsterAttackPreferences() {
    static constexpr std::initializer_list<MonsterAttackPreference> result = {
        ATTACK_PREFERENCE_KNIGHT,
        ATTACK_PREFERENCE_PALADIN,
        ATTACK_PREFERENCE_ARCHER,
        ATTACK_PREFERENCE_DRUID,
        ATTACK_PREFERENCE_CLERIC,
        ATTACK_PREFERENCE_SORCERER,
        ATTACK_PREFERENCE_RANGER,
        ATTACK_PREFERENCE_THIEF,
        ATTACK_PREFERENCE_MONK,
        ATTACK_PREFERENCE_MALE,
        ATTACK_PREFERENCE_FEMALE,
        ATTACK_PREFERENCE_HUMAN,
        ATTACK_PREFERENCE_ELF,
        ATTACK_PREFERENCE_DWARF,
        ATTACK_PREFERENCE_GOBLIN
    };

    return result;
}

SpriteId spriteForMonsterProjectile(MonsterProjectile projectile) {
    switch (projectile) {
    default:                                assert(false); [[fallthrough]];
    case MONSTER_PROJECTILE_NONE:           return SPRITE_NULL;
    case MONSTER_PROJECTILE_ARROW:          return SPRITE_PROJECTILE_ARROW;
    case MONSTER_PROJECTILE_FLAMING_ARROW:  return SPRITE_PROJECTILE_FLAMING_ARROW;
    case MONSTER_PROJECTILE_FIRE_BOLT:      return SPRITE_PROJECTILE_FIRE_BOLT;
    case MONSTER_PROJECTILE_AIR_BOLT:       return SPRITE_PROJECTILE_AIR_BOLT;
    case MONSTER_PROJECTILE_WATER_BOLT:     return SPRITE_PROJECTILE_WATER_BOLT;
    case MONSTER_PROJECTILE_EARTH_BOLT:     return SPRITE_PROJECTILE_EARTH_BOLT;
    case MONSTER_PROJECTILE_SPIRIT_BOLT:    return SPRITE_PROJECTILE_SPIRIT_BOLT;
    case MONSTER_PROJECTILE_MIND_BOLT:      return SPRITE_PROJECTILE_MIND_BOLT;
    case MONSTER_PROJECTILE_BODY_BOLT:      return SPRITE_PROJECTILE_BODY_BOLT;
    case MONSTER_PROJECTILE_LIGHT_BOLT:     return SPRITE_PROJECTILE_LIGHT_BOLT;
    case MONSTER_PROJECTILE_DARK_BOLT:      return SPRITE_PROJECTILE_DARK_BOLT;
    case MONSTER_PROJECTILE_ENERGY_BOLT:    return SPRITE_PROJECTILE_BLASTER;
    }
}
