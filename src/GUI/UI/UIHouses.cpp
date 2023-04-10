#include "UIHouses.h"

#include <cstdlib>
#include <thread>

#include "Application/GameOver.h"

#include "Arcomage/Arcomage.h"

#include "Engine/AssetsManager.h"
#include "Engine/Awards.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Events.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/MapInfo.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/Objects/Monsters.h"
#include "Engine/Party.h"
#include "Engine/SaveLoad.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/stru159.h"

#include "GUI/GUIBountyHunting.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIDialogue.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIGuilds.h"
#include "GUI/UI/UIPartyCreation.h"
#include "GUI/UI/UIShops.h"
#include "GUI/UI/UIStatusBar.h"

#include "Io/Mouse.h"
#include "Io/KeyboardInputHandler.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

#include "Utility/String.h"
#include "Library/Random/Random.h"
#include "Utility/Math/TrigLut.h"

using namespace std::chrono_literals; // NOLINT

using Io::TextInputType;

int uHouse_ExitPic;
int _F8B1DC_currentShopOption;  // F8B1DC
int dword_591080;               // 591080

BuildingType in_current_building_type;  // 00F8B198
DIALOGUE_TYPE dialog_menu_id;     // 00F8B19C

Image *_591428_endcap = nullptr;

void GenerateStandartShopItems();
void GenerateSpecialShopItems();

#pragma pack(push, 1)
struct stru365_travel_info {
    unsigned char uMapInfoID;
    unsigned char pSchedule[7];
    unsigned int uTravelTime; // In days.
    int arrival_x;
    int arrival_y;
    int arrival_z;
    int arrival_view_yaw;
    unsigned int uQuestBit;  // quest bit required to set for this travel option
                             // to be enabled; otherwise 0
};
#pragma pack(pop)
stru365_travel_info transport_schedule[35] = {  // 004F09B0
                                                // location name   days    x        y       z
    { MAP_STEADWICK, { 1, 0, 1, 0, 1, 0, 0 }, 2, -18048, 4636, 833, 1536, 0 },  // for stable
    { MAP_PIERPONT, { 0, 1, 0, 1, 0, 1, 0 }, 2, -2527, -6773, 1153, 896, 0 },
    { MAP_TATALIA, { 1, 0, 1, 0, 1, 0, 0 }, 2, 4730, -10580, 320, 1024, 0 },
    { MAP_HARMONDALE, { 0, 1, 0, 1, 0, 1, 0 }, 2, -5692, 11137, 1, 1024, 0 },
    { MAP_DEYJA, { 1, 0, 0, 1, 0, 0, 0 }, 3, 7227, -16007, 2625, 640, 0 },
    { MAP_BRAKADA_DESERT, { 0, 0, 1, 0, 0, 1, 0 }, 3, 8923, 17191, 1, 512, 0 },
    { MAP_AVLEE, { 1, 0, 1, 0, 1, 0, 0 }, 3, 17059, 12331, 512, 1152, 0 },
    { MAP_DEYJA, { 0, 1, 0, 0, 1, 0, 1 }, 2, 7227, -16007, 2625, 640, 0 },
    { MAP_HARMONDALE, { 0, 1, 0, 1, 0, 1, 0 }, 2, -5692, 11137, 1, 1024, 0 },
    { MAP_STEADWICK, { 1, 0, 1, 0, 1, 0, 0 }, 3, -18048, 4636, 833, 1536, 0 },
    { MAP_PIERPONT, { 0, 1, 0, 1, 0, 1, 0 }, 2, -2527, -6773, 1153, 896, 0 },
    { MAP_STEADWICK, { 1, 0, 1, 0, 1, 0, 1 }, 3, -18048, 4636, 833, 1536, 0 },
    { MAP_HARMONDALE, { 0, 1, 0, 0, 0, 1, 0 }, 5, -5692, 11137, 1, 1024, 0 },
    { MAP_STEADWICK, { 0, 1, 0, 1, 0, 1, 0 }, 2, -18048, 4636, 833, 1536, 0 },
    { MAP_PIERPONT, { 0, 1, 0, 1, 0, 1, 0 }, 3, -2527, -6773, 1153, 896, 0 },
    { MAP_DEYJA, { 0, 0, 1, 0, 0, 0, 1 }, 5, 7227, -16007, 2625, 640, 0 },
    { MAP_TATALIA, { 0, 1, 0, 1, 0, 1, 0 }, 2, -2183, -6941, 97, 0, 0 },
    { MAP_AVLEE, { 1, 0, 0, 0, 1, 0, 0 }, 4, 7913, 9476, 193, 0, 0 },
    { MAP_EVENMORN_ISLE, { 0, 0, 0, 0, 0, 0, 1 }, 7, 15616, 6390, 193, 1536, QBIT_EVENMORN_MAP_FOUND },
    { MAP_BRAKADA_DESERT, { 0, 0, 1, 0, 0, 0, 0 }, 6, 19171, -19722, 193, 1024, 0 },
    { MAP_AVLEE, { 0, 1, 0, 1, 0, 1, 0 }, 3, 7913, 9476, 193, 0, 0 },
    { MAP_BRAKADA_DESERT, { 1, 0, 1, 0, 0, 0, 0 }, 6, 19171, -19722, 193, 1024, 0 },
    { MAP_TATALIA, { 1, 0, 1, 0, 1, 0, 0 }, 4, -2183, -6941, 97, 0, 0 },
    { MAP_PIERPONT, { 0, 0, 0, 0, 0, 1, 0 }, 6, -709, -14087, 193, 1024, 0 },  // for boat
    { MAP_STEADWICK, { 0, 0, 0, 0, 0, 0, 1 }, 6, -10471, 13497, 193, 1536, 0 },
    { MAP_EVENMORN_ISLE, { 0, 1, 0, 1, 0, 0, 0 }, 1, 15616, 6390, 193, 1536, QBIT_EVENMORN_MAP_FOUND },
    { MAP_BRAKADA_DESERT, { 0, 1, 0, 1, 0, 0, 0 }, 1, 19171, -19722, 193, 1024, 0 },
    { MAP_STEADWICK, { 0, 1, 0, 1, 0, 1, 0 }, 2, -10471, 13497, 193, 1536, 0 },
    { MAP_BRAKADA_DESERT, { 1, 0, 1, 0, 0, 0, 0 }, 4, 19171, -19722, 193, 1024, 0 },
    { MAP_EVENMORN_ISLE, { 0, 0, 0, 0, 0, 0, 1 }, 5, 15616, 6390, 193, 1536, QBIT_EVENMORN_MAP_FOUND },
    { MAP_AVLEE, { 0, 0, 0, 0, 1, 0, 0 }, 5, 7913, 9476, 193, 0, 0 },
    { MAP_STEADWICK, { 0, 1, 0, 0, 0, 1, 0 }, 4, -10471, 13497, 193, 1536, 0 },
    { MAP_PIERPONT, { 1, 0, 1, 0, 1, 0, 0 }, 3, -709, -14087, 193, 1024, 0 },
    { MAP_TATALIA, { 0, 0, 0, 1, 0, 0, 0 }, 5, -2183, -6941, 97, 0, 0 },
    { MAP_ARENA, { 0, 0, 0, 0, 0, 0, 1 }, 4, 3844, 2906, 193, 512, 0 }
};

unsigned char transport_routes[20][4] = {
    { 0, 1, 1, 34 },         // HOUSE_STABLES_HARMONDALE
    { 2, 3, 4, 5 },          // HOUSE_STABLES_STEADWICK
    { 6, 7, 8, 8 },          // HOUSE_STABLES_TULAREAN_FOREST
    { 9, 10, 10, 10 },       // HOUSE_STABLES_DEYJA
    { 11, 11, 12, 12 },      // HOUSE_STABLES_BRACADA_DESERT
    { 13, 13, 13, 13 },      // HOUSE_STABLES_TATALIA
    { 14, 14, 15, 15 },      // HOUSE_STABLES_AVLEE
    { 255, 255, 255, 255 },  // HOUSE_STABLES_61
    { 255, 255, 255, 255 },  // HOUSE_STABLES_62
    { 255, 255, 255, 255 },  // HOUSE_BOATS_EMERALD_ISLE
    { 16, 17, 18, 19 },      // HOUSE_BOATS_ERATHIA
    { 18, 20, 21, 21 },      // HOUSE_BOATS_TULAREAN_FOREST
    { 22, 23, 24, 25 },      // HOUSE_BOATS_BRACADA_DESERT
    { 22, 22, 23, 23 },      // HOUSE_BOATS_EVENMORN_ISLAND
    { 255, 255, 255, 255 },  // HOUSE_BOATS_68
    { 27, 28, 29, 30 },      // HOUSE_BOATS_TATALIA
    { 31, 32, 33, 33 },      // HOUSE_BOATS_AVLEE
    { 24, 24, 24, 24 },      // HOUSE_BOATS_71
    { 255, 255, 255, 255 },  // HOUSE_BOATS_72
    { 255, 255, 255, 255 }   // HOUSE_BOATS_73
};

std::array<const stru159, 196> pAnimatedRooms = { {  // 0x4E5F70
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

struct ITEM_VARIATION {
    ITEM_TREASURE_LEVEL treasure_level;
    uint16_t item_class[4];
};

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
    default:
        return 0;
    }
}

const std::array<ITEM_VARIATION, 15> shopWeap_variation_ord = {{
    { ITEM_TREASURE_LEVEL_INVALID, { 00, 00, 00, 00 } },
    { ITEM_TREASURE_LEVEL_1, { 23, 27, 20, 20 } },
    { ITEM_TREASURE_LEVEL_1, { 23, 24, 28, 20 } },
    { ITEM_TREASURE_LEVEL_2, { 23, 24, 25, 20 } },
    { ITEM_TREASURE_LEVEL_2, { 27, 27, 26, 26 } },
    { ITEM_TREASURE_LEVEL_4, { 24, 30, 25, 27 } },
    { ITEM_TREASURE_LEVEL_4, { 24, 30, 25, 27 } },
    { ITEM_TREASURE_LEVEL_3, { 30, 24, 20, 20 } },
    { ITEM_TREASURE_LEVEL_2, { 20, 20, 20, 20 } },
    { ITEM_TREASURE_LEVEL_3, { 27, 27, 26, 26 } },
    { ITEM_TREASURE_LEVEL_3, { 28, 28, 25, 25 } },
    { ITEM_TREASURE_LEVEL_2, { 23, 23, 24, 24 } },
    { ITEM_TREASURE_LEVEL_3, { 23, 23, 26, 26 } },
    { ITEM_TREASURE_LEVEL_2, { 30, 26, 26, 26 } },
    { ITEM_TREASURE_LEVEL_2, { 28, 25, 28, 29 } }
}};

const std::array<ITEM_VARIATION, 28> shopArmr_variation_ord = {{
    { ITEM_TREASURE_LEVEL_1, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_1, { 31, 31, 31, 34 } },
    { ITEM_TREASURE_LEVEL_1, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_1, { 31, 31, 32, 34 } },
    { ITEM_TREASURE_LEVEL_2, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_2, { 31, 32, 32, 33 } },
    { ITEM_TREASURE_LEVEL_2, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_2, { 31, 31, 32, 32 } },
    { ITEM_TREASURE_LEVEL_4, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_4, { 31, 32, 33, 34 } },
    { ITEM_TREASURE_LEVEL_4, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_4, { 31, 32, 33, 34 } },
    { ITEM_TREASURE_LEVEL_3, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_3, { 31, 31, 31, 31 } },
    { ITEM_TREASURE_LEVEL_2, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_2, { 31, 32, 34, 34 } },
    { ITEM_TREASURE_LEVEL_3, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_3, { 31, 31, 32, 32 } },
    { ITEM_TREASURE_LEVEL_3, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_3, { 32, 32, 32, 33 } },
    { ITEM_TREASURE_LEVEL_3, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_3, { 31, 31, 31, 32 } },
    { ITEM_TREASURE_LEVEL_3, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_3, { 33, 31, 32, 34 } },
    { ITEM_TREASURE_LEVEL_3, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_3, { 33, 31, 32, 34 } },
    { ITEM_TREASURE_LEVEL_4, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_4, { 33, 31, 32, 34 } }
}};

const ITEM_TREASURE_LEVEL shopMagic_treasure_lvl[14] = {
    ITEM_TREASURE_LEVEL_INVALID,
    ITEM_TREASURE_LEVEL_1,
    ITEM_TREASURE_LEVEL_1,
    ITEM_TREASURE_LEVEL_2,
    ITEM_TREASURE_LEVEL_2,
    ITEM_TREASURE_LEVEL_4,
    ITEM_TREASURE_LEVEL_4,
    ITEM_TREASURE_LEVEL_3,
    ITEM_TREASURE_LEVEL_2,
    ITEM_TREASURE_LEVEL_2,
    ITEM_TREASURE_LEVEL_2,
    ITEM_TREASURE_LEVEL_2,
    ITEM_TREASURE_LEVEL_2,
    ITEM_TREASURE_LEVEL_2
};

const ITEM_TREASURE_LEVEL shopAlch_treasure_lvl[13] = {
    ITEM_TREASURE_LEVEL_INVALID,
    ITEM_TREASURE_LEVEL_1,
    ITEM_TREASURE_LEVEL_1,
    ITEM_TREASURE_LEVEL_2,
    ITEM_TREASURE_LEVEL_2,
    ITEM_TREASURE_LEVEL_3,
    ITEM_TREASURE_LEVEL_3,
    ITEM_TREASURE_LEVEL_4,
    ITEM_TREASURE_LEVEL_4,
    ITEM_TREASURE_LEVEL_2,
    ITEM_TREASURE_LEVEL_2,
    ITEM_TREASURE_LEVEL_2,
    ITEM_TREASURE_LEVEL_2
};

const std::array<ITEM_VARIATION, 15> shopWeap_variation_spc = {{
    { ITEM_TREASURE_LEVEL_INVALID, { 00, 00, 00, 00 } },
    { ITEM_TREASURE_LEVEL_2, { 25, 30, 20, 20 } },
    { ITEM_TREASURE_LEVEL_2, { 23, 24, 28, 20 } },
    { ITEM_TREASURE_LEVEL_3, { 23, 24, 25, 20 } },
    { ITEM_TREASURE_LEVEL_3, { 27, 27, 26, 26 } },
    { ITEM_TREASURE_LEVEL_5, { 23, 26, 28, 27 } },
    { ITEM_TREASURE_LEVEL_5, { 23, 26, 28, 27 } },
    { ITEM_TREASURE_LEVEL_4, { 30, 24, 20, 20 } },
    { ITEM_TREASURE_LEVEL_3, { 20, 20, 20, 20 } },
    { ITEM_TREASURE_LEVEL_4, { 27, 27, 26, 26 } },
    { ITEM_TREASURE_LEVEL_4, { 28, 28, 25, 25 } },
    { ITEM_TREASURE_LEVEL_4, { 23, 23, 24, 24 } },
    { ITEM_TREASURE_LEVEL_4, { 24, 24, 27, 20 } },
    { ITEM_TREASURE_LEVEL_4, { 30, 26, 26, 26 } },
    { ITEM_TREASURE_LEVEL_4, { 28, 25, 28, 29 } }
}};

const ITEM_TREASURE_LEVEL shopMagicSpc_treasure_lvl[14] = {
    ITEM_TREASURE_LEVEL_INVALID,
    ITEM_TREASURE_LEVEL_2,
    ITEM_TREASURE_LEVEL_2,
    ITEM_TREASURE_LEVEL_3,
    ITEM_TREASURE_LEVEL_3,
    ITEM_TREASURE_LEVEL_5,
    ITEM_TREASURE_LEVEL_5,
    ITEM_TREASURE_LEVEL_4,
    ITEM_TREASURE_LEVEL_3,
    ITEM_TREASURE_LEVEL_3,
    ITEM_TREASURE_LEVEL_3,
    ITEM_TREASURE_LEVEL_3,
    ITEM_TREASURE_LEVEL_3,
    ITEM_TREASURE_LEVEL_3
};

const ITEM_TREASURE_LEVEL shopAlchSpc_treasure_lvl[13] = {
    ITEM_TREASURE_LEVEL_INVALID,
    ITEM_TREASURE_LEVEL_2,
    ITEM_TREASURE_LEVEL_2,
    ITEM_TREASURE_LEVEL_3,
    ITEM_TREASURE_LEVEL_3,
    ITEM_TREASURE_LEVEL_4,
    ITEM_TREASURE_LEVEL_4,
    ITEM_TREASURE_LEVEL_5,
    ITEM_TREASURE_LEVEL_5,
    ITEM_TREASURE_LEVEL_3,
    ITEM_TREASURE_LEVEL_2,
    ITEM_TREASURE_LEVEL_2,
    ITEM_TREASURE_LEVEL_2
};

const std::array<ITEM_VARIATION, 28> shopArmr_variation_spc = {{
    { ITEM_TREASURE_LEVEL_2, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_2, { 31, 31, 31, 34 } },
    { ITEM_TREASURE_LEVEL_2, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_2, { 31, 31, 32, 34 } },
    { ITEM_TREASURE_LEVEL_3, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_3, { 31, 32, 32, 33 } },
    { ITEM_TREASURE_LEVEL_3, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_3, { 31, 31, 32, 32 } },
    { ITEM_TREASURE_LEVEL_5, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_5, { 31, 32, 33, 34 } },
    { ITEM_TREASURE_LEVEL_5, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_5, { 31, 32, 33, 34 } },
    { ITEM_TREASURE_LEVEL_4, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_4, { 31, 31, 31, 31 } },
    { ITEM_TREASURE_LEVEL_3, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_3, { 31, 32, 34, 34 } },
    { ITEM_TREASURE_LEVEL_4, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_4, { 31, 31, 32, 33 } },
    { ITEM_TREASURE_LEVEL_4, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_4, { 32, 32, 33, 34 } },
    { ITEM_TREASURE_LEVEL_4, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_4, { 31, 31, 31, 32 } },
    { ITEM_TREASURE_LEVEL_4, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_4, { 32, 32, 32, 32 } },
    { ITEM_TREASURE_LEVEL_4, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_4, { 34, 34, 34, 34 } },
    { ITEM_TREASURE_LEVEL_5, { 35, 35, 38, 38 } },
    { ITEM_TREASURE_LEVEL_5, { 33, 33, 33, 33 } }
}};

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
        CreateShopDialogueButtonAtRow(0, DIALOGUE_TOWNHALL_MESSAGE);

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

    if (pPlayers[pParty->getActiveCharacter()]->CanAct()) {
        pDialogueWindow->pNumPresenceButton = dword_F8B1E0;
        return true;
    } else {
        pDialogueWindow->pNumPresenceButton = 0;
        GUIWindow window = *pPrimaryWindow;
        window.uFrameX = 483;
        window.uFrameWidth = 143;
        window.uFrameZ = 334;

        std::string str = localization->FormatString(
            LSTR_FMT_S_IS_IN_NO_CODITION_TO_S,
            pPlayers[pParty->getActiveCharacter()]->pName.c_str(),
            localization->GetString(LSTR_DO_ANYTHING));
        window.DrawTitleText(
            pFontArrus, 0,
            (212 - pFontArrus->CalcTextHeight(str, window.uFrameWidth, 0)) / 2 +
            101,
            ui_house_player_cant_interact_color, str, 3);
        return false;
    }
}

//----- (0044622E) --------------------------------------------------------
bool EnterHouse(HOUSE_ID uHouseID) {
    GameUI_StatusBar_Clear();
    GameUI_SetStatusBar("");
    pCurrentFrameMessageQueue->Flush();
    uDialogueType = DIALOGUE_NULL;
    keyboardInputHandler->SetWindowInputStatus(WindowInputStatus::WINDOW_INPUT_CANCELLED);
    keyboardInputHandler->ResetKeys();

    if (uHouseID == HOUSE_THRONEROOM_WIN_GOOD || uHouseID == HOUSE_THRONEROOM_WIN_EVIL) {
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_ShowGameOverWindow, 0, 0);
        return 0;
    }

    int uOpenTime = p2DEvents[uHouseID - HOUSE_SMITH_EMERALD_ISLE].uOpenTime;
    int uCloseTime = p2DEvents[uHouseID - HOUSE_SMITH_EMERALD_ISLE].uCloseTime;
    current_npc_text.clear();
    dword_F8B1E4 = 0;
    memset(player_levels.data(), 0, 16);
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
            pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_StoreClosed);
        }

        return 0;
    } else {
        if (uHouseID < 53) {  // entering shops and guilds
            if (!(pParty->PartyTimes._shop_ban_times[uHouseID]) ||
                (pParty->PartyTimes._shop_ban_times[uHouseID] <=
                    pParty->GetPlayingTime())) {
                pParty->PartyTimes._shop_ban_times[uHouseID] = GameTime(0);
            } else {
                GameUI_SetStatusBar(LSTR_BANNED_FROM_SHOP);
                return 0;
            }
        }
        // pAudioPlayer->PauseSounds(-1);

        uCurrentHouse_Animation = p2DEvents[uHouseID - HOUSE_SMITH_EMERALD_ISLE].uAnimationID;
        in_current_building_type = pAnimatedRooms[uCurrentHouse_Animation].uBuildingType;
        if (in_current_building_type == BuildingType_Throne_Room && pParty->uFine) {  // going to jail
            uCurrentHouse_Animation = (int16_t)p2DEvents[186].uAnimationID;
            uHouseID = HOUSE_JAIL;
            pParty->GetPlayingTime().AddYears(1);  // += 123863040;
            in_current_building_type = pAnimatedRooms[p2DEvents[HOUSE_LORD_AND_JUDGE_EMERALD_ISLE].uAnimationID].uBuildingType;
            ++pParty->uNumPrisonTerms;
            pParty->uFine = 0;
            for (uint i = 0; i < 4; ++i) {
                pParty->pPlayers[i].uTimeToRecovery = 0;
                pParty->pPlayers[i].uNumDivineInterventionCastsThisDay = 0;
                pParty->pPlayers[i].SetVariable(VAR_Award, Award_PrisonTerms);
            }
        }

        std::string pContainer = DialogueBackgroundResourceByAlignment[pParty->alignment];

        pDialogueNPCCount = 0;
        game_ui_dialogue_background = assets->GetImage_Solid(pContainer);

        PrepareHouse(uHouseID);

        if (uNumDialogueNPCPortraits == 1) pDialogueNPCCount = 1;
        pMediaPlayer->OpenHouseMovie(pAnimatedRooms[uCurrentHouse_Animation].video_name, 1u);
        dword_5C35D4 = 1;
        if ((signed int)uHouseID < HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE || (signed int)uHouseID > HOUSE_SELF_GUILD_2) {
            if ((signed int)uHouseID >= HOUSE_STABLES_HARMONDALE && (signed int)uHouseID <= HOUSE_BOATS_PLACEHOLDER_2 && !IsTravelAvailable(uHouseID - HOUSE_STABLES_HARMONDALE)) {
                return 1;
            }
        } else {  // guilds
            int membership = guild_membership_flags[uHouseID - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE];

            // TODO(pskelton): check this behaviour
            if (!pParty->hasActiveCharacter())  // avoid nzi
                pParty->setActiveToFirstCanAct();

            if (!_449B57_test_bit(pPlayers[pParty->getActiveCharacter()]->_achieved_awards_bits, membership)) {
                PlayHouseSound(uHouseID, HouseSound_Greeting_2);
                return 1;
            }
        }
        PlayHouseSound(uHouseID, HouseSound_Greeting);
        dword_5C35D4 = 1;
        return 1;
    }
}

//----- (0044606A) --------------------------------------------------------
void PrepareHouse(HOUSE_ID house) {
    int16_t uExitMapID;  // ax@2
                         //  int v7; // ebx@11
                         //  int v13; // [sp+30h] [bp-30h]@11
    int npc_id_arr[6];   // [sp+34h] [bp-2Ch]@1
    int uAnimationID;    // [sp+50h] [bp-10h]@1

    uAnimationID = p2DEvents[house - HOUSE_SMITH_EMERALD_ISLE].uAnimationID;
    memset(npc_id_arr, 0, sizeof(npc_id_arr));
    uNumDialogueNPCPortraits = 0;
    uHouse_ExitPic = p2DEvents[house - HOUSE_SMITH_EMERALD_ISLE].uExitPicID;
    if (uHouse_ExitPic) {
        uExitMapID = p2DEvents[house - HOUSE_SMITH_EMERALD_ISLE]._quest_bit;
        if (uExitMapID > 0) {
            if (_449B57_test_bit(pParty->_quest_bits, uExitMapID)) {
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
        pDialogueNPCPortraits[i] = assets->GetImage_ColorKey(
            fmt::format("npc{:03}", npc_id_arr[i]));
    }

    if (uHouse_ExitPic) {
        pDialogueNPCPortraits[uNumDialogueNPCPortraits] =
            assets->GetImage_ColorKey(pHouse_ExitPictures[uHouse_ExitPic]);
        ++uNumDialogueNPCPortraits;
        uHouse_ExitPic = p2DEvents[house - HOUSE_SMITH_EMERALD_ISLE].uExitMapID;
    }
}

//----- (004B1E92) --------------------------------------------------------
void PlayHouseSound(unsigned int uHouseID, HouseSoundID sound) {
    if (uHouseID > 0) {
        if (pAnimatedRooms[p2DEvents[uHouseID - HOUSE_SMITH_EMERALD_ISLE].uAnimationID].uRoomSoundId) {
            int roomSoundId = pAnimatedRooms[p2DEvents[uHouseID - HOUSE_SMITH_EMERALD_ISLE].uAnimationID].uRoomSoundId;
            // PID 806 was used which is PID(OBJECT_Face, 100)
            pAudioPlayer->playUISound((SoundID)(sound + 100 * (roomSoundId + 300)));
        }
    }
}

//----- (004BCACC) --------------------------------------------------------
void OnSelectShopDialogueOption(DIALOGUE_TYPE option) {
    int experience_for_next_level;  // eax@5
    signed int v36;                 // esi@227
    int pPrice;                     // ecx@227

    if (!pDialogueWindow->pNumPresenceButton) return;
    render->ClearZBuffer();

    if (dialog_menu_id == DIALOGUE_MAIN) {
        Sizei renDims = render->GetRenderDimensions();
        if (in_current_building_type == BuildingType_Training) {
            if (option == DIALOGUE_TRAINING_HALL_TRAIN) {
                experience_for_next_level = 0;
                if (pPlayers[pParty->getActiveCharacter()]->uLevel > 0) {
                    for (uint i = 0; i < pPlayers[pParty->getActiveCharacter()]->uLevel;
                        i++)
                        experience_for_next_level += i + 1;
                }
                if (pPlayers[pParty->getActiveCharacter()]->uLevel <
                    pMaxLevelPerTrainingHallType
                    [window_SpeakInHouse->wData.val - 89] &&
                    (int64_t)pPlayers[pParty->getActiveCharacter()]->uExperience <
                    1000 * experience_for_next_level)  // test experience
                    return;
            }
            pDialogueWindow->Release();
            pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, {renDims.w, 345}, 0);
            pBtn_ExitCancel = pDialogueWindow->CreateButton({526, 445}, {75, 33}, 1, 0,
                UIMSG_Escape, 0, InputAction::Invalid, localization->GetString(LSTR_END_CONVERSATION), {ui_buttdesc2});
            pDialogueWindow->CreateButton({8, 8}, {450, 320}, 1, 0, UIMSG_BuyInShop_Identify_Repair, 0);
        }
        if (in_current_building_type != BuildingType_Training) {
            if ((in_current_building_type == BuildingType_Stables ||
                in_current_building_type == BuildingType_Boats) &&
                transport_schedule
                [transport_routes[window_SpeakInHouse->wData.val - HOUSE_STABLES_HARMONDALE]
                [option - DIALOGUE_TRANSPORT_SCHEDULE_1]]
            .pSchedule[pParty->uCurrentDayOfMonth % 7] ||
                in_current_building_type != BuildingType_Temple ||
                in_current_building_type != BuildingType_MindGuild) {
                pDialogueWindow->Release();
                pDialogueWindow = new GUIWindow(WINDOW_Dialogue, {0, 0}, {renDims.w, 345}, 0);
                pBtn_ExitCancel = pDialogueWindow->CreateButton({526, 445}, {75, 33}, 1, 0,
                    UIMSG_Escape, 0, InputAction::Invalid, localization->GetString(LSTR_END_CONVERSATION), {ui_buttdesc2});
                pDialogueWindow->CreateButton({8, 8}, {450, 320}, 1, 0, UIMSG_BuyInShop_Identify_Repair, 0);
            } else if (pParty->hasActiveCharacter()) {
                if (!pPlayers[pParty->getActiveCharacter()]->IsPlayerHealableByTemple())
                    return;
            }
        }
        dialog_menu_id = option;
        if (in_current_building_type < BuildingType_TownHall_MM6) {
            shop_ui_background = assets->GetImage_ColorKey(
                _4F03B8_shop_background_names[(int)in_current_building_type]);
        }
    }

    // NEW
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
    {
        if (pParty->PartyTimes.Shops_next_generation_time[window_SpeakInHouse->wData.val - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE] >= pParty->GetPlayingTime()) {
            for (uint i = 0; i < 12; ++i) {
                if (pParty->SpellBooksInGuilds[window_SpeakInHouse->wData.val - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE][i].uItemID != ITEM_NULL)
                    shop_ui_items_in_store[i] = assets->GetImage_ColorKey(
                        pParty->SpellBooksInGuilds
                        [window_SpeakInHouse->wData.val - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE][i].GetIconName());
            }
        } else {  // generation new books
            SpellBookGenerator();
            pParty->PartyTimes.Shops_next_generation_time[window_SpeakInHouse->wData.val - HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE] = GameTime(pParty->GetPlayingTime() + GameTime::FromDays(
                    p2DEvents[window_SpeakInHouse->wData.val - 1].generation_interval_days));
        }
        break;
    }
    case BuildingType_TownHall:
    {
        if (option == DIALOGUE_TOWNHALL_MESSAGE) {
            bountyHuntingDialogueOptionClicked();
        } else if (option == DIALOGUE_TOWNHALL_PAY_FINE) {
            keyboardInputHandler->StartTextInput(TextInputType::Number, 10, window_SpeakInHouse);
            return;
        }
        break;
    }
    case BuildingType_Bank:
    {
        if (dialog_menu_id >= DIALOGUE_BANK_PUT_GOLD && dialog_menu_id <= DIALOGUE_BANK_GET_GOLD)
            keyboardInputHandler->StartTextInput(TextInputType::Number, 10, window_SpeakInHouse);
        return;
        break;
    }
    case BuildingType_WeaponShop:
    case BuildingType_ArmorShop:
    case BuildingType_MagicShop:
    case BuildingType_AlchemistShop:
    case BuildingType_Tavern:
    case BuildingType_Temple:
    case BuildingType_Training:
    {
        break;
    }
    default:
    {
        return;
        break;
    }
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
    case DIALOGUE_SHOP_BUY_STANDARD:
    case DIALOGUE_SHOP_BUY_SPECIAL:
    {
        if (pParty->PartyTimes.Shops_next_generation_time
            [window_SpeakInHouse->wData.val] <
            pParty->GetPlayingTime()) {
            GenerateStandartShopItems();
            GenerateSpecialShopItems();
            pParty->PartyTimes
                .Shops_next_generation_time[window_SpeakInHouse->wData.val] =
                GameTime(pParty->GetPlayingTime() +
                GameTime::FromDays(
                    p2DEvents[window_SpeakInHouse->wData.val - 1]
                    .generation_interval_days));
        }
        if (option == DIALOGUE_SHOP_BUY_STANDARD) {
            if (ItemAmountForShop(p2DEvents[window_SpeakInHouse->wData.val - HOUSE_SMITH_EMERALD_ISLE].uType)) {
                for (uint i = 0; i < ItemAmountForShop(p2DEvents[window_SpeakInHouse->wData.val - HOUSE_SMITH_EMERALD_ISLE].uType); ++i) {
                    if (pParty
                        ->StandartItemsInShops[window_SpeakInHouse
                        ->wData.val][i].uItemID != ITEM_NULL)
                        shop_ui_items_in_store[i] = assets->GetImage_ColorKey(
                            pParty->StandartItemsInShops[window_SpeakInHouse->wData.val][i].GetIconName());
                }
            }
            if (in_current_building_type == BuildingType_WeaponShop) {
                if (ItemAmountForShop(p2DEvents[window_SpeakInHouse->wData.val - HOUSE_SMITH_EMERALD_ISLE].uType)) {
                    for (uint i = 0; i < ItemAmountForShop(p2DEvents[window_SpeakInHouse->wData.val - HOUSE_SMITH_EMERALD_ISLE].uType); ++i) {
                        if (pParty
                            ->StandartItemsInShops
                            [window_SpeakInHouse->wData.val][i]
                            .uItemID != ITEM_NULL) {
                            // Note that we're using grng here for a reason - we want recorded mouse clicks to work.
                            weapons_Ypos[i] = grng->random(300 - shop_ui_items_in_store[i]->GetHeight());
                        }
                    }
                }
            }
        }
        if (option == DIALOGUE_SHOP_BUY_SPECIAL) {
            if (ItemAmountForShop(p2DEvents[window_SpeakInHouse->wData.val - HOUSE_SMITH_EMERALD_ISLE].uType)) {
                for (uint i = 0; i < ItemAmountForShop(p2DEvents[window_SpeakInHouse->wData.val - HOUSE_SMITH_EMERALD_ISLE].uType); ++i) {
                    if (pParty->SpecialItemsInShops[
                        window_SpeakInHouse->wData.val][i].uItemID != ITEM_NULL)
                        shop_ui_items_in_store[i] = assets->GetImage_ColorKey(
                            pParty->SpecialItemsInShops
                            [window_SpeakInHouse->wData.val][i].GetIconName());
                }
            }
            if (in_current_building_type == BuildingType_WeaponShop) {
                if (ItemAmountForShop(p2DEvents[window_SpeakInHouse->wData.val - HOUSE_SMITH_EMERALD_ISLE].uType)) {
                    for (uint i = 0; i < ItemAmountForShop(p2DEvents[window_SpeakInHouse->wData.val - HOUSE_SMITH_EMERALD_ISLE].uType); ++i) {
                        if (pParty->SpecialItemsInShops[
                            window_SpeakInHouse->wData.val][i].uItemID != ITEM_NULL) {
                            // Note that we're using grng here for a reason - we want recorded mouse clicks to work.
                            weapons_Ypos[i] = grng->random(300 - shop_ui_items_in_store[i]->GetHeight());
                        }
                    }
                }
            }
        }
        break;
    }
    case DIALOGUE_SHOP_SELL:
    case DIALOGUE_SHOP_IDENTIFY:
    case DIALOGUE_SHOP_REPAIR:
    {
        dialog_menu_id = option;
        pParty->PickedItem_PlaceInInventory_or_Drop();
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
            v36 = (int64_t)(p2DEvents[
                    window_SpeakInHouse->wData.val - 1].flt_24 * 500.0);
            if (p2DEvents[window_SpeakInHouse->wData.val - 1].uType >= BuildingType_FireGuild &&
                p2DEvents[window_SpeakInHouse->wData.val - 1].uType <= BuildingType_SelfGuild) {
                // guild prices use the other multipler
                v36 = (int64_t)(p2DEvents[
                    window_SpeakInHouse->wData.val - 1].fPriceMultiplier * 500.0);
            }

            pPrice = v36 * (100 - pPlayers[pParty->getActiveCharacter()]->GetMerchant()) / 100;
            if (pPrice < v36 / 3) pPrice = v36 / 3;
            auto skill = GetLearningDialogueSkill(option);
            if (skillMaxMasteryPerClass[pPlayers[pParty->getActiveCharacter()]->classType][skill] != PLAYER_SKILL_MASTERY_NONE) {
                if (!pPlayers[pParty->getActiveCharacter()]->pActiveSkills[skill]) {
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
                        pPlayers[pParty->getActiveCharacter()]->pActiveSkills[skill] = 1;
                        pPlayers[pParty->getActiveCharacter()]->PlayAwardSound_Anim97_Face(SPEECH_SkillLearned);
                    }
                }
            }
        }
        break;
    }
    }
}

int GetTravelTimeTransportDays(int schedule_id) {
    int travel_time = transport_schedule[schedule_id].uTravelTime;
    if (window_SpeakInHouse->wData.val >= HOUSE_BOATS_EMERALD_ISLE) {
        if (CheckHiredNPCSpeciality(Sailor)) travel_time -= 2;
        if (CheckHiredNPCSpeciality(Navigator)) travel_time -= 3;
        if (CheckHiredNPCSpeciality(Pirate)) travel_time -= 2;
    } else {
        if (CheckHiredNPCSpeciality(Horseman)) travel_time -= 2;
    }
    if (CheckHiredNPCSpeciality(Explorer)) --travel_time;
    if (travel_time < 1) travel_time = 1;
    return travel_time;
}

void TravelByTransport() {
    GUIWindow travel_window = *window_SpeakInHouse;
    travel_window.uFrameX = 483;
    travel_window.uFrameWidth = 143;
    travel_window.uFrameZ = 334;

    assert(pParty->hasActiveCharacter()); // code in this function couldn't handle pParty->getActiveCharacter() = 0 and crash

    int base_price = p2DEvents[window_SpeakInHouse->wData.val - 1].uType == BuildingType_Stables ? 25 : 50;
    base_price *= p2DEvents[window_SpeakInHouse->wData.val - 1].fPriceMultiplier;
    int pPrice = base_price * (100 - pParty->getOptionallySharedSkillStrongestEffect(PLAYER_SKILL_TYPE::PLAYER_SKILL_MERCHANT)) / 100;
    if (pPrice < base_price / 3) pPrice = base_price / 3;
    int route_id = window_SpeakInHouse->wData.val - HOUSE_STABLES_HARMONDALE;

    if (dialog_menu_id == DIALOGUE_MAIN) {
        if (HouseUI_CheckIfPlayerCanInteract()) {
            int index = 0;

            std::string travelcost = localization->FormatString(LSTR_FMT_TRAVEL_COST_D_GOLD, pPrice);
            int pTextHeight = pFontArrus->CalcTextHeight(travelcost, travel_window.uFrameWidth, 0);
            int pRealTextHeight = pTextHeight + (pFontArrus->GetHeight() - 3) + 146;
            int pPrimaryTextHeight = pRealTextHeight;

            int pCurrentButton = 2;
            int lastsched{ 255 };
            std::string pTopicArray[5]{};

            for (uint i = pDialogueWindow->pStartingPosActiveItem; i < (unsigned int)(pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem); ++i) {
                int schedule_id = transport_routes[route_id][index];
                GUIButton *pButton = pDialogueWindow->GetControl(i);

                bool route_active = 0;

                if (schedule_id != 255 && (lastsched != schedule_id)) {
                    Assert(schedule_id < 35);
                    if (pCurrentButton >= 6)
                        route_active = true;
                    else
                        route_active = transport_schedule[schedule_id].pSchedule[pParty->uCurrentDayOfMonth % 7];
                }

                lastsched = schedule_id;

                if (schedule_id != 255 && route_active &&
                    (!transport_schedule[schedule_id].uQuestBit ||
                        _449B57_test_bit(pParty->_quest_bits, transport_schedule[schedule_id].uQuestBit))) {
                    uint16_t color{};
                    if (pDialogueWindow->pCurrentPosActiveItem == pCurrentButton)
                        color = colorTable.PaleCanary.c16();
                    else
                        color = colorTable.White.c16();

                    pTopicArray[index] = fmt::format("\f{:05}", color);

                    int travel_time = GetTravelTimeTransportDays(schedule_id);

                    std::string str = localization->FormatString(LSTR_FMT_D_DAYS_TO_S, travel_time, pMapStats->pInfos[transport_schedule[schedule_id].uMapInfoID].pName.c_str());
                    pTopicArray[index] += str + "\n \n";
                    pButton->uY = pRealTextHeight;
                    pTextHeight = pFontArrus->CalcTextHeight(str, travel_window.uFrameWidth, 0);
                    pButton->uHeight = pTextHeight;
                    pButton->uW = pButton->uY + pTextHeight - 1 + 6;
                    pRealTextHeight += (pFontArrus->GetHeight() - 3) + pTextHeight;
                } else {
                    pTopicArray[index] = "";
                    if (pButton) {
                        pButton->uW = 0;
                        pButton->uHeight = 0;
                        pButton->uY = 0;
                    }
                }
                ++index;
                ++pCurrentButton;
            }

            if (pRealTextHeight != pPrimaryTextHeight) {
                // height differences means we have travel options
                travel_window.DrawTitleText(pFontArrus, 0, 146, 0,
                    fmt::format("{}\n \n{}{}{}{}{}", travelcost, pTopicArray[0], pTopicArray[1], pTopicArray[2], pTopicArray[3], pTopicArray[4]), 3);
            } else {
                travel_window.DrawTitleText(pFontArrus, 0, (174 - pFontArrus->CalcTextHeight(localization->GetString(LSTR_COME_BACK_ANOTHER_DAY), travel_window.uFrameWidth, 0)) / 2 + 138,
                                            colorTable.White.c16(), localization->GetString(LSTR_COME_BACK_ANOTHER_DAY), 3);
                pAudioPlayer->PauseSounds(-1);
            }
        }
    } else {  //после нажатия топика - travel option selected
        if (dialog_menu_id >= DIALOGUE_TRANSPORT_SCHEDULE_1 &&
            dialog_menu_id <= DIALOGUE_TRANSPORT_SCHEDULE_4) {
            if (pParty->GetGold() < pPrice) {
                GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
                PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_Greeting_2);
                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                return;
            }

            pParty->TakeGold(pPrice);

            int choice_id = dialog_menu_id - DIALOGUE_TRANSPORT_SCHEDULE_1;
            stru365_travel_info *pTravel = &transport_schedule[transport_routes[route_id][choice_id]];

            if (pTravel->pSchedule[pParty->uCurrentDayOfMonth % 7]) {
                if (pCurrentMapName != pMapStats->pInfos[pTravel->uMapInfoID].pFilename) {
                    SaveGame(1, 0);
                    pCurrentMapName = pMapStats->pInfos[pTravel->uMapInfoID].pFilename;

                    dword_6BE364_game_settings_1 |= GAME_SETTINGS_0001;
                    Party_Teleport_Cam_Pitch = 0;
                    Party_Teleport_Z_Speed = 0;
                    Party_Teleport_Cam_Yaw = pTravel->arrival_view_yaw;
                    uGameState = GAME_STATE_CHANGE_LOCATION;
                    Party_Teleport_X_Pos = pTravel->arrival_x;
                    Party_Teleport_Y_Pos = pTravel->arrival_y;
                    Party_Teleport_Z_Pos = pTravel->arrival_z;
                    Start_Party_Teleport_Flag = pTravel->arrival_x | pTravel->arrival_y | pTravel->arrival_z | pTravel->arrival_view_yaw;
                } else {
                    // travelling to map we are already in
                    pCamera3D->_viewYaw = 0;

                    pParty->uFlags |= PARTY_FLAGS_1_ForceRedraw;
                    pParty->vPosition.x = pTravel->arrival_x;
                    pParty->vPosition.y = pTravel->arrival_y;
                    pParty->vPosition.z = pTravel->arrival_z;
                    pParty->uFallStartZ = pParty->vPosition.z;
                    pParty->_viewPitch = 0;
                    pParty->_viewYaw = pTravel->arrival_view_yaw;
                }

                PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_NotEnoughMoney);
                int traveltimedays = GetTravelTimeTransportDays(transport_routes[route_id][choice_id]);

                PlayerSpeech pSpeech;
                int speechlength = 0;
                if (route_id >= HOUSE_BOATS_EMERALD_ISLE - HOUSE_STABLES_HARMONDALE) {
                    pSpeech = SPEECH_TravelBoat;
                    speechlength = 2500;
                } else {
                    pSpeech = SPEECH_TravelHorse;
                    speechlength = 1500;
                }

                RestAndHeal(24 * 60 * traveltimedays);
                pPlayers[pParty->getActiveCharacter()]->playReaction(pSpeech);
                auto timeLimit = std::chrono::system_clock::now() + std::chrono::milliseconds(speechlength);
                while (std::chrono::system_clock::now() < timeLimit) {
                    std::this_thread::sleep_for(1ms);
                }
                while (HouseDialogPressCloseBtn()) {}
                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 0, 0);
            } else {
                dialog_menu_id = DIALOGUE_MAIN;
                pAudioPlayer->playUISound(SOUND_error);
            }
        }
    }
}

bool IsTravelAvailable(int a1) {
    for (uint i = 0; i < 4; ++i) {
        if (transport_schedule[transport_routes[a1][i]]
            .pSchedule[pParty->uCurrentDayOfMonth % 7]) {
            if (!transport_schedule[transport_routes[a1][i]].uQuestBit ||
                _449B57_test_bit(
                    pParty->_quest_bits,
                    transport_schedule[transport_routes[a1][i]].uQuestBit))
                return true;
        }
    }
    return false;
}

//----- (004B7911) --------------------------------------------------------
void TownHallDialog() {
    int v1;                       // eax@10
    int v2;                       // esi@10
    signed int pStringSum;        // ebx@24
    signed int v16;               // ebx@28
    int v17;                      // ebx@28
    GUIButton *pButton;           // eax@30
    int pTextHeight;              // eax@30
    uint16_t pTextColor;  // ax@30
    int v29;                      // [sp+10Ch] [bp-10h]@28
    int v31;                      // [sp+114h] [bp-8h]@29
    GUIFont *pOutString;          // [sp+118h] [bp-4h]@21

    GUIWindow townHall_window = *window_SpeakInHouse;
    townHall_window.uFrameX = 483;
    townHall_window.uFrameWidth = 143;
    townHall_window.uFrameZ = 334;

    std::string fine_str = fmt::format("{}: {}", localization->GetString(LSTR_CURRENT_FINE), pParty->uFine);
    townHall_window.DrawTitleText(pFontArrus, 0, 260, colorTable.PaleCanary.c16(), fine_str, 3);

    switch (dialog_menu_id) {
    case DIALOGUE_MAIN:
    {
        pStringSum = 1;
        pTextHeight = 0;
        pShopOptions[0] = localization->GetString(LSTR_BOUNTY_HUNT);
        if (pParty->uFine > 0) {
            pShopOptions[1] = localization->GetString(LSTR_PAY_FINE);
            pStringSum = 2;
        }
        for (uint i = 0; i < pStringSum; ++i)
            pTextHeight += pFontArrus->CalcTextHeight(
                pShopOptions[i], townHall_window.uFrameWidth, 0);
        v29 = (100 - pTextHeight) / pStringSum;
        v16 = 80 - pStringSum * ((100 - pTextHeight) / pStringSum);
        v17 = (v16 / 2) - v29 / 2 + 158;
        if (pDialogueWindow->pNumPresenceButton > 0) {
            v31 = 2;
            uint j = 0;
            for (uint i = pDialogueWindow->pStartingPosActiveItem;
                i < pDialogueWindow->pNumPresenceButton +
                pDialogueWindow->pStartingPosActiveItem;
                ++i) {
                pButton = pDialogueWindow->GetControl(i);
                pButton->uY = v29 + v17;
                pTextHeight = pFontArrus->CalcTextHeight(
                    pShopOptions[j], townHall_window.uFrameWidth, 0);
                pButton->uHeight = pTextHeight;
                v17 = pButton->uY + pTextHeight - 1;
                pButton->uW = v17 + 6;
                pTextColor = colorTable.PaleCanary.c16();
                if (pDialogueWindow->pCurrentPosActiveItem != v31)
                    pTextColor = colorTable.White.c16();
                townHall_window.DrawTitleText(pFontArrus, 0, pButton->uY, pTextColor, pShopOptions[j], 3);
                ++v31;
                ++j;
            }
        }
        break;
    }
    case DIALOGUE_TOWNHALL_MESSAGE:
    {
        current_npc_text = bountyHuntingText();
        GUIWindow window = *pDialogueWindow;
        window.uFrameWidth = 458;
        window.uFrameZ = 457;
        pOutString = pFontArrus;
        pTextHeight = pFontArrus->CalcTextHeight(current_npc_text, window.uFrameWidth, 13) + 7;
        if (352 - pTextHeight < 8) {
            pOutString = pFontCreate;
            pTextHeight = pFontCreate->CalcTextHeight(current_npc_text, window.uFrameWidth, 13) + 7;
        }
        render->DrawTextureCustomHeight(8 / 640.0f, (352 - pTextHeight) / 480.0f, ui_leather_mm7, pTextHeight);
        render->DrawTextureNew(8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
        // window.DrawText(pOutString, 13, 354 - pTextHeight, 0, pOutString->FitTextInAWindow(current_npc_text, window.uFrameWidth, 13), 0, 0, 0);
        window.DrawText(pOutString, {13, 354 - pTextHeight}, 0, current_npc_text, 0, 0, 0);
        break;
    }
    case DIALOGUE_TOWNHALL_PAY_FINE:
    {
        if (window_SpeakInHouse->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_IN_PROGRESS) {
            townHall_window.DrawTitleText(pFontArrus, 0, 146, colorTable.PaleCanary.c16(),
                                          fmt::format("{}\n{}", localization->GetString(LSTR_PAY), localization->GetString(LSTR_HOW_MUCH)), 3);
            townHall_window.DrawTitleText(pFontArrus, 0, 186, colorTable.White.c16(), keyboardInputHandler->GetTextInput().c_str(), 3);
            townHall_window.DrawFlashingInputCursor(pFontArrus->GetLineWidth(keyboardInputHandler->GetTextInput().c_str()) / 2 + 80, 185, pFontArrus);
            return;
        } else if (window_SpeakInHouse->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_CONFIRMED) {
            int sum = atoi(keyboardInputHandler->GetTextInput().c_str());
            if (sum > 0) {
                int party_gold = pParty->GetGold();
                if (sum > party_gold) {
                    PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_NotEnoughMoney);
                    sum = party_gold;
                }

                if (sum > 0) {
                    int required_sum = pParty->GetFine();
                    if (sum > required_sum)
                        sum = required_sum;

                    pParty->TakeGold(sum);
                    pParty->TakeFine(sum);
                    if (pParty->hasActiveCharacter())
                        pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_BankDeposit);
                }
            }
        }
        window_SpeakInHouse->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        break;
    }
    default:
        break;
    }
    return;
}

void BankDialog() {
    GUIWindow bank_window = *window_SpeakInHouse;
    bank_window.uFrameX = 483;
    bank_window.uFrameWidth = 143;
    bank_window.uFrameZ = 334;
    bank_window.DrawTitleText(
        pFontArrus, 0, 220, colorTable.PaleCanary.c16(),
        fmt::format("{}: {}", localization->GetString(LSTR_BALANCE), pParty->uNumGoldInBank),
        3);
    switch (dialog_menu_id) {
    case DIALOGUE_MAIN:
    {
        uint16_t pColorText = colorTable.PaleCanary.c16();
        if (pDialogueWindow->pCurrentPosActiveItem != 2) {
            pColorText = colorTable.White.c16();
        }
        bank_window.DrawTitleText(pFontArrus, 0, 146, pColorText,
            localization->GetString(LSTR_DEPOSIT), 3);
        pColorText = colorTable.PaleCanary.c16();
        if (pDialogueWindow->pCurrentPosActiveItem != 3) {
            pColorText = colorTable.White.c16();
        }
        bank_window.DrawTitleText(pFontArrus, 0, 176, pColorText,
            localization->GetString(LSTR_WITHDRAW), 3);
        break;
    }
    case DIALOGUE_BANK_PUT_GOLD:
    {
        if (window_SpeakInHouse->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_IN_PROGRESS) {
            bank_window.DrawTitleText(pFontArrus, 0, 146, colorTable.PaleCanary.c16(),
                                      fmt::format("{}\n{}", localization->GetString(LSTR_DEPOSIT), localization->GetString(LSTR_HOW_MUCH)), 3);
            bank_window.DrawTitleText(pFontArrus, 0, 186, colorTable.White.c16(), keyboardInputHandler->GetTextInput().c_str(), 3);
            bank_window.DrawFlashingInputCursor(pFontArrus->GetLineWidth(keyboardInputHandler->GetTextInput().c_str()) / 2 + 80, 185, pFontArrus);
            return;
        }
        if (window_SpeakInHouse->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_CONFIRMED) {
            int sum = atoi(keyboardInputHandler->GetTextInput().c_str());
            if (sum <= 0) {
                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                return;
            }

            int party_gold = pParty->GetGold();
            if (sum > party_gold) {
                PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_NotEnoughMoney);
                sum = party_gold;
            }

            if (sum > 0) {
                pParty->TakeGold(sum);
                pParty->AddBankGold(sum);
                if (pParty->hasActiveCharacter()) {
                    pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_BankDeposit);
                }
            }
        }
        window_SpeakInHouse->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        break;
    }
    case DIALOGUE_BANK_GET_GOLD:
    {
        if (window_SpeakInHouse->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_IN_PROGRESS) {
            bank_window.DrawTitleText(pFontArrus, 0, 146, colorTable.PaleCanary.c16(),
                                      fmt::format("{}\n{}", localization->GetString(LSTR_WITHDRAW), localization->GetString(LSTR_HOW_MUCH)), 3);
            bank_window.DrawTitleText(pFontArrus, 0, 186, colorTable.White.c16(), keyboardInputHandler->GetTextInput().c_str(), 3);
            bank_window.DrawFlashingInputCursor(pFontArrus->GetLineWidth(keyboardInputHandler->GetTextInput().c_str()) / 2 + 80, 185, pFontArrus);
            return;
        } else if (window_SpeakInHouse->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_CONFIRMED) {
            window_SpeakInHouse->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
            int sum = atoi(keyboardInputHandler->GetTextInput().c_str());
            if (sum <= 0) {
                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                return;
            }

            int bank_gold = pParty->GetBankGold();
            if (sum > bank_gold) {
                PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_NotEnoughMoney);
                sum = bank_gold;
            }

            if (sum > 0) {
                pParty->TakeBankGold(sum);
                pParty->AddGold(sum);
            }
        }
        window_SpeakInHouse->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        break;
    }
    default:
        break;
    }
}

//----- (004B8285) --------------------------------------------------------
void TavernDialog() {
    int pPriceRoom;
    int pPriceFood;
    int pPriceSkill;
    int pItemNum;
    double v2;                // st7@1
    int pNumString;           // edi@16
    signed int v9;            // esi@16
    GUIButton *pButton;       // eax@65
    int pSkillCount;
    signed int pOptionsCount;       // edi@77
    signed int i;                   // esi@79
                                    //  signed int v53; // edi@81
    const char *pText;              // [sp-4h] [bp-278h]@93
    unsigned int pTopic1Height;     // [sp+26Fh] [bp-5h]@55
    uint8_t pTopic2Height;  // [sp+267h] [bp-Dh]@57
    uint8_t pTopic3Height;  // [sp+253h] [bp-21h]@59
    uint8_t pTopic4Height = 0;
    int pTextHeight;
    int all_text_height;  // [sp+260h] [bp-14h]@18
    GUIFont *pOutString;

    GUIWindow dialog_window = *window_SpeakInHouse;
    dialog_window.uFrameX = 483;
    dialog_window.uFrameWidth = 143;
    dialog_window.uFrameZ = 334;
    v2 = p2DEvents[window_SpeakInHouse->wData.val - 1].fPriceMultiplier;

    // TODO(pskelton): check this behaviour
    if (!pParty->hasActiveCharacter())  // avoid nzi
        pParty->setActiveToFirstCanAct();

    pPriceRoom = ((v2 * v2) / 10) *
                 (100 - pParty->getOptionallySharedSkillStrongestEffect(
                            PLAYER_SKILL_TYPE::PLAYER_SKILL_MERCHANT)) /
        100;  // nzi
    if (pPriceRoom < ((v2 * v2) / 10) / 3) pPriceRoom = ((v2 * v2) / 10) / 3;
    if (pPriceRoom <= 0) pPriceRoom = 1;

    pPriceFood = ((v2 * v2) * v2 / 100) *
                 (100 - pParty->getOptionallySharedSkillStrongestEffect(
                            PLAYER_SKILL_TYPE::PLAYER_SKILL_MERCHANT)) /
                 100;
    if (pPriceFood < ((v2 * v2) * v2 / 100) / 3)
        pPriceFood = ((v2 * v2) * v2 / 100) / 3;
    if (pPriceFood <= 0) pPriceFood = 1;

    switch (dialog_menu_id) {
    case DIALOGUE_MAIN:
    {
        if (!HouseUI_CheckIfPlayerCanInteract()) return;

        std::string topic1 = fmt::format("\f{:05}",
                pDialogueWindow->pCurrentPosActiveItem == 2
                ? colorTable.PaleCanary.c16()
                : colorTable.White.c16()) +
                             localization->FormatString(LSTR_FMT_RENT_ROOM_FOR_D_GOLD, pPriceRoom);
        pTopic1Height = pFontArrus->CalcTextHeight(
            topic1, dialog_window.uFrameWidth, 0);

        std::string topic2 = fmt::format("\f{:05}",
                pDialogueWindow->pCurrentPosActiveItem == 3
                ? colorTable.PaleCanary.c16()
                : colorTable.White.c16())
            + localization->FormatString(
                LSTR_FMT_BUY_D_FOOD_FOR_D_GOLD,
                (unsigned int)
                p2DEvents[window_SpeakInHouse->wData.val - 1].fPriceMultiplier,
                pPriceFood);
        pTopic2Height = pFontArrus->CalcTextHeight(topic2, dialog_window.uFrameWidth, 0);

        std::string topic3 = fmt::format("\f{:05}",
                pDialogueWindow->pCurrentPosActiveItem == 4
                ? colorTable.PaleCanary.c16()
                : colorTable.White.c16()) +
                             localization->GetString(LSTR_LEARN_SKILLS);
        pTopic3Height = pFontArrus->CalcTextHeight(topic3, dialog_window.uFrameWidth, 0);

        std::string topic4;
        if (window_SpeakInHouse->wData.val >= 108 &&
            window_SpeakInHouse->wData.val <= 120) {
            topic4 =
                fmt::format("\f{:05}",
                    pDialogueWindow->pCurrentPosActiveItem == 5
                    ? colorTable.PaleCanary.c16()
                    : colorTable.White.c16()) +
                localization->GetString(LSTR_PLAY_ARCOMAGE);
            pTopic4Height = pFontArrus->CalcTextHeight(
                topic4, dialog_window.uFrameWidth, 0);
        }

        if (pDialogueWindow->pNumPresenceButton) {
            for (pItemNum = pDialogueWindow->pStartingPosActiveItem;
                pItemNum < pDialogueWindow->pStartingPosActiveItem +
                pDialogueWindow->pNumPresenceButton;
                ++pItemNum) {
                pButton = pDialogueWindow->GetControl(pItemNum);
                if (pButton->msg_param == 15) {
                    pButton->uHeight = pTopic1Height;
                    pButton->uY = 146;
                    pButton->uW = pTopic1Height + 145 + 6;
                } else if (pButton->msg_param == 16) {
                    pButton->uHeight = pTopic2Height;
                    pButton->uY =
                        pTopic1Height + (pFontArrus->GetHeight() ) + 146;
                    pButton->uW = (pTopic1Height +
                        (pFontArrus->GetHeight() ) + 146) +
                        pTopic2Height - 1 + 6;
                } else if (pButton->msg_param == 96) {
                    pButton->uY = pTopic1Height + pTopic2Height +
                        2 * (pFontArrus->GetHeight() ) + 146;
                    pButton->uHeight = pTopic3Height;
                    pButton->uW =
                        pTopic3Height +
                        (pTopic1Height + pTopic2Height +
                            2 * (pFontArrus->GetHeight() ) + 146) -
                        1 + 6;
                } else if (pButton->msg_param == 101) {
                    pButton->uHeight = pTopic4Height;
                    pButton->uY = pTopic1Height +
                        3 * (pFontArrus->GetHeight() ) +
                        pTopic4Height + pTopic2Height + 146;
                    pButton->uW =
                        (pTopic1Height + 3 * (pFontArrus->GetHeight() ) +
                            pTopic4Height + pTopic2Height + 146) +
                        pTopic4Height - 1 + 6;
                }
            }
            dialog_window.DrawTitleText(
                pFontArrus, 0, 146, 0,
                fmt::format("{}\n \n{}\n \n{}\n \n{}", topic1, topic2, topic3, topic4),
                3);
        }
        break;
    }
    case DIALOGUE_TAVERN_ARCOMAGE_RULES:
    {
        pOutString = pFontArrus;

        std::string str = pNPCTopics[354].pText;
        dialog_window.uFrameWidth = game_viewport_width;
        dialog_window.uFrameZ = 452;
        pTextHeight = pFontArrus->CalcTextHeight(str, dialog_window.uFrameWidth, 12) + 7;
        if (352 - pTextHeight < 8) {
            pOutString = pFontCreate;
            pTextHeight = pFontCreate->CalcTextHeight(str, dialog_window.uFrameWidth, 12) + 7;
        }
        render->DrawTextureCustomHeight(8 / 640.0f, (352 - pTextHeight) / 480.0f, ui_leather_mm7, pTextHeight);
        render->DrawTextureNew(8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
        window_SpeakInHouse->DrawText(pOutString, {12, 354 - pTextHeight}, colorTable.Black.c16(),
            pOutString->FitTextInAWindow(str, dialog_window.uFrameWidth, 12), 0, 0, 0);
        break;
    }

    case DIALOGUE_TAVERN_ARCOMAGE_VICTORY_CONDITIONS:
    {
        std::string label =
            pNPCTopics[window_SpeakInHouse->wData.val + 247].pText;
        dialog_window.uFrameWidth = game_viewport_width;
        dialog_window.uFrameZ = 452;
        pTextHeight = pFontArrus->CalcTextHeight(
            label, dialog_window.uFrameWidth, 12) +
            7;
        render->DrawTextureCustomHeight(8 / 640.0f,
            (352 - pTextHeight) / 480.0f,
            ui_leather_mm7, pTextHeight);
        render->DrawTextureNew(
            8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
        window_SpeakInHouse->DrawText(pFontArrus, {12, 354 - pTextHeight}, colorTable.Black.c16(),
            pFontArrus->FitTextInAWindow(label, dialog_window.uFrameWidth, 12));
        break;
    }

    case DIALOGUE_TAVERN_ARCOMAGE_RESULT:
    {
        if (pArcomageGame->bGameInProgress == 1) return;
        if (pArcomageGame->uGameWinner) {
            if (pArcomageGame->uGameWinner == 1)
                pText = localization->GetString(LSTR_YOU_WON);
            else
                pText = localization->GetString(LSTR_YOU_LOST);
        } else {
            pText = localization->GetString(LSTR_A_TIE);
        }
        dialog_window.DrawTitleText(
            pFontArrus, 0,
            (174 - pFontArrus->CalcTextHeight(
                pText, dialog_window.uFrameWidth, 0)) /
            2 +
            138, colorTable.PaleCanary.c16(), pText, 3);
        break;
    }
    case DIALOGUE_TAVERN_REST:
    {
        if (pParty->GetGold() >= pPriceRoom) {
            pParty->TakeGold(pPriceRoom);
            PlayHouseSound(window_SpeakInHouse->wData.val,
                HouseSound_NotEnoughMoney);
            dialog_menu_id = DIALOGUE_NULL;
            HouseDialogPressCloseBtn();
            GetHouseGoodbyeSpeech();
            pMediaPlayer->Unload();

            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_RentRoom, window_SpeakInHouse->wData.val, 1);
            window_SpeakInHouse->Release();
            window_SpeakInHouse = 0;
            return;
        }
        GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
        PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_Goodbye);
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        break;
    }

    case DIALOGUE_LEARN_SKILLS:
    {
        if (!HouseUI_CheckIfPlayerCanInteract()) return;
        pSkillCount = 0;
        v9 = (int64_t)(p2DEvents[
            window_SpeakInHouse->wData.val - 1].flt_24 * 500.0);
        pPriceSkill = v9 * (100 - pPlayers[pParty->getActiveCharacter()]->GetMerchant()) / 100;
        if (pPriceSkill < v9 / 3) pPriceSkill = v9 / 3;
        all_text_height = 0;
        for (int i = pDialogueWindow->pStartingPosActiveItem;
            i < pDialogueWindow->pStartingPosActiveItem +
            pDialogueWindow->pNumPresenceButton;
            ++i) {
            auto skill = GetLearningDialogueSkill(
                (DIALOGUE_TYPE)pDialogueWindow->GetControl(i)->msg_param
            );
            if (skillMaxMasteryPerClass[pPlayers[pParty->getActiveCharacter()]->classType][skill] != PLAYER_SKILL_MASTERY_NONE
                && !pPlayers[pParty->getActiveCharacter()]->pActiveSkills[skill]) {
                all_text_height = pFontArrus->CalcTextHeight(
                    localization->GetSkillName(skill),
                    dialog_window.uFrameWidth, 0);
                pSkillCount++;
            }
        }
        SkillTrainingDialogue(&dialog_window, pSkillCount, all_text_height, pPriceSkill);
        return;
    }

    case DIALOGUE_TAVERN_BUY_FOOD:
    {
        if ((double)pParty->GetFood() >=
            p2DEvents[window_SpeakInHouse->wData.val - 1].fPriceMultiplier) {
            GameUI_SetStatusBar(LSTR_RATIONS_FULL);
            if (pParty->hasActiveCharacter())
                pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_PacksFull);
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
        if (pParty->GetGold() >= pPriceFood) {
            pParty->TakeGold(pPriceFood);
            pParty->SetFood(p2DEvents[window_SpeakInHouse->wData.val - 1].fPriceMultiplier);
            PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_Greeting_2);
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
        GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
        PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_Goodbye);
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        break;
    }

    case DIALOGUE_TAVERN_ARCOMAGE_MAIN:
    {
        if (HouseUI_CheckIfPlayerCanInteract()) {
            pOptionsCount = 2;
            pShopOptions[0] = localization->GetString(LSTR_RULES);
            pShopOptions[1] = localization->GetString(LSTR_VICTORY_CONDITIONS);
            if (pParty->hasItem(ITEM_QUEST_ARCOMAGE_DECK)) {
                pShopOptions[2] = localization->GetString(LSTR_PLAY);
                pOptionsCount = 3;
            }
            for (i = 0; i < pOptionsCount; ++i)
                all_text_height = pFontArrus->CalcTextHeight(
                    pShopOptions[i], dialog_window.uFrameWidth, 0);
            all_text_height = (174 - all_text_height) / pOptionsCount;

            int v54 = (174 -
                pOptionsCount * (174 - all_text_height) / pOptionsCount -
                all_text_height) /
                2 -
                (174 - all_text_height) / pOptionsCount / 2 + 138;
            pNumString = 0;
            for (pItemNum = pDialogueWindow->pStartingPosActiveItem;
                pItemNum < pDialogueWindow->pNumPresenceButton +
                pDialogueWindow->pStartingPosActiveItem;
                pItemNum++) {
                pButton = pDialogueWindow->GetControl(pItemNum);
                pButton->uY = all_text_height + v54;
                pTextHeight = pFontArrus->CalcTextHeight(
                    pShopOptions[pNumString], dialog_window.uFrameWidth, 0);
                pButton->uHeight = pTextHeight;
                v54 = pButton->uY + pTextHeight - 1;
                pButton->uW = v54 + 6;
                int pColorText = colorTable.PaleCanary.c16();
                if (pDialogueWindow->pCurrentPosActiveItem != pItemNum)
                    pColorText = colorTable.White.c16();
                dialog_window.DrawTitleText(pFontArrus, 0, pButton->uY,
                    pColorText,
                    pShopOptions[pNumString], 3);
                ++pNumString;
            }
        }
        break;
    }
    default: { break; }
    }
}

void TempleDialog() {
    int pPrice;                   // edi@1
    int pTextHeight;              // eax@11
    uint16_t pTextColor;  // ax@21
    DDM_DLV_Header *ddm;          // edi@29
    GUIButton *pButton;           // edi@64
    uint8_t index;        // [sp+1B7h] [bp-Dh]@64
    int v64;                      // [sp+1B8h] [bp-Ch]@6
    unsigned int pCurrentItem;    // [sp+1BCh] [bp-8h]@6
    int all_text_height;          // [sp+1C0h] [bp-4h]@6

    GUIWindow tample_window = *window_SpeakInHouse;
    tample_window.uFrameX = 483;
    tample_window.uFrameWidth = 143;
    tample_window.uFrameZ = 334;

    // TODO(pskelton): check this behaviour
    if (!pParty->hasActiveCharacter()) {  // avoid nzi
        pParty->setActiveToFirstCanAct();
    }

    pPrice = pPlayers[pParty->getActiveCharacter()]->GetTempleHealCostModifier(
        p2DEvents[window_SpeakInHouse->wData.val - 1].fPriceMultiplier);
    if (dialog_menu_id == DIALOGUE_MAIN) {
        index = 1;
        pButton = pDialogueWindow->GetControl(
            pDialogueWindow->pStartingPosActiveItem);
        pButton->uHeight = 0;
        pButton->uY = 0;
        if (pPlayers[pParty->getActiveCharacter()]->IsPlayerHealableByTemple()) {
            static std::string shop_option_container;
            shop_option_container =
                fmt::format("{} {} {}",
                    localization->GetString(LSTR_HEAL), pPrice,
                    localization->GetString(LSTR_GOLD));
            pShopOptions[0] = shop_option_container.c_str();
            index = 0;
        }
        pShopOptions[1] = localization->GetString(LSTR_DONATE);
        pShopOptions[2] = localization->GetString(LSTR_LEARN_SKILLS);
        all_text_height = 0;
        if (index < pDialogueWindow->pNumPresenceButton) {
            uint i = index;
            for (uint j = index; j < pDialogueWindow->pNumPresenceButton; ++j) {
                all_text_height += pFontArrus->CalcTextHeight(
                    pShopOptions[1 * i], tample_window.uFrameWidth, 0);
                i++;
            }
        }
        v64 = (174 - (signed int)all_text_height) /
            (pDialogueWindow->pNumPresenceButton - index);
        if (v64 > 32) v64 = 32;
        all_text_height =
            (174 - v64 * (pDialogueWindow->pNumPresenceButton - index) -
            (signed int)all_text_height) /
            2 -
            v64 / 2 + 138;
        if (index + pDialogueWindow->pStartingPosActiveItem <
            pDialogueWindow->pStartingPosActiveItem +
            pDialogueWindow->pNumPresenceButton) {
            uint i = index;
            for (pCurrentItem = index + pDialogueWindow->pStartingPosActiveItem;
                (signed int)pCurrentItem <
                pDialogueWindow->pNumPresenceButton +
                pDialogueWindow->pStartingPosActiveItem;
                ++pCurrentItem) {
                pButton = pDialogueWindow->GetControl(pCurrentItem);
                pButton->uY = v64 + all_text_height;
                pTextHeight = pFontArrus->CalcTextHeight(
                    pShopOptions[1 * i], tample_window.uFrameWidth, 0);
                pButton->uHeight = pTextHeight;
                pButton->uW = pButton->uY + pTextHeight - 1 + 6;
                all_text_height = pButton->uW;
                pTextColor = colorTable.PaleCanary.c16();
                if (pDialogueWindow->pCurrentPosActiveItem != index + 2)
                    pTextColor = colorTable.White.c16();
                tample_window.DrawTitleText(pFontArrus, 0, pButton->uY,
                    pTextColor, pShopOptions[1 * i], 3);
                i++;
                index++;
            }
        }
        return;
    }
    //-------------------------------------------------
    if (dialog_menu_id == DIALOGUE_TEMPLE_HEAL) {
        if (!pPlayers[pParty->getActiveCharacter()]->IsPlayerHealableByTemple()) return;
        if (pParty->GetGold() < pPrice) {
            GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
            PlayHouseSound(window_SpeakInHouse->wData.val, HouseSound_NotEnoughMoney);
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
        pParty->TakeGold(pPrice);

        pPlayers[pParty->getActiveCharacter()]->conditions.ResetAll();
        pPlayers[pParty->getActiveCharacter()]->sHealth =
            pPlayers[pParty->getActiveCharacter()]->GetMaxHealth();
        pPlayers[pParty->getActiveCharacter()]->sMana =
            pPlayers[pParty->getActiveCharacter()]->GetMaxMana();

        if (window_SpeakInHouse->wData.val != 78 &&
            (window_SpeakInHouse->wData.val <= 80 ||
            window_SpeakInHouse->wData.val > 82)) {
            if (pPlayers[pParty->getActiveCharacter()]->conditions.Has(Condition_Zombie)) {
                // если состояние зомби
                pPlayers[pParty->getActiveCharacter()]->uCurrentFace =
                    pPlayers[pParty->getActiveCharacter()]->uPrevFace;
                pPlayers[pParty->getActiveCharacter()]->uVoiceID =
                    pPlayers[pParty->getActiveCharacter()]->uPrevVoiceID;
                GameUI_ReloadPlayerPortraits(
                    pParty->getActiveCharacter() - 1,
                    pPlayers[pParty->getActiveCharacter()]->uPrevFace);
            }
            pAudioPlayer->playExclusiveSound(SOUND_heal);
            pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_TempleHeal);
            pOtherOverlayList->_4418B1(20, pParty->getActiveCharacter() + 99, 0, 65536);
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
        if (pPlayers[pParty->getActiveCharacter()]->conditions.Has(Condition_Zombie)) {
            // LODWORD(pPlayers[pParty->getActiveCharacter()]->pConditions[Condition_Zombie])
            // =
            // LODWORD(pPlayers[pParty->getActiveCharacter()]->pConditions[Condition_Zombie]);
        } else {
            if (pPlayers[pParty->getActiveCharacter()]->conditions.HasNone({Condition_Eradicated, Condition_Petrified, Condition_Dead})) {
                pAudioPlayer->playExclusiveSound(SOUND_heal);
                pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_TempleHeal);
                pOtherOverlayList->_4418B1(20, pParty->getActiveCharacter() + 99, 0, 65536);
                pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                return;
            }
            pPlayers[pParty->getActiveCharacter()]->uPrevFace =
                pPlayers[pParty->getActiveCharacter()]->uCurrentFace;
            pPlayers[pParty->getActiveCharacter()]->uPrevVoiceID =
                pPlayers[pParty->getActiveCharacter()]->uVoiceID;
            pPlayers[pParty->getActiveCharacter()]->SetCondition(Condition_Zombie, 1);
            pPlayers[pParty->getActiveCharacter()]->uVoiceID =
                (pPlayers[pParty->getActiveCharacter()]->GetSexByVoice() != 0) + 23;
            pPlayers[pParty->getActiveCharacter()]->uCurrentFace =
                (pPlayers[pParty->getActiveCharacter()]->GetSexByVoice() != 0) + 23;
            GameUI_ReloadPlayerPortraits(
                pParty->getActiveCharacter() - 1,
                (pPlayers[pParty->getActiveCharacter()]->GetSexByVoice() != 0) + 23);
            pPlayers[pParty->getActiveCharacter()]->conditions.Set(Condition_Zombie, pParty->GetPlayingTime());
            // v39 = (GUIWindow *)HIDWORD(pParty->uTimePlayed);
        }
        // HIDWORD(pPlayers[pParty->getActiveCharacter()]->pConditions[Condition_Zombie]) =
        // (int)v39;
        pPlayers[pParty->getActiveCharacter()]->conditions.Set(Condition_Zombie, pParty->GetPlayingTime());
        pAudioPlayer->playExclusiveSound(SOUND_heal);
        pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_TempleHeal);
        pOtherOverlayList->_4418B1(20, pParty->getActiveCharacter() + 99, 0, 65536);
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        return;
    }
    //---------------------------------------------------
    if (dialog_menu_id == DIALOGUE_TEMPLE_DONATE) {
        pPrice = p2DEvents[window_SpeakInHouse->wData.val - 1].fPriceMultiplier;
        if (pParty->GetGold() >= pPrice) {
            pParty->TakeGold(pPrice);
            if (uCurrentlyLoadedLevelType != LEVEL_Outdoor)
                ddm = &pIndoor->dlv;
            else
                ddm = &pOutdoor->ddm;

            if (ddm->uReputation > -5) {
                ddm->uReputation = ddm->uReputation - 1;
                if (ddm->uReputation - 1 < -5) ddm->uReputation = -5;
            }
            if ((uint8_t)byte_F8B1EF[pParty->getActiveCharacter()] == pParty->uCurrentDayOfMonth % 7) {
                if (ddm->uReputation <= -5) {
                    pushTempleSpell(SPELL_AIR_WIZARD_EYE);
                }
                if (ddm->uReputation <= -10) {
                    pushTempleSpell(SPELL_SPIRIT_PRESERVATION);
                }
                if (ddm->uReputation <= -15) {
                    pushTempleSpell(SPELL_BODY_PROTECTION_FROM_MAGIC);
                }
                if (ddm->uReputation <= -20) {
                    pushTempleSpell(SPELL_LIGHT_HOUR_OF_POWER);
                }
                if (ddm->uReputation <= -25) {
                    pushTempleSpell(SPELL_LIGHT_DAY_OF_PROTECTION);
                }
            }
            ++byte_F8B1EF[pParty->getActiveCharacter()];
            pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_TempleDonate);
            GameUI_SetStatusBar(LSTR_THANK_YOU);
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
        GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
        PlayHouseSound(window_SpeakInHouse->wData.val,
            HouseSound_NotEnoughMoney);
        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
        return;
    }
    //------------------------------------------------
    if (dialog_menu_id == DIALOGUE_LEARN_SKILLS) {
        if (HouseUI_CheckIfPlayerCanInteract()) {
            all_text_height = 0;
            pCurrentItem =
                (int64_t)(p2DEvents[
                    window_SpeakInHouse->wData.val -
                    1]
                    .flt_24 *
                    500.0);
            v64 = (signed int)(pCurrentItem *
                (100 -
                    pPlayers[pParty->getActiveCharacter()]->GetMerchant())) /
                100;
            if (v64 < (signed int)pCurrentItem / 3)
                v64 = (signed int)pCurrentItem / 3;
            pCurrentItem = 0;
            for (int i = pDialogueWindow->pStartingPosActiveItem;
                i < pDialogueWindow->pNumPresenceButton +
                pDialogueWindow->pStartingPosActiveItem;
                ++i) {
                auto skill = GetLearningDialogueSkill(
                    (DIALOGUE_TYPE)pDialogueWindow->GetControl(i)->msg_param
                );
                if (skillMaxMasteryPerClass[pPlayers[pParty->getActiveCharacter()]->classType][skill] != PLAYER_SKILL_MASTERY_NONE
                    && !pPlayers[pParty->getActiveCharacter()]->pActiveSkills[skill]) {
                    all_text_height += pFontArrus->CalcTextHeight(
                        localization->GetSkillName(skill),
                        tample_window.uFrameWidth, 0);
                    ++pCurrentItem;
                }
            }

            SkillTrainingDialogue(&tample_window, pCurrentItem, all_text_height, v64);
        }
    }
}

void TrainingDialog(const char *s) {
    uint64_t v5;  // edi@3
    int v8;               // edx@4
    double v9;            // st7@6
    signed int v10;       // esi@6
    int pPrice = 0;           // ecx@6
    int v14;              // esi@14
    int v33;              // eax@36
    unsigned int v36;     // eax@38
    unsigned int v42;     // eax@46
    int index;
    int all_text_height;          // eax@68
    int v49;                      // ebx@69
    GUIButton *pButton;           // eax@71
    int pTextHeight;              // eax@71
    int v69;                      // [sp+70h] [bp-14h]@6

    GUIWindow training_dialog_window = *window_SpeakInHouse;
    training_dialog_window.uFrameX = 483;
    training_dialog_window.uFrameWidth = 143;
    training_dialog_window.uFrameZ = 334;

    // TODO(pskelton): check this behaviour
    if (!pParty->hasActiveCharacter())  // avoid nzi
        pParty->setActiveToFirstCanAct();

    v5 = 1000ull * pPlayers[pParty->getActiveCharacter()]->uLevel *
        (pPlayers[pParty->getActiveCharacter()]->uLevel + 1) /
        2;  // E n = n(n + 1) / 2
            // v68 = pMaxLevelPerTrainingHallType[(unsigned
            // int)window_SpeakInHouse->ptr_1C -
            // HOUSE_TRAINING_HALL_EMERALD_ISLE];
    if (pPlayers[pParty->getActiveCharacter()]->uExperience >= v5) {
        v8 = pPlayers[pParty->getActiveCharacter()]->classType % 4 + 1;
        if (v8 == 4) v8 = 3;
        v9 = (double)pPlayers[pParty->getActiveCharacter()]->uLevel;
        v69 = v8;
        v10 = (int64_t)(v9 *
            p2DEvents[window_SpeakInHouse->wData.val - 1]
            .fPriceMultiplier *
            (double)v8);
        pPrice = v10 * (100 - pPlayers[pParty->getActiveCharacter()]->GetMerchant()) / 100;
        if (pPrice < v10 / 3) pPrice = v10 / 3;
    }
    //-------------------------------------------------------
    all_text_height = 0;
    if (HouseUI_CheckIfPlayerCanInteract()) {
        if (dialog_menu_id == DIALOGUE_MAIN) {
            if (HouseUI_CheckIfPlayerCanInteract()) {
                index = 0;
                pShopOptions[0] = s;  // set first item to fucntion param - this
                                      // always gets overwritten below??
                pShopOptions[1] =
                    localization->GetString(LSTR_LEARN_SKILLS);
                if (pDialogueWindow->pStartingPosActiveItem <
                    pDialogueWindow->pStartingPosActiveItem +
                    pDialogueWindow->pNumPresenceButton) {
                    for (int i = pDialogueWindow->pStartingPosActiveItem;
                        i < pDialogueWindow->pNumPresenceButton +
                        pDialogueWindow->pStartingPosActiveItem;
                        ++i) {
                        if (pDialogueWindow->GetControl(i)->msg_param ==
                            DIALOGUE_TRAINING_HALL_TRAIN) {
                            static std::string shop_option_str_container;
                            if (pPlayers[pParty->getActiveCharacter()]->uLevel >=
                                pMaxLevelPerTrainingHallType
                                [window_SpeakInHouse->wData.val -
                                HOUSE_TRAINING_HALL_EMERALD_ISLE]) {
                                shop_option_str_container = fmt::format(
                                    "{}\n \n{}",
                                    localization->GetString(LSTR_TEACHER_LEVEL_TOO_LOW),
                                    localization->GetString(LSTR_CANT_TRAIN_FURTHER));
                                pShopOptions[index] = shop_option_str_container.c_str();

                            } else {
                                if (pPlayers[pParty->getActiveCharacter()]->uExperience < v5)
                                    shop_option_str_container = localization->FormatString(
                                        LSTR_XP_UNTIL_NEXT_LEVEL,
                                        (uint)(v5 - pPlayers[pParty->getActiveCharacter()]->uExperience),
                                        pPlayers[pParty->getActiveCharacter()]->uLevel + 1);
                                else
                                    shop_option_str_container = localization->FormatString(
                                        LSTR_FMT_TRAIN_LEVEL_D_FOR_D_GOLD,
                                        pPlayers[pParty->getActiveCharacter()]->uLevel + 1,
                                        pPrice);
                                pShopOptions[index] = shop_option_str_container.c_str();
                            }
                        }
                        all_text_height += pFontArrus->CalcTextHeight(
                            pShopOptions[index],
                            training_dialog_window.uFrameWidth, 0);
                        ++index;
                    }
                }
                v49 =
                    (2 * (87 - (174 - all_text_height) / 2) - all_text_height) /
                    2 -
                    (174 - all_text_height) / 2 / 2 + 138;
                if (pDialogueWindow->pStartingPosActiveItem <
                    pDialogueWindow->pStartingPosActiveItem +
                    pDialogueWindow->pNumPresenceButton) {
                    index = 0;
                    for (int i = pDialogueWindow->pStartingPosActiveItem;
                        i < pDialogueWindow->pStartingPosActiveItem +
                        pDialogueWindow->pNumPresenceButton;
                        ++i) {
                        pButton = pDialogueWindow->GetControl(i);
                        pButton->uY = (174 - all_text_height) / 2 + v49;
                        pTextHeight = pFontArrus->CalcTextHeight(
                            pShopOptions[index],
                            training_dialog_window.uFrameWidth, 0);
                        pButton->uHeight = pTextHeight;
                        pButton->uW = pTextHeight + pButton->uY - 1 + 6;
                        v49 = pButton->uW;
                        int pTextColor = colorTable.Jonquil.c16();
                        if (pDialogueWindow->pCurrentPosActiveItem != i)
                            pTextColor = colorTable.White.c16();
                        training_dialog_window.DrawTitleText(
                            pFontArrus, 0, pButton->uY, pTextColor,
                            pShopOptions[index], 3);
                        ++index;
                    }
                }
            }
        }
        //------------------------------------------------------------------
        if (dialog_menu_id == DIALOGUE_TRAINING_HALL_TRAIN) {
            std::string label;

            if (!HouseUI_CheckIfPlayerCanInteract()) {
                v33 = pFontArrus->CalcTextHeight(
                    pNPCTopics[122].pText, training_dialog_window.uFrameWidth,
                    0);
                training_dialog_window.DrawTitleText(
                    pFontArrus, 0, (212 - v33) / 2 + 101, colorTable.Jonquil.c16(), pNPCTopics[122].pText, 3);
                pDialogueWindow->pNumPresenceButton = 0;
                return;
            }
            if (pPlayers[pParty->getActiveCharacter()]->uLevel <
                pMaxLevelPerTrainingHallType
                [window_SpeakInHouse->wData.val -
                HOUSE_TRAINING_HALL_EMERALD_ISLE]) {
                if ((int64_t)pPlayers[pParty->getActiveCharacter()]->uExperience >=
                    v5) {
                    if (pParty->GetGold() >= pPrice) {
                        pParty->TakeGold(pPrice);
                        PlayHouseSound(
                            window_SpeakInHouse->wData.val,
                            HouseSound_NotEnoughMoney);
                        ++pPlayers[pParty->getActiveCharacter()]->uLevel;
                        pPlayers[pParty->getActiveCharacter()]->uSkillPoints +=
                            pPlayers[pParty->getActiveCharacter()]->uLevel / 10 + 5;
                        pPlayers[pParty->getActiveCharacter()]->sHealth =
                            pPlayers[pParty->getActiveCharacter()]->GetMaxHealth();
                        pPlayers[pParty->getActiveCharacter()]->sMana =
                            pPlayers[pParty->getActiveCharacter()]->GetMaxMana();
                        uint max_level_in_party = player_levels[0];
                        for (uint _it = 1; _it < 4; ++_it) {
                            if (player_levels[_it] > max_level_in_party)
                                max_level_in_party = player_levels[_it];
                        }
                        ++player_levels[pParty->getActiveCharacter() - 1];
                        if (player_levels[pParty->getActiveCharacter() - 1] >
                            max_level_in_party) {  // if we reach new maximum
                                                   // party level feature is
                                                   // broken thou, since this
                                                   // array is always zeroed in
                                                   // EnterHouse
                            v42 = 60 * (_494820_training_time(pParty->uCurrentHour) + 4) - pParty->uCurrentMinute;
                            if (window_SpeakInHouse->wData.val == HOUSE_TRAINING_HALL_PIT || window_SpeakInHouse->wData.val == HOUSE_TRAINING_HALL_NIGHON)
                                v42 += 12 * 60;
                            RestAndHeal(v42 + 7 * 24 * 60);
                            if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
                                pOutdoor->SetFog();
                        }
                        pPlayers[pParty->getActiveCharacter()]->playReaction(SPEECH_LevelUp);

                        GameUI_SetStatusBar(
                            LSTR_FMT_S_NOW_LEVEL_D,
                            pPlayers[pParty->getActiveCharacter()]->pName.c_str(),
                            pPlayers[pParty->getActiveCharacter()]->uLevel,
                            pPlayers[pParty->getActiveCharacter()]->uLevel / 10 + 5
                        );

                        pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                        return;
                    }

                    GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
                    PlayHouseSound(window_SpeakInHouse->wData.val, (HouseSoundID)4);
                    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
                    return;
                }
                label = localization->FormatString(
                    LSTR_XP_UNTIL_NEXT_LEVEL,
                    (unsigned int)(v5 - pPlayers[pParty->getActiveCharacter()]->uExperience),
                    pPlayers[pParty->getActiveCharacter()]->uLevel + 1);
                v36 = (212 - pFontArrus->CalcTextHeight(
                        label, training_dialog_window.uFrameWidth, 0)) / 2 + 88;
            } else {
                label = fmt::format(
                    "{}\n \n{}",
                    localization->GetString(LSTR_TEACHER_LEVEL_TOO_LOW),
                    localization->GetString(LSTR_CANT_TRAIN_FURTHER));
                v36 = (212 - pFontArrus->CalcTextHeight(
                        label, training_dialog_window.uFrameWidth, 0)) / 2 + 101;
            }
            training_dialog_window.DrawTitleText(
                pFontArrus, 0, v36, colorTable.Jonquil.c16(), label, 3);

            PlayHouseSound(window_SpeakInHouse->wData.val, (HouseSoundID)3);
            pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
    }

    //-------------------------------------------------------------
    if (dialog_menu_id == DIALOGUE_LEARN_SKILLS) {
        if (HouseUI_CheckIfPlayerCanInteract()) {
            v14 = (int64_t)(p2DEvents[
                window_SpeakInHouse->wData.val - 1].flt_24 * 500.0);
            pPrice = v14 * (100 - pPlayers[pParty->getActiveCharacter()]->GetMerchant()) / 100;

            if (pPrice < v14 / 3) pPrice = v14 / 3;
            index = 0;
            for (int i = pDialogueWindow->pStartingPosActiveItem;
                (signed int)i < pDialogueWindow->pNumPresenceButton +
                pDialogueWindow->pStartingPosActiveItem;
                ++i) {
                auto skill = GetLearningDialogueSkill(
                    (DIALOGUE_TYPE)pDialogueWindow->GetControl(i)->msg_param
                );
                if (skillMaxMasteryPerClass[pPlayers[pParty->getActiveCharacter()]->classType][skill] != PLAYER_SKILL_MASTERY_NONE
                    && !pPlayers[pParty->getActiveCharacter()]->pActiveSkills[skill]) {
                    all_text_height += pFontArrus->CalcTextHeight(
                        localization->GetSkillName(skill),
                        training_dialog_window.uFrameWidth, 0);
                    ++index;
                }
            }

            SkillTrainingDialogue(&training_dialog_window, index, all_text_height, pPrice);
        }
    }
    return;
}

/**
  * @offset 0x004B6478.
  *
  * @brief Mercenary guild in MM6.
  */
void MercenaryGuildDialog() {
    signed int v3;                // esi@1
    int pPrice;                   // ebx@1
    unsigned int v5;              // esi@5
    short *v6;                       // edi@6
    int all_text_height;          // eax@20
    int pTextHeight;              // eax@29
    int v27;                      // [sp-4h] [bp-80h]@8
    int v32;                      // [sp+6Ch] [bp-10h]@1
    int index;                    // [sp+74h] [bp-8h]@17

    GUIWindow dialog_window = *window_SpeakInHouse;
    dialog_window.uFrameX = 483;
    dialog_window.uFrameWidth = 143;
    dialog_window.uFrameZ = 334;

    v32 = (uint8_t)(((p2DEvents[window_SpeakInHouse->wData.val - 1].uType != BuildingType_MercenaryGuild) - 1) & 0x96) + 100;
    v3 = (int64_t)((double)v32 * p2DEvents[window_SpeakInHouse->wData.val - 1].fPriceMultiplier);
    pPrice = v3 * (100 - pPlayers[pParty->getActiveCharacter()]->GetMerchant()) / 100;
    if (pPrice < v3 / 3) pPrice = v3 / 3;
    if (dialog_menu_id == DIALOGUE_MAIN) {
        if (!_449B57_test_bit((uint8_t *)pPlayers[pParty->getActiveCharacter()]->_achieved_awards_bits, word_4F0754[2 * window_SpeakInHouse->wData.val])) {
            // 171 looks like Mercenary Stronghold message from NPCNews.txt in MM6
            pTextHeight = pFontArrus->CalcTextHeight(pNPCTopics[171].pText, dialog_window.uFrameWidth, 0);
            dialog_window.DrawTitleText(pFontArrus, 0, (212 - pTextHeight) / 2 + 101, colorTable.PaleCanary.c16(), pNPCTopics[171].pText, 3);
            pDialogueWindow->pNumPresenceButton = 0;
            return;
        }
        if (!HouseUI_CheckIfPlayerCanInteract()) return;
        all_text_height = 0;
        index = 0;
        for (int i = pDialogueWindow->pStartingPosActiveItem; i < pDialogueWindow->pNumPresenceButton + pDialogueWindow->pStartingPosActiveItem; ++i) {
            auto skill = GetLearningDialogueSkill((DIALOGUE_TYPE)pDialogueWindow->GetControl(i)->msg_param);
            // Was class type / 3
            if (skillMaxMasteryPerClass[pPlayers[pParty->getActiveCharacter()]->classType][skill] != PLAYER_SKILL_MASTERY_NONE
                && !pPlayers[pParty->getActiveCharacter()]->pActiveSkills[skill]) {
                all_text_height += pFontArrus->CalcTextHeight(localization->GetSkillName(skill), dialog_window.uFrameWidth, 0);
                ++index;
            }
        }


        SkillTrainingDialogue(&dialog_window, index, all_text_height, pPrice);
        return;
    }

    if (HouseUI_CheckIfPlayerCanInteract()) {
        v5 = 0;
        __debugbreak();  // what type of house that even is?
        // pSkillAvailabilityPerClass[8 + v58->uClass][4 + v23]
        // or
        // skillMaxMasteryPerClass[v58->uClass][v23 - 36]
        // or
        // skillMaxMasteryPerClass[v58->uClass - 1][v23 +
        // 1]
        __debugbreak();  // whacky condition - fix
        if (false
            // if ( !*(&byte_4ED94C[37 * v1->uClass / 3] + dword_F8B19C)
            || (v6 = (short *)(&pPlayers[pParty->getActiveCharacter()]->uIntelligence +
                dialog_menu_id),
                *(short *)v6)) {
            pAudioPlayer->playUISound(SOUND_error);
        } else {
            if (pParty->GetGold() < pPrice) {
                GameUI_SetStatusBar(LSTR_NOT_ENOUGH_GOLD);
                v27 = 4;
            } else {
                pParty->TakeGold(pPrice);
                *(short *)v6 = 1;
                v27 = 2;
            }
            PlayHouseSound(window_SpeakInHouse->wData.val, (HouseSoundID)v27);
        }
    } else {
        v5 = 0;
    }
    pCurrentFrameMessageQueue->AddGUIMessage(UIMSG_Escape, 1, v5);
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
    uint16_t pTextColor;  // ax@60
    GUIFont *pTextFont;           // ebx@64
    int pTextHeight;
    GUIWindow w;      // [sp+Ch] [bp-110h]@64
    char *pInString;  // [sp+114h] [bp-8h]@12

    GUIWindow house_window = *pDialogueWindow;
    if (pDialogueNPCCount == uNumDialogueNPCPortraits && uHouse_ExitPic) {
        house_window.uFrameX = 493;
        house_window.uFrameWidth = 126;
        house_window.uFrameZ = 366;
        house_window.DrawTitleText(pFontCreate, 0, 2, 0,
            pMapStats->pInfos[uHouse_ExitPic].pName, 3);
        house_window.uFrameX = 483;
        house_window.uFrameWidth = 143;
        house_window.uFrameZ = 334;
        if (!pTransitionStrings[uHouse_ExitPic]) {
            auto str = localization->FormatString(
                LSTR_FMT_ENTER_S,
                pMapStats->pInfos[uHouse_ExitPic].pName.c_str()
            );
            house_window.DrawTitleText(
                pFontCreate, 0,
                (212 - pFontCreate->CalcTextHeight(
                    str, house_window.uFrameWidth, 0)) /
                2 +
                101,
                0, str, 3);
            return;
        }

        house_window.DrawTitleText(
            pFontCreate, 0,
            (212 -
                pFontCreate->CalcTextHeight(pTransitionStrings[uHouse_ExitPic],
                    house_window.uFrameWidth, 0)) /
            2 +
            101,
            0, pTransitionStrings[uHouse_ExitPic], 3);
        return;
    }
    house_window.uFrameWidth -= 10;
    house_window.uFrameZ -= 10;
    pNPC = HouseNPCData[pDialogueNPCCount + -(dword_591080 != 0)];  //- 1

    house_window.DrawTitleText(pFontCreate, 483, 113, colorTable.EasternBlue.c16(), NameAndTitle(pNPC), 3);

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
                pDialogueWindow->DrawText(pFontArrus, {13, 354 - h}, colorTable.Black.c16(),
                    pFontArrus->FitTextInAWindow(pInString, house_window.uFrameWidth, 13), 0, 0, 0);
            }
        }
    }
    // for right panel
    GUIWindow right_panel_window = *pDialogueWindow;
    right_panel_window.uFrameX = 483;
    right_panel_window.uFrameWidth = 143;
    right_panel_window.uFrameZ = 334;
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
            current_npc_text = bountyHuntingText();
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
        v36 = (174 - all_text_height) / index;
        if (v36 > 32) v36 = 32;
        v40 = (174 - v36 * index - all_text_height) / 2 - v36 / 2 + 138;
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
            pTextColor = colorTable.Jonquil.c16();
            if (pDialogueWindow->pCurrentPosActiveItem != i)
                pTextColor = colorTable.White.c16();
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
        house_window.DrawText(pTextFont, {13, 354 - pTextHeight}, colorTable.Black.c16(), pTextFont->FitTextInAWindow(current_npc_text, w.uFrameWidth, 13), 0, 0, 0);
    }
}

void JailDialog() {
    GUIWindow jail_dialogue_window = *window_SpeakInHouse;
    jail_dialogue_window.uFrameX = 483;
    jail_dialogue_window.uFrameWidth = 143;
    jail_dialogue_window.uFrameZ = 334;
    jail_dialogue_window.DrawTitleText(
        pFontArrus, 0,
        (310 - pFontArrus->CalcTextHeight(localization->GetString(LSTR_ONE_YEAR_SENTENCE),
            jail_dialogue_window.uFrameWidth,
            0)) / 2 + 18, colorTable.PaleCanary.c16(), localization->GetString(LSTR_ONE_YEAR_SENTENCE), 3);
}

void InitializeBuildingResidents() {
    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    p2DEventsTXT_Raw = pEvents_LOD->LoadCompressedTexture("2dEvents.txt").string_view();
    strtok(p2DEventsTXT_Raw.data(), "\r");
    strtok(NULL, "\r");

    for (i = 0; i < 525; ++i) {
        test_string = strtok(NULL, "\r") + 1;
        break_loop = false;
        decode_step = 0;
        do {
            c = *(unsigned char *)test_string;
            temp_str_len = 0;
            while ((c != '\t') && (c > 0)) {
                ++temp_str_len;
                c = test_string[temp_str_len];
            }
            tmp_pos = test_string + temp_str_len;
            if (*tmp_pos == 0) break_loop = true;
            *tmp_pos = 0;
            if (temp_str_len) {
                switch (decode_step) {
                case 2:
                {
                    if (istarts_with(test_string, "wea")) {
                        p2DEvents[i].uType = BuildingType_WeaponShop;
                        break;
                    }
                    if (istarts_with(test_string, "arm")) {
                        p2DEvents[i].uType = BuildingType_ArmorShop;
                        break;
                    }
                    if (istarts_with(test_string, "mag")) {
                        p2DEvents[i].uType = BuildingType_MagicShop;
                        break;
                    }
                    if (istarts_with(test_string, "alc")) {
                        p2DEvents[i].uType = BuildingType_AlchemistShop;
                        break;
                    }
                    if (istarts_with(test_string, "sta")) {
                        p2DEvents[i].uType = BuildingType_Stables;
                        break;
                    }
                    if (istarts_with(test_string, "boa")) {
                        p2DEvents[i].uType = BuildingType_Boats;
                        break;
                    }
                    if (istarts_with(test_string, "tem")) {
                        p2DEvents[i].uType = BuildingType_Temple;
                        break;
                    }
                    if (istarts_with(test_string, "tra")) {
                        p2DEvents[i].uType = BuildingType_Training;
                        break;
                    }
                    if (istarts_with(test_string, "tow")) {
                        p2DEvents[i].uType = BuildingType_TownHall;
                        break;
                    }

                    if (istarts_with(test_string, "tav")) {
                        p2DEvents[i].uType = BuildingType_Tavern;
                        break;
                    }
                    if (istarts_with(test_string, "ban")) {
                        p2DEvents[i].uType = BuildingType_Bank;
                        break;
                    }
                    if (istarts_with(test_string, "fir")) {
                        p2DEvents[i].uType = BuildingType_FireGuild;
                        break;
                    }
                    if (istarts_with(test_string, "air")) {
                        p2DEvents[i].uType = BuildingType_AirGuild;
                        break;
                    }
                    if (istarts_with(test_string, "wat")) {
                        p2DEvents[i].uType = BuildingType_WaterGuild;
                        break;
                    }
                    if (istarts_with(test_string, "ear")) {
                        p2DEvents[i].uType = BuildingType_EarthGuild;
                        break;
                    }
                    if (istarts_with(test_string, "spi")) {
                        p2DEvents[i].uType = BuildingType_SpiritGuild;
                        break;
                    }
                    if (istarts_with(test_string, "min")) {
                        p2DEvents[i].uType = BuildingType_MindGuild;
                        break;
                    }
                    if (istarts_with(test_string, "bod")) {
                        p2DEvents[i].uType = BuildingType_BodyGuild;
                        break;
                    }
                    if (istarts_with(test_string, "lig")) {
                        p2DEvents[i].uType = BuildingType_LightGuild;
                        break;
                    }
                    if (istarts_with(test_string, "dar")) {
                        p2DEvents[i].uType = BuildingType_DarkGuild;
                        break;
                    }
                    if (istarts_with(test_string, "ele")) { // "Element Guild" from mm6
                        p2DEvents[i].uType = BuildingType_ElementalGuild;
                        break;
                    }
                    if (istarts_with(test_string, "sel")) {
                        p2DEvents[i].uType = BuildingType_SelfGuild;
                        break;
                    }
                    if (istarts_with(test_string, "mir")) {
                        p2DEvents[i].uType = BuildingType_MirroredPath;
                        break;
                    }
                    if (istarts_with(test_string, "mer")) { // "Thieves Guild" from mm6
                        p2DEvents[i].uType = BuildingType_TownHall; //TODO: Is this right and not Merc Guild (18)?
                        break;
                    }
                    p2DEvents[i].uType = BuildingType_MercenaryGuild;
                } break;

                case 4:
                    p2DEvents[i].uAnimationID = atoi(test_string);
                    break;
                case 5:
                    p2DEvents[i].pName = removeQuotes(test_string);
                    break;
                case 6:
                    p2DEvents[i].pProprieterName =
                        removeQuotes(test_string);
                    break;
                case 7:
                    p2DEvents[i].pProprieterTitle =
                        removeQuotes(test_string);
                    break;
                case 8:
                    p2DEvents[i].field_14 = atoi(test_string);
                    break;
                case 9:
                    p2DEvents[i]._state = atoi(test_string);
                    break;
                case 10:
                    p2DEvents[i]._rep = atoi(test_string);
                    break;
                case 11:
                    p2DEvents[i]._per = atoi(test_string);
                    break;
                case 12:
                    p2DEvents[i].fPriceMultiplier = atof(test_string);
                    break;
                case 13:
                    p2DEvents[i].flt_24 = atof(test_string);
                    break;
                case 15:
                    p2DEvents[i].generation_interval_days =
                        atoi(test_string);
                    break;
                case 18:
                    p2DEvents[i].uOpenTime = atoi(test_string);
                    break;
                case 19:
                    p2DEvents[i].uCloseTime = atoi(test_string);
                    break;
                case 20:
                    p2DEvents[i].uExitPicID = atoi(test_string);
                    break;
                case 21:
                    p2DEvents[i].uExitMapID = atoi(test_string);
                    break;
                case 22:
                    p2DEvents[i]._quest_bit = atoi(test_string);
                    break;
                case 23:
                    p2DEvents[i].pEnterText = removeQuotes(test_string);
                    break;
                }
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 24) && !break_loop);
    }
}

int HouseDialogPressCloseBtn() {
    pCurrentFrameMessageQueue->Flush();
    keyboardInputHandler->SetWindowInputStatus(WindowInputStatus::WINDOW_INPUT_CANCELLED);
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
                    UIMSG_ClickHouseNPCPortrait, i, InputAction::Invalid, byte_591180[i].data());
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

void BackToHouseMenu() {
    auto pMouse = EngineIocContainer::ResolveMouse();
    pMouse->ClearPickedItem();
    if (window_SpeakInHouse && window_SpeakInHouse->wData.val == 165 &&
        !pMovie_Track) {
        bGameoverLoop = true;
        HouseDialogPressCloseBtn();
        window_SpeakInHouse->Release();
        pParty->uFlags &= 0xFFFFFFFD;
        if (EnterHouse(HOUSE_BODY_GUILD_ERATHIA)) {
            pAudioPlayer->playUISound(SOUND_Invalid);
            window_SpeakInHouse = new GUIWindow_House({0, 0}, render->GetRenderDimensions(), HOUSE_BODY_GUILD_ERATHIA, "");
            window_SpeakInHouse->CreateButton({61, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 1, InputAction::SelectChar1);
            window_SpeakInHouse->CreateButton({177, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 2, InputAction::SelectChar2);
            window_SpeakInHouse->CreateButton({292, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 3, InputAction::SelectChar3);
            window_SpeakInHouse->CreateButton({407, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 4, InputAction::SelectChar4);
        }
        bGameoverLoop = false;
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
                    v21 = shopWeap_variation_spc
                    [window_SpeakInHouse->wData.val].item_class[j];
                else
                    v21 = shopWeap_variation_ord
                    [window_SpeakInHouse->wData.val].item_class[j];

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
                    if (i)
                        v15 = shopArmr_variation_spc
                        [window_SpeakInHouse->wData.val - 15 + j].item_class[k];
                    else
                        v15 = shopArmr_variation_ord
                        [window_SpeakInHouse->wData.val - 15 + j].item_class[k];

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

//----- (004B8F94) --------------------------------------------------------
void GenerateSpecialShopItems() {
    signed int item_count;
    signed int shop_index;
    ITEM_TREASURE_LEVEL treasure_lvl = ITEM_TREASURE_LEVEL_INVALID;
    int item_class = 0;
    int mdf;

    shop_index = window_SpeakInHouse->wData.val;
    if (ItemAmountForShop(p2DEvents[shop_index - 1].uType)) {
        for (item_count = 0; item_count < ItemAmountForShop(p2DEvents[shop_index - 1].uType); ++item_count) {
            if (shop_index <= 14) {  // weapon shop
                treasure_lvl =
                    shopWeap_variation_spc[shop_index].treasure_level;
                item_class =
                    shopWeap_variation_spc[shop_index].item_class[grng->random(4)];
            } else if (shop_index <= 28) {  // armor shop
                mdf = 0;
                if (item_count > 3) ++mdf;
                treasure_lvl =
                    shopArmr_variation_spc[2 * (shop_index - 15) + mdf]
                    .treasure_level;
                item_class = shopArmr_variation_spc[2 * (shop_index - 15) + mdf]
                    .item_class[grng->random(4)];
            } else if (shop_index <= 41) {  // magic shop
                treasure_lvl = shopMagicSpc_treasure_lvl[shop_index - 28];
                item_class = 22;          // misc
            } else if (shop_index <= 53) {  // alchemist shop
                if (item_count < 6) {
                    pParty->SpecialItemsInShops[shop_index][item_count].Reset();
                    pParty->SpecialItemsInShops[shop_index][item_count]
                        .uItemID = grng->randomSample(RecipeScrolls());  // mscrool
                    continue;
                } else {
                    treasure_lvl = shopAlchSpc_treasure_lvl[shop_index - 41];
                    item_class = 44;  // potion
                }
            }
            pItemTable->GenerateItem(
                treasure_lvl, item_class,
                &pParty->SpecialItemsInShops[shop_index][item_count]);
            pParty->SpecialItemsInShops[shop_index][item_count]
                .SetIdentified();  // identified
        }
    }
    pParty->InTheShopFlags[shop_index] = 0;
}

//----- (004B8E3D) --------------------------------------------------------
void GenerateStandartShopItems() {
    signed int item_count;
    signed int shop_index;
    ITEM_TREASURE_LEVEL treasure_lvl = ITEM_TREASURE_LEVEL_INVALID;
    int item_class = 0;
    int mdf;

    shop_index = window_SpeakInHouse->wData.val;
    if (ItemAmountForShop(p2DEvents[shop_index - 1].uType)) {
        for (item_count = 0; item_count < ItemAmountForShop(p2DEvents[shop_index - 1].uType); ++item_count) {
            if (shop_index <= 14) {  // weapon shop
                treasure_lvl =
                    shopWeap_variation_ord[shop_index].treasure_level;
                item_class =
                    shopWeap_variation_ord[shop_index].item_class[grng->random(4)];
            } else if (shop_index <= 28) {  // armor shop
                mdf = 0;
                if (item_count > 3) ++mdf;  // rechek offsets
                treasure_lvl =
                    shopArmr_variation_ord[2 * (shop_index - 15) + mdf]
                    .treasure_level;
                item_class = shopArmr_variation_ord[2 * (shop_index - 15) + mdf]
                    .item_class[grng->random(4)];
            } else if (shop_index <= 41) {  // magic shop
                treasure_lvl = shopMagic_treasure_lvl[shop_index - 28];
                item_class = 22;          // misc
            } else if (shop_index <= 53) {  // alchemist shop
                if (item_count < 6) {
                    pParty->StandartItemsInShops[shop_index][item_count]
                        .Reset();
                    pParty->StandartItemsInShops[shop_index][item_count]
                        .uItemID = ITEM_POTION_BOTTLE;  // potion bottle
                    continue;
                } else {
                    treasure_lvl = shopAlch_treasure_lvl[shop_index - 41];
                    item_class = 45;  // reagent
                }
            }
            pItemTable->GenerateItem(
                treasure_lvl, item_class,
                &pParty->StandartItemsInShops[shop_index][item_count]);
            pParty->StandartItemsInShops[shop_index][item_count]
                .SetIdentified();  // identified
        }
    }
    pParty->InTheShopFlags[shop_index] = 0;
}

GUIWindow_House::GUIWindow_House(Pointi position, Sizei dimensions, HOUSE_ID houseId, const std::string &hint) :
    GUIWindow(WINDOW_HouseInterior, position, dimensions, houseId, hint) {
    pEventTimer->Pause();  // pause timer so not attacked
    // pAudioPlayer->PauseSounds(-1);

    current_screen_type = CURRENT_SCREEN::SCREEN_HOUSE;
    pBtn_ExitCancel = CreateButton({471, 445}, {169, 35}, 1, 0, UIMSG_Escape, 0, InputAction::Invalid, localization->GetString(LSTR_EXIT_BUILDING),
        {ui_exit_cancel_button_background});
    for (int v26 = 0; v26 < uNumDialogueNPCPortraits; ++v26) {
        const char *v29;
        std::string v30;
        if (v26 + 1 == uNumDialogueNPCPortraits && uHouse_ExitPic) {
            v30 = pMapStats->pInfos[uHouse_ExitPic].pName;
            v29 = localization->GetString(LSTR_FMT_ENTER_S);
        } else {
            if (v26 || !dword_591080)
                v30 = HouseNPCData[v26 + 1 - ((dword_591080 != 0) ? 1 : 0)]->pName;
            else
                v30 = p2DEvents[houseId - 1].pProprieterName;
            v29 = localization->GetString(LSTR_FMT_CONVERSE_WITH_S);
        }
        sprintf(byte_591180[v26].data(), v29, v30.c_str());
        HouseNPCPortraitsButtonsList[v26] = CreateButton(
            {pNPCPortraits_x[uNumDialogueNPCPortraits - 1][v26], pNPCPortraits_y[uNumDialogueNPCPortraits - 1][v26]}, {63, 73}, 1, 0,
            UIMSG_ClickHouseNPCPortrait, v26, InputAction::Invalid, byte_591180[v26].data());
    }
    if (uNumDialogueNPCPortraits == 1) {
        window_SpeakInHouse = this;
        _4B4224_UpdateNPCTopics(0);
    }
}

void GUIWindow_House::Update() {
    HouseDialogManager();
    if (!window_SpeakInHouse)
        return;
    if (window_SpeakInHouse->wData.val >= 53)
        return;
    if (pParty->PartyTimes._shop_ban_times[window_SpeakInHouse->wData.val] <= pParty->GetPlayingTime()) {
        if (window_SpeakInHouse->wData.val < 53)
            pParty->PartyTimes._shop_ban_times[window_SpeakInHouse->wData.val] = GameTime(0);
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
