#include "UICharacter.h"

#include <cstdio>
#include <algorithm>
#include <vector>
#include <string>
#include <array>
#include <unordered_map>

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Objects/Character.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Spells/CastSpellInfo.h"
#include "Engine/Time/Timer.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/AwardTable.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Data/AwardEnums.h"

#include "GUI/GUIWindow.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/ItemGrid.h"
#include "GUI/UI/UIInventory.h"

#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/MapAccess.h"

void CharacterUI_LoadPaperdollTextures();

/**
 * Prepare textures of character doll with wetsuit on.
 *
 * @param uPlayerID     ID of player, 1-based.
 * @offset 0x43EF2B
 */
void WetsuitOn(int uPlayerID);

/**
 * Prepare textures of character doll with wetsuit off.
 *
 * @param uPlayerID     ID of player, 1-based.
 * @offset 0x43F0BD
 */
void WetsuitOff(int uPlayerID);

int bRingsShownInCharScreen;  // 5118E0

Color ui_mainmenu_copyright_color;

Color ui_character_tooltip_header_default_color;
Color ui_character_default_text_color;
Color ui_character_skill_highlight_color;
Color ui_character_header_text_color;
Color ui_character_bonus_text_color;
Color ui_character_bonus_text_color_neg;
Color ui_character_skill_upgradeable_color;
Color ui_character_skill_default_color;
Color ui_character_stat_default_color;
Color ui_character_stat_buffed_color;
Color ui_character_stat_debuffed_color;
Color ui_character_skillinfo_can_learn;
Color ui_character_skillinfo_can_learn_gm;
Color ui_character_skillinfo_cant_learn;
Color ui_character_condition_normal_color;
Color ui_character_condition_light_color;
Color ui_character_condition_moderate_color;
Color ui_character_condition_severe_color;
std::array<Color, 6> ui_character_award_color;

Color ui_game_minimap_outline_color;
Color ui_game_minimap_actor_friendly_color;
Color ui_game_minimap_actor_hostile_color;
Color ui_game_minimap_actor_corpse_color;
Color ui_game_minimap_decoration_color_1;
Color ui_game_minimap_projectile_color;
Color ui_game_minimap_treasure_color;

Color ui_gamemenu_video_gamma_title_color;
Color ui_gamemenu_keys_action_name_color;
Color ui_gamemenu_keys_key_selection_blink_color_1;
Color ui_gamemenu_keys_key_selection_blink_color_2;
Color ui_gamemenu_keys_key_default_color;

Color ui_book_quests_title_color;
Color ui_book_quests_text_color;
Color ui_book_autonotes_title_color;
Color ui_book_autonotes_text_color;
Color ui_book_map_title_color;
Color ui_book_map_coordinates_color;

Color ui_book_calendar_title_color;
Color ui_book_calendar_time_color;
Color ui_book_calendar_day_color;
Color ui_book_calendar_month_color;
Color ui_book_calendar_year_color;
Color ui_book_calendar_moon_color;
Color ui_book_calendar_location_color;

Color ui_book_journal_title_color;
Color ui_book_journal_text_color;
Color ui_book_journal_text_shadow;

Color ui_game_dialogue_npc_name_color;
Color ui_game_dialogue_option_highlight_color;
Color ui_game_dialogue_option_normal_color;

Color ui_house_player_cant_interact_color;

void set_default_ui_skin_colors() {
    ui_mainmenu_copyright_color = colorTable.White;

    ui_character_tooltip_header_default_color = colorTable.PaleCanary;
    ui_character_default_text_color = colorTable.White;
    ui_character_header_text_color = colorTable.PaleCanary;
    ui_character_bonus_text_color = colorTable.Green;
    ui_character_bonus_text_color_neg = colorTable.Red;

    ui_character_skill_upgradeable_color = colorTable.BoltBlue;
    ui_character_skill_default_color = colorTable.Red;
    ui_character_skill_highlight_color = colorTable.Red;

    ui_character_stat_default_color = colorTable.White;
    ui_character_stat_buffed_color = colorTable.Green;
    ui_character_stat_debuffed_color = colorTable.Scarlet;

    ui_character_skillinfo_can_learn = colorTable.White;
    ui_character_skillinfo_can_learn_gm = colorTable.Yellow;
    ui_character_skillinfo_cant_learn = colorTable.Red;

    ui_character_condition_normal_color = colorTable.White;
    ui_character_condition_light_color = colorTable.Green;
    ui_character_condition_moderate_color = colorTable.Sunflower;
    ui_character_condition_severe_color = colorTable.Scarlet;

    ui_character_award_color[0] = colorTable.Magenta;
    ui_character_award_color[1] = colorTable.Malibu;
    ui_character_award_color[2] = colorTable.MoonRaker;
    ui_character_award_color[3] = colorTable.ScreaminGreen;
    ui_character_award_color[4] = colorTable.Canary;
    ui_character_award_color[5] = colorTable.Mimosa;

    ui_game_minimap_outline_color = colorTable.Blue;
    ui_game_minimap_actor_friendly_color = colorTable.Green;
    ui_game_minimap_actor_hostile_color = colorTable.Red;
    ui_game_minimap_actor_corpse_color = colorTable.Yellow;
    ui_game_minimap_decoration_color_1 = colorTable.White;
    ui_game_minimap_projectile_color = colorTable.Red;
    ui_game_minimap_treasure_color = colorTable.Blue;

    ui_gamemenu_video_gamma_title_color = colorTable.PaleCanary;
    ui_gamemenu_keys_action_name_color = colorTable.White;
    ui_gamemenu_keys_key_selection_blink_color_1 = colorTable.Temptress;
    ui_gamemenu_keys_key_selection_blink_color_2 = colorTable.Sunflower;
    ui_gamemenu_keys_key_default_color = colorTable.White;

    ui_book_quests_title_color = colorTable.White;
    ui_book_quests_text_color = colorTable.White;
    ui_book_autonotes_title_color = colorTable.White;
    ui_book_autonotes_text_color = colorTable.White;
    ui_book_map_title_color = colorTable.White;
    ui_book_map_coordinates_color = colorTable.White;

    ui_book_calendar_title_color = colorTable.White;
    ui_book_calendar_time_color = colorTable.Tundora;
    ui_book_calendar_day_color = colorTable.Tundora;
    ui_book_calendar_month_color = colorTable.Tundora;
    ui_book_calendar_year_color = colorTable.Tundora;
    ui_book_calendar_moon_color = colorTable.Tundora;
    ui_book_calendar_location_color = colorTable.Tundora;

    ui_book_journal_title_color = colorTable.White;
    ui_book_journal_text_color = colorTable.White;
    ui_book_journal_text_shadow = colorTable.Black;

    ui_game_dialogue_npc_name_color = colorTable.EasternBlue;
    ui_game_dialogue_option_highlight_color = colorTable.Sunflower;
    ui_game_dialogue_option_normal_color = colorTable.White;

    ui_house_player_cant_interact_color = colorTable.PaleCanary;
}

GraphicsImage *paperdoll_drhs[4];
GraphicsImage *paperdoll_dlhus[4];
GraphicsImage *paperdoll_dlhs[4];
GraphicsImage *paperdoll_dbods[5];
GraphicsImage *paperdoll_armor_texture[4][17][3];  // 0x511294
// int paperdoll_array_51132C[165];
GraphicsImage *paperdoll_dlaus[5];
GraphicsImage *paperdoll_dlads[4];
GraphicsImage *paperdoll_flying_feet[22];      // 005115E0
GraphicsImage *paperdoll_boots_texture[4][6];  // 511638
GraphicsImage *paperdoll_cloak_collar_texture[4][10];
GraphicsImage *paperdoll_cloak_texture[4][10];
GraphicsImage *paperdoll_helm_texture[2][16];  // 511698
GraphicsImage *paperdoll_belt_texture[4][7];   // 511718

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
const std::unordered_map<ItemId, int> paperdoll_boots_indexByType = {
    {ITEM_LEATHER_BOOTS, 0},
    {ITEM_STEEL_PLATED_BOOTS, 1},
    {ITEM_RANGER_BOOTS, 2},
    {ITEM_KNIGHTS_BOOTS, 3},
    {ITEM_PALADIN_BOOTS, 4},
    {ITEM_ARTIFACT_SEVEN_LEAGUE_BOOTS, 5},
    {ITEM_ARTIFACT_HERMES_SANDALS, 6},
};
const std::unordered_map<int, ItemId> paperdoll_boots_typeByIndex = inverted(paperdoll_boots_indexByType);

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
const std::unordered_map<ItemId, int> paperdoll_cloak_indexByType = {
    {ITEM_LEATHER_CLOAK, 0},
    {ITEM_HUNTSMANS_CLOAK, 1},
    {ITEM_RANGERS_CLOAK, 2},
    {ITEM_ELEGANT_CLOAK, 3},
    {ITEM_GLORIOUS_CLOAK, 4},
    {ITEM_RELIC_TWILIGHT, 5},
    {ITEM_ARTIFACT_CLOAK_OF_THE_SHEEP, 6},
    {ITEM_SPECIAL_SUN_CLOAK, 7},
    {ITEM_SPECIAL_MOON_CLOAK, 8},
    {ITEM_SPECIAL_VAMPIRES_CAPE, 9}
};
const std::unordered_map<int, ItemId> paperdoll_cloak_typeByIndex = inverted(paperdoll_cloak_indexByType);

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
const std::unordered_map<ItemId, int> paperdoll_belt_indexByType = {
    {ITEM_LEATHER_BELT, 0},
    {ITEM_FINE_BELT, 1},
    {ITEM_STRONG_BELT, 2},
    {ITEM_SILVER_BELT, 3},
    {ITEM_GILDED_BELT, 4},
    {ITEM_RELIC_TITANS_BELT, 5},
    {ITEM_ARTIFACT_HEROS_BELT, 6}
};
const std::unordered_map<int, ItemId> paperdoll_belt_typeByIndex = inverted(paperdoll_belt_indexByType);

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
const std::unordered_map<ItemId, int> paperdoll_helm_indexByType = {
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
    {ITEM_SPECIAL_SHADOWS_MASK, 15}
};
const std::unordered_map<int, ItemId> paperdoll_helm_typeByIndex = inverted(paperdoll_helm_indexByType);

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
const std::unordered_map<ItemId, int> paperdoll_armor_indexByType = {
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
const std::unordered_map<int, ItemId> paperdoll_armor_typeByIndex = inverted(paperdoll_armor_indexByType);

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

static constexpr std::array<const char *, 25> dlad_texnames_by_face = {
    "pc01lad", "pc02lad", "pc03lad", "pc04lad", "pc05lad", "pc06lad", "pc07lad",
    "pc08lad", "pc09lad", "pc10lad", "pc11lad", "pc12lad", "pc13lad", "pc14lad",
    "pc15lad", "pc16lad", "pc17lad", "pc18lad", "pc19lad", "pc20lad", "pc21lad",
    "pc22lad", "pc23lad", "pc24lad", "pc25lad"};
static constexpr std::array<const char *, 25> dlau_texnames_by_face = {
    "pc01lau", "pc02lau", "pc03lau", "pc04lau", "pc05lau", "pc06lau", "pc07lau",
    "pc08lau", "pc09lau", "pc10lau", "pc11lau", "pc12lau", "pc13lau", "pc14lau",
    "pc15lau", "pc16lau", "pc17lau", "pc18lau", "pc19lau", "pc20lau", "pc21lau",
    "pc22lau", "pc23lau", "pc24lau", "pc25lau"};
static constexpr std::array<const char *, 25> dbod_texnames_by_face = {
    "pc01bod", "pc02bod", "pc03bod", "pc04bod", "pc05bod", "pc06bod", "pc07bod",
    "pc08bod", "pc09bod", "pc10bod", "pc11bod", "pc12bod", "pc13bod", "pc14bod",
    "pc15bod", "pc16bod", "pc17bod", "pc18bod", "pc19bod", "pc20bod", "pc21bod",
    "pc22bod", "pc23bod", "pc24bod", "pc25bod"};
static constexpr std::array<const char *, 25> drh_texnames_by_face = {
    "pc01rh", "pc02rh", "pc03rh", "pc04rh", "pc05rh", "pc06rh", "pc07rh",
    "pc08rh", "pc09rh", "pc10rh", "pc11rh", "pc12rh", "pc13rh", "pc14rh",
    "pc15rh", "pc16rh", "pc17rh", "pc18rh", "pc19rh", "pc20rh", "pc21rh",
    "pc22rh", "pc23rh", "pc24rh", "pc25rh"};
static constexpr std::array<const char *, 25> dlh_texnames_by_face = {
    "pc01lh", "pc02lh", "pc03lh", "pc04lh", "pc05lh", "pc06lh", "pc07lh",
    "pc08lh", "pc09lh", "pc10lh", "pc11lh", "pc12lh", "pc13lh", "pc14lh",
    "pc15lh", "pc16lh", "pc17lh", "pc18lh", "pc19lh", "pc20lh", "pc21lh",
    "pc22lh", "pc23lh", "pc24lh", "pc25lh"};
static constexpr std::array<const char *, 25> dlhu_texnames_by_face = {
    "pc01lhu", "pc02lhu", "pc03lhu", "pc04lhu", "pc05lhu", "pc06lhu", "pc07lhu",
    "pc08lhu", "pc09lhu", "pc10lhu", "pc11lhu", "pc12lhu", "pc13lhu", "pc14lhu",
    "pc15lhu", "pc16lhu", "pc17lhu", "pc18lhu", "pc19lhu", "pc20lhu", "pc21lhu",
    "pc22lhu", "pc23lhu", "pc24lhu", "pc25lhu"};

GraphicsImage *ui_character_skills_background = nullptr;
GraphicsImage *ui_character_awards_background = nullptr;
GraphicsImage *ui_character_stats_background = nullptr;
GraphicsImage *ui_character_inventory_background = nullptr;
GraphicsImage *ui_character_inventory_background_strip = nullptr;
GraphicsImage *ui_character_inventory_magnification_glass = nullptr;
GraphicsImage *ui_character_inventory_paperdoll_background = nullptr;
GraphicsImage *ui_character_inventory_paperdoll_rings_background = nullptr;

static GraphicsImage *scrollstop = nullptr;

std::array<GraphicsImage *, 16> paperdoll_dbrds;

int savedInventoryLeftClickButtonW;
int savedInventoryLeftClickButtonZ;
int savedInventoryLeftClickButtonY;
int savedInventoryLeftClickButtonX;

GUIWindow_CharacterRecord::GUIWindow_CharacterRecord(int uActiveCharacter, ScreenType screen)
    : GUIWindow(WINDOW_CharacterRecord, {0, 0}, render->GetRenderDimensions()) {
    pEventTimer->setPaused(true);
    bRingsShownInCharScreen = false;
    CharacterUI_LoadPaperdollTextures();
    current_screen_type = screen;

    pCharacterScreen_StatsBtn = CreateButton({pViewport->viewportTL_X + 12, pViewport->viewportTL_Y + 308},
                                             paperdoll_dbrds[9]->size(), 1, 0,
                                             UIMSG_ClickStatsBtn, 0, Io::InputAction::Stats, localization->GetString(LSTR_STATS),
                                             {{paperdoll_dbrds[10], paperdoll_dbrds[9]}});
    pCharacterScreen_SkillsBtn = CreateButton({pViewport->viewportTL_X + 102, pViewport->viewportTL_Y + 308},
                                              paperdoll_dbrds[7]->size(), 1, 0,
                                              UIMSG_ClickSkillsBtn, 0, Io::InputAction::Skills, localization->GetString(LSTR_SKILLS),
                                              {{paperdoll_dbrds[8], paperdoll_dbrds[7]}});
    pCharacterScreen_InventoryBtn = CreateButton({pViewport->viewportTL_X + 192, pViewport->viewportTL_Y + 308},
                                                 paperdoll_dbrds[5]->size(), 1, 0,
                                                 UIMSG_ClickInventoryBtn, 0, Io::InputAction::Inventory,
                                                 localization->GetString(LSTR_INVENTORY),
                                                 {{paperdoll_dbrds[6], paperdoll_dbrds[5]}});
    pCharacterScreen_AwardsBtn = CreateButton({pViewport->viewportTL_X + 282, pViewport->viewportTL_Y + 308},
                                              paperdoll_dbrds[3]->size(), 1, 0,
                                              UIMSG_ClickAwardsBtn, 0, Io::InputAction::Awards, localization->GetString(LSTR_AWARDS),
                                              {{paperdoll_dbrds[4], paperdoll_dbrds[3]}});
    pCharacterScreen_ExitBtn = CreateButton({pViewport->viewportTL_X + 371, pViewport->viewportTL_Y + 308},
                                            paperdoll_dbrds[1]->size(), 1, 0,
                                            UIMSG_ClickExitCharacterWindowBtn, 0, Io::InputAction::Invalid,
                                            localization->GetString(LSTR_EXIT_DIALOGUE),
                                            {{paperdoll_dbrds[2], paperdoll_dbrds[1]}});
    CreateButton({0, 0}, {476, 345}, 1, 122, UIMSG_InventoryLeftClick, 0);
    pCharacterScreen_DetalizBtn = CreateButton({600, 300}, {30, 30}, 1, 0,
        UIMSG_ChangeDetaliz, 0, Io::InputAction::Invalid, localization->GetString(LSTR_DETAIL_TOGGLE));
    pCharacterScreen_DollBtn = CreateButton({476, 0}, {164, 345}, 1, 0, UIMSG_ClickPaperdoll, 0);

    CreateButton({61, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 1, Io::InputAction::SelectChar1);
    CreateButton({177, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 2, Io::InputAction::SelectChar2);
    CreateButton({292, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 3, Io::InputAction::SelectChar3);
    CreateButton({407, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 4, Io::InputAction::SelectChar4);

    CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_CycleCharacters, 0, Io::InputAction::CharCycle);
    fillAwardsData();

    ui_character_skills_background = assets->getImage_ColorKey("fr_skill");
    ui_character_awards_background = assets->getImage_ColorKey("fr_award");
    ui_character_stats_background = assets->getImage_ColorKey("fr_stats");
    ui_character_inventory_background_strip = assets->getImage_ColorKey("fr_strip");

    scrollstop = assets->getImage_ColorKey("con_x");
}

void GUIWindow_CharacterRecord::releaseAwardsScrollBar() {
    if (_awardsScrollBarCreated) {
        _awardsScrollBarCreated = false;
        pBtn_Scroll->Release();
        pBtn_Up->Release();
        pBtn_Down->Release();
        pBtn_Down = 0;
        pBtn_Up = 0;
        for (GUIButton *pButton : pGUIWindow_CurrentMenu->vButtons) {
            if (pButton->msg == UIMSG_InventoryLeftClick) {
                pButton->uX = savedInventoryLeftClickButtonX;
                pButton->uY = savedInventoryLeftClickButtonY;
                pButton->uZ = savedInventoryLeftClickButtonZ;
                pButton->uW = savedInventoryLeftClickButtonW;
            }
        }
    }
}

void GUIWindow_CharacterRecord::createAwardsScrollBar() {
    if (!_awardsScrollBarCreated) {
        _awardsScrollBarCreated = true;
        for (GUIButton *pButton : pGUIWindow_CurrentMenu->vButtons) {
            if (pButton->msg == UIMSG_InventoryLeftClick) {
                savedInventoryLeftClickButtonX = pButton->uX;
                savedInventoryLeftClickButtonY = pButton->uY;
                savedInventoryLeftClickButtonZ = pButton->uZ;
                savedInventoryLeftClickButtonW = pButton->uW;
                pButton->uW = 0;
                pButton->uZ = 0;
                pButton->uY = 0;
                pButton->uX = 0;
            }
        }
        pBtn_Up = pGUIWindow_CurrentMenu->CreateButton({438, 46}, ui_ar_up_up->size(), 1, 0,
                                                       UIMSG_ClickAwardsUpBtn, 0, Io::InputAction::Invalid, "", {{ui_ar_up_up, ui_ar_up_dn}});
        pBtn_Down = pGUIWindow_CurrentMenu->CreateButton({438, 292}, ui_ar_dn_up->size(), 1, 0,
                                                         UIMSG_ClickAwardsDownBtn, 0, Io::InputAction::Invalid, "", {{ui_ar_dn_up, ui_ar_dn_dn}});
        pBtn_Scroll = pGUIWindow_CurrentMenu->CreateButton({440, 62}, {16, 232}, 1, 0, UIMSG_ClickAwardScrollBar, 0, Io::InputAction::Invalid, "");
    }
}

void GUIWindow_CharacterRecord::Update() {
    auto player = &pParty->activeCharacter();

    render->ClearHitMap();
    switch (current_character_screen_window) {
        case WINDOW_CharacterWindow_Stats: {
            CharacterUI_ReleaseButtons();
            releaseAwardsScrollBar();
            CharacterUI_StatsTab_Draw(player);
            render->DrawTextureNew(pCharacterScreen_StatsBtn->uX / 640.0f, pCharacterScreen_StatsBtn->uY / 480.0f, assets->getImage_ColorKey("ib-cd1-d"));
            break;
        }
        case WINDOW_CharacterWindow_Skills: {
            if (dword_507CC0_activ_ch != pParty->activeCharacterIndex()) {
                CharacterUI_ReleaseButtons();
                CharacterUI_SkillsTab_CreateButtons();
            }
            releaseAwardsScrollBar();
            CharacterUI_SkillsTab_Draw(player);
            render->DrawTextureNew(pCharacterScreen_SkillsBtn->uX / 640.0f, pCharacterScreen_SkillsBtn->uY / 480.0f, assets->getImage_ColorKey("ib-cd2-d"));
            break;
        }
        case WINDOW_CharacterWindow_Awards: {
            CharacterUI_ReleaseButtons();
            createAwardsScrollBar();
            CharacterUI_AwardsTab_Draw(player);
            render->DrawTextureNew(pCharacterScreen_AwardsBtn->uX / 640.0f, pCharacterScreen_AwardsBtn->uY / 480.0f, assets->getImage_ColorKey("ib-cd4-d"));
            break;
        }
        case WINDOW_CharacterWindow_Inventory: {
            CharacterUI_ReleaseButtons();
            releaseAwardsScrollBar();
            CharacterUI_InventoryTab_Draw(player, false);
            render->DrawTextureNew(pCharacterScreen_InventoryBtn->uX / 640.0f, pCharacterScreen_InventoryBtn->uY / 480.0f, assets->getImage_ColorKey("ib-cd3-d"));
            break;
        }
        default:
            assert(false);
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
    releaseAwardsScrollBar();
    new OnButtonClick3(WINDOW_CharacterWindow_Stats,
        {pCharacterScreen_StatsBtn->uX, pCharacterScreen_StatsBtn->uY}, {0, 0}, pCharacterScreen_StatsBtn);
}

void GUIWindow_CharacterRecord::ShowSkillsTab() {
    current_character_screen_window = WINDOW_CharacterWindow_Skills;
    CharacterUI_ReleaseButtons();
    releaseAwardsScrollBar();
    CharacterUI_SkillsTab_CreateButtons();
    new OnButtonClick3(WINDOW_CharacterWindow_Skills,
        {pCharacterScreen_SkillsBtn->uX, pCharacterScreen_SkillsBtn->uY}, {0, 0}, pCharacterScreen_SkillsBtn);
}

void GUIWindow_CharacterRecord::ShowInventoryTab() {
    current_character_screen_window = WINDOW_CharacterWindow_Inventory;
    releaseAwardsScrollBar();
    CharacterUI_ReleaseButtons();
    new OnButtonClick3(WINDOW_CharacterWindow_Inventory,
        {pCharacterScreen_InventoryBtn->uX, pCharacterScreen_InventoryBtn->uY}, {0, 0}, pCharacterScreen_InventoryBtn);
}

void GUIWindow_CharacterRecord::ShowAwardsTab() {
    releaseAwardsScrollBar();
    CharacterUI_ReleaseButtons();
    createAwardsScrollBar();
    current_character_screen_window = WINDOW_CharacterWindow_Awards;
    new OnButtonClick3(WINDOW_CharacterWindow_Awards,
        {pCharacterScreen_AwardsBtn->uX, pCharacterScreen_AwardsBtn->uY}, {0, 0}, pCharacterScreen_AwardsBtn);
    fillAwardsData();
}

void GUIWindow_CharacterRecord::ToggleRingsOverlay() {
    int x, y, w, h;

    bRingsShownInCharScreen ^= 1;
    pCharacterScreen_DetalizBtn->Release();
    pCharacterScreen_DollBtn->Release();
    if (bRingsShownInCharScreen) {
        h = ui_exit_cancel_button_background->height();
        w = ui_exit_cancel_button_background->width();
        y = 445;
        x = 471;
    } else {
        h = 30;
        w = 30;
        y = 300;
        x = 600;
    }
    pCharacterScreen_DetalizBtn = pGUIWindow_CurrentMenu->CreateButton({x, y}, {w, h}, 1, 0,
        UIMSG_ChangeDetaliz, 0, Io::InputAction::Invalid, localization->GetString(LSTR_DETAIL_TOGGLE));
    pCharacterScreen_DollBtn = pGUIWindow_CurrentMenu->CreateButton({476, 0}, {164, 345}, 1, 0, UIMSG_ClickPaperdoll, 0);
}

TargetedSpellUI *CastSpellInfo::GetCastSpellInInventoryWindow() {
    pEventTimer->setPaused(true);
    bRingsShownInCharScreen = 0;
    CharacterUI_LoadPaperdollTextures();
    current_screen_type = SCREEN_CASTING;
    TargetedSpellUI *CS_inventory_window = new GUIWindow_Inventory_CastSpell({0, 0}, render->GetRenderDimensions(), this, "");
    pCharacterScreen_ExitBtn = CS_inventory_window->CreateButton({394, 318}, {75, 33}, 1, 0,
        UIMSG_ClickExitCharacterWindowBtn, 0, Io::InputAction::Invalid, localization->GetString(LSTR_EXIT_DIALOGUE),
        {{paperdoll_dbrds[2], paperdoll_dbrds[1]}});
    CS_inventory_window->CreateButton({0, 0}, {0x1DCu, 0x159u}, 1, 122, UIMSG_InventoryLeftClick, 0);
    pCharacterScreen_DollBtn = CS_inventory_window->CreateButton({0x1DCu, 0}, {0xA4u, 0x159u}, 1, 0, UIMSG_ClickPaperdoll, 0);

    CS_inventory_window->CreateButton({61, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 1, Io::InputAction::SelectChar1);
    CS_inventory_window->CreateButton({177, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 2, Io::InputAction::SelectChar2);
    CS_inventory_window->CreateButton({292, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 3, Io::InputAction::SelectChar3);
    CS_inventory_window->CreateButton({407, 424}, {31, 0}, 2, 94, UIMSG_SelectCharacter, 4, Io::InputAction::SelectChar4);

    return CS_inventory_window;
}

static int drawSkillTable(Character *player, int x, int y, const std::initializer_list<Skill> skill_list,
                          int right_margin, std::string_view skill_group_name) {
    int y_offset = y;
    Pointi pt = mouse->position();

    auto str = fmt::format("{}\r{:03}{}", skill_group_name, right_margin, localization->GetString(LSTR_LEVEL));
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {x, y}, ui_character_header_text_color, str);

    int num_skills_drawn = 0;
    for (Skill skill : skill_list) {
        for (size_t j = 0; j < pGUIWindow_CurrentMenu->vButtons.size(); ++j) {
            GUIButton *button = pGUIWindow_CurrentMenu->GetControl(j);
            if ((short)(button->uData) >= 0) {
                continue;  // skips an of the stats skills innv awards buttons
            }

            if (static_cast<Skill>(button->uData & 0x7FFF) != skill) {
                continue;  // skips buttons that dont match skill
            }

            ++num_skills_drawn;
            y_offset = button->uY;

            int skill_level = player->getSkillValue(skill).level();

            Color skill_color = colorTable.White;
            Color skill_mastery_color;
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

            Mastery skill_mastery = player->getSkillValue(skill).mastery();
            if (skill_mastery == MASTERY_NOVICE) {
                std::string Strsk;
                if (skills_max_level[skill] == 1) { // Non-investable skill
                    Strsk = fmt::format("{}\r{:03}-", localization->GetSkillName(skill), right_margin);
                } else {
                    Strsk = fmt::format("{}\r{:03}{}", localization->GetSkillName(skill), right_margin, skill_level);
                }
                pGUIWindow_CurrentMenu->DrawText(assets->pFontLucida.get(), {x, button->uY}, skill_color, Strsk);
            } else {
                std::string skill_level_str = skill_mastery == MASTERY_NOVICE ? "" : localization->MasteryName(skill_mastery);

                if (skill_mastery_color == Color()) {
                    skill_mastery_color = ui_character_header_text_color;
                }

                auto Strsk = fmt::format("{} {::}{}{::}\r{:03}{}",
                        localization->GetSkillName(skill), skill_mastery_color.tag(), skill_level_str, skill_color.tag(), right_margin, skill_level);
                pGUIWindow_CurrentMenu->DrawText(assets->pFontLucida.get(), {x, button->uY}, skill_color, Strsk);
            }
        }
    }

    if (!num_skills_drawn) {
        y_offset += assets->pFontLucida->GetHeight() - 3;
        pGUIWindow_CurrentMenu->DrawText(assets->pFontLucida.get(), {x, y_offset}, colorTable.White, localization->GetString(LSTR_NONE));
    }

    return y_offset;
}

//----- (00419719) --------------------------------------------------------
void GUIWindow_CharacterRecord::CharacterUI_SkillsTab_Draw(Character *player) {
    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, ui_character_skills_background);

    auto str = fmt::format(
        "{} {::}{}\f00000\r177{}: {::}{}\f00000",  // ^Pv[]
                     localization->GetString(LSTR_SKILLS_FOR),
                     ui_character_header_text_color.tag(), player->name,
                     localization->GetString(LSTR_SKILL_POINTS),
                     player->uSkillPoints ? ui_character_bonus_text_color.tag()
                                          : ui_character_default_text_color.tag(),
                     player->uSkillPoints);
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {24, 18}, colorTable.White, str);

    int y = 2 * assets->pFontLucida->GetHeight() + 13;
    y = drawSkillTable(player, 24, y, allWeaponSkills(), 400, localization->GetString(LSTR_WEAPONS));

    y += 2 * assets->pFontLucida->GetHeight() - 10;
    drawSkillTable(player, 24, y, allMagicSkills(), 400, localization->GetString(LSTR_MAGIC));

    y = 2 * assets->pFontLucida->GetHeight() + 13;
    y = drawSkillTable(player, 248, y, allArmorSkills(), 177, localization->GetString(LSTR_ARMOR));

    y += 2 * assets->pFontLucida->GetHeight() - 10;
    y = drawSkillTable(player, 248, y, allMiscSkills(), 177, localization->GetString(LSTR_MISC));
}

GUIWindow GUIWindow_CharacterRecord::prepareAwardsWindow() {
    GUIWindow awardsWindow;

    awardsWindow.uFrameX = 12;
    awardsWindow.uFrameY = 48;
    awardsWindow.uFrameWidth = 424;
    awardsWindow.uFrameHeight = 290;
    awardsWindow.uFrameZ = 435;
    awardsWindow.uFrameW = 337;

    return awardsWindow;
}

std::string GUIWindow_CharacterRecord::getAchievedAwardsString(int idx) {
    std::string str;

    // TODO(captainurist): fmt can throw
    switch (_achievedAwardsList[idx]) {
      case Award_Arena_PageWins:
        str = fmt::sprintf(pAwards[_achievedAwardsList[idx]].pText, pParty->uNumArenaWins[ARENA_LEVEL_PAGE]);
        break;
      case Award_Arena_SquireWins:
        str = fmt::sprintf(pAwards[_achievedAwardsList[idx]].pText, pParty->uNumArenaWins[ARENA_LEVEL_SQUIRE]);
        break;
      case Award_Arena_KnightWins:
        str = fmt::sprintf(pAwards[_achievedAwardsList[idx]].pText, pParty->uNumArenaWins[ARENA_LEVEL_KNIGHT]);
        break;
      case Award_Arena_LordWins:
        str = fmt::sprintf(pAwards[_achievedAwardsList[idx]].pText, pParty->uNumArenaWins[ARENA_LEVEL_LORD]);
        break;
      case Award_ArcomageWins:
        str = fmt::sprintf(pAwards[_achievedAwardsList[idx]].pText, pParty->uNumArcomageWins);
        break;
      case Award_ArcomageLoses:
        str = fmt::sprintf(pAwards[_achievedAwardsList[idx]].pText, pParty->uNumArcomageLoses);
        break;
      case Award_Deaths:
        str = fmt::sprintf(pAwards[_achievedAwardsList[idx]].pText, pParty->uNumDeaths);
        break;
      case Award_BountiesCollected:
        str = fmt::sprintf(pAwards[_achievedAwardsList[idx]].pText, pParty->uNumBountiesCollected);
        break;
      case Award_Fine:
        str = fmt::sprintf(pAwards[_achievedAwardsList[idx]].pText, pParty->uFine);
        break;
      case Award_PrisonTerms:
        str = fmt::sprintf(pAwards[_achievedAwardsList[idx]].pText, pParty->uNumPrisonTerms);
        break;
      default:
        break;
    }

    if (str.empty()) {
        str = std::string(pAwards[_achievedAwardsList[idx]].pText);
    }

    return str;
}

void GUIWindow_CharacterRecord::clickAwardsUp() {
    if (_startAwardElem) {
        _startAwardElem--;
    }
}

void GUIWindow_CharacterRecord::clickAwardsDown() {
    if (!_awardLimitReached) {
        _startAwardElem++;
    }
}

void GUIWindow_CharacterRecord::clickAwardsScroll(int yPos) {
    if (!_scrollableAwardSteps) {
        return;
    }

    int segmentHeight = pBtn_Scroll->uHeight / _scrollableAwardSteps;

    _startAwardElem = std::clamp((int)std::round((float)(yPos - pBtn_Scroll->uY) / segmentHeight), 0, _scrollableAwardSteps);
}

void GUIWindow_CharacterRecord::CharacterUI_AwardsTab_Draw(Character *player) {
    GUIWindow window = prepareAwardsWindow();
    int stopPos = 0;

    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, ui_character_awards_background);

    std::string str = fmt::format("{} {::}{}\f00000", localization->GetString(LSTR_AWARDS_FOR),
                                  ui_character_header_text_color.tag(), NameAndTitle(player->name, player->classType));

    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {24, 18}, colorTable.White, str);


    if (_awardsCharacterId != pParty->activeCharacterIndex()) {
        fillAwardsData();
    }

    int currentlyDisplayedElems = 0;
    for (int i = _startAwardElem; i < _achievedAwardsList.size(); ++i) {
        std::string str = getAchievedAwardsString(i);

        window.DrawText(assets->pFontArrus.get(), {0, 0}, ui_character_award_color[pAwards[_achievedAwardsList[i]].uPriority % 6], str);
        window.uFrameY = assets->pFontArrus->CalcTextHeight(str, window.uFrameWidth, 0) + window.uFrameY + 8;
        currentlyDisplayedElems++;
        if (window.uFrameY > window.uFrameHeight) {
            break;
        }
    }

    _awardLimitReached = (_startAwardElem + currentlyDisplayedElems) == _achievedAwardsList.size();

    if (_startAwardElem) {
        if (!_awardLimitReached) {
            stopPos = (float(_startAwardElem) / _scrollableAwardSteps) * 211.0f;
        } else {
            stopPos = 211;
        }
    }
    render->DrawTextureNew(439 / 640.f, (65 + stopPos) / 480.f, scrollstop);
}

//----- (0041A2C1) --------------------------------------------------------
int GetSizeInInventorySlots(int uNumPixels) {
    if (uNumPixels < 14)
        uNumPixels = 14;

    return ((uNumPixels - 14) >> 5) + 1;
}

//----- (0041A556) --------------------------------------------------------
void draw_leather() {
    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, ui_leather_mm7);
}

//----- (0043CC7C) --------------------------------------------------------
void CharacterUI_DrawPaperdoll(Character *player) {
    int index;
    int item_X;
    int item_Y;

    int IsDwarf;
    int pBodyComplection;
    if (player->GetRace() == RACE_DWARF) {
        IsDwarf = 1;
        pBodyComplection = player->GetSexByVoice() == SEX_MALE ? 2 : 3;
    } else {
        IsDwarf = 0;
        pBodyComplection = player->GetSexByVoice() == SEX_MALE ? 0 : 1;
    }

    int uPlayerID = pParty->getCharacterIdInParty(player);

    render->ResetUIClipRect();
    render->DrawTextureNew(467 / 640.0f, 0, ui_character_inventory_paperdoll_background);

    InventoryEntry itemMainHand = player->inventory.entry(ITEM_SLOT_MAIN_HAND);
    InventoryEntry itemOffHand = player->inventory.entry(ITEM_SLOT_OFF_HAND);
    bool bTwoHandedGrip = itemMainHand && (itemMainHand->type() == ITEM_TYPE_TWO_HANDED || itemMainHand->skill() == SKILL_SPEAR && !itemOffHand);

    // Aqua-Lung
    if (player->hasUnderwaterSuitEquipped()) {
        // TODO(captainurist): need to also z-draw arms and wrists.
        render->DrawTextureNew(pPaperdoll_BodyX / 640.0f, pPaperdoll_BodyY / 480.0f, paperdoll_dbods[uPlayerID]);
        if (!bRingsShownInCharScreen)
            render->DrawToHitMap(pPaperdoll_BodyX / 640.0f, pPaperdoll_BodyY / 480.0f, paperdoll_dbods[uPlayerID], player->inventory.entry(ITEM_SLOT_ARMOUR).index());

        // hands aren't in two handed grip pose
        if (!bTwoHandedGrip) {
            item_X = pPaperdoll_BodyX + pPaperdoll_LeftHand[pBodyComplection][0];
            item_Y = pPaperdoll_BodyY + pPaperdoll_LeftHand[pBodyComplection][1];

            render->DrawTextureNew(item_X / 640.0f, item_Y / 480.0f, paperdoll_dlads[uPlayerID]);
        }

        // main hand's item
        if (itemMainHand) {
            item_X = pPaperdoll_BodyX + paperdoll_Weapon[pBodyComplection][1][0] - pItemTable->items[itemMainHand->itemId].paperdollAnchorOffset.x;
            item_Y = pPaperdoll_BodyY + paperdoll_Weapon[pBodyComplection][1][1] - pItemTable->items[itemMainHand->itemId].paperdollAnchorOffset.y;

            GraphicsImage *texture = nullptr;
            if (itemMainHand->itemId == ITEM_BLASTER)
                texture = assets->getImage_Alpha("item64v1");

            CharacterUI_DrawItem(item_X, item_Y, itemMainHand.get(), itemMainHand.index(), texture, !bRingsShownInCharScreen);
        }
    } else {
        // bow
        if (InventoryEntry bow = player->inventory.entry(ITEM_SLOT_BOW)) {
            item_X = pPaperdoll_BodyX + paperdoll_Weapon[pBodyComplection][2][0] - pItemTable->items[bow->itemId].paperdollAnchorOffset.x;
            item_Y = pPaperdoll_BodyY + paperdoll_Weapon[pBodyComplection][2][1] - pItemTable->items[bow->itemId].paperdollAnchorOffset.y;

            CharacterUI_DrawItem(item_X, item_Y, bow.get(), bow.index(), nullptr, !bRingsShownInCharScreen);
        }

        // cloak
        if (InventoryEntry cloak = player->inventory.entry(ITEM_SLOT_CLOAK)) {
            index = valueOr(paperdoll_cloak_indexByType, cloak->itemId, -1);
            if (index != -1) {
                item_X = pPaperdoll_BodyX + paperdoll_Cloak[pBodyComplection][index][0];
                item_Y = pPaperdoll_BodyY + paperdoll_Cloak[pBodyComplection][index][1];

                GraphicsImage *texture = paperdoll_cloak_texture[pBodyComplection][index];
                CharacterUI_DrawItem(item_X, item_Y, cloak.get(), cloak.index(), texture, !bRingsShownInCharScreen);
            }
        }

        // paperdoll
        render->DrawTextureNew(pPaperdoll_BodyX / 640.0f, pPaperdoll_BodyY / 480.0f, paperdoll_dbods[uPlayerID]);

        // armor
        if (InventoryEntry armor = player->inventory.entry(ITEM_SLOT_ARMOUR)) {
            index = valueOr(paperdoll_armor_indexByType, armor->itemId, -1);
            if (index != -1) {
                item_X = pPaperdoll_BodyX + paperdoll_Armor_Coord[pBodyComplection][index][0];
                item_Y = pPaperdoll_BodyY + paperdoll_Armor_Coord[pBodyComplection][index][1];

                GraphicsImage *texture = paperdoll_armor_texture[pBodyComplection][index][0];
                CharacterUI_DrawItem(item_X, item_Y, armor.get(), armor.index(), texture, !bRingsShownInCharScreen);
            }
        }

        // boots
        if (InventoryEntry boots = player->inventory.entry(ITEM_SLOT_BOOTS)) {
            index = valueOr(paperdoll_boots_indexByType, boots->itemId, -1);
            if (index != -1) {
                item_X = pPaperdoll_BodyX + paperdoll_Boot[pBodyComplection][index][0];
                item_Y = pPaperdoll_BodyY + paperdoll_Boot[pBodyComplection][index][1];

                GraphicsImage *texture = nullptr;
                if (boots->itemId == ITEM_ARTIFACT_HERMES_SANDALS) {
                    texture = paperdoll_flying_feet[player->uCurrentFace];
                } else {
                    texture = paperdoll_boots_texture[pBodyComplection][index];
                }

                CharacterUI_DrawItem(item_X, item_Y, boots.get(), boots.index(), texture, !bRingsShownInCharScreen);
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
        if (InventoryEntry belt = player->inventory.entry(ITEM_SLOT_BELT)) {
            index = valueOr(paperdoll_belt_indexByType, belt->itemId, -1);
            if (index != -1) {
                item_X = pPaperdoll_BodyX + paperdoll_Belt[pBodyComplection][index][0];
                item_Y = pPaperdoll_BodyY + paperdoll_Belt[pBodyComplection][index][1];
                GraphicsImage *texture = nullptr;
                if (IsDwarf != 1 || index == 5)
                    texture = paperdoll_belt_texture[pBodyComplection][index];
                else
                    texture = paperdoll_belt_texture[pBodyComplection - 2][index];

                CharacterUI_DrawItem(item_X, item_Y, belt.get(), belt.index(), texture, !bRingsShownInCharScreen);
            }
        }

        // armor's shoulders
        if (InventoryEntry armor = player->inventory.entry(ITEM_SLOT_ARMOUR)) {
            index = valueOr(paperdoll_armor_indexByType, armor->itemId, -1);
            if (index != -1) {
                GraphicsImage *texture = nullptr;
                // Some armors doesn't have sleeves so use normal one for two-handed or none if it also unavailable
                if (bTwoHandedGrip && paperdoll_shoulder_second_coord[pBodyComplection][index][0]) {
                    item_X = pPaperdoll_BodyX + paperdoll_shoulder_second_coord[pBodyComplection][index][0];
                    item_Y = pPaperdoll_BodyY + paperdoll_shoulder_second_coord[pBodyComplection][index][1];

                    texture = paperdoll_armor_texture[pBodyComplection][index][2];
                } else if (paperdoll_shoulder_coord[pBodyComplection][index][0]) {
                    item_X = pPaperdoll_BodyX + paperdoll_shoulder_coord[pBodyComplection][index][0];
                    item_Y = pPaperdoll_BodyY + paperdoll_shoulder_coord[pBodyComplection][index][1];

                    texture = paperdoll_armor_texture[pBodyComplection][index][1];
                }

                if (texture)
                    CharacterUI_DrawItem(item_X, item_Y, armor.get(), armor.index(), texture, !bRingsShownInCharScreen);
            }
        }

        // cloak's collar
        if (InventoryEntry cloak = player->inventory.entry(ITEM_SLOT_CLOAK)) {
            index = valueOr(paperdoll_cloak_indexByType, cloak->itemId, -1);
            if (index != -1) {
                // leather cloak has no collar
                if (paperdoll_CloakCollar[pBodyComplection][index][0]) {
                    item_X = pPaperdoll_BodyX + paperdoll_CloakCollar[pBodyComplection][index][0];
                    item_Y = pPaperdoll_BodyY + paperdoll_CloakCollar[pBodyComplection][index][1];

                    GraphicsImage *texture = paperdoll_cloak_collar_texture[pBodyComplection][index];
                    CharacterUI_DrawItem(item_X, item_Y, cloak.get(), cloak.index(), texture, !bRingsShownInCharScreen);
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
        if (InventoryEntry helm = player->inventory.entry(ITEM_SLOT_HELMET)) {
            index = valueOr(paperdoll_helm_indexByType, helm->itemId, -1);
            if (index != -1) {
                item_X = pPaperdoll_BodyX + paperdoll_Helm[pBodyComplection][index][0];
                item_Y = pPaperdoll_BodyY + paperdoll_Helm[pBodyComplection][index][1];

                GraphicsImage *texture = nullptr;
                if (IsDwarf != 1 || helm->itemId != ITEM_PHYNAXIAN_HELM)
                    texture = paperdoll_helm_texture[std::to_underlying(player->GetSexByVoice())][index];
                else
                    texture = paperdoll_dbrds[11];

                CharacterUI_DrawItem(item_X, item_Y, helm.get(), helm.index(), texture, !bRingsShownInCharScreen);
            }
        }

        // main hand's item
        if (itemMainHand) {
            item_X = pPaperdoll_BodyX + paperdoll_Weapon[pBodyComplection][1][0] - pItemTable->items[itemMainHand->itemId].paperdollAnchorOffset.x;
            item_Y = pPaperdoll_BodyY + paperdoll_Weapon[pBodyComplection][1][1] - pItemTable->items[itemMainHand->itemId].paperdollAnchorOffset.y;

            GraphicsImage *texture = nullptr;
            if (itemMainHand->itemId == ITEM_BLASTER)
                texture = assets->getImage_Alpha("item64v1");

            CharacterUI_DrawItem(item_X, item_Y, itemMainHand.get(), itemMainHand.index(), texture, !bRingsShownInCharScreen);
        }

        // offhand's item
        if (itemOffHand) {
            item_X = pPaperdoll_BodyX + paperdoll_Weapon[pBodyComplection][0][0] - pItemTable->items[itemOffHand->itemId].paperdollAnchorOffset.x;
            item_Y = pPaperdoll_BodyY + paperdoll_Weapon[pBodyComplection][0][1] - pItemTable->items[itemOffHand->itemId].paperdollAnchorOffset.y;

            /*
             * MM6 artifacts.
             * These cases should never execute in MM7 as we have spell books in these positions.
             * Also MM6 doesn't have variable size paperdoll's so cordinates need to account pPaperdoll_BodyX/Y.
             */
            if (itemOffHand->skill() == SKILL_DAGGER || itemOffHand->skill() == SKILL_SWORD) {
                switch (itemOffHand->itemId) {
                    case ITEM_SPELLBOOK_TORCH_LIGHT: // Mordred
                        item_X = 596;
                        item_Y = 86;
                        assert(false);
                        break;
                    case ITEM_SPELLBOOK_FIRE_AURA: // Excalibur
                        item_X = 596;
                        item_Y = 28;
                        assert(false);
                        break;
                    case ITEM_SPELLBOOK_JUMP: // Hades
                        item_X = 595;
                        item_Y = 33;
                        assert(false);
                        break;
                    default:
                        break;
                }
            }

            CharacterUI_DrawItem(item_X, item_Y, itemOffHand.get(), itemOffHand.index(), nullptr, !bRingsShownInCharScreen);
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
void CharacterUI_InventoryTab_Draw(Character *player, bool Cover_Strip) {
    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f, ui_character_inventory_background);

    if (Cover_Strip) {
        ui_character_inventory_background_strip = assets->getImage_ColorKey("fr_strip");
        render->DrawTextureNew(8 / 640.0f, 305 / 480.0f, ui_character_inventory_background_strip);
    }

    render->SetUIClipRect({ 14, 17, 32 * 14, 32 * 9 });
    CharacterUI_DrawPickedItemUnderlay({ 14, 17 });
    render->ResetUIClipRect();

    for (InventoryEntry entry : player->inventory.entries()) {
        if (entry.zone() != INVENTORY_ZONE_GRID)
            continue;

        Pointi cellPos = mapFromInventoryGrid(entry.geometry().topLeft(), Pointi(14, 17));

        GraphicsImage *pTexture = assets->getImage_Alpha(entry->GetIconName());

        signed int X_offset = itemOffset(pTexture->width());
        signed int Y_offset = itemOffset(pTexture->height());
        CharacterUI_DrawItem(cellPos.x + X_offset, cellPos.y + Y_offset, entry.get(), Cover_Strip);
    }
}

void CharacterUI_DrawPickedItemUnderlay(Vec2i gridOffset) {
    if (pParty->pPickedItem.itemId != ITEM_NULL) {
        // draw shadow of position
        Pointi mousePos = mouse->position();
        Pointi mouseOffset = mouse->pickedItemOffset;

        // this is a correction so that the shadow is drawn in the same place as the item
        // in the inventory grid, and that it is not drawn centered on the mouse cursor
        Pointi inventoryPos = mapToInventoryGrid(
            Pointi(mousePos.x + mouseOffset.x, mousePos.y + mouseOffset.y),
            gridOffset, &pParty->pPickedItem
        );
        Sizei itemSize = pParty->pPickedItem.inventorySize();

        render->FillRectFast(inventoryPos.x * 32 + gridOffset.x, inventoryPos.y * 32 + gridOffset.y, itemSize.w * 32, itemSize.h * 32, Color(96, 96, 96, 128));
    }
}

static void CharacterUI_DrawItem(int x, int y, Item *item, int id, GraphicsImage *item_texture, bool doZDraw) {
    if (!item_texture)
        item_texture = assets->getImage_Alpha(item->GetIconName());

    if (item->ItemEnchanted()) { // enchant animation
        GraphicsImage *enchantment_texture = nullptr;
        if (item->AuraEffectRed())
            enchantment_texture = assets->getImage_ColorKey("sptext01");
        else if (item->AuraEffectBlue())
            enchantment_texture = assets->getImage_ColorKey("sp28a");
        else if (item->AuraEffectGreen())
            enchantment_texture = assets->getImage_ColorKey("sp30a");
        else if (item->AuraEffectPurple())
            enchantment_texture = assets->getImage_ColorKey("sp91a");
        else
            assert(false);

        ItemEnchantmentTimer = std::max(0_ticks, ItemEnchantmentTimer - pEventTimer->dt());
        if (!ItemEnchantmentTimer) {
            item->ResetEnchantAnimation(); // TODO(captainurist): doesn't belong here, and doesn't belong in Item.
            ptr_50C9A4_ItemToEnchant = nullptr;
        }

        // TODO(pskelton): #time check tickcount usage here
        render->BlendTextures(x, y, item_texture, enchantment_texture, platform->tickCount() / 10, 0, 255);
    } else if (item->IsBroken()) {
        render->DrawTransparentRedShade(x / 640.0f, y / 480.0f, item_texture);
    } else if (!item->IsIdentified() && (engine->config->gameplay.ShowUndentifiedItem.value() || id)) { // TODO(captainurist): after my changes id==0 is a valid item id
        render->DrawTransparentGreenShade(x / 640.0f, y / 480.0f, item_texture);
    } else {
        render->DrawTextureNew(x / 640.0f, y / 480.0f, item_texture);
    }

    if (doZDraw)
        render->DrawToHitMap(x / 640.0f, y / 480.0f, item_texture, id);
}

//----- (0043E825) --------------------------------------------------------
void CharacterUI_DrawPaperdollWithRingOverlay(Character *player) {
    CharacterUI_DrawPaperdoll(player);

    render->DrawTextureNew(473 / 640.0f, 0, ui_character_inventory_paperdoll_rings_background);
    render->DrawTextureNew(468 / 640.0f, 0, game_ui_right_panel_frame);
    render->DrawTextureNew(pCharacterScreen_DetalizBtn->uX / 640.0f,
                                pCharacterScreen_DetalizBtn->uY / 480.0f,
                                ui_exit_cancel_button_background);

    for (unsigned i = 0; i < 6; ++i) {
        InventoryEntry entry = player->inventory.entry(ringSlot(i));
        if (!entry)
            continue;

        static int pPaperdollRingsX[6] = {0x1EA, 0x21A, 0x248, 0x1EA, 0x21A, 0x248};
        static int pPaperdollRingsY[6] = {0x0CA, 0x0CA, 0x0CA, 0x0FA, 0x0FA, 0x0FA};

        CharacterUI_DrawItem(
            pPaperdollRingsX[i], pPaperdollRingsY[i],
            entry.get(),
            entry.index());
    }
    if (InventoryEntry entry = player->inventory.entry(ITEM_SLOT_AMULET))
        CharacterUI_DrawItem(493, 91, entry.get(), entry.index());
    if (InventoryEntry entry = player->inventory.entry(ITEM_SLOT_GAUNTLETS))
        CharacterUI_DrawItem(586, 88, entry.get(), entry.index());
}

//----- (0043BCA7) --------------------------------------------------------
void CharacterUI_LoadPaperdollTextures() {
    ui_character_inventory_magnification_glass = assets->getImage_Alpha("MAGNIF-B");
    ui_character_inventory_paperdoll_background = assets->getImage_ColorKey("BACKDOLL");
    ui_character_inventory_paperdoll_rings_background = assets->getImage_Alpha("BACKHAND");

    for (int i = 0; i < pParty->pCharacters.size(); ++i) {
        if (pParty->pCharacters[i].hasUnderwaterSuitEquipped()) {
            WetsuitOn(i + 1);
        } else {
            WetsuitOff(i + 1);
        }
    }

    ui_ar_up_up = assets->getImage_Solid("ar_up_up");
    ui_ar_up_dn = assets->getImage_Solid("ar_up_dn");
    ui_ar_dn_up = assets->getImage_Solid("ar_dn_up");
    ui_ar_dn_dn = assets->getImage_Solid("ar_dn_dn");

    paperdoll_dbrds[9] = assets->getImage_Solid("ib-cd1-d");
    paperdoll_dbrds[7] = assets->getImage_Solid("ib-cd2-d");
    paperdoll_dbrds[5] = assets->getImage_Solid("ib-cd3-d");
    paperdoll_dbrds[3] = assets->getImage_Solid("ib-cd4-d");
    paperdoll_dbrds[1] = assets->getImage_Solid("ib-cd5-d");

    auto loadTexture = [&](const auto &map, int itemIndex, int bodyIndex, int shoulderIndex) {
        std::string name = GetItemTextureFilename(*valuePtr(map, itemIndex), bodyIndex + 1, shoulderIndex);
        return assets->getImage_Alpha(name);
    };

    for (unsigned i = 0; i < 2; ++i) {
        for (unsigned j : {0, 1, 2, 3, 4, 6}) // Belt
            paperdoll_belt_texture[i][j] = loadTexture(paperdoll_belt_typeByIndex, j, i, 0);

        for (unsigned j = 0; j < 16; ++j)  // Helm
            paperdoll_helm_texture[i][j] = loadTexture(paperdoll_helm_typeByIndex, j, i, 0);

        if (IsDwarfPresentInParty(true))  // the phynaxian helm uses a slightly
                                          // different graphic for dwarves
            paperdoll_dbrds[11] = assets->getImage_Alpha("item092v3");
    }

    for (unsigned i = 0; i < 4; ++i) {
        if (ShouldLoadTexturesForRaceAndGender(i)) {
            paperdoll_belt_texture[i][5] = loadTexture(paperdoll_belt_typeByIndex, 5, i, 0);  // Titans belt

            for (int v32 = 0; v32 < 17; ++v32) {  // simple armor
                paperdoll_armor_texture[i][v32][0] = loadTexture(paperdoll_armor_typeByIndex, v32, i, 0);  // armor
                paperdoll_armor_texture[i][v32][1] = loadTexture(paperdoll_armor_typeByIndex, v32, i, 1);  // shoulder 1
                paperdoll_armor_texture[i][v32][2] = loadTexture(paperdoll_armor_typeByIndex, v32, i, 2);  // shoulder 2
            }

            for (int v33 = 0; v33 < 6; ++v33)  // boots
                paperdoll_boots_texture[i][v33] = loadTexture(paperdoll_boots_typeByIndex, v33, i, 0);

            for (int v38 = 0; v38 < 10; ++v38) {  // Cloak
                paperdoll_cloak_texture[i][v38] = loadTexture(paperdoll_cloak_typeByIndex, v38, i, 0);
                paperdoll_cloak_collar_texture[i][v38] = loadTexture(paperdoll_cloak_typeByIndex, v38, i, 1);
            }
        }
    }
}

void GUIWindow_CharacterRecord::CharacterUI_SkillsTab_CreateButtons() {
    Skill skill;

    int buttons_count = 0;
    if (dword_507CC0_activ_ch) CharacterUI_ReleaseButtons();
    dword_507CC0_activ_ch = pParty->activeCharacterIndex();
    for (GUIButton *pButton : pGUIWindow_CurrentMenu->vButtons) {
        if (pButton->msg == UIMSG_InventoryLeftClick) {
            savedInventoryLeftClickButtonX = pButton->uX;
            savedInventoryLeftClickButtonY = pButton->uY;
            savedInventoryLeftClickButtonZ = pButton->uZ;
            savedInventoryLeftClickButtonW = pButton->uW;
            pButton->uW = 0;
            pButton->uZ = 0;
            pButton->uY = 0;
            pButton->uX = 0;
        }
        buttons_count++;
    }
    int first_rows = 0;
    Character *curr_player = &pParty->activeCharacter();

    int uCurrFontHeght = assets->pFontLucida->GetHeight();
    int current_Y = 2 * uCurrFontHeght + 13;
    int width = 204;
    for (Skill skill : allWeaponSkills()) {
        if (curr_player->getSkillValue(skill).level()) {
            current_Y += uCurrFontHeght - 3;
            ++buttons_count;
            ++first_rows;
            unsigned skill_id = std::to_underlying(skill);
            pGUIWindow_CurrentMenu->CreateButton({24, current_Y}, {width, uCurrFontHeght - 3}, 3, skill_id | 0x8000, UIMSG_SkillUp, skill_id);
        }
    }
    if (!first_rows) current_Y += uCurrFontHeght - 3;
    current_Y += 2 * uCurrFontHeght - 6;
    for (Skill skill : allMagicSkills()) {
        if (curr_player->getSkillValue(skill).level() /*&& buttons_count < 15*/) {
            current_Y += uCurrFontHeght - 3;
            ++buttons_count;
            unsigned skill_id = std::to_underlying(skill);
            pGUIWindow_CurrentMenu->CreateButton({24, current_Y}, {width, uCurrFontHeght - 3}, 3, skill_id | 0x8000, UIMSG_SkillUp, skill_id);
        }
    }
    first_rows = 0;
    current_Y = 2 * uCurrFontHeght + 13;
    for (Skill skill : allArmorSkills()) {
        if (curr_player->getSkillValue(skill).level()) {
            current_Y += uCurrFontHeght - 3;
            ++buttons_count;
            ++first_rows;
            unsigned skill_id = std::to_underlying(skill);
            pGUIWindow_CurrentMenu->CreateButton({246, current_Y}, {width, uCurrFontHeght - 3}, 3, skill_id | 0x8000, UIMSG_SkillUp, skill_id);
        }
    }
    if (!first_rows) current_Y += uCurrFontHeght - 3;
    current_Y += 2 * uCurrFontHeght - 6;
    for (Skill skill : allMiscSkills()) {
        if (curr_player->getSkillValue(skill).level()) {
            current_Y += uCurrFontHeght - 3;
            ++buttons_count;
            unsigned skill_id = std::to_underlying(skill);
            pGUIWindow_CurrentMenu->CreateButton({246, current_Y}, {width, uCurrFontHeght - 3}, 3, skill_id | 0x8000, UIMSG_SkillUp, skill_id);
        }
    }
}

void GUIWindow_CharacterRecord::CharacterUI_StatsTab_Draw(Character *player) {
    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f,
                                ui_character_stats_background);

    auto str1 =
        fmt::format("{::}{}\f00000\r180{}: {::}{}\f00000\n\n\n",
                    ui_character_header_text_color.tag(),
                    NameAndTitle(player->name, player->classType),
                    localization->GetString(LSTR_SKILL_POINTS),
                    player->uSkillPoints ? ui_character_bonus_text_color.tag() : ui_character_default_text_color.tag(),
                    player->uSkillPoints);
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {26, 18}, colorTable.White, str1);

    // Left column
    auto formatLeftCol = [] (LstrId lstr, int current, int max) {
        Color color16 = UI_GetHealthManaAndOtherQualitiesStringColor(current, max);
        if (max < 1000) {
            return fmt::format("{}{::}\r424{}\f00000 /\t185{}\n", localization->GetString(lstr), color16.tag(), current, max);
        } else {
            return fmt::format("{}{::}\r388{}\f00000 / {}\n", localization->GetString(lstr), color16.tag(), current, max);
        }
    };

    int pY = 53;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {26, pY}, colorTable.White,
                                     formatLeftCol(LSTR_MIGHT, player->GetActualMight(), player->GetBaseMight()));

    pY += assets->pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {26, pY}, colorTable.White,
                                     formatLeftCol(LSTR_INTELLECT, player->GetActualIntelligence(), player->GetBaseIntelligence()));

    pY += assets->pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {26, pY}, colorTable.White,
                                     formatLeftCol(LSTR_PERSONALITY, player->GetActualPersonality(), player->GetBasePersonality()));

    pY += assets->pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {26, pY}, colorTable.White,
                                     formatLeftCol(LSTR_ENDURANCE, player->GetActualEndurance(), player->GetBaseEndurance()));

    pY += assets->pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {26, pY}, colorTable.White,
                                     formatLeftCol(LSTR_ACCURACY, player->GetActualAccuracy(), player->GetBaseAccuracy()));

    pY += assets->pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {26, pY}, colorTable.White,
                                     formatLeftCol(LSTR_SPEED, player->GetActualSpeed(), player->GetBaseSpeed()));

    pY += assets->pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {26, pY}, colorTable.White,
                                     formatLeftCol(LSTR_LUCK, player->GetActualLuck(), player->GetBaseLuck()));

    pY += 2 * assets->pFontArrus->GetHeight() + 5;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {26, pY}, colorTable.White,
                                     formatLeftCol(LSTR_HIT_POINTS, player->health, player->GetMaxHealth()));

    pY += assets->pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {26, pY}, colorTable.White,
                                     formatLeftCol(LSTR_SPELL_POINTS, player->mana, player->GetMaxMana()));

    pY += assets->pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {26, pY}, colorTable.White,
                                     formatLeftCol(LSTR_ARMOR_CLASS, player->GetActualAC(), player->GetBaseAC()));

    pY += 2 * assets->pFontArrus->GetHeight() - 2;
    auto str12 =
        fmt::format("{}: {::}{}\n",
                     localization->GetString(LSTR_CONDITION),
                     GetConditionDrawColor(player->GetMajorConditionIdx()).tag(),
                     localization->GetCharacterConditionName(player->GetMajorConditionIdx()));
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {26, pY}, colorTable.White, str12, 226, 0);

    pY += assets->pFontArrus->GetHeight() + -1;
    std::string spellName = localization->GetString(LSTR_NONE);
    if (player->uQuickSpell != SPELL_NONE)
        spellName = pSpellStats->pInfos[player->uQuickSpell].pShortName;
    auto str13 = fmt::format("{}: {}", localization->GetString(LSTR_QUICK_SPELL), spellName);
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontArrus.get(), {26, pY}, colorTable.White, str13, 226, 0);

    // Right column
    auto formatRightCol = [] (LstrId lstr, int current, int max, bool immune = false) {
        Color color16 = UI_GetHealthManaAndOtherQualitiesStringColor(current, max);
        if (immune) {
            return fmt::format("{}{::}\r180{}\n", localization->GetString(lstr), color16.tag(), localization->GetString(LSTR_IMMUNE));
        } else {
            if (current < 100 && max < 100) {
                return fmt::format("{}{::}\t110{}\f00000 / {}\n", localization->GetString(lstr), color16.tag(), current, max);
            } else {
                return fmt::format("{}{::}\r180{}\f00000 / {}\n", localization->GetString(lstr), color16.tag(), current, max);
            }
        }
    };

    pY = 50;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {266, pY}, colorTable.White,
                                     formatRightCol(LSTR_AGE, player->GetActualAge(), player->GetBaseAge()));

    pY += assets->pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {266, pY}, colorTable.White,
                                     formatRightCol(LSTR_LEVEL, player->GetActualLevel(), player->GetBaseLevel()));

    pY += assets->pFontArrus->GetHeight() - 2;
    auto str16 =
        fmt::format("{}\r180{::}{}\f00000\n\n",
                    localization->GetString(player->experience <= 9999999 ? LSTR_EXPERIENCE : LSTR_EXP),
                    player->GetExperienceDisplayColor().tag(), player->experience);
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {266, pY}, colorTable.White, str16);

    pY += 2 * assets->pFontArrus->GetHeight();
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {266, pY}, colorTable.White,
                                     fmt::format("{}\t100{:+}\n", localization->GetString(LSTR_ATTACK), player->GetActualAttack(false)));

    pY += assets->pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {266, pY}, colorTable.White,
                                     fmt::format("{}\t100 {}\n", localization->GetString(LSTR_DAMAGE), player->GetMeleeDamageString()));

    pY += assets->pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {266, pY}, colorTable.White,
                                     fmt::format("{}\t100{:+}\n", localization->GetString(LSTR_SHOOT), player->GetRangedAttack()));

    pY += assets->pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {266, pY}, colorTable.White,
                                     fmt::format("{}\t100 {}\n\n", localization->GetString(LSTR_DAMAGE), player->GetRangedDamageString()));

    pY += 2 * assets->pFontArrus->GetHeight() - 4;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {266, pY}, colorTable.White,
                                     formatRightCol(LSTR_FIRE, player->GetActualResistance(ATTRIBUTE_RESIST_FIRE),
                                                    player->GetBaseResistance(ATTRIBUTE_RESIST_FIRE)));

    pY += assets->pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {266, pY}, colorTable.White,
                                     formatRightCol(LSTR_AIR, player->GetActualResistance(ATTRIBUTE_RESIST_AIR),
                                                    player->GetBaseResistance(ATTRIBUTE_RESIST_AIR)));

    pY += assets->pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {266, pY}, colorTable.White,
                                     formatRightCol(LSTR_WATER, player->GetActualResistance(ATTRIBUTE_RESIST_WATER),
                                                    player->GetBaseResistance(ATTRIBUTE_RESIST_WATER)));

    pY += assets->pFontArrus->GetHeight() - 2;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {266, pY}, colorTable.White,
                                     formatRightCol(LSTR_EARTH, player->GetActualResistance(ATTRIBUTE_RESIST_EARTH),
                                                    player->GetBaseResistance(ATTRIBUTE_RESIST_EARTH)));

    pY += assets->pFontArrus->GetHeight() - 2;
    bool immuneToMind = player->classType == CLASS_LICH && player->GetBaseResistance(ATTRIBUTE_RESIST_MIND) == 200;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {266, pY}, colorTable.White,
                                     formatRightCol(LSTR_MIND, player->GetActualResistance(ATTRIBUTE_RESIST_MIND),
                                                    player->GetBaseResistance(ATTRIBUTE_RESIST_MIND), immuneToMind));

    pY += assets->pFontArrus->GetHeight() - 2;
    bool immuneToBody = player->classType == CLASS_LICH && player->GetBaseResistance(ATTRIBUTE_RESIST_BODY) == 200;
    pGUIWindow_CurrentMenu->DrawText(assets->pFontArrus.get(), {266, pY}, colorTable.White,
                                     formatRightCol(LSTR_BODY, player->GetActualResistance(ATTRIBUTE_RESIST_BODY),
                                                    player->GetBaseResistance(ATTRIBUTE_RESIST_BODY), immuneToBody));
}

void GUIWindow_CharacterRecord::fillAwardsData() {
    Character *pPlayer = &pParty->activeCharacter();

    _awardsCharacterId = pParty->activeCharacterIndex();
    _startAwardElem = 0;
    _scrollableAwardSteps = 0;
    _awardLimitReached = false;

    _achievedAwardsList.clear();
    for (int i = 1; i < pAwards.size(); ++i) {
        if (pPlayer->_achievedAwardsBits[i] && !pAwards[i].pText.empty()) {
            _achievedAwardsList.push_back(i);
        }
    }

    std::stable_sort(_achievedAwardsList.begin(), _achievedAwardsList.end(), [&] (int a, int b) { return pAwards[a].uPriority < pAwards[b].uPriority; });

    GUIWindow window = prepareAwardsWindow();
    int y = 0;

    for (int i = (_achievedAwardsList.size() - 1); i >= 0; --i) {
        std::string str = getAchievedAwardsString(i);

        y += assets->pFontArrus->CalcTextHeight(str, window.uFrameWidth, 0) + 8;
        if (y > window.uFrameHeight) {
            _scrollableAwardSteps = i + 2;
            break;
        }
    }
}

void WetsuitOn(int uPlayerID) {
    if (uPlayerID > 0) {
        int playerId0 = uPlayerID - 1;
        Character *player = &pParty->pCharacters[playerId0];
        int texture_num;

        if (player->GetRace() == RACE_DWARF) {
            texture_num = (player->GetSexByVoice() != SEX_MALE) + 3;
        } else {
            texture_num = (player->GetSexByVoice() != SEX_MALE) + 1;
        }
        paperdoll_dbods[playerId0] = assets->getImage_Alpha(fmt::format("pc23v{}Bod", texture_num));  // Body texture
        paperdoll_dlads[playerId0] = assets->getImage_Alpha(fmt::format("pc23v{}lad", texture_num));  // Left Hand
        paperdoll_dlaus[playerId0] = assets->getImage_Alpha(fmt::format("pc23v{}lau", texture_num));  // Left Hand2
        paperdoll_drhs[playerId0] = assets->getImage_Alpha(fmt::format("pc23v{}rh", texture_num));  // Right Hand
        paperdoll_dlhs[playerId0] = assets->getImage_Alpha(fmt::format("pc23v{}lh", texture_num));  // Left Palm
        paperdoll_dlhus[playerId0] = assets->getImage_Alpha(fmt::format("pc23v{}lhu", texture_num));  // Left Fist

        if (player->uCurrentFace == 12 || player->uCurrentFace == 13) {
            paperdoll_dbrds[player->uCurrentFace] = nullptr;
        }
        paperdoll_flying_feet[player->uCurrentFace] = nullptr;
    }
}

void WetsuitOff(int uPlayerID) {
    if (uPlayerID > 0) {
        int playerId0 = uPlayerID - 1;
        Character *player = &pParty->pCharacters[playerId0];

        paperdoll_dbods[playerId0] = assets->getImage_Alpha(dbod_texnames_by_face[player->uCurrentFace]);
        paperdoll_dlads[playerId0] = assets->getImage_Alpha(dlad_texnames_by_face[player->uCurrentFace]);
        paperdoll_dlaus[playerId0] = assets->getImage_Alpha(dlau_texnames_by_face[player->uCurrentFace]);
        paperdoll_drhs[playerId0] = assets->getImage_Alpha(drh_texnames_by_face[player->uCurrentFace]);
        paperdoll_dlhs[playerId0] = assets->getImage_Alpha(dlh_texnames_by_face[player->uCurrentFace]);
        paperdoll_dlhus[playerId0] = assets->getImage_Alpha(dlhu_texnames_by_face[player->uCurrentFace]);

        if (player->uCurrentFace == 12 || player->uCurrentFace == 13) {
            paperdoll_dbrds[player->uCurrentFace] = assets->getImage_Alpha(fmt::format("pc{:02}brd", player->uCurrentFace + 1));
        }

        paperdoll_flying_feet[player->uCurrentFace] = assets->getImage_Alpha(fmt::format("item281pc{:02}", player->uCurrentFace + 1));
    }
}

//----- (00468F8A) --------------------------------------------------------
void OnPaperdollLeftClick() {
    int mousex = mouse->position().x;
    int mousey = mouse->position().y;

    static int RingsX[6] = {0x1EA, 0x21A, 0x248, 0x1EA, 0x21A, 0x248};
    static int RingsY[6] = {0x0CA, 0x0CA, 0x0CA, 0x0FA, 0x0FA, 0x0FA};

    static int glovex = 586;
    static int glovey = 88;

    static int amuletx = 493;
    static int amulety = 91;

    int cellSize = 32;
    ItemSlot pos = ITEM_SLOT_INVALID;

    // uint16_t v5; // ax@7
    // int equippos; // esi@27
    // int v8; // eax@29
    Skill pSkillType = SKILL_INVALID;

    ItemType pEquipType = ITEM_TYPE_INVALID;
    CastSpellInfo *pSpellInfo;

    InventoryEntry twohandedequip;
    Item _this;  // [sp+Ch] [bp-40h]@1
    _this.Reset();
    InventoryEntry mainhandequip = pParty->activeCharacter().inventory.entry(ITEM_SLOT_MAIN_HAND);
    InventoryEntry shieldequip = pParty->activeCharacter().inventory.entry(ITEM_SLOT_OFF_HAND);

    if (mainhandequip && mainhandequip->type() == ITEM_TYPE_TWO_HANDED) {
        twohandedequip = mainhandequip;
    }

    ItemId pickeditem = pParty->pPickedItem.itemId;

    if (pParty->pPickedItem.itemId != ITEM_NULL) {  // hold item
        pEquipType = pParty->pPickedItem.type();
        pSkillType = pParty->pPickedItem.skill();

        if (pSkillType == SKILL_SPEAR) {
            if (shieldequip) {
                // cant use spear in one hand till master
                if (pParty->activeCharacter().getActualSkillValue(SKILL_SPEAR).mastery() < MASTERY_MASTER) {
                    pParty->activeCharacter().playReaction(SPEECH_CANT_EQUIP);

                    return;
                }

                pickeditem = pParty->pPickedItem.itemId;
            }
        } else {
            if ((pSkillType == SKILL_SHIELD || pSkillType == SKILL_SWORD || pSkillType == SKILL_DAGGER) && mainhandequip &&
                mainhandequip->skill() == SKILL_SPEAR) {
                // cant use spear in one hand till master
                if (pParty->activeCharacter().getActualSkillValue(SKILL_SPEAR).mastery() < MASTERY_MASTER) {
                    pParty->activeCharacter().playReaction(SPEECH_CANT_EQUIP);
                    return;
                }
            }
        }

        if (!pParty->activeCharacter().CanEquip_RaceAndAlignmentCheck(pickeditem)) {  // special item checks
            pParty->activeCharacter().playReaction(SPEECH_CANT_EQUIP);
            return;
        }

        switch (pEquipType) {
            case ITEM_TYPE_BOW:
            case ITEM_TYPE_ARMOUR:
            case ITEM_TYPE_HELMET:
            case ITEM_TYPE_BELT:
            case ITEM_TYPE_CLOAK:
            case ITEM_TYPE_GAUNTLETS:
            case ITEM_TYPE_BOOTS:
            case ITEM_TYPE_AMULET: {
                if (!pParty->activeCharacter().HasSkill(pSkillType)) {  // hasnt got the skill to use that
                    pParty->activeCharacter().playReaction(SPEECH_CANT_EQUIP);
                    return;
                }

                if (pParty->activeCharacter().hasUnderwaterSuitEquipped() && (pEquipType != ITEM_TYPE_ARMOUR || engine->IsUnderwater())) {  // cant put anything on wearing wetsuit
                    pAudioPlayer->playUISound(SOUND_error);
                    return;
                }

                bool puttingWetsuitsOn = pParty->pPickedItem.itemId == ITEM_QUEST_WETSUIT;
                bool takingWetsuitsOff = false;

                assert(itemSlotsForItemType(pEquipType).size() == 1);
                ItemSlot slot = itemSlotsForItemType(pEquipType)[0];
                if (InventoryEntry entry = pParty->activeCharacter().inventory.entry(slot)) {
                    Item tmpItem = pParty->activeCharacter().inventory.take(entry);
                    takingWetsuitsOff = tmpItem.itemId == ITEM_QUEST_WETSUIT;
                    pParty->activeCharacter().inventory.equip(slot, pParty->takeHoldingItem());
                    pParty->setHoldingItem(tmpItem);
                } else {
                    // Need to check canEquip here b/c we can run out of inventory space. The branch above doesn't need
                    // this check b/c we're freeing space first.
                    if (pParty->activeCharacter().inventory.canEquip(slot)) {
                        pParty->activeCharacter().inventory.equip(slot, pParty->takeHoldingItem());
                    } else {
                        pAudioPlayer->playUISound(SOUND_error); // Out of inventory space.
                        return;
                    }
                }

                if (puttingWetsuitsOn)
                    WetsuitOn(pParty->activeCharacterIndex());
                if (takingWetsuitsOff)
                    WetsuitOff(pParty->activeCharacterIndex());

                return;
            }

                // ------------------------dress rings(одевание колец)----------------------------------
            case ITEM_TYPE_RING:
                if (pParty->activeCharacter().hasUnderwaterSuitEquipped()) {  // cant put anything
                                                                                        // on wearing wetsuit
                    pAudioPlayer->playUISound(SOUND_error);
                    return;
                }

                if (!bRingsShownInCharScreen) {  // rings not displayd
                    for (ItemSlot equippos : allRingSlots()) {
                        if (pParty->activeCharacter().inventory.canEquip(equippos)) {
                            pParty->activeCharacter().inventory.equip(equippos, pParty->takeHoldingItem());
                            return;
                        }
                    }

                    // cant fit rings so swap out
                    InventoryEntry freeslot = pParty->activeCharacter().inventory.entry(ITEM_SLOT_RING6);  // slot of last ring
                    if (!freeslot) {
                        pAudioPlayer->playUISound(SOUND_error); // Out of inventory space.
                        return;
                    }

                    Item tmp = pParty->activeCharacter().inventory.take(freeslot);
                    pParty->activeCharacter().inventory.equip(ITEM_SLOT_RING6, pParty->takeHoldingItem());
                    pParty->setHoldingItem(tmp);
                    return;

                } else {  // rings displayed if in ring area
                    for (int i = 0; i < 6; ++i) {
                        if (mousex >= RingsX[i] &&
                            mousex <= (RingsX[i] + cellSize) &&
                            mousey >= RingsY[i] &&
                            mousey <= (RingsY[i] + cellSize)) {  // check against ring slots
                            pos = ringSlot(i);
                        }
                    }

                    if (pos != ITEM_SLOT_INVALID) {  // we have a position to aim for
                        InventoryEntry entry = pParty->activeCharacter().inventory.entry(pos);
                        if (!entry) {  // no item in slot so just drop
                            if (pParty->activeCharacter().inventory.canEquip(pos)) {  // drop ring into free space
                                pParty->activeCharacter().inventory.equip(pos, pParty->takeHoldingItem());
                                return;
                            }
                        } else {  // item so swap out
                            Item tmp = pParty->activeCharacter().inventory.take(entry);
                            pParty->activeCharacter().inventory.equip(pos, pParty->takeHoldingItem());
                            pParty->setHoldingItem(tmp);
                            return;
                        }
                    } else {  // not click on right area so exit
                        return;
                    }

                    return;  // shouldnt get here but in case??
                }

                // ------------------dress shield(одеть щит)------------------------------------------------------
            case ITEM_TYPE_SHIELD:  //Щит
                if (pParty->activeCharacter().hasUnderwaterSuitEquipped()) {  // в акваланге
                    pAudioPlayer->playUISound(SOUND_error);
                    return;
                }
                if (!pParty->activeCharacter().HasSkill(pSkillType)) {  // нет навыка
                    pParty->activeCharacter().playReaction(SPEECH_CANT_EQUIP);
                    return;
                }
                if (shieldequip) {  // смена щита щитом
                    Item tmp = pParty->activeCharacter().inventory.take(shieldequip);
                    pParty->activeCharacter().inventory.equip(ITEM_SLOT_OFF_HAND, pParty->takeHoldingItem());
                    pParty->setHoldingItem(tmp);
                    if (!twohandedequip) {
                        return;
                    }
                } else {
                    if (!pParty->activeCharacter().inventory.canEquip(ITEM_SLOT_OFF_HAND)) {
                        pAudioPlayer->playUISound(SOUND_error); // Out of inventory space.
                        return;
                    }
                    if (!twohandedequip) {  // обычная установка щита на пустую руку
                        pParty->activeCharacter().inventory.equip(ITEM_SLOT_OFF_HAND, pParty->takeHoldingItem());
                        return;
                    }
                    // ставим щит когда держит двуручный меч
                    Item tmp = pParty->activeCharacter().inventory.take(twohandedequip);
                    pParty->activeCharacter().inventory.equip(ITEM_SLOT_OFF_HAND, pParty->takeHoldingItem());
                    pParty->setHoldingItem(tmp);
                }
                return;
                // -------------------------taken in hand(взять в руку)-------------------------------------------
            case ITEM_TYPE_SINGLE_HANDED:
            case ITEM_TYPE_WAND:
                if (pParty->activeCharacter().hasUnderwaterSuitEquipped() && !isAncientWeapon(pParty->pPickedItem.itemId)) {
                    pAudioPlayer->playUISound(SOUND_error);
                    return;
                }
                if (!pParty->activeCharacter().HasSkill(pSkillType)) {
                    pParty->activeCharacter().playReaction(SPEECH_CANT_EQUIP);
                    return;
                }
                // dagger at expert or sword at master in left hand
                if (pSkillType == SKILL_DAGGER && (pParty->activeCharacter().getActualSkillValue(SKILL_DAGGER).mastery() >= MASTERY_EXPERT)
                    || pSkillType == SKILL_SWORD && (pParty->activeCharacter().getActualSkillValue(SKILL_SWORD).mastery() >= MASTERY_MASTER)) {
                    if (mouse->position().x >= 560) {
                        if (!twohandedequip) {
                            if (shieldequip) {
                                Item tmp = pParty->activeCharacter().inventory.take(shieldequip);
                                pParty->activeCharacter().inventory.equip(ITEM_SLOT_OFF_HAND, pParty->takeHoldingItem());
                                pParty->setHoldingItem(tmp);
                                if (pEquipType != ITEM_TYPE_WAND) {
                                    return;
                                }
                                break;
                            }
                            if (!pParty->activeCharacter().inventory.canEquip(ITEM_SLOT_OFF_HAND)) {
                                pAudioPlayer->playUISound(SOUND_error); // Out of inventory space.
                                return;
                            }
                            pParty->activeCharacter().inventory.equip(ITEM_SLOT_OFF_HAND, pParty->takeHoldingItem());
                            if (pEquipType != ITEM_TYPE_WAND) return;
                            break;
                        }
                    }
                }
                if (!mainhandequip) {
                    if (!pParty->activeCharacter().inventory.canEquip(ITEM_SLOT_MAIN_HAND)) {
                        pAudioPlayer->playUISound(SOUND_error); // Out of inventory space.
                        return;
                    }
                    pParty->activeCharacter().inventory.equip(ITEM_SLOT_MAIN_HAND, pParty->takeHoldingItem());
                    if (pEquipType != ITEM_TYPE_WAND) return;
                    break;
                }

                {
                    Item tmp = pParty->activeCharacter().inventory.take(mainhandequip);
                    pParty->activeCharacter().inventory.equip(ITEM_SLOT_MAIN_HAND, pParty->takeHoldingItem());
                    pParty->setHoldingItem(tmp);
                    break;
                }
                // ---------------------------take two hands(взять двумя
                // руками)---------------------------------
            case ITEM_TYPE_TWO_HANDED:
                if (pParty->activeCharacter().hasUnderwaterSuitEquipped()) {
                    pAudioPlayer->playUISound(SOUND_error);
                    return;
                }
                if (!pParty->activeCharacter().HasSkill(pSkillType)) {
                    pParty->activeCharacter().playReaction(SPEECH_CANT_EQUIP);
                    return;
                }
                if (mainhandequip) {  // взять двуручный меч когда нет
                                      // щита(замещение оружия)
                    if (shieldequip) {
                        pAudioPlayer->playUISound(SOUND_error);
                        return;
                    }
                    Item tmp = pParty->activeCharacter().inventory.take(mainhandequip);
                    pParty->activeCharacter().inventory.equip(ITEM_SLOT_MAIN_HAND, pParty->takeHoldingItem());
                    pParty->setHoldingItem(tmp);
                } else {
                    if (shieldequip) {
                        Item tmp = pParty->activeCharacter().inventory.take(shieldequip);
                        pParty->activeCharacter().inventory.equip(ITEM_SLOT_MAIN_HAND, pParty->takeHoldingItem());
                        pParty->setHoldingItem(tmp);
                    } else if (pParty->activeCharacter().inventory.canEquip(ITEM_SLOT_MAIN_HAND)) {
                        pParty->activeCharacter().inventory.equip(ITEM_SLOT_MAIN_HAND, pParty->takeHoldingItem());
                    } else {
                        pAudioPlayer->playUISound(SOUND_error); // Out of inventory space.
                        return;
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

        if (mousex >= amuletx && mousex <= (amuletx + cellSize) &&
            mousey >= amulety && mousey <= (amulety + 2 * cellSize)) {
            // amulet
            // pitem = pParty->activeCharacter().GetAmuletItem(); //9
            pos = ITEM_SLOT_AMULET;
        }

        if (mousex >= glovex && mousex <= (glovex + cellSize) && mousey >= glovey &&
            mousey <= (glovey + 2 * cellSize)) {
            // glove
            // pitem = pParty->activeCharacter().GetGloveItem(); //7
            pos = ITEM_SLOT_GAUNTLETS;
        }

        for (int i = 0; i < 6; ++i) {
            if (mousex >= RingsX[i] && mousex <= (RingsX[i] + cellSize) &&
                mousey >= RingsY[i] && mousey <= (RingsY[i] + cellSize)) {
                // ring
                // pitem = pParty->activeCharacter().GetNthRingItem(i); //10+i
                pos = ringSlot(i);
            }
        }

        InventoryEntry entry;
        if (pos != ITEM_SLOT_INVALID)
            entry = pParty->activeCharacter().inventory.entry(pos);

        if (!entry) return;
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
            pSpellInfo = pGUIWindow_CastTargetedSpell->spellInfo();
            pSpellInfo->flags &= ~ON_CAST_TargetedEnchantment;
            pSpellInfo->targetCharacterIndex = pParty->activeCharacterIndex() - 1;
            pSpellInfo->targetInventoryIndex = entry.index();

            ptr_50C9A4_ItemToEnchant = entry.get();
            IsEnchantingInProgress = false;
            engine->_messageQueue->clear();
            mouse->SetCursorImage("MICON1");
            AfterEnchClickEventId = UIMSG_Escape;
            AfterEnchClickEventSecondParam = 0;
            AfterEnchClickEventTimeout = Duration::fromRealtimeSeconds(2);
        } else {
            if (!ptr_50C9A4_ItemToEnchant) {  // снять вещь
                pParty->setHoldingItem(pParty->activeCharacter().inventory.take(entry));

                // pParty->setHoldingItem(&pParty->activeCharacter().pInventoryItemList[v34
                // - 1]);
                //  pParty->activeCharacter().pEquipment[pParty->activeCharacter().pInventoryItemList[v34
                //  - 1].uBodyAnchor - 1] = 0;
                //  pParty->activeCharacter().pInventoryItemList[v34 -
                //  1].Reset();

                // return
                // &this->pInventoryItemList[this->pEquipment[index] -
                // 1];
            }
        }

        // for (unsigned i = 0; i < 6; ++i)
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
        int v34 = render->QueryHitMap(mouse->position(), -1);
        InventoryEntry entry = pParty->activeCharacter().inventory.entry(v34);

        if (entry) {
            // v36 = v34 - 1;
            // v38 = &pCharacters[pParty->_activeCharacter]->pInventoryItemList[v34 - 1];
            pEquipType = entry->type();
            if (entry->itemId == ITEM_QUEST_WETSUIT) {
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
                pSpellInfo = pGUIWindow_CastTargetedSpell->spellInfo();
                pSpellInfo->flags &= ~ON_CAST_TargetedEnchantment;
                pSpellInfo->targetCharacterIndex = pParty->activeCharacterIndex() - 1;
                pSpellInfo->targetInventoryIndex = v34 - 1;

                ptr_50C9A4_ItemToEnchant = entry.get();
                IsEnchantingInProgress = false;
                engine->_messageQueue->clear();
                mouse->SetCursorImage("MICON1");
                AfterEnchClickEventId = UIMSG_Escape;
                AfterEnchClickEventSecondParam = 0;
                AfterEnchClickEventTimeout = Duration::fromRealtimeSeconds(2);
            } else {
                if (!ptr_50C9A4_ItemToEnchant) {  // снять вещь
                    pParty->setHoldingItem(pParty->activeCharacter().inventory.take(entry));
                }
            }
        } else {  // снять лук
            if (InventoryEntry entry = pParty->activeCharacter().inventory.entry(ITEM_SLOT_BOW)) {
                pParty->setHoldingItem(pParty->activeCharacter().inventory.take(entry));
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
                pButton->uX = savedInventoryLeftClickButtonX;
                pButton->uY = savedInventoryLeftClickButtonY;
                pButton->uZ = savedInventoryLeftClickButtonZ;
                pButton->uW = savedInventoryLeftClickButtonW;
            }
        }
    }
}

bool ringscreenactive() { return bRingsShownInCharScreen; }
