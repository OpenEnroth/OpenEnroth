#pragma once

#include <cstdint>

/**
 * Phrase IDs for phrases displayed in shops when hovering over items.
 *
 * IDs work for selling, buying, repairing and identifying items.
 */
enum class MerchantPhrase {
    MERCHANT_PHRASE_NOT_ENOUGH_GOLD = 0,    // Not used at the moment.
    MERCHANT_PHRASE_PRICE = 1,              // When hovering over an item w/o a merchant skill.
    MERCHANT_PHRASE_PRICE_HAGGLE = 2,       // When hovering over an item while having a merchant skill.
    MERCHANT_PHRASE_PRICE_HAGGLE_TO_ACTUAL_PRICE = 3,   // When hovering over an item while having a merchant skill
                                                        // that reduces the price of an item to its actual price.
    MERCHANT_PHRASE_INCOMPATIBLE_ITEM = 4,  // When hovering over an incompatible item, e.g. a weapon at an armor shop.
    MERCHANT_PHRASE_INVALID_ACTION = 5,     // When hovering over an item that you cannot perform any action on,
                                            // e.g. repairing a non-broken item, or selling a quest item.
    MERCAHNT_PHRASE_STOLEN_ITEM = 6,        // When hovering over a stolen item.

    MERCHANT_PHRASE_FIRST = MERCHANT_PHRASE_NOT_ENOUGH_GOLD,
    MERCHANT_PHRASE_LAST = MERCAHNT_PHRASE_STOLEN_ITEM
};
using enum MerchantPhrase;

enum class NpcProfession : int32_t {
    NPC_PROFESSION_NONE = 0,
    NPC_PROFESSION_SMITH = 1,            // Repairs any weapon.
    NPC_PROFESSION_ARMORER = 2,          // Repairs any armor.
    NPC_PROFESSION_ALCHEMIST = 3,        // Repairs any magic item.
    NPC_PROFESSION_SCHOLAR = 4,          // Identifies any item and adds 5% to experience gained.
    NPC_PROFESSION_GUIDE = 5,            // Travel on foot takes 1 day less.
    NPC_PROFESSION_TRACKER = 6,          // Travel on foot takes 2 days less.
    NPC_PROFESSION_PATHFINDER = 7,       // Travel on foot takes 3 days less.
    NPC_PROFESSION_SAILOR = 8,           // Travel by boat takes 2 days less.
    NPC_PROFESSION_NAVIGATOR = 9,        // Travel by boat takes 3 days less.
    NPC_PROFESSION_HEALER = 10,          // Heals the party once a day.
    NPC_PROFESSION_EXPERT_HEALER = 11,   // Heals the party and cures every condition except Dead, Petrified and Eradicated once a day.
    NPC_PROFESSION_MASTER_HEALER = 12,   // Heals the party and cures every condition except Eradicated once a day.
    NPC_PROFESSION_TEACHER = 13,         // Adds 10% to experience gained.
    NPC_PROFESSION_INSTRUCTOR = 14,      // Adds 15% to experience gained.
    NPC_PROFESSION_ARMS_MASTER = 15,     // Adds 2 to the Armsmaster skill.
    NPC_PROFESSION_WEAPONS_MASTER = 16,  // Adds 3 to the Armsmaster skill.
    NPC_PROFESSION_APPRENTICE = 17,      // Adds 2 to the Fire, Air, Water and Earth skills.
    NPC_PROFESSION_MYSTIC = 18,          // Adds 3 to the Fire, Air, Water and Earth skills.
    NPC_PROFESSION_SPELL_MASTER = 19,    // Adds 4 to the Fire, Air, Water and Earth skills.
    NPC_PROFESSION_TRADER = 20,          // Adds 4 to the Merchant skill.
    NPC_PROFESSION_MERCHANT = 21,        // Adds 6 to the Merchant skill.
    NPC_PROFESSION_SCOUT = 22,           // Adds 6 to the Perception skill.
    NPC_PROFESSION_HERBALIST = 23,       // Adds 4 to the Alchemy skill.
    NPC_PROFESSION_APOTHECARY = 24,      // Adds 8 to the Alchemy skill.
    NPC_PROFESSION_TINKER = 25,          // Adds 4 to the Disarm Trap skill.
    NPC_PROFESSION_LOCKSMITH = 26,       // Adds 6 to the Disarm Trap skill.
    NPC_PROFESSION_FOOL = 27,            // Adds 5 to Luck.
    NPC_PROFESSION_CHIMNEY_SWEEP = 28,   // Adds 20 to Luck.
    NPC_PROFESSION_PORTER = 29,          // Resting uses 1 food less.
    NPC_PROFESSION_QUARTER_MASTER = 30,  // Resting uses 2 food less.
    NPC_PROFESSION_FACTOR = 31,          // Adds 10% to gold found.
    NPC_PROFESSION_BANKER = 32,          // Adds 20% to gold found.
    NPC_PROFESSION_COOK = 33,            // Makes 1 food a day.
    NPC_PROFESSION_CHEF = 34,            // Makes 2 food a day.
    NPC_PROFESSION_HORSEMAN = 35,        // Travel from stables takes 2 days less.
    NPC_PROFESSION_BARD = 36,            // Never generated in MM7 and has no effect.
    NPC_PROFESSION_ENCHANTER = 37,       // Adds 20 to all resistances.
    NPC_PROFESSION_CARTOGRAPHER = 38,    // Keeps Wizard Eye on at Expert or better.
    NPC_PROFESSION_WIND_MASTER = 39,     // Casts Fly once a day.
    NPC_PROFESSION_WATER_MASTER = 40,    // Casts Water Walk once a day.
    NPC_PROFESSION_GATE_MASTER = 41,     // Casts Town Portal once a day.
    NPC_PROFESSION_CHAPLAIN = 42,        // Casts Bless once a day.
    NPC_PROFESSION_PIPER = 43,           // Casts Heroism once a day.
    NPC_PROFESSION_EXPLORER = 44,        // Travel on foot, by boat and from stables takes 1 day less.
    NPC_PROFESSION_PIRATE = 45,          // Travel by boat takes 2 days less, adds 10% to gold found and worsens reputation by 5.
    NPC_PROFESSION_SQUIRE = 46,          // Has no effect. TODO(captainurist): vanilla MM7 gives +2 to weapon and armor skills, OE doesn't.
    NPC_PROFESSION_PSYCHIC = 47,         // Adds 5 to the Perception skill and 10 to Luck.
    NPC_PROFESSION_GYPSY = 48,           // Resting uses 1 food less, adds 3 to the Merchant skill and worsens reputation by 5.
    NPC_PROFESSION_DIPLOMAT = 49,        // Never generated in MM7 and has no effect.
    NPC_PROFESSION_DUPER = 50,           // Adds 8 to the Merchant skill and worsens reputation by 5.
    NPC_PROFESSION_BURGLAR = 51,         // Adds 8 to the Disarm Trap and Stealing skills and worsens reputation by 5.
    NPC_PROFESSION_FALLEN_WIZARD = 52,   // Casts Hour of Power once a day and worsens reputation by 5.
    NPC_PROFESSION_ACOLYTE = 53,         // Adds 2 to the Spirit, Mind and Body skills.
    NPC_PROFESSION_INITIATE = 54,        // Adds 3 to the Spirit, Mind and Body skills.
    NPC_PROFESSION_PRELATE = 55,         // Adds 4 to the Spirit, Mind and Body skills.
    NPC_PROFESSION_MONK = 56,            // Adds 2 to the Unarmed and Dodge skills.
    NPC_PROFESSION_SAGE = 57,            // Adds 6 to the Monster ID skill. TODO(captainurist): vanilla MM7 also gives Item ID +6, OE doesn't.
    NPC_PROFESSION_HUNTER = 58,          // Adds 6 to the Monster ID skill.

    NPC_PROFESSION_FIRST = NPC_PROFESSION_SMITH,
    NPC_PROFESSION_LAST = NPC_PROFESSION_HUNTER
};
using enum NpcProfession;
