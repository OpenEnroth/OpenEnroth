#pragma once

#include <utility>

#include "HouseEnums.h"

#include "Utility/Segment.h"

inline Segment<HouseId> allTownhallHouses() {
    return {HOUSE_FIRST_TOWN_HALL, HOUSE_LAST_TOWN_HALL};
}

inline Segment<HouseId> allHouses() {
    return {HOUSE_FIRST, HOUSE_LAST};
}

inline Segment<HouseId> allArcomageTaverns() {
    return {HOUSE_FIRST_ARCOMAGE_TAVERN, HOUSE_LAST_ARCOMAGE_TAVERN};
}

inline bool isShop(HouseId houseId) {
    return houseId >= HOUSE_FIRST_SHOP && houseId <= HOUSE_LAST_SHOP;
}

inline bool isWeaponShop(HouseId houseId) {
    return houseId >= HOUSE_FIRST_WEAPON_SHOP && houseId <= HOUSE_LAST_WEAPON_SHOP;
}

inline bool isArmorShop(HouseId houseId) {
    return houseId >= HOUSE_FIRST_ARMOR_SHOP && houseId <= HOUSE_LAST_ARMOR_SHOP;
}

inline bool isMagicShop(HouseId houseId) {
    return houseId >= HOUSE_FIRST_MAGIC_SHOP && houseId <= HOUSE_LAST_MAGIC_SHOP;
}

inline bool isAlchemyShop(HouseId houseId) {
    return houseId >= HOUSE_FIRST_ALCHEMY_SHOP && houseId <= HOUSE_LAST_ALCHEMY_SHOP;
}

inline bool isMagicGuild(HouseId houseId) {
    return houseId >= HOUSE_FIRST_MAGIC_GUILD && houseId <= HOUSE_LAST_MAGIC_GUILD;
}

inline bool isStable(HouseId houseId) {
    return houseId >= HOUSE_FIRST_STABLE && houseId <= HOUSE_LAST_STABLE;
}

inline bool isBoat(HouseId houseId) {
    return houseId >= HOUSE_FIRST_BOAT && houseId <= HOUSE_LAST_BOAT;
}

inline bool isTavern(HouseId houseId) {
    return houseId >= HOUSE_FIRST_TAVERN && houseId <= HOUSE_LAST_TAVERN;
}

inline bool isArcomageTavern(HouseId houseId) {
    return houseId >= HOUSE_FIRST_ARCOMAGE_TAVERN && houseId <= HOUSE_LAST_ARCOMAGE_TAVERN;
}

inline int arcomageTopicForTavern(HouseId houseId) {
    assert(isArcomageTavern(houseId));
    return std::to_underlying(houseId) - std::to_underlying(HOUSE_FIRST_ARCOMAGE_TAVERN) + 355;
}
