#pragma once

#include <string>

#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Graphics/FrameLimiter.h"

#include "Utility/Geometry/Point.h"
#include "Utility/Geometry/Rect.h"

#include "Platform/PlatformEnums.h"

struct ArcomageCard {
    char pCardName[32];
    int slot;
    char card_resource_type;  // 1- brick, 2-gems, 3-beasts
    int8_t needed_quarry_level;
    int8_t needed_magic_level;
    int8_t needed_zoo_level;
    int8_t needed_bricks;
    int8_t needed_gems;
    int8_t needed_beasts;
    bool can_be_discarded;
    int compare_param;
    char field_30;  // play again
    char draw_extra_card_count;
    int8_t to_player_quarry_lvl;
    int8_t to_player_magic_lvl;
    int8_t to_player_zoo_lvl;
    int8_t to_player_bricks;
    int8_t to_player_gems;
    int8_t to_player_beasts;
    int8_t to_player_buildings;
    int8_t to_player_wall;
    int8_t to_player_tower;
    int8_t to_enemy_quarry_lvl;
    int8_t to_enemy_magic_lvl;
    int8_t to_enemy_zoo_lvl;
    int8_t to_enemy_bricks;
    int8_t to_enemy_gems;
    int8_t to_enemy_beasts;
    int8_t to_enemy_buildings;
    int8_t to_enemy_wall;
    int8_t to_enemy_tower;
    int8_t to_pl_enm_quarry_lvl;
    int8_t to_pl_enm_magic_lvl;
    int8_t to_pl_enm_zoo_lvl;
    int8_t to_pl_enm_bricks;
    int8_t to_pl_enm_gems;
    int8_t to_pl_enm_beasts;
    int8_t to_pl_enm_buildings;
    int8_t to_pl_enm_wall;
    int8_t to_pl_enm_tower;
    char field_4D;  // play again 2
    int8_t can_draw_extra_card2;
    int8_t to_player_quarry_lvl2;
    int8_t to_player_magic_lvl2;
    int8_t to_player_zoo_lvl2;
    int8_t to_player_bricks2;
    int8_t to_player_gems2;
    int8_t to_player_beasts2;
    int8_t to_player_buildings2;
    int8_t to_player_wall2;
    int8_t to_player_tower2;
    int8_t to_enemy_quarry_lvl2;
    int8_t to_enemy_magic_lvl2;
    int8_t to_enemy_zoo_lvl2;
    int8_t to_enemy_bricks2;
    int8_t to_enemy_gems2;
    int8_t to_enemy_beasts2;
    int8_t to_enemy_buildings2;
    int8_t to_enemy_wall2;
    int8_t to_enemy_tower2;
    int8_t to_pl_enm_quarry_lvl2;
    int8_t to_pl_enm_magic_lvl2;
    int8_t to_pl_enm_zoo_lvl2;
    int8_t to_pl_enm_bricks2;
    int8_t to_pl_enm_gems2;
    int8_t to_pl_enm_beasts2;
    int8_t to_pl_enm_buildings2;
    int8_t to_pl_enm_wall2;
    int8_t to_pl_enm_tower2;
    char field_6A;  // unused??
    char field_6B;  // unused??
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
    int cards_at_hand[10] {};
    Pointi card_shift[10] {};
};

enum class ArcomageMessageType {
    ARCO_MSG_NULL,
    ARCO_MSG_KEYDOWN,
    ARCO_MSG_FORCEQUIT,
    ARCO_MSG_LM_UP,
    ARCO_MSG_RM_UP,
    ARCO_MSG_LM_DOWN,
    ARCO_MSG_RM_DOWN,
    ARCO_MSG_SWITCH_FULLSCREEN,
    ARCO_MSG_ESCAPE
};
using enum ArcomageMessageType;

struct ArcomageGame_InputMSG {
    ArcomageMessageType am_input_type{ ARCO_MSG_NULL };
    int field_4{};
    int am_input_key{};  // waht other key
};

class GUIFont;

struct ArcomageGame {
    ArcomageGame() {}

    // static bool LoadBackground();
    static bool LoadSprites();
    static bool MsgLoop(int a1, ArcomageGame_InputMSG *a2);
    static void playSound(unsigned int event_id);
    static void onKeyPress(PlatformKey key);
    static void OnMouseClick(char right_left, bool bDown);
    static void OnMouseMove(int x, int y);
    static void GetCardRect(unsigned int uCardID, Recti *pCardRect);
    static void PrepareArcomage();
    // static void DoBlt_Copy(uint16_t *pPixels);  // idb

    static void Loop();

    FrameLimiter _frameLimiter;
    int _targetFPS{ 32 };

    ArcomageGame_InputMSG stru1;

    int mouse_x = 0;
    int mouse_y = 0;
    char mouse_left = 0;
    char field_39 = 0;
    char mouse_right = 0;
    char field_3B = 0;
    GUIFont *pfntComic = nullptr;  // ptr_48;
    GUIFont *pfntArrus = nullptr;  // ptr_4C;
    int field_50 = 0;
    int field_54 = 0;  // blend mode ??
    Texture *pGameBackground = nullptr;
    Texture *pSprites = nullptr;
    int uGameWinner = 0;
    int Victory_type = 0;
    std::string pPlayer1Name;
    std::string pPlayer2Name;
    char force_am_exit = 0;
    char GameOver = 0;
    char prev_mouse_left = 0;
    char prev_mouse_right = 0;
    char check_exit = 0;
    char bGameInProgress = 0;
};

extern ArcomageGame *pArcomageGame;
extern ArcomageCard pCards[87];
extern void set_stru1_field_8_InArcomage(int inValue);

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
    int unused_param_1;
    int unused_param_2;
    int unused_param_3;
    float gravity_acc;
    int spark_array_size;
    int create_per_frame;
    float unused_acc_1;
    float unused_acc_2;
    int min_lifespan;
    int max_lifespan;
    spark_point_struct *sparks_array;
};

struct explosion_effect_struct {
    static explosion_effect_struct *New();

    int Free();
    int StartFill(effect_params_struct *params);
    int Clear(char stop_init, char wipe);
    int UpdateEffect();
    int IsEffectActive();

    int mem_signature;
    int spark_array_size;
    int remaining_sparks_to_init;
    int start_x_min;
    int start_y_max;
    int start_x_max;
    int start_y_min;
    int unused_param_1;
    int unused_param_2;
    int unused_param_3;
    float gravity;
    int num_init_per_cycle;
    int prev_init_overflow;
    int unused_acc_1;
    int unused_acc_2;
    int min_lifespan;
    int max_lifespan;
    int unused_param_4;
    int unused_param_5;
    int unused_param_6;
    int unused_param_7;
    spark_point_struct *spark_array_ptr = nullptr;
    char effect_active;
    char params_filled;
    char unused_param_8;
    char unused_param_9;
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
    spark_point_struct effect_sparks[150] {};
};
