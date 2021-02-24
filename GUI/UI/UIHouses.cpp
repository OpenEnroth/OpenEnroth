#include <stdlib.h>

#include "src/Application/GameOver.h"

#include "Arcomage/Arcomage.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Events.h"
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

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIGuilds.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIPartyCreation.h"
#include "GUI/UI/UIShops.h"
#include "GUI/UI/UIStatusBar.h"

#include "Io/Mouse.h"
#include "Io/KeyboardInputHandler.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

#include "Platform/Api.h"
#include "Platform/OSWindow.h"


using Io::TextInputType;
using EngineIoc = Engine_::IocContainer;

int uHouse_ExitPic;
int _F8B1DC_currentShopOption;  // F8B1DC
int dword_591080;               // 591080

BuildingType in_current_building_type;  // 00F8B198
HOUSE_DIALOGUE_MENU dialog_menu_id;     // 00F8B19C

Image *_591428_endcap = nullptr;

void GenerateStandartShopItems();
void GenerateSpecialShopItems();

#pragma pack(push, 1)
struct stru365_travel_info {
    unsigned char uMapInfoID;
    unsigned char pSchedule[7];
    unsigned int uTravelTime;
    int arrival_x;
    int arrival_y;
    int arrival_z;
    int arrival_rot_y;
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
    { MAP_EVENMORN_ISLE, { 0, 0, 0, 0, 0, 0, 1 }, 7, 15616, 6390, 193, 1536, PARTY_QUEST_EVENMORN_MAP_FOUND },
    { MAP_BRAKADA_DESERT, { 0, 0, 1, 0, 0, 0, 0 }, 6, 19171, -19722, 193, 1024, 0 },
    { MAP_AVLEE, { 0, 1, 0, 1, 0, 1, 0 }, 3, 7913, 9476, 193, 0, 0 },
    { MAP_BRAKADA_DESERT, { 1, 0, 1, 0, 0, 0, 0 }, 6, 19171, -19722, 193, 1024, 0 },
    { MAP_TATALIA, { 1, 0, 1, 0, 1, 0, 0 }, 4, -2183, -6941, 97, 0, 0 },
    { MAP_PIERPONT, { 0, 0, 0, 0, 0, 1, 0 }, 6, -709, -14087, 193, 1024, 0 },  // for boat
    { MAP_STEADWICK, { 0, 0, 0, 0, 0, 0, 1 }, 6, -10471, 13497, 193, 1536, 0 },
    { MAP_EVENMORN_ISLE, { 0, 1, 0, 1, 0, 0, 0 }, 1, 15616, 6390, 193, 1536, PARTY_QUEST_EVENMORN_MAP_FOUND },
    { MAP_BRAKADA_DESERT, { 0, 1, 0, 1, 0, 0, 0 }, 1, 19171, -19722, 193, 1024, 0 },
    { MAP_STEADWICK, { 0, 1, 0, 1, 0, 1, 0 }, 2, -10471, 13497, 193, 1536, 0 },
    { MAP_BRAKADA_DESERT, { 1, 0, 1, 0, 0, 0, 0 }, 4, 19171, -19722, 193, 1024, 0 },
    { MAP_EVENMORN_ISLE, { 0, 0, 0, 0, 0, 0, 1 }, 5, 15616, 6390, 193, 1536, PARTY_QUEST_EVENMORN_MAP_FOUND },
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
    { "", 0x4, 0x1F4, 0, 0, 0 },
    { "Human Armor01", 0x20, 0x2C0, 2, 58, 0 },
    { "Necromancer Armor01", 0x20, 0x2D7, 2, 70, 0 },
    { "Dwarven Armor01", 0x20, 0x2EE, 2, 5, 0 },
    { "Wizard Armor", 0x20, 0x3BD, 2, 19, 0 },
    { "Warlock Armor", 0x20, 0x2D6, 2, 35, 0 },
    { "Elf Armor", 0x20, 0x2BC, 2, 79, 0 },
    { "Human Alchemisht01", 0xE, 0x2BE, 4, 95, 0 },
    { "Necromancer Alchemist01", 0xE, 0x2D6, 4, 69, 0 },
    { "Dwarven Achemist01", 0xE, 0x387, 4, 4, 0 },
    { "Wizard Alchemist", 0xE, 0x232, 4, 25, 0 },
    { "Warlock Alchemist", 0xE, 0x2BE, 4, 42, 0 },
    { "Elf Alchemist", 0xE, 0x38A, 4, 84, 0 },
    { "Human Bank01", 0x6, 0x384, 22, 52, 0 },
    { "Necromancer Bank01", 0x6, 0x2D8, 22, 71, 0 },
    { "Dwarven Bank", 0x6, 0x2F3, 22, 6, 0 },
    { "Wizard Bank", 0x6, 0x3BA, 22, 20, 0 },
    { "Warlock Bank", 0x6, 0x39F, 22, 36, 0 },
    { "Elf Bank", 0x6, 0x2BC, 22, 71, 0 },
    { "Boat01", 0xF, 0x4C, 28, 53, 3 },
    { "Boat01d", 0xF, 0x4C, 28, 53, 3 },
    { "Human Magic Shop01", 0xA, 0x2C8, 3, 54, 0 },
    { "Necromancer Magic Shop01", 0xE, 0x2DC, 3, 66, 0 },
    { "Dwarven Magic Shop01", 0x2A, 0x2EF, 3, 91, 0 },
    { "Wizard Magic Shop", 0x1E, 0x2DF, 3, 15, 0 },
    { "Warlock Magic Shop", 0x7, 0x3B9, 3, 15, 0 },
    { "Elf Magic Shop", 0x24, 0x2CC, 3, 82, 0 },
    { "Human Stables01", 0x21, 0x31, 27, 48, 3 },
    { "Necromancer Stables", 0x21, 0x2DD, 27, 67, 3 },
    { "", 0x21, 0x2F0, 27, 91, 3 },
    { "Wizard Stables", 0x21, 0x3BA, 27, 16, 3 },
    { "Warlock Stables", 0x21, 0x181, 27, 77, 3 },
    { "Elf Stables", 0x21, 0x195, 27, 77, 3 },
    { "Human Tavern01", 0xD, 0x2C2, 21, 49, 0 },
    { "Necromancer Tavern 01", 0xD, 0x3B0, 21, 57, 0 },
    { "Dwarven Tavern01", 0xD, 0x2FE, 21, 94, 0 },
    { "Wizard Tavern", 0xD, 0x3BB, 21, 17, 0 },
    { "Warlock Tavern", 0xD, 0x3A8, 21, 33, 0 },
    { "Elf Tavern", 0xD, 0x2CD, 21, 78, 0 },
    { "Human Temple01", 0x24, 0x2DB, 23, 50, 3 },
    { "Necromancer Temple", 0x24, 0x2DF, 23, 60, 3 },
    { "Dwarven Temple01", 0x24, 0x2F1, 23, 86, 3 },
    { "Wizard Temple", 0x24, 0x2E0, 23, 10, 3 },
    { "Warlock Temple", 0x24, 0x3A4, 23, 27, 3 },
    { "Elf Temple", 0x24, 0x2CE, 23, 72, 3 },
    { "Human Town Hall", 0x10, 0x39C, 17, 14, 0 },
    { "Necromancer Town Hall01", 0x10, 0x3A4, 17, 61, 0 },
    { "Dwarven Town Hall", 0x10, 0x2DB, 17, 88, 0 },
    { "Wizard Town Hall", 0x10, 0x3BD, 17, 11, 0 },
    { "Warlock Town Hall", 0x10, 0x2DB, 17, 28, 0 },
    { "Elf Town Hall", 0x10, 0x27A, 17, 73, 0 },
    { "Human Training Ground01", 0x18, 0x2C7, 30, 44, 0 },
    { "Necromancer Training Ground", 0x18, 0x3AD, 30, 62, 0 },
    { "Dwarven Training Ground", 0x18, 0x2F2, 30, 89, 0 },
    { "Wizard Training Ground", 0x18, 0x3A3, 30, 12, 0 },
    { "Warlock Training Ground", 0x18, 0x3A6, 30, 29, 0 },
    { "Elf Training Ground", 0x18, 0x19F, 30, 74, 0 },
    { "Human Weapon Smith01", 0x16, 0x2C1, 1, 45, 4 },
    { "Necromancer Weapon Smith01", 0x16, 0x2D9, 1, 63, 4 },
    { "Dwarven Weapon Smith01", 0x16, 0x2EE, 1, 82, 4 },
    { "Wizard Weapon Smith", 0x16, 0x2D5, 1, 13, 4 },
    { "Warlock Weapon Smith", 0x16, 0x2D7, 1, 23, 4 },
    { "Elf Weapon Smith", 0x16, 0x2CA, 1, 75, 4 },
    { "Air Guild", 0x1D, 0xA4, 6, 1, 3 },
    { "Body Guild", 0x19, 0x3BF, 11, 2, 0 },
    { "Dark Guild", 0x19, 0x2D1, 13, 3, 0 },
    { "Earth Guild", 0x19, 0x2CB, 8, 83, 0 },
    { "Fire Guild", 0x1C, 0x2BF, 5, 56, 0 },
    { "Light Guild", 0x1C, 0x2D5, 12, 46, 0 },
    { "Mind Guild", 0x1C, 0xE5, 10, 40, 0 },
    { "Spirit Guild", 0x1C, 0x2D2, 9, 41, 0 },
    { "Water Guild", 0x1B, 0x2D3, 7, 24, 0 },
    { "Lord And Judge Out01", 1, 0, 29, 39, 0 },
    { "Human Poor House 1", 8, 0, 29, 0, 0 },
    { "Human Poor House 2", 8, 0, 29, 0, 0 },
    { "Human Poor House 3", 8, 0, 29, 0, 0 },
    { "Human Medium House 1", 8, 0, 29, 0, 0 },
    { "Human Medium House 2", 8, 0, 29, 0, 0 },
    { "Human Medium House 3", 8, 0, 29, 0, 0 },
    { "Human Rich House 1", 8, 0, 29, 0, 0 },
    { "Human Rich House 2", 8, 0, 29, 0, 0 },
    { "Human Rich House 3", 8, 0, 29, 0, 0 },
    { "Elven Poor House 1", 8, 0, 29, 0, 0 },
    { "Elven Poor House 2", 8, 0, 29, 0, 0 },
    { "Elven Poor House 3", 8, 0, 29, 0, 0 },
    { "Elven Medium House 1", 8, 0, 29, 0, 0 },
    { "Elven Medium House 2", 8, 0, 29, 0, 0 },
    { "Elven Medium House 3", 8, 0, 29, 0, 0 },
    { "Elven Rich House 1", 8, 0, 29, 0, 0 },
    { "Elven Rich House 2", 8, 0, 29, 0, 0 },
    { "Elven Rich House 3", 8, 0, 29, 0, 0 },
    { "Dwarven Poor House 1", 8, 0, 29, 0, 0 },
    { "Dwarven Poor House 2", 8, 0, 29, 0, 0 },
    { "Dwarven Poor House 3", 8, 0, 29, 0, 0 },
    { "Dwarven Medium House 1", 8, 0, 29, 0, 0 },
    { "Dwarven Medium house 2", 8, 0, 29, 0, 0 },
    { "Dwarven Medium House 3", 8, 0, 29, 0, 0 },
    { "Dwarven Rich House 1", 8, 0, 29, 0, 0 },
    { "Dwarven Rich House 2", 8, 0, 29, 0, 0 },
    { "Dwarven Rich House 3", 8, 0, 29, 0, 0 },
    { "Wizard Poor House 1", 8, 0, 29, 0, 0 },
    { "Wizard Poor House 2", 8, 0, 29, 0, 0 },
    { "Wizard Poor House 3", 8, 0, 29, 0, 0 },
    { "Wizard Medium House 1", 8, 0, 29, 0, 0 },
    { "Wizard Medium House 2", 8, 0, 29, 0, 0 },
    { "Wizard Mdeium House 3", 8, 0, 29, 0, 0 },
    { "Wizard Rich House 1", 8, 0, 29, 0, 0 },
    { "Wizard Rich House 2", 8, 0, 29, 0, 0 },
    { "Wizard Rich House 3", 8, 0, 29, 0, 0 },
    { "Necromancer Poor House 1", 8, 0, 29, 0, 0 },
    { "Necromancer Poor House 2", 8, 0, 29, 0, 0 },
    { "Necromancer Poor House 3", 8, 0, 29, 0, 0 },
    { "Necromancer Medium House 1", 8, 0, 29, 0, 0 },
    { "Necromancer Medium House 2", 8, 0, 29, 0, 0 },
    { "Necromancer Mdeium House 3", 8, 0, 29, 0, 0 },
    { "Necromancer Rich House 1", 8, 0, 29, 0, 0 },
    { "Necromancer Rich House 2", 8, 0, 29, 0, 0 },
    { "Necromancer Rich House 3", 8, 0, 29, 0, 0 },
    { "Warlock Poor House 1", 8, 0, 29, 0, 0 },
    { "Warlock Poor House 2", 8, 0, 29, 0, 0 },
    { "Warlock Poor House 3", 8, 0, 29, 0, 0 },
    { "Warlock Medium House 1", 8, 0, 29, 0, 0 },
    { "Warlock Medium House 2", 8, 0, 29, 0, 0 },
    { "Warlock Mdeium House 3", 8, 0, 29, 0, 0 },
    { "Warlock Rich House 1", 8, 0, 29, 0, 0 },
    { "Warlock Rich House 2", 8, 0, 29, 0, 0 },
    { "Warlock Rich House 3", 8, 0, 29, 0, 0 },
    { "Out01 Temple of the Moon", 0x24, 0, 25, 0, 0 },
    { "Out01 Dragon Cave", 0x24, 0, 25, 0, 0 },
    { "Out02 Castle Harmondy", 0x24, 0, 25, 0, 0 },
    { "Out02 White Cliff Cave", 0x24, 0, 25, 0, 0 },
    { "Out03 Erathian Sewer", 0x24, 0, 25, 0, 0 },
    { "Out03 Fort Riverstride", 0x24, 0, 25, 0, 0 },
    { "Out03 Castle Gryphonheart", 0x24, 0, 25, 0, 0 },
    { "Out04 Elf Castle", 0x24, 0, 25, 0, 0 },
    { "Out04 Tularean Caves", 0x24, 0, 25, 0, 0 },
    { "Out04 Clanker's Laboratory", 0x24, 0, 25, 0, 0 },
    { "Out05 Hall of the Pit", 0x24, 0, 25, 0, 0 },
    { "Out05 Watchtower 6", 0x24, 0, 25, 0, 0 },
    { "Out06 School of Sorcery", 0x24, 0, 25, 0, 0 },
    { "Out06 Red Dwarf Mines", 0x24, 0, 25, 0, 0 },
    { "Out07 Castle Lambert", 0x24, 0, 25, 0, 0 },
    { "Out07 Walls of Mist", 0x24, 0, 25, 0, 0 },
    { "Out07 Temple of the Light", 0x24, 0, 25, 0, 0 },
    { "Out08 Evil Entrance", 0x24, 0, 25, 0, 0 },
    { "Out08 Breeding Zone", 0x24, 0, 25, 0, 0 },
    { "Out08 Temple of the Dark", 0x24, 0, 25, 0, 0 },
    { "Out09 Grand Temple of the Moon", 0x24, 0, 25, 0, 0 },
    { "Out09 Grand Tempple of the Sun", 0x24, 0, 25, 0, 0 },
    { "Out10 Thunderfist Mountain", 0x24, 0, 25, 0, 0 },
    { "Out10 The Maze", 0x24, 0, 25, 0, 0 },
    { "Out10 Connecting Tunnel Cave #1", 0x24, 0, 25, 0, 0 },
    { "", 0x24, 0, 25, 0, 0 },
    { "", 0x24, 0, 25, 0, 0 },
    { "Out11 Stone City", 0x24, 0, 25, 0, 0 },
    { "", 0x24, 0, 25, 0, 0 },
    { "Out12 Colony Zod", 0x24, 0, 25, 0, 0 },
    { "Out12 Connecting Tunnel Cave #1", 0x24, 0, 25, 0, 0 },
    { "", 0x24, 0, 25, 0, 0 },
    { "", 0x24, 0, 25, 0, 0 },
    { "Out13 Mercenary Guild", 0x24, 0, 25, 0, 0 },
    { "Out13 Tidewater Caverns", 0x24, 0, 25, 0, 0 },
    { "Out13 Wine Cellar", 0x24, 0, 25, 0, 0 },
    { "Out14 Titan's Stronghold", 0x24, 0, 25, 0, 0 },
    { "Out14 Temple Of Baa", 0x24, 0, 25, 0, 0 },
    { "Out14 Hall under the Hill", 0x24, 0, 25, 0, 0 },
    { "Out15 The Linclon", 0x24, 0, 25, 0, 0 },
    { "Jail", 0x24, 0, 31, 0, 0 },
    { "Harmondale Throne Room", 0x24, 0, 29, 0, 0 },
    { "Gryphonheart Throne Room", 0x24, 0, 20, 0, 0 },
    { "Elf Castle Throne Room", 0x24, 0, 20, 0, 0 },
    { "Wizard Castle Throne Room", 0x24, 0, 20, 0, 0 },
    { "Necromancer Casstle Trone Room", 0x24, 0, 20, 0, 0 },
    { "Master Thief", 0x24, 0, 29, 0, 0 },
    { "Dwarven King", 0x24, 0, 20, 0, 0 },
    { "Arms Master", 0x24, 0, 29, 0, 0 },
    { "Warlock", 0x24, 0, 29, 0, 0 },
    { "Lord Markam", 0x24, 0, 20, 0, 0 },
    { "Arbiter Neutral Town", 0x24, 0, 29, 0, 0 },
    { "Arbiter Good Town", 0x24, 0, 29, 0, 0 },
    { "Arbiter Evil Town", 0x24, 0, 29, 0, 0 },
    { "Necromancer Throne Room Empty", 0x24, 0, 20, 0, 0 },
    { "", 0x24, 0, 29, 0, 0 },
    { "", 0x24, 0, 29, 0, 0 },
    { "", 0x24, 0, 29, 0, 0 },
    { "", 0x24, 0, 29, 0, 0 },
    { "Boat01", 0xF, 0, 29, 53, 3 },
    { "", 0x24, 0, 28, 0, 0 },
    { "", 0x24, 0, 28, 0, 0 },
    { "", 0x24, 0, 28, 0, 0 },
    { "", 0x24, 0, 29, 0, 0 },
    { "Arbiter Room Neutral", 0x24, 0, 29, 0, 0 },
    { "Out02 Castle Harmondy Abandoned", 0x24, 0, 25, 0, 0 },
    { "Human Temple02", 0x24, 0x3AB, 23, 27, 0 },
    { "Player Castle Good", 0x24, 0, 25, 0, 0 },
    { "Player Castle Bad", 0x24, 0, 25, 0, 0 }
} };

static std::array<const char *, 19> _4F03B8_shop_background_names = {
    { "", "WEPNTABL", "ARMORY", "MAGSHELF", "MAGSHELF", "MAGSHELF", "MAGSHELF",
    "MAGSHELF", "MAGSHELF", "MAGSHELF", "MAGSHELF", "MAGSHELF", "MAGSHELF",
    "MAGSHELF", "MAGSHELF", "MAGSHELF", "MAGSHELF", "MAGSHELF", "MAGSHELF" } };

struct ITEM_VARIATION {
    unsigned __int16 treasure_level;
    unsigned __int16 item_class[4];
};

std::array<const char, 5> uItemsAmountPerShopType = { 0, 6, 8, 12, 12 };

const ITEM_VARIATION shopWeap_variation_ord[15] = {
    { 0, { 00, 00, 00, 00 } }, { 1, { 23, 27, 20, 20 } }, { 1, { 23, 24, 28, 20 } },
    { 2, { 23, 24, 25, 20 } }, { 2, { 27, 27, 26, 26 } }, { 4, { 24, 30, 25, 27 } },
    { 4, { 24, 30, 25, 27 } }, { 3, { 30, 24, 20, 20 } }, { 2, { 20, 20, 20, 20 } },
    { 3, { 27, 27, 26, 26 } }, { 3, { 28, 28, 25, 25 } }, { 2, { 23, 23, 24, 24 } },
    { 3, { 23, 23, 26, 26 } }, { 2, { 30, 26, 26, 26 } }, { 2, { 28, 25, 28, 29 } }
};

const ITEM_VARIATION shopArmr_variation_ord[28] = {
    { 1, { 35, 35, 38, 38 } }, { 1, { 31, 31, 31, 34 } }, { 1, { 35, 35, 38, 38 } },
    { 1, { 31, 31, 32, 34 } }, { 2, { 35, 35, 38, 38 } }, { 2, { 31, 32, 32, 33 } },
    { 2, { 35, 35, 38, 38 } }, { 2, { 31, 31, 32, 32 } }, { 4, { 35, 35, 38, 38 } },
    { 4, { 31, 32, 33, 34 } }, { 4, { 35, 35, 38, 38 } }, { 4, { 31, 32, 33, 34 } },
    { 3, { 35, 35, 38, 38 } }, { 3, { 31, 31, 31, 31 } }, { 2, { 35, 35, 38, 38 } },
    { 2, { 31, 32, 34, 34 } }, { 3, { 35, 35, 38, 38 } }, { 3, { 31, 31, 32, 32 } },
    { 3, { 35, 35, 38, 38 } }, { 3, { 32, 32, 32, 33 } }, { 3, { 35, 35, 38, 38 } },
    { 3, { 31, 31, 31, 32 } }, { 3, { 35, 35, 38, 38 } }, { 3, { 33, 31, 32, 34 } },
    { 3, { 35, 35, 38, 38 } }, { 3, { 33, 31, 32, 34 } }, { 4, { 35, 35, 38, 38 } },
    { 4, { 33, 31, 32, 34 } }
};

const unsigned __int16 shopMagic_treasure_lvl[14] = { 0, 1, 1, 2, 2, 4, 4,
3, 2, 2, 2, 2, 2, 2 };
const unsigned __int16 shopAlch_treasure_lvl[13] = { 0, 1, 1, 2, 2, 3, 3,
4, 4, 2, 2, 2, 2 };

const ITEM_VARIATION shopWeap_variation_spc[15] = {
    { 0, { 00, 00, 00, 00 } }, { 2, { 25, 30, 20, 20 } }, { 2, { 23, 24, 28, 20 } },
    { 3, { 23, 24, 25, 20 } }, { 3, { 27, 27, 26, 26 } }, { 5, { 23, 26, 28, 27 } },
    { 5, { 23, 26, 28, 27 } }, { 4, { 30, 24, 20, 20 } }, { 3, { 20, 20, 20, 20 } },
    { 4, { 27, 27, 26, 26 } }, { 4, { 28, 28, 25, 25 } }, { 4, { 23, 23, 24, 24 } },
    { 4, { 24, 24, 27, 20 } }, { 4, { 30, 26, 26, 26 } }, { 4, { 28, 25, 28, 29 } }
};

const unsigned __int16 shopMagicSpc_treasure_lvl[14] = { 0, 2, 2, 3, 3, 5, 5,
4, 3, 3, 3, 3, 3, 3 };
const unsigned __int16 shopAlchSpc_treasure_lvl[13] = { 0, 2, 2, 3, 3, 4, 4,
5, 5, 3, 2, 2, 2 };

const ITEM_VARIATION shopArmr_variation_spc[28] = {
    { 2, { 35, 35, 38, 38 } }, { 2, { 31, 31, 31, 34 } }, { 2, { 35, 35, 38, 38 } },
    { 2, { 31, 31, 32, 34 } }, { 3, { 35, 35, 38, 38 } }, { 3, { 31, 32, 32, 33 } },
    { 3, { 35, 35, 38, 38 } }, { 3, { 31, 31, 32, 32 } }, { 5, { 35, 35, 38, 38 } },
    { 5, { 31, 32, 33, 34 } }, { 5, { 35, 35, 38, 38 } }, { 5, { 31, 32, 33, 34 } },
    { 4, { 35, 35, 38, 38 } }, { 4, { 31, 31, 31, 31 } }, { 3, { 35, 35, 38, 38 } },
    { 3, { 31, 32, 34, 34 } }, { 4, { 35, 35, 38, 38 } }, { 4, { 31, 31, 32, 33 } },
    { 4, { 35, 35, 38, 38 } }, { 4, { 32, 32, 33, 34 } }, { 4, { 35, 35, 38, 38 } },
    { 4, { 31, 31, 31, 32 } }, { 4, { 35, 35, 38, 38 } }, { 4, { 32, 32, 32, 32 } },
    { 4, { 35, 35, 38, 38 } }, { 4, { 34, 34, 34, 34 } }, { 5, { 35, 35, 38, 38 } },
    { 5, { 33, 33, 33, 33 } }
};

void FillAviableSkillsToTeach(enum BuildingType type);

//----- (004B3A72) --------------------------------------------------------
void InitializaDialogueOptions_Tavern(BuildingType type) {
    int num_buttons;  // esi@1

    num_buttons = 0;
    if (type == BuildingType_Tavern) {
        num_buttons = 2;
        CreateButtonInColumn(0, 102);
        CreateButtonInColumn(1, 103);
        if (pParty->HasItem(651)) {  // Arcomage Deck
            num_buttons = 3;
            CreateButtonInColumn(2, 104);
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
    {
        CreateButtonInColumn(0, 3);
        CreateButtonInColumn(1, 4);
        CreateButtonInColumn(2, 5);
        pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
    } break;

    case BuildingType_AlchemistShop:
    {
        CreateButtonInColumn(0, 3);
        CreateButtonInColumn(1, 4);
        pDialogueWindow->_41D08F_set_keyboard_control_group(2, 1, 0, 2);
    } break;
    }

    dword_F8B1E0 = pDialogueWindow->pNumPresenceButton;
}
// F8B1E0: using guessed type int dword_F8B1E0;

//----- (004B3B42) --------------------------------------------------------
void InitializaDialogueOptions(BuildingType type) {
    switch (type) {
    case BuildingType_WeaponShop:
    case BuildingType_ArmorShop:
    case BuildingType_MagicShop:
    case BuildingType_AlchemistShop:
    {
        CreateButtonInColumn(0, 2);
        CreateButtonInColumn(1, 95);
        CreateButtonInColumn(2, 94);
        CreateButtonInColumn(3, 96);
        pDialogueWindow->_41D08F_set_keyboard_control_group(4, 1, 0, 2);
    } break;

    case BuildingType_FireGuild:
    {
        CreateButtonInColumn(0, 18);
        CreateButtonInColumn(1, 48);
        CreateButtonInColumn(2, 72);
        pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
    } break;

    case BuildingType_AirGuild:
    {
        CreateButtonInColumn(0, 18);
        CreateButtonInColumn(1, 49);
        CreateButtonInColumn(2, 72);
        pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
    } break;

    case BuildingType_WaterGuild:
    {
        CreateButtonInColumn(0, 18);
        CreateButtonInColumn(1, 50);
        CreateButtonInColumn(2, 72);
        pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
    } break;

    case BuildingType_EarthGuild:
    {
        CreateButtonInColumn(0, 18);
        CreateButtonInColumn(1, 51);
        CreateButtonInColumn(2, 72);
        pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
    } break;

    case BuildingType_SpiritGuild:
    {
        CreateButtonInColumn(0, 18);
        CreateButtonInColumn(1, 52);
        CreateButtonInColumn(2, 61);
        pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
    } break;

    case BuildingType_MindGuild:
    {
        CreateButtonInColumn(0, 18);
        CreateButtonInColumn(1, 53);
        CreateButtonInColumn(2, 61);
        pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
    } break;

    case BuildingType_BodyGuild:
    {
        CreateButtonInColumn(0, 18);
        CreateButtonInColumn(1, 54);
        CreateButtonInColumn(2, 61);
        pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
    } break;

    case BuildingType_LightGuild:
    {
        CreateButtonInColumn(0, 18);
        CreateButtonInColumn(1, 55);
        pDialogueWindow->_41D08F_set_keyboard_control_group(2, 1, 0, 2);
    } break;

    case BuildingType_DarkGuild:
    {
        CreateButtonInColumn(0, 18);
        CreateButtonInColumn(1, 56);
        pDialogueWindow->_41D08F_set_keyboard_control_group(2, 1, 0, 2);
    } break;

    case BuildingType_ElementalGuild:
    {
        CreateButtonInColumn(0, 18);
        CreateButtonInColumn(1, 48);
        CreateButtonInColumn(2, 49);
        CreateButtonInColumn(3, 50);
        CreateButtonInColumn(4, 51);
        pDialogueWindow->_41D08F_set_keyboard_control_group(5, 1, 0, 2);
    } break;

    case BuildingType_SelfGuild:
    {
        CreateButtonInColumn(0, 18);
        CreateButtonInColumn(1, 52);
        CreateButtonInColumn(2, 53);
        CreateButtonInColumn(3, 54);
        pDialogueWindow->_41D08F_set_keyboard_control_group(4, 1, 0, 2);
    } break;

    case BuildingType_16:
    case BuildingType_TownHall:
    {
        int num_buttons = 1;
        CreateButtonInColumn(0, 99);

        if (pParty->uFine) {
            num_buttons++;
            CreateButtonInColumn(1, 100);
        }

        pDialogueWindow->_41D08F_set_keyboard_control_group(num_buttons, 1,
            0, 2);
    } break;

    case BuildingType_Bank:
    {
        CreateButtonInColumn(0, 7);
        CreateButtonInColumn(1, 8);
        pDialogueWindow->_41D08F_set_keyboard_control_group(2, 1, 0, 2);
    } break;

    case BuildingType_Temple:
    {
        CreateButtonInColumn(0, 10);
        CreateButtonInColumn(1, 11);
        CreateButtonInColumn(2, 96);
        pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
    } break;

    case BuildingType_Stables:
    case BuildingType_Boats:
    {
        CreateButtonInColumn(0, 105);
        CreateButtonInColumn(1, 106);
        CreateButtonInColumn(2, 107);
        CreateButtonInColumn(3, 108);
        pDialogueWindow->_41D08F_set_keyboard_control_group(4, 1, 0, 2);
    } break;

    case BuildingType_Training:
    {
        CreateButtonInColumn(0, 17);
        CreateButtonInColumn(1, 96);
        pDialogueWindow->_41D08F_set_keyboard_control_group(2, 1, 0, 2);
    } break;

    case BuildingType_Tavern:
    {
        CreateButtonInColumn(0, 15);
        CreateButtonInColumn(1, 16);
        CreateButtonInColumn(2, 96);
        if ((int64_t)window_SpeakInHouse->ptr_1C < 108 ||
            (int64_t)window_SpeakInHouse->ptr_1C > 120) {
            pDialogueWindow->_41D08F_set_keyboard_control_group(3, 1, 0, 2);
            break;
        }
        CreateButtonInColumn(3, 101);
        pDialogueWindow->_41D08F_set_keyboard_control_group(4, 1, 0, 2);
    } break;

    case BuildingType_18:
    case BuildingType_19:
    case BuildingType_Throne_Room:
    case BuildingType_24:
    case BuildingType_Unic:
    case BuildingType_1A:
    case BuildingType_House:
    case BuildingType_Jail:
        break;

    default:
        Error("Invalid enumeration value: %u", type);
    }

    /*  if ( a1 > 13 )
    {
    if ( a1 > 22 )
    {
    if ( a1 == 23 )
    {
    CreateButtonInColumn(0, 0xAu);
    CreateButtonInColumn(1, 0xBu);
    v14 = 96;
    LABEL_41:
    CreateButtonInColumn(2, v14);
    v17 = 2;
    v11 = 0;
    v10 = 1;
    v9 = 3;
    goto LABEL_42;
    }
    if ( a1 <= 26 )
    goto LABEL_43;
    if ( a1 > 28 )
    {
    if ( a1 != 30 )
    goto LABEL_43;
    CreateButtonInColumn(0, 0x11u);
    v16 = 96;
    goto LABEL_37;
    }
    CreateButtonInColumn(0, 0x69u);
    CreateButtonInColumn(1, 0x6Au);
    CreateButtonInColumn(2, 0x6Bu);
    v12 = 108;
    }
    else
    {
    if ( a1 == 22 )
    {
    CreateButtonInColumn(0, 7u);
    v16 = 8;
    goto LABEL_37;
    }
    v1 = a1 - 14;
    if ( !v1 ) // == 14
    {
    CreateButtonInColumn(0, 0x12u);
    CreateButtonInColumn(1, 0x30u);
    CreateButtonInColumn(2, 0x31u);
    CreateButtonInColumn(3, 0x32u);
    CreateButtonInColumn(4, 0x33u);
    v17 = 2;
    v11 = 0;
    v10 = 1;
    v9 = 5;
    goto LABEL_42;
    }
    v2 = v1 - 1;
    if ( v2 ) // > 15
    {
    v3 = v2 - 2;
    if ( v3 ) // > 17
    {
    if ( v3 != 4 ) // 18, 19, 20
    goto LABEL_43;
    CreateButtonInColumn(0, 0xFu); // 21
    CreateButtonInColumn(1, 0x10u);
    v4 = 3;
    CreateButtonInColumn(2, 0x60u);
    v5 = (signed int)window_SpeakInHouse->ptr_1C;
    if ( v5 < 108 || v5 > 120 )
    goto LABEL_28;
    v4 = 4;
    v6 = 101;
    v7 = 3;
    }
    else // 16, 17
    {
    v4 = 1;
    CreateButtonInColumn(0, 0x63u);
    if ( !pParty->uFine )
    {
    LABEL_28:
    v17 = 2;
    v11 = 0;
    v10 = 1;
    v9 = v4;
    LABEL_42:
    pDialogueWindow->_41D08F_set_keyboard_control_group(v9, v10,
    v11, v17); goto LABEL_43;
    }
    v4 = 2;
    v7 = 1;
    v6 = 100;
    }
    CreateButtonInColumn(v7, v6);
    goto LABEL_28;
    }
    CreateButtonInColumn(0, 0x12u); // 15
    CreateButtonInColumn(1, 0x34u);
    CreateButtonInColumn(2, 0x35u);
    v12 = 54;
    }
    LABEL_39:
    CreateButtonInColumn(3, v12);
    v17 = 2;
    v11 = 0;
    v10 = 1;
    v9 = 4;
    goto LABEL_42;
    }
    if ( a1 == 13 )
    {
    CreateButtonInColumn(0, 0x12u);
    v16 = 56;
    LABEL_37:
    CreateButtonInColumn(1, v16);
    v17 = 2;
    v11 = 0;
    v10 = 1;
    v9 = 2;
    goto LABEL_42;
    }
    switch ( a1 )
    {
    case 1:
    case 2:
    case 3:
    case 4:
    CreateButtonInColumn(0, 2u);
    CreateButtonInColumn(1, 0x5Fu);
    CreateButtonInColumn(2, 0x5Eu);
    v12 = 96;
    goto LABEL_39;
    case 5:
    CreateButtonInColumn(0, 0x12u);
    v13 = 48;
    goto LABEL_9;
    case 6:
    CreateButtonInColumn(0, 0x12u);
    v13 = 49;
    goto LABEL_9;
    case 7:
    CreateButtonInColumn(0, 0x12u);
    v13 = 50;
    goto LABEL_9;
    case 8:
    CreateButtonInColumn(0, 0x12u);
    v13 = 51;
    LABEL_9:
    CreateButtonInColumn(1, v13);
    v14 = 72;
    goto LABEL_41;
    case 9:
    CreateButtonInColumn(0, 0x12u);
    v15 = 52;
    goto LABEL_13;
    case 10:
    CreateButtonInColumn(0, 0x12u);
    v15 = 53;
    goto LABEL_13;
    case 11:
    CreateButtonInColumn(0, 0x12u);
    v15 = 54;
    LABEL_13:
    CreateButtonInColumn(1, v15);
    v14 = 61;
    goto LABEL_41;
    case 12:
    CreateButtonInColumn(0, 0x12u);
    v16 = 55;
    goto LABEL_37;
    default:
    break;
    }
    LABEL_43:*/
    dword_F8B1E0 = pDialogueWindow->pNumPresenceButton;
}
// F8B1E0: using guessed type int dword_F8B1E0;

bool HouseUI_CheckIfPlayerCanInteract() {
    if (uActiveCharacter == 0) {  // to avoid access zeroeleement
        return false;
    }

    if (pPlayers[uActiveCharacter]->CanAct()) {
        pDialogueWindow->pNumPresenceButton = dword_F8B1E0;
        return true;
    } else {
        pDialogueWindow->pNumPresenceButton = 0;
        GUIWindow window = *pPrimaryWindow;
        window.uFrameX = 483;
        window.uFrameWidth = 143;
        window.uFrameZ = 334;

        String str = localization->FormatString(
            427,  // %s is in no condition to %s
            pPlayers[uActiveCharacter]->pName,
            localization->GetString(562));  // do anything
        window.DrawTitleText(
            pFontArrus, 0,
            (212 - pFontArrus->CalcTextHeight(str, window.uFrameWidth, 0)) / 2 +
            101,
            ui_house_player_cant_interact_color, str, 3);
        return false;
    }
}

//----- (0044622E) --------------------------------------------------------
bool EnterHouse(enum HOUSE_ID uHouseID) {
    int uOpenTime;   // eax@5
    int uCloseTime;  // esi@5
                     // unsigned int v5; // esi@5
                     // int v6; // edx@5
    int am_pm_flag_open;   // ecx@10
    int am_pm_flag_close;  // eax@10
                           // int v9; // esi@10
                           // int v11; // ecx@17
                           // unsigned int v12; // kr00_4@25
                           // int v14; // eax@25
                           // unsigned int v17; // eax@37
    int v18;       // edi@37
    int v19;       // edi@41
    String pContainer;  // [sp+Ch] [bp-30h]@32
                          // unsigned int v24; // [sp+34h] [bp-8h]@5

    GameUI_StatusBar_Clear();
    GameUI_StatusBar_OnEvent("");
    pMessageQueue_50CBD0->Flush();
    viewparams->bRedrawGameUI = 1;
    uDialogueType = 0;
    keyboardInputHandler->SetWindowInputStatus(WindowInputStatus::WINDOW_INPUT_CANCELLED);
    keyboardInputHandler->ResetKeys();
    if (uHouseID == HOUSE_600 || uHouseID == HOUSE_601) {
        Application::GameOver_Loop(0);
        return 0;
    }
    uOpenTime = p2DEvents[uHouseID - 1].uOpenTime;
    uCloseTime = p2DEvents[uHouseID - 1].uCloseTime;
    current_npc_text.clear();
    dword_F8B1E4 = 0;
    memset(byte_F8B1F0.data(), 0, 4);
    memset(player_levels.data(), 0, 16);
    render->ClearZBuffer(0, 479);

    if (((uCloseTime - 1 <= uOpenTime) &&
        ((pParty->uCurrentHour < uOpenTime) &&
        (pParty->uCurrentHour >(uCloseTime - 1)))) ||
            ((uCloseTime - 1 > uOpenTime) &&
        ((pParty->uCurrentHour < uOpenTime) ||
                (pParty->uCurrentHour >(uCloseTime - 1))))) {
        am_pm_flag_open = 0;
        am_pm_flag_close = 0;
        if (uOpenTime > 12) {
            uOpenTime -= 12;
            am_pm_flag_open = 1;
        }
        if (uCloseTime > 12) {
            uCloseTime -= 12;
            am_pm_flag_close = 1;
        }

        GameUI_StatusBar_OnEvent(localization->FormatString(
            414,  // This place is open from %d%s to %d%s
            uOpenTime, localization->GetAmPm(am_pm_flag_open), uCloseTime,
            localization->GetAmPm(am_pm_flag_close)));
        if (uActiveCharacter)
            pPlayers[uActiveCharacter]->PlaySound(SPEECH_3, 0);
        return 0;
    } else {
        // v10 = uHouseID;
        if (uHouseID < 53) {  // entering shops and guilds
            if (!(pParty->PartyTimes._shop_ban_times[uHouseID]) ||
                (pParty->PartyTimes._shop_ban_times[uHouseID] <=
                    pParty->GetPlayingTime())) {
                pParty->PartyTimes._shop_ban_times[uHouseID] = GameTime(0);
            } else {
                GameUI_StatusBar_OnEvent(localization->GetString(
                    191));  // "You've been banned from this shop!"
                return 0;
            }
        }
        pAudioPlayer->StopChannels(-1, -1);

        uCurrentHouse_Animation = p2DEvents[uHouseID - 1].uAnimationID;
        in_current_building_type =
            (BuildingType)pAnimatedRooms[uCurrentHouse_Animation].uBuildingType;
        if (in_current_building_type == BuildingType_Throne_Room &&
            pParty->uFine) {  // going 2 jail
            uCurrentHouse_Animation =
                (signed __int16)p2DEvents[186].uAnimationID;
            uHouseID = HOUSE_JAIL;
            pParty->GetPlayingTime().AddYears(1);  // += 123863040;
            in_current_building_type =
                (BuildingType)
                pAnimatedRooms[p2DEvents[HOUSE_LORD_AND_JUDGE_EMERALD_ISLE]
                .uAnimationID]
                .uBuildingType;
            ++pParty->uNumPrisonTerms;
            pParty->uFine = 0;
            for (uint i = 0; i < 4; ++i) {
                pParty->pPlayers[i].uTimeToRecovery = 0;
                pParty->pPlayers[i].uNumDivineInterventionCastsThisDay = 0;
                pParty->pPlayers[i].SetVariable(VAR_Award, 87);
            }
        }

        switch (pParty->alignment) {
            case PartyAlignment::PartyAlignment_Good:
                pContainer = "evt02-b";
                break;
            case PartyAlignment::PartyAlignment_Neutral:
                pContainer = "evt02";
                break;
            case PartyAlignment::PartyAlignment_Evil:
                pContainer = "evt02-c";
                break;
            default:
                Error("Invalid alignment: %u", pParty->alignment);
        }

        pDialogueNPCCount = 0;
        game_ui_dialogue_background = assets->GetImage_Solid(pContainer);

        PrepareHouse(uHouseID);
        v18 = 1;

        if (uNumDialogueNPCPortraits == 1) pDialogueNPCCount = 1;
        pMediaPlayer->OpenHouseMovie(
            pAnimatedRooms[uCurrentHouse_Animation].video_name, 1u);
        dword_5C35D4 = 1;
        if ((signed int)uHouseID < 139 || (signed int)uHouseID > 172) {
            if ((signed int)uHouseID >= HOUSE_STABLES_HARMONDALE &&
                (signed int)uHouseID <= 73 &&
                !IsTravelAvailable(uHouseID - HOUSE_STABLES_HARMONDALE)) {
                return 1;
            }
        } else {  // guilds
            v19 = guild_mambership_flags
                [uHouseID -
                HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE];  // guilds flags
                                                          // v20 = uHouseID;
                                                          // if ( !((unsigned __int8)(0x80u >> v19 % 8) &
                                                          // pPlayers[uActiveCharacter]->_guilds_member_bits[v19 /8]) )

            if (uActiveCharacter == 0)  // avoid nzi
                uActiveCharacter = pParty->GetFirstCanAct();

            if (!_449B57_test_bit(
                pPlayers[uActiveCharacter]->_achieved_awards_bits, v19)) {
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
    __int16 uExitMapID;  // ax@2
                         //  int v7; // ebx@11
                         //  int v13; // [sp+30h] [bp-30h]@11
    int npc_id_arr[6];   // [sp+34h] [bp-2Ch]@1
    int uAnimationID;    // [sp+50h] [bp-10h]@1

    uAnimationID = p2DEvents[house - 1].uAnimationID;
    memset(npc_id_arr, 0, sizeof(npc_id_arr));
    uNumDialogueNPCPortraits = 0;
    uHouse_ExitPic = p2DEvents[house - 1].uExitPicID;
    if (uHouse_ExitPic) {
        uExitMapID = p2DEvents[house - 1]._quest_related;
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
            StringPrintf("npc%03u", npc_id_arr[i]), 0x7FF);
    }

    if (uHouse_ExitPic) {
        pDialogueNPCPortraits[uNumDialogueNPCPortraits] =
            assets->GetImage_ColorKey(pHouse_ExitPictures[uHouse_ExitPic],
                0x7FF);
        ++uNumDialogueNPCPortraits;
        uHouse_ExitPic = p2DEvents[house - 1].uExitMapID;
    }
}

//----- (004B1E92) --------------------------------------------------------
void PlayHouseSound(unsigned int uHouseID, HouseSoundID sound) {
    if (pAnimatedRooms[p2DEvents[uHouseID-1].uAnimationID].uRoomSoundId)
        pAudioPlayer->PlaySound(
        (SoundID)(sound +
            100 * (pAnimatedRooms[p2DEvents[uHouseID-1].uAnimationID]
                .uRoomSoundId +
                300)),
            806, 0, -1, 0, 0);
}

//----- (004BCACC) --------------------------------------------------------
void OnSelectShopDialogueOption(signed int uMessageParam) {
    int experience_for_next_level;  // eax@5
    int v16;                        // eax@32
    __int16 v24;                    // ax@163
    signed int v36;                 // esi@227
    int pPrice;                     // ecx@227

    if (!pDialogueWindow->pNumPresenceButton) return;
    render->ClearZBuffer(0, 479);
    if (dialog_menu_id == HOUSE_DIALOGUE_MAIN) {
        if (in_current_building_type == BuildingType_Training) {
            if (uMessageParam == HOUSE_DIALOGUE_TRAININGHALL_TRAIN) {
                experience_for_next_level = 0;
                if (pPlayers[uActiveCharacter]->uLevel > 0) {
                    for (uint i = 0; i < pPlayers[uActiveCharacter]->uLevel;
                        i++)
                        experience_for_next_level += i + 1;
                }
                if (pPlayers[uActiveCharacter]->uLevel <
                    pMaxLevelPerTrainingHallType
                    [(uint64_t)window_SpeakInHouse->ptr_1C - 89] &&
                    (signed __int64)pPlayers[uActiveCharacter]->uExperience <
                    1000 * experience_for_next_level)  // test experience
                    return;
            }
            pDialogueWindow->Release();
            pDialogueWindow = new GUIWindow(WINDOW_Dialogue, 0, 0, window->GetWidth(), 345, 0);
            pBtn_ExitCancel = pDialogueWindow->CreateButton(
                526, 445, 75, 33, 1, 0, UIMSG_Escape, 0, GameKey::None,
                localization->GetString(74),  // "End Conversation"
                { { ui_buttdesc2 } });
            pDialogueWindow->CreateButton(8, 8, 450, 320, 1, 0,
                UIMSG_BuyInShop_Identify_Repair, 0);
        }
        if (in_current_building_type != BuildingType_Training) {
            if ((in_current_building_type == BuildingType_Stables ||
                in_current_building_type == BuildingType_Boats) &&
                transport_schedule
                [transport_routes[(uint64_t)window_SpeakInHouse->ptr_1C - HOUSE_STABLES_HARMONDALE]
                [uMessageParam - HOUSE_DIALOGUE_TRANSPORT_SCHEDULE_1]]
            .pSchedule[pParty->uCurrentDayOfMonth % 7] ||
                in_current_building_type != BuildingType_Temple ||
                uMessageParam != BuildingType_MindGuild) {
                pDialogueWindow->Release();
                pDialogueWindow = new GUIWindow(WINDOW_Dialogue, 0, 0, window->GetWidth(), 345, 0);
                pBtn_ExitCancel = pDialogueWindow->CreateButton(
                    526, 445, 75, 33, 1, 0, UIMSG_Escape, 0, GameKey::None,
                    localization->GetString(74),  // "End Conversation"
                    { { ui_buttdesc2 } });
                pDialogueWindow->CreateButton(8, 8, 450, 320, 1, 0,
                    UIMSG_BuyInShop_Identify_Repair,
                    0);
            } else if (uActiveCharacter) {
                if (!pPlayers[uActiveCharacter]->IsPlayerHealableByTemple())
                    return;
            }
        }
        dialog_menu_id = (HOUSE_DIALOGUE_MENU)uMessageParam;
        if (in_current_building_type < BuildingType_19) {
            shop_ui_background = assets->GetImage_ColorKey(
                _4F03B8_shop_background_names[(int)in_current_building_type],
                0x7FF);
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
    case BuildingType_16:
    {
        if (pParty->PartyTimes
            .Shops_next_generation_time[window_SpeakInHouse->par1C -
            139] >=
            pParty->GetPlayingTime()) {
            for (uint i = 0; i < 12; ++i) {
                if (pParty
                    ->SpellBooksInGuilds[window_SpeakInHouse->par1C -
                    139][i]
                    .uItemID)
                    shop_ui_items_in_store[i] = assets->GetImage_ColorKey(
                        pParty
                        ->SpellBooksInGuilds
                        [window_SpeakInHouse->par1C - 139][i]
                        .GetIconName(),
                        0x7FF);
            }
        } else {  // generation new books
            SpellBookGenerator();
            pParty->PartyTimes
                .Shops_next_generation_time[window_SpeakInHouse->par1C -
                139] =
                GameTime(pParty->GetPlayingTime() +
                GameTime::FromDays(
                    p2DEvents[(uint64_t)window_SpeakInHouse->ptr_1C - 1]
                    .generation_interval_days));
        }
        break;
    }
    case BuildingType_TownHall:
    {
        if (uMessageParam == HOUSE_DIALOGUE_TOWNHALL_MESSAGE) {
            if (pParty->PartyTimes.bountyHunting_next_generation_time[(
                int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)] <
                pParty->GetPlayingTime()) {  // new generation
                pParty->monster_for_hunting_killed[(
                    int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)] =
                    false;
                pParty->PartyTimes.bountyHunting_next_generation_time[(
                    int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)] =
                    GameTime((int64_t)((double)(309657600 *
                    (pParty->uCurrentMonth +
                        12ll *
                        pParty->uCurrentYear -
                        14015)) *
                        0.033333335));
                pParty->monster_id_for_hunting[(
                    int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)] =
                    rand() % 258 + 1;
                v16 = (int64_t)((char *)window_SpeakInHouse->ptr_1C - 102);
                if (!v16) {
                    while (1) {
                        v24 = pParty->monster_id_for_hunting[(
                            int64_t)((char *)window_SpeakInHouse->ptr_1C -
                                102)];
                        if ((unsigned __int16)v24 < 115 ||
                            (unsigned __int16)v24 > 132) {
                            if (((unsigned __int16)v24 < 235 ||
                                (unsigned __int16)v24 > 252) &&
                                ((unsigned __int16)v24 < 133 ||
                                (unsigned __int16)v24 > 150) &&
                                    ((unsigned __int16)v24 < 0x97u ||
                                (unsigned __int16)v24 > 0xBAu) &&
                                        ((unsigned __int16)v24 < 0xBEu ||
                                (unsigned __int16)v24 > 0xC0u) &&
                                            ((unsigned __int16)v24 < 0xC4u ||
                                (unsigned __int16)v24 > 0xC6u) &&
                                                ((unsigned __int16)v24 < 0x2Bu ||
                                (unsigned __int16)v24 > 0x2Du) &&
                                                    ((unsigned __int16)v24 < 0xCDu ||
                                (unsigned __int16)v24 > 0xCFu) &&
                                                        ((unsigned __int16)v24 < 0x5Eu ||
                                (unsigned __int16)v24 > 0x60u) &&
                                                            ((unsigned __int16)v24 < 0xFDu ||
                                (unsigned __int16)v24 > 0xFFu) &&
                                                                ((unsigned __int16)v24 < 0x6Du ||
                                (unsigned __int16)v24 > 0x6Fu) &&
                                                                    ((unsigned __int16)v24 < 0x61u ||
                                (unsigned __int16)v24 > 0x63u))
                                break;
                        }
                        pParty->monster_id_for_hunting[(
                            int64_t)((char *)window_SpeakInHouse->ptr_1C -
                                102)] = rand() % 258 + 1;
                    }
                }
                if (v16 == 1) {
                    while (1) {
                        v24 = pParty->monster_id_for_hunting[(
                            int64_t)((char *)window_SpeakInHouse->ptr_1C -
                                102)];
                        if ((unsigned __int16)v24 < 115 ||
                            (unsigned __int16)v24 > 132) {
                            if (((unsigned __int16)v24 < 0xE8u ||
                                (unsigned __int16)v24 > 0xF9u) &&
                                ((unsigned __int16)v24 < 0x85u ||
                                (unsigned __int16)v24 > 0x96u) &&
                                    ((unsigned __int16)v24 < 0x97u ||
                                (unsigned __int16)v24 > 0xBAu) &&
                                        ((unsigned __int16)v24 < 0xBEu ||
                                (unsigned __int16)v24 > 0xC0u) &&
                                            ((unsigned __int16)v24 < 0xC4u ||
                                (unsigned __int16)v24 > 0xC6u) &&
                                                ((unsigned __int16)v24 < 0x2Bu ||
                                (unsigned __int16)v24 > 0x2Du) &&
                                                    ((unsigned __int16)v24 < 0x52u ||
                                (unsigned __int16)v24 > 0x54u) &&
                                                        ((unsigned __int16)v24 < 4 ||
                                (unsigned __int16)v24 > 6) &&
                                                            ((unsigned __int16)v24 < 0x37u ||
                                (unsigned __int16)v24 > 0x39u) &&
                                                                ((unsigned __int16)v24 < 0x3Au ||
                                (unsigned __int16)v24 > 0x3Cu) &&
                                                                    ((unsigned __int16)v24 < 0x3Du ||
                                (unsigned __int16)v24 > 0x3Fu) &&
                                                                        ((unsigned __int16)v24 < 0xFDu ||
                                (unsigned __int16)v24 > 0xFFu) &&
                                                                            ((unsigned __int16)v24 < 0x61u ||
                                (unsigned __int16)v24 > 0x63u) &&
                                                                                ((unsigned __int16)v24 < 0xCDu ||
                                (unsigned __int16)v24 > 0xCFu))
                                break;
                        }
                        pParty->monster_id_for_hunting[(
                            int64_t)((char *)window_SpeakInHouse->ptr_1C -
                                102)] = rand() % 258 + 1;
                    }
                }
                if (v16 == 2) {
                    while (1) {
                        v24 = pParty->monster_id_for_hunting[(
                            int64_t)((char *)window_SpeakInHouse->ptr_1C -
                                102)];
                        if ((unsigned __int16)v24 < 0x73u ||
                            (unsigned __int16)v24 > 0x84u) {
                            if (((unsigned __int16)v24 < 0xE8u ||
                                (unsigned __int16)v24 > 0xF9u) &&
                                ((unsigned __int16)v24 < 0x85u ||
                                (unsigned __int16)v24 > 0x96u) &&
                                    ((unsigned __int16)v24 < 0x97u ||
                                (unsigned __int16)v24 > 0xBAu) &&
                                        ((unsigned __int16)v24 < 0xBEu ||
                                (unsigned __int16)v24 > 0xC0u) &&
                                            ((unsigned __int16)v24 < 0xC4u ||
                                (unsigned __int16)v24 > 0xC6u) &&
                                                ((unsigned __int16)v24 < 0x2Bu ||
                                (unsigned __int16)v24 > 0x2Du) &&
                                                    ((unsigned __int16)v24 < 0x31u ||
                                (unsigned __int16)v24 > 0x33u) &&
                                                        ((unsigned __int16)v24 < 0x34u ||
                                (unsigned __int16)v24 > 0x36u) &&
                                                            ((unsigned __int16)v24 < 0xFDu ||
                                (unsigned __int16)v24 > 0xFFu) &&
                                                                ((unsigned __int16)v24 < 0x61u ||
                                (unsigned __int16)v24 > 0x63u) &&
                                                                    ((unsigned __int16)v24 < 0x1Cu ||
                                (unsigned __int16)v24 > 0x1Eu))
                                break;
                        }
                        pParty->monster_id_for_hunting[(
                            int64_t)((char *)window_SpeakInHouse->ptr_1C -
                                102)] = rand() % 258 + 1;
                    }
                }
                if (v16 == 3) {
                    while (1) {
                        v24 = pParty->monster_id_for_hunting[(
                            int64_t)((char *)window_SpeakInHouse->ptr_1C -
                                102)];
                        if ((unsigned __int16)v24 < 0x73u ||
                            (unsigned __int16)v24 > 0x84u) {
                            if (((unsigned __int16)v24 < 0xE8u ||
                                (unsigned __int16)v24 > 0xF9u) &&
                                ((unsigned __int16)v24 < 0x85u ||
                                (unsigned __int16)v24 > 0x96u) &&
                                    ((unsigned __int16)v24 < 0x97u ||
                                (unsigned __int16)v24 > 0xBAu) &&
                                        ((unsigned __int16)v24 < 0xBEu ||
                                (unsigned __int16)v24 > 0xC0u) &&
                                            ((unsigned __int16)v24 < 0xC4u ||
                                (unsigned __int16)v24 > 0xC6u) &&
                                                ((unsigned __int16)v24 < 0x2Bu ||
                                (unsigned __int16)v24 > 0x2Du) &&
                                                    ((unsigned __int16)v24 < 0x5Eu ||
                                (unsigned __int16)v24 > 0x60u) &&
                                                        ((unsigned __int16)v24 < 0x43u ||
                                (unsigned __int16)v24 > 0x45u) &&
                                                            ((unsigned __int16)v24 < 0x4Fu ||
                                (unsigned __int16)v24 > 0x51u) &&
                                                                ((unsigned __int16)v24 < 0xC1u ||
                                (unsigned __int16)v24 > 0xC3u) &&
                                                                    ((unsigned __int16)v24 < 0x13u ||
                                (unsigned __int16)v24 > 0x15u) &&
                                                                        ((unsigned __int16)v24 < 0xFDu ||
                                (unsigned __int16)v24 > 0xFFu) &&
                                                                            ((unsigned __int16)v24 < 0x61u ||
                                (unsigned __int16)v24 > 0x63u) &&
                                                                                ((unsigned __int16)v24 < 0x6Au ||
                                (unsigned __int16)v24 > 0x6Cu))
                                break;
                        }
                        pParty->monster_id_for_hunting[(
                            int64_t)((char *)window_SpeakInHouse->ptr_1C -
                                102)] = rand() % 258 + 1;
                    }
                }
                if (v16 == 4) {
                    while (1) {
                        v24 = pParty->monster_id_for_hunting[(
                            int64_t)((char *)window_SpeakInHouse->ptr_1C -
                                102)];
                        if ((unsigned __int16)v24 < 0x73u ||
                            (unsigned __int16)v24 > 0x84u) {
                            if (((unsigned __int16)v24 < 0xE8u ||
                                (unsigned __int16)v24 > 0xF9u) &&
                                ((unsigned __int16)v24 < 0x85u ||
                                (unsigned __int16)v24 > 0x96u) &&
                                    ((unsigned __int16)v24 < 0x97u ||
                                (unsigned __int16)v24 > 0xBAu) &&
                                        ((unsigned __int16)v24 < 0xBEu ||
                                (unsigned __int16)v24 > 0xC0u) &&
                                            ((unsigned __int16)v24 < 0xC4u ||
                                (unsigned __int16)v24 > 0xC6u) &&
                                                ((unsigned __int16)v24 < 0x2Bu ||
                                (unsigned __int16)v24 > 0x2Du) &&
                                                    ((unsigned __int16)v24 < 0x6Du ||
                                (unsigned __int16)v24 > 0x6Fu) &&
                                                        ((unsigned __int16)v24 < 0x46u ||
                                (unsigned __int16)v24 > 0x48u) &&
                                                            ((unsigned __int16)v24 < 0x100u ||
                                (unsigned __int16)v24 > 0x102u) &&
                                                                ((unsigned __int16)v24 < 0xD9u ||
                                (unsigned __int16)v24 > 0xDBu) &&
                                                                    ((unsigned __int16)v24 < 0xC7u ||
                                (unsigned __int16)v24 > 0xC9u) &&
                                                                        ((unsigned __int16)v24 < 0xE5u ||
                                (unsigned __int16)v24 > 0xE7u) &&
                                                                            ((unsigned __int16)v24 < 0xDFu ||
                                (unsigned __int16)v24 > 0xE1u) &&
                                                                                ((unsigned __int16)v24 < 0x5Bu ||
                                (unsigned __int16)v24 > 0x5Du) &&
                                                                                    ((unsigned __int16)v24 < 0x49u ||
                                (unsigned __int16)v24 > 0x4Bu) &&
                                                                                        ((unsigned __int16)v24 < 0xFDu ||
                                (unsigned __int16)v24 > 0xFFu) &&
                                                                                            ((unsigned __int16)v24 < 0x61u ||
                                (unsigned __int16)v24 > 0x63u) &&
                                                                                                ((unsigned __int16)v24 < 0x10u ||
                                (unsigned __int16)v24 > 0x12u))
                                break;
                        }
                        pParty->monster_id_for_hunting[(
                            int64_t)((char *)window_SpeakInHouse->ptr_1C -
                                102)] = rand() % 258 + 1;
                    }
                }
            }
            bountyHunting_monster_id_for_hunting =
                pParty->monster_id_for_hunting[(
                    int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)];
            if (!pParty->monster_for_hunting_killed[(
                int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)]) {
                bountyHunting_text =
                    pNPCTopics[351].pText;  //"В этом месяцу назначена
                                            //награда за голову %s..."
                if (!pParty->monster_id_for_hunting[(
                    int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)])
                    bountyHunting_text =
                    pNPCTopics[353].pText;  //"Кое кто уже приходил в
                                            //этом месяце за наградой"
            } else {
                if (pParty->monster_id_for_hunting[(
                    int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)] >
                    0) {  // get prize
                    pParty->PartyFindsGold(
                        100 * pMonsterStats
                        ->pInfos
                        [(unsigned __int16)
                        pParty->monster_id_for_hunting[(
                            int64_t)((char *)
                                window_SpeakInHouse
                                ->ptr_1C -
                                102)]]
                    .uLevel,
                        0);
                    for (uint i = 0; i < 4; ++i)
                        pParty->pPlayers[i].SetVariable(VAR_Award, 86);
                    pParty->uNumBountiesCollected +=
                        100 *
                        pMonsterStats
                        ->pInfos[pParty->monster_id_for_hunting[(
                            int64_t)((char *)window_SpeakInHouse->ptr_1C -
                                102)]]
                        .uLevel;
                    pParty->monster_id_for_hunting[(
                        int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)] =
                        0;
                    pParty->monster_for_hunting_killed[(
                        int64_t)((char *)window_SpeakInHouse->ptr_1C - 102)] =
                        false;
                }
                bountyHunting_text =
                    pNPCTopics[352].pText;  //"Поздравляю! Вы успешно..."
            }
        } else if (uMessageParam == HOUSE_DIALOGUE_TOWNHALL_PAY_FINE) {
            keyboardInputHandler->StartTextInput(TextInputType::Number, 10, window_SpeakInHouse);
        }
        break;
    }
    case BuildingType_Bank:
    {
        if (dialog_menu_id >= 7 && dialog_menu_id <= 8)
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

    switch (uMessageParam) {
    case HOUSE_DIALOGUE_LEARN_SKILLS:
    {
        pDialogueWindow->eWindowType = WINDOW_MainMenu;
        UI_CreateEndConversationButton();
        FillAviableSkillsToTeach(in_current_building_type);
        break;
    }
    case HOUSE_DIALOGUE_TAVERN_ARCOMAGE_MAIN:
    {
        pDialogueWindow->eWindowType = WINDOW_MainMenu;
        UI_CreateEndConversationButton();
        InitializaDialogueOptions_Tavern(in_current_building_type);
        break;
    }
    case HOUSE_DIALOGUE_TAVERN_ARCOMAGE_RULES:
    case HOUSE_DIALOGUE_TAVERN_ARCOMAGE_VICTORY_CONDITIONS:
    {
        dialog_menu_id = (HOUSE_DIALOGUE_MENU)uMessageParam;
        break;
    }
    case HOUSE_DIALOGUE_TAVERN_ARCOMAGE_RESULT:
    {
        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_PlayArcomage, 0, 0);
        dialog_menu_id = HOUSE_DIALOGUE_TAVERN_ARCOMAGE_RESULT;
        break;
    }
    case HOUSE_DIALOGUE_SHOP_BUY_STANDARD:
    case HOUSE_DIALOGUE_SHOP_BUY_SPECIAL:
    {
        if (pParty->PartyTimes.Shops_next_generation_time
            [(uint64_t)window_SpeakInHouse->ptr_1C] <
            pParty->GetPlayingTime()) {
            GenerateStandartShopItems();
            GenerateSpecialShopItems();
            pParty->PartyTimes
                .Shops_next_generation_time[window_SpeakInHouse->par1C] =
                GameTime(pParty->GetPlayingTime() +
                GameTime::FromDays(
                    p2DEvents[(uint64_t)window_SpeakInHouse->ptr_1C - 1]
                    .generation_interval_days));
        }
        if (uMessageParam == HOUSE_DIALOGUE_SHOP_BUY_STANDARD) {
            if (uItemsAmountPerShopType
                [p2DEvents[(uint64_t)window_SpeakInHouse->ptr_1C -
                1]
                .uType]) {
                for (uint i = 0;
                    i < (unsigned __int8)uItemsAmountPerShopType
                    [p2DEvents[(uint64_t)
                    window_SpeakInHouse->ptr_1C -
                    1]
                    .uType];
                    ++i) {
                    if (pParty
                        ->StandartItemsInShops[(int64_t)window_SpeakInHouse
                        ->ptr_1C][i]
                        .uItemID)
                        shop_ui_items_in_store[i] =
                        assets->GetImage_ColorKey(
                            pParty
                            ->StandartItemsInShops
                            [(int64_t)window_SpeakInHouse->ptr_1C]
                    [i]
                    .GetIconName(),
                        0x7FF);
                }
            }
            if (in_current_building_type == BuildingType_WeaponShop) {
                if (uItemsAmountPerShopType
                    [p2DEvents
                    [(uint64_t)window_SpeakInHouse->ptr_1C - 1]
                .uType]) {
                    for (uint i = 0;
                        i <
                        (unsigned __int8)uItemsAmountPerShopType
                        [p2DEvents[(uint64_t)
                        window_SpeakInHouse->ptr_1C -
                        1]
                        .uType];
                        ++i) {
                        if (pParty
                            ->StandartItemsInShops
                            [(int64_t)window_SpeakInHouse->ptr_1C][i]
                            .uItemID)
                            weapons_Ypos[i] =
                            rand() %
                            (300 -
                                shop_ui_items_in_store[i]->GetHeight());
                    }
                }
            }
        }
        if (uMessageParam == HOUSE_DIALOGUE_SHOP_BUY_SPECIAL) {
            if (uItemsAmountPerShopType
                [p2DEvents[(uint64_t)window_SpeakInHouse->ptr_1C -
                1]
                .uType]) {
                for (uint i = 0;
                    i < (unsigned __int8)uItemsAmountPerShopType
                    [p2DEvents[(uint64_t)
                    window_SpeakInHouse->ptr_1C -
                    1]
                    .uType];
                    ++i) {
                    if (pParty
                        ->SpecialItemsInShops[(uint64_t)
                        window_SpeakInHouse
                        ->ptr_1C][i]
                        .uItemID)
                        shop_ui_items_in_store[i] =
                        assets->GetImage_ColorKey(
                            pParty
                            ->SpecialItemsInShops
                            [(uint64_t)
                            window_SpeakInHouse->ptr_1C][i]
                            .GetIconName(),
                            0x7FF);
                }
            }
            if (in_current_building_type == BuildingType_WeaponShop) {
                if (uItemsAmountPerShopType
                    [p2DEvents
                    [(uint64_t)window_SpeakInHouse->ptr_1C - 1]
                .uType]) {
                    for (uint i = 0;
                        i <
                        (unsigned __int8)uItemsAmountPerShopType
                        [p2DEvents[(uint64_t)
                        window_SpeakInHouse->ptr_1C -
                        1]
                        .uType];
                        ++i) {
                        if (pParty
                            ->SpecialItemsInShops
                            [(uint64_t)
                            window_SpeakInHouse->ptr_1C][i]
                            .uItemID)
                            weapons_Ypos[i] =
                            rand() %
                            (300 -
                                shop_ui_items_in_store[i]->GetHeight());
                    }
                }
            }
        }
        break;
    }
    case HOUSE_DIALOGUE_SHOP_SELL:
    case HOUSE_DIALOGUE_SHOP_IDENTIFY:
    case HOUSE_DIALOGUE_SHOP_REPAIR:
    {
        dialog_menu_id = (HOUSE_DIALOGUE_MENU)uMessageParam;
        pParty->PickedItem_PlaceInInventory_or_Drop();
        break;
    }
    case HOUSE_DIALOGUE_SHOP_DISPLAY_EQUIPMENT:
    {
        pDialogueWindow->eWindowType = WINDOW_MainMenu;
        UI_CreateEndConversationButton();
        InitializaDialogueOptions_Shops(in_current_building_type);
        break;
    }
    default:
    {
        if (uMessageParam >= HOUSE_DIALOGUE_36 &&
            uMessageParam <= HOUSE_DIALOGUE_GUILD_LEARN_SKILL) {
            v36 =
                (signed __int64)(p2DEvents[(uint64_t)
                    window_SpeakInHouse->ptr_1C -
                    1]
                    .flt_24 *
                    500.0);
            pPrice = v36 *
                (100 - pPlayers[uActiveCharacter]->GetMerchant()) /
                100;
            if (pPrice < v36 / 3) pPrice = v36 / 3;
            if (byte_4ED970_skill_learn_ability_by_class_table
                [pPlayers[uActiveCharacter]->classType]
            [uMessageParam - 36]) {
                if (!pPlayers[uActiveCharacter]
                    ->pActiveSkills[uMessageParam - 36]) {
                    if (pParty->GetGold() < pPrice) {
                        GameUI_StatusBar_OnEvent(localization->GetString(LSTR_NOT_ENOUGH_GOLD));
                        if (in_current_building_type ==
                            BuildingType_Training ||
                            in_current_building_type == BuildingType_Tavern)
                            PlayHouseSound(
                            (uint64_t)window_SpeakInHouse->ptr_1C,
                                HouseSound_Goodbye);
                        else
                            PlayHouseSound(
                            (uint64_t)window_SpeakInHouse->ptr_1C,
                                HouseSound_NotEnoughMoney_TrainingSuccessful);
                    } else {
                        Party::TakeGold(pPrice);
                        dword_F8B1E4 = 1;
                        pPlayers[uActiveCharacter]
                            ->pActiveSkills[uMessageParam - 36] = 1;
                        pPlayers[uActiveCharacter]->PlaySound(SPEECH_78, 0);
                    }
                }
            }
        }
        break;
    }
    }
}

void TravelByTransport() {
    signed int v4;                 // ebx@1
    stru365_travel_info *pTravel;  // esi@7
    signed int v12;                // esi@13
    signed int v13;                // edi@14
    int v14;                       // eax@26
    int v15;                       // edi@26
    int pTextHeight;               // eax@36
    int pRealTextHeight;           // esi@36
    int schedule_id;               // esi@39
    GUIButton *pButton;            // ebx@39
    signed int v25;                // eax@41
    MapInfo pMap;                  // [sp-3Ch] [bp-2CCh]@62
    char pTopicArray[5][100];      // [sp+14h] [bp-27Ch]@37
    int pPrimaryTextHeight;        // [sp+260h] [bp-30h]@36
    int index;                     // [sp+27Ch] [bp-14h]@36
    unsigned int pPrice;           // [sp+288h] [bp-8h]@1
    int travel_time;               // [sp+28Ch] [bp-4h]@48
    enum PlayerSpeech pSpeech;
    unsigned int pCurrentButton;

    GUIWindow travel_window = *window_SpeakInHouse;
    travel_window.uFrameX = 483;
    travel_window.uFrameWidth = 143;
    travel_window.uFrameZ = 334;

    v4 = p2DEvents[(uint64_t)window_SpeakInHouse->ptr_1C - 1].uType ==
        BuildingType_Stables
        ? 25
        : 50;
    v4 *= p2DEvents[(uint64_t)window_SpeakInHouse->ptr_1C - 1]
        .fPriceMultiplier;

    pPrice = v4 * (100 - pPlayers[uActiveCharacter]->GetMerchant()) / 100;
    if ((signed int)pPrice < v4 / 3) pPrice = v4 / 3;
    if (dialog_menu_id == HOUSE_DIALOGUE_MAIN) {
        if (HouseUI_CheckIfPlayerCanInteract()) {
            index = 0;
            strcpy(pTopicArray[4], "");

            String str =
                localization->FormatString(405, pPrice);  // Price: %lu gold
            pTextHeight =
                pFontArrus->CalcTextHeight(str, travel_window.uFrameWidth, 0);
            pRealTextHeight = pTextHeight + (pFontArrus->GetHeight() - 3) + 146;
            pPrimaryTextHeight = pRealTextHeight;
            pCurrentButton = 2;
            for (uint i = pDialogueWindow->pStartingPosActiveItem;
                i < (unsigned int)(pDialogueWindow->pNumPresenceButton +
                    pDialogueWindow->pStartingPosActiveItem);
                ++i) {
                schedule_id =
                    transport_routes[(uint64_t)window_SpeakInHouse->ptr_1C -
                    HOUSE_STABLES_HARMONDALE][index];
                pButton = pDialogueWindow->GetControl(i);

                if (schedule_id != 255) {
                    // if (schedule_id >= 25)
                    // logger->Warning("Transport UI: schedule overflow");
                    if (pCurrentButton >= 6)
                        v25 = true;
                    else
                        v25 = transport_schedule[schedule_id]
                        .pSchedule[pParty->uCurrentDayOfMonth % 7];
                }

                if (schedule_id != 255 && v25 &&
                    (!transport_schedule[schedule_id].uQuestBit ||
                        _449B57_test_bit(
                            pParty->_quest_bits,
                            transport_schedule[schedule_id]
                            .uQuestBit))) {  // ошибка: рисуется несколько строчек
                                             // одного направления путешествия
                                             // get color for current string(определение цвета текущей
                                             // строки)----------
                    if (pDialogueWindow->pCurrentPosActiveItem ==
                        pCurrentButton)
                        sprintf(pTopicArray[index], "\f%05d",
                            Color16(255, 255, 155));
                    else
                        sprintf(pTopicArray[index], "\f%05d",
                            Color16(255, 255, 255));
                    // hired NPC premium(премия наёмного
                    // НПС)----------------------------------
                    travel_time = transport_schedule[schedule_id].uTravelTime;
                    if ((int64_t)window_SpeakInHouse->ptr_1C >=
                        HOUSE_BOATS_EMERALD_ISLE) {
                        if (CheckHiredNPCSpeciality(Sailor)) travel_time -= 2;
                        if (CheckHiredNPCSpeciality(Navigator))
                            travel_time -= 3;
                        if (CheckHiredNPCSpeciality(Pirate)) travel_time -= 2;
                    } else {
                        if (CheckHiredNPCSpeciality(Horseman)) travel_time -= 2;
                    }
                    if (CheckHiredNPCSpeciality(Explorer)) --travel_time;

                    if (travel_time < 1) travel_time = 1;
                    if (schedule_id != 255) {
                        memcpy(
                            &pMap,
                            &pMapStats->pInfos[transport_schedule[schedule_id]
                            .uMapInfoID],
                            0x44u);
                        String str = localization->FormatString(
                            404, travel_time,
                            pMap.pName.c_str());  // Time - %d days, destination %s
                        strcat(pTopicArray[index], str.c_str());
                        strcat(pTopicArray[index], "\n \n");
                        pButton->uY = pRealTextHeight;
                        pTextHeight = pFontArrus->CalcTextHeight(
                            str, travel_window.uFrameWidth, 0);
                        pButton->uHeight = pTextHeight;
                        pButton->uW = pButton->uY + pTextHeight - 1 + 6;
                        pRealTextHeight +=
                            (pFontArrus->GetHeight() - 3) + pTextHeight;
                    }
                } else {
                    strcpy(pTopicArray[index], "");
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
                __debugbreak();  // first %s on format string is ambiguous state
                                 // of pTmpBuf2
                travel_window.DrawTitleText(
                    pFontArrus, 0, 146, 0,
                    StringPrintf("%s\n \n%s%s%s%s%s", "", pTopicArray[0],
                        pTopicArray[1], pTopicArray[2], pTopicArray[3],
                        pTopicArray[4]),
                    3);
            } else {
                travel_window.DrawTitleText(
                    pFontArrus, 0,
                    (174 -
                        pFontArrus->CalcTextHeight(localization->GetString(561),
                            travel_window.uFrameWidth, 0)) /
                    2 +
                    138,  //"Извините, приходите в другой день"
                    Color16(255, 255, 255), localization->GetString(561), 3);
                pAudioPlayer->StopChannels(-1, -1);
            }
        }
    } else {  //после нажатия топика
        if (dialog_menu_id >= HOUSE_DIALOGUE_TRANSPORT_SCHEDULE_1 &&
            dialog_menu_id <= HOUSE_DIALOGUE_TRANSPORT_SCHEDULE_4) {
            if (pParty->GetGold() < pPrice) {
                GameUI_StatusBar_OnEvent(localization->GetString(LSTR_NOT_ENOUGH_GOLD));
                PlayHouseSound((uint64_t)window_SpeakInHouse->ptr_1C, HouseSound_Greeting_2);
                pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
                return;
            }

            Party::TakeGold(pPrice);

            pTravel =
                &transport_schedule
                [transport_routes[(uint64_t)
                window_SpeakInHouse->ptr_1C -
                HOUSE_STABLES_HARMONDALE]
                [dialog_menu_id -
                HOUSE_DIALOGUE_TRANSPORT_SCHEDULE_1]];
            if (pTravel->pSchedule[pParty->uCurrentDayOfMonth % 7]) {
                if (pCurrentMapName != pMapStats->pInfos[pTravel->uMapInfoID].pFilename) {
                    SaveGame(1, 0);
                    pCurrentMapName = pMapStats->pInfos[pTravel->uMapInfoID].pFilename;

                    dword_6BE364_game_settings_1 |= GAME_SETTINGS_0001;
                    Party_Teleport_Cam_Pitch = 0;
                    Party_Teleport_Z_Speed = 0;
                    Party_Teleport_Cam_Yaw =
                        pTravel->arrival_rot_y;
                    uGameState = GAME_STATE_CHANGE_LOCATION;
                    Party_Teleport_X_Pos = pTravel->arrival_x;
                    Party_Teleport_Y_Pos = pTravel->arrival_y;
                    Party_Teleport_Z_Pos = pTravel->arrival_z;
                    Start_Party_Teleport_Flag = pTravel->arrival_x | pTravel->arrival_y |
                        pTravel->arrival_z | pTravel->arrival_rot_y;
                } else {
                    pIndoorCameraD3D->sRotationZ = 0;

                    pParty->uFlags |= PARTY_FLAGS_1_0002;
                    pParty->vPosition.x = pTravel->arrival_x;
                    pParty->vPosition.y = pTravel->arrival_y;
                    pParty->vPosition.z = pTravel->arrival_z;
                    pParty->uFallStartY = pParty->vPosition.z;
                    pParty->sRotationX = 0;
                    pParty->sRotationZ = pTravel->arrival_rot_y;
                }
                PlayHouseSound((uint64_t)window_SpeakInHouse->ptr_1C,
                    HouseSound_NotEnoughMoney_TrainingSuccessful);
                v12 = pTravel->uTravelTime;
                if ((int64_t)window_SpeakInHouse->ptr_1C >= 63) {
                    pSpeech = SPEECH_SetSail;
                    v13 = 2500;
                    if (CheckHiredNPCSpeciality(Sailor)) v12 -= 2;
                    if (CheckHiredNPCSpeciality(Navigator)) v12 -= 3;
                    if (CheckHiredNPCSpeciality(Pirate)) v12 -= 2;
                } else {
                    pSpeech = SPEECH_CarriageReady;
                    v13 = 1500;
                    if (CheckHiredNPCSpeciality(Horseman)) v12 -= 2;
                }
                if (CheckHiredNPCSpeciality(Explorer)) --v12;
                if (v12 < 1) v12 = 1;
                RestAndHeal(24 * 60 * v12);
                pPlayers[uActiveCharacter]->PlaySound(pSpeech, 0);
                v14 = OS_GetTime();
                v15 = v14 + v13;
                if (v15 < v14) v15 = v14;
                while (OS_GetTime() < v15) OS_Sleep(1);
                while (HouseDialogPressCloseBtn());
                pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);
                return;
            } else {
                dialog_menu_id = HOUSE_DIALOGUE_MAIN;
                pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
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
    unsigned __int16 pTextColor;  // ax@30
    int v29;                      // [sp+10Ch] [bp-10h]@28
    int v31;                      // [sp+114h] [bp-8h]@29
    GUIFont *pOutString;          // [sp+118h] [bp-4h]@21

    GUIWindow townHall_window = *window_SpeakInHouse;
    townHall_window.uFrameX = 483;
    townHall_window.uFrameWidth = 143;
    townHall_window.uFrameZ = 334;

    auto fine_str =
        StringPrintf("%s: %d", localization->GetString(605),
            pParty->uFine);  // Fine: %d   /   Текущий штраф: %d
    townHall_window.DrawTitleText(pFontArrus, 0, 260,
        Color16(0xFFu, 0xFFu, 0x9Bu), fine_str, 3);

    switch (dialog_menu_id) {
    case HOUSE_DIALOGUE_MAIN:
    {
        pStringSum = 1;
        pTextHeight = 0;
        pShopOptions[0] = localization->GetString(
            604);  // Bounty Hunting     Охота за вознаграждение
        if (pParty->uFine > 0) {
            pShopOptions[1] = localization->GetString(
                603);  // Pay fine     Заплатить штраф
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
                pTextColor = Color16(0xFFu, 0xFFu, 0x9Bu);
                if (pDialogueWindow->pCurrentPosActiveItem != v31)
                    pTextColor = Color16(0xFFu, 0xFFu, 0xFFu);
                townHall_window.DrawTitleText(pFontArrus, 0, pButton->uY,
                    pTextColor, pShopOptions[j],
                    3);
                ++v31;
                ++j;
            }
        }
        break;
    }
    case HOUSE_DIALOGUE_TOWNHALL_MESSAGE:
    {
        current_npc_text = StringPrintf(
            bountyHunting_text,
            StringPrintf(
                "\f%05d%s\f%05d", Color16(0xFFu, 0xFFu, 0x9Bu),
                pMonsterStats->pInfos[bountyHunting_monster_id_for_hunting]
                .pName,
                Color16(0xFFu, 0xFFu, 0xFFu)).c_str(),
            100 *
            pMonsterStats->pInfos[bountyHunting_monster_id_for_hunting]
            .uLevel);
        GUIWindow window = *pDialogueWindow;
        window.uFrameWidth = 458;
        window.uFrameZ = 457;
        pOutString = pFontArrus;
        pTextHeight = pFontArrus->CalcTextHeight(current_npc_text,
            window.uFrameWidth, 13) +
            7;
        if (352 - pTextHeight < 8) {
            pOutString = pFontCreate;
            pTextHeight = pFontCreate->CalcTextHeight(
                current_npc_text, window.uFrameWidth, 13) +
                7;
        }
        render->DrawTextureCustomHeight(8 / 640.0f,
            (352 - pTextHeight) / 480.0f,
            ui_leather_mm7, pTextHeight);
        render->DrawTextureAlphaNew(
            8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
        window.DrawText(pOutString, 13, 354 - pTextHeight, 0,
            pOutString->FitTextInAWindow(
                current_npc_text, window.uFrameWidth, 13),
            0, 0, 0);
        break;
    }
    case HOUSE_DIALOGUE_TOWNHALL_PAY_FINE:
    {
        if (window_SpeakInHouse->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_IN_PROGRESS) {
            townHall_window.DrawTitleText(
                pFontArrus, 0, 146, Color16(0xFFu, 0xFFu, 0x9Bu),
                StringPrintf("%s\n%s", localization->GetString(606),
                    localization->GetString(112)),
                3);  // "Pay"   "How Much?"
            townHall_window.DrawTitleText(
                pFontArrus, 0, 186, Color16(0xFFu, 0xFFu, 0xFFu),
                keyboardInputHandler->GetTextInput().c_str(), 3);
            townHall_window.DrawFlashingInputCursor(
                pFontArrus->GetLineWidth(keyboardInputHandler->GetTextInput().c_str()) / 2 + 80,
                185, pFontArrus);
            return;
        }
        if (window_SpeakInHouse->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_CONFIRMED) {
            v1 = atoi(keyboardInputHandler->GetTextInput().c_str());
            v2 = v1;
            if (v1 <= 0) {
                pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
                return;
            }
            if (v1 > pParty->GetGold()) {
                PlayHouseSound(
                    (uint64_t)window_SpeakInHouse->ptr_1C,
                    HouseSound_NotEnoughMoney_TrainingSuccessful);
                v2 = pParty->GetGold();
            }
            if (v2 > pParty->uFine) v2 = pParty->uFine;
            Party::TakeGold(v2);
            pParty->uFine -= v2;
            if (pParty->uFine < 0) pParty->uFine = 0;
            if (uActiveCharacter)
                pPlayers[uActiveCharacter]->PlaySound(SPEECH_81, 0);
        }
        if (window_SpeakInHouse->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_CANCELLED) {
            window_SpeakInHouse->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
        }
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
        pFontArrus, 0, 220, Color16(0xFFu, 0xFFu, 0x9Bu),
        StringPrintf("%s: %d", localization->GetString(25),
            pParty->uNumGoldInBank),
        3);  // Balance   /   Баланс
    switch (dialog_menu_id) {
    case HOUSE_DIALOGUE_MAIN:
    {
        uint16_t pColorText = Color16(0xFFu, 0xFFu, 0x9Bu);
        if (pDialogueWindow->pCurrentPosActiveItem != 2) {
            pColorText = Color16(0xFFu, 0xFFu, 0xFFu);
        }
        bank_window.DrawTitleText(pFontArrus, 0, 146, pColorText,
            localization->GetString(60), 3);
        pColorText = Color16(0xFFu, 0xFFu, 0x9Bu);
        if (pDialogueWindow->pCurrentPosActiveItem != 3) {
            pColorText = Color16(0xFFu, 0xFFu, 0xFFu);
        }
        bank_window.DrawTitleText(pFontArrus, 0, 176, pColorText,
            localization->GetString(244), 3);
        break;
    }
    case HOUSE_DIALOGUE_BANK_PUT_GOLD:
    {
        if (window_SpeakInHouse->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_IN_PROGRESS) {
            bank_window.DrawTitleText(
                pFontArrus, 0, 146, Color16(0xFFu, 0xFFu, 0x9Bu),
                StringPrintf("%s\n%s", localization->GetString(60),
                    localization->GetString(112)),
                3);  // Deposit  How much?   /   "Положить" "Сколько?"
            bank_window.DrawTitleText(pFontArrus, 0, 186,
                Color16(0xFFu, 0xFFu, 0xFFu),
                keyboardInputHandler->GetTextInput().c_str(), 3);
            bank_window.DrawFlashingInputCursor(
                pFontArrus->GetLineWidth(keyboardInputHandler->GetTextInput().c_str()) / 2 + 80,
                185, pFontArrus);
            return;
        }
        if (window_SpeakInHouse->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_CONFIRMED) {
            int entered_sum = atoi(keyboardInputHandler->GetTextInput().c_str());
            unsigned int takes_sum = entered_sum;
            if (!entered_sum) {
                pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
                return;
            }
            if (entered_sum > pParty->GetGold()) {
                PlayHouseSound(
                    (uint64_t)window_SpeakInHouse->ptr_1C,
                    HouseSound_NotEnoughMoney_TrainingSuccessful);
                takes_sum = pParty->GetGold();
            }
            if (takes_sum) {
                Party::TakeGold(takes_sum);
                pParty->uNumGoldInBank += takes_sum;
                if (uActiveCharacter) {
                    pPlayers[uActiveCharacter]->PlaySound(SPEECH_81, 0);
                }
            }
            window_SpeakInHouse->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
        if (window_SpeakInHouse->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_CANCELLED) {
            window_SpeakInHouse->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
        }
        return;
    }
    case HOUSE_DIALOGUE_BANK_GET_GOLD:
    {
        if (window_SpeakInHouse->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_IN_PROGRESS) {
            bank_window.DrawTitleText(
                pFontArrus, 0, 146, Color16(0xFFu, 0xFFu, 0x9Bu),
                StringPrintf("%s\n%s", localization->GetString(244),
                    localization->GetString(112)),
                3);  // Withdraw  How much?   /   "Снять" "Сколько?"
            bank_window.DrawTitleText(pFontArrus, 0, 186,
                Color16(0xFFu, 0xFFu, 0xFFu),
                keyboardInputHandler->GetTextInput().c_str(), 3);
            bank_window.DrawFlashingInputCursor(
                pFontArrus->GetLineWidth(keyboardInputHandler->GetTextInput().c_str()) / 2 + 80,
                185, pFontArrus);
            return;
        }
        if (window_SpeakInHouse->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_CONFIRMED) {
            window_SpeakInHouse->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
            int entered_sum = atoi(keyboardInputHandler->GetTextInput().c_str());
            unsigned int takes_sum = entered_sum;
            if (entered_sum) {
                if (entered_sum > pParty->uNumGoldInBank) {
                    PlayHouseSound(
                        (uint64_t)window_SpeakInHouse->ptr_1C,
                        HouseSound_NotEnoughMoney_TrainingSuccessful);
                    takes_sum = pParty->uNumGoldInBank;
                }
                if (takes_sum) {
                    Party::AddGold(takes_sum);
                    pParty->uNumGoldInBank -= takes_sum;
                }
            }
            window_SpeakInHouse->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
        if (window_SpeakInHouse->keyboard_input_status == WindowInputStatus::WINDOW_INPUT_CANCELLED) {
            window_SpeakInHouse->keyboard_input_status = WindowInputStatus::WINDOW_INPUT_NONE;
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
        }
        return;
    }
    default: { break; }
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
    unsigned int pColorText;  // eax@57
    GUIButton *pButton;       // eax@65
    int pSkillCount;
    signed int pOptionsCount;       // edi@77
    signed int i;                   // esi@79
                                    //  signed int v53; // edi@81
    int v54;                        // edi@81
    const char *pText;              // [sp-4h] [bp-278h]@93
    int v91;                        // [sp+270h] [bp-4h]@3
    unsigned int pTopic1Height;     // [sp+26Fh] [bp-5h]@55
    unsigned __int8 pTopic2Height;  // [sp+267h] [bp-Dh]@57
    unsigned __int8 pTopic3Height;  // [sp+253h] [bp-21h]@59
    unsigned __int8 pTopic4Height = 0;
    int pTextHeight;
    int all_text_height;  // [sp+260h] [bp-14h]@18
    GUIFont *pOutString;

    GUIWindow dialog_window = *window_SpeakInHouse;
    dialog_window.uFrameX = 483;
    dialog_window.uFrameWidth = 143;
    dialog_window.uFrameZ = 334;
    v2 = p2DEvents[(uint64_t)window_SpeakInHouse->ptr_1C - 1].fPriceMultiplier;

    if (uActiveCharacter == 0)  // avoid nzi
        uActiveCharacter = pParty->GetFirstCanAct();

    pPriceRoom = ((v2 * v2) / 10) *
        (100 - pPlayers[uActiveCharacter]->GetMerchant()) /
        100;  // nzi
    if (pPriceRoom < ((v2 * v2) / 10) / 3) pPriceRoom = ((v2 * v2) / 10) / 3;
    if (pPriceRoom <= 0) pPriceRoom = 1;

    pPriceFood = ((v2 * v2) * v2 / 100) *
        (100 - pPlayers[uActiveCharacter]->GetMerchant()) / 100;
    if (pPriceFood < ((v2 * v2) * v2 / 100) / 3)
        pPriceFood = ((v2 * v2) * v2 / 100) / 3;
    if (pPriceFood <= 0) pPriceFood = 1;

    switch (dialog_menu_id) {
    case HOUSE_DIALOGUE_MAIN:
    {
        if (!HouseUI_CheckIfPlayerCanInteract()) return;

        String topic1 =
            StringPrintf("\f%05d",
                pDialogueWindow->pCurrentPosActiveItem == 2
                ? Color16(0xFFu, 0xFFu, 0x9Bu)
                : Color16(0xFFu, 0xFFu, 0xFFu)) +
            localization->FormatString(
                178, pPriceRoom);  // Rent room for %d gold
        pTopic1Height = pFontArrus->CalcTextHeight(
            topic1, dialog_window.uFrameWidth, 0);

        String topic2 =
            StringPrintf("\f%05d",
                pDialogueWindow->pCurrentPosActiveItem == 3
                ? Color16(0xFFu, 0xFFu, 0x9Bu)
                : Color16(0xFFu, 0xFFu, 0xFFu)) +
            localization->FormatString(
                86,  // Buy food for %d days for %d gold
                (unsigned int)
                p2DEvents[(uint64_t)window_SpeakInHouse->ptr_1C - 1]
                .fPriceMultiplier,
                pPriceFood);
        pTopic2Height = pFontArrus->CalcTextHeight(
            topic2, dialog_window.uFrameWidth, 0);

        String topic3 =
            StringPrintf("\f%05d",
                pDialogueWindow->pCurrentPosActiveItem == 4
                ? Color16(0xFFu, 0xFFu, 0x9Bu)
                : Color16(0xFFu, 0xFFu, 0xFFu)) +
            localization->GetString(160);  // Learn Skills
        pTopic3Height = pFontArrus->CalcTextHeight(
            topic3, dialog_window.uFrameWidth, 0);

        String topic4;
        if ((signed int)window_SpeakInHouse->par1C >= 108 &&
            (signed int)window_SpeakInHouse->par1C <= 120) {
            topic4 =
                StringPrintf("\f%05d",
                    pDialogueWindow->pCurrentPosActiveItem == 5
                    ? Color16(0xFFu, 0xFFu, 0x9Bu)
                    : Color16(0xFFu, 0xFFu, 0xFFu)) +
                localization->GetString(611);  // Play Arcomage
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
                StringPrintf("%s\n \n%s\n \n%s\n \n%s", topic1.c_str(),
                    topic2.c_str(), topic3.c_str(),
                    topic4.c_str()),
                3);
        }
        break;
    }
    case HOUSE_DIALOGUE_TAVERN_ARCOMAGE_RULES:
    {
        pOutString = pFontArrus;

        String str = pNPCTopics[354].pText;
        dialog_window.uFrameWidth = game_viewport_width;
        dialog_window.uFrameZ = 452;
        pTextHeight =
            pFontArrus->CalcTextHeight(str, dialog_window.uFrameWidth, 12) +
            7;
        if (352 - pTextHeight < 8) {
            pOutString = pFontCreate;
            pTextHeight = pFontCreate->CalcTextHeight(
                str, dialog_window.uFrameWidth, 12) +
                7;
        }
        render->DrawTextureCustomHeight(8 / 640.0f,
            (352 - pTextHeight) / 480.0f,
            ui_leather_mm7, pTextHeight);
        render->DrawTextureAlphaNew(
            8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
        window_SpeakInHouse->DrawText(
            pOutString, 12, 354 - pTextHeight, 0,
            pOutString->FitTextInAWindow(str, dialog_window.uFrameWidth,
                12),
            0, 0, 0);
        break;
    }

    case HOUSE_DIALOGUE_TAVERN_ARCOMAGE_VICTORY_CONDITIONS:
    {
        String label =
            pNPCTopics[(uint64_t)window_SpeakInHouse->ptr_1C + 247].pText;
        dialog_window.uFrameWidth = game_viewport_width;
        dialog_window.uFrameZ = 452;
        pTextHeight = pFontArrus->CalcTextHeight(
            label, dialog_window.uFrameWidth, 12) +
            7;
        render->DrawTextureCustomHeight(8 / 640.0f,
            (352 - pTextHeight) / 480.0f,
            ui_leather_mm7, pTextHeight);
        render->DrawTextureAlphaNew(
            8 / 640.0f, (347 - pTextHeight) / 480.0f, _591428_endcap);
        window_SpeakInHouse->DrawText(
            pFontArrus, 12, 354 - pTextHeight, 0,
            pFontArrus->FitTextInAWindow(label, dialog_window.uFrameWidth,
                12));
        break;
    }

    case HOUSE_DIALOGUE_TAVERN_ARCOMAGE_RESULT:
    {
        if (pArcomageGame->bGameInProgress == 1) return;
        if (pArcomageGame->uGameWinner) {
            if (pArcomageGame->uGameWinner == 1)
                pText = localization->GetString(640);  // You won!
            else
                pText = localization->GetString(641);  // You lost!
        } else {
            pText = localization->GetString(639);  // A tie!
        }
        dialog_window.DrawTitleText(
            pFontArrus, 0,
            (174 - pFontArrus->CalcTextHeight(
                pText, dialog_window.uFrameWidth, 0)) /
            2 +
            138,
            Color16(0xFFu, 0xFFu, 0x9Bu), pText, 3);
        break;
    }
    case HOUSE_DIALOGUE_TAVERN_REST:
    {
        if (pParty->GetGold() >= pPriceRoom) {
            Party::TakeGold(pPriceRoom);
            PlayHouseSound((uint64_t)window_SpeakInHouse->ptr_1C,
                HouseSound_NotEnoughMoney_TrainingSuccessful);
            dialog_menu_id = HOUSE_DIALOGUE_NULL;
            HouseDialogPressCloseBtn();
            GetHouseGoodbyeSpeech();
            pMediaPlayer->Unload();

            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_RentRoom, (int64_t)window_SpeakInHouse->ptr_1C, 1);
            window_SpeakInHouse->Release();
            window_SpeakInHouse = 0;
            return;
        }
        GameUI_StatusBar_OnEvent(localization->GetString(LSTR_NOT_ENOUGH_GOLD));
        PlayHouseSound((uint64_t)window_SpeakInHouse->ptr_1C, HouseSound_Goodbye);
        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
        break;
    }

    case HOUSE_DIALOGUE_LEARN_SKILLS:
    {
        if (!HouseUI_CheckIfPlayerCanInteract()) return;
        pSkillCount = 0;
        v9 = (signed __int64)(p2DEvents[(uint64_t)
            window_SpeakInHouse->ptr_1C -
            1]
            .flt_24 *
            500.0);
        pPriceSkill =
            v9 * (100 - pPlayers[uActiveCharacter]->GetMerchant()) / 100;
        if (pPriceSkill < v9 / 3) pPriceSkill = v9 / 3;
        all_text_height = 0;
        for (int i = pDialogueWindow->pStartingPosActiveItem;
            i < pDialogueWindow->pStartingPosActiveItem +
            pDialogueWindow->pNumPresenceButton;
            ++i) {
            if (byte_4ED970_skill_learn_ability_by_class_table
                [pPlayers[uActiveCharacter]->classType]
            [pDialogueWindow->GetControl(i)->msg_param - 36] &&
                !pPlayers[uActiveCharacter]->pActiveSkills
                [pDialogueWindow->GetControl(i)->msg_param - 36]) {
                all_text_height = pFontArrus->CalcTextHeight(
                    localization->GetSkillName(
                        pDialogueWindow->GetControl(i)->msg_param - 36),
                    dialog_window.uFrameWidth, 0);
                pSkillCount++;
            }
        }
        if (pSkillCount) {
            auto label = localization->FormatString(
                401, pPriceSkill);  //Стоимость навыка: %lu
            dialog_window.DrawTitleText(pFontArrus, 0, 0x92u, 0, label, 3);
            v91 = (149 - all_text_height) / pSkillCount;
            if ((149 - all_text_height) / pSkillCount > 32) v91 = 32;
            v54 = (149 - pSkillCount * v91 - all_text_height) / 2 -
                v91 / 2 + 162;
            for (int i = pDialogueWindow->pStartingPosActiveItem;
                i < pDialogueWindow->pStartingPosActiveItem +
                pDialogueWindow->pNumPresenceButton;
                i++) {
                pButton = pDialogueWindow->GetControl(i);
                if (!byte_4ED970_skill_learn_ability_by_class_table
                    [pPlayers[uActiveCharacter]->classType]
                [pButton->msg_param - 36] ||
                    pPlayers[uActiveCharacter]
                    ->pActiveSkills[pButton->msg_param - 36]) {
                    pButton->uW = 0;
                    pButton->uHeight = 0;
                    pButton->uY = 0;
                } else {
                    pButton->uY = v91 + v54;
                    pTextHeight = pFontArrus->CalcTextHeight(
                        localization->GetSkillName(pButton->msg_param - 36),
                        dialog_window.uFrameWidth, 0);
                    pButton->uHeight = pTextHeight;
                    v54 = pTextHeight + pButton->uY - 1;
                    pButton->uW = v54 + 6;
                    pColorText = Color16(0xFFu, 0xFFu, 0x9Bu);
                    if (pDialogueWindow->pCurrentPosActiveItem != i)
                        pColorText = Color16(0xFFu, 0xFFu, 0xFFu);
                    dialog_window.DrawTitleText(
                        pFontArrus, 0, pButton->uY, pColorText,
                        localization->GetSkillName(pButton->msg_param - 36),
                        3);
                }
            }
            return;
        }

        String str = localization->FormatString(
            544, pPlayers[uActiveCharacter]->pName,
            localization->GetClassName(
                pPlayers[uActiveCharacter]
                ->classType)) +  // Советую вам %s %s поискать
                                 // знания еще где-нибудь
            "\n \n" +
            localization->GetString(
                528);  //Больше ничего не могу предложить.
        pTextHeight = (174 - pFontArrus->CalcTextHeight(
            str, dialog_window.uFrameWidth, 0)) /
            2 +
            138;
        dialog_window.DrawTitleText(pFontArrus, 0, pTextHeight,
            Color16(0xFFu, 0xFFu, 0x9Bu), str, 3);
        return;
    }

    case HOUSE_DIALOGUE_TAVERN_BUY_FOOD:
    {
        if ((double)pParty->GetFood() >=
            p2DEvents[(uint64_t)window_SpeakInHouse->ptr_1C - 1]
            .fPriceMultiplier) {
            GameUI_StatusBar_OnEvent(localization->GetString(140));  // You already bought food!
            if (uActiveCharacter)
                pPlayers[uActiveCharacter]->PlaySound(SPEECH_67, 0);
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
        if (pParty->GetGold() >= pPriceFood) {
            Party::TakeGold(pPriceFood);
            Party::SetFood(
                p2DEvents[(uint64_t)window_SpeakInHouse->ptr_1C - 1]
                .fPriceMultiplier);
            PlayHouseSound((uint64_t)window_SpeakInHouse->ptr_1C, HouseSound_Greeting_2);
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
        GameUI_StatusBar_OnEvent(localization->GetString(LSTR_NOT_ENOUGH_GOLD));
        PlayHouseSound((uint64_t)window_SpeakInHouse->ptr_1C, HouseSound_Goodbye);
        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
        break;
    }

    case HOUSE_DIALOGUE_TAVERN_ARCOMAGE_MAIN:
    {
        if (HouseUI_CheckIfPlayerCanInteract()) {
            pOptionsCount = 2;
            pShopOptions[0] = localization->GetString(620);  //Правила
            pShopOptions[1] =
                localization->GetString(622);  //Условия победы
            if (pParty->HasItem(
                651)) {  // 651 - Колода для игры в АркоМэйдж в Items.txt
                pShopOptions[2] = localization->GetString(621);  //Играть
                pOptionsCount = 3;
            }
            for (i = 0; i < pOptionsCount; ++i)
                all_text_height = pFontArrus->CalcTextHeight(
                    pShopOptions[i], dialog_window.uFrameWidth, 0);
            all_text_height = (174 - all_text_height) / pOptionsCount;

            v54 = (174 -
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
                pColorText = Color16(0xFFu, 0xFFu, 0x9Bu);
                if (pDialogueWindow->pCurrentPosActiveItem != pItemNum)
                    pColorText = Color16(0xFFu, 0xFFu, 0xFFu);
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
    unsigned __int16 pTextColor;  // ax@21
    DDM_DLV_Header *v26;          // edi@29
    unsigned int v30;             // edx@36
    GUIButton *pButton;           // edi@64
    unsigned __int8 index;        // [sp+1B7h] [bp-Dh]@64
    int v64;                      // [sp+1B8h] [bp-Ch]@6
    unsigned int pCurrentItem;    // [sp+1BCh] [bp-8h]@6
    int all_text_height;          // [sp+1C0h] [bp-4h]@6

    GUIWindow tample_window = *window_SpeakInHouse;
    tample_window.uFrameX = 483;
    tample_window.uFrameWidth = 143;
    tample_window.uFrameZ = 334;

    if (uActiveCharacter == 0) {  // avoid nzi
        uActiveCharacter = pParty->GetFirstCanAct();
    }

    pPrice = pPlayers[uActiveCharacter]->GetTempleHealCostModifier(
        p2DEvents[window_SpeakInHouse->par1C - 1].fPriceMultiplier);
    if (dialog_menu_id == HOUSE_DIALOGUE_MAIN) {
        index = 1;
        pButton = pDialogueWindow->GetControl(
            pDialogueWindow->pStartingPosActiveItem);
        pButton->uHeight = 0;
        pButton->uY = 0;
        if (pPlayers[uActiveCharacter]->IsPlayerHealableByTemple()) {
            static String shop_option_container;
            shop_option_container =
                StringPrintf("%s %d %s", localization->GetString(104), pPrice,
                    localization->GetString(97));  //"Лечить" "Золото"
            pShopOptions[0] = shop_option_container.c_str();
            index = 0;
        }
        pShopOptions[1] = localization->GetString(68);  //"Пожертвовать"
        pShopOptions[2] = localization->GetString(160);  //"Обучиться навыкам"
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
                pTextColor = Color16(0xFFu, 0xFFu, 0x9Bu);
                if (pDialogueWindow->pCurrentPosActiveItem != index + 2)
                    pTextColor = Color16(0xFFu, 0xFFu, 0xFFu);
                tample_window.DrawTitleText(pFontArrus, 0, pButton->uY,
                    pTextColor, pShopOptions[1 * i], 3);
                i++;
                index++;
            }
        }
        return;
    }
    //-------------------------------------------------
    if (dialog_menu_id == HOUSE_DIALOGUE_TEMPLE_HEAL) {
        if (!pPlayers[uActiveCharacter]->IsPlayerHealableByTemple()) return;
        if (pParty->GetGold() < pPrice) {
            GameUI_StatusBar_OnEvent(localization->GetString(LSTR_NOT_ENOUGH_GOLD));
            PlayHouseSound((uint64_t)window_SpeakInHouse->ptr_1C, HouseSound_NotEnoughMoney_TrainingSuccessful);
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
        Party::TakeGold(pPrice);

        pPlayers[uActiveCharacter]->conditions_times.fill(
            GameTime(0));  // sets all condition times to zero
        pPlayers[uActiveCharacter]->sHealth =
            pPlayers[uActiveCharacter]->GetMaxHealth();
        pPlayers[uActiveCharacter]->sMana =
            pPlayers[uActiveCharacter]->GetMaxMana();

        if ((int64_t)window_SpeakInHouse->ptr_1C != 78 &&
            ((int64_t)window_SpeakInHouse->ptr_1C <= 80 ||
            (int64_t)window_SpeakInHouse->ptr_1C > 82)) {
            if (pPlayers[uActiveCharacter]
                ->conditions_times[Condition_Zombie]
                .Valid()) {  // если состояние зомби
                pPlayers[uActiveCharacter]->uCurrentFace =
                    pPlayers[uActiveCharacter]->uPrevFace;
                pPlayers[uActiveCharacter]->uVoiceID =
                    pPlayers[uActiveCharacter]->uPrevVoiceID;
                GameUI_ReloadPlayerPortraits(
                    uActiveCharacter - 1,
                    pPlayers[uActiveCharacter]->uPrevFace);
            }
            pAudioPlayer->PlaySound((SoundID)SOUND_heal, -1, 0, -1, 0, 0);
            pPlayers[uActiveCharacter]->PlaySound(SPEECH_82, 0);
            pOtherOverlayList->_4418B1(20, uActiveCharacter + 99, 0, 65536);
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
        if (pPlayers[uActiveCharacter]
            ->conditions_times[Condition_Zombie]
            .Valid()) {
            // LODWORD(pPlayers[uActiveCharacter]->pConditions[Condition_Zombie])
            // =
            // LODWORD(pPlayers[uActiveCharacter]->pConditions[Condition_Zombie]);
        } else {
            if (!pPlayers[uActiveCharacter]
                ->conditions_times[Condition_Eradicated]
                .Valid() &&
                !pPlayers[uActiveCharacter]
                ->conditions_times[Condition_Pertified]
                .Valid() &&
                !pPlayers[uActiveCharacter]
                ->conditions_times[Condition_Dead]
                .Valid()) {
                pAudioPlayer->PlaySound((SoundID)SOUND_heal, -1, 0, -1, 0, 0);
                pPlayers[uActiveCharacter]->PlaySound(SPEECH_82, 0);
                pOtherOverlayList->_4418B1(20, uActiveCharacter + 99, 0, 65536);
                pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
                return;
            }
            pPlayers[uActiveCharacter]->uPrevFace =
                pPlayers[uActiveCharacter]->uCurrentFace;
            pPlayers[uActiveCharacter]->uPrevVoiceID =
                pPlayers[uActiveCharacter]->uVoiceID;
            pPlayers[uActiveCharacter]->SetCondition(Condition_Zombie, 1);
            pPlayers[uActiveCharacter]->uVoiceID =
                (pPlayers[uActiveCharacter]->GetSexByVoice() != 0) + 23;
            pPlayers[uActiveCharacter]->uCurrentFace =
                (pPlayers[uActiveCharacter]->GetSexByVoice() != 0) + 23;
            GameUI_ReloadPlayerPortraits(
                uActiveCharacter - 1,
                (pPlayers[uActiveCharacter]->GetSexByVoice() != 0) + 23);
            pPlayers[uActiveCharacter]->conditions_times[Condition_Zombie] =
                pParty->GetPlayingTime();
            // v39 = (GUIWindow *)HIDWORD(pParty->uTimePlayed);
        }
        // HIDWORD(pPlayers[uActiveCharacter]->pConditions[Condition_Zombie]) =
        // (int)v39;
        pPlayers[uActiveCharacter]->conditions_times[Condition_Zombie] =
            pParty->GetPlayingTime();
        pAudioPlayer->PlaySound((SoundID)SOUND_heal, -1, 0, -1, 0, 0);
        pPlayers[uActiveCharacter]->PlaySound(SPEECH_82, 0);
        pOtherOverlayList->_4418B1(20, uActiveCharacter + 99, 0, 65536);
        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
        return;
    }
    //---------------------------------------------------
    if (dialog_menu_id == HOUSE_DIALOGUE_TEMPLE_DONATE) {
        pPrice = p2DEvents[(uint64_t)window_SpeakInHouse->ptr_1C - 1].fPriceMultiplier;
        if (pParty->GetGold() >= pPrice) {
            Party::TakeGold(pPrice);
            v26 = &pOutdoor->ddm;
            if (uCurrentlyLoadedLevelType != LEVEL_Outdoor) v26 = &pIndoor->dlv;
            if (v26->uReputation > -5) {
                v26->uReputation = v26->uReputation - 1;
                if (v26->uReputation - 1 < -5) v26->uReputation = -5;
            }
            if ((unsigned __int8)byte_F8B1EF[uActiveCharacter] ==
                pParty->uCurrentDayOfMonth % 7) {
                if (v26->uReputation <= -5) {
                    v30 = pParty->uCurrentDayOfMonth % 7 + 1;
                    v30 |= 0x80;
                    _42777D_CastSpell_UseWand_ShootArrow(
                        SPELL_AIR_WIZARD_EYE, uActiveCharacter - 1, v30, 48, 0);
                }
                if (v26->uReputation <= -10) {
                    v30 = pParty->uCurrentDayOfMonth % 7 + 1;
                    v30 |= 0x80;
                    _42777D_CastSpell_UseWand_ShootArrow(
                        SPELL_SPIRIT_PRESERVATION, uActiveCharacter - 1, v30,
                        48, 0);
                }
                if (v26->uReputation <= -15) {
                    v30 = pParty->uCurrentDayOfMonth % 7 + 1;
                    v30 |= 0x80;
                    _42777D_CastSpell_UseWand_ShootArrow(
                        SPELL_BODY_PROTECTION_FROM_MAGIC, uActiveCharacter - 1,
                        v30, 48, 0);
                }
                if (v26->uReputation <= -20) {
                    v30 = pParty->uCurrentDayOfMonth % 7 + 1;
                    v30 |= 0x80;
                    _42777D_CastSpell_UseWand_ShootArrow(
                        SPELL_LIGHT_HOUR_OF_POWER, uActiveCharacter - 1, v30,
                        48, 0);
                }
                if (v26->uReputation <= -25) {
                    v30 = pParty->uCurrentDayOfMonth % 7 + 1;
                    v30 |= 0x80;
                    _42777D_CastSpell_UseWand_ShootArrow(
                        SPELL_LIGHT_DAY_OF_PROTECTION, uActiveCharacter - 1,
                        v30, 48, 0);
                }
            }
            ++byte_F8B1EF[uActiveCharacter];
            pPlayers[uActiveCharacter]->PlaySound(SPEECH_83, 0);
            GameUI_StatusBar_OnEvent(
                localization->GetString(527));  // "Thank You!"
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
        GameUI_StatusBar_OnEvent(localization->GetString(LSTR_NOT_ENOUGH_GOLD));
        PlayHouseSound((uint64_t)window_SpeakInHouse->ptr_1C,
            HouseSound_NotEnoughMoney_TrainingSuccessful);
        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
        return;
    }
    //------------------------------------------------
    if (dialog_menu_id == HOUSE_DIALOGUE_LEARN_SKILLS) {
        if (HouseUI_CheckIfPlayerCanInteract()) {
            all_text_height = 0;
            pCurrentItem =
                (signed __int64)(p2DEvents[(int64_t)
                    window_SpeakInHouse->ptr_1C -
                    1]
                    .flt_24 *
                    500.0);
            v64 = (signed int)(pCurrentItem *
                (100 -
                    pPlayers[uActiveCharacter]->GetMerchant())) /
                100;
            if (v64 < (signed int)pCurrentItem / 3)
                v64 = (signed int)pCurrentItem / 3;
            pCurrentItem = 0;
            for (int i = pDialogueWindow->pStartingPosActiveItem;
                i < pDialogueWindow->pNumPresenceButton +
                pDialogueWindow->pStartingPosActiveItem;
                ++i) {
                if (byte_4ED970_skill_learn_ability_by_class_table
                    [pPlayers[uActiveCharacter]->classType]
                [pDialogueWindow->GetControl(i)->msg_param - 36] &&
                    !pPlayers[uActiveCharacter]->pActiveSkills
                    [pDialogueWindow->GetControl(i)->msg_param - 36]) {
                    all_text_height += pFontArrus->CalcTextHeight(
                        localization->GetSkillName(
                            pDialogueWindow->GetControl(i)->msg_param - 36),
                        tample_window.uFrameWidth, 0);
                    ++pCurrentItem;
                }
            }
            if (pCurrentItem) {
                auto str = localization->FormatString(401, v64);
                tample_window.DrawTitleText(pFontArrus, 0, 0x92u, 0, str, 3);
                v64 = (149 - (signed int)all_text_height) /
                    (signed int)pCurrentItem;
                if (v64 > 32) v64 = 32;
                all_text_height = (signed int)(149 - pCurrentItem * v64 -
                    (int)all_text_height) /
                    2 -
                    v64 / 2 + 162;
                if (pDialogueWindow->pStartingPosActiveItem <
                    pDialogueWindow->pStartingPosActiveItem +
                    pDialogueWindow->pNumPresenceButton) {
                    pCurrentItem = 2;
                    for (int i = pDialogueWindow->pStartingPosActiveItem;
                        i < pDialogueWindow->pNumPresenceButton +
                        pDialogueWindow->pStartingPosActiveItem;
                        ++i) {
                        pButton = pDialogueWindow->GetControl(i);
                        if (!byte_4ED970_skill_learn_ability_by_class_table
                            [pPlayers[uActiveCharacter]->classType]
                        [pButton->msg_param - 36] ||
                            pPlayers[uActiveCharacter]
                            ->pActiveSkills[pButton->msg_param - 36]) {
                            pButton->uW = 0;
                            pButton->uHeight = 0;
                            pButton->uY = 0;
                        } else {
                            pButton->uY = v64 + all_text_height;
                            pTextHeight = pFontArrus->CalcTextHeight(
                                localization->GetSkillName(pButton->msg_param -
                                    36),
                                tample_window.uFrameWidth, 0);
                            pButton->uHeight = pTextHeight;
                            pButton->uW = pButton->uY + pTextHeight - 1 + 6;
                            all_text_height = pButton->uW;
                            pTextColor = Color16(0xFFu, 0xFFu, 0x9Bu);
                            if (pDialogueWindow->pCurrentPosActiveItem !=
                                pCurrentItem)
                                pTextColor = Color16(0xFFu, 0xFFu, 0xFFu);
                            tample_window.DrawTitleText(
                                pFontArrus, 0, pButton->uY, pTextColor,
                                localization->GetSkillName(pButton->msg_param -
                                    36),
                                3);
                        }
                        pCurrentItem++;
                    }
                }
            } else {
                String str =
                    localization->FormatString(
                        544, pPlayers[uActiveCharacter]->pName,
                        localization->GetClassName(
                            pPlayers[uActiveCharacter]
                            ->classType)) +  // Советую вам %s %s поискать
                                             // знания еще где-нибудь
                    "\n \n" +
                    localization->GetString(
                        528);  //Больше ничего не могу предложить.

                pTextHeight = pFontArrus->CalcTextHeight(
                    str, tample_window.uFrameWidth, 0);
                tample_window.DrawTitleText(
                    pFontArrus, 0, (174 - pTextHeight) / 2 + 138,
                    Color16(0xFFu, 0xFFu, 0x9Bu), str, 3);
            }
        }
    }
}

void TrainingDialog(const char *s) {
    unsigned __int64 v5;  // edi@3
    int v8;               // edx@4
    double v9;            // st7@6
    signed int v10;       // esi@6
    int pPrice = 0;           // ecx@6
    int v14;              // esi@14
    int v19;              // ecx@24
    int v33;              // eax@36
    unsigned int v36;     // eax@38
    unsigned int v42;     // eax@46
    int index;
    int all_text_height;          // eax@68
    int v49;                      // ebx@69
    GUIButton *pButton;           // eax@71
    int pTextHeight;              // eax@71
    unsigned __int16 pTextColor;  // ax@71
    int v69;                      // [sp+70h] [bp-14h]@6
    int v73;                      // [sp+80h] [bp-4h]@14

    GUIWindow training_dialog_window = *window_SpeakInHouse;
    training_dialog_window.uFrameX = 483;
    training_dialog_window.uFrameWidth = 143;
    training_dialog_window.uFrameZ = 334;

    if (uActiveCharacter == 0)  // avoid nzi
        uActiveCharacter = pParty->GetFirstCanAct();

    v5 = 1000ull * pPlayers[uActiveCharacter]->uLevel *
        (pPlayers[uActiveCharacter]->uLevel + 1) /
        2;  // E n = n(n + 1) / 2
            // v68 = pMaxLevelPerTrainingHallType[(unsigned
            // int)window_SpeakInHouse->ptr_1C -
            // HOUSE_TRAINING_HALL_EMERALD_ISLE];
    if (pPlayers[uActiveCharacter]->uExperience >= v5) {
        v8 = pPlayers[uActiveCharacter]->classType % 4 + 1;
        if (v8 == 4) v8 = 3;
        v9 = (double)pPlayers[uActiveCharacter]->uLevel;
        v69 = v8;
        v10 = (signed __int64)(v9 *
            p2DEvents[(int64_t)window_SpeakInHouse->ptr_1C - 1]
            .fPriceMultiplier *
            (double)v8);
        pPrice = v10 * (100 - pPlayers[uActiveCharacter]->GetMerchant()) / 100;
        if (pPrice < v10 / 3) pPrice = v10 / 3;
    }
    //-------------------------------------------------------
    all_text_height = 0;
    if (HouseUI_CheckIfPlayerCanInteract()) {
        if (dialog_menu_id == HOUSE_DIALOGUE_MAIN) {
            if (HouseUI_CheckIfPlayerCanInteract()) {
                index = 0;
                pShopOptions[0] = s;  // set first item to fucntion param - this
                                      // always gets overwritten below??
                pShopOptions[1] =
                    localization->GetString(160);  // "Learn Skills"
                if (pDialogueWindow->pStartingPosActiveItem <
                    pDialogueWindow->pStartingPosActiveItem +
                    pDialogueWindow->pNumPresenceButton) {
                    for (int i = pDialogueWindow->pStartingPosActiveItem;
                        i < pDialogueWindow->pNumPresenceButton +
                        pDialogueWindow->pStartingPosActiveItem;
                        ++i) {
                        if (pDialogueWindow->GetControl(i)->msg_param ==
                            HOUSE_DIALOGUE_TRAININGHALL_TRAIN) {
                            static String shop_option_str_container;
                            if (pPlayers[uActiveCharacter]->uLevel >=
                                pMaxLevelPerTrainingHallType
                                [(uint64_t)window_SpeakInHouse->ptr_1C -
                                HOUSE_TRAINING_HALL_EMERALD_ISLE]) {
                                shop_option_str_container =
                                    String(localization->GetString(536)) +
                                    "\n \n" +
                                    localization->GetString(
                                        529);  // "With your skills, you should
                                               // be working here as a teacher."
                                               // "Sorry, but we are unable to
                                               // train you."
                                pShopOptions[index] =
                                    shop_option_str_container.c_str();

                            } else {
                                if (pPlayers[uActiveCharacter]->uExperience <
                                    v5)
                                    shop_option_str_container =
                                    localization->FormatString(
                                        LSTR_XP_UNTIL_NEXT_LEVEL,
                                        (uint)(v5 - pPlayers[uActiveCharacter]->uExperience),
                                        pPlayers[uActiveCharacter]->uLevel + 1);
                                else
                                    shop_option_str_container =
                                    localization->FormatString(
                                        537,
                                        pPlayers[uActiveCharacter]->uLevel + 1,
                                        pPrice);  // "Train to level %d for
                                                  // %d gold"
                                pShopOptions[index] =
                                    shop_option_str_container.c_str();
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
                        pTextColor = Color16(0xE1u, 0xCDu, 0x23u);
                        if (pDialogueWindow->pCurrentPosActiveItem != i)
                            pTextColor = Color16(255, 255, 255);
                        training_dialog_window.DrawTitleText(
                            pFontArrus, 0, pButton->uY, pTextColor,
                            pShopOptions[index], 3);
                        ++index;
                    }
                }
            }
        }
        //------------------------------------------------------------------
        if (dialog_menu_id == HOUSE_DIALOGUE_TRAININGHALL_TRAIN) {
            String label;

            if (!HouseUI_CheckIfPlayerCanInteract()) {
                v33 = pFontArrus->CalcTextHeight(
                    pNPCTopics[122].pText, training_dialog_window.uFrameWidth,
                    0);
                training_dialog_window.DrawTitleText(
                    pFontArrus, 0, (212 - v33) / 2 + 101,
                    Color16(0xE1u, 0xCDu, 0x23u), pNPCTopics[122].pText, 3);
                pDialogueWindow->pNumPresenceButton = 0;
                return;
            }
            if (pPlayers[uActiveCharacter]->uLevel <
                pMaxLevelPerTrainingHallType
                [(uint64_t)window_SpeakInHouse->ptr_1C -
                HOUSE_TRAINING_HALL_EMERALD_ISLE]) {
                if ((signed __int64)pPlayers[uActiveCharacter]->uExperience >=
                    v5) {
                    if (pParty->GetGold() >= pPrice) {
                        Party::TakeGold(pPrice);
                        PlayHouseSound(
                            (uint64_t)window_SpeakInHouse->ptr_1C,
                            HouseSound_NotEnoughMoney_TrainingSuccessful);
                        ++pPlayers[uActiveCharacter]->uLevel;
                        pPlayers[uActiveCharacter]->uSkillPoints +=
                            pPlayers[uActiveCharacter]->uLevel / 10 + 5;
                        pPlayers[uActiveCharacter]->sHealth =
                            pPlayers[uActiveCharacter]->GetMaxHealth();
                        pPlayers[uActiveCharacter]->sMana =
                            pPlayers[uActiveCharacter]->GetMaxMana();
                        uint max_level_in_party = player_levels[0];
                        for (uint _it = 1; _it < 4; ++_it) {
                            if (player_levels[_it] > max_level_in_party)
                                max_level_in_party = player_levels[_it];
                        }
                        ++player_levels[uActiveCharacter - 1];
                        if (player_levels[uActiveCharacter - 1] >
                            max_level_in_party) {  // if we reach new maximum
                                                   // party level feature is
                                                   // broken thou, since this
                                                   // array is always zeroed in
                                                   // EnterHouse
                            v42 = 60 * (_494820_training_time(
                                pParty->uCurrentHour) +
                                4) -
                                pParty->uCurrentMinute;
                            if ((uint64_t)window_SpeakInHouse->ptr_1C ==
                                HOUSE_TRAINING_HALL_94 ||
                                (uint64_t)window_SpeakInHouse->ptr_1C ==
                                HOUSE_TRAINING_HALL_95)
                                v42 += 12 * 60;
                            RestAndHeal(v42 + 7 * 24 * 60);
                            if (uCurrentlyLoadedLevelType == LEVEL_Outdoor)
                                pOutdoor->SetFog();
                        }
                        pPlayers[uActiveCharacter]->PlaySound(SPEECH_87, 0);

                        GameUI_StatusBar_OnEvent(localization->FormatString(
                            430, pPlayers[uActiveCharacter]->pName,
                            pPlayers[uActiveCharacter]->uLevel,
                            pPlayers[uActiveCharacter]->uLevel / 10 +
                            5));  // %s is now Level %lu and has earned %lu
                                  // Skill Points!

                        pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
                        return;
                    }

                    GameUI_StatusBar_OnEvent(localization->GetString(LSTR_NOT_ENOUGH_GOLD));
                    PlayHouseSound((uint64_t)window_SpeakInHouse->ptr_1C, (HouseSoundID)4);
                    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
                    return;
                }
                label = localization->FormatString(
                    LSTR_XP_UNTIL_NEXT_LEVEL,
                    (unsigned int)(v5 - pPlayers[uActiveCharacter]->uExperience),
                    pPlayers[uActiveCharacter]->uLevel + 1);
                v36 =
                    (212 - pFontArrus->CalcTextHeight(
                        label, training_dialog_window.uFrameWidth, 0)) /
                    2 +
                    88;
            } else {
                label = StringPrintf(
                    "%s\n \n%s",
                    localization->GetString(
                        536),  // With your skills, you should be working here
                               // as a teacher.
                    localization->GetString(
                        529));  // Sorry, but we are unable to train you.
                v36 =
                    (212 - pFontArrus->CalcTextHeight(
                        label, training_dialog_window.uFrameWidth, 0)) /
                    2 +
                    101;
            }
            training_dialog_window.DrawTitleText(
                pFontArrus, 0, v36, Color16(0xE1u, 0xCDu, 0x23u), label, 3);

            PlayHouseSound((uint64_t)window_SpeakInHouse->ptr_1C,
                (HouseSoundID)3);
            pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, 0);
            return;
        }
    }

    //-------------------------------------------------------------
    if (dialog_menu_id == HOUSE_DIALOGUE_LEARN_SKILLS) {
        if (HouseUI_CheckIfPlayerCanInteract()) {
            v14 = (signed __int64)(p2DEvents[(uint64_t)
                window_SpeakInHouse->ptr_1C -
                1]
                .flt_24 *
                500.0);
            pPrice =
                v14 * (100 - pPlayers[uActiveCharacter]->GetMerchant()) / 100;
            if (pPrice < v14 / 3) pPrice = v14 / 3;
            index = 0;
            for (int i = pDialogueWindow->pStartingPosActiveItem;
                (signed int)i < pDialogueWindow->pNumPresenceButton +
                pDialogueWindow->pStartingPosActiveItem;
                ++i) {
                if (byte_4ED970_skill_learn_ability_by_class_table
                    [pPlayers[uActiveCharacter]->classType]
                [pDialogueWindow->GetControl(i)->msg_param - 36] &&
                    !pPlayers[uActiveCharacter]->pActiveSkills
                    [pDialogueWindow->GetControl(i)->msg_param - 36]) {
                    all_text_height += pFontArrus->CalcTextHeight(
                        localization->GetSkillName(
                            pDialogueWindow->GetControl(i)->msg_param - 36),
                        training_dialog_window.uFrameWidth, 0);
                    ++index;
                }
            }
            if (index) {
                training_dialog_window.DrawTitleText(
                    pFontArrus, 0, 0x92u, 0,
                    localization->FormatString(401, pPrice),
                    3);  // "Skill Cost: %lu"
                v73 = (signed int)(149 - all_text_height) / index;
                if (v73 > 32) v73 = 32;
                v19 = (signed int)(149 - index * v73 - all_text_height) / 2 -
                    v73 / 2 + 162;
                for (int i = pDialogueWindow->pStartingPosActiveItem;
                    (signed int)i < pDialogueWindow->pStartingPosActiveItem +
                    pDialogueWindow->pNumPresenceButton;
                    ++i) {
                    pButton = pDialogueWindow->GetControl(i);
                    if (!byte_4ED970_skill_learn_ability_by_class_table
                        [pPlayers[uActiveCharacter]->classType]
                    [pButton->msg_param - 36] ||
                        pPlayers[uActiveCharacter]
                        ->pActiveSkills[pButton->msg_param - 36]) {
                        pButton->uW = 0;
                        pButton->uHeight = 0;
                        pButton->uY = 0;
                    } else {
                        pButton->uY = v73 + v19;
                        pTextHeight = pFontArrus->CalcTextHeight(
                            localization->GetSkillName(pButton->msg_param - 36),
                            training_dialog_window.uFrameWidth, 0);
                        pButton->uHeight = pTextHeight;
                        pButton->uW = pButton->uY + pTextHeight - 1 + 6;
                        v19 = pButton->uY + pTextHeight - 1;
                        pTextColor = Color16(0xE1u, 0xCDu, 0x23u);
                        if (pDialogueWindow->pCurrentPosActiveItem != i)
                            pTextColor = Color16(255, 255, 255);
                        training_dialog_window.DrawTitleText(
                            pFontArrus, 0, pButton->uY, pTextColor,
                            localization->GetSkillName(pButton->msg_param - 36),
                            3);
                    }
                }
            } else {
                auto label = localization->FormatString(
                    544, pPlayers[uActiveCharacter]->pName,
                    localization->GetClassName(
                        pPlayers[uActiveCharacter]
                        ->classType)) +  // Seek knowledge
                                         // elsewhere %s the %s
                    "\n \n" +
                    localization->GetString(
                        528);  // I can offer you nothing further.
                auto label_height = pFontArrus->CalcTextHeight(
                    label, training_dialog_window.uFrameWidth, 0);
                training_dialog_window.DrawTitleText(
                    pFontArrus, 0, (174 - label_height) / 2 + 138,
                    Color16(0xE1u, 0xCDu, 0x23u), label, 3);
            }
        }
    }
    return;
}

void sub_4B6478() {
    signed int v3;                // esi@1
    int pPrice;                   // ebx@1
    unsigned int v5;              // esi@5
    short *v6;                       // edi@6
    int all_text_height;          // eax@20
    GUIButton *pButton;           // esi@27
    int pTextHeight;              // eax@29
    unsigned __int16 pTextColor;  // ax@29
    int v27;                      // [sp-4h] [bp-80h]@8
    int v32;                      // [sp+6Ch] [bp-10h]@1
    int index;                    // [sp+74h] [bp-8h]@17

    GUIWindow dialog_window = *window_SpeakInHouse;
    dialog_window.uFrameX = 483;
    dialog_window.uFrameWidth = 143;
    dialog_window.uFrameZ = 334;

    v32 =
        (unsigned __int8)(((p2DEvents
            [(uint64_t)window_SpeakInHouse->ptr_1C - 1]
    .uType != 18) -
            1) &
            0x96) +
        100;
    v3 = (signed __int64)((double)v32 *
        p2DEvents[(uint64_t)window_SpeakInHouse->ptr_1C -
        1]
        .fPriceMultiplier);
    pPrice = v3 * (100 - pPlayers[uActiveCharacter]->GetMerchant()) / 100;
    if (pPrice < v3 / 3) pPrice = v3 / 3;
    if (dialog_menu_id == HOUSE_DIALOGUE_MAIN) {
        if (!(unsigned __int16)_449B57_test_bit(
            (unsigned __int8 *)pPlayers[uActiveCharacter]
            ->_achieved_awards_bits,
            word_4F0754[2 * (uint64_t)window_SpeakInHouse->ptr_1C])) {
            pTextHeight = pFontArrus->CalcTextHeight(
                pNPCTopics[171].pText, dialog_window.uFrameWidth, 0);
            dialog_window.DrawTitleText(
                pFontArrus, 0, (212 - pTextHeight) / 2 + 101,
                Color16(0xFFu, 0xFFu, 0x9Bu), pNPCTopics[171].pText, 3);
            pDialogueWindow->pNumPresenceButton = 0;
            return;
        }
        if (!HouseUI_CheckIfPlayerCanInteract()) return;
        all_text_height = 0;
        index = 0;
        for (int i = pDialogueWindow->pStartingPosActiveItem;
            i < pDialogueWindow->pNumPresenceButton +
            pDialogueWindow->pStartingPosActiveItem;
            ++i) {
            if (byte_4ED970_skill_learn_ability_by_class_table
                [pPlayers[uActiveCharacter]->classType / 3]
            [pDialogueWindow->GetControl(i)->msg_param - 36] &&
                !pPlayers[uActiveCharacter]
                ->pActiveSkills[pDialogueWindow->GetControl(i)->msg_param -
                36]) {
                all_text_height += pFontArrus->CalcTextHeight(
                    localization->GetSkillName(
                        pDialogueWindow->GetControl(i)->msg_param - 36),
                    dialog_window.uFrameWidth, 0);
                ++index;
            }
        }
        if (!index) {
            String str = localization->FormatString(
                544, pPlayers[uActiveCharacter]->pName,
                localization->GetClassName(
                    pPlayers[uActiveCharacter]->classType)) +
                "\n \n" +
                localization->GetString(
                    528);  // Больше ничего не могу предложить.
            pTextHeight =
                pFontArrus->CalcTextHeight(str, dialog_window.uFrameWidth, 0);
            dialog_window.DrawTitleText(pFontArrus, 0,
                (174 - pTextHeight) / 2 + 138,
                Color16(0xFFu, 0xFFu, 0x9Bu), str, 3);
            return;
        }

        auto skill_price_label =
            localization->FormatString(401, pPrice);  //Стоимость навыка: %lu
        dialog_window.DrawTitleText(pFontArrus, 0, 0x92u, 0, skill_price_label,
            3);

        v32 = (149 - all_text_height) / index;
        if (v32 > 32) v32 = 32;
        index = (149 - index * v32 - all_text_height) / 2 - v32 / 2 + 162;
        if (pDialogueWindow->pStartingPosActiveItem <
            pDialogueWindow->pStartingPosActiveItem +
            pDialogueWindow->pNumPresenceButton) {
            for (int i = pDialogueWindow->pStartingPosActiveItem;
                i < pDialogueWindow->pNumPresenceButton +
                pDialogueWindow->pStartingPosActiveItem;
                ++i) {
                pButton = pDialogueWindow->GetControl(i);
                if (byte_4ED970_skill_learn_ability_by_class_table
                    [pPlayers[uActiveCharacter]->classType / 3]
                [pButton->msg_param - 36]) {
                    if (!pPlayers[uActiveCharacter]
                        ->pActiveSkills[pButton->msg_param - 36]) {
                        pButton->uY = v32 + index;
                        pTextHeight = pFontArrus->CalcTextHeight(
                            localization->GetSkillName(pButton->msg_param - 36),
                            dialog_window.uFrameWidth, 0);
                        pButton->uHeight = pTextHeight;
                        pButton->uW = pButton->uY + pTextHeight - 1 + 6;
                        index = pButton->uY + pTextHeight - 1;
                        pTextColor = Color16(0xFFu, 0xFFu, 0x9Bu);
                        if (pDialogueWindow->pCurrentPosActiveItem != i)
                            pTextColor = Color16(0xFFu, 0xFFu, 0xFFu);
                        dialog_window.DrawTitleText(
                            pFontArrus, 0, pButton->uY, pTextColor,
                            localization->GetSkillName(pButton->msg_param - 36),
                            3);
                    }
                } else {
                    pButton->uW = 0;
                    pButton->uHeight = 0;
                    pButton->uY = 0;
                }
            }
        }
        return;
    }

    if (HouseUI_CheckIfPlayerCanInteract()) {
        v5 = 0;
        __debugbreak();
        // pSkillAvailabilityPerClass[8 + v58->uClass][4 + v23]
        // or
        // byte_4ED970_skill_learn_ability_by_class_table[v58->uClass][v23 - 36]
        // or
        // byte_4ED970_skill_learn_ability_by_class_table[v58->uClass - 1][v23 +
        // 1]
        __debugbreak();  // whacky condition - fix
        if (false
            // if ( !*(&byte_4ED94C[37 * v1->uClass / 3] + dword_F8B19C)
            || (v6 = (short *)(&pPlayers[uActiveCharacter]->uIntelligence +
                dialog_menu_id),
                *(short *)v6)) {
            pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
        } else {
            if (pParty->GetGold() < pPrice) {
                GameUI_StatusBar_OnEvent(localization->GetString(LSTR_NOT_ENOUGH_GOLD));
                v27 = 4;
            } else {
                Party::TakeGold(pPrice);
                *(short *)v6 = 1;
                v27 = 2;
            }
            PlayHouseSound((uint64_t)window_SpeakInHouse->ptr_1C, (HouseSoundID)v27);
        }
    } else {
        v5 = 0;
    }
    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 1, v5);
    return;  // void func
}

void SimpleHouseDialog() {
    NPCData *pNPC;       // esi@6
    char *v15;           // esi@14
    GUIButton *pButton;  // eax@15
    char *v29;           // esi@42
    int v31;             // ST1C_4@42
    int index;           // esi@51
    int v36;
    signed int all_text_height;   // ecx@54
    int v40;                      // edi@57
    unsigned __int16 pTextColor;  // ax@60
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
                411, pMapStats->pInfos[uHouse_ExitPic]
                .pName.c_str());  // Enter %s   /   Войти в ^Pv[%s]
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
    pNPC = HouseNPCData[(uint64_t)((char *)pDialogueNPCCount +
        -(dword_591080 != 0))];  //- 1

    String npc_name;
    if (pNPC->uProfession)
        npc_name = localization->FormatString(
            429, pNPC->pName,
            localization->GetNpcProfessionName(
                pNPC->uProfession));  // %s %s   /   ^Pi[%s] %s
    else
        npc_name = pNPC->pName;
    house_window.DrawTitleText(pFontCreate, 483, 113,
        Color16(0x15u, 0x99u, 0xE9u), npc_name, 3);

    if (!dword_591080) {
        if (!uDialogueType) {
            if (pNPC->greet) {
                house_window.uFrameWidth = game_viewport_width;
                house_window.uFrameZ = 452;
                pInString = pNPCStats->pNPCGreetings[pNPC->greet]
                    .pGreetings[((pNPC->uFlags & 3) == 2)];
                // pInString = (char *)*(&pNPCStats->field_17884 +
                // ((pNPC->uFlags & 3) == 2) + 2 * pNPC->greet);
                render->DrawTextureCustomHeight(
                    8 / 640.0f,
                    (352 - (pFontArrus->CalcTextHeight(
                        pInString, house_window.uFrameWidth, 13) +
                        7)) /
                    480.0f,
                    ui_leather_mm7,
                    (pFontArrus->CalcTextHeight(pInString,
                        house_window.uFrameWidth, 13) +
                        7));

                int h = (pFontArrus->CalcTextHeight(
                    pInString, house_window.uFrameWidth, 13) +
                    7);
                render->DrawTextureAlphaNew(8 / 640.0f, (347 - h) / 480.0f,
                    _591428_endcap);
                pDialogueWindow->DrawText(
                    pFontArrus, 13, 354 - h, 0,
                    pFontArrus->FitTextInAWindow(pInString,
                        house_window.uFrameWidth, 13),
                    0, 0, 0);
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
        case 19:  // evt_A
            v15 = (char *)pNPCTopics[pNPC->evt_A].pTopic;
            if (!v15) {
                pButton->msg_param = 0;
                v15 = "";
            }
            pButton->sLabel = v15;
            continue;
        case 20:  // evt_B
            v15 = (char *)pNPCTopics[pNPC->evt_B].pTopic;
            if (!v15) {
                pButton->msg_param = 0;
                v15 = "";
            }
            if (uDialogueType != 84) {
                pButton->sLabel = v15;
                continue;
            }
            current_npc_text = StringPrintf(
                current_npc_text.c_str(),
                StringPrintf("\f%05d%s\f00000\n",
                    Color16(0xE1u, 0xCDu, 0x23u),
                    pItemsTable->pItems[contract_approved]
                    .pUnidentifiedName)
                .c_str());
            pButton->sLabel = v15;
            continue;

        case 21:  // evt_C
            v15 = (char *)pNPCTopics[pNPC->evt_C].pTopic;
            if (!v15) {
                pButton->msg_param = 0;
                v15 = "";
            }
            pButton->sLabel = v15;
            continue;
        case 22:  // evt_D
            v15 = (char *)pNPCTopics[pNPC->evt_D].pTopic;
            if (!v15) {
                pButton->msg_param = 0;
                v15 = "";
            }
            pButton->sLabel = v15;
            continue;
        case 23:  // evt_E
            v15 = (char *)pNPCTopics[pNPC->evt_E].pTopic;
            if (!v15) {
                pButton->msg_param = 0;
                v15 = "";
            }
            pButton->sLabel = v15;
            continue;
        case 24:  // evt_F
            v15 = (char *)pNPCTopics[pNPC->evt_F].pTopic;
            if (!v15) {
                pButton->msg_param = 0;
                v15 = "";
            }
            pButton->sLabel = v15;
            continue;
        case 76:
            pButton->sLabel =
                localization->GetString(406);  // Hire / Нанять
            continue;
        case 77:
            pButton->sLabel =
                localization->GetString(407);  // Details / Подробнее
            continue;
        case 79:
            pButton->sLabel =
                _4B254D_SkillMasteryTeacher((int64_t)right_panel_window.ptr_1C);
            continue;
        case 82:
            pButton->sLabel =
                ContractSelectText((int64_t)right_panel_window.ptr_1C);
            continue;
        case 83:
            v29 =
                pMonsterStats->pInfos[bountyHunting_monster_id_for_hunting]
                .pName;
            v31 = *(int *)v29;

            current_npc_text = StringPrintf(
                bountyHunting_text,
                StringPrintf("\f%05d%s\f%05d", Color16(0xFFu, 0xFFu, 0x9Bu),
                    v31, Color16(0xFFu, 0xFFu, 0xFFu))
                .c_str(),
                100 * (unsigned __int8)v29[8]);

            pButton->sLabel.clear();
            continue;
        }
        if (pButton->msg_param > 0 && pButton->msg_param < 13) {
            pButton->sLabel = localization->GetString(122);  // Вступить
            continue;
        }
        if (pButton->msg_param > 13 && pButton->msg_param < 19) {
            pButton->sLabel.clear();
            continue;
        }
        if (pButton->msg_param != 93) {
            pButton->sLabel.clear();
            continue;
        }
    }
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
            pTextColor = Color16(0xE1u, 0xCDu, 0x23u);
            if (pDialogueWindow->pCurrentPosActiveItem != i)
                pTextColor = Color16(0xFFu, 0xFFu, 0xFFu);
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
        render->DrawTextureAlphaNew(8 / 640.0f, (347 - pTextHeight) / 480.0f,
            _591428_endcap);
        house_window.DrawText(
            pTextFont, 13, 354 - pTextHeight, 0,
            pTextFont->FitTextInAWindow(current_npc_text, w.uFrameWidth, 13), 0,
            0, 0);
    }
}

void JailDialog() {
    GUIWindow jail_dialogue_window = *window_SpeakInHouse;
    jail_dialogue_window.uFrameX = 483;
    jail_dialogue_window.uFrameWidth = 143;
    jail_dialogue_window.uFrameZ = 334;
    jail_dialogue_window.DrawTitleText(
        pFontArrus, 0,
        (310 - pFontArrus->CalcTextHeight(localization->GetString(672),
            jail_dialogue_window.uFrameWidth,
            0)) /
        2 +
        18,
        Color16(0xFFu, 0xFFu, 0x9Bu), localization->GetString(672),
        3);  //"За многочисленные преступления и злодеяния вы были приговорены к
             //одному году заключения."
}

void InitializeBuildingResidents() {
    int i;
    char *test_string;
    unsigned char c;
    bool break_loop;
    unsigned int temp_str_len;
    char *tmp_pos;
    int decode_step;

    free(p2DEventsTXT_Raw);
    p2DEventsTXT_Raw = (char *)pEvents_LOD->LoadCompressedTexture("2dEvents.txt");
    strtok(p2DEventsTXT_Raw, "\r");
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
                    if (!_strnicmp(test_string, "wea", 3)) {
                        p2DEvents[i].uType = BuildingType_WeaponShop;
                        break;
                    }
                    if (!_strnicmp(test_string, "arm", 3)) {
                        p2DEvents[i].uType = BuildingType_ArmorShop;
                        break;
                    }
                    if (!_strnicmp(test_string, "mag", 3)) {
                        p2DEvents[i].uType = BuildingType_MagicShop;
                        break;
                    }
                    if (!_strnicmp(test_string, "alc", 3)) {
                        p2DEvents[i].uType = BuildingType_AlchemistShop;
                        break;
                    }
                    if (!_strnicmp(test_string, "sta", 3)) {
                        p2DEvents[i].uType = BuildingType_Stables;
                        break;
                    }
                    if (!_strnicmp(test_string, "boa", 3)) {
                        p2DEvents[i].uType = BuildingType_Boats;
                        break;
                    }
                    if (!_strnicmp(test_string, "tem", 3)) {
                        p2DEvents[i].uType = BuildingType_Temple;
                        break;
                    }
                    if (!_strnicmp(test_string, "tra", 3)) {
                        p2DEvents[i].uType = BuildingType_Training;
                        break;
                    }
                    if (!_strnicmp(test_string, "tow", 3)) {
                        p2DEvents[i].uType = BuildingType_TownHall;
                        break;
                    }

                    if (!_strnicmp(test_string, "tav", 3)) {
                        p2DEvents[i].uType = BuildingType_Tavern;
                        break;
                    }
                    if (!_strnicmp(test_string, "ban", 3)) {
                        p2DEvents[i].uType = BuildingType_Bank;
                        break;
                    }
                    if (!_strnicmp(test_string, "fir", 3)) {
                        p2DEvents[i].uType = BuildingType_FireGuild;
                        break;
                    }
                    if (!_strnicmp(test_string, "air", 3)) {
                        p2DEvents[i].uType = BuildingType_AirGuild;
                        break;
                    }
                    if (!_strnicmp(test_string, "wat", 3)) {
                        p2DEvents[i].uType = BuildingType_WaterGuild;
                        break;
                    }
                    if (!_strnicmp(test_string, "ear", 3)) {
                        p2DEvents[i].uType = BuildingType_EarthGuild;
                        break;
                    }
                    if (!_strnicmp(test_string, "spi", 3)) {
                        p2DEvents[i].uType = BuildingType_SpiritGuild;
                        break;
                    }
                    if (!_strnicmp(test_string, "min", 3)) {
                        p2DEvents[i].uType = BuildingType_MindGuild;
                        break;
                    }
                    if (!_strnicmp(test_string, "bod", 3)) {
                        p2DEvents[i].uType = BuildingType_BodyGuild;
                        break;
                    }
                    if (!_strnicmp(test_string, "lig", 3)) {
                        p2DEvents[i].uType = BuildingType_LightGuild;
                        break;
                    }
                    if (!_strnicmp(test_string, "dar", 3)) {
                        p2DEvents[i].uType = BuildingType_DarkGuild;
                        break;
                    }
                    if (!_strnicmp(test_string, "ele",
                        3)) {  // "Element Guild" from mm6
                        p2DEvents[i].uType = BuildingType_ElementalGuild;
                        break;
                    }
                    if (!_strnicmp(test_string, "sel", 3)) {
                        p2DEvents[i].uType = BuildingType_SelfGuild;
                        break;
                    }
                    if (!_strnicmp(test_string, "mir", 3)) {
                        p2DEvents[i].uType = BuildingType_16;
                        break;
                    }
                    if (!_strnicmp(test_string, "mer",
                        3)) {  // "Merc Guild" from mm6
                        p2DEvents[i].uType = BuildingType_TownHall;
                        break;
                    }
                    p2DEvents[i].uType = BuildingType_18;
                } break;

                case 4:
                    p2DEvents[i].uAnimationID = atoi(test_string);
                    break;
                case 5:
                    p2DEvents[i].pName = RemoveQuotes(test_string);
                    break;
                case 6:
                    p2DEvents[i].pProprieterName =
                        RemoveQuotes(test_string);
                    break;
                case 7:
                    p2DEvents[i].pProprieterTitle =
                        RemoveQuotes(test_string);
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
                    p2DEvents[i]._quest_related = atoi(test_string);
                    break;
                case 23:
                    p2DEvents[i].pEnterText = RemoveQuotes(test_string);
                    break;
                }
            }
            ++decode_step;
            test_string = tmp_pos + 1;
        } while ((decode_step < 24) && !break_loop);
    }
}

int HouseDialogPressCloseBtn() {
    pMessageQueue_50CBD0->Flush();
    keyboardInputHandler->SetWindowInputStatus(WindowInputStatus::WINDOW_INPUT_CANCELLED);
    keyboardInputHandler->ResetKeys();
    activeLevelDecoration = nullptr;
    current_npc_text.clear();
    if (pDialogueNPCCount == 0) return 0;

    if (dialog_menu_id == HOUSE_DIALOGUE_SHOP_BUY_SPECIAL &&
        shop_ui_background) {
        shop_ui_background->Release();
        shop_ui_background = nullptr;
    }

    switch (dialog_menu_id) {
    case -1:
        _4B4224_UpdateNPCTopics((int64_t)((char *)pDialogueNPCCount - 1));
        BackToHouseMenu();
        break;

    case HOUSE_DIALOGUE_SHOP_DISPLAY_EQUIPMENT:
    case HOUSE_DIALOGUE_LEARN_SKILLS:
    case HOUSE_DIALOGUE_TAVERN_ARCOMAGE_MAIN:
        BackToHouseMenu();
        UI_CreateEndConversationButton();
        dialog_menu_id = HOUSE_DIALOGUE_MAIN;
        InitializaDialogueOptions(in_current_building_type);
        break;

    case HOUSE_DIALOGUE_SHOP_SELL:
    case HOUSE_DIALOGUE_SHOP_IDENTIFY:
    case HOUSE_DIALOGUE_SHOP_REPAIR:
        UI_CreateEndConversationButton();
        dialog_menu_id = HOUSE_DIALOGUE_SHOP_DISPLAY_EQUIPMENT;
        InitializaDialogueOptions_Shops(in_current_building_type);
        break;

    case HOUSE_DIALOGUE_TAVERN_ARCOMAGE_RULES:
    case HOUSE_DIALOGUE_TAVERN_ARCOMAGE_VICTORY_CONDITIONS:
    case HOUSE_DIALOGUE_TAVERN_ARCOMAGE_RESULT:
        BackToHouseMenu();
        UI_CreateEndConversationButton();
        dialog_menu_id = HOUSE_DIALOGUE_TAVERN_ARCOMAGE_MAIN;
        InitializaDialogueOptions_Tavern(in_current_building_type);
        break;

    case HOUSE_DIALOGUE_NULL:
    case HOUSE_DIALOGUE_MAIN:
        pDialogueNPCCount = 0;
        pDialogueWindow->Release();
        dialog_menu_id = HOUSE_DIALOGUE_NULL;
        pDialogueWindow = 0;

        if (uNumDialogueNPCPortraits == 1) return 0;

        pBtn_ExitCancel = window_SpeakInHouse->vButtons.front();
        if (uNumDialogueNPCPortraits > 0) {
            for (uint i = 0; i < (unsigned int)uNumDialogueNPCPortraits;
                ++i) {
                HouseNPCPortraitsButtonsList[i] =
                    window_SpeakInHouse->CreateButton(
                        pNPCPortraits_x[uNumDialogueNPCPortraits - 1][i],
                        pNPCPortraits_y[uNumDialogueNPCPortraits - 1][i],
                        63, 73, 1, 0, UIMSG_ClickHouseNPCPortrait, i, GameKey::None,
                        byte_591180[i].data());
            }
        }

        BackToHouseMenu();
        break;

    default:
        BackToHouseMenu();
        dialog_menu_id = HOUSE_DIALOGUE_MAIN;
        InitializaDialogueOptions(in_current_building_type);
        break;
    }
    return 1;
}

void BackToHouseMenu() {
    auto pMouse = EngineIoc::ResolveMouse();
    pMouse->ClearPickedItem();
    if (window_SpeakInHouse && window_SpeakInHouse->ptr_1C == (void *)165 &&
        !pMovie_Track) {
        bGameoverLoop = true;
        HouseDialogPressCloseBtn();
        window_SpeakInHouse->Release();
        pParty->uFlags &= 0xFFFFFFFD;
        if (EnterHouse(HOUSE_BODY_GUILD_ERATHIA)) {
            pAudioPlayer->PlaySound(SOUND_Invalid, 0, 0, -1, 0, 0);
            window_SpeakInHouse = new GUIWindow_House(0, 0, window->GetWidth(), window->GetHeight(), (GUIButton*)165, "");
            window_SpeakInHouse->CreateButton(0x3Du, 0x1A8u, 0x1Fu, 0, 2, 94, UIMSG_SelectCharacter, 1, GameKey::Digit1);
            window_SpeakInHouse->CreateButton(0xB1u, 0x1A8u, 0x1Fu, 0, 2, 94, UIMSG_SelectCharacter, 2, GameKey::Digit2);
            window_SpeakInHouse->CreateButton(0x124u, 0x1A8u, 0x1Fu, 0, 2, 94, UIMSG_SelectCharacter, 3, GameKey::Digit3);
            window_SpeakInHouse->CreateButton(0x197u, 0x1A8u, 0x1Fu, 0, 2, 94, UIMSG_SelectCharacter, 4, GameKey::Digit4);
        }
        bGameoverLoop = false;
    }
}

//----- (004BE571) --------------------------------------------------------
int sub_4BE571_AddItemToSet(int valueToAdd, int *outPutSet,
    int elemsAlreadyPresent, int elemsNeeded) {
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
    unsigned int v29;  // edx@56
    int v15;           // ecx@19
    int v33;           // [sp-4h] [bp-2Ch]@23
    int v34;           // [sp-4h] [bp-2Ch]@43
    int v21;           // ecx@34
    int v35[5];        // [sp+Ch] [bp-1Ch]@8
    int v37 = 0;       // [sp+24h] [bp-4h]@1*
    int i = 0;

    _F8B1DC_currentShopOption = 0;

    switch (type) {
    case BuildingType_WeaponShop:
    {
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 4; ++j) {
                if (i)
                    v21 = shopWeap_variation_spc
                    [(uint64_t)window_SpeakInHouse->ptr_1C]
                .item_class[j];
                else
                    v21 = shopWeap_variation_ord
                    [(uint64_t)window_SpeakInHouse->ptr_1C]
                .item_class[j];

                switch (v21) {
                case 23:
                    v34 = 37;
                    break;
                case 24:
                    v34 = 38;
                    break;
                case 25:
                    v34 = 39;
                    break;
                case 26:
                    v34 = 40;
                    break;
                case 27:
                    v34 = 41;
                    break;
                case 28:
                    v34 = 42;
                    break;
                case 30:
                    v34 = 36;
                    break;
                default:
                    continue;
                }
                v37 = sub_4BE571_AddItemToSet(v34, v35, v37, 5);
            }
        }
    } break;

    case BuildingType_ArmorShop:
    {
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 4; ++k) {
                    if (i)
                        v15 =
                        shopArmr_variation_spc
                        [(uint64_t)window_SpeakInHouse->ptr_1C -
                        15 + j]
                    .item_class[k];
                    else
                        v15 =
                        shopArmr_variation_ord
                        [(uint64_t)window_SpeakInHouse->ptr_1C -
                        15 + j]
                    .item_class[k];
                    switch (v15) {
                    case 31:
                        v33 = 45;
                        break;
                    case 32:
                        v33 = 46;
                        break;
                    case 33:
                        v33 = 47;
                        break;
                    case 34:
                        v33 = 44;
                        break;
                    default:
                        continue;
                    }
                    v37 = sub_4BE571_AddItemToSet(v33, v35, v37, 5);
                }
            }
        }
    } break;

    case BuildingType_MagicShop:
        v37 = 2;
        v35[0] = 57;
        v35[1] = 59;
        break;
    case BuildingType_AlchemistShop:
        v37 = 2;
        v35[0] = 71;
        v35[1] = 68;
        break;
    case BuildingType_Tavern:
        v37 = 3;
        v35[0] = 70;
        v35[1] = 65;
        v35[2] = 62;
        break;
    case BuildingType_Temple:
        v37 = 3;
        v35[0] = 67;
        v35[1] = 66;
        v35[2] = 58;
        break;
    case BuildingType_Training:
        v37 = 2;
        v35[0] = 69;
        v35[1] = 60;
        break;
    }

    for (i = 0; i < v37; ++i) {
        v29 = v35[i];
        switch (v29) {
        case 40:
            v30 = localization->GetSkillName(4);
            break;
        case 5:
            v30 = localization->GetSkillName(23);
            break;
        case 36:
            v30 = localization->GetSkillName(0);
            break;
        case 37:
            v30 = localization->GetSkillName(1);
            break;
        case 38:
            v30 = localization->GetSkillName(2);
            break;
        case 39:
            v30 = localization->GetSkillName(3);
            break;
        case 41:
            v30 = localization->GetSkillName(5);
            break;
        case 42:
            v30 = localization->GetSkillName(6);
            break;
        case 44:
            v30 = localization->GetSkillName(8);
            break;
        case 45:
            v30 = localization->GetSkillName(9);
            break;
        case 46:
            v30 = localization->GetSkillName(10);
            break;
        case 47:
            v30 = localization->GetSkillName(11);
            break;
        case 66:
            v30 = localization->GetSkillName(30);
            break;
        case 57:
            v30 = localization->GetSkillName(21);
            break;
        case 58:
            v30 = localization->GetSkillName(22);
            break;
        case 60:
            v30 = localization->GetSkillName(24);
            break;
        case 62:
            v30 = localization->GetSkillName(26);
            break;
        case 65:
            v30 = localization->GetSkillName(29);
            break;
        case 67:
            v30 = localization->GetSkillName(31);
            break;
        case 68:
            v30 = localization->GetSkillName(32);
            break;
        case 69:
            v30 = localization->GetSkillName(33);
            break;
        case 70:
            v30 = localization->GetSkillName(34);
            break;
        case 71:
            v30 = localization->GetSkillName(35);
            break;
        default:
            v30 = localization->GetString(127);  // "No Text!"
        }
        pShopOptions[_F8B1DC_currentShopOption] = v30;
        ++_F8B1DC_currentShopOption;
        CreateButtonInColumn(i + 1, v29);
    }
    pDialogueWindow->_41D08F_set_keyboard_control_group(i, 1, 0, 2);
    dword_F8B1E0 = pDialogueWindow->pNumPresenceButton;
}

//----- (004B8F94) --------------------------------------------------------
void GenerateSpecialShopItems() {
    signed int item_count;
    signed int shop_index;
    int treasure_lvl = 0;
    int item_class = 0;
    int mdf;

    shop_index = (int64_t)window_SpeakInHouse->ptr_1C;
    if (uItemsAmountPerShopType[p2DEvents[shop_index - 1].uType]) {
        for (item_count = 0;
            item_count <
            uItemsAmountPerShopType[p2DEvents[shop_index - 1].uType];
            ++item_count) {
            if (shop_index <= 14) {  // weapon shop
                treasure_lvl =
                    shopWeap_variation_spc[shop_index].treasure_level;
                item_class =
                    shopWeap_variation_spc[shop_index].item_class[rand() % 4];
            } else if (shop_index <= 28) {  // armor shop
                mdf = 0;
                if (item_count > 3) ++mdf;
                treasure_lvl =
                    shopArmr_variation_spc[2 * (shop_index - 15) + mdf]
                    .treasure_level;
                item_class = shopArmr_variation_spc[2 * (shop_index - 15) + mdf]
                    .item_class[rand() % 4];
            } else if (shop_index <= 41) {  // magic shop
                treasure_lvl = shopMagicSpc_treasure_lvl[shop_index - 28];
                item_class = 22;          // misc
            } else if (shop_index <= 53) {  // alchemist shop
                if (item_count < 6) {
                    pParty->SpecialItemsInShops[shop_index][item_count].Reset();
                    pParty->SpecialItemsInShops[shop_index][item_count]
                        .uItemID =
                        rand() % 32 + ITEM_RECIPE_REJUVENATION;  // mscrool
                    continue;
                } else {
                    treasure_lvl = shopAlchSpc_treasure_lvl[shop_index - 41];
                    item_class = 44;  // potion
                }
            }
            pItemsTable->GenerateItem(
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
    int treasure_lvl = 0;
    int item_class = 0;
    int mdf;

    shop_index = (int64_t)window_SpeakInHouse->ptr_1C;
    if (uItemsAmountPerShopType[p2DEvents[shop_index - 1].uType]) {
        for (item_count = 0;
            item_count <
            uItemsAmountPerShopType[p2DEvents[shop_index - 1].uType];
            ++item_count) {
            if (shop_index <= 14) {  // weapon shop
                treasure_lvl =
                    shopWeap_variation_ord[shop_index].treasure_level;
                item_class =
                    shopWeap_variation_ord[shop_index].item_class[rand() % 4];
            } else if (shop_index <= 28) {  // armor shop
                mdf = 0;
                if (item_count > 3) ++mdf;  // rechek offsets
                treasure_lvl =
                    shopArmr_variation_ord[2 * (shop_index - 15) + mdf]
                    .treasure_level;
                item_class = shopArmr_variation_ord[2 * (shop_index - 15) + mdf]
                    .item_class[rand() % 4];
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
            pItemsTable->GenerateItem(
                treasure_lvl, item_class,
                &pParty->StandartItemsInShops[shop_index][item_count]);
            pParty->StandartItemsInShops[shop_index][item_count]
                .SetIdentified();  // identified
        }
    }
    pParty->InTheShopFlags[shop_index] = 0;
}

GUIWindow_House::GUIWindow_House(unsigned int x, unsigned int y, unsigned int width, unsigned int height, GUIButton *button, const String &hint) :
    GUIWindow(WINDOW_HouseInterior, x, y, width, height, button, hint) {
    pEventTimer->Pause();  // pause timer so not attacked
    pAudioPlayer->StopChannels(-1, -1);

    current_screen_type = CURRENT_SCREEN::SCREEN_HOUSE;
    pBtn_ExitCancel = CreateButton(471, 445, 169, 35, 1, 0, UIMSG_Escape, 0, GameKey::None, localization->GetString(80),  // Quit building
        { { ui_exit_cancel_button_background } });
    for (int v26 = 0; v26 < uNumDialogueNPCPortraits; ++v26) {
        const char *v29;
        String v30;
        if (v26 + 1 == uNumDialogueNPCPortraits && uHouse_ExitPic) {
            v30 = pMapStats->pInfos[uHouse_ExitPic].pName;
            v29 = localization->GetString(411);  // Enter %s
        } else {
            if (v26 || !dword_591080)
                v30 = HouseNPCData[v26 + 1 - ((dword_591080 != 0) ? 1 : 0)]->pName;
            else
                v30 = p2DEvents[(int64_t)button - 1].pProprieterName;
            v29 = localization->GetString(435);
        }
        sprintf(byte_591180[v26].data(), v29, v30.c_str());
        HouseNPCPortraitsButtonsList[v26] = CreateButton(pNPCPortraits_x[uNumDialogueNPCPortraits - 1][v26],
            pNPCPortraits_y[uNumDialogueNPCPortraits - 1][v26],
            63, 73, 1, 0, UIMSG_ClickHouseNPCPortrait, v26, GameKey::None, byte_591180[v26].data());
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
    if (window_SpeakInHouse->par1C >= 53)
        return;
    if (pParty->PartyTimes._shop_ban_times[window_SpeakInHouse->par1C] <= pParty->GetPlayingTime()) {
        if (window_SpeakInHouse->par1C < 53)
            pParty->PartyTimes._shop_ban_times[window_SpeakInHouse->par1C] = GameTime(0);
        return;
    }
    // dialog_menu_id = HOUSE_DIALOGUE_MAIN;
    pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, 0, 0);  // banned from shop so leaving
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
    if (engine->config->flip_on_exit) {
        pParty->sRotationZ = (TrigLUT->uIntegerDoublePi - 1) & (TrigLUT->uIntegerPi + pParty->sRotationZ);
        pIndoorCameraD3D->sRotationZ = pParty->sRotationZ;
    }
    pParty->uFlags |= PARTY_FLAGS_1_0002;

    GUIWindow::Release();
}
