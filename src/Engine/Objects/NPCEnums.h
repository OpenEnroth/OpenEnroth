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
    NPC_PROFESSION_SMITH = 1,            // GM Weapon Repair.
    NPC_PROFESSION_ARMORER = 2,          // GM Armor Repair.
    NPC_PROFESSION_ALCHEMIST = 3,        // GM Magic Item Repair.
    NPC_PROFESSION_SCHOLAR = 4,          // GM Item ID,                 Learning: +5.
    NPC_PROFESSION_GUIDE = 5,            // Travel by foot: -1 day.
    NPC_PROFESSION_TRACKER = 6,          // Travel by foot: -2 days.
    NPC_PROFESSION_PATHFINDER = 7,       // Travel by foot: -3 days.
    NPC_PROFESSION_SAILOR = 8,           // Travel by sea: -2 days.
    NPC_PROFESSION_NAVIGATOR = 9,        // Travel by sea: -3 days.
    NPC_PROFESSION_HEALER = 10,          // Heal party once a day.
    NPC_PROFESSION_EXPERT_HEALER = 11,   // Heal party and cure conditions once a day.
    NPC_PROFESSION_MASTER_HEALER = 12,   // Heal party and cure all conditions except Eradicated once a day.
    NPC_PROFESSION_TEACHER = 13,         // Learning: +10.
    NPC_PROFESSION_INSTRUCTOR = 14,      // Learning: +15.
    NPC_PROFESSION_ARMS_MASTER = 15,     // Armsmaster: +2.
    NPC_PROFESSION_WEAPONS_MASTER = 16,  // Armsmaster: +3.
    NPC_PROFESSION_APPRENTICE = 17,      // Fire: +2,          Air: +2,    Water: +2,   Earth: +2.
    NPC_PROFESSION_MYSTIC = 18,          // Fire: +3,          Air: +3,    Water: +3,   Earth: +3.
    NPC_PROFESSION_SPELL_MASTER = 19,    // Fire: +4,          Air: +4,    Water: +4,   Earth: +4.
    NPC_PROFESSION_TRADER = 20,          // Merchant: +4.
    NPC_PROFESSION_MERCHANT = 21,        // Merchant: +6.
    NPC_PROFESSION_SCOUT = 22,           // Perception: +6.
    NPC_PROFESSION_HERBALIST = 23,       // Alchemy: +4.
    NPC_PROFESSION_APOTHECARY = 24,      // Alchemy: +8.
    NPC_PROFESSION_TINKER = 25,          // Traps: +4.
    NPC_PROFESSION_LOCKSMITH = 26,       // Traps: +6.
    NPC_PROFESSION_FOOL = 27,            // Luck: +5.
    NPC_PROFESSION_CHIMNEY_SWEEP = 28,   // Luck: +20.
    NPC_PROFESSION_PORTER = 29,          // Food for rest: -1.
    NPC_PROFESSION_QUARTER_MASTER = 30,  // Food for rest: -2.
    NPC_PROFESSION_FACTOR = 31,          // Gold finds: +10%.
    NPC_PROFESSION_BANKER = 32,          // Gold finds: +20%.
    NPC_PROFESSION_COOK = 33,            // Makes 1 food a day.
    NPC_PROFESSION_CHEF = 34,            // Makes 2 food a day.
    NPC_PROFESSION_HORSEMAN = 35,        // Travel by stable: -2 days.
    NPC_PROFESSION_BARD = 36,            // Never generated in MM7, no effect.
    NPC_PROFESSION_ENCHANTER = 37,       // Resist All: +20.
    NPC_PROFESSION_CARTOGRAPHER = 38,    // Wizard Eye: Expert.
    NPC_PROFESSION_WIND_MASTER = 39,     // Casts Fly once a day.
    NPC_PROFESSION_WATER_MASTER = 40,    // Casts Water Walk once a day.
    NPC_PROFESSION_GATE_MASTER = 41,     // Casts Town Portal once a day.
    NPC_PROFESSION_CHAPLAIN = 42,        // Casts Bless once a day.
    NPC_PROFESSION_PIPER = 43,           // Casts Heroism once a day.
    NPC_PROFESSION_EXPLORER = 44,        // Travel by foot: -1 day,     Travel by sea: -1 day, Travel by stable: -1 day.
    NPC_PROFESSION_PIRATE = 45,          // Travel by sea: -2 days,     Gold finds: +10%, Reputation: +5.
    NPC_PROFESSION_SQUIRE = 46,          // No effect.                  TODO(captainurist): vanilla MM7 gives +2 to weapon and armor skills, OE doesn't.
    NPC_PROFESSION_PSYCHIC = 47,         // Perception: +5,             Luck: +10.
    NPC_PROFESSION_GYPSY = 48,           // Food for rest: -1,          Merchant: +3, Reputation: +5.
    NPC_PROFESSION_DIPLOMAT = 49,        // Never generated in MM7, no effect.
    NPC_PROFESSION_DUPER = 50,           // Merchant: +8,               Reputation: +5.
    NPC_PROFESSION_BURGLAR = 51,         // Traps: +8,                  Stealing: +8, Reputation: +5.
    NPC_PROFESSION_FALLEN_WIZARD = 52,   // Reputation: +5,             Casts Hour of Power once a day.
    NPC_PROFESSION_ACOLYTE = 53,         // Spirit: +2,                 Mind: +2,              Body: +2.
    NPC_PROFESSION_INITIATE = 54,        // Spirit: +3,                 Mind: +3,              Body: +3.
    NPC_PROFESSION_PRELATE = 55,         // Spirit: +4,                 Mind: +4,              Body: +4.
    NPC_PROFESSION_MONK = 56,            // Unarmed: +2,                Dodge: +2.
    NPC_PROFESSION_SAGE = 57,            // Monster ID: +6.             TODO(captainurist): vanilla MM7 also gives Item ID +6, OE doesn't.
    NPC_PROFESSION_HUNTER = 58,          // Monster ID: +6.

    NPC_PROFESSION_FIRST = NPC_PROFESSION_SMITH,
    NPC_PROFESSION_LAST = NPC_PROFESSION_HUNTER
};
using enum NpcProfession;
