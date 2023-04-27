#include "GUI/UI/UICharacter.h"

#include <algorithm>
#include <vector>
#include <string>
#include <unordered_map>

#include "Engine/AssetsManager.h"
#include "Engine/Awards.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Objects/ItemTable.h"
#include "Engine/Party.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Time.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIInventory.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/MapAccess.h"
#include "Library/Random/Random.h"

void CharacterUI_LoadPaperdollTextures();

/**
 * Prepare textures of character doll with wetsuit on.
 *
 * @param uPlayerID     ID of player, 1-based.
 * @offset 0x43EF2B
 */
void WetsuitOn(unsigned int uPlayerID);

/**
 * Prepare textures of character doll with wetsuit off.
 *
 * @param uPlayerID     ID of player, 1-based.
 * @offset 0x43F0BD
 */
void WetsuitOff(unsigned int uPlayerID);

int bRingsShownInCharScreen;  // 5118E0

// TODO(pskelton): convert to color32

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
    ui_mainmenu_copyright_color = colorTable.White.c16();

    ui_character_tooltip_header_default_color = colorTable.PaleCanary.c16();
    ui_character_default_text_color = colorTable.White.c16();
    ui_character_header_text_color = colorTable.PaleCanary.c16();
    ui_character_bonus_text_color = colorTable.Green.c16();
    ui_character_bonus_text_color_neg = colorTable.Red.c16();

    ui_character_skill_upgradeable_color = color16(0, 175, 255);
    ui_character_skill_default_color = colorTable.Red.c16();
    ui_character_skill_highlight_color = colorTable.Red.c16();

    ui_character_stat_default_color = colorTable.White.c16();
    ui_character_stat_buffed_color = colorTable.Green.c16();
    ui_character_stat_debuffed_color = colorTable.Scarlet.c16();

    ui_character_skillinfo_can_learn = colorTable.White.c16();
    ui_character_skillinfo_can_learn_gm = colorTable.Yellow.c16();
    ui_character_skillinfo_cant_learn = colorTable.Red.c16();

    ui_character_condition_normal_color = colorTable.White.c16();
    ui_character_condition_light_color = colorTable.Green.c16();
    ui_character_condition_moderate_color = colorTable.Sunflower.c16();
    ui_character_condition_severe_color = colorTable.Scarlet.c16();

    ui_character_award_color[0] = colorTable.Magenta.c16();
    ui_character_award_color[1] = colorTable.Malibu.c16();
    ui_character_award_color[2] = colorTable.MoonRaker.c16();
    ui_character_award_color[3] = colorTable.ScreaminGreen.c16();
    ui_character_award_color[4] = colorTable.Canary.c16();
    ui_character_award_color[5] = colorTable.Mimosa.c16();

    ui_game_minimap_outline_color = colorTable.Blue.c16();
    ui_game_minimap_actor_friendly_color = colorTable.Green.c16();
    ui_game_minimap_actor_hostile_color = colorTable.Red.c16();
    ui_game_minimap_actor_corpse_color = colorTable.Yellow.c16();
    ui_game_minimap_decoration_color_1 = colorTable.White.c16();
    ui_game_minimap_projectile_color = colorTable.Red.c16();
    ui_game_minimap_treasure_color = colorTable.Blue.c16();

    ui_game_character_record_playerbuff_colors[0] = colorTable.Anakiwa.c16();
    ui_game_character_record_playerbuff_colors[1] = colorTable.Mercury.c16();
    ui_game_character_record_playerbuff_colors[2] = colorTable.FlushOrange.c16();
    ui_game_character_record_playerbuff_colors[3] = colorTable.Gray.c16();
    ui_game_character_record_playerbuff_colors[4] = colorTable.Mercury.c16();
    ui_game_character_record_playerbuff_colors[5] = colorTable.DarkOrange.c16();
    ui_game_character_record_playerbuff_colors[6] = colorTable.FlushOrange.c16();
    ui_game_character_record_playerbuff_colors[7] = colorTable.DarkOrange.c16();
    ui_game_character_record_playerbuff_colors[8] = colorTable.Mercury.c16();
    ui_game_character_record_playerbuff_colors[9] = colorTable.PurplePink.c16();
    ui_game_character_record_playerbuff_colors[10] = colorTable.MoonRaker.c16();
    ui_game_character_record_playerbuff_colors[11] = colorTable.Mercury.c16();
    ui_game_character_record_playerbuff_colors[12] = colorTable.FlushOrange.c16();
    ui_game_character_record_playerbuff_colors[13] = colorTable.Anakiwa.c16();
    ui_game_character_record_playerbuff_colors[14] = colorTable.Gray.c16();
    ui_game_character_record_playerbuff_colors[15] = colorTable.PaleCanary.c16();
    ui_game_character_record_playerbuff_colors[16] = colorTable.PaleCanary.c16();
    ui_game_character_record_playerbuff_colors[17] = colorTable.PaleCanary.c16();
    ui_game_character_record_playerbuff_colors[18] = colorTable.PaleCanary.c16();
    ui_game_character_record_playerbuff_colors[19] = colorTable.PaleCanary.c16();
    ui_game_character_record_playerbuff_colors[20] = colorTable.PaleCanary.c16();
    ui_game_character_record_playerbuff_colors[21] = colorTable.PaleCanary.c16();
    ui_game_character_record_playerbuff_colors[22] = colorTable.AzureRadiance.c16();
    ui_game_character_record_playerbuff_colors[23] = colorTable.AzureRadiance.c16();

    ui_gamemenu_video_gamma_title_color = colorTable.PaleCanary.c16();
    ui_gamemenu_keys_action_name_color = colorTable.White.c16();
    ui_gamemenu_keys_key_selection_blink_color_1 = colorTable.Temptress.c16();
    ui_gamemenu_keys_key_selection_blink_color_2 = colorTable.Sunflower.c16();
    ui_gamemenu_keys_key_default_color = colorTable.White.c16();

    ui_book_quests_title_color = colorTable.White.c16();
    ui_book_quests_text_color = colorTable.White.c16();
    ui_book_autonotes_title_color = colorTable.White.c16();
    ui_book_autonotes_text_color = colorTable.White.c16();
    ui_book_map_title_color = colorTable.White.c16();
    ui_book_map_coordinates_color = colorTable.White.c16();

    ui_book_calendar_title_color = colorTable.White.c16();
    ui_book_calendar_time_color = colorTable.Tundora.c16();
    ui_book_calendar_day_color = colorTable.Tundora.c16();
    ui_book_calendar_month_color = colorTable.Tundora.c16();
    ui_book_calendar_year_color = colorTable.Tundora.c16();
    ui_book_calendar_moon_color = colorTable.Tundora.c16();
    ui_book_calendar_location_color = colorTable.Tundora.c16();

    ui_book_journal_title_color = colorTable.White.c16();
    ui_book_journal_text_color = colorTable.White.c16();
    ui_book_journal_text_shadow = colorTable.Black.c16();

    ui_game_dialogue_npc_name_color = colorTable.EasternBlue.c16();
    ui_game_dialogue_option_highlight_color = colorTable.Sunflower.c16();
    ui_game_dialogue_option_normal_color = colorTable.White.c16();

    ui_house_player_cant_interact_color = colorTable.PaleCanary.c16();
}

Image *paperdoll_drhs[4];
Image *paperdoll_dlhus[4];
Image *paperdoll_dlhs[4];
Image *paperdoll_dbods[5];
Image *paperdoll_armor_texture[4][17][3];  // 0x511294
// int paperdoll_array_51132C[165];
Image *paperdoll_dlaus[5];
Image *paperdoll_dlads[4];
Image *paperdoll_flying_feet[22];      // 005115E0
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

// body complexion, item index, x/y position
const int paperdoll_Boot[4][7][2] = {  // 4E5490
    // human/goblin/elf male
    0x0E, 0x11D, 0x0D, 0x11D, 0x0C, 0x10A, 0x0A, 0xFF,  0x0D, 0xF9,
    0x0C, 0x10E, 0x0D, 0x137,

    // human/goblin/elf female
    0x14, 0x125, 0x13, 0x122, 0x15, 0x120, 0x15, 0x114, 0x13, 0x10A,
    0x11, 0x116, 0x11, 0x13E,

    // dwarf male
    0x1D, 0x121, 0x1C, 0x11F, 0x1B, 0x11B, 0x1C, 0x117, 0x16, 0x116,
    0x1B, 0x11B, 0x1B, 0x137,

    // dwarf female
    0x1F, 0x127, 0x1F, 0x122, 0x1B, 0x11B, 0x1D, 0x117, 0x1D, 0x116,
    0x1B, 0x11F, 0x1D, 0x137,
};
const std::unordered_map<ITEM_TYPE, int> paperdoll_boots_indexByType = {
    {ITEM_LEATHER_BOOTS, 0},
    {ITEM_STEEL_PLATED_BOOTS, 1},
    {ITEM_RANGER_BOOTS, 2},
    {ITEM_KNIGHTS_BOOTS, 3},
    {ITEM_PALADIN_BOOTS, 4},
    {ITEM_ARTIFACT_SEVEN_LEAGUE_BOOTS, 5},
    {ITEM_ARTIFACT_HERMES_SANDALS, 6},
};
const std::unordered_map<int, ITEM_TYPE> paperdoll_boots_typeByIndex = inverted(paperdoll_boots_indexByType);

const int paperdoll_Cloak[4][10][2] = {  // 4E5570
    0x11, 0x68, 0x0F, 0x68, 0x14, 0x71, 0x19, 0x6B, 0x21, 0x6F, 0x05, 0x68,
    0x05, 0x68, 0x14, 0x71, 0x03, 0x6B, 0x0F, 0x6F,

    0x15, 0x64, 0xB,  0x6B, 0x0E, 0x67, 0x15, 0x6B, 0x1B, 0x6F, 0x03, 0x6B,
    0,    0x6B, 0xE,  0x67, 0,    0x6B, 0x3,  0x6F,

    0x10, 0x8A, 0x9,  0x8B, 0x18, 0x98, 0x25, 0x91, 0x29, 0x90, 0x08, 0x8A,
    0x9,  0x8B, 0x18, 0x98, 0x3,  0x91, 0x3,  0x90,

    0x14, 0x92, 0x10, 0x92, 0x15, 0x98, 0x1F, 0x91, 0x22, 0x90, 0x08, 0x92,
    0x0C, 0x92, 0x15, 0x98, 0x03, 0x91, 0x03, 0x90,
};
const std::unordered_map<ITEM_TYPE, int> paperdoll_cloak_indexByType = {
    {ITEM_LEATHER_CLOAK, 0},
    {ITEM_HUNTSMANS_CLOAK, 1},
    {ITEM_RANGERS_CLOAK, 2},
    {ITEM_ELEGANT_CLOAK, 3},
    {ITEM_GLORIOUS_CLOAK, 4},
    {ITEM_RELIC_TWILIGHT, 5},
    {ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, 6},
    {ITEM_RARE_SUN_CLOAK, 7},
    {ITEM_RARE_MOON_CLOAK, 8},
    {ITEM_RARE_VAMPIRES_CAPE, 9}
};
const std::unordered_map<int, ITEM_TYPE> paperdoll_cloak_typeByIndex = inverted(paperdoll_cloak_indexByType);

const int paperdoll_CloakCollar[4][10][2] = {  // 4E56B0
    0,    0,    0x34, 0x64, 0x21, 0x69, 0x1D, 0x67, 0x20, 0x67, 0x21, 0x68,
    0x34, 0x64, 0x21, 0x69, 0x1D, 0x67, 0x1F, 0x67,

    0,    0,    0x35, 0x66, 0x29, 0x68, 0x1F, 0x68, 0x1F, 0x6A, 0x21, 0x6A,
    0x2B, 0x66, 0x26, 0x68, 0x1F, 0x68, 0x1F, 0x6A,

    0,    0,    0x30, 0x87, 0x1E, 0x86, 0x1B, 0x86, 0x1C, 0x8A, 0x21, 0x87,
    0x30, 0x87, 0x1E, 0x86, 0x1B, 0x86, 0x1C, 0x8A,

    0,    0,    0x38, 0x8A, 0x24, 0x8B, 0x1D, 0x8B, 0x21, 0x8C, 0x27, 0x8A,
    0x34, 0x8A, 0x24, 0x8B, 0x25, 0x8B, 0x21, 0x8C,
};
// int dword_4E56B4;
const int paperdoll_Belt[4][7][2] = {  // 4E57F0
    0x3A, 0xB6, 0x37, 0xB2, 0x34, 0xB9, 0x3A, 0xB9, 0x37, 0xB7, 0x38, 0xAC,
    0x37, 0xB7,

    0x3E, 0xAD, 0x3A, 0xAC, 0x37, 0xB0, 0x3A, 0xB1, 0x39, 0xB0, 0x3C, 0xA5,
    0x39, 0xB0,

    0x3B, 0xD5, 0x37, 0xD2, 0x31, 0xD5, 0x39, 0xD6, 0x37, 0xD8, 0x37, 0xD1,
    0x37, 0xD8,

    0x42, 0xD2, 0x3F, 0xD0, 0x3B, 0xD7, 0x3C, 0xD5, 0x3B, 0xD6, 0x3E, 0xCF,
    0x36, 0xD6,
};
const std::unordered_map<ITEM_TYPE, int> paperdoll_belt_indexByType = {
    {ITEM_LEATHER_BELT, 0},
    {ITEM_FINE_BELT, 1},
    {ITEM_STRONG_BELT, 2},
    {ITEM_SILVER_BELT, 3},
    {ITEM_GILDED_BELT, 4},
    {ITEM_RELIC_TITANS_BELT, 5},
    {ITEM_ARTIFACT_HEROS_BELT, 6}
};
const std::unordered_map<int, ITEM_TYPE> paperdoll_belt_typeByIndex = inverted(paperdoll_belt_indexByType);

const int paperdoll_Helm[4][16][2] = {  // 4E58D0
    0x3E, 0x1F, 0x41, 0x2C, 0x37, 0x2F, 0x31, 0x32, 0x37, 0x2A, 0x39, 0x28,
    0x36, 0x34, 0x41, 0x38, 0x40, 0x31, 0x40, 0x21, 0x40, 0x31, 0x3C, 0x33,
    0x3D, 0x24, 0x3A, 0x1A, 0x37, 0x2A, 0x41, 0x48,

    0x41, 0x1E, 0x42, 0x2B, 0x37, 0x2F, 0x34, 0x30, 0x39, 0x29, 0x3A, 0x26,
    0x36, 0x34, 0x41, 0x37, 0x42, 0x32, 0x40, 0x21, 0x40, 0x31, 0x40, 0x2F,
    0x3E, 0x22, 0x3B, 0x1A, 0x39, 0x29, 0x42, 0x47,

    0x3F, 0x47, 0x41, 0x56, 0x37, 0x59, 0x32, 0x5E, 0x37, 0x58, 0x39, 0x54,
    0x34, 0x61, 0x40, 0x61, 0x41, 0x5D, 0x3E, 0x4F, 0x3E, 0x5B, 0x3D, 0x5B,
    0x3F, 0x4C, 0x3B, 0x45, 0x37, 0x58, 0x41, 0x74,

    0x45, 0x45, 0x46, 0x54, 0x3A, 0x55, 0x38, 0x58, 0x3C, 0x54, 0x3F, 0x52,
    0x39, 0x5B, 0x45, 0x5C, 0x47, 0x5C, 0x44, 0x4B, 0x44, 0x57, 0x43, 0x55,
    0x44, 0x4A, 0x3E, 0x45, 0x3C, 0x54, 0x47, 0x70,
};
const std::unordered_map<ITEM_TYPE, int> paperdoll_helm_indexByType = {
    {ITEM_HORNED_HELM, 0},
    {ITEM_CONSCRIPTS_HELM, 1},
    {ITEM_FULL_HELM, 2},
    {ITEM_PHYNAXIAN_HELM, 3},
    {ITEM_MOGRED_HELM, 4},
    {ITEM_PEASANT_HAT, 5},
    {ITEM_TRAVELLERS_HAT, 6},
    {ITEM_FANCY_HAT, 7},
    {ITEM_CROWN, 8},
    {ITEM_NOBLE_CROWN, 9},
    {ITEM_REGAL_CROWN, 10},
    {ITEM_RELIC_TALEDONS_HELM, 11},
    {ITEM_RELIC_SCHOLARS_CAP, 12},
    {ITEM_RELIC_PHYNAXIAN_CROWN, 13},
    {ITEM_ARTIFACT_MINDS_EYE, 14},
    {ITEM_RARE_SHADOWS_MASK, 15}
};
const std::unordered_map<int, ITEM_TYPE> paperdoll_helm_typeByIndex = inverted(paperdoll_helm_indexByType);

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
    0x2C, 0x67, 0x30, 0x69, 0x2D, 0x67, 0x2C, 0x64, 0x14, 0x66, 0x22, 0x67,
    0x20, 0x66, 0x25, 0x66, 0x12, 0x66, 0x0A, 0x66, 0x13, 0x64, 0x0E, 0x64,
    0x0A, 0x63, 0x14, 0x66, 0x0A, 0x63, 0x0A, 0x66, 0x25, 0x66,

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
const std::unordered_map<ITEM_TYPE, int> paperdoll_armor_indexByType = {
    {ITEM_LEATHER_ARMOR, 0},
    {ITEM_STUDDED_LEATHER, 1},
    {ITEM_OFFICERS_LEATHER, 2},
    {ITEM_REGNAN_LEATHER, 3},
    {ITEM_ROYAL_LEATHER, 4},
    {ITEM_CHAIN_MAIL, 5},
    {ITEM_STEEL_CHAIN_MAIL, 6},
    {ITEM_FINE_CHAIN_MAIL, 7},
    {ITEM_RESPLENDENT_CHAIN_MAIL, 8},
    {ITEM_GOLDEN_CHAIN_MAIL, 9},
    {ITEM_PLATE_ARMOR, 10},
    {ITEM_SPLENDID_PLATE_ARMOR, 11},
    {ITEM_NOBLE_PLATE_ARMOR, 12},
    {ITEM_RELIC_HARECKS_LEATHER, 13},
    {ITEM_ARTIFACT_YORUBA, 14},
    {ITEM_ARTIFACT_GOVERNORS_ARMOR, 15},
    {ITEM_ARTIFACT_ELVEN_CHAINMAIL, 16}
};
const std::unordered_map<int, ITEM_TYPE> paperdoll_armor_typeByIndex = inverted(paperdoll_armor_indexByType);

const int paperdoll_shoulder_coord[4][17][2] = {  // 4E5050
    0x64, 0x67, 0x61, 0x67, 0x65, 0x68, 0x6E, 0x74, 0x6C, 0x68, 0x61, 0x67,
    0x66, 0x68, 0x6C, 0x6A, 0x6E, 0x6D, 0x67, 0x69, 0x70, 0x67, 0x6E, 0x6D,
    0x6C, 0x6F, 0x6C, 0x68, 0x6C, 0x6F, 0x67, 0x69, 0x6C, 0x6A,

    0x60, 0x6B, 0x60, 0x6C, 0x60, 0x6B, 0x61, 0x6A, 0x60, 0x69, 0,    0,
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

    0,    0,    0,    0,    0,    0,    0x60, 0x6C, 0x5E, 0x69, 0,    0,
    0,    0,    0x5D, 0x6A, 0x5B, 0x6A, 0x5B, 0x6A, 0x59, 0x69, 0x56, 0x68,
    0x38, 0x6E, 0x5E, 0x69, 0x38, 0x6E, 0x5B, 0x6A, 0x5D, 0x6A,

    0,    0,    0,    0,    0,    0,    0x72, 0x8D, 0x62, 0x89, 0,    0,
    0,    0,    0x69, 0x8C, 0x5E, 0x8D, 0x61, 0x8D, 0x5F, 0x8D, 0x60, 0x8D,
    0x2E, 0x8C, 0x62, 0x89, 0x2E, 0x8C, 0x61, 0x8D, 0x69, 0x8C,

    0,    0,    0,    0,    0,    0,    0x72, 0x91, 0x67, 0x8F, 0,    0,
    0,    0,    0,    0,    0x64, 0x93, 0x65, 0x8C, 0x65, 0x91, 0x67, 0x91,
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
    unsigned int uActiveCharacter, CURRENT_SCREEN screen)
    : GUIWindow(WINDOW_CharacterRecord, {0, 0}, render->GetRenderDimensions(), uActiveCharacter) {
    pEventTimer->Pause();
    bRingsShownInCharScreen = false;
    CharacterUI_LoadPaperdollTextures();
    current_screen_type = screen;

    pCharacterScreen_StatsBtn = CreateButton({pViewport->uViewportTL_X + 12, pViewport->uViewportTL_Y + 308},
        {paperdoll_dbrds[9]->GetWidth(), paperdoll_dbrds[9]->GetHeight()}, 1, 0,
        UIMSG_ClickStatsBtn, 0, InputAction::Stats, localization->GetString(LSTR_STATS),
        {{paperdoll_dbrds[10], paperdoll_dbrds[9]}});
    pCharacterScreen_SkillsBtn = CreateButton({pViewport->uViewportTL_X + 102, pViewport->uViewportTL_Y + 308},
        {paperdoll_dbrds[7]->GetWidth(), paperdoll_dbrds[7]->GetHeight()}, 1, 0,
        UIMSG_ClickSkillsBtn, 0, InputAction::Skills, localization->GetString(LSTR_SKILLS),
        {{paperdoll_dbrds[8], paperdoll_dbrds[7]}});
    pCharacterScreen_InventoryBtn = CreateButton({pViewport->uViewportTL_X + 192, pViewport->uViewportTL_Y + 308},
        {paperdoll_dbrds[5]->GetWidth(), paperdoll_dbrds[5]->GetHeight()}, 1, 0,
        UIMSG_ClickInventoryBtn, 0, InputAction::Inventory,
        localization->GetString(LSTR_INVENTORY),
        {{paperdoll_dbrds[6], paperdoll_dbrds[5]}});
    pCharacterScreen_AwardsBtn = CreateButton({pViewport->uViewportTL_X + 282, pViewport->uViewportTL_Y + 308},
        {paperdoll_dbrds[3]->GetWidth(), paperdoll_dbrds[3]->GetHeight()}, 1, 0,
        UIMSG_ClickAwardsBtn, 0, InputAction::Awards, localization->GetString(LSTR_AWARDS),
        {{paperdoll_dbrds[4], paperdoll_dbrds[3]}});
    pCharacterScreen_ExitBtn = CreateButton({pViewport->uViewportTL_X + 371, pViewport->uViewportTL_Y + 308},
        {paperdoll_dbrds[1]->GetWidth(), paperdoll_dbrds[1]->GetHeight()}, 1, 0,
        UIMSG_ClickExitCharacterWindowBtn, 0, InputAction::Invalid,
        localization->GetString(LSTR_DIALOGUE_EXIT),
        {{paperdoll_dbrds[2], paperdoll_dbrds[1]}});
    CreateButton({0, 0}, {476, 345}, 1, 122, UIMSG_InventoryLeftClick, 0);
    pCharacterScreen_DetalizBtn = CreateButton({600, 300}, {30, 30}, 1, 0,
        UIMSG_ChangeDetaliz, 0, InputAction::Invalid, localization->GetString(LSTR_DETAIL_TOGGLE));
    pCharacterScreen_DollBtn = CreateButton({476, 0}, {164, 345}, 1, 0, UIMSG_ClickPaperdoll, 0);

    CreateButton({61, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 1, InputAction::SelectChar1);
    CreateButton({177, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 2, InputAction::SelectChar2);
    CreateButton({292, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 3, InputAction::SelectChar3);
    CreateButton({407, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 4, InputAction::SelectChar4);

    CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_CycleCharacters, 0, InputAction::CharCycle);
    FillAwardsData();

    ui_character_skills_background = assets->GetImage_ColorKey("fr_skill");
    ui_character_awards_background = assets->GetImage_ColorKey("fr_award");
    ui_character_stats_background = assets->GetImage_ColorKey("fr_stats");
    ui_character_inventory_background_strip = assets->GetImage_ColorKey("fr_strip");
}

void GUIWindow_CharacterRecord::Update() {
    auto player = &pParty->activeCharacter();

    render->ClearZBuffer();
    switch (current_character_screen_window) {
        case WINDOW_CharacterWindow_Stats: {
            CharacterUI_ReleaseButtons();
            ReleaseAwardsScrollBar();
            CharacterUI_StatsTab_Draw(player);
            render->DrawTextureNew(
                pCharacterScreen_StatsBtn->uX / 640.0f,
                pCharacterScreen_StatsBtn->uY / 480.0f,
                assets->GetImage_ColorKey("ib-cd1-d"));
            break;
        }
        case WINDOW_CharacterWindow_Skills: {
            if (dword_507CC0_activ_ch != pParty->activeCharacterIndex()) {
                CharacterUI_ReleaseButtons();
                CharacterUI_SkillsTab_CreateButtons();
            }
            ReleaseAwardsScrollBar();
            CharacterUI_SkillsTab_Draw(player);
            render->DrawTextureNew(
                pCharacterScreen_SkillsBtn->uX / 640.0f,
                pCharacterScreen_SkillsBtn->uY / 480.0f,
                assets->GetImage_ColorKey("ib-cd2-d"));
            break;
        }
        case WINDOW_CharacterWindow_Awards: {
            CharacterUI_ReleaseButtons();
            CreateAwardsScrollBar();
            CharacterUI_AwardsTab_Draw(player);
            render->DrawTextureNew(
                pCharacterScreen_AwardsBtn->uX / 640.0f,
                pCharacterScreen_AwardsBtn->uY / 480.0f,
                assets->GetImage_ColorKey("ib-cd4-d"));
            break;
        }
        case WINDOW_CharacterWindow_Inventory: {
            CharacterUI_ReleaseButtons();
            ReleaseAwardsScrollBar();
            CharacterUI_InventoryTab_Draw(player, false);
            render->DrawTextureNew(
                pCharacterScreen_InventoryBtn->uX / 640.0f,
                pCharacterScreen_InventoryBtn->uY / 480.0f,
                assets->GetImage_ColorKey("ib-cd3-d"));
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
    new OnButtonClick3(WINDOW_CharacterWindow_Stats,
        {pCharacterScreen_StatsBtn->uX, pCharacterScreen_StatsBtn->uY}, {0, 0}, pCharacterScreen_StatsBtn);
}

void GUIWindow_CharacterRecord::ShowSkillsTab() {
    current_character_screen_window = WINDOW_CharacterWindow_Skills;
    CharacterUI_ReleaseButtons();
    ReleaseAwardsScrollBar();
    CharacterUI_SkillsTab_CreateButtons();
    new OnButtonClick3(WINDOW_CharacterWindow_Skills,
        {pCharacterScreen_SkillsBtn->uX, pCharacterScreen_SkillsBtn->uY}, {0, 0}, pCharacterScreen_SkillsBtn);
}

void GUIWindow_CharacterRecord::ShowInventoryTab() {
    current_character_screen_window = WINDOW_CharacterWindow_Inventory;
    ReleaseAwardsScrollBar();
    CharacterUI_ReleaseButtons();
    new OnButtonClick3(WINDOW_CharacterWindow_Inventory,
        {pCharacterScreen_InventoryBtn->uX, pCharacterScreen_InventoryBtn->uY}, {0, 0}, pCharacterScreen_InventoryBtn);
}

void GUIWindow_CharacterRecord::ShowAwardsTab() {
    ReleaseAwardsScrollBar();
    CharacterUI_ReleaseButtons();
    CreateAwardsScrollBar();
    current_character_screen_window = WINDOW_CharacterWindow_Awards;
    new OnButtonClick3(WINDOW_CharacterWindow_Awards,
        {pCharacterScreen_AwardsBtn->uX, pCharacterScreen_AwardsBtn->uY}, {0, 0}, pCharacterScreen_AwardsBtn);
    FillAwardsData();
}

void GUIWindow_CharacterRecord::ToggleRingsOverlay() {
    int x, y, w, h;

    bRingsShownInCharScreen ^= 1;
    pCharacterScreen_DetalizBtn->Release();
    pCharacterScreen_DollBtn->Release();
    if (bRingsShownInCharScreen) {
        h = ui_character_inventory_paperdoll_rings_close->GetHeight();
        w = ui_character_inventory_paperdoll_rings_close->GetWidth();
        y = 445;
        x = 471;
    } else {
        h = 30;
        w = 30;
        y = 300;
        x = 600;
    }
    pCharacterScreen_DetalizBtn = pGUIWindow_CurrentMenu->CreateButton({x, y}, {w, h}, 1, 0,
        UIMSG_ChangeDetaliz, 0, InputAction::Invalid, localization->GetString(LSTR_DETAIL_TOGGLE));
    pCharacterScreen_DollBtn = pGUIWindow_CurrentMenu->CreateButton({476, 0}, {164, 345}, 1, 0, UIMSG_ClickPaperdoll, 0);
}

GUIWindow *CastSpellInfo::GetCastSpellInInventoryWindow() {
    pEventTimer->Pause();
    bRingsShownInCharScreen = 0;
    CharacterUI_LoadPaperdollTextures();
    current_screen_type = CURRENT_SCREEN::SCREEN_CASTING;
    GUIWindow *CS_inventory_window = new GUIWindow_Inventory_CastSpell({0, 0}, render->GetRenderDimensions(), this, "");
    pCharacterScreen_ExitBtn = CS_inventory_window->CreateButton({394, 318}, {75, 33}, 1, 0,
        UIMSG_ClickExitCharacterWindowBtn, 0, InputAction::Invalid, localization->GetString(LSTR_DIALOGUE_EXIT),
        {{paperdoll_dbrds[2], paperdoll_dbrds[1]}});
    CS_inventory_window->CreateButton({0, 0}, {0x1DCu, 0x159u}, 1, 122, UIMSG_InventoryLeftClick, 0);
    pCharacterScreen_DollBtn = CS_inventory_window->CreateButton({0x1DCu, 0}, {0xA4u, 0x159u}, 1, 0, UIMSG_ClickPaperdoll, 0);

    CS_inventory_window->CreateButton({61, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 1, InputAction::SelectChar1);
    CS_inventory_window->CreateButton({177, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 2, InputAction::SelectChar2);
    CS_inventory_window->CreateButton({292, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 3, InputAction::SelectChar3);
    CS_inventory_window->CreateButton({407, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 4, InputAction::SelectChar4);

    return CS_inventory_window;
}

static int CharacterUI_SkillsTab_Draw__DrawSkillTable(
    Player *player, int x, int y, const std::initializer_list<PLAYER_SKILL_TYPE> skill_list,
    int right_margin, const char *skill_group_name) {
    int y_offset = y;
    Pointi pt = mouse->GetCursorPos();

    auto str = fmt::format("{}\r{:03}{}", skill_group_name, right_margin, localization->GetString(LSTR_LEVEL));
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {x, y}, ui_character_header_text_color, str, 0, 0, 0);

    int num_skills_drawn = 0;
    for (PLAYER_SKILL_TYPE skill : skill_list) {
        for (size_t j = 0; j < pGUIWindow_CurrentMenu->vButtons.size(); ++j) {
            GUIButton *button = pGUIWindow_CurrentMenu->GetControl(j);
            if ((short)(button->uData) >= 0) {
                continue;  // skips an of the stats skills innv awards buttons
            }

            if (static_cast<PLAYER_SKILL_TYPE>(button->uData & 0x7FFF) != skill) {
                continue;  // skips buttons that dont match skill
            }

            ++num_skills_drawn;
            y_offset = button->uY;

            PLAYER_SKILL_LEVEL skill_level = player->GetSkillLevel(skill);

            uint skill_color = 0;
            uint skill_mastery_color = 0;
            if (player->uSkillPoints > skill_level && skills_max_level[skill] != 1) {
                skill_color = ui_character_skill_upgradeable_color;
            }

            if (pt.x >= button->uX && pt.x < button->uZ && pt.y >= button->uY && pt.y < button->uW) {
                if (player->uSkillPoints > skill_level && skills_max_level[skill] > skill_level && skills_max_level[skill] != 1) {
                    skill_mastery_color = ui_character_bonus_text_color;
                } else {
                    skill_mastery_color = ui_character_skill_default_color;
                }
                skill_color = skill_mastery_color;
            }

            PLAYER_SKILL_MASTERY skill_mastery = player->GetSkillMastery(skill);
            if (skill_mastery == PLAYER_SKILL_MASTERY_NOVICE) {
                std::string Strsk;
                if (skills_max_level[skill] == 1) { // Non-investable skill
                    Strsk = fmt::format("{}\r{:03}-", localization->GetSkillName(skill), right_margin);
                } else {
                    Strsk = fmt::format("{}\r{:03}{}", localization->GetSkillName(skill), right_margin, skill_level);
                }
                pGUIWindow_CurrentMenu->DrawText(pFontLucida, {x, button->uY}, skill_color, Strsk, 0, 0, 0);
            } else {
                const char *skill_level_str = skill_mastery == PLAYER_SKILL_MASTERY_NOVICE ? "" : localization->MasteryName(skill_mastery);

                if (!skill_mastery_color) {
                    skill_mastery_color = ui_character_header_text_color;
                }

                auto Strsk = fmt::format("{} \f{:05}{}\f{:05}\r{:03}{}",
                        localization->GetSkillName(skill), skill_mastery_color, skill_level_str, skill_color, right_margin, skill_level);
                pGUIWindow_CurrentMenu->DrawText(pFontLucida, {x, button->uY}, skill_color, Strsk, 0, 0, 0);
            }
        }
    }

    if (!num_skills_drawn) {
        y_offset += pFontLucida->GetHeight() - 3;
        pGUIWindow_CurrentMenu->DrawText(pFontLucida, {x, y_offset}, 0, localization->GetString(LSTR_NONE), 0, 0, 0);
    }

    return y_offset;
}

//----- (00419719) --------------------------------------------------------
void GUIWindow_CharacterRecord::CharacterUI_SkillsTab_Draw(Player *player) {
    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f,
                                ui_character_skills_background);

    auto str = fmt::format(
        "{} \f{:05}{}\f00000\r177{}: \f{:05}{}\f00000",  // ^Pv[]
                     localization->GetString(LSTR_SKILLS_FOR),
                     ui_character_header_text_color, player->name.c_str(),
                     localization->GetString(LSTR_SKILL_POINTS),
                     player->uSkillPoints ? ui_character_bonus_text_color
                                          : ui_character_default_text_color,
                     player->uSkillPoints);
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {24, 18}, 0, str, 0, 0, 0);

    int y = 2 * pFontLucida->GetHeight() + 13;
    y = CharacterUI_SkillsTab_Draw__DrawSkillTable(
        player, 24, y, WeaponSkills(), 400,
        localization->GetString(LSTR_WEAPONS));

    y += 2 * pFontLucida->GetHeight() - 10;
    CharacterUI_SkillsTab_Draw__DrawSkillTable(
        player, 24, y, MagicSkills(), 400,
        localization->GetString(LSTR_MAGIC));

    y = 2 * pFontLucida->GetHeight() + 13;
    y = CharacterUI_SkillsTab_Draw__DrawSkillTable(
        player, 248, y, ArmorSkills(), 177,
        localization->GetString(LSTR_ARMOR));

    y += 2 * pFontLucida->GetHeight() - 10;
    y = CharacterUI_SkillsTab_Draw__DrawSkillTable(
        player, 248, y, MiscSkills(), 177,
        localization->GetString(LSTR_MISC));
}

//----- (0041A000) --------------------------------------------------------
void GUIWindow_CharacterRecord::CharacterUI_AwardsTab_Draw(Player *player) {
    int items_per_page;       // eax@1
    // char Source[100];         // [sp+Ch] [bp-C4h]@1
    GUIWindow awards_window;  // [sp+70h] [bp-60h]@1

    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f,
                                ui_character_awards_background);

    std::string str = fmt::format(
        "{} \f{:05}{}\f00000",
        localization->GetString(LSTR_AWARDS_FOR), ui_character_header_text_color,
        NameAndTitle(player->name, player->classType));

    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {24, 18}, 0, str, 0, 0, 0);
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
        std::string str;
        const char *v6 = pAwards[achieved_awards[i]].pText;

        // TODO(captainurist): fmt can throw
        switch (achieved_awards[i]) {
            case Award_Arena_PageWins:
                str = fmt::sprintf(v6, pParty->uNumArenaWins[0]);
                break;
            case Award_Arena_SquireWins:
                str = fmt::sprintf(v6, pParty->uNumArenaWins[1]);
                break;
            case Award_Arena_KnightWins:
                str = fmt::sprintf(v6, pParty->uNumArenaWins[2]);
                break;
            case Award_Arena_LordWins:
                str = fmt::sprintf(v6, pParty->uNumArenaWins[3]);
                break;
            case Award_ArcomageWins:
                str = fmt::sprintf(v6, pParty->uNumArcomageWins);
                break;
            case Award_ArcomageLoses:
                str = fmt::sprintf(v6, pParty->uNumArcomageLoses);
                break;
            case Award_Deaths:
                str = fmt::sprintf(v6, pParty->uNumDeaths);
                break;
            case Award_BountiesCollected:
                str = fmt::sprintf(v6, pParty->uNumBountiesCollected);
                break;
            case Award_Fine:
                str = fmt::sprintf(v6, pParty->uFine);
                break;
            case Award_PrisonTerms:
                str = fmt::sprintf(v6, pParty->uNumPrisonTerms);
                break;
            default:
                break;
        }

        if (str.empty())
            str = std::string(v6);

        awards_window.DrawText(pFontArrus, {0, 0}, ui_character_award_color[pAwards[achieved_awards[i]].uPriority % 6], str, 0, 0, 0);
        awards_window.uFrameY = pFontArrus->CalcTextHeight(str, awards_window.uFrameWidth, 0) + awards_window.uFrameY + 8;
        if (awards_window.uFrameY > awards_window.uFrameHeight)
            break;

        ++num_achieved_awards;
    }
}

//----- (0041A2C1) --------------------------------------------------------
unsigned int GetSizeInInventorySlots(unsigned int uNumPixels) {
    if ((signed int)uNumPixels < 14)
        uNumPixels = 14;

    return ((signed int)(uNumPixels - 14) >> 5) + 1;
}

//----- (0041A556) --------------------------------------------------------
void draw_leather() {
    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, ui_leather_mm7);
}

//----- (0043CC7C) --------------------------------------------------------
void CharacterUI_DrawPaperdoll(Player *player) {
    int index;
    int item_X;
    int item_Y;
    ItemGen *item;

    int IsDwarf;
    int pBodyComplection;
    if (player->GetRace() == CHARACTER_RACE_DWARF) {
        IsDwarf = 1;
        pBodyComplection = player->GetSexByVoice() == SEX_MALE ? 2 : 3;
    } else {
        IsDwarf = 0;
        pBodyComplection = player->GetSexByVoice() == SEX_MALE ? 0 : 1;
    }

    int uPlayerID = pParty->getCharacterIdInParty(player);

    render->ResetUIClipRect();
    render->DrawTextureNew(467 / 640.0f, 0, ui_character_inventory_paperdoll_background);

    ItemGen *itemMainHand = player->GetMainHandItem();
    ItemGen *itemOffHand = player->GetOffHandItem();
    bool bTwoHandedGrip = itemMainHand && (itemMainHand->GetItemEquipType() == EQUIP_TWO_HANDED || itemMainHand->GetPlayerSkillType() == PLAYER_SKILL_SPEAR && !itemOffHand);

    // Aqua-Lung
    if (player->hasUnderwaterSuitEquipped()) {
        render->DrawTextureNew(pPaperdoll_BodyX / 640.0f, pPaperdoll_BodyY / 480.0f, paperdoll_dbods[uPlayerID]);
        if (!bRingsShownInCharScreen)
            render->ZDrawTextureAlpha(pPaperdoll_BodyX / 640.0f, pPaperdoll_BodyY / 480.0f, paperdoll_dbods[uPlayerID], player->pEquipment.uArmor);

        // hands aren't in two handed grip pose
        if (!bTwoHandedGrip) {
            item_X = pPaperdoll_BodyX + pPaperdoll_LeftHand[pBodyComplection][0];
            item_Y = pPaperdoll_BodyY + pPaperdoll_LeftHand[pBodyComplection][1];

            render->DrawTextureNew(item_X / 640.0f, item_Y / 480.0f, paperdoll_dlads[uPlayerID]);
        }

        // main hand's item
        item = itemMainHand;
        if (item) {
            item_X = pPaperdoll_BodyX + paperdoll_Weapon[pBodyComplection][1][0] - pItemTable->pItems[item->uItemID].uEquipX;
            item_Y = pPaperdoll_BodyY + paperdoll_Weapon[pBodyComplection][1][1] - pItemTable->pItems[item->uItemID].uEquipY;

            Texture *texture = nullptr;
            if (item->uItemID == ITEM_BLASTER)
                texture = assets->GetImage_Alpha("item64v1");

            CharacterUI_DrawItem(item_X, item_Y, item, player->pEquipment.uMainHand, texture, !bRingsShownInCharScreen);
        }
    } else {
        // bow
        item = player->GetBowItem();
        if (item) {
            item_X = pPaperdoll_BodyX + paperdoll_Weapon[pBodyComplection][2][0] - pItemTable->pItems[item->uItemID].uEquipX;
            item_Y = pPaperdoll_BodyY + paperdoll_Weapon[pBodyComplection][2][1] - pItemTable->pItems[item->uItemID].uEquipY;

            CharacterUI_DrawItem(item_X, item_Y, item, player->pEquipment.uBow, nullptr, !bRingsShownInCharScreen);
        }

        // cloak
        item = player->GetCloakItem();
        if (item) {
            index = valueOr(paperdoll_cloak_indexByType, item->uItemID, -1);
            if (index != -1) {
                item_X = pPaperdoll_BodyX + paperdoll_Cloak[pBodyComplection][index][0];
                item_Y = pPaperdoll_BodyY + paperdoll_Cloak[pBodyComplection][index][1];

                Texture *texture = (Texture *)paperdoll_cloak_texture[pBodyComplection][index];
                CharacterUI_DrawItem(item_X, item_Y, item, player->pEquipment.uCloak, texture, !bRingsShownInCharScreen);
            }
        }

        // paperdoll
        render->DrawTextureNew(pPaperdoll_BodyX / 640.0f, pPaperdoll_BodyY / 480.0f, paperdoll_dbods[uPlayerID]);

        // armor
        item = player->GetArmorItem();
        if (item) {
            index = valueOr(paperdoll_armor_indexByType, item->uItemID, -1);
            if (index != -1) {
                item_X = pPaperdoll_BodyX + paperdoll_Armor_Coord[pBodyComplection][index][0];
                item_Y = pPaperdoll_BodyY + paperdoll_Armor_Coord[pBodyComplection][index][1];

                Texture *texture = (Texture *)paperdoll_armor_texture[pBodyComplection][index][0];
                CharacterUI_DrawItem(item_X, item_Y, item, player->pEquipment.uArmor, texture, !bRingsShownInCharScreen);
            }
        }

        // boots
        item = player->GetBootItem();
        if (item) {
            index = valueOr(paperdoll_boots_indexByType, item->uItemID, -1);
            if (index != -1) {
                item_X = pPaperdoll_BodyX + paperdoll_Boot[pBodyComplection][index][0];
                item_Y = pPaperdoll_BodyY + paperdoll_Boot[pBodyComplection][index][1];

                Texture *texture = nullptr;
                if (item->uItemID == ITEM_ARTIFACT_HERMES_SANDALS) {
                    texture = (Texture *)paperdoll_flying_feet[player->uCurrentFace];
                } else {
                    texture = (Texture *)paperdoll_boots_texture[pBodyComplection][index];
                }

                CharacterUI_DrawItem(item_X, item_Y, item, player->pEquipment.uBoot, texture, !bRingsShownInCharScreen);
            }
        }

        // offhand depending on grip
        if (!bTwoHandedGrip) {
            item_X = pPaperdoll_BodyX + pPaperdoll_LeftHand[pBodyComplection][0];
            item_Y = pPaperdoll_BodyY + pPaperdoll_LeftHand[pBodyComplection][1];
            render->DrawTextureNew(item_X / 640.0f, item_Y / 480.0f, paperdoll_dlads[uPlayerID]);
        } else {
            item_X = pPaperdoll_BodyX + pPaperdoll_SecondLeftHand[pBodyComplection][0];
            item_Y = pPaperdoll_BodyY + pPaperdoll_SecondLeftHand[pBodyComplection][1];
            render->DrawTextureNew(item_X / 640.0f, item_Y / 480.0f, paperdoll_dlaus[uPlayerID]);
        }

        // belt
        item = player->GetBeltItem();
        if (item) {
            index = valueOr(paperdoll_belt_indexByType, item->uItemID, -1);
            if (index != -1) {
                item_X = pPaperdoll_BodyX + paperdoll_Belt[pBodyComplection][index][0];
                item_Y = pPaperdoll_BodyY + paperdoll_Belt[pBodyComplection][index][1];
                Texture *texture = nullptr;
                if (IsDwarf != 1 || index == 5)
                    texture = (Texture *)paperdoll_belt_texture[pBodyComplection][index];
                else
                    texture = (Texture *)paperdoll_belt_texture[pBodyComplection - 2][index];

                CharacterUI_DrawItem(item_X, item_Y, item, player->pEquipment.uBelt, texture, !bRingsShownInCharScreen);
            }
        }

        // armor's shoulders
        item = player->GetArmorItem();
        if (item) {
            index = valueOr(paperdoll_armor_indexByType, item->uItemID, -1);
            if (index != -1) {
                Texture *texture = nullptr;
                // Some armors doesn't have sleeves so use normal one for two-handed or none if it also unavailable
                if (bTwoHandedGrip && paperdoll_shoulder_second_coord[pBodyComplection][index][0]) {
                    item_X = pPaperdoll_BodyX + paperdoll_shoulder_second_coord[pBodyComplection][index][0];
                    item_Y = pPaperdoll_BodyY + paperdoll_shoulder_second_coord[pBodyComplection][index][1];

                    texture = (Texture *)paperdoll_armor_texture[pBodyComplection][index][2];
                } else if (paperdoll_shoulder_coord[pBodyComplection][index][0]) {
                    item_X = pPaperdoll_BodyX + paperdoll_shoulder_coord[pBodyComplection][index][0];
                    item_Y = pPaperdoll_BodyY + paperdoll_shoulder_coord[pBodyComplection][index][1];

                    texture = (Texture *)paperdoll_armor_texture[pBodyComplection][index][1];
                }

                if (texture)
                    CharacterUI_DrawItem(item_X, item_Y, item, player->pEquipment.uArmor, texture, !bRingsShownInCharScreen);
            }
        }

        // cloak's collar
        item = player->GetCloakItem();
        if (item) {
            index = valueOr(paperdoll_cloak_indexByType, item->uItemID, -1);
            if (index != -1) {
                // leather cloak has no collar
                if (paperdoll_CloakCollar[pBodyComplection][index][0]) {
                    item_X = pPaperdoll_BodyX + paperdoll_CloakCollar[pBodyComplection][index][0];
                    item_Y = pPaperdoll_BodyY + paperdoll_CloakCollar[pBodyComplection][index][1];

                    Texture *texture = (Texture *)paperdoll_cloak_collar_texture[pBodyComplection][index];
                    CharacterUI_DrawItem(item_X, item_Y, item, player->pEquipment.uCloak, texture, !bRingsShownInCharScreen);
                }
            }
        }

        // beard
        if ((player->uCurrentFace == 12 || player->uCurrentFace == 13) && paperdoll_dbrds[player->uCurrentFace]) {
            item_X = pPaperdoll_BodyX + pPaperdoll_Beards[2 * player->uCurrentFace - 24];
            item_Y = pPaperdoll_BodyY + pPaperdoll_Beards[2 * player->uCurrentFace - 23];

            render->DrawTextureNew(item_X / 640.0f, item_Y / 480.0f, paperdoll_dbrds[player->uCurrentFace]);
        }

        // helm
        item = player->GetHelmItem();
        if (item) {
            index = valueOr(paperdoll_helm_indexByType, item->uItemID, -1);
            if (index != -1) {
                item_X = pPaperdoll_BodyX + paperdoll_Helm[pBodyComplection][index][0];
                item_Y = pPaperdoll_BodyY + paperdoll_Helm[pBodyComplection][index][1];

                Texture *texture = nullptr;
                if (IsDwarf != 1 || item->uItemID != ITEM_PHYNAXIAN_HELM)
                    texture = (Texture *)paperdoll_helm_texture[player->GetSexByVoice()][index];
                else
                    texture = (Texture *)paperdoll_dbrds[11];

                CharacterUI_DrawItem(item_X, item_Y, item, player->pEquipment.uHelm, texture, !bRingsShownInCharScreen);
            }
        }

        // main hand's item
        item = itemMainHand;
        if (item) {
            item_X = pPaperdoll_BodyX + paperdoll_Weapon[pBodyComplection][1][0] - pItemTable->pItems[item->uItemID].uEquipX;
            item_Y = pPaperdoll_BodyY + paperdoll_Weapon[pBodyComplection][1][1] - pItemTable->pItems[item->uItemID].uEquipY;

            Texture *texture = nullptr;
            if (item->uItemID == ITEM_BLASTER)
                texture = assets->GetImage_Alpha("item64v1");

            CharacterUI_DrawItem(item_X, item_Y, item, player->pEquipment.uMainHand, texture, !bRingsShownInCharScreen);
        }

        // offhand's item
        item = itemOffHand;
        if (item) {
            item_X = pPaperdoll_BodyX + paperdoll_Weapon[pBodyComplection][0][0] - pItemTable->pItems[item->uItemID].uEquipX;
            item_Y = pPaperdoll_BodyY + paperdoll_Weapon[pBodyComplection][0][1] - pItemTable->pItems[item->uItemID].uEquipY;

            /*
             * MM6 artifacts.
             * These cases should never execute in MM7 as we have spell books in these positions.
             * Also MM6 doesn't have variable size paperdoll's so cordinates need to account pPaperdoll_BodyX/Y.
             */
            if (item->GetPlayerSkillType() == PLAYER_SKILL_DAGGER || item->GetPlayerSkillType() == PLAYER_SKILL_SWORD) {
                switch (item->uItemID) {
                    case ITEM_SPELLBOOK_TORCH_LIGHT: // Mordred
                        item_X = 596;
                        item_Y = 86;
                        __debugbreak();
                        break;
                    case ITEM_SPELLBOOK_FIRE_AURA: // Excalibur
                        item_X = 596;
                        item_Y = 28;
                        __debugbreak();
                        break;
                    case ITEM_SPELLBOOK_JUMP: // Hades
                        item_X = 595;
                        item_Y = 33;
                        __debugbreak();
                        break;
                    default:
                        break;
                }
            }

            CharacterUI_DrawItem(item_X, item_Y, item, player->pEquipment.uOffHand, nullptr, !bRingsShownInCharScreen);
        }
    }

    // mainhand's wrist
    {
        item_X = pPaperdoll_BodyX + pPaperdoll_RightHand[pBodyComplection][0];
        item_Y = pPaperdoll_BodyY + pPaperdoll_RightHand[pBodyComplection][1];

        render->DrawTextureNew(item_X / 640.0f, item_Y / 480.0f, paperdoll_drhs[uPlayerID]);
    }

    // offhand's wrist
    if (bTwoHandedGrip) {
        item_X = pPaperdoll_BodyX + pPaperdoll_SecondLeftHand[pBodyComplection][0];
        item_Y = pPaperdoll_BodyY + pPaperdoll_SecondLeftHand[pBodyComplection][1];

        render->DrawTextureNew(item_X / 640.0f, item_Y / 480.0f, paperdoll_dlhus[uPlayerID]);
    } else if (!itemOffHand || itemOffHand && !itemOffHand->isShield()) {
        item_X = pPaperdoll_BodyX + pPaperdollLeftEmptyHand[pBodyComplection][0];
        item_Y = pPaperdoll_BodyY + pPaperdollLeftEmptyHand[pBodyComplection][1];
        render->DrawTextureNew(item_X / 640.0f, item_Y / 480.0f, paperdoll_dlhs[uPlayerID]);
    }

    // magnifying glass
    if (!bRingsShownInCharScreen)
        render->DrawTextureNew(603 / 640.0f, 299 / 480.0f, ui_character_inventory_magnification_glass);

    render->DrawTextureNew(468 / 640.0f, 0, game_ui_right_panel_frame);
}

//----- (0041A2D1) --------------------------------------------------------
void CharacterUI_InventoryTab_Draw(Player *player, bool Cover_Strip) {
    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f,
        ui_character_inventory_background);
    if (Cover_Strip) {
        if (ui_character_inventory_background_strip == nullptr) {
            // strip doesnt load if you havent already look at
                        // inventorys
            ui_character_inventory_background_strip =
                assets->GetImage_ColorKey("fr_strip");
        }
        render->DrawTextureNew(8 / 640.0f, 305 / 480.0f,
            ui_character_inventory_background_strip);
    }

    for (uint i = 0; i < 126; ++i) {
        if (player->pInventoryMatrix[i] <= 0) continue;
        if (player->pInventoryItemList[player->pInventoryMatrix[i] - 1].uItemID == ITEM_NULL)
            continue;
        unsigned int uCellY = 32 * (i / 14) + 17;
        unsigned int uCellX = 32 * (i % 14) + 14;

        Image *pTexture = assets->GetImage_Alpha(player->pInventoryItemList[player->pInventoryMatrix[i] - 1].GetIconName());

        int width = pTexture->GetWidth();
        int height = pTexture->GetHeight();
        if (width < 14) width = 14;
        if (height < 14) height = 14;

        signed int X_offset = (((signed int)((width - 14) & 0xFFFFFFE0) + 32) - width) / 2;
        signed int Y_offset = (((signed int)((height - 14) & 0xFFFFFFE0) + 32) - height) / 2;
        CharacterUI_DrawItem(uCellX + X_offset, uCellY + Y_offset, &(player->pInventoryItemList[player->pInventoryMatrix[i] - 1]), Cover_Strip);
    }
}

static void CharacterUI_DrawItem(int x, int y, ItemGen *item, int id, Texture *item_texture, bool doZDraw) {
    if (!item_texture)
        item_texture = assets->GetImage_Alpha(item->GetIconName());

    if (item->ItemEnchanted()) { // enchant animation
        Image *enchantment_texture = nullptr;
        if (item->AuraEffectRed())
            enchantment_texture = assets->GetImage_ColorKey("sptext01");
        else if (item->AuraEffectBlue())
            enchantment_texture = assets->GetImage_ColorKey("sp28a");
        else if (item->AuraEffectGreen())
            enchantment_texture = assets->GetImage_ColorKey("sp30a");
        else if (item->AuraEffectPurple())
            enchantment_texture = assets->GetImage_ColorKey("sp91a");
        else
            __debugbreak();

        ItemEnchantmentTimer -= pEventTimer->uTimeElapsed;
        if (ItemEnchantmentTimer <= 0) {
            ItemEnchantmentTimer = 0;
            item->ResetEnchantAnimation();
            ptr_50C9A4_ItemToEnchant = nullptr;
        }

        // TODO(pskelton): check tickcount usage here
        render->BlendTextures(x, y, item_texture, enchantment_texture, platform->tickCount() / 10, 0, 255);
    } else if (item->IsBroken()) {
        render->DrawTransparentRedShade(x / 640.0f, y / 480.0f, item_texture);
    } else if (!item->IsIdentified() && (engine->config->gameplay.ShowUndentifiedItem.value() || id)) {
        render->DrawTransparentGreenShade(x / 640.0f, y / 480.0f, item_texture);
    } else {
        render->DrawTextureNew(x / 640.0f, y / 480.0f, item_texture);
    }

    if (doZDraw)
        render->ZDrawTextureAlpha(x / 640.0f, y / 480.0f, item_texture, id);
}

//----- (0043E825) --------------------------------------------------------
void CharacterUI_DrawPaperdollWithRingOverlay(Player *player) {
    CharacterUI_DrawPaperdoll(player);

    render->DrawTextureNew(
        473 / 640.0f, 0, ui_character_inventory_paperdoll_rings_background);
    render->DrawTextureNew(468 / 640.0f, 0, game_ui_right_panel_frame);
    render->DrawTextureNew(pCharacterScreen_DetalizBtn->uX / 640.0f,
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

    if (!ui_character_inventory_magnification_glass)
        ui_character_inventory_magnification_glass = assets->GetImage_Alpha("MAGNIF-B");

    // if ( !pParty->uAlignment || pParty->uAlignment == 1 || pParty->uAlignment == 2 )
    if (!ui_character_inventory_paperdoll_background)
        ui_character_inventory_paperdoll_background = assets->GetImage_ColorKey("BACKDOLL");

    ui_character_inventory_paperdoll_rings_background = assets->GetImage_Alpha("BACKHAND");

    ui_character_inventory_paperdoll_rings_close = ui_exit_cancel_button_background;
    for (int i = 0; i < pParty->pPlayers.size(); ++i) {
        if (pParty->pPlayers[i].hasUnderwaterSuitEquipped()) {
            WetsuitOn(i + 1);
        } else {
            WetsuitOff(i + 1);
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

    // TODO(captainurist): safe to delete?
#if 0
    for (uint i = 0; i < 54; ++i) {  // test equipment
        party_has_equipment[i] = 0;
        if (pParty->pPickedItem.uItemID != i + 66) {
            for (uint j = 0; j < 4; ++j) {
                if (pParty->pPlayers[j].hasItem(i + 66, 0))
                    party_has_equipment[i] = 1;
            }
        }
    }
#endif

    auto loadTexture = [&](const auto &map, int itemIndex, int bodyIndex, int shoulderIndex) {
        std::string name = GetItemTextureFilename(*valuePtr(map, itemIndex), bodyIndex + 1, shoulderIndex);
        return assets->GetImage_Alpha(name);
    };

    for (uint i = 0; i < 2; ++i) {
        for (uint j : {0, 1, 2, 3, 4, 6}) // Belt
            paperdoll_belt_texture[i][j] = loadTexture(paperdoll_belt_typeByIndex, j, i, 0);

        for (uint j = 0; j < 16; ++j)  // Helm
            paperdoll_helm_texture[i][j] = loadTexture(paperdoll_helm_typeByIndex, j, i, 0);

        if (IsDwarfPresentInParty(true))  // the phynaxian helm uses a slightly
                                          // different graphic for dwarves
            paperdoll_dbrds[11] = assets->GetImage_Alpha("item092v3");
    }
    // v43 = 0;
    for (uint i = 0; i < 4; ++i) {
        if (ShouldLoadTexturesForRaceAndGender(i)) {
            paperdoll_belt_texture[i][5] = loadTexture(paperdoll_belt_typeByIndex, 5, i, 0);  // Titans belt

            for (v32 = 0; v32 < 17; ++v32) {  // simple armor
                paperdoll_armor_texture[i][v32][0] = loadTexture(paperdoll_armor_typeByIndex, v32, i, 0);  // armor
                paperdoll_armor_texture[i][v32][1] = loadTexture(paperdoll_armor_typeByIndex, v32, i, 1);  // shoulder 1
                paperdoll_armor_texture[i][v32][2] = loadTexture(paperdoll_armor_typeByIndex, v32, i, 2);  // shoulder 2
            }

            for (v33 = 0; v33 < 6; ++v33)  // boots
                paperdoll_boots_texture[i][v33] = loadTexture(paperdoll_boots_typeByIndex, v33, i, 0);

            for (v38 = 0; v38 < 10; ++v38) {  // Cloak
                paperdoll_cloak_texture[i][v38] = loadTexture(paperdoll_cloak_typeByIndex, v38, i, 0);
                paperdoll_cloak_collar_texture[i][v38] = loadTexture(paperdoll_cloak_typeByIndex, v38, i, 1);
            }
        }
        // else
        //{
        // v26 = v43;
        //}
        // v43 = v26 + 40;
    }
}

void GUIWindow_CharacterRecord::CharacterUI_SkillsTab_CreateButtons() {
    PLAYER_SKILL_TYPE skill;

    int buttons_count = 0;
    if (dword_507CC0_activ_ch) CharacterUI_ReleaseButtons();
    dword_507CC0_activ_ch = pParty->activeCharacterIndex();
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
    Player *curr_player = &pParty->activeCharacter();

    int uCurrFontHeght = pFontLucida->GetHeight();
    int current_Y = 2 * uCurrFontHeght + 13;
    int width = 204;
    for (PLAYER_SKILL_TYPE skill : WeaponSkills()) {
        if (curr_player->GetSkillLevel(skill)) {
            current_Y += uCurrFontHeght - 3;
            ++buttons_count;
            ++first_rows;
            uint skill_id = std::to_underlying(skill);
            pGUIWindow_CurrentMenu->CreateButton({24, current_Y}, {width, uCurrFontHeght - 3}, 3, skill_id | 0x8000, UIMSG_SkillUp, skill_id);
        }
    }
    if (!first_rows) current_Y += uCurrFontHeght - 3;
    current_Y += 2 * uCurrFontHeght - 6;
    for (PLAYER_SKILL_TYPE skill : MagicSkills()) {
        if (curr_player->GetSkillLevel(skill)/*&& buttons_count < 15*/) {
            current_Y += uCurrFontHeght - 3;
            ++buttons_count;
            uint skill_id = std::to_underlying(skill);
            pGUIWindow_CurrentMenu->CreateButton({24, current_Y}, {width, uCurrFontHeght - 3}, 3, skill_id | 0x8000, UIMSG_SkillUp, skill_id);
        }
    }
    first_rows = 0;
    current_Y = 2 * uCurrFontHeght + 13;
    for (PLAYER_SKILL_TYPE skill : ArmorSkills()) {
        if (curr_player->GetSkillLevel(skill)) {
            current_Y += uCurrFontHeght - 3;
            ++buttons_count;
            ++first_rows;
            uint skill_id = std::to_underlying(skill);
            pGUIWindow_CurrentMenu->CreateButton({246, current_Y}, {width, uCurrFontHeght - 3}, 3, skill_id | 0x8000, UIMSG_SkillUp, skill_id);
        }
    }
    if (!first_rows) current_Y += uCurrFontHeght - 3;
    current_Y += 2 * uCurrFontHeght - 6;
    for (PLAYER_SKILL_TYPE skill : MiscSkills()) {
        if (curr_player->GetSkillLevel(skill)) {
            current_Y += uCurrFontHeght - 3;
            ++buttons_count;
            uint skill_id = std::to_underlying(skill);
            pGUIWindow_CurrentMenu->CreateButton({246, current_Y}, {width, uCurrFontHeght - 3}, 3, skill_id | 0x8000, UIMSG_SkillUp, skill_id);
        }
    }

    if (buttons_count) {
        pGUIWindow_CurrentMenu->_41D08F_set_keyboard_control_group(
            buttons_count, 1, 0, buttons_count);
    }
}

void GUIWindow_CharacterRecord::CharacterUI_StatsTab_Draw(Player *player) {
    const char *text_format;  // [sp+14h] [bp-Ch]@4

    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f,
                                ui_character_stats_background);

    auto str1 =
        fmt::format("\f{:05}{}\f00000\r180{}: \f{:05}{}\f00000\n\n\n",
                    ui_character_header_text_color,
                    NameAndTitle(player->name, player->classType),
                    localization->GetString(LSTR_SKILL_POINTS),
                    player->uSkillPoints ? ui_character_bonus_text_color : ui_character_default_text_color,
                    player->uSkillPoints);
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {26, 18}, 0, str1);

    // Left column
    auto formatLeftCol = [] (int lstr, int current, int max) {
        const char *str = localization->GetString(lstr);
        int color16 = UI_GetHealthManaAndOtherQualitiesStringColor(current, max);
        if (max < 1000) {
            return fmt::format("{}\f{:05}\r424{}\f00000 /\t185{}\n", str, color16, current, max);
        } else {
            return fmt::format("{}\f{:05}\r388{}\f00000 / {}\n", str, color16, current, max);
        }
    };

    int pY = 53;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {26, pY}, 0,
                                     formatLeftCol(LSTR_MIGHT, player->GetActualMight(), player->GetBaseStrength()));
    // TODO(captainurist): GetActualMight vs GetBaseStrength, we need consistent naming

    pY += pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {26, pY}, 0,
                                     formatLeftCol(LSTR_INTELLECT, player->GetActualIntelligence(), player->GetBaseIntelligence()));

    pY += pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {26, pY}, 0,
                                     formatLeftCol(LSTR_PERSONALITY, player->GetActualWillpower(), player->GetBaseWillpower()));
    // TODO(captainurist): LSTR_PERSONALITY vs GetActualWillpower, we need consistent naming

    pY += pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {26, pY}, 0,
                                     formatLeftCol(LSTR_ENDURANCE, player->GetActualEndurance(), player->GetBaseEndurance()));

    pY += pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {26, pY}, 0,
                                     formatLeftCol(LSTR_ACCURACY, player->GetActualAccuracy(), player->GetBaseAccuracy()));

    pY += pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {26, pY}, 0,
                                     formatLeftCol(LSTR_SPEED, player->GetActualSpeed(), player->GetBaseSpeed()));

    pY += pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {26, pY}, 0,
                                     formatLeftCol(LSTR_LUCK, player->GetActualLuck(), player->GetBaseLuck()));

    pY += 2 * pFontArrus->GetHeight() + 5;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {26, pY}, 0,
                                     formatLeftCol(LSTR_HIT_POINTS, player->health, player->GetMaxHealth()));

    pY += pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {26, pY}, 0,
                                     formatLeftCol(LSTR_SPELL_POINTS, player->mana, player->GetMaxMana()));

    pY += pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {26, pY}, 0,
                                     formatLeftCol(LSTR_ARMOR_CLASS, player->GetActualAC(), player->GetBaseAC()));

    pY += 2 * pFontArrus->GetHeight() - 2;
    auto str12 =
        fmt::format("{}: \f{:05}{}\n",
                     localization->GetString(LSTR_CONDITION),
                     GetConditionDrawColor(player->GetMajorConditionIdx()),
                     localization->GetCharacterConditionName(player->GetMajorConditionIdx()));
    pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, {26, pY}, 0, str12, 226, 0);

    pY += pFontArrus->GetHeight() + -1;
    const char *spellName = localization->GetString(LSTR_NONE);
    if (player->uQuickSpell != SPELL_NONE)
        spellName = pSpellStats->pInfos[player->uQuickSpell].pShortName;
    auto str13 = fmt::format("{}: {}", localization->GetString(LSTR_QUICK_SPELL), spellName);
    pGUIWindow_CurrentMenu->DrawTextInRect(pFontArrus, {26, pY}, 0, str13, 226, 0);

    // Right column
    auto formatRightCol = [] (int lstr, int current, int max, bool immune = false) {
        const char *str = localization->GetString(lstr);
        int color16 = UI_GetHealthManaAndOtherQualitiesStringColor(current, max);
        if (immune) {
            return fmt::format("{}\f{:05}\r180{}\n", str, color16, localization->GetString(LSTR_IMMUNE));
        } else {
            if (current < 100 && max < 100) {
                return fmt::format("{}\f{:05}\t110{}\f00000 / {}\n", str, color16, current, max);
            } else {
                return fmt::format("{}\f{:05}\r180{}\f00000 / {}\n", str, color16, current, max);
            }
        }
    };

    pY = 50;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {266, pY}, 0,
                                     formatRightCol(LSTR_AGE, player->GetActualAge(), player->GetBaseAge()));

    pY += pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {266, pY}, 0,
                                     formatRightCol(LSTR_LEVEL, player->GetActualLevel(), player->GetBaseLevel()));

    pY += pFontArrus->GetHeight() - 2;
    auto str16 =
        fmt::format("{}\r180\f{:05}{}\f00000\n\n",
                    localization->GetString(player->experience <= 9999999 ? LSTR_EXPERIENCE : LSTR_EXP),
                    player->GetExperienceDisplayColor(), player->experience);
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {266, pY}, 0, str16);

    pY += 2 * pFontArrus->GetHeight();
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {266, pY}, 0,
                                     fmt::format("{}\t100{:+}\n", localization->GetString(LSTR_ATTACK), player->GetActualAttack(false)));

    pY += pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {266, pY}, 0,
                                     fmt::format("{}\t100 {}\n", localization->GetString(LSTR_DAMAGE), player->GetMeleeDamageString()));

    pY += pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {266, pY}, 0,
                                     fmt::format("{}\t100{:+}\n", localization->GetString(LSTR_SHOOT), player->GetRangedAttack()));

    pY += pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {266, pY}, 0,
                                     fmt::format("{}\t100 {}\n\n", localization->GetString(LSTR_DAMAGE), player->GetRangedDamageString()));

    pY += 2 * pFontArrus->GetHeight() - 4;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {266, pY}, 0,
                                     formatRightCol(LSTR_FIRE, player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_FIRE),
                                                    player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_FIRE)));

    pY += pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {266, pY}, 0,
                                     formatRightCol(LSTR_AIR, player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_AIR),
                                                    player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_AIR)));

    pY += pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {266, pY}, 0,
                                     formatRightCol(LSTR_WATER, player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_WATER),
                                                    player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_WATER)));

    pY += pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {266, pY}, 0,
                                     formatRightCol(LSTR_EARTH, player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_EARTH),
                                                    player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_EARTH)));

    pY += pFontArrus->GetHeight() - 2;
    bool immuneToMind = player->classType == PLAYER_CLASS_LICH && player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_MIND) == 200;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {266, pY}, 0,
                                     formatRightCol(LSTR_MIND, player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_MIND),
                                                    player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_MIND), immuneToMind));

    pY += pFontArrus->GetHeight() - 2;
    bool immuneToBody = player->classType == PLAYER_CLASS_LICH && player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_BODY) == 200;
    pGUIWindow_CurrentMenu->DrawText(pFontArrus, {266, pY}, 0,
                                     formatRightCol(LSTR_BODY, player->GetActualResistance(CHARACTER_ATTRIBUTE_RESIST_BODY),
                                                    player->GetBaseResistance(CHARACTER_ATTRIBUTE_RESIST_BODY), immuneToBody));
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
    Player *pPlayer = &pParty->activeCharacter();

    memset(achieved_awards.data(), 0, 4000);
    num_achieved_awards = 0;

    BtnDown_flag = 0;
    BtnUp_flag = 0;
    books_page_number = 0;
    books_primary_item_per_page = 0;
    for (int i = 1; i < 105; ++i) {
        if (pPlayer->_achievedAwardsBits[i] && pAwards[i].pText) {
            achieved_awards[num_achieved_awards++] = (AwardType)i;
        }
    }
    full_num_items_in_book = num_achieved_awards;
    num_achieved_awards = 0;

    // sort awards index

    if (full_num_items_in_book > 0) {
        // TODO(captainurist): what was the code below doing?
        //for (int i = 0; i < full_num_items_in_book; ++i)
        //    achieved_awards[full_num_items_in_book + i] =
        //        (AwardType)Random(16);  //случайные значения от 0 до 15
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

void WetsuitOn(unsigned int uPlayerID) {
    if (uPlayerID > 0) {
        int playerId0 = uPlayerID - 1;
        Player *player = &pParty->pPlayers[playerId0];
        int texture_num;

        if (player->GetRace() == CHARACTER_RACE_DWARF) {
            texture_num = (player->GetSexByVoice() != 0) + 3;
        } else {
            texture_num = (player->GetSexByVoice() != 0) + 1;
        }
        paperdoll_dbods[playerId0] = assets->GetImage_Alpha(fmt::format("pc23v{}Bod", texture_num));  // Body texture
        paperdoll_dlads[playerId0] = assets->GetImage_Alpha(fmt::format("pc23v{}lad", texture_num));  // Left Hand
        paperdoll_dlaus[playerId0] = assets->GetImage_Alpha(fmt::format("pc23v{}lau", texture_num));  // Left Hand2
        paperdoll_drhs[playerId0] = assets->GetImage_Alpha(fmt::format("pc23v{}rh", texture_num));  // Right Hand
        paperdoll_dlhs[playerId0] = assets->GetImage_Alpha(fmt::format("pc23v{}lh", texture_num));  // Left Palm
        paperdoll_dlhus[playerId0] = assets->GetImage_Alpha(fmt::format("pc23v{}lhu", texture_num));  // Left Fist

        if (player->uCurrentFace == 12 || player->uCurrentFace == 13) {
            paperdoll_dbrds[player->uCurrentFace] = nullptr;
        }
        paperdoll_flying_feet[player->uCurrentFace] = nullptr;
    }
}

void WetsuitOff(unsigned int uPlayerID) {
    if (uPlayerID > 0) {
        int playerId0 = uPlayerID - 1;
        Player *player = &pParty->pPlayers[playerId0];

        paperdoll_dbods[playerId0] = assets->GetImage_Alpha(dbod_texnames_by_face[player->uCurrentFace]);
        paperdoll_dlads[playerId0] = assets->GetImage_Alpha(dlad_texnames_by_face[player->uCurrentFace]);
        paperdoll_dlaus[playerId0] = assets->GetImage_Alpha(dlau_texnames_by_face[player->uCurrentFace]);
        paperdoll_drhs[playerId0] = assets->GetImage_Alpha(drh_texnames_by_face[player->uCurrentFace]);
        paperdoll_dlhs[playerId0] = assets->GetImage_Alpha(dlh_texnames_by_face[player->uCurrentFace]);
        paperdoll_dlhus[playerId0] = assets->GetImage_Alpha(dlhu_texnames_by_face[player->uCurrentFace]);

        if (player->uCurrentFace == 12 || player->uCurrentFace == 13) {
            paperdoll_dbrds[player->uCurrentFace] = assets->GetImage_Alpha(fmt::format("pc{:02}brd", player->uCurrentFace + 1));
        }

        paperdoll_flying_feet[player->uCurrentFace] = assets->GetImage_Alpha(fmt::format("item281pc{:02}", player->uCurrentFace + 1));
    }
}

//----- (00468F8A) --------------------------------------------------------
void OnPaperdollLeftClick() {
    int mousex = mouse->uMouseX;
    int mousey = mouse->uMouseY;

    static int RingsX[6] = {0x1EA, 0x21A, 0x248, 0x1EA, 0x21A, 0x248};
    static int RingsY[6] = {0x0CA, 0x0CA, 0x0CA, 0x0FA, 0x0FA, 0x0FA};

    static int glovex = 586;
    static int glovey = 88;

    static int amuletx = 493;
    static int amulety = 91;

    int slot = 32;
    ITEM_SLOT pos = ITEM_SLOT_INVALID;

    ItemGen *pitem = NULL;  // condesnse with this??
                            // pitem.Reset();

    // uint16_t v5; // ax@7
    // int equippos; // esi@27
    // int v8; // eax@29
    int v17;  // eax@44
    PLAYER_SKILL_TYPE pSkillType = PLAYER_SKILL_INVALID;

    int v23;  // eax@62
    int v26;  // eax@69
    int v34;  // esi@90

    //  unsigned int v48; // [sp+30h] [bp-1Ch]@88
    ITEM_TYPE v50;  // [sp+38h] [bp-14h]@50
    // int v51; // [sp+3Ch] [bp-10h]@1
    int freeslot;  // [sp+40h] [bp-Ch]@5
    ITEM_EQUIP_TYPE pEquipType = EQUIP_NONE;
    CastSpellInfo *pSpellInfo;

    int twohandedequip = 0;
    ItemGen _this;  // [sp+Ch] [bp-40h]@1
    _this.Reset();
    int mainhandequip = pParty->activeCharacter().pEquipment.uMainHand;
    unsigned int shieldequip = pParty->activeCharacter().pEquipment.uOffHand;

    if (mainhandequip && pParty->activeCharacter().pInventoryItemList[mainhandequip - 1].GetItemEquipType() == EQUIP_TWO_HANDED) {
        twohandedequip = mainhandequip;
    }

    ITEM_TYPE pickeditem = pParty->pPickedItem.uItemID;

    if (pParty->pPickedItem.uItemID != ITEM_NULL) {  // hold item
        pEquipType = pParty->pPickedItem.GetItemEquipType();
        pSkillType = pParty->pPickedItem.GetPlayerSkillType();

        if (pSkillType == PLAYER_SKILL_SPEAR) {
            if (shieldequip) {
                // cant use spear in one hand till master
                if (pParty->activeCharacter().GetActualSkillMastery(PLAYER_SKILL_SPEAR) < PLAYER_SKILL_MASTERY_MASTER) {
                    pParty->activeCharacter().playReaction(SPEECH_CantEquip);

                    return;
                }

                pickeditem = pParty->pPickedItem.uItemID;
            }
        } else {
            if ((pSkillType == PLAYER_SKILL_SHIELD || pSkillType == PLAYER_SKILL_SWORD || pSkillType == PLAYER_SKILL_DAGGER) && mainhandequip &&
                pParty->activeCharacter().pInventoryItemList[mainhandequip - 1].GetPlayerSkillType() == PLAYER_SKILL_SPEAR) {
                // cant use spear in one hand till master
                if (pParty->activeCharacter().GetActualSkillMastery(PLAYER_SKILL_SPEAR) < PLAYER_SKILL_MASTERY_MASTER) {
                    pParty->activeCharacter().playReaction(SPEECH_CantEquip);
                    return;
                }
            }
        }

        if (!pParty->activeCharacter().CanEquip_RaceAndAlignmentCheck(pickeditem)) {  // special item checks
            pParty->activeCharacter().playReaction(SPEECH_CantEquip);
            return;
        }

        if (pParty->pPickedItem.uItemID == ITEM_QUEST_WETSUIT) {  // wetsuit check is done above
            pParty->activeCharacter().EquipBody(EQUIP_ARMOUR);
            WetsuitOn(pParty->activeCharacterIndex());
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

                if (!pParty->activeCharacter().HasSkill(pSkillType)) {  // hasnt got the skill to use that
                    pParty->activeCharacter().playReaction(SPEECH_CantEquip);
                    return;
                }

                if (pParty->activeCharacter().hasUnderwaterSuitEquipped() &&
                    (pEquipType != EQUIP_ARMOUR || engine->IsUnderwater())) {  // cant put anything on wearing wetsuit
                    pAudioPlayer->playUISound(SOUND_error);
                    return;
                }

                pParty->activeCharacter().EquipBody(pEquipType);  // equips item

                if (pParty->pPickedItem.uItemID == ITEM_QUEST_WETSUIT) // just taken wetsuit off
                    WetsuitOff(pParty->activeCharacterIndex());

                return;

                // ------------------------dress rings(одевание
                // колец)----------------------------------
            case EQUIP_RING:
                if (pParty->activeCharacter().hasUnderwaterSuitEquipped()) {  // cant put anything
                                                                                        // on wearing wetsuit
                    pAudioPlayer->playUISound(SOUND_error);
                    return;
                }

                if (!bRingsShownInCharScreen) {  // rings not displayd
                    //слоты для колец
                    // equippos = 0;

                    for (ITEM_SLOT equippos : RingSlots()) {
                        if (!pParty->activeCharacter().pEquipment.pIndices[equippos]) {
                            freeslot = pParty->activeCharacter().findFreeInventoryListSlot();
                            if (freeslot >= 0) {  // drop ring into free space
                                pParty->pPickedItem.uBodyAnchor = equippos;
                                pParty->activeCharacter().pInventoryItemList[freeslot] = pParty->pPickedItem;
                                pParty->activeCharacter().pEquipment.pIndices[equippos] = freeslot + 1;
                                mouse->RemoveHoldingItem();
                                return;
                            }
                        }
                    }

                    // cant fit rings so swap out
                    freeslot = pParty->activeCharacter().pEquipment.uRings[5] - 1;  // slot of last ring
                    _this = pParty->pPickedItem;  // copy hold item to this
                    pParty->activeCharacter().pInventoryItemList[freeslot].uBodyAnchor = ITEM_SLOT_INVALID;
                    pParty->pPickedItem.Reset();  // drop holding item
                    pParty->setHoldingItem(&pParty->activeCharacter().pInventoryItemList[freeslot]); // set holding item to ring to swap out
                    _this.uBodyAnchor = ITEM_SLOT_RING6;
                    pParty->activeCharacter().pInventoryItemList[freeslot] = _this;  // swap from this in
                    pParty->activeCharacter().pEquipment.uRings[5] = freeslot + 1;  // anchor
                    return;

                } else {  // rings displayed if in ring area
                    for (int i = 0; i < 6; ++i) {
                        if (mousex >= RingsX[i] &&
                            mousex <= (RingsX[i] + slot) &&
                            mousey >= RingsY[i] &&
                            mousey <= (RingsY[i] + slot)) {  // check against ring slots
                            pos = RingSlot(i);
                        }
                    }

                    if (pos != ITEM_SLOT_INVALID) {  // we have a position to aim for
                        pitem = pParty->activeCharacter().GetNthEquippedIndexItem(pos);
                        if (!pitem) {  // no item in slot so just drop
                            freeslot = pParty->activeCharacter().findFreeInventoryListSlot();
                            if (freeslot >= 0) {  // drop ring into free space
                                pParty->pPickedItem.uBodyAnchor = pos;
                                pParty->activeCharacter().pInventoryItemList[freeslot] = pParty->pPickedItem;
                                pParty->activeCharacter().pEquipment.pIndices[pos] = freeslot + 1;
                                mouse->RemoveHoldingItem();
                                return;
                            }
                        } else {  // item so swap out
                            freeslot = pParty->activeCharacter().pEquipment.pIndices[pos] - 1; // slot of ring selected
                            _this = pParty->pPickedItem;  // copy hold item to this
                            pParty->activeCharacter().pInventoryItemList[freeslot].uBodyAnchor = ITEM_SLOT_INVALID;
                            pParty->pPickedItem.Reset();  // drop holding item
                            pParty->setHoldingItem(&pParty->activeCharacter().pInventoryItemList[freeslot]); // set holding item to ring to swap out
                            _this.uBodyAnchor = pos;
                            pParty->activeCharacter().pInventoryItemList[freeslot] = _this;  // swap from this in
                            pParty->activeCharacter().pEquipment.pIndices[pos] = freeslot + 1;  // anchor
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
                if (pParty->activeCharacter().hasUnderwaterSuitEquipped()) {  // в акваланге
                    pAudioPlayer->playUISound(SOUND_error);
                    return;
                }
                if (!pParty->activeCharacter().HasSkill(pSkillType)) {  // нет навыка
                    pParty->activeCharacter().playReaction(SPEECH_CantEquip);
                    return;
                }
                if (shieldequip) {  // смена щита щитом
                    --shieldequip;
                    _this = pParty->pPickedItem;
                    pParty->activeCharacter().pInventoryItemList[shieldequip].uBodyAnchor = ITEM_SLOT_INVALID;
                    pParty->pPickedItem.Reset();
                    pParty->setHoldingItem(&pParty->activeCharacter().pInventoryItemList[shieldequip]);
                    _this.uBodyAnchor = ITEM_SLOT_OFF_HAND;
                    pParty->activeCharacter().pInventoryItemList[shieldequip] = _this;
                    pParty->activeCharacter().pEquipment.uOffHand = shieldequip + 1;
                    if (twohandedequip == 0) {
                        return;
                    }
                } else {
                    freeslot = pParty->activeCharacter().findFreeInventoryListSlot();
                    if (freeslot < 0) return;
                    if (!twohandedequip) {  // обычная установка щита на пустую
                                            // руку
                        pParty->pPickedItem.uBodyAnchor = ITEM_SLOT_OFF_HAND;
                        v17 = freeslot + 1;
                        pParty->activeCharacter().pInventoryItemList[freeslot] = pParty->pPickedItem;
                        pParty->activeCharacter().pEquipment.uOffHand = v17;
                        mouse->RemoveHoldingItem();
                        return;
                    }
                    mainhandequip--;  //ставим щит когда держит двуручный меч
                    _this = pParty->pPickedItem;
                    pParty->activeCharacter().pInventoryItemList[mainhandequip].uBodyAnchor = ITEM_SLOT_INVALID;
                    pParty->pPickedItem.Reset();
                    pParty->setHoldingItem(&pParty->activeCharacter().pInventoryItemList[mainhandequip]);
                    _this.uBodyAnchor = ITEM_SLOT_OFF_HAND;
                    pParty->activeCharacter().pInventoryItemList[freeslot] = _this;
                    pParty->activeCharacter().pEquipment.uOffHand = freeslot + 1;
                }
                pParty->activeCharacter().pEquipment.uMainHand = 0;
                return;
                // -------------------------taken in hand(взять в
                // руку)-------------------------------------------
            case EQUIP_SINGLE_HANDED:
            case EQUIP_WAND:
                if (pParty->activeCharacter().hasUnderwaterSuitEquipped() &&
                    pParty->pPickedItem.uItemID != ITEM_BLASTER &&
                    pParty->pPickedItem.uItemID != ITEM_BLASTER_RIFLE) {
                    pAudioPlayer->playUISound(SOUND_error);
                    return;
                }
                if (!pParty->activeCharacter().HasSkill(pSkillType)) {
                    pParty->activeCharacter().playReaction(SPEECH_CantEquip);
                    return;
                }
                v50 = ITEM_NULL;
                // dagger at expert or sword at master in left hand
                if (pSkillType == PLAYER_SKILL_DAGGER && (pParty->activeCharacter().GetActualSkillMastery(PLAYER_SKILL_DAGGER) >= PLAYER_SKILL_MASTERY_EXPERT)
                    || pSkillType == PLAYER_SKILL_SWORD && (pParty->activeCharacter().GetActualSkillMastery(PLAYER_SKILL_SWORD) >= PLAYER_SKILL_MASTERY_MASTER)) {
                    if ((signed int)mouse->uMouseX >= 560) {
                        if (!twohandedequip) {
                            if (shieldequip) {
                                --shieldequip;
                                _this = pParty->pPickedItem;
                                pParty->activeCharacter().pInventoryItemList[shieldequip].uBodyAnchor = ITEM_SLOT_INVALID;
                                pParty->pPickedItem.Reset();
                                pParty->setHoldingItem(&pParty->activeCharacter().pInventoryItemList[shieldequip]);
                                _this.uBodyAnchor = ITEM_SLOT_OFF_HAND;
                                pParty->activeCharacter().pInventoryItemList[shieldequip] = _this;
                                pParty->activeCharacter().pEquipment.uOffHand = shieldequip + 1;
                                if (pEquipType != EQUIP_WAND) {
                                    return;
                                }
                                v50 = _this.uItemID;
                                break;
                            }
                            v23 = pParty->activeCharacter().findFreeInventoryListSlot();
                            if (v23 < 0) return;
                            pParty->pPickedItem.uBodyAnchor = ITEM_SLOT_OFF_HAND;
                            pParty->activeCharacter().pInventoryItemList[v23] = pParty->pPickedItem;
                            pParty->activeCharacter().pEquipment.uOffHand = v23 + 1;
                            mouse->RemoveHoldingItem();
                            if (pEquipType != EQUIP_WAND) return;
                            v50 = pParty->activeCharacter().pInventoryItemList[v23].uItemID;
                            break;
                        }
                    }
                }
                if (!mainhandequip) {
                    v26 = pParty->activeCharacter().findFreeInventoryListSlot();
                    if (v26 < 0) return;
                    pParty->pPickedItem.uBodyAnchor = ITEM_SLOT_MAIN_HAND;
                    pParty->activeCharacter().pInventoryItemList[v26] = pParty->pPickedItem;
                    pParty->activeCharacter().pEquipment.uMainHand = v26 + 1;
                    mouse->RemoveHoldingItem();
                    if (pEquipType != EQUIP_WAND) return;
                    break;
                }
                --mainhandequip;
                _this = pParty->pPickedItem;
                pParty->activeCharacter().pInventoryItemList[mainhandequip].uBodyAnchor = ITEM_SLOT_INVALID;
                pParty->pPickedItem.Reset();
                pParty->setHoldingItem(&pParty->activeCharacter().pInventoryItemList[mainhandequip]);
                _this.uBodyAnchor = ITEM_SLOT_MAIN_HAND;
                pParty->activeCharacter().pInventoryItemList[mainhandequip] = _this;
                pParty->activeCharacter().pEquipment.uMainHand = mainhandequip + 1;
                if (pEquipType == EQUIP_WAND) v50 = _this.uItemID;
                if (twohandedequip) {
                    pParty->activeCharacter().pEquipment.uOffHand = 0;
                }
                break;
                // ---------------------------take two hands(взять двумя
                // руками)---------------------------------
            case EQUIP_TWO_HANDED:
                if (pParty->activeCharacter().hasUnderwaterSuitEquipped()) {
                    pAudioPlayer->playUISound(SOUND_error);
                    return;
                }
                if (!pParty->activeCharacter().HasSkill(pSkillType)) {
                    pParty->activeCharacter().playReaction(SPEECH_CantEquip);
                    return;
                }
                if (mainhandequip) {  // взять двуручный меч когда нет
                                      // щита(замещение оружия)
                    if (shieldequip) {
                        pAudioPlayer->playUISound(SOUND_error);
                        return;
                    }
                    --mainhandequip;
                    _this = pParty->pPickedItem;
                    pParty->activeCharacter().pInventoryItemList[mainhandequip].uBodyAnchor = ITEM_SLOT_INVALID;
                    pParty->pPickedItem.Reset();
                    pParty->setHoldingItem(&pParty->activeCharacter().pInventoryItemList[mainhandequip]);
                    _this.uBodyAnchor = ITEM_SLOT_MAIN_HAND;
                    pParty->activeCharacter().pInventoryItemList[mainhandequip] = _this;
                    pParty->activeCharacter().pEquipment.uMainHand = mainhandequip + 1;
                } else {
                    freeslot = pParty->activeCharacter().findFreeInventoryListSlot();
                    if (freeslot >= 0) {
                        if (shieldequip) {  // взять двуручный меч когда есть
                                            // щит(замещение щитом)
                            shieldequip--;
                            _this = pParty->pPickedItem;
                            pParty->activeCharacter().pInventoryItemList[shieldequip].uBodyAnchor = ITEM_SLOT_INVALID;
                            pParty->pPickedItem.Reset();
                            pParty->setHoldingItem(&pParty->activeCharacter().pInventoryItemList[shieldequip]);
                            _this.uBodyAnchor = ITEM_SLOT_MAIN_HAND;
                            pParty->activeCharacter().pInventoryItemList[freeslot] = _this;
                            pParty->activeCharacter().pEquipment.uOffHand = 0;
                            pParty->activeCharacter().pEquipment.uMainHand = freeslot + 1;
                        } else {
                            pParty->pPickedItem.uBodyAnchor = ITEM_SLOT_MAIN_HAND;
                            pParty->activeCharacter().pInventoryItemList[freeslot] = pParty->pPickedItem;
                            pParty->activeCharacter().pEquipment.uMainHand = freeslot + 1;
                            mouse->RemoveHoldingItem();
                        }
                    }
                }
                return;
                //-------------------------------------------------------------------------------
            default:
                pParty->activeCharacter().useItem(pParty->activeCharacterIndex() - 1, false);
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
            // pitem = pParty->activeCharacter().GetAmuletItem(); //9
            pos = ITEM_SLOT_AMULET;
        }

        if (mousex >= glovex && mousex <= (glovex + slot) && mousey >= glovey &&
            mousey <= (glovey + 2 * slot)) {
            // glove
            // pitem = pParty->activeCharacter().GetGloveItem(); //7
            pos = ITEM_SLOT_GAUTNLETS;
        }

        for (int i = 0; i < 6; ++i) {
            if (mousex >= RingsX[i] && mousex <= (RingsX[i] + slot) &&
                mousey >= RingsY[i] && mousey <= (RingsY[i] + slot)) {
                // ring
                // pitem = pParty->activeCharacter().GetNthRingItem(i); //10+i
                pos = RingSlot(i);
            }
        }

        if (pos != ITEM_SLOT_INVALID)
            pitem = pParty->activeCharacter().GetNthEquippedIndexItem(pos);

        if (!pitem) return;
        // pParty->activeCharacter().get

        // enchant / recharge item
        if (IsEnchantingInProgress) {
            /* *((char *)pGUIWindow_CastTargetedSpell->ptr_1C + 8) &=
             *0x7Fu;//CastSpellInfo
             *((short *)pGUIWindow_CastTargetedSpell->ptr_1C + 2) =
             *pParty->activeCharacterIndex() - 1;
             *((int *)pGUIWindow_CastTargetedSpell->ptr_1C + 3) = v36;
             *((short *)pGUIWindow_CastTargetedSpell->ptr_1C + 3) =
             *pEquipType;*/
            pSpellInfo = static_cast<CastSpellInfo *>(pGUIWindow_CastTargetedSpell->wData.ptr);
            pSpellInfo->uFlags &= ~ON_CAST_TargetedEnchantment;
            pSpellInfo->uPlayerID_2 = pParty->activeCharacterIndex() - 1;
            pSpellInfo->spell_target_pid = pParty->activeCharacter().pEquipment.pIndices[pos];
            pSpellInfo->field_6 = std::to_underlying(pitem->GetItemEquipType());

            ptr_50C9A4_ItemToEnchant = pitem;
            IsEnchantingInProgress = false;
            pCurrentFrameMessageQueue->Flush();
            mouse->SetCursorImage("MICON1");
            AfterEnchClickEventId = UIMSG_Escape;
            AfterEnchClickEventSecondParam = 0;
            AfterEnchClickEventTimeout = Timer::Second * 2;
        } else {
            if (!ptr_50C9A4_ItemToEnchant) {  // снять вещь
                pParty->setHoldingItem(pitem);
                pParty->activeCharacter().pEquipment.pIndices[pitem->uBodyAnchor] = 0;
                pitem->Reset();

                // pParty->setHoldingItem(&pParty->activeCharacter().pInventoryItemList[v34
                // - 1]);
                //  pParty->activeCharacter().pEquipment.pIndices[pParty->activeCharacter().pInventoryItemList[v34
                //  - 1].uBodyAnchor - 1] = 0;
                //  pParty->activeCharacter().pInventoryItemList[v34 -
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
                ->pActiveZBuffer[mouse->uMouseX + mouse->uMouseY * render->GetRenderDimensions().w] & 0xFFFF;
        if (v34) {
            // v36 = v34 - 1;
            // v38 = &pPlayers[pParty->_activeCharacter]->pInventoryItemList[v34 - 1];
            pEquipType = pParty->activeCharacter().pInventoryItemList[v34 - 1].GetItemEquipType();
            if (pParty->activeCharacter().pInventoryItemList[v34 - 1].uItemID == ITEM_QUEST_WETSUIT) {
                if (engine->IsUnderwater()) {
                    pAudioPlayer->playUISound(SOUND_error);
                    return;
                }
                WetsuitOff(pParty->activeCharacterIndex());
            }

            if (IsEnchantingInProgress) {  // наложить закл на экипировку
                /* *((char *)pGUIWindow_CastTargetedSpell->ptr_1C + 8) &=
                 *0x7Fu;//CastSpellInfo
                 *((short *)pGUIWindow_CastTargetedSpell->ptr_1C + 2) =
                 *pParty->activeCharacterIndex() - 1;
                 *((int *)pGUIWindow_CastTargetedSpell->ptr_1C + 3) = v36;
                 *((short *)pGUIWindow_CastTargetedSpell->ptr_1C + 3) =
                 *pEquipType;*/
                pSpellInfo = static_cast<CastSpellInfo *>(pGUIWindow_CastTargetedSpell->wData.ptr);
                pSpellInfo->uFlags &= ~ON_CAST_TargetedEnchantment;
                pSpellInfo->uPlayerID_2 = pParty->activeCharacterIndex() - 1;
                pSpellInfo->spell_target_pid = v34 - 1;
                pSpellInfo->field_6 = std::to_underlying(pEquipType);

                ptr_50C9A4_ItemToEnchant =
                    &pParty->activeCharacter().pInventoryItemList[v34 - 1];
                IsEnchantingInProgress = false;
                pCurrentFrameMessageQueue->Flush();
                mouse->SetCursorImage("MICON1");
                AfterEnchClickEventId = UIMSG_Escape;
                AfterEnchClickEventSecondParam = 0;
                AfterEnchClickEventTimeout = Timer::Second * 2;
            } else {
                if (!ptr_50C9A4_ItemToEnchant) {  // снять вещь
                    pParty->setHoldingItem(&pParty->activeCharacter().pInventoryItemList[v34 - 1]);
                    pParty->activeCharacter().pEquipment.pIndices[pParty->activeCharacter().pInventoryItemList[v34 - 1].uBodyAnchor] = 0;
                    pParty->activeCharacter().pInventoryItemList[v34 - 1].Reset();
                }
            }
        } else {  // снять лук
            if (pParty->activeCharacter().pEquipment.uBow) {
                _this = pParty->activeCharacter().pInventoryItemList[pParty->activeCharacter().pEquipment.uBow - 1];
                pParty->setHoldingItem(&_this);
                _this.Reset();
                pParty->activeCharacter().pEquipment.uBow = 0;
            }
        }
    }
}

void CharacterUI_ReleaseButtons() {
    if (dword_507CC0_activ_ch) {
        dword_507CC0_activ_ch = 0;
        std::vector<GUIButton*> to_delete;
        for (GUIButton *pButton : pGUIWindow_CurrentMenu->vButtons) {
            if (pButton->uData & 0x8000) {
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
