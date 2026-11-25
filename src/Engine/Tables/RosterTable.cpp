#include "RosterTable.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Objects/ItemEnumFunctions.h"
#include "Engine/Spells/SpellEnumFunctions.h"
#include "Engine/Tables/ItemTable.h"

#include "Library/Logger/Logger.h"
#include "Library/Serialization/Serialization.h"

#include "Utility/Memory/Blob.h"
#include "Utility/String/Ascii.h"
#include "Utility/String/Split.h"
#include "Utility/String/Transformations.h"

std::vector<RosterRow> rosterMm8;

namespace {

// roster.txt header column names for the 7 base attributes, in the exact order the file itself
// uses (note this is NOT the engine's own `Attribute` enum order - the file has Speed before
// Accuracy, the enum has Accuracy before Speed - so columns are matched by name, never position).
constexpr std::array<std::pair<const char *, Attribute>, 7> kAttributeColumns = {{
    {"might", ATTRIBUTE_MIGHT},
    {"intelligence", ATTRIBUTE_INTELLIGENCE},
    {"personality", ATTRIBUTE_PERSONALITY},
    {"endurance", ATTRIBUTE_ENDURANCE},
    {"speed", ATTRIBUTE_SPEED},
    {"accuracy", ATTRIBUTE_ACCURACY},
    {"luck", ATTRIBUTE_LUCK},
}};

// The six base-resistance columns, in file order (roster.txt has no Spirit column: Spirit shares
// `sResBodyBase` with Body, see `Character::GetBaseResistance()`).
constexpr std::array<std::pair<const char *, Attribute>, 6> kResistanceColumns = {{
    {"fire resist", ATTRIBUTE_RESIST_FIRE},
    {"air resist", ATTRIBUTE_RESIST_AIR},
    {"water resist", ATTRIBUTE_RESIST_WATER},
    {"earth resist", ATTRIBUTE_RESIST_EARTH},
    {"mind resist", ATTRIBUTE_RESIST_MIND},
    {"body resist", ATTRIBUTE_RESIST_BODY},
}};

// roster.txt's own skill-column header names, in file order, mapped to the engine's `Skill` enum.
// `SKILL_INVALID` marks a header with no engine equivalent. `SKILL_DIPLOMACY` and `SKILL_THIEVERY`
// have no roster.txt column.
constexpr std::array<std::pair<const char *, Skill>, 39> kSkillColumns = {{
    {"staff", SKILL_STAFF},
    {"sword", SKILL_SWORD},
    {"dagger", SKILL_DAGGER},
    {"axe", SKILL_AXE},
    {"spear", SKILL_SPEAR},
    {"bow", SKILL_BOW},
    {"mace", SKILL_MACE},
    {"blaster", SKILL_BLASTER},
    {"shield", SKILL_SHIELD},
    {"leather", SKILL_LEATHER},
    {"chain", SKILL_CHAIN},
    {"plate", SKILL_PLATE},
    {"fire", SKILL_FIRE},
    {"air", SKILL_AIR},
    {"water", SKILL_WATER},
    {"earth", SKILL_EARTH},
    {"spirit", SKILL_SPIRIT},
    {"mind", SKILL_MIND},
    {"body", SKILL_BODY},
    {"light", SKILL_LIGHT},
    {"dark", SKILL_DARK},
    {"darkelf", SKILL_INVALID},
    {"vampire", SKILL_INVALID},
    {"dragon", SKILL_INVALID},
    {"id item", SKILL_ITEM_ID},
    {"merchant", SKILL_MERCHANT},
    {"repair", SKILL_REPAIR},
    {"b building", SKILL_BODYBUILDING},
    {"meditate", SKILL_MEDITATION},
    {"perception", SKILL_PERCEPTION},
    {"regeneration", SKILL_INVALID},
    {"disarm trap", SKILL_TRAP_DISARM},
    {"dodging", SKILL_DODGE},
    {"unarmed", SKILL_UNARMED},
    {"ident mon", SKILL_MONSTER_ID},
    {"armsmaster", SKILL_ARMSMASTER},
    {"stealing", SKILL_STEALING},
    {"alchemy", SKILL_ALCHEMY},
    {"learning", SKILL_LEARNING},
}};

std::string lower(std::string_view s) {
    return ascii::toLower(s);
}

// The header names the schools twice: skill tier/level pairs first, then these know-counts. "Fire resist" and its
// siblings come before both, so a substring match on "fire" lands on a resistance column.
constexpr std::array<std::pair<const char *, MagicSchool>, 9> kSpellCountColumns = {{
    {"fire", MAGIC_SCHOOL_FIRE},
    {"air", MAGIC_SCHOOL_AIR},
    {"water", MAGIC_SCHOOL_WATER},
    {"earth", MAGIC_SCHOOL_EARTH},
    {"spirit", MAGIC_SCHOOL_SPIRIT},
    {"mind", MAGIC_SCHOOL_MIND},
    {"body", MAGIC_SCHOOL_BODY},
    {"light", MAGIC_SCHOOL_LIGHT},
    {"dark", MAGIC_SCHOOL_DARK},
}};

Mastery masteryFromTierLetter(std::string_view tier) {
    if (tier.empty())
        return MASTERY_NONE;
    switch (std::tolower(static_cast<unsigned char>(tier[0]))) {
    case 'b': return MASTERY_NOVICE;    // roster.txt's own header: "B, E, M, G for Basic, Expert, Master, Grandmaster".
    case 'e': return MASTERY_EXPERT;
    case 'm': return MASTERY_MASTER;
    case 'g': return MASTERY_GRANDMASTER;
    default: return MASTERY_NONE;
    }
}

} // namespace

void initializeRosterMm8(const Blob &roster) {
    rosterMm8.clear();
    if (roster.size() == 0)
        return; // Not shipped (e.g. an MM7 install) - leave rosterMm8 empty.

    std::vector<std::string_view> lines;
    for (std::string_view line : split(roster.str()).by("\r\n").skip(""))
        lines.push_back(line);
    if (lines.size() < 3)
        return; // 2 header rows + at least 1 data row expected; anything less isn't a real table.

    std::vector<std::string_view> header = split(lines[1]).by('\t');

    // Columns are looked up by header keyword, never by index.
    auto findCol = [&](std::string_view keyword) -> int {
        for (size_t c = 0; c < header.size(); c++) {
            if (lower(header[c]).find(keyword) != std::string::npos)
                return static_cast<int>(c);
        }
        return -1;
    };
    int nameCol = findCol("name");
    int classCol = findCol("class");
    int pictureCol = findCol("picture");
    int levelCol = findCol("level");
    int questBitCol = findCol("questbit");
    int equipmentCol = findCol("equipment");
    if (nameCol < 0 || classCol < 0 || pictureCol < 0 || levelCol < 0 || questBitCol < 0 || equipmentCol < 0) {
        MM_WARNING("RosterTable: roster.txt header is missing a required column "
                         "(name={} class={} picture={} level={} questbit={} equipment={}), not loading any rows.",
                         nameCol, classCol, pictureCol, levelCol, questBitCol, equipmentCol);
        return;
    }

    // The nth header cell equal to `name`, -1 if there is none.
    auto findExactCol = [&](std::string_view name, int nth) -> int {
        for (size_t c = 0; c < header.size(); c++) {
            if (lower(trim(header[c])) == name && nth-- == 0)
                return static_cast<int>(c);
        }
        return -1;
    };

    std::array<int, kAttributeColumns.size()> attributeCols;
    for (size_t i = 0; i < kAttributeColumns.size(); i++)
        attributeCols[i] = findCol(kAttributeColumns[i].first);

    std::array<int, kSkillColumns.size()> skillCols;
    for (size_t i = 0; i < kSkillColumns.size(); i++)
        skillCols[i] = findExactCol(kSkillColumns[i].first, 0);

    std::array<int, kSpellCountColumns.size()> spellCountCols;
    for (size_t i = 0; i < kSpellCountColumns.size(); i++)
        spellCountCols[i] = findExactCol(kSpellCountColumns[i].first, 1);

    std::array<int, kResistanceColumns.size()> resistanceCols;
    for (size_t i = 0; i < kResistanceColumns.size(); i++)
        resistanceCols[i] = findExactCol(kResistanceColumns[i].first, 0);

    int skillPointsCol = findExactCol("skillpoints", 0);
    int experienceCol = findExactCol("experience", 0);

    int maxIndex = 0;
    for (size_t i = 2; i < lines.size(); i++) {
        std::vector<std::string_view> tokens = split(lines[i]).by('\t');
        if (tokens.size() < 3 || tokens[0].empty())
            continue;
        maxIndex = std::max(maxIndex, fromString<int>(tokens[0]));
    }
    rosterMm8.resize(maxIndex + 1);

    auto tokenAt = [](const std::vector<std::string_view> &tokens, int col) -> std::string_view {
        return (col >= 0 && static_cast<size_t>(col) < tokens.size()) ? tokens[col] : std::string_view();
    };

    for (size_t i = 2; i < lines.size(); i++) {
        std::vector<std::string_view> tokens = split(lines[i]).by('\t');
        if (tokens.size() < 3 || tokens[0].empty())
            continue;
        int idx = fromString<int>(tokens[0]);
        if (idx < 0 || static_cast<size_t>(idx) >= rosterMm8.size())
            continue;

        RosterRow &row = rosterMm8[idx];
        row.name = unquote(tokenAt(tokens, nameCol));
        row.className = std::string(tokenAt(tokens, classCol));
        row.pictureId = fromString<int>(tokenAt(tokens, pictureCol));
        row.level = fromString<int>(tokenAt(tokens, levelCol));
        std::string_view experienceCell = trim(tokenAt(tokens, experienceCol));
        row.experience = experienceCell.empty() ? 0 : fromString<int64_t>(experienceCell);
        std::string_view questBitCell = tokenAt(tokens, questBitCol);
        row.questBit = questBitCell.empty() ? 0 : fromString<int>(questBitCell);
        std::string_view equipmentCell = trim(tokenAt(tokens, equipmentCol));
        row.equipmentItemId = equipmentCell.empty() ? ITEM_NULL : static_cast<ItemId>(fromString<int>(equipmentCell) % 1000);

        row.stats.fill(0);
        for (size_t a = 0; a < kAttributeColumns.size(); a++) {
            std::string_view cell = tokenAt(tokens, attributeCols[a]);
            if (!cell.empty())
                row.stats[kAttributeColumns[a].second] = fromString<int>(cell);
        }

        row.skills.fill(CombinedSkillValue::none());
        for (size_t s = 0; s < kSkillColumns.size(); s++) {
            Skill skill = kSkillColumns[s].second;
            if (skill == SKILL_INVALID)
                continue; // No engine equivalent for this roster.txt column (see kSkillColumns).
            int tierCol = skillCols[s];
            if (tierCol < 0)
                continue;
            std::string_view tier = tokenAt(tokens, tierCol);
            std::string_view levelCell = tokenAt(tokens, tierCol + 1);
            Mastery mastery = masteryFromTierLetter(tier);
            if (mastery == MASTERY_NONE)
                continue; // Blank cell - row doesn't carry this skill.
            int skillLevel = fromString<int>(levelCell);
            if (skillLevel <= 0)
                continue;
            row.skills[skill] = CombinedSkillValue(skillLevel, mastery);
        }

        row.knownSpellCounts.fill(0);
        for (size_t k = 0; k < kSpellCountColumns.size(); k++) {
            std::string_view cell = trim(tokenAt(tokens, spellCountCols[k]));
            if (!cell.empty())
                row.knownSpellCounts[kSpellCountColumns[k].second] = fromString<int>(cell);
        }

        std::string_view skillPointsCell = trim(tokenAt(tokens, skillPointsCol));
        row.freeSkillPoints = skillPointsCell.empty() ? 0 : fromString<int>(skillPointsCell);

        row.baseResistances.fill(0);
        for (size_t r = 0; r < kResistanceColumns.size(); r++) {
            std::string_view cell = trim(tokenAt(tokens, resistanceCols[r]));
            if (!cell.empty())
                row.baseResistances[kResistanceColumns[r].second] = fromString<int>(cell);
        }
    }
}

// roster.txt writes a promoted hero's class as the race name plus "2" ("darkelf2" = Dark Elf Patriarch, "necromancer2" =
// Lich, the level 30 and level 50 heroes).
static bool rosterRowIsPromoted(int rosterId) {
    const std::string &className = rosterMm8[rosterId].className;
    return !className.empty() && className.back() == '2';
}

static Race raceForRosterRow(int rosterId) {
    const RosterRow &row = rosterMm8[rosterId];
    const std::string raceName = lower(rosterRowIsPromoted(rosterId) ? row.className.substr(0, row.className.size() - 1) : row.className);

    Race race = RACE_MM8_KNIGHT; // Fallback if the class text doesn't match a known MM8 race name.
    bool raceFound = false;
    for (Race candidate : allMm8Races()) {
        // The column has no spaces: "darkelf" is the race "Dark Elf".
        std::string candidateName = lower(mm8RaceName(candidate));
        std::erase(candidateName, ' ');
        if (candidateName == raceName) {
            race = candidate;
            raceFound = true;
            break;
        }
    }
    if (!raceFound) {
        MM_WARNING("characterFromRosterRow: roster.txt row {} has unrecognized class '{}', "
                         "defaulting to Knight.", rosterId, row.className);
    }
    return race;
}

static Class classForRosterRow(int rosterId) {
    Race race = raceForRosterRow(rosterId);
    return rosterRowIsPromoted(rosterId) ? mm8PromotedClassForRace(race) : mm8BaseClassForRace(race);
}

// Equipped in the first free slot the item type fits and the race may wear, the bag otherwise. Makes no grng draw.
static void equipRosterItem(Character &character, ItemId itemId) {
    Item item;
    item.itemId = itemId;
    if (character.CanEquip_RaceAndAlignmentCheck(itemId)) {
        ItemSlot slot = ITEM_SLOT_INVALID;
        for (ItemSlot candidate : itemSlotsForItemType(pItemTable->items[itemId].type)) {
            // Main hand wins over the off hand: a single-handed weapon in the off hand leaves `IsUnarmed()` true.
            if (character.inventory.canEquip(candidate) && (slot == ITEM_SLOT_INVALID || candidate == ITEM_SLOT_MAIN_HAND))
                slot = candidate;
        }
        if (slot != ITEM_SLOT_INVALID) {
            character.inventory.equip(slot, item);
            return;
        }
    }
    character.inventory.tryAdd(item);
}

// Everything except the class: the skill, health and mana writes overwrite whatever the class set.
static void fillRosterCharacter(Character &character, int rosterId) {
    const RosterRow &row = rosterMm8[rosterId];
    character.name = row.name;
    // roster.txt's "Picture" column is 1-based (matches the `pcNN-01` texture suffix directly,
    // e.g. Picture=22 -> "pc22-01"); `uCurrentFace` is 0-based (the engine's own convention builds
    // the same texture name as `pc{:02}-01` from `uCurrentFace + 1` - see
    // `GameUI_LoadPlayerPortraitsAndVoices()`), hence the -1 here.
    character.uCurrentFace = row.pictureId > 0 ? row.pictureId - 1 : 0;
    character.uLevel = static_cast<uint16_t>(row.level);
    character.experience = row.experience;
    character.uSkillPoints = row.freeSkillPoints;
    character.sResFireBase = row.baseResistances[ATTRIBUTE_RESIST_FIRE];
    character.sResAirBase = row.baseResistances[ATTRIBUTE_RESIST_AIR];
    character.sResWaterBase = row.baseResistances[ATTRIBUTE_RESIST_WATER];
    character.sResEarthBase = row.baseResistances[ATTRIBUTE_RESIST_EARTH];
    character.sResMindBase = row.baseResistances[ATTRIBUTE_RESIST_MIND];
    character.sResBodyBase = row.baseResistances[ATTRIBUTE_RESIST_BODY];
    for (Attribute stat : allStatAttributes())
        character._stats[stat] = row.stats[stat];
    for (Skill skill : allVisibleSkills())
        character.setSkillValue(skill, row.skills[skill]);
    for (MagicSchool school : allMagicSchools()) {
        Segment<SpellId> spells = spellsForMagicSchool(school);
        for (int i = 0; i < std::min<int>(row.knownSpellCounts[school], spells.size()); i++)
            character.bHaveSpell[spells[i]] = true;
    }
    character.health = character.GetMaxHealth();
    character.mana = character.GetMaxMana();
    character.rosterId = rosterId;
    if (row.equipmentItemId != ITEM_NULL)
        equipRosterItem(character, row.equipmentItemId);
}

Character characterFromRosterRow(int rosterId) {
    assert(rosterId > 0 && static_cast<size_t>(rosterId) < rosterMm8.size());
    Character character;
    character.ChangeClass(classForRosterRow(rosterId));
    fillRosterCharacter(character, rosterId);
    return character;
}

Character seatedCharacterFromRosterRow(int rosterId) {
    assert(rosterId > 0 && static_cast<size_t>(rosterId) < rosterMm8.size());
    Character character;
    // Not `ChangeClass()`: it draws from `grng`, and a seat at new game would shift every later game roll.
    character.classType = classForRosterRow(rosterId);
    character.uBirthYear = 1147; // TODO: roster.txt's Birthdate column is not parsed yet.
    fillRosterCharacter(character, rosterId);
    return character;
}
