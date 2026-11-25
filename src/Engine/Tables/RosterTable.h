#pragma once

#include <string>
#include <vector>

#include "Engine/Objects/Character.h"
#include "Engine/Objects/CharacterEnums.h"
#include "Engine/Objects/CombinedSkillValue.h"
#include "Engine/Objects/ItemEnums.h"
#include "Engine/Spells/SpellEnums.h"

#include "Utility/IndexedArray.h"

class Blob;

/**
 * One row of MM8's `roster.txt` - the table of recruitable companions. Row 0 is always
 * "Placeholder" (the player's own Champion of Jadame slot, not a recruitable companion); rows 1-50
 * are real companions.
 *
 * `roster.txt` is not shipped for MM7 (or any non-MM8 game type) - `rosterMm8` stays empty there.
 */
struct RosterRow {
    std::string name;

    /**
     * Raw "Class" column text (e.g. "troll", "cleric") - actually a race name: MM8 collapses
     * race and class into one (see `Race`'s `RACE_MM8_*` values), so this is what
     * `characterFromRosterRow()` resolves via `mm8RaceName()` to pick the character's `Race`.
     */
    std::string className;

    /**
     * "Picture" column. Indexes the same `pcNN-01` player-portrait pool the char-create/HUD code
     * already uses (`GameUI_LoadPlayerPortraitsAndVoices()`, `pc{:02}-{:02}` with
     * `uCurrentFace + 1`) - NOT `npcdata.txt`'s `portraitId`/`npc%04d` space (neither
     * `npc0022` nor `npc0006` exist in `icons.lod`, while `pc22-01`/`pc06-01` both do).
     */
    int pictureId = 0;

    int level = 0;

    /** "Experience" column - what the hero joins the party with. */
    int64_t experience = 0;

    IndexedArray<int, ATTRIBUTE_FIRST_STAT, ATTRIBUTE_LAST_STAT> stats;
    IndexedArray<CombinedSkillValue, SKILL_FIRST, SKILL_LAST> skills;

    /**
     * Second "Fire".."Dark" column block: how many spells of each school the row already knows, counted from the
     * school's first spell in spellbook order (3 in Spirit = Detect Life, Bless, Fate). The DarkElf/Vampire/Dragon
     * counts have no spells behind them (those are innate abilities) and are not read.
     */
    IndexedArray<int, MAGIC_SCHOOL_FIRST, MAGIC_SCHOOL_LAST> knownSpellCounts;

    /**
     * "Skillpoints" column - the free skill points the row starts with (`Character::uSkillPoints`).
     */
    int freeSkillPoints = 0;

    /**
     * "Fire resist".."Body resist" columns - the base resistances (`Character::sResFireBase` and its siblings).
     * The race and item bonuses are added on top of these.
     */
    IndexedArray<int, ATTRIBUTE_RESIST_FIRE, ATTRIBUTE_RESIST_BODY> baseResistances;

    /**
     * "QuestBit" column (index 112) - the `QuestBit` this row's join sets (e.g. 404 for row 4,
     * 434 for row 34). No script sets these bits, the original
     * executable does on join. 0 if
     * the row has no QuestBit (e.g. row 0, "Placeholder"). Set by `Party::recruitRoster()`'s
     * 601-649 topic trigger, not by `characterFromRosterRow()` itself.
     */
    int questBit = 0;

    /**
     * "Equipment" column - the item this row starts with, as `ItemId` (cell % 1000; the cell's
     * thousands are the item's enchantment level, 1-6, which is not carried over yet). ITEM_NULL if
     * the row has none.
     */
    ItemId equipmentItemId = ITEM_NULL;
};

/** MM8's roster.txt rows, indexed by the file's own row id (1-based; index 0 is "Placeholder"). Empty unless MM8 data was loaded. */
extern std::vector<RosterRow> rosterMm8;

/**
 * Parses MM8's `roster.txt`. An empty blob (the table isn't shipped, e.g. an MM7 install) leaves
 * `rosterMm8` empty and changes nothing.
 */
void initializeRosterMm8(const Blob &roster);

/**
 * Builds a party-ready `Character` from `rosterMm8[rosterId]`: name, race/class and portrait
 * always come from the row; level, the 7 base attributes, the free skill points, the base
 * resistances and every skill the row names a recognized mastery/level pair for are set from the row too. The returned character's
 * `rosterId` field is set to `rosterId`.
 *
 * @param rosterId                      A valid index into `rosterMm8`, 1-50 (0, "Placeholder", is
 *                                       the champion slot, not a recruitable companion, and is not
 *                                       valid here).
 */
Character characterFromRosterRow(int rosterId);

/**
 * Same as `characterFromRosterRow()`, but draws nothing from `grng`. Only for the new-game Inn seat, which runs
 * before the first game roll, so the seat leaves the random stream as it was.
 *
 * @param rosterId                      A valid index into `rosterMm8`, 1-50 (see `characterFromRosterRow()`).
 */
Character seatedCharacterFromRosterRow(int rosterId);
