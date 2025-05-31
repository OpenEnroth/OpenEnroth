#include "MonsterEnumFunctions.h"

#include <string>
#include <utility>
#include <vector>

struct MonsterData {
    CharacterSex sex = SEX_MALE;
    Race race = RACE_HUMAN;
    bool appearsInArena = false;

    constexpr MonsterData() = default;
    constexpr MonsterData(CharacterSex sex, Race race, bool appearsInArena) : sex(sex), race(race), appearsInArena(appearsInArena) {}
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

CharacterSex sexForMonsterType(MonsterType monsterType) {
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

    // TODO(captainurist): should also include mega-dragon?
    case MONSTER_TYPE_DRAGON:
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
