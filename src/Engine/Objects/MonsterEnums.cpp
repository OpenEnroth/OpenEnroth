#include "MonsterEnums.h"

#include "Utility/IndexedArray.h"

struct SexAndRace {
    CharacterSex sex = SEX_MALE;
    Race race = RACE_HUMAN;

    constexpr SexAndRace() = default;
    constexpr SexAndRace(CharacterSex sex, Race race) : sex(sex), race(race) {}
};

// TODO(captainurist): a bit weird that all the monsters belong to RACE_HUMAN.
static constexpr IndexedArray<SexAndRace, MONSTER_TYPE_FIRST, MONSTER_TYPE_LAST> sexAndRaceByMonsterType = {
    {MONSTER_TYPE_ANGEL,                    {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_ARCHER,                   {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_BAT,                      {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_BEHEMOTH,                 {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_BEHOLDER,                 {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_CLERIC_MOON,              {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_CLERIC_SUN,               {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_DEVIL,                    {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_DRAGON,                   {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_DRAGONFLY,                {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_DWARF,                    {SEX_MALE,      RACE_DWARF}},
    {MONSTER_TYPE_ELEMENTAL_AIR,            {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_ELEMENTAL_EARTH,          {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_ELEMENTAL_FIRE,           {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_ELEMENTAL_LIGHT,          {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_ELEMENTAL_WATER,          {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_ELF_ARCHER,               {SEX_MALE,      RACE_ELF}},
    {MONSTER_TYPE_ELF_SPEARMAN,             {SEX_MALE,      RACE_ELF}},
    {MONSTER_TYPE_FIGHTER_CHAIN,            {SEX_FEMALE,    RACE_HUMAN}},
    {MONSTER_TYPE_FIGHTER_LEATHER,          {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_FIGHTER_PLATE,            {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_GARGOYLE,                 {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_GENIE,                    {SEX_FEMALE,    RACE_HUMAN}},
    {MONSTER_TYPE_GHOST,                    {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_GOBLIN,                   {SEX_MALE,      RACE_GOBLIN}}, // Was RACE_HUMAN in original binary.
    {MONSTER_TYPE_GOG,                      {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_GOLEM,                    {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_GRIFFIN,                  {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_HARPY,                    {SEX_FEMALE,    RACE_HUMAN}},
    {MONSTER_TYPE_HYDRA,                    {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_LICH,                     {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_MAGE,                     {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_MANTICORE,                {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_MEDUSA,                   {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_MINOTAUR,                 {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_MONK,                     {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_NECROMANCER,              {SEX_FEMALE,    RACE_HUMAN}},
    {MONSTER_TYPE_OOZE,                     {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_PEASANT_DWARF_FEMALE_A,   {SEX_FEMALE,    RACE_DWARF}},
    {MONSTER_TYPE_PEASANT_DWARF_FEMALE_B,   {SEX_FEMALE,    RACE_DWARF}},
    {MONSTER_TYPE_PEASANT_DWARF_FEMALE_C,   {SEX_FEMALE,    RACE_DWARF}},
    {MONSTER_TYPE_PEASANT_DWARF_MALE_A,     {SEX_MALE,      RACE_DWARF}},
    {MONSTER_TYPE_PEASANT_DWARF_MALE_B,     {SEX_MALE,      RACE_DWARF}},
    {MONSTER_TYPE_PEASANT_DWARF_MALE_C,     {SEX_MALE,      RACE_DWARF}},
    {MONSTER_TYPE_PEASANT_ELF_FEMALE_A,     {SEX_FEMALE,    RACE_ELF}},
    {MONSTER_TYPE_PEASANT_ELF_FEMALE_B,     {SEX_FEMALE,    RACE_ELF}},
    {MONSTER_TYPE_PEASANT_ELF_FEMALE_C,     {SEX_FEMALE,    RACE_ELF}},
    {MONSTER_TYPE_PEASANT_ELF_MALE_A,       {SEX_MALE,      RACE_ELF}},
    {MONSTER_TYPE_PEASANT_ELF_MALE_B,       {SEX_MALE,      RACE_ELF}},
    {MONSTER_TYPE_PEASANT_ELF_MALE_C,       {SEX_MALE,      RACE_ELF}},
    {MONSTER_TYPE_PEASANT_HUMAN1_FEMALE_A,  {SEX_FEMALE,    RACE_HUMAN}},
    {MONSTER_TYPE_PEASANT_HUMAN1_FEMALE_B,  {SEX_FEMALE,    RACE_HUMAN}},
    {MONSTER_TYPE_PEASANT_HUMAN1_FEMALE_C,  {SEX_FEMALE,    RACE_HUMAN}},
    {MONSTER_TYPE_PEASANT_HUMAN1_MALE_A,    {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_PEASANT_HUMAN1_MALE_B,    {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_PEASANT_HUMAN1_MALE_C,    {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_PEASANT_HUMAN2_MALE_A,    {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_PEASANT_HUMAN2_MALE_B,    {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_PEASANT_HUMAN2_MALE_C,    {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_PEASANT_HUMAN2_FEMALE_A,  {SEX_FEMALE,    RACE_HUMAN}},
    {MONSTER_TYPE_PEASANT_HUMAN2_FEMALE_B,  {SEX_FEMALE,    RACE_HUMAN}},
    {MONSTER_TYPE_PEASANT_HUMAN2_FEMALE_C,  {SEX_FEMALE,    RACE_HUMAN}},
    {MONSTER_TYPE_RAT,                      {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_ROBOT,                    {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_ROC,                      {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_SEA_MONSTER,              {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_SKELETON_WARRIOR,         {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_SPIDER,                   {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_SWORDSMAN,                {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_THIEF,                    {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_TITAN,                    {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_TROGLODYTE,               {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_VAMPIRE,                  {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_WARLOCK,                  {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_WIGHT,                    {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_WYVERN,                   {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_ZOMBIE,                   {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_PEASANT_GOBLIN_FEMALE_A,  {SEX_FEMALE,    RACE_GOBLIN}},
    {MONSTER_TYPE_PEASANT_GOBLIN_FEMALE_B,  {SEX_FEMALE,    RACE_GOBLIN}},
    {MONSTER_TYPE_PEASANT_GOBLIN_FEMALE_C,  {SEX_FEMALE,    RACE_GOBLIN}},
    {MONSTER_TYPE_PEASANT_GOBLIN_MALE_A,    {SEX_MALE,      RACE_GOBLIN}},
    {MONSTER_TYPE_PEASANT_GOBLIN_MALE_B,    {SEX_MALE,      RACE_GOBLIN}},
    {MONSTER_TYPE_PEASANT_GOBLIN_MALE_C,    {SEX_MALE,      RACE_GOBLIN}},
    {MONSTER_TYPE_TROLL,                    {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_TREANT,                   {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_GHOUL,                    {SEX_MALE,      RACE_HUMAN}},

    // OE addition, these weren't in the original data tables:
    {MONSTER_TYPE_BLASTERGUY,               {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_ULTRA_DRAGON,             {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_UNUSED_CAT,               {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_UNUSED_CHICKEN,           {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_UNUSED_DOG,               {SEX_MALE,      RACE_HUMAN}},
    {MONSTER_TYPE_UNUSED_RAT,               {SEX_MALE,      RACE_HUMAN}},
};

CharacterSex sexForMonsterType(MonsterType monsterType) {
    return sexAndRaceByMonsterType[monsterType].sex;
}

Race raceForMonsterType(MonsterType monsterType) {
    return sexAndRaceByMonsterType[monsterType].race;
}
