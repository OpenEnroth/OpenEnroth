#include "GUI/UI/UICharacter.h"

#include <algorithm>
#include <vector>

#include "Engine/AssetsManager.h"
#include "Engine/Awards.h"
#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/MapInfo.h"
#include "Engine/Party.h"
#include "Engine/Time.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"

#include "Engine/Spells/CastSpellInfo.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIProgressBar.h"
#include "GUI/UI/UIInventory.h"

#include "Media/Audio/AudioPlayer.h"

#include "IO/Mouse.h"

using EngineIoc = Engine_::IocContainer;

static Mouse *pMouse = EngineIoc::ResolveMouse();

void CharacterUI_LoadPaperdollTextures();
void WetsuitOn(unsigned int uPlayerID);
void WetsuitOff(unsigned int uPlayerID);

int bRingsShownInCharScreen;  // 5118E0

unsigned int ui_mainmenu_copyright_color;

unsigned int ui_character_tooltip_header_default_color;
unsigned int ui_character_default_text_color;
unsigned int ui_character_skill_highlight_color;
unsigned int ui_character_header_text_color;
unsigned int ui_character_bonus_text_color;
unsigned int ui_character_bonus_text_color_neg;
unsigned int ui_character_skill_upgradeable_color;
unsigned int ui_character_skill_default_color;
unsigned int ui_character_stat_default_color;
unsigned int ui_character_stat_buffed_color;
unsigned int ui_character_stat_debuffed_color;
unsigned int ui_character_skillinfo_can_learn;
unsigned int ui_character_skillinfo_can_learn_gm;
unsigned int ui_character_skillinfo_cant_learn;
unsigned int ui_character_condition_normal_color;
unsigned int ui_character_condition_light_color;
unsigned int ui_character_condition_moderate_color;
unsigned int ui_character_condition_severe_color;
std::array<unsigned int, 6> ui_character_award_color;

unsigned int ui_game_minimap_outline_color;
unsigned int ui_game_minimap_actor_friendly_color;
unsigned int ui_game_minimap_actor_hostile_color;
unsigned int ui_game_minimap_actor_corpse_color;
unsigned int ui_game_minimap_decoration_color_1;
unsigned int ui_game_minimap_projectile_color;
unsigned int ui_game_minimap_treasure_color;
std::array<unsigned int, 24> ui_game_character_record_playerbuff_colors;

unsigned int ui_gamemenu_video_gamma_title_color;
unsigned int ui_gamemenu_keys_action_name_color;
unsigned int ui_gamemenu_keys_key_selection_blink_color_1;
unsigned int ui_gamemenu_keys_key_selection_blink_color_2;
unsigned int ui_gamemenu_keys_key_default_color;

unsigned int ui_book_quests_title_color;
unsigned int ui_book_quests_text_color;
unsigned int ui_book_autonotes_title_color;
unsigned int ui_book_autonotes_text_color;
unsigned int ui_book_map_title_color;
unsigned int ui_book_map_coordinates_color;

unsigned int ui_book_calendar_title_color;
unsigned int ui_book_calendar_time_color;
unsigned int ui_book_calendar_day_color;
unsigned int ui_book_calendar_month_color;
unsigned int ui_book_calendar_year_color;
unsigned int ui_book_calendar_moon_color;
unsigned int ui_book_calendar_location_color;

unsigned int ui_book_journal_title_color;
unsigned int ui_book_journal_text_color;
unsigned int ui_book_journal_text_shadow;

unsigned int ui_game_dialogue_npc_name_color;
unsigned int ui_game_dialogue_option_highlight_color;
unsigned int ui_game_dialogue_option_normal_color;

unsigned int ui_house_player_cant_interact_color;

void set_default_ui_skin() {
    ui_mainmenu_copyright_color = Color16(255, 255, 255);

    ui_character_tooltip_header_default_color = Color16(255, 255, 155);
    ui_character_default_text_color = Color16(255, 255, 255);
    ui_character_header_text_color = Color16(255, 255, 155);
    ui_character_bonus_text_color = Color16(0, 255, 0);
    ui_character_bonus_text_color_neg = Color16(255, 0, 0);

    ui_character_skill_upgradeable_color = Color16(0, 175, 255);
    ui_character_skill_default_color = Color16(255, 0, 0);
    ui_character_skill_highlight_color = Color16(255, 0, 0);

    ui_character_stat_default_color = Color16(255, 255, 255);
    ui_character_stat_buffed_color = Color16(0, 255, 0);
    ui_character_stat_debuffed_color = Color16(255, 35, 0);

    ui_character_skillinfo_can_learn = Color16(255, 255, 255);
    ui_character_skillinfo_can_learn_gm = Color16(255, 255, 0);
    ui_character_skillinfo_cant_learn = Color16(255, 0, 0);

    ui_character_condition_normal_color = Color16(255, 255, 255);
    ui_character_condition_light_color = Color16(0, 255, 0);
    ui_character_condition_moderate_color = Color16(225, 205, 35);
    ui_character_condition_severe_color = Color16(255, 35, 0);

    ui_character_award_color[0] = Color16(248, 108, 160);
    ui_character_award_color[1] = Color16(112, 220, 248);
    ui_character_award_color[2] = Color16(192, 192, 240);
    ui_character_award_color[3] = Color16(64, 244, 96);
    ui_character_award_color[4] = Color16(232, 244, 96);
    ui_character_award_color[5] = Color16(240, 252, 192);

    ui_game_minimap_outline_color = Color16(0, 0, 255);
    ui_game_minimap_actor_friendly_color = Color16(0, 255, 0);
    ui_game_minimap_actor_hostile_color = Color16(255, 0, 0);
    ui_game_minimap_actor_corpse_color = Color16(255, 255, 0);
    ui_game_minimap_decoration_color_1 = Color16(255, 255, 255);
    ui_game_minimap_projectile_color = Color16(255, 0, 0);
    ui_game_minimap_treasure_color = Color16(0, 0, 255);

    ui_game_character_record_playerbuff_colors[0] = Color16(150, 212, 255);
    ui_game_character_record_playerbuff_colors[1] = Color16(225, 225, 225);
    ui_game_character_record_playerbuff_colors[2] = Color16(255, 128, 0);
    ui_game_character_record_playerbuff_colors[3] = Color16(128, 128, 128);
    ui_game_character_record_playerbuff_colors[4] = Color16(225, 225, 225);
    ui_game_character_record_playerbuff_colors[5] = Color16(255, 85, 0);
    ui_game_character_record_playerbuff_colors[6] = Color16(255, 128, 0);
    ui_game_character_record_playerbuff_colors[7] = Color16(255, 85, 0);
    ui_game_character_record_playerbuff_colors[8] = Color16(225, 225, 225);
    ui_game_character_record_playerbuff_colors[9] = Color16(235, 15, 255);
    ui_game_character_record_playerbuff_colors[10] = Color16(192, 192, 240);
    ui_game_character_record_playerbuff_colors[11] = Color16(225, 225, 225);
    ui_game_character_record_playerbuff_colors[12] = Color16(255, 128, 0);
    ui_game_character_record_playerbuff_colors[13] = Color16(150, 212, 255);
    ui_game_character_record_playerbuff_colors[14] = Color16(128, 128, 128);
    ui_game_character_record_playerbuff_colors[15] = Color16(255, 255, 155);
    ui_game_character_record_playerbuff_colors[16] = Color16(255, 255, 155);
    ui_game_character_record_playerbuff_colors[17] = Color16(255, 255, 155);
    ui_game_character_record_playerbuff_colors[18] = Color16(255, 255, 155);
    ui_game_character_record_playerbuff_colors[19] = Color16(255, 255, 155);
    ui_game_character_record_playerbuff_colors[20] = Color16(255, 255, 155);
    ui_game_character_record_playerbuff_colors[21] = Color16(255, 255, 155);
    ui_game_character_record_playerbuff_colors[22] = Color16(0, 128, 255);
    ui_game_character_record_playerbuff_colors[23] = Color16(0, 128, 255);

    ui_gamemenu_video_gamma_title_color = Color16(255, 255, 155);
    ui_gamemenu_keys_action_name_color = Color16(255, 255, 255);
    ui_gamemenu_keys_key_selection_blink_color_1 = Color16(50, 0, 0);
    ui_gamemenu_keys_key_selection_blink_color_2 = Color16(225, 205, 35);
    ui_gamemenu_keys_key_default_color = Color16(255, 255, 255);

    ui_book_quests_title_color = Color16(255, 255, 255);
    ui_book_quests_text_color = Color16(255, 255, 255);
    ui_book_autonotes_title_color = Color16(255, 255, 255);
    ui_book_autonotes_text_color = Color16(255, 255, 255);
    ui_book_map_title_color = Color16(255, 255, 255);
    ui_book_map_coordinates_color = Color16(255, 255, 255);

    ui_book_calendar_title_color = Color16(255, 255, 255);
    ui_book_calendar_time_color = Color16(75, 75, 75);
    ui_book_calendar_day_color = Color16(75, 75, 75);
    ui_book_calendar_month_color = Color16(75, 75, 75);
    ui_book_calendar_year_color = Color16(75, 75, 75);
    ui_book_calendar_moon_color = Color16(75, 75, 75);
    ui_book_calendar_location_color = Color16(75, 75, 75);

    ui_book_journal_title_color = Color16(255, 255, 255);
    ui_book_journal_text_color = Color16(255, 255, 255);
    ui_book_journal_text_shadow = Color16(0, 0, 0);

    ui_game_dialogue_npc_name_color = Color16(21, 153, 233);
    ui_game_dialogue_option_highlight_color = Color16(225, 205, 35);
    ui_game_dialogue_option_normal_color = Color16(255, 255, 255);

    ui_house_player_cant_interact_color = Color16(255, 255, 155);
}

Image *papredoll_drhs[4];
Image *papredoll_dlhus[4];
Image *papredoll_dlhs[4];
Image *papredoll_dbods[5];
Image *paperdoll_armor_texture[4][17][3];  // 0x511294
// int paperdoll_array_51132C[165];
Image *papredoll_dlaus[5];
Image *papredoll_dlads[4];
Image *papredoll_flying_feet[22];      // 005115E0
Image *paperdoll_boots_texture[4][6];  // 511638
Image *paperdoll_cloak_collar_texture[4][10];
Image *paperdoll_cloak_texture[4][10];
Image *paperdoll_helm_texture[2][16];  // 511698
Image *paperdoll_belt_texture[4][7];   // 511718

const int paperdoll_Weapon[4][16][2] = {
    // 4E4C30
    {{128, 205},
     {30, 144},
     {88, 85},
     {0, 0},
     {0, 0},
     {0, 0},
     {17, 104},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0}},
    {{131, 201},
     {38, 158},
     {98, 87},
     {0, 0},
     {0, 0},
     {0, 0},
     {21, 100},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0}},
    {{131, 216},
     {29, 186},
     {88, 119},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0}},
    {{123, 216},
     {35, 184},
     {98, 119},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0},
     {0, 0}},
};

const int paperdoll_Boot[4][7][2] = {  // 4E5490
    0xE,  0x11D, 0xD,  0x11D, 0xC,  0x10A, 0xA,  0xFF,  0xD,  0xF9,
    0xD,  0x137, 0xC,  0x10E, 0x14, 0x125, 0x13, 0x122, 0x15, 0x120,
    0x15, 0x114, 0x13, 0x10A, 0x11, 0x13E, 0x11, 0x116, 0x1D, 0x121,
    0x1C, 0x11F, 0x1B, 0x11B, 0x1C, 0x117, 0x16, 0x116, 0x1B, 0x137,
    0x1B, 0x11B, 0x1F, 0x127, 0x1F, 0x122, 0x1B, 0x11B, 0x1D, 0x117,
    0x1D, 0x116, 0x1D, 0x137, 0x1B, 0x11F,
};
const int paperdoll_Cloak[4][10][2] = {  // 4E5570
    0x11, 0x68, 0xF,  0x68, 0x14, 0x71, 0x19, 0x6B, 0x21, 0x6F, 0x5,  0x68,
    0x5,  0x68, 0x14, 0x71, 0x3,  0x6B, 0xF,  0x6F, 0x15, 0x64, 0xB,  0x6B,
    0xE,  0x67, 0x15, 0x6B, 0x1B, 0x6F, 0x3,  0x6B, 0,    0x6B, 0xE,  0x67,
    0,    0x6B, 0x3,  0x6F, 0x10, 0x8A, 0x9,  0x8B, 0x18, 0x98, 0x25, 0x91,
    0x29, 0x90, 0x8,  0x8A, 0x9,  0x8B, 0x18, 0x98, 0x3,  0x91, 0x3,  0x90,
    0x14, 0x92, 0x10, 0x92, 0x15, 0x98, 0x1F, 0x91, 0x22, 0x90, 0x8,  0x92,
    0xC,  0x92, 0x15, 0x98, 0x3,  0x91, 0x3,  0x90,
};
const int paperdoll_CloakCollar[4][10][2] = {  // 4E56B0
    0x11, 0x68, 0x34, 0x64, 0x21, 0x69, 0x1D, 0x67, 0x20, 0x67, 0x21, 0x68,
    0x34, 0x64, 0x21, 0x69, 0x1D, 0x67, 0x1F, 0x67, 0x13, 0x64, 0x35, 0x66,
    0x29, 0x68, 0x1F, 0x68, 0x1F, 0x6A, 0x21, 0x6A, 0x2B, 0x66, 0x26, 0x68,
    0x1F, 0x68, 0x1F, 0x6A, 0,    0,    0x30, 0x87, 0x1E, 0x86, 0x1B, 0x86,
    0x1C, 0x8A, 0x21, 0x87, 0x30, 0x87, 0x1E, 0x86, 0x1B, 0x86, 0x1C, 0x8A,
    0,    0,    0x38, 0x8A, 0x24, 0x8B, 0x1D, 0x8B, 0x21, 0x8C, 0x27, 0x8A,
    0x34, 0x8A, 0x24, 0x8B, 0x25, 0x8B, 0x21, 0x8C,
};
// int dword_4E56B4;
const int paperdoll_Belt[4][7][2] = {  // 4E57F0
    0x3A, 0xB6, 0x37, 0xB2, 0x34, 0xB9, 0x3A, 0xB9, 0x37, 0xB7, 0x38, 0xAC,
    0x37, 0xB7, 0x3E, 0xAD, 0x3A, 0xAC, 0x37, 0xB0, 0x3A, 0xB1, 0x39, 0xB0,
    0x3C, 0xA5, 0x39, 0xB0, 0x3B, 0xD5, 0x37, 0xD2, 0x31, 0xD5, 0x39, 0xD6,
    0x37, 0xD8, 0x37, 0xD1, 0x37, 0xD8, 0x42, 0xD2, 0x3F, 0xD0, 0x3B, 0xD7,
    0x3C, 0xD5, 0x3B, 0xD6, 0x3E, 0xCF, 0x36, 0xD6,
};
const int paperdoll_Helm[4][16][2] = {  // 4E58D0
      0x3E, 0x1F, 0x41, 0x2C, 0x37, 0x2F, 0x31, 0x32, 0x37, 0x2A, 0x39, 0x28,
      0x36, 0x34, 0x41, 0x38, 0x40, 0x31, 0x40, 0x21, 0x40, 0x31, 0x3C, 0x33,
      0x3D, 0x24, 0x3A, 0x1A, 0x37, 0x2A, 0x41, 0x48, 0x41, 0x1E, 0x42, 0x2B,
      0x37, 0x2F, 0x34, 0x30, 0x39, 0x29, 0x3A, 0x26, 0x36, 0x34, 0x41, 0x37,
      0x42, 0x32, 0x40, 0x21, 0x40, 0x31, 0x40, 0x2F, 0x3E, 0x22, 0x3B, 0x1A,
      0x39, 0x29, 0x42, 0x47, 0x3F, 0x47, 0x41, 0x56, 0x37, 0x59, 0x32, 0x5E,
      0x37, 0x58, 0x39, 0x54, 0x34, 0x61, 0x40, 0x61, 0x41, 0x5D, 0x3E, 0x4F,
      0x3E, 0x5B, 0x3D, 0x5B, 0x3F, 0x4C, 0x3B, 0x45, 0x37, 0x58, 0x41, 0x74,
      0x45, 0x45, 0x46, 0x54, 0x3A, 0x55, 0x38, 0x58, 0x3C, 0x54, 0x3F, 0x52,
      0x39, 0x5B, 0x45, 0x5C, 0x47, 0x5C, 0x44, 0x4B, 0x44, 0x57, 0x43, 0x55,
      0x44, 0x4A, 0x3E, 0x45, 0x3C, 0x54, 0x47, 0x70,
};
const int pPaperdoll_Beards[4] = {  // 4E5AD0
    52,
    130,
    56,
    136,
};
const int pPaperdoll_LeftHand[4][2] = {  // 4E5AE0
    0x67, 0x6A, 0x65, 0x6C, 0x74, 0x8D, 0x74, 0x93,
};
const int pPaperdoll_SecondLeftHand[4][2] = {  // 4E5B00
    0x1A, 0x6B, 0x28, 0x6D, 0x19, 0x8D, 0x20, 0x92,
};
const int pPaperdoll_RightHand[4][2] = {  // 4E5B20
    0x1E, 0x90, 0x22, 0x9E, 0x19, 0xBA, 0x1F, 0xB8,
};
const int pPaperdollLeftEmptyHand[4][2] = {  // 4E5B40
    0x80, 0xCD, 0x83, 0xC9, 0x83, 0xD8, 0x7B, 0xD8,
};

int pPaperdoll_BodyX = 481;                  // 004E4C28
int pPaperdoll_BodyY = 0;                    // 004E4C2C
const int paperdoll_Armor_Coord[4][17][2] = {  // 4E4E30
    // X     Y
    0x2C, 0x67, 0x30, 0x69, 0x2D, 0x67, 0x2C, 0x64, 0x14, 0x66, 0x22, 0x67,
    0x20, 0x66, 0x25, 0x66, 0x12, 0x66,  // Human
    0x0A, 0x66, 0x13, 0x64, 0x0E, 0x64, 0x0A, 0x63, 0x14, 0x66, 0x0A, 0x63,
    0x0A, 0x66, 0x25, 0x66,

    0x32, 0x68, 0x32, 0x69, 0x35, 0x69, 0x33, 0x68, 0x24, 0x67, 0x30, 0x69,
    0x33, 0x68, 0x31, 0x69, 0x19, 0x69, 0x19, 0x6A, 0x16, 0x66, 0x16, 0x65,
    0x0F, 0x6B, 0x24, 0x67, 0x0F, 0x6B, 0x19, 0x6A, 0x31, 0x69,

    0x2A, 0x8C, 0x29, 0x8C, 0x2A, 0x89, 0x29, 0x86, 0x12, 0x87, 0x2D, 0x89,
    0x2A, 0x88, 0x25, 0x87, 0x12, 0x8B, 0x12, 0x8B, 0x11, 0x8A, 0x15, 0x87,
    0x09, 0x89, 0x12, 0x87, 0x09, 0x89, 0x12, 0x8B, 0x25, 0x87,

    0x33, 0x90, 0x32, 0x90, 0x34, 0x91, 0x32, 0x8E, 0x21, 0x8B, 0x31, 0x8B,
    0x33, 0x8E, 0x2F, 0x8F, 0x16, 0x8D, 0x18, 0x8C, 0x19, 0x8C, 0x1B, 0x8E,
    0x0C, 0x8C, 0x21, 0x8B, 0x0C, 0x8C, 0x18, 0x8C, 0x2F, 0x8F,
};
const int paperdoll_shoulder_coord[4][17][2] = {  // 4E5050
    0x64, 0x67, 0x61, 0x67, 0x65, 0x68, 0x6E, 0x74, 0x6C, 0x68, 0x61, 0x67,
    0x66, 0x68, 0x6C, 0x6A, 0x6E, 0x6D, 0x67, 0x69, 0x70, 0x67, 0x6E, 0x6D,
    0x6C, 0x6F, 0x6C, 0x68, 0x6C, 0x6F, 0x67, 0x69, 0x6C, 0x6A,

    0x60, 0x6B, 0x60, 0x6C, 0x60, 0x6B, 0x61, 0x6A, 0x60, 0x69, 0x60, 0x6A,
    0x60, 0x6A, 0x61, 0x69, 0x63, 0x6A, 0x64, 0x6A, 0x61, 0x66, 0x66, 0x67,
    0x64, 0x6C, 0x60, 0x69, 0x64, 0x6C, 0x64, 0x6A, 0x61, 0x69,

    0x6D, 0x8C, 0x75, 0x8C, 0,    0,    0x72, 0x8D, 0x6A, 0x89, 0,    0,
    0x73, 0x8C, 0x69, 0x8C, 0x6E, 0x8D, 0x71, 0x8D, 0x70, 0x8D, 0x72, 0x8D,
    0x74, 0x8E, 0x6A, 0x89, 0x74, 0x8E, 0x71, 0x8D, 0x69, 0x8C,

    0x72, 0x91, 0x72, 0x91, 0,    0,    0x6E, 0x92, 0x6F, 0x91, 0,    0,
    0,    0,    0x6E, 0x91, 0x71, 0x90, 0x72, 0x8D, 0x72, 0x90, 0x73, 0x93,
    0x73, 0x90, 0x6F, 0x91, 0x73, 0x90, 0x72, 0x8D, 0x6E, 0x91,
};
const int paperdoll_shoulder_second_coord[4][17][2] = {  // dword_4E5270
    0,    0,    0x61, 0x67, 0,    0,    0x64, 0x69, 0x64, 0x68, 0,    0,
    0,    0,    0x5E, 0x66, 0x5F, 0x69, 0x55, 0x69, 0x5F, 0x67, 0x5F, 0x68,
    0x32, 0x69, 0x64, 0x68, 0x32, 0x69, 0x55, 0x69, 0x5E, 0x66,

    0,    0,    0x60, 0x6C, 0,    0,    0x60, 0x6C, 0x5E, 0x69, 0,    0,
    0,    0,    0x5D, 0x6A, 0x5B, 0x6A, 0x5B, 0x6A, 0x59, 0x69, 0x56, 0x68,
    0x38, 0x6E, 0x5E, 0x69, 0x38, 0x6E, 0x5B, 0x6A, 0x5D, 0x6A,

    0,    0,    0x75, 0x8C, 0,    0,    0x72, 0x8D, 0x62, 0x89, 0,    0,
    0,    0,    0x69, 0x8C, 0x5E, 0x8D, 0x61, 0x8D, 0x5F, 0x8D, 0x60, 0x8D,
    0x2E, 0x8C, 0x62, 0x89, 0x2E, 0x8C, 0x61, 0x8D, 0x69, 0x8C,

    0,    0,    0x72, 0x91, 0,    0,    0x72, 0x91, 0x67, 0x8F, 0,    0,
    0,    0,    0x6E, 0x91, 0x64, 0x93, 0x65, 0x8C, 0x65, 0x91, 0x67, 0x91,
    0x36, 0x90, 0x67, 0x8F, 0x36, 0x90, 0x65, 0x8C, 0x6E, 0x91,
};

const char *dlad_texnames_by_face[25] = {
    "pc01lad", "pc02lad", "pc03lad", "pc04lad", "pc05lad", "pc06lad", "pc07lad",
    "pc08lad", "pc09lad", "pc10lad", "pc11lad", "pc12lad", "pc13lad", "pc14lad",
    "pc15lad", "pc16lad", "pc17lad", "pc18lad", "pc19lad", "pc20lad", "pc21lad",
    "pc22lad", "pc23lad", "pc24lad", "pc25lad"};
const char *dlau_texnames_by_face[25] = {
    "pc01lau", "pc02lau", "pc03lau", "pc04lau", "pc05lau", "pc06lau", "pc07lau",
    "pc08lau", "pc09lau", "pc10lau", "pc11lau", "pc12lau", "pc13lau", "pc14lau",
    "pc15lau", "pc16lau", "pc17lau", "pc18lau", "pc19lau", "pc20lau", "pc21lau",
    "pc22lau", "pc23lau", "pc24lau", "pc25lau"};
const char *dbod_texnames_by_face[25] = {
    "pc01bod", "pc02bod", "pc03bod", "pc04bod", "pc05bod", "pc06bod", "pc07bod",
    "pc08bod", "pc09bod", "pc10bod", "pc11bod", "pc12bod", "pc13bod", "pc14bod",
    "pc15bod", "pc16bod", "pc17bod", "pc18bod", "pc19bod", "pc20bod", "pc21bod",
    "pc22bod", "pc23bod", "pc24bod", "pc25bod"};
const char *drh_texnames_by_face[25] = {
    "pc01rh", "pc02rh", "pc03rh", "pc04rh", "pc05rh", "pc06rh", "pc07rh",
    "pc08rh", "pc09rh", "pc10rh", "pc11rh", "pc12rh", "pc13rh", "pc14rh",
    "pc15rh", "pc16rh", "pc17rh", "pc18rh", "pc19rh", "pc20rh", "pc21rh",
    "pc22rh", "pc23rh", "pc24rh", "pc25rh"};
const char *dlh_texnames_by_face[25] = {
    "pc01lh", "pc02lh", "pc03lh", "pc04lh", "pc05lh", "pc06lh", "pc07lh",
    "pc08lh", "pc09lh", "pc10lh", "pc11lh", "pc12lh", "pc13lh", "pc14lh",
    "pc15lh", "pc16lh", "pc17lh", "pc18lh", "pc19lh", "pc20lh", "pc21lh",
    "pc22lh", "pc23lh", "pc24lh", "pc25lh"};
const char *dlhu_texnames_by_face[25] = {
    "pc01lhu", "pc02lhu", "pc03lhu", "pc04lhu", "pc05lhu", "pc06lhu", "pc07lhu",
    "pc08lhu", "pc09lhu", "pc10lhu", "pc11lhu", "pc12lhu", "pc13lhu", "pc14lhu",
    "pc15lhu", "pc16lhu", "pc17lhu", "pc18lhu", "pc19lhu", "pc20lhu", "pc21lhu",
    "pc22lhu", "pc23lhu", "pc24lhu", "pc25lhu"};

const int pArmorSkills[5] = {PLAYER_SKILL_LEATHER, PLAYER_SKILL_CHAIN,
                             PLAYER_SKILL_PLATE, PLAYER_SKILL_SHIELD,
                             PLAYER_SKILL_DODGE};
const int pWeaponSkills[9] = {
    PLAYER_SKILL_AXE,   PLAYER_SKILL_BOW,     PLAYER_SKILL_DAGGER,
    PLAYER_SKILL_MACE,  PLAYER_SKILL_SPEAR,   PLAYER_SKILL_STAFF,
    PLAYER_SKILL_SWORD, PLAYER_SKILL_UNARMED, PLAYER_SKILL_BLASTER};
const int pMiscSkills[12] = {PLAYER_SKILL_ALCHEMY,      PLAYER_SKILL_ARMSMASTER,
                             PLAYER_SKILL_BODYBUILDING, PLAYER_SKILL_ITEM_ID,
                             PLAYER_SKILL_MONSTER_ID,   PLAYER_SKILL_LEARNING,
                             PLAYER_SKILL_TRAP_DISARM,  PLAYER_SKILL_MEDITATION,
                             PLAYER_SKILL_MERCHANT,     PLAYER_SKILL_PERCEPTION,
                             PLAYER_SKILL_REPAIR,       PLAYER_SKILL_STEALING};
const int pMagicSkills[9] = {
    PLAYER_SKILL_FIRE,  PLAYER_SKILL_AIR,    PLAYER_SKILL_WATER,
    PLAYER_SKILL_EARTH, PLAYER_SKILL_SPIRIT, PLAYER_SKILL_MIND,
    PLAYER_SKILL_BODY,  PLAYER_SKILL_LIGHT,  PLAYER_SKILL_DARK};

Image *ui_character_skills_background = nullptr;
Image *ui_character_awards_background = nullptr;
Image *ui_character_stats_background = nullptr;
Image *ui_character_inventory_background = nullptr;
Image *ui_character_inventory_background_strip = nullptr;
Image *ui_character_inventory_magnification_glass = nullptr;
Image *ui_character_inventory_paperdoll_background = nullptr;
Image *ui_character_inventory_paperdoll_rings_background = nullptr;
Image *ui_character_inventory_paperdoll_rings_close = nullptr;

std::array<Image *, 16> paperdoll_dbrds;

GUIWindow_CharacterRecord::GUIWindow_CharacterRecord(
    unsigned int uActiveCharacter, enum CURRENT_SCREEN screen)
    : GUIWindow(WINDOW_CharacterRecord, 0, 0, window->GetWidth(), window->GetHeight(),
                (GUIButton *)uActiveCharacter) {
    pEventTimer->Pause();
    pAudioPlayer->StopChannels(-1, -1);
    bRingsShownInCharScreen = false;
    CharacterUI_LoadPaperdollTextures();
    current_screen_type = screen;

    pCharacterScreen_StatsBtn = CreateButton(
        pViewport->uViewportTL_X + 12, pViewport->uViewportTL_Y + 308,
        paperdoll_dbrds[9]->GetWidth(), paperdoll_dbrds[9]->GetHeight(), 1, 0,
        UIMSG_ClickStatsBtn, 0, 'S', localization->GetString(216),  // Stats
        {{paperdoll_dbrds[10], paperdoll_dbrds[9]}});
    pCharacterScreen_SkillsBtn = CreateButton(
        pViewport->uViewportTL_X + 102, pViewport->uViewportTL_Y + 308,
        paperdoll_dbrds[7]->GetWidth(), paperdoll_dbrds[7]->GetHeight(), 1, 0,
        UIMSG_ClickSkillsBtn, 0, 'K', localization->GetString(205),  // Skills
        {{paperdoll_dbrds[8], paperdoll_dbrds[7]}});
    pCharacterScreen_InventoryBtn = CreateButton(
        pViewport->uViewportTL_X + 192, pViewport->uViewportTL_Y + 308,
        paperdoll_dbrds[5]->GetWidth(), paperdoll_dbrds[5]->GetHeight(), 1, 0,
        UIMSG_ClickInventoryBtn, 0, 'I',
        localization->GetString(120),  // Inventory
        {{paperdoll_dbrds[6], paperdoll_dbrds[5]}});
    pCharacterScreen_AwardsBtn = CreateButton(
        pViewport->uViewportTL_X + 282, pViewport->uViewportTL_Y + 308,
        paperdoll_dbrds[3]->GetWidth(), paperdoll_dbrds[3]->GetHeight(), 1, 0,
        UIMSG_ClickAwardsBtn, 0, 'A', localization->GetString(22),  // Awards
        {{paperdoll_dbrds[4], paperdoll_dbrds[3]}});
    pCharacterScreen_ExitBtn = CreateButton(
        pViewport->uViewportTL_X + 371, pViewport->uViewportTL_Y + 308,
        paperdoll_dbrds[1]->GetWidth(), paperdoll_dbrds[1]->GetHeight(), 1, 0,
        UIMSG_ClickExitCharacterWindowBtn, 0, 0,
        localization->GetString(79),  // Exit
        {{paperdoll_dbrds[2], paperdoll_dbrds[1]}});
    CreateButton(0, 0, 476, 345, 1, 122, UIMSG_InventoryLeftClick, 0, 0, "");
    pCharacterScreen_DetalizBtn =
        CreateButton(600, 300, 30, 30, 1, 0, UIMSG_ChangeDetaliz, 0, 0,
                     localization->GetString(64));
    pCharacterScreen_DollBtn =
        CreateButton(476, 0, 164, 345, 1, 0, UIMSG_ClickPaperdoll, 0, 0, "");

    CreateButton(61, 424, 31, 0, 2, 94, UIMSG_SelectCharacter, 1, '1', "");
    CreateButton(177, 424, 31, 0, 2, 94, UIMSG_SelectCharacter, 2, '2', "");
    CreateButton(292, 424, 31, 0, 2, 94, UIMSG_SelectCharacter, 3, '3', "");
    CreateButton(407, 424, 31, 0, 2, 94, UIMSG_SelectCharacter, 4, '4', "");

    CreateButton(0, 0, 0, 0, 1, 0, UIMSG_CycleCharacters, 0, '\t', "");
    FillAwardsData();

    ui_character_skills_background =
        assets->GetImage_ColorKey("fr_skill", 0x7FF);
    ui_character_awards_background =
        assets->GetImage_ColorKey("fr_award", 0x7FF);
    ui_character_stats_background =
        assets->GetImage_ColorKey("fr_stats", 0x7FF);
    ui_character_inventory_background_strip =
        assets->GetImage_ColorKey("fr_strip", 0x7FF);
}

void GUIWindow_CharacterRecord::Update() {
    auto player = pPlayers[uActiveCharacter];

    render->ClearZBuffer(0, 479);
    switch (current_character_screen_window) {
        case WINDOW_CharacterWindow_Stats: {
            CharacterUI_ReleaseButtons();
            ReleaseAwardsScrollBar();
            CharacterUI_StatsTab_Draw(player);
            render->DrawTextureAlphaNew(
                pCharacterScreen_StatsBtn->uX / 640.0f,
                pCharacterScreen_StatsBtn->uY / 480.0f,
                assets->GetImage_ColorKey("ib-cd1-d", 0x7FF));
            break;
        }
        case WINDOW_CharacterWindow_Skills: {
            if (dword_507CC0_activ_ch != uActiveCharacter) {
                CharacterUI_ReleaseButtons();
                CharacterUI_SkillsTab_CreateButtons();
            }
            ReleaseAwardsScrollBar();
            CharacterUI_SkillsTab_Draw(player);
            render->DrawTextureAlphaNew(
                pCharacterScreen_SkillsBtn->uX / 640.0f,
                pCharacterScreen_SkillsBtn->uY / 480.0f,
                assets->GetImage_ColorKey("ib-cd2-d", 0x7FF));
            break;
        }
        case WINDOW_CharacterWindow_Awards: {
            CharacterUI_ReleaseButtons();
            ReleaseAwardsScrollBar();
            CreateAwardsScrollBar();
            CharacterUI_AwardsTab_Draw(player);
            render->DrawTextureAlphaNew(
                pCharacterScreen_AwardsBtn->uX / 640.0f,
                pCharacterScreen_AwardsBtn->uY / 480.0f,
                assets->GetImage_ColorKey("ib-cd4-d", 0x7FF));
            break;
        }
        case WINDOW_CharacterWindow_Inventory: {
            CharacterUI_ReleaseButtons();
            ReleaseAwardsScrollBar();
            CharacterUI_InventoryTab_Draw(player, false);
            render->DrawTextureAlphaNew(
                pCharacterScreen_InventoryBtn->uX / 640.0f,
                pCharacterScreen_InventoryBtn->uY / 480.0f,
                assets->GetImage_ColorKey("ib-cd3-d", 0x7FF));
            break;
        }
        default:
            __debugbreak();
            break;
    }

    if (bRingsShownInCharScreen)
        CharacterUI_DrawPaperdollWithRingOverlay(player);
    else
        CharacterUI_DrawPaperdoll(player);
}

void GUIWindow_CharacterRecord::ShowStatsTab() {
    current_character_screen_window = WINDOW_CharacterWindow_Stats;
    CharacterUI_ReleaseButtons();
    ReleaseAwardsScrollBar();
    new OnButtonClick3(
        WINDOW_CharacterWindow_Stats,
        pCharacterScreen_StatsBtn->uX,
        pCharacterScreen_StatsBtn->uY, 0, 0,
        pCharacterScreen_StatsBtn);
}

void GUIWindow_CharacterRecord::ShowSkillsTab() {
    current_character_screen_window = WINDOW_CharacterWindow_Skills;
    CharacterUI_ReleaseButtons();
    ReleaseAwardsScrollBar();
    CharacterUI_SkillsTab_CreateButtons();
    new OnButtonClick3(
        WINDOW_CharacterWindow_Skills,
        pCharacterScreen_SkillsBtn->uX,
        pCharacterScreen_SkillsBtn->uY, 0, 0,
        pCharacterScreen_SkillsBtn);
}

void GUIWindow_CharacterRecord::ShowInventoryTab() {
    current_character_screen_window = WINDOW_CharacterWindow_Inventory;
    ReleaseAwardsScrollBar();
    CharacterUI_ReleaseButtons();
    new OnButtonClick3(
        WINDOW_CharacterWindow_Inventory,
        pCharacterScreen_InventoryBtn->uX,
        pCharacterScreen_InventoryBtn->uY, 0, 0,
        pCharacterScreen_InventoryBtn);
}

void GUIWindow_CharacterRecord::ShowAwardsTab() {
    ReleaseAwardsScrollBar();
    CharacterUI_ReleaseButtons();
    CreateAwardsScrollBar();
    current_character_screen_window = WINDOW_CharacterWindow_Awards;
    new OnButtonClick3(
        WINDOW_CharacterWindow_Awards,
        pCharacterScreen_AwardsBtn->uX,
        pCharacterScreen_AwardsBtn->uY, 0, 0,
        pCharacterScreen_AwardsBtn);
    FillAwardsData();
}

void GUIWindow_CharacterRecord::ToggleRingsOverlay() {
    int v128, v125, v123, v121;

    bRingsShownInCharScreen ^= 1;
    pCharacterScreen_DetalizBtn->Release();
    pCharacterScreen_DollBtn->Release();
    if (bRingsShownInCharScreen) {
        v128 = ui_character_inventory_paperdoll_rings_close->GetHeight();
        v125 = ui_character_inventory_paperdoll_rings_close->GetWidth();
        v123 = 445;
        v121 = 470;
    } else {
        v128 = 30;
        v125 = 30;
        v123 = 300;
        v121 = 600;
    }
    pCharacterScreen_DetalizBtn = pGUIWindow_CurrentMenu->CreateButton(
        v121, v123, v125, v128, 1, 0, UIMSG_ChangeDetaliz, 0, 0,
        localization->GetString(64));  // "Detail Toggle"
    pCharacterScreen_DollBtn = pGUIWindow_CurrentMenu->CreateButton(
        0x1DCu, 0, 0xA4u, 0x159u, 1, 0, UIMSG_ClickPaperdoll, 0, 0, "");
    viewparams->bRedrawGameUI = true;
}

GUIWindow *CastSpellInfo::GetCastSpellInInventoryWindow() {
    pEventTimer->Pause();
    pAudioPlayer->StopChannels(-1, -1);
    bRingsShownInCharScreen = 0;
    CharacterUI_LoadPaperdollTextures();
    current_screen_type = CURRENT_SCREEN::SCREEN_CASTING;
    GUIWindow *CS_inventory_window = new GUIWindow_Inventory_CastSpell(
        0, 0, window->GetWidth(), window->GetHeight(), (GUIButton *)this, "");
    pCharacterScreen_ExitBtn = CS_inventory_window->CreateButton(
        394, 318, 75, 33, 1, 0, UIMSG_ClickExitCharacterWindowBtn, 0, 0,
        localization->GetString(79),  // Close
        {{paperdoll_dbrds[2], paperdoll_dbrds[1]}});
    CS_inventory_window->CreateButton(0, 0, 0x1DCu, 0x159u, 1, 122,
                                      UIMSG_InventoryLeftClick, 0, 0, "");
    pCharacterScreen_DollBtn = CS_inventory_window->CreateButton(
        0x1DCu, 0, 0xA4u, 0x159u, 1, 0, UIMSG_ClickPaperdoll, 0, 0, "");

    CS_inventory_window->CreateButton(61, 424, 31, 0, 2, 94,
                                      UIMSG_SelectCharacter, 1, '1', "");
    CS_inventory_window->CreateButton(177, 424, 31, 0, 2, 94,
                                      UIMSG_SelectCharacter, 2, '2', "");
    CS_inventory_window->CreateButton(292, 424, 31, 0, 2, 94,
                                      UIMSG_SelectCharacter, 3, '3', "");
    CS_inventory_window->CreateButton(407, 424, 31, 0, 2, 94,
                                      UIMSG_SelectCharacter, 4, '4', "");

    return CS_inventory_window;
}

static int CharacterUI_SkillsTab_Draw__DrawSkillTable(
    Player *player, int x, int y, const int *skill_list, int skill_list_size,
    int right_margin, const char *skill_group_name) {
    int y_offset = y;

    auto str = StringPrintf("%s\r%03d%s", skill_group_name, right_margin,
                            localization->GetString(131));  // "Magic"    "Level"
    pGUIWindow_CurrentMenu->DrawText(
        pFontArrus, x, y, ui_character_header_text_color, str, 0, 0, 0);

    int num_skills_drawn = 0;
    for (int i = 0; i < skill_list_size; ++i) {
        PLAYER_SKILL_TYPE skill = (PLAYER_SKILL_TYPE)skill_list[i];
        for (size_t j = 0; j < pGUIWindow_CurrentMenu->vButtons.size(); ++j) {
            GUIButton *v8 = pGUIWindow_CurrentMenu->GetControl(j);
            int v9 = v8->field_1C;
            if ((short)(v8->field_1C) >= 0)
                continue;  // skips an of the stats skills innv awards buttons

            if ((v9 & 0x7FFF) != skill)
                continue;  // skips buttons that dont match skill

            ++num_skills_drawn;
            y_offset = v8->uY;

            // ushort skill_value = player->pActiveSkills[skill];
            int skill_level = player->GetActualSkillLevel(skill);

            uint skill_color = 0;
            uint skill_mastery_color = 0;
            if (player->uSkillPoints > skill_level)
                skill_color = ui_character_skill_upgradeable_color;

            if (pGUIWindow_CurrentMenu->pCurrentPosActiveItem ==
                j) {  // this needs to reset??
                if (player->uSkillPoints > skill_level)
                    skill_mastery_color = ui_character_bonus_text_color;
                else
                    skill_mastery_color = ui_character_skill_default_color;
                skill_color = skill_mastery_color;
            }

            if (player->GetActualSkillMastery(skill) == 1) {
                auto Strsk = StringPrintf("%s\r%03d%2d",
                                          localization->GetSkillName(skill),
                                          right_margin, skill_level);
                pGUIWindow_CurrentMenu->DrawText(pFontLucida, x, v8->uY,
                                                 skill_color, Strsk, 0, 0, 0);
            } else {
                const char *skill_level_str = nullptr;

                switch (player->GetActualSkillMastery(skill)) {
                    case 4:
                        skill_level_str = localization->GetString(96);
                        break;  // Grand
                    case 3:
                        skill_level_str = localization->GetString(432);
                        break;  // Master
                    case 2:
                        skill_level_str = localization->GetString(433);
                        break;  // Expert
                }

                if (!skill_mastery_color)
                    skill_mastery_color = ui_character_header_text_color;

                auto Strsk = StringPrintf(
                    "%s \f%05d%s\f%05d\r%03d%2d",
                    localization->GetSkillName(skill), skill_mastery_color,
                    skill_level_str, skill_color, right_margin, skill_level);
                pGUIWindow_CurrentMenu->DrawText(pFontLucida, x, v8->uY,
                                                 skill_color, Strsk, 0, 0, 0);
            }
        }
    }

    if (!num_skills_drawn) {
        y_offset += pFontLucida->GetHeight() - 3;
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, x, y_offset, 0,
                                         localization->GetString(153), 0, 0,
                                         0);  // None
    }

    return y_offset;
}

//----- (00419719) --------------------------------------------------------
void GUIWindow_CharacterRecord::CharacterUI_SkillsTab_Draw(Player *player) {
    render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f,
                                ui_character_skills_background);

    auto str =
        StringPrintf("%s \f%05d%s\f00000\r177%s: \f%05d%d\f00000",  // ^Pv[]
                     localization->GetString(206),  // Skills for
                     ui_character_header_text_color, player->pName,
                     localization->GetString(207),  // Skill Points
                     player->uSkillPoints ? ui_character_bonus_text_color
                                          : ui_character_default_text_color,
                     player->uSkillPoints);
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 24, 18, 0, str, 0, 0, 0);

    int y = 2 * pFontLucida->GetHeight() + 13;
    y = CharacterUI_SkillsTab_Draw__DrawSkillTable(
        player, 24, y, pWeaponSkills, 9, 400,
        localization->GetString(242));  // "Weapons"

    y += 2 * pFontLucida->GetHeight() - 10;
    CharacterUI_SkillsTab_Draw__DrawSkillTable(
        player, 24, y, pMagicSkills, 9, 400,
        localization->GetString(138));  // "Magic"

    y = 2 * pFontLucida->GetHeight() + 13;
    y = CharacterUI_SkillsTab_Draw__DrawSkillTable(
        player, 248, y, pArmorSkills, 5, 177,
        localization->GetString(11));  // "Armor"

    y += 2 * pFontLucida->GetHeight() - 10;
    y = CharacterUI_SkillsTab_Draw__DrawSkillTable(
        player, 248, y, pMiscSkills, 12, 177,
        localization->GetString(143));  // "Misc"
}

//----- (0041A000) --------------------------------------------------------
void GUIWindow_CharacterRecord::CharacterUI_AwardsTab_Draw(Player *player) {
    int items_per_page;       // eax@1
    // char Source[100];         // [sp+Ch] [bp-C4h]@1
    GUIWindow awards_window;  // [sp+70h] [bp-60h]@1

    render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f,
                                ui_character_awards_background);

    String str =
        StringPrintf("%s \f%05d", localization->GetString(23),
                     ui_character_header_text_color)  // Awards for
        + localization->FormatString(
              429, player->pName,
              localization->GetClassName(player->classType))  // %s the %s
        + "\f00000";

    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 24, 18, 0, str, 0, 0, 0);
    items_per_page = books_primary_item_per_page;
    awards_window.uFrameX = 12;
    awards_window.uFrameY = 48;
    awards_window.uFrameWidth = 424;
    awards_window.uFrameHeight = 290;
    awards_window.uFrameZ = 435;
    awards_window.uFrameW = 337;
    if (BtnDown_flag && num_achieved_awards + books_primary_item_per_page <
                            full_num_items_in_book)
        items_per_page = books_primary_item_per_page++ + 1;
    if (BtnUp_flag && items_per_page) {
        --items_per_page;
        books_primary_item_per_page = items_per_page;
    }

    if (books_page_number < 0) {
        items_per_page += num_achieved_awards;
        books_primary_item_per_page = items_per_page;
        if ((signed int)(num_achieved_awards + items_per_page) >
            full_num_items_in_book) {
            items_per_page = full_num_items_in_book - num_achieved_awards;
            books_primary_item_per_page = items_per_page;
        }
    } else if (books_page_number > 0) {
        items_per_page -= num_achieved_awards;
        books_primary_item_per_page = items_per_page;
        if (items_per_page < 0) {
            items_per_page = 0;
            books_primary_item_per_page = items_per_page;
        }
    }
    BtnDown_flag = 0;
    BtnUp_flag = 0;
    num_achieved_awards = 0;
    books_page_number = 0;

    for (int i = items_per_page; i < full_num_items_in_book; ++i) {
        String str;
        auto v6 = pAwards[achieved_awards[i]]
                      .pText;  // (char *)dword_723E80_award_related[v20 / 4];

        switch (achieved_awards[i]) {
            case Award_Arena_PageWins:
                str = StringPrintf(v6, pParty->uNumArenaPageWins);
                break;
            case Award_Arena_SquireWins:
                str = StringPrintf(v6, pParty->uNumArenaSquireWins);
                break;
            case Award_Arena_KnightWins:
                str = StringPrintf(v6, pParty->uNumArenaKnightWins);
                break;
            case Award_Arena_LordWins:
                str = StringPrintf(v6, pParty->uNumArenaLordWins);
                break;
            case Award_ArcomageWins:
                str = StringPrintf(v6, pParty->uNumArcomageWins);
                break;
            case Award_ArcomageLoses:
                str = StringPrintf(v6, pParty->uNumArcomageLoses);
                break;
            case Award_Deaths:
                str = StringPrintf(v6, pParty->uNumDeaths);
                break;
            case Award_BountiesCollected:
                str = StringPrintf(v6, pParty->uNumBountiesCollected);
                break;
            case Award_Fine:
                str = StringPrintf(v6, pParty->uFine);
                break;
            case Award_PrisonTerms:
                str = StringPrintf(v6, pParty->uNumPrisonTerms);
                break;
        }

        if (str.length() < 1) str = String(v6);

        awards_window.DrawText(
            pFontArrus, 0, 0,
            ui_character_award_color[pAwards[achieved_awards[i]].uPriority % 6],
            str, 0, 0, 0);
        awards_window.uFrameY =
            pFontArrus->CalcTextHeight(str, awards_window.uFrameWidth, 0) +
            awards_window.uFrameY + 8;
        if (awards_window.uFrameY > awards_window.uFrameHeight) break;

        ++num_achieved_awards;
    }
}

//----- (0041A2C1) --------------------------------------------------------
unsigned int GetSizeInInventorySlots(unsigned int uNumPixels) {
    if ((signed int)uNumPixels < 14) uNumPixels = 14;
    return ((signed int)(uNumPixels - 14) >> 5) + 1;
}

//----- (0041A556) --------------------------------------------------------
void draw_leather() {
    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, ui_leather_mm7);
}

//----- (0043CC7C) --------------------------------------------------------
void CharacterUI_DrawPaperdoll(Player *player) {
    ItemGen *item;              // edi@38
    int item_X;                 // ebx@38
    int index;                  // eax@65
    Image *v59;                 // ebx@129
    Image *v75;                 // ebx@170
    Image *v94;                 // ebx@214
    Image *v127;                // ebx@314
    Image *v153;                // eax@370
    char *v166;                 // [sp-8h] [bp-54h]@16
    const char *container = nullptr;      // [sp-8h] [bp-54h]@79
    char *v181;                 // [sp-8h] [bp-54h]@337
    int item_Y;                 // [sp+10h] [bp-3Ch]@38
    int pBodyComplection;       // [sp+24h] [bp-28h]@6
    bool two_handed_left_fist;  // [sp+34h] [bp-18h]@361
    int IsDwarf;                // [sp+40h] [bp-Ch]@4

    if (player->GetRace() == CHARACTER_RACE_DWARF) {
        IsDwarf = 1;
        pBodyComplection = player->GetSexByVoice() == SEX_MALE ? 2 : 3;
    } else {
        IsDwarf = 0;
        pBodyComplection = player->GetSexByVoice() == SEX_MALE ? 0 : 1;
    }

    int uPlayerID = 0;
    for (uint i = 0; i < 4; ++i) {
        if (pPlayers[i + 1] == player) {
            uPlayerID = i + 1;
            break;
        }
    }

    render->ResetUIClipRect();
    render->DrawTextureAlphaNew(467 / 640.0f, 0,
                                ui_character_inventory_paperdoll_background);
    if (IsPlayerWearingWatersuit[uPlayerID]) {  // акваланг
        render->DrawTextureAlphaNew(pPaperdoll_BodyX / 640.0f,
                                    pPaperdoll_BodyY / 480.0f,
                                    papredoll_dbods[uPlayerID - 1]);
        if (!bRingsShownInCharScreen)
            render->ZDrawTextureAlpha(
                pPaperdoll_BodyX / 640.0f, pPaperdoll_BodyY / 480.0f,
                papredoll_dbods[uPlayerID - 1], player->pEquipment.uArmor);

        //Рука не занята или ...
        if (!player->GetItem(&PlayerEquipment::uMainHand) ||
            (player->GetMainHandItem()->GetItemEquipType() !=
             EQUIP_TWO_HANDED) &&
                (player->GetMainHandItem()->GetItemEquipType() !=
                     PLAYER_SKILL_SPEAR ||
                 player->GetItem(&PlayerEquipment::uShield)))
            render->DrawTextureAlphaNew(
                (pPaperdoll_BodyX + pPaperdoll_LeftHand[pBodyComplection][0]) /
                    640.0f,
                (pPaperdoll_BodyY + pPaperdoll_LeftHand[pBodyComplection][1]) /
                    480.0f,
                papredoll_dlads[uPlayerID - 1]);
        // -----------------------------------------------------(Hand/Рука)---------------------------------------------------------------
        if (player->GetItem(&PlayerEquipment::uMainHand)) {
            item = player->GetMainHandItem();
            item_X = pPaperdoll_BodyX +
                     paperdoll_Weapon[pBodyComplection][1][0] -
                     pItemsTable->pItems[item->uItemID].uEquipX;
            item_Y = pPaperdoll_BodyY +
                     paperdoll_Weapon[pBodyComplection][1][1] -
                     pItemsTable->pItems[item->uItemID].uEquipY;
            if (item->uItemID == ITEM_BLASTER)
                v166 = "item64v1";
            else
                v166 = item->GetIconName();

            auto texture = assets->GetImage_Alpha(v166);

            if (item->ItemEnchanted()) {
                if (item->AuraEffectRed())
                    container = "sptext01";
                else if (item->AuraEffectBlue())
                    container = "sp28a";
                else if (item->AuraEffectGreen())
                    container = "sp30a";
                else if (item->AuraEffectPurple())
                    container = "sp91a";
                _50C9A8_item_enchantment_timer -= pEventTimer->uTimeElapsed;
                if (_50C9A8_item_enchantment_timer <= 0) {
                    _50C9A8_item_enchantment_timer = 0;
                    item->ResetEnchantAnimation();
                    ptr_50C9A4_ItemToEnchant = nullptr;
                }
                render->BlendTextures(
                    item_X, item_Y, texture,
                    assets->GetImage_ColorKey(container, 0x7FF),
                    OS_GetTime() / 10, 0,
                    255);  // should this pass enchant timer?

            } else if (item->uAttributes & ITEM_BROKEN) {
                render->DrawTransparentRedShade(item_X / 640.0f,
                    item_Y / 480.0f, texture);
            } else if (item->uAttributes & ITEM_IDENTIFIED) {
                render->DrawTextureAlphaNew(item_X / 640.0f, item_Y / 480.0f,
                    texture);
            } else {
                render->DrawTransparentGreenShade(item_X / 640.0f,
                    item_Y / 480.0f, texture);
            }

            if (!bRingsShownInCharScreen)
                render->ZDrawTextureAlpha(item_X / 640.0f, item_Y / 480.0f,
                                          texture,
                                          player->pEquipment.uMainHand);
        }
    } else {  // без акваланга
        // ----------------(Bow/
        // Лук)-------------------------------------------------
        if (player->GetItem(&PlayerEquipment::uBow)) {
            item = player->GetBowItem();
            item_X = pPaperdoll_BodyX +
                     paperdoll_Weapon[pBodyComplection][2][0] -
                     pItemsTable->pItems[item->uItemID].uEquipX;
            item_Y = pPaperdoll_BodyY +
                     paperdoll_Weapon[pBodyComplection][2][1] -
                     pItemsTable->pItems[item->uItemID].uEquipY;

            auto texture = assets->GetImage_Alpha(item->GetIconName());
            if (item->ItemEnchanted()) {  // применён закл
                if (item->AuraEffectRed())
                    container = "sptext01";
                else if (item->AuraEffectBlue())
                    container = "sp28a";
                else if (item->AuraEffectGreen())
                    container = "sp30a";
                else if (item->AuraEffectPurple())
                    container = "sp91a";
                _50C9A8_item_enchantment_timer -= pEventTimer->uTimeElapsed;
                if (_50C9A8_item_enchantment_timer <= 0) {
                    _50C9A8_item_enchantment_timer = 0;
                    item->ResetEnchantAnimation();
                    ptr_50C9A4_ItemToEnchant = nullptr;
                }
                render->BlendTextures(
                    item_X, item_Y, texture,
                    assets->GetImage_ColorKey(container, 0x7FF),
                    OS_GetTime() / 10, 0, 255);
            } else if (item->uAttributes & ITEM_BROKEN) {
                render->DrawTransparentRedShade(item_X / 640.0f,
                    item_Y / 480.0f, texture);
            } else if (!(item->uAttributes & ITEM_IDENTIFIED)) {
                render->DrawTransparentGreenShade(item_X / 640.0f,
                    item_Y / 480.0f, texture);
            } else {  // опознанный лук
                render->DrawTextureAlphaNew(item_X / 640.0f, item_Y / 480.0f,
                    texture);
            }

            if (!bRingsShownInCharScreen)
                render->ZDrawTextureAlpha(item_X / 640.0f, item_Y / 480.0f,
                                          texture, player->pEquipment.uBow);
        }
        // -----------------------------(Cloak/Плащ)---------------------------------------------------------
        if (player->GetItem(&PlayerEquipment::uCloak)) {
            item = player->GetCloakItem();
            switch (item->uItemID) {
                case ITEM_RELIC_TWILIGHT:
                    index = 5;
                    break;
                case ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP:
                    index = 6;
                    break;
                case ITEM_RARE_SUN_CLOAK:
                    index = 7;
                    break;
                case ITEM_RARE_MOON_CLOAK:
                    index = 8;
                    break;
                case ITEM_RARE_VAMPIRES_CAPE:
                    index = 9;
                    break;
                default:
                    index = item->uItemID - 105;
                    break;
            }
            if (index >= 0 && index < 10) {
                item_X = pPaperdoll_BodyX +
                         paperdoll_Cloak[pBodyComplection][index][0];
                item_Y = pPaperdoll_BodyY +
                         paperdoll_Cloak[pBodyComplection][index][1];

                if (item->ItemEnchanted()) {
                    if (item->AuraEffectRed())
                        container = "sptext01";
                    else if (item->AuraEffectBlue())
                        container = "sp28a";
                    else if (item->AuraEffectGreen())
                        container = "sp30a";
                    else if (item->AuraEffectPurple())
                        container = "sp91a";
                    _50C9A8_item_enchantment_timer -= pEventTimer->uTimeElapsed;
                    if (_50C9A8_item_enchantment_timer <= 0) {
                        _50C9A8_item_enchantment_timer = 0;
                        item->ResetEnchantAnimation();  // ~0x000000F0
                        ptr_50C9A4_ItemToEnchant = nullptr;
                    }
                    render->BlendTextures(
                        item_X, item_Y,
                        paperdoll_cloak_texture[pBodyComplection][index],
                        assets->GetImage_ColorKey(container, 0x7FF),
                        OS_GetTime() / 10, 0, 255);
                } else if (item->uAttributes & ITEM_BROKEN) {
                    render->DrawTransparentRedShade(
                        item_X / 640.0f, item_Y / 480.0f,
                        paperdoll_cloak_texture[pBodyComplection][index]);
                } else {
                    render->DrawTextureAlphaNew(
                        item_X / 640.0f, item_Y / 480.0f,
                        paperdoll_cloak_texture[pBodyComplection][index]);
                }

                if (!bRingsShownInCharScreen)
                    render->ZDrawTextureAlpha(
                        item_X / 640.0f, item_Y / 480.0f,
                        paperdoll_cloak_texture[pBodyComplection][index],
                        player->pEquipment.uCloak);
            }
        }
        // -------------------------------(Paperdoll/Кукла)-------------------------------------------
        render->DrawTextureAlphaNew(pPaperdoll_BodyX / 640.0f,
                                    pPaperdoll_BodyY / 480.0f,
                                    papredoll_dbods[uPlayerID - 1]);
        // -------------------------------(Armor/Броня)-----------------------------------------------
        if (player->GetItem(&PlayerEquipment::uArmor)) {
            item = player->GetArmorItem();
            switch (item->uItemID) {
                case ITEM_ARTIFACT_GOVERNORS_ARMOR:
                    index = 15;
                    break;
                case ITEM_ARTIFACT_YORUBA:
                    index = 14;
                    break;
                case ITEM_RELIC_HARECS_LEATHER:
                    index = 13;
                    break;
                case ITEM_ELVEN_CHAINMAIL:
                    index = 16;
                    break;
                default:
                    index = item->uItemID - 66;
                    break;
            }
            if (index >= 0 && index < 17) {
                item_X = pPaperdoll_BodyX +
                         paperdoll_Armor_Coord[pBodyComplection][index][0];
                item_Y = pPaperdoll_BodyY +
                         paperdoll_Armor_Coord[pBodyComplection][index][1];

                if (item->ItemEnchanted()) {
                    if (item->AuraEffectRed())
                        container = "sptext01";
                    else if (item->AuraEffectBlue())
                        container = "sp28a";
                    else if (item->AuraEffectGreen())
                        container = "sp30a";
                    else if (item->AuraEffectPurple())
                        container = "sp91a";
                    _50C9A8_item_enchantment_timer -= pEventTimer->uTimeElapsed;
                    if (_50C9A8_item_enchantment_timer <= 0) {
                        _50C9A8_item_enchantment_timer = 0;
                        item->ResetEnchantAnimation();  // ~0x000000F0
                        ptr_50C9A4_ItemToEnchant = nullptr;
                    }
                    render->BlendTextures(
                        item_X, item_Y,
                        paperdoll_armor_texture[pBodyComplection][index][0],
                        assets->GetImage_ColorKey(container, 0x7FF),
                        OS_GetTime() / 10, 0, 255);
                } else if (item->uAttributes & ITEM_BROKEN) {
                    render->DrawTransparentRedShade(
                        item_X / 640.0f, item_Y / 480.0f,
                        paperdoll_armor_texture[pBodyComplection][index][0]);
                } else if (!(item->uAttributes & ITEM_IDENTIFIED)) {
                    render->DrawTransparentGreenShade(
                        item_X / 640.0f, item_Y / 480.0f,
                        paperdoll_armor_texture[pBodyComplection][index][0]);
                } else {
                    render->DrawTextureAlphaNew(
                        item_X / 640.0f, item_Y / 480.0f,
                        paperdoll_armor_texture[pBodyComplection][index][0]);
                }

                if (!bRingsShownInCharScreen)
                    render->ZDrawTextureAlpha(
                        item_X / 640.0f, item_Y / 480.0f,
                        paperdoll_armor_texture[pBodyComplection][index][0],
                        player->pEquipment.uArmor);
            }
        }
        // ----------------------------------(Boot/Обувь)--------------------------------------------------------
        if (player->GetItem(&PlayerEquipment::uBoot)) {
            item = player->GetBootItem();
            switch (item->uItemID) {
                case ITEM_ARTIFACT_HERMES_SANDALS:
                    index = 5;
                    v59 = papredoll_flying_feet[player->uCurrentFace];
                    break;
                case ITEM_ARTIFACT_LEAGUE_BOOTS:
                    index = 6;
                    v59 = paperdoll_boots_texture[pBodyComplection][5];
                    break;
                default:
                    index = item->uItemID - 115;
                    v59 = paperdoll_boots_texture[pBodyComplection][index];
                    break;
            }
            if (index >= 0 && index < 7) {
                item_X = pPaperdoll_BodyX +
                         paperdoll_Boot[pBodyComplection][index][0];
                item_Y = pPaperdoll_BodyY +
                         paperdoll_Boot[pBodyComplection][index][1];

                if (item->ItemEnchanted()) {
                    if (item->AuraEffectRed())
                        container = "sptext01";
                    else if (item->AuraEffectBlue())
                        container = "sp28a";
                    else if (item->AuraEffectGreen())
                        container = "sp30a";
                    else if (item->AuraEffectPurple())
                        container = "sp91a";
                    _50C9A8_item_enchantment_timer -= pEventTimer->uTimeElapsed;
                    if (_50C9A8_item_enchantment_timer <= 0) {
                        _50C9A8_item_enchantment_timer = 0;
                        item->ResetEnchantAnimation();  // ~0x000000F0
                        ptr_50C9A4_ItemToEnchant = nullptr;
                    }
                    render->BlendTextures(
                        item_X, item_Y, v59,
                        assets->GetImage_ColorKey(container, 0x7FF),
                        OS_GetTime() * 0.1, 0, 255);
                } else if (item->uAttributes & ITEM_BROKEN) {
                    render->DrawTransparentRedShade(item_X / 640.0f,
                        item_Y / 480.0f, v59);
                } else if (item->uAttributes & ITEM_IDENTIFIED) {
                    render->DrawTextureAlphaNew(item_X / 640.0f,
                        item_Y / 480.0f, v59);
                } else {
                    render->DrawTransparentGreenShade(item_X / 640.0f,
                        item_Y / 480.0f, v59);
                }

                if (!bRingsShownInCharScreen)
                    render->ZDrawTextureAlpha(item_X / 640.0f, item_Y / 480.0f,
                                              v59, player->pEquipment.uBoot);
            }
        }
        // --------------------------------------------(Hand/Рука)------------------------------------------------------
        if (!player->GetItem(&PlayerEquipment::uMainHand) ||
            (player->GetMainHandItem()->GetItemEquipType() !=
             EQUIP_TWO_HANDED) &&
                (player->GetMainHandItem()->GetPlayerSkillType() !=
                     PLAYER_SKILL_SPEAR ||
                 player->GetItem(&PlayerEquipment::uShield)))
            render->DrawTextureAlphaNew(
                (pPaperdoll_BodyX + pPaperdoll_LeftHand[pBodyComplection][0]) /
                    640.0f,
                (pPaperdoll_BodyY + pPaperdoll_LeftHand[pBodyComplection][1]) /
                    480.0f,
                papredoll_dlads[uPlayerID - 1]);
        // --------------------------------------------(Belt/Пояс)-------------------------------------------------------
        if (player->GetItem(&PlayerEquipment::uBelt)) {
            item = player->GetBeltItem();
            switch (item->uItemID) {
                case ITEM_RILIC_TITANS_BELT:
                    index = 5;
                    break;
                case ITEM_ARTIFACT_HEROS_BELT:
                    index = 6;
                    break;
                default:
                    index = item->uItemID - 100;
                    break;
            }
            if (index >= 0 && index < 7) {
                item_X = pPaperdoll_BodyX +
                         paperdoll_Belt[pBodyComplection][index][0];
                item_Y = pPaperdoll_BodyY +
                         paperdoll_Belt[pBodyComplection][index][1];
                if (IsDwarf != 1 || index == 5)
                    v75 = paperdoll_belt_texture[pBodyComplection][index];
                else
                    v75 = paperdoll_belt_texture[pBodyComplection - 2][index];

                if (item->ItemEnchanted()) {
                    if (item->AuraEffectRed())
                        container = "sptext01";
                    else if (item->AuraEffectBlue())
                        container = "sp28a";
                    else if (item->AuraEffectGreen())
                        container = "sp30a";
                    else if (item->AuraEffectPurple())
                        container = "sp91a";
                    _50C9A8_item_enchantment_timer -= pEventTimer->uTimeElapsed;
                    if (_50C9A8_item_enchantment_timer <= 0) {
                        _50C9A8_item_enchantment_timer = 0;
                        item->ResetEnchantAnimation();  // ~0x000000F0
                        ptr_50C9A4_ItemToEnchant = nullptr;
                    }
                    render->BlendTextures(
                        item_X, item_Y, v75,
                        assets->GetImage_ColorKey(container, 0x7FF),
                        OS_GetTime() / 10, 0, 255);
                } else if (item->uAttributes & ITEM_BROKEN) {
                    render->DrawTransparentRedShade(item_X / 640.0f,
                        item_Y / 480.0f, v75);
                } else if (item->uAttributes & ITEM_IDENTIFIED) {
                    render->DrawTextureAlphaNew(item_X / 640.0f,
                        item_Y / 480.0f, v75);
                } else {
                    render->DrawTransparentGreenShade(item_X / 640.0f,
                        item_Y / 480.0f, v75);
                }

                if (!bRingsShownInCharScreen)
                    render->ZDrawTextureAlpha(item_X / 640.0f, item_Y / 480.0f,
                                              v75, player->pEquipment.uBelt);
            }
        }
        // --------------------------------(Shoulder/Плечи)---------------------------------------------
        if (player->GetItem(&PlayerEquipment::uArmor)) {
            item = player->GetArmorItem();
            switch (item->uItemID) {
                case ITEM_ARTIFACT_GOVERNORS_ARMOR:
                    index = 15;
                    break;
                case ITEM_ARTIFACT_YORUBA:
                    index = 14;
                    break;
                case ITEM_RELIC_HARECS_LEATHER:
                    index = 13;
                    break;
                case ITEM_ELVEN_CHAINMAIL:
                    index = 16;
                    break;
                default:
                    index = item->uItemID - 66;
                    break;
            }
            if (index >= 0 && index < 17) {
                if (player->GetItem(&PlayerEquipment::uMainHand) &&
                    (player->GetMainHandItem()->GetItemEquipType() ==
                         EQUIP_TWO_HANDED ||
                     player->GetMainHandItem()->GetPlayerSkillType() ==
                             PLAYER_SKILL_SPEAR &&
                         !player->GetItem(
                             &PlayerEquipment::uShield))) {  // без щита
                    v94 = paperdoll_armor_texture[pBodyComplection][index][2];
                    if (paperdoll_armor_texture[pBodyComplection][index][2]) {
                        v94 =
                            paperdoll_armor_texture[pBodyComplection][index][1];
                        item_X = pPaperdoll_BodyX +
                                 paperdoll_shoulder_coord[pBodyComplection]
                                                         [index][0];
                        item_Y = pPaperdoll_BodyY +
                                 paperdoll_shoulder_coord[pBodyComplection]
                                                         [index][1];
                    } else {
                        item_X =
                            pPaperdoll_BodyX +
                            paperdoll_shoulder_second_coord[pBodyComplection]
                                                           [index][0];
                        item_Y =
                            pPaperdoll_BodyY +
                            paperdoll_shoulder_second_coord[pBodyComplection]
                                                           [index][1];
                    }
                    if (v94) {
                        if (item->ItemEnchanted()) {
                            if (paperdoll_armor_texture[pBodyComplection][index]
                                                       [2]) {
                                if (item->AuraEffectRed())
                                    container = "sptext01";
                                else if (item->AuraEffectBlue())
                                    container = "sp28a";
                                else if (item->AuraEffectGreen())
                                    container = "sp30a";
                                else if (item->AuraEffectPurple())
                                    container = "sp91a";
                                _50C9A8_item_enchantment_timer -=
                                    pEventTimer->uTimeElapsed;
                                if (_50C9A8_item_enchantment_timer <= 0) {
                                    _50C9A8_item_enchantment_timer = 0;
                                    item->ResetEnchantAnimation();  // ~0x000000F0
                                    ptr_50C9A4_ItemToEnchant = nullptr;
                                }
                                render->BlendTextures(
                                    item_X, item_Y, v94,
                                    assets->GetImage_ColorKey(container, 0x7FF),
                                    OS_GetTime() / 10, 0, 255);
                            }
                        } else if (item->uAttributes & ITEM_BROKEN) {
                            render->DrawTransparentRedShade(
                                item_X / 640.0f, item_Y / 480.0f, v94);
                        } else if (item->uAttributes & ITEM_IDENTIFIED) {
                            render->DrawTextureAlphaNew(item_X / 640.0f,
                                item_Y / 480.0f, v94);
                        } else {
                            render->DrawTransparentGreenShade(
                                item_X / 640.0f, item_Y / 480.0f, v94);
                        }
                    }
                } else {  // без ничего или с щитом
                    // v94 =
                    // paperdoll_armor_texture[pBodyComplection][index][1];
                    if (paperdoll_armor_texture[pBodyComplection][index][1]) {
                        item_X = pPaperdoll_BodyX +
                                 paperdoll_shoulder_coord[pBodyComplection]
                                                         [index][0];
                        item_Y = pPaperdoll_BodyY +
                                 paperdoll_shoulder_coord[pBodyComplection]
                                                         [index][1];

                        if (item->ItemEnchanted()) {
                            if (item->AuraEffectRed())
                                container = "sptext01";
                            else if (item->AuraEffectBlue())
                                container = "sp28a";
                            else if (item->AuraEffectGreen())
                                container = "sp30a";
                            else if (item->AuraEffectPurple())
                                container = "sp91a";
                            _50C9A8_item_enchantment_timer -=
                                pEventTimer->uTimeElapsed;
                            if (_50C9A8_item_enchantment_timer <= 0) {
                                _50C9A8_item_enchantment_timer = 0;
                                item->ResetEnchantAnimation();  // ~0x000000F0
                                ptr_50C9A4_ItemToEnchant = nullptr;
                            }
                            render->BlendTextures(
                                item_X, item_Y,
                                paperdoll_armor_texture[pBodyComplection][index]
                                                       [1],
                                assets->GetImage_ColorKey(container, 0x7FF),
                                OS_GetTime() / 10, 0, 255);
                        } else if (item->uAttributes & ITEM_BROKEN) {
                            render->DrawTransparentRedShade(
                                item_X / 640.0f, item_Y / 480.0f,
                                paperdoll_armor_texture[pBodyComplection][index]
                                [1]);
                        } else if (item->uAttributes & ITEM_IDENTIFIED) {
                            render->DrawTextureAlphaNew(
                                item_X / 640.0f, item_Y / 480.0f,
                                paperdoll_armor_texture[pBodyComplection][index]
                                [1]);
                        } else {
                            render->DrawTransparentGreenShade(
                                item_X / 640.0f, item_Y / 480.0f,
                                paperdoll_armor_texture[pBodyComplection][index]
                                [1]);
                        }
                    }
                }
            }
        }
        // ----------------------------------------------(Cloak collar/воротник
        // плаща)-------------------------------------
        if (player->GetItem(&PlayerEquipment::uCloak)) {
            item = player->GetCloakItem();
            switch (item->uItemID) {
                case ITEM_RELIC_TWILIGHT:
                    index = 5;
                    break;
                case ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP:
                    index = 6;
                    break;
                case ITEM_RARE_SUN_CLOAK:
                    index = 7;
                    break;
                case ITEM_RARE_MOON_CLOAK:
                    index = 8;
                    break;
                case ITEM_RARE_VAMPIRES_CAPE:
                    index = 9;
                    break;
                default:
                    index = item->uItemID - 105;
            }
            if (index > 0 && index < 10) {  // leather cloak has no collar
                item_X = pPaperdoll_BodyX +
                         paperdoll_CloakCollar[pBodyComplection][index][0];
                item_Y = pPaperdoll_BodyY +
                         paperdoll_CloakCollar[pBodyComplection][index][1];
                // int r = pIcons_LOD->FindTextureByName("item325v2a1");

                if (paperdoll_cloak_collar_texture[pBodyComplection][index]) {
                    if (item->ItemEnchanted()) {
                        if (item->AuraEffectRed())
                            container = "sptext01";
                        else if (item->AuraEffectBlue())
                            container = "sp28a";
                        else if (item->AuraEffectGreen())
                            container = "sp30a";
                        else if (item->AuraEffectPurple())
                            container = "sp91a";
                        _50C9A8_item_enchantment_timer -=
                            pEventTimer->uTimeElapsed;
                        if (_50C9A8_item_enchantment_timer <= 0) {
                            _50C9A8_item_enchantment_timer = 0;
                            item->ResetEnchantAnimation();  // ~0x000000F0
                            ptr_50C9A4_ItemToEnchant = nullptr;
                        }
                        render->BlendTextures(
                            item_X, item_Y,
                            paperdoll_cloak_collar_texture[pBodyComplection]
                                                          [index],
                            assets->GetImage_ColorKey(container, 0x7FF),
                            OS_GetTime() / 10, 0, 255);
                    } else if (item->uAttributes & ITEM_BROKEN) {
                        render->DrawTransparentRedShade(
                            item_X / 640.0f, item_Y / 480.0f,
                            paperdoll_cloak_collar_texture[pBodyComplection]
                            [index]);
                    } else {
                        render->DrawTextureAlphaNew(
                            item_X / 640.0f, item_Y / 480.0f,
                            paperdoll_cloak_collar_texture[pBodyComplection]
                            [index]);
                    }

                    if (!bRingsShownInCharScreen)
                        render->ZDrawTextureAlpha(
                            item_X / 640.0f, item_Y / 480.0f,
                            paperdoll_cloak_collar_texture[pBodyComplection]
                                                          [index],
                            player->pEquipment.uCloak);
                }
            }
        }
        // --------------------------------------------(Beards/Борода)-------------------------------------------------------
        if (player->uCurrentFace == 12 || player->uCurrentFace == 13) {
            if (paperdoll_dbrds[player->uCurrentFace])
                render->DrawTextureAlphaNew(
                    (pPaperdoll_BodyX +
                     pPaperdoll_Beards[2 * player->uCurrentFace - 24]) /
                        640.0f,
                    (pPaperdoll_BodyY +
                     pPaperdoll_Beards[2 * player->uCurrentFace - 23]) /
                        480.0f,
                    paperdoll_dbrds[player->uCurrentFace]);
        }
        // --------------------------------------------(Helm/Шлем)------------------------------------------------------------
        if (player->GetItem(&PlayerEquipment::uHelm)) {
            item = player->GetHelmItem();
            switch (item->uItemID) {
                case ITEM_RELIC_TALEDONS_HELM:
                    index = 11;
                    break;
                case ITEM_RELIC_SCHOLARS_CAP:
                    index = 12;
                    break;
                case ITEM_RELIC_PHYNAXIAN_CROWN:
                    index = 13;
                    break;
                case ITEM_ARTIFACT_MINDS_EYE:
                    index = 14;
                    break;
                case ITEM_RARE_SHADOWS_MASK:
                    index = 15;
                    break;
                default:
                    index = item->uItemID - 89;
            }
            if (index >= 0 && index < 16) {
                item_X = pPaperdoll_BodyX +
                         paperdoll_Helm[pBodyComplection][index][0];
                item_Y = pPaperdoll_BodyY +
                         paperdoll_Helm[pBodyComplection][index][1];
                if (IsDwarf != 1 || item->uItemID != ITEM_92)
                    v127 =
                        paperdoll_helm_texture[player->GetSexByVoice()][index];
                else
                    v127 = paperdoll_dbrds[11];

                if (item->ItemEnchanted()) {
                    if (item->AuraEffectRed())
                        container = "sptext01";
                    else if (item->AuraEffectBlue())
                        container = "sp28a";
                    else if (item->AuraEffectGreen())
                        container = "sp30a";
                    else if (item->AuraEffectPurple())
                        container = "sp91a";
                    _50C9A8_item_enchantment_timer -= pEventTimer->uTimeElapsed;
                    if (_50C9A8_item_enchantment_timer <= 0) {
                        _50C9A8_item_enchantment_timer = 0;
                        item->ResetEnchantAnimation();
                        ptr_50C9A4_ItemToEnchant = nullptr;
                    }
                    render->BlendTextures(
                        item_X, item_Y, v127,
                        assets->GetImage_ColorKey(container, 0x7FF),
                        OS_GetTime() / 10, 0, 255);
                } else if (item->uAttributes & ITEM_BROKEN) {
                    render->DrawTransparentRedShade(item_X / 640.0f,
                        item_Y / 480.0f, v127);
                } else if (item->uAttributes & ITEM_IDENTIFIED) {
                    render->DrawTextureAlphaNew(item_X / 640.0f,
                        item_Y / 480.0f, v127);
                } else {
                    render->DrawTransparentGreenShade(item_X / 640.0f,
                        item_Y / 480.0f, v127);
                }

                if (!bRingsShownInCharScreen)
                    render->ZDrawTextureAlpha(item_X / 640.0f, item_Y / 480.0f,
                                              v127, player->pEquipment.uHelm);
            }
        }
        // ------------------------------------------------(Hand3/Рука3)-------------------------------------------
        if (player->GetItem(&PlayerEquipment::uMainHand)) {
            item = player->GetMainHandItem();
            item_X = pPaperdoll_BodyX +
                     paperdoll_Weapon[pBodyComplection][1][0] -
                     pItemsTable->pItems[item->uItemID].uEquipX;
            item_Y = pPaperdoll_BodyY +
                     paperdoll_Weapon[pBodyComplection][1][1] -
                     pItemsTable->pItems[item->uItemID].uEquipY;
            if (item->uItemID == ITEM_BLASTER)
                v181 = "item64v1";
            else
                v181 = item->GetIconName();

            auto texture = assets->GetImage_Alpha(v181);

            if (item->ItemEnchanted()) {
                if (item->AuraEffectRed())
                    container = "sptext01";
                else if (item->AuraEffectBlue())
                    container = "sp28a";
                else if (item->AuraEffectGreen())
                    container = "sp30a";
                else if (item->AuraEffectPurple())
                    container = "sp91a";
                _50C9A8_item_enchantment_timer -= pEventTimer->uTimeElapsed;
                if (_50C9A8_item_enchantment_timer <= 0) {
                    _50C9A8_item_enchantment_timer = 0;
                    item->ResetEnchantAnimation();
                    ptr_50C9A4_ItemToEnchant = nullptr;
                }
                render->BlendTextures(
                    item_X, item_Y, texture,
                    assets->GetImage_ColorKey(container, 0x7FF),
                    OS_GetTime() / 10, 0, 255);
            } else if (item->uAttributes & ITEM_BROKEN) {
                render->DrawTransparentRedShade(item_X / 640.0f,
                    item_Y / 480.0f, texture);
            } else if (item->uAttributes & ITEM_IDENTIFIED) {
                render->DrawTextureAlphaNew(item_X / 640.0f, item_Y / 480.0f,
                    texture);
            } else {
                render->DrawTransparentGreenShade(item_X / 640.0f,
                    item_Y / 480.0f, texture);
            }

            if (!bRingsShownInCharScreen)
                render->ZDrawTextureAlpha(item_X / 640.0f, item_Y / 480.0f,
                                          texture,
                                          player->pEquipment.uMainHand);
        }
        // --------------------------------------------------(Shield/Щит)---------------------------------------------
        if (player->GetItem(&PlayerEquipment::uShield)) {
            item = player->GetOffHandItem();
            if (item->GetPlayerSkillType() == PLAYER_SKILL_DAGGER ||
                item->GetPlayerSkillType() == PLAYER_SKILL_SWORD) {
                // v151 = item->uItemID - 400;
                item_X = 596;
                two_handed_left_fist = true;
                switch (item->uItemID) {
                    case 400:
                        item_Y = 86;
                        break;
                    case 403:
                        item_Y = 28;
                        break;
                    case 415:
                        item_X = 595;
                        item_Y = 33;
                        break;
                    default:
                        item_X = pPaperdoll_BodyX +
                                 paperdoll_Weapon[pBodyComplection][0][0] -
                                 pItemsTable->pItems[item->uItemID].uEquipX;
                        item_Y = pPaperdoll_BodyY +
                                 paperdoll_Weapon[pBodyComplection][0][1] -
                                 pItemsTable->pItems[item->uItemID].uEquipY;
                        break;
                }
            } else {
                two_handed_left_fist = false;
                item_X = pPaperdoll_BodyX +
                         paperdoll_Weapon[pBodyComplection][0][0] -
                         pItemsTable->pItems[item->uItemID].uEquipX;
                item_Y = pPaperdoll_BodyY +
                         paperdoll_Weapon[pBodyComplection][0][1] -
                         pItemsTable->pItems[item->uItemID].uEquipY;
            }
            v153 = assets->GetImage_Alpha(item->GetIconName());

            if (item->ItemEnchanted()) {
                // if (item->AuraEffectRed())
                    container = "sptext01";
                /*else*/
                if (item->AuraEffectBlue())
                    container = "sp28a";
                else if (item->AuraEffectGreen())
                    container = "sp30a";
                else if (item->AuraEffectPurple())
                    container = "sp91a";

                _50C9A8_item_enchantment_timer -= pEventTimer->uTimeElapsed;
                if (_50C9A8_item_enchantment_timer <= 0) {
                    _50C9A8_item_enchantment_timer = 0;
                    item->ResetEnchantAnimation();
                    ptr_50C9A4_ItemToEnchant = nullptr;
                }
                render->BlendTextures(
                    item_X, item_Y, v153,
                    assets->GetImage_ColorKey(container, 0x7FF),
                    OS_GetTime() / 10, 0, 255);
            } else if (item->uAttributes & ITEM_BROKEN) {
                render->DrawTransparentRedShade(item_X / 640.0f,
                    item_Y / 480.0f, v153);
            } else if (!(item->uAttributes & ITEM_IDENTIFIED)) {
                render->DrawTransparentGreenShade(item_X / 640.0f,
                    item_Y / 480.0f, v153);
            } else {
                render->DrawTextureAlphaNew(item_X / 640.0f, item_Y / 480.0f,
                    v153);
            }

            if (two_handed_left_fist)  // two-handed - left fist/двуручие -
                                       // левая кисть
                render->DrawTextureAlphaNew(
                    (pPaperdoll_BodyX +
                     pPaperdollLeftEmptyHand[pBodyComplection][0]) /
                        640.0f,
                    (pPaperdoll_BodyY +
                     pPaperdollLeftEmptyHand[pBodyComplection][1]) /
                        480.0f,
                    papredoll_dlhs[uPlayerID - 1]);
            if (!bRingsShownInCharScreen)
                render->ZDrawTextureAlpha(item_X / 640.0f, item_Y / 480.0f,
                                          v153, player->pEquipment.uShield);
        }
    }
    // --------------------------------------------------------(RightHand/Правая
    // кисть)--------------------------------------------------
    render->DrawTextureAlphaNew(
        (pPaperdoll_BodyX + pPaperdoll_RightHand[pBodyComplection][0]) / 640.0f,
        (pPaperdoll_BodyY + pPaperdoll_RightHand[pBodyComplection][1]) / 480.0f,
        papredoll_drhs[uPlayerID - 1]);
    // ---------------------------------------------(two-handed - hand/Двуручие -
    // рука)--------------------------------------------------
    if (player->GetItem(&PlayerEquipment::uMainHand)) {
        if (player->GetMainHandItem()->GetItemEquipType() == EQUIP_TWO_HANDED ||
            player->GetMainHandItem()->GetPlayerSkillType() ==
                    PLAYER_SKILL_SPEAR &&
                !player->GetItem(&PlayerEquipment::uShield))
            render->DrawTextureAlphaNew(
                (pPaperdoll_BodyX +
                 pPaperdoll_SecondLeftHand[pBodyComplection][0]) /
                    640.0f,
                (pPaperdoll_BodyY +
                 pPaperdoll_SecondLeftHand[pBodyComplection][1]) /
                    480.0f,
                papredoll_dlaus[uPlayerID - 1]);
    }
    // --------------------------------------------------------(two-handed -
    // fist/двуручие - кисть)----------------------------------------------------
    if (player->GetItem(&PlayerEquipment::uMainHand)) {
        item = player->GetMainHandItem();
        if (item->GetItemEquipType() == EQUIP_TWO_HANDED ||
            item->GetPlayerSkillType() == PLAYER_SKILL_SPEAR &&
                !player->GetItem(&PlayerEquipment::uShield))
            render->DrawTextureAlphaNew(
                (pPaperdoll_BodyX +
                 pPaperdoll_SecondLeftHand[pBodyComplection][0]) /
                    640.0f,
                (pPaperdoll_BodyY +
                 pPaperdoll_SecondLeftHand[pBodyComplection][1]) /
                    480.0f,
                papredoll_dlhus[uPlayerID - 1]);
    }

    if (!bRingsShownInCharScreen)  //рисование лупы
        render->DrawTextureAlphaNew(603 / 640.0f, 299 / 480.0f,
                                    ui_character_inventory_magnification_glass);

    render->DrawTextureAlphaNew(468 / 640.0f, 0, game_ui_right_panel_frame);
}

//----- (0041A2D1) --------------------------------------------------------
void CharacterUI_InventoryTab_Draw(Player *player, bool Cover_Strip) {
    render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f,
        ui_character_inventory_background);
    if (Cover_Strip) {
        if (ui_character_inventory_background_strip == nullptr) {
            // strip doesnt load if you havent already look at
                        // inventorys
            ui_character_inventory_background_strip =
                assets->GetImage_ColorKey("fr_strip", 0x7FF);
        }
        render->DrawTextureAlphaNew(8 / 640.0f, 305 / 480.0f,
            ui_character_inventory_background_strip);
    }

    for (uint i = 0; i < 126; ++i) {
        if (player->pInventoryMatrix[i] <= 0) continue;
        if (!player->pInventoryItemList[player->pInventoryMatrix[i] - 1]
                 .uItemID)
            continue;
        unsigned int uCellY = 32 * (i / 14) + 17;
        unsigned int uCellX = 32 * (i % 14) + 14;

        Image *pTexture = assets->GetImage_Alpha(
            player->pInventoryItemList[player->pInventoryMatrix[i] - 1]
                .GetIconName());

        int width = pTexture->GetWidth();
        int height = pTexture->GetHeight();
        if (width < 14) width = 14;
        if (height < 14) height = 14;

        signed int X_offset = (((signed int)((width - 14) & 0xFFFFFFE0) + 32) - width) / 2;
        signed int Y_offset = (((signed int)((height - 14) & 0xFFFFFFE0) + 32) - height) / 2;
        CharacterUI_DrawItem(uCellX + X_offset, uCellY + Y_offset,
            &(player->pInventoryItemList[player->pInventoryMatrix[i] - 1]), 0);
    }
}

static void CharacterUI_DrawItem(int x, int y, ItemGen *item, int id) {
    auto item_texture = assets->GetImage_Alpha(item->GetIconName());

    if (item->uAttributes & ITEM_ENCHANT_ANIMATION) {  // enchant animation
        Image *enchantment_texture = nullptr;
        switch (item->uAttributes & ITEM_ENCHANT_ANIMATION) {
            case ITEM_AURA_EFFECT_RED:
                enchantment_texture =
                    assets->GetImage_ColorKey("sptext01", 0x7FF);
                break;
            case ITEM_AURA_EFFECT_BLUE:
                enchantment_texture = assets->GetImage_ColorKey("sp28a", 0x7FF);
                break;
            case ITEM_AURA_EFFECT_GREEN:
                enchantment_texture = assets->GetImage_ColorKey("sp30a", 0x7FF);
                break;
            case ITEM_AURA_EFFECT_PURPLE:
                enchantment_texture = assets->GetImage_ColorKey("sp91a", 0x7FF);
                break;
        }

        _50C9A8_item_enchantment_timer -= pEventTimer->uTimeElapsed;
        if (_50C9A8_item_enchantment_timer <= 0) {
            _50C9A8_item_enchantment_timer = 0;
            item->uAttributes &= 0xFFFFFF0F;
            ptr_50C9A4_ItemToEnchant = nullptr;
        }
        render->BlendTextures(x, y, item_texture, enchantment_texture,
                              OS_GetTime() / 10, 0, 255);
    } else {
        if (item->IsBroken())
            render->DrawTransparentRedShade(x / 640.0f, y / 480.0f,
                                            item_texture);
        else if (!item->IsIdentified())
            render->DrawTransparentGreenShade(x / 640.0f, y / 480.0f,
                                              item_texture);
        else
            render->DrawTextureAlphaNew(x / 640.0f, y / 480.0f, item_texture);

        render->ZBuffer_Fill_2(x, y, item_texture, id);  // blank functions
    }
}

//----- (0043E825) --------------------------------------------------------
void CharacterUI_DrawPaperdollWithRingOverlay(Player *player) {
    CharacterUI_DrawPaperdoll(player);

    render->DrawTextureAlphaNew(
        473 / 640.0f, 0, ui_character_inventory_paperdoll_rings_background);
    render->DrawTextureAlphaNew(468 / 640.0f, 0, game_ui_right_panel_frame);
    render->DrawTextureAlphaNew(pCharacterScreen_DetalizBtn->uX / 640.0f,
                                pCharacterScreen_DetalizBtn->uY / 480.0f,
                                ui_character_inventory_paperdoll_rings_close);

    for (uint i = 0; i < 6; ++i) {
        if (!player->pEquipment.uRings[i]) continue;
        static int pPaperdollRingsX[6] = {0x1EA, 0x21A, 0x248,
                                          0x1EA, 0x21A, 0x248};
        static int pPaperdollRingsY[6] = {0x0CA, 0x0CA, 0x0CA,
                                          0x0FA, 0x0FA, 0x0FA};
        CharacterUI_DrawItem(
            pPaperdollRingsX[i], pPaperdollRingsY[i],
            &player->pInventoryItemList[player->pEquipment.uRings[i] - 1],
            player->pEquipment.uRings[i]);
    }
    if (player->pEquipment.uAmulet)
        CharacterUI_DrawItem(493, 91, player->GetAmuletItem(),
                             player->pEquipment.uAmulet);
    if (player->pEquipment.uGlove)
        CharacterUI_DrawItem(586, 88, player->GetGloveItem(),
                             player->pEquipment.uGlove);
}

//----- (0043BCA7) --------------------------------------------------------
void CharacterUI_LoadPaperdollTextures() {
    int v3;                // ebx@10
    Player *pPlayer;       // edi@12
    signed int v32;        // [sp+10h] [bp-28h]@75
    signed int v33;        // [sp+10h] [bp-28h]@77
    int pItemTXTNum;       // [sp+14h] [bp-24h]@75
    signed int v38;        // [sp+14h] [bp-24h]@79
    char pContainer[128];  // [sp+24h] [bp-14h]@12

    if (!ui_character_inventory_magnification_glass)
        ui_character_inventory_magnification_glass =
            assets->GetImage_Alpha("MAGNIF-B");

    // if ( !pParty->uAlignment || pParty->uAlignment == 1 || pParty->uAlignment
    // == 2 )
    if (!ui_character_inventory_paperdoll_background)
        ui_character_inventory_paperdoll_background =
            assets->GetImage_ColorKey("BACKDOLL", 0x7FF);

    ui_character_inventory_paperdoll_rings_background =
        assets->GetImage_Alpha("BACKHAND");

    ui_character_inventory_paperdoll_rings_close =
        ui_exit_cancel_button_background;
    for (uint i = 0; i < 4; ++i) {
        if (pPlayers[i + 1]->HasUnderwaterSuitEquipped()) {
            if (pPlayers[i + 1]->GetRace() == CHARACTER_RACE_DWARF)
                v3 = (pPlayers[i + 1]->GetSexByVoice() != 0) + 3;
            else
                v3 = (pPlayers[i + 1]->GetSexByVoice() != 0) + 1;
            sprintf(pContainer, "pc23v%dBod", v3);
            papredoll_dbods[i] =
                assets->GetImage_Alpha(pContainer);  // Body texture
            sprintf(pContainer, "pc23v%dlad", v3);
            papredoll_dlads[i] =
                assets->GetImage_Alpha(pContainer);  // Left Hand
            sprintf(pContainer, "pc23v%dlau", v3);
            papredoll_dlaus[i] =
                assets->GetImage_Alpha(pContainer);  // Left Hand2
            sprintf(pContainer, "pc23v%drh", v3);
            papredoll_drhs[i] =
                assets->GetImage_Alpha(pContainer);  // Right Hand
            sprintf(pContainer, "pc23v%dlh", v3);
            papredoll_dlhs[i] =
                assets->GetImage_Alpha(pContainer);  // Left Palm
            sprintf(pContainer, "pc23v%dlhu", v3);
            papredoll_dlhus[i] =
                assets->GetImage_Alpha(pContainer);  // Left Fist
            pPlayer = pPlayers[i + 1];

            if (pPlayer->uCurrentFace == 12 || pPlayer->uCurrentFace == 13)
                paperdoll_dbrds[(char)pPlayer->uCurrentFace] = nullptr;
            papredoll_flying_feet[pPlayer->uCurrentFace] = nullptr;

            IsPlayerWearingWatersuit[i + 1] = true;
        } else {
            papredoll_dbods[i] = assets->GetImage_Alpha(
                dbod_texnames_by_face[pPlayers[i + 1]->uCurrentFace]);
            papredoll_dlads[i] = assets->GetImage_Alpha(
                dlad_texnames_by_face[pPlayers[i + 1]->uCurrentFace]);
            papredoll_dlaus[i] = assets->GetImage_Alpha(
                dlau_texnames_by_face[pPlayers[i + 1]->uCurrentFace]);
            papredoll_drhs[i] = assets->GetImage_Alpha(
                drh_texnames_by_face[pPlayers[i + 1]->uCurrentFace]);
            papredoll_dlhs[i] = assets->GetImage_Alpha(
                dlh_texnames_by_face[pPlayers[i + 1]->uCurrentFace]);
            papredoll_dlhus[i] = assets->GetImage_Alpha(
                dlhu_texnames_by_face[pPlayers[i + 1]->uCurrentFace]);

            if (pPlayers[i + 1]->uCurrentFace == 12 ||
                pPlayers[i + 1]->uCurrentFace == 13) {
                paperdoll_dbrds[pPlayers[i + 1]->uCurrentFace] =
                    assets->GetImage_Alpha(StringPrintf(
                        "pc%02dbrd", pPlayers[i + 1]->uCurrentFace + 1));
            }

            papredoll_flying_feet[pPlayers[i + 1]->uCurrentFace] =
                assets->GetImage_Alpha(StringPrintf(
                    "item281pc%02d", pPlayers[i + 1]->uCurrentFace + 1));
            IsPlayerWearingWatersuit[i + 1] = 0;
        }
    }

    ui_ar_up_up = assets->GetImage_Alpha("ar_up_up");
    ui_ar_up_dn = assets->GetImage_Alpha("ar_up_dn");
    ui_ar_dn_up = assets->GetImage_Alpha("ar_dn_up");
    ui_ar_dn_dn = assets->GetImage_Alpha("ar_dn_dn");

    paperdoll_dbrds[9] = assets->GetImage_Solid("ib-cd1-d");
    paperdoll_dbrds[7] = assets->GetImage_Solid("ib-cd2-d");
    paperdoll_dbrds[5] = assets->GetImage_Solid("ib-cd3-d");
    paperdoll_dbrds[3] = assets->GetImage_Solid("ib-cd4-d");
    paperdoll_dbrds[1] = assets->GetImage_Solid("ib-cd5-d");

    for (uint i = 0; i < 54; ++i) {  // test equipment
        party_has_equipment[i] = 0;
        if (pParty->pPickedItem.uItemID != i + 66) {
            for (uint j = 0; j < 4; ++j) {
                if (pParty->pPlayers[j].HasItem(i + 66, 0))
                    party_has_equipment[i] = 1;
            }
        }
    }
    memset(byte_5111F6_OwnedArtifacts.data(), 0,
           sizeof(byte_5111F6_OwnedArtifacts));
    for (uint i = 0; i < 4; ++i) {
        Player *player = &pParty->pPlayers[i];

        if (player->HasItem(ITEM_ARTIFACT_GOVERNORS_ARMOR, 1))
            byte_5111F6_OwnedArtifacts[0] = 1;
        if (player->HasItem(ITEM_ARTIFACT_YORUBA, 1))
            byte_5111F6_OwnedArtifacts[1] = 1;
        if (player->HasItem(ITEM_RELIC_HARECS_LEATHER, 1))
            byte_5111F6_OwnedArtifacts[2] = 1;
        if (player->HasItem(ITEM_ARTIFACT_LEAGUE_BOOTS, 1))
            byte_5111F6_OwnedArtifacts[3] = 1;
        if (player->HasItem(ITEM_RELIC_TALEDONS_HELM, 1))
            byte_5111F6_OwnedArtifacts[4] = 1;
        if (player->HasItem(ITEM_RELIC_SCHOLARS_CAP, 1))
            byte_5111F6_OwnedArtifacts[5] = 1;
        if (player->HasItem(ITEM_RELIC_PHYNAXIAN_CROWN, 1))
            byte_5111F6_OwnedArtifacts[6] = 1;
        if (player->HasItem(ITEM_ARTIFACT_MINDS_EYE, 1))
            byte_5111F6_OwnedArtifacts[7] = 1;
        if (player->HasItem(ITEM_RARE_SHADOWS_MASK, 1))
            byte_5111F6_OwnedArtifacts[8] = 1;
        if (player->HasItem(ITEM_RILIC_TITANS_BELT, 1))
            byte_5111F6_OwnedArtifacts[9] = 1;
        if (player->HasItem(ITEM_ARTIFACT_HEROS_BELT, 1))
            byte_5111F6_OwnedArtifacts[10] = 1;
        if (player->HasItem(ITEM_RELIC_TWILIGHT, 1))
            byte_5111F6_OwnedArtifacts[11] = 1;
        if (player->HasItem(ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, 1))
            byte_5111F6_OwnedArtifacts[12] = 1;
        if (player->HasItem(ITEM_RARE_SUN_CLOAK, 1))
            byte_5111F6_OwnedArtifacts[13] = 1;
        if (player->HasItem(ITEM_RARE_MOON_CLOAK, 1))
            byte_5111F6_OwnedArtifacts[14] = 1;
        if (player->HasItem(ITEM_RARE_VAMPIRES_CAPE, 1))
            byte_5111F6_OwnedArtifacts[15] = 1;
        if (player->HasItem(ITEM_ELVEN_CHAINMAIL, 1))
            byte_5111F6_OwnedArtifacts[16] = 1;
    }

    for (uint i = 0; i < 2; ++i) {
        for (uint j = 0; j < 5; ++j) {  // Belt
            GetItemTextureFilename(pContainer, j + 100, i + 1, 0);
            paperdoll_belt_texture[i][j] = assets->GetImage_Alpha(pContainer);
        }
        GetItemTextureFilename(pContainer, 535, i + 1, 0);
        paperdoll_belt_texture[i][6] = assets->GetImage_Alpha(pContainer);
        for (uint j = 0; j < 11; ++j) {  // Helm
            GetItemTextureFilename(pContainer, j + 89, i + 1, 0);
            paperdoll_helm_texture[i][j] = assets->GetImage_Alpha(pContainer);
        }
        GetItemTextureFilename(pContainer, 521, i + 1, 0);
        paperdoll_helm_texture[i][11] = assets->GetImage_Alpha(pContainer);
        GetItemTextureFilename(pContainer, 522, i + 1, 0);
        paperdoll_helm_texture[i][12] = assets->GetImage_Alpha(pContainer);
        GetItemTextureFilename(pContainer, 523, i + 1, 0);
        paperdoll_helm_texture[i][13] = assets->GetImage_Alpha(pContainer);
        GetItemTextureFilename(pContainer, 532, i + 1, 0);
        paperdoll_helm_texture[i][14] = assets->GetImage_Alpha(pContainer);
        GetItemTextureFilename(pContainer, 544, i + 1, 0);
        paperdoll_helm_texture[i][15] = assets->GetImage_Alpha(pContainer);

        if (IsDwarfPresentInParty(true))  // the phynaxian helm uses a slightly
                                          // different graphic for dwarves
            paperdoll_dbrds[11] = assets->GetImage_Alpha("item092v3");
    }
    // v43 = 0;
    for (uint i = 0; i < 4; ++i) {
        if (ShouldLoadTexturesForRaceAndGender(i)) {
            GetItemTextureFilename(pContainer, 524, i + 1, 0);
            paperdoll_belt_texture[i][5] =
                assets->GetImage_Alpha(pContainer);  // Titans belt
            pItemTXTNum = 66;
            for (v32 = 0; v32 < 13; ++v32) {  // simple armor
                GetItemTextureFilename(pContainer, pItemTXTNum, i + 1, 0);
                paperdoll_armor_texture[i][v32][0] =
                    assets->GetImage_Alpha(pContainer);  // armor
                GetItemTextureFilename(pContainer, pItemTXTNum, i + 1, 1);
                paperdoll_armor_texture[i][v32][1] =
                    assets->GetImage_Alpha(pContainer);  // shoulder 1
                GetItemTextureFilename(pContainer, pItemTXTNum, i + 1, 2);
                paperdoll_armor_texture[i][v32][2] =
                    assets->GetImage_Alpha(pContainer);  // shoulder 2
                pItemTXTNum++;
            }
            GetItemTextureFilename(pContainer, 516, i + 1, 0);  // artefacts
            paperdoll_armor_texture[i][v32][0] =
                assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 516, i + 1, 1);
            paperdoll_armor_texture[i][v32][1] =
                assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 516, i + 1, 2);
            paperdoll_armor_texture[i][v32][2] =
                assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 505, i + 1, 0);
            paperdoll_armor_texture[i][v32 + 1][0] =
                assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 505, i + 1, 1);
            paperdoll_armor_texture[i][v32 + 1][1] =
                assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 505, i + 1, 2);
            paperdoll_armor_texture[i][v32 + 1][2] =
                assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 504, i + 1, 0);
            paperdoll_armor_texture[i][v32 + 2][0] =
                assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 504, i + 1, 1);
            paperdoll_armor_texture[i][v32 + 2][1] =
                assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 504, i + 1, 2);
            paperdoll_armor_texture[i][v32 + 2][2] =
                assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 533, i + 1, 0);
            paperdoll_armor_texture[i][v32 + 3][0] =
                assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 533, i + 1, 1);
            paperdoll_armor_texture[i][v32 + 3][1] =
                assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 533, i + 1, 2);
            paperdoll_armor_texture[i][v32 + 3][2] =
                assets->GetImage_Alpha(pContainer);
            for (v33 = 0; v33 < 5; ++v33) {  // boots
                GetItemTextureFilename(pContainer, v33 + 115, i + 1, 0);
                paperdoll_boots_texture[i][v33] =
                    assets->GetImage_Alpha(pContainer);
            }
            GetItemTextureFilename(pContainer, 512, i + 1, 0);
            paperdoll_boots_texture[i][v33] =
                assets->GetImage_Alpha(pContainer);
            for (v38 = 0; v38 < 5; ++v38) {  // Cloak
                GetItemTextureFilename(pContainer, v38 + 105, i + 1, 0);
                paperdoll_cloak_texture[i][v38] =
                    assets->GetImage_Alpha(pContainer);
                GetItemTextureFilename(pContainer, v38 + 105, i + 1, 1);
                paperdoll_cloak_collar_texture[i][v38] =
                    assets->GetImage_Alpha(pContainer);
            }

            GetItemTextureFilename(pContainer, 525, i + 1, 0);
            paperdoll_cloak_texture[i][5] = assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 530, i + 1, 0);
            paperdoll_cloak_texture[i][6] = assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 547, i + 1, 0);
            paperdoll_cloak_texture[i][7] = assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 548, i + 1, 0);
            paperdoll_cloak_texture[i][8] = assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 550, i + 1, 0);
            paperdoll_cloak_texture[i][9] = assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 525, i + 1, 1);
            paperdoll_cloak_collar_texture[i][5] =
                assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 530, i + 1, 1);
            paperdoll_cloak_collar_texture[i][6] =
                assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 547, i + 1, 1);
            paperdoll_cloak_collar_texture[i][7] =
                assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 548, i + 1, 1);
            paperdoll_cloak_collar_texture[i][8] =
                assets->GetImage_Alpha(pContainer);
            GetItemTextureFilename(pContainer, 550, i + 1, 1);
            paperdoll_cloak_collar_texture[i][9] =
                assets->GetImage_Alpha(pContainer);
        }
        // else
        //{
        // v26 = v43;
        //}
        // v43 = v26 + 40;
    }
}

void GUIWindow_CharacterRecord::CharacterUI_SkillsTab_CreateButtons() {
    int skill_id;  // [sp+18h] [bp-Ch]@8

    int buttons_count = 0;
    if (dword_507CC0_activ_ch) CharacterUI_ReleaseButtons();
    dword_507CC0_activ_ch = uActiveCharacter;
    for (GUIButton *pButton : pGUIWindow_CurrentMenu->vButtons) {
        if (pButton->msg == UIMSG_InventoryLeftClick) {
            dword_50698C_uX = pButton->uX;
            dword_506988_uY = pButton->uY;
            dword_506984_uZ = pButton->uZ;
            dword_506980_uW = pButton->uW;
            pButton->uW = 0;
            pButton->uZ = 0;
            pButton->uY = 0;
            pButton->uX = 0;
        }
        buttons_count++;
    }
    int first_rows = 0;
    Player *curr_player = &pParty->pPlayers[uActiveCharacter - 1];

    int uCurrFontHeght = pFontLucida->GetHeight();
    unsigned int current_Y = 2 * uCurrFontHeght + 13;
    for (int i = 0; i < 9; ++i) {
        skill_id = pWeaponSkills[i];
        if (curr_player->pActiveSkills[skill_id] & 0x3F) {
            current_Y += uCurrFontHeght - 3;
            ++buttons_count;
            ++first_rows;
            pGUIWindow_CurrentMenu->CreateButton(
                24, current_Y, 204, uCurrFontHeght - 3, 3, skill_id | 0x8000,
                UIMSG_SkillUp, skill_id, 0, "");
        }
    }
    if (!first_rows) current_Y += uCurrFontHeght - 3;
    current_Y += 2 * uCurrFontHeght - 6;
    for (int i = 0; i < 9; ++i) {
        skill_id = pMagicSkills[i];
        if (curr_player->pActiveSkills[skill_id] & 0x3F && buttons_count < 15) {
            current_Y += uCurrFontHeght - 3;
            ++buttons_count;
            pGUIWindow_CurrentMenu->CreateButton(
                24, current_Y, 204, uCurrFontHeght - 3, 3, skill_id | 0x8000,
                UIMSG_SkillUp, skill_id, 0, "");
        }
    }
    first_rows = 0;
    current_Y = 2 * uCurrFontHeght + 13;
    for (int i = 0; i < 5; ++i) {
        skill_id = pArmorSkills[i];
        if (curr_player->pActiveSkills[skill_id] & 0x3F) {
            current_Y += uCurrFontHeght - 3;
            ++buttons_count;
            ++first_rows;
            pGUIWindow_CurrentMenu->CreateButton(
                246, current_Y, 204, uCurrFontHeght - 3, 3, skill_id | 0x8000,
                UIMSG_SkillUp, skill_id, 0, "");
        }
    }
    if (!first_rows) current_Y += uCurrFontHeght - 3;
    current_Y += 2 * uCurrFontHeght - 6;
    for (int i = 0; i < 12; ++i) {
        skill_id = pMiscSkills[i];
        if (curr_player->pActiveSkills[skill_id] & 0x3F) {
            current_Y += uCurrFontHeght - 3;
            ++buttons_count;
            pGUIWindow_CurrentMenu->CreateButton(
                246, current_Y, 204, uCurrFontHeght - 3, 3, skill_id | 0x8000,
                UIMSG_SkillUp, skill_id, 0, "");
        }
    }

    if (buttons_count) {
        pGUIWindow_CurrentMenu->_41D08F_set_keyboard_control_group(
            buttons_count, 1, 0, buttons_count);
    }
}

void GUIWindow_CharacterRecord::CharacterUI_StatsTab_Draw(Player *player) {
    int pY;                   // ST34_4@4
    const char *pText;        // eax@9
    const char *text_format;  // [sp+14h] [bp-Ch]@4

    render->DrawTextureAlphaNew(8 / 640.0f, 8 / 480.0f,
                                ui_character_stats_background);

    auto str1 =
        StringPrintf("\f%05d", ui_character_header_text_color) +
        localization->FormatString(
            429, player->pName,
            localization->GetClassName(
                player->classType))  // "^Pi[%s] %s" / "%s the %s"
        + StringPrintf("\f00000\r180%s: \f%05d%d\f00000\n\n\n",
                       localization->GetString(207),  // "Skill points"
                       player->uSkillPoints ? ui_character_bonus_text_color
                                            : ui_character_default_text_color,
                       player->uSkillPoints);
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 26, 18, 0, str1);

    // First column(Первая колонка)
    pY = 53;
    auto str2 = StringPrintf(
        "%s\f%05u\r424%d\f00000 /\t185%d\n", localization->GetString(144),
        UI_GetHealthManaAndOtherQualitiesStringColor(player->GetActualMight(),
                                                     player->GetBaseStrength()),
        player->GetActualMight(), player->GetBaseStrength());  // Might
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 26, pY, 0, str2);

    pY += pFontArrus->GetHeight() - 2;
    auto str3 = StringPrintf(
        "%s\f%05u\r424%d\f00000 /\t185%d\n", localization->GetString(116),
        UI_GetHealthManaAndOtherQualitiesStringColor(
            player->GetActualIntelligence(), player->GetBaseIntelligence()),
        player->GetActualIntelligence(),
        player->GetBaseIntelligence());  // Intellect
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 26, pY, 0, str3);

    pY += pFontArrus->GetHeight() - 2;
    auto str4 = StringPrintf(
        "%s\f%05u\r424%d\f00000 /\t185%d\n", localization->GetString(163),
        UI_GetHealthManaAndOtherQualitiesStringColor(
            player->GetActualWillpower(), player->GetBaseWillpower()),
        player->GetActualWillpower(), player->GetBaseWillpower());  // Willpower
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 26, pY, 0, str4);

    pY += pFontArrus->GetHeight() - 2;
    auto str5 = StringPrintf(
        "%s\f%05u\r424%d\f00000 /\t185%d\n", localization->GetString(75),
        UI_GetHealthManaAndOtherQualitiesStringColor(
            player->GetActualEndurance(), player->GetBaseEndurance()),
        player->GetActualEndurance(), player->GetBaseEndurance());
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 26, pY, 0, str5);

    pY += pFontArrus->GetHeight() - 2;
    auto str6 = StringPrintf(
        "%s\f%05u\r424%d\f00000 /\t185%d\n", localization->GetString(1),
        UI_GetHealthManaAndOtherQualitiesStringColor(
            player->GetActualAccuracy(), player->GetBaseAccuracy()),
        player->GetActualAccuracy(), player->GetBaseAccuracy());
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 26, pY, 0, str6);

    pY += pFontArrus->GetHeight() - 2;
    auto str7 = StringPrintf(
        "%s\f%05u\r424%d\f00000 /\t185%d\n", localization->GetString(211),
        UI_GetHealthManaAndOtherQualitiesStringColor(player->GetActualSpeed(),
                                                     player->GetBaseSpeed()),
        player->GetActualSpeed(), player->GetBaseSpeed());
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 26, pY, 0, str7);

    pY += pFontArrus->GetHeight() - 2;
    auto str8 = StringPrintf(
        "%s\f%05u\r424%d\f00000 /\t185%d\n\n", localization->GetString(136),
        UI_GetHealthManaAndOtherQualitiesStringColor(player->GetActualLuck(),
                                                     player->GetBaseLuck()),
        player->GetActualLuck(), player->GetBaseLuck());
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 26, pY, 0, str8);

    text_format = "%s\f%05u\r424%d\f00000 /\t185%d\n";
    if (player->GetMaxHealth() >= 1000)
        text_format = "%s\f%05u\r388%d\f00000 / %d\n";
    pY += 2 * pFontArrus->GetHeight() + 5;
    auto str9 = StringPrintf(text_format, localization->GetString(108),
                             UI_GetHealthManaAndOtherQualitiesStringColor(
                                 player->sHealth, player->GetMaxHealth()),
                             player->sHealth, player->GetMaxHealth());
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 26, pY, 0, str9);

    text_format = "%s\f%05u\r424%d\f00000 /\t185%d\n";
    if (player->GetMaxMana() >= 1000)
        text_format = "%s\f%05u\r388%d\f00000 / %d\n";
    pY += pFontArrus->GetHeight() - 2;
    auto str10 = StringPrintf(text_format, localization->GetString(212),
                              UI_GetHealthManaAndOtherQualitiesStringColor(
                                  player->sMana, player->GetMaxMana()),
                              player->sMana, player->GetMaxMana());
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 26, pY, 0, str10);

    pY += pFontArrus->GetHeight() - 2;
    auto str11 = StringPrintf("%s\f%05u\r424%d\f00000 /\t185%d\n\n",
                              localization->GetString(12),
                              UI_GetHealthManaAndOtherQualitiesStringColor(
                                  player->GetActualAC(), player->GetBaseAC()),
                              player->GetActualAC(), player->GetBaseAC());
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 26, pY, 0, str11);

    pY += 2 * pFontArrus->GetHeight() - 2;
    auto str12 =
        StringPrintf("%s: \f%05d%s\n",
                     localization->GetString(47),  // Condition   /   Состояние
                     GetConditionDrawColor(player->GetMajorConditionIdx()),
                     localization->GetCharacterConditionName(
                         player->GetMajorConditionIdx()));
    pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 26, pY, 0, str12, 226,
                                           0);

    pY += pFontArrus->GetHeight() + -1;
    pText = localization->GetString(153);  // Нет
    if (player->uQuickSpell)
        pText = pSpellStats->pInfos[player->uQuickSpell].pShortName;
    auto str13 = StringPrintf("%s: %s", localization->GetString(172),
                              pText);  // Б. применение
    pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, 26, pY, 0, str13, 226,
                                           0);

    // Second column (Вторая колонка)
    pY = 50;
    text_format = Stat_string_format_2_column_less_100;
    if (player->GetActualAge() > 99)
        text_format = Stat_string_format_2_column_over_100;
    auto str14 = StringPrintf(text_format, localization->GetString(5),
                              UI_GetHealthManaAndOtherQualitiesStringColor(
                                  player->GetActualAge(), player->GetBaseAge()),
                              player->GetActualAge(), player->GetBaseAge());
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 266, pY, 0, str14);

    text_format = Stat_string_format_2_column_less_100;
    if (player->GetBaseLevel() > 99)
        text_format = Stat_string_format_2_column_over_100;

    pY += pFontArrus->GetHeight() - 2;
    auto str15 =
        StringPrintf(text_format, localization->GetString(131),  // Уров.
                     UI_GetHealthManaAndOtherQualitiesStringColor(
                         player->GetActualLevel(), player->GetBaseLevel()),
                     player->GetActualLevel(), player->GetBaseLevel());
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 266, pY, 0, str15);

    pY += pFontArrus->GetHeight() - 2;
    pText = localization->GetString(17);  // Exp.
    if (player->uExperience <= 9999999)
        pText = localization->GetString(83);  // Experience
    auto str16 =
        StringPrintf("%s\r180\f%05d%lu\f00000\n\n", pText,
                     player->GetExperienceDisplayColor(), player->uExperience);
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 266, pY, 0, str16);

    pY += 2 * pFontArrus->GetHeight();
    auto str17 = StringPrintf("%s\t100%+d\n", localization->GetString(18),
                              player->GetActualAttack(false));
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 266, pY, 0, str17);

    pY += pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(
        pFontArrus, 266, pY, 0,
        StringPrintf("%s\t100 %s\n", localization->GetString(53),
                     player->GetMeleeDamageString().c_str()));

    pY += pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(
        pFontArrus, 266, pY, 0,
        StringPrintf("%s\t100%+d\n", localization->GetString(203),
                     player->GetRangedAttack()));

    pY += pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(
        pFontArrus, 266, pY, 0,
        StringPrintf("%s\t100 %s\n\n", localization->GetString(53),
                     player->GetRangedDamageString().c_str()));

    text_format = Stat_string_format_2_column_less_100;
    if (player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_FIRE) > 99 ||
        player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_FIRE) > 99)
        text_format = Stat_string_format_2_column_over_100;
    pY += 2 * pFontArrus->GetHeight() - 4;
    auto str18 = StringPrintf(
        text_format, localization->GetSpellSchoolName(0),
        UI_GetHealthManaAndOtherQualitiesStringColor(
            player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_FIRE),
            player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_FIRE)),
        player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_FIRE),
        player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_FIRE));
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 266, pY, 0, str18);

    text_format = Stat_string_format_2_column_less_100;
    if (player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_AIR) > 99 ||
        player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_AIR) > 99)
        text_format = Stat_string_format_2_column_over_100;
    pY += pFontArrus->GetHeight() - 2;
    auto str19 = StringPrintf(
        text_format, localization->GetSpellSchoolName(1),
        UI_GetHealthManaAndOtherQualitiesStringColor(
            player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_AIR),
            player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_AIR)),
        player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_AIR),
        player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_AIR));
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 266, pY, 0, str19);

    text_format = Stat_string_format_2_column_less_100;
    if (player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_WATER) > 99 ||
        player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_WATER) > 99)
        text_format = Stat_string_format_2_column_over_100;
    pY += pFontArrus->GetHeight() - 2;
    auto str20 = StringPrintf(
        text_format, localization->GetSpellSchoolName(2),
        UI_GetHealthManaAndOtherQualitiesStringColor(
            player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_WATER),
            player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_WATER)),
        player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_WATER),
        player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_WATER));
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 266, pY, 0, str20);

    text_format = Stat_string_format_2_column_less_100;
    if (player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_EARTH) > 99)
        text_format = Stat_string_format_2_column_over_100;
    pY += pFontArrus->GetHeight() - 2;
    auto str21 = StringPrintf(
        text_format, localization->GetSpellSchoolName(3),
        UI_GetHealthManaAndOtherQualitiesStringColor(
            player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_EARTH),
            player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_EARTH)),
        player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_EARTH),
        player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_EARTH));
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 266, pY, 0, str21);

    text_format = Stat_string_format_2_column_less_100;
    if (player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_MIND) > 99 ||
        player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_MIND) > 99)
        text_format = Stat_string_format_2_column_over_100;
    pY += pFontArrus->GetHeight() - 2;
    auto str22 = StringPrintf(
        text_format, localization->GetSpellSchoolName(4),
        UI_GetHealthManaAndOtherQualitiesStringColor(
            player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_MIND),
            player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_MIND)),
        player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_MIND),
        player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_MIND));
    if (player->classType == PLAYER_CLASS_LICH &&
        player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_MIND) == 200) {
        str22 = StringPrintf(
            Stat_string_format_2_column_text, localization->GetString(142),
            UI_GetHealthManaAndOtherQualitiesStringColor(
                player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_MIND),
                200),
            localization->GetString(625));
    }
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 266, pY, 0, str22);

    text_format = Stat_string_format_2_column_less_100;
    if (player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_BODY) > 99 ||
        player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_BODY) > 99)
        text_format = Stat_string_format_2_column_over_100;
    pY += pFontArrus->GetHeight() - 2;
    int it = player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_BODY);
    auto str23 = StringPrintf(
        text_format, localization->GetSpellSchoolName(6),
        UI_GetHealthManaAndOtherQualitiesStringColor(
            player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_BODY),
            player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_BODY)),
        player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_BODY),
        player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_BODY));
    if (player->classType == PLAYER_CLASS_LICH &&
        player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_BODY) == 200) {
        str23 = StringPrintf(
            Stat_string_format_2_column_text, localization->GetString(29),
            UI_GetHealthManaAndOtherQualitiesStringColor(
                player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_BODY),
                200),
            localization->GetString(625));
    }
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, 266, pY, 0, str23);
}

bool awardSort(int i, int j) {
    if (pAwards[i].uPriority == 0)  // none
        return false;
    else if (pAwards[j].uPriority == 0)
        return true;
    else if (pAwards[i].uPriority == 1)  // fines,arena stuff,etc
        return false;
    else if (pAwards[j].uPriority == 1)
        return true;
    else if (pAwards[i].uPriority == 5)  // joined guilds
        return false;
    else if (pAwards[j].uPriority == 5)
        return true;
    else
        return (pAwards[i].uPriority < pAwards[j].uPriority);
}

//----- (00419100) --------------------------------------------------------
void FillAwardsData() {
    Player *pPlayer = pPlayers[uActiveCharacter];

    memset(achieved_awards.data(), 0, 4000);
    num_achieved_awards = 0;

    BtnDown_flag = 0;
    BtnUp_flag = 0;
    books_page_number = 0;
    books_primary_item_per_page = 0;
    for (int i = 1; i < 105; ++i) {
        if (_449B57_test_bit(pPlayer->_achieved_awards_bits, i) &&
            pAwards[i].pText)
            achieved_awards[num_achieved_awards++] = (AwardType)i;
    }
    full_num_items_in_book = num_achieved_awards;
    num_achieved_awards = 0;

    // sort awards index

    if (full_num_items_in_book > 0) {
        for (int i = 0; i < full_num_items_in_book; ++i)
            achieved_awards[full_num_items_in_book + i] =
                (AwardType)(rand() % 16);  //случайные значения от 0 до 15
        for (int i = 1; i < full_num_items_in_book; ++i) {
            for (int j = i; j < full_num_items_in_book; ++j) {
                AwardType tmp;
                if (pAwards[achieved_awards[j]].uPriority <
                    pAwards[achieved_awards[i]].uPriority) {
                    tmp = achieved_awards[j];
                    achieved_awards[j] = achieved_awards[i];
                    achieved_awards[i] = tmp;
                }
            }
        }
    }

    //  if (full_num_items_in_book > 0)
    /* {
        std::stable_sort(achieved_awards.begin(), achieved_awards.end(),
    awardSort);
    }*/
}

//----- (0043EF2B) --------------------------------------------------------
void WetsuitOn(unsigned int uPlayerID) {
    CHARACTER_RACE player_race;  // edi@2
    signed int player_sex;       // eax@2
    int texture_num;             // ecx@5
    char pContainer[20];         // [sp+4h] [bp-1Ch]@7

    if (uPlayerID > 0) {
        player_race = pPlayers[uPlayerID]->GetRace();
        player_sex = pPlayers[uPlayerID]->GetSexByVoice();
        if (player_race == CHARACTER_RACE_DWARF)
            texture_num = (player_sex != 0) + 3;
        else
            texture_num = (player_sex != 0) + 1;

        sprintf(pContainer, "pc23v%dBod", texture_num);
        papredoll_dbods[uPlayerID - 1] = assets->GetImage_Alpha(pContainer);
        sprintf(pContainer, "pc23v%dlad", texture_num);
        papredoll_dlads[uPlayerID - 1] = assets->GetImage_Alpha(pContainer);
        sprintf(pContainer, "pc23v%dlau", texture_num);
        papredoll_dlaus[uPlayerID - 1] = assets->GetImage_Alpha(pContainer);
        sprintf(pContainer, "pc23v%drh", texture_num);
        papredoll_drhs[uPlayerID - 1] = assets->GetImage_Alpha(pContainer);
        sprintf(pContainer, "pc23v%dlh", texture_num);
        papredoll_dlhs[uPlayerID - 1] = assets->GetImage_Alpha(pContainer);
        sprintf(pContainer, "pc23v%dlhu", texture_num);
        papredoll_dlhus[uPlayerID - 1] = assets->GetImage_Alpha(pContainer);

        if (pPlayers[uPlayerID]->uCurrentFace == 12 ||
            pPlayers[uPlayerID]->uCurrentFace == 13)
            paperdoll_dbrds[pPlayers[uPlayerID]->uCurrentFace] = nullptr;
        papredoll_flying_feet[pPlayers[uPlayerID]->uCurrentFace] = nullptr;

        IsPlayerWearingWatersuit[uPlayerID] = true;
    }
}

//----- (0043F0BD) --------------------------------------------------------
void WetsuitOff(unsigned int uPlayerID) {
    if (uPlayerID > 0) {
        papredoll_dbods[uPlayerID - 1] = assets->GetImage_Alpha(
            dbod_texnames_by_face[pPlayers[uPlayerID]->uCurrentFace]);
        papredoll_dlads[uPlayerID - 1] = assets->GetImage_Alpha(
            dlad_texnames_by_face[pPlayers[uPlayerID]->uCurrentFace]);
        papredoll_dlaus[uPlayerID - 1] = assets->GetImage_Alpha(
            dlau_texnames_by_face[pPlayers[uPlayerID]->uCurrentFace]);
        papredoll_drhs[uPlayerID - 1] = assets->GetImage_Alpha(
            drh_texnames_by_face[pPlayers[uPlayerID]->uCurrentFace]);
        papredoll_dlhs[uPlayerID - 1] = assets->GetImage_Alpha(
            dlh_texnames_by_face[pPlayers[uPlayerID]->uCurrentFace]);
        papredoll_dlhus[uPlayerID - 1] = assets->GetImage_Alpha(
            dlhu_texnames_by_face[pPlayers[uPlayerID]->uCurrentFace]);

        // wchar_t name[1024];
        if (pPlayers[uPlayerID]->uCurrentFace == 12 ||
            pPlayers[uPlayerID]->uCurrentFace == 13) {
            paperdoll_dbrds[pPlayers[uPlayerID]->uCurrentFace] =
                assets->GetImage_Alpha(StringPrintf(
                    "pc%02dbrd", pPlayers[uPlayerID]->uCurrentFace + 1));
        }

        papredoll_flying_feet[pPlayers[uPlayerID]->uCurrentFace] =
            assets->GetImage_Alpha(StringPrintf(
                "item281pc%02d", pPlayers[uPlayerID]->uCurrentFace + 1));

        IsPlayerWearingWatersuit[uPlayerID] = false;
    }
}

//----- (00468F8A) --------------------------------------------------------
void OnPaperdollLeftClick() {
    int mousex = pMouse->uMouseClickX;
    int mousey = pMouse->uMouseClickY;

    static int RingsX[6] = {0x1EA, 0x21A, 0x248, 0x1EA, 0x21A, 0x248};
    static int RingsY[6] = {0x0CA, 0x0CA, 0x0CA, 0x0FA, 0x0FA, 0x0FA};

    static int glovex = 586;
    static int glovey = 88;

    static int amuletx = 493;
    static int amulety = 91;

    int slot = 32;
    int pos = -1;

    ItemGen *pitem = NULL;  // condesnse with this??
                            // pitem.Reset();

    unsigned int pSkillType;  // esi@5
    // unsigned __int16 v5; // ax@7
    // int equippos; // esi@27
    // int v8; // eax@29
    int v17;  // eax@44

    int v23;  // eax@62
    int v26;  // eax@69
    int v34;  // esi@90

    //  unsigned int v48; // [sp+30h] [bp-1Ch]@88
    unsigned int v50;  // [sp+38h] [bp-14h]@50
    // int v51; // [sp+3Ch] [bp-10h]@1
    int freeslot;  // [sp+40h] [bp-Ch]@5
    ITEM_EQUIP_TYPE pEquipType = EQUIP_NONE;
    CastSpellInfo *pSpellInfo;

    int twohandedequip = 0;
    ItemGen _this;  // [sp+Ch] [bp-40h]@1
    _this.Reset();
    int mainhandequip = pPlayers[uActiveCharacter]->pEquipment.uMainHand;
    unsigned int shieldequip = pPlayers[uActiveCharacter]->pEquipment.uShield;

    if (mainhandequip && pPlayers[uActiveCharacter]
                                 ->pInventoryItemList[mainhandequip - 1]
                                 .GetItemEquipType() == EQUIP_TWO_HANDED)
        twohandedequip = mainhandequip;

    unsigned int pickeditem = pParty->pPickedItem.uItemID;

    if (pParty->pPickedItem.uItemID) {  // hold item
        pEquipType = pParty->pPickedItem.GetItemEquipType();
        pSkillType = pParty->pPickedItem.GetPlayerSkillType();

        if (pSkillType == 4) {  // PLAYER_SKILL_SPEAR
            if (shieldequip) {
                if (pPlayers[uActiveCharacter]->GetActualSkillMastery(
                        PLAYER_SKILL_SPEAR) <
                    3) {  // cant use spear in one hand till master
                    pPlayers[uActiveCharacter]->PlaySound(SPEECH_39, 0);

                    return;
                }

                pickeditem = pParty->pPickedItem.uItemID;
            }
        } else {
            if ((pSkillType == 8 || pSkillType == 1 ||
                 pSkillType == 2)  // shield  sword or dagger to place
                && mainhandequip &&
                pPlayers[uActiveCharacter]
                        ->pInventoryItemList[mainhandequip - 1]
                        .GetPlayerSkillType() == 4) {  // spear in mainhand
                if (pPlayers[uActiveCharacter]->GetActualSkillMastery(
                        PLAYER_SKILL_SPEAR) <
                    3) {  // cant use spear in one hand till master
                    pPlayers[uActiveCharacter]->PlaySound(SPEECH_39, 0);
                    return;
                }
            }
        }

        if (!pPlayers[uActiveCharacter]->CanEquip_RaceAndAlignmentCheck(
                pickeditem)) {  // special item checks
            pPlayers[uActiveCharacter]->PlaySound(SPEECH_39, 0);
            return;
        }

        if (pParty->pPickedItem.uItemID ==
            ITEM_WETSUIT) {  // wetsuit check is done above
            pPlayers[uActiveCharacter]->EquipBody((ITEM_EQUIP_TYPE)3);
            WetsuitOn(uActiveCharacter);
            return;
        }

        switch (pEquipType) {
            case EQUIP_BOW:
            case EQUIP_ARMOUR:
            case EQUIP_HELMET:
            case EQUIP_BELT:
            case EQUIP_CLOAK:
            case EQUIP_GAUNTLETS:
            case EQUIP_BOOTS:
            case EQUIP_AMULET:

                if (!pPlayers[uActiveCharacter]->HasSkill(
                        pSkillType)) {  // hasnt got the skill to use that
                    pPlayers[uActiveCharacter]->PlaySound(SPEECH_39, 0);
                    return;
                }

                if (pPlayers[uActiveCharacter]->HasUnderwaterSuitEquipped() &&
                    (pEquipType != EQUIP_ARMOUR || engine->IsUnderwater())) {  // cant put anything on wearing wetsuit
                    pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
                    return;
                }

                pPlayers[uActiveCharacter]->EquipBody(
                    pEquipType);  // equips item

                if (pParty->pPickedItem.uItemID ==
                    ITEM_WETSUIT)  // just taken wetsuit off
                    WetsuitOff(uActiveCharacter);

                return;

                // ------------------------dress rings(одевание
                // колец)----------------------------------
            case EQUIP_RING:

                if (pPlayers[uActiveCharacter]
                        ->HasUnderwaterSuitEquipped()) {  // cant put anything
                                                          // on wearing wetsuit
                    pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
                    return;
                }

                if (!bRingsShownInCharScreen) {  // rings not displayd
                    //слоты для колец
                    // equippos = 0;

                    for (int equippos = 0; equippos < 6; ++equippos) {
                        if (!pPlayers[uActiveCharacter]
                                 ->pEquipment.uRings[equippos]) {  // 0 to 5
                            freeslot = pPlayers[uActiveCharacter]
                                           ->FindFreeInventoryListSlot();
                            if (freeslot >= 0) {  // drop ring into free space
                                pParty->pPickedItem.uBodyAnchor = equippos + 11;
                                memcpy(
                                    &pPlayers[uActiveCharacter]
                                         ->pInventoryItemList[freeslot],
                                    &pParty->pPickedItem,
                                    sizeof(pPlayers[uActiveCharacter]
                                               ->pInventoryItemList[freeslot]));
                                pPlayers[uActiveCharacter]
                                    ->pEquipment.uRings[equippos] =
                                    freeslot + 1;
                                pMouse->RemoveHoldingItem();
                                return;
                            }
                        }
                    }

                    // cant fit rings so swap out
                    freeslot =
                        pPlayers[uActiveCharacter]->pEquipment.uRings[5] -
                        1;  // slot of last ring
                    memcpy(&_this, &pParty->pPickedItem,
                           sizeof(_this));  // copy hold item to this
                    pPlayers[uActiveCharacter]
                        ->pInventoryItemList[freeslot]
                        .uBodyAnchor = 0;
                    pParty->pPickedItem.Reset();  // drop holding item
                    pParty->SetHoldingItem(
                        &pPlayers[uActiveCharacter]
                             ->pInventoryItemList[freeslot]);  // set holding
                                                               // item to ring
                                                               // to swap out
                    _this.uBodyAnchor = 16;
                    memcpy(&pPlayers[uActiveCharacter]
                                ->pInventoryItemList[freeslot],
                           &_this, 0x24u);  // swap from this in
                    pPlayers[uActiveCharacter]->pEquipment.uRings[5] =
                        freeslot + 1;  // anchor
                    return;

                } else {  // rings displayed
                    // if in ring area
                    for (int i = 0; i < 6; ++i) {
                        if (mousex >= RingsX[i] &&
                            mousex <= (RingsX[i] + slot) &&
                            mousey >= RingsY[i] &&
                            mousey <= (RingsY[i] +
                                       slot)) {  // check against ring slots
                            pos = i;
                        }
                    }

                    if (pos != -1) {  // we have a position to aim for
                        pitem =
                            pPlayers[uActiveCharacter]->GetNthEquippedIndexItem(
                                pos + 10);
                        if (!pitem) {  // no item in slot so just drop
                            freeslot = pPlayers[uActiveCharacter]
                                           ->FindFreeInventoryListSlot();
                            if (freeslot >= 0) {  // drop ring into free space
                                pParty->pPickedItem.uBodyAnchor = pos + 11;
                                memcpy(
                                    &pPlayers[uActiveCharacter]
                                         ->pInventoryItemList[freeslot],
                                    &pParty->pPickedItem,
                                    sizeof(pPlayers[uActiveCharacter]
                                               ->pInventoryItemList[freeslot]));
                                pPlayers[uActiveCharacter]
                                    ->pEquipment.uRings[pos] = freeslot + 1;
                                pMouse->RemoveHoldingItem();
                                return;
                            }
                        } else {  // item so swap out
                            freeslot = pPlayers[uActiveCharacter]
                                           ->pEquipment.uRings[pos] -
                                       1;  // slot of ring selected
                            memcpy(&_this, &pParty->pPickedItem,
                                   sizeof(_this));  // copy hold item to this
                            pPlayers[uActiveCharacter]
                                ->pInventoryItemList[freeslot]
                                .uBodyAnchor = 0;
                            pParty->pPickedItem.Reset();  // drop holding item
                            pParty->SetHoldingItem(
                                &pPlayers[uActiveCharacter]->pInventoryItemList
                                     [freeslot]);  // set holding item to ring
                                                   // to swap out
                            _this.uBodyAnchor = pos + 11;
                            memcpy(&pPlayers[uActiveCharacter]
                                        ->pInventoryItemList[freeslot],
                                   &_this, 0x24u);  // swap from this in
                            pPlayers[uActiveCharacter]->pEquipment.uRings[pos] =
                                freeslot + 1;  // anchor
                            return;
                        }

                    } else {  // not click on right area so exit
                        return;
                    }

                    return;  // shouldnt get here but in case??
                }

                // ------------------dress shield(одеть
                // щит)------------------------------------------------------
            case EQUIP_SHIELD:  //Щит
                if (pPlayers[uActiveCharacter]
                        ->HasUnderwaterSuitEquipped()) {  // в акваланге
                    pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
                    return;
                }
                if (!pPlayers[uActiveCharacter]->HasSkill(
                        pSkillType)) {  // нет навыка
                    pPlayers[uActiveCharacter]->PlaySound(SPEECH_39, 0);
                    return;
                }
                if (shieldequip) {  // смена щита щитом
                    --shieldequip;
                    memcpy(&_this, &pParty->pPickedItem, sizeof(_this));
                    pPlayers[uActiveCharacter]
                        ->pInventoryItemList[shieldequip]
                        .uBodyAnchor = 0;
                    pParty->pPickedItem.Reset();
                    pParty->SetHoldingItem(
                        &pPlayers[uActiveCharacter]
                             ->pInventoryItemList[shieldequip]);
                    _this.uBodyAnchor = 1;
                    memcpy(&pPlayers[uActiveCharacter]
                                ->pInventoryItemList[shieldequip],
                           &_this, 0x24u);
                    pPlayers[uActiveCharacter]->pEquipment.uShield =
                        shieldequip + 1;
                    if (twohandedequip == EQUIP_SINGLE_HANDED) return;
                } else {
                    freeslot =
                        pPlayers[uActiveCharacter]->FindFreeInventoryListSlot();
                    if (freeslot < 0) return;
                    if (!twohandedequip) {  // обычная установка щита на пустую
                                            // руку
                        pParty->pPickedItem.uBodyAnchor = 1;
                        v17 = freeslot + 1;
                        memcpy(&pPlayers[uActiveCharacter]
                                    ->pInventoryItemList[freeslot],
                               &pParty->pPickedItem,
                               sizeof(pPlayers[uActiveCharacter]
                                          ->pInventoryItemList[freeslot]));
                        pPlayers[uActiveCharacter]->pEquipment.uShield = v17;
                        pMouse->RemoveHoldingItem();
                        return;
                    }
                    mainhandequip--;  //ставим щит когда держит двуручный меч
                    memcpy(&_this, &pParty->pPickedItem, sizeof(_this));
                    pPlayers[uActiveCharacter]
                        ->pInventoryItemList[mainhandequip]
                        .uBodyAnchor = 0;
                    pParty->pPickedItem.Reset();
                    pParty->SetHoldingItem(
                        &pPlayers[uActiveCharacter]
                             ->pInventoryItemList[mainhandequip]);
                    _this.uBodyAnchor = 1;
                    memcpy(&pPlayers[uActiveCharacter]
                                ->pInventoryItemList[freeslot],
                           &_this,
                           sizeof(pPlayers[uActiveCharacter]
                                      ->pInventoryItemList[freeslot]));
                    pPlayers[uActiveCharacter]->pEquipment.uShield =
                        freeslot + 1;
                }
                pPlayers[uActiveCharacter]->pEquipment.uMainHand = 0;
                return;
                // -------------------------taken in hand(взять в
                // руку)-------------------------------------------
            case EQUIP_SINGLE_HANDED:
            case EQUIP_WAND:
                if (pPlayers[uActiveCharacter]->HasUnderwaterSuitEquipped() &&
                    pParty->pPickedItem.uItemID != 64 &&
                    pParty->pPickedItem.uItemID != 65) {
                    pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
                    return;
                }
                if (!pPlayers[uActiveCharacter]->HasSkill(pSkillType)) {
                    pPlayers[uActiveCharacter]->PlaySound(SPEECH_39, 0);
                    return;
                }
                v50 = 0;

                if (pSkillType == 2 &&
                        (pPlayers[uActiveCharacter]->GetActualSkillMastery(
                             PLAYER_SKILL_DAGGER) >
                         1)  // dagger in left hand at expert
                    || pSkillType == 1 &&
                           (pPlayers[uActiveCharacter]->GetActualSkillMastery(
                                PLAYER_SKILL_SWORD) >
                            2)) {  // sword in left hand at master
                    // v18 = pMouse->uMouseClickX;
                    // v19 = pMouse->uMouseClickY;
                    if ((signed int)pMouse->uMouseClickX >= 560) {
                        if (!twohandedequip) {
                            if (shieldequip) {
                                --shieldequip;
                                memcpy(&_this, &pParty->pPickedItem,
                                       sizeof(_this));
                                pPlayers[uActiveCharacter]
                                    ->pInventoryItemList[shieldequip]
                                    .uBodyAnchor = 0;
                                pParty->pPickedItem.Reset();
                                pParty->SetHoldingItem(
                                    &pPlayers[uActiveCharacter]
                                         ->pInventoryItemList[shieldequip]);
                                _this.uBodyAnchor = 1;
                                memcpy(&pPlayers[uActiveCharacter]
                                            ->pInventoryItemList[shieldequip],
                                       &_this, 0x24u);
                                pPlayers[uActiveCharacter]->pEquipment.uShield =
                                    shieldequip + 1;
                                if (pEquipType != EQUIP_WAND) return;
                                v50 = _this.uItemID;
                                break;
                            }
                            v23 = pPlayers[uActiveCharacter]
                                      ->FindFreeInventoryListSlot();
                            if (v23 < 0) return;
                            pParty->pPickedItem.uBodyAnchor = 1;
                            memcpy(&pPlayers[uActiveCharacter]
                                        ->pInventoryItemList[v23],
                                   &pParty->pPickedItem,
                                   sizeof(pPlayers[uActiveCharacter]
                                              ->pInventoryItemList[v23]));
                            pPlayers[uActiveCharacter]->pEquipment.uShield =
                                v23 + 1;
                            pMouse->RemoveHoldingItem();
                            if (pEquipType != EQUIP_WAND) return;
                            v50 = pPlayers[uActiveCharacter]->pInventoryItemList[v23].uItemID;
                            break;
                        }
                    }
                }
                if (!mainhandequip) {
                    v26 =
                        pPlayers[uActiveCharacter]->FindFreeInventoryListSlot();
                    if (v26 < 0) return;
                    pParty->pPickedItem.uBodyAnchor = 2;
                    memcpy(&pPlayers[uActiveCharacter]->pInventoryItemList[v26],
                           &pParty->pPickedItem,
                           sizeof(pPlayers[uActiveCharacter]
                                      ->pInventoryItemList[v26]));
                    pPlayers[uActiveCharacter]->pEquipment.uMainHand = v26 + 1;
                    pMouse->RemoveHoldingItem();
                    if (pEquipType != EQUIP_WAND) return;
                    break;
                }
                --mainhandequip;
                memcpy(&_this, &pParty->pPickedItem, sizeof(_this));
                pPlayers[uActiveCharacter]
                    ->pInventoryItemList[mainhandequip]
                    .uBodyAnchor = 0;
                pParty->pPickedItem.Reset();
                pParty->SetHoldingItem(
                    &pPlayers[uActiveCharacter]
                         ->pInventoryItemList[mainhandequip]);
                _this.uBodyAnchor = 2;
                memcpy(&pPlayers[uActiveCharacter]
                            ->pInventoryItemList[mainhandequip],
                       &_this, 0x24);
                pPlayers[uActiveCharacter]->pEquipment.uMainHand =
                    mainhandequip + 1;
                if (pEquipType == EQUIP_WAND) v50 = _this.uItemID;
                if (twohandedequip)
                    pPlayers[uActiveCharacter]->pEquipment.uShield = 0;
                break;
                // ---------------------------take two hands(взять двумя
                // руками)---------------------------------
            case EQUIP_TWO_HANDED:
                if (pPlayers[uActiveCharacter]->HasUnderwaterSuitEquipped()) {
                    pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
                    return;
                }
                if (!pPlayers[uActiveCharacter]->HasSkill(pSkillType)) {
                    pPlayers[uActiveCharacter]->PlaySound(SPEECH_39, 0);
                    return;
                }
                if (mainhandequip) {  // взять двуручный меч когда нет
                                      // щита(замещение оружия)
                    if (shieldequip) {
                        pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
                        return;
                    }
                    --mainhandequip;
                    memcpy(&_this, &pParty->pPickedItem, sizeof(_this));
                    pPlayers[uActiveCharacter]
                        ->pInventoryItemList[mainhandequip]
                        .uBodyAnchor = 0;
                    pParty->pPickedItem.Reset();
                    pParty->SetHoldingItem(
                        &pPlayers[uActiveCharacter]
                             ->pInventoryItemList[mainhandequip]);
                    _this.uBodyAnchor = 2;
                    memcpy(&pPlayers[uActiveCharacter]
                                ->pInventoryItemList[mainhandequip],
                           &_this, 0x24u);
                    pPlayers[uActiveCharacter]->pEquipment.uMainHand =
                        mainhandequip + 1;
                } else {
                    freeslot =
                        pPlayers[uActiveCharacter]->FindFreeInventoryListSlot();
                    if (freeslot >= 0) {
                        if (shieldequip) {  // взять двуручный меч когда есть
                                            // щит(замещение щитом)
                            shieldequip--;
                            memcpy(&_this, &pParty->pPickedItem, sizeof(_this));
                            pPlayers[uActiveCharacter]
                                ->pInventoryItemList[shieldequip]
                                .uBodyAnchor = 0;
                            pParty->pPickedItem.Reset();
                            pParty->SetHoldingItem(
                                &pPlayers[uActiveCharacter]
                                     ->pInventoryItemList[shieldequip]);
                            _this.uBodyAnchor = 2;
                            memcpy(&pPlayers[uActiveCharacter]
                                        ->pInventoryItemList[freeslot],
                                   &_this,
                                   sizeof(pPlayers[uActiveCharacter]
                                              ->pInventoryItemList[freeslot]));
                            pPlayers[uActiveCharacter]->pEquipment.uShield = 0;
                            pPlayers[uActiveCharacter]->pEquipment.uMainHand =
                                freeslot + 1;
                        } else {
                            pParty->pPickedItem.uBodyAnchor = 2;
                            memcpy(&pPlayers[uActiveCharacter]
                                        ->pInventoryItemList[freeslot],
                                   &pParty->pPickedItem,
                                   sizeof(pPlayers[uActiveCharacter]
                                              ->pInventoryItemList[freeslot]));
                            pPlayers[uActiveCharacter]->pEquipment.uMainHand =
                                freeslot + 1;
                            pMouse->RemoveHoldingItem();
                        }
                    }
                }
                return;
                //-------------------------------------------------------------------------------
            default:
                pPlayers[uActiveCharacter]->UseItem_DrinkPotion_etc(
                    uActiveCharacter, 0);  //выпить напиток и др.
                return;
        }
        return;
    }

    // no hold item

    // check if on rings screen - it doesnt use zbuffer
    if (bRingsShownInCharScreen) {
        // assume slot width 32 as per inventory
        if (mousex < 490 || mousex > 618) return;

        if (mousey < 88 || mousey > 282) return;

        if (mousex >= amuletx && mousex <= (amuletx + slot) &&
            mousey >= amulety && mousey <= (amulety + 2 * slot)) {
            // amulet
            // pitem = pPlayers[uActiveCharacter]->GetAmuletItem(); //9
            pos = 9;
        }

        if (mousex >= glovex && mousex <= (glovex + slot) && mousey >= glovey &&
            mousey <= (glovey + 2 * slot)) {
            // glove
            // pitem = pPlayers[uActiveCharacter]->GetGloveItem(); //7
            pos = 7;
        }

        for (int i = 0; i < 6; ++i) {
            if (mousex >= RingsX[i] && mousex <= (RingsX[i] + slot) &&
                mousey >= RingsY[i] && mousey <= (RingsY[i] + slot)) {
                // ring
                // pitem = pPlayers[uActiveCharacter]->GetNthRingItem(i); //10+i
                pos = 10 + i;
            }
        }

        if (pos != -1)
            pitem = pPlayers[uActiveCharacter]->GetNthEquippedIndexItem(pos);

        if (!pitem) return;
        // pPlayers[uActiveCharacter]->get

        // enchanting??
        if (_50C9A0_IsEnchantingInProgress) {  // наложить закл на экипировку
            /* *((char *)pGUIWindow_CastTargetedSpell->ptr_1C + 8) &=
             *0x7Fu;//CastSpellInfo
             *((short *)pGUIWindow_CastTargetedSpell->ptr_1C + 2) =
             *uActiveCharacter - 1;
             *((int *)pGUIWindow_CastTargetedSpell->ptr_1C + 3) = v36;
             *((short *)pGUIWindow_CastTargetedSpell->ptr_1C + 3) =
             *pEquipType;*/
            pSpellInfo = (CastSpellInfo *)pGUIWindow_CastTargetedSpell->ptr_1C;
            pSpellInfo->uFlags &= 0x7F;
            pSpellInfo->uPlayerID_2 = uActiveCharacter - 1;
            pSpellInfo->spell_target_pid =
                pPlayers[uActiveCharacter]->pEquipment.pIndices[pos];
            pSpellInfo->field_6 = pEquipType;

            ptr_50C9A4_ItemToEnchant = pitem;
            _50C9A0_IsEnchantingInProgress = 0;
            pMessageQueue_50CBD0->Flush();
            pMouse->SetCursorImage("MICON1");
            _50C9D4_AfterEnchClickEventSecondParam = 0;
            _50C9D0_AfterEnchClickEventId = 113;
            _50C9D8_AfterEnchClickEventTimeout = 256;
        } else {
            if (!ptr_50C9A4_ItemToEnchant) {  // снять вещь
                pParty->SetHoldingItem(pitem);
                pPlayers[uActiveCharacter]
                    ->pEquipment.pIndices[pitem->uBodyAnchor - 1] = 0;
                pitem->Reset();

                // pParty->SetHoldingItem(&pPlayers[uActiveCharacter]->pInventoryItemList[v34
                // - 1]);
                //  pPlayers[uActiveCharacter]->pEquipment.pIndices[pPlayers[uActiveCharacter]->pInventoryItemList[v34
                //  - 1].uBodyAnchor - 1] = 0;
                //  pPlayers[uActiveCharacter]->pInventoryItemList[v34 -
                //  1].Reset();

                // return
                // &this->pInventoryItemList[this->pEquipment.pIndices[index] -
                // 1];
            }
        }

        // for (uint i = 0; i < 6; ++i)
        //{
        // if (!player->pEquipment.uRings[i])
        //  continue;

        // CharacterUI_DrawItem(pPaperdollRingsX[i], pPaperdollRingsY[i],
        // &player->pInventoryItemList[player->pEquipment.uRings[i] - 1],
        //  player->pEquipment.uRings[i]);
        //}
        // if (player->pEquipment.uAmulet)
        // CharacterUI_DrawItem(493, 91, player->GetAmuletItem(),
        // player->pEquipment.uAmulet);
        // if (player->pEquipment.uGlove)
        // CharacterUI_DrawItem(586, 88, player->GetGloveItem(),
        // player->pEquipment.uGlove);

    } else {  // z picking as before
        v34 =
            render
                ->pActiveZBuffer[pMouse->uMouseClickX +
                                 pSRZBufferLineOffsets[pMouse->uMouseClickY]] &
            0xFFFF;
        if (v34) {
            // v36 = v34 - 1;
            // v38 = &pPlayers[uActiveCharacter]->pInventoryItemList[v34 - 1];
            pEquipType = pPlayers[uActiveCharacter]
                             ->pInventoryItemList[v34 - 1]
                             .GetItemEquipType();
            if (pPlayers[uActiveCharacter]->pInventoryItemList[v34 - 1].uItemID == ITEM_WETSUIT) {
                if (engine->IsUnderwater()) {
                    pAudioPlayer->PlaySound(SOUND_error, 0, 0, -1, 0, 0);
                    return;
                }
                WetsuitOff(uActiveCharacter);
            }

            if (_50C9A0_IsEnchantingInProgress) {  // наложить закл на экипировку
                /* *((char *)pGUIWindow_CastTargetedSpell->ptr_1C + 8) &=
                 *0x7Fu;//CastSpellInfo
                 *((short *)pGUIWindow_CastTargetedSpell->ptr_1C + 2) =
                 *uActiveCharacter - 1;
                 *((int *)pGUIWindow_CastTargetedSpell->ptr_1C + 3) = v36;
                 *((short *)pGUIWindow_CastTargetedSpell->ptr_1C + 3) =
                 *pEquipType;*/
                pSpellInfo =
                    (CastSpellInfo *)pGUIWindow_CastTargetedSpell->ptr_1C;
                pSpellInfo->uFlags &= 0x7F;
                pSpellInfo->uPlayerID_2 = uActiveCharacter - 1;
                pSpellInfo->spell_target_pid = v34 - 1;
                pSpellInfo->field_6 = pEquipType;

                ptr_50C9A4_ItemToEnchant =
                    &pPlayers[uActiveCharacter]->pInventoryItemList[v34 - 1];
                _50C9A0_IsEnchantingInProgress = 0;
                pMessageQueue_50CBD0->Flush();
                pMouse->SetCursorImage("MICON1");
                _50C9D4_AfterEnchClickEventSecondParam = 0;
                _50C9D0_AfterEnchClickEventId = 113;
                _50C9D8_AfterEnchClickEventTimeout = 256;
            } else {
                if (!ptr_50C9A4_ItemToEnchant) {  // снять вещь
                    pParty->SetHoldingItem(&pPlayers[uActiveCharacter]
                                                ->pInventoryItemList[v34 - 1]);
                    pPlayers[uActiveCharacter]
                        ->pEquipment.pIndices[pPlayers[uActiveCharacter]
                                                  ->pInventoryItemList[v34 - 1]
                                                  .uBodyAnchor -
                                              1] = 0;
                    pPlayers[uActiveCharacter]
                        ->pInventoryItemList[v34 - 1]
                        .Reset();
                }
            }
        } else {  // снять лук
            if (pPlayers[uActiveCharacter]->pEquipment.uBow) {
                _this = pPlayers[uActiveCharacter]->pInventoryItemList
                            [pPlayers[uActiveCharacter]->pEquipment.uBow - 1];
                pParty->SetHoldingItem(&_this);
                _this.Reset();
                pPlayers[uActiveCharacter]->pEquipment.uBow = 0;
            }
        }
    }
}

void CharacterUI_ReleaseButtons() {
    if (dword_507CC0_activ_ch) {
        dword_507CC0_activ_ch = 0;
        std::vector<GUIButton*> to_delete;
        for (GUIButton *pButton : pGUIWindow_CurrentMenu->vButtons) {
            if (pButton->field_1C & 0x8000) {
                to_delete.push_back(pButton);
            }
        }
        for (GUIButton *pButton : to_delete) {
            pButton->Release();
        }
        for (GUIButton *pButton : pGUIWindow_CurrentMenu->vButtons) {
            if (pButton->msg == UIMSG_InventoryLeftClick) {
                pButton->uX = dword_50698C_uX;
                pButton->uY = dword_506988_uY;
                pButton->uZ = dword_506984_uZ;
                pButton->uW = dword_506980_uW;
                pGUIWindow_CurrentMenu->_41D08F_set_keyboard_control_group(
                    1, 0, 0, 0);
            }
        }
    }
}

bool ringscreenactive() { return bRingsShownInCharScreen; }
