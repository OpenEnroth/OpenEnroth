#include "UIHouses.h"

#include <cstdlib>
#include <limits>
#include <vector>

#include "Arcomage/Arcomage.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/MapInfo.h"
#include "Engine/Party.h"
#include "Engine/PriceCalculator.h"
#include "Engine/SaveLoad.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Tables/BuildingTable.h"
#include "Engine/Tables/AwardTable.h"
#include "Engine/Tables/MerchantTable.h"
#include "Engine/Tables/TransitionTable.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIDialogue.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIPartyCreation.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/Houses/MagicGuild.h"
#include "GUI/UI/Houses/Bank.h"
#include "GUI/UI/Houses/Jail.h"
#include "GUI/UI/Houses/Tavern.h"
#include "GUI/UI/Houses/Temple.h"
#include "GUI/UI/Houses/Training.h"
#include "GUI/UI/Houses/Transport.h"
#include "GUI/UI/Houses/MercenaryGuild.h"
#include "GUI/UI/Houses/TownHall.h"
#include "GUI/UI/Houses/Shops.h"

#include "Io/Mouse.h"
#include "Io/KeyboardInputHandler.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

#include "Utility/String.h"
#include "Library/Random/Random.h"
#include "Utility/Math/TrigLut.h"

using Io::TextInputType;

int uHouse_ExitPic;
int dword_591080;               // 591080

BuildingType in_current_building_type;  // 00F8B198
DIALOGUE_TYPE dialog_menu_id;     // 00F8B19C

GraphicsImage *_591428_endcap = nullptr;

std::array<const HouseAnimDescr, 196> pAnimatedRooms = { {  // 0x4E5F70
    { "", 0x4, 0x1F4, BUILDING_INVALID, 0, 0 },
    { "Human Armor01", 0x20, 0x2C0, BUILDING_ARMOR_SHOP, 58, 0 },
    { "Necromancer Armor01", 0x20, 0x2D7, BUILDING_ARMOR_SHOP, 70, 0 },
    { "Dwarven Armor01", 0x20, 0x2EE, BUILDING_ARMOR_SHOP, 5, 0 },
    { "Wizard Armor", 0x20, 0x3BD, BUILDING_ARMOR_SHOP, 19, 0 },
    { "Warlock Armor", 0x20, 0x2D6, BUILDING_ARMOR_SHOP, 35, 0 },
    { "Elf Armor", 0x20, 0x2BC, BUILDING_ARMOR_SHOP, 79, 0 },
    { "Human Alchemisht01", 0xE, 0x2BE, BUILDING_ALCHEMY_SHOP, 95, 0 },
    { "Necromancer Alchemist01", 0xE, 0x2D6, BUILDING_ALCHEMY_SHOP, 69, 0 },
    { "Dwarven Achemist01", 0xE, 0x387, BUILDING_ALCHEMY_SHOP, 4, 0 },
    { "Wizard Alchemist", 0xE, 0x232, BUILDING_ALCHEMY_SHOP, 25, 0 },
    { "Warlock Alchemist", 0xE, 0x2BE, BUILDING_ALCHEMY_SHOP, 42, 0 },
    { "Elf Alchemist", 0xE, 0x38A, BUILDING_ALCHEMY_SHOP, 84, 0 },
    { "Human Bank01", 0x6, 0x384, BUILDING_BANK, 52, 0 },
    { "Necromancer Bank01", 0x6, 0x2D8, BUILDING_BANK, 71, 0 },
    { "Dwarven Bank", 0x6, 0x2F3, BUILDING_BANK, 6, 0 },
    { "Wizard Bank", 0x6, 0x3BA, BUILDING_BANK, 20, 0 },
    { "Warlock Bank", 0x6, 0x39F, BUILDING_BANK, 36, 0 },
    { "Elf Bank", 0x6, 0x2BC, BUILDING_BANK, 71, 0 },
    { "Boat01", 0xF, 0x4C, BUILDING_BOAT, 53, 3 },
    { "Boat01d", 0xF, 0x4C, BUILDING_BOAT, 53, 3 }, // this movie doesn't exist
    { "Human Magic Shop01", 0xA, 0x2C8, BUILDING_MAGIC_SHOP, 54, 0 },
    { "Necromancer Magic Shop01", 0xE, 0x2DC, BUILDING_MAGIC_SHOP, 66, 0 },
    { "Dwarven Magic Shop01", 0x2A, 0x2EF, BUILDING_MAGIC_SHOP, 91, 0 },
    { "Wizard Magic Shop", 0x1E, 0x2DF, BUILDING_MAGIC_SHOP, 15, 0 },
    { "Warlock Magic Shop", 0x7, 0x3B9, BUILDING_MAGIC_SHOP, 15, 0 },
    { "Elf Magic Shop", 0x24, 0x2CC, BUILDING_MAGIC_SHOP, 82, 0 },
    { "Human Stables01", 0x21, 0x31, BUILDING_STABLE, 48, 3 },
    { "Necromancer Stables", 0x21, 0x2DD, BUILDING_STABLE, 67, 3 },
    { "", 0x21, 0x2F0, BUILDING_STABLE, 91, 3 },
    { "Wizard Stables", 0x21, 0x3BA, BUILDING_STABLE, 16, 3 },
    { "Warlock Stables", 0x21, 0x181, BUILDING_STABLE, 77, 3 },  // movie exist but unused in MM7 as Nighon doesn't have stables
    { "Elf Stables", 0x21, 0x195, BUILDING_STABLE, 77, 3 },
    { "Human Tavern01", 0xD, 0x2C2, BUILDING_TAVERN, 49, 0 },
    { "Necromancer Tavern 01", 0xD, 0x3B0, BUILDING_TAVERN, 57, 0 },
    { "Dwarven Tavern01", 0xD, 0x2FE, BUILDING_TAVERN, 94, 0 },
    { "Wizard Tavern", 0xD, 0x3BB, BUILDING_TAVERN, 17, 0 },
    { "Warlock Tavern", 0xD, 0x3A8, BUILDING_TAVERN, 33, 0 },
    { "Elf Tavern", 0xD, 0x2CD, BUILDING_TAVERN, 78, 0 },
    { "Human Temple01", 0x24, 0x2DB, BUILDING_TEMPLE, 50, 3 },
    { "Necromancer Temple", 0x24, 0x2DF, BUILDING_TEMPLE, 60, 3 },
    { "Dwarven Temple01", 0x24, 0x2F1, BUILDING_TEMPLE, 86, 3 },
    { "Wizard Temple", 0x24, 0x2E0, BUILDING_TEMPLE, 10, 3 },
    { "Warlock Temple", 0x24, 0x3A4, BUILDING_TEMPLE, 27, 3 },
    { "Elf Temple", 0x24, 0x2CE, BUILDING_TEMPLE, 72, 3 },
    { "Human Town Hall", 0x10, 0x39C, BUILDING_TOWN_HALL, 14, 0 },
    { "Necromancer Town Hall01", 0x10, 0x3A4, BUILDING_TOWN_HALL, 61, 0 },
    { "Dwarven Town Hall", 0x10, 0x2DB, BUILDING_TOWN_HALL, 88, 0 }, // this movie doesn't exist, stone city doesn't have town hall
    { "Wizard Town Hall", 0x10, 0x3BD, BUILDING_TOWN_HALL, 11, 0 },
    { "Warlock Town Hall", 0x10, 0x2DB, BUILDING_TOWN_HALL, 28, 0 },
    { "Elf Town Hall", 0x10, 0x27A, BUILDING_TOWN_HALL, 73, 0 },
    { "Human Training Ground01", 0x18, 0x2C7, BUILDING_TRAINING_GROUND, 44, 0 },
    { "Necromancer Training Ground", 0x18, 0x3AD, BUILDING_TRAINING_GROUND, 62, 0 },
    { "Dwarven Training Ground", 0x18, 0x2F2, BUILDING_TRAINING_GROUND, 89, 0 },
    { "Wizard Training Ground", 0x18, 0x3A3, BUILDING_TRAINING_GROUND, 12, 0 },
    { "Warlock Training Ground", 0x18, 0x3A6, BUILDING_TRAINING_GROUND, 29, 0 },
    { "Elf Training Ground", 0x18, 0x19F, BUILDING_TRAINING_GROUND, 74, 0 },
    { "Human Weapon Smith01", 0x16, 0x2C1, BUILDING_WEAPON_SHOP, 45, 4 },
    { "Necromancer Weapon Smith01", 0x16, 0x2D9, BUILDING_WEAPON_SHOP, 63, 4 },
    { "Dwarven Weapon Smith01", 0x16, 0x2EE, BUILDING_WEAPON_SHOP, 82, 4 },
    { "Wizard Weapon Smith", 0x16, 0x2D5, BUILDING_WEAPON_SHOP, 13, 4 },
    { "Warlock Weapon Smith", 0x16, 0x2D7, BUILDING_WEAPON_SHOP, 23, 4 },
    { "Elf Weapon Smith", 0x16, 0x2CA, BUILDING_WEAPON_SHOP, 75, 4 },
    { "Air Guild", 0x1D, 0xA4, BUILDING_AIR_GUILD, 1, 3 },
    { "Body Guild", 0x19, 0x3BF, BUILDING_BODY_GUILD, 2, 0 },
    { "Dark Guild", 0x19, 0x2D1, BUILDING_DARK_GUILD, 3, 0 },
    { "Earth Guild", 0x19, 0x2CB, BUILDING_EARTH_GUILD, 83, 0 },
    { "Fire Guild", 0x1C, 0x2BF, BUILDING_FIRE_GUILD, 56, 0 },
    { "Light Guild", 0x1C, 0x2D5, BUILDING_LIGHT_GUILD, 46, 0 },
    { "Mind Guild", 0x1C, 0xE5, BUILDING_MIND_GUILD, 40, 0 },
    { "Spirit Guild", 0x1C, 0x2D2, BUILDING_SPIRIT_GUILD, 41, 0 },
    { "Water Guild", 0x1B, 0x2D3, BUILDING_WATER_GUILD, 24, 0 },
    { "Lord and Judge Out01", 1, 0, BUILDING_HOUSE, 39, 0 },
    { "Human Poor House 1", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Human Poor House 2", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Human Poor House 3", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Human Medium House 1", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Human Medium House 2", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Human Medium House 3", 8, 0, BUILDING_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Human Rich House 1", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Human Rich House 2", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Human Rich House 3", 8, 0, BUILDING_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Elven Poor House 1", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Elven Poor House 2", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Elven Poor House 3", 8, 0, BUILDING_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Elven Medium House 1", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Elven Medium House 2", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Elven Medium House 3", 8, 0, BUILDING_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Elven Rich House 1", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Elven Rich House 2", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Elven Rich House 3", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Dwarven Poor House 1", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Dwarven Poor House 2", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Dwarven Poor House 3", 8, 0, BUILDING_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Dwarven Medium House 1", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Dwarven Medium House 2", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Dwarven Medium House 3", 8, 0, BUILDING_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Dwarven Rich House 1", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Dwarven Rich House 2", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Dwarven Rich House 3", 8, 0, BUILDING_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Wizard Poor House 1", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Wizard Poor House 2", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Wizard Poor House 3", 8, 0, BUILDING_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Wizard Medium House 1", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Wizard Medium House 2", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Wizard Medium House 3", 8, 0, BUILDING_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Wizard Rich House 1", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Wizard Rich House 2", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Wizard Rich House 3", 8, 0, BUILDING_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Necromancer Poor House 1", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Necromancer Poor House 2", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Necromancer Poor House 3", 8, 0, BUILDING_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Necromancer Medium House 1", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Necromancer Medium House 2", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Necromancer Medium House 3", 8, 0, BUILDING_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Necromancer Rich House 1", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Necromancer Rich House 2", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Necromancer Rich House 3", 8, 0, BUILDING_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Warlock Poor House 1", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Warlock Poor House 2", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Warlock Poor House 3", 8, 0, BUILDING_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Warlock Medium House 1", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Warlock Medium House 2", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Warlock Medium House 3", 8, 0, BUILDING_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Warlock Rich House 1", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Warlock Rich House 2", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Warlock Rich House 3", 8, 0, BUILDING_HOUSE, 0, 0 },
    { "Out01 Temple of the Moon", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out01 Dragon Cave", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out02 Castle Harmondy", 0x24, 0, BUILDING_CASTLE, 0, 0 },
    { "Out02 White Cliff Cave", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out03 Erathian Sewer", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out03 Fort Riverstride", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out03 Castle Gryphonheart", 0x24, 0, BUILDING_CASTLE, 0, 0 },
    { "Out04 Elf Castle", 0x24, 0, BUILDING_CASTLE, 0, 0 },
    { "Out04 Tularean Caves", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out04 Clanker's Laboratory", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out05 Hall of the Pit", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out05 Watchtower 6", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out06 School of Sorcery", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out06 Red Dwarf Mines", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out07 Castle Lambent", 0x24, 0, BUILDING_CASTLE, 0, 0 },
    { "Out07 Walls of Mist", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out07 Temple of the Light", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out08 Evil Entrance", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out08 Breeding Zone", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out08 Temple of the Dark", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out09 Grand Temple of the Moon", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out09 Grand Temple of the Sun", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out10 Thunderfist Mountain", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out10 The Maze", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out10 Connecting Tunnel Cave #1", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out11 Stone City", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out12 Colony Zod", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out12 Connecting Tunnel Cave #1", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out13 Mercenary Guild", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out13 Tidewater Caverns", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out13 Wine Cellar", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out14 Titan's Stronghold", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out14 Temple of Baa", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out14 Hall under the Hill", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Out15 The Linclon", 0x24, 0, BUILDING_DUNGEON, 0, 0 },
    { "Jail", 0x24, 0, BUILDING_JAIL, 0, 0 },
    { "Harmondale Throne Room", 0x24, 0, BUILDING_HOUSE, 0, 0 },
    { "Gryphonheart Throne Room", 0x24, 0, BUILDING_THRONE_ROOM, 0, 0 },
    { "Elf Castle Throne Room", 0x24, 0, BUILDING_THRONE_ROOM, 0, 0 },
    { "Wizard Castle Throne Room", 0x24, 0, BUILDING_THRONE_ROOM, 0, 0 },
    { "Necromancer Castle Throne Rooms", 0x24, 0, BUILDING_THRONE_ROOM, 0, 0 },
    { "Master Thief", 0x24, 0, BUILDING_HOUSE, 0, 0 },
    { "Dwarven King", 0x24, 0, BUILDING_THRONE_ROOM, 0, 0 },
    { "Arms Master", 0x24, 0, BUILDING_HOUSE, 0, 0 },
    { "Warlock", 0x24, 0, BUILDING_HOUSE, 0, 0 },
    { "Lord Markam", 0x24, 0, BUILDING_THRONE_ROOM, 0, 0 },
    { "Arbiter Neutral Town", 0x24, 0, BUILDING_HOUSE, 0, 0 },
    { "Arbiter Good Town", 0x24, 0, BUILDING_HOUSE, 0, 0 },
    { "Arbiter Evil Town", 0x24, 0, BUILDING_HOUSE, 0, 0 },
    { "Necromancer Throne Room Empty", 0x24, 0, BUILDING_THRONE_ROOM, 0, 0 },
    { "", 0x24, 0, BUILDING_HOUSE, 0, 0 },
    { "", 0x24, 0, BUILDING_HOUSE, 0, 0 },
    { "", 0x24, 0, BUILDING_HOUSE, 0, 0 },
    { "", 0x24, 0, BUILDING_HOUSE, 0, 0 },
    { "Boat01", 0xF, 0, BUILDING_HOUSE, 53, 3 },
    { "", 0x24, 0, BUILDING_BOAT, 0, 0 },
    { "", 0x24, 0, BUILDING_BOAT, 0, 0 },
    { "", 0x24, 0, BUILDING_BOAT, 0, 0 },
    { "", 0x24, 0, BUILDING_HOUSE, 0, 0 },
    { "Arbiter Room Neutral", 0x24, 0, BUILDING_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Out02 Castle Harmondy Abandoned", 0x24, 0, BUILDING_CASTLE, 0, 0 },
    { "Human Temple02", 0x24, 0x3AB, BUILDING_TEMPLE, 27, 0 },
    { "Player Castle Good", 0x24, 0, BUILDING_CASTLE, 0, 0 },
    { "Player Castle Bad", 0x24, 0, BUILDING_CASTLE, 0, 0 }
} };

IndexedArray<int, BUILDING_WEAPON_SHOP, BUILDING_DARK_GUILD> itemAmountInShop = {{
    {BUILDING_WEAPON_SHOP,   6},
    {BUILDING_ARMOR_SHOP,    8},
    {BUILDING_MAGIC_SHOP,   12},
    {BUILDING_ALCHEMY_SHOP, 12},
    {BUILDING_FIRE_GUILD,   12},
    {BUILDING_AIR_GUILD,    12},
    {BUILDING_WATER_GUILD,  12},
    {BUILDING_EARTH_GUILD,  12},
    {BUILDING_SPIRIT_GUILD, 12},
    {BUILDING_MIND_GUILD,   12},
    {BUILDING_BODY_GUILD,   12},
    {BUILDING_LIGHT_GUILD,  12},
    {BUILDING_DARK_GUILD,   12}
}};

IndexedArray<std::string, BUILDING_WEAPON_SHOP, BUILDING_MIRRORED_PATH_GUILD> shopBackgroundNames = {{
    {BUILDING_WEAPON_SHOP,           "WEPNTABL"},
    {BUILDING_ARMOR_SHOP,            "ARMORY"},
    {BUILDING_MAGIC_SHOP,            "MAGSHELF"},
    {BUILDING_ALCHEMY_SHOP,          "MAGSHELF"},
    {BUILDING_FIRE_GUILD,            "MAGSHELF"},
    {BUILDING_AIR_GUILD,             "MAGSHELF"},
    {BUILDING_WATER_GUILD,           "MAGSHELF"},
    {BUILDING_EARTH_GUILD,           "MAGSHELF"},
    {BUILDING_SPIRIT_GUILD,          "MAGSHELF"},
    {BUILDING_MIND_GUILD,            "MAGSHELF"},
    {BUILDING_BODY_GUILD,            "MAGSHELF"},
    {BUILDING_LIGHT_GUILD,           "MAGSHELF"},
    {BUILDING_DARK_GUILD,            "MAGSHELF"},
    {BUILDING_ELEMENTAL_GUILD,       "MAGSHELF"},
    {BUILDING_SELF_GUILD,            "MAGSHELF"},
    {BUILDING_MIRRORED_PATH_GUILD,   "MAGSHELF"}
}};

std::array<std::string, 6> portraitClickLabel;

bool enterHouse(HOUSE_ID uHouseID) {
    GameUI_StatusBar_Clear();
    GameUI_SetStatusBar("");
    pCurrentFrameMessageQueue->Flush();
    uDialogueType = DIALOGUE_NULL;
    keyboardInputHandler->SetWindowInputStatus(WINDOW_INPUT_CANCELLED);
    keyboardInputHandler->ResetKeys();

    if (uHouseID == HOUSE_THRONEROOM_WIN_GOOD || uHouseID == HOUSE_THRONEROOM_WIN_EVIL) {
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_ShowGameOverWindow, 0, 0);
        return false;
    }

    int uOpenTime = buildingTable[uHouseID - HOUSE_SMITH_EMERALD_ISLE].uOpenTime;
    int uCloseTime = buildingTable[uHouseID - HOUSE_SMITH_EMERALD_ISLE].uCloseTime;
    current_npc_text.clear();
    render->ClearZBuffer();

    if (((uCloseTime - 1 <= uOpenTime) && ((pParty->uCurrentHour < uOpenTime) && (pParty->uCurrentHour >(uCloseTime - 1)))) ||
            ((uCloseTime - 1 > uOpenTime) && ((pParty->uCurrentHour < uOpenTime) ||
            (pParty->uCurrentHour >(uCloseTime - 1))))) {
        int am_pm_flag_open = 0;
        int am_pm_flag_close = 0;
        if (uOpenTime > 12) {
            uOpenTime -= 12;
            am_pm_flag_open = 1;
        }
        if (uCloseTime > 12) {
            uCloseTime -= 12;
            am_pm_flag_close = 1;
        }

        GameUI_SetStatusBar(LSTR_FMT_OPEN_TIME, uOpenTime, localization->GetAmPm(am_pm_flag_open), uCloseTime, localization->GetAmPm(am_pm_flag_close));
        if (pParty->hasActiveCharacter()) {
            pParty->activeCharacter().playReaction(SPEECH_StoreClosed);
        }

        return false;
    } else {
        if (isShop(uHouseID)) {  // entering shops and guilds
            if (!(pParty->PartyTimes.shopBanTimes[uHouseID]) || (pParty->PartyTimes.shopBanTimes[uHouseID] <= pParty->GetPlayingTime())) {
                pParty->PartyTimes.shopBanTimes[uHouseID] = GameTime(0);
            } else {
                GameUI_SetStatusBar(LSTR_BANNED_FROM_SHOP);
                return false;
            }
        }

        uCurrentHouse_Animation = buildingTable[uHouseID - HOUSE_SMITH_EMERALD_ISLE].uAnimationID;
        in_current_building_type = pAnimatedRooms[uCurrentHouse_Animation].uBuildingType;
        if (in_current_building_type == BUILDING_THRONE_ROOM && pParty->uFine) {  // going to jail
            uHouseID = HOUSE_JAIL;
            uCurrentHouse_Animation = buildingTable[uHouseID - 1].uAnimationID;
            restAndHeal(GameTime::FromYears(1));
            in_current_building_type = pAnimatedRooms[buildingTable[HOUSE_LORD_AND_JUDGE_EMERALD_ISLE].uAnimationID].uBuildingType;
            ++pParty->uNumPrisonTerms;
            pParty->uFine = 0;
            for (Player &player : pParty->pPlayers) {
                player.timeToRecovery = 0;
                player.uNumDivineInterventionCastsThisDay = 0;
                player.SetVariable(VAR_Award, Award_PrisonTerms);
            }
        }

        std::string pContainer = DialogueBackgroundResourceByAlignment[pParty->alignment];

        pDialogueNPCCount = 0;
        game_ui_dialogue_background = assets->getImage_Solid(pContainer);

        PrepareHouse(uHouseID);

        if (uNumDialogueNPCPortraits == 1)
            pDialogueNPCCount = 1;
        pMediaPlayer->OpenHouseMovie(pAnimatedRooms[uCurrentHouse_Animation].video_name, 1u);
        dword_5C35D4 = 1;
        if (isMagicGuild(uHouseID)) {
            // TODO(pskelton): check this behaviour
            if (!pParty->hasActiveCharacter())  // avoid nzi
                pParty->setActiveToFirstCanAct();

            if (!pParty->activeCharacter()._achievedAwardsBits[guildMembershipFlags[uHouseID]]) {
                playHouseSound(uHouseID, HOUSE_SOUND_MAGIC_GUILD_MEMBERS_ONLY);
                return true;
            }
        } else if ((isStable(uHouseID) || isBoat(uHouseID)) && !isTravelAvailable(uHouseID)) {
            return true;
        }
        playHouseSound(uHouseID, HOUSE_SOUND_GENERAL_GREETING);
        dword_5C35D4 = 1;
        return true;
    }
}

//----- (0044606A) --------------------------------------------------------
void PrepareHouse(HOUSE_ID house) {
    int16_t uExitMapID;  // ax@2
                         //  int v7; // ebx@11
                         //  int v13; // [sp+30h] [bp-30h]@11
    int npc_id_arr[6];   // [sp+34h] [bp-2Ch]@1
    int uAnimationID;    // [sp+50h] [bp-10h]@1

    uAnimationID = buildingTable[house - HOUSE_SMITH_EMERALD_ISLE].uAnimationID;
    memset(npc_id_arr, 0, sizeof(npc_id_arr));
    uNumDialogueNPCPortraits = 0;
    uHouse_ExitPic = buildingTable[house - HOUSE_SMITH_EMERALD_ISLE].uExitPicID;
    if (uHouse_ExitPic) {
        uExitMapID = buildingTable[house - HOUSE_SMITH_EMERALD_ISLE]._quest_bit;
        if (uExitMapID > 0) {
            if (pParty->_questBits[uExitMapID]) {
                uHouse_ExitPic = 0;
            }
        }
    }

    dword_591080 = pAnimatedRooms[uAnimationID].house_npc_id;
    HouseNPCData[0] = 0;
    uNumDialogueNPCPortraits = 0;
    if (dword_591080) {
        npc_id_arr[0] = dword_591080;
        uNumDialogueNPCPortraits = 1;
    }

    for (uint i = 1; i < pNPCStats->uNumNewNPCs; ++i) {
        if (pNPCStats->pNewNPCData[i].Location2D == house) {
            if (!(pNPCStats->pNewNPCData[i].uFlags & NPC_HIRED)) {
                HouseNPCData[uNumDialogueNPCPortraits + 1 -
                    ((dword_591080 != 0) ? 1 : 0)] =
                    &pNPCStats->pNewNPCData[i];
                npc_id_arr[uNumDialogueNPCPortraits] =
                    pNPCStats->pNewNPCData[i].uPortraitID;
                ++uNumDialogueNPCPortraits;
                if (!(pNPCStats->pNewNPCData[i].uFlags & NPC_GREETED_SECOND)) {
                    if (pNPCStats->pNewNPCData[i].uFlags & NPC_GREETED_FIRST) {
                        pNPCStats->pNewNPCData[i].uFlags &= ~NPC_GREETED_FIRST;
                        pNPCStats->pNewNPCData[i].uFlags |= NPC_GREETED_SECOND;
                    } else {
                        pNPCStats->pNewNPCData[i].uFlags |= NPC_GREETED_FIRST;
                    }
                }
            }
        }
    }

    for (int i = 0; i < uNumDialogueNPCPortraits; ++i) {
        pDialogueNPCPortraits[i] = assets->getImage_ColorKey(
            fmt::format("npc{:03}", npc_id_arr[i]));
    }

    if (uHouse_ExitPic) {
        pDialogueNPCPortraits[uNumDialogueNPCPortraits] =
            assets->getImage_ColorKey(pHouse_ExitPictures[uHouse_ExitPic]);
        ++uNumDialogueNPCPortraits;
        uHouse_ExitPic = buildingTable[house - HOUSE_SMITH_EMERALD_ISLE].uExitMapID;
    }
}

void SimpleHouseDialog() {
    NPCData *pNPC;       // esi@6
    std::string v15;           // esi@14
    GUIButton *pButton;  // eax@15
    char *v29;           // esi@42
    int v31;             // ST1C_4@42
    int index;           // esi@51
    int v36;
    signed int all_text_height;   // ecx@54
    int v40;                      // edi@57
    Color pTextColor;  // ax@60
    GUIFont *pTextFont;           // ebx@64
    int pTextHeight;
    GUIWindow w;      // [sp+Ch] [bp-110h]@64
    std::string pInString;  // [sp+114h] [bp-8h]@12

    GUIWindow house_window = *pDialogueWindow;
    if (pDialogueNPCCount == uNumDialogueNPCPortraits && uHouse_ExitPic) {
        house_window.uFrameX = 493;
        house_window.uFrameWidth = 126;
        house_window.uFrameZ = 366;
        house_window.DrawTitleText(pFontCreate, 0, 2, colorTable.White,
            pMapStats->pInfos[uHouse_ExitPic].pName, 3);
        house_window.uFrameX = SIDE_TEXT_BOX_POS_X;
        house_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
        house_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;
        if (pTransitionStrings[uHouse_ExitPic].empty()) {
            auto str = localization->FormatString(LSTR_FMT_ENTER_S, pMapStats->pInfos[uHouse_ExitPic].pName.c_str());
            house_window.DrawTitleText(pFontCreate, 0, (212 - pFontCreate->CalcTextHeight(str, house_window.uFrameWidth, 0)) / 2 + 101, colorTable.White, str, 3);
            return;
        }

        int vertMargin = (212 - pFontCreate->CalcTextHeight(pTransitionStrings[uHouse_ExitPic], house_window.uFrameWidth, 0)) / 2 + 101;
        house_window.DrawTitleText(pFontCreate, 0, vertMargin, colorTable.White, pTransitionStrings[uHouse_ExitPic], 3);
        return;
    }
    house_window.uFrameWidth -= 10;
    house_window.uFrameZ -= 10;
    pNPC = HouseNPCData[pDialogueNPCCount + -(dword_591080 != 0)];  //- 1

    house_window.DrawTitleText(pFontCreate, SIDE_TEXT_BOX_POS_X, SIDE_TEXT_BOX_POS_Y, colorTable.EasternBlue, NameAndTitle(pNPC), 3);

    if (!dword_591080) {
        if (uDialogueType == DIALOGUE_NULL) {
            if (pNPC->greet) {
                house_window.uFrameWidth = game_viewport_width;
                house_window.uFrameZ = 452;
                if (pNPC->uFlags & NPC_GREETED_SECOND) {
                    pInString = pNPCStats->pNPCGreetings[pNPC->greet].pGreeting2;
                } else {
                    pInString = pNPCStats->pNPCGreetings[pNPC->greet].pGreeting1;
                }
                // pInString = (char *)*(&pNPCStats->field_17884 +
                // ((pNPC->uFlags & 3) == 2) + 2 * pNPC->greet);
                render->DrawTextureCustomHeight(
                    8 / 640.0f,
                    (352 - (pFontArrus->CalcTextHeight(
                        pInString, house_window.uFrameWidth, 13) +
                        7)) / 480.0f,
                    ui_leather_mm7,
                    (pFontArrus->CalcTextHeight(pInString,
                        house_window.uFrameWidth, 13) + 7));

                int h = (pFontArrus->CalcTextHeight(pInString, house_window.uFrameWidth, 13) + 7);
                render->DrawTextureNew(8 / 640.0f, (347 - h) / 480.0f, _591428_endcap);
                pDialogueWindow->DrawText(pFontArrus, {13, 354 - h}, colorTable.White,
                    pFontArrus->FitTextInAWindow(pInString, house_window.uFrameWidth, 13));
            }
        }
    }
    // for right panel
    GUIWindow right_panel_window = *pDialogueWindow;
    right_panel_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    right_panel_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    right_panel_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;
    for (int i = right_panel_window.pStartingPosActiveItem;
        i < right_panel_window.pStartingPosActiveItem +
        right_panel_window.pNumPresenceButton;
        ++i) {
        pButton = right_panel_window.GetControl(i);
        switch (pButton->msg_param) {
        case DIALOGUE_SCRIPTED_LINE_1:
            v15 = pNPCTopics[pNPC->dialogue_1_evt_id].pTopic;
            if (v15.empty()) {
                pButton->msg_param = 0;
            }
            pButton->sLabel = v15;
            continue;
        case DIALOGUE_SCRIPTED_LINE_2:
            v15 = pNPCTopics[pNPC->dialogue_2_evt_id].pTopic;
            if (v15.empty()) {
                pButton->msg_param = 0;
            }

            pButton->sLabel = v15;
            continue;

        case DIALOGUE_SCRIPTED_LINE_3:
            v15 = pNPCTopics[pNPC->dialogue_3_evt_id].pTopic;
            if (v15.empty()) {
                pButton->msg_param = 0;
            }
            pButton->sLabel = v15;
            continue;
        case DIALOGUE_SCRIPTED_LINE_4:
            v15 = pNPCTopics[pNPC->dialogue_4_evt_id].pTopic;
            if (v15.empty()) {
                pButton->msg_param = 0;
            }
            pButton->sLabel = v15;
            continue;
        case DIALOGUE_SCRIPTED_LINE_5:
            v15 = pNPCTopics[pNPC->dialogue_5_evt_id].pTopic;
            if (v15.empty()) {
                pButton->msg_param = 0;
            }
            pButton->sLabel = v15;
            continue;
        case DIALOGUE_SCRIPTED_LINE_6:
            v15 = pNPCTopics[pNPC->dialogue_6_evt_id].pTopic;
            if (v15.empty()) {
                pButton->msg_param = 0;
            }
            pButton->sLabel = v15;
            continue;
        case DIALOGUE_HIRE_FIRE:
            pButton->sLabel = localization->GetString(LSTR_HIRE);
            continue;
        case DIALOGUE_PROFESSION_DETAILS:
            pButton->sLabel = localization->GetString(LSTR_MORE_INFORMATION);
            continue;
        case DIALOGUE_79_mastery_teacher:
            pButton->sLabel = _4B254D_SkillMasteryTeacher(right_panel_window.wData.val);
            continue;
        case DIALOGUE_82_join_guild:
            pButton->sLabel = GetJoinGuildDialogueOption(static_cast<GUILD_ID>(right_panel_window.wData.val));
            continue;
        case DIALOGUE_83_bounty_hunting:
            current_npc_text = ((GUIWindow_TownHall*)window_SpeakInHouse)->bountyHuntingText();
            pButton->sLabel.clear();
            continue;
        }
        if (pButton->msg_param > 0 && pButton->msg_param < DIALOGUE_13_hiring_related) {
            pButton->sLabel = localization->GetString(LSTR_JOIN);
            continue;
        }
        if (pButton->msg_param > DIALOGUE_13_hiring_related && pButton->msg_param < DIALOGUE_SCRIPTED_LINE_1) {
            pButton->sLabel.clear();
            continue;
        }
        if (pButton->msg_param != DIALOGUE_93) {
            pButton->sLabel.clear();
            continue;
        }
    }
    // DIALOGUE_93 fallthrough
    index = 0;
    all_text_height = 0;
    for (int i = pDialogueWindow->pStartingPosActiveItem;
        i < pDialogueWindow->pStartingPosActiveItem +
        pDialogueWindow->pNumPresenceButton;
        ++i) {
        pButton = pDialogueWindow->GetControl(i);
        all_text_height += pFontArrus->CalcTextHeight(
            pButton->sLabel, right_panel_window.uFrameWidth, 0);
        index++;
    }
    if (index) {
        v36 = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - all_text_height) / index;
        if (v36 > SIDE_TEXT_BOX_MAX_SPACING) v36 = SIDE_TEXT_BOX_MAX_SPACING;
        v40 = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - v36 * index - all_text_height) / 2 - v36 / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
        for (int i = pDialogueWindow->pStartingPosActiveItem;
            i < pDialogueWindow->pNumPresenceButton +
            pDialogueWindow->pStartingPosActiveItem;
            ++i) {
            pButton = pDialogueWindow->GetControl(i);
            pButton->uY = v36 + v40;
            pTextHeight = pFontArrus->CalcTextHeight(
                pButton->sLabel, right_panel_window.uFrameWidth, 0);
            pButton->uHeight = pTextHeight;
            v40 = pButton->uY + pTextHeight - 1;
            pButton->uW = v40 + 6;
            pTextColor = colorTable.Jonquil;
            if (pDialogueWindow->pCurrentPosActiveItem != i)
                pTextColor = colorTable.White;
            right_panel_window.DrawTitleText(pFontArrus, 0, pButton->uY,
                pTextColor, pButton->sLabel, 3);
        }
    }
    if (current_npc_text.length() > 0) {
        w.uFrameWidth = 458;
        w.uFrameZ = 457;
        pTextFont = pFontArrus;
        pTextHeight =
            pFontArrus->CalcTextHeight(current_npc_text, w.uFrameWidth, 13) + 7;
        if (352 - pTextHeight < 8) {
            pTextFont = pFontCreate;
            pTextHeight = pFontCreate->CalcTextHeight(current_npc_text,
                w.uFrameWidth, 13) +
                7;
        }
        render->DrawTextureCustomHeight(8 / 640.0f,
            (352 - pTextHeight) / 480.0f,
            ui_leather_mm7, pTextHeight);
        render->DrawTextureNew(8 / 640.0f, (347 - pTextHeight) / 480.0f,
            _591428_endcap);
        house_window.DrawText(pTextFont, {13, 354 - pTextHeight}, colorTable.White, pTextFont->FitTextInAWindow(current_npc_text, w.uFrameWidth, 13));
    }
}

void onSelectShopDialogueOption(DIALOGUE_TYPE option) {
    if (!pDialogueWindow || !pDialogueWindow->pNumPresenceButton) {
        return;
    }

    render->ClearZBuffer();

    dialog_menu_id = option;
    window_SpeakInHouse->houseDialogueOptionSelected(option);
    window_SpeakInHouse->reinitDialogueWindow();
    window_SpeakInHouse->initializeDialog();
}

bool houseDialogPressEscape() {
    pCurrentFrameMessageQueue->Flush();
    keyboardInputHandler->SetWindowInputStatus(WINDOW_INPUT_CANCELLED);
    keyboardInputHandler->ResetKeys();
    activeLevelDecoration = nullptr;
    current_npc_text.clear();

    if (pDialogueNPCCount == 0) {
        return false;
    }

    if (dialog_menu_id == DIALOGUE_OTHER) {
        _4B4224_UpdateNPCTopics(pDialogueNPCCount - 1);
        BackToHouseMenu();
        return true;
    }

    if (dialog_menu_id == DIALOGUE_NULL || dialog_menu_id == DIALOGUE_MAIN) {
        pDialogueNPCCount = 0;
        if (pDialogueWindow) {
            pDialogueWindow->Release();
        }
        if (shop_ui_background) {
            shop_ui_background->Release();
            shop_ui_background = nullptr;
        }
        dialog_menu_id = DIALOGUE_NULL;
        pDialogueWindow = nullptr;

        if (uNumDialogueNPCPortraits == 1) {
            return false;
        }

        pBtn_ExitCancel = window_SpeakInHouse->vButtons.front();
        for (int i = 0; i < uNumDialogueNPCPortraits; ++i) {
            Pointi pos = {pNPCPortraits_x[uNumDialogueNPCPortraits - 1][i], pNPCPortraits_y[uNumDialogueNPCPortraits - 1][i]};
            HouseNPCPortraitsButtonsList[i] = window_SpeakInHouse->CreateButton(pos, {63, 73}, 1, 0, UIMSG_ClickHouseNPCPortrait, i,
                                                                                InputAction::Invalid, portraitClickLabel[i]);
        }

        BackToHouseMenu();
        return true;
    }

    dialog_menu_id = window_SpeakInHouse->getOptionOnEscape();
    window_SpeakInHouse->reinitDialogueWindow();
    window_SpeakInHouse->initializeDialog();

    return true;
}

void createHouseUI(HOUSE_ID houseId) {
    switch (in_current_building_type) {
      case BUILDING_FIRE_GUILD:
      case BUILDING_AIR_GUILD:
      case BUILDING_WATER_GUILD:
      case BUILDING_EARTH_GUILD:
      case BUILDING_SPIRIT_GUILD:
      case BUILDING_MIND_GUILD:
      case BUILDING_BODY_GUILD:
      case BUILDING_LIGHT_GUILD:
      case BUILDING_DARK_GUILD:
      case BUILDING_MIRRORED_PATH_GUILD:
        window_SpeakInHouse = new GUIWindow_MagicGuild(houseId);
        break;
      case BUILDING_BANK:
        window_SpeakInHouse = new GUIWindow_Bank(houseId);
        break;
      case BUILDING_TEMPLE:
        window_SpeakInHouse = new GUIWindow_Temple(houseId);
        break;
      case BUILDING_TAVERN:
        window_SpeakInHouse = new GUIWindow_Tavern(houseId);
        break;
      case BUILDING_TRAINING_GROUND:
        window_SpeakInHouse = new GUIWindow_Training(houseId);
        break;
      case BUILDING_STABLE:
      case BUILDING_BOAT:
        window_SpeakInHouse = new GUIWindow_Transport(houseId);
        break;
      case BUILDING_TOWN_HALL:
        window_SpeakInHouse = new GUIWindow_TownHall(houseId);
        break;
      case BUILDING_JAIL:
        window_SpeakInHouse = new GUIWindow_Jail(houseId);
        break;
      case BUILDING_MERCENARY_GUILD:
        window_SpeakInHouse = new GUIWindow_MercenaryGuild(houseId);
        break;
      case BUILDING_WEAPON_SHOP:
        window_SpeakInHouse = new GUIWindow_WeaponShop(houseId);
        break;
      case BUILDING_ARMOR_SHOP:
        window_SpeakInHouse = new GUIWindow_ArmorShop(houseId);
        break;
      case BUILDING_MAGIC_SHOP:
        window_SpeakInHouse = new GUIWindow_MagicShop(houseId);
        break;
      case BUILDING_ALCHEMY_SHOP:
        window_SpeakInHouse = new GUIWindow_AlchemyShop(houseId);
        break;
      default:
        window_SpeakInHouse = new GUIWindow_House(houseId);
        break;
    }
    window_SpeakInHouse->CreateButton({61, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 1, InputAction::SelectChar1, "");
    window_SpeakInHouse->CreateButton({177, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 2, InputAction::SelectChar2, "");
    window_SpeakInHouse->CreateButton({292, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 3, InputAction::SelectChar3, "");
    window_SpeakInHouse->CreateButton({407, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 4, InputAction::SelectChar4, "");
    window_SpeakInHouse->CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_CycleCharacters, 0, InputAction::CharCycle, "");
    if (uNumDialogueNPCPortraits == 1) {
        _4B4224_UpdateNPCTopics(0);
    }
}

// TODO(Nik-RE-dev): looks like this function is not needed anymore
void BackToHouseMenu() {
    auto pMouse = EngineIocContainer::ResolveMouse();
    pMouse->ClearPickedItem();
    // TODO(Nik-RE-dev): Looks like it's artifact of MM6
#if 0
    if (window_SpeakInHouse && window_SpeakInHouse->wData.val == 165 &&
        !pMovie_Track) {
        bGameoverLoop = true;
        houseDialogPressEscape();
        window_SpeakInHouse->Release();
        pParty->uFlags &= 0xFFFFFFFD;
        if (enterHouse(HOUSE_BODY_GUILD_MASTER_ERATHIA)) {
            pAudioPlayer->playUISound(SOUND_Invalid);
            createHouseUI(HOUSE_BODY_GUILD_MASTER_ERATHIA);
        }
        bGameoverLoop = false;
    }
#endif
}

void playHouseSound(HOUSE_ID houseID, HouseSoundType type) {
    if (houseID != HOUSE_INVALID && pAnimatedRooms[buildingTable[std::to_underlying(houseID) - 1].uAnimationID].uRoomSoundId) {
        int roomSoundId = pAnimatedRooms[buildingTable[std::to_underlying(houseID) - 1].uAnimationID].uRoomSoundId;
        SoundID soundId = SoundID(std::to_underlying(type) + 100 * (roomSoundId + 300));
        pAudioPlayer->playHouseSound(soundId, true);
    }
}

void GUIWindow_House::reinitDialogueWindow() {
    if (pDialogueWindow) {
        pDialogueWindow->Release();
    }

    pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, {render->GetPresentDimensions().w, 345}, 0);
    pBtn_ExitCancel = pDialogueWindow->CreateButton({471, 445}, {169, 35}, 1, 0, UIMSG_Escape, 0, InputAction::Invalid,
        localization->GetString(LSTR_END_CONVERSATION), {ui_exit_cancel_button_background});
    pDialogueWindow->CreateButton({8, 8}, {450, 320}, 1, 0, UIMSG_HouseScreenClick, 0, InputAction::Invalid, "");
}

bool GUIWindow_House::checkIfPlayerCanInteract() {
    if (!pParty->hasActiveCharacter()) {  // to avoid access zeroeleement
        return false;
    }

    // Do nothing if no current conversation exist
    if (!pDialogueWindow) {
        return true;
    }

    if (pParty->activeCharacter().CanAct()) {
        pDialogueWindow->pNumPresenceButton = _savedButtonsNum;
        return true;
    } else {
        pDialogueWindow->pNumPresenceButton = 0;
        GUIWindow window = *window_SpeakInHouse;
        window.uFrameX = SIDE_TEXT_BOX_POS_X;
        window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
        window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

        std::string str = localization->FormatString(LSTR_FMT_S_IS_IN_NO_CODITION_TO_S, pParty->activeCharacter().name.c_str(), localization->GetString(LSTR_DO_ANYTHING));
        window.DrawTitleText(pFontArrus, 0, (212 - pFontArrus->CalcTextHeight(str, window.uFrameWidth, 0)) / 2 + 101, ui_house_player_cant_interact_color, str, 3);
        return false;
    }
}

// TODO(Nik-RE-dev): maybe need to unify selectColor for all dialogue
void GUIWindow_House::drawOptions(std::vector<std::string> &optionsText, Color selectColor, int topOptionShift, bool denseSpacing) {
    GUIWindow window = *this;
    window.uFrameX = SIDE_TEXT_BOX_POS_X;
    window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    assert(optionsText.size() == pDialogueWindow->pNumPresenceButton);

    int allTextHeight = 0;
    int activeOptions = 0;
    for (int i = 0; i < optionsText.size(); ++i) {
        if (!optionsText[i].empty()) {
            allTextHeight += pFontArrus->CalcTextHeight(optionsText[i], window.uFrameWidth, 0);
            activeOptions++;
        }
    }

    int spacing = 0;
    int offset = topOptionShift;
    if (!denseSpacing) {
        spacing = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - topOptionShift - allTextHeight) / activeOptions;
        if (spacing > SIDE_TEXT_BOX_MAX_SPACING) {
            spacing = SIDE_TEXT_BOX_MAX_SPACING;
        }
        offset += (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - topOptionShift - spacing * activeOptions - allTextHeight) / 2 - spacing / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
    }

    for (int i = 0; i < pDialogueWindow->pNumPresenceButton; ++i) {
        int buttonIndex = i + pDialogueWindow->pStartingPosActiveItem;
        GUIButton *button = pDialogueWindow->GetControl(buttonIndex);

        if (!optionsText[i].empty()) {
            Color textColor = (pDialogueWindow->pCurrentPosActiveItem == buttonIndex) ? selectColor : textColor = colorTable.White;
            int textHeight = pFontArrus->CalcTextHeight(optionsText[i], window.uFrameWidth, 0);
            button->uY = spacing + offset;
            button->uHeight = textHeight;
            button->uW = button->uY + textHeight - 1 + 6;
            if (denseSpacing) {
                offset += pFontArrus->GetHeight() - 3 + textHeight;
            } else {
                offset = button->uW;
            }
            window.DrawTitleText(pFontArrus, 0, button->uY, textColor, optionsText[i], 3);
        } else if (button) {
            button->uW = 0;
            button->uHeight = 0;
            button->uY = 0;
        }
    }
}

void GUIWindow_House::houseDialogManager() {
    assert(window_SpeakInHouse != nullptr);

    GUIWindow pWindow = *this;
    pWindow.uFrameWidth -= 18;
    pWindow.uFrameZ -= 18;
    render->DrawTextureNew(477 / 640.0f, 0, game_ui_dialogue_background);
    render->DrawTextureNew(468 / 640.0f, 0, game_ui_right_panel_frame);

    if (pDialogueNPCCount != uNumDialogueNPCPortraits || !uHouse_ExitPic) {
        if (!buildingTable[wData.val - 1].pName.empty()) {
            if (current_screen_type != CURRENT_SCREEN::SCREEN_SHOP_INVENTORY) {
                int v3 = 2 * pFontCreate->GetHeight() - 6 - pFontCreate->CalcTextHeight(buildingTable[wData.val - 1].pName, 130, 0);
                if (v3 < 0)
                    v3 = 0;
                pWindow.DrawTitleText(pFontCreate, 0x1EAu, v3 / 2 + 4, colorTable.White, buildingTable[wData.val - 1].pName, 3);
            }
        }
    }

    pWindow.uFrameWidth += 8;
    pWindow.uFrameZ += 8;
    if (!pDialogueNPCCount) {
        if (in_current_building_type == BUILDING_JAIL) {
            houseSpecificDialogue();
            if (pDialogueNPCCount == uNumDialogueNPCPortraits && uHouse_ExitPic) {
                render->DrawTextureNew(556 / 640.0f, 451 / 480.0f, dialogue_ui_x_x_u);
                render->DrawTextureNew(476 / 640.0f, 451 / 480.0f, dialogue_ui_x_ok_u);
            } else {
                render->DrawTextureNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);
            }
            return;
        }
        if (!current_npc_text.empty()) {
            GUIWindow pDialogWindow;
            pDialogWindow.uFrameWidth = 458;
            pDialogWindow.uFrameZ = 457;
            int pTextHeight = pFontArrus->CalcTextHeight(current_npc_text, pDialogWindow.uFrameWidth, 13);
            int v6 = pTextHeight + 7;
            render->DrawTextureCustomHeight(8 / 640.0f, (352 - (pTextHeight + 7)) / 480.0f, ui_leather_mm7, pTextHeight + 7);
            render->DrawTextureNew(8 / 640.0f, (347 - v6) / 480.0f, _591428_endcap);
            DrawText(pFontArrus, {13, 354 - v6}, colorTable.White, pFontArrus->FitTextInAWindow(current_npc_text, pDialogWindow.uFrameWidth, 13));
        }
        if (uNumDialogueNPCPortraits <= 0) {
            if (pDialogueNPCCount == uNumDialogueNPCPortraits &&
                uHouse_ExitPic) {
                render->DrawTextureNew(556 / 640.0f, 451 / 480.0f, dialogue_ui_x_x_u);
                render->DrawTextureNew(476 / 640.0f, 451 / 480.0f, dialogue_ui_x_ok_u);
            } else {
                render->DrawTextureNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);
            }
            return;
        }

        for (int v8 = 0; v8 < uNumDialogueNPCPortraits; ++v8) {
            render->DrawTextureNew((pNPCPortraits_x[uNumDialogueNPCPortraits - 1][v8] - 4) / 640.0f,
                                   (pNPCPortraits_y[uNumDialogueNPCPortraits - 1][v8] - 4) / 480.0f, game_ui_evtnpc);
            render->DrawTextureNew(pNPCPortraits_x[uNumDialogueNPCPortraits - 1][v8] / 640.0f,
                                   pNPCPortraits_y[uNumDialogueNPCPortraits - 1][v8] / 480.0f, pDialogueNPCPortraits[v8]);
            if (uNumDialogueNPCPortraits < 4) {
                std::string pTitleText;
                int v9 = 0;
                if (v8 + 1 == uNumDialogueNPCPortraits && uHouse_ExitPic) {
                    pTitleText = pMapStats->pInfos[uHouse_ExitPic].pName;
                    v9 = 94 * v8 + SIDE_TEXT_BOX_POS_Y;
                } else {
                    if (!v8 && dword_591080) {
                        pTitleText = buildingTable[wData.val - 1].pProprieterTitle;
                        pWindow.DrawTitleText(pFontCreate, SIDE_TEXT_BOX_POS_X, SIDE_TEXT_BOX_POS_Y, colorTable.EasternBlue, pTitleText, 3);
                        continue;
                    }
                    pTitleText = HouseNPCData[v8 + 1 - (dword_591080 != 0)]->pName;
                    v9 = pNPCPortraits_y[uNumDialogueNPCPortraits - 1][v8] + pDialogueNPCPortraits[v8]->height() + 2;
                }
                pWindow.DrawTitleText(pFontCreate, SIDE_TEXT_BOX_POS_X, v9, colorTable.EasternBlue, pTitleText, 3);
            }
        }
        if (pDialogueNPCCount == uNumDialogueNPCPortraits && uHouse_ExitPic) {
            render->DrawTextureNew(556 / 640.0f, 451 / 480.0f, dialogue_ui_x_x_u);
            render->DrawTextureNew(476 / 640.0f, 451 / 480.0f, dialogue_ui_x_ok_u);
        } else {
            render->DrawTextureNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);
        }
        return;
    }

    int v4 = pDialogueNPCCount - 1;
    render->DrawTextureNew((pNPCPortraits_x[0][0] - 4) / 640.0f, (pNPCPortraits_y[0][0] - 4) / 480.0f, game_ui_evtnpc);
    render->DrawTextureNew(pNPCPortraits_x[0][0] / 640.0f, pNPCPortraits_y[0][0] / 480.0f, pDialogueNPCPortraits[v4]);
    if (current_screen_type == CURRENT_SCREEN::SCREEN_SHOP_INVENTORY) {
        CharacterUI_InventoryTab_Draw(&pParty->activeCharacter(), true);
        if (pDialogueNPCCount == uNumDialogueNPCPortraits && uHouse_ExitPic) {
            render->DrawTextureNew(556 / 640.0f, 451 / 480.0f, dialogue_ui_x_x_u);
            render->DrawTextureNew(476 / 640.0f, 451 / 480.0f, dialogue_ui_x_ok_u);
        } else {
            render->DrawTextureNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);
        }
        return;
    }
    if (v4 || !dword_591080) {  // emerald isle ship before quest's done
        SimpleHouseDialog();
    } else {
        std::string nameAndTitle = NameAndTitle(buildingTable[wData.val - 1].pProprieterName, buildingTable[wData.val - 1].pProprieterTitle);
        pWindow.DrawTitleText(pFontCreate, SIDE_TEXT_BOX_POS_X, SIDE_TEXT_BOX_POS_Y, colorTable.EasternBlue, nameAndTitle, 3);
        houseSpecificDialogue();
    }
    if (pDialogueNPCCount == uNumDialogueNPCPortraits && uHouse_ExitPic) {
        render->DrawTextureNew(556 / 640.0f, 451 / 480.0f, dialogue_ui_x_x_u);
        render->DrawTextureNew(476 / 640.0f, 451 / 480.0f, dialogue_ui_x_ok_u);
    } else {
        render->DrawTextureNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);
    }
}

void GUIWindow_House::initializeDialog() {
    if (!pDialogueWindow) {
        return;
    }

    std::vector<DIALOGUE_TYPE> optionList = listDialogueOptions(dialog_menu_id);

    if (optionList.size()) {
        for (int i = 0; i < optionList.size(); i++) {
            CreateShopDialogueButtonAtRow(i, optionList[i]);
        }
        pDialogueWindow->_41D08F_set_keyboard_control_group(optionList.size(), 1, 0, 2);
    }
    _savedButtonsNum = pDialogueWindow->pNumPresenceButton;
}

void GUIWindow_House::learnSkillsDialogue() {
    if (!checkIfPlayerCanInteract()) {
        return;
    }

    bool haveLearnableSkills = false;
    std::vector<std::string> optionsText;
    int cost = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(), buildingTable[wData.val - 1]);
    int buttonsLimit = pDialogueWindow->pStartingPosActiveItem + pDialogueWindow->pNumPresenceButton;
    for (int i = pDialogueWindow->pStartingPosActiveItem; i < buttonsLimit; i++) {
        PLAYER_SKILL_TYPE skill = GetLearningDialogueSkill((DIALOGUE_TYPE)pDialogueWindow->GetControl(i)->msg_param);
        if (skillMaxMasteryPerClass[pParty->activeCharacter().classType][skill] != PLAYER_SKILL_MASTERY_NONE &&
            !pParty->activeCharacter().pActiveSkills[skill]) {
            optionsText.push_back(localization->GetSkillName(skill));
            haveLearnableSkills = true;
        } else {
            optionsText.push_back("");
        }
    }

    GUIWindow dialogue = *this;
    dialogue.uFrameX = SIDE_TEXT_BOX_POS_X;
    dialogue.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    dialogue.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    if (!haveLearnableSkills) {
        Player &player = pParty->activeCharacter();
        std::string str = localization->FormatString(LSTR_FMT_SEEK_KNOWLEDGE_ELSEWHERE, player.name.c_str(), localization->GetClassName(player.classType));
        str = str + "\n \n" + localization->GetString(LSTR_NO_FURTHER_OFFERS);

        int text_height = pFontArrus->CalcTextHeight(str, dialogue.uFrameWidth, 0);
        dialogue.DrawTitleText(pFontArrus, 0, (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - text_height) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET, colorTable.PaleCanary, str, 3);
        return;
    }

    std::string skill_price_label = localization->FormatString(LSTR_FMT_SKILL_COST_D, cost);
    dialogue.DrawTitleText(pFontArrus, 0, 146, colorTable.White, skill_price_label, 3);

    drawOptions(optionsText, colorTable.Sunflower, 18);
}

void GUIWindow_House::learnSelectedSkill(PLAYER_SKILL_TYPE skill) {
    int pPrice = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(), buildingTable[wData.val - 1]);
    if (skillMaxMasteryPerClass[pParty->activeCharacter().classType][skill] != PLAYER_SKILL_MASTERY_NONE) {
        if (!pParty->activeCharacter().pActiveSkills[skill]) {
            if (pParty->GetGold() < pPrice) {
                GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
                if (buildingType() == BUILDING_TRAINING_GROUND) {
                    playHouseSound(houseId(), HOUSE_SOUND_TRAINING_NOT_ENOUGH_GOLD);
                } else if (buildingType() == BUILDING_TAVERN) {
                    playHouseSound(houseId(), HOUSE_SOUND_TAVERN_NOT_ENOUGH_GOLD);
                } else {
                    playHouseSound(houseId(), HOUSE_SOUND_GENERAL_NOT_ENOUGH_GOLD);
                }
            } else {
                pParty->TakeGold(pPrice);
                _transactionPerformed = true;
                pParty->activeCharacter().pActiveSkills[skill] = 1;
                pParty->activeCharacter().playReaction(SPEECH_SkillLearned);
            }
        }
    }
}

GUIWindow_House::GUIWindow_House(HOUSE_ID houseId) : GUIWindow(WINDOW_HouseInterior, {0, 0}, render->GetRenderDimensions(), houseId) {
    pEventTimer->Pause();  // pause timer so not attacked

    current_screen_type = CURRENT_SCREEN::SCREEN_HOUSE;
    pBtn_ExitCancel = CreateButton({471, 445}, {169, 35}, 1, 0, UIMSG_Escape, 0, InputAction::Invalid,
                                   localization->GetString(LSTR_EXIT_BUILDING), {ui_exit_cancel_button_background});

    if (in_current_building_type <= BUILDING_MIRRORED_PATH_GUILD) {
        shop_ui_background = assets->getImage_ColorKey(shopBackgroundNames[in_current_building_type]);
    }

    for (int curNpc = 0; curNpc < uNumDialogueNPCPortraits; curNpc++) {
        int labelFmt;
        std::string labelData;
        if (curNpc + 1 == uNumDialogueNPCPortraits && uHouse_ExitPic) {
            labelData = pMapStats->pInfos[uHouse_ExitPic].pName;
            labelFmt = LSTR_FMT_ENTER_S;
        } else {
            if (curNpc || !dword_591080) {
                labelData = HouseNPCData[curNpc + 1 - ((dword_591080 != 0) ? 1 : 0)]->pName;
            } else {
                labelData = buildingTable[houseId - 1].pProprieterName;
            }
            labelFmt = LSTR_FMT_CONVERSE_WITH_S;
        }
        portraitClickLabel[curNpc] = localization->FormatString(labelFmt, labelData.c_str());
        HouseNPCPortraitsButtonsList[curNpc] = CreateButton(
            {pNPCPortraits_x[uNumDialogueNPCPortraits - 1][curNpc], pNPCPortraits_y[uNumDialogueNPCPortraits - 1][curNpc]}, {63, 73}, 1, 0,
            UIMSG_ClickHouseNPCPortrait, curNpc, InputAction::Invalid, portraitClickLabel[curNpc]);
    }
}

void GUIWindow_House::Update() {
    if (!window_SpeakInHouse) {
        return;
    }
    houseDialogManager();
    if (!isShop(houseId())) {
        return;
    }
    if (pParty->PartyTimes.shopBanTimes[houseId()] <= pParty->GetPlayingTime()) {
        pParty->PartyTimes.shopBanTimes[houseId()] = GameTime(0);
        return;
    }
    // dialog_menu_id = DIALOGUE_MAIN;
    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);  // banned from shop so leaving
}

void GUIWindow_House::Release() {
    for (int i = 0; i < uNumDialogueNPCPortraits; ++i) {
        if (pDialogueNPCPortraits[i]) {
            pDialogueNPCPortraits[i]->Release();
            pDialogueNPCPortraits[i] = nullptr;
        }
    }
    uNumDialogueNPCPortraits = 0;

    if (game_ui_dialogue_background) {
        game_ui_dialogue_background->Release();
        game_ui_dialogue_background = nullptr;
    }

    dword_5C35D4 = 0;
    if (engine->config->settings.FlipOnExit.value()) {
        pParty->_viewYaw = (TrigLUT.uIntegerDoublePi - 1) & (TrigLUT.uIntegerPi + pParty->_viewYaw);
        pCamera3D->_viewYaw = pParty->_viewYaw;
    }
    pParty->uFlags |= PARTY_FLAGS_1_ForceRedraw;

    GUIWindow::Release();
}

void GUIWindow_House::houseDialogueOptionSelected(DIALOGUE_TYPE option) {
    // Nothing
}

void GUIWindow_House::houseSpecificDialogue() {
    // Nothing
}

std::vector<DIALOGUE_TYPE> GUIWindow_House::listDialogueOptions(DIALOGUE_TYPE option) {
    return {};
}

DIALOGUE_TYPE GUIWindow_House::getOptionOnEscape() {
    if (dialog_menu_id == DIALOGUE_MAIN) {
        return DIALOGUE_NULL;
    }
    return DIALOGUE_MAIN;
}

void GUIWindow_House::houseScreenClick() {
    // Nothing to do by default
}

void GUIWindow_House::playHouseGoodbyeSpeech() {
    // No speech by default
}
