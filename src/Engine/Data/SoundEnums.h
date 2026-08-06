#pragma once

#include <cassert>
#include <cstdint>
#include <utility>

#include "Utility/Flags.h"

/**
 * There's 2000+ sounds in MM7, and there is little point in adding all of them here via codegen. Only the ids that are
 * actually used in the codebase should be kept in this enum.
 */
enum class SoundId : int16_t {
    SOUND_Invalid = 0,
    SOUND_enter = 6,
    SOUND_WoodDoorClosing = 7,
    SOUND_fireBall = 8,
    SOUND_BoatCreaking = 18,
    SOUND_ClickMinus = 20,
    SOUND_ClickMovingSelector = 21,
    SOUND_ClickPlus = 23,
    SOUND_ClickSkill = 24,
    SOUND_error = 27,
    SOUND_dull_strike = 44,
    SOUND_metal_vs_metal01h = 45,
    SOUND_metal_vs_metal03h = 47,
    SOUND_48 = 48,
    SOUND_RunBadlands = 49,
    SOUND_RunCarpet = 50,
    SOUND_RunCooledLava = 51,
    SOUND_RunDesert = 52,
    SOUND_RunDirt = 53,
    SOUND_RunGrass = 54,
    SOUND_Run55_94 = 55, // sound like some ground-like surface
    SOUND_RunGround = 56,
    SOUND_RunRoad = 57,
    SOUND_RunSnow = 58,
    SOUND_Run59_98 = 59, // sound like road walk but duller
    SOUND_Run60_99 = 60, // sound like indoor stone walk with echo added
    SOUND_RunSwamp = 61,
    SOUND_RunWater = 62,
    SOUND_RunWaterIndoor = 63,
    SOUND_RunWood = 64,
    SOUND_Run65_104 = 65, // sound like wood walk but thinner
    SOUND_SelectingANewCharacter = 66,
    SOUND_shoot_blaster01 = 67,
    SOUND_shoot_bow01 = 71,
    SOUND_StartMainChoice02 = 75,
    SOUND_swing_with_axe01 = 78,
    SOUND_swing_with_axe03 = 80,
    SOUND_swing_with_blunt_weapon01 = 81,
    SOUND_swing_with_blunt_weapon03 = 83,
    SOUND_swing_with_sword01 = 84,
    SOUND_swing_with_sword02 = 85,
    SOUND_WalkBadlands = 88,
    SOUND_WalkCarpet = 89,
    SOUND_WalkCooledLava = 90,
    SOUND_WalkDesert = 91,
    SOUND_WalkDirt = 92,
    SOUND_WalkGrass = 93,
    SOUND_Walk55_94 = 94, // sound like some ground-like surface
    SOUND_WalkGround = 95,
    SOUND_WalkRoad = 96,
    SOUND_WalkSnow = 97,
    SOUND_Walk59_98 = 98, // sound like road walk but duller
    SOUND_Walk60_99 = 99, // sound like indoor stone walk with echo added
    SOUND_WalkSwamp = 100,
    SOUND_WalkWater = 101,
    SOUND_WalkWaterIndoor = 102,
    SOUND_WalkWood = 103,
    SOUND_Walk65_104 = 104, // sound like wood walk but thinner
    SOUND_metal_armor_strike1 = 105,
    SOUND_metal_armor_strike2 = 106,
    SOUND_metal_armor_strike3 = 107,
    SOUND_dull_armor_strike1 = 108,
    SOUND_dull_armor_strike2 = 109,
    SOUND_dull_armor_strike3 = 110,
    SOUND_bricks_down = 120,
    SOUND_bricks_up = 121,
    SOUND_damage = 122,
    SOUND_deal = 123,
    SOUND_defeat = 124,
    SOUND_querry_up = 125,
    SOUND_querry_down = 126,
    SOUND_shuffle = 127,
    SOUND_title = 128,
    SOUND_tower_up = 129,
    SOUND_typing = 130,
    SOUND_victory = 131,
    SOUND_wall_up = 132,
    SOUND_luteguitar = 133,
    SOUND_panflute = 134,
    SOUND_trumpet = 135,
    SOUND_gold01 = 200,
    SOUND_heal = 202,
    SOUND_fizzle = 203,
    SOUND_TurnPage1 = 204,
    SOUND_TurnPage2 = 205,
    SOUND_EndTurnBasedMode = 206,
    SOUND_StartTurnBasedMode = 207,
    SOUND_openchest0101 = 208,
    SOUND_spellfail0201 = 209,
    SOUND_drink = 210,
    SOUND_eat = 211,
    SOUND_gong = 215,
    SOUND_hurp = 217,
    SOUND_church = 218,
    SOUND_chimes = 219,
    SOUND_splash = 220,
    SOUND_star1 = 221,
    SOUND_star2 = 222,
    SOUND_star4 = 224,
    SOUND_eradicate = 225,
    SOUND_eleccircle = 226,
    SOUND_encounter = 227,
    SOUND_openbook = 230,
    SOUND_closebook = 231,
    SOUND_teleport = 232,
    SOUND_wood_door0101 = 300,
    SOUND_wood_door0201 = 302,
    SOUND_wood_door0301 = 304,
    SOUND_wood_door0401 = 306,
    SOUND_wood_door0501 = 308,
    SOUND_stone_door0101 = 400,
    SOUND_stone_door0201 = 402,
    SOUND_stone_door0301 = 404,
    SOUND_stone_door0401 = 406,
    SOUND_stone_door0501 = 408,
    SOUND_hf445a = 5788,
    SOUND_Haste = 10040,
    SOUND_21fly03 = 11090,
    SOUND_WaterWalk = 12040,
    SOUND_Stoneskin = 13040,
    SOUND_Bless = 14010,
    SOUND_Fate = 14020,
    SOUND_51heroism03 = 14060,
    SOUND_RechargeItem = 17020,
    SOUND_94dayofprotection03 = 17070,
    SOUND_9armageddon01 = 17080,
    SOUND_Sacrifice2 = 18060,
    SOUND_quest = 20001,
};
using enum SoundId;

inline SoundId doorClosedSound(SoundId doorSound) {
    assert(doorSound == SOUND_wood_door0101 || doorSound == SOUND_wood_door0201 || doorSound == SOUND_wood_door0301 ||
           doorSound == SOUND_wood_door0401 || doorSound == SOUND_wood_door0501 || doorSound == SOUND_stone_door0101 ||
           doorSound == SOUND_stone_door0201 || doorSound == SOUND_stone_door0301 || doorSound == SOUND_stone_door0401 ||
           doorSound == SOUND_stone_door0501);
    return static_cast<SoundId>(std::to_underlying(doorSound) + 1);
}

/**
 * Enum value is used to load mp3 files, which are named `2.mp3`-`20.mp3`.
 */
enum class MusicId {
    MUSIC_INVALID = 0,
    MUSIC_DEYJA_BRACADA_DESERT = 2,
    MUSIC_DUNGEON = 3,                           // Most of the game dungeons.
    MUSIC_HARMONDALE_TATALIA_AVLEE = 4,
    MUSIC_BARROWS = 5,                           // Barrows I-XV & Zokarr's Tomb.
    MUSIC_EVENMORN_ISLAND = 6,
    MUSIC_PIT_CASTLE_GLOAMING = 7,
    MUSIC_LINCOLN_ARENA = 8,
    MUSIC_SHOALS = 9,
    MUSIC_CELESTE = 10,
    MUSIC_MOUNT_NIGHON = 11,
    MUSIC_BARROW_DOWNS_LAND_OF_THE_GIANTS = 12,
    MUSIC_TEMPLES = 13,                          // The Temple of the Moon, Temple of the Light, Temple of the Dark, Grand Temple of the Moon, Grand Temple of the Sun, The Temple of Baa.
    MUSIC_CASTLE_GRYPHONHEART_CASTLE_NAVAN = 14,
    MUSIC_CASTLE_LAMBENT = 15,
    MUSIC_ENDGAME_DUNGEON = 16,                  // The Dragon Caves, Thunderfist Mountain, The Titans' Stronghold, Tunnels to Eeofol.
    MUSIC_ERATHIA = 17,
    MUSIC_TULAREAN_FOREST = 18,
    MUSIC_CASTLE_HARMONDALE = 19,                // Lord Markham's Manor, The Bandit Caves, Castle Harmondale, Fort Riverstride, The School of Sorcery, Stone City, The Mercenary Guild, William Setag's Tower, The Strange Temple, The Small House.
    MUSIC_EMERALD_ISLAND = 20,

    MUSIC_MAIN_MENU = MUSIC_CASTLE_GRYPHONHEART_CASTLE_NAVAN,
    MUSIC_CREDITS = MUSIC_CASTLE_LAMBENT,
};
using enum MusicId;

enum class SoundType {
    SOUND_TYPE_LEVEL = 0,
    SOUND_TYPE_SYSTEM = 1,
    SOUND_TYPE_SWAP = 2,
    SOUND_TYPE_UNKNOWN = 3,
    SOUND_TYPE_LOCK = 4,
};
using enum SoundType;

enum class SoundFlag {
    SOUND_FLAG_LOCKED = 0x1,
    SOUND_FLAG_3D = 0x2,
};
using enum SoundFlag;
MM_DECLARE_FLAGS(SoundFlags, SoundFlag)
MM_DECLARE_OPERATORS_FOR_FLAGS(SoundFlags)

enum class SoundPlaybackMode {
    /** Normal sound, requires a pid. */
    SOUND_MODE_PID,

    /** For generic UI sounds, plays independently of other sounds. */
    SOUND_MODE_UI,

    /** Playing the same sound id in this mode stops the already running one. */
    SOUND_MODE_EXCLUSIVE,

    /** Playing the same sound id in this mode just ignores the playback request. */
    SOUND_MODE_NON_RESETTABLE,

    /** Only one walk sound can be playing at a time. Playing a new one stops the previous one. */
    SOUND_MODE_WALKING,

    /** Same as `SOUND_MODE_EXCLUSIVE`, but played back at music volume. */
    SOUND_MODE_MUSIC,

    /** Same as `SOUND_MODE_EXCLUSIVE`, but played back at speech volume. */
    SOUND_MODE_SPEECH,

    /** House door sounds, only one sound can be active at a time. */
    SOUND_MODE_HOUSE_DOOR,

    /** House speech, only one sound can be active at a time. */ // TODO(captainurist): played back at speech volume?
    SOUND_MODE_HOUSE_SPEECH,
};
using enum SoundPlaybackMode;

enum class SoundPlaybackResult {
    SOUND_PLAYBACK_INVALID,
    SOUND_PLAYBACK_FAILED,
    SOUND_PLAYBACK_SKIPPED,
    SOUND_PLAYBACK_SUCCEEDED
};
using enum SoundPlaybackResult;
