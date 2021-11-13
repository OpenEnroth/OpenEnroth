#include "Engine/mm7_data.h"

#include "Engine/Engine.h"

#include "GUI/GUIWindow.h"
#include "Party.h"

#include "Engine/Autonotes.h"
#include "Engine/Awards.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/stru123.h"
#include "Engine/stru160.h"
#include "Engine/stru298.h"
#include "Engine/stru314.h"
#include "Engine/stru367.h"
#include "MapInfo.h"
#include "OurMath.h"

struct TrigTableLookup* TrigLUT = new TrigTableLookup;
struct MapStats *pMapStats;
struct Viewport *pViewport = new Viewport;
struct ViewingParams *viewparams = new ViewingParams;
stru123 stru_5E4C90_MapPersistVars;
stru298 AttackerInfo;
std::array<Autonote, 196> pAutonoteTxt;
std::array<Award, 105> pAwards;
std::array<AwardType, 1000> achieved_awards;
int num_achieved_awards;
int full_num_items_in_book;

std::array<stru160, 66> array_4EB8B8 = {  // vertices for spheres
    {stru160(0.0000000f, 0.0000000f, 1.0000000f),
     stru160(0.0000000f, 0.3826830f, 0.9238799f),
     stru160(0.3826830f, 0.0000000f, 0.9238799f),
     stru160(0.4082480f, 0.4082480f, 0.8164970f),
     stru160(0.0000000f, 0.7071070f, 0.7071070f),
     stru160(0.7071070f, 0.0000000f, 0.7071070f),
     stru160(0.4082480f, 0.8164970f, 0.4082480f),
     stru160(0.8164970f, 0.4082480f, 0.4082480f),
     stru160(0.7071070f, 0.7071070f, 0.0000000f),
     stru160(0.0000000f, 0.9238799f, 0.3826830f),
     stru160(0.3826830f, 0.9238799f, 0.0000000f),
     stru160(0.0000000f, 1.0000000f, 0.0000000f),
     stru160(0.9238799f, 0.3826830f, 0.0000000f),
     stru160(0.9238799f, 0.0000000f, 0.3826830f),
     stru160(1.0000000f, 0.0000000f, 0.0000000f),
     stru160(0.0000000f, -0.3826830f, 0.9238799f),
     stru160(0.4082480f, -0.4082480f, 0.8164970f),
     stru160(0.0000000f, -0.7071070f, 0.7071070f),
     stru160(0.8164970f, -0.4082480f, 0.4082480f),
     stru160(0.4082480f, -0.8164970f, 0.4082480f),
     stru160(0.7071070f, -0.7071070f, 0.0000000f),
     stru160(0.9238799f, -0.3826830f, 0.0000000f),
     stru160(0.3826830f, -0.9238799f, 0.0000000f),
     stru160(0.0000000f, -0.9238799f, 0.3826830f),
     stru160(0.0000000f, -1.0000000f, 0.0000000f),
     stru160(-0.3826830f, 0.0000000f, 0.9238799f),
     stru160(-0.4082480f, -0.4082480f, 0.8164970f),
     stru160(-0.7071070f, 0.0000000f, 0.7071070f),
     stru160(-0.4082480f, -0.8164970f, 0.4082480f),
     stru160(-0.8164970f, -0.4082480f, 0.4082480f),
     stru160(-0.7071070f, -0.7071070f, 0.0000000f),
     stru160(-0.3826830f, -0.9238799f, 0.0000000f),
     stru160(-0.9238799f, -0.3826830f, 0.0000000f),
     stru160(-0.9238799f, 0.0000000f, 0.3826830f),
     stru160(-1.0000000f, 0.0000000f, 0.0000000f),
     stru160(-0.4082480f, 0.4082480f, 0.8164970f),
     stru160(-0.8164970f, 0.4082480f, 0.4082480f),
     stru160(-0.4082480f, 0.8164970f, 0.4082480f),
     stru160(-0.7071070f, 0.7071070f, 0.0000000f),
     stru160(-0.9238799f, 0.3826830f, 0.0000000f),
     stru160(-0.3826830f, 0.9238799f, 0.0000000f),
     stru160(0.0000000f, 0.0000000f, -1.0000000f),
     stru160(0.0000000f, 0.3826830f, -0.9238799f),
     stru160(0.3826830f, 0.0000000f, -0.9238799f),
     stru160(0.4082480f, 0.4082480f, -0.8164970f),
     stru160(0.0000000f, 0.7071070f, -0.7071070f),
     stru160(0.7071070f, 0.0000000f, -0.7071070f),
     stru160(0.4082480f, 0.8164970f, -0.4082480f),
     stru160(0.8164970f, 0.4082480f, -0.4082480f),
     stru160(0.0000000f, 0.9238799f, -0.3826830f),
     stru160(0.9238799f, 0.0000000f, -0.3826830f),
     stru160(0.0000000f, -0.3826830f, -0.9238799f),
     stru160(0.4082480f, -0.4082480f, -0.8164970f),
     stru160(0.0000000f, -0.7071070f, -0.7071070f),
     stru160(0.8164970f, -0.4082480f, -0.4082480f),
     stru160(0.4082480f, -0.8164970f, -0.4082480f),
     stru160(0.0000000f, -0.9238799f, -0.3826830f),
     stru160(-0.3826830f, 0.0000000f, -0.9238799f),
     stru160(-0.4082480f, -0.4082480f, -0.8164970f),
     stru160(-0.7071070f, 0.0000000f, -0.7071070f),
     stru160(-0.4082480f, -0.8164970f, -0.4082480f),
     stru160(-0.8164970f, -0.4082480f, -0.4082480f),
     stru160(-0.9238799f, 0.0000000f, -0.3826830f),
     stru160(-0.4082480f, 0.4082480f, -0.8164970f),
     stru160(-0.8164970f, 0.4082480f, -0.4082480f),
     stru160(-0.4082480f, 0.8164970f, -0.4082480f)}};
std::array<stru160, 128> array_4EBBD0;
std::array<__int32, 128 * 3> array_4EBBD0_x = {  // indicies for triangle in sphere
    {0x00000000, 0x00000001, 0x00000002, 0x00000001, 0x00000003, 0x00000002,
     0x00000001, 0x00000004, 0x00000003, 0x00000003, 0x00000005, 0x00000002,
     0x00000004, 0x00000006, 0x00000003, 0x00000006, 0x00000007, 0x00000003,
     0x00000006, 0x00000008, 0x00000007, 0x00000007, 0x00000005, 0x00000003,
     0x00000004, 0x00000009, 0x00000006, 0x00000009, 0x0000000A, 0x00000006,
     0x00000009, 0x0000000B, 0x0000000A, 0x0000000A, 0x00000008, 0x00000006,
     0x00000008, 0x0000000C, 0x00000007, 0x0000000C, 0x0000000D, 0x00000007,
     0x0000000C, 0x0000000E, 0x0000000D, 0x0000000D, 0x00000005, 0x00000007,
     0x00000000, 0x00000002, 0x0000000F, 0x00000002, 0x00000010, 0x0000000F,
     0x00000002, 0x00000005, 0x00000010, 0x00000010, 0x00000011, 0x0000000F,
     0x00000005, 0x00000012, 0x00000010, 0x00000012, 0x00000013, 0x00000010,
     0x00000012, 0x00000014, 0x00000013, 0x00000013, 0x00000011, 0x00000010,
     0x00000005, 0x0000000D, 0x00000012, 0x0000000D, 0x00000015, 0x00000012,
     0x0000000D, 0x0000000E, 0x00000015, 0x00000015, 0x00000014, 0x00000012,
     0x00000014, 0x00000016, 0x00000013, 0x00000016, 0x00000017, 0x00000013,
     0x00000016, 0x00000018, 0x00000017, 0x00000017, 0x00000011, 0x00000013,
     0x00000000, 0x0000000F, 0x00000019, 0x0000000F, 0x0000001A, 0x00000019,
     0x0000000F, 0x00000011, 0x0000001A, 0x0000001A, 0x0000001B, 0x00000019,
     0x00000011, 0x0000001C, 0x0000001A, 0x0000001C, 0x0000001D, 0x0000001A,
     0x0000001C, 0x0000001E, 0x0000001D, 0x0000001D, 0x0000001B, 0x0000001A,
     0x00000011, 0x00000017, 0x0000001C, 0x00000017, 0x0000001F, 0x0000001C,
     0x00000017, 0x00000018, 0x0000001F, 0x0000001F, 0x0000001E, 0x0000001C,
     0x0000001E, 0x00000020, 0x0000001D, 0x00000020, 0x00000021, 0x0000001D,
     0x00000020, 0x00000022, 0x00000021, 0x00000021, 0x0000001B, 0x0000001D,
     0x00000000, 0x00000019, 0x00000001, 0x00000019, 0x00000023, 0x00000001,
     0x00000019, 0x0000001B, 0x00000023, 0x00000023, 0x00000004, 0x00000001,
     0x0000001B, 0x00000024, 0x00000023, 0x00000024, 0x00000025, 0x00000023,
     0x00000024, 0x00000026, 0x00000025, 0x00000025, 0x00000004, 0x00000023,
     0x0000001B, 0x00000021, 0x00000024, 0x00000021, 0x00000027, 0x00000024,
     0x00000021, 0x00000022, 0x00000027, 0x00000027, 0x00000026, 0x00000024,
     0x00000026, 0x00000028, 0x00000025, 0x00000028, 0x00000009, 0x00000025,
     0x00000028, 0x0000000B, 0x00000009, 0x00000009, 0x00000004, 0x00000025,
     0x00000029, 0x0000002A, 0x0000002B, 0x0000002A, 0x0000002C, 0x0000002B,
     0x0000002A, 0x0000002D, 0x0000002C, 0x0000002C, 0x0000002E, 0x0000002B,
     0x0000002D, 0x0000002F, 0x0000002C, 0x0000002F, 0x00000030, 0x0000002C,
     0x0000002F, 0x00000008, 0x00000030, 0x00000030, 0x0000002E, 0x0000002C,
     0x0000002D, 0x00000031, 0x0000002F, 0x00000031, 0x0000000A, 0x0000002F,
     0x00000031, 0x0000000B, 0x0000000A, 0x0000000A, 0x00000008, 0x0000002F,
     0x00000008, 0x0000000C, 0x00000030, 0x0000000C, 0x00000032, 0x00000030,
     0x0000000C, 0x0000000E, 0x00000032, 0x00000032, 0x0000002E, 0x00000030,
     0x00000029, 0x0000002B, 0x00000033, 0x0000002B, 0x00000034, 0x00000033,
     0x0000002B, 0x0000002E, 0x00000034, 0x00000034, 0x00000035, 0x00000033,
     0x0000002E, 0x00000036, 0x00000034, 0x00000036, 0x00000037, 0x00000034,
     0x00000036, 0x00000014, 0x00000037, 0x00000037, 0x00000035, 0x00000034,
     0x0000002E, 0x00000032, 0x00000036, 0x00000032, 0x00000015, 0x00000036,
     0x00000032, 0x0000000E, 0x00000015, 0x00000015, 0x00000014, 0x00000036,
     0x00000014, 0x00000016, 0x00000037, 0x00000016, 0x00000038, 0x00000037,
     0x00000016, 0x00000018, 0x00000038, 0x00000038, 0x00000035, 0x00000037,
     0x00000029, 0x00000033, 0x00000039, 0x00000033, 0x0000003A, 0x00000039,
     0x00000033, 0x00000035, 0x0000003A, 0x0000003A, 0x0000003B, 0x00000039,
     0x00000035, 0x0000003C, 0x0000003A, 0x0000003C, 0x0000003D, 0x0000003A,
     0x0000003C, 0x0000001E, 0x0000003D, 0x0000003D, 0x0000003B, 0x0000003A,
     0x00000035, 0x00000038, 0x0000003C, 0x00000038, 0x0000001F, 0x0000003C,
     0x00000038, 0x00000018, 0x0000001F, 0x0000001F, 0x0000001E, 0x0000003C,
     0x0000001E, 0x00000020, 0x0000003D, 0x00000020, 0x0000003E, 0x0000003D,
     0x00000020, 0x00000022, 0x0000003E, 0x0000003E, 0x0000003B, 0x0000003D,
     0x00000029, 0x00000039, 0x0000002A, 0x00000039, 0x0000003F, 0x0000002A,
     0x00000039, 0x0000003B, 0x0000003F, 0x0000003F, 0x0000002D, 0x0000002A,
     0x0000003B, 0x00000040, 0x0000003F, 0x00000040, 0x00000041, 0x0000003F,
     0x00000040, 0x00000026, 0x00000041, 0x00000041, 0x0000002D, 0x0000003F,
     0x0000003B, 0x0000003E, 0x00000040, 0x0000003E, 0x00000027, 0x00000040,
     0x0000003E, 0x00000022, 0x00000027, 0x00000027, 0x00000026, 0x00000040,
     0x00000026, 0x00000028, 0x00000041, 0x00000028, 0x00000031, 0x00000041,
     0x00000028, 0x0000000B, 0x00000031, 0x00000031, 0x0000002D, 0x00000041}};

#include "MapsLongTimer.h"
MapsLongTimer MapsLongTimersList[100];

#include "Engine/Tables/IconFrameTable.h"
struct IconFrameTable *pIconsFrameTable;

#include "Engine/Tables/PlayerFrameTable.h"
struct PlayerFrameTable *pPlayerFrameTable;  // idb

std::array<stat_coord, 26> stat_string_coord =  // 4E2940
    {{
        {0x1A, 0x39, 0xDC, 0x12},   {0x1A, 0x4A, 0xDC, 0x12},
        {0x1A, 0x5B, 0xDC, 0x12},   {0x1A, 0x6C, 0xDC, 0x12},
        {0x1A, 0x7D, 0xDC, 0x12},   {0x1A, 0x8E, 0xDC, 0x12},
        {0x1A, 0x9F, 0xDC, 0x12},   {0x1A, 0xC6, 0xDC, 0x12},
        {0x1A, 0xD7, 0xDC, 0x12},   {0x1A, 0xE8, 0xDC, 0x12},
        {0x1A, 0x10C, 0xDC, 0x12},  {0x1A, 0x11E, 0xDC, 0x12},
        {0x111, 0x36, 0xBA, 0x12},  {0x111, 0x47, 0xBA, 0x12},
        {0x111, 0x58, 0xBA, 0x12},  {0x111, 0x7E, 0xBA, 0x12},
        {0x111, 0x8F, 0xBA, 0x12},  {0x111, 0xA0, 0xBA, 0x12},
        {0x111, 0xB1, 0xBA, 0x12},  {0x111, 0xCA, 0xBA, 0x12},
        {0x111, 0xDD, 0xBA, 0x12},  {0x111, 0xF0, 0xBA, 0x12},
        {0x111, 0x103, 0xBA, 0x12}, {0x111, 0x116, 0xBA, 0x12},
        {0x111, 0x129, 0xBA, 0x12}, {0x13E, 0x12, 0x89, 0x12},
    }};

//-------------------------------------------------------------------------
// Data declarations

unsigned int game_viewport_width;
unsigned int game_viewport_height;
int game_viewport_x;
int game_viewport_y;
int game_viewport_z;
int game_viewport_w;

std::array<unsigned int, 2> saveload_dlg_xs = {{82, 0}};
std::array<unsigned int, 2> saveload_dlg_ys = {{60, 0}};
std::array<unsigned int, 2> saveload_dlg_zs = {{460, 640}};
std::array<unsigned int, 2> saveload_dlg_ws = {{344, 480}};
int pWindowList_at_506F50_minus1_indexing[1];
int dword_4C9890[10];
int dword_4C9920[16];
char _4D864C_force_sw_render_rules;
float flt_4D86CC = 1.0f;  // spot light scale??
int dword_4D86D8 = 0x40000000;

float flt_4D84E8 = 0.0f;

unsigned int uGammaPos = 0;
std::array<int, 8> BtnTurnCoord = {{
    0xA4,
    0x5D,
    0x16,
    0xB,
    0x5,
    0xD,
    0x7,
    0x3B,
}};
std::array<__int16, 4> RightClickPortraitXmin = {{0x14, 0x83, 0xF2, 0x165}};
std::array<__int16, 4> RightClickPortraitXmax = {{0x53, 0xC6, 0x138, 0x1A7}};

std::array<unsigned int, 4> pHealthBarPos = {{22, 137, 251, 366}};
std::array<unsigned int, 4> pManaBarPos = {{102, 217, 331, 447}};

std::array<char, 88> monster_popup_y_offsets = {
    {-20, 20, 0,   -40, 0,   0,   0,   0,   0,   0,   -50, 20,  0,   -10, -10,
     -20, 10, -10, 0,   0,   0,   -20, 10,  -10, 0,   0,   0,   -20, -10, 0,
     0,   0,  -40, -20, 0,   0,   0,   -50, -30, -30, -30, -30, -30, -30, 0,
     0,   0,  0,   0,   0,   -20, -20, -20, 20,  20,  20,  10,  10,  10,  10,
     10,  10, -90, -60, -40, -20, -20, -80, -10, 0,   0,   -40, 0,   0,   0,
     -20, 10, 0,   0,   0,   0,   0,   0,   -60, 0,   0,   0,   0}};
unsigned char hourglass_icon_idx = 12;

const char *format_4E2D80 = "\f%05d%s\f00000\n";
// const char *format_4E2DC8 = "\f%05d";
const char *format_4E2DE8 = "\f%05d%s\f00000 - ";
const char *Stat_string_format_2_column_text = "%s\f%05u\r180%s\n";
const char *Stat_string_format_2_column_less_100 = "%s\f%05u\t110%d\f00000 / %d\n";
const char *Stat_string_format_2_column_over_100 = "%s\f%05u\r180%d\f00000 / %d\n";
int dword_4E455C;
std::array<int, 6> dword_4E4560;
std::array<int, 6> dword_4E4578;
std::array<int, 6> dword_4E4590;
std::array<int, 6> dword_4E45A8;
std::array<float, 10> flt_4E4A80 = {{
    0.050000001f, 0.1f,  0.30000001f, 0.5f,
    0.60000002f,  1.0f,  6.0f,        25.0f,
    50.0f,        100.0f
}};

std::array<std::array<int, 2>, 14> pPartySpellbuffsUI_XYs = {{
    {{477, 247}},
    {{497, 247}},
    {{522, 247}},
    {{542, 247}},
    {{564, 247}},
    {{581, 247}},
    {{614, 247}},
    {{477, 279}},
    {{497, 279}},
    {{522, 279}},
    {{542, 279}},
    {{564, 279}},
    {{589, 279}},
    {{612, 279}}
}};
std::array<unsigned char, 14> byte_4E5DD8 = {
    {PARTY_BUFF_FEATHER_FALL, PARTY_BUFF_RESIST_FIRE, PARTY_BUFF_RESIST_AIR,
     PARTY_BUFF_RESIST_WATER, PARTY_BUFF_RESIST_MIND, PARTY_BUFF_RESIST_EARTH,
     PARTY_BUFF_RESIST_BODY, PARTY_BUFF_HEROISM, PARTY_BUFF_HASTE,
     PARTY_BUFF_SHIELD, PARTY_BUFF_STONE_SKIN, PARTY_BUFF_PROTECTION_FROM_MAGIC,
     PARTY_BUFF_IMMOLATION, PARTY_BUFF_DAY_OF_GODS}};
std::array<unsigned __int8, 14> pPartySpellbuffsUI_smthns = {
    {14, 1, 10, 4, 7, 2, 9, 3, 6, 15, 8, 3, 12, 0}};

std::array<std::array<int, 6>, 6> pNPCPortraits_x = {{
    {{521, 0, 0, 0, 0, 0}},
    {{521, 521, 0, 0, 0, 0}},
    {{521, 521, 521, 0, 0, 0}},
    {{521, 486, 564, 521, 0, 0}},
    {{521, 486, 564, 486, 564, 0}},
    {{486, 564, 486, 564, 486, 564}}
}};
std::array<std::array<int, 6>, 6> pNPCPortraits_y = {{
    {{38, 0, 0, 0, 0, 0}},
    {{38, 165, 0, 0, 0, 0}},
    {{38, 133, 228, 0, 0, 0}},
    {{38, 133, 133, 228, 0, 0}},
    {{38, 133, 133, 228, 228, 0}},
    {{38, 38, 133, 133, 228, 228}}
}};
std::array<const char *, 11> pHouse_ExitPictures = {{
    "",         "ticon01", "ticon02", "ticon03", "ticon04", "ticon05",
    "istairup", "itrap",   "outside", "idoor",   "isecdoor"
}};

std::array<int16_t, 11> word_4E8152 = {{0, 0, 0, 90, 8, 2, 70, 20, 10, 50, 30}};  // level spawn monster levels ABC

char _4E94D0_light_type = 5;
char _4E94D2_light_type = 6;
char _4E94D3_light_type = 10;
// char *off_4EB080; // idb
std::array<char *, 465> pTransitionStrings = {{"", nullptr}};
std::array<const char *, 25> pPlayerPortraitsNames = {{
    "pc01-", "pc02",  "pc03",  "pc04",  "pc05-", "pc06", "pc07",
    "pc08",  "pc09-", "pc10",  "pc11-", "pc12",  "pc13", "pc14",
    "pc15",  "pc16",  "pc17-", "pc18",  "pc19",  "pc20", "pc21-",
    "pc22-", "pc23",  "pc24-", "pc25-"
}};

std::array<std::array<unsigned char, 25>, 48> byte_4ECF08 = {{
    {{2, 1, 2, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1}},  // 1
    {{2, 1, 2, 1, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2}},  // 2
    {{2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}},  // 3
    {{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}},  // 4
    {{2, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}},  // 5
    {{2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}},  // 6
    {{2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}},  // 7
    {{2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1}},  // 8
    {{2, 1, 2, 2, 2, 2, 1, 2, 1, 2, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 2, 2, 2, 2}},  // 9
    {{2, 1, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}},  // 10
    {{2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2}},  // 11
    {{1, 1, 2, 1, 2, 1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 1, 1, 2, 1, 2, 1, 2, 2}},  // 12
    {{2, 1, 2, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}},  // 13
    {{1, 1, 2, 1, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 1, 2, 2, 1, 2, 1, 2, 1, 2, 2}},  // 14
    {{2, 1, 1, 2, 1, 1, 2, 1, 2, 2, 2, 1, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2}},  // 15
    {{1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1}},  // 16
    {{1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},  // 17
    {{1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 2, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 1}},  // 18
    {{1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},  // 19
    {{1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},  // 20
    {{1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},  // 21
    {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},  // 22
    {{2, 2, 2, 1, 2, 2, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1}},  // 23
    {{2, 2, 2, 1, 2, 2, 2, 2, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2}},  // 24
    {{1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 1, 1, 1, 1, 2, 2, 2, 1, 2, 1, 1}},  // 25
    {{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}},  // 26
    {{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 1, 2, 1, 2, 2, 2, 2, 2, 2, 2}},  // 27
    {{1, 2, 2, 1, 2, 2, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},  // 28
    {{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 2, 2, 2, 2, 2, 1, 2, 2, 1, 2, 1, 1}},  // 29
    {{1, 2, 2, 1, 2, 1, 2, 2, 1, 2, 2, 1, 1, 1, 2, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1}},  // 30
    {{1, 1, 2, 1, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 1}},  // 31
    {{1, 1, 2, 1, 1, 2, 2, 2, 2, 1, 2, 2, 1, 1, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2}},  // 32
    {{1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1}},  // 33
    {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 2, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1}},  // 34
    {{1, 1, 1, 2, 1, 2, 2, 1, 1, 2, 2, 1, 2, 2, 1, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2}},  // 35
    {{1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2}},  // 36
    {{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2}},  // 37
    {{2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 1, 2, 1, 1}},  // 38
    {{1, 1, 1, 1, 2, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1}},  // 39
    {{1, 2, 2, 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},  // 40
    {{2, 1, 2, 1, 2, 1, 2, 2, 2, 1, 1, 2, 1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1}},  // 41
    {{1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 1, 2, 1, 1}},  // 42
    {{1, 2, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}},  // 43
    {{1, 2, 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 2, 1, 1, 2, 1, 2, 2}},  // 44
    {{2, 1, 1, 1, 2, 2, 1, 1, 2, 1, 2, 1, 2, 1, 2, 2, 1, 2, 2, 2, 1, 2, 1, 2, 2}},  // 45
    {{1, 1, 1, 2, 2, 2, 1, 1, 2, 1, 2, 1, 2, 2, 2, 2, 2, 1, 1, 2, 1, 2, 1, 2, 2}},  // 46
    {{1, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 2, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1}},  // 47
    {{1, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 2, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1}}   // 48
}};
std::array<std::array<unsigned char, 8>, 110> SoundSetAction = {{
    {{0, 0, 0, 0, 0, 0, 0, 0}},     {{38, 0, 0, 37, 38, 45, 0, 0}},
    {{37, 0, 0, 47, 53, 0, 0, 0}},  {{2, 0, 0, 25, 0, 0, 0, 0}},
    {{1, 0, 0, 47, 0, 0, 0, 0}},    {{3, 0, 0, 41, 42, 0, 0, 0}},
    {{0, 0, 0, 33, 0, 0, 0, 0}},    {{5, 0, 0, 21, 22, 23, 24, 45}},
    {{6, 0, 0, 47, 0, 0, 0, 0}},    {{7, 0, 0, 43, 48, 0, 0, 0}},
    {{8, 0, 0, 38, 0, 0, 0, 0}},    {{9, 0, 0, 39, 0, 0, 0, 0}},
    {{13, 0, 0, 37, 38, 0, 0, 0}},  {{23, 0, 0, 25, 0, 0, 0, 0}},
    {{0, 0, 0, 37, 38, 45, 0, 0}},  {{25, 0, 0, 25, 0, 0, 0, 0}},
    {{26, 0, 0, 38, 47, 0, 0, 0}},  {{27, 0, 0, 41, 42, 0, 0, 0}},
    {{28, 0, 0, 25, 0, 0, 0, 0}},   {{0, 0, 0, 51, 0, 0, 0, 0}},
    {{30, 0, 0, 25, 0, 0, 0, 0}},   {{29, 0, 0, 27, 0, 0, 0, 0}},
    {{32, 33, 0, 37, 38, 0, 0, 0}}, {{34, 0, 0, 53, 0, 0, 0, 0}},
    {{15, 0, 0, 34, 35, 36, 0, 0}}, {{0, 0, 0, 3, 0, 0, 0, 0}},
    {{22, 0, 0, 5, 0, 0, 0, 0}},    {{20, 0, 0, 8, 0, 0, 0, 0}},
    {{20, 0, 0, 9, 0, 0, 0, 0}},    {{19, 0, 0, 7, 0, 0, 0, 0}},
    {{21, 0, 0, 2, 0, 0, 0, 0}},    {{18, 0, 0, 6, 0, 0, 0, 0}},
    {{0, 0, 0, 11, 0, 0, 0, 0}},    {{17, 0, 0, 0, 0, 0, 0, 0}},
    {{0, 0, 0, 12, 0, 0, 0, 0}},    {{17, 0, 0, 0, 0, 0, 0, 0}},
    {{0, 0, 0, 37, 0, 0, 0, 0}},    {{0, 0, 0, 49, 0, 0, 0, 0}},
    {{24, 0, 0, 25, 0, 0, 0, 0}},   {{31, 0, 0, 25, 0, 0, 0, 0}},
    {{0, 0, 0, 41, 42, 0, 0, 0}},   {{0, 0, 0, 39, 48, 0, 0, 0}},
    {{0, 0, 0, 46, 0, 0, 0, 0}},    {{0, 0, 0, 39, 48, 0, 0, 0}},
    {{0, 0, 0, 41, 42, 0, 0, 0}},   {{0, 0, 0, 39, 48, 0, 0, 0}},
    {{41, 0, 0, 21, 0, 0, 0, 0}},   {{40, 0, 0, 21, 0, 0, 0, 0}},
    {{16, 0, 0, 46, 0, 0, 0, 0}},   {{0, 0, 0, 40, 0, 0, 0, 0}},
    {{0, 0, 0, 39, 0, 0, 0, 0}},    {{0, 0, 0, 37, 38, 0, 0, 0}},
    {{0, 0, 0, 48, 49, 50, 0, 0}},  {{0, 0, 0, 44, 0, 0, 0, 0}},
    {{0, 0, 0, 48, 0, 0, 0, 0}},    {{0, 0, 0, 37, 38, 45, 0, 0}},
    {{0, 0, 0, 41, 42, 0, 0, 0}},   {{0, 0, 0, 47, 0, 0, 0, 0}},
    {{0, 0, 0, 39, 48, 0, 0, 0}},   {{0, 0, 0, 39, 48, 0, 0, 0}},
    {{0, 0, 0, 37, 38, 0, 0, 0}},   {{0, 0, 0, 37, 38, 0, 0, 0}},
    {{0, 0, 0, 37, 38, 0, 0, 0}},   {{0, 0, 0, 31, 0, 0, 0, 0}},
    {{0, 0, 0, 32, 0, 0, 0, 0}},    {{47, 0, 0, 46, 0, 0, 0, 0}},
    {{22, 0, 0, 5, 0, 0, 0, 0}},    {{0, 0, 0, 25, 0, 0, 0, 0}},
    {{0, 0, 0, 37, 0, 0, 0, 0}},    {{0, 0, 0, 52, 0, 0, 0, 0}},
    {{0, 0, 0, 45, 0, 0, 0, 0}},    {{0, 0, 0, 27, 0, 0, 0, 0}},
    {{0, 0, 0, 37, 38, 0, 0, 0}},   {{0, 0, 0, 49, 0, 0, 0, 0}},
    {{0, 0, 0, 48, 0, 0, 0, 0}},    {{0, 0, 0, 37, 38, 45, 0, 0}},
    {{0, 0, 0, 48, 0, 0, 0, 0}},    {{0, 0, 0, 37, 38, 45, 0, 0}},
    {{0, 0, 0, 47, 0, 0, 0, 0}},    {{0, 0, 0, 48, 0, 0, 0, 0}},
    {{44, 0, 0, 39, 48, 0, 0, 0}},  {{0, 0, 0, 48, 49, 0, 0, 0}},
    {{0, 0, 0, 38, 0, 0, 0, 0}},    {{0, 0, 0, 31, 0, 0, 0, 0}},
    {{32, 33, 0, 37, 38, 0, 0, 0}}, {{0, 0, 0, 37, 38, 0, 0, 0}},
    {{0, 0, 0, 37, 38, 0, 0, 0}},   {{0, 0, 0, 47, 0, 0, 0, 0}},
    {{0, 0, 0, 38, 0, 0, 0, 0}},    {{0, 0, 0, 48, 0, 0, 0, 0}},
    {{0, 0, 0, 0, 0, 0, 0, 0}},     {{0, 0, 0, 38, 0, 0, 0, 0}},
    {{0, 0, 0, 47, 0, 0, 0, 0}},    {{0, 0, 0, 38, 0, 0, 0, 0}},
    {{0, 0, 0, 0, 0, 0, 0, 0}},     {{0, 0, 0, 0, 0, 0, 0, 0}},
    {{0, 0, 0, 38, 0, 0, 0, 0}},    {{0, 0, 0, 38, 0, 0, 0, 0}},
    {{0, 0, 0, 5, 0, 0, 0, 0}},     {{0, 0, 0, 37, 38, 0, 0, 0}},
    {{0, 0, 0, 44, 0, 0, 0, 0}},    {{0, 0, 0, 48, 0, 0, 0, 0}},
    {{4, 0, 0, 0, 0, 0, 0, 0}},     {{0, 0, 0, 58, 0, 0, 0, 0}},
    {{10, 0, 0, 45, 0, 0, 0, 0}},   {{11, 0, 0, 47, 0, 0, 0, 0}},
    {{12, 0, 0, 43, 48, 0, 0, 0}},  {{39, 0, 0, 48, 0, 0, 0, 0}},
    {{14, 0, 0, 48, 0, 0, 0, 0}},   {{17, 0, 0, 0, 0, 0, 0, 0}}
}};
std::array<int16_t, 4> pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing = {{34, 149, 264, 379}};
std::array<std::array<char, 37>, 36> byte_4ED970_skill_learn_ability_by_class_table = {{
    {{2, 3, 2, 3, 3, 2, 3, 4, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 3, 0, 2, 1, 0, 1, 2, 2, 0, 3, 0, 0, 1}},  // Knight
    {{2, 3, 2, 3, 3, 2, 3, 4, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 3, 0, 2, 1, 0, 1, 2, 2, 0, 3, 0, 0, 1}},
    {{2, 4, 2, 3, 4, 2, 3, 4, 4, 3, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 4, 4, 0, 2, 1, 0, 1, 2, 2, 0, 4, 0, 0, 1}},
    {{2, 4, 2, 3, 4, 2, 3, 4, 4, 3, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 4, 4, 0, 2, 1, 0, 1, 2, 2, 0, 4, 0, 0, 1}},

    {{0, 3, 3, 0, 0, 2, 2, 4, 1, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 1, 2, 0, 3, 0, 0, 3, 3, 2, 0, 3, 3, 2, 2}},  // Thief
    {{0, 3, 3, 0, 0, 2, 2, 4, 1, 3, 2, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 3, 3, 1, 2, 0, 3, 0, 0, 3, 3, 2, 0, 3, 3, 2, 2}},
    {{0, 3, 4, 0, 0, 2, 2, 4, 1, 4, 2, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 3, 3, 1, 2, 0, 3, 0, 0, 4, 3, 2, 0, 3, 4, 2, 2}},
    {{0, 3, 4, 0, 0, 2, 2, 4, 1, 4, 2, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 3, 3, 1, 2, 0, 3, 0, 0, 4, 3, 2, 0, 3, 4, 2, 2}},

    {{3, 2, 2, 0, 2, 1, 0, 4, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 2, 1, 0, 1, 3, 3, 2, 3, 1, 0, 3}},  // Monk
    {{3, 2, 2, 0, 2, 1, 0, 4, 0, 3, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 3, 0, 2, 1, 0, 2, 3, 3, 2, 3, 1, 0, 3}},
    {{4, 2, 2, 0, 2, 1, 0, 4, 0, 3, 0, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0, 4, 0, 2, 1, 0, 2, 4, 4, 2, 3, 1, 0, 4}},
    {{4, 2, 2, 0, 2, 1, 0, 4, 0, 3, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 4, 0, 2, 1, 0, 3, 4, 4, 2, 3, 2, 0, 4}},

    {{1, 3, 2, 2, 2, 2, 3, 4, 3, 2, 2, 3, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 2, 3, 3, 2, 1, 3, 0, 0, 1, 1, 0, 2, 0, 0, 1}},  // Paladin
    {{1, 3, 2, 2, 2, 2, 3, 4, 3, 2, 2, 3, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 2, 3, 3, 2, 1, 3, 0, 0, 1, 1, 0, 2, 0, 0, 1}},
    {{1, 3, 2, 2, 2, 2, 4, 4, 4, 2, 2, 3, 0, 0, 0, 0, 3, 3, 3, 1, 0, 0, 2, 4, 3, 2, 1, 4, 0, 0, 1, 1, 0, 2, 0, 0, 1}},
    {{1, 3, 2, 2, 2, 2, 4, 4, 4, 2, 2, 3, 0, 0, 0, 0, 3, 3, 3, 0, 1, 0, 2, 4, 3, 2, 1, 4, 0, 0, 1, 1, 0, 2, 0, 0, 1}},

    {{1, 2, 2, 2, 3, 3, 0, 4, 0, 3, 3, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 0, 0, 1, 2, 1, 0, 2, 0, 0, 3}},  // Archer
    {{1, 2, 2, 2, 3, 3, 0, 4, 0, 3, 3, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 0, 0, 1, 2, 1, 0, 2, 0, 0, 3}},
    {{1, 2, 2, 2, 3, 4, 0, 4, 0, 3, 4, 0, 3, 3, 3, 3, 0, 0, 0, 1, 0, 0, 2, 2, 2, 2, 4, 0, 0, 2, 2, 1, 0, 2, 0, 0, 3}},
    {{1, 2, 2, 2, 3, 4, 0, 4, 0, 3, 4, 0, 3, 3, 3, 3, 0, 0, 0, 0, 1, 0, 2, 2, 2, 2, 4, 0, 0, 2, 2, 1, 0, 2, 0, 0, 3}},

    {{1, 2, 2, 3, 2, 3, 0, 4, 2, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 1, 3, 2, 0, 2, 2, 1, 3, 2, 2, 1, 2}},  // Ranger
    {{1, 2, 2, 3, 2, 3, 0, 4, 2, 3, 3, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 2, 1, 3, 2, 0, 2, 2, 1, 3, 2, 2, 1, 2}},
    {{1, 2, 2, 4, 2, 3, 0, 4, 2, 3, 3, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 1, 1, 1, 2, 1, 3, 2, 0, 2, 2, 1, 4, 2, 2, 1, 2}},
    {{1, 2, 2, 4, 2, 3, 0, 4, 2, 3, 3, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 1, 1, 1, 2, 1, 3, 2, 0, 2, 2, 1, 4, 2, 2, 1, 2}},

    {{1, 0, 0, 0, 0, 2, 3, 4, 3, 2, 2, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 3, 3, 1, 3, 2, 3, 0, 0, 0, 0, 2, 0, 0, 2, 3}},  // Cleric
    {{1, 0, 0, 0, 0, 2, 3, 4, 3, 2, 2, 0, 0, 0, 0, 0, 3, 3, 3, 0, 0, 0, 3, 3, 1, 3, 2, 3, 0, 0, 0, 0, 2, 0, 0, 2, 3}},
    {{1, 0, 0, 0, 0, 2, 3, 4, 3, 2, 2, 0, 0, 0, 0, 0, 4, 4, 4, 4, 0, 0, 4, 3, 1, 3, 2, 3, 0, 0, 0, 0, 2, 0, 0, 2, 3}},
    {{1, 0, 0, 0, 0, 2, 3, 4, 3, 2, 2, 0, 0, 0, 0, 0, 4, 4, 4, 0, 4, 0, 4, 3, 1, 3, 2, 3, 0, 0, 0, 0, 2, 0, 0, 2, 3}},

    {{1, 0, 3, 0, 0, 1, 2, 4, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 2, 0, 0, 2, 2, 0, 0, 3, 2, 0, 0, 0, 0, 0, 2, 1, 0, 3, 3}},  // Druid
    {{1, 0, 3, 0, 0, 1, 2, 4, 2, 2, 0, 0, 3, 3, 3, 3, 3, 3, 3, 0, 0, 2, 2, 0, 0, 3, 2, 0, 0, 0, 0, 0, 2, 1, 0, 3, 3}},
    {{1, 0, 3, 0, 0, 1, 2, 4, 2, 2, 0, 0, 3, 3, 3, 3, 3, 3, 3, 0, 0, 2, 2, 0, 0, 4, 2, 0, 0, 0, 0, 0, 2, 1, 0, 4, 3}},
    {{1, 0, 3, 0, 0, 1, 2, 4, 2, 2, 0, 0, 3, 3, 3, 3, 3, 3, 3, 0, 0, 2, 2, 0, 0, 4, 2, 0, 0, 0, 0, 0, 2, 1, 0, 4, 3}},

    {{3, 0, 2, 0, 0, 1, 0, 4, 0, 2, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 3, 1, 2, 0, 3, 2, 1, 0, 0, 0, 0, 3, 0, 0, 3, 3}},  // Sorcerer
    {{3, 0, 2, 0, 0, 1, 0, 4, 0, 2, 0, 0, 3, 3, 3, 3, 0, 0, 0, 0, 0, 3, 1, 2, 0, 3, 2, 1, 0, 0, 0, 0, 3, 0, 0, 3, 3}},
    {{3, 0, 2, 0, 0, 1, 0, 4, 0, 2, 0, 0, 4, 4, 4, 4, 0, 0, 0, 4, 0, 4, 1, 2, 0, 3, 2, 1, 0, 0, 0, 0, 4, 0, 0, 3, 3}},
    {{3, 0, 2, 0, 0, 1, 0, 4, 0, 2, 0, 0, 4, 4, 4, 4, 0, 0, 0, 0, 4, 4, 1, 2, 0, 3, 2, 1, 0, 0, 0, 0, 4, 0, 0, 3, 3}}
}};
std::array<unsigned int, 2> pHiredNPCsIconsOffsetsX = {{489, 559}};
std::array<unsigned int, 2> pHiredNPCsIconsOffsetsY = {{152, 152}};
std::array<int, 2> Party_Spec_Motion_status_ids = {{7, 18}};  // dword_4EE07C
std::array<short, 28> word_4EE150 = {{1,  2,  3,  4,  5,  7,  32, 33, 36, 37,
                                      38, 40, 41, 42, 43, 45, 46, 47, 48, 49,
                                      50, 51, 52, 53, 54, 55, 56, 60}};

int ScreenshotFileNumber;  // dword_4EFA80

std::array<int, 32> guild_mambership_flags = {
    {54, 54, 54, 54, 52, 52, 52, 52, 55, 55, 55, 55, 53, 53, 53, 53,
     58, 58, 58, 58, 57, 57, 57, 57, 56, 56, 56, 56, 59, 59, 60, 60}};
std::array<int16_t, 49> word_4F0754;
std::array<int16_t, 54> _4F0882_evt_VAR_PlayerItemInHands_vals = {{
    0x0D4, 0x270, 0x0D5, 0x21C, 0x0D6, 0x2BE, 0x0D7, 0x2BD, 0x0D8,
    0x289, 0x0D9, 0x258, 0x0DA, 0x2AB, 0x0DB, 0x281, 0x0DC, 0x280,
    0x0DD, 0x284, 0x0DE, 0x285, 0x0DF, 0x283, 0x0E0, 0x282, 0x0E1,
    0x27F, 0x0E2, 0x1E7, 0x0E3, 0x287, 0x0E4, 0x272, 0x0E5, 0x267,
    0x0E6, 0x275, 0x0E7, 0x25A, 0x0E8, 0x2A4, 0x0E9, 0x2A5, 0x0EA,
    0x2A3, 0x0EB, 0x25C, 0x0EC, 0x25D, 0x0ED, 0x259, 0x0F1, 0x21E
}};
std::array<unsigned short, 6> pMaxLevelPerTrainingHallType = {{5, 15, 25, 25, 200, 200}};

std::array<int, 11> price_for_membership = {{100, 100, 50, 50, 50, 50, 50, 50, 50, 1000, 1000}};

std::array<int16_t, 32> word_4F0F30 = {{
    4, 7, 10, 11, 4, 7, 10, 11, 4, 7, 10, 11, 4, 7,  10, 11,
    4, 7, 10, 11, 4, 7, 10, 11, 4, 7, 10, 11, 7, 11, 7,  11
}};
std::array<int, 500> ai_array_4F5E68;
std::array<int, 500> ai_array_4F6638_actor_ids;
std::array<int, 500> ai_near_actors_targets_pid;
int ai_arrays_size;
std::array<int, 500> ai_near_actors_distances;
std::array<unsigned int, 500> ai_near_actors_ids;
std::array<int, 182> dword_4F8580 = {};

char byte_4FAA24;  // turn over break??
// HWND dword_4FAA28; // idb

std::array<unsigned int, 480> pSRZBufferLineOffsets;
std::array<char, 777> books_num_items_per_page;
int lloyds_beacon_spell_id;  // dword_506338
int lloyds_beacon_sound_id;  // dword_50633C
signed int sRecoveryTime;    // idb
unsigned int uRequiredMana;  // idb
int _506348_current_lloyd_playerid;
__int64 lloyds_beacon_spell_level;  // qword_506350 604800 *sepell level
int MapBookOpen;
int books_page_number;
int books_primary_item_per_page;
int Autonotes_Instructors_page_flag;  // dword_50652C
int Autonotes_Misc_page_flag;         // dword_506530
int Book_PageBtn6_flag;               // dword_506534
int Book_PageBtn5_flag;               // dword_506538
int Book_PageBtn4_flag;               // dword_50653C
int Book_PageBtn3_flag;               // dword_506540
int BtnDown_flag;                     // BtnDown_flag
int BtnUp_flag;                       // BtnUp_flag
int quick_spell_at_page;
char byte_506550;
int _506568_autonote_type;
bool bRecallingBeacon;
int uLastPointedObjectID;
// unsigned __int8 bMonsterInfoUI_bDollInitialized;
int dword_506980_uW;
int dword_506984_uZ;
int dword_506988_uY;
int dword_50698C_uX;
int KeyboardPageNum;
int uRestUI_FoodRequiredToRest;
int _506F14_resting_stage;
int _506F18_num_minutes_to_sleep;
int dword_506F1C;
char bFlashHistoryBook;
char bFlashAutonotesBook;
char bFlashQuestBook;
GUIButton *pBtn_ZoomOut;  // idb
GUIButton *pBtn_ZoomIn;  // idb
unsigned int uGameUIFontShadow;
unsigned int uGameUIFontMain;
int dword_507B00_spell_info_to_draw_in_popup;
unsigned int uActiveCharacter;
int dword_507BF0_is_there_popup_onscreen;
int awards_scroll_bar_created;
int dword_507CC0_activ_ch;
GameTime GameUI_RightPanel_BookFlashTimer;
int _507CD4_RestUI_hourglass_anim_controller;
bool OpenedTelekinesis;
std::array<int, 50> dword_50B570;
std::array<int, 50> dword_50B638;
stru367 PortalFace;
std::array<int, 100> dword_50BC10;
std::array<int, 100> dword_50BDA0;
std::array<int, 100> _50BF30_actors_in_viewport_ids;
char town_portal_caster_id;
int some_active_character;
std::array<unsigned int, 5> pIconIDs_Turn;
unsigned int uIconID_TurnStop;
unsigned int uIconID_TurnHour;
int uIconID_CharacterFrame;  // idb
unsigned int uIconID_TurnStart;
int dword_50C994 = 0;
int dword_50C998_turnbased_icon_1A = 0;
int uSpriteID_Spell11;  // idb
bool _50C9A0_IsEnchantingInProgress;
int _50C9A8_item_enchantment_timer = 0;
int _50C9D0_AfterEnchClickEventId;
int _50C9D4_AfterEnchClickEventSecondParam;
int _50C9D8_AfterEnchClickEventTimeout;
int dword_50C9DC;
struct NPCData *ptr_50C9E0;
// int dword_50C9E8; // idb
// int dword_50C9EC[120];
int dword_50CDC8;
Vec3_int_ layingitem_vel_50FDFC;
char pStartingMapName[32];  // idb
std::array<unsigned __int8, 5> IsPlayerWearingWatersuit;
std::array<char, 54> party_has_equipment;
std::array<char, 17> byte_5111F6_OwnedArtifacts;

unsigned int uNumBlueFacesInBLVMinimap;  // could remove
std::array<unsigned __int16, 50> pBlueFacesInBLVMinimapIDs;
std::array<class Image *, 14> party_buff_icons;
unsigned int uIconIdx_FlySpell;
unsigned int uIconIdx_WaterWalk;
GameTime _5773B8_event_timer;

struct Actor *pDialogue_SpeakingActor;
unsigned int uDialogueType;
int sDialogue_SpeakingActorNPC_ID;
struct LevelDecoration *_591094_decoration;
int uCurrentHouse_Animation;
char *Party_Teleport_Map_Name;
// int Party_Teleport_Z_Speed;
// int Party_Teleport_Cam_Pitch;
// int Party_Teleport_Cam_Yaw;
// int Party_Teleport_Z_Pos;
// int Party_Teleport_Y_Pos;
// int Party_Teleport_X_Pos;
std::array<std::array<char, 100>, 6> byte_591180;  // idb
std::array<struct NPCData *, 7> HouseNPCData;  // 0 zero element holds standart house npc
GUIButton *HouseNPCPortraitsButtonsList[6];  // dword_5913F4
String branchless_dialogue_str;
int EvtTargetObj;                               // 0x5B5920
int _unused_5B5924_is_travel_ui_drawn = false;  // 005B5924
int Party_Teleport_X_Pos;
int Party_Teleport_Y_Pos;
int Party_Teleport_Z_Pos;
int Party_Teleport_Cam_Yaw;
int Party_Teleport_Cam_Pitch;
int Party_Teleport_Z_Speed;
int Start_Party_Teleport_Flag;
int dword_5B65C4_cancelEventProcessing;
int MapsLongTimers_count;  // dword_5B65C8 счётчик таймеров для колодцев,
                           // фаерволов-ловушек
int npcIdToDismissAfterDialogue;
signed int dword_5B65D0_dialogue_actor_npc_id;
int dword_5C3418;
int dword_5C341C;
// std::array<char, 777> byte_5C3427;
String game_ui_status_bar_event_string;
String game_ui_status_bar_string;
unsigned int game_ui_status_bar_event_string_time_left;
int bForceDrawFooter;
int _5C35C0_force_party_death = false;
int bDialogueUI_InitializeActor_NPC_ID;
char *p2DEventsTXT_Raw;

int dword_5C35D4;
char byte_5C45AF[777];

std::array<char, 100> Journal_limitation_factor;
int ui_current_text_color;
int64_t qword_5C6DF0;
int dword_5C6DF8;
char *pHealthPointsAttributeDescription;
char *pSpellPointsAttributeDescription;
char *pArmourClassAttributeDescription;
char *pPlayerConditionAttributeDescription;  // idb
char *pFastSpellAttributeDescription;
char *pPlayerAgeAttributeDescription;
char *pPlayerLevelAttributeDescription;
char *pPlayerExperienceAttributeDescription;
char *pAttackBonusAttributeDescription;
char *pAttackDamageAttributeDescription;
char *pMissleBonusAttributeDescription;
char *pMissleDamageAttributeDescription;
char *pFireResistanceAttributeDescription;
char *pAirResistanceAttributeDescription;
char *pWaterResistanceAttributeDescription;
char *pEarthResistanceAttributeDescription;
char *pMindResistanceAttributeDescription;
char *pBodyResistanceAttributeDescription;
char *pSkillPointsAttributeDescription;

struct FactionTable *pFactionTable;
// std::array<char, 777> byte_5C8D1A;

std::array<char, 777> byte_5E4C15;

char *pMonstersTXT_Raw;
char *pMonsterPlacementTXT_Raw;
char *pSpellsTXT_Raw;
char *pMapStatsTXT_Raw;
char *pHostileTXT_Raw;
char *pPotionsTXT_Raw;
char *pPotionNotesTXT_Raw;

std::array<int, 777> _6807B8_level_decorations_ids;  // idb -level decoratiosn sound
int _6807E0_num_decorations_with_sounds_6807B8;
std::array<int, 3000> event_triggers;  // 6807E8
size_t num_event_triggers;             // 6836C8
std::array<int, 64> dword_69B010;
float flt_69B138_dist;
char byte_69BD41_unused;
int pSaveListPosition;
unsigned int uLoadGameUI_SelectedSlot;
char cMM7GameCDDriveLetter;  // idb
unsigned int uGameState;
int uDefaultTravelTime_ByFoot;
int day_attrib;
int day_fogrange_1;
int day_fogrange_2;
struct TileTable *pTileTable;                                        // idb
std::array<char, 777> pDefaultSkyTexture;                            // idb
std::array<char, 16> byte_6BE124_cfg_textures_DefaultGroundTexture;  // idb
int _6BE134_odm_main_tile_group;
int dword_6BE138;  // are you sure check game menu - load slots
int dword_6BE13C_uCurrentlyLoadedLocationID;
float fWalkSpeedMultiplier = 1.0f;
float fBackwardWalkSpeedMultiplier = 1.0f;
float fTurnSpeedMultiplier = 1.0f;
float flt_6BE150_look_up_down_dangle = 1.0f;
String pCurrentMapName;
unsigned int uLevelMapStatsID;
int dword_6BE364_game_settings_1 = 0;
float _6BE3A0_fov = 0.55000001f;
float flt_6BE3A4_debug_recmod1;
float flt_6BE3A8_debug_recmod2;
float flt_6BE3AC_debug_recmod1_x_1_6;
std::string s_SavedMapName;  // idb
char bNoNPCHiring = false;
std::array<int, 100> dword_720020_zvalues;
std::array<int, 299> dword_7201B0_zvalues;
std::array<__int16, 104> word_7209A0_intercepts_ys_plus_ys;
std::array<__int16, 104> word_720A70_intercepts_xs_plus_xs;
std::array<__int16, 104> word_720B40_intercepts_zs;
std::array<__int16, 102> word_720C10_intercepts_xs;
std::array<__int16, 777> word_720CE0_ys;  // idb
std::array<__int16, 777> word_720DB0_xs;  // idb
std::array<int, 20> dword_720E80;
std::array<int, 20> dword_720ED0;
std::array<int, 20> ceiling_height_level;
std::array<__int16, 104> odm_floor_face_vert_coord_Y;  // word_720F70
std::array<__int16, 104> odm_floor_face_vert_coord_X;  // word_721040
std::array<int, 20> current_Face_id;                   // dword_721110
std::array<int, 20> current_BModel_id;                 // dword_721160
std::array<int, 20> odm_floor_level;                   // idb
int blv_prev_party_x;
int blv_prev_party_z;
int blv_prev_party_y;
std::array<NPCTopic, 789> pNPCTopics;
std::array<const char *, 513> pQuestTable;
char *dword_723718_autonote_related;  // idb
std::array<const char *, 82> pScrolls;
int dword_7241C8;
char *pAwardsTXT_Raw;
char *pScrollsTXT_Raw;
char *pMerchantsTXT_Raw;
std::array<const char *, 7> pMerchantsBuyPhrases;
std::array<const char *, 7> pMerchantsSellPhrases;
std::array<const char *, 7> pMerchantsRepairPhrases;
std::array<const char *, 7> pMerchantsIdentifyPhrases;
char *pTransitionsTXT_Raw;
char *pAutonoteTXT_Raw;
char *pQuestsTXT_Raw;
uint32_t uNumTerrainNormals;
struct Vec3_float_ *pTerrainNormals;
std::array<unsigned short, 128 * 128 * 2> pTerrainNormalIndices;
std::array<unsigned int, 128 * 128 * 2> pTerrainSomeOtherData;
int dword_A74C88;
int uPlayerCreationUI_ArrowAnim;
unsigned int uPlayerCreationUI_SelectedCharacter;
int dword_A74CDC;
int64_t _A750D8_player_speech_timer;  // qword_A750D8
int uSpeakingCharacter;
char byte_AE5B91;
std::array<int, 32> dword_F1B430;
// int dword_F8B144; // nexindex [-1] to the following
std::array<int, 4> player_levels = {{1, 1, 1, 1}};
std::array<int16_t, 6> weapons_Ypos;           // word_F8B158
int16_t bountyHunting_monster_id_for_hunting;  // word_F8B1A0
const char *bountyHunting_text;                // word_F8B1A4
int contract_approved;
int dword_F8B1AC_award_bit_number;  // idb
int dword_F8B1B0_MasteryBeingTaught;
int gold_transaction_amount;  // F8B1B4
std::array<const char *, 4> pShopOptions;
int dword_F8B1D8;
int dword_F8B1E0;
int dword_F8B1E4;
String current_npc_text;                        // F8B1E8
char dialogue_show_profession_details = false;  // F8B1EC
std::array<char, 777> byte_F8B1EF;
std::array<char, 4> byte_F8B1F0;

int bGameoverLoop = 0;
std::array<int16_t, 104> intersect_face_vertex_coords_list_a;  // word_F8BC48
std::array<int16_t, 104> intersect_face_vertex_coords_list_b;  // word_F8BD18
