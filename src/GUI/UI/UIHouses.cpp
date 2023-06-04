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
int _F8B1DC_currentShopOption;  // F8B1DC
int dword_591080;               // 591080

BuildingType in_current_building_type;  // 00F8B198
DIALOGUE_TYPE dialog_menu_id;     // 00F8B19C

GraphicsImage *_591428_endcap = nullptr;

std::array<const HouseAnimDescr, 196> pAnimatedRooms = { {  // 0x4E5F70
    { "", 0x4, 0x1F4, BuildingType_Invalid, 0, 0 },
    { "Human Armor01", 0x20, 0x2C0, BuildingType_ArmorShop, 58, 0 },
    { "Necromancer Armor01", 0x20, 0x2D7, BuildingType_ArmorShop, 70, 0 },
    { "Dwarven Armor01", 0x20, 0x2EE, BuildingType_ArmorShop, 5, 0 },
    { "Wizard Armor", 0x20, 0x3BD, BuildingType_ArmorShop, 19, 0 },
    { "Warlock Armor", 0x20, 0x2D6, BuildingType_ArmorShop, 35, 0 },
    { "Elf Armor", 0x20, 0x2BC, BuildingType_ArmorShop, 79, 0 },
    { "Human Alchemisht01", 0xE, 0x2BE, BuildingType_AlchemistShop, 95, 0 },
    { "Necromancer Alchemist01", 0xE, 0x2D6, BuildingType_AlchemistShop, 69, 0 },
    { "Dwarven Achemist01", 0xE, 0x387, BuildingType_AlchemistShop, 4, 0 },
    { "Wizard Alchemist", 0xE, 0x232, BuildingType_AlchemistShop, 25, 0 },
    { "Warlock Alchemist", 0xE, 0x2BE, BuildingType_AlchemistShop, 42, 0 },
    { "Elf Alchemist", 0xE, 0x38A, BuildingType_AlchemistShop, 84, 0 },
    { "Human Bank01", 0x6, 0x384, BuildingType_Bank, 52, 0 },
    { "Necromancer Bank01", 0x6, 0x2D8, BuildingType_Bank, 71, 0 },
    { "Dwarven Bank", 0x6, 0x2F3, BuildingType_Bank, 6, 0 },
    { "Wizard Bank", 0x6, 0x3BA, BuildingType_Bank, 20, 0 },
    { "Warlock Bank", 0x6, 0x39F, BuildingType_Bank, 36, 0 },
    { "Elf Bank", 0x6, 0x2BC, BuildingType_Bank, 71, 0 },
    { "Boat01", 0xF, 0x4C, BuildingType_Boats, 53, 3 },
    { "Boat01d", 0xF, 0x4C, BuildingType_Boats, 53, 3 }, // this movie doesn't exist
    { "Human Magic Shop01", 0xA, 0x2C8, BuildingType_MagicShop, 54, 0 },
    { "Necromancer Magic Shop01", 0xE, 0x2DC, BuildingType_MagicShop, 66, 0 },
    { "Dwarven Magic Shop01", 0x2A, 0x2EF, BuildingType_MagicShop, 91, 0 },
    { "Wizard Magic Shop", 0x1E, 0x2DF, BuildingType_MagicShop, 15, 0 },
    { "Warlock Magic Shop", 0x7, 0x3B9, BuildingType_MagicShop, 15, 0 },
    { "Elf Magic Shop", 0x24, 0x2CC, BuildingType_MagicShop, 82, 0 },
    { "Human Stables01", 0x21, 0x31, BuildingType_Stables, 48, 3 },
    { "Necromancer Stables", 0x21, 0x2DD, BuildingType_Stables, 67, 3 },
    { "", 0x21, 0x2F0, BuildingType_Stables, 91, 3 },
    { "Wizard Stables", 0x21, 0x3BA, BuildingType_Stables, 16, 3 },
    { "Warlock Stables", 0x21, 0x181, BuildingType_Stables, 77, 3 },  // movie exist but unused in MM7 as Nighon doesn't have stables
    { "Elf Stables", 0x21, 0x195, BuildingType_Stables, 77, 3 },
    { "Human Tavern01", 0xD, 0x2C2, BuildingType_Tavern, 49, 0 },
    { "Necromancer Tavern 01", 0xD, 0x3B0, BuildingType_Tavern, 57, 0 },
    { "Dwarven Tavern01", 0xD, 0x2FE, BuildingType_Tavern, 94, 0 },
    { "Wizard Tavern", 0xD, 0x3BB, BuildingType_Tavern, 17, 0 },
    { "Warlock Tavern", 0xD, 0x3A8, BuildingType_Tavern, 33, 0 },
    { "Elf Tavern", 0xD, 0x2CD, BuildingType_Tavern, 78, 0 },
    { "Human Temple01", 0x24, 0x2DB, BuildingType_Temple, 50, 3 },
    { "Necromancer Temple", 0x24, 0x2DF, BuildingType_Temple, 60, 3 },
    { "Dwarven Temple01", 0x24, 0x2F1, BuildingType_Temple, 86, 3 },
    { "Wizard Temple", 0x24, 0x2E0, BuildingType_Temple, 10, 3 },
    { "Warlock Temple", 0x24, 0x3A4, BuildingType_Temple, 27, 3 },
    { "Elf Temple", 0x24, 0x2CE, BuildingType_Temple, 72, 3 },
    { "Human Town Hall", 0x10, 0x39C, BuildingType_TownHall, 14, 0 },
    { "Necromancer Town Hall01", 0x10, 0x3A4, BuildingType_TownHall, 61, 0 },
    { "Dwarven Town Hall", 0x10, 0x2DB, BuildingType_TownHall, 88, 0 }, // this movie doesn't exist, stone city doesn't have town hall
    { "Wizard Town Hall", 0x10, 0x3BD, BuildingType_TownHall, 11, 0 },
    { "Warlock Town Hall", 0x10, 0x2DB, BuildingType_TownHall, 28, 0 },
    { "Elf Town Hall", 0x10, 0x27A, BuildingType_TownHall, 73, 0 },
    { "Human Training Ground01", 0x18, 0x2C7, BuildingType_Training, 44, 0 },
    { "Necromancer Training Ground", 0x18, 0x3AD, BuildingType_Training, 62, 0 },
    { "Dwarven Training Ground", 0x18, 0x2F2, BuildingType_Training, 89, 0 },
    { "Wizard Training Ground", 0x18, 0x3A3, BuildingType_Training, 12, 0 },
    { "Warlock Training Ground", 0x18, 0x3A6, BuildingType_Training, 29, 0 },
    { "Elf Training Ground", 0x18, 0x19F, BuildingType_Training, 74, 0 },
    { "Human Weapon Smith01", 0x16, 0x2C1, BuildingType_WeaponShop, 45, 4 },
    { "Necromancer Weapon Smith01", 0x16, 0x2D9, BuildingType_WeaponShop, 63, 4 },
    { "Dwarven Weapon Smith01", 0x16, 0x2EE, BuildingType_WeaponShop, 82, 4 },
    { "Wizard Weapon Smith", 0x16, 0x2D5, BuildingType_WeaponShop, 13, 4 },
    { "Warlock Weapon Smith", 0x16, 0x2D7, BuildingType_WeaponShop, 23, 4 },
    { "Elf Weapon Smith", 0x16, 0x2CA, BuildingType_WeaponShop, 75, 4 },
    { "Air Guild", 0x1D, 0xA4, BuildingType_AirGuild, 1, 3 },
    { "Body Guild", 0x19, 0x3BF, BuildingType_BodyGuild, 2, 0 },
    { "Dark Guild", 0x19, 0x2D1, BuildingType_DarkGuild, 3, 0 },
    { "Earth Guild", 0x19, 0x2CB, BuildingType_EarthGuild, 83, 0 },
    { "Fire Guild", 0x1C, 0x2BF, BuildingType_FireGuild, 56, 0 },
    { "Light Guild", 0x1C, 0x2D5, BuildingType_LightGuild, 46, 0 },
    { "Mind Guild", 0x1C, 0xE5, BuildingType_MindGuild, 40, 0 },
    { "Spirit Guild", 0x1C, 0x2D2, BuildingType_SpiritGuild, 41, 0 },
    { "Water Guild", 0x1B, 0x2D3, BuildingType_WaterGuild, 24, 0 },
    { "Lord and Judge Out01", 1, 0, BuildingType_House, 39, 0 },
    { "Human Poor House 1", 8, 0, BuildingType_House, 0, 0 },
    { "Human Poor House 2", 8, 0, BuildingType_House, 0, 0 },
    { "Human Poor House 3", 8, 0, BuildingType_House, 0, 0 },
    { "Human Medium House 1", 8, 0, BuildingType_House, 0, 0 },
    { "Human Medium House 2", 8, 0, BuildingType_House, 0, 0 },
    { "Human Medium House 3", 8, 0, BuildingType_House, 0, 0 }, // this movie doesn't exist
    { "Human Rich House 1", 8, 0, BuildingType_House, 0, 0 },
    { "Human Rich House 2", 8, 0, BuildingType_House, 0, 0 },
    { "Human Rich House 3", 8, 0, BuildingType_House, 0, 0 }, // this movie doesn't exist
    { "Elven Poor House 1", 8, 0, BuildingType_House, 0, 0 },
    { "Elven Poor House 2", 8, 0, BuildingType_House, 0, 0 },
    { "Elven Poor House 3", 8, 0, BuildingType_House, 0, 0 }, // this movie doesn't exist
    { "Elven Medium House 1", 8, 0, BuildingType_House, 0, 0 },
    { "Elven Medium House 2", 8, 0, BuildingType_House, 0, 0 },
    { "Elven Medium House 3", 8, 0, BuildingType_House, 0, 0 }, // this movie doesn't exist
    { "Elven Rich House 1", 8, 0, BuildingType_House, 0, 0 },
    { "Elven Rich House 2", 8, 0, BuildingType_House, 0, 0 },
    { "Elven Rich House 3", 8, 0, BuildingType_House, 0, 0 },
    { "Dwarven Poor House 1", 8, 0, BuildingType_House, 0, 0 },
    { "Dwarven Poor House 2", 8, 0, BuildingType_House, 0, 0 },
    { "Dwarven Poor House 3", 8, 0, BuildingType_House, 0, 0 }, // this movie doesn't exist
    { "Dwarven Medium House 1", 8, 0, BuildingType_House, 0, 0 },
    { "Dwarven Medium House 2", 8, 0, BuildingType_House, 0, 0 },
    { "Dwarven Medium House 3", 8, 0, BuildingType_House, 0, 0 }, // this movie doesn't exist
    { "Dwarven Rich House 1", 8, 0, BuildingType_House, 0, 0 },
    { "Dwarven Rich House 2", 8, 0, BuildingType_House, 0, 0 },
    { "Dwarven Rich House 3", 8, 0, BuildingType_House, 0, 0 }, // this movie doesn't exist
    { "Wizard Poor House 1", 8, 0, BuildingType_House, 0, 0 },
    { "Wizard Poor House 2", 8, 0, BuildingType_House, 0, 0 },
    { "Wizard Poor House 3", 8, 0, BuildingType_House, 0, 0 }, // this movie doesn't exist
    { "Wizard Medium House 1", 8, 0, BuildingType_House, 0, 0 },
    { "Wizard Medium House 2", 8, 0, BuildingType_House, 0, 0 },
    { "Wizard Medium House 3", 8, 0, BuildingType_House, 0, 0 }, // this movie doesn't exist
    { "Wizard Rich House 1", 8, 0, BuildingType_House, 0, 0 },
    { "Wizard Rich House 2", 8, 0, BuildingType_House, 0, 0 },
    { "Wizard Rich House 3", 8, 0, BuildingType_House, 0, 0 }, // this movie doesn't exist
    { "Necromancer Poor House 1", 8, 0, BuildingType_House, 0, 0 },
    { "Necromancer Poor House 2", 8, 0, BuildingType_House, 0, 0 },
    { "Necromancer Poor House 3", 8, 0, BuildingType_House, 0, 0 }, // this movie doesn't exist
    { "Necromancer Medium House 1", 8, 0, BuildingType_House, 0, 0 },
    { "Necromancer Medium House 2", 8, 0, BuildingType_House, 0, 0 },
    { "Necromancer Medium House 3", 8, 0, BuildingType_House, 0, 0 }, // this movie doesn't exist
    { "Necromancer Rich House 1", 8, 0, BuildingType_House, 0, 0 },
    { "Necromancer Rich House 2", 8, 0, BuildingType_House, 0, 0 },
    { "Necromancer Rich House 3", 8, 0, BuildingType_House, 0, 0 }, // this movie doesn't exist
    { "Warlock Poor House 1", 8, 0, BuildingType_House, 0, 0 },
    { "Warlock Poor House 2", 8, 0, BuildingType_House, 0, 0 },
    { "Warlock Poor House 3", 8, 0, BuildingType_House, 0, 0 }, // this movie doesn't exist
    { "Warlock Medium House 1", 8, 0, BuildingType_House, 0, 0 },
    { "Warlock Medium House 2", 8, 0, BuildingType_House, 0, 0 },
    { "Warlock Medium House 3", 8, 0, BuildingType_House, 0, 0 }, // this movie doesn't exist
    { "Warlock Rich House 1", 8, 0, BuildingType_House, 0, 0 },
    { "Warlock Rich House 2", 8, 0, BuildingType_House, 0, 0 },
    { "Warlock Rich House 3", 8, 0, BuildingType_House, 0, 0 },
    { "Out01 Temple of the Moon", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out01 Dragon Cave", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out02 Castle Harmondy", 0x24, 0, BuildingType_Castle, 0, 0 },
    { "Out02 White Cliff Cave", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out03 Erathian Sewer", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out03 Fort Riverstride", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out03 Castle Gryphonheart", 0x24, 0, BuildingType_Castle, 0, 0 },
    { "Out04 Elf Castle", 0x24, 0, BuildingType_Castle, 0, 0 },
    { "Out04 Tularean Caves", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out04 Clanker's Laboratory", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out05 Hall of the Pit", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out05 Watchtower 6", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out06 School of Sorcery", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out06 Red Dwarf Mines", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out07 Castle Lambent", 0x24, 0, BuildingType_Castle, 0, 0 },
    { "Out07 Walls of Mist", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out07 Temple of the Light", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out08 Evil Entrance", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out08 Breeding Zone", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out08 Temple of the Dark", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out09 Grand Temple of the Moon", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out09 Grand Temple of the Sun", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out10 Thunderfist Mountain", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out10 The Maze", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out10 Connecting Tunnel Cave #1", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out11 Stone City", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out12 Colony Zod", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out12 Connecting Tunnel Cave #1", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out13 Mercenary Guild", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out13 Tidewater Caverns", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out13 Wine Cellar", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out14 Titan's Stronghold", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out14 Temple of Baa", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out14 Hall under the Hill", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Out15 The Linclon", 0x24, 0, BuildingType_Dungeon, 0, 0 },
    { "Jail", 0x24, 0, BuildingType_Jail, 0, 0 },
    { "Harmondale Throne Room", 0x24, 0, BuildingType_House, 0, 0 },
    { "Gryphonheart Throne Room", 0x24, 0, BuildingType_Throne_Room, 0, 0 },
    { "Elf Castle Throne Room", 0x24, 0, BuildingType_Throne_Room, 0, 0 },
    { "Wizard Castle Throne Room", 0x24, 0, BuildingType_Throne_Room, 0, 0 },
    { "Necromancer Castle Throne Rooms", 0x24, 0, BuildingType_Throne_Room, 0, 0 },
    { "Master Thief", 0x24, 0, BuildingType_House, 0, 0 },
    { "Dwarven King", 0x24, 0, BuildingType_Throne_Room, 0, 0 },
    { "Arms Master", 0x24, 0, BuildingType_House, 0, 0 },
    { "Warlock", 0x24, 0, BuildingType_House, 0, 0 },
    { "Lord Markam", 0x24, 0, BuildingType_Throne_Room, 0, 0 },
    { "Arbiter Neutral Town", 0x24, 0, BuildingType_House, 0, 0 },
    { "Arbiter Good Town", 0x24, 0, BuildingType_House, 0, 0 },
    { "Arbiter Evil Town", 0x24, 0, BuildingType_House, 0, 0 },
    { "Necromancer Throne Room Empty", 0x24, 0, BuildingType_Throne_Room, 0, 0 },
    { "", 0x24, 0, BuildingType_House, 0, 0 },
    { "", 0x24, 0, BuildingType_House, 0, 0 },
    { "", 0x24, 0, BuildingType_House, 0, 0 },
    { "", 0x24, 0, BuildingType_House, 0, 0 },
    { "Boat01", 0xF, 0, BuildingType_House, 53, 3 },
    { "", 0x24, 0, BuildingType_Boats, 0, 0 },
    { "", 0x24, 0, BuildingType_Boats, 0, 0 },
    { "", 0x24, 0, BuildingType_Boats, 0, 0 },
    { "", 0x24, 0, BuildingType_House, 0, 0 },
    { "Arbiter Room Neutral", 0x24, 0, BuildingType_House, 0, 0 }, // this movie doesn't exist
    { "Out02 Castle Harmondy Abandoned", 0x24, 0, BuildingType_Castle, 0, 0 },
    { "Human Temple02", 0x24, 0x3AB, BuildingType_Temple, 27, 0 },
    { "Player Castle Good", 0x24, 0, BuildingType_Castle, 0, 0 },
    { "Player Castle Bad", 0x24, 0, BuildingType_Castle, 0, 0 }
} };

static std::array<const char *, 19> _4F03B8_shop_background_names = {
    { "", "WEPNTABL", "ARMORY", "MAGSHELF", "MAGSHELF", "MAGSHELF", "MAGSHELF",
    "MAGSHELF", "MAGSHELF", "MAGSHELF", "MAGSHELF", "MAGSHELF", "MAGSHELF",
    "MAGSHELF", "MAGSHELF", "MAGSHELF", "MAGSHELF", "MAGSHELF", "MAGSHELF" } };

int ItemAmountForShop(BuildingType buildingType) {
    switch(buildingType) {
      case BuildingType_WeaponShop:
      case BuildingType_GeneralStore:
        return 6;
      case BuildingType_ArmorShop:
        return 8;
      case BuildingType_MagicShop:
        return 12;
      case BuildingType_AlchemistShop:
        return 12;
      case BuildingType_FireGuild:
      case BuildingType_AirGuild:
      case BuildingType_WaterGuild:
      case BuildingType_EarthGuild:
      case BuildingType_SpiritGuild:
      case BuildingType_MindGuild:
      case BuildingType_BodyGuild:
      case BuildingType_LightGuild:
      case BuildingType_DarkGuild:
      case BuildingType_MirroredPath:
        return 12;
      default:
        return 0;
    }
}

std::array<std::string, 6> portraitClickLabel;

void FillAviableSkillsToTeach(BuildingType type);

//----- (004B3A72) --------------------------------------------------------
void InitializaDialogueOptions_Tavern(BuildingType type) {
    int num_buttons;  // esi@1

    num_buttons = 0;
    if (type == BuildingType_Tavern) {
        num_buttons = 2;
        CreateShopDialogueButtonAtRow(0, DIALOGUE_TAVERN_ARCOMAGE_RULES);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_TAVERN_ARCOMAGE_VICTORY_CONDITIONS);
        if (pParty->hasItem(ITEM_QUEST_ARCOMAGE_DECK)) {
            num_buttons = 3;
            CreateShopDialogueButtonAtRow(2, DIALOGUE_TAVERN_ARCOMAGE_RESULT);
        }
    }
    pDialogueWindow->_41D08F_set_keyboard_control_group(num_buttons, 1, 0, 2);
    dword_F8B1E0 = pDialogueWindow->pNumPresenceButton;
}
// F8B1E0: using guessed type int dword_F8B1E0;

//----- (004B3AD4) --------------------------------------------------------
void InitializaDialogueOptions_Shops(BuildingType type) {
    switch (type) {
    case BuildingType_WeaponShop:
    case BuildingType_ArmorShop:
    case BuildingType_MagicShop:
        CreateShopDialogueButtonAtRow(0, DIALOGUE_SHOP_SELL);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_SHOP_IDENTIFY);
        CreateShopDialogueButtonAtRow(2, DIALOGUE_SHOP_REPAIR);
        pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
        break;

    case BuildingType_AlchemistShop:
        CreateShopDialogueButtonAtRow(0, DIALOGUE_SHOP_SELL);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_SHOP_IDENTIFY);
        pDialogueWindow->_41D08F_set_keyboard_control_group(2, 1, 0, 2);
        break;

    case BuildingType_GeneralStore:
        CreateShopDialogueButtonAtRow(0, DIALOGUE_SHOP_SELL);
        pDialogueWindow->_41D08F_set_keyboard_control_group(1, 1, 0, 2);
        break;

    default:
        break;
    }

    dword_F8B1E0 = pDialogueWindow->pNumPresenceButton;
}


//----- (004B3B42) --------------------------------------------------------
void InitializaDialogueOptions(BuildingType type) {
    switch (type) {
    case BuildingType_WeaponShop:
    case BuildingType_ArmorShop:
    case BuildingType_MagicShop:
    case BuildingType_AlchemistShop:
    {
        CreateShopDialogueButtonAtRow(0, DIALOGUE_SHOP_BUY_STANDARD);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_SHOP_BUY_SPECIAL);
        CreateShopDialogueButtonAtRow(2, DIALOGUE_SHOP_DISPLAY_EQUIPMENT);
        CreateShopDialogueButtonAtRow(3, DIALOGUE_LEARN_SKILLS);
        pDialogueWindow->_41D08F_set_keyboard_control_group(4, 1, 0, 2);
    } break;

    case BuildingType_FireGuild:
    {
        CreateShopDialogueButtonAtRow(0, DIALOGUE_GUILD_BUY_BOOKS);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_LEARN_FIRE);
        CreateShopDialogueButtonAtRow(2, DIALOGUE_LEARN_LEARNING);
        pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
    } break;

    case BuildingType_AirGuild:
    {
        CreateShopDialogueButtonAtRow(0, DIALOGUE_GUILD_BUY_BOOKS);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_LEARN_AIR);
        CreateShopDialogueButtonAtRow(2, DIALOGUE_LEARN_LEARNING);
        pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
    } break;

    case BuildingType_WaterGuild:
    {
        CreateShopDialogueButtonAtRow(0, DIALOGUE_GUILD_BUY_BOOKS);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_LEARN_WATER);
        CreateShopDialogueButtonAtRow(2, DIALOGUE_LEARN_LEARNING);
        pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
    } break;

    case BuildingType_EarthGuild:
    {
        CreateShopDialogueButtonAtRow(0, DIALOGUE_GUILD_BUY_BOOKS);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_LEARN_EARTH);
        CreateShopDialogueButtonAtRow(2, DIALOGUE_LEARN_LEARNING);
        pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
    } break;

    case BuildingType_SpiritGuild:
    {
        CreateShopDialogueButtonAtRow(0, DIALOGUE_GUILD_BUY_BOOKS);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_LEARN_SPIRIT);
        CreateShopDialogueButtonAtRow(2, DIALOGUE_LEARN_MEDITATION);
        pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
    } break;

    case BuildingType_MindGuild:
    {
        CreateShopDialogueButtonAtRow(0, DIALOGUE_GUILD_BUY_BOOKS);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_LEARN_MIND);
        CreateShopDialogueButtonAtRow(2, DIALOGUE_LEARN_MEDITATION);
        pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
    } break;

    case BuildingType_BodyGuild:
    {
        CreateShopDialogueButtonAtRow(0, DIALOGUE_GUILD_BUY_BOOKS);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_LEARN_BODY);
        CreateShopDialogueButtonAtRow(2, DIALOGUE_LEARN_MEDITATION);
        pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
    } break;

    case BuildingType_LightGuild:
    {
        CreateShopDialogueButtonAtRow(0, DIALOGUE_GUILD_BUY_BOOKS);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_LEARN_LIGHT);
        pDialogueWindow->_41D08F_set_keyboard_control_group(2, 1, 0, 2);
    } break;

    case BuildingType_DarkGuild:
    {
        CreateShopDialogueButtonAtRow(0, DIALOGUE_GUILD_BUY_BOOKS);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_LEARN_DARK);
        pDialogueWindow->_41D08F_set_keyboard_control_group(2, 1, 0, 2);
    } break;

    case BuildingType_ElementalGuild:
    {
        CreateShopDialogueButtonAtRow(0, DIALOGUE_GUILD_BUY_BOOKS);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_LEARN_FIRE);
        CreateShopDialogueButtonAtRow(2, DIALOGUE_LEARN_AIR);
        CreateShopDialogueButtonAtRow(3, DIALOGUE_LEARN_WATER);
        CreateShopDialogueButtonAtRow(4, DIALOGUE_LEARN_EARTH);
        pDialogueWindow->_41D08F_set_keyboard_control_group(5, 1, 0, 2);
    } break;

    case BuildingType_SelfGuild:
    {
        CreateShopDialogueButtonAtRow(0, DIALOGUE_GUILD_BUY_BOOKS);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_LEARN_SPIRIT);
        CreateShopDialogueButtonAtRow(2, DIALOGUE_LEARN_MIND);
        CreateShopDialogueButtonAtRow(3, DIALOGUE_LEARN_BODY);
        pDialogueWindow->_41D08F_set_keyboard_control_group(4, 1, 0, 2);
    } break;

    case BuildingType_MirroredPath:
    {
        CreateShopDialogueButtonAtRow(0, DIALOGUE_GUILD_BUY_BOOKS);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_LEARN_DARK);
        CreateShopDialogueButtonAtRow(2, DIALOGUE_LEARN_LIGHT);
        pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
    } break;
    case BuildingType_TownHall:
    {
        int num_buttons = 1;
        CreateShopDialogueButtonAtRow(0, DIALOGUE_TOWNHALL_BOUNTY_HUNT);

        if (pParty->uFine) {
            num_buttons++;
            CreateShopDialogueButtonAtRow(1, DIALOGUE_TOWNHALL_PAY_FINE);
        }

        pDialogueWindow->_41D08F_set_keyboard_control_group(num_buttons, 1, 0, 2);
    } break;

    case BuildingType_Bank:
    {
        CreateShopDialogueButtonAtRow(0, DIALOGUE_BANK_PUT_GOLD);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_BANK_GET_GOLD);
        pDialogueWindow->_41D08F_set_keyboard_control_group(2, 1, 0, 2);
    } break;

    case BuildingType_Temple:
    {
        CreateShopDialogueButtonAtRow(0, DIALOGUE_TEMPLE_HEAL);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_TEMPLE_DONATE);
        CreateShopDialogueButtonAtRow(2, DIALOGUE_LEARN_SKILLS);
        pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
    } break;

    case BuildingType_Stables:
    case BuildingType_Boats:
    {
        CreateShopDialogueButtonAtRow(0, DIALOGUE_TRANSPORT_SCHEDULE_1);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_TRANSPORT_SCHEDULE_2);
        CreateShopDialogueButtonAtRow(2, DIALOGUE_TRANSPORT_SCHEDULE_3);
        CreateShopDialogueButtonAtRow(3, DIALOGUE_TRANSPORT_SCHEDULE_4);
        pDialogueWindow->_41D08F_set_keyboard_control_group(4, 1, 0, 2);
    } break;

    case BuildingType_Training:
    {
        CreateShopDialogueButtonAtRow(0, DIALOGUE_TRAINING_HALL_TRAIN);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_LEARN_SKILLS);
        pDialogueWindow->_41D08F_set_keyboard_control_group(2, 1, 0, 2);
    } break;

    case BuildingType_Tavern:
    case BuildingType_AdventuresInn:
    {
        CreateShopDialogueButtonAtRow(0, DIALOGUE_TAVERN_REST);
        CreateShopDialogueButtonAtRow(1, DIALOGUE_TAVERN_BUY_FOOD);
        CreateShopDialogueButtonAtRow(2, DIALOGUE_LEARN_SKILLS);
        if (window_SpeakInHouse->wData.val >= HOUSE_TAVERN_EMERALD_ISLE && window_SpeakInHouse->wData.val <= HOUSE_TAVERN_STONE_CITY) {
            CreateShopDialogueButtonAtRow(3, DIALOGUE_TAVERN_ARCOMAGE_MAIN);
            pDialogueWindow->_41D08F_set_keyboard_control_group(4, 1, 0, 2);
        } else {
            pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
        }
    } break;

    case BuildingType_GeneralStore:
    {
        CreateShopDialogueButtonAtRow(0, DIALOGUE_SHOP_BUY_STANDARD);
        pDialogueWindow->_41D08F_set_keyboard_control_group(1, 1, 0, 2);
    } break;

    case BuildingType_Jail:
    case BuildingType_TownHall_MM6:
    case BuildingType_Throne_Room:
    case BuildingType_Castle:
    case BuildingType_Dungeon:
    case BuildingType_Seer:
    case BuildingType_Circus:
    case BuildingType_MercenaryGuild:
    case BuildingType_ShadowGuild:
        break;

    default:
        Error("Invalid building type: %u", type);
    }
    dword_F8B1E0 = pDialogueWindow->pNumPresenceButton;
}


bool HouseUI_CheckIfPlayerCanInteract() {
    if (!pParty->hasActiveCharacter()) {  // to avoid access zeroeleement
        return false;
    }

    if (pParty->activeCharacter().CanAct()) {
        pDialogueWindow->pNumPresenceButton = dword_F8B1E0;
        return true;
    } else {
        pDialogueWindow->pNumPresenceButton = 0;
        GUIWindow window = *pPrimaryWindow;
        window.uFrameX = SIDE_TEXT_BOX_POS_X;
        window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
        window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

        std::string str = localization->FormatString(
            LSTR_FMT_S_IS_IN_NO_CODITION_TO_S,
            pParty->activeCharacter().name.c_str(),
            localization->GetString(LSTR_DO_ANYTHING));
        window.DrawTitleText(
            pFontArrus, 0,
            (212 - pFontArrus->CalcTextHeight(str, window.uFrameWidth, 0)) / 2 +
            101,
            ui_house_player_cant_interact_color, str, 3);
        return false;
    }
}

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
    dword_F8B1E4 = 0;
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
        if (in_current_building_type == BuildingType_Throne_Room && pParty->uFine) {  // going to jail
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
                PlayHouseSound(uHouseID, HouseSound_Greeting_2);
                return true;
            }
        } else if ((isStable(uHouseID) || isBoat(uHouseID)) && !isTravelAvailable(uHouseID)) {
            return true;
        }
        PlayHouseSound(uHouseID, HouseSound_Greeting);
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
            if (!(pNPCStats->pNewNPCData[i].uFlags & 0x80)) {
                HouseNPCData[uNumDialogueNPCPortraits + 1 -
                    ((dword_591080 != 0) ? 1 : 0)] =
                    &pNPCStats->pNewNPCData[i];
                npc_id_arr[uNumDialogueNPCPortraits] =
                    pNPCStats->pNewNPCData[i].uPortraitID;
                ++uNumDialogueNPCPortraits;
                if ((pNPCStats->pNewNPCData[i].uFlags & 3) != 2)
                    ++pNPCStats->pNewNPCData[i].uFlags;
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

//----- (004B1E92) --------------------------------------------------------
void PlayHouseSound(unsigned int uHouseID, HouseSoundID sound) {
    if (uHouseID > 0) {
        if (pAnimatedRooms[buildingTable[uHouseID - HOUSE_SMITH_EMERALD_ISLE].uAnimationID].uRoomSoundId) {
            int roomSoundId = pAnimatedRooms[buildingTable[uHouseID - HOUSE_SMITH_EMERALD_ISLE].uAnimationID].uRoomSoundId;
            pAudioPlayer->playHouseSound((SoundID)(sound + 100 * (roomSoundId + 300)), true);
        }
    }
}

//----- (004B1D27) --------------------------------------------------------
void GetHouseGoodbyeSpeech() {
    int v7[4];      // [sp+Ch] [bp-10h]@12

    if (in_current_building_type != BuildingType_Invalid) {
        if (in_current_building_type > BuildingType_MagicShop) {
            if (in_current_building_type == BuildingType_Bank) {
                if (!dword_F8B1E4) return;
            } else {
                if (in_current_building_type != BuildingType_Temple) return;
            }
            PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_Greeting_2);
            return;
        }
        if (pParty->PartyTimes.shopBanTimes[window_SpeakInHouse->houseId()] <= pParty->GetPlayingTime()) {
            if (pParty->GetGold() <= 10000) {
                if (!dword_F8B1E4) return;
                PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_Goodbye);
                return;
            }
            PlayHouseSound(window_SpeakInHouse->wData.val, (HouseSoundID)(dword_F8B1E4 + 3));
            if (!dword_F8B1E4 && !pParty->_delayedReactionTimer) {
                int id = pParty->getRandomActiveCharacterId(vrng.get());

                if (id != -1) {
                    pParty->setDelayedReaction(SPEECH_ShopRude, id);
                    return;
                }
            }
        } else {  // caught stealing
            if (!pParty->_delayedReactionTimer) {
                int id = pParty->getRandomActiveCharacterId(vrng.get());

                if (id != -1) {
                    pParty->setDelayedReaction(SPEECH_ShopRude, id);
                    return;
                }
            }
        }
    }
}

//----- (004BCACC) --------------------------------------------------------
void OnSelectShopDialogueOption(DIALOGUE_TYPE option) {
    if (!pDialogueWindow->pNumPresenceButton)
        return;
    render->ClearZBuffer();

    if (dialog_menu_id == DIALOGUE_MAIN) {
        pDialogueWindow->Release();
        pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, {render->GetRenderDimensions().w, 345}, 0);
        pBtn_ExitCancel = pDialogueWindow->CreateButton({526, 445}, {75, 33}, 1, 0, UIMSG_Escape, 0, InputAction::Invalid,
                                                        localization->GetString(LSTR_END_CONVERSATION), {ui_buttdesc2});
        pDialogueWindow->CreateButton({8, 8}, {450, 320}, 1, 0, UIMSG_BuyInShop_Identify_Repair, 0);
        dialog_menu_id = option;
        if (in_current_building_type < BuildingType_TownHall_MM6) {
            shop_ui_background = assets->getImage_ColorKey(_4F03B8_shop_background_names[(int)in_current_building_type]);
        }
    }

    // NEW
    // TODO(Nik-RE-dev): houseDialogueOptionSelected must be called without switch
    switch (in_current_building_type) {
      case BuildingType_FireGuild:
      case BuildingType_AirGuild:
      case BuildingType_WaterGuild:
      case BuildingType_EarthGuild:
      case BuildingType_SpiritGuild:
      case BuildingType_MindGuild:
      case BuildingType_BodyGuild:
      case BuildingType_LightGuild:
      case BuildingType_DarkGuild:
      case BuildingType_ElementalGuild:
      case BuildingType_SelfGuild:
      case BuildingType_MirroredPath:
      case BuildingType_Bank:
      case BuildingType_Temple:
      case BuildingType_Tavern:
      case BuildingType_Training:
      case BuildingType_Jail:
      case BuildingType_MercenaryGuild:
      case BuildingType_TownHall:
      case BuildingType_WeaponShop:
      case BuildingType_ArmorShop:
      case BuildingType_MagicShop:
      case BuildingType_AlchemistShop:
        ((GUIWindow_House*)window_SpeakInHouse)->houseDialogueOptionSelected(option);
        break;
      default:
        return;
    }

    switch (option) {
    case DIALOGUE_LEARN_SKILLS:
    {
        pDialogueWindow->eWindowType = WINDOW_MainMenu;
        UI_CreateEndConversationButton();
        FillAviableSkillsToTeach(in_current_building_type);
        break;
    }
    case DIALOGUE_TAVERN_ARCOMAGE_MAIN:
    {
        pDialogueWindow->eWindowType = WINDOW_MainMenu;
        UI_CreateEndConversationButton();
        InitializaDialogueOptions_Tavern(in_current_building_type);
        break;
    }
    case DIALOGUE_TAVERN_ARCOMAGE_RULES:
    case DIALOGUE_TAVERN_ARCOMAGE_VICTORY_CONDITIONS:
    {
        dialog_menu_id = option;
        break;
    }
    case DIALOGUE_TAVERN_ARCOMAGE_RESULT:
    {
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_PlayArcomage, 0, 0);
        dialog_menu_id = DIALOGUE_TAVERN_ARCOMAGE_RESULT;
        break;
    }
    case DIALOGUE_SHOP_SELL:
    case DIALOGUE_SHOP_IDENTIFY:
    case DIALOGUE_SHOP_REPAIR:
    {
        dialog_menu_id = option;
        pParty->placeHeldItemInInventoryOrDrop();
        break;
    }
    case DIALOGUE_SHOP_DISPLAY_EQUIPMENT:
    {
        pDialogueWindow->eWindowType = WINDOW_MainMenu;
        UI_CreateEndConversationButton();
        InitializaDialogueOptions_Shops(in_current_building_type);
        break;
    }
    default:
    {
        if (IsSkillLearningDialogue(option)) {
            int pPrice = PriceCalculator::skillLearningCostForPlayer(&pParty->activeCharacter(),
                                                                         buildingTable[window_SpeakInHouse->wData.val - 1]);  // ecx@227
            auto skill = GetLearningDialogueSkill(option);
            if (skillMaxMasteryPerClass[pParty->activeCharacter().classType][skill] != PLAYER_SKILL_MASTERY_NONE) {
                if (!pParty->activeCharacter().pActiveSkills[skill]) {
                    if (pParty->GetGold() < pPrice) {
                        GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
                        if (in_current_building_type == BuildingType_Training
                            || in_current_building_type == BuildingType_Tavern)
                            PlayHouseSound(
                            window_SpeakInHouse->wData.val, HouseSound_Goodbye);
                        else
                            PlayHouseSound(
                            window_SpeakInHouse->wData.val,
                                HouseSound_NotEnoughMoney);
                    } else {
                        pParty->TakeGold(pPrice);
                        dword_F8B1E4 = 1;
                        pParty->activeCharacter().pActiveSkills[skill] = 1;
                        pParty->activeCharacter().playReaction(SPEECH_SkillLearned);
                    }
                }
            }
        }
        break;
    }
    }
}

void SimpleHouseDialog() {
    NPCData *pNPC;       // esi@6
    const char *v15;           // esi@14
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
    char *pInString;  // [sp+114h] [bp-8h]@12

    GUIWindow house_window = *pDialogueWindow;
    if (pDialogueNPCCount == uNumDialogueNPCPortraits && uHouse_ExitPic) {
        house_window.uFrameX = 493;
        house_window.uFrameWidth = 126;
        house_window.uFrameZ = 366;
        house_window.DrawTitleText(pFontCreate, 0, 2, Color(),
            pMapStats->pInfos[uHouse_ExitPic].pName, 3);
        house_window.uFrameX = SIDE_TEXT_BOX_POS_X;
        house_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
        house_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;
        if (pTransitionStrings[uHouse_ExitPic].empty()) {
            auto str = localization->FormatString(LSTR_FMT_ENTER_S, pMapStats->pInfos[uHouse_ExitPic].pName.c_str());
            house_window.DrawTitleText(pFontCreate, 0, (212 - pFontCreate->CalcTextHeight(str, house_window.uFrameWidth, 0)) / 2 + 101, Color(), str, 3);
            return;
        }

        int vertMargin = (212 - pFontCreate->CalcTextHeight(pTransitionStrings[uHouse_ExitPic], house_window.uFrameWidth, 0)) / 2 + 101;
        house_window.DrawTitleText(pFontCreate, 0, vertMargin, Color(), pTransitionStrings[uHouse_ExitPic], 3);
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
                pInString = pNPCStats->pNPCGreetings[pNPC->greet].pGreetings[((pNPC->uFlags & 3) == 2)];
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
                pDialogueWindow->DrawText(pFontArrus, {13, 354 - h}, Color(),
                    pFontArrus->FitTextInAWindow(pInString, house_window.uFrameWidth, 13), 0, 0, Color());
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
            v15 = (char *)pNPCTopics[pNPC->dialogue_1_evt_id].pTopic;
            if (!v15) {
                pButton->msg_param = 0;
                v15 = "";
            }
            pButton->sLabel = v15;
            continue;
        case DIALOGUE_SCRIPTED_LINE_2:
            v15 = (char *)pNPCTopics[pNPC->dialogue_2_evt_id].pTopic;
            if (!v15) {
                pButton->msg_param = 0;
                v15 = "";
            }

            pButton->sLabel = v15;
            continue;

        case DIALOGUE_SCRIPTED_LINE_3:
            v15 = (char *)pNPCTopics[pNPC->dialogue_3_evt_id].pTopic;
            if (!v15) {
                pButton->msg_param = 0;
                v15 = "";
            }
            pButton->sLabel = v15;
            continue;
        case DIALOGUE_SCRIPTED_LINE_4:
            v15 = (char *)pNPCTopics[pNPC->dialogue_4_evt_id].pTopic;
            if (!v15) {
                pButton->msg_param = 0;
                v15 = "";
            }
            pButton->sLabel = v15;
            continue;
        case DIALOGUE_SCRIPTED_LINE_5:
            v15 = (char *)pNPCTopics[pNPC->dialogue_5_evt_id].pTopic;
            if (!v15) {
                pButton->msg_param = 0;
                v15 = "";
            }
            pButton->sLabel = v15;
            continue;
        case DIALOGUE_SCRIPTED_LINE_6:
            v15 = (char *)pNPCTopics[pNPC->dialogue_6_evt_id].pTopic;
            if (!v15) {
                pButton->msg_param = 0;
                v15 = "";
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
        house_window.DrawText(pTextFont, {13, 354 - pTextHeight}, Color(), pTextFont->FitTextInAWindow(current_npc_text, w.uFrameWidth, 13), 0, 0, Color());
    }
}

//----- (004BE571) --------------------------------------------------------
int sub_4BE571_AddItemToSet(
    DIALOGUE_TYPE valueToAdd,
    DIALOGUE_TYPE*outPutSet,
    int elemsAlreadyPresent,
    int elemsNeeded
) {
    int i;  // esi@3

    if (elemsAlreadyPresent < elemsNeeded) {
        for (i = 0; i < elemsAlreadyPresent; ++i) {
            if (valueToAdd == outPutSet[i]) return elemsAlreadyPresent;
        }
        outPutSet[elemsAlreadyPresent] = valueToAdd;
        return elemsAlreadyPresent + 1;
    }
    return elemsNeeded;
}

//----- (004B3703) --------------------------------------------------------
void FillAviableSkillsToTeach(BuildingType type) {
    const char *v30;   // ecx@65
    int v15;           // ecx@19
    int v21;           // ecx@34

    DIALOGUE_TYPE options[5];
    int num_options = 0;

    _F8B1DC_currentShopOption = 0;

    switch (type) {
    case BuildingType_WeaponShop:
    {
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 4; ++j) {
                if (i)
                    v21 = weaponShopVariationSpecial[window_SpeakInHouse->houseId()].item_class[j];
                else
                    v21 = weaponShopVariationStandart[window_SpeakInHouse->houseId()].item_class[j];

                DIALOGUE_TYPE v34;
                switch (v21) {
                case 23: v34 = DIALOGUE_LEARN_SWORD; break;
                case 24: v34 = DIALOGUE_LEARN_DAGGER; break;
                case 25: v34 = DIALOGUE_LEARN_AXE; break;
                case 26: v34 = DIALOGUE_LEARN_SPEAR; break;
                case 27: v34 = DIALOGUE_LEARN_BOW; break;
                case 28: v34 = DIALOGUE_LEARN_MACE; break;
                case 30: v34 = DIALOGUE_LEARN_STAFF; break;
                default:
                    continue;
                }
                num_options = sub_4BE571_AddItemToSet(v34, options, num_options, 5);
            }
        }
    } break;

    case BuildingType_ArmorShop:
    {
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 4; ++k) {
                    if (j) {
                        if (i)
                            v15 = armorShopBottomRowVariationSpecial[window_SpeakInHouse->houseId()].item_class[k];
                        else
                            v15 = armorShopBottomRowVariationStandart[window_SpeakInHouse->houseId()].item_class[k];
                    } else {
                        if (i)
                            v15 = armorShopTopRowVariationSpecial[window_SpeakInHouse->houseId()].item_class[k];
                        else
                            v15 = armorShopTopRowVariationStandart[window_SpeakInHouse->houseId()].item_class[k];
                    }

                    DIALOGUE_TYPE v33;
                    switch (v15) {
                    case 31: v33 = DIALOGUE_LEARN_LEATHER; break;
                    case 32: v33 = DIALOGUE_LEARN_CHAIN; break;
                    case 33: v33 = DIALOGUE_LEARN_PLATE; break;
                    case 34: v33 = DIALOGUE_LEARN_SHIELD; break;
                    default:
                        continue;
                    }
                    num_options = sub_4BE571_AddItemToSet(v33, options, num_options, 5);
                }
            }
        }
    } break;

    case BuildingType_MagicShop:
        num_options = 2;
        options[0] = DIALOGUE_LEARN_ITEM_ID;
        options[1] = DIALOGUE_LEARN_REPAIR;
        break;
    case BuildingType_AlchemistShop:
        num_options = 2;
        options[0] = DIALOGUE_LEARN_ALCHEMY;
        options[1] = DIALOGUE_LEARN_MONSTER_ID;
        break;
    case BuildingType_Tavern:
        num_options = 3;
        options[0] = DIALOGUE_LEARN_STEALING;
        options[1] = DIALOGUE_LEARN_TRAP_DISARM;
        options[2] = DIALOGUE_LEARN_PERCEPTION;
        break;
    case BuildingType_Temple:
        num_options = 3;
        options[0] = DIALOGUE_LEARN_UNARMED;
        options[1] = DIALOGUE_LEARN_DODGE;
        options[2] = DIALOGUE_LEARN_MERCHANT;
        break;
    case BuildingType_Training:
        num_options = 2;
        options[0] = DIALOGUE_LEARN_ARMSMASTER;
        options[1] = DIALOGUE_LEARN_BODYBUILDING;
        break;
    default:
        break;
    }

    for (int i = 0; i < num_options; ++i) {
        DIALOGUE_TYPE menu = options[i];
        switch (menu) {
        case DIALOGUE_LEARN_SPEAR:
            v30 = localization->GetSkillName(PLAYER_SKILL_SPEAR);
            break;
        case DIALOGUE_SHOP_REPAIR:
            v30 = localization->GetSkillName(PLAYER_SKILL_REPAIR);
            break;
        case DIALOGUE_LEARN_STAFF:
            v30 = localization->GetSkillName(PLAYER_SKILL_STAFF);
            break;
        case DIALOGUE_LEARN_SWORD:
            v30 = localization->GetSkillName(PLAYER_SKILL_SWORD);
            break;
        case DIALOGUE_LEARN_DAGGER:
            v30 = localization->GetSkillName(PLAYER_SKILL_DAGGER);
            break;
        case DIALOGUE_LEARN_AXE:
            v30 = localization->GetSkillName(PLAYER_SKILL_AXE);
            break;
        case DIALOGUE_LEARN_BOW:
            v30 = localization->GetSkillName(PLAYER_SKILL_BOW);
            break;
        case DIALOGUE_LEARN_MACE:
            v30 = localization->GetSkillName(PLAYER_SKILL_MACE);
            break;
        case DIALOGUE_LEARN_SHIELD:
            v30 = localization->GetSkillName(PLAYER_SKILL_SHIELD);
            break;
        case DIALOGUE_LEARN_LEATHER:
            v30 = localization->GetSkillName(PLAYER_SKILL_LEATHER);
            break;
        case DIALOGUE_LEARN_CHAIN:
            v30 = localization->GetSkillName(PLAYER_SKILL_CHAIN);
            break;
        case DIALOGUE_LEARN_PLATE:
            v30 = localization->GetSkillName(PLAYER_SKILL_PLATE);
            break;
        case DIALOGUE_LEARN_DODGE:
            v30 = localization->GetSkillName(PLAYER_SKILL_DODGE);
            break;
        case DIALOGUE_LEARN_ITEM_ID:
            v30 = localization->GetSkillName(PLAYER_SKILL_ITEM_ID);
            break;
        case DIALOGUE_LEARN_MERCHANT:
            v30 = localization->GetSkillName(PLAYER_SKILL_MERCHANT);
            break;
        case DIALOGUE_LEARN_BODYBUILDING:
            v30 = localization->GetSkillName(PLAYER_SKILL_BODYBUILDING);
            break;
        case DIALOGUE_LEARN_PERCEPTION:
            v30 = localization->GetSkillName(PLAYER_SKILL_PERCEPTION);
            break;
        case DIALOGUE_LEARN_TRAP_DISARM:
            v30 = localization->GetSkillName(PLAYER_SKILL_TRAP_DISARM);
            break;
        case DIALOGUE_LEARN_UNARMED:
            v30 = localization->GetSkillName(PLAYER_SKILL_UNARMED);
            break;
        case DIALOGUE_LEARN_MONSTER_ID:
            v30 = localization->GetSkillName(PLAYER_SKILL_MONSTER_ID);
            break;
        case DIALOGUE_LEARN_ARMSMASTER:
            v30 = localization->GetSkillName(PLAYER_SKILL_ARMSMASTER);
            break;
        case DIALOGUE_LEARN_STEALING:
            v30 = localization->GetSkillName(PLAYER_SKILL_STEALING);
            break;
        case DIALOGUE_LEARN_ALCHEMY:
            v30 = localization->GetSkillName(PLAYER_SKILL_ALCHEMY);
            break;
        case DIALOGUE_LEARN_CLUB:
            v30 = localization->GetSkillName(PLAYER_SKILL_CLUB);
            break;
        default:
            v30 = localization->GetString(LSTR_NO_TEXT);
        }
        pShopOptions[_F8B1DC_currentShopOption] = v30;
        ++_F8B1DC_currentShopOption;
        CreateShopDialogueButtonAtRow(i + 1, menu);
    }
    pDialogueWindow->_41D08F_set_keyboard_control_group(num_options, 1, 0, 2);
    dword_F8B1E0 = pDialogueWindow->pNumPresenceButton;
}

int HouseDialogPressCloseBtn() {
    pCurrentFrameMessageQueue->Flush();
    keyboardInputHandler->SetWindowInputStatus(WINDOW_INPUT_CANCELLED);
    keyboardInputHandler->ResetKeys();
    activeLevelDecoration = nullptr;
    current_npc_text.clear();
    if (pDialogueNPCCount == 0) return 0;

    if (dialog_menu_id == DIALOGUE_SHOP_BUY_SPECIAL &&
        shop_ui_background) {
        shop_ui_background->Release();
        shop_ui_background = nullptr;
    }

    switch (dialog_menu_id) {
    case -1:
        _4B4224_UpdateNPCTopics(pDialogueNPCCount - 1);
        BackToHouseMenu();
        break;

    case DIALOGUE_SHOP_DISPLAY_EQUIPMENT:
    case DIALOGUE_LEARN_SKILLS:
    case DIALOGUE_TAVERN_ARCOMAGE_MAIN:
        BackToHouseMenu();
        UI_CreateEndConversationButton();
        dialog_menu_id = DIALOGUE_MAIN;
        InitializaDialogueOptions(in_current_building_type);
        break;

    case DIALOGUE_SHOP_SELL:
    case DIALOGUE_SHOP_IDENTIFY:
    case DIALOGUE_SHOP_REPAIR:
        UI_CreateEndConversationButton();
        dialog_menu_id = DIALOGUE_SHOP_DISPLAY_EQUIPMENT;
        InitializaDialogueOptions_Shops(in_current_building_type);
        break;

    case DIALOGUE_TAVERN_ARCOMAGE_RULES:
    case DIALOGUE_TAVERN_ARCOMAGE_VICTORY_CONDITIONS:
    case DIALOGUE_TAVERN_ARCOMAGE_RESULT:
        BackToHouseMenu();
        UI_CreateEndConversationButton();
        dialog_menu_id = DIALOGUE_TAVERN_ARCOMAGE_MAIN;
        InitializaDialogueOptions_Tavern(in_current_building_type);
        break;

    case DIALOGUE_NULL:
    case DIALOGUE_MAIN:
        pDialogueNPCCount = 0;
        pDialogueWindow->Release();
        dialog_menu_id = DIALOGUE_NULL;
        pDialogueWindow = 0;

        if (uNumDialogueNPCPortraits == 1) return 0;

        pBtn_ExitCancel = window_SpeakInHouse->vButtons.front();
        if (uNumDialogueNPCPortraits > 0) {
            for (uint i = 0; i < (unsigned int)uNumDialogueNPCPortraits; ++i) {
                HouseNPCPortraitsButtonsList[i] = window_SpeakInHouse->CreateButton(
                    {pNPCPortraits_x[uNumDialogueNPCPortraits - 1][i], pNPCPortraits_y[uNumDialogueNPCPortraits - 1][i]}, {63, 73}, 1, 0,
                    UIMSG_ClickHouseNPCPortrait, i, InputAction::Invalid, portraitClickLabel[i]);
            }
        }

        BackToHouseMenu();
        break;

    default:
        BackToHouseMenu();
        dialog_menu_id = DIALOGUE_MAIN;
        InitializaDialogueOptions(in_current_building_type);
        break;
    }
    return 1;
}

void createHouseUI(HOUSE_ID houseId) {
    switch (in_current_building_type) {
      case BuildingType_FireGuild:
      case BuildingType_AirGuild:
      case BuildingType_WaterGuild:
      case BuildingType_EarthGuild:
      case BuildingType_SpiritGuild:
      case BuildingType_MindGuild:
      case BuildingType_BodyGuild:
      case BuildingType_LightGuild:
      case BuildingType_DarkGuild:
      case BuildingType_MirroredPath:
        window_SpeakInHouse = new GUIWindow_MagicGuild(houseId);
        break;
      case BuildingType_Bank:
        window_SpeakInHouse = new GUIWindow_Bank(houseId);
        break;
      case BuildingType_Temple:
        window_SpeakInHouse = new GUIWindow_Temple(houseId);
        break;
      case BuildingType_Tavern:
        window_SpeakInHouse = new GUIWindow_Tavern(houseId);
        break;
      case BuildingType_Training:
        window_SpeakInHouse = new GUIWindow_Training(houseId);
        break;
      case BuildingType_Stables:
      case BuildingType_Boats:
        window_SpeakInHouse = new GUIWindow_Transport(houseId);
        break;
      case BuildingType_TownHall:
        window_SpeakInHouse = new GUIWindow_TownHall(houseId);
        break;
      case BuildingType_Jail:
        window_SpeakInHouse = new GUIWindow_Jail(houseId);
        break;
      case BuildingType_MercenaryGuild:
        window_SpeakInHouse = new GUIWindow_MercenaryGuild(houseId);
        break;
      case BuildingType_WeaponShop:
        window_SpeakInHouse = new GUIWindow_WeaponShop(houseId);
        break;
      case BuildingType_ArmorShop:
        window_SpeakInHouse = new GUIWindow_ArmorShop(houseId);
        break;
      case BuildingType_MagicShop:
        window_SpeakInHouse = new GUIWindow_MagicShop(houseId);
        break;
      case BuildingType_AlchemistShop:
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
}

void BackToHouseMenu() {
    auto pMouse = EngineIocContainer::ResolveMouse();
    pMouse->ClearPickedItem();
    // TODO(Nik-RE-dev): Looks like it's artifact of MM6
#if 0
    if (window_SpeakInHouse && window_SpeakInHouse->wData.val == 165 &&
        !pMovie_Track) {
        bGameoverLoop = true;
        HouseDialogPressCloseBtn();
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
        if (in_current_building_type == BuildingType_Jail) {
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
            DrawText(pFontArrus, {13, 354 - v6}, Color(), pFontArrus->FitTextInAWindow(current_npc_text, pDialogWindow.uFrameWidth, 13), 0, 0, Color());
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

GUIWindow_House::GUIWindow_House(HOUSE_ID houseId) : GUIWindow(WINDOW_HouseInterior, {0, 0}, render->GetRenderDimensions(), houseId) {
    pEventTimer->Pause();  // pause timer so not attacked

    current_screen_type = CURRENT_SCREEN::SCREEN_HOUSE;
    pBtn_ExitCancel = CreateButton({471, 445}, {169, 35}, 1, 0, UIMSG_Escape, 0, InputAction::Invalid,
                                   localization->GetString(LSTR_EXIT_BUILDING), {ui_exit_cancel_button_background});
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
    if (uNumDialogueNPCPortraits == 1) {
        window_SpeakInHouse = this;
        _4B4224_UpdateNPCTopics(0);
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
