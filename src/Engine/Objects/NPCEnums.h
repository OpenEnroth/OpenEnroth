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

// TODO(captainurist): #enum renamings needed
enum class NpcProfession : int32_t {
    NoProfession = 0,
    Smith = 1,       // GM Weapon Repair;
    Armorer = 2,     // GM Armor Repair;
    Alchemist = 3,   // GM Potion Repair;
    Scholar = 4,     // GM Item ID;               Learning: +5
    Guide = 5,       // Travel by foot: -1 day;
    Tracker = 6,     // Travel by foot: -2 days;
    Pathfinder = 7,  // Travel by foot: -3 days;
    Sailor = 8,      // Travel by sea: -2 days;
    Navigator = 9,   // Travel by sea: -3 days;
    Healer = 10,
    ExpertHealer = 11,
    MasterHealer = 12,
    Teacher = 13,        // Learning: +10;
    Instructor = 14,     // Learning: +15;
    Armsmaster = 15,     // Armsmaster: +2;
    Weaponsmaster = 16,  // Armsmaster: +3;
    Apprentice = 17,    // Fire: +2;         Air: +2;    Water: +2;   Earth: +2;
    Mystic = 18,        // Fire: +3;         Air: +3;    Water: +3;   Earth: +3;
    Spellmaster = 19,   // Fire: +4;         Air: +4;    Water: +4;   Earth: +4;
    Trader = 20,        // Merchant: +4;
    Merchant = 21,      // Merchant: +6;
    Scout = 22,         // Perception: +6;
    Herbalist = 23,     // Alchemy: +4;
    Apothecary = 24,    // Alchemy: +8;
    Tinker = 25,        // Traps: +4;
    Locksmith = 26,     // Traps: +6;
    Fool = 27,          // Luck: +5;
    ChimneySweep = 28,  // Luck: +20;
    Porter = 29,        // Food for rest: -1;
    QuarterMaster = 30,  // Food for rest: -2;
    Factor = 31,         // Gold finds: +10%;
    Banker = 32,         // Gold finds: +20%;
    Cook = 33,
    Chef = 34,
    Horseman = 35,  // Travel by foot: -2 days;
    Bard = 36,
    Enchanter = 37,     // Resist All: +20;
    Cartographer = 38,  // Wizard Eye level 2;
    WindMaster = 39,
    WaterMaster = 40,
    GateMaster = 41,
    Acolyte = 42,
    Piper = 43,
    Explorer = 44,  // Travel by foot -1 day;     Travel by sea: -1 day;
    Pirate = 45,    // Travel by sea: -2 days;    Gold finds: +10%; Reputation: +5;
    Squire = 46,
    Psychic = 47,  // Perception: +5;            Luck: +10;
    Gypsy = 48,    // Food for rest: -1;         Merchant: +3; Reputation: +5;
    Diplomat = 49,
    Duper = 50,    // Merchant: +8;              Reputation: +5;
    Burglar = 51,  // Traps: +8;                 Stealing: +8; Reputation: +5;
    FallenWizard = 52,  // Reputation: +5;
    Acolyte2 = 53,  // Spirit: +2;                Mind: +2;              Body: +2;
    Initiate = 54,  // Spirit: +3;                Mind: +3;              Body: +3;
    Prelate = 55,      // Spirit: +4;                Mind: +4;              Body: +4;
    Monk = 56,   // Unarmed: +2;               Dodge: +2;
    Sage = 57,   // Monster ID: +6
    Hunter = 58,  // Monster ID: +6

    NPC_PROFESSION_FIRST = NoProfession,
    NPC_PROFESSION_LAST = Hunter,

    NPC_PROFESSION_FIRST_VALID = Smith,
    NPC_PROFESSION_LAST_VALID = NPC_PROFESSION_LAST
};
using enum NpcProfession;
