#include "Monsters.h"

#include <algorithm>
#include <array>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "Library/Logger/Logger.h"
#include "Library/Serialization/Serialization.h"

#include "Utility/MapAccess.h"
#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Split.h"
#include "Utility/Exception.h"
#include "Utility/String/Transformations.h"

MonsterStats *pMonsterStats;
MonsterList *pMonsterList;

void ParseDamage(std::string_view damage_str, uint8_t *dice_rolls,
                 uint8_t *dice_sides, uint8_t *dmg_bonus);
MonsterProjectile ParseMissleAttackType(std::string_view missle_attack_str);
MonsterSpecialAttack ParseSpecialAttack(std::string_view spec_att_str);

//----- (004548E2) --------------------------------------------------------
SpellId ParseSpellType(std::string_view name) {
    static const std::map<std::string, SpellId, ascii::NoCaseLess> monsterSpellMap = {
        {"Acid Burst",        SPELL_WATER_ACID_BURST},
        {"Blades",             SPELL_EARTH_BLADES},
        {"Bless",              SPELL_SPIRIT_BLESS},
        {"Day of Protection",  SPELL_LIGHT_DAY_OF_PROTECTION},
        {"Dispel Magic",       SPELL_LIGHT_DISPEL_MAGIC},
        {"Dragon Breath",      SPELL_DARK_DRAGON_BREATH},
        {"Fate",               SPELL_SPIRIT_FATE},
        {"Fire Bolt",          SPELL_FIRE_FIRE_BOLT},
        {"Fireball",           SPELL_FIRE_FIREBALL},
        {"Hammerhands",        SPELL_BODY_HAMMERHANDS},
        {"Harm",               SPELL_BODY_HARM},
        {"Haste",              SPELL_FIRE_HASTE},
        {"Heroism",            SPELL_SPIRIT_HEROISM},
        {"Hour of Power",      SPELL_LIGHT_HOUR_OF_POWER},
        {"Ice Blast",          SPELL_WATER_ICE_BLAST},
        {"Ice Bolt",           SPELL_WATER_ICE_BOLT},
        {"Incinerate",         SPELL_FIRE_INCINERATE},
        {"Light Bolt",         SPELL_LIGHT_LIGHT_BOLT},
        {"Lightning Bolt",     SPELL_AIR_LIGHTNING_BOLT},
        {"Meteor Shower",      SPELL_FIRE_METEOR_SHOWER},
        {"Mind Blast",         SPELL_MIND_MIND_BLAST},
        {"Pain Reflection",    SPELL_DARK_PAIN_REFLECTION},
        {"Power Cure",         SPELL_BODY_POWER_CURE},
        {"Psychic Shock",      SPELL_MIND_PSYCHIC_SHOCK},
        {"Rock Blast",         SPELL_EARTH_ROCK_BLAST},
        {"Shield",             SPELL_AIR_SHIELD},
        {"Shrapmetal",         SPELL_DARK_SHARPMETAL},
        {"Sparks",             SPELL_AIR_SPARKS},
        {"Stone Skin",         SPELL_EARTH_STONESKIN},
        {"Toxic Cloud",        SPELL_DARK_TOXIC_CLOUD},
    };

    auto it = monsterSpellMap.find(name);
    if (it != monsterSpellMap.end())
        return it->second;
    logger->warning("Unknown monster spell {}", name);
    return SPELL_NONE;
}

CombinedSkillValue ParseSkillValue(std::string_view skillString, std::string_view masteryString) {
    int skill;
    if (!tryDeserialize(skillString, &skill))
        return CombinedSkillValue::none(); // TODO(captainurist): this does happen, investigate.

    Mastery mastery;
    if (masteryString == "N") {
        mastery = MASTERY_NOVICE;
    } else if (masteryString == "E") {
        mastery = MASTERY_EXPERT;
    } else if (masteryString == "M") {
        mastery = MASTERY_MASTER;
    } else if (masteryString == "G") {
        mastery = MASTERY_GRANDMASTER;
    } else {
        throw Exception("Invalid character skill mastery string '{}'", masteryString);
    }

    return CombinedSkillValue(skill, mastery);
}

//----- (00454CB4) --------------------------------------------------------
static DamageType ParseAttackType(std::string_view damage_type_str) {
    // Match the abbreviated names used in monsters.txt (e.g. "Phys", "Ener").
    static const std::map<std::string, DamageType, ascii::NoCaseLess> damageTypeMap = {
        {"Fire",   DAMAGE_FIRE},
        {"Air",    DAMAGE_AIR},
        {"Water",  DAMAGE_WATER},
        {"Earth",  DAMAGE_EARTH},
        {"Phys",   DAMAGE_PHYSICAL},
        {"Spirit", DAMAGE_SPIRIT},
        {"Mind",   DAMAGE_MIND},
        {"Body",   DAMAGE_BODY},
        {"Light",  DAMAGE_LIGHT},
        {"Dark",   DAMAGE_DARK},
        // TODO(captainurist): "Ener" should map to DAMAGE_ENERGY but the original parser only inspected the
        // first letter, so it collided with "Earth". Preserving the buggy mapping so existing trace tests
        // still pass; fix-and-retrace separately.
        {"Ener",   DAMAGE_EARTH},
    };
    return valueOr(damageTypeMap, damage_type_str, DAMAGE_PHYSICAL);
}

//----- (00454D7D) --------------------------------------------------------
// Damage cell format: "<rolls>D<sides>[+<bonus>]" or just "<value>" (treated as rolls=value, sides=1).
void ParseDamage(std::string_view damage_str, uint8_t *dice_rolls,
                 uint8_t *dice_sides, uint8_t *dmg_bonus) {
    *dice_rolls = 0;
    *dice_sides = 1;
    *dmg_bonus = 0;
    if (damage_str.empty())
        return;

    // Split off optional "+<bonus>" tail.
    std::string_view head = damage_str;
    if (size_t plus = damage_str.find('+'); plus != std::string_view::npos) {
        head = damage_str.substr(0, plus);
        std::string_view bonusPart = damage_str.substr(plus + 1);
        if (!bonusPart.empty())
            *dmg_bonus = fromString<int>(bonusPart);
    }

    // Split head on 'D' (case-insensitive). With 'D' we have rolls D sides; without it, head is just rolls.
    if (size_t d = std::min(head.find('d'), head.find('D')); d != std::string_view::npos) {
        if (d > 0)
            *dice_rolls = fromString<int>(head.substr(0, d));
        std::string_view sidesPart = head.substr(d + 1);
        if (!sidesPart.empty())
            *dice_sides = fromString<int>(sidesPart);
    } else {
        *dice_rolls = fromString<int>(head);
    }
}

//----- (00454E3A) --------------------------------------------------------
MonsterProjectile ParseMissleAttackType(std::string_view missle_attack_str) {
    // TODO(captainurist): this is broken, we get "FireAr" for flaming arrow here.

    if (ascii::noCaseEquals(missle_attack_str, "ARROW"))
        return MONSTER_PROJECTILE_ARROW;
    else if (ascii::noCaseEquals(missle_attack_str, "ARROWF"))
        return MONSTER_PROJECTILE_FLAMING_ARROW;
    else if (ascii::noCaseEquals(missle_attack_str, "FIRE"))
        return MONSTER_PROJECTILE_FIRE_BOLT;
    else if (ascii::noCaseEquals(missle_attack_str, "AIR"))
        return MONSTER_PROJECTILE_AIR_BOLT;
    else if (ascii::noCaseEquals(missle_attack_str, "WATER"))
        return MONSTER_PROJECTILE_WATER_BOLT;
    else if (ascii::noCaseEquals(missle_attack_str, "EARTH"))
        return MONSTER_PROJECTILE_EARTH_BOLT;
    else if (ascii::noCaseEquals(missle_attack_str, "SPIRIT"))
        return MONSTER_PROJECTILE_SPIRIT_BOLT;
    else if (ascii::noCaseEquals(missle_attack_str, "MIND"))
        return MONSTER_PROJECTILE_MIND_BOLT;
    else if (ascii::noCaseEquals(missle_attack_str, "BODY"))
        return MONSTER_PROJECTILE_BODY_BOLT;
    else if (ascii::noCaseEquals(missle_attack_str, "LIGHT"))
        return MONSTER_PROJECTILE_LIGHT_BOLT;
    else if (ascii::noCaseEquals(missle_attack_str, "DARK"))
        return MONSTER_PROJECTILE_DARK_BOLT;
    else if (ascii::noCaseEquals(missle_attack_str, "ENER"))
        return MONSTER_PROJECTILE_ENERGY_BOLT;
    else
        return MONSTER_PROJECTILE_NONE;
}

MonsterSpecialAttack ParseSpecialAttack(std::string_view spec_att_str) {
    std::string tmp = ascii::toLower(spec_att_str);

    if (tmp.starts_with("curse"))
        return SPECIAL_ATTACK_CURSE;
    else if (tmp.starts_with("weak"))
        return SPECIAL_ATTACK_WEAK;
    else if (tmp.starts_with("asleep"))
        return SPECIAL_ATTACK_SLEEP;
    else if (tmp.starts_with("afraid"))
        return SPECIAL_ATTACK_FEAR;
    else if (tmp.starts_with("drunk"))
        return SPECIAL_ATTACK_DRUNK;
    else if (tmp.starts_with("insane"))
        return SPECIAL_ATTACK_INSANE;
    else if (tmp.starts_with("poison weak") || tmp.starts_with("poison1"))
        return SPECIAL_ATTACK_POISON_WEAK;
    else if (tmp.starts_with("poison medium") || tmp.starts_with("poison2"))
        return SPECIAL_ATTACK_POISON_MEDIUM;
    else if (tmp.starts_with("poison severe") || tmp.starts_with("poison3"))
        return SPECIAL_ATTACK_POISON_SEVERE;
    else if (tmp.starts_with("disease weak") || tmp.starts_with("disease1"))
        return SPECIAL_ATTACK_DISEASE_WEAK;
    else if (tmp.starts_with("disease medium") || tmp.starts_with("disease2"))
        return SPECIAL_ATTACK_DISEASE_MEDIUM;
    else if (tmp.starts_with("disease severe") || tmp.starts_with("disease3"))
        return SPECIAL_ATTACK_DISEASE_SEVERE;
    else if (tmp.starts_with("paralyze"))
        return SPECIAL_ATTACK_PARALYZED;
    else if (tmp.starts_with("uncon"))
        return SPECIAL_ATTACK_UNCONSCIOUS;
    else if (tmp.starts_with("dead"))
        return SPECIAL_ATTACK_DEAD;
    else if (tmp.starts_with("stone"))
        return SPECIAL_ATTACK_PETRIFIED;
    else if (tmp.starts_with("errad"))
        return SPECIAL_ATTACK_ERADICATED;
    else if (tmp.starts_with("brkitem"))
        return SPECIAL_ATTACK_BREAK_ANY;
    else if (tmp.starts_with("brkarmor"))
        return SPECIAL_ATTACK_BREAK_ARMOR;
    else if (tmp.starts_with("brkweapon"))
        return SPECIAL_ATTACK_BREAK_WEAPON;
    else if (tmp.starts_with("steal"))
        return SPECIAL_ATTACK_STEAL;
    else if (tmp.starts_with("age"))
        return SPECIAL_ATTACK_AGING;
    else if (tmp.starts_with("drainsp"))
        return SPECIAL_ATTACK_MANA_DRAIN;
    else if (tmp.starts_with("none") || tmp.starts_with("0"))
        return SPECIAL_ATTACK_NONE;
    else
        logger->warning("ParseSpecialAttack:: Unknown monster special attack '{}'", tmp);

    return SPECIAL_ATTACK_NONE;
}

//----- (004563FF) --------------------------------------------------------
MonsterId MonsterStats::FindMonsterByInternalName(std::string_view internalName) {
    for (MonsterId i : infos.indices()) {
        if (!infos[i].name.empty() && ascii::noCaseEquals(infos[i].internalName, internalName))
            return i;
    }
    return MONSTER_INVALID;
}

//----- (00454F4E) --------------------------------------------------------
void MonsterStats::InitializePlacements(std::string_view placements) {
    // placemon.txt table structure: index | name (localized).
    for (std::string_view line : split(placements).by("\r\n").drop(1).skip("")) {
        std::array<std::string_view, 2> tokens = split(line).by('\t');
        int i = fromString<int>(tokens[0]);
        uniqueNames[i] = unquote(tokens[1]);
    }
}

// TODO(captainurist): move to MonsterTable?
//----- (0045501E) --------------------------------------------------------
void MonsterStats::Initialize(std::string_view monsters) {
    // monsters.txt table structure: id | name (localized) | internal name | level | hp | ac | exp | treasure |
    //                               blood splat | flying | movement | ai | hostility | speed | recovery |
    //                               attack prefs | special attack | a1 type | a1 dmg | a1 missile |
    //                               a2 chance | a2 type | a2 dmg | a2 missile |
    //                               s1 chance | s1 (skill+spell) | s2 chance | s2 (skill+spell) |
    //                               res fire/air/water/earth/mind/spirit/body/light/dark/physical |
    //                               special ability.
    static const std::map<std::string, MonsterMovementType, ascii::NoCaseLess> movementMap = {
        {"Short",  MONSTER_MOVEMENT_TYPE_SHORT},
        {"stand",  MONSTER_MOVEMENT_TYPE_STATIONARY},
        {"Long",   MONSTER_MOVEMENT_TYPE_LONG},
        {"Med",    MONSTER_MOVEMENT_TYPE_MEDIUM},
        {"Global", MONSTER_MOVEMENT_TYPE_GLOBAL},
        {"Free",   MONSTER_MOVEMENT_TYPE_FREE},
    };

    static const std::map<std::string, MonsterAiType, ascii::NoCaseLess> aiMap = {
        {"Suicidal", MONSTER_AI_SUICIDE},
        {"Wimp",     MONSTER_AI_WIMP},
        {"Normal",   MONSTER_AI_NORMAL},
        {"Aggress",  MONSTER_AI_AGGRESSIVE},
    };

    static const std::map<char, MonsterAttackPreference> attackPrefMap = {
        {'a', ATTACK_PREFERENCE_ARCHER},
        {'c', ATTACK_PREFERENCE_CLERIC},
        {'d', ATTACK_PREFERENCE_DRUID},
        {'k', ATTACK_PREFERENCE_KNIGHT},
        {'m', ATTACK_PREFERENCE_MONK},
        {'p', ATTACK_PREFERENCE_PALADIN},
        {'r', ATTACK_PREFERENCE_RANGER},
        {'s', ATTACK_PREFERENCE_SORCERER},
        {'t', ATTACK_PREFERENCE_THIEF},

        {'f', ATTACK_PREFERENCE_FEMALE}, // MM6 data uses 'f'.
        {'o', ATTACK_PREFERENCE_FEMALE}, // MM7 data uses 'o'.
        {'x', ATTACK_PREFERENCE_MALE},

        {'h', ATTACK_PREFERENCE_HUMAN},
        {'e', ATTACK_PREFERENCE_ELF},
        {'w', ATTACK_PREFERENCE_DWARF},
        {'g', ATTACK_PREFERENCE_GOBLIN},
    };

    static const std::map<std::string, RandomItemType, ascii::NoCaseLess> itemTypeMap = {
        {"WEAPON",    RANDOM_ITEM_WEAPON},
        {"ARMOR",     RANDOM_ITEM_ARMOR},
        {"MISC",      RANDOM_ITEM_MICS},
        {"SWORD",     RANDOM_ITEM_SWORD},
        {"DAGGER",    RANDOM_ITEM_DAGGER},
        {"AXE",       RANDOM_ITEM_AXE},
        {"SPEAR",     RANDOM_ITEM_SPEAR},
        {"BOW",       RANDOM_ITEM_BOW},
        {"MACE",      RANDOM_ITEM_MACE},
        {"CLUB",      RANDOM_ITEM_CLUB},
        {"STAFF",     RANDOM_ITEM_STAFF},
        {"LEATHER",   RANDOM_ITEM_LEATHER_ARMOR},
        {"CHAIN",     RANDOM_ITEM_CHAIN_ARMOR},
        {"PLATE",     RANDOM_ITEM_PLATE_ARMOR},
        {"SHIELD",    RANDOM_ITEM_SHIELD},
        {"HELM",      RANDOM_ITEM_HELMET},
        {"BELT",      RANDOM_ITEM_BELT},
        {"CAPE",      RANDOM_ITEM_CLOAK},
        {"GAUNTLETS", RANDOM_ITEM_GAUNTLETS},
        {"BOOTS",     RANDOM_ITEM_BOOTS},
        {"RING",      RANDOM_ITEM_RING},
        {"AMULET",    RANDOM_ITEM_AMULET},
        {"WAND",      RANDOM_ITEM_WAND},
        {"SCROLL",    RANDOM_ITEM_SPELL_SCROLL},
        {"GEM",       RANDOM_ITEM_GEM},
    };

    int i = 0;

    // "Imm" in any of the resistance columns means full immunity, encoded as 200.
    auto parseResistance = [](std::string_view s) -> uint8_t {
        return ascii::noCaseEquals(s, "Imm") ? 200 : fromString<int>(s);
    };

    auto parseYesNo = [](std::string_view s) -> bool {
        if (ascii::noCaseEquals(s, "Y"))
            return true;
        if (ascii::noCaseEquals(s, "N"))
            return false;
        throw Exception("Expected 'Y' or 'N', got '{}'", s);
    };

    // Parse a string of single-char flags. Letters set ATTACK_PREFERENCE_* class flags, digits 2-4 set
    // numCharactersAttackedPerSpecialAbility.
    auto parseAttackPrefs = [](std::string_view cell, MonsterInfo &info) {
        info.attackPreferences = 0;
        info.numCharactersAttackedPerSpecialAbility = 0;
        for (char c : cell) {
            char lc = ascii::toLower(c);
            if (lc >= '2' && lc <= '4')
                info.numCharactersAttackedPerSpecialAbility = lc - '0';
            else if (auto it = attackPrefMap.find(lc); it != attackPrefMap.end())
                info.attackPreferences |= it->second;
        }
    };

    // Special-attack cell: "<name>[x<level>]", default level 1, e.g. "BrkArmor", "Diseasex2".
    auto parseSpecialAttack = [](std::string_view cell, MonsterInfo &info) {
        info.specialAttackLevel = 1;
        info.specialAttackType = SPECIAL_ATTACK_NONE;
        if (cell.size() <= 1)
            return;
        std::string_view name = cell;
        size_t x = 0;
        while (x < name.size() && ascii::toLower(name[x]) != 'x') x++;
        if (x < name.size()) {
            info.specialAttackLevel = fromString<int>(name.substr(x + 1));
            name = name.substr(0, x);
        }
        info.specialAttackType = ParseSpecialAttack(name);
    };

    // Spell cells. Format: "<spell name>,<mastery>[,]<skill>".
    // Spell names may contain spaces. Efreet's spell cell is "Lightning Bolt,M10", so the second comma is optional.
    auto parseSpellEntry = [](std::string_view cell, SpellId &outSpellId, CombinedSkillValue &outMastery) {
        outSpellId = SPELL_NONE;
        outMastery = CombinedSkillValue::none();
        if (cell.size() < 2)
            return;
        std::string unquoted = unquote(cell);
        std::array<std::string_view, 3> parts = split(unquoted).by(',');
        if (parts[0].empty())
            return;
        outSpellId = ParseSpellType(parts[0]);
        std::string_view mastery = parts[1];
        std::string_view skill = parts[2];
        if (skill.empty() && mastery.size() > 1) {
            skill = mastery.substr(1);
            mastery = mastery.substr(0, 1);
        }
        if (!skill.empty())
            outMastery = ParseSkillValue(skill, mastery);
    };

    // Special-ability cell. Format is one of:
    //   "Shot Nx"            - multi-shot, N extra shots.
    //   "Summon <where> <name> [a|b|c|...]" - summon a monster, group letter at end picks monster A/B/C subtype.
    //   "Explode <dmg>"      - explode-on-death, damage parsed via ParseDamage.
    auto parseSpecialAbility = [](std::string_view cell, MonsterInfo &info) {
        info.specialAbilityType = MONSTER_SPECIAL_ABILITY_NONE;
        info.specialAbilityDamageDiceBonus = 0;
        std::string normalized(unquote(cell));
        for (char &c : normalized) {
            if (c == ',' || c == '\t') c = ' ';
        }
        std::vector<std::string_view> props;
        split(std::string_view(normalized)).by(' ').skip("").to(&props);
        if (props.empty() || props.size() >= 10)
            return;

        if (ascii::noCaseEquals(props[0], "shot")) {
            info.specialAbilityType = MONSTER_SPECIAL_ABILITY_MULTI_SHOT;
            // props[1] is "xN" (e.g. "x2", "x3"), skip the leading 'x'.
            info.specialAbilityDamageDiceBonus = fromString<int>(props[1].substr(1));
        } else if (ascii::noCaseEquals(props[0], "summon")) {
            info.specialAbilityType = MONSTER_SPECIAL_ABILITY_SUMMON;
            // Format: "Summon <where> <name...> [a|b|c]"
            assert(props.size() >= 4 && "Malformed summon ability");
            std::string summonName(props[2]);
            for (size_t k = 3; k < props.size(); ++k) {
                summonName += " ";
                summonName += std::string(props[k]);
            }
            switch (ascii::toLower(props.back()[0])) {
                case 'a': info.specialAbilityDamageDiceRolls = 1; break;
                case 'b': info.specialAbilityDamageDiceRolls = 2; break;
                case 'c': info.specialAbilityDamageDiceRolls = 3; break;
                default:  info.specialAbilityDamageDiceRolls = 0;
            }
            if (!pMonsterList->monsters.empty())
                info.field_3C_some_special_attack = std::to_underlying(GetBaseMonsterId(pMonsterList->GetMonsterIDByName(summonName)));
            info.specialAbilityDamageDiceSides = ascii::noCaseEquals(props[1], "ground") ? 1 : 0; // TODO(captainurist): do a honest parse of air/ground.
            if (info.field_3C_some_special_attack == -1)
                info.specialAbilityType = MONSTER_SPECIAL_ABILITY_NONE;
        } else if (ascii::noCaseEquals(props[0], "explode")) {
            info.specialAbilityType = MONSTER_SPECIAL_ABILITY_EXPLODE;
            ParseDamage(
                props[1],
                &info.specialAbilityDamageDiceRolls,
                &info.specialAbilityDamageDiceSides,
                &info.specialAbilityDamageDiceBonus);
            info.field_3C_some_special_attack = std::to_underlying(ParseAttackType(props[0])); // TODO(captainurist): makes no sense.
        }
    };

    // Treasure cell format: "[NN%][MdS][+][Llvl[ItemType]]"
    // - NN%       — drop chance (0..100). If dice or level are present without %, chance defaults to 100.
    // - MdS       — gold roll: M dice of S sides each.
    // - +         — optional separator between dice and level; both `5%5D10+L2Cape` and `300D10L5` occur.
    // - Llvl      — treasure level digit (1..7).
    // - ItemType  — item-type name (e.g. "Amulet"), looked up in itemTypeMap.
    auto parseTreasure = [](std::string_view s, MonsterInfo &info) {
        info.treasureDropChance = 0;
        info.goldDiceRolls = 0;
        info.goldDiceSides = 0;
        info.treasureType = RANDOM_ITEM_ANY;
        info.treasureLevel = ITEM_TREASURE_LEVEL_INVALID;

        // Edge case: 37 monsters have a literal "0" cell - they drop nothing, leave fields at default.
        if (s == "0")
            return;

        // Step 1: split off the "<chance>%" prefix.
        std::string_view rest = s;
        if (size_t pct = s.find('%'); pct != std::string_view::npos) {
            info.treasureDropChance = fromString<int>(s.substr(0, pct));
            rest = s.substr(pct + 1);
        } else {
            info.treasureDropChance = 100; // No %, but other flags present.
        }

        // Step 2: split the rest on the first 'L' (case-insensitive) into a dice part and a level/item part.
        std::string_view dicePart;
        std::string_view levelPart;
        size_t l = std::min(rest.find('l'), rest.find('L'));
        if (l != std::string_view::npos) {
            dicePart = rest.substr(0, l);
            levelPart = rest.substr(l);
        } else {
            dicePart = rest;
        }

        // Step 3: trim trailing '+' from dice part and parse "<rolls>D<sides>".
        if (!dicePart.empty() && dicePart.back() == '+')
            dicePart.remove_suffix(1);
        if (size_t d = std::min(dicePart.find('d'), dicePart.find('D')); d != std::string_view::npos) {
            info.goldDiceRolls = fromString<int>(dicePart.substr(0, d));
            info.goldDiceSides = fromString<int>(dicePart.substr(d + 1));
        }

        // Step 4: level part is "L<digit>[ItemType]".
        if (!levelPart.empty()) {
            levelPart = levelPart.substr(1); // drop the 'L'
            if (!levelPart.empty() && levelPart[0] >= '0' && levelPart[0] <= '9') {
                info.treasureLevel = ItemTreasureLevel(levelPart[0] - '0');
                std::string_view itemName = levelPart.substr(1);
                if (!itemName.empty())
                    info.treasureType = valueOr(itemTypeMap, itemName, RANDOM_ITEM_ANY);
            }
        }
    };

    // HP and EXP cells are mostly plain numbers, but a couple of monsters use a quoted thousand-separated
    // format like `" 1,300 "`. Strip surrounding quotes/whitespace and any thousand-separator commas, then parse as int.
    auto parseThousand = [](std::string_view s) {
        std::string buf(trim(unquote(s)));
        std::erase(buf, ',');
        return fromString<int>(buf);
    };

    for (std::string_view line : split(monsters).by("\r\n").drop(4).skip("")) {
        if (i >= 264) break;  // TODO(captainurist): get rid of magic numbers in txt deserialization.
        std::array<std::string_view, 39> tokens = split(line).by('\t');
        i++;

        MonsterId id = static_cast<MonsterId>(fromString<int>(tokens[0]));
        MonsterInfo &info = infos[id];
        info.id = id;
        info.name = unquote(tokens[1]);
        info.internalName = unquote(tokens[2]);
        info.level = fromString<int>(tokens[3]);
        info.hp = parseThousand(tokens[4]);
        info.ac = fromString<int>(tokens[5]);
        info.exp = parseThousand(tokens[6]);
        parseTreasure(tokens[7], info);
        info.bloodSplatOnDeath = fromString<int>(tokens[8]) != 0;
        info.flying = parseYesNo(tokens[9]);
        info.movementType = valueOr(movementMap, tokens[10], MONSTER_MOVEMENT_TYPE_FREE);
        info.aiType = valueOr(aiMap, tokens[11], MONSTER_AI_AGGRESSIVE);
        info.hostilityType = static_cast<MonsterHostility>(fromString<int>(tokens[12]));
        info.baseSpeed = fromString<int>(tokens[13]);
        info.recoveryTime = Duration::fromTicks(fromString<int>(tokens[14]));

        parseAttackPrefs(tokens[15], info);
        parseSpecialAttack(tokens[16], info);

        info.attack1Type = ParseAttackType(tokens[17]);
        ParseDamage(tokens[18], &info.attack1DamageDiceRolls, &info.attack1DamageDiceSides, &info.attack1DamageBonus);
        info.attack1MissileType = ParseMissleAttackType(tokens[19]);
        info.attack2Chance = fromString<int>(tokens[20]);
        info.attack2Type = ParseAttackType(tokens[21]);
        ParseDamage(tokens[22], &info.attack2DamageDiceRolls, &info.attack2DamageDiceSides, &info.attack2DamageBonus);
        info.attack2MissileType = ParseMissleAttackType(tokens[23]);
        info.spell1UseChance = fromString<int>(tokens[24]);
        parseSpellEntry(tokens[25], info.spell1Id, info.spell1SkillMastery);
        info.spell2UseChance = fromString<int>(tokens[26]);
        parseSpellEntry(tokens[27], info.spell2Id, info.spell2SkillMastery);
        info.resFire     = parseResistance(tokens[28]);
        info.resAir      = parseResistance(tokens[29]);
        info.resWater    = parseResistance(tokens[30]);
        info.resEarth    = parseResistance(tokens[31]);
        info.resMind     = parseResistance(tokens[32]);
        info.resSpirit   = parseResistance(tokens[33]);
        info.resBody     = parseResistance(tokens[34]);
        info.resLight    = parseResistance(tokens[35]);
        info.resDark     = parseResistance(tokens[36]);
        info.resPhysical = parseResistance(tokens[37]);
        parseSpecialAbility(tokens[38], info);
    }
}

//----- (0044FA08) --------------------------------------------------------
MonsterId MonsterList::GetMonsterIDByName(std::string_view pMonsterName) {
    for (MonsterId i : monsters.indices()) {
        if (ascii::noCaseEquals(monsters[i].internalMonsterName, pMonsterName))
            return i;
    }
    logger->error("Monster not found: {}", pMonsterName);
    return MONSTER_INVALID;
}

MonsterId GetBaseMonsterId(MonsterId monsterId) {
    if (monsterId == MONSTER_INVALID) return MONSTER_INVALID;
    // Base monster IDs are always the first in their group of 3 (e.g. MONSTER_ANGEL_A, MONSTER_ANGEL_B, MONSTER_ANGEL_C).
    return MonsterId(std::to_underlying(monsterId) - ((std::to_underlying(monsterId) - 1) % 3));
}
