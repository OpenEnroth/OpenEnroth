#include "Shops.h"

#include <cstdlib>
#include <algorithm>
#include <string>
#include <array>
#include <vector>

#include "Engine/Engine.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/AssetsManager.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/LocationFunctions.h"
#include "Engine/Localization.h"
#include "Engine/MapInfo.h"
#include "Engine/Objects/Items.h"
#include "Engine/Party.h"
#include "Engine/PriceCalculator.h"
#include "Engine/Random/Random.h"
#include "Engine/Tables/MerchantTable.h"
#include "Engine/Tables/ItemTable.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIStatusBar.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/IndexedArray.h"

struct ITEM_VARIATION {
    ItemTreasureLevel treasureLevel;
    std::array<RandomItemType, 4> itemClass;
};

static constexpr IndexedArray<ITEM_VARIATION, HOUSE_FIRST_WEAPON_SHOP, HOUSE_LAST_WEAPON_SHOP> weaponShopVariationStandard = {{
    {HOUSE_WEAPON_SHOP_EMERALD_ISLAND,      { ITEM_TREASURE_LEVEL_1, { RANDOM_ITEM_SWORD,   RANDOM_ITEM_BOW,    RANDOM_ITEM_WEAPON, RANDOM_ITEM_WEAPON } }},
    {HOUSE_WEAPON_SHOP_HARMONDALE,          { ITEM_TREASURE_LEVEL_1, { RANDOM_ITEM_SWORD,   RANDOM_ITEM_DAGGER, RANDOM_ITEM_MACE,   RANDOM_ITEM_WEAPON } }},
    {HOUSE_WEAPON_SHOP_ERATHIA,             { ITEM_TREASURE_LEVEL_2, { RANDOM_ITEM_SWORD,   RANDOM_ITEM_DAGGER, RANDOM_ITEM_AXE,    RANDOM_ITEM_WEAPON } }},
    {HOUSE_WEAPON_SHOP_TULAREAN_FOREST,     { ITEM_TREASURE_LEVEL_2, { RANDOM_ITEM_BOW,     RANDOM_ITEM_BOW,    RANDOM_ITEM_SPEAR,  RANDOM_ITEM_SPEAR } }},
    {HOUSE_WEAPON_SHOP_CELESTE,             { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_DAGGER,  RANDOM_ITEM_STAFF,  RANDOM_ITEM_AXE,    RANDOM_ITEM_BOW } }},
    {HOUSE_WEAPON_SHOP_PIT,                 { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_DAGGER,  RANDOM_ITEM_STAFF,  RANDOM_ITEM_AXE,    RANDOM_ITEM_BOW } }},
    {HOUSE_WEAPON_SHOP_MOUNT_NIGHON,        { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_STAFF,   RANDOM_ITEM_DAGGER, RANDOM_ITEM_WEAPON, RANDOM_ITEM_WEAPON } }},
    {HOUSE_WEAPON_SHOP_TATALIA_1,           { ITEM_TREASURE_LEVEL_2, { RANDOM_ITEM_WEAPON,  RANDOM_ITEM_WEAPON, RANDOM_ITEM_WEAPON, RANDOM_ITEM_WEAPON } }},
    {HOUSE_WEAPON_SHOP_AVLEE,               { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_BOW,     RANDOM_ITEM_BOW,    RANDOM_ITEM_SPEAR,  RANDOM_ITEM_SPEAR } }},
    {HOUSE_WEAPON_SHOP_STONE_CITY,          { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_MACE,    RANDOM_ITEM_MACE,   RANDOM_ITEM_AXE,    RANDOM_ITEM_AXE } }},
    {HOUSE_WEAPON_SHOP_CASTLE_HARMONDALE,   { ITEM_TREASURE_LEVEL_2, { RANDOM_ITEM_SWORD,   RANDOM_ITEM_SWORD,  RANDOM_ITEM_DAGGER, RANDOM_ITEM_DAGGER } }},
    {HOUSE_WEAPON_SHOP_TATALIA_2,           { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_SWORD,   RANDOM_ITEM_SWORD,  RANDOM_ITEM_SPEAR,  RANDOM_ITEM_SPEAR } }},
    {HOUSE_13,                              { ITEM_TREASURE_LEVEL_2, { RANDOM_ITEM_STAFF,   RANDOM_ITEM_SPEAR,  RANDOM_ITEM_SPEAR,  RANDOM_ITEM_SPEAR } }},
    {HOUSE_14,                              { ITEM_TREASURE_LEVEL_2, { RANDOM_ITEM_MACE,    RANDOM_ITEM_AXE,    RANDOM_ITEM_MACE,   RANDOM_ITEM_CLUB } }}
}};

static constexpr IndexedArray<ITEM_VARIATION, HOUSE_FIRST_WEAPON_SHOP, HOUSE_LAST_WEAPON_SHOP> weaponShopVariationSpecial = {{
    {HOUSE_WEAPON_SHOP_EMERALD_ISLAND,      { ITEM_TREASURE_LEVEL_2, { RANDOM_ITEM_AXE,     RANDOM_ITEM_STAFF,  RANDOM_ITEM_WEAPON, RANDOM_ITEM_WEAPON } }},
    {HOUSE_WEAPON_SHOP_HARMONDALE,          { ITEM_TREASURE_LEVEL_2, { RANDOM_ITEM_SWORD,   RANDOM_ITEM_DAGGER, RANDOM_ITEM_MACE,   RANDOM_ITEM_WEAPON } }},
    {HOUSE_WEAPON_SHOP_ERATHIA,             { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_SWORD,   RANDOM_ITEM_DAGGER, RANDOM_ITEM_AXE,    RANDOM_ITEM_WEAPON } }},
    {HOUSE_WEAPON_SHOP_TULAREAN_FOREST,     { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_BOW,     RANDOM_ITEM_BOW,    RANDOM_ITEM_SPEAR,  RANDOM_ITEM_SPEAR } }},
    {HOUSE_WEAPON_SHOP_CELESTE,             { ITEM_TREASURE_LEVEL_5, { RANDOM_ITEM_SWORD,   RANDOM_ITEM_SPEAR,  RANDOM_ITEM_MACE,   RANDOM_ITEM_BOW } }},
    {HOUSE_WEAPON_SHOP_PIT,                 { ITEM_TREASURE_LEVEL_5, { RANDOM_ITEM_SWORD,   RANDOM_ITEM_SPEAR,  RANDOM_ITEM_MACE,   RANDOM_ITEM_BOW } }},
    {HOUSE_WEAPON_SHOP_MOUNT_NIGHON,        { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_STAFF,   RANDOM_ITEM_DAGGER, RANDOM_ITEM_WEAPON, RANDOM_ITEM_WEAPON } }},
    {HOUSE_WEAPON_SHOP_TATALIA_1,           { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_WEAPON,  RANDOM_ITEM_WEAPON, RANDOM_ITEM_WEAPON, RANDOM_ITEM_WEAPON } }},
    {HOUSE_WEAPON_SHOP_AVLEE,               { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_BOW,     RANDOM_ITEM_BOW,    RANDOM_ITEM_SPEAR,  RANDOM_ITEM_SPEAR } }},
    {HOUSE_WEAPON_SHOP_STONE_CITY,          { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_MACE,    RANDOM_ITEM_MACE,   RANDOM_ITEM_AXE,    RANDOM_ITEM_AXE } }},
    {HOUSE_WEAPON_SHOP_CASTLE_HARMONDALE,   { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_SWORD,   RANDOM_ITEM_SWORD,  RANDOM_ITEM_DAGGER, RANDOM_ITEM_DAGGER } }},
    {HOUSE_WEAPON_SHOP_TATALIA_2,           { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_DAGGER,  RANDOM_ITEM_DAGGER, RANDOM_ITEM_BOW,    RANDOM_ITEM_WEAPON } }},
    {HOUSE_13,                              { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_STAFF,   RANDOM_ITEM_SPEAR,  RANDOM_ITEM_SPEAR,  RANDOM_ITEM_SPEAR } }},
    {HOUSE_14,                              { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_MACE,    RANDOM_ITEM_AXE,    RANDOM_ITEM_MACE,   RANDOM_ITEM_CLUB } }}
}};

static constexpr IndexedArray<ITEM_VARIATION, HOUSE_FIRST_ARMOR_SHOP, HOUSE_LAST_ARMOR_SHOP> armorShopTopRowVariationStandard = {{
    {HOUSE_ARMOR_SHOP_EMERALD_ISLAND,       { ITEM_TREASURE_LEVEL_1, { RANDOM_ITEM_HELMET,  RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_HARMONDALE,           { ITEM_TREASURE_LEVEL_1, { RANDOM_ITEM_HELMET,  RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_ERATHIA,              { ITEM_TREASURE_LEVEL_2, { RANDOM_ITEM_HELMET,  RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_TULAREAN_FOREST,      { ITEM_TREASURE_LEVEL_2, { RANDOM_ITEM_HELMET,  RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_CELESTE,              { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_HELMET,  RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_PIT,                  { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_HELMET,  RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_MOUNT_NIGHON,         { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_HELMET,  RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_TATALIA_1,            { ITEM_TREASURE_LEVEL_2, { RANDOM_ITEM_HELMET,  RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_AVLEE,                { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_HELMET,  RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_STONE_CITY,           { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_HELMET,  RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_CASTLE_HARMONDALE,    { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_HELMET,  RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_TATALIA_2,            { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_HELMET,  RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_27,                              { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_HELMET,  RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_28,                              { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_HELMET,  RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }}
}};

static constexpr IndexedArray<ITEM_VARIATION, HOUSE_FIRST_ARMOR_SHOP, HOUSE_LAST_ARMOR_SHOP> armorShopBottomRowVariationStandard = {{
    {HOUSE_ARMOR_SHOP_EMERALD_ISLAND,       { ITEM_TREASURE_LEVEL_1, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_SHIELD } }},
    {HOUSE_ARMOR_SHOP_HARMONDALE,           { ITEM_TREASURE_LEVEL_1, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_SHIELD } }},
    {HOUSE_ARMOR_SHOP_ERATHIA,              { ITEM_TREASURE_LEVEL_2, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_PLATE_ARMOR } }},
    {HOUSE_ARMOR_SHOP_TULAREAN_FOREST,      { ITEM_TREASURE_LEVEL_2, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_CHAIN_ARMOR } }},
    {HOUSE_ARMOR_SHOP_CELESTE,              { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_PLATE_ARMOR,    RANDOM_ITEM_SHIELD } }},
    {HOUSE_ARMOR_SHOP_PIT,                  { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_PLATE_ARMOR,    RANDOM_ITEM_SHIELD } }},
    {HOUSE_ARMOR_SHOP_MOUNT_NIGHON,         { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_LEATHER_ARMOR } }},
    {HOUSE_ARMOR_SHOP_TATALIA_1,            { ITEM_TREASURE_LEVEL_2, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_SHIELD,         RANDOM_ITEM_SHIELD } }},
    {HOUSE_ARMOR_SHOP_AVLEE,                { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_CHAIN_ARMOR } }},
    {HOUSE_ARMOR_SHOP_STONE_CITY,           { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_CHAIN_ARMOR,     RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_PLATE_ARMOR } }},
    {HOUSE_ARMOR_SHOP_CASTLE_HARMONDALE,    { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_CHAIN_ARMOR } }},
    {HOUSE_ARMOR_SHOP_TATALIA_2,            { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_PLATE_ARMOR,     RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_SHIELD } }},
    {HOUSE_27,                              { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_PLATE_ARMOR,     RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_SHIELD } }},
    {HOUSE_28,                              { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_PLATE_ARMOR,     RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_SHIELD } }}
}};

static constexpr IndexedArray<ITEM_VARIATION, HOUSE_FIRST_ARMOR_SHOP, HOUSE_LAST_ARMOR_SHOP> armorShopTopRowVariationSpecial = {{
    {HOUSE_ARMOR_SHOP_EMERALD_ISLAND,       { ITEM_TREASURE_LEVEL_2, { RANDOM_ITEM_HELMET, RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_HARMONDALE,           { ITEM_TREASURE_LEVEL_2, { RANDOM_ITEM_HELMET, RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_ERATHIA,              { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_HELMET, RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_TULAREAN_FOREST,      { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_HELMET, RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_CELESTE,              { ITEM_TREASURE_LEVEL_5, { RANDOM_ITEM_HELMET, RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_PIT,                  { ITEM_TREASURE_LEVEL_5, { RANDOM_ITEM_HELMET, RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_MOUNT_NIGHON,         { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_HELMET, RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_TATALIA_1,            { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_HELMET, RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_AVLEE,                { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_HELMET, RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_STONE_CITY,           { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_HELMET, RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_CASTLE_HARMONDALE,    { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_HELMET, RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_ARMOR_SHOP_TATALIA_2,            { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_HELMET, RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_27,                              { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_HELMET, RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }},
    {HOUSE_28,                              { ITEM_TREASURE_LEVEL_5, { RANDOM_ITEM_HELMET, RANDOM_ITEM_HELMET, RANDOM_ITEM_GAUNTLETS, RANDOM_ITEM_GAUNTLETS } }}
}};

static constexpr IndexedArray<ITEM_VARIATION, HOUSE_FIRST_ARMOR_SHOP, HOUSE_LAST_ARMOR_SHOP> armorShopBottomRowVariationSpecial = {{
    {HOUSE_ARMOR_SHOP_EMERALD_ISLAND,       { ITEM_TREASURE_LEVEL_2, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_SHIELD } }},
    {HOUSE_ARMOR_SHOP_HARMONDALE,           { ITEM_TREASURE_LEVEL_2, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_SHIELD } }},
    {HOUSE_ARMOR_SHOP_ERATHIA,              { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_PLATE_ARMOR } }},
    {HOUSE_ARMOR_SHOP_TULAREAN_FOREST,      { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_CHAIN_ARMOR} }},
    {HOUSE_ARMOR_SHOP_CELESTE,              { ITEM_TREASURE_LEVEL_5, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_PLATE_ARMOR,    RANDOM_ITEM_SHIELD } }},
    {HOUSE_ARMOR_SHOP_PIT,                  { ITEM_TREASURE_LEVEL_5, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_PLATE_ARMOR,    RANDOM_ITEM_SHIELD } }},
    {HOUSE_ARMOR_SHOP_MOUNT_NIGHON,         { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_LEATHER_ARMOR } }},
    {HOUSE_ARMOR_SHOP_TATALIA_1,            { ITEM_TREASURE_LEVEL_3, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_SHIELD,         RANDOM_ITEM_SHIELD } }},
    {HOUSE_ARMOR_SHOP_AVLEE,                { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_PLATE_ARMOR } }},
    {HOUSE_ARMOR_SHOP_STONE_CITY,           { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_CHAIN_ARMOR,     RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_PLATE_ARMOR,    RANDOM_ITEM_SHIELD } }},
    {HOUSE_ARMOR_SHOP_CASTLE_HARMONDALE,    { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_LEATHER_ARMOR,   RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_LEATHER_ARMOR,  RANDOM_ITEM_CHAIN_ARMOR} }},
    {HOUSE_ARMOR_SHOP_TATALIA_2,            { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_CHAIN_ARMOR,     RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_CHAIN_ARMOR,    RANDOM_ITEM_CHAIN_ARMOR} }},
    {HOUSE_27,                              { ITEM_TREASURE_LEVEL_4, { RANDOM_ITEM_SHIELD,          RANDOM_ITEM_SHIELD,         RANDOM_ITEM_SHIELD,         RANDOM_ITEM_SHIELD } }},
    {HOUSE_28,                              { ITEM_TREASURE_LEVEL_5, { RANDOM_ITEM_PLATE_ARMOR,     RANDOM_ITEM_PLATE_ARMOR,    RANDOM_ITEM_PLATE_ARMOR,    RANDOM_ITEM_PLATE_ARMOR } }}
}};

static constexpr IndexedArray<ItemTreasureLevel, HOUSE_FIRST_MAGIC_SHOP, HOUSE_LAST_MAGIC_SHOP> magicShopVariationStandard = {{
    {HOUSE_MAGIC_SHOP_EMERALD_ISLAND,       ITEM_TREASURE_LEVEL_1},
    {HOUSE_MAGIC_SHOP_HARMONDALE,           ITEM_TREASURE_LEVEL_1},
    {HOUSE_MAGIC_SHOP_ERATHIA,              ITEM_TREASURE_LEVEL_2},
    {HOUSE_MAGIC_SHOP_TULAREAN_FOREST,      ITEM_TREASURE_LEVEL_2},
    {HOUSE_MAGIC_SHOP_DEYJA,                ITEM_TREASURE_LEVEL_4},
    {HOUSE_MAGIC_SHOP_BRACADA_DESERT,       ITEM_TREASURE_LEVEL_4},
    {HOUSE_MAGIC_SHOP_CELESTE,              ITEM_TREASURE_LEVEL_3},
    {HOUSE_MAGIC_SHOP_PIT,                  ITEM_TREASURE_LEVEL_2},
    {HOUSE_MAGIC_SHOP_MOUNT_NIGHON,         ITEM_TREASURE_LEVEL_2},
    {HOUSE_MAGIC_SHOP_STONE_CITY,           ITEM_TREASURE_LEVEL_2},
    {HOUSE_39,                              ITEM_TREASURE_LEVEL_2},
    {HOUSE_40,                              ITEM_TREASURE_LEVEL_2},
    {HOUSE_41,                              ITEM_TREASURE_LEVEL_2}
}};

static constexpr IndexedArray<ItemTreasureLevel, HOUSE_FIRST_MAGIC_SHOP, HOUSE_LAST_MAGIC_SHOP> magicShopVariationSpecial = {{
    {HOUSE_MAGIC_SHOP_EMERALD_ISLAND,       ITEM_TREASURE_LEVEL_2},
    {HOUSE_MAGIC_SHOP_HARMONDALE,           ITEM_TREASURE_LEVEL_2},
    {HOUSE_MAGIC_SHOP_ERATHIA,              ITEM_TREASURE_LEVEL_3},
    {HOUSE_MAGIC_SHOP_TULAREAN_FOREST,      ITEM_TREASURE_LEVEL_3},
    {HOUSE_MAGIC_SHOP_DEYJA,                ITEM_TREASURE_LEVEL_5},
    {HOUSE_MAGIC_SHOP_BRACADA_DESERT,       ITEM_TREASURE_LEVEL_5},
    {HOUSE_MAGIC_SHOP_CELESTE,              ITEM_TREASURE_LEVEL_4},
    {HOUSE_MAGIC_SHOP_PIT,                  ITEM_TREASURE_LEVEL_3},
    {HOUSE_MAGIC_SHOP_MOUNT_NIGHON,         ITEM_TREASURE_LEVEL_3},
    {HOUSE_MAGIC_SHOP_STONE_CITY,           ITEM_TREASURE_LEVEL_3},
    {HOUSE_39,                              ITEM_TREASURE_LEVEL_3},
    {HOUSE_40,                              ITEM_TREASURE_LEVEL_3},
    {HOUSE_41,                              ITEM_TREASURE_LEVEL_3}
}};

static constexpr IndexedArray<ItemTreasureLevel, HOUSE_FIRST_ALCHEMY_SHOP, HOUSE_LAST_ALCHEMY_SHOP> alchemyShopVariationStandard = {{
    {HOUSE_ALCHEMY_SHOP_EMERALD_ISLAND,     ITEM_TREASURE_LEVEL_1},
    {HOUSE_ALCHEMY_SHOP_HARMONDALE,         ITEM_TREASURE_LEVEL_1},
    {HOUSE_ALCHEMY_SHOP_ERATHIA,            ITEM_TREASURE_LEVEL_2},
    {HOUSE_ALCHEMY_SHOP_TULAREAN_FOREST,    ITEM_TREASURE_LEVEL_2},
    {HOUSE_ALCHEMY_SHOP_DEYJA,              ITEM_TREASURE_LEVEL_3},
    {HOUSE_ALCHEMY_SHOP_BRACADA_DESERT,     ITEM_TREASURE_LEVEL_3},
    {HOUSE_ALCHEMY_SHOP_CELESTE,            ITEM_TREASURE_LEVEL_4},
    {HOUSE_ALCHEMY_SHOP_PIT,                ITEM_TREASURE_LEVEL_4},
    {HOUSE_ALCHEMY_SHOP_STONE_CITY,         ITEM_TREASURE_LEVEL_2},
    {HOUSE_ALCHEMY_SHOP_CASTLE_HARMONDALE,  ITEM_TREASURE_LEVEL_2},
    {HOUSE_52,                              ITEM_TREASURE_LEVEL_2},
    {HOUSE_53,                              ITEM_TREASURE_LEVEL_2}
}};

static constexpr IndexedArray<ItemTreasureLevel, HOUSE_FIRST_ALCHEMY_SHOP, HOUSE_LAST_ALCHEMY_SHOP> alchemyShopVariationSpecial = {{
    {HOUSE_ALCHEMY_SHOP_EMERALD_ISLAND,     ITEM_TREASURE_LEVEL_2},
    {HOUSE_ALCHEMY_SHOP_HARMONDALE,         ITEM_TREASURE_LEVEL_2},
    {HOUSE_ALCHEMY_SHOP_ERATHIA,            ITEM_TREASURE_LEVEL_3},
    {HOUSE_ALCHEMY_SHOP_TULAREAN_FOREST,    ITEM_TREASURE_LEVEL_3},
    {HOUSE_ALCHEMY_SHOP_DEYJA,              ITEM_TREASURE_LEVEL_4},
    {HOUSE_ALCHEMY_SHOP_BRACADA_DESERT,     ITEM_TREASURE_LEVEL_4},
    {HOUSE_ALCHEMY_SHOP_CELESTE,            ITEM_TREASURE_LEVEL_5},
    {HOUSE_ALCHEMY_SHOP_PIT,                ITEM_TREASURE_LEVEL_5},
    {HOUSE_ALCHEMY_SHOP_STONE_CITY,         ITEM_TREASURE_LEVEL_3},
    {HOUSE_ALCHEMY_SHOP_CASTLE_HARMONDALE,  ITEM_TREASURE_LEVEL_2},
    {HOUSE_52,                              ITEM_TREASURE_LEVEL_2},
    {HOUSE_53,                              ITEM_TREASURE_LEVEL_2}
}};

GraphicsImage *shop_ui_background = nullptr;

std::array<GraphicsImage *, 12> shop_ui_items_in_store;
std::array<int, 6> weaponYPos;

bool isStealingModeActive() {
    return keyboardInputHandler->IsStealingToggled() && pParty->activeCharacter().CanSteal();
}

void addUniqueItemClasses(const ITEM_VARIATION &variation, std::vector<RandomItemType> &set) {
    for (RandomItemType itemClass : variation.itemClass) {
        if (std::find(set.begin(), set.end(), itemClass) == set.end()) {
            set.push_back(itemClass);
        }
    }
}

DialogueId getSkillLearnDualogueForItemClass(RandomItemType itemClass) {
    switch (itemClass) {
      case RANDOM_ITEM_SWORD:
        return DIALOGUE_LEARN_SWORD;
      case RANDOM_ITEM_DAGGER:
        return DIALOGUE_LEARN_DAGGER;
      case RANDOM_ITEM_AXE:
        return DIALOGUE_LEARN_AXE;
      case RANDOM_ITEM_SPEAR:
        return DIALOGUE_LEARN_SPEAR;
      case RANDOM_ITEM_BOW:
        return DIALOGUE_LEARN_BOW;
      case RANDOM_ITEM_MACE:
        return DIALOGUE_LEARN_MACE;
      case RANDOM_ITEM_STAFF:
        return DIALOGUE_LEARN_STAFF;
      case RANDOM_ITEM_LEATHER_ARMOR:
        return DIALOGUE_LEARN_LEATHER;
      case RANDOM_ITEM_CHAIN_ARMOR:
        return DIALOGUE_LEARN_CHAIN;
      case RANDOM_ITEM_PLATE_ARMOR:
        return DIALOGUE_LEARN_PLATE;
      case RANDOM_ITEM_SHIELD:
        return DIALOGUE_LEARN_SHIELD;
      default:
        return DIALOGUE_NULL;
    }
}

void GUIWindow_Shop::mainDialogue() {
    if (!checkIfPlayerCanInteract()) {
        return;
    }

    std::vector<std::string> optionsText = {localization->GetString(LSTR_STANDARD), localization->GetString(LSTR_SPECIAL),
                                            localization->GetString(LSTR_DISPLAY), localization->GetString(LSTR_LEARN_SKILLS)};

    drawOptions(optionsText, colorTable.Sunflower);
}

void GUIWindow_Shop::displayEquipmentDialogue() {
    draw_leather();
    CharacterUI_InventoryTab_Draw(&pParty->activeCharacter(), true);

    std::vector<std::string> optionsText = {localization->GetString(LSTR_SELL), localization->GetString(LSTR_IDENTIFY)};

    if (buildingType() != BUILDING_ALCHEMY_SHOP) {
        optionsText.push_back(localization->GetString(LSTR_REPAIR));
    }

    drawOptions(optionsText, colorTable.Sunflower);
}

void GUIWindow_Shop::sellDialogue() {
    // TODO(pskelton): extract common code around shop item picking
    GUIWindow dialogwin = *this;
    dialogwin.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialogwin.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialogwin.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    draw_leather();
    CharacterUI_InventoryTab_Draw(&pParty->activeCharacter(), true);

    if (checkIfPlayerCanInteract()) {
        engine->_statusBar->drawForced(localization->GetString(LSTR_SELECT_ITEM_TO_SELL), colorTable.White);

        Pointi pt = dialogwin.mouse->GetCursorPos();

        int invindex = ((pt.x - 14) / 32) + 14 * ((pt.y - 17) / 32);
        if (pt.x <= 13 || pt.x >= 462)
            return;

        int pItemID = pParty->activeCharacter().GetItemListAtInventoryIndex(invindex);
        if (pItemID) {
            ItemGen *item = &pParty->activeCharacter().pInventoryItemList[pItemID - 1];
            MerchantPhrase phrases_id = pParty->activeCharacter().SelectPhrasesTransaction(item, buildingType(), houseId(), SHOP_SCREEN_SELL);
            std::string str = BuildDialogueString(pMerchantsSellPhrases[phrases_id], pParty->activeCharacterIndex() - 1, houseNpcs[currentHouseNpc].npc, item, houseId(), SHOP_SCREEN_SELL);
            int vertMargin = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - assets->pFontArrus->CalcTextHeight(str, dialogwin.uFrameWidth, 0)) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
            dialogwin.DrawTitleText(assets->pFontArrus.get(), 0, vertMargin, colorTable.White, str, 3);
        }
    }
}

void GUIWindow_Shop::identifyDialogue() {
    // TODO(pskelton): extract common code around shop item picking
    GUIWindow dialogwin = *this;
    dialogwin.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialogwin.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialogwin.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    draw_leather();
    CharacterUI_InventoryTab_Draw(&pParty->activeCharacter(), true);

    if (checkIfPlayerCanInteract()) {
        engine->_statusBar->drawForced(localization->GetString(LSTR_SELECT_ITEM_TO_IDENTIFY), colorTable.White);

        Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();

        int invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
        if (pt.x <= 13 || pt.x >= 462)
            return;

        int pItemID = pParty->activeCharacter().GetItemListAtInventoryIndex(invindex);

        if (pItemID) {
            ItemGen *item = &pParty->activeCharacter().pInventoryItemList[pItemID - 1];

            std::string str;
            if (!item->IsIdentified()) {
                MerchantPhrase phrases_id = pParty->activeCharacter().SelectPhrasesTransaction(item, buildingType(), houseId(), SHOP_SCREEN_IDENTIFY);
                str = BuildDialogueString(pMerchantsIdentifyPhrases[phrases_id], pParty->activeCharacterIndex() - 1, houseNpcs[currentHouseNpc].npc, item, houseId(), SHOP_SCREEN_IDENTIFY);
            } else {
                str = BuildDialogueString("%24", pParty->activeCharacterIndex() - 1, houseNpcs[currentHouseNpc].npc, item, houseId(), SHOP_SCREEN_IDENTIFY);
            }

            int vertMargin = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - assets->pFontArrus->CalcTextHeight(str, dialogwin.uFrameWidth, 0)) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
            dialogwin.DrawTitleText(assets->pFontArrus.get(), 0, vertMargin, colorTable.White, str, 3);
        }
    }
}

void GUIWindow_Shop::repairDialogue() {
    // TODO(pskelton): extract common code around shop item picking
    GUIWindow dialogwin = *this;
    dialogwin.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialogwin.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialogwin.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    draw_leather();
    CharacterUI_InventoryTab_Draw(&pParty->activeCharacter(), true);

    if (checkIfPlayerCanInteract()) {
        engine->_statusBar->drawForced(localization->GetString(LSTR_SELECT_ITEM_TO_REPAIR), colorTable.White);

        Pointi pt = dialogwin.mouse->GetCursorPos();

        int invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
        if (pt.x <= 13 || pt.x >= 462)
            return;

        int pItemID = pParty->activeCharacter().GetItemListAtInventoryIndex(invindex);
        if (pItemID == 0)
            return;

        if (pParty->activeCharacter().pInventoryItemList[pItemID - 1].uAttributes & ITEM_BROKEN) {
            ItemGen *item = &pParty->activeCharacter().pInventoryItemList[pItemID - 1];
            MerchantPhrase phrases_id = pParty->activeCharacter().SelectPhrasesTransaction(item, buildingType(), houseId(), SHOP_SCREEN_REPAIR);
            std::string str = BuildDialogueString(pMerchantsRepairPhrases[phrases_id], pParty->activeCharacterIndex() - 1, houseNpcs[currentHouseNpc].npc, item, houseId(), SHOP_SCREEN_REPAIR);
            int vertMargin = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - assets->pFontArrus->CalcTextHeight(str, dialogwin.uFrameWidth, 0)) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
            dialogwin.DrawTitleText(assets->pFontArrus.get(), 0, vertMargin, colorTable.White, str, 3);
        }
    }
}

void GUIWindow_WeaponShop::shopWaresDialogue(bool isSpecial) {
    // TODO(pskelton): extract common code around shop item picking
    GUIWindow dialogwin = *this;
    dialogwin.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialogwin.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialogwin.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
    int item_X = 0;

    for (int i = 0; i < 6; ++i) {
        bool itemPresent = (isSpecial ? pParty->specialItemsInShops[houseId()][i].uItemID : pParty->standartItemsInShops[houseId()][i].uItemID) != ITEM_NULL;
        if (itemPresent) {
            render->DrawTextureNew(((60 - (shop_ui_items_in_store[i]->width() / 2)) + item_X) / 640.0f, (weaponYPos[i] + 30) / 480.0f, shop_ui_items_in_store[i]);
        }

        item_X += 70;
    }

    if (checkIfPlayerCanInteract()) {
        int item_num = 0;
        for (int i = 0; i < 6; ++i) {
            item_num += (isSpecial ? pParty->specialItemsInShops[houseId()][i].uItemID : pParty->standartItemsInShops[houseId()][i].uItemID) != ITEM_NULL;
        }

        if (isStealingModeActive()) {
            engine->_statusBar->drawForced(localization->GetString(LSTR_STEAL_ITEM), colorTable.White);
        } else {
            engine->_statusBar->drawForced(localization->GetString(LSTR_SELECT_ITEM_TO_BUY), colorTable.White);
        }

        if (item_num) {
            Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();
            ItemGen *item;

            int testx = (pt.x - 30) / 70;
            if (testx >= 0 && testx < 6) {  // testx limits check
                if (isSpecial) {
                    item = &pParty->specialItemsInShops[houseId()][testx];
                } else {
                    item = &pParty->standartItemsInShops[houseId()][testx];
                }

                if (item->uItemID != ITEM_NULL) {  // item picking
                    int testpos = ((60 - (shop_ui_items_in_store[testx]->width() / 2)) + testx * 70);

                    if (pt.x >= testpos && pt.x < (testpos + (shop_ui_items_in_store[testx]->width()))) {
                        if (pt.y >= weaponYPos[testx] + 30 && pt.y < (weaponYPos[testx] + 30 + (shop_ui_items_in_store[testx]->height()))) {
                            std::string str;
                            if (!isStealingModeActive()) {
                                MerchantPhrase phrase = pParty->activeCharacter().SelectPhrasesTransaction(item, BUILDING_WEAPON_SHOP, houseId(), SHOP_SCREEN_BUY);
                                str = BuildDialogueString(pMerchantsBuyPhrases[phrase], pParty->activeCharacterIndex() - 1, houseNpcs[currentHouseNpc].npc, item, houseId(), SHOP_SCREEN_BUY);
                            } else {
                                str = BuildDialogueString(localization->GetString(LSTR_STEAL_ITEM_FMT), pParty->activeCharacterIndex() - 1, houseNpcs[currentHouseNpc].npc, item, houseId(), SHOP_SCREEN_BUY);
                            }
                            int vertMargin = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - assets->pFontArrus->CalcTextHeight(str, dialogwin.uFrameWidth, 0)) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
                            dialogwin.DrawTitleText(assets->pFontArrus.get(), 0, vertMargin, colorTable.White, str, 3);
                        }
                    }
                }
            }
        } else {
            dialogwin.DrawShops_next_generation_time_string(pParty->PartyTimes.shopNextRefreshTime[houseId()] - pParty->GetPlayingTime());
        }
    }
}

void GUIWindow_ArmorShop::shopWaresDialogue(bool isSpecial) {
    // TODO(pskelton): extract common code around shop item picking
    GUIWindow dialogwin = *this;
    dialogwin.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialogwin.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialogwin.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);
    int item_x = 0;

    for (int i = 0; i < 8; ++i) {
        bool itemPresent = (isSpecial ? pParty->specialItemsInShops[houseId()][i].uItemID : pParty->standartItemsInShops[houseId()][i].uItemID) != ITEM_NULL;

        if (itemPresent) {
            if (i >= 4) {
                render->DrawTextureNew(((90 - (shop_ui_items_in_store[i]->width() / 2)) + item_x - 420) / 640.0f, 126 / 480.0f, shop_ui_items_in_store[i]);
            } else {
                int x = ((86 - (shop_ui_items_in_store[i]->width() / 2)) + item_x);
                int y = (98 - (shop_ui_items_in_store[i]->height()));
                render->DrawTextureNew(x / 640.0f, y / 480.0f, shop_ui_items_in_store[i]);
            }
        }
        item_x += 105;
    }

    if (checkIfPlayerCanInteract()) {
        int pItemCount = 0;
        for (int i = 0; i < 6; ++i) {
            pItemCount += (isSpecial ? pParty->specialItemsInShops[houseId()][i].uItemID : pParty->standartItemsInShops[houseId()][i].uItemID) != ITEM_NULL;
        }

        if (isStealingModeActive()) {
            engine->_statusBar->drawForced(localization->GetString(LSTR_STEAL_ITEM), colorTable.White);
        } else {
            engine->_statusBar->drawForced(localization->GetString(LSTR_SELECT_ITEM_TO_BUY), colorTable.White);
        }

        if (pItemCount) {
            Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();

            // testx limits check
            int testx = (pt.x - 40) / 105;
            if (testx >= 0 && testx < 4) {
                if (pt.y >= 126) {
                    testx += 4;
                }

                ItemGen *item;
                if (isSpecial) {
                    item = &pParty->specialItemsInShops[houseId()][testx];
                } else {
                    item = &pParty->standartItemsInShops[houseId()][testx];
                }

                if (item->uItemID != ITEM_NULL) {
                    int testpos;
                    if (testx >= 4) {
                        testpos = ((90 - (shop_ui_items_in_store[testx]->width() / 2)) + (testx * 105) - 420);
                    } else {
                        testpos = ((86 - (shop_ui_items_in_store[testx]->width() / 2)) + testx * 105);
                    }

                    if (pt.x >= testpos &&
                        pt.x <= testpos + (shop_ui_items_in_store[testx]->width())) {
                        if ((pt.y >= 126 && pt.y < (126 + (shop_ui_items_in_store[testx]->height()))) ||
                            (pt.y <= 98 && pt.y >= (98 - (shop_ui_items_in_store[testx]->height())))) {
                            // y is 126 to 126 + height low or 98-height to 98

                            std::string str;
                            if (!isStealingModeActive()) {
                                MerchantPhrase phrase = pParty->activeCharacter().SelectPhrasesTransaction(item, buildingType(), houseId(), SHOP_SCREEN_BUY);
                                str = BuildDialogueString(pMerchantsBuyPhrases[phrase], pParty->activeCharacterIndex() - 1, houseNpcs[currentHouseNpc].npc, item, houseId(), SHOP_SCREEN_BUY);
                            } else {
                                str = BuildDialogueString(localization->GetString(LSTR_STEAL_ITEM_FMT), pParty->activeCharacterIndex() - 1, houseNpcs[currentHouseNpc].npc, item, houseId(), SHOP_SCREEN_BUY);
                            }
                            int vertMargin = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - assets->pFontArrus->CalcTextHeight(str, dialogwin.uFrameWidth, 0)) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
                            dialogwin.DrawTitleText(assets->pFontArrus.get(), 0, vertMargin, colorTable.White, str, 3);
                        }
                    }
                }
            }
        } else {
            dialogwin.DrawShops_next_generation_time_string(pParty->PartyTimes.shopNextRefreshTime[houseId()] - pParty->GetPlayingTime());
        }
    }
}

void GUIWindow_MagicAlchemyShop::shopWaresDialogue(bool isSpecial) {
    // TODO(pskelton): extract common code around shop item picking
    GUIWindow dialogwin = *this;
    dialogwin.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialogwin.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialogwin.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, shop_ui_background);

    for (int i = 0; i < 12; ++i) {
        bool itemPresent = (isSpecial ? pParty->specialItemsInShops[houseId()][i].uItemID : pParty->standartItemsInShops[houseId()][i].uItemID) != ITEM_NULL;
        int itemx, itemy;

        if (itemPresent) {
            if (i >= 6) {
                itemy = 308 - (shop_ui_items_in_store[i]->height());
                if (itemy < 0)
                    itemy = 0;
                itemx = 75 * i - (shop_ui_items_in_store[i]->width() / 2) + 40 - 450;
            } else {
                itemy = 152 - (shop_ui_items_in_store[i]->height());
                if (itemy < 0)
                    itemy = 0;
                itemx = 75 * i - (shop_ui_items_in_store[i]->width() / 2) + 40;
            }

            if (i == 0 || i == 6) {
                if (itemx < 18)
                    itemx = 18;
            }

            if (i == 5 || i == 11) {
                if (itemx > 457 - (shop_ui_items_in_store[i]->width())) {
                    itemx = 457 - (shop_ui_items_in_store[i]->width());
                }
            }

            render->DrawTextureNew(itemx / 640.0f, itemy / 480.0f, shop_ui_items_in_store[i]);
        }
    }

    if (checkIfPlayerCanInteract()) {
        int item_num = 0;

        for (int i = 0; i < 12; ++i) {
            item_num += (isSpecial ? pParty->specialItemsInShops[houseId()][i].uItemID : pParty->standartItemsInShops[houseId()][i].uItemID) != ITEM_NULL;
        }

        if (isStealingModeActive()) {
            engine->_statusBar->drawForced(localization->GetString(LSTR_STEAL_ITEM), colorTable.White);
        } else {
            engine->_statusBar->drawForced(localization->GetString(LSTR_SELECT_ITEM_TO_BUY), colorTable.White);
        }

        if (item_num) {
            Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();

            // testx limits check
            int testx = (pt.x) / 75;
            if (testx >= 0 && testx < 6) {
                if (pt.y > 152) {
                    testx += 6;
                }

                ItemGen *item;
                if (isSpecial) {
                    item = &pParty->specialItemsInShops[houseId()][testx];
                } else {
                    item = &pParty->standartItemsInShops[houseId()][testx];
                }

                if (item->uItemID != ITEM_NULL) {
                    int testpos;
                    if (pt.y > 152) {
                        testpos = 75 * testx - (shop_ui_items_in_store[testx]->width() / 2) + 40 - 450;
                    } else {
                        testpos = 75 * testx - (shop_ui_items_in_store[testx]->width() / 2) + 40;
                    }

                    if (pt.x >= testpos &&
                        pt.x <= testpos + (shop_ui_items_in_store[testx]->width())) {
                        if ((pt.y <= 308 && pt.y >= (308 - (shop_ui_items_in_store[testx]->height()))) ||
                            (pt.y <= 152 && pt.y >= (152 - (shop_ui_items_in_store[testx]->height())))) {
                            // y is 152-h to 152 or 308-height to 308

                            std::string str;
                            if (!isStealingModeActive()) {
                                MerchantPhrase phrase = pParty->activeCharacter().SelectPhrasesTransaction(item, buildingType(), houseId(), SHOP_SCREEN_BUY);
                                str = BuildDialogueString(pMerchantsBuyPhrases[phrase], pParty->activeCharacterIndex() - 1, houseNpcs[currentHouseNpc].npc, item, houseId(), SHOP_SCREEN_BUY);
                            } else {
                                str = BuildDialogueString(localization->GetString(LSTR_STEAL_ITEM_FMT), pParty->activeCharacterIndex() - 1, houseNpcs[currentHouseNpc].npc, item, houseId(), SHOP_SCREEN_BUY);
                            }
                            int vertMargin = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - assets->pFontArrus->CalcTextHeight(str, dialogwin.uFrameWidth, 0)) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
                            dialogwin.DrawTitleText(assets->pFontArrus.get(), 0, vertMargin, colorTable.White, str, 3);
                        }
                    }
                }
            }
        } else {
            dialogwin.DrawShops_next_generation_time_string(pParty->PartyTimes.shopNextRefreshTime[houseId()] - pParty->GetPlayingTime());
        }
    }
}

void GUIWindow_WeaponShop::generateShopItems(bool isSpecial) {
    std::array<ItemGen, 12> &itemArray = isSpecial ? pParty->specialItemsInShops[houseId()] : pParty->standartItemsInShops[houseId()];
    const ITEM_VARIATION variation = isSpecial ? weaponShopVariationSpecial[houseId()] : weaponShopVariationStandard[houseId()];

    for (int i = 0; i < itemAmountInShop[buildingType()]; i++) {
        RandomItemType itemClass = variation.itemClass[grng->random(4)];
        pItemTable->generateItem(variation.treasureLevel, itemClass, &itemArray[i]);
        itemArray[i].SetIdentified();
    }

    pParty->InTheShopFlags[houseId()] = 0;
}

void GUIWindow_ArmorShop::generateShopItems(bool isSpecial) {
    std::array<ItemGen, 12> &itemArray = isSpecial ? pParty->specialItemsInShops[houseId()] : pParty->standartItemsInShops[houseId()];
    const ITEM_VARIATION variationTop = isSpecial ? armorShopTopRowVariationSpecial[houseId()] : armorShopTopRowVariationStandard[houseId()];
    const ITEM_VARIATION variationBottom = isSpecial ? armorShopBottomRowVariationSpecial[houseId()] : armorShopBottomRowVariationStandard[houseId()];

    for (int i = 0; i < itemAmountInShop[buildingType()]; i++) {
        RandomItemType itemClass;
        ItemTreasureLevel treasureLvl;

        if (i >= 4) {
            treasureLvl = variationBottom.treasureLevel;
            itemClass = variationBottom.itemClass[grng->random(4)];
        } else {
            treasureLvl = variationTop.treasureLevel;
            itemClass = variationTop.itemClass[grng->random(4)];
        }
        pItemTable->generateItem(treasureLvl, itemClass, &itemArray[i]);
        itemArray[i].SetIdentified();
    }

    pParty->InTheShopFlags[houseId()] = 0;
}

void GUIWindow_MagicShop::generateShopItems(bool isSpecial) {
    std::array<ItemGen, 12> &itemArray = isSpecial ? pParty->specialItemsInShops[houseId()] : pParty->standartItemsInShops[houseId()];
    ItemTreasureLevel treasureLvl = isSpecial ? magicShopVariationSpecial[houseId()] : magicShopVariationStandard[houseId()];

    for (int i = 0; i < itemAmountInShop[buildingType()]; i++) {
        pItemTable->generateItem(treasureLvl, RANDOM_ITEM_MICS, &itemArray[i]);
        itemArray[i].SetIdentified();
    }

    pParty->InTheShopFlags[houseId()] = 0;
}

void GUIWindow_AlchemyShop::generateShopItems(bool isSpecial) {
    std::array<ItemGen, 12> &itemArray = isSpecial ? pParty->specialItemsInShops[houseId()] : pParty->standartItemsInShops[houseId()];
    ItemTreasureLevel treasureLvl = isSpecial ? alchemyShopVariationSpecial[houseId()] : alchemyShopVariationStandard[houseId()];
    RandomItemType bottomRowItemClass = isSpecial ? RANDOM_ITEM_POTION : RANDOM_ITEM_REAGENT;

    for (int i = 0; i < itemAmountInShop[buildingType()]; i++) {
        if (i < 6) {
            itemArray[i].Reset();
            if (isSpecial) {
                itemArray[i].uItemID = grng->randomSample(allRecipeScrolls());
            } else {
                itemArray[i].uItemID = ITEM_POTION_BOTTLE;
            }
        } else {
            pItemTable->generateItem(treasureLvl, bottomRowItemClass, &itemArray[i]);
            itemArray[i].SetIdentified();
        }
    }

    pParty->InTheShopFlags[houseId()] = 0;
}

std::vector<DialogueId> GUIWindow_WeaponShop::listShopLearnableSkills() {
    std::vector<RandomItemType> itemClasses;
    std::vector<DialogueId> skillsOptions;

    addUniqueItemClasses(weaponShopVariationStandard[houseId()], itemClasses);
    addUniqueItemClasses(weaponShopVariationSpecial[houseId()], itemClasses);

    for (RandomItemType itemClass : itemClasses) {
        DialogueId dialogue = getSkillLearnDualogueForItemClass(itemClass);
        if (dialogue != DIALOGUE_NULL) {
            skillsOptions.push_back(dialogue);
        }
    }

    return skillsOptions;
}

std::vector<DialogueId> GUIWindow_ArmorShop::listShopLearnableSkills() {
    std::vector<RandomItemType> itemClasses;
    std::vector<DialogueId> skillsOptions;

    addUniqueItemClasses(armorShopTopRowVariationStandard[houseId()], itemClasses);
    addUniqueItemClasses(armorShopBottomRowVariationStandard[houseId()], itemClasses);
    addUniqueItemClasses(armorShopTopRowVariationSpecial[houseId()], itemClasses);
    addUniqueItemClasses(armorShopBottomRowVariationSpecial[houseId()], itemClasses);

    for (RandomItemType itemClass : itemClasses) {
        DialogueId dialogue = getSkillLearnDualogueForItemClass(itemClass);
        if (dialogue != DIALOGUE_NULL) {
            skillsOptions.push_back(dialogue);
        }
    }

    return skillsOptions;
}

std::vector<DialogueId> GUIWindow_MagicShop::listShopLearnableSkills() {
    return {DIALOGUE_LEARN_ITEM_ID, DIALOGUE_LEARN_REPAIR};
}

std::vector<DialogueId> GUIWindow_AlchemyShop::listShopLearnableSkills() {
    return {DIALOGUE_LEARN_ALCHEMY, DIALOGUE_LEARN_MONSTER_ID};
}

void GUIWindow_Shop::houseSpecificDialogue() {
    switch (_currentDialogue) {
      case DIALOGUE_MAIN:
        mainDialogue();
        break;
      case DIALOGUE_SHOP_BUY_STANDARD:
        shopWaresDialogue(false);
        break;
      case DIALOGUE_SHOP_BUY_SPECIAL:
        shopWaresDialogue(true);
        break;
      case DIALOGUE_SHOP_DISPLAY_EQUIPMENT:
        displayEquipmentDialogue();
        break;
      case DIALOGUE_SHOP_SELL:
        sellDialogue();
        break;
      case DIALOGUE_SHOP_IDENTIFY:
        identifyDialogue();
        break;
      case DIALOGUE_SHOP_REPAIR:
        repairDialogue();
        break;
      case DIALOGUE_LEARN_SKILLS:
        learnSkillsDialogue(colorTable.Sunflower);
        break;
      default:
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
        break;
    }
}

void GUIWindow_Shop::houseDialogueOptionSelected(DialogueId option) {
    _currentDialogue = option;
    if (option == DIALOGUE_SHOP_BUY_STANDARD || option == DIALOGUE_SHOP_BUY_SPECIAL) {
        if (pParty->PartyTimes.shopNextRefreshTime[houseId()] < pParty->GetPlayingTime()) {
            generateShopItems(false);
            generateShopItems(true);
            Time nextGenTime = pParty->GetPlayingTime() + Duration::fromDays(buildingTable[houseId()].generation_interval_days);
            pParty->PartyTimes.shopNextRefreshTime[houseId()] = nextGenTime;
        }

        BuildingType shopType = buildingType();
        const std::array<ItemGen, 12> &itemArray = (option == DIALOGUE_SHOP_BUY_STANDARD) ? pParty->standartItemsInShops[houseId()] : pParty->specialItemsInShops[houseId()];
        for (int i = 0; i < itemAmountInShop[shopType]; ++i) {
            if (itemArray[i].uItemID != ITEM_NULL) {
                shop_ui_items_in_store[i] = assets->getImage_ColorKey(itemArray[i].GetIconName());
            }
        }
        if (shopType == BUILDING_WEAPON_SHOP) {
            for (int i = 0; i < itemAmountInShop[shopType]; ++i) {
                if (itemArray[i].uItemID != ITEM_NULL) {
                    // Note that we're using grng here for a reason - we want recorded mouse clicks to work.
                    weaponYPos[i] = grng->random(300 - (shop_ui_items_in_store[i]->height()));
                }
            }
        }
    } else if (option == DIALOGUE_SHOP_SELL || option == DIALOGUE_SHOP_IDENTIFY || option == DIALOGUE_SHOP_REPAIR) {
        pParty->placeHeldItemInInventoryOrDrop();
    } else if (IsSkillLearningDialogue(option)) {
        learnSelectedSkill(GetLearningDialogueSkill(option));
    }
}

std::vector<DialogueId> GUIWindow_Shop::listDialogueOptions() {
    switch (_currentDialogue) {
      case DIALOGUE_MAIN:
        return {DIALOGUE_SHOP_BUY_STANDARD, DIALOGUE_SHOP_BUY_SPECIAL, DIALOGUE_SHOP_DISPLAY_EQUIPMENT, DIALOGUE_LEARN_SKILLS};
      case DIALOGUE_SHOP_DISPLAY_EQUIPMENT:
        return {DIALOGUE_SHOP_SELL, DIALOGUE_SHOP_IDENTIFY, DIALOGUE_SHOP_REPAIR};
      case DIALOGUE_LEARN_SKILLS:
        return listShopLearnableSkills();
      default:
        return {};
    }
}

std::vector<DialogueId> GUIWindow_AlchemyShop::listDialogueOptions() {
    if (_currentDialogue == DIALOGUE_SHOP_DISPLAY_EQUIPMENT) {
        return {DIALOGUE_SHOP_SELL, DIALOGUE_SHOP_IDENTIFY};
    }
    return GUIWindow_Shop::listDialogueOptions();
}

void GUIWindow_Shop::updateDialogueOnEscape() {
    if (IsSkillLearningDialogue(_currentDialogue)) {
        _currentDialogue = DIALOGUE_LEARN_SKILLS;
        return;
    }
    if (_currentDialogue == DIALOGUE_SHOP_SELL || _currentDialogue == DIALOGUE_SHOP_IDENTIFY || _currentDialogue == DIALOGUE_SHOP_REPAIR) {
        _currentDialogue = DIALOGUE_SHOP_DISPLAY_EQUIPMENT;
        return;
    }
    if (_currentDialogue == DIALOGUE_MAIN) {
        _currentDialogue = DIALOGUE_NULL;
        return;
    }
    _currentDialogue = DIALOGUE_MAIN;
}

void GUIWindow_Shop::playHouseGoodbyeSpeech() {
    bool rudeReaction = true;

    if (pParty->PartyTimes.shopBanTimes[houseId()] <= pParty->GetPlayingTime()) {
        if (pParty->GetGold() <= 10000) {
            if (_transactionPerformed) {
                playHouseSound(houseId(), HOUSE_SOUND_SHOP_GOODBYE_POLITE);
            }
            return;
        }
        playHouseSound(houseId(), _transactionPerformed ? HOUSE_SOUND_SHOP_GOODBYE_POLITE : HOUSE_SOUND_SHOP_GOODBYE_RUDE);
        rudeReaction = !_transactionPerformed;
    }
    if (rudeReaction && !pParty->_delayedReactionTimer) {
        int id = pParty->getRandomActiveCharacterId(vrng);

        if (id != -1) {
            pParty->setDelayedReaction(SPEECH_SHOP_RUDE, id);
            return;
        }
    }
}

// Alchemy shop is special. Instead of polite and rude goodbye dialogue it have
// polite and enthusiastic ones.
void GUIWindow_AlchemyShop::playHouseGoodbyeSpeech() {
    if (pParty->PartyTimes.shopBanTimes[houseId()] <= pParty->GetPlayingTime()) {
        playHouseSound(houseId(), _transactionPerformed ? HOUSE_SOUND_ALCHEMY_SHOP_GOODBYE_BOUGHT : HOUSE_SOUND_ALCHEMY_SHOP_GOODBYE_REGULAR);
    } else if (!pParty->_delayedReactionTimer) {
        int id = pParty->getRandomActiveCharacterId(vrng);

        if (id != -1) {
            pParty->setDelayedReaction(SPEECH_SHOP_RUDE, id);
        }
    }
}

void GUIWindow_Shop::houseScreenClick() {
    // TODO(pskelton): extract common code around shop item picking
    if (current_screen_type == SCREEN_SHOP_INVENTORY) {
        pParty->activeCharacter().OnInventoryLeftClick();
        return;
    }

    if (!checkIfPlayerCanInteract()) {
        pAudioPlayer->playUISound(SOUND_error);
        return;
    }

    Pointi pt = EngineIocContainer::ResolveMouse()->GetCursorPos();

    switch (_currentDialogue) {
        case DIALOGUE_SHOP_DISPLAY_EQUIPMENT: {
            current_character_screen_window = WINDOW_CharacterWindow_Inventory;
            pParty->activeCharacter().OnInventoryLeftClick();
            break;
        }

        case DIALOGUE_SHOP_SELL: {
            int invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
            if (pt.x <= 13 || pt.x >= 462) {
                return;
            }

            int pItemID = pParty->activeCharacter().GetItemListAtInventoryIndex(invindex);
            if (!pItemID) {
                return;
            }

            if (pParty->activeCharacter().pInventoryItemList[pItemID - 1].canSellRepairIdentifyAt(houseId())) {
                _transactionPerformed = true;
                pParty->activeCharacter().SalesProcess(invindex, pItemID - 1, houseId());
                render->ClearZBuffer();
                pParty->activeCharacter().playReaction(SPEECH_ITEM_SOLD);
                return;
            }

            pParty->activeCharacter().playReaction(SPEECH_WRONG_SHOP);
            pAudioPlayer->playUISound(SOUND_error);
            break;
        }

        case DIALOGUE_SHOP_IDENTIFY: {
            int invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
            if (pt.x <= 13 || pt.x >= 462) {
                return;
            }

            int pItemID = pParty->activeCharacter().GetItemListAtInventoryIndex(invindex);
            if (!pItemID) {
                return;
            }

            float fPriceMultiplier = buildingTable[houseId()].fPriceMultiplier;
            int uPriceItemService = PriceCalculator::itemIdentificationPriceForPlayer(&pParty->activeCharacter(), fPriceMultiplier);
            ItemGen &item = pParty->activeCharacter().pInventoryItemList[pItemID - 1];

            if (!(item.uAttributes & ITEM_IDENTIFIED)) {
                if (item.canSellRepairIdentifyAt(houseId())) {
                    if (pParty->GetGold() >= uPriceItemService) {
                        _transactionPerformed = true;
                        pParty->TakeGold(uPriceItemService);
                        item.uAttributes |= ITEM_IDENTIFIED;
                        pParty->activeCharacter().playReaction(SPEECH_SHOP_IDENTIFY);
                        engine->_statusBar->setEvent(LSTR_DONE);
                        return;
                    }

                    playHouseSound(houseId(), HOUSE_SOUND_GENERAL_NOT_ENOUGH_GOLD);
                    engine->_statusBar->setEvent(LSTR_NOT_ENOUGH_GOLD);
                    return;
                }

                pAudioPlayer->playUISound(SOUND_error);
                pParty->activeCharacter().playReaction(SPEECH_WRONG_SHOP);
                return;
            }

            pParty->activeCharacter().playReaction(SPEECH_ALREADY_INDENTIFIED);
            break;
        }

        case DIALOGUE_SHOP_REPAIR: {
            int invindex = ((pt.x - 14) >> 5) + 14 * ((pt.y - 17) >> 5);
            if (pt.x <= 13 || pt.x >= 462) {
                return;
            }

            int pItemID = pParty->activeCharacter().GetItemListAtInventoryIndex(invindex);
            if (!pItemID) {
                return;
            }

            ItemGen &item = pParty->activeCharacter().pInventoryItemList[pItemID - 1];
            float fPriceMultiplier = buildingTable[houseId()].fPriceMultiplier;
            int uPriceItemService = PriceCalculator::itemRepairPriceForPlayer(&pParty->activeCharacter(), item.GetValue(), fPriceMultiplier);

            if (item.uAttributes & ITEM_BROKEN) {
                if (item.canSellRepairIdentifyAt(houseId())) {
                    if (pParty->GetGold() >= uPriceItemService) {
                        _transactionPerformed = true;
                        pParty->TakeGold(uPriceItemService);
                        item.uAttributes = (item.uAttributes & ~ITEM_BROKEN) | ITEM_IDENTIFIED;
                        pParty->activeCharacter().playReaction(SPEECH_SHOP_REPAIR);
                        engine->_statusBar->setEvent(LSTR_GOOD_AS_NEW);
                        return;
                    }

                    playHouseSound(houseId(), HOUSE_SOUND_GENERAL_NOT_ENOUGH_GOLD);
                    engine->_statusBar->setEvent(LSTR_NOT_ENOUGH_GOLD);
                    return;
                }

                pAudioPlayer->playUISound(SOUND_error);
                pParty->activeCharacter().playReaction(SPEECH_WRONG_SHOP);
                return;
            }

            pParty->activeCharacter().playReaction(SPEECH_ALREADY_INDENTIFIED);
            break;
        }

        case DIALOGUE_SHOP_BUY_STANDARD:
        case DIALOGUE_SHOP_BUY_SPECIAL: {
            int testx;
            int testpos;
            ItemGen *boughtItem = nullptr;

            switch (buildingType()) {
              case BUILDING_WEAPON_SHOP:
                testx = (pt.x - 30) / 70;
                if (testx >= 0 && testx < 6) {
                    if (_currentDialogue == DIALOGUE_SHOP_BUY_STANDARD)
                        boughtItem = &pParty->standartItemsInShops[houseId()][testx];
                    else
                        boughtItem = &pParty->specialItemsInShops[houseId()][testx];

                    if (boughtItem->uItemID != ITEM_NULL) {
                        testpos = ((60 - (shop_ui_items_in_store[testx]->width() / 2)) + testx * 70);
                        if (pt.x >= testpos && pt.x < (testpos + (shop_ui_items_in_store[testx]->width()))) {
                            if (pt.y >= weaponYPos[testx] + 30 && pt.y < (weaponYPos[testx] + 30 + (shop_ui_items_in_store[testx]->height()))) {
                                break;  // good
                            }
                        }
                    }
                }
                return;

              case BUILDING_ARMOR_SHOP:
                testx = (pt.x - 40) / 105;
                if (testx >= 0 && testx < 4) {
                    if (pt.y >= 126) {
                        testx += 4;
                    }

                    if (_currentDialogue == DIALOGUE_SHOP_BUY_STANDARD)
                        boughtItem = &pParty->standartItemsInShops[houseId()][testx];
                    else
                        boughtItem = &pParty->specialItemsInShops[houseId()][testx];

                    if (boughtItem->uItemID != ITEM_NULL) {
                        if (testx >= 4) {
                            testpos = ((90 - (shop_ui_items_in_store[testx]->width() / 2)) + (testx * 105) - 420);  // low row
                        } else {
                            testpos = ((86 - (shop_ui_items_in_store[testx]->width() / 2)) + testx * 105);
                        }

                        if (pt.x >= testpos && pt.x <= testpos + (shop_ui_items_in_store[testx]->width())) {
                            if ((pt.y >= 126 && pt.y < (126 + (shop_ui_items_in_store[testx]->height()))) ||
                                (pt.y <= 98 && pt.y >= (98 - (shop_ui_items_in_store[testx]->height())))) {
                                break;  // good
                            }
                        }
                    }
                }
                return;

              case BUILDING_ALCHEMY_SHOP:
              case BUILDING_MAGIC_SHOP:
                testx = (pt.x) / 75;
                if (testx >= 0 && testx < 6) {
                    if (pt.y > 152) {
                        testx += 6;
                    }

                    if (_currentDialogue == DIALOGUE_SHOP_BUY_STANDARD)
                        boughtItem = &pParty->standartItemsInShops[houseId()][testx];
                    else
                        boughtItem = &pParty->specialItemsInShops[houseId()][testx];

                    if (boughtItem->uItemID != ITEM_NULL) {
                        if (pt.y > 152) {
                            testpos = 75 * testx - (shop_ui_items_in_store[testx]->width() / 2) + 40 - 450;
                        } else {
                            testpos = 75 * testx - (shop_ui_items_in_store[testx]->width() / 2) + 40;
                        }

                        if (pt.x >= testpos && pt.x <= testpos + (shop_ui_items_in_store[testx]->width())) {
                            if ((pt.y <= 308 && pt.y >= (308 - (shop_ui_items_in_store[testx]->height()))) ||
                                (pt.y <= 152 && pt.y >= (152 - (shop_ui_items_in_store[testx]->height())))) {
                                // y is 152-h to 152 or 308-height to 308
                                break;  // good
                            }
                        }
                    }
                }
                return;

              default:
                return;
            }

            float fPriceMultiplier = buildingTable[houseId()].fPriceMultiplier;
            int uPriceItemService = PriceCalculator::itemBuyingPriceForPlayer(&pParty->activeCharacter(), boughtItem->GetValue(), fPriceMultiplier);
            int stealResult = 0;
            int stealDifficulty = 0;
            int fine;
            if (engine->_currentLoadedMapId != MAP_INVALID) {
                stealDifficulty = pMapStats->pInfos[engine->_currentLoadedMapId].baseStealingFine;
            }
            int partyReputation = pParty->GetPartyReputation();
            if (isStealingModeActive()) {
                stealResult = pParty->activeCharacter().StealFromShop(boughtItem, stealDifficulty, partyReputation, 0, &fine);
                if (!stealResult) {
                    // caught stealing no item
                    processStealingResult(0, fine);
                    return;
                }
            } else if (pParty->GetGold() < uPriceItemService) {
                playHouseSound(houseId(), HOUSE_SOUND_GENERAL_NOT_ENOUGH_GOLD);
                engine->_statusBar->setEvent(LSTR_NOT_ENOUGH_GOLD);
                return;
            }

            int itemSlot = pParty->activeCharacter().AddItem(-1, boughtItem->uItemID);
            if (itemSlot) {
                boughtItem->SetIdentified();
                pParty->activeCharacter().pInventoryItemList[itemSlot - 1] = *boughtItem;
                if (stealResult != 0) {  // stolen
                    pParty->activeCharacter().pInventoryItemList[itemSlot - 1].SetStolen();
                    processStealingResult(stealResult, fine);
                } else {
                    _transactionPerformed = true;
                    pParty->TakeGold(uPriceItemService);
                }
                boughtItem->Reset();
                render->ClearZBuffer();
                pParty->activeCharacter().playReaction(SPEECH_ITEM_BUY);
                return;
            } else {
                pParty->activeCharacter().playReaction(SPEECH_NO_ROOM);
                engine->_statusBar->setEvent(LSTR_INVENTORY_IS_FULL);
                return;
            }
            break;
        }

        default:
            // Do nothing
            break;
    }
}

void GUIWindow_Shop::processStealingResult(int stealingResult, int fineToAdd) {  // not working properly??
    int reputationDelta = 0;

    if (stealingResult == 0 || stealingResult == 1) {  // got caught
        pParty->uFine = std::clamp(pParty->uFine + fineToAdd, 0, 4000000);
        if (pParty->uFine) {
            for (Character &player : pParty->pCharacters) {
                if (!player._achievedAwardsBits[Award_Fine]) {
                    player._achievedAwardsBits.set(Award_Fine);
                }
            }
        }
        if (stealingResult == 1)
            reputationDelta = 2;
        else
            reputationDelta = 1;

        // only ban when caught
        pParty->PartyTimes.shopBanTimes[houseId()] = pParty->GetPlayingTime() + Duration::fromDays(1);
    } else {
        reputationDelta = 2;
    }

    pParty->InTheShopFlags[houseId()] = 1;

    LocationInfo &loc = currentLocationInfo();
    loc.reputation += reputationDelta;
    if (loc.reputation > 10000)
        loc.reputation = 10000;
}
