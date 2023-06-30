#include "UIHouses.h"

#include <cstdlib>
#include <vector>
#include <utility>

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/MapInfo.h"
#include "Engine/Party.h"
#include "Engine/PriceCalculator.h"
#include "Engine/SaveLoad.h"
#include "Engine/Tables/BuildingTable.h"
#include "Engine/Tables/AwardTable.h"
#include "Engine/Tables/TransitionTable.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Events/Processor.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIWindow.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIDialogue.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UITransition.h"
#include "GUI/UI/NPCTopics.h"
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

#include "Library/Random/Random.h"
#include "Utility/String.h"
#include "Utility/Math/TrigLut.h"

using Io::TextInputType;

BuildingType in_current_building_type;  // 00F8B198
DIALOGUE_TYPE dialog_menu_id;     // 00F8B19C

GraphicsImage *_591428_endcap = nullptr;

std::vector<HouseNpcDesc> houseNpcs;
int currentHouseNpc;

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

bool enterHouse(HOUSE_ID uHouseID) {
    GameUI_StatusBar_Clear();
    GameUI_SetStatusBar("");
    engine->_messageQueue->clear();
    uDialogueType = DIALOGUE_NULL;
    keyboardInputHandler->SetWindowInputStatus(WINDOW_INPUT_CANCELLED);
    keyboardInputHandler->ResetKeys();

    if (uHouseID == HOUSE_THRONEROOM_WIN_GOOD || uHouseID == HOUSE_THRONEROOM_WIN_EVIL) {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_ShowGameOverWindow, 0, 0);
        return false;
    }

    current_npc_text.clear();
    render->ClearZBuffer();

    int openHours = buildingTable[uHouseID].uOpenTime;
    int closeHours = buildingTable[uHouseID].uCloseTime;
    GameTime currentTime = pParty->GetPlayingTime();
    GameTime currentTimeDays = GameTime::FromDays(currentTime.GetDays());
    bool isOpened = false;
    GameTime openTime = currentTimeDays.AddHours(openHours);
    GameTime closeTime = currentTimeDays.AddHours(closeHours);

    if (closeHours > openHours) {
        // Store opened within one day
        isOpened = (currentTime >= openTime) && (currentTime <= closeTime);
    } else {
        // Store opens in one day and closes on next day
        isOpened = (currentTime <= closeTime) || (currentTime >= openTime);
    }

    if (!isOpened) {
        int amPmOpen = 0;
        int amPmClose = 0;

        if (openHours > 12) {
            openHours -= 12;
            amPmOpen = 1;
        }
        if (closeHours > 12) {
            closeHours -= 12;
            amPmClose = 1;
        }

        GameUI_SetStatusBar(LSTR_FMT_OPEN_TIME, openHours, localization->GetAmPm(amPmOpen), closeHours, localization->GetAmPm(amPmClose));
        if (pParty->hasActiveCharacter()) {
            pParty->activeCharacter().playReaction(SPEECH_STORE_CLOSED);
        }

        return false;
    }

    if (isShop(uHouseID)) {
        if (!(pParty->PartyTimes.shopBanTimes[uHouseID]) || (pParty->PartyTimes.shopBanTimes[uHouseID] <= pParty->GetPlayingTime())) {
            pParty->PartyTimes.shopBanTimes[uHouseID] = GameTime(0);
        } else {
            GameUI_SetStatusBar(LSTR_BANNED_FROM_SHOP);
            return false;
        }
    }

    uCurrentHouse_Animation = buildingTable[uHouseID].uAnimationID;
    in_current_building_type = pAnimatedRooms[uCurrentHouse_Animation].uBuildingType;
    if (in_current_building_type == BUILDING_THRONE_ROOM && pParty->uFine) {  // going to jail
        uHouseID = HOUSE_JAIL;
        uCurrentHouse_Animation = buildingTable[uHouseID].uAnimationID;
        in_current_building_type = pAnimatedRooms[uCurrentHouse_Animation].uBuildingType;
        restAndHeal(GameTime::FromYears(1));
        ++pParty->uNumPrisonTerms;
        pParty->uFine = 0;
        for (Character &player : pParty->pCharacters) {
            player.timeToRecovery = 0;
            player.uNumDivineInterventionCastsThisDay = 0;
            player.SetVariable(VAR_Award, Award_PrisonTerms);
        }
    }

    currentHouseNpc = -1;
    game_ui_dialogue_background = assets->getImage_Solid(dialogueBackgroundResourceByAlignment[pParty->alignment]);

    prepareHouse(uHouseID);

    if (houseNpcs.size() == 1) {
        currentHouseNpc = 0;
    }
    pMediaPlayer->OpenHouseMovie(pAnimatedRooms[uCurrentHouse_Animation].video_name, 1u);
    if (isMagicGuild(uHouseID)) {
        // TODO(pskelton): check this behaviour
        if (!pParty->hasActiveCharacter()) { // avoid nzi
            pParty->setActiveToFirstCanAct();
        }

        if (!pParty->activeCharacter()._achievedAwardsBits[guildMembershipFlags[uHouseID]]) {
            playHouseSound(uHouseID, HOUSE_SOUND_MAGIC_GUILD_MEMBERS_ONLY);
            return true;
        }
    } else if ((isStable(uHouseID) || isBoat(uHouseID)) && !isTravelAvailable(uHouseID)) {
        return true;
    }
    playHouseSound(uHouseID, HOUSE_SOUND_GENERAL_GREETING);
    return true;
}

void prepareHouse(HOUSE_ID house) {
    // Default proprietor of non-simple houses
    int proprietorId = pAnimatedRooms[buildingTable[house].uAnimationID].house_npc_id;
    if (proprietorId) {
        HouseNpcDesc desc;
        desc.type = HOUSE_PROPRIETOR;
        desc.label = localization->FormatString(LSTR_FMT_CONVERSE_WITH_S, buildingTable[house].pProprieterName.c_str());
        desc.icon = assets->getImage_ColorKey(fmt::format("npc{:03}", proprietorId));

        houseNpcs.push_back(desc);
    }

    // NPCs of this house
    for (int i = 1; i < pNPCStats->uNumNewNPCs; ++i) {
        if (pNPCStats->pNewNPCData[i].Location2D == house) {
            if (!(pNPCStats->pNewNPCData[i].uFlags & NPC_HIRED)) {
                HouseNpcDesc desc;
                desc.type = HOUSE_NPC;
                desc.label = localization->FormatString(LSTR_FMT_CONVERSE_WITH_S, pNPCStats->pNewNPCData[i].pName.c_str());
                desc.icon = assets->getImage_ColorKey(fmt::format("npc{:03}", pNPCStats->pNewNPCData[i].uPortraitID));
                desc.npc = &pNPCStats->pNewNPCData[i];

                houseNpcs.push_back(desc);
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

    // Dungeon entry (not present in MM7)
    if (buildingTable[house].uExitPicID) {
        if (!buildingTable[house]._quest_bit || !pParty->_questBits[buildingTable[house]._quest_bit]) {
            int id = buildingTable[house].uExitMapID;

            HouseNpcDesc desc;
            desc.type = HOUSE_TRANSITION;
            desc.label = localization->FormatString(LSTR_FMT_ENTER_S, pMapStats->pInfos[id].pName.c_str());
            desc.icon = assets->getImage_ColorKey(pHouse_ExitPictures[id]);
            desc.targetMapID = id;

            houseNpcs.push_back(desc);
        }
    }
}

void ClickNPCTopic(DIALOGUE_TYPE topic) {
    int pEventNumber;  // ecx@8
    char *v12;         // eax@53
    char *v13;         // eax@56
    char *v14;         // eax@57
    char *v15;         // eax@58
    int pPrice;        // ecx@70

    uDialogueType = (DIALOGUE_TYPE)(topic + 1);
    NPCData *pCurrentNPCInfo = houseNpcs[currentHouseNpc].npc;
    if (topic <= DIALOGUE_SCRIPTED_LINE_6) {
        switch (topic) {
        case DIALOGUE_13_hiring_related:
            current_npc_text = BuildDialogueString(
                pNPCStats->pProfessions[pCurrentNPCInfo->profession].pJoinText,
                pParty->activeCharacterIndex() - 1, 0, HOUSE_INVALID, 0);
            NPCHireableDialogPrepare();
            dialogue_show_profession_details = false;
            BackToHouseMenu();
            return;
        case DIALOGUE_SCRIPTED_LINE_1:
            pEventNumber = pCurrentNPCInfo->dialogue_1_evt_id;
            break;
        case DIALOGUE_SCRIPTED_LINE_2:
            pEventNumber = pCurrentNPCInfo->dialogue_2_evt_id;
            break;
        case DIALOGUE_SCRIPTED_LINE_3:
            pEventNumber = pCurrentNPCInfo->dialogue_3_evt_id;
            break;
        case DIALOGUE_SCRIPTED_LINE_4:
            pEventNumber = pCurrentNPCInfo->dialogue_4_evt_id;
            break;
        case DIALOGUE_SCRIPTED_LINE_5:
            pEventNumber = pCurrentNPCInfo->dialogue_5_evt_id;
            break;
        case DIALOGUE_SCRIPTED_LINE_6:
            pEventNumber = pCurrentNPCInfo->dialogue_6_evt_id;
            break;
        default:
            BackToHouseMenu();
            return;
        }

        if (pEventNumber < 200 || pEventNumber > 310) {
            if (pEventNumber < 400 || pEventNumber > 410) {
                if (pEventNumber == 139) {
                    OracleDialogue();
                } else {
                    if (pEventNumber == 311) {
                        // TODO(Nik-RE-dev): event 311 belongs to one of the teleports in Bracada
                        __debugbreak();
                        //openBountyHuntingDialogue();
                    } else {
                        current_npc_text.clear();
                        activeLevelDecoration = (LevelDecoration *)1;
                        eventProcessor(pEventNumber, 0, 1);
                        activeLevelDecoration = nullptr;
                    }
                }
            } else {
                _dword_F8B1D8_last_npc_topic_menu = topic;
                DrawJoinGuildWindow((GUILD_ID)(pEventNumber - 400));
            }
        } else {
            _4B3FE5_training_dialogue(pEventNumber);
        }
        BackToHouseMenu();
        return;
    }
    if (topic != DIALOGUE_HIRE_FIRE) {
        if (topic == DIALOGUE_PROFESSION_DETAILS) {
            // uBoxHeight = pCurrentNPCInfo->uProfession;
            __debugbreak();  // probably hirelings found in buildings, not
                             // present in MM7, changed
                             // "pCurrentNPCInfo->uProfession - 1" to
                             // "pCurrentNPCInfo->uProfession", have to check in
                             // other versions whether it's ok
            if (dialogue_show_profession_details) {
                current_npc_text = BuildDialogueString(
                    pNPCStats->pProfessions[pCurrentNPCInfo->profession].pJoinText,
                    pParty->activeCharacterIndex() - 1, 0, HOUSE_INVALID, 0);
            } else {
                current_npc_text = BuildDialogueString(
                    pNPCStats->pProfessions[pCurrentNPCInfo->profession].pBenefits,
                    pParty->activeCharacterIndex() - 1, 0, HOUSE_INVALID, 0);
            }
            dialogue_show_profession_details = ~dialogue_show_profession_details;
        } else {
            if (topic == DIALOGUE_79_mastery_teacher) {
                if (guild_membership_approved) {
                    pParty->TakeGold(gold_transaction_amount);
                    if (pParty->hasActiveCharacter()) {
                        pParty->activeCharacter().SetSkillMastery(dword_F8B1AC_skill_being_taught, dword_F8B1B0_MasteryBeingTaught);
                        pParty->activeCharacter().playReaction(SPEECH_SKILL_MASTERY_INC);
                    }
                    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
                }
            } else {
                if (topic == DIALOGUE_82_join_guild && guild_membership_approved) {
                    // join guild
                    pParty->TakeGold(gold_transaction_amount, true);
                    for (Character &player : pParty->pCharacters)
                        player.SetVariable(VAR_Award, dword_F8B1AC_award_bit_number);

                    switch (_dword_F8B1D8_last_npc_topic_menu) {
                    case DIALOGUE_SCRIPTED_LINE_1:
                        pEventNumber = pCurrentNPCInfo->dialogue_1_evt_id;
                        if (pEventNumber >= 400 && pEventNumber <= 416)
                            pCurrentNPCInfo->dialogue_1_evt_id = 0;
                        break;
                    case DIALOGUE_SCRIPTED_LINE_2:
                        pEventNumber = pCurrentNPCInfo->dialogue_2_evt_id;
                        if (pEventNumber >= 400 && pEventNumber <= 416)
                            pCurrentNPCInfo->dialogue_2_evt_id = 0;
                        break;
                    case DIALOGUE_SCRIPTED_LINE_3:
                        pEventNumber = pCurrentNPCInfo->dialogue_3_evt_id;
                        if (pEventNumber >= 400 && pEventNumber <= 416)
                            pCurrentNPCInfo->dialogue_3_evt_id = 0;
                        break;
                    case DIALOGUE_SCRIPTED_LINE_4:
                        pEventNumber = pCurrentNPCInfo->dialogue_4_evt_id;
                        if (pEventNumber >= 400 && pEventNumber <= 416)
                            pCurrentNPCInfo->dialogue_4_evt_id = 0;
                        break;
                    case DIALOGUE_SCRIPTED_LINE_5:
                        pEventNumber = pCurrentNPCInfo->dialogue_5_evt_id;
                        if (pEventNumber >= 400 && pEventNumber <= 416)
                            pCurrentNPCInfo->dialogue_5_evt_id = 0;
                        break;
                    case DIALOGUE_SCRIPTED_LINE_6:
                        pEventNumber = pCurrentNPCInfo->dialogue_6_evt_id;
                        if (pEventNumber >= 400 && pEventNumber <= 416)
                            pCurrentNPCInfo->dialogue_6_evt_id = 0;
                        break;
                    default:
                        break;
                    }
                    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
                    if (pParty->hasActiveCharacter()) {
                        pParty->activeCharacter().playReaction(SPEECH_JOINED_GUILD);
                        BackToHouseMenu();
                        return;
                    }
                }
            }
        }
        BackToHouseMenu();
        return;
    }

    if (!pParty->pHirelings[0].pName.empty() && !pParty->pHirelings[1].pName.empty()) {
        GameUI_SetStatusBar(LSTR_HIRE_NO_ROOM);
        BackToHouseMenu();
        return;
    }

    if (pCurrentNPCInfo->profession != Burglar) {  // burglars have no hiring price
        __debugbreak();  // probably hirelings found in buildings, not present
                         // in MM7, changed "pCurrentNPCInfo->uProfession - 1"
                         // to "pCurrentNPCInfo->uProfession", have to check in
                         // other versions whether it's ok
        pPrice =
            pNPCStats->pProfessions[pCurrentNPCInfo->profession].uHirePrice;
        if (pParty->GetGold() < (unsigned int)pPrice) {
            GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
            dialogue_show_profession_details = false;
            uDialogueType = DIALOGUE_13_hiring_related;
            current_npc_text = BuildDialogueString(
                pNPCStats->pProfessions[pCurrentNPCInfo->profession].pJoinText,
                pParty->activeCharacterIndex() - 1, 0, HOUSE_INVALID, 0);
            if (pParty->hasActiveCharacter()) {
                pParty->activeCharacter().playReaction(SPEECH_NOT_ENOUGH_GOLD);
            }
            GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
            BackToHouseMenu();
            return;
        } else {
            pParty->TakeGold(pPrice);
        }
    }

    pCurrentNPCInfo->uFlags |= NPC_HIRED;
    pParty->hirelingScrollPosition = 0;
    pParty->CountHirelings();
    if (!pParty->pHirelings[0].pName.empty()) {
        pParty->pHirelings[1] = *pCurrentNPCInfo;
        pParty->pHireling2Name = pCurrentNPCInfo->pName;
    } else {
        pParty->pHirelings[0] = *pCurrentNPCInfo;
        pParty->pHireling1Name = pCurrentNPCInfo->pName;
    }
    pParty->hirelingScrollPosition = 0;
    pParty->CountHirelings();
    prepareHouse(window_SpeakInHouse->houseId());
    dialog_menu_id = DIALOGUE_MAIN;

    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 1, 0);
    if (pParty->hasActiveCharacter()) {
        pParty->activeCharacter().playReaction(SPEECH_HIRE_NPC);
    }

    BackToHouseMenu();
}

void updateNPCTopics(int npc) {
    int num_menu_buttons = 0;

    currentHouseNpc = npc;
    Sizei renDims = render->GetRenderDimensions();
    if (houseNpcs[npc].type == HOUSE_TRANSITION) {
        pDialogueWindow->Release();
        pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, renDims, 0);
        transition_button_label = houseNpcs[npc].label;
        pBtn_ExitCancel = pDialogueWindow->CreateButton({566, 445}, {75, 33}, 1, 0, UIMSG_Escape, 0, Io::InputAction::No, localization->GetString(LSTR_CANCEL), {ui_buttdesc2});
        pBtn_YES = pDialogueWindow->CreateButton({486, 445}, {75, 33}, 1, 0, UIMSG_HouseTransitionConfirmation, 1, Io::InputAction::Yes, transition_button_label, {ui_buttyes2});
        pDialogueWindow->CreateButton({pNPCPortraits_x[0][0], pNPCPortraits_y[0][0]}, {63, 73}, 1, 0, UIMSG_HouseTransitionConfirmation, 1,
                                      Io::InputAction::EventTrigger, transition_button_label);
        pDialogueWindow->CreateButton({8, 8}, {460, 344}, 1, 0, UIMSG_HouseTransitionConfirmation, 1, Io::InputAction::Yes, transition_button_label);
    } else {
        if (dialog_menu_id == DIALOGUE_OTHER) {
            pDialogueWindow->Release();
        } else {
            for (int i = 0; i < houseNpcs.size(); ++i) {
                houseNpcs[i].button->Release();
                houseNpcs[i].button = nullptr;
            }
        }
        pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, {renDims.w, 345}, 0);
        pBtn_ExitCancel = pDialogueWindow->CreateButton({471, 445}, {169, 35}, 1, 0, UIMSG_Escape, 0, Io::InputAction::Invalid,
                                                        localization->GetString(LSTR_END_CONVERSATION), {ui_exit_cancel_button_background});
        pDialogueWindow->CreateButton({8, 8}, {450, 320}, 1, 0, UIMSG_HouseScreenClick, 0);
        dialog_menu_id = DIALOGUE_MAIN;
        if (houseNpcs[npc].type == HOUSE_PROPRIETOR) {
            window_SpeakInHouse->initializeDialog();
        } else { // NPC
            NPCData *npcData = houseNpcs[npc].npc;
            if (npcData->is_joinable) {
                num_menu_buttons = 1;
                pDialogueWindow->CreateButton({480, 160}, {140, 30}, 1, 0, UIMSG_ClickNPCTopic, DIALOGUE_13_hiring_related);
            }

            #define AddScriptedDialogueLine(EVENT_ID, MSG_PARAM) \
                if (EVENT_ID) { \
                    if (num_menu_buttons < 4) { \
                        int res = npcDialogueEventProcessor(EVENT_ID); \
                        if (res == 1 || res == 2) \
                            pDialogueWindow->CreateButton({480, 160 + 30 * num_menu_buttons++}, {140, 30}, 1, 0, UIMSG_ClickNPCTopic, MSG_PARAM); \
                    } \
                }

            AddScriptedDialogueLine(npcData->dialogue_1_evt_id, DIALOGUE_SCRIPTED_LINE_1);
            AddScriptedDialogueLine(npcData->dialogue_2_evt_id, DIALOGUE_SCRIPTED_LINE_2);
            AddScriptedDialogueLine(npcData->dialogue_3_evt_id, DIALOGUE_SCRIPTED_LINE_3);
            AddScriptedDialogueLine(npcData->dialogue_4_evt_id, DIALOGUE_SCRIPTED_LINE_4);
            AddScriptedDialogueLine(npcData->dialogue_5_evt_id, DIALOGUE_SCRIPTED_LINE_5);
            AddScriptedDialogueLine(npcData->dialogue_6_evt_id, DIALOGUE_SCRIPTED_LINE_6);

            pDialogueWindow->_41D08F_set_keyboard_control_group(num_menu_buttons, 1, 0, 2);
            // TODO(Nik-RE-dev): initial number of buttons used only in non-simple houses now
            //                   but this causes situation where dead character can talk to someone
            //dword_F8B1E0 = pDialogueWindow->pNumPresenceButton;
        }
    }
}

void onSelectHouseDialogueOption(DIALOGUE_TYPE option) {
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
    engine->_messageQueue->clear();
    keyboardInputHandler->SetWindowInputStatus(WINDOW_INPUT_CANCELLED);
    keyboardInputHandler->ResetKeys();
    activeLevelDecoration = nullptr;
    current_npc_text.clear();

    if (currentHouseNpc == -1) {
        return false;
    }

    if (dialog_menu_id == DIALOGUE_OTHER) {
        updateNPCTopics(currentHouseNpc);
        BackToHouseMenu();
        return true;
    }

    if (dialog_menu_id == DIALOGUE_NULL || dialog_menu_id == DIALOGUE_MAIN) {
        currentHouseNpc = -1;
        if (pDialogueWindow) {
            pDialogueWindow->Release();
        }
        if (shop_ui_background) {
            shop_ui_background->Release();
            shop_ui_background = nullptr;
        }
        dialog_menu_id = DIALOGUE_NULL;
        pDialogueWindow = nullptr;

        if (houseNpcs.size() == 1) {
            return false;
        }

        pBtn_ExitCancel = window_SpeakInHouse->vButtons.front();
        for (int i = 0; i < houseNpcs.size(); ++i) {
            Pointi pos = {pNPCPortraits_x[houseNpcs.size() - 1][i], pNPCPortraits_y[houseNpcs.size() - 1][i]};
            houseNpcs[i].button = window_SpeakInHouse->CreateButton(pos, {63, 73}, 1, 0, UIMSG_ClickHouseNPCPortrait, i,
                                                                               Io::InputAction::Invalid, houseNpcs[i].label);
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
      case BUILDING_ELEMENTAL_GUILD:
      case BUILDING_SELF_GUILD:
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
    window_SpeakInHouse->CreateButton({61, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 1, Io::InputAction::SelectChar1, "");
    window_SpeakInHouse->CreateButton({177, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 2, Io::InputAction::SelectChar2, "");
    window_SpeakInHouse->CreateButton({292, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 3, Io::InputAction::SelectChar3, "");
    window_SpeakInHouse->CreateButton({407, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 4, Io::InputAction::SelectChar4, "");
    window_SpeakInHouse->CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_CycleCharacters, 0, Io::InputAction::CharCycle, "");
    if (houseNpcs.size() == 1) {
        updateNPCTopics(0);
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
    if (houseID != HOUSE_INVALID && pAnimatedRooms[buildingTable[houseID].uAnimationID].uRoomSoundId) {
        int roomSoundId = pAnimatedRooms[buildingTable[houseID].uAnimationID].uRoomSoundId;
        SoundID soundId = SoundID(std::to_underlying(type) + 100 * (roomSoundId + 300));
        pAudioPlayer->playHouseSound(soundId, true);
    }
}

void GUIWindow_House::houseNPCDialogue() {
    GUIWindow house_window = *pDialogueWindow;
    if (houseNpcs[currentHouseNpc].type == HOUSE_TRANSITION) {
        int id = houseNpcs[currentHouseNpc].targetMapID;
        house_window.uFrameX = 493;
        house_window.uFrameWidth = 126;
        house_window.uFrameZ = 366;
        house_window.DrawTitleText(pFontCreate, 0, 2, colorTable.White, pMapStats->pInfos[id].pName, 3);
        house_window.uFrameX = SIDE_TEXT_BOX_POS_X;
        house_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
        house_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;
        if (pTransitionStrings[id].empty()) {
            auto str = localization->FormatString(LSTR_FMT_ENTER_S, pMapStats->pInfos[id].pName.c_str());
            house_window.DrawTitleText(pFontCreate, 0, (212 - pFontCreate->CalcTextHeight(str, house_window.uFrameWidth, 0)) / 2 + 101, colorTable.White, str, 3);
            return;
        }

        int vertMargin = (212 - pFontCreate->CalcTextHeight(pTransitionStrings[id], house_window.uFrameWidth, 0)) / 2 + 101;
        house_window.DrawTitleText(pFontCreate, 0, vertMargin, colorTable.White, pTransitionStrings[id], 3);
        return;
    }

    house_window.uFrameWidth -= 10;
    house_window.uFrameZ -= 10;
    NPCData *pNPC = houseNpcs[currentHouseNpc].npc;

    house_window.DrawTitleText(pFontCreate, SIDE_TEXT_BOX_POS_X, SIDE_TEXT_BOX_POS_Y, colorTable.EasternBlue, NameAndTitle(pNPC), 3);

    if (houseNpcs[0].type != HOUSE_PROPRIETOR) {
        if (uDialogueType == DIALOGUE_NULL) {
            if (pNPC->greet) {
                std::string greetString;

                house_window.uFrameWidth = game_viewport_width;
                house_window.uFrameZ = 452;
                if (pNPC->uFlags & NPC_GREETED_SECOND) {
                    greetString = pNPCStats->pNPCGreetings[pNPC->greet].pGreeting2;
                } else {
                    greetString = pNPCStats->pNPCGreetings[pNPC->greet].pGreeting1;
                }

                int textHeight = pFontArrus->CalcTextHeight(greetString, house_window.uFrameWidth, 13) + 7;
                render->DrawTextureCustomHeight(8 / 640.0f, (352 - textHeight) / 480.0f, ui_leather_mm7, textHeight);
                render->DrawTextureNew(8 / 640.0f, (347 - textHeight) / 480.0f, _591428_endcap);
                pDialogueWindow->DrawText(pFontArrus, {13, 354 - textHeight}, colorTable.White, pFontArrus->FitTextInAWindow(greetString, house_window.uFrameWidth, 13));
            }
        }
    }

    // for right panel
    GUIWindow right_panel_window = *pDialogueWindow;
    right_panel_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    right_panel_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    right_panel_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    std::vector<std::string> optionsText;

    int buttonLimit = pDialogueWindow->pStartingPosActiveItem + pDialogueWindow->pNumPresenceButton;
    for (int i = pDialogueWindow->pStartingPosActiveItem; i < buttonLimit; ++i) {
        GUIButton *pButton = right_panel_window.GetControl(i);
        switch (pButton->msg_param) {
          case DIALOGUE_SCRIPTED_LINE_1:
            if (pNPCTopics[pNPC->dialogue_1_evt_id].pTopic.empty()) {
                optionsText.push_back("");
                pButton->msg_param = 0;
            } else {
                optionsText.push_back(pNPCTopics[pNPC->dialogue_1_evt_id].pTopic);
            }
            continue;
          case DIALOGUE_SCRIPTED_LINE_2:
            if (pNPCTopics[pNPC->dialogue_2_evt_id].pTopic.empty()) {
                optionsText.push_back("");
                pButton->msg_param = 0;
            } else {
                optionsText.push_back(pNPCTopics[pNPC->dialogue_2_evt_id].pTopic);
            }
            continue;
          case DIALOGUE_SCRIPTED_LINE_3:
            if (pNPCTopics[pNPC->dialogue_3_evt_id].pTopic.empty()) {
                optionsText.push_back("");
                pButton->msg_param = 0;
            } else {
                optionsText.push_back(pNPCTopics[pNPC->dialogue_3_evt_id].pTopic);
            }
            continue;
          case DIALOGUE_SCRIPTED_LINE_4:
            if (pNPCTopics[pNPC->dialogue_4_evt_id].pTopic.empty()) {
                optionsText.push_back("");
                pButton->msg_param = 0;
            } else {
                optionsText.push_back(pNPCTopics[pNPC->dialogue_4_evt_id].pTopic);
            }
            continue;
          case DIALOGUE_SCRIPTED_LINE_5:
            if (pNPCTopics[pNPC->dialogue_5_evt_id].pTopic.empty()) {
                optionsText.push_back("");
                pButton->msg_param = 0;
            } else {
                optionsText.push_back(pNPCTopics[pNPC->dialogue_5_evt_id].pTopic);
            }
            continue;
          case DIALOGUE_SCRIPTED_LINE_6:
            if (pNPCTopics[pNPC->dialogue_6_evt_id].pTopic.empty()) {
                optionsText.push_back("");
                pButton->msg_param = 0;
            } else {
                optionsText.push_back(pNPCTopics[pNPC->dialogue_6_evt_id].pTopic);
            }
            continue;
          case DIALOGUE_HIRE_FIRE:
            optionsText.push_back(localization->GetString(LSTR_HIRE));
            continue;
          case DIALOGUE_PROFESSION_DETAILS:
            optionsText.push_back(localization->GetString(LSTR_MORE_INFORMATION));
            continue;
          case DIALOGUE_79_mastery_teacher:
            optionsText.push_back(_4B254D_SkillMasteryTeacher(right_panel_window.wData.val));
            continue;
          case DIALOGUE_82_join_guild:
            optionsText.push_back(GetJoinGuildDialogueOption(static_cast<GUILD_ID>(right_panel_window.wData.val)));
            continue;
          case DIALOGUE_83_bounty_hunting:
            current_npc_text = ((GUIWindow_TownHall*)window_SpeakInHouse)->bountyHuntingText();
            optionsText.push_back("");
            continue;
        }

        if (pButton->msg_param > 0 && pButton->msg_param < DIALOGUE_13_hiring_related) {
            optionsText.push_back(localization->GetString(LSTR_JOIN));
            continue;
        }
        if (pButton->msg_param > DIALOGUE_13_hiring_related && pButton->msg_param < DIALOGUE_SCRIPTED_LINE_1) {
            optionsText.push_back("");
            continue;
        }
        if (pButton->msg_param != DIALOGUE_93) {
            optionsText.push_back("");
            continue;
        }
    }

    if (optionsText.size()) {
        drawOptions(optionsText, colorTable.Jonquil);
    }

    if (current_npc_text.length() > 0) {
        GUIWindow win;

        win.uFrameWidth = 458;
        win.uFrameZ = 457;
        GUIFont *pTextFont = pFontArrus;
        int pTextHeight = pFontArrus->CalcTextHeight(current_npc_text, win.uFrameWidth, 13) + 7;
        if (352 - pTextHeight < 8) {
            pTextFont = pFontCreate;
            pTextHeight = pFontCreate->CalcTextHeight(current_npc_text, win.uFrameWidth, 13) + 7;
        }
        render->DrawTextureCustomHeight(8 / 640.0f, (352 - pTextHeight) / 480.0f, ui_leather_mm7, pTextHeight);
        render->DrawTextureNew(8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
        house_window.DrawText(pTextFont, {13, 354 - pTextHeight}, colorTable.White, pTextFont->FitTextInAWindow(current_npc_text, win.uFrameWidth, 13));
    }
}

void GUIWindow_House::reinitDialogueWindow() {
    if (pDialogueWindow) {
        pDialogueWindow->Release();
    }

    pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, {render->GetPresentDimensions().w, 345}, 0);
    pBtn_ExitCancel = pDialogueWindow->CreateButton({471, 445}, {169, 35}, 1, 0, UIMSG_Escape, 0, Io::InputAction::Invalid,
        localization->GetString(LSTR_END_CONVERSATION), {ui_exit_cancel_button_background});
    pDialogueWindow->CreateButton({8, 8}, {450, 320}, 1, 0, UIMSG_HouseScreenClick, 0, Io::InputAction::Invalid, "");
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
        spacing = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - topOptionShift - allTextHeight) / (activeOptions ? activeOptions : 1);
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
            button->sLabel = optionsText[i];
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
            button->sLabel.clear();
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

    if (currentHouseNpc == -1 || houseNpcs[currentHouseNpc].type != HOUSE_TRANSITION) {
        // Draw house title
        if (!buildingTable[houseId()].pName.empty()) {
            if (current_screen_type != CURRENT_SCREEN::SCREEN_SHOP_INVENTORY) {
                int yPos = 2 * pFontCreate->GetHeight() - 6 - pFontCreate->CalcTextHeight(buildingTable[houseId()].pName, 130, 0);
                if (yPos < 0) {
                    yPos = 0;
                }
                pWindow.DrawTitleText(pFontCreate, 0x1EAu, yPos / 2 + 4, colorTable.White, buildingTable[houseId()].pName, 3);
            }
        }
    }

    pWindow.uFrameWidth += 8;
    pWindow.uFrameZ += 8;
    if (currentHouseNpc == -1) {
        // Either house have no residents or current screen is for selecting resident to begin dialogue
        render->DrawTextureNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);

        if (in_current_building_type == BUILDING_JAIL) {
            houseSpecificDialogue();
            return;
        }
        if (!current_npc_text.empty()) {
            // TODO(Nik-RE-dev): separate text field drawing and merge with similar code from other places
            GUIWindow pDialogWindow;
            pDialogWindow.uFrameWidth = 458;
            pDialogWindow.uFrameZ = 457;
            int pTextHeight = pFontArrus->CalcTextHeight(current_npc_text, pDialogWindow.uFrameWidth, 13);
            int pTextBackgroundHeight = pTextHeight + 7;
            render->DrawTextureCustomHeight(8 / 640.0f, (352 - pTextBackgroundHeight) / 480.0f, ui_leather_mm7, pTextBackgroundHeight);
            render->DrawTextureNew(8 / 640.0f, (347 - pTextBackgroundHeight) / 480.0f, _591428_endcap);
            DrawText(pFontArrus, {13, 354 - pTextBackgroundHeight}, colorTable.White, pFontArrus->FitTextInAWindow(current_npc_text, pDialogWindow.uFrameWidth, 13));
        }

        for (int i = 0; i < houseNpcs.size(); ++i) {
            render->DrawTextureNew((pNPCPortraits_x[houseNpcs.size() - 1][i] - 4) / 640.0f,
                                   (pNPCPortraits_y[houseNpcs.size() - 1][i] - 4) / 480.0f, game_ui_evtnpc);
            render->DrawTextureNew(pNPCPortraits_x[houseNpcs.size() - 1][i] / 640.0f,
                                   pNPCPortraits_y[houseNpcs.size() - 1][i] / 480.0f, houseNpcs[i].icon);
            if (houseNpcs.size() < 4) {
                std::string pTitleText = "";
                int yPos = 0;
                switch (houseNpcs[i].type) {
                  case HOUSE_TRANSITION:
                    pTitleText = pMapStats->pInfos[houseNpcs[i].targetMapID].pName;
                    yPos = 94 * i + SIDE_TEXT_BOX_POS_Y;
                    break;
                  case HOUSE_PROPRIETOR:
                    pTitleText = buildingTable[houseId()].pProprieterTitle;
                    yPos = SIDE_TEXT_BOX_POS_Y;
                    break;
                  case HOUSE_NPC:
                    pTitleText = houseNpcs[i].npc->pName;
                    yPos = pNPCPortraits_y[houseNpcs.size() - 1][i] + houseNpcs[i].icon->height() + 2;
                    break;
                }
                pWindow.DrawTitleText(pFontCreate, SIDE_TEXT_BOX_POS_X, yPos, colorTable.EasternBlue, pTitleText, 3);
            }
        }
        return;
    }

    render->DrawTextureNew((pNPCPortraits_x[0][0] - 4) / 640.0f, (pNPCPortraits_y[0][0] - 4) / 480.0f, game_ui_evtnpc);
    render->DrawTextureNew(pNPCPortraits_x[0][0] / 640.0f, pNPCPortraits_y[0][0] / 480.0f, houseNpcs[currentHouseNpc].icon);
    if (current_screen_type == CURRENT_SCREEN::SCREEN_SHOP_INVENTORY) {
        CharacterUI_InventoryTab_Draw(&pParty->activeCharacter(), true);
        render->DrawTextureNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);
        return;
    }
    if (currentHouseNpc || houseNpcs[0].type != HOUSE_PROPRIETOR) {
        // Dialogue with NPC in house
        houseNPCDialogue();
    } else {
        std::string nameAndTitle = NameAndTitle(buildingTable[houseId()].pProprieterName, buildingTable[houseId()].pProprieterTitle);
        pWindow.DrawTitleText(pFontCreate, SIDE_TEXT_BOX_POS_X, SIDE_TEXT_BOX_POS_Y, colorTable.EasternBlue, nameAndTitle, 3);
        houseSpecificDialogue();
    }
    if (currentHouseNpc != -1 && houseNpcs[currentHouseNpc].type == HOUSE_TRANSITION) {
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
            pDialogueWindow->CreateButton({480, 146 + 30 * i}, {140, 30}, 1, 0, UIMSG_SelectHouseDialogueOption, optionList[i], Io::InputAction::Invalid, "");
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
    int cost = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(), buildingTable[houseId()]);
    int buttonsLimit = pDialogueWindow->pStartingPosActiveItem + pDialogueWindow->pNumPresenceButton;
    for (int i = pDialogueWindow->pStartingPosActiveItem; i < buttonsLimit; i++) {
        CharacterSkillType skill = GetLearningDialogueSkill((DIALOGUE_TYPE)pDialogueWindow->GetControl(i)->msg_param);
        if (skillMaxMasteryPerClass[pParty->activeCharacter().classType][skill] != CHARACTER_SKILL_MASTERY_NONE &&
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
        Character &player = pParty->activeCharacter();
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

void GUIWindow_House::learnSelectedSkill(CharacterSkillType skill) {
    int pPrice = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(), buildingTable[houseId()]);
    if (skillMaxMasteryPerClass[pParty->activeCharacter().classType][skill] != CHARACTER_SKILL_MASTERY_NONE) {
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
                pParty->activeCharacter().pActiveSkills[skill] = CombinedSkillValue::novice();
                pParty->activeCharacter().playReaction(SPEECH_SKILL_LEARNED);
            }
        }
    }
}

GUIWindow_House::GUIWindow_House(HOUSE_ID houseId) : GUIWindow(WINDOW_HouseInterior, {0, 0}, render->GetRenderDimensions(), houseId) {
    pEventTimer->Pause();  // pause timer so not attacked

    current_screen_type = CURRENT_SCREEN::SCREEN_HOUSE;
    pBtn_ExitCancel = CreateButton({471, 445}, {169, 35}, 1, 0, UIMSG_Escape, 0, Io::InputAction::Invalid,
                                   localization->GetString(LSTR_EXIT_BUILDING), {ui_exit_cancel_button_background});

    if (in_current_building_type <= BUILDING_MIRRORED_PATH_GUILD) {
        shop_ui_background = assets->getImage_ColorKey(shopBackgroundNames[in_current_building_type]);
    }

    for (int i = 0; i < houseNpcs.size(); ++i) {
        Pointi pos = {pNPCPortraits_x[houseNpcs.size() - 1][i], pNPCPortraits_y[houseNpcs.size() - 1][i]};
        houseNpcs[i].button = CreateButton(pos, {63, 73}, 1, 0, UIMSG_ClickHouseNPCPortrait, i,
                                                      Io::InputAction::Invalid, houseNpcs[i].label);
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
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);  // banned from shop so leaving
}

void GUIWindow_House::Release() {
    for (HouseNpcDesc &desc : houseNpcs) {
        if (desc.icon) {
            desc.icon->Release();
        }
    }
    houseNpcs.clear();

    if (game_ui_dialogue_background) {
        game_ui_dialogue_background->Release();
        game_ui_dialogue_background = nullptr;
    }

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
