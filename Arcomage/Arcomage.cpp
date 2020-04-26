#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Strings.h"
#include "Engine/Time.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/ImageLoader.h"

#include "Arcomage/Arcomage.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIHouses.h"

#include "Media/Audio/AudioPlayer.h"
#include "Media/MediaPlayer.h"

#include "Platform/Api.h"

void SetStartConditions();
void SetStartGameData();
void FillPlayerDeck();
void InitalHandsFill();
void GetNextCardFromDeck(int player_num);
int GetEmptyCardSlotIndex(int player_num);
void IncreaseResourcesInTurn(int player_num);
void TurnChange();
bool IsGameOver();
int CalculateCardPower(ArcomagePlayer *player, ArcomagePlayer *enemy,
                       ArcomageCard *pCard, int mastery);
char PlayerTurn(int player_num);
void DrawGameUI(int animation_stage);
void DrawSparks();
void DrawRectanglesForText();
void DrawPlayersText();
void DrawPlayerLevels(const String &str, Point *pXY);
void DrawBricksCount(const String &str, Point *pXY);
void DrawGemsCount(const String &str, Point *pXY);
void DrawBeastsCount(const String &str, Point *pXY);
void DrawPlayersTowers();
void DrawPlayersWall();
void DrawCards();
void DrawCardAnimation(int a1);
int GetPlayerHandCardCount(int player_num);
int DrawCardsRectangles(int player_num);
bool DiscardCard(int player_num, int card_slot_index);
bool PlayCard(int player_num, int card_slot_num);
bool CanCardBePlayed(int player_num, int hand_card_indx);
void ApplyCardToPlayer(int player_num, unsigned int uCardID);
int new_explosion_effect(Point *a1, int a2);
int ApplyDamageToBuildings(int player_num, int damage);
void GameResultsApply();

void am_DrawText(const String &str, Point *pXY);
void DrawRect(Rect *pXYZW, unsigned __int16 uColor, char bSolidFill);
int rand_interval(int min, int max);  // idb
void am_IntToString(int val, char *pOut);

unsigned int R8G8B8_to_TargetFormat(int uColor) {
    return Color16(uColor & 0xFF, (uColor >> 8) & 0xFF, (uColor >> 16) & 0xFF);
}

struct ArcomageStartConditions {
    __int16 max_tower;
    __int16 max_resources;
    __int16 tower_height;
    __int16 wall_height;
    __int16 quarry_level;
    __int16 magic_level;
    __int16 zoo_level;
    __int16 bricks_amount;
    __int16 gems_amount;
    __int16 beasts_amount;
    int mastery_lvl;
};

const ArcomageStartConditions start_conditions[13] = {
    {30, 100, 15, 5, 2, 2, 2, 10, 10, 10, 0},
    {50, 150, 20, 5, 2, 2, 2, 5, 5, 5, 1},
    {50, 150, 20, 5, 2, 2, 2, 5, 5, 5, 2},
    {75, 200, 25, 10, 3, 3, 3, 5, 5, 5, 2},
    {75, 200, 20, 10, 3, 3, 3, 5, 5, 5, 1},
    {100, 300, 30, 15, 4, 4, 4, 10, 10, 10, 1},
    {100, 300, 30, 15, 4, 4, 4, 10, 10, 10, 2},
    {150, 400, 20, 10, 5, 5, 5, 25, 25, 25, 0},
    {200, 500, 20, 10, 1, 1, 1, 15, 15, 15, 2},
    {100, 300, 20, 50, 1, 1, 5, 5, 5, 25, 0},
    {125, 350, 10, 20, 3, 1, 2, 15, 5, 10, 2},
    {125, 350, 10, 20, 3, 1, 2, 15, 5, 10, 1},
    {100, 300, 50, 50, 5, 3, 5, 20, 10, 20, 0}};

#define SIG_MEMALOC 0x67707274  // memory allocated
#define SIG_MEMFREE 0x78787878  // memory free

ArcomageGame *pArcomageGame = new ArcomageGame;

ArcomagePlayer am_Players[2];
AcromageCardOnTable shown_cards[10];
am_effects_struct am_effects_array[10];

ArcomageDeck playDeck;
ArcomageDeck deckMaster;

char Player2Name[] = "Enemy";
char Player1Name[] = "Player";

struct am_ai_cardpowerstruct {
    int slot_index;
    int card_power;
};

am_ai_cardpowerstruct cards_power[10];

bool Player_Gets_First_Turn = 1;  // who starts the game
bool Player_Cards_Shift = 1;  // shifts the cards round at the bottom of the screen so they arent all level
char use_start_bonus = 1;

int start_tower_height;
int start_wall_height;
int start_quarry_level;
int start_magic_level;
int start_zoo_level;

int start_bricks_amount;
int start_gems_amount;
int start_beasts_amount;

int minimum_cards_at_hand = 5;
int quarry_bonus = 1;  // acts as effective min level
int magic_bonus = 1;
int zoo_bonus = 1;

int max_tower_height = 50;
int max_resources_amount = 100;

int opponent_mastery = 1;
char opponents_turn;
char See_Opponents_Cards = 0;
int current_player_num;

char need_to_discard_card;

int current_card_slot_index;
int played_card_id;
int discarded_card_id;

int deck_walk_index;

int Card_Hover_Index;
int num_cards_to_discard;
int num_actions_left;

sPoint anim_card_spd_drawncard;  // anim card speed draw from deck
Point anim_card_pos_drawncard;  // anim card pos draw from deck
sPoint anim_card_spd_playdiscard;  // anim card speeds   play/discard
Point anim_card_pos_playdiscard;  // anim card posisiotn play/discard

char drawn_card_anim_start;
int drawn_card_anim_cnt;
int drawn_card_slot_index;
char playdiscard_anim_start;
char hide_card_anim_start;
char hide_card_anim_runnning;
int hide_card_anim_count;

struct arcomage_mouse {
    bool Update();
    bool Inside(Rect* pXYZW);

    int x;
    int y;
    char curr_mouse_left;
    char mouse_left_state_changed;
    char curr_mouse_right;
    char mouse_right_state_changed;
};

bool arcomage_mouse::Update() {
    this->x = pArcomageGame->mouse_x;
    this->y = pArcomageGame->mouse_y;
    this->curr_mouse_left = pArcomageGame->mouse_left;
    this->mouse_left_state_changed = (pArcomageGame->mouse_left == pArcomageGame->prev_mouse_left);
    this->curr_mouse_right = pArcomageGame->mouse_right;
    this->mouse_right_state_changed = (pArcomageGame->mouse_right == pArcomageGame->prev_mouse_right);
    pArcomageGame->prev_mouse_left = pArcomageGame->mouse_left;
    pArcomageGame->prev_mouse_right = pArcomageGame->mouse_right;
    return true;
}

bool arcomage_mouse::Inside(Rect *pXYZW) {
    return (x >= pXYZW->x) && (x <= pXYZW->z) && (y >= pXYZW->y) && (y <= pXYZW->w);
}

void ArcomageGame::OnMouseClick(char right_left, bool bDown) {
    if (right_left)
        pArcomageGame->mouse_right = bDown;
    else
        pArcomageGame->mouse_left = bDown;
}

void ArcomageGame::OnMouseMove(int x, int y) {
    pArcomageGame->mouse_x = x;
    pArcomageGame->mouse_y = y;
}

explosion_effect_struct *explosion_effect_struct::New() {
    explosion_effect_struct *v2 = (explosion_effect_struct *)malloc(0x5Cu);
    if (v2 == nullptr) {
        logger->Warning("Malloc error");
        Error("Malloc");  // is this recoverable
    }
    v2->mem_signature = SIG_MEMALOC;
    v2->remaining_sparks_to_init = 0;
    v2->prev_init_overflow = 0.0;
    v2->effect_active = 0;
    v2->params_filled = 0;

    return v2;
}

int explosion_effect_struct::Free() {
    if (this->mem_signature == SIG_MEMALOC) {
        this->mem_signature = SIG_MEMFREE;
        free(this);
        return 0;
    }

    return 2;
}

int explosion_effect_struct::StartFill(effect_params_struct *params) {
    // fill effect params;
    if (this->mem_signature == SIG_MEMALOC) {
        this->spark_array_size = params->spark_array_size;
        this->start_x_min = params->effect_area.x << 16;
        this->start_y_max = params->effect_area.y << 16;
        this->start_x_max = params->effect_area.z << 16;
        this->start_y_min = params->effect_area.w << 16;
        this->unused_param_1 = params->unused_param_1;
        this->unused_param_2 = params->unused_param_2;
        this->unused_param_3 = params->unused_param_3;
        this->gravity_unshift = (float)(params->gravity_acc * 65536.0);
        this->num_init_per_cycle = params->create_per_frame;
        this->unused_acc_1 = (int)(params->unused_acc_1 * 65536.0);
        this->unused_acc_1 = (int)(params->unused_acc_1 * 65536.0);
        this->min_lifespan = params->min_lifespan;
        this->max_lifespan = params->max_lifespan;
        this->spark_array_ptr = params->sparks_array;
        this->params_filled = 1;
        return 0;
    }

    return 2;
}

int explosion_effect_struct::Clear(char stop_init, char wipe) {
    if (mem_signature == SIG_MEMALOC) {
        if (stop_init) {
            remaining_sparks_to_init = 0;
            prev_init_overflow = 0.0;
        }
        if (params_filled && wipe) {
            for (int i = 0; i < spark_array_size; ++i) spark_array_ptr[i].spark_remaining_life = 0;
            effect_active = 0;
        }
        return 0;
    }

    return 2;
}

int explosion_effect_struct::UpdateEffect() {
    if (this->mem_signature != SIG_MEMALOC) return 2;

    // calculate how many sparks to initiate
    float total_to_init = 0;
    if (this->remaining_sparks_to_init > 0) {
        total_to_init = this->prev_init_overflow + this->num_init_per_cycle;
         if (total_to_init > this->remaining_sparks_to_init) total_to_init = this->remaining_sparks_to_init;
    }

    // if sparks left to initiate or effect is still active
    if (total_to_init >= 1.0 || this->effect_active) {
        bool active_check = 0;
        spark_point_struct* spark_ptr = this->spark_array_ptr;

        // cycle through array
        for (int v21 = this->spark_array_size; v21; v21--) {
            // spark alive so update
            if (spark_ptr->spark_remaining_life > 0) {
                // reduce spark life
                --spark_ptr->spark_remaining_life;

                // update x pos
                spark_ptr->spark_x_unshift += spark_ptr->spark_x_speed;
                spark_ptr->spark_position.x = spark_ptr->spark_x_unshift >> 16;

                // update y pos
                spark_ptr->spark_y_speed += this->gravity_unshift;
                spark_ptr->spark_y_unshift += spark_ptr->spark_y_speed;
                spark_ptr->spark_position.y = spark_ptr->spark_y_unshift >> 16;

                // set effect still active
                active_check = 1;
            } else {
                if (total_to_init >= 1.0) {
                    // spark dead - initialze new spark
                    spark_ptr->spark_remaining_life = rand_interval(this->min_lifespan, this->max_lifespan);
                    spark_ptr->spark_x_speed = (rand() % 17 - 8) << 16;
                    spark_ptr->spark_y_speed = (rand() % 17 - 8) << 16;
                    spark_ptr->spark_x_unshift = rand_interval(this->start_x_min, (this->start_x_max - 1));
                    spark_ptr->spark_position.x = spark_ptr->spark_x_unshift >> 16;
                    spark_ptr->spark_y_unshift = rand_interval((this->start_y_min - 1), this->start_y_max);;
                    spark_ptr->spark_position.y = spark_ptr->spark_y_unshift >> 16;
                    --this->remaining_sparks_to_init;
                    --total_to_init;

                    // set effect still active
                    active_check = 1;
                }
            }
            ++spark_ptr;
        }

        this->effect_active = active_check;
        this->prev_init_overflow = total_to_init;
    }
    return 0;
}

int explosion_effect_struct::IsEffectActive() {
    // returns 2 if effect still active - 0/1/3 otherwise
    if (mem_signature == SIG_MEMALOC) {
        if (remaining_sparks_to_init <= 0)
            return effect_active != 0 ? 2 : 0;
        else
            return 1;
    }

    return 3;
}

int new_explosion_effect(Point* startXY, int effect_value) {
    // find first empty effect slot
    signed int arr_slot = 0;
    for (arr_slot = 0; arr_slot < 10; arr_slot++) {
        if (am_effects_array[arr_slot].have_effect) {
            if (am_effects_array[arr_slot].explosion_eff->IsEffectActive() == 0) {
                am_effects_array[arr_slot].have_effect = 0;
                break;
            }
        } else {
            break;
        }
        if (arr_slot == 9) return 2;
    }

    // set slot active and effect colour
    am_effects_array[arr_slot].have_effect = 1;
    if (effect_value <= 0) {
        am_effects_array[arr_slot].effect_sign = 0;
        effect_value = -effect_value;
    } else {
        am_effects_array[arr_slot].effect_sign = 1;
    }

    // fill effect params
    am_effects_array[arr_slot].eff_params.effect_area.x = startXY->x - 20;
    am_effects_array[arr_slot].eff_params.effect_area.z = startXY->x + 20;
    am_effects_array[arr_slot].eff_params.effect_area.y = startXY->y - 20;
    am_effects_array[arr_slot].eff_params.effect_area.w = startXY->y + 20;
    am_effects_array[arr_slot].eff_params.unused_param_1 = -60;
    am_effects_array[arr_slot].eff_params.unused_param_2 = 60;
    am_effects_array[arr_slot].eff_params.unused_param_3 = 180;
    am_effects_array[arr_slot].eff_params.gravity_acc = 0.5;
    am_effects_array[arr_slot].eff_params.spark_array_size = 150;
    am_effects_array[arr_slot].eff_params.create_per_frame = 50.0;
    am_effects_array[arr_slot].eff_params.unused_acc_1 = 3.0;
    am_effects_array[arr_slot].eff_params.unused_acc_1 = 8.0;
    am_effects_array[arr_slot].eff_params.min_lifespan = 5;
    am_effects_array[arr_slot].eff_params.max_lifespan = 15;
    am_effects_array[arr_slot].eff_params.sparks_array = &am_effects_array[arr_slot].effect_sparks[0];

    // fill explosion struct
    explosion_effect_struct* explos = am_effects_array[arr_slot].explosion_eff;
    if ((explos->StartFill(&am_effects_array[arr_slot].eff_params)) == 2) return 2;
    if (!explos->params_filled) return 3;
    if (10 * effect_value > 150) effect_value = 15;
    explos->remaining_sparks_to_init = 10 * effect_value;
    explos->prev_init_overflow = 0.0;
    explos->effect_active = 0;

    // unused parameters
    explos->unused_param_4 = 0;
    explos->unused_param_6 = 0;
    explos->unused_param_5 = 0;
    explos->unused_param_7 = 0;

    // wipe spark array
    for (int i = 0; i < explos->spark_array_size; ++i) explos->spark_array_ptr[i].spark_remaining_life = 0;

    return 0;
}

void DrawSparks() {
    int rgb_pixel_color;

    for (int i = 0; i < 10; ++i) {
        if (am_effects_array[i].have_effect &&
            (am_effects_array[i].explosion_eff->IsEffectActive() == 2)) {
            // scan and find extents of spark array
            bool first = 0;
            int xmin = -1;
            int xmax = -1;
            int ymin = -1;
            int ymax = -1;
            for (int j = 0; j < 150; ++j) {
                if (am_effects_array[i].effect_sparks[j].spark_remaining_life > 0) {
                    if (first == 0) {
                        if (am_effects_array[i].effect_sparks[j].spark_position.x >= 0 && am_effects_array[i].effect_sparks[j].spark_position.y >= 0) {
                            if (am_effects_array[i].effect_sparks[j].spark_position.x <= 639 && am_effects_array[i].effect_sparks[j].spark_position.y <= 479) {
                                xmin = xmax = am_effects_array[i].effect_sparks[j].spark_position.x;
                                ymin = ymax = am_effects_array[i].effect_sparks[j].spark_position.y;
                                first = 1;
                            }
                        }
                    } else {
                        if (am_effects_array[i].effect_sparks[j].spark_position.x >= 0 && am_effects_array[i].effect_sparks[j].spark_position.y >= 0) {
                            if (am_effects_array[i].effect_sparks[j].spark_position.x <= 639 && am_effects_array[i].effect_sparks[j].spark_position.y <= 479) {
                                if (am_effects_array[i].effect_sparks[j].spark_position.x < xmin)
                                    xmin = am_effects_array[i].effect_sparks[j].spark_position.x;
                                if (am_effects_array[i].effect_sparks[j].spark_position.x > xmax)
                                    xmax = am_effects_array[i].effect_sparks[j].spark_position.x;
                                if (am_effects_array[i].effect_sparks[j].spark_position.y < ymin)
                                    ymin = am_effects_array[i].effect_sparks[j].spark_position.y;
                                if (am_effects_array[i].effect_sparks[j].spark_position.y > ymax)
                                    ymax = am_effects_array[i].effect_sparks[j].spark_position.y;
                            }
                        }
                    }
                }
            }

            // increase extents to cover larger square particle drawing
            xmax +=2;
            ymax +=2;
            uint width = xmax - xmin;
            uint height = ymax - ymin;

            if (width && height) {
                // create temp image
                Image* temp = Image::Create(width, height, IMAGE_FORMAT_A8R8G8B8);
                uint32_t* temppix = (uint32_t*)temp->GetPixels(IMAGE_FORMAT_A8R8G8B8);

                // set the pixel color
                rgb_pixel_color = 0xFF00FF00;  // green
                if (!am_effects_array[i].effect_sign) rgb_pixel_color = 0xFFFF0000;  // red

                // draw pixels to image
                for (int j = 0; j < 150; ++j) {
                    if (am_effects_array[i].effect_sparks[j].spark_remaining_life > 0) {
                        // check limits
                        if (am_effects_array[i].effect_sparks[j].spark_position.x >= 0 && am_effects_array[i].effect_sparks[j].spark_position.y >= 0) {
                            if (am_effects_array[i].effect_sparks[j].spark_position.x <= 639 && am_effects_array[i].effect_sparks[j].spark_position.y <= 479) {
                                uint x = am_effects_array[i].effect_sparks[j].spark_position.x - xmin;
                                uint y = am_effects_array[i].effect_sparks[j].spark_position.y - ymin;

                                if (j % 2) {
                                    // draw single pixel
                                    temppix[x + y * width] = rgb_pixel_color;
                                } else {
                                    // draw square
                                    temppix[x + y * width] = rgb_pixel_color;
                                    temppix[((x + 1) + y * width)] = rgb_pixel_color;
                                    temppix[(x + (y+1) * width)] = rgb_pixel_color;
                                    temppix[((x + 1) + (y+1) * width)] = rgb_pixel_color;
                                }
                            }
                        }
                    }
                }

                // draw created image to xmin,ymin
                render->DrawTextureAlphaNew(xmin / 640., ymin / 480., temp);
                temp->Release();
            }
        }
    }
}

void ArcomageGame::PlaySound(unsigned int event_id) {
    SoundID play_sound_id;  // eax@10

    switch (event_id) {
        case 40:
        case 43:
        case 46:
            play_sound_id = SOUND_bricks_down;
            break;
        case 39:
        case 41:
        case 42:
        case 44:
        case 45:
        case 47:
            play_sound_id = SOUND_bricks_up;
            break;
        case 0:
        case 12:
        case 14:
        case 15:
        case 16:
        case 48:
        case 50:
        case 53:
            play_sound_id = SOUND_damage;
            break;
        case 21:
        case 22:
        case 23:
            play_sound_id = SOUND_deal;
            break;
        case 56:
            play_sound_id = SOUND_defeat;
            break;
        case 31:
        case 34:
        case 37:
            play_sound_id = SOUND_querry_up;
            break;
        case 1:
        case 30:
        case 32:
        case 33:
        case 35:
        case 36:
        case 38:
            play_sound_id = SOUND_querry_down;
            break;
        case 20:
            play_sound_id = SOUND_shuffle;
            break;
        case 3:
            play_sound_id = SOUND_title;
            break;
        case 52:
        case 54:
            play_sound_id = SOUND_tower_up;
            break;
        case 10:
        case 11:
        case 13:
            play_sound_id = SOUND_typing;
            break;
        case 55:
            play_sound_id = SOUND_victory;
            break;
        case 49:
        case 51:
            play_sound_id = SOUND_wall_up;
            break;
        default:
            return;
    }
    pAudioPlayer->PlaySound(play_sound_id, 0, 0, -1, 0, 0);
}

bool ArcomageGame::MsgLoop(int a1, ArcomageGame_InputMSG *a2) {
    void *v2 = a2;
    pArcomageGame->field_0 = 0;
    pArcomageGame->stru1.am_input_type = 0;

    window->PeekSingleMessage();

    memcpy(v2, &pArcomageGame->stru1, 0xCu);
    return pArcomageGame->stru1.am_input_type != 0;
}

bool ArcomageGame::LoadSprites() {
    // load layout sprite
    pArcomageGame->pSprites = assets->GetImage_PCXFromIconsLOD("sprites.pcx");
    pArcomageGame->pSpritesPixels = (unsigned __int16 *)pArcomageGame->pSprites->GetPixels(IMAGE_FORMAT_R5G6B5);
    return true;
}

int CalculateCardPower(ArcomagePlayer *player, ArcomagePlayer *enemy,
                       ArcomageCard *pCard, int mastery) {
    enum V_IND {
        P_TOWER_M10,
        P_WALL_M10,
        E_TOWER,
        E_WALL,
        E_BUILDINGS,
        E_QUARRY,
        E_MAGIC,
        E_ZOO,
        E_RES,
        V_INDEX_MAX
    };

    // mastery coeffs
    // base mastery focus on growing walls + tower
    // second level high priority on resource gen
    const int mastery_coeff[V_INDEX_MAX][2] = {
        {10, 5},  // P_TOWER_M10
        {2, 1},   // P_WALL_M10
        {1, 10},  // E_TOWER
        {1, 3},   // E_WALL
        {1, 7},   // E_BUILDINGS
        {1, 5},   // E_QUARRY
        {1, 40},  // E_MAGIC
        {1, 40},  // E_ZOO
        {1, 2}    // E_RES
    };

    int card_power = 0;
    int element_power = 0;

    if (pCard->to_player_tower == 99 || pCard->to_pl_enm_tower == 99 ||
        pCard->to_player_tower2 == 99 || pCard->to_pl_enm_tower2 == 99) {
        element_power = enemy->tower_height - player->tower_height;
    } else {
        element_power = pCard->to_player_tower + pCard->to_pl_enm_tower +
                        pCard->to_player_tower2 + pCard->to_pl_enm_tower2;
    }

    if (player->tower_height >= 10) {
        card_power += mastery_coeff[P_TOWER_M10][mastery] * element_power;
    } else {
        card_power += 20 * element_power;
    }

    if (pCard->to_player_wall == 99 || pCard->to_pl_enm_wall == 99 ||
        pCard->to_player_wall2 == 99 || pCard->to_pl_enm_wall2 == 99) {
        element_power = enemy->wall_height - player->wall_height;
    } else {
        element_power = pCard->to_player_wall + pCard->to_pl_enm_wall +
                        pCard->to_player_wall2 + pCard->to_pl_enm_wall2;
    }

    if (player->wall_height >= 10) {
        card_power += mastery_coeff[P_WALL_M10][mastery] * element_power;  // 1
    } else {
        card_power += 5 * element_power;
    }

    card_power +=
        7 * (pCard->to_player_buildings + pCard->to_pl_enm_buildings +
             pCard->to_player_buildings2 + pCard->to_pl_enm_buildings2);

    if (pCard->to_player_quarry_lvl == 99 ||
        pCard->to_pl_enm_quarry_lvl == 99 ||
        pCard->to_player_quarry_lvl2 == 99 ||
        pCard->to_pl_enm_quarry_lvl2 == 99) {
        element_power = enemy->quarry_level - player->quarry_level;
    } else {
        element_power =
            pCard->to_player_quarry_lvl + pCard->to_pl_enm_quarry_lvl +
            pCard->to_player_quarry_lvl2 + pCard->to_pl_enm_quarry_lvl;
    }

    card_power += 40 * element_power;

    if (pCard->to_player_magic_lvl == 99 || pCard->to_pl_enm_magic_lvl == 99 ||
        pCard->to_player_magic_lvl2 == 99 ||
        pCard->to_pl_enm_magic_lvl2 == 99) {
        element_power = enemy->magic_level - player->magic_level;
    } else {
        element_power =
            pCard->to_player_magic_lvl + pCard->to_pl_enm_magic_lvl +
            pCard->to_player_magic_lvl2 + pCard->to_pl_enm_magic_lvl2;
    }
    card_power += 40 * element_power;

    if (pCard->to_player_zoo_lvl == 99 || pCard->to_pl_enm_zoo_lvl == 99 ||
        pCard->to_player_zoo_lvl2 == 99 || pCard->to_pl_enm_zoo_lvl2 == 99) {
        element_power = enemy->zoo_level - player->zoo_level;
    } else {
        element_power = pCard->to_player_zoo_lvl + pCard->to_pl_enm_zoo_lvl +
                        pCard->to_player_zoo_lvl2 + pCard->to_pl_enm_zoo_lvl2;
    }
    card_power += 40 * element_power;

    if (pCard->to_player_bricks == 99 || pCard->to_pl_enm_bricks == 99 ||
        pCard->to_player_bricks2 == 99 || pCard->to_pl_enm_bricks2 == 99) {
        element_power = enemy->resource_bricks - player->resource_bricks;
    } else {
        element_power = pCard->to_player_bricks + pCard->to_pl_enm_bricks +
                        pCard->to_player_bricks2 + pCard->to_pl_enm_bricks2;
    }
    card_power += 2 * element_power;

    if (pCard->to_player_gems == 99 || pCard->to_pl_enm_gems == 99 ||
        pCard->to_player_gems2 == 99 || pCard->to_pl_enm_gems2 == 99) {
        element_power = enemy->resource_gems - player->resource_gems;
    } else {
        element_power = pCard->to_player_gems + pCard->to_pl_enm_gems +
                        pCard->to_player_gems2 + pCard->to_pl_enm_gems2;
    }
    card_power += 2 * element_power;

    if (pCard->to_player_beasts == 99 || pCard->to_pl_enm_beasts == 99 ||
        pCard->to_player_beasts2 == 99 || pCard->to_pl_enm_beasts2 == 99) {
        element_power = enemy->resource_beasts - player->resource_beasts;
    } else {
        element_power = pCard->to_player_beasts + pCard->to_pl_enm_beasts +
                        pCard->to_player_beasts2 + pCard->to_pl_enm_beasts2;
    }
    card_power += 2 * element_power;

    if (pCard->to_enemy_tower == 99 || pCard->to_enemy_tower2 == 99) {
        element_power = player->tower_height - enemy->tower_height;
    } else {
        element_power = -(pCard->to_enemy_tower + pCard->to_enemy_tower2);
    }
    card_power += mastery_coeff[E_TOWER][mastery] * element_power;

    if (pCard->to_enemy_wall == 99 || pCard->to_enemy_wall2 == 99) {
        element_power = player->wall_height - enemy->wall_height;
    } else {
        element_power = -(pCard->to_enemy_wall + pCard->to_enemy_wall2);
    }
    card_power += mastery_coeff[E_WALL][mastery] * element_power;

    card_power -= mastery_coeff[E_BUILDINGS][mastery] *
                  (pCard->to_enemy_buildings + pCard->to_enemy_buildings2);

    if (pCard->to_enemy_quarry_lvl == 99 || pCard->to_enemy_quarry_lvl2 == 99) {
        element_power = player->quarry_level - enemy->quarry_level;  // 5
    } else {
        element_power =
            -(pCard->to_enemy_quarry_lvl + pCard->to_enemy_quarry_lvl2);  // 5
    }
    card_power += mastery_coeff[E_QUARRY][mastery] * element_power;

    if (pCard->to_enemy_magic_lvl == 99 || pCard->to_enemy_magic_lvl2 == 99) {
        element_power = player->magic_level - enemy->magic_level;  // 40
    } else {
        element_power =
            -(pCard->to_enemy_magic_lvl + pCard->to_enemy_magic_lvl2);
    }
    card_power += mastery_coeff[E_MAGIC][mastery] * element_power;

    if (pCard->to_enemy_zoo_lvl == 99 || pCard->to_enemy_zoo_lvl2 == 99) {
        element_power = player->zoo_level - enemy->zoo_level;  // 40
    } else {
        element_power = -(pCard->to_enemy_zoo_lvl + pCard->to_enemy_zoo_lvl2);
    }
    card_power += mastery_coeff[E_ZOO][mastery] * element_power;

    if (pCard->to_enemy_bricks == 99 || pCard->to_enemy_bricks2 == 99) {
        element_power = player->resource_bricks - enemy->resource_bricks;  // 2
    } else {
        element_power = -(pCard->to_enemy_bricks + pCard->to_enemy_bricks2);
    }
    card_power += mastery_coeff[E_RES][mastery] * element_power;

    if (pCard->to_enemy_gems == 99 || pCard->to_enemy_gems2 == 99) {
        element_power = player->resource_gems - enemy->resource_gems;  // 2
    } else {
        element_power = -(pCard->to_enemy_gems + pCard->to_enemy_gems2);
    }
    card_power += mastery_coeff[E_RES][mastery] * element_power;

    if (pCard->to_enemy_beasts == 99 || pCard->to_enemy_beasts2 == 99) {
        element_power = player->resource_beasts - enemy->resource_beasts;  // 2
    } else {
        element_power = -(pCard->to_enemy_beasts + pCard->to_enemy_beasts2);
    }
    card_power += mastery_coeff[E_RES][mastery] * element_power;

    if (pCard->field_30 || pCard->field_4D) {
        card_power *= 10;
    }

    if (pCard->card_resource_type == 1) {
        element_power = player->resource_bricks - pCard->needed_bricks;
    } else if (pCard->card_resource_type == 2) {
        element_power = player->resource_gems - pCard->needed_gems;
    } else if (pCard->card_resource_type == 3) {
        element_power = player->resource_beasts - pCard->needed_beasts;
    }
    if (element_power > 3) {
        element_power = 3;
    }
    card_power += 5 * element_power;

    if (enemy->tower_height <= pCard->to_enemy_tower2 + pCard->to_enemy_tower) {
        card_power += 9999;
    }

    if (pCard->to_enemy_tower2 + pCard->to_enemy_tower + pCard->to_enemy_wall +
            pCard->to_enemy_wall2 + pCard->to_enemy_buildings +
            pCard->to_enemy_buildings2 >=
        enemy->wall_height + enemy->tower_height) {
        card_power += 9999;
    }

    if ((pCard->to_player_tower2 + pCard->to_pl_enm_tower2 +
         pCard->to_player_tower + pCard->to_pl_enm_tower +
         player->tower_height) >= max_tower_height) {
        card_power += 9999;
    }

    return card_power;
}

bool OpponentsAITurn(int player_num) {
    if (player_num == 0) __debugbreak();

    int ai_player_cards_count = GetPlayerHandCardCount(player_num);
    if (ai_player_cards_count == 0) return true;

    // opponent_mastery = 2; // testing

    opponents_turn = 1;
    if (opponent_mastery == 0) {
        // select card at random to play
        int random_card_slot;
        if (need_to_discard_card == 0) {
            for (int i = 0; i < 10; ++i) {
                random_card_slot = rand_interval(0, ai_player_cards_count - 1);
                if (CanCardBePlayed(player_num, random_card_slot))
                    return PlayCard(player_num, random_card_slot);
            }
        }

        // if that fails discard card at random
        random_card_slot = rand_interval(0, ai_player_cards_count - 1);
        return DiscardCard(player_num, random_card_slot);

    } else if ((opponent_mastery == 1) || (opponent_mastery == 2)) {
        // apply some cunning
        ArcomagePlayer* player = &am_Players[player_num];
        ArcomagePlayer* enemy = &am_Players[(player_num + 1) % 2];

        // wipe cards power array - set negative for unfilled card slots
        for (int i = 0; i < 10; ++i) {
            if (i >= ai_player_cards_count) {
                cards_power[i].slot_index = -1;
                cards_power[i].card_power = -9999;
            } else {
                cards_power[i].slot_index = i;
                cards_power[i].card_power = 0;
            }
        }

        // calculate how effective each card would be
        for (int i = 0; i < ai_player_cards_count; ++i) {
            ArcomageCard* calc_card = &pCards[am_Players[player_num].cards_at_hand[cards_power[i].slot_index]];
            cards_power[i].card_power =
                CalculateCardPower(player, enemy, calc_card, opponent_mastery - 1);
        }

        // bubble sort the card powers in order
        for (int j = ai_player_cards_count - 1; j >= 0; --j) {
            for (int m = 0; m < j; ++m) {
                if (cards_power[m].card_power < cards_power[m + 1].card_power) {
                    int tempslot = cards_power[m].slot_index;
                    int temppow = cards_power[m].card_power;
                    cards_power[m].slot_index = cards_power[m + 1].slot_index;
                    cards_power[m].card_power = cards_power[m + 1].card_power;
                    cards_power[m + 1].slot_index = tempslot;
                    cards_power[m + 1].card_power = temppow;
                }
            }
        }

        // if we have to discard pick the least powerful to chuck
        int discard_slot = 0;
        for (int i = ai_player_cards_count - 1; i; --i) {
            if (i >= 0) {
                if (pCards[am_Players[player_num].cards_at_hand[cards_power[i].slot_index]].can_be_discarded) {
                    discard_slot = cards_power[i].slot_index;
                }
            }
        }

        if (!need_to_discard_card) {
            // try and play most powerful card
            for (int i = 0; i < ai_player_cards_count - 1; ++i) {
                if (CanCardBePlayed(player_num, cards_power[i].slot_index) && cards_power[i].card_power) {
                    return PlayCard(player_num, cards_power[i].slot_index);
                }
            }
        }

        // fall back - have to discard
        return DiscardCard(player_num, discard_slot);
    }
    return true;  // result != 0;
}

void ArcomageGame::Loop() {
    bool am_turn_not_finished = 0;
    while (!pArcomageGame->GameOver) {
        pArcomageGame->force_redraw_1 = 1;
        am_turn_not_finished = 1;
        IncreaseResourcesInTurn(current_player_num);
        // LABEL_8:
        while (am_turn_not_finished) {
            played_card_id = -1;
            GetNextCardFromDeck(current_player_num);
            while (1) {
                am_turn_not_finished = PlayerTurn(current_player_num);
                if (GetPlayerHandCardCount(current_player_num) <=
                    minimum_cards_at_hand) {
                    need_to_discard_card = 0;
                    break;
                }
                need_to_discard_card = 1;
                if (pArcomageGame->force_am_exit) break;
            }
        }
        pArcomageGame->GameOver = IsGameOver();
        if (!pArcomageGame->GameOver) TurnChange();
        if (pArcomageGame->force_am_exit) pArcomageGame->GameOver = 1;
    }


    GameResultsApply();

    if (pArcomageGame->check_exit == 0) {
        // add in pause till keypress here
        ArcomageGame_InputMSG v10;
        int frame_quant_time;
        int cnt = 0;
        while (1) {
            // frame limiter apprx 32fps 128 / 32 = 4
            do {
                frame_quant_time = (int)pEventTimer->Time() - (int)pArcomageGame->event_timer_time;
            } while (frame_quant_time < 4);
            pArcomageGame->event_timer_time = (int)pEventTimer->Time();

            ArcomageGame::MsgLoop(20, &v10);
            if (v10.am_input_type == 1) {
                if (v10.field_4) break;
                continue;
            }
            if ((v10.am_input_type == 7) || (v10.am_input_type == 8)) break;
            if (v10.am_input_type == 10) break;

            Point explos_coords;

            cnt++;
            if (cnt >= 8) {
                cnt = 0;
                if (pArcomageGame->uGameWinner == 1) {
                    if (am_Players[1].tower_height > 0) {
                        int div = (am_Players[1].tower_height / 10);
                        if (div == 0) div = 1;
                        am_Players[1].tower_height -= div;
                        explos_coords.x = 514;
                        explos_coords.y = 296;
                        new_explosion_effect(&explos_coords, -div);
                    }
                    if (am_Players[1].wall_height > 0) {
                        int div = (am_Players[1].wall_height / 10);
                        if (div == 0) div = 1;
                        am_Players[1].wall_height -= div;
                        explos_coords.x = 442;
                        explos_coords.y = 296;
                        new_explosion_effect(&explos_coords, -div);
                    }
                } else {
                    if (am_Players[0].tower_height > 0) {
                        int div = (am_Players[0].tower_height / 10);
                        if (div == 0) div = 1;
                        am_Players[0].tower_height -= div;
                        explos_coords.x = 122;
                        explos_coords.y = 296;
                        new_explosion_effect(&explos_coords, -div);
                    }
                    if (am_Players[0].wall_height > 0) {
                        int div = (am_Players[0].wall_height / 10);
                        if (div == 0) div = 1;
                        am_Players[0].wall_height -= div;
                        explos_coords.x = 180;
                        explos_coords.y = 296;
                        new_explosion_effect(&explos_coords, -div);
                    }
                }
            }

            // draw you lost/ you won + fireworks

            int rand = rand_interval(0, 38);
            if (rand == 38) {
                if (pArcomageGame->uGameWinner == 1) {
                    explos_coords.x = rand_interval(75, 175);
                    explos_coords.y = rand_interval(50, 150);
                    new_explosion_effect(&explos_coords, 5);
                } else {
                    explos_coords.x = rand_interval(465, 565);
                    explos_coords.y = rand_interval(50, 150);
                    new_explosion_effect(&explos_coords, 5);
                }
            }


            DrawGameUI(0);
        }
    }

    for (int i = 0; i < 10; ++i) {
        am_effects_array[i].explosion_eff->Clear(1, 1);
        am_effects_array[i].explosion_eff->Free();
    }

    pArcomageGame->pGameBackground->Release();
    pArcomageGame->pGameBackground = nullptr;

    pArcomageGame->pSprites->Release();
    pArcomageGame->pSprites = nullptr;

    pArcomageGame->bGameInProgress = false;
    viewparams->bRedrawGameUI = true;

    if (pMovie_Track) BackToHouseMenu();
}

void SetStartGameData() {
    signed int j;                       // edx@7
    int card_id_counter;                // edx@13
    signed int i;                       // ecx@13
    signed int card_dispenser_counter;  // eax@13

    SetStartConditions();

    current_player_num = !Player_Gets_First_Turn;
    strcpy(am_Players[1].pPlayerName, pArcomageGame->pPlayer2Name);
    am_Players[1].IsHisTurn = 0;  // !Player_Gets_First_Turn;
    strcpy(am_Players[0].pPlayerName, pArcomageGame->pPlayer1Name);
    am_Players[0].IsHisTurn = 1;  // Player_Gets_First_Turn;

    for (i = 0; i < 2; ++i) {
        am_Players[i].tower_height = start_tower_height;
        am_Players[i].wall_height = start_wall_height;
        am_Players[i].quarry_level = start_quarry_level;
        am_Players[i].magic_level = start_magic_level;
        am_Players[i].zoo_level = start_zoo_level;
        am_Players[i].resource_bricks = start_bricks_amount;
        am_Players[i].resource_gems = start_gems_amount;
        am_Players[i].resource_beasts = start_beasts_amount;

        for (j = 0; j < 10; ++j) {
            am_Players[i].cards_at_hand[j] = -1;
            if (Player_Cards_Shift) {
                am_Players[i].card_shift[j].x = -1;
                am_Players[i].card_shift[j].y = -1;
            } else {
                am_Players[i].card_shift[j].x = 0;
                am_Players[i].card_shift[j].y = 0;
            }
        }
    }
    strcpy(deckMaster.name, "Master Deck");
    for (i = 0, card_dispenser_counter = -2, card_id_counter = 0; i < DECK_SIZE;
         ++i, ++card_dispenser_counter) {
        deckMaster.cardsInUse[i] = 0;
        deckMaster.cards_IDs[i] = card_id_counter;
        switch (card_dispenser_counter) {
            case 0:
            case 2:
            case 6:
            case 9:
            case 13:
            case 18:
            case 23:
            case 33:
            case 36:
            case 38:
            case 44:
            case 46:
            case 52:
            case 57:
            case 69:
            case 71:
            case 75:
            case 79:
            case 81:
            case 84:
            case 89:
                break;
            default:
                ++card_id_counter;
        }
    }
    FillPlayerDeck();
}

void FillPlayerDeck() {
    int m;
    int rand_deck_pos;
    char card_taken_flags[DECK_SIZE];
    int i, j;

    ArcomageGame::PlaySound(20);
    memset(deckMaster.cardsInUse, 0, DECK_SIZE);
    memset(card_taken_flags, 0, DECK_SIZE);

    for (i = 0; i < 2; ++i) {
        for (j = 0; j < 10; ++j) {
            if (am_Players[i].cards_at_hand[j] > -1) {
                for (m = 0; m < DECK_SIZE; ++m) {
                    if (deckMaster.cards_IDs[m] ==
                            am_Players[i].cards_at_hand[j] &&
                        deckMaster.cardsInUse[m] == 0) {
                        // mark which cards are already in players hands
                        deckMaster.cardsInUse[m] = 1;
                        break;
                    }
                }
            }
        }
    }

    for (i = 0; i < DECK_SIZE; ++i) {
        do {
            rand_deck_pos = rand() % DECK_SIZE;
        } while (card_taken_flags[rand_deck_pos] == 1);

        card_taken_flags[rand_deck_pos] = 1;
        playDeck.cards_IDs[i] = deckMaster.cards_IDs[rand_deck_pos];
        playDeck.cardsInUse[i] = deckMaster.cardsInUse[rand_deck_pos];
    }

    deck_walk_index = 0;
    pArcomageGame->force_redraw_1 = 1;
}

void InitalHandsFill() {
    for (int i = 0; i < minimum_cards_at_hand; ++i) {
        // GetNextCardFromDeck(0);
        // GetNextCardFromDeck(1);
        GetNextCardFromDeck(Player_Gets_First_Turn);
    }
    pArcomageGame->force_redraw_1 = 1;
}

void GetNextCardFromDeck(int player_num) {
    signed int deck_index;      // eax@1
    int new_card_id;            // edi@1
    signed int card_slot_indx;  // eax@7

    deck_index = deck_walk_index;
    for (;;) {
        if (deck_index >= DECK_SIZE) {
            FillPlayerDeck();
            deck_index = deck_walk_index = 0;
        }
        if (!playDeck.cardsInUse[deck_index]) {
            new_card_id = playDeck.cards_IDs[deck_index];
            ++deck_index;
            deck_walk_index = deck_index;
            break;
        }
        ++deck_index;
        deck_walk_index = deck_index;
    }

    ArcomageGame::PlaySound(21);
    card_slot_indx = GetEmptyCardSlotIndex(player_num);
    if (card_slot_indx != -1) {
        drawn_card_slot_index = card_slot_indx;
        am_Players[player_num].cards_at_hand[card_slot_indx] = new_card_id;
        am_Players[player_num].card_shift[card_slot_indx].x = rand_interval(-4, 4);
        am_Players[player_num].card_shift[card_slot_indx].y = rand_interval(-4, 4);
        pArcomageGame->force_redraw_1 = 1;
        drawn_card_anim_start = 1;
    }
}

int GetEmptyCardSlotIndex(int player_num) {
    // find first empty card slot
    for (int i = 0; i < 10; ++i) {
        if (am_Players[player_num].cards_at_hand[i] == -1) return i;
    }
    return -1;
}

void IncreaseResourcesInTurn(int player_num) {
    // increase player resources
    am_Players[player_num].resource_bricks +=
        quarry_bonus + am_Players[player_num].quarry_level;
    am_Players[player_num].resource_gems +=
        magic_bonus + am_Players[player_num].magic_level;
    am_Players[player_num].resource_beasts +=
        zoo_bonus + am_Players[player_num].zoo_level;
}

void TurnChange() {
    char player_name[64];    // [sp+4h] [bp-64h]@4
    ArcomageGame_InputMSG v10;  // [sp+54h] [bp-14h]@7
    Point v11;               // [sp+60h] [bp-8h]@4

    if (!pArcomageGame->force_am_exit) {
        if (am_Players[0].IsHisTurn != 1 || am_Players[1].IsHisTurn != 1) {
            ++current_player_num;
            hide_card_anim_start = 1;
            if (current_player_num >= 2) current_player_num = 0;
        } else {
            // this is never called - pause when switching turns

            // nullsub_1();
            //   v11.x = 0;
            //   v11.y = 0;
            strcpy(player_name, "The Next Player is: ");  //"След"
                                                            // v0 = 0;
            v11.y = 200;
            v11.x = 320;  // - 12 * v0 / 2;
            am_DrawText(player_name, &v11);
            hide_card_anim_start = 1;
            ++current_player_num;
            if (current_player_num >= 2) current_player_num = 0;
            strcpy(player_name, am_Players[current_player_num].pPlayerName);
            // v4 = 0;
            v11.y = 260;
            v11.x = 320;  // - 12 * v4 / 2;
            am_DrawText(player_name, &v11);
            /* v6.left = 0;
            v6.right = 640;
            v6.top = 0;
            v6.bottom = 480;*/
            // nullsub_1();
            render->Present();
            // nullsub_1();
            while (1) {
                while (!ArcomageGame::MsgLoop(20, &v10));
                if (v10.am_input_type == 1) {
                    if (v10.field_4) break;
                    // nullsub_1();
                    continue;
                }
                if ((v10.am_input_type > 4) && (v10.am_input_type <= 8)) break;
                if (v10.am_input_type == 10) {
                    pArcomageGame->force_am_exit = 1;
                    // byte_4FAA74 = 1;
                    break;
                }
            }
            /*  v11.x = 0;
            v11.y = 0;
            v6.left = 0;
            v6.right = 640;
            v6.top = 0;
            v6.bottom = 480;*/
            // nullsub_1();
            render->Present();
        }
    }
}

bool IsGameOver() {
    // check if victory conditions have been met
    bool result = false;
    for (int i = 0; i < 2; ++i) {
        if (am_Players[i].tower_height <= 0) result = true;
        if (am_Players[i].tower_height >= max_tower_height) result = true;
        if (am_Players[i].resource_bricks >= max_resources_amount ||
            am_Players[i].resource_gems >= max_resources_amount ||
            am_Players[i].resource_beasts >= max_resources_amount)
            result = true;
    }

    return result;
}

char PlayerTurn(int player_num) {
    Rect pSrcXYZW;
    Point pTargetXY;
    ArcomageGame_InputMSG get_message;

    // reset player turn
    opponents_turn = 0;
    num_actions_left = 0;

    // reset animations
    int animation_stage = 20;
    drawn_card_anim_cnt = 10;
    hide_card_anim_count = 5;
    discarded_card_id = -1;
    if (drawn_card_slot_index != -1) drawn_card_anim_start = 1;

    // timing and loop
    int frame_quant_time;
    bool break_loop = false;
    do {
        // frame limiter apprx 32fps 128 / 32 = 4
        do {
            frame_quant_time = (int)pEventTimer->Time() - (int)pArcomageGame->event_timer_time;
        } while (frame_quant_time < 4);
        pArcomageGame->event_timer_time = (int)pEventTimer->Time();

        // get input message
        if (pArcomageGame->force_am_exit) break_loop = true;
        ArcomageGame::MsgLoop(0, &get_message);
        switch (get_message.am_input_type) {
            case 2:  // unkown ??
                if (get_message.field_4 == 129 && get_message.am_input_key == 1) {
                    pAudioPlayer->StopChannels(-1, -1);
                    num_actions_left = 0;
                    break_loop = true;
                    pArcomageGame->force_am_exit = 1;
                }
                break;
            case 9:  // toggle fullscreen
                pArcomageGame->force_redraw_1 = 1;
                break;
            case 10:  // hit escape key
                if (pArcomageGame->check_exit == 1) {
                    pAudioPlayer->StopChannels(-1, -1);
                    pArcomageGame->GameOver = 1;
                    pArcomageGame->uGameWinner = 2;
                    pArcomageGame->Victory_type = -2;
                    break_loop = true;
                    pArcomageGame->force_am_exit = 1;
                } else {
                    pArcomageGame->check_exit = 1;
                    pArcomageGame->force_redraw_1 = 1;
                }
                break;
        }

        // time to start the AIs turn
        if (am_Players[current_player_num].IsHisTurn != 1 && !opponents_turn &&
            !playdiscard_anim_start && !drawn_card_anim_start) {
            if (hide_card_anim_start) hide_card_anim_runnning = 1;
            OpponentsAITurn(current_player_num);
            playdiscard_anim_start = 1;
        }

        if (drawn_card_slot_index != -1 && drawn_card_anim_cnt > 10) drawn_card_anim_cnt = 10;

        if (playdiscard_anim_start || drawn_card_anim_start || am_Players[current_player_num].IsHisTurn != 1) {
            // player cant act
            pArcomageGame->force_redraw_1 = 1;

            // card drawing animation
            if (drawn_card_anim_start) {
                --drawn_card_anim_cnt;
                if (drawn_card_anim_cnt < 0) {
                    drawn_card_anim_start = 0;
                    drawn_card_anim_cnt = 10;
                    break_loop = false;
                    if (GetPlayerHandCardCount(current_player_num) <= minimum_cards_at_hand) {
                        GetNextCardFromDeck(current_player_num);
                    }
                }
            }

            // card played/ discarded animation
            if (playdiscard_anim_start) {
                --animation_stage;
                if (animation_stage < 0) {
                    if (num_actions_left > 1) {
                        --num_actions_left;
                        opponents_turn = 0;
                    } else {
                        break_loop = true;
                    }
                    playdiscard_anim_start = 0;
                    animation_stage = 20;
                }
            }
        } else {
            // can play cards
            if (need_to_discard_card) {
                // any mouse - try and discard
                if ((get_message.am_input_type == 7 || get_message.am_input_type == 8) && DiscardCard(player_num, current_card_slot_index)) {
                    if (hide_card_anim_start) hide_card_anim_runnning = 1;
                    if (num_cards_to_discard > 0) {
                        --num_cards_to_discard;
                        need_to_discard_card = (GetPlayerHandCardCount(player_num) > minimum_cards_at_hand);
                    }
                    playdiscard_anim_start = 1;
                }
            } else {
                if (get_message.am_input_type == 7) {
                    // left mouse - try and play card
                    if (PlayCard(player_num, current_card_slot_index)) {
                        playdiscard_anim_start = 1;
                        if (hide_card_anim_start) hide_card_anim_runnning = 1;
                    }
                }
                if (get_message.am_input_type == 8) {
                    // right mouse - try and discard card
                    if (DiscardCard(player_num, current_card_slot_index)) {
                        playdiscard_anim_start = 1;
                        if (hide_card_anim_start) hide_card_anim_runnning = 1;
                    }
                }
            }
        }

        // if the hover rectangle has moved redraw
        if (Card_Hover_Index != DrawCardsRectangles(player_num)) {
            Card_Hover_Index = DrawCardsRectangles(player_num);
            pArcomageGame->force_redraw_1 = 1;
        }

        // do we need to draw
        if (pArcomageGame->force_redraw_1) {
            DrawGameUI(animation_stage);
            pArcomageGame->force_redraw_1 = 0;
        }
    } while (!break_loop);

    return num_actions_left > 0;
}

void DrawGameUI(int animation_stage) {
    // set copy image location
    pArcomageGame->pBlit_Copy_pixels = pArcomageGame->pSpritesPixels;
    // draw background
    render->DrawTextureNew(0, 0, pArcomageGame->pGameBackground);

    // draw gui items
    DrawRectanglesForText();
    DrawCards();          //рисуем карты
    DrawPlayersTowers();  //рисуем башню
    DrawPlayersWall();    //рисуем стену
    DrawPlayersText();    //рисуем текст

    DrawCardAnimation(animation_stage);
    current_card_slot_index = DrawCardsRectangles(current_player_num);

    // update explosion effects
    for (int i = 0; i < 10; ++i) {
        if (am_effects_array[i].have_effect) am_effects_array[i].explosion_eff->UpdateEffect();
    }
    DrawSparks();

    // draw texts
    if (pArcomageGame->check_exit) {
        String t = "Are you sure you want to resign?\n    Press ESC again to confirm";
        Point p(200, 200);
        am_DrawText(t, &p);
    }
    if (pArcomageGame->GameOver) {
        String t = "Game Over!";
        Point p(270, 200);
        am_DrawText(t, &p);
    }

    render->Present();
}

void DrawRectanglesForText() {
    Rect pSrcRect;
    Point pTargetXY;

    // resources rectangles
    pSrcRect.x = 765;
    pSrcRect.y = 0;
    pSrcRect.z = 843;
    pSrcRect.w = 216;

    pTargetXY.x = 8;
    pTargetXY.y = 56;
    render->am_Blt_Chroma(&pSrcRect, &pTargetXY, 0, 2);

    pTargetXY.x = 555;
    pTargetXY.y = 56;
    render->am_Blt_Chroma(&pSrcRect, &pTargetXY, 0, 2);

    // players name rectangle
    pSrcRect.x = 283;
    pSrcRect.y = 166;
    pSrcRect.z = 361;
    pSrcRect.w = 190;
    pTargetXY.x = 8;
    pTargetXY.y = 13;
    render->am_Blt_Chroma(&pSrcRect, &pTargetXY, pArcomageGame->field_54, 2);

    pTargetXY.x = 555;
    pTargetXY.y = 13;
    render->am_Blt_Chroma(&pSrcRect, &pTargetXY, pArcomageGame->field_54, 2);

    // tower height rectangle
    pSrcRect.x = 234;
    pSrcRect.y = 166;
    pSrcRect.z = 283;
    pSrcRect.w = 190;
    pTargetXY.x = 100;
    pTargetXY.y = 296;
    render->am_Blt_Chroma(&pSrcRect, &pTargetXY, pArcomageGame->field_54, 2);

    pTargetXY.x = 492;
    pTargetXY.y = 296;
    render->am_Blt_Chroma(&pSrcRect, &pTargetXY, pArcomageGame->field_54, 2);

    // wall height rectangle
    pSrcRect.x = 192;
    pSrcRect.y = 166;
    pSrcRect.z = 234;
    pSrcRect.w = 190;
    pTargetXY.x = 168;
    pTargetXY.y = 296;
    render->am_Blt_Chroma(&pSrcRect, &pTargetXY, pArcomageGame->field_54, 2);

    pTargetXY.x = 430;
    pTargetXY.y = 296;
    render->am_Blt_Chroma(&pSrcRect, &pTargetXY, pArcomageGame->field_54, 2);
}

void DrawPlayersText() {
    int res_value;
    char text_buff[32];
    Point text_position;

    if (need_to_discard_card) {
        // DISCARD A CARD
        strcpy(text_buff, localization->GetString(266));
        text_position.x = 320 - pArcomageGame->pfntArrus->GetLineWidth(text_buff) / 2;
        text_position.y = 306;
        am_DrawText(text_buff, &text_position);
    }

    // player names
    strcpy(text_buff, am_Players[0].pPlayerName);
    if (!current_player_num) strcat(text_buff, "***");
    text_position.x = 47 - pArcomageGame->pfntComic->GetLineWidth(text_buff) / 2;
    text_position.y = 21;
    am_DrawText(text_buff, &text_position);

    strcpy(text_buff, am_Players[1].pPlayerName);
    if (current_player_num == 1) strcat(text_buff, "***");
    text_position.x = 595 - pArcomageGame->pfntComic->GetLineWidth(text_buff) / 2;
    text_position.y = 21;
    am_DrawText(text_buff, &text_position);

    // tower heights
    am_IntToString(am_Players[0].tower_height, text_buff);
    text_position.x = 123 - pArcomageGame->pfntComic->GetLineWidth(text_buff) / 2;
    text_position.y = 305;
    am_DrawText(text_buff, &text_position);

    am_IntToString(am_Players[1].tower_height, text_buff);
    text_position.x = 515 - pArcomageGame->pfntComic->GetLineWidth(text_buff) / 2;
    text_position.y = 305;
    am_DrawText(text_buff, &text_position);

    // wall heights
    am_IntToString(am_Players[0].wall_height, text_buff);
    text_position.x = 188 - pArcomageGame->pfntComic->GetLineWidth(text_buff) / 2;
    text_position.y = 305;
    am_DrawText(text_buff, &text_position);

    am_IntToString(am_Players[1].wall_height, text_buff);
    text_position.x = 451 - pArcomageGame->pfntComic->GetLineWidth(text_buff) / 2;
    text_position.y = 305;
    am_DrawText(text_buff, &text_position);

    // quarry levels
    res_value = am_Players[0].quarry_level;
    if (use_start_bonus) res_value = am_Players[0].quarry_level + quarry_bonus;
    text_position.x = 14;
    text_position.y = 92;
    DrawPlayerLevels(StringFromInt(res_value), &text_position);

    res_value = am_Players[1].quarry_level;
    if (use_start_bonus) res_value = am_Players[1].quarry_level + quarry_bonus;
    text_position.y = 92;
    text_position.x = 561;
    DrawPlayerLevels(StringFromInt(res_value), &text_position);

    // magic levels
    res_value = am_Players[0].magic_level;
    if (use_start_bonus) res_value = am_Players[0].magic_level + magic_bonus;
    text_position.y = 164;
    text_position.x = 14;
    DrawPlayerLevels(StringFromInt(res_value), &text_position);

    res_value = am_Players[1].magic_level;
    if (use_start_bonus) res_value = am_Players[1].magic_level + magic_bonus;
    text_position.y = 164;
    text_position.x = 561;
    DrawPlayerLevels(StringFromInt(res_value), &text_position);

    // zoo levels
    res_value = am_Players[0].zoo_level;
    if (use_start_bonus) res_value = am_Players[0].zoo_level + zoo_bonus;
    text_position.y = 236;
    text_position.x = 14;
    DrawPlayerLevels(StringFromInt(res_value), &text_position);

    res_value = am_Players[1].zoo_level;
    if (use_start_bonus) res_value = am_Players[1].zoo_level + zoo_bonus;
    text_position.y = 236;
    text_position.x = 561;
    DrawPlayerLevels(StringFromInt(res_value), &text_position);

    // bricks
    text_position.y = 114;
    text_position.x = 10;
    DrawBricksCount(StringFromInt(am_Players[0].resource_bricks), &text_position);

    text_position.x = 557;
    text_position.y = 114;
    DrawBricksCount(StringFromInt(am_Players[1].resource_bricks), &text_position);

    // gems
    text_position.x = 10;
    text_position.y = 186;
    DrawGemsCount(StringFromInt(am_Players[0].resource_gems), &text_position);

    text_position.x = 557;
    text_position.y = 186;
    DrawGemsCount(StringFromInt(am_Players[1].resource_gems), &text_position);

    // beasts
    text_position.x = 10;
    text_position.y = 258;
    DrawBeastsCount(StringFromInt(am_Players[0].resource_beasts), &text_position);

    text_position.x = 557;
    text_position.y = 258;
    DrawBeastsCount(StringFromInt(am_Players[1].resource_beasts), &text_position);
}

void DrawPlayerLevels(const String &str, Point *pXY) {
    Rect pSrcRect;
    Point pTargetPoint;

    pTargetPoint.x = pXY->x;
    pTargetPoint.y = pXY->y;
    for (auto i = str.begin(); i != str.end(); ++i) {
        // get each digit
        if (*i) {
            // calc position in sprite layout
            int v7 = 22 * *i;
            pSrcRect.z = v7 - 842;
            pSrcRect.x = v7 - 864;
            pSrcRect.y = 190;
            pSrcRect.w = 207;
            // draw digit
            render->am_Blt_Chroma(&pSrcRect, &pTargetPoint, pArcomageGame->field_54, 1);
            pTargetPoint.x += 22;
        }
    }
}

void DrawBricksCount(const String &str, Point *pXY) {
    Rect pSrcRect;
    Point pTargetPoint;

    pTargetPoint.x = pXY->x;
    pTargetPoint.y = pXY->y;
    for (auto i = str.begin(); i != str.end(); ++i) {
        // get each digit
        if (*i) {
            // calc position in sprite layout
            int v7 = 13 * *i;
            pSrcRect.x = v7 - 370;
            pSrcRect.z = v7 - 357;
            pSrcRect.y = 128;
            pSrcRect.w = 138;
            // draw digit
            render->am_Blt_Chroma(&pSrcRect, &pTargetPoint, 0, 2);
            pTargetPoint.x += 13;
        }
    }
}

void DrawGemsCount(const String &str, Point *pXY) {
    Rect pSrcRect;
    Point pTargetPoint;

    pTargetPoint.x = pXY->x;
    pTargetPoint.y = pXY->y;
    for (auto i = str.begin(); i != str.end(); ++i) {
        // get each digit
        if (*i) {
            // calc position in sprite layout
            int  v7 = 13 * *i;
            pSrcRect.x = v7 - 370;
            pSrcRect.z = v7 - 357;
            pSrcRect.y = 138;
            pSrcRect.w = 148;
            // draw digit
            render->am_Blt_Chroma(&pSrcRect, &pTargetPoint, 0, 2);
            pTargetPoint.x += 13;
        }
    }
}

void DrawBeastsCount(const String &str, Point *pXY) {
    Rect pSrcRect;
    Point pTargetPoint;

    pTargetPoint.x = pXY->x;
    pTargetPoint.y = pXY->y;
    for (auto i = str.begin(); i != str.end(); ++i) {
        // get each digit
        if (*i) {
            // calc position in sprite layout
            int x_offset = 13 * *i;
            pSrcRect.x = x_offset - 370;
            pSrcRect.z = x_offset - 357;
            pSrcRect.y = 148;
            pSrcRect.w = 158;
            // draw digit
            render->am_Blt_Chroma(&pSrcRect, &pTargetPoint, 0, 2);
            pTargetPoint.x += 13;
        }
    }
}

void DrawPlayersTowers() {
    Rect pSrcXYZW;
    Point pTargetXY;

    // draw player 0 tower
    int tower_height = am_Players[0].tower_height;
    // check limits
    if (tower_height > max_tower_height) tower_height = max_tower_height;
    pSrcXYZW.y = 0;
    pSrcXYZW.x = 892;
    pSrcXYZW.z = 937;
    // calc height ratio
    int tower_top = 200 * tower_height / max_tower_height;
    pSrcXYZW.w = tower_top;
    pTargetXY.x = 102;
    pTargetXY.y = 297 - tower_top;
    if (tower_height > 0) render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY, 0, 2);  //стена башни

    // draw player 0 top
    pSrcXYZW.y = 0;
    pSrcXYZW.x = 384;
    pSrcXYZW.z = 452;
    pSrcXYZW.w = 94;
    pTargetXY.y = 203 - tower_top;
    pTargetXY.x = 91;
    render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY, pArcomageGame->field_54, 2);  //верхушка башни

    // draw player 1 tower
    tower_height = am_Players[1].tower_height;
    // set limits
    if (tower_height > max_tower_height) tower_height = max_tower_height;
    // calc tower height ratio
    tower_top = 200 * tower_height / max_tower_height;
    pSrcXYZW.y = 0;
    pSrcXYZW.x = 892;
    pSrcXYZW.z = 937;
    pSrcXYZW.w = tower_top;
    pTargetXY.x = 494;
    pTargetXY.y = 297 - tower_top;
    if (tower_height > 0) render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY, 0, 2);

    // draw tower 1 top
    pSrcXYZW.x = 384;
    pSrcXYZW.z = 452;
    pSrcXYZW.y = 94;
    pSrcXYZW.w = 188;
    pTargetXY.x = 483;
    pTargetXY.y = 203 - tower_top;
    render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY, pArcomageGame->field_54, 2);
}

void DrawPlayersWall() {
    Rect pSrcXYZW;
    Point pTargetXY;

    // draw player 0 wall
    int player_0_h = am_Players[0].wall_height;
    // fix limit
    if (player_0_h > 100) player_0_h = 100;

    // is there any wall the draw
    if (player_0_h > 0) {
        pSrcXYZW.y = 0;
        pSrcXYZW.x = 843;
        // calc ratio of wall to draw
        int player_0_pixh = 200 * player_0_h / 100;
        pSrcXYZW.z = 867;
        pSrcXYZW.w = player_0_pixh;
        pTargetXY.x = 177;
        pTargetXY.y = 297 - player_0_pixh;
        render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY, pArcomageGame->field_54, 2);
    }

    // draw player 1 wall
    int player_1_h = am_Players[1].wall_height;
    if (player_1_h > 100) player_1_h = 100;
    if (player_1_h > 0) {
        pSrcXYZW.y = 0;
        pSrcXYZW.x = 843;
        int player_1_pixh = 200 * player_1_h / 100;
        pSrcXYZW.z = 867;
        pSrcXYZW.w = player_1_pixh;
        pTargetXY.x = 439;
        pTargetXY.y = 297 - player_1_pixh;
        render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY, pArcomageGame->field_54, 2);
    }
}

void DrawCards() {
    // draw hand of current player and any cards on the table
    Rect pSrcXYZW;
    Point pTargetXY;

    // draw player hand
    int card_count = GetPlayerHandCardCount(current_player_num);
    pTargetXY.y = 327;
    int card_spacing = (window->GetWidth() - 96 * card_count) / (card_count + 1);
    pTargetXY.x = card_spacing;

    for (int card_slot = 0; card_slot < card_count; ++card_slot) {
        // shift card pos
        if (Player_Cards_Shift) {
            pTargetXY.x += am_Players[current_player_num].card_shift[card_slot].x;
            pTargetXY.y += am_Players[current_player_num].card_shift[card_slot].y;
        }

        if (am_Players[current_player_num].cards_at_hand[card_slot] == -1) {
            // need to acess another slot if card sent for animatoin
            ++card_count;
        } else if (card_slot != drawn_card_slot_index) {
            // draw back of card for opponents turn
            if (am_Players[current_player_num].IsHisTurn == 0 && See_Opponents_Cards == 0) {
                pSrcXYZW.x = 192;
                pSrcXYZW.z = 288;
                pSrcXYZW.y = 0;
                pSrcXYZW.w = 128;
                render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY, 0, 2);  //рисуется оборотные стороны карт противника
            } else {
                pArcomageGame->GetCardRect(am_Players[current_player_num].cards_at_hand[card_slot], &pSrcXYZW);
                if (!CanCardBePlayed(current_player_num, card_slot)) {
                    // рисуются неактивные карты - greyed out
                    render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY, 0, 0);
                } else {
                    render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY, 0, 2);  //рисуются активные карты
                }
            }
        }

        // unshift by card pos
        if (Player_Cards_Shift) {
            pTargetXY.x -= am_Players[current_player_num].card_shift[card_slot].x;
            pTargetXY.y -= am_Players[current_player_num].card_shift[card_slot].y;
        }

        // shift draw postion along
        pTargetXY.x += card_spacing + 96;
    }

    //  draw shown cards on table
    for (int table_cards = 0; table_cards < 10; ++table_cards) {
        if (hide_card_anim_runnning == 0) {
            // cards stationary
            if (shown_cards[table_cards].uCardId != -1) {
                // draw card - greyed out
                pArcomageGame->GetCardRect(shown_cards[table_cards].uCardId, &pSrcXYZW);
                render->am_Blt_Chroma(&pSrcXYZW, &shown_cards[table_cards].hide_anim_pos, 0, 0);
            }
            if (shown_cards[table_cards].discarded != 0) {
                // draw discarded text
                pTargetXY.x = shown_cards[table_cards].hide_anim_pos.x + 12;
                pTargetXY.y = shown_cards[table_cards].hide_anim_pos.y + 40;
                pSrcXYZW.x = 843;
                pSrcXYZW.z = 916;
                pSrcXYZW.y = 200;
                pSrcXYZW.w = 216;
                render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY, pArcomageGame->field_54, 2);
            }
        } else if (hide_card_anim_count <= 0) {
            // animation finished
            if (table_cards == 9) {
                // reset anim at last card
                hide_card_anim_runnning = 0;
                hide_card_anim_start = 0;
                hide_card_anim_count = 5;
            }

            // reset table slot
            shown_cards[table_cards].uCardId = -1;
            shown_cards[table_cards].hide_anim_pos.x = shown_cards[table_cards].table_pos.x;
            shown_cards[table_cards].hide_anim_pos.y = shown_cards[table_cards].table_pos.y;
            shown_cards[table_cards].discarded = 0;
        } else {
            // animation
            if (shown_cards[table_cards].uCardId != -1) {
                // update position and draw
                shown_cards[table_cards].hide_anim_pos.x += shown_cards[table_cards].hide_anim_spd.x;
                shown_cards[table_cards].hide_anim_pos.y += shown_cards[table_cards].hide_anim_spd.y;
                pArcomageGame->GetCardRect(shown_cards[table_cards].uCardId, &pSrcXYZW);
                render->am_Blt_Chroma(&pSrcXYZW, &shown_cards[table_cards].hide_anim_pos, 0, 0);
            }
        }
    }

    // update hide card anim
    if (hide_card_anim_runnning != 0) --hide_card_anim_count;

    // blank deck card
    pSrcXYZW.x = 192;
    pSrcXYZW.z = 288;
    pSrcXYZW.y = 0;
    pSrcXYZW.w = 128;
    pTargetXY.x = 120;
    pTargetXY.y = 18;
    render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY, 0, 0);
}

void DrawCardAnimation(int animation_stage) {
    // draws any card currently in animation stage
    // draw - discard - play

    Rect pSrcXYZW;
    Point pTargetXY;

    // drawing card anim
    if (drawn_card_slot_index != -1) {
        if (drawn_card_anim_cnt >= 9) {
            // animation start so calcualte posiotn and speeds
            anim_card_pos_drawncard.y = 18;
            anim_card_pos_drawncard.x = 120;
            int card_count = GetPlayerHandCardCount(current_player_num);
            int card_spacing = (window->GetWidth() - (96 * card_count)) / (card_count + 1);

            int targetx = drawn_card_slot_index * (card_spacing + 96) + card_spacing;
            int targety = 327;

            if (Player_Cards_Shift) {
                targetx += am_Players[current_player_num].card_shift[drawn_card_slot_index].x;
                targety += am_Players[current_player_num].card_shift[drawn_card_slot_index].y;
            }

            anim_card_spd_drawncard.x = (targetx - (signed)anim_card_pos_drawncard.x) / 10;
            anim_card_spd_drawncard.y = (targety - (signed)anim_card_pos_drawncard.y) / 10;

            // move the card
            anim_card_pos_drawncard.y += anim_card_spd_drawncard.y;
            anim_card_pos_drawncard.x += anim_card_spd_drawncard.x;

            // draw the blank card at animation position
            pSrcXYZW.x = 192;
            pSrcXYZW.y = 0;
            pSrcXYZW.z = 288;
            pSrcXYZW.w = 128;
            render->am_Blt_Chroma(&pSrcXYZW, &anim_card_pos_drawncard, 0, 2);
        } else {
            // animation is running - update position and draw
            pSrcXYZW.x = 192;
            pSrcXYZW.y = 0;
            pSrcXYZW.z = 288;
            pSrcXYZW.w = 128;
            anim_card_pos_drawncard.x += anim_card_spd_drawncard.x;
            anim_card_pos_drawncard.y += anim_card_spd_drawncard.y;
            render->am_Blt_Chroma(&pSrcXYZW, &anim_card_pos_drawncard, 0, 2);

            // reset if animation is finished
            if (!drawn_card_anim_cnt) drawn_card_slot_index = -1;
        }
    }
    // end drawing card from deck anim

    // discard card anim
    if (discarded_card_id != -1) {
        // anim runs 20 -> 10
        if (animation_stage <= 10) {
            if (animation_stage == 10) {
                // card has reached its destination - find first table slot
                int card_slot = 0;
                for (card_slot = 0; card_slot < 10; ++card_slot) {
                    if (shown_cards[card_slot].uCardId == -1) {
                        // set card to slot
                        shown_cards[card_slot].uCardId = discarded_card_id;
                        shown_cards[card_slot].discarded = 1;
                        break;
                    }
                }

                // update positiona and draw
                pArcomageGame->GetCardRect(discarded_card_id, &pSrcXYZW);
                pTargetXY.x = shown_cards[card_slot].table_pos.x;
                pTargetXY.y = shown_cards[card_slot].table_pos.y;
                render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY, 0, 0);

                // reset discard anim
                discarded_card_id = -1;
            }
        } else {
            // update pos and draw
            pArcomageGame->GetCardRect(discarded_card_id, &pSrcXYZW);
            anim_card_pos_playdiscard.x += anim_card_spd_playdiscard.x;
            anim_card_pos_playdiscard.y += anim_card_spd_playdiscard.y;
            render->am_Blt_Chroma(&pSrcXYZW, &anim_card_pos_playdiscard, 0, 0);
        }
    }
    // end discard card anim

    // play card anim
    if (played_card_id != -1) {
        // 20 -> 16 move to play position
        if (animation_stage > 15.0) {
            // update positiona and draw
            pArcomageGame->GetCardRect(played_card_id, &pSrcXYZW);
            anim_card_pos_playdiscard.x += anim_card_spd_playdiscard.x;
            anim_card_pos_playdiscard.y += anim_card_spd_playdiscard.y;
            render->am_Blt_Chroma(&pSrcXYZW, &anim_card_pos_playdiscard, 0, 2);
            return;
        }

        // 15 -> 6 hold and play card
        if (animation_stage > 5) {
            if (animation_stage == 15) {
                // card arrived at centre - execute effects
                ApplyCardToPlayer(current_player_num, played_card_id);
            }

            // draw in centre
            pArcomageGame->GetCardRect(played_card_id, &pSrcXYZW);
            pTargetXY.x = 272;
            pTargetXY.y = 173;
            render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY, 0, 2);
            return;
        }

        // 5 set up move to table positon
        if (animation_stage == 5.0) {
            // find slots and update speeds
            anim_card_pos_playdiscard.x = 272;
            anim_card_pos_playdiscard.y = 173;
            int v17;
            for (v17 = 0; v17 < 10; ++v17) {
                if (shown_cards[v17].uCardId == -1) break;
            }
            anim_card_spd_playdiscard.x = ((int)shown_cards[v17].table_pos.x - 272) / 5;
            anim_card_spd_playdiscard.y = ((int)shown_cards[v17].table_pos.y - 173) / 5;

            // draw card in position
            pArcomageGame->GetCardRect(played_card_id, &pSrcXYZW);
            pTargetXY.x = 272;
            pTargetXY.y = 173;
            render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY, 0, 2);
            return;
        }

        // 4-> 1  move to table slot
        if (animation_stage > 0) {
            // update position and draw
            pArcomageGame->GetCardRect(played_card_id, &pSrcXYZW);
            anim_card_pos_playdiscard.x += anim_card_spd_playdiscard.x;
            anim_card_pos_playdiscard.y += anim_card_spd_playdiscard.y;
            render->am_Blt_Chroma(&pSrcXYZW, &anim_card_pos_playdiscard, 0, 0);
            return;
        }

        // 0 draw in table pos and reset
        if (animation_stage == 0) {
            // find free table slot
            int v19;
            for (v19 = 0; v19 < 10; ++v19) {
                if (shown_cards[v19].uCardId == -1) {
                    shown_cards[v19].uCardId = played_card_id;
                    break;
                }
            }

            // update position and draw
            pArcomageGame->GetCardRect(played_card_id, &pSrcXYZW);
            pTargetXY.x = shown_cards[v19].table_pos.x;
            pTargetXY.y = shown_cards[v19].table_pos.y;
            render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY, 0, 0);

            // reset anim
            played_card_id = -1;
        }
    }
    // end play card anim
}

void ArcomageGame::GetCardRect(unsigned int uCardID, Rect *pCardRect) {
    // get card image position from layout
    // slot encoded as 'YX' eg 76 = yslot 7 and xslot 6
    int xslot = pCards[uCardID].slot % 10;
    int yslot = pCards[uCardID].slot / 10;
    pCardRect->y = 128 * yslot + 220;
    pCardRect->x = 96 * xslot;
    pCardRect->w = pCardRect->y + 128;
    pCardRect->z = pCardRect->x + 96;
}

int GetPlayerHandCardCount(int player_num) {
    int card_count = 0;
    for (int i = 0; i < 10; ++i) {
        if (am_Players[player_num].cards_at_hand[i] != -1) ++card_count;
    }

    return card_count;
}

signed int DrawCardsRectangles(int player_num) {
    // draws the framing rectangle around cards on hover
    arcomage_mouse get_mouse;
    Rect pXYZW;
    int color;

    // only do for the human player
    if (am_Players[player_num].IsHisTurn) {
        // get the mouse position
        if (get_mouse.Update()) {
            // calc spacings and first card position
            int card_count = GetPlayerHandCardCount(player_num);
            int card_spacing = (window->GetWidth() - 96 * card_count) / (card_count + 1);
            pXYZW.y = 327;
            pXYZW.w = 455;
            pXYZW.x = card_spacing;
            int card_offset = pXYZW.x + 96;
            pXYZW.z = card_offset;

            // loop through hand of cards
            for (int hand_index = 0; hand_index < card_count; hand_index++) {
                // if there is a card
                if (am_Players[player_num].cards_at_hand[hand_index] != -1) {
                    // shift rectangle co ords
                    if (Player_Cards_Shift) {
                        pXYZW.x += am_Players[player_num].card_shift[hand_index].x;
                        pXYZW.z += am_Players[player_num].card_shift[hand_index].x;
                        pXYZW.y += am_Players[player_num].card_shift[hand_index].y;
                        pXYZW.w += am_Players[player_num].card_shift[hand_index].y;
                    }

                    // see if mouse is hovering
                    if (get_mouse.Inside(&pXYZW)) {
                        if (CanCardBePlayed(player_num, hand_index))
                            color = 0x00FFFFFF;  //белый цвет - white frame
                        else
                            color = 0x000000FF;  //красный цвет - red frame

                        // draw outline and return
                        DrawRect(&pXYZW, R8G8B8_to_TargetFormat(color), 0);
                        return hand_index;
                    }

                    //рамка чёрного цвета - black frame
                    DrawRect(&pXYZW, R8G8B8_to_TargetFormat(0), 0);

                    // unshift rectangle co ords
                    if (Player_Cards_Shift) {
                        pXYZW.x -= am_Players[player_num].card_shift[hand_index].x;
                        pXYZW.z -= am_Players[player_num].card_shift[hand_index].x;
                        pXYZW.y -= am_Players[player_num].card_shift[hand_index].y;
                        pXYZW.w -= am_Players[player_num].card_shift[hand_index].y;
                    }

                    // shift offsets along a card width
                    pXYZW.x += card_offset;
                    pXYZW.z += card_offset;
                }
            }
        }
    }
    // no card hovered
    return -1;
}

bool DiscardCard(int player_num, signed int card_slot_index) {
    // check for valid card slot
    if (card_slot_index <= -1) return false;

    // can the card be discarded
    if (pCards[am_Players[player_num].cards_at_hand[card_slot_index]].can_be_discarded) {
        // calc animation position and move speed
        int card_count = GetPlayerHandCardCount(current_player_num);
        int card_spacing = (window->GetWidth() - (96 * card_count)) / (card_count + 1);

        anim_card_pos_playdiscard.x = am_Players[player_num].card_shift[card_slot_index].x + (card_slot_index * (card_spacing + 96) + card_spacing);
        anim_card_pos_playdiscard.y = am_Players[player_num].card_shift[card_slot_index].y + 327;

        // find first free table slot
        int table_slot = 0;
        if (!hide_card_anim_start) {
            for (table_slot = 0; table_slot < 10; ++table_slot) {
                if (shown_cards[table_slot].uCardId == -1) break;
            }
        }

        anim_card_spd_playdiscard.x = ((int)shown_cards[table_slot].table_pos.x - (int)anim_card_pos_playdiscard.x) / 10;
        anim_card_spd_playdiscard.y = ((int)shown_cards[table_slot].table_pos.y - (int)anim_card_pos_playdiscard.y) / 10;

        // play sound - set anim card and remove from player
        ArcomageGame::PlaySound(22);
        discarded_card_id = am_Players[player_num].cards_at_hand[card_slot_index];
        am_Players[player_num].cards_at_hand[card_slot_index] = -1;
        need_to_discard_card = 0;
        pArcomageGame->force_redraw_1 = 1;

        return true;
    } else {
        // cannot make this move
        return false;
    }
}

bool PlayCard(int player_num, int card_slot_num) {
    // check for valid card slot
    if (card_slot_num <= -1) return false;

    // can the card be played
    if (CanCardBePlayed(player_num, card_slot_num)) {
        // calc animation position and move speed
        int cards_at_hand = GetPlayerHandCardCount(current_player_num);
        int card_spacing = (window->GetWidth() - (96 * cards_at_hand)) / (cards_at_hand + 1);

        anim_card_pos_playdiscard.x = am_Players[player_num].card_shift[card_slot_num].x + (card_slot_num * (card_spacing + 96) + card_spacing);
        anim_card_pos_playdiscard.y = am_Players[player_num].card_shift[card_slot_num].y + 327;

        anim_card_spd_playdiscard.x = (272 - (int)anim_card_pos_playdiscard.x) / 5;
        anim_card_spd_playdiscard.y = -30;  // (-150 / 5)

        // play sound and take resource cost
        ArcomageCard* pCard = &pCards[am_Players[player_num].cards_at_hand[card_slot_num]];
        ArcomageGame::PlaySound(23);
        am_Players[player_num].resource_bricks -= pCard->needed_bricks;
        am_Players[player_num].resource_beasts -= pCard->needed_beasts;
        am_Players[player_num].resource_gems -= pCard->needed_gems;

        // set anim card and remove from player
        played_card_id = am_Players[player_num].cards_at_hand[card_slot_num];
        am_Players[player_num].cards_at_hand[card_slot_num] = -1;
        pArcomageGame->force_redraw_1 = 1;

        return true;
    } else {
        // cannot make this move
        return false;
    }
}

bool CanCardBePlayed(int player_num, int hand_card_indx) {
    bool result = true;
    ArcomagePlayer* pPlayer = &am_Players[player_num];
    ArcomageCard *test_card = &pCards[pPlayer->cards_at_hand[hand_card_indx]];

    // test card conditions
    if (test_card->needed_quarry_level > pPlayer->quarry_level) result = false;
    if (test_card->needed_magic_level > pPlayer->magic_level) result = false;
    if (test_card->needed_zoo_level > pPlayer->zoo_level) result = false;
    if (test_card->needed_bricks > pPlayer->resource_bricks) result = false;
    if (test_card->needed_gems > pPlayer->resource_gems) result = false;
    if (test_card->needed_beasts > pPlayer->resource_beasts) result = false;

    return result;
}

void ApplyCardToPlayer(int player_num, unsigned int uCardID) {
#define APPLY_TO_PLAYER(PLAYER, ENEMY, FIELD, VAL, RES)   \
    if (VAL != 0) {                                       \
        if (VAL == 99) {                                  \
            if (PLAYER->FIELD < ENEMY->FIELD) {           \
                PLAYER->FIELD = ENEMY->FIELD;             \
                RES = ENEMY->FIELD - PLAYER->FIELD;       \
            }                                             \
        } else {                                          \
            PLAYER->FIELD += (signed int)(VAL);           \
            if (PLAYER->FIELD < 0) PLAYER->FIELD = 0;     \
            RES = (signed int)(VAL);                      \
        }                                                 \
    }

#define APPLY_TO_ENEMY(PLAYER, ENEMY, FIELD, VAL, RES) \
    APPLY_TO_PLAYER(ENEMY, PLAYER, FIELD, VAL, RES)

#define APPLY_TO_BOTH(PLAYER, ENEMY, FIELD, VAL, RES_P, RES_E) \
    if (VAL != 0) {                                            \
        if (VAL == 99) {                                       \
            if (PLAYER->FIELD != ENEMY->FIELD) {               \
                if (PLAYER->FIELD <= ENEMY->FIELD) {           \
                    PLAYER->FIELD = ENEMY->FIELD;              \
                    RES_P = ENEMY->FIELD - PLAYER->FIELD;      \
                } else {                                       \
                    ENEMY->FIELD = PLAYER->FIELD;              \
                    RES_E = PLAYER->FIELD - ENEMY->FIELD;      \
                }                                              \
            }                                                  \
        } else {                                               \
            PLAYER->FIELD += (signed int)(VAL);                \
            ENEMY->FIELD += (signed int)(VAL);                 \
            if (PLAYER->FIELD < 0) {                           \
                PLAYER->FIELD = 0;                             \
            }                                                  \
            if (ENEMY->FIELD < 0) {                            \
                ENEMY->FIELD = 0;                              \
            }                                                  \
            RES_P = (signed int)(VAL);                         \
            RES_E = (signed int)(VAL);                         \
        }                                                      \
    }

    ArcomagePlayer *player = &am_Players[player_num];
    int enemy_num = ((player_num + 1) % 2);
    ArcomagePlayer *enemy = &am_Players[enemy_num];
    ArcomageCard* pCard = &pCards[uCardID];

    int buildings_e = 0;
    int buildings_p = 0;
    int dmg_e = 0;
    int dmg_p = 0;
    int tower_e = 0;
    int tower_p = 0;
    int wall_e = 0;
    int wall_p = 0;

    int beasts_e = 0;
    int beasts_p = 0;
    int gems_e = 0;
    int gems_p = 0;
    int bricks_e = 0;
    int bricks_p = 0;
    int zoo_e = 0;
    int zoo_p = 0;
    int magic_e = 0;
    int magic_p = 0;
    int quarry_e = 0;
    int quarry_p = 0;

    switch (pCard->compare_param) {
        case 2:
            if (player->quarry_level <
                enemy->quarry_level)  //если рудники < рудника врага
                goto LABEL_26;
            goto LABEL_231;
        case 3:
            if (player->magic_level < enemy->magic_level) goto LABEL_26;
            goto LABEL_231;
        case 4:
            if (player->zoo_level <
                enemy->zoo_level)  //если зверинец < зверинца врага
                goto LABEL_26;
            goto LABEL_231;
        case 5:
            if (player->quarry_level == enemy->quarry_level) goto LABEL_26;
            goto LABEL_231;
        case 6:
            if (player->magic_level == enemy->magic_level) goto LABEL_26;
            goto LABEL_231;
        case 7:
            if (player->zoo_level == enemy->zoo_level) goto LABEL_26;
            goto LABEL_231;
        case 8:
            if (player->quarry_level < enemy->quarry_level) goto LABEL_26;
            goto LABEL_231;
        case 9:
            if (player->magic_level < enemy->magic_level) goto LABEL_26;
            goto LABEL_231;
        case 10:
            if (player->zoo_level < enemy->zoo_level) goto LABEL_26;
            goto LABEL_231;
        case 11:
            if (!player->wall_height) goto LABEL_26;
            goto LABEL_231;
        case 12:
            if (player->wall_height) goto LABEL_26;
            goto LABEL_231;
        case 13:
            if (!enemy->wall_height) goto LABEL_26;
            goto LABEL_231;
        case 14:
            if (enemy->wall_height) goto LABEL_26;
            goto LABEL_231;
        case 15:
            if (player->wall_height < enemy->wall_height) goto LABEL_26;
            goto LABEL_231;
        case 16:
            if (player->tower_height < enemy->tower_height) goto LABEL_26;
            goto LABEL_231;
        case 17:
            if (player->wall_height == enemy->wall_height) goto LABEL_26;
            goto LABEL_231;
        case 18:
            if (player->tower_height == enemy->tower_height) goto LABEL_26;
            goto LABEL_231;
        case 19:
            if (player->wall_height < enemy->wall_height) goto LABEL_26;
            goto LABEL_231;
        case 20:
            if (player->tower_height < enemy->tower_height) goto LABEL_26;
            goto LABEL_231;
        default:
        LABEL_26:
            num_actions_left =
                pCard->draw_extra_card_count + (pCard->field_30 == 1);
            num_cards_to_discard = pCard->draw_extra_card_count;
            for (char i = 0; i < pCard->draw_extra_card_count; i++)
                GetNextCardFromDeck(player_num);

            need_to_discard_card =
                GetPlayerHandCardCount(player_num) > minimum_cards_at_hand;

            APPLY_TO_PLAYER(player, enemy, quarry_level,
                            pCard->to_player_quarry_lvl, quarry_p);
            APPLY_TO_PLAYER(player, enemy, magic_level,
                            pCard->to_player_magic_lvl, magic_p);
            APPLY_TO_PLAYER(player, enemy, zoo_level, pCard->to_player_zoo_lvl,
                            zoo_p);
            APPLY_TO_PLAYER(player, enemy, resource_bricks,
                            pCard->to_player_bricks, bricks_p);
            APPLY_TO_PLAYER(player, enemy, resource_gems, pCard->to_player_gems,
                            gems_p);
            APPLY_TO_PLAYER(player, enemy, resource_beasts,
                            pCard->to_player_beasts, beasts_p);
            if (pCard->to_player_buildings) {
                dmg_p = ApplyDamageToBuildings(
                    player_num, (signed int)pCard->to_player_buildings);
                buildings_p = (signed int)pCard->to_player_buildings - dmg_p;
            }
            APPLY_TO_PLAYER(player, enemy, wall_height, pCard->to_player_wall,
                            wall_p);
            APPLY_TO_PLAYER(player, enemy, tower_height, pCard->to_player_tower,
                            tower_p);

            APPLY_TO_ENEMY(player, enemy, quarry_level,
                           pCard->to_enemy_quarry_lvl, quarry_e);
            APPLY_TO_ENEMY(player, enemy, magic_level,
                           pCard->to_enemy_magic_lvl, magic_e);
            APPLY_TO_ENEMY(player, enemy, zoo_level, pCard->to_enemy_zoo_lvl,
                           zoo_e);
            APPLY_TO_ENEMY(player, enemy, resource_bricks,
                           pCard->to_enemy_bricks, bricks_e);
            APPLY_TO_ENEMY(player, enemy, resource_gems, pCard->to_enemy_gems,
                           gems_e);
            APPLY_TO_ENEMY(player, enemy, resource_beasts,
                           pCard->to_enemy_beasts, beasts_e);
            if (pCard->to_enemy_buildings) {
                dmg_e = ApplyDamageToBuildings(
                    enemy_num, (signed int)pCard->to_enemy_buildings);
                buildings_e = (signed int)pCard->to_enemy_buildings - dmg_e;
            }
            APPLY_TO_ENEMY(player, enemy, wall_height, pCard->to_enemy_wall,
                           wall_e);
            APPLY_TO_ENEMY(player, enemy, tower_height, pCard->to_enemy_tower,
                           tower_e);

            APPLY_TO_BOTH(player, enemy, quarry_level,
                          pCard->to_pl_enm_quarry_lvl, quarry_p, quarry_e);
            APPLY_TO_BOTH(player, enemy, magic_level,
                          pCard->to_pl_enm_magic_lvl, magic_p, magic_e);
            APPLY_TO_BOTH(player, enemy, zoo_level, pCard->to_pl_enm_zoo_lvl,
                          zoo_p, zoo_e);
            APPLY_TO_BOTH(player, enemy, resource_bricks,
                          pCard->to_pl_enm_bricks, bricks_p, bricks_e);
            APPLY_TO_BOTH(player, enemy, resource_gems, pCard->to_pl_enm_gems,
                          gems_p, gems_e);
            APPLY_TO_BOTH(player, enemy, resource_beasts,
                          pCard->to_pl_enm_beasts, beasts_p, beasts_e);
            if (pCard->to_pl_enm_buildings) {
                dmg_p = ApplyDamageToBuildings(
                    player_num, (signed int)pCard->to_pl_enm_buildings);
                dmg_e = ApplyDamageToBuildings(
                    enemy_num, (signed int)pCard->to_pl_enm_buildings);
                buildings_p = (signed int)pCard->to_pl_enm_buildings - dmg_p;
                buildings_e = (signed int)pCard->to_pl_enm_buildings - dmg_e;
            }
            APPLY_TO_BOTH(player, enemy, wall_height, pCard->to_pl_enm_wall,
                          wall_p, wall_e);
            APPLY_TO_BOTH(player, enemy, tower_height, pCard->to_pl_enm_tower,
                          tower_p, tower_e);
            break;
        case 0:
        LABEL_231:
            num_actions_left = pCard->can_draw_extra_card2 + (pCard->field_4D == 1);
            num_cards_to_discard = pCard->can_draw_extra_card2;
            for (char i = 0; i < pCard->can_draw_extra_card2; i++)
                GetNextCardFromDeck(player_num);

            need_to_discard_card =
                GetPlayerHandCardCount(player_num) > minimum_cards_at_hand;

            APPLY_TO_PLAYER(player, enemy, quarry_level,
                            pCard->to_player_quarry_lvl2, quarry_p);
            APPLY_TO_PLAYER(player, enemy, magic_level,
                            pCard->to_player_magic_lvl2, magic_p);
            APPLY_TO_PLAYER(player, enemy, zoo_level, pCard->to_player_zoo_lvl2,
                            zoo_p);
            APPLY_TO_PLAYER(player, enemy, resource_bricks,
                            pCard->to_player_bricks2, bricks_p);
            APPLY_TO_PLAYER(player, enemy, resource_gems,
                            pCard->to_player_gems2, gems_p);
            APPLY_TO_PLAYER(player, enemy, resource_beasts,
                            pCard->to_player_beasts2, beasts_p);
            if (pCard->to_player_buildings2) {
                dmg_p = ApplyDamageToBuildings(
                    player_num, (signed int)pCard->to_player_buildings2);
                buildings_p = (signed int)pCard->to_player_buildings2 - dmg_p;
            }
            APPLY_TO_PLAYER(player, enemy, wall_height, pCard->to_player_wall2,
                            wall_p);
            APPLY_TO_PLAYER(player, enemy, tower_height,
                            pCard->to_player_tower2, tower_p);

            APPLY_TO_ENEMY(player, enemy, quarry_level,
                           pCard->to_enemy_quarry_lvl2, quarry_e);
            APPLY_TO_ENEMY(player, enemy, magic_level,
                           pCard->to_enemy_magic_lvl2, magic_e);
            APPLY_TO_ENEMY(player, enemy, zoo_level, pCard->to_enemy_zoo_lvl2,
                           zoo_e);
            APPLY_TO_ENEMY(player, enemy, resource_bricks,
                           pCard->to_enemy_bricks2, bricks_e);
            APPLY_TO_ENEMY(player, enemy, resource_gems, pCard->to_enemy_gems2,
                           gems_e);
            APPLY_TO_ENEMY(player, enemy, resource_beasts,
                           pCard->to_enemy_beasts2, beasts_e);
            if (pCard->to_enemy_buildings2) {
                dmg_e = ApplyDamageToBuildings(
                    enemy_num, (signed int)pCard->to_enemy_buildings2);
                buildings_e = (signed int)pCard->to_enemy_buildings2 - dmg_e;
            }
            APPLY_TO_ENEMY(player, enemy, wall_height, pCard->to_enemy_wall2,
                           wall_e);
            APPLY_TO_ENEMY(player, enemy, tower_height, pCard->to_enemy_tower2,
                           tower_e);

            APPLY_TO_BOTH(player, enemy, quarry_level,
                          pCard->to_pl_enm_quarry_lvl2, quarry_p, quarry_e);
            APPLY_TO_BOTH(player, enemy, magic_level,
                          pCard->to_pl_enm_magic_lvl2, magic_p, magic_e);
            APPLY_TO_BOTH(player, enemy, zoo_level, pCard->to_pl_enm_zoo_lvl2,
                          zoo_p, zoo_e);
            APPLY_TO_BOTH(player, enemy, resource_bricks,
                          pCard->to_pl_enm_bricks2, bricks_p, bricks_e);
            APPLY_TO_BOTH(player, enemy, resource_gems, pCard->to_pl_enm_gems2,
                          gems_p, gems_e);
            APPLY_TO_BOTH(player, enemy, resource_beasts,
                          pCard->to_pl_enm_beasts2, beasts_p, beasts_e);

            if (pCard->to_pl_enm_buildings2) {
                dmg_p = ApplyDamageToBuildings(
                    player_num, (signed int)pCard->to_pl_enm_buildings2);
                dmg_e = ApplyDamageToBuildings(
                    enemy_num, (signed int)pCard->to_pl_enm_buildings2);
                buildings_p = (signed int)pCard->to_pl_enm_buildings2 - dmg_p;
                buildings_e = (signed int)pCard->to_pl_enm_buildings2 - dmg_e;
            }
            APPLY_TO_BOTH(player, enemy, wall_height, pCard->to_pl_enm_wall2,
                          wall_p, wall_e);
            APPLY_TO_BOTH(player, enemy, tower_height, pCard->to_pl_enm_tower2,
                          tower_p, tower_e);
            break;
    }
    //  }


    // call sound if required
    if (quarry_p > 0 || quarry_e > 0) pArcomageGame->PlaySound(30);
    if (quarry_p < 0 || quarry_e < 0) pArcomageGame->PlaySound(31);
    if (magic_p > 0 || magic_e > 0) pArcomageGame->PlaySound(33);
    if (magic_p < 0 || magic_e < 0) pArcomageGame->PlaySound(34);
    if (zoo_p > 0 || zoo_e > 0) pArcomageGame->PlaySound(36);
    if (zoo_p < 0 || zoo_e < 0) pArcomageGame->PlaySound(37);
    if (bricks_p > 0 || bricks_e > 0) pArcomageGame->PlaySound(39);
    if (bricks_p < 0 || bricks_e < 0) pArcomageGame->PlaySound(40);
    if (gems_p > 0 || gems_e > 0) pArcomageGame->PlaySound(42);
    if (gems_p < 0 || gems_e < 0) pArcomageGame->PlaySound(43);
    if (beasts_p > 0 || beasts_e > 0) pArcomageGame->PlaySound(45u);
    if (beasts_p < 0 || beasts_e < 0) pArcomageGame->PlaySound(46);
    if (buildings_p || buildings_e || dmg_p || dmg_e) pArcomageGame->PlaySound(48);
    if (wall_p > 0 || wall_e > 0) pArcomageGame->PlaySound(49);
    if (wall_p < 0 || wall_e < 0) pArcomageGame->PlaySound(50);
    if (tower_p > 0 || tower_e > 0) pArcomageGame->PlaySound(52);
    if (tower_p < 0 || tower_e < 0) pArcomageGame->PlaySound(53);


    // call spark effect if required
    Point explos_coords;
    if (player_num) {
        if (quarry_p) {
            explos_coords.x = 573;
            explos_coords.y = 92;
            new_explosion_effect(&explos_coords, quarry_p);
        }
        if (quarry_e) {
            explos_coords.x = 26;
            explos_coords.y = 92;
            new_explosion_effect(&explos_coords, quarry_e);
        }
        if (magic_p) {
            explos_coords.x = 573;
            explos_coords.y = 164;
            new_explosion_effect(&explos_coords, magic_p);
        }
        if (magic_e) {
            explos_coords.x = 26;
            explos_coords.y = 164;
            new_explosion_effect(&explos_coords, magic_e);
        }
        if (zoo_p) {
            explos_coords.x = 573;
            explos_coords.y = 236;
            new_explosion_effect(&explos_coords, zoo_p);
        }
        if (zoo_e) {
            explos_coords.x = 26;
            explos_coords.y = 236;
            new_explosion_effect(&explos_coords, zoo_e);
        }
        if (bricks_p) {
            explos_coords.x = 563;
            explos_coords.y = 114;
            new_explosion_effect(&explos_coords, bricks_p);
        }
        if (bricks_e) {
            explos_coords.x = 16;
            explos_coords.y = 114;
            new_explosion_effect(&explos_coords, bricks_e);
        }
        if (gems_p) {
            explos_coords.x = 563;
            explos_coords.y = 186;
            new_explosion_effect(&explos_coords, gems_p);
        }
        if (gems_e) {
            explos_coords.x = 16;
            explos_coords.y = 186;
            new_explosion_effect(&explos_coords, gems_e);
        }
        if (beasts_p) {
            explos_coords.x = 563;
            explos_coords.y = 258;
            new_explosion_effect(&explos_coords, beasts_p);
        }
        if (beasts_e) {
            explos_coords.x = 16;
            explos_coords.y = 258;
            new_explosion_effect(&explos_coords, beasts_e);
        }
        if (wall_p) {
            explos_coords.x = 442;
            explos_coords.y = 296;
            new_explosion_effect(&explos_coords, wall_p);
        }
        if (wall_e) {
            explos_coords.x = 180;
            explos_coords.y = 296;
            new_explosion_effect(&explos_coords, wall_e);
        }
        if (tower_p) {
            explos_coords.x = 514;
            explos_coords.y = 296;
            new_explosion_effect(&explos_coords, tower_p);
        }
        if (tower_e) {
            explos_coords.x = 122;
            explos_coords.y = 296;
            new_explosion_effect(&explos_coords, tower_e);
        }
        if (dmg_p) {
            explos_coords.x = 442;
            explos_coords.y = 296;
            new_explosion_effect(&explos_coords, dmg_p);
        }
        if (buildings_p) {
            explos_coords.x = 514;
            explos_coords.y = 296;
            new_explosion_effect(&explos_coords, buildings_p);
        }
        if (dmg_e) {
            explos_coords.x = 180;
            explos_coords.y = 296;
            new_explosion_effect(&explos_coords, dmg_e);
        }
        if (buildings_e) {
            explos_coords.x = 122;
            explos_coords.y = 296;
            new_explosion_effect(&explos_coords, buildings_e);
        }
    } else {
        if (quarry_p) {
            explos_coords.x = 26;
            explos_coords.y = 92;
            new_explosion_effect(&explos_coords, quarry_p);
        }
        if (quarry_e) {
            explos_coords.x = 573;
            explos_coords.y = 92;
            new_explosion_effect(&explos_coords, quarry_e);
        }
        if (magic_p) {
            explos_coords.x = 26;
            explos_coords.y = 164;
            new_explosion_effect(&explos_coords, magic_p);
        }
        if (magic_e) {
            explos_coords.x = 573;
            explos_coords.y = 164;
            new_explosion_effect(&explos_coords, magic_e);
        }
        if (zoo_p) {
            explos_coords.x = 26;
            explos_coords.y = 236;
            new_explosion_effect(&explos_coords, zoo_p);
        }
        if (zoo_e) {
            explos_coords.x = 573;
            explos_coords.y = 236;
            new_explosion_effect(&explos_coords, zoo_e);
        }
        if (bricks_p) {
            explos_coords.x = 16;
            explos_coords.y = 114;
            new_explosion_effect(&explos_coords, bricks_p);
        }
        if (bricks_e) {
            explos_coords.x = 563;
            explos_coords.y = 114;
            new_explosion_effect(&explos_coords, bricks_e);
        }
        if (gems_p) {
            explos_coords.x = 16;
            explos_coords.y = 186;
            new_explosion_effect(&explos_coords, gems_p);
        }
        if (gems_e) {
            explos_coords.x = 563;
            explos_coords.y = 186;
            new_explosion_effect(&explos_coords, gems_e);
        }
        if (beasts_p) {
            explos_coords.x = 16;
            explos_coords.y = 258;
            new_explosion_effect(&explos_coords, beasts_p);
        }
        if (beasts_e) {
            explos_coords.x = 563;
            explos_coords.y = 258;
            new_explosion_effect(&explos_coords, beasts_e);
        }
        if (wall_p) {
            explos_coords.x = 180;
            explos_coords.y = 296;
            new_explosion_effect(&explos_coords, wall_p);
        }
        if (wall_e) {
            explos_coords.x = 442;
            explos_coords.y = 296;
            new_explosion_effect(&explos_coords, wall_e);
        }
        if (tower_p) {
            explos_coords.x = 122;
            explos_coords.y = 296;
            new_explosion_effect(&explos_coords, tower_p);
        }
        if (tower_e) {
            explos_coords.x = 514;
            explos_coords.y = 296;
            new_explosion_effect(&explos_coords, tower_e);
        }
        if (dmg_p) {
            explos_coords.x = 180;
            explos_coords.y = 296;
            new_explosion_effect(&explos_coords, dmg_p);
        }
        if (buildings_p) {
            explos_coords.x = 122;
            explos_coords.y = 296;
            new_explosion_effect(&explos_coords, buildings_p);
        }
        if (dmg_e) {
            explos_coords.x = 442;
            explos_coords.y = 296;
            new_explosion_effect(&explos_coords, dmg_e);
        }
        if (buildings_e) {
            explos_coords.x = 514;
            explos_coords.y = 296;
            new_explosion_effect(&explos_coords, buildings_e);
        }
    }

#undef APPLY_TO_BOTH
#undef APPLY_TO_ENEMY
#undef APPLY_TO_PLAYER
}



int ApplyDamageToBuildings(int player_num, int damage) {
    int wall = am_Players[player_num].wall_height;
    int result = 0;

    if (wall >= -damage) {  // wall absorbs all damage
        result = damage;
        am_Players[player_num].wall_height += damage;
    } else {
        damage += wall;  // reduce damage by size of wall
        am_Players[player_num].wall_height = 0;
        result = -wall;
        am_Players[player_num].tower_height += damage;  // apply remaining to tower
    }

    if (am_Players[player_num].tower_height < 0)
        am_Players[player_num].tower_height = 0;

    return result;
}

void GameResultsApply() {
    int winner;               // esi@1
    int victory_type;         // edi@1
    int pl_resource;          // edx@25
    int en_resource;          // eax@28
    unsigned int tavern_num;  // eax@54

    winner = -1;
    victory_type = -1;
    // nullsub_1();
    /*strcpy(pText, "The Winner is: ");//"Победил: " Ritor1: архаизм
    xy.y = 160;
    xy.x = 320; //- 12 * v2 / 2;
    am_DrawText(-1, pText, &xy);*/

    //проверка построена ли башня
    if (am_Players[0].tower_height < max_tower_height &&
        am_Players[1].tower_height >=
            max_tower_height) {  //наша башня не построена, а у врага построена
        winner = 2;  //победил игрок 2(враг)
        victory_type = 0;
    } else if (am_Players[0].tower_height >= max_tower_height &&
               am_Players[1].tower_height <
                   max_tower_height) {  //наша башня построена, а у врага нет
        winner = 1;  //победил игрок 1(мы)
        victory_type = 0;
    } else if (am_Players[0].tower_height >= max_tower_height &&
               am_Players[1].tower_height >=
                   max_tower_height) {  //и у нас, и у врага построена
        if (am_Players[0].tower_height ==
            am_Players[1].tower_height) {  //наши башни равны
            winner = 0;        //никто не победил
            victory_type = 4;  //ничья
        } else {               //наши башни не равны
            winner =
                (am_Players[0].tower_height <= am_Players[1].tower_height) +
                1;  //победил тот, у кого выше
            victory_type = 0;
        }
    }

    //проверка разрушена ли башня
    if (am_Players[0].tower_height <= 0 &&
        am_Players[1].tower_height > 0) {  //наша башня разрушена, а у врага нет
        winner = 2;        // победил игрок 2(враг)
        victory_type = 2;  //победил разрушив башню врага
    } else if (am_Players[0].tower_height > 0 &&
               am_Players[1].tower_height <=
                   0) {  //у врага башня разрушена, а у нас нет
        winner = 1;        //победил игрок 1(мы)
        victory_type = 2;  //победил разрушив башню врага
    } else if (am_Players[0].tower_height <= 0 &&
               am_Players[1].tower_height <=
                   0) {  //наша башня разрушена, и у врага разрушена
        if (am_Players[0].tower_height ==
            am_Players[1].tower_height) {  //если башни равны
            if (am_Players[0].wall_height ==
                am_Players[1].wall_height) {  //если стены равны
                winner = 0;
                victory_type = 4;
            } else {  //если стены не равны
                winner =
                    (am_Players[0].wall_height <= am_Players[1].wall_height) +
                    1;  //победил тот, у кого стена выше
                victory_type = 1;  //победа когда больше стена при ничье
            }
        } else {  //башни не равны
            winner =
                (am_Players[0].tower_height <= am_Players[1].tower_height) +
                1;  // побеждает тот у кого башня больше
            victory_type = 2;  //победил разрушив башню врага
        }
    }

    //проверка набраны ли ресурсы
    //проверка какого ресурса больше всего у игрока 1(нас)
    pl_resource =
        am_Players[0].resource_bricks;  //кирпичей больше чем др. ресурсов
    if (am_Players[0].resource_gems > am_Players[0].resource_bricks &&
        am_Players[0].resource_gems >
            am_Players[0].resource_beasts)  //драг.камней больше всего
        pl_resource = am_Players[0].resource_gems;
    else if (am_Players[0].resource_beasts > am_Players[0].resource_gems &&
             am_Players[0].resource_beasts >
                 am_Players[0].resource_bricks)  //зверей больше всего
        pl_resource = am_Players[0].resource_beasts;

    //проверка какого ресурса больше у игрока 2(врага)
    en_resource =
        am_Players[1].resource_bricks;  //кирпичей больше чем др. ресурсов
    if (am_Players[1].resource_gems > am_Players[1].resource_bricks &&
        am_Players[1].resource_gems >
            am_Players[1].resource_beasts)  //драг.камней больше всего
        en_resource = am_Players[1].resource_gems;
    else if (am_Players[1].resource_beasts > am_Players[1].resource_gems &&
             am_Players[1].resource_beasts >
                 am_Players[1].resource_bricks)  //зверей больше всего
        en_resource = am_Players[1].resource_beasts;

    //сравнение ресурсов игроков
    if (winner == -1 && victory_type == -1) {  //нет победителя по башням
        if (pl_resource < max_resources_amount &&
            en_resource >=
                max_resources_amount) {  //враг набрал нужное количество
            winner = 2;  // враг победил
            victory_type = 3;  //победа собрав нужное количество ресурсов
        } else if (pl_resource >= max_resources_amount &&
                   en_resource <
                       max_resources_amount) {  //мы набрали нужное количество
            winner = 1;  // мы победили
            victory_type = 3;  //победа собрав нужное количество ресурсов
        } else if (pl_resource >= max_resources_amount &&
                   en_resource >=
                       max_resources_amount) {  //и у нас и у врага нужное
                                                //количество ресурсов
            if (pl_resource == en_resource) {  // ресурсы равны
                winner = 0;        //ресурсы равны
                victory_type = 4;  //ничья
            } else {
                winner = (pl_resource <= en_resource) +
                         1;  //ресурсы не равны, побеждает тот у кого больше
                victory_type = 3;  //победа собрав нужное количество ресурсов
            }
        }
    } else if (winner == 0 && victory_type == 4) {  // при ничье по башням и стене
        if (pl_resource != en_resource) {  //ресурсы не равны
            winner =
                (pl_resource <= en_resource) + 1;  //победил тот у кого больше
            victory_type =
                5;  //победа когда при ничье большее количество ресурсов
        } else {    //ресурсы равны
            winner = 0;        //нет победителя
            victory_type = 4;  //ничья
        }
    }

    //подведение итогов
    pArcomageGame->Victory_type = victory_type;
    pArcomageGame->uGameWinner = winner;
    if (winner == 1) {  //победитель игрок 1(мы)
        if ((window_SpeakInHouse->par1C >= 108) &&
            (window_SpeakInHouse->par1C <= 120)) {  //таверны
            if (!pParty->pArcomageWins[window_SpeakInHouse->par1C - 108]) {
                pParty->pArcomageWins[window_SpeakInHouse->par1C - 108] = 1;
                pParty->PartyFindsGold(
                    p2DEvents[window_SpeakInHouse->par1C - 1].fPriceMultiplier *
                        100.0,
                    0);  //вознаграждение
            }
        }
        //проверка выполнен ли квест по аркомагу
        tavern_num = 0;
        for (uint i = 108; i <= 120; ++i) {
            if (!pParty->pArcomageWins[i - 108]) break;
            tavern_num++;
        }
        if (tavern_num == 13)
            _449B7E_toggle_bit(pParty->_quest_bits, 238,
                               1);  // 238 - Won all Arcomage games

        for (int i = 0; i < 4; ++i) {  //внесение записи в Заслуги
            if (!_449B57_test_bit(pParty->pPlayers[i]._achieved_awards_bits, PLAYER_GUILD_BITS__FINED))
                _449B7E_toggle_bit(pParty->pPlayers[i]._achieved_awards_bits,
                                   PLAYER_GUILD_BITS__ARCOMAGE_WIN, 1);
        }
        ++pParty->uNumArcomageWins;
        if (pParty->uNumArcomageWins > 1000000)  //ограничение количества побед
            pParty->uNumArcomageWins = 1000000;
    } else {  //проигрыш
        for (int i = 0; i < 4; ++i) {  //внесение записи в Заслуги
            if (!_449B57_test_bit(pParty->pPlayers[i]._achieved_awards_bits, PLAYER_GUILD_BITS__FINED))
                _449B7E_toggle_bit(pParty->pPlayers[i]._achieved_awards_bits,
                                   PLAYER_GUILD_BITS__ARCOMAGE_LOSE, 1);
        }
        ++pParty->uNumArcomageLoses;
        if (pParty->uNumArcomageLoses >
            1000000)  //ограничение количества проигрышей
            pParty->uNumArcomageLoses = 1000000;
    }
}

void ArcomageGame::PrepareArcomage() {
    // stop all audio and set player names
    pAudioPlayer->StopChannels(-1, -1);
    strcpy(pArcomageGame->pPlayer1Name, Player1Name);
    strcpy(pArcomageGame->pPlayer2Name, Player2Name);

    // load in background pic and render
    pArcomageGame->pGameBackground = assets->GetImage_PCXFromIconsLOD("layout.pcx");
    render->DrawTextureNew(0, 0, pArcomageGame->pGameBackground);
    render->Present();

    // load in layout pic containing all AM sprites and set fonts
    ArcomageGame::LoadSprites();
    pArcomageGame->pfntComic = pFontComic;
    pArcomageGame->pfntArrus = pFontArrus;

    // calc and clear shown table card slots
    hide_card_anim_runnning = 0;
    hide_card_anim_start = 0;
    for (int i = 0; i < 10; ++i) {
        int xslot = (i + 1) % 4;
        int yslot = (i + 1) / 4;
        shown_cards[i].uCardId = -1;
        shown_cards[i].discarded = 0;
        shown_cards[i].table_pos.x = 100 * xslot + 120;
        shown_cards[i].table_pos.y = 138 * yslot + 18;
        shown_cards[i].hide_anim_spd.x = -100 * xslot / 5;
        shown_cards[i].hide_anim_spd.y = -138 * yslot / 5;
        shown_cards[i].hide_anim_pos.x = shown_cards[i].table_pos.x;
        shown_cards[i].hide_anim_pos.y = shown_cards[i].table_pos.y;
    }

    // create new explosion effect struct array
    for (int i = 0; i < 10; ++i)
        am_effects_array[i].explosion_eff = explosion_effect_struct::New();

    // load in start condtions and create initial deck and deal
    SetStartGameData();
    InitalHandsFill();

    // set card params
    current_card_slot_index = -1;
    drawn_card_slot_index = -1;
    need_to_discard_card = 0;

    // set exiting params
    pArcomageGame->force_am_exit = 0;
    pArcomageGame->check_exit = 0;
    pArcomageGame->GameOver = 0;
}

void SetStartConditions() {
    const ArcomageStartConditions *st_cond;
    st_cond = &start_conditions[window_SpeakInHouse->par1C - 108];

    // set start conditions
    start_tower_height = st_cond->tower_height;
    start_wall_height = st_cond->wall_height;
    start_quarry_level = st_cond->quarry_level - 1;
    start_magic_level = st_cond->magic_level - 1;
    start_zoo_level = st_cond->zoo_level - 1;
    start_bricks_amount = st_cond->bricks_amount;
    start_gems_amount = st_cond->gems_amount;
    start_beasts_amount = st_cond->beasts_amount;
    // win conditions
    max_tower_height = st_cond->max_tower;
    max_resources_amount = st_cond->max_resources;
    // opponent skill level
    opponent_mastery = st_cond->mastery_lvl;

    // bonus acts as min level
    minimum_cards_at_hand = 5;
    quarry_bonus = 1;
    magic_bonus = 1;
    zoo_bonus = 1;
}

void am_DrawText(const String &str, Point *pXY) {
    pPrimaryWindow->DrawText(pFontComic, pXY->x,
                             pXY->y - ((pFontComic->GetHeight() - 3) / 2) + 3,
                             0, str, 0, 0, 0);
}

void DrawRect(Rect *pXYZW, unsigned __int16 uColor, char bSolidFill) {
    if (bSolidFill) {
        for (int i = pXYZW->y; i <= pXYZW->w; ++i)
            render->RasterLine2D(pXYZW->x, i, pXYZW->z, i, uColor);
    } else {
        render->RasterLine2D(pXYZW->x, pXYZW->y, pXYZW->z, pXYZW->y, uColor);
        render->RasterLine2D(pXYZW->z, pXYZW->y, pXYZW->z, pXYZW->w, uColor);
        render->RasterLine2D(pXYZW->z, pXYZW->w, pXYZW->x, pXYZW->w, uColor);
        render->RasterLine2D(pXYZW->x, pXYZW->w, pXYZW->x, pXYZW->y, uColor);
    }
}

int rand_interval(int min, int max) { return min + rand() % (max - min + 1); }

void am_IntToString(int val, char *pOut) { sprintf(pOut, "%d", val); }

void set_stru1_field_8_InArcomage(int inValue) {  // what is this meant to be doing??
    switch (inValue) {
        case 91:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 123;
            break;
        case 92:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 124;
            break;
        case 93:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 125;
            break;
        case 96:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 126;
            break;
        case 61:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 43;
            break;
        case 55:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 38;
            break;
        case 56:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 42;
            break;
        case 57:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 40;
            break;
        case 59:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 58;
            break;
        case 54:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 94;
            break;
        case 50:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 64;
            break;
        case 51:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 35;
            break;
        case 52:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 36;
            break;
        case 53:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 37;
            break;
        case 49:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 33;
            break;
        case 39:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 34;
            break;
        case 44:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 60;
            break;
        case 46:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 62;
            break;
        case 47:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 63;
            break;
        case 48:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = 41;
            break;
        default:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.am_input_key) = inValue;
            break;
    }
}
