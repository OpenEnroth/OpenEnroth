#pragma once

#include <cstdint>

enum class HouseType : uint16_t {
    HOUSE_TYPE_INVALID = 0,
    HOUSE_TYPE_WEAPON_SHOP = 1,
    HOUSE_TYPE_ARMOR_SHOP = 2,
    HOUSE_TYPE_MAGIC_SHOP = 3,
    HOUSE_TYPE_ALCHEMY_SHOP = 4,
    HOUSE_TYPE_FIRE_GUILD = 5,
    HOUSE_TYPE_AIR_GUILD = 6,
    HOUSE_TYPE_WATER_GUILD = 7,
    HOUSE_TYPE_EARTH_GUILD = 8,
    HOUSE_TYPE_SPIRIT_GUILD = 9,
    HOUSE_TYPE_MIND_GUILD = 10,
    HOUSE_TYPE_BODY_GUILD = 11,
    HOUSE_TYPE_LIGHT_GUILD = 12,
    HOUSE_TYPE_DARK_GUILD = 13,
    HOUSE_TYPE_ELEMENTAL_GUILD = 14, // In MM6 it was Element Guild
    HOUSE_TYPE_SELF_GUILD = 15, // Exist only in MM6 and MM8
    HOUSE_TYPE_MIRRORED_PATH_GUILD = 16, // Mirrored Path Guild of Light and Dark (not made it into any released game)
    HOUSE_TYPE_TOWN_HALL = 17, // Thieves guild in MM6.
    HOUSE_TYPE_MERCENARY_GUILD = 18, // TODO(captainurist): Somehow this is the type of almost all houses in the game. So, not a mercenary guild?
    HOUSE_TYPE_TOWN_HALL_MM6 = 19,
    HOUSE_TYPE_THRONE_ROOM = 20,
    HOUSE_TYPE_TAVERN = 21,
    HOUSE_TYPE_BANK = 22,
    HOUSE_TYPE_TEMPLE = 23,
    HOUSE_TYPE_CASTLE = 24,
    HOUSE_TYPE_DUNGEON = 25,
    HOUSE_TYPE_SEER = 26,
    HOUSE_TYPE_STABLE = 27,
    HOUSE_TYPE_BOAT = 28,
    HOUSE_TYPE_HOUSE = 29,
    HOUSE_TYPE_TRAINING_GROUND = 30,
    HOUSE_TYPE_JAIL = 31,
    HOUSE_TYPE_CIRCUS = 32, // MM6 Circus
    HOUSE_TYPE_GENERAL_STORE = 33, // MM6 General Store (where it was 3)
    HOUSE_TYPE_SHADOW_GUILD = 34, // MM6 Thieves Guild (where it was 17)
    HOUSE_TYPE_ADVENTURERS_INN = 35 // MM6 Adventurer's Inn
};
using enum HouseType;
