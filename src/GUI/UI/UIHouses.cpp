#include "UIHouses.h"

#include <cmath>
#include <cstdlib>
#include <memory>
#include <vector>
#include <utility>
#include <algorithm>
#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Data/AwardEnums.h"
#include "Engine/Data/HouseEnumFunctions.h"
#include "Engine/GameType.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Localization.h"
#include "Engine/Tables/MapTable.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Party.h"
#include "Engine/PriceCalculator.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Tables/HouseTable.h"
#include "Engine/Tables/RosterTable.h"
#include "Engine/Tables/TransitionTable.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIWindow.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIDialogue.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIStatusBar.h"
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

#include "Io/KeyboardInputHandler.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/Audio/SoundList.h"
#include "Media/MediaPlayer.h"

#include "Utility/Math/TrigLut.h"

std::shared_ptr<GraphicsImage> _591428_endcap;

std::vector<HouseNpcDesc> houseNpcs;
int currentHouseNpc;

std::array<const HouseAnimDescr, 196> pAnimatedRooms = { {  // 0x4E5F70
    { "", 0x4, 0x1F4, HOUSE_TYPE_INVALID, 0, 0 },
    { "Human Armor01", 0x20, 0x2C0, HOUSE_TYPE_ARMOR_SHOP, 58, 0 },
    { "Necromancer Armor01", 0x20, 0x2D7, HOUSE_TYPE_ARMOR_SHOP, 70, 0 },
    { "Dwarven Armor01", 0x20, 0x2EE, HOUSE_TYPE_ARMOR_SHOP, 5, 0 },
    { "Wizard Armor", 0x20, 0x3BD, HOUSE_TYPE_ARMOR_SHOP, 19, 0 },
    { "Warlock Armor", 0x20, 0x2D6, HOUSE_TYPE_ARMOR_SHOP, 35, 0 },
    { "Elf Armor", 0x20, 0x2BC, HOUSE_TYPE_ARMOR_SHOP, 79, 0 },
    { "Human Alchemisht01", 0xE, 0x2BE, HOUSE_TYPE_ALCHEMY_SHOP, 95, 0 },
    { "Necromancer Alchemist01", 0xE, 0x2D6, HOUSE_TYPE_ALCHEMY_SHOP, 69, 0 },
    { "Dwarven Achemist01", 0xE, 0x387, HOUSE_TYPE_ALCHEMY_SHOP, 4, 0 },
    { "Wizard Alchemist", 0xE, 0x232, HOUSE_TYPE_ALCHEMY_SHOP, 25, 0 },
    { "Warlock Alchemist", 0xE, 0x2BE, HOUSE_TYPE_ALCHEMY_SHOP, 42, 0 },
    { "Elf Alchemist", 0xE, 0x38A, HOUSE_TYPE_ALCHEMY_SHOP, 84, 0 },
    { "Human Bank01", 0x6, 0x384, HOUSE_TYPE_BANK, 52, 0 },
    { "Necromancer Bank01", 0x6, 0x2D8, HOUSE_TYPE_BANK, 71, 0 },
    { "Dwarven Bank", 0x6, 0x2F3, HOUSE_TYPE_BANK, 6, 0 },
    { "Wizard Bank", 0x6, 0x3BA, HOUSE_TYPE_BANK, 20, 0 },
    { "Warlock Bank", 0x6, 0x39F, HOUSE_TYPE_BANK, 36, 0 },
    { "Elf Bank", 0x6, 0x2BC, HOUSE_TYPE_BANK, 80, 0 }, // Vanilla MM7 has 71, the Evil Bank lines.
    { "Boat01", 0xF, 0x4C, HOUSE_TYPE_BOAT, 53, 3 },
    { "Boat01d", 0xF, 0x4C, HOUSE_TYPE_BOAT, 53, 3 }, // this movie doesn't exist
    { "Human Magic Shop01", 0xA, 0x2C8, HOUSE_TYPE_MAGIC_SHOP, 54, 0 },
    { "Necromancer Magic Shop01", 0xE, 0x2DC, HOUSE_TYPE_MAGIC_SHOP, 66, 0 },
    { "Dwarven Magic Shop01", 0x2A, 0x2EF, HOUSE_TYPE_MAGIC_SHOP, 91, 0 },
    { "Wizard Magic Shop", 0x1E, 0x2DF, HOUSE_TYPE_MAGIC_SHOP, 15, 0 },
    { "Warlock Magic Shop", 0x7, 0x3B9, HOUSE_TYPE_MAGIC_SHOP, 31, 0 },
    { "Elf Magic Shop", 0x24, 0x2CC, HOUSE_TYPE_MAGIC_SHOP, 82, 0 },
    { "Human Stables01", 0x21, 0x31, HOUSE_TYPE_STABLE, 48, 3 },
    { "Necromancer Stables", 0x21, 0x2DD, HOUSE_TYPE_STABLE, 67, 3 },
    { "", 0x21, 0x2F0, HOUSE_TYPE_STABLE, 91, 3 },
    { "Wizard Stables", 0x21, 0x3BA, HOUSE_TYPE_STABLE, 16, 3 },
    { "Warlock Stables", 0x21, 0x181, HOUSE_TYPE_STABLE, 32, 3 },  // movie exist but unused in MM7 as Nighon doesn't have stables
    { "Elf Stables", 0x21, 0x195, HOUSE_TYPE_STABLE, 77, 3 },
    { "Human Tavern01", 0xD, 0x2C2, HOUSE_TYPE_TAVERN, 49, 0 },
    { "Necromancer Tavern 01", 0xD, 0x3B0, HOUSE_TYPE_TAVERN, 57, 0 },
    { "Dwarven Tavern01", 0xD, 0x2FE, HOUSE_TYPE_TAVERN, 94, 0 },
    { "Wizard Tavern", 0xD, 0x3BB, HOUSE_TYPE_TAVERN, 17, 0 },
    { "Warlock Tavern", 0xD, 0x3A8, HOUSE_TYPE_TAVERN, 33, 0 },
    { "Elf Tavern", 0xD, 0x2CD, HOUSE_TYPE_TAVERN, 78, 0 },
    { "Human Temple01", 0x24, 0x2DB, HOUSE_TYPE_TEMPLE, 50, 3 },
    { "Necromancer Temple", 0x24, 0x2DF, HOUSE_TYPE_TEMPLE, 60, 3 },
    { "Dwarven Temple01", 0x24, 0x2F1, HOUSE_TYPE_TEMPLE, 86, 3 },
    { "Wizard Temple", 0x24, 0x2E0, HOUSE_TYPE_TEMPLE, 10, 3 },
    { "Warlock Temple", 0x24, 0x3A4, HOUSE_TYPE_TEMPLE, 27, 3 },
    { "Elf Temple", 0x24, 0x2CE, HOUSE_TYPE_TEMPLE, 72, 3 },
    { "Human Town Hall", 0x10, 0x39C, HOUSE_TYPE_TOWN_HALL, 14, 0 },
    { "Necromancer Town Hall01", 0x10, 0x3A4, HOUSE_TYPE_TOWN_HALL, 61, 0 },
    { "Dwarven Town Hall", 0x10, 0x2DB, HOUSE_TYPE_TOWN_HALL, 88, 0 }, // this movie doesn't exist, stone city doesn't have town hall
    { "Wizard Town Hall", 0x10, 0x3BD, HOUSE_TYPE_TOWN_HALL, 11, 0 },
    { "Warlock Town Hall", 0x10, 0x2DB, HOUSE_TYPE_TOWN_HALL, 28, 0 },
    { "Elf Town Hall", 0x10, 0x27A, HOUSE_TYPE_TOWN_HALL, 73, 0 },
    { "Human Training Ground01", 0x18, 0x2C7, HOUSE_TYPE_TRAINING_GROUND, 44, 0 },
    { "Necromancer Training Ground", 0x18, 0x3AD, HOUSE_TYPE_TRAINING_GROUND, 62, 0 },
    { "Dwarven Training Ground", 0x18, 0x2F2, HOUSE_TYPE_TRAINING_GROUND, 89, 0 },
    { "Wizard Training Ground", 0x18, 0x3A3, HOUSE_TYPE_TRAINING_GROUND, 12, 0 },
    { "Warlock Training Ground", 0x18, 0x3A6, HOUSE_TYPE_TRAINING_GROUND, 29, 0 },
    { "Elf Training Ground", 0x18, 0x19F, HOUSE_TYPE_TRAINING_GROUND, 74, 0 },
    { "Human Weapon Smith01", 0x16, 0x2C1, HOUSE_TYPE_WEAPON_SHOP, 45, 4 },
    { "Necromancer Weapon Smith01", 0x16, 0x2D9, HOUSE_TYPE_WEAPON_SHOP, 63, 4 },
    { "Dwarven Weapon Smith01", 0x16, 0x2EE, HOUSE_TYPE_WEAPON_SHOP, 85, 4 }, // Vanilla MM7 has 82, the Elf Magic Shop's. The sound table loader moves the dwarf smith's lines to the free room 85.
    { "Wizard Weapon Smith", 0x16, 0x2D5, HOUSE_TYPE_WEAPON_SHOP, 13, 4 },
    { "Warlock Weapon Smith", 0x16, 0x2D7, HOUSE_TYPE_WEAPON_SHOP, 23, 4 },
    { "Elf Weapon Smith", 0x16, 0x2CA, HOUSE_TYPE_WEAPON_SHOP, 75, 4 },
    { "Air Guild", 0x1D, 0xA4, HOUSE_TYPE_AIR_GUILD, 1, 3 },
    { "Body Guild", 0x19, 0x3BF, HOUSE_TYPE_BODY_GUILD, 2, 0 },
    { "Dark Guild", 0x19, 0x2D1, HOUSE_TYPE_DARK_GUILD, 3, 0 },
    { "Earth Guild", 0x19, 0x2CB, HOUSE_TYPE_EARTH_GUILD, 83, 0 },
    { "Fire Guild", 0x1C, 0x2BF, HOUSE_TYPE_FIRE_GUILD, 56, 0 },
    { "Light Guild", 0x1C, 0x2D5, HOUSE_TYPE_LIGHT_GUILD, 46, 0 },
    { "Mind Guild", 0x1C, 0xE5, HOUSE_TYPE_MIND_GUILD, 40, 0 },
    { "Spirit Guild", 0x1C, 0x2D2, HOUSE_TYPE_SPIRIT_GUILD, 41, 0 },
    { "Water Guild", 0x1B, 0x2D3, HOUSE_TYPE_WATER_GUILD, 24, 0 },
    { "Lord and Judge Out01", 1, 0, HOUSE_TYPE_HOUSE, 39, 0 },
    { "Human Poor House 1", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Human Poor House 2", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Human Poor House 3", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Human Medium House 1", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Human Medium House 2", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Human Medium House 3", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Human Rich House 1", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Human Rich House 2", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Human Rich House 3", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Elven Poor House 1", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Elven Poor House 2", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Elven Poor House 3", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Elven Medium House 1", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Elven Medium House 2", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Elven Medium House 3", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Elven Rich House 1", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Elven Rich House 2", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Elven Rich House 3", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Dwarven Poor House 1", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Dwarven Poor House 2", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Dwarven Poor House 3", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Dwarven Medium House 1", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Dwarven Medium House 2", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Dwarven Medium House 3", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Dwarven Rich House 1", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Dwarven Rich House 2", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Dwarven Rich House 3", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Wizard Poor House 1", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Wizard Poor House 2", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Wizard Poor House 3", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Wizard Medium House 1", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Wizard Medium House 2", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Wizard Medium House 3", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Wizard Rich House 1", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Wizard Rich House 2", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Wizard Rich House 3", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Necromancer Poor House 1", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Necromancer Poor House 2", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Necromancer Poor House 3", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Necromancer Medium House 1", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Necromancer Medium House 2", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Necromancer Medium House 3", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Necromancer Rich House 1", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Necromancer Rich House 2", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Necromancer Rich House 3", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Warlock Poor House 1", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Warlock Poor House 2", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Warlock Poor House 3", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Warlock Medium House 1", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Warlock Medium House 2", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Warlock Medium House 3", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Warlock Rich House 1", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Warlock Rich House 2", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Warlock Rich House 3", 8, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Out01 Temple of the Moon", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out01 Dragon Cave", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out02 Castle Harmondy", 0x24, 0, HOUSE_TYPE_CASTLE, 0, 0 },
    { "Out02 White Cliff Cave", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out03 Erathian Sewer", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out03 Fort Riverstride", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out03 Castle Gryphonheart", 0x24, 0, HOUSE_TYPE_CASTLE, 0, 0 },
    { "Out04 Elf Castle", 0x24, 0, HOUSE_TYPE_CASTLE, 0, 0 },
    { "Out04 Tularean Caves", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out04 Clanker's Laboratory", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out05 Hall of the Pit", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out05 Watchtower 6", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out06 School of Sorcery", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out06 Red Dwarf Mines", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out07 Castle Lambent", 0x24, 0, HOUSE_TYPE_CASTLE, 0, 0 },
    { "Out07 Walls of Mist", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out07 Temple of the Light", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out08 Evil Entrance", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out08 Breeding Zone", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out08 Temple of the Dark", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out09 Grand Temple of the Moon", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out09 Grand Temple of the Sun", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out10 Thunderfist Mountain", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out10 The Maze", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out10 Connecting Tunnel Cave #1", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out11 Stone City", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out12 Colony Zod", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out12 Connecting Tunnel Cave #1", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out13 Mercenary Guild", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out13 Tidewater Caverns", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out13 Wine Cellar", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out14 Titan's Stronghold", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out14 Temple of Baa", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out14 Hall under the Hill", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Out15 The Linclon", 0x24, 0, HOUSE_TYPE_DUNGEON, 0, 0 },
    { "Jail", 0x24, 0, HOUSE_TYPE_JAIL, 0, 0 },
    { "Harmondale Throne Room", 0x24, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Gryphonheart Throne Room", 0x24, 0, HOUSE_TYPE_THRONE_ROOM, 0, 0 },
    { "Elf Castle Throne Room", 0x24, 0, HOUSE_TYPE_THRONE_ROOM, 0, 0 },
    { "Wizard Castle Throne Room", 0x24, 0, HOUSE_TYPE_THRONE_ROOM, 0, 0 },
    { "Necromancer Castle Throne Rooms", 0x24, 0, HOUSE_TYPE_THRONE_ROOM, 0, 0 },
    { "Master Thief", 0x24, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Dwarven King", 0x24, 0, HOUSE_TYPE_THRONE_ROOM, 0, 0 },
    { "Arms Master", 0x24, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Warlock", 0x24, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Lord Markam", 0x24, 0, HOUSE_TYPE_THRONE_ROOM, 0, 0 },
    { "Arbiter Neutral Town", 0x24, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Arbiter Good Town", 0x24, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Arbiter Evil Town", 0x24, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Necromancer Throne Room Empty", 0x24, 0, HOUSE_TYPE_THRONE_ROOM, 0, 0 },
    { "", 0x24, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "", 0x24, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "", 0x24, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "", 0x24, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Boat01", 0xF, 0, HOUSE_TYPE_HOUSE, 53, 3 },
    { "", 0x24, 0, HOUSE_TYPE_BOAT, 0, 0 },
    { "", 0x24, 0, HOUSE_TYPE_BOAT, 0, 0 },
    { "", 0x24, 0, HOUSE_TYPE_BOAT, 0, 0 },
    { "", 0x24, 0, HOUSE_TYPE_HOUSE, 0, 0 },
    { "Arbiter Room Neutral", 0x24, 0, HOUSE_TYPE_HOUSE, 0, 0 }, // this movie doesn't exist
    { "Out02 Castle Harmondy Abandoned", 0x24, 0, HOUSE_TYPE_CASTLE, 0, 0 },
    { "Human Temple02", 0x24, 0x3AB, HOUSE_TYPE_TEMPLE, 27, 0 },
    { "Player Castle Good", 0x24, 0, HOUSE_TYPE_CASTLE, 0, 0 },
    { "Player Castle Bad", 0x24, 0, HOUSE_TYPE_CASTLE, 0, 0 }
} };

// MM8's per-house animation table (video/npc/sound), distinct from MM7's `pAnimatedRooms` above.
// From MM8's Data/Tables/House Movies.txt. The row index is 2dEvents.txt's "Picture" column.
std::array<const HouseAnimDescr, 161> pAnimatedRoomsMM8 = { {
    { "lmedhs1", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 0
    { "lmedhs1", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 1
    { "lmedhs2", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 2
    { "lporhs1", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 3
    { "lporhs2", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 4
    { "ltemple", 0, 2108, HOUSE_TYPE_TEMPLE, 1, 0 },  // 5
    { "lwpshp", 0, 2109, HOUSE_TYPE_WEAPON_SHOP, 2, 0 },  // 6
    { "larmshp", 0, 2110, HOUSE_TYPE_ARMOR_SHOP, 3, 0 },  // 7
    { "lalcshop", 0, 2111, HOUSE_TYPE_ALCHEMY_SHOP, 4, 0 },  // 8
    { "lmagshp", 0, 2112, HOUSE_TYPE_MAGIC_SHOP, 5, 0 },  // 9
    { "lsplshp", 0, 2113, HOUSE_TYPE_ELEMENTAL_GUILD, 6, 0 },  // 10
    { "ltrhall", 0, 2114, HOUSE_TYPE_TRAINING_GROUND, 7, 0 },  // 11
    { "lbank", 0, 2115, HOUSE_TYPE_BANK, 8, 0 },  // 12
    { "ltavern", 0, 2116, HOUSE_TYPE_TAVERN, 9, 0 },  // 13
    { "lchwar", 0, 0, HOUSE_TYPE_THRONE_ROOM, 0, 0 },  // 14
    { "derchhs1", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 15
    { "derchhs2", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 16
    { "demedhs", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 17
    { "deporhs", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 18
    { "detemple", 0, 2118, HOUSE_TYPE_TEMPLE, 12, 0 },  // 19
    { "dewpshp", 0, 2119, HOUSE_TYPE_WEAPON_SHOP, 13, 0 },  // 20
    { "dearmshp", 0, 2128, HOUSE_TYPE_ARMOR_SHOP, 14, 0 },  // 21
    { "dealcshp", 0, 2121, HOUSE_TYPE_ALCHEMY_SHOP, 15, 0 },  // 22
    { "demagshp", 0, 2102, HOUSE_TYPE_MAGIC_SHOP, 16, 0 },  // 23
    { "desplshp", 0, 2123, HOUSE_TYPE_ELEMENTAL_GUILD, 17, 0 },  // 24
    { "detrhall", 0, 2124, HOUSE_TYPE_TRAINING_GROUND, 18, 0 },  // 25
    { "debank", 0, 2125, HOUSE_TYPE_BANK, 19, 0 },  // 26
    { "detavern", 0, 2126, HOUSE_TYPE_TAVERN, 20, 0 },  // 27
    { "demguild", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 28
    { "deboat", 0, 2127, HOUSE_TYPE_BOAT, 21, 0 },  // 29
    { "decouncl", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 30
    { "mrchhs", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 31
    { "mmedhs", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 32
    { "mporhs", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 33
    { "mdrownhs", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 34
    { "mtemple", 0, 2171, HOUSE_TYPE_TEMPLE, 23, 0 },  // 35
    { "mwepshp", 0, 2130, HOUSE_TYPE_WEAPON_SHOP, 24, 0 },  // 36
    { "marmshp", 0, 2131, HOUSE_TYPE_ARMOR_SHOP, 25, 0 },  // 37
    { "malcshp", 0, 2166, HOUSE_TYPE_ALCHEMY_SHOP, 26, 0 },  // 38
    { "mmagshp", 0, 2167, HOUSE_TYPE_MAGIC_SHOP, 27, 0 },  // 39
    { "msplshp", 0, 2168, HOUSE_TYPE_ELEMENTAL_GUILD, 28, 0 },  // 40
    { "mtrhall", 0, 2170, HOUSE_TYPE_TRAINING_GROUND, 29, 0 },  // 41
    { "mbank", 0, 2136, HOUSE_TYPE_BANK, 30, 0 },  // 42
    { "mtavern", 0, 2169, HOUSE_TYPE_TAVERN, 31, 0 },  // 43
    { "mledden", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 44
    { "dhrchhs", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 45
    { "dhmedhs", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 46
    { "dhporhs", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 47
    { "dhtemple", 0, 1138, HOUSE_TYPE_TEMPLE, 34, 0 },  // 48
    { "dhwepshp", 0, 2139, HOUSE_TYPE_WEAPON_SHOP, 35, 0 },  // 49
    { "dharmshp", 0, 2140, HOUSE_TYPE_ARMOR_SHOP, 36, 0 },  // 50
    { "dhmagshp", 0, 2141, HOUSE_TYPE_MAGIC_SHOP, 38, 0 },  // 51
    { "dhtrhall", 0, 2142, HOUSE_TYPE_TRAINING_GROUND, 40, 0 },  // 52
    { "dhtavern", 0, 1137, HOUSE_TYPE_TAVERN, 42, 0 },  // 53
    { "cquixote", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 54
    { "clrchhs", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 55
    { "clporhs", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 56
    { "tthrone", 0, 0, HOUSE_TYPE_THRONE_ROOM, 0, 0 },  // 57
    { "nrchhs1", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 58
    { "nrchss2", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 59
    { "nmedhs", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 60
    { "nporhs", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 61
    { "ntemple", 0, 2145, HOUSE_TYPE_TEMPLE, 45, 0 },  // 62
    { "nwepshp", 0, 2146, HOUSE_TYPE_WEAPON_SHOP, 46, 0 },  // 63
    { "narmshp", 0, 2147, HOUSE_TYPE_ARMOR_SHOP, 47, 0 },  // 64
    { "nalcshp", 0, 2148, HOUSE_TYPE_ALCHEMY_SHOP, 48, 0 },  // 65
    { "nmagshp", 0, 2145, HOUSE_TYPE_MAGIC_SHOP, 49, 0 },  // 66
    { "nsplshp", 0, 2150, HOUSE_TYPE_ELEMENTAL_GUILD, 50, 0 },  // 67
    { "ntrhall", 0, 1149, HOUSE_TYPE_TRAINING_GROUND, 51, 0 },  // 68
    { "nbank", 0, 2152, HOUSE_TYPE_BANK, 52, 0 },  // 69
    { "ntavern", 0, 2153, HOUSE_TYPE_TAVERN, 53, 0 },  // 70
    { "tsthrone", 0, 0, HOUSE_TYPE_THRONE_ROOM, 0, 0 },  // 71
    { "dlcavern", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 72
    { "felmcell", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 73
    { "eelmcell", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 74
    { "welmcell", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 75
    { "aelmcell", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 76
    { "trporhs1", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 77
    { "trporhs2", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 78
    { "trtavern", 0, 2158, HOUSE_TYPE_TAVERN, 68, 0 },  // 79
    { "wrboat", 0, 2180, HOUSE_TYPE_BOAT, 0, 0 },  // 80
    { "wrthrone", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 81
    { "rrchhs1", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 82
    { "rrchs2", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 83
    { "rmedhs", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 84
    { "rtemple", 0, 1228, HOUSE_TYPE_TEMPLE, 56, 0 },  // 85
    { "rwepshp", 0, 2177, HOUSE_TYPE_WEAPON_SHOP, 57, 0 },  // 86
    { "rarmshp", 0, 2173, HOUSE_TYPE_ARMOR_SHOP, 58, 0 },  // 87
    { "ralcshp", 0, 2172, HOUSE_TYPE_ALCHEMY_SHOP, 59, 0 },  // 88
    { "rmagshp", 0, 2174, HOUSE_TYPE_MAGIC_SHOP, 60, 0 },  // 89
    { "rsplshp", 0, 2175, HOUSE_TYPE_ELEMENTAL_GUILD, 61, 0 },  // 90
    { "rtavern", 0, 2176, HOUSE_TYPE_TAVERN, 64, 0 },  // 91
    { "rsub", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 92
    { "esthrone", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 93
    { "stables", 0, 1249, HOUSE_TYPE_STABLE, 22, 0 },  // 94
    { "tavern", 0, 2178, HOUSE_TYPE_TAVERN, 64, 0 },  // 95
    { "inn", 0, 2167, HOUSE_TYPE_ADVENTURERS_INN, 67, 0 },  // 96
    { "oracle", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 97
    { "felmhs", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 98
    { "welmhs", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 99
    { "eelmhs", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 100
    { "aelmhs", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 101
    { "ab_temp", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 102
    { "p_outpst", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 103
    { "smg_cove", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 104
    { "dire_lr", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 105
    { "mrch_hs", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 106
    { "in_cryst", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 107
    { "wasp_nst", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 108
    { "ogre_ft", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 109
    { "trol_tmb", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 110
    { "cyc_lr", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 111
    { "ch_fire", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 112
    { "dh_camp", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 113
    { "drgn_cav", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 114
    { "naga_vlt", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 115
    { "tpl_sun", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 116
    { "ab_druid", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 117
    { "mino_lr", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 118
    { "barb_frt", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 119
    { "p_strong", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 120
    { "a_p_keep", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 121
    { "s_subpen", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 122
    { "nec_gild", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 123
    { "mad_lab", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 124
    { "cstl_air", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 125
    { "rav_nest", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 126
    { "cstl_fir", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 127
    { "war_camp", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 128
    { "esch_pal", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 129
    { "pr_elords", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 130
    { "gw_fire", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 131
    { "gw_earth", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 132
    { "gw_air", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 133
    { "gw_water", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 134
    { "ab_tempBD", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 135
    { "pr_lorde", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 136
    { "pr_lordf", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 137
    { "pr_lordw", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 138
    { "lsplshp", 0, 2113, HOUSE_TYPE_ELEMENTAL_GUILD, 6, 0 },  // 139
    { "lsplshp", 0, 2113, HOUSE_TYPE_SELF_GUILD, 6, 0 },  // 140
    { "lsplshp", 0, 2113, HOUSE_TYPE_LIGHT_GUILD, 6, 0 },  // 141
    { "lsplshp", 0, 2113, HOUSE_TYPE_DARK_GUILD, 6, 0 },  // 142
    { "desplshp", 0, 2123, HOUSE_TYPE_ELEMENTAL_GUILD, 17, 0 },  // 143
    { "desplshp", 0, 2123, HOUSE_TYPE_SELF_GUILD, 17, 0 },  // 144
    { "desplshp", 0, 2123, HOUSE_TYPE_LIGHT_GUILD, 17, 0 },  // 145
    { "desplshp", 0, 2123, HOUSE_TYPE_DARK_GUILD, 17, 0 },  // 146
    { "msplshp", 0, 2168, HOUSE_TYPE_ELEMENTAL_GUILD, 28, 0 },  // 147
    { "msplshp", 0, 2168, HOUSE_TYPE_SELF_GUILD, 28, 0 },  // 148
    { "msplshp", 0, 2168, HOUSE_TYPE_LIGHT_GUILD, 28, 0 },  // 149
    { "msplshp", 0, 2168, HOUSE_TYPE_DARK_GUILD, 28, 0 },  // 150
    { "nsplshp", 0, 2150, HOUSE_TYPE_ELEMENTAL_GUILD, 50, 0 },  // 151
    { "nsplshp", 0, 2150, HOUSE_TYPE_SELF_GUILD, 50, 0 },  // 152
    { "nsplshp", 0, 2150, HOUSE_TYPE_LIGHT_GUILD, 50, 0 },  // 153
    { "nsplshp", 0, 2150, HOUSE_TYPE_DARK_GUILD, 50, 0 },  // 154
    { "rsplshp", 0, 2175, HOUSE_TYPE_ELEMENTAL_GUILD, 61, 0 },  // 155
    { "rsplshp", 0, 2175, HOUSE_TYPE_SELF_GUILD, 61, 0 },  // 156
    { "rsplshp", 0, 2175, HOUSE_TYPE_LIGHT_GUILD, 61, 0 },  // 157
    { "rsplshp", 0, 2175, HOUSE_TYPE_DARK_GUILD, 61, 0 },  // 158
    { "null", 0, 0, HOUSE_TYPE_DUNGEON, 0, 0 },  // 159
    { "cave", 0, 0, HOUSE_TYPE_INVALID, 0, 0 },  // 160
} };

const HouseAnimDescr &animatedRoom(int uAnimationID) {
    if (gameType() == GAME_TYPE_MM8)
        return pAnimatedRoomsMM8[uAnimationID];
    return pAnimatedRooms[uAnimationID];
}

const IndexedArray<int, HOUSE_TYPE_WEAPON_SHOP, HOUSE_TYPE_SELF_GUILD> itemAmountInShop = {{
    {HOUSE_TYPE_WEAPON_SHOP,   6},
    {HOUSE_TYPE_ARMOR_SHOP,    8},
    {HOUSE_TYPE_MAGIC_SHOP,   12},
    {HOUSE_TYPE_ALCHEMY_SHOP, 12},
    {HOUSE_TYPE_FIRE_GUILD,   12},
    {HOUSE_TYPE_AIR_GUILD,    12},
    {HOUSE_TYPE_WATER_GUILD,  12},
    {HOUSE_TYPE_EARTH_GUILD,  12},
    {HOUSE_TYPE_SPIRIT_GUILD, 12},
    {HOUSE_TYPE_MIND_GUILD,   12},
    {HOUSE_TYPE_BODY_GUILD,   12},
    {HOUSE_TYPE_LIGHT_GUILD,  12},
    {HOUSE_TYPE_DARK_GUILD,   12},
    {HOUSE_TYPE_ELEMENTAL_GUILD, 12}, // Combined guilds use the same 12-slot shelf as single-school guilds.
    {HOUSE_TYPE_SELF_GUILD,      12}
}};

static constexpr IndexedArray<const char *, HOUSE_TYPE_WEAPON_SHOP, HOUSE_TYPE_MIRRORED_PATH_GUILD> shopBackgroundNames = {{
    {HOUSE_TYPE_WEAPON_SHOP,           "WEPNTABL"},
    {HOUSE_TYPE_ARMOR_SHOP,            "ARMORY"},
    {HOUSE_TYPE_MAGIC_SHOP,            "MAGSHELF"},
    {HOUSE_TYPE_ALCHEMY_SHOP,          "MAGSHELF"},
    {HOUSE_TYPE_FIRE_GUILD,            "MAGSHELF"},
    {HOUSE_TYPE_AIR_GUILD,             "MAGSHELF"},
    {HOUSE_TYPE_WATER_GUILD,           "MAGSHELF"},
    {HOUSE_TYPE_EARTH_GUILD,           "MAGSHELF"},
    {HOUSE_TYPE_SPIRIT_GUILD,          "MAGSHELF"},
    {HOUSE_TYPE_MIND_GUILD,            "MAGSHELF"},
    {HOUSE_TYPE_BODY_GUILD,            "MAGSHELF"},
    {HOUSE_TYPE_LIGHT_GUILD,           "MAGSHELF"},
    {HOUSE_TYPE_DARK_GUILD,            "MAGSHELF"},
    {HOUSE_TYPE_ELEMENTAL_GUILD,       "MAGSHELF"},
    {HOUSE_TYPE_SELF_GUILD,            "MAGSHELF"},
    {HOUSE_TYPE_MIRRORED_PATH_GUILD,   "MAGSHELF"}
}};

bool enterHouse(HouseId uHouseID) {
    // A free-roam NPC dialogue (e.g. the daily-timer greeting, out01.evt event 500) may still
    // be open here, `EVENT_SpeakInHouse` calls this function on any screen. The house overwrites
    // `pDialogueWindow` and `game_ui_dialogue_background`, so close the dialogue first, as Escape does.
    if (current_screen_type == SCREEN_NPC_DIALOGUE) {
        DialogueEnding();
        current_screen_type = SCREEN_GAME;
    }

    // A house can be entered again while `window_SpeakInHouse` is still open (a double click on a
    // door, `EVENT_SpeakInHouse` firing again). Close the open house window first, or its
    // destructor releases the images the new window uses.
    if (window_SpeakInHouse) {
        window_SpeakInHouse = nullptr;
    }

    engine->_statusBar->clearAll();
    engine->_messageQueue->clear();

    if (uHouseID == HOUSE_THRONEROOM_WIN_GOOD || uHouseID == HOUSE_THRONEROOM_WIN_EVIL) {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_ShowGameOverWindow, 0, 0);
        return false;
    }

    current_npc_text.clear();

    int openHours = houseTable[uHouseID].uOpenTime;
    int closeHours = houseTable[uHouseID].uCloseTime;
    Time currentTime = pParty->GetPlayingTime();
    Time currentTimeDays = Time::fromDays(currentTime.toDays());
    bool isOpened = false;
    Time openTime = currentTimeDays + Duration::fromHours(openHours);
    Time closeTime = currentTimeDays + Duration::fromHours(closeHours);

    if (closeHours > openHours) {
        // Store opens during the day.
        isOpened = (currentTime >= openTime) && (currentTime <= closeTime);
    } else {
        // Store opens at night.
        isOpened = (currentTime <= closeTime) || (currentTime >= openTime);
    }

    if (!isOpened) {
        CivilTime openCivilTime = openTime.toCivilTime();
        CivilTime closeCivilTime = closeTime.toCivilTime();

        engine->_statusBar->setEvent(LSTR_THIS_PLACE_IS_OPEN_FROM_DS_TO_DS,
                                     openCivilTime.hourAmPm,
                                     localization->amPm(openCivilTime.isPm),
                                     closeCivilTime.hourAmPm,
                                     localization->amPm(closeCivilTime.isPm));
        if (pParty->hasActiveCharacter())
            pParty->activeCharacter().playReaction(SPEECH_STORE_CLOSED);

        return false;
    }

    if (isShop(uHouseID)) {
        if (!(pParty->PartyTimes.shopBanTimes[uHouseID]) || (pParty->PartyTimes.shopBanTimes[uHouseID] <= pParty->GetPlayingTime())) {
            pParty->PartyTimes.shopBanTimes[uHouseID] = Time();
        } else {
            engine->_statusBar->setEvent(LSTR_YOUVE_BEEN_BANNED_FROM_THIS_SHOP);
            return false;
        }
    }

    uCurrentHouse_Animation = houseTable[uHouseID].uAnimationID;
    // The jail is Might and Magic VII's (a throne room entered with a fine = a year in prison). MM8 has no jail, and its
    // house 187 is another house, the Temple of the Sun Leader Room.
    if (gameType() != GAME_TYPE_MM8 && animatedRoom(uCurrentHouse_Animation).uBuildingType == HOUSE_TYPE_THRONE_ROOM && pParty->uFine) {  // going to jail
        uHouseID = HOUSE_JAIL;
        uCurrentHouse_Animation = houseTable[uHouseID].uAnimationID;
        restAndHeal(Duration::fromYears(1));
        ++pParty->uNumPrisonTerms;
        pParty->uFine = 0;
        for (Character &player : pParty->pCharacters) {
            player.timeToRecovery = 0_ticks;
            player.uNumDivineInterventionCastsThisDay = 0;
            player.giveAward(AWARD_PRISON_TERMS);
        }
    }

    currentHouseNpc = -1;

    prepareHouse(uHouseID);

    if (houseNpcs.size() == 1)
        currentHouseNpc = 0;
    pMediaPlayer->OpenHouseMovie(animatedRoom(uCurrentHouse_Animation).video_name, 1u);
    if (isMagicGuild(uHouseID)) {
        // TODO(pskelton): check this behaviour
        if (!pParty->hasActiveCharacter()) // avoid nzi
            pParty->setActiveToFirstCanAct();

        if (gameType() != GAME_TYPE_MM8 && !pParty->activeCharacter()._achievedAwardsBits[membershipAwardForGuild(uHouseID)]) {
            playHouseSound(uHouseID, HOUSE_SOUND_MAGIC_GUILD_MEMBERS_ONLY);
            return true;
        }
    } else if ((isStable(uHouseID) || isBoat(uHouseID)) && !isTravelAvailable(uHouseID)) {
        return true;
    }
    playHouseSound(uHouseID, HOUSE_SOUND_GENERAL_GREETING);
    return true;
}

// An MM8 companion who has joined (in the party, or benched at the Adventurer's Inn) has left home. The companion is the
// house NPC whose own join slot (601-649, roster row id + 600) names his roster row.
static bool companionHasLeftHome(const NPCData &npc) {
    if (gameType() != GAME_TYPE_MM8)
        return false;
    for (unsigned int eventId : {npc.dialogue_1_evt_id, npc.dialogue_2_evt_id, npc.dialogue_3_evt_id,
                                 npc.dialogue_4_evt_id, npc.dialogue_5_evt_id, npc.dialogue_6_evt_id}) {
        if (!isRosterJoinTopic(eventId))
            continue;
        const int rosterId = static_cast<int>(eventId) - 600;
        if (static_cast<size_t>(rosterId) >= rosterMm8.size() || rosterMm8[rosterId].name != npc.name)
            continue;
        if (pParty->isRosterInParty(rosterId) || pParty->mm8Roster.isInInn(rosterId))
            return true;
    }
    return false;
}

void prepareHouse(HouseId house) {
    houseNpcs.clear();

    // Default proprietor of non-simple houses
    int proprietorId = animatedRoom(houseTable[house].uAnimationID).house_npc_id;
    if (proprietorId) {
        HouseNpcDesc desc;
        desc.type = HOUSE_PROPRIETOR;
        desc.label = localization->format(LSTR_CONVERSE_WITH_S, houseTable[house].pProprieterName);
        desc.icon = assets->getImage_ColorKey(npcPortraitTextureName(proprietorId));

        houseNpcs.push_back(desc);
    }

    // NPCs of this house
    for (int i = 1; i < pNPCStats->uNumNewNPCs; ++i) {
        if (pNPCStats->pNPCData[i].house == house) {
            if (!(pNPCStats->pNPCData[i].flags & NPC_HIRED) && !companionHasLeftHome(pNPCStats->pNPCData[i])) {
                HouseNpcDesc desc;
                desc.type = HOUSE_NPC;
                desc.label = localization->format(LSTR_CONVERSE_WITH_S, pNPCStats->pNPCData[i].name);
                desc.icon = assets->getImage_ColorKey(npcPortraitTextureName(pNPCStats->pNPCData[i].portraitId));
                desc.npc = &pNPCStats->pNPCData[i];

                houseNpcs.push_back(desc);
                if (!(pNPCStats->pNPCData[i].flags & NPC_GREETED_SECOND)) {
                    if (pNPCStats->pNPCData[i].flags & NPC_GREETED_FIRST) {
                        pNPCStats->pNPCData[i].flags &= ~NPC_GREETED_FIRST;
                        pNPCStats->pNPCData[i].flags |= NPC_GREETED_SECOND;
                    } else {
                        pNPCStats->pNPCData[i].flags |= NPC_GREETED_FIRST;
                    }
                }
            }
        }
    }

    // Dungeon entry (not present in MM7)
    if (houseTable[house].uExitPicID) {
        if (houseTable[house]._quest_bit == QBIT_INVALID || !pParty->_questBits[houseTable[house]._quest_bit]) {
            MapId id = houseTable[house].uExitMapID;

            HouseNpcDesc desc;
            desc.type = HOUSE_TRANSITION;
            desc.label = localization->format(LSTR_ENTER_S, pMapTable->pInfos[id].name);
            desc.icon = assets->getIcon(pHouse_ExitPictures[static_cast<int>(id)]);
            desc.targetMapID = id;

            houseNpcs.push_back(desc);
        }
    }
}

// MM8's "Close" button ("c_close_up") is 83x30 at (516,431), not MM7's 169x35 at (471,445).
// GUIWindow_House recreates pBtn_ExitCancel at several points as its dialogue state
// changes (ctor, reinitDialogueWindow, NPCHireableDialogPrepare). CreateButton's dimensions are
// closed-interval (stores w/h + 1), so the size returned is one less than the asset's size.
// Shared with the free-roam GUIWindow_Dialogue (UIDialogue.cpp).
std::pair<Pointi, Sizei> exitCancelButtonGeometry() {
    if (gameType() == GAME_TYPE_MM8) {
        return {{516, 431}, {82, 29}};
    }
    return {{471, 445}, {169, 35}};
}

/**
 * TODO(Nik-RE-dev): untested until houses NPC can join the party
 *
 * @offset 0x4B40E6
 */
void NPCHireableDialogPrepare() {
    int v0 = 0;
    NPCData *v1 = houseNpcs[currentHouseNpc].npc;

    pDialogueWindow = std::make_unique<GUIWindow>(WINDOW_DIALOGUE, Pointi(0, 0), Sizei(render->GetRenderDimensions().w, 350));
    auto [exitCancelPos, exitCancelSize] = exitCancelButtonGeometry();
    pBtn_ExitCancel = pDialogueWindow->CreateButton(exitCancelPos, exitCancelSize, BUTTON_TYPE_NORMAL, 0,
        UIMSG_Escape, 0, INPUT_ACTION_INVALID, localization->str(LSTR_CANCEL), {ui_exit_cancel_button_background}
    );
    pDialogueWindow->CreateButton({0, 0}, {0, 0}, BUTTON_TYPE_NORMAL, 0, UIMSG_HouseScreenClick, 0);
    if (!pNPCStats->pProfessions[v1->profession].pBenefits.empty()) {
        pDialogueWindow->CreateButton("HouseNpcDialogue_ProfessionDetails", {480, 160}, {140, 30}, BUTTON_TYPE_NORMAL, 0,
            UIMSG_SelectHouseNPCDialogueOption, std::to_underlying(DIALOGUE_PROFESSION_DETAILS), INPUT_ACTION_INVALID, localization->str(LSTR_MORE_INFORMATION)
        );
        v0 = 1;
    }
    pDialogueWindow->CreateButton("HouseNpcDialogue_HireFire", {480, 30 * v0 + 160}, {140, 30}, BUTTON_TYPE_NORMAL, 0,
        UIMSG_SelectHouseNPCDialogueOption, std::to_underlying(DIALOGUE_HIRE_FIRE), INPUT_ACTION_INVALID, localization->str(LSTR_HIRE));
    pDialogueWindow->setKeyboardControlGroup(v0 + 1, false, 0, 2);
    window_SpeakInHouse->setCurrentDialogue(DIALOGUE_OTHER);
}

void selectHouseNPCDialogueOption(DialogueId topic) {
    NPCData *pCurrentNPCInfo = houseNpcs[currentHouseNpc].npc;

    if (topic >= DIALOGUE_SCRIPTED_LINE_1 && topic <= DIALOGUE_SCRIPTED_LINE_6) {
        DialogueId newTopic = handleScriptedNPCTopicSelection(topic, pCurrentNPCInfo);

        if (newTopic != DIALOGUE_MAIN) {
            window_SpeakInHouse->setCurrentDialogue(DIALOGUE_OTHER);
            window_SpeakInHouse->reinitDialogueWindow();
            window_SpeakInHouse->initializeNPCDialogueButtons(listNPCDialogueOptions(newTopic));
        }
        BackToHouseMenu();
        return;
    }

    if (topic == DIALOGUE_13_hiring_related) {
        current_npc_text = BuildDialogueString(pNPCStats->pProfessions[pCurrentNPCInfo->profession].pJoinText,
                                               pParty->activeCharacterIndex(), pCurrentNPCInfo);
        NPCHireableDialogPrepare();
        dialogue_show_profession_details = false;
        BackToHouseMenu();
        return;
    }

    selectSpecialNPCTopicSelection(topic, pCurrentNPCInfo);

    if (topic != DIALOGUE_HIRE_FIRE) {
        if (topic == DIALOGUE_PROFESSION_DETAILS) {
            if (dialogue_show_profession_details) {
                current_npc_text = BuildDialogueString(pNPCStats->pProfessions[pCurrentNPCInfo->profession].pBenefits,
                                                       pParty->activeCharacterIndex(), pCurrentNPCInfo);
            } else {
                current_npc_text = BuildDialogueString(pNPCStats->pProfessions[pCurrentNPCInfo->profession].pJoinText,
                                                       pParty->activeCharacterIndex(), pCurrentNPCInfo);
            }
        }
        BackToHouseMenu();
        return;
    }

    if (!pCurrentNPCInfo->Hired()) {
        current_npc_text = BuildDialogueString(pNPCStats->pProfessions[pCurrentNPCInfo->profession].pJoinText,
                                               pParty->activeCharacterIndex(), pCurrentNPCInfo);
        BackToHouseMenu();
        return;
    }

    prepareHouse(window_SpeakInHouse->houseId());
    BackToHouseMenu();
}

void updateHouseNPCTopics(int npc) {
    int num_menu_buttons = 0;

    currentHouseNpc = npc;
    if (houseNpcs[npc].type == HOUSE_TRANSITION) {
        // TODO(Nik-RE-dev): can use GUIWindow_Transition
        pDialogueWindow = std::make_unique<GUIWindow>(WINDOW_DIALOGUE, Pointi(0, 0), render->GetRenderDimensions());
        pBtn_ExitCancel = pDialogueWindow->CreateButton({566, 445}, {75, 33}, BUTTON_TYPE_NORMAL, 0, UIMSG_Escape, 0, INPUT_ACTION_TRANSITION_NO, localization->str(LSTR_CANCEL), {ui_buttdesc2});
        pBtn_YES = pDialogueWindow->CreateButton({486, 445}, {75, 33}, BUTTON_TYPE_NORMAL, 0, UIMSG_HouseTransitionConfirmation, 1, INPUT_ACTION_TRANSITION_YES, houseNpcs[npc].label, {ui_buttyes2});
        pDialogueWindow->CreateButton({pNPCPortraits_x[0][0], pNPCPortraits_y[0][0]}, {63, 73}, BUTTON_TYPE_NORMAL, 0, UIMSG_HouseTransitionConfirmation, 1,
                                      INPUT_ACTION_INTERACT, houseNpcs[npc].label);
        pDialogueWindow->CreateButton({8, 8}, {460, 344}, BUTTON_TYPE_NORMAL, 0, UIMSG_HouseTransitionConfirmation, 1, INPUT_ACTION_TRANSITION_YES, houseNpcs[npc].label);
    } else {
        if (window_SpeakInHouse->currentDialogue() != DIALOGUE_OTHER) {
            for (int i = 0; i < houseNpcs.size(); ++i) {
                houseNpcs[i].button->Release();
                houseNpcs[i].button = nullptr;
            }
        }
        window_SpeakInHouse->setCurrentDialogue(DIALOGUE_MAIN);
        window_SpeakInHouse->reinitDialogueWindow();
        if (houseNpcs[npc].type == HOUSE_PROPRIETOR) {
            window_SpeakInHouse->initializeProprietorDialogue();
        } else {
            window_SpeakInHouse->initializeNPCDialogue(npc);
        }
    }
}

void selectProprietorDialogueOption(DialogueId option) {
    if (!pDialogueWindow || !pDialogueWindow->pNumPresenceButton)
        return;

    pParty->placeHeldItemInInventoryOrDrop();

    window_SpeakInHouse->houseDialogueOptionSelected(option);
    window_SpeakInHouse->reinitDialogueWindow();
    window_SpeakInHouse->initializeProprietorDialogue();
}

bool houseDialogueGoBack() {
    engine->_messageQueue->clear();
    current_npc_text.clear();
    pParty->placeHeldItemInInventoryOrDrop();

    if (currentHouseNpc == -1)
        return false;

    if (window_SpeakInHouse->currentDialogue() == DIALOGUE_OTHER) {
        updateHouseNPCTopics(currentHouseNpc);
        BackToHouseMenu();
        return true;
    }

    if (window_SpeakInHouse->currentDialogue() == DIALOGUE_NULL ||
        window_SpeakInHouse->currentDialogue() == DIALOGUE_MAIN) {
        currentHouseNpc = -1;
        window_SpeakInHouse->updateDialogueOnEscape();
        // we are in the middle of the update windows loop, so we need to delay closing the dialogue window until the end of the frame
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_CloseDialogueWindow, 0, 0);

        if (houseNpcs.size() == 1)
            return false;

        pBtn_ExitCancel = window_SpeakInHouse->vButtons.front();
        for (int i = 0; i < houseNpcs.size(); ++i) {
            Pointi pos = {pNPCPortraits_x[houseNpcs.size() - 1][i], pNPCPortraits_y[houseNpcs.size() - 1][i]};
            houseNpcs[i].button = window_SpeakInHouse->CreateButton(fmt::format("House_Npc{}", i), pos, {63, 73}, BUTTON_TYPE_NORMAL, 0,
                                                                    UIMSG_ClickHouseNPCPortrait, i, INPUT_ACTION_INVALID, houseNpcs[i].label);
        }

        BackToHouseMenu();
        return true;
    }

    window_SpeakInHouse->updateDialogueOnEscape();
    window_SpeakInHouse->reinitDialogueWindow();
    window_SpeakInHouse->initializeProprietorDialogue();

    return true;
}

void houseDialogueGoBackToTop() {
    while (houseDialogueGoBack()) {}
}

void createHouseUI(HouseId houseId) {
    // The original's Adventurer's Inn is a screen of its own (id 29), not
    // reached through GUIWindow_Tavern's dialogue options even though the house is data-mapped to
    // HOUSE_TYPE_TAVERN.
    if (houseTable[houseId].isGenericAdventurersInn) {
        window_SpeakInHouse = std::make_unique<GUIWindow_AdventurersInn>(houseId);
    } else {
    switch (houseTable[houseId].uType) {
      case HOUSE_TYPE_FIRE_GUILD:
      case HOUSE_TYPE_AIR_GUILD:
      case HOUSE_TYPE_WATER_GUILD:
      case HOUSE_TYPE_EARTH_GUILD:
      case HOUSE_TYPE_SPIRIT_GUILD:
      case HOUSE_TYPE_MIND_GUILD:
      case HOUSE_TYPE_BODY_GUILD:
      case HOUSE_TYPE_LIGHT_GUILD:
      case HOUSE_TYPE_DARK_GUILD:
      case HOUSE_TYPE_ELEMENTAL_GUILD:
      case HOUSE_TYPE_SELF_GUILD:
      case HOUSE_TYPE_MIRRORED_PATH_GUILD:
        window_SpeakInHouse = std::make_unique<GUIWindow_MagicGuild>(houseId);
        break;
      case HOUSE_TYPE_BANK:
        window_SpeakInHouse = std::make_unique<GUIWindow_Bank>(houseId);
        break;
      case HOUSE_TYPE_TEMPLE:
        window_SpeakInHouse = std::make_unique<GUIWindow_Temple>(houseId);
        break;
      case HOUSE_TYPE_TAVERN:
        window_SpeakInHouse = std::make_unique<GUIWindow_Tavern>(houseId);
        break;
      case HOUSE_TYPE_TRAINING_GROUND:
        window_SpeakInHouse = std::make_unique<GUIWindow_Training>(houseId);
        break;
      case HOUSE_TYPE_STABLE:
      case HOUSE_TYPE_BOAT:
        window_SpeakInHouse = std::make_unique<GUIWindow_Transport>(houseId);
        break;
      case HOUSE_TYPE_TOWN_HALL:
        window_SpeakInHouse = std::make_unique<GUIWindow_TownHall>(houseId);
        break;
      case HOUSE_TYPE_JAIL:
        window_SpeakInHouse = std::make_unique<GUIWindow_Jail>(houseId);
        break;
      case HOUSE_TYPE_MERCENARY_GUILD:
        window_SpeakInHouse = std::make_unique<GUIWindow_MercenaryGuild>(houseId);
        break;
      case HOUSE_TYPE_WEAPON_SHOP:
        window_SpeakInHouse = std::make_unique<GUIWindow_WeaponShop>(houseId);
        break;
      case HOUSE_TYPE_ARMOR_SHOP:
        window_SpeakInHouse = std::make_unique<GUIWindow_ArmorShop>(houseId);
        break;
      case HOUSE_TYPE_MAGIC_SHOP:
        window_SpeakInHouse = std::make_unique<GUIWindow_MagicShop>(houseId);
        break;
      case HOUSE_TYPE_ALCHEMY_SHOP:
        window_SpeakInHouse = std::make_unique<GUIWindow_AlchemyShop>(houseId);
        break;
      default:
        window_SpeakInHouse = std::make_unique<GUIWindow_House>(houseId);
        break;
    }
    }

    if (houseNpcs.size() == 1)
        updateHouseNPCTopics(0);

    // The world stops redrawing on SCREEN_HOUSE, so the last exploration frame, with its HUD
    // chrome, would stay frozen around the panel. Draw two clean frames, one per buffer of the
    // double-buffered renderer.
    engine->setWorldDrawForced(true);
    engine->Draw();
    engine->Draw();
    engine->setWorldDrawForced(false);
}

void BackToHouseMenu() {
    // Dead since #840, the crash this block caused in MM7 - id 165 is MM6's High Council but MM7's Master
    // Guild of Body in Erathia. It rebuilt house 165's window on a return to the menu with no movie playing.
    // Four things in it no longer compile, so MM6 will need more than flipping the #if.
#if 0
    if (window_SpeakInHouse && window_SpeakInHouse->houseId() == 165 &&
        !pMovie_Track) {
        GameOverNoSound = true;
        houseDialogueGoBack();
        window_SpeakInHouse->Release();
        pParty->uFlags &= 0xFFFFFFFD;
        if (enterHouse(HOUSE_BODY_GUILD_MASTER_ERATHIA)) {
            pAudioPlayer->playUISound(SOUND_Invalid);
            createHouseUI(HOUSE_BODY_GUILD_MASTER_ERATHIA);
        }
        GameOverNoSound = false;
    }
#endif
}

static SoundId houseSoundId(int roomSoundId, HouseSoundType type) {
    return SoundId(30000 + 100 * roomSoundId + std::to_underlying(type));
}

void playHouseSound(HouseId houseID, HouseSoundType type) {
    if (houseID != HOUSE_INVALID && animatedRoom(houseTable[houseID].uAnimationID).uRoomSoundId) {
        // TODO(captainurist): encapsulate
        int roomSoundId = animatedRoom(houseTable[houseID].uAnimationID).uRoomSoundId;
        SoundId soundId = houseSoundId(roomSoundId, type);
        // This `30000 + 100 * room + type` scheme is MM8's own - MM8's
        // `dsounds.bin` holds that whole 30101..37004 block under the expected `eventNN_01a..d`
        // names - but MM8 did not record four variants for every room (room 67 has only `a`,
        // room 46 only `a`/`b`/`c`). A variant that was never recorded is silent in the original
        // too, so ask the table. MM7's table fills every variant this scheme names.
        if (gameType() == GAME_TYPE_MM8 && !pSoundList->soundInfo(soundId))
            return;
        pAudioPlayer->playHouseSound(soundId, true);
    }
}

int64_t houseSoundLengthMs(HouseId houseId, HouseSoundType type) {
    int roomSoundId = animatedRoom(houseTable[houseId].uAnimationID).uRoomSoundId;
    if (!roomSoundId)
        return 0;
    SoundId soundId = houseSoundId(roomSoundId, type);
    if (gameType() == GAME_TYPE_MM8 && !pSoundList->soundInfo(soundId))
        return 0; // MM8 did not record every variant for every room, see playHouseSound.
    return static_cast<int64_t>(std::ceil(pAudioPlayer->getSoundLength(soundId) * 1000));
}

void GUIWindow_House::houseNPCDialogue() {
    if (houseNpcs[currentHouseNpc].type == HOUSE_TRANSITION) {
        Recti house_window = this->frameRect;
        MapId id = houseNpcs[currentHouseNpc].targetMapID;
        house_window.x = 493;
        house_window.w = 126;
        DrawTitleText(assets->pFontCreate.get(), 0, 2, colorTable.White, pMapTable->pInfos[id].name, 3, house_window);
        house_window.x = SIDE_TEXT_BOX_POS_X;
        house_window.w = SIDE_TEXT_BOX_WIDTH;
        if (pTransitionStrings[std::to_underlying(id)].empty()) { // TODO(captainurist): this is a weird access into pTransitionStrings, investigate & add docs
            auto str = localization->format(LSTR_ENTER_S, pMapTable->pInfos[id].name);
            DrawTitleText(assets->pFontCreate.get(), 0, (212 - assets->pFontCreate->CalcTextHeight(str, house_window.w, 0)) / 2 + 101, colorTable.White, str, 3, house_window);
            return;
        }

        int vertMargin = (212 - assets->pFontCreate->CalcTextHeight(pTransitionStrings[std::to_underlying(id)], house_window.w, 0)) / 2 + 101;
        DrawTitleText(assets->pFontCreate.get(), 0, vertMargin, colorTable.White, pTransitionStrings[std::to_underlying(id)], 3, house_window);
        return;
    }

    NPCData *pNPC = houseNpcs[currentHouseNpc].npc;
    drawNpcHouseNameAndTitle(pNPC);
    drawNpcHouseGreetingMessage(pNPC);
    drawNpcHouseDialogueOptions(pNPC);
    drawNpcHouseDialogueResponse();
}

void GUIWindow_House::drawNpcHouseNameAndTitle(NPCData *npcData) {
    Recti window = this->frameRect;
    window.w -= 10;
    DrawTitleText(assets->pFontCreate.get(), SIDE_TEXT_BOX_POS_X, SIDE_TEXT_BOX_POS_Y, colorTable.EasternBlue, NameAndTitle(npcData), 3, window);
}

void GUIWindow_House::drawNpcHouseGreetingMessage(NPCData *npcData) {
    if (houseNpcs[0].type != HOUSE_PROPRIETOR) {
        if (current_npc_text.length() == 0 && _currentDialogue == DIALOGUE_MAIN) {
            if (npcData->greetingIndex) {
                std::string greetString;
                if (npcData->flags & NPC_GREETED_SECOND) {
                    greetString = pNPCStats->pNPCGreetings[npcData->greetingIndex].pGreeting2;
                } else {
                    greetString = pNPCStats->pNPCGreetings[npcData->greetingIndex].pGreeting1;
                }
                DrawDialoguePanel(greetString);
            }
        }
    }
}

void GUIWindow_House::drawNpcHouseDialogueOptions(NPCData* npcData) const {
    std::vector<std::string> optionsText;

    int buttonLimit = pDialogueWindow->pStartingPosActiveItem + pDialogueWindow->pNumPresenceButton;
    for (int i = pDialogueWindow->pStartingPosActiveItem; i < buttonLimit; ++i) {
        GUIButton *pButton = pDialogueWindow->GetControl(i);
        if (pButton) {
            DialogueId topic = (DialogueId)pButton->msg_param;
            std::string str = npcDialogueOptionString(topic, npcData);
            if (str.empty() && topic >= DIALOGUE_SCRIPTED_LINE_1 && topic <= DIALOGUE_SCRIPTED_LINE_6)
                pButton->msg_param = 0;
            optionsText.push_back(str);
        }
    }

    if (optionsText.size())
        drawOptions(optionsText, colorTable.Sunflower);
}

void GUIWindow_House::drawNpcHouseDialogueResponse() {
    DrawDialoguePanel(current_npc_text);
}

void GUIWindow_House::reinitDialogueWindow() {
    if (pDialogueWindow) {
        // reset dialogue window to default state, so it can be reused for different NPCs dialogues without creating new one
        pDialogueWindow->frameRect = { 0, 0, render->GetPresentDimensions().w, 345 };
        pDialogueWindow->sHint = "";
        pDialogueWindow->receives_keyboard_input = false;
        pDialogueWindow->DeleteButtons();
    } else {
        pDialogueWindow = std::make_unique<GUIWindow>(WINDOW_DIALOGUE, Pointi(0, 0), Sizei(render->GetPresentDimensions().w, 345));
    }

    auto [exitCancelPos, exitCancelSize] = exitCancelButtonGeometry();
    pBtn_ExitCancel = pDialogueWindow->CreateButton(exitCancelPos, exitCancelSize, BUTTON_TYPE_NORMAL, 0, UIMSG_Escape, 0, INPUT_ACTION_INVALID,
        localization->str(LSTR_END_CONVERSATION), {ui_exit_cancel_button_background});
    pDialogueWindow->CreateButton({8, 8}, {450, 320}, BUTTON_TYPE_NORMAL, 0, UIMSG_HouseScreenClick, 0, INPUT_ACTION_INVALID, "");
}

bool GUIWindow_House::checkIfPlayerCanInteract() {
    if (!pParty->hasActiveCharacter())  // to avoid access zeroeleement
        return false;

    // Do nothing if no current conversation exist
    if (!pDialogueWindow)
        return true;

    if (pParty->activeCharacter().CanAct()) {
        pDialogueWindow->pNumPresenceButton = _savedButtonsNum;
        return true;
    } else {
        pDialogueWindow->pNumPresenceButton = 0;
        Recti window = window_SpeakInHouse->frameRect;
        window.x = SIDE_TEXT_BOX_POS_X;
        window.w = SIDE_TEXT_BOX_WIDTH;

        std::string str = localization->format(LSTR_S_IS_IN_NO_CONDITION_TO_S, pParty->activeCharacter().name, localization->str(LSTR_DO_ANYTHING));
        DrawTitleText(assets->pFontArrus.get(), 0, (212 - assets->pFontArrus->CalcTextHeight(str, window.w, 0)) / 2 + 101, ui_house_player_cant_interact_color, str, 3, window);
        return false;
    }
}

// TODO(Nik-RE-dev): maybe need to unify selectColor for all dialogue
void GUIWindow_House::drawOptions(std::vector<std::string> &optionsText, Color selectColor, int topOptionShift, bool denseSpacing) const {
    Recti window = this->frameRect;
    window.x = SIDE_TEXT_BOX_POS_X;
    window.w = SIDE_TEXT_BOX_WIDTH;

    assert(optionsText.size() == pDialogueWindow->pNumPresenceButton);

    int allTextHeight = 0;
    int activeOptions = 0;
    for (int i = 0; i < optionsText.size(); ++i) {
        if (!optionsText[i].empty()) {
            allTextHeight += assets->pFontArrus->CalcTextHeight(optionsText[i], window.w, 0);
            activeOptions++;
        }
    }

    int spacing = 0;
    int offset = topOptionShift;
    if (!denseSpacing) {
        spacing = (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - topOptionShift - allTextHeight) / (activeOptions ? activeOptions : 1);
        if (spacing > SIDE_TEXT_BOX_MAX_SPACING)
            spacing = SIDE_TEXT_BOX_MAX_SPACING;
        offset += (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - topOptionShift - spacing * activeOptions - allTextHeight) / 2 - spacing / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET;
    }

    for (int i = 0; i < pDialogueWindow->pNumPresenceButton; ++i) {
        int buttonIndex = i + pDialogueWindow->pStartingPosActiveItem;
        GUIButton *button = pDialogueWindow->GetControl(buttonIndex);

        if (!optionsText[i].empty()) {
            Color textColor = (pDialogueWindow->pCurrentPosActiveItem == buttonIndex) ? selectColor : colorTable.White;
            int textHeight = assets->pFontArrus->CalcTextHeight(optionsText[i], window.w, 0);
            button->rect.y = spacing + offset;
            button->rect.h = textHeight + 6;
            button->label = optionsText[i];
            if (denseSpacing) {
                offset += assets->pFontArrus->GetHeight() - 3 + textHeight;
            } else {
                offset = button->rect.y + textHeight - 1 + 6;
            }
            DrawTitleText(assets->pFontArrus.get(), 0, button->rect.y, textColor, optionsText[i], 3, window);
        } else if (button) {
            button->rect.y = 0;
            button->rect.h = 0;
            button->label.clear();
        }
    }
}

void GUIWindow_House::houseDialogManager() {
    assert(window_SpeakInHouse != nullptr);

    Recti pWindow = this->frameRect;
    pWindow.w -= 18;
    render->DrawQuad2D(_dialogueBackground, {477, 0});
    render->DrawQuad2D(game_ui_right_panel_frame, {468, 0});

    if (currentHouseNpc == -1 || houseNpcs[currentHouseNpc].type != HOUSE_TRANSITION) {
        // Draw house title
        if (!houseTable[houseId()].name.empty()) {
            if (current_screen_type != SCREEN_SHOP_INVENTORY) {
                int yPos = 2 * assets->pFontCreate->GetHeight() - 6 - assets->pFontCreate->CalcTextHeight(houseTable[houseId()].name, 130, 0);
                if (yPos < 0)
                    yPos = 0;
                DrawTitleText(assets->pFontCreate.get(), 0x1EAu, yPos / 2 + 4, colorTable.White, houseTable[houseId()].name, 3, pWindow);
            }
        }
    }

    pWindow.w += 8;
    if (currentHouseNpc == -1) {
        // Either house have no residents or current screen is for selecting resident to begin dialogue
        render->DrawQuad2D(ui_exit_cancel_button_background, pBtn_ExitCancel->rect.topLeft());

        if (buildingType() == HOUSE_TYPE_JAIL) {
            houseSpecificDialogue();
            return;
        }
        DrawDialoguePanel(current_npc_text);

        for (int i = 0; i < houseNpcs.size(); ++i) {
            int portraitX = pNPCPortraits_x[houseNpcs.size() - 1][i];
            int portraitY = pNPCPortraits_y[houseNpcs.size() - 1][i];
            render->DrawQuad2D(game_ui_evtnpc, {portraitX - 4, portraitY - 4});
            render->DrawQuad2D(houseNpcs[i].icon, {portraitX, portraitY});
            if (houseNpcs.size() < 4) {
                std::string pTitleText = "";
                int yPos = 0;
                switch (houseNpcs[i].type) {
                  case HOUSE_TRANSITION:
                    pTitleText = pMapTable->pInfos[houseNpcs[i].targetMapID].name;
                    yPos = 94 * i + SIDE_TEXT_BOX_POS_Y;
                    break;
                  case HOUSE_PROPRIETOR:
                    pTitleText = houseTable[houseId()].pProprieterTitle;
                    yPos = SIDE_TEXT_BOX_POS_Y;
                    break;
                  case HOUSE_NPC:
                    pTitleText = houseNpcs[i].npc->name;
                    yPos = pNPCPortraits_y[houseNpcs.size() - 1][i] + houseNpcs[i].icon->height() + 2;
                    break;
                }
                DrawTitleText(assets->pFontCreate.get(), SIDE_TEXT_BOX_POS_X, yPos, colorTable.EasternBlue, pTitleText, 3, pWindow);
            }
        }
        return;
    }

    render->DrawQuad2D(game_ui_evtnpc, {pNPCPortraits_x[0][0] - 4, pNPCPortraits_y[0][0] - 4});
    render->DrawQuad2D(houseNpcs[currentHouseNpc].icon, {pNPCPortraits_x[0][0], pNPCPortraits_y[0][0]});
    if (current_screen_type == SCREEN_SHOP_INVENTORY) {
        CharacterUI_InventoryTab_Draw(&pParty->activeCharacter(), true);
        render->DrawQuad2D(ui_exit_cancel_button_background, pBtn_ExitCancel->rect.topLeft());
        return;
    }
    if (currentHouseNpc || houseNpcs[0].type != HOUSE_PROPRIETOR) {
        // Dialogue with NPC in house
        houseNPCDialogue();
    } else {
        std::string nameAndTitle = NameAndTitle(houseTable[houseId()].pProprieterName, houseTable[houseId()].pProprieterTitle);
        DrawTitleText(assets->pFontCreate.get(), SIDE_TEXT_BOX_POS_X, SIDE_TEXT_BOX_POS_Y, colorTable.EasternBlue, nameAndTitle, 3, pWindow);
        houseSpecificDialogue();
    }
    if (currentHouseNpc != -1 && houseNpcs[currentHouseNpc].type == HOUSE_TRANSITION) {
        render->DrawQuad2D(dialogue_ui_x_x_u, {556, 451});
        render->DrawQuad2D(dialogue_ui_x_ok_u, {476, 451});
    } else {
        render->DrawQuad2D(ui_exit_cancel_button_background, pBtn_ExitCancel->rect.topLeft());
    }
}

void GUIWindow_House::initializeProprietorDialogue() {
    if (!pDialogueWindow)
        return;

    std::vector<DialogueId> optionList = listDialogueOptions();

    for (int i = 0; i < optionList.size(); i++)
        pDialogueWindow->CreateButton(fmt::format("HouseDialogue_Option{}", i), {480, 146 + 30 * i}, {140, 30}, BUTTON_TYPE_NORMAL, 0,
                                      UIMSG_SelectProprietorDialogueOption, std::to_underlying(optionList[i]), INPUT_ACTION_INVALID, "");
    pDialogueWindow->setKeyboardControlGroup(optionList.size(), false, 0, 2);
    _savedButtonsNum = pDialogueWindow->pNumPresenceButton;
}

void GUIWindow_House::initializeNPCDialogue(int npc) {
    if (!pDialogueWindow)
        return;

    initializeNPCDialogueButtons(prepareScriptedNPCDialogueTopics(houseNpcs[npc].npc));
}

void GUIWindow_House::initializeNPCDialogueButtons(std::vector<DialogueId> optionList) {
    for (int i = 0; i < optionList.size(); i++)
        pDialogueWindow->CreateButton(fmt::format("HouseNpcDialogue_Option{}", i), {480, 160 + 30 * i}, {140, 30}, BUTTON_TYPE_NORMAL, 0,
                                      UIMSG_SelectHouseNPCDialogueOption, std::to_underlying(optionList[i]), INPUT_ACTION_INVALID, "");
    pDialogueWindow->setKeyboardControlGroup(optionList.size(), false, 0, 2);
    _savedButtonsNum = pDialogueWindow->pNumPresenceButton;
}

void GUIWindow_House::learnSkillsDialogue(Color selectColor) {
    if (!checkIfPlayerCanInteract())
        return;

    bool haveLearnableSkills = false;
    std::vector<std::string> optionsText;
    int cost = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()]);
    int buttonsLimit = pDialogueWindow->pStartingPosActiveItem + pDialogueWindow->pNumPresenceButton;
    for (int i = pDialogueWindow->pStartingPosActiveItem; i < buttonsLimit; i++) {
        Skill skill = GetLearningDialogueSkill((DialogueId)pDialogueWindow->GetControl(i)->msg_param);
        if (skillMaxMasteryForClass(pParty->activeCharacter().classType, skill) != MASTERY_NONE &&
            !pParty->activeCharacter().pActiveSkills[skill]) {
            optionsText.push_back(localization->skillName(skill));
            haveLearnableSkills = true;
        } else {
            optionsText.push_back("");
        }
    }

    Recti dialogue = this->frameRect;
    dialogue.x = SIDE_TEXT_BOX_POS_X;
    dialogue.w = SIDE_TEXT_BOX_WIDTH;

    if (!haveLearnableSkills) {
        Character &player = pParty->activeCharacter();
        std::string str = localization->format(LSTR_SEEK_KNOWLEDGE_ELSEWHERE_S_THE_S, player.name, localization->className(player.classType));
        str = str + "\n \n" + localization->str(LSTR_I_CAN_OFFER_YOU_NOTHING_FURTHER);

        int text_height = assets->pFontArrus->CalcTextHeight(str, dialogue.w, 0);
        DrawTitleText(assets->pFontArrus.get(), 0, (SIDE_TEXT_BOX_BODY_TEXT_HEIGHT - text_height) / 2 + SIDE_TEXT_BOX_BODY_TEXT_OFFSET, colorTable.PaleCanary, str, 3, dialogue);
    } else {
        std::string skill_price_label = localization->format(LSTR_SKILL_COST_LU, cost);
        DrawTitleText(assets->pFontArrus.get(), 0, 146, colorTable.White, skill_price_label, 3, dialogue);
    }

    drawOptions(optionsText, selectColor, 18);
}

void GUIWindow_House::learnSelectedSkill(Skill skill) {
    int pPrice = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(), houseTable[houseId()]);
    if (skillMaxMasteryForClass(pParty->activeCharacter().classType, skill) != MASTERY_NONE) {
        if (!pParty->activeCharacter().pActiveSkills[skill]) {
            if (pParty->GetGold() < pPrice) {
                engine->_statusBar->setEvent(LSTR_YOU_DONT_HAVE_ENOUGH_GOLD);
                if (buildingType() == HOUSE_TYPE_TRAINING_GROUND) {
                    playHouseSound(houseId(), HOUSE_SOUND_TRAINING_NOT_ENOUGH_GOLD);
                } else if (buildingType() == HOUSE_TYPE_TAVERN) {
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

GUIWindow_House::GUIWindow_House(HouseId houseId) : GUIWindow(WINDOW_HOUSE_INTERIOR, {0, 0}, render->GetRenderDimensions()), _houseId(houseId) {
    gameTimer->setPaused(true);  // pause timer so not attacked

    current_screen_type = SCREEN_HOUSE;
    auto [exitCancelPos, exitCancelSize] = exitCancelButtonGeometry();
    pBtn_ExitCancel = CreateButton(exitCancelPos, exitCancelSize, BUTTON_TYPE_NORMAL, 0, UIMSG_Escape, 0, INPUT_ACTION_INVALID,
                                   localization->str(LSTR_EXIT_BUILDING), {ui_exit_cancel_button_background});

    _dialogueBackground = dialogueBackground(pParty->alignment);
    if (buildingType() >= HOUSE_TYPE_WEAPON_SHOP && buildingType() <= HOUSE_TYPE_MIRRORED_PATH_GUILD)
        _shopBackground = assets->getIcon(shopBackgroundNames[buildingType()]);

    for (int i = 0; i < houseNpcs.size(); ++i) {
        Pointi pos = {pNPCPortraits_x[houseNpcs.size() - 1][i], pNPCPortraits_y[houseNpcs.size() - 1][i]};
        houseNpcs[i].button = CreateButton(fmt::format("House_Npc{}", i), pos, {63, 73}, BUTTON_TYPE_NORMAL, 0,
                                           UIMSG_ClickHouseNPCPortrait, i, INPUT_ACTION_INVALID, houseNpcs[i].label);
    }

    CreateCharacterButtons();
}

void GUIWindow_House::Update() {
    if (!window_SpeakInHouse)
        return;
    // houseDialogManager() can destroy this window (under its
    // houseSpecificDialogue() call), so read what we need from `this` before calling it.
    HouseId house = houseId();
    houseDialogManager();
    if (!isShop(house)) {
        return;
    }
    if (pParty->PartyTimes.shopBanTimes[house] <= pParty->GetPlayingTime()) {
        pParty->PartyTimes.shopBanTimes[house] = Time();
        return;
    }
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);  // banned from shop so leaving
}

GUIWindow_House::~GUIWindow_House() {
    keyboardInputHandler->EndTextInput(this); // Banks and town halls ask for a gold amount.

    houseNpcs.clear();

    if (engine->config->settings.FlipOnExit.value()) {
        pParty->_viewYaw = (TrigLUT.uIntegerDoublePi - 1) & (TrigLUT.uIntegerPi + pParty->_viewYaw);
        pCamera3D->_viewYaw = pParty->_viewYaw;
    }
}

void GUIWindow_House::houseDialogueOptionSelected(DialogueId option) {
    _currentDialogue = option;
}

void GUIWindow_House::houseSpecificDialogue() {
    // Nothing
}

std::vector<DialogueId> GUIWindow_House::listDialogueOptions() {
    return {};
}

void GUIWindow_House::updateDialogueOnEscape() {
    if (_currentDialogue == DIALOGUE_MAIN) {
        _currentDialogue = DIALOGUE_NULL;
        return;
    }
    _currentDialogue = DIALOGUE_MAIN;
}

void GUIWindow_House::houseScreenClick() {
    // Nothing to do by default
}

void GUIWindow_House::playHouseGoodbyeSpeech() {
    // No speech by default
}

void GUIWindow_House::innClickBenchPortrait(int rosterId) {
    // Nothing to do by default - see GUIWindow_AdventurersInn.
}

void GUIWindow_House::innClickActivePortrait(int rosterId) {
    // Nothing to do by default - see GUIWindow_AdventurersInn.
}

void GUIWindow_House::innClickDismiss(int rosterId) {
    // Nothing to do by default - see GUIWindow_AdventurersInn.
}
