#pragma once

#include <array>
#include <string>

#include "Engine/Graphics/FrameLimiter.h"

#include "Library/Geometry/Point.h"
#include "Library/Geometry/Rect.h"

#include "Library/Platform/Interface/PlatformEnums.h"

class GraphicsImage;

enum class ArcomageCheck {
    CHECK_ALWAYS_SECONDARY = 0,
    CHECK_ALWAYS_PRIMARY = 1,
    CHECK_LESSER_QUARRY = 2,
    CHECK_LESSER_MAGIC = 3,
    CHECK_LESSER_ZOO = 4,
    CHECK_EQUAL_QUARRY = 5,
    CHECK_EQUAL_MAGIC = 6,
    CHECK_EQUAL_ZOO = 7,
    CHECK_GREATER_QUARRY = 8,
    CHECK_GREATER_MAGIC = 9,
    CHECK_GREATER_ZOO = 10,
    CHECK_NO_WALL = 11,
    CHECK_HAVE_WALL = 12,
    CHECK_ENEMY_HAS_NO_WALL = 13,
    CHECK_ENEMY_HAS_WALL = 14,
    CHECK_LESSER_WALL = 15,
    CHECK_LESSER_TOWER = 16,
    CHECK_EQUAL_WALL = 17,
    CHECK_EQUAL_TOWER = 18,
    CHECK_GREATER_WALL = 19,
    CHECK_GREATER_TOWER = 20
};
using enum ArcomageCheck;

struct ArcomageCard {
    char pCardName[32];
    int32_t slot = 0;
    int8_t card_resource_type = 0;  // 1- brick, 2-gems, 3-beasts
    int8_t needed_quarry_level = 0;
    int8_t needed_magic_level = 0;
    int8_t needed_zoo_level = 0;
    int8_t needed_bricks = 0;
    int8_t needed_gems = 0;
    int8_t needed_beasts = 0;
    bool can_be_discarded = true;
    ArcomageCheck compare_param = CHECK_ALWAYS_PRIMARY;
    int8_t field_30 = 0;  // play again
    int8_t draw_extra_card_count = 0;
    int8_t to_player_quarry_lvl = 0;
    int8_t to_player_magic_lvl = 0;
    int8_t to_player_zoo_lvl = 0;
    int8_t to_player_bricks = 0;
    int8_t to_player_gems = 0;
    int8_t to_player_beasts = 0;
    int8_t to_player_buildings = 0;
    int8_t to_player_wall = 0;
    int8_t to_player_tower = 0;
    int8_t to_enemy_quarry_lvl = 0;
    int8_t to_enemy_magic_lvl = 0;
    int8_t to_enemy_zoo_lvl = 0;
    int8_t to_enemy_bricks = 0;
    int8_t to_enemy_gems = 0;
    int8_t to_enemy_beasts = 0;
    int8_t to_enemy_buildings = 0;
    int8_t to_enemy_wall = 0;
    int8_t to_enemy_tower = 0;
    int8_t to_pl_enm_quarry_lvl = 0;
    int8_t to_pl_enm_magic_lvl = 0;
    int8_t to_pl_enm_zoo_lvl = 0;
    int8_t to_pl_enm_bricks = 0;
    int8_t to_pl_enm_gems = 0;
    int8_t to_pl_enm_beasts = 0;
    int8_t to_pl_enm_buildings = 0;
    int8_t to_pl_enm_wall = 0;
    int8_t to_pl_enm_tower = 0;
    int8_t field_4D = 0;  // play again 2
    int8_t can_draw_extra_card2 = 0;
    int8_t to_player_quarry_lvl2 = 0;
    int8_t to_player_magic_lvl2 = 0;
    int8_t to_player_zoo_lvl2 = 0;
    int8_t to_player_bricks2 = 0;
    int8_t to_player_gems2 = 0;
    int8_t to_player_beasts2 = 0;
    int8_t to_player_buildings2 = 0;
    int8_t to_player_wall2 = 0;
    int8_t to_player_tower2 = 0;
    int8_t to_enemy_quarry_lvl2 = 0;
    int8_t to_enemy_magic_lvl2 = 0;
    int8_t to_enemy_zoo_lvl2 = 0;
    int8_t to_enemy_bricks2 = 0;
    int8_t to_enemy_gems2 = 0;
    int8_t to_enemy_beasts2 = 0;
    int8_t to_enemy_buildings2 = 0;
    int8_t to_enemy_wall2 = 0;
    int8_t to_enemy_tower2 = 0;
    int8_t to_pl_enm_quarry_lvl2 = 0;
    int8_t to_pl_enm_magic_lvl2 = 0;
    int8_t to_pl_enm_zoo_lvl2 = 0;
    int8_t to_pl_enm_bricks2 = 0;
    int8_t to_pl_enm_gems2 = 0;
    int8_t to_pl_enm_beasts2 = 0;
    int8_t to_pl_enm_buildings2 = 0;
    int8_t to_pl_enm_wall2 = 0;
    int8_t to_pl_enm_tower2 = 0;
    int8_t field_6A = 0;  // unused??
    int8_t field_6B = 0;  // unused??
};

struct AcromageCardOnTable {
    int uCardId = 0;
    int discarded = 0;
    Pointi table_pos;
    Pointi hide_anim_spd;
    Pointi hide_anim_pos;
};

struct ArcomagePlayer {
    std::string pPlayerName;
    int IsHisTurn = 0;  // doesnt appear to be used correctly - always player 0 turn
    int tower_height = 0;
    int wall_height = 0;
    int quarry_level = 0;
    int magic_level = 0;
    int zoo_level = 0;
    int resource_bricks = 0;
    int resource_gems = 0;
    int resource_beasts = 0;
    int cards_at_hand[10] {}; // TODO(pskelton): make vector ?
    Pointi card_shift[10] {};
};

enum class ArcomageMessageType {
    ARCO_MSG_NULL,

    ARCO_MSG_ESCAPE,
    ARCO_MSG_PLAYCARD,
    ARCO_MSG_DISCARD,
    ARCO_MSG_LEFT,
    ARCO_MSG_RIGHT,
};
using enum ArcomageMessageType;

struct ArcomageGame_InputMSG {
    ArcomageMessageType am_input_type{ ARCO_MSG_NULL };
    PlatformKey am_input_key{ PlatformKey::KEY_NONE };
};

class GUIFont;

struct ArcomageGame {
    ArcomageGame() {}

    static bool LoadSprites();
    static bool MsgLoop(int a1, ArcomageGame_InputMSG *a2);
    static void playSound(int event_id);
    static void onKeyPress(PlatformKey key);
    static void OnMouseClick(char right_left, bool bDown);
    static void OnMouseMove(const Pointi& pos);
    static void GetCardRect(int uCardID, Recti *pCardRect);
    static void PrepareArcomage();
    static void Loop();

    FrameLimiter _frameLimiter;
    int _targetFPS = 32;

    ArcomageGame_InputMSG _amMsg;
    Pointi _mousePos;

    GUIFont *pfntComic = nullptr;  // ptr_48;
    GUIFont *pfntArrus = nullptr;  // ptr_4C;

    GraphicsImage *pGameBackground = nullptr;
    GraphicsImage *pSprites = nullptr;

    int _gameWinner = 0; // 0 = none, 1 = player, 2 = enemy
    int _victoryType = 0;
    std::string _player1Name = "Player";
    std::string _player2Name = "Enemy";

    bool _forceExit = false;
    bool _gameOver = false;
    bool _checkExit = false;
    bool _gameInProgress = false;
};

extern ArcomageGame *pArcomageGame;
extern ArcomageCard pCards[87];

struct spark_point_struct {
    int spark_remaining_life = 0;
    Pointi spark_position {};
    float spark_x_pos = 0;
    float spark_y_pos = 0;
    float spark_x_speed = 0;
    float spark_y_speed = 0;
};

struct effect_params_struct {
    Recti effect_area;
    int unused_param_1 = 0;
    int unused_param_2 = 0;
    int unused_param_3 = 0;
    float gravity_acc = 0;
    int spark_array_size = 0;
    int create_per_frame = 0;
    float unused_acc_1 = 0;
    float unused_acc_2 = 0;
    int min_lifespan = 0;
    int max_lifespan = 0;
    spark_point_struct *sparks_array = nullptr;
};

struct explosion_effect_struct {
    static explosion_effect_struct *New();

    int Free();
    int StartFill(effect_params_struct *params);
    int Clear(char stop_init, char wipe);
    int UpdateEffect();
    int IsEffectActive();

    int mem_signature = 0;
    int spark_array_size = 0;
    int remaining_sparks_to_init = 0;
    int start_x_min = 0;
    int start_y_max = 0;
    int start_x_max = 0;
    int start_y_min = 0;
    int unused_param_1 = 0;
    int unused_param_2 = 0;
    int unused_param_3 = 0;
    float gravity = 0;
    int num_init_per_cycle = 0;
    int prev_init_overflow = 0;
    int unused_acc_1 = 0;
    int unused_acc_2 = 0;
    int min_lifespan = 0;
    int max_lifespan = 0;
    int unused_param_4 = 0;
    int unused_param_5 = 0;
    int unused_param_6 = 0;
    int unused_param_7 = 0;
    spark_point_struct *spark_array_ptr = nullptr;
    char effect_active = 0;
    char params_filled = 0;
    char unused_param_8 = 0;
    char unused_param_9 = 0;
};

#define DECK_SIZE 108

struct ArcomageDeck {
    std::string name{};
    char cardsInUse[DECK_SIZE]{};
    int cards_IDs[DECK_SIZE]{};
};

struct am_effects_struct {
    char have_effect = 0;
    char effect_sign = 0;
    char _pad_2 = 0;
    char _pad_3 = 0;
    effect_params_struct eff_params {};
    explosion_effect_struct *explosion_eff = nullptr;
    std::array<spark_point_struct, 150> effect_sparks {};
};
