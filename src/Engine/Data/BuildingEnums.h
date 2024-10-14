#pragma once

#include <cstdint>

enum class BuildingType : uint16_t {
    BUILDING_INVALID = 0,
    BUILDING_WEAPON_SHOP = 1,
    BUILDING_ARMOR_SHOP = 2,
    BUILDING_MAGIC_SHOP = 3,
    BUILDING_ALCHEMY_SHOP = 4,
    BUILDING_FIRE_GUILD = 5,
    BUILDING_AIR_GUILD = 6,
    BUILDING_WATER_GUILD = 7,
    BUILDING_EARTH_GUILD = 8,
    BUILDING_SPIRIT_GUILD = 9,
    BUILDING_MIND_GUILD = 10,
    BUILDING_BODY_GUILD = 11,
    BUILDING_LIGHT_GUILD = 12,
    BUILDING_DARK_GUILD = 13,
    BUILDING_ELEMENTAL_GUILD = 14, // In MM6 it was Element Guild
    BUILDING_SELF_GUILD = 15, // Exist only in MM6 and MM8
    BUILDING_MIRRORED_PATH_GUILD = 16, // Mirrored Path Guild of Light and Dark (not made it into any released game)
    BUILDING_TOWN_HALL = 17, // Thieves guild in MM6.
    BUILDING_MERCENARY_GUILD = 18, // TODO(captainurist): Somehow this is the type of almost all houses in the game. So, not a mercenary guild?
    BUILDING_TOWN_HALL_MM6 = 19,
    BUILDING_THRONE_ROOM = 20,
    BUILDING_TAVERN = 21,
    BUILDING_BANK = 22,
    BUILDING_TEMPLE = 23,
    BUILDING_CASTLE = 24,
    BUILDING_DUNGEON = 25,
    BUILDING_SEER = 26,
    BUILDING_STABLE = 27,
    BUILDING_BOAT = 28,
    BUILDING_HOUSE = 29,
    BUILDING_TRAINING_GROUND = 30,
    BUILDING_JAIL = 31,
    BUILDING_CIRCUS = 32, // MM6 Circus
    BUILDING_GENERAL_STORE = 33, // MM6 General Store (where it was 3)
    BUILDING_SHADOW_GUILD = 34, // MM6 Thieves Guild (where it was 17)
    BUILDING_ADVENTURERS_INN = 35 // MM6 Adventurer's Inn
};
using enum BuildingType;
