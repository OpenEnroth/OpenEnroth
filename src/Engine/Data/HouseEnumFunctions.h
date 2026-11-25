#pragma once

#include <cassert>
#include <utility>

#include "Engine/GameType.h"

#include "AwardEnums.h"
#include "HouseEnums.h"

#include "Utility/Segment.h"

inline Segment<HouseId> allTownhallHouses() {
    return {HOUSE_FIRST_TOWN_HALL, HOUSE_LAST_TOWN_HALL};
}

inline Segment<HouseId> allHouses() {
    return {HOUSE_FIRST, HOUSE_LAST};
}

// MM7 has 13 Arcomage taverns (houses 108..120); MM8's 11 real taverns are houses 107..117
// (118+ are Placeholder rows). The set is game-variant dependent; the save layout stays 13 slots, indexed by position.
inline Segment<HouseId> allArcomageTaverns() {
    if (gameType() == GAME_TYPE_MM8)
        return {HOUSE_TAVERN_EMERALD_ISLAND, static_cast<HouseId>(117)};
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

// "combined" guilds (real MM6/MM8 concept, a single building teaching several magic
// schools at once) as opposed to the single-school HOUSE_TYPE_FIRE_GUILD..HOUSE_TYPE_DARK_GUILD
// towers. HOUSE_TYPE_MIRRORED_PATH_GUILD is a third combined type but never shipped in any
// released game (see its own enum comment), so it's intentionally not included here.
inline bool isCombinedMagicGuild(HouseType houseType) {
    return houseType == HOUSE_TYPE_ELEMENTAL_GUILD || houseType == HOUSE_TYPE_SELF_GUILD;
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
    return allArcomageTaverns().contains(houseId);
}

// Position of an Arcomage tavern within the tavern set; indexes pArcomageWins, start conditions and topics.
inline int arcomageTavernIndex(HouseId houseId) {
    assert(isArcomageTavern(houseId));
    return std::to_underlying(houseId) - std::to_underlying(allArcomageTaverns().front());
}

inline int arcomageTopicForTavern(HouseId houseId) {
    assert(isArcomageTavern(houseId));
    return arcomageTavernIndex(houseId) + 355;
}

AwardId membershipAwardForGuild(HouseId houseId);

AwardId membershipAwardForGuild(GuildId guildId);
