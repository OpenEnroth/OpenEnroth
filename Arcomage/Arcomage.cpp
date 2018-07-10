#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Strings.h"
#include "Engine/Time.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"

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
int am_40D2B4(Point *a1, int a2);
int ApplyDamageToBuildings(int player_num, int damage);
void GameResultsApply();

void am_DrawText(const String &str, Point *pXY);
void am_BeginScene(unsigned __int16 *pPcxPixels, int a2, int a3);  // idb
void am_EndScene();
void DrawRect(Rect *pXYZW, unsigned __int16 uColor, char bSolidFill);
void DrawSquare(Point *pTargetXY, unsigned __int16 uColor);
void DrawPixel(Point *pTargetXY, unsigned __int16 uColor);
int rand_interval(int min, int max);  // idb
void am_IntToString(int val, char *pOut);

//----- (0040DEDB) --------------------------------------------------------
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

#define SIG_trpg 0x67707274
#define SIG_xxxx 0x78787878

ArcomageGame *pArcomageGame = new ArcomageGame;

ArcomagePlayer am_Players[2];         // 00505708
AcromageCardOnTable shown_cards[10];  // 004FAA78
stru272 array_4FABD0[10];             // 4FABD0

ArcomageDeck playDeck;    // 00505288
ArcomageDeck deckMaster;  // 005054C8

char Player2Name[] = "Enemy";
char Player1Name[] = "Player";

struct stru273 {
    bool _40DD2F();
    bool Inside(Rect *pXYZW);

    int x;
    int y;
    char curr_mouse_left;
    char mouse_left_state_changed;
    char curr_mouse_right;
    char mouse_right_state_changed;
};

struct am_2 {
    int slot_index;
    int card_power;
};

am_2 cards_power[10];
std::array<int16_t, 12> am_sounds;

char byte_4E185C = 1;
char am_byte_4E185D = 1;
char use_start_bonus = 1;

int start_tower_height;
int start_wall_height;
int start_quarry_level;
int start_magic_level;
int start_zoo_level;

int minimum_cards_at_hand = 5;  // 004E1874
int quarry_bonus = 1;
int magic_bonus = 1;
int zoo_bonus = 1;

int max_tower_height = 50;
int max_resources_amount = 100;

int opponent_mastery = 1;

bool am_gameover;  // 004FAA2C
char byte_4FAA2D;

int am_default_starting_player = 0;  // 505890
int current_player_num;              // 004FAA6C
char need_to_discard_card;           // 04FAA77

int current_card_slot_index;  // 004FABBC
int played_card_id;           // 4FABC0
int uCardID;                  // 4FAA50

int deck_walk_index;  // 004FABC8

int start_bricks_amount;
int start_gems_amount;
int start_beasts_amount;

Point amuint_4FAA3C_blt_xy;
Point am_uint_4FAA44_blt_xy;
Point amuint_4FAA54_blt_xy;
Point amuint_4FAA5C_blt_xy;

int dword_4FAA64;
int dword_4FAA68;

int dword_4FABB8;

char byte_4FAA00;

int amuint_4FAA34;
int amuint_4FAA38;
int amuint_4FAA4C;

char byte_4FAA2E;

int dword_4FAA70;
char byte_4FAA74;
char am_byte_4FAA75;
char am_byte_4FAA76;

int amuint_4FABC4;

char byte_505880;
char byte_505881;

bool stru273::_40DD2F() {
    this->x = pArcomageGame->mouse_x;
    this->y = pArcomageGame->mouse_y;
    this->curr_mouse_left = pArcomageGame->mouse_left;
    this->mouse_left_state_changed =
        pArcomageGame->mouse_left == pArcomageGame->prev_mouse_left;
    this->curr_mouse_right = pArcomageGame->mouse_right;
    this->mouse_right_state_changed =
        pArcomageGame->mouse_right == pArcomageGame->prev_mouse_right;
    pArcomageGame->prev_mouse_left = pArcomageGame->mouse_left;
    pArcomageGame->prev_mouse_right = pArcomageGame->mouse_right;
    return true;
}

bool stru273::Inside(Rect *pXYZW) {
    return (x >= pXYZW->x) && (x <= pXYZW->z) && (y >= pXYZW->y) &&
           (y <= pXYZW->w);
}

stru272_stru0 *stru272_stru0::New() {
    stru272_stru0 *v2 = (stru272_stru0 *)malloc(0x5Cu);
    v2->signature = SIG_trpg;
    v2->position_in_sparks_arr = 0;
    v2->field_30 = 0.0;
    v2->field_58 = 0;
    v2->field_59 = 0;

    return v2;
}

int stru272_stru0::Free() {
    if (this->signature == SIG_trpg) {
        this->signature = SIG_xxxx;
        free(this);
        return 0;
    }

    return 2;
}

int stru272_stru0::StartFill(stru272_stru2 *a2) {
    // stru272_stru0* a1 = this;
    if (this->signature == SIG_trpg) {
        this->field_4 = a2->field_20;
        this->field_C = a2->effect_area.x << 16;
        this->field_10 = a2->effect_area.y << 16;
        this->field_14 = a2->effect_area.z << 16;
        this->field_18 = a2->effect_area.w << 16;
        this->field_1C = a2->field_10;
        this->field_20 = a2->field_14;
        this->field_24 = a2->field_18;
        this->field_28 = (float)(a2->field_1Cf * 65536.0);
        this->field_2C = a2->field_24f;
        this->field_34 = (int)(a2->field_28f * 65536.0);
        this->field_38 = (int)(a2->field_2Cf * 65536.0);
        this->field_3C = a2->field_30;
        this->field_40 = a2->field_34;
        this->field_54 = a2->sparks_array;
        this->field_59 = 1;
        return 0;
    }

    return 2;
}

int stru272_stru0::Clear(char a2, char a3) {
    if (signature == SIG_trpg) {
        if (a2) {
            position_in_sparks_arr = 0;
            field_30 = 0.0;
        }
        if (field_59 && a3) {
            for (int i = 0; i < field_4; ++i) field_54[i].have_spark = 0;
            field_58 = 0;
        }
        return 0;
    }

    return 2;
}

int stru272_stru0::DrawEffect() {
    //    stru272_stru0 *v1; // edi@1
    int v3;             // ST18_4@3
    double v4;          // st7@3
    double v5;          // st6@4
    char v6;            // bl@8
    stru272_stru1 *v7;  // esi@8
    int v8;             // ecx@10
    signed int v9;      // eax@10
    int v10;            // ecx@10
    signed int v11;     // eax@10
    int v12;            // ebx@12
    int v13;            // ST1C_4@12
    int v14;            // ebx@12
    int v15;            // ST1C_4@12
    signed int v16;     // edx@12
    int v17;            // ebx@12
    int v18;            // ST1C_4@12
    signed int v19;     // edx@12
    int v20;            // [sp+8h] [bp-10h]@8
    int v21;            // [sp+Ch] [bp-Ch]@8
    float v22;          // [sp+14h] [bp-4h]@3

    if (this->signature != SIG_trpg) return 2;
    v3 = this->position_in_sparks_arr;
    v22 = this->field_30;
    v4 = v3;
    if (v3 > 0) {
        v5 = v22 + this->field_2C;
        v22 = v5;
        if (v5 > v4) v22 = v4;
    }
    if (v22 >= 1.0 || this->field_58) {
        v6 = 0;
        v7 = this->field_54;
        v20 = this->field_28;

        for (v21 = this->field_4; v21; v21--) {
            if (v7->have_spark > 0) {
                v8 = v7->field_14;
                --v7->have_spark;
                v9 = v8 + v7->field_C;
                v10 = v20 + v7->field_18;
                v7->field_C = v9;
                v7->spark_position.x = v9 >> 16;
                v11 = v10 + v7->field_10;
                v7->field_18 = v10;
                v7->field_10 = v11;
                v7->spark_position.y = v11 >> 16;
                v6 = 1;
                // goto LABEL_14;
            } else {
                if (v22 >= 1.0) {
                    v12 = this->field_40;
                    v13 = this->field_3C;
                    v7->have_spark = rand_interval(v13, v12);
                    v7->field_14 = (rand() % 17 - 8) << 16;
                    v7->field_18 = (rand() % 17 - 8) << 16;
                    v14 = this->field_14 - 1;
                    v15 = this->field_C;
                    v16 = rand_interval(v15, v14);
                    v7->field_C = v16;
                    v7->spark_position.x = v16 >> 16;
                    v17 = this->field_18 - 1;
                    v18 = this->field_10;
                    v19 = rand_interval(v17, v18);
                    v7->field_10 = v19;
                    v7->spark_position.y = v19 >> 16;
                    --this->position_in_sparks_arr;
                    v22 = v22 - 1.0;
                    v6 = 1;
                }
            }
            // LABEL_14:
            ++v7;
            // --v21;
            // if ( !v21 )
            // {
            // this->field_58 = v6;
            // this->field_30 = v22;
            // return 0;
            // }
        }
        this->field_58 = v6;
        this->field_30 = v22;
    }
    return 0;
}

int stru272_stru0::_40E2A7() {
    if (signature == SIG_trpg) {
        if (position_in_sparks_arr <= 0)
            return field_58 != 0 ? 2 : 0;
        else
            return 1;
    }

    return 3;
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

void ArcomageGame::DoBlt_Copy(unsigned __int16 *pPixels) {
    render->Present();

    Point pTargetPoint = {0, 0};

    Rect pSrcRect;
    pSrcRect.x = 0;
    pSrcRect.y = 0;
    pSrcRect.z = window->GetWidth();
    pSrcRect.w = window->GetHeight();

    render->BeginScene();
    pArcomageGame->pBlit_Copy_pixels = pPixels;
    render->am_Blt_Copy(&pSrcRect, &pTargetPoint, 2);
    render->EndScene();
    pArcomageGame->pBlit_Copy_pixels = nullptr;
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

bool ArcomageGame::MsgLoop(int a1, ArcomageGame_stru1 *a2) {
    void *v2 = a2;
    pArcomageGame->field_0 = 0;
    pArcomageGame->stru1.field_0 = 0;

    window->PeekSingleMessage();

    memcpy(v2, &pArcomageGame->stru1, 0xCu);
    return pArcomageGame->stru1.field_0 != 0;
}

void am_BeginScene(unsigned __int16 *pPcxPixels, int a2, int a3) {
    render->BeginScene();
    pArcomageGame->pBlit_Copy_pixels = pPcxPixels;
}

void am_EndScene() {
    render->EndScene();
    pArcomageGame->pBlit_Copy_pixels = nullptr;
}

bool ArcomageGame::LoadSprites() {
    pArcomageGame->pSprites = assets->GetImage_PCXFromIconsLOD("sprites.pcx");
    pArcomageGame->pSpritesPixels =
        (unsigned __int16 *)pArcomageGame->pSprites->GetPixels(
            IMAGE_FORMAT_R5G6B5);
    return true;
}

bool ArcomageGame::LoadBackground() {
    pArcomageGame->pGameBackground =
        assets->GetImage_PCXFromIconsLOD("layout.pcx");
    pArcomageGame->pBackgroundPixels =
        (unsigned __int16 *)pArcomageGame->pGameBackground->GetPixels(
            IMAGE_FORMAT_R5G6B5);
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
    int element_power;

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

    if (pCard->field_24 == 1) {
        element_power = player->resource_bricks - pCard->needed_bricks;
    } else if (pCard->field_24 == 2) {
        element_power = player->resource_gems - pCard->needed_gems;
    } else if (pCard->field_24 == 3) {
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
    int all_player_cards_count;  // eax@9
    int random_card_slot;        // edi@9
    ArcomageCard *v12;           // ecx@20
    int v56;                     // ecx@141
    int v57;                     // edx@141
    int v132;                    // [sp-14h] [bp-14h]@0
    ArcomagePlayer *enemy;       // [sp-10h] [bp-10h]@5
    ArcomagePlayer *player;      // [sp-Ch] [bp-Ch]@5

    byte_4FAA00 = 1;
    if (opponent_mastery == 0) {
        if (need_to_discard_card == 0) {  // am_byte_4FAA77
            for (int i = 0; i < 10; ++i) {
                all_player_cards_count = GetPlayerHandCardCount(player_num);
                random_card_slot = rand_interval(0, all_player_cards_count - 1);
                if (CanCardBePlayed(player_num, random_card_slot))
                    return PlayCard(player_num, random_card_slot);
            }
        }
        all_player_cards_count = GetPlayerHandCardCount(player_num);
        random_card_slot = rand_interval(0, all_player_cards_count - 1);
        return DiscardCard(player_num, random_card_slot);
    } else if ((opponent_mastery == 1) || (opponent_mastery == 2)) {
        player = &am_Players[player_num];
        enemy = &am_Players[(player_num + 1) % 2];
        all_player_cards_count = GetPlayerHandCardCount(player_num);
        for (int i = 0; i < 10; ++i) {
            if (i >= all_player_cards_count) {
                cards_power[i].slot_index = -1;
                cards_power[i].card_power = -9999;
            } else {
                cards_power[i].slot_index = i;
                cards_power[i].card_power = 0;
            }
        }
        for (int i = 0; i < all_player_cards_count; ++i) {
            v12 = &pCards[am_Players[player_num]
                              .cards_at_hand[cards_power[i].slot_index]];
            cards_power[i].card_power =
                CalculateCardPower(player, enemy, v12, opponent_mastery - 1);
        }

        for (int j = all_player_cards_count - 1; j >= 0; --j) {
            for (int m = 0; m < j; ++m) {
                if (cards_power[m].card_power < cards_power[m + 1].card_power) {
                    v56 = cards_power[m].slot_index;
                    v57 = cards_power[m].card_power;
                    cards_power[m].slot_index = cards_power[m + 1].slot_index;
                    cards_power[m].card_power = cards_power[m + 1].card_power;
                    cards_power[m + 1].slot_index = cards_power[m].slot_index;
                    cards_power[m + 1].card_power = cards_power[m].card_power;
                }
            }
        }
        if (need_to_discard_card) {
            for (int i = all_player_cards_count - 1; i; --i) {
                if (pCards[am_Players[player_num]
                               .cards_at_hand[cards_power[i].slot_index]]
                        .can_be_discarded) {
                    v132 = cards_power[i].slot_index;
                }
            }
        } else {
            for (int i = all_player_cards_count - 1; i; --i) {
                if (pCards[am_Players[player_num]
                               .cards_at_hand[cards_power[i].slot_index]]
                        .can_be_discarded) {
                    v132 = cards_power[i].slot_index;
                }
            }
            for (int i = 0; i < all_player_cards_count - 1; ++i) {
                if (CanCardBePlayed(player_num, cards_power[i].slot_index) &&
                    cards_power[i].card_power) {
                    return PlayCard(player_num, cards_power[i].slot_index);
                }
            }
        }
        return DiscardCard(player_num, v132);
    }
    return true;  // result != 0;
}

void ArcomageGame::Loop() {
    while (!pArcomageGame->GameOver) {
        pArcomageGame->field_F6 = 1;
        byte_4FAA24 = 1;
        IncreaseResourcesInTurn(current_player_num);
        // LABEL_8:
        while (byte_4FAA24) {
            played_card_id = -1;
            GetNextCardFromDeck(current_player_num);
            while (1) {
                byte_4FAA24 = PlayerTurn(current_player_num);
                if (GetPlayerHandCardCount(current_player_num) <=
                    minimum_cards_at_hand) {
                    need_to_discard_card = 0;
                    break;
                }
                need_to_discard_card = 1;
                if (pArcomageGame->field_F4) break;
            }
        }
        pArcomageGame->GameOver = IsGameOver();
        if (!pArcomageGame->GameOver) TurnChange();
        if (pArcomageGame->field_F4) pArcomageGame->GameOver = 1;
    }
    GameResultsApply();
    if (am_gameover)
        dword_4FAA70 = 0;
    else
        dword_4FAA70 = -1;

    for (int i = 0; i < 10; ++i) {
        array_4FABD0[i].field_40->Clear(1, 1);
        array_4FABD0[i].field_40->Free();
    }

    delete pArcomageGame->pGameBackground;
    pArcomageGame->pGameBackground = nullptr;

    delete pArcomageGame->pSprites;
    pArcomageGame->pSprites = nullptr;

    pArcomageGame->bGameInProgress = false;
    viewparams->bRedrawGameUI = true;
    if (pMovie_Track) BackToHouseMenu();
    //  for (int i = 0; i < 12; ++i)
    //    pSoundList->UnloadSound(am_sounds[i], 1);
}

void SetStartGameData() {
    signed int j;                       // edx@7
    int card_id_counter;                // edx@13
    signed int i;                       // ecx@13
    signed int card_dispenser_counter;  // eax@13

    current_player_num = am_default_starting_player;
    SetStartConditions();
    for (i = 0; i < 2; ++i) {
        if (i) {
            strcpy(am_Players[1].pPlayerName, pArcomageGame->pPlayer2Name);
            if (byte_4E185C)
                am_Players[1].IsHisTurn = 0;
            else
                am_Players[1].IsHisTurn = 1;
        } else {
            strcpy(am_Players[0].pPlayerName, pArcomageGame->pPlayer1Name);
            am_Players[0].IsHisTurn = 1;
        }
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
            if (am_byte_4E185D) {
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
    amuint_4FABC4 = -1;
    pArcomageGame->field_F6 = 1;
}

void InitalHandsFill() {
    for (int i = 0; i < minimum_cards_at_hand; ++i) {
        GetNextCardFromDeck(0);
        GetNextCardFromDeck(1);
    }
    pArcomageGame->field_F6 = 1;
}

void GetNextCardFromDeck(int player_num) {
    signed int deck_index;      // eax@1
    int new_card_id;            // edi@1
    signed int card_slot_indx;  // eax@7

    deck_index = deck_walk_index;
    for (;;) {
        if (deck_index >= DECK_SIZE) {
            FillPlayerDeck();
            deck_index = deck_walk_index;
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
        amuint_4FAA4C = card_slot_indx;
        am_Players[player_num].cards_at_hand[card_slot_indx] = new_card_id;
        am_Players[player_num].card_shift[card_slot_indx].x =
            rand_interval(-4, 4);
        am_Players[player_num].card_shift[card_slot_indx].y =
            rand_interval(-4, 4);
        pArcomageGame->field_F6 = 1;
        byte_4FAA2D = 1;
    }
}

int GetEmptyCardSlotIndex(int player_num) {
    for (int i = 0; i < 10; ++i) {
        if (am_Players[player_num].cards_at_hand[i] == -1) return i;
    }
    return -1;
}

void IncreaseResourcesInTurn(int player_num) {
    am_Players[player_num].resource_bricks +=
        quarry_bonus + am_Players[player_num].quarry_level;
    am_Players[player_num].resource_gems +=
        magic_bonus + am_Players[player_num].magic_level;
    am_Players[player_num].resource_beasts +=
        zoo_bonus + am_Players[player_num].zoo_level;
}

void TurnChange() {
    char player_name[64];    // [sp+4h] [bp-64h]@4
    ArcomageGame_stru1 v10;  // [sp+54h] [bp-14h]@7
    Point v11;               // [sp+60h] [bp-8h]@4

    if (!pArcomageGame->field_F4) {
        if (am_Players[0].IsHisTurn != 1 || am_Players[1].IsHisTurn != 1) {
            ++current_player_num;
            am_byte_4FAA75 = 1;
            if (current_player_num >= 2) current_player_num = 0;
        } else {
            // nullsub_1();
            //   v11.x = 0;
            //   v11.y = 0;
            strcpy(player_name, "The Next Player is: ");  //"След"
                                                            // v0 = 0;
            v11.y = 200;
            v11.x = 320;  // - 12 * v0 / 2;
            am_DrawText(player_name, &v11);
            am_byte_4FAA75 = 1;
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
                if (v10.field_0 == 1) {
                    if (v10.field_4) break;
                    // nullsub_1();
                    continue;
                }
                if ((v10.field_0 > 4) && (v10.field_0 <= 8)) break;
                if (v10.field_0 == 10) {
                    pArcomageGame->field_F4 = 1;
                    byte_4FAA74 = 1;
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
    bool result = false;
    for (int i = 0; i < 2; ++i) {
        if (am_Players[i].tower_height <= 0) result = true;
        if (am_Players[i].tower_height >= max_tower_height) result = true;
        if (am_Players[i].resource_bricks >= max_resources_amount ||
            am_Players[i].resource_gems >= max_resources_amount ||
            am_Players[i].resource_beasts >= max_resources_amount)
            result = true;
    }

    return am_gameover = result;
}

char PlayerTurn(int player_num) {
    unsigned __int64 v3;  // kr00_8@3

    Rect pSrcXYZW;          // [sp+70h] [bp-40h]@75
    Point pTargetXY;        // [sp+90h] [bp-20h]@75
    ArcomageGame_stru1 a2;  // [sp+98h] [bp-18h]@8
    int animation_stage;    // [sp+A4h] [bp-Ch]@1
    bool break_loop;        // [sp+AFh] [bp-1h]@1

    uCardID = -1;
    break_loop = false;
    animation_stage = 20;
    byte_4FAA00 = 0;
    dword_4FAA68 = 0;
    amuint_4FAA38 = 10;
    amuint_4FAA34 = 5;
    if (amuint_4FAA4C != -1) byte_4FAA2D = 1;
    do {
        do {
            v3 = pEventTimer->Time() - pArcomageGame->event_timer_time;
        } while (v3 < 6);
        pArcomageGame->event_timer_time = (unsigned int)pEventTimer->Time();
        if (pArcomageGame->field_F4) break_loop = true;
        ArcomageGame::MsgLoop(0, &a2);
        switch (a2.field_0) {
            case 2:
                if (a2.field_4 == 129 && a2.field_8 == 1) {
                    pAudioPlayer->StopChannels(-1, -1);
                    dword_4FAA68 = 0;
                    break_loop = true;
                    pArcomageGame->field_F4 = 1;
                }
                break;
            case 9:
                pArcomageGame->field_F6 = 1;
                break;
            case 10:
                pAudioPlayer->StopChannels(-1, -1);
                byte_4FAA74 = 1;
                break_loop = true;
                pArcomageGame->field_F4 = 1;
                break;
        }

        if (am_Players[current_player_num].IsHisTurn != 1 && !byte_4FAA00 &&
            !byte_4FAA2E && !byte_4FAA2D) {
            if (am_byte_4FAA75) am_byte_4FAA76 = 1;
            OpponentsAITurn(current_player_num);
            byte_4FAA2E = 1;
        }
        if (amuint_4FAA4C != -1 && amuint_4FAA38 > 10) amuint_4FAA38 = 10;
        if (byte_4FAA2E || byte_4FAA2D ||
            am_Players[current_player_num].IsHisTurn != 1) {
            pArcomageGame->field_F6 = 1;
            if (byte_4FAA2D) {
                --amuint_4FAA38;
                if (amuint_4FAA38 < 0) {
                    byte_4FAA2D = 0;
                    amuint_4FAA38 = 10;
                    break_loop = false;
                }
            }
            if (byte_4FAA2E) {
                --animation_stage;
                if (animation_stage < 0) {
                    if (dword_4FAA68 > 1) {
                        --dword_4FAA68;
                        byte_4FAA00 = 0;
                    } else {
                        break_loop = true;
                    }
                    byte_4FAA2E = 0;
                    animation_stage = 20;
                }
            }
        } else {
            if (need_to_discard_card) {
                if (a2.field_0 == 7 &&
                    DiscardCard(player_num, current_card_slot_index)) {
                    if (am_byte_4FAA75) am_byte_4FAA76 = 1;
                    if (dword_4FAA64 > 0) {
                        --dword_4FAA64;
                        need_to_discard_card =
                            GetPlayerHandCardCount(player_num) >
                            minimum_cards_at_hand;
                    }
                    byte_4FAA2E = 1;
                }
                if (a2.field_0 == 8 &&
                    DiscardCard(player_num, current_card_slot_index)) {
                    if (am_byte_4FAA75) am_byte_4FAA76 = 1;
                    if (dword_4FAA64 > 0) {
                        --dword_4FAA64;
                        need_to_discard_card =
                            GetPlayerHandCardCount(player_num) >
                            minimum_cards_at_hand;
                    }
                    byte_4FAA2E = 1;
                }
            } else {
                if (a2.field_0 == 7) {
                    if (PlayCard(player_num, current_card_slot_index)) {
                        byte_4FAA2E = 1;
                        if (am_byte_4FAA75) am_byte_4FAA76 = 1;
                    }
                }
                if (a2.field_0 == 8) {
                    if (DiscardCard(player_num, current_card_slot_index)) {
                        byte_4FAA2E = 1;
                        if (am_byte_4FAA75) am_byte_4FAA76 = 1;
                    }
                }
            }
        }

        if (dword_4FABB8 != DrawCardsRectangles(player_num)) {
            dword_4FABB8 = DrawCardsRectangles(player_num);
            pArcomageGame->field_F6 = 1;
        }
        if (pArcomageGame->field_F6) {
            DrawGameUI(animation_stage);
            ArcomageGame::DoBlt_Copy(pArcomageGame->pBackgroundPixels);
            pArcomageGame->field_F6 = 0;
        }
        if (pArcomageGame->field_F9) {
            pTargetXY.x = 0;
            pTargetXY.y = 0;
            pSrcXYZW.x = 0;
            pSrcXYZW.z = window->GetWidth();
            pSrcXYZW.y = 0;
            pSrcXYZW.w = window->GetHeight();
            am_BeginScene(pArcomageGame->pBackgroundPixels, -1, 1);
            render->am_Blt_Copy(&pSrcXYZW, &pTargetXY, 2);
            am_EndScene();
            DrawGameUI(animation_stage);
            render->Present();
            pArcomageGame->field_F9 = 0;
        }
    } while (!break_loop);
    return dword_4FAA68 > 0;
}

//----- (0040A9AF) --------------------------------------------------------
void DrawGameUI(int animation_stage) {
    am_BeginScene(pArcomageGame->pSpritesPixels, -1, 1);
    DrawRectanglesForText();
    am_EndScene();

    am_BeginScene(pArcomageGame->pSpritesPixels, -1, 1);
    DrawCards();          //рисуем карты
    DrawPlayersTowers();  //рисуем башню
    DrawPlayersWall();    //рисуем стену
    DrawPlayersText();    //рисуем текст
    am_EndScene();

    am_BeginScene(pArcomageGame->pSpritesPixels, -1, 1);
    DrawCardAnimation(animation_stage);

    for (int i = 0; i < 10; ++i) {
        if (array_4FABD0[i].have_effect) array_4FABD0[i].field_40->DrawEffect();
    }
    current_card_slot_index = DrawCardsRectangles(current_player_num);
    DrawSparks();
    am_EndScene();
}

//----- (0040AA4E) --------------------------------------------------------
void DrawSparks() {
    int rgb_pixel_color;  // [sp-4h] [bp-2Ch]@4
    unsigned int pixel_color;

    for (int i = 0; i < 10; ++i) {
        if (array_4FABD0[i].have_effect &&
            (array_4FABD0[i].field_40->_40E2A7() == 2)) {
            rgb_pixel_color = 0x0000FF00;
            if (!array_4FABD0[i].effect_sign) rgb_pixel_color = 0x000000FF;
            pixel_color = R8G8B8_to_TargetFormat(rgb_pixel_color);
            for (int j = 0; j < 150; ++j) {
                if (array_4FABD0[i].effect_sparks[j].have_spark > 0) {
                    if (j % 2)
                        DrawPixel(
                            &array_4FABD0[i].effect_sparks[j].spark_position,
                            pixel_color);
                    else
                        DrawSquare(
                            &array_4FABD0[i].effect_sparks[j].spark_position,
                            pixel_color);
                }
            }
        }
    }
}

//----- (0040AB0A) --------------------------------------------------------
void DrawRectanglesForText() {
    Rect pSrcRect;    // [sp+Ch] [bp-18h]@1
    Point pTargetXY;  // [sp+1Ch] [bp-8h]@1

    // resources rectangles
    pSrcRect.x = 765;
    pSrcRect.y = 0;
    pSrcRect.z = 843;
    pSrcRect.w = 216;

    pTargetXY.x = 8;
    pTargetXY.y = 56;
    render->am_Blt_Copy(&pSrcRect, &pTargetXY, 2);

    pTargetXY.x = 555;
    pTargetXY.y = 56;
    render->am_Blt_Copy(&pSrcRect, &pTargetXY, 2);

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

//----- (0040AC5F) --------------------------------------------------------
void DrawPlayersText() {
    int res_value;        // ecx@18
    char text_buff[32];   // [sp+Ch] [bp-28h]@2
    Point text_position;  // [sp+2Ch] [bp-8h]@2

    if (need_to_discard_card) {
        strcpy(text_buff, localization->GetString(266));  // DISCARD A CARD
        text_position.x =
            320 - pArcomageGame->pfntArrus->GetLineWidth(text_buff) / 2;
        text_position.y = 306;
        am_DrawText(text_buff, &text_position);
    }
    strcpy(text_buff, am_Players[0].pPlayerName);
    if (!current_player_num) strcat(text_buff, "***");
    text_position.x =
        47 - pArcomageGame->pfntComic->GetLineWidth(text_buff) / 2;
    text_position.y = 21;
    am_DrawText(text_buff, &text_position);

    strcpy(text_buff, am_Players[1].pPlayerName);
    if (current_player_num == 1) strcat(text_buff, "***");
    text_position.x =
        595 - pArcomageGame->pfntComic->GetLineWidth(text_buff) / 2;
    text_position.y = 21;
    am_DrawText(text_buff, &text_position);

    am_IntToString(am_Players[0].tower_height, text_buff);
    text_position.x =
        123 - pArcomageGame->pfntComic->GetLineWidth(text_buff) / 2;
    text_position.y = 305;
    am_DrawText(text_buff, &text_position);

    am_IntToString(am_Players[1].tower_height, text_buff);
    text_position.x =
        515 - pArcomageGame->pfntComic->GetLineWidth(text_buff) / 2;
    text_position.y = 305;
    am_DrawText(text_buff, &text_position);

    am_IntToString(am_Players[0].wall_height, text_buff);
    text_position.x =
        188 - pArcomageGame->pfntComic->GetLineWidth(text_buff) / 2;
    text_position.y = 305;
    am_DrawText(text_buff, &text_position);

    am_IntToString(am_Players[1].wall_height, text_buff);
    text_position.x =
        451 - pArcomageGame->pfntComic->GetLineWidth(text_buff) / 2;
    text_position.y = 305;
    am_DrawText(text_buff, &text_position);

    res_value = am_Players[0].quarry_level;
    if (use_start_bonus) res_value = am_Players[0].quarry_level + quarry_bonus;
    text_position.x = 14;  // - 6 * 0 / 2;
    text_position.y = 92;
    DrawPlayerLevels(StringFromInt(res_value), &text_position);

    res_value = am_Players[1].quarry_level;
    if (use_start_bonus) res_value = am_Players[1].quarry_level + quarry_bonus;
    text_position.y = 92;
    text_position.x = 561;  //- 6 * v2 / 2;
    DrawPlayerLevels(StringFromInt(res_value), &text_position);

    res_value = am_Players[0].magic_level;
    if (use_start_bonus) res_value = am_Players[0].magic_level + magic_bonus;
    text_position.y = 164;
    text_position.x = 14;  //- 6 * v4 / 2;
    DrawPlayerLevels(StringFromInt(res_value), &text_position);

    res_value = am_Players[1].magic_level;
    if (use_start_bonus) res_value = am_Players[1].magic_level + magic_bonus;
    text_position.y = 164;
    text_position.x = 561;  //- 6 * v6 / 2;
    DrawPlayerLevels(StringFromInt(res_value), &text_position);

    res_value = am_Players[0].zoo_level;
    if (use_start_bonus) res_value = am_Players[0].zoo_level + zoo_bonus;
    text_position.y = 236;
    text_position.x = 14;  // - 6 * v8 / 2;
    DrawPlayerLevels(StringFromInt(res_value), &text_position);

    res_value = am_Players[1].zoo_level;
    if (use_start_bonus) res_value = am_Players[1].zoo_level + zoo_bonus;
    text_position.y = 236;
    text_position.x = 561;  // - 6 * v10 / 2;
    DrawPlayerLevels(StringFromInt(res_value), &text_position);

    text_position.y = 114;
    text_position.x = 10;
    DrawBricksCount(StringFromInt(am_Players[0].resource_bricks),
                    &text_position);

    text_position.x = 557;
    text_position.y = 114;
    DrawBricksCount(StringFromInt(am_Players[1].resource_bricks),
                    &text_position);

    text_position.x = 10;
    text_position.y = 186;
    DrawGemsCount(StringFromInt(am_Players[0].resource_gems), &text_position);

    text_position.x = 557;
    text_position.y = 186;
    DrawGemsCount(StringFromInt(am_Players[1].resource_gems), &text_position);

    text_position.x = 10;
    text_position.y = 258;
    DrawBeastsCount(StringFromInt(am_Players[0].resource_beasts),
                    &text_position);

    text_position.x = 557;
    text_position.y = 258;
    DrawBeastsCount(StringFromInt(am_Players[1].resource_beasts),
                    &text_position);
}

//----- (0040B102) --------------------------------------------------------
void DrawPlayerLevels(const String &str, Point *pXY) {
    int v7;  // eax@3
    Rect pSrcRect;
    Point pTargetPoint;

    am_BeginScene(pArcomageGame->pSpritesPixels, -1, 1);
    {
        pTargetPoint.x = pXY->x;
        pTargetPoint.y = pXY->y;

        for (auto i = str.begin(); i != str.end(); ++i) {
            v7 = 22 * *i;
            pSrcRect.z = v7 - 842;
            pSrcRect.x = v7 - 864;
            pSrcRect.y = 190;
            pSrcRect.w = 207;
            render->am_Blt_Chroma(&pSrcRect, &pTargetPoint,
                                  pArcomageGame->field_54, 1);
            pTargetPoint.x += 22;
        }
    }
    am_EndScene();
}

//----- (0040B17E) --------------------------------------------------------
void DrawBricksCount(const String &str, Point *pXY) {
    int v7;  // eax@3
    Rect pSrcRect;
    Point pTargetPoint;

    am_BeginScene(pArcomageGame->pSpritesPixels, -1, 1);
    {
        pTargetPoint.x = pXY->x;
        pTargetPoint.y = pXY->y;
        for (auto i = str.begin(); i != str.end(); ++i) {
            if (*i) {
                v7 = 13 * *i;
                pSrcRect.x = v7 - 370;
                pSrcRect.z = v7 - 357;
                pSrcRect.y = 128;
                pSrcRect.w = 138;
                render->am_Blt_Copy(&pSrcRect, &pTargetPoint, 2);
                pTargetPoint.x += 13;
            }
        }
    }
    am_EndScene();
}

//----- (0040B1F3) --------------------------------------------------------
void DrawGemsCount(const String &str, Point *pXY) {
    int v7;  // eax@3
    Rect pSrcRect;
    Point pTargetPoint;

    am_BeginScene(pArcomageGame->pSpritesPixels, -1, 1);
    {
        pTargetPoint.x = pXY->x;
        pTargetPoint.y = pXY->y;
        for (auto i = str.begin(); i != str.end(); ++i) {
            if (*i) {
                v7 = 13 * *i;
                pSrcRect.x = v7 - 370;
                pSrcRect.z = v7 - 357;
                pSrcRect.y = 138;
                pSrcRect.w = 148;
                render->am_Blt_Copy(&pSrcRect, &pTargetPoint, 2);
                pTargetPoint.x += 13;
            }
        }
    }
    am_EndScene();
}

//----- (0040B268) --------------------------------------------------------
void DrawBeastsCount(const String &str, Point *pXY) {
    int v7;  // eax@3
    Rect pSrcRect;
    Point pTargetPoint;

    am_BeginScene(pArcomageGame->pSpritesPixels, -1, 1);
    {
        pTargetPoint.x = pXY->x;
        pTargetPoint.y = pXY->y;
        for (auto i = str.begin(); i != str.end(); ++i) {
            if (*i) {
                v7 = 13 * *i;
                pSrcRect.x = v7 - 370;
                pSrcRect.z = v7 - 357;
                pSrcRect.y = 148;
                pSrcRect.w = 158;
                render->am_Blt_Copy(&pSrcRect, &pTargetPoint, 2);
                pTargetPoint.x += 13;
            }
        }
    }
    am_EndScene();
}

//----- (0040B2DD) --------------------------------------------------------
void DrawPlayersTowers() {
    int tower_height;  // eax@1
    int tower_top;     // esi@3
    Rect pSrcXYZW;     // [sp+0h] [bp-18h]@3
    Point pTargetXY;   // [sp+10h] [bp-8h]@3

    tower_height = am_Players[0].tower_height;
    if (am_Players[0].tower_height > max_tower_height)
        tower_height = max_tower_height;
    pSrcXYZW.y = 0;
    pSrcXYZW.x = 892;
    pSrcXYZW.z = 937;
    tower_top = 200 * tower_height / max_tower_height;
    pSrcXYZW.w = tower_top;
    pTargetXY.x = 102;
    pTargetXY.y = 297 - tower_top;
    render->am_Blt_Copy(&pSrcXYZW, &pTargetXY, 2);  //стена башни

    pSrcXYZW.y = 0;
    pSrcXYZW.x = 384;
    pSrcXYZW.z = 452;
    pSrcXYZW.w = 94;
    pTargetXY.y = 203 - tower_top;
    pTargetXY.x = 91;
    render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY, pArcomageGame->field_54,
                          2);  //верхушка башни

    tower_height = am_Players[1].tower_height;
    if (am_Players[1].tower_height > max_tower_height)
        tower_height = max_tower_height;
    tower_top = 200 * tower_height / max_tower_height;
    pSrcXYZW.y = 0;
    pSrcXYZW.x = 892;
    pSrcXYZW.z = 937;
    pSrcXYZW.w = tower_top;

    pTargetXY.x = 494;
    pTargetXY.y = 297 - tower_top;
    render->am_Blt_Copy(&pSrcXYZW, &pTargetXY, 2);
    // draw tower up cone
    pSrcXYZW.x = 384;
    pSrcXYZW.z = 452;
    pSrcXYZW.y = 94;
    pSrcXYZW.w = 188;

    pTargetXY.x = 483;
    pTargetXY.y = 203 - tower_top;
    render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY, pArcomageGame->field_54, 2);
}

//----- (0040B400) --------------------------------------------------------
void DrawPlayersWall() {
    int v0;           // eax@1
    int v1;           // eax@4
    int v3;           // eax@5
    int v4;           // eax@8
    Rect pSrcXYZW;    // [sp+4h] [bp-18h]@4
    Point pTargetXY;  // [sp+14h] [bp-8h]@4

    v0 = am_Players[0].wall_height;

    if (am_Players[0].wall_height > 100) v0 = 100;

    if (am_Players[0].wall_height > 0) {
        pSrcXYZW.y = 0;
        pSrcXYZW.x = 843;
        v1 = 200 * v0 / 100;
        pSrcXYZW.z = 867;
        pSrcXYZW.w = v1;
        pTargetXY.x = 177;
        pTargetXY.y = 297 - v1;
        render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY, pArcomageGame->field_54,
                              2);
    }

    v3 = am_Players[1].wall_height;
    if (am_Players[1].wall_height > 100) v3 = 100;
    if (am_Players[1].wall_height > 0) {
        pSrcXYZW.y = 0;
        pSrcXYZW.x = 843;
        v4 = 200 * v3 / 100;
        pSrcXYZW.z = 867;
        pSrcXYZW.w = v4;
        pTargetXY.x = 439;
        pTargetXY.y = 297 - v4;
        render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY, pArcomageGame->field_54,
                              2);
    }
}

//----- (0040B4B9) --------------------------------------------------------
void DrawCards() {
    int v0;           // esi@1
    int v2;           // edi@1
    unsigned int v7;  // ecx@4
    signed int v11;   // edi@18
    Rect pSrcXYZW;    // [sp+Ch] [bp-1Ch]@8
    Point pTargetXY;  // [sp+1Ch] [bp-Ch]@1
    int v24;          // [sp+24h] [bp-4h]@1

    v0 = GetPlayerHandCardCount(current_player_num);
    pTargetXY.y = 327;
    v24 = (window->GetWidth() - 96 * v0) / (v0 + 1);
    pTargetXY.x = (window->GetWidth() - 96 * v0) / (v0 + 1);
    for (v2 = 0; v2 < v0; ++v2) {
        // v3 = current_player_num;
        if (am_byte_4E185D) {
            pTargetXY.x += am_Players[current_player_num].card_shift[v2].x;
            pTargetXY.y += am_Players[current_player_num].card_shift[v2].y;
        }
        v7 = am_Players[current_player_num].cards_at_hand[v2];
        if (am_Players[current_player_num].cards_at_hand[v2] == -1) {
            ++v0;
        } else if (v2 != amuint_4FAA4C) {
            if (am_Players[current_player_num].IsHisTurn == 0 &&
                byte_505881 == 0) {
                pSrcXYZW.x = 192;
                pSrcXYZW.z = 288;
                pSrcXYZW.y = 0;
                pSrcXYZW.w = 128;
                render->am_Blt_Copy(
                    &pSrcXYZW, &pTargetXY,
                    2);  //рисуется оборотные стороны карт противника
                pTargetXY.x += v24 + 96;
            } else {
                pArcomageGame->GetCardRect(v7, &pSrcXYZW);
                if (!CanCardBePlayed(current_player_num, v2)) {
                    render->am_Blt_Copy(&pSrcXYZW, &pTargetXY,
                                        0);  //рисуются неактивные карты
                    pTargetXY.x += v24 + 96;
                } else {
                    render->am_Blt_Copy(&pSrcXYZW, &pTargetXY,
                                        2);  //рисуются активные карты
                    pTargetXY.x += v24 + 96;
                }
            }
        } else {
            pTargetXY.x += v24 + 96;
        }
        // LABEL_15:
        if (am_byte_4E185D) {
            pTargetXY.x -= am_Players[current_player_num].card_shift[v2].x;
            pTargetXY.y -= am_Players[current_player_num].card_shift[v2].y;
        }
    }

    for (v11 = 0; v11 < 10; ++v11) {
        if (am_byte_4FAA76 == 0) {
            if (shown_cards[v11].uCardId != -1) {
                pArcomageGame->GetCardRect(shown_cards[v11].uCardId, &pSrcXYZW);
                render->am_Blt_Copy(&pSrcXYZW, &shown_cards[v11].field_18_point,
                                    0);
            }
            if (shown_cards[v11].field_4 != 0) {
                pTargetXY.x = shown_cards[v11].field_18_point.x + 12;
                pTargetXY.y = shown_cards[v11].field_18_point.y + 40;
                pSrcXYZW.x = 843;
                pSrcXYZW.z = 916;
                pSrcXYZW.y = 200;
                pSrcXYZW.w = 216;
                render->am_Blt_Chroma(&pSrcXYZW, &pTargetXY,
                                      pArcomageGame->field_54, 2);
            }
        } else if (amuint_4FAA34 <= 0) {
            if (v11 == 9) {
                am_byte_4FAA76 = 0;
                am_byte_4FAA75 = 0;
                amuint_4FAA34 = 5;
            }
            if (shown_cards[v11].uCardId != -1)
                amuint_4FABC4 = shown_cards[v11].uCardId;
            shown_cards[v11].uCardId = -1;
            shown_cards[v11].field_18_point.x = shown_cards[v11].field_8.x;
            shown_cards[v11].field_18_point.y = shown_cards[v11].field_8.y;
            shown_cards[v11].field_4 = 0;
        } else {
            if (shown_cards[v11].uCardId != -1) {
                shown_cards[v11].field_18_point.x +=
                    shown_cards[v11].field_10_xplus;
                shown_cards[v11].field_18_point.y +=
                    shown_cards[v11].field_14_y_plus;
                pArcomageGame->GetCardRect(shown_cards[v11].uCardId, &pSrcXYZW);
                render->am_Blt_Copy(&pSrcXYZW, &shown_cards[v11].field_18_point,
                                    0);
            }
        }
    }
    if (am_byte_4FAA76 != 0) --amuint_4FAA34;
    pSrcXYZW.x = 192;
    pSrcXYZW.z = 288;
    pSrcXYZW.y = 0;
    pSrcXYZW.w = 128;
    pTargetXY.x = 120;
    pTargetXY.y = 18;
    render->am_Blt_Copy(&pSrcXYZW, &pTargetXY, 0);
}

//----- (0040B76F) --------------------------------------------------------
void DrawCardAnimation(int animation_stage) {
    int v1;           // eax@3
    int v2;           // eax@3
    int v4;           // eax@4
    int v8;           // eax@15
    double v15;       // st7@22
    int v17;          // eax@32
    char v18;         // zf@37
    int v19;          // eax@41
    Rect pSrcXYZW;    // [sp+Ch] [bp-1Ch]@6
    Point pTargetXY;  // [sp+1Ch] [bp-Ch]@20
    int v26;          // [sp+24h] [bp-4h]@1

    v26 = animation_stage;
    if (amuint_4FAA4C != -1) {
        if (amuint_4FAA38 >= 9) {
            am_uint_4FAA44_blt_xy.y = 18;
            am_uint_4FAA44_blt_xy.x = 120;
            v1 = GetPlayerHandCardCount(current_player_num);
            v2 = (window->GetWidth() - 96 * v1) / v1 + 96;
            if (am_byte_4E185D) {
                // v3 = 188 * current_player_num + 8 * amuint_4FAA4C;
                // amuint_4FAA3C_blt_xy.x = (amuint_4FAA4C * v2 +
                // *(am_Players[0].arr_6C[0] + v3) - 120) / 10;
                amuint_4FAA3C_blt_xy.x = (amuint_4FAA4C * v2 +
                                          am_Players[current_player_num]
                                              .card_shift[amuint_4FAA4C]
                                              .x -
                                          120) /
                                         10;
                v4 = (am_Players[current_player_num]
                          .card_shift[amuint_4FAA4C]
                          .y +
                      309) /
                     10;  // (*(&am_Players[0].arr_6C[0][1] + v3) + 309) / 10;
            } else {
                amuint_4FAA3C_blt_xy.x = (amuint_4FAA4C * v2 - 120) / 10;
                v4 = 30;
            }
            am_uint_4FAA44_blt_xy.y += v4;
            am_uint_4FAA44_blt_xy.x += amuint_4FAA3C_blt_xy.x;
            amuint_4FAA3C_blt_xy.y = v4;
            pSrcXYZW.x = 192;
            pSrcXYZW.y = 0;
            pSrcXYZW.z = 288;
            pSrcXYZW.w = 128;
            render->am_Blt_Copy(&pSrcXYZW, &am_uint_4FAA44_blt_xy, 2);
        } else {
            pSrcXYZW.x = 192;
            pSrcXYZW.y = 0;
            pSrcXYZW.z = 288;
            pSrcXYZW.w = 128;
            am_uint_4FAA44_blt_xy.x += amuint_4FAA3C_blt_xy.x;
            am_uint_4FAA44_blt_xy.y += amuint_4FAA3C_blt_xy.y;
            render->am_Blt_Copy(&pSrcXYZW, &am_uint_4FAA44_blt_xy, 2);
            if (!amuint_4FAA38) amuint_4FAA4C = -1;
        }
    }
    if (uCardID != -1) {
        if (v26 <= 10) {
            if (v26 == 10) {
                pArcomageGame->GetCardRect(uCardID, &pSrcXYZW);
                //  v8 = 0;
                for (v8 = 0; v8 < 10; ++v8) {
                    if (shown_cards[v8].uCardId == -1) {
                        shown_cards[v8].uCardId = uCardID;
                        shown_cards[v8].field_4 = 1;
                        break;
                    }
                }
                /* v9 = shown_cards;
                while ( v9->uCardId != -1 )
                {
                ++v9;
                ++v8;
                if ( v9 >= &dword_4FABB8 )
                goto LABEL_20;
                }
                v10 = v8;
                shown_cards[v10].uCardId = uCardID;
                shown_cards[v10].field_4 = 1;*/
                // LABEL_20:
                pTargetXY.x = shown_cards[v8].field_8.x;
                pTargetXY.y = shown_cards[v8].field_8.y;
                render->am_Blt_Copy(&pSrcXYZW, &pTargetXY, 0);
                uCardID = -1;
            }
        } else {
            pArcomageGame->GetCardRect(uCardID, &pSrcXYZW);
            amuint_4FAA5C_blt_xy.x += amuint_4FAA54_blt_xy.x;
            amuint_4FAA5C_blt_xy.y += amuint_4FAA54_blt_xy.y;
            render->am_Blt_Copy(&pSrcXYZW, &amuint_4FAA5C_blt_xy, 0);
        }
    }
    if (played_card_id != -1) {
        v15 = v26;
        if (v15 > 15.0) {
            pArcomageGame->GetCardRect(played_card_id, &pSrcXYZW);
            amuint_4FAA5C_blt_xy.x += amuint_4FAA54_blt_xy.x;
            amuint_4FAA5C_blt_xy.y += amuint_4FAA54_blt_xy.y;
            render->am_Blt_Copy(&pSrcXYZW, &amuint_4FAA5C_blt_xy, 2);
            return;
        }
        if (v15 == 15.0) {
            ApplyCardToPlayer(current_player_num, played_card_id);
            pArcomageGame->GetCardRect(played_card_id, &pSrcXYZW);
            pTargetXY.x = 272;
            pTargetXY.y = 173;
            render->am_Blt_Copy(&pSrcXYZW, &pTargetXY, 2);
            return;
        }
        if (v15 == 5.0) {
            amuint_4FAA5C_blt_xy.x = 272;
            amuint_4FAA5C_blt_xy.y = 173;
            for (v17 = 0; v17 < 10; ++v17) {
                if (shown_cards[v17].uCardId == -1) break;
            }
            amuint_4FAA54_blt_xy.x = (shown_cards[v17].field_8.x - 272) / 5;
            amuint_4FAA54_blt_xy.y = (shown_cards[v17].field_8.y - 173) / 5;
            pArcomageGame->GetCardRect(played_card_id, &pSrcXYZW);
            pTargetXY.x = 272;
            pTargetXY.y = 173;
            render->am_Blt_Copy(&pSrcXYZW, &pTargetXY, 2);
            return;
        }
        if (v15 >= 5.0) {
            v18 = v26 == 0;
        } else {
            v18 = v26 == 0;
            if (v26 > 0) {
                pArcomageGame->GetCardRect(played_card_id, &pSrcXYZW);
                amuint_4FAA5C_blt_xy.x += amuint_4FAA54_blt_xy.x;
                amuint_4FAA5C_blt_xy.y += amuint_4FAA54_blt_xy.y;
                render->am_Blt_Copy(&pSrcXYZW, &amuint_4FAA5C_blt_xy, 0);
                return;
            }
        }
        if (!v18) {
            pArcomageGame->GetCardRect(played_card_id, &pSrcXYZW);
            pTargetXY.x = 272;
            pTargetXY.y = 173;
            render->am_Blt_Copy(&pSrcXYZW, &pTargetXY, 2);
            return;
        }
        pArcomageGame->GetCardRect(played_card_id, &pSrcXYZW);
        for (v19 = 0; v19 < 10; ++v19) {
            if (shown_cards[v19].uCardId == -1) {
                shown_cards[v19].uCardId = played_card_id;
                break;
            }
        }
        pTargetXY.x = shown_cards[v19].field_8.x;
        pTargetXY.y = shown_cards[v19].field_8.y;
        render->am_Blt_Copy(&pSrcXYZW, &pTargetXY, 0);
        played_card_id = -1;
    }
}

//----- (0040BB12) --------------------------------------------------------
void ArcomageGame::GetCardRect(unsigned int uCardID, Rect *pCardRect) {
    int v3;  // edx@1
    int v4;  // ecx@1

    v3 = pCards[uCardID].slot % 10;
    v4 = (pCards[uCardID].slot / 10 << 7) + 220;
    pCardRect->y = v4;
    pCardRect->x = 96 * v3;
    pCardRect->w = v4 + 128;
    pCardRect->z = 96 * v3 + 96;
}

//----- (0040BB49) --------------------------------------------------------
int GetPlayerHandCardCount(int player_num) {
    int card_count;  // eax@1

    card_count = 0;
    for (int i = 0; i < 10; ++i) {
        if (am_Players[player_num].cards_at_hand[i] != -1) ++card_count;
    }
    return card_count;
}

//----- (0040BB67) --------------------------------------------------------
signed int DrawCardsRectangles(int player_num) {
    int v5;          // eax@3
    int color;       // ST00_4@19
    Rect pXYZW;      // [sp+Ch] [bp-3Ch]@3
    stru273 v26;     // [sp+1Ch] [bp-2Ch]@2
    __int32 var18;   // [sp+30h] [bp-18h]@3
    int hand_index;  // [sp+3Ch] [bp-Ch]@3

    // __debugbreak(); // need do fix rectangle not fit to card

    if (am_Players[player_num].IsHisTurn) {
        if (v26._40DD2F()) {
            v5 = GetPlayerHandCardCount(player_num);
            pXYZW.y = 327;
            pXYZW.w = 455;
            pXYZW.x = (window->GetWidth() - 96 * v5) / (v5 + 1);
            var18 = pXYZW.x + 96;
            pXYZW.z = pXYZW.x + 96;
            for (hand_index = 0; hand_index < v5; hand_index++) {
                // for ( i = 0; i < 10; ++i )
                //{
                if (am_Players[player_num].cards_at_hand[hand_index] != -1) {
                    // break;
                    //}
                    if (am_byte_4E185D) {
                        pXYZW.x +=
                            am_Players[player_num].card_shift[hand_index].x;
                        pXYZW.z +=
                            am_Players[player_num].card_shift[hand_index].x;
                        pXYZW.y +=
                            am_Players[player_num].card_shift[hand_index].y;
                        pXYZW.w +=
                            am_Players[player_num].card_shift[hand_index].y;
                    }
                    if (v26.Inside(&pXYZW)) {
                        if (CanCardBePlayed(player_num, hand_index))
                            color = 0x00FFFFFF;  //белый цвет
                        else
                            color = 0x000000FF;  //красный цвет
                        DrawRect(&pXYZW, R8G8B8_to_TargetFormat(color), 0);
                        return hand_index;
                    }
                    DrawRect(&pXYZW, R8G8B8_to_TargetFormat(0),
                             0);  //рамка чёрного цвета
                    if (am_byte_4E185D) {
                        pXYZW.x -=
                            am_Players[player_num].card_shift[hand_index].x;
                        pXYZW.z -=
                            am_Players[player_num].card_shift[hand_index].x;
                        pXYZW.y -=
                            am_Players[player_num].card_shift[hand_index].y;
                        pXYZW.w -=
                            am_Players[player_num].card_shift[hand_index].y;
                    }
                    pXYZW.x += var18;
                    pXYZW.z += var18;
                }
            }
        }
    }
    return -1;
}

bool DiscardCard(int player_num, signed int card_slot_index) {
    int v8;   // eax@8
    int v10;  // ecx@8
    int v12;  // eax@8
    int i;

    if (card_slot_index <= -1) return false;
    int v2 = 0;

    for (i = 0; i < 10; ++i) {
        if (am_Players[player_num].cards_at_hand[i] != -1) {
            if (card_slot_index == v2) break;
            ++v2;
        }
    }

    if (pCards[am_Players[player_num].cards_at_hand[i]].can_be_discarded) {
        ArcomageGame::PlaySound(22);
        v8 = GetPlayerHandCardCount(current_player_num);
        v10 = am_Players[player_num].card_shift[i].x +
              (window->GetWidth() - 96 * v8) / (v8 + 1);
        amuint_4FAA5C_blt_xy.x = v10;
        amuint_4FAA5C_blt_xy.y =
            am_Players[player_num].card_shift[i].y + 327;  // v11;
        v12 = 0;

        if (!am_byte_4FAA75) {
            for (v12 = 0; v12 < 10; ++v12) {
                if (shown_cards[v12].uCardId == -1) break;
            }
        }
        pArcomageGame->field_F6 = 1;
        amuint_4FAA54_blt_xy.x = (shown_cards[v12].field_8.x - v10) / 10;
        amuint_4FAA54_blt_xy.y = (shown_cards[v12].field_8.y - 327) / 10;
        uCardID = am_Players[player_num].cards_at_hand[i];
        am_Players[player_num].cards_at_hand[i] = -1;
        need_to_discard_card = 0;
        return true;
    } else {
        return false;
    }
}

bool PlayCard(int player_num, int card_slot_num) {
    int card_index;       // edi@2
    int cards_at_hand;    // eax@8
    int v12;              // ecx@8
    ArcomageCard *pCard;  // eax@8

    if (card_slot_num <= -1) return false;

    int v4 = 0;
    for (card_index = 0; card_index < 10; ++card_index) {
        if (am_Players[player_num].cards_at_hand[card_index] != -1) {
            if (card_slot_num == v4) break;
            ++v4;
        }
    }

    if (CanCardBePlayed(player_num, card_index)) {
        ArcomageGame::PlaySound(23);
        cards_at_hand = GetPlayerHandCardCount(current_player_num);
        pArcomageGame->field_F6 = 1;
        v12 = am_Players[player_num].card_shift[card_index].x +
              (window->GetWidth() - 96 * cards_at_hand) / (cards_at_hand + 1) +
              96 * card_index;

        //  v13 = *(int *)((char *)&am_Players[0].arr_6C[0][1] + v10) + 327;
        amuint_4FAA5C_blt_xy.x = v12;  // v12;
        amuint_4FAA5C_blt_xy.y =
            am_Players[player_num].card_shift[card_index].y + 327;  // v13;

        amuint_4FAA54_blt_xy.x = (272 - v12) / 5;
        amuint_4FAA54_blt_xy.y = -30;

        pCard = &pCards[am_Players[player_num].cards_at_hand[card_index]];
        am_Players[player_num].resource_bricks -= pCard->needed_bricks;
        am_Players[player_num].resource_beasts -= pCard->needed_beasts;
        am_Players[player_num].resource_gems -= pCard->needed_gems;
        played_card_id = am_Players[player_num].cards_at_hand[card_index];
        am_Players[player_num].cards_at_hand[card_index] = -1;
        return true;
    } else {
        return false;
    }
}

bool CanCardBePlayed(int player_num, int hand_card_indx) {
    bool result;              // eax@1
    ArcomageCard *test_card;  // ecx@1
    ArcomagePlayer *pPlayer;  // esi@1

    pPlayer = &am_Players[player_num];
    result = true;
    test_card = &pCards[am_Players[player_num].cards_at_hand[hand_card_indx]];
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
            if (PLAYER->##FIELD < ENEMY->##FIELD) {       \
                PLAYER->##FIELD = ENEMY->##FIELD;         \
                RES = ENEMY->##FIELD - PLAYER->##FIELD;   \
            }                                             \
        } else {                                          \
            PLAYER->##FIELD += (signed int)(VAL);         \
            if (PLAYER->##FIELD < 0) PLAYER->##FIELD = 0; \
            RES = (signed int)(VAL);                      \
        }                                                 \
    }

#define APPLY_TO_ENEMY(PLAYER, ENEMY, FIELD, VAL, RES) \
    APPLY_TO_PLAYER(ENEMY, PLAYER, FIELD, VAL, RES)

#define APPLY_TO_BOTH(PLAYER, ENEMY, FIELD, VAL, RES_P, RES_E) \
    if (VAL != 0) {                                            \
        if (VAL == 99) {                                       \
            if (PLAYER->##FIELD != ENEMY->##FIELD) {           \
                if (PLAYER->##FIELD <= ENEMY->##FIELD) {       \
                    PLAYER->##FIELD = ENEMY->##FIELD;          \
                    RES_P = ENEMY->##FIELD - PLAYER->##FIELD;  \
                } else {                                       \
                    ENEMY->##FIELD = PLAYER->##FIELD;          \
                    RES_E = PLAYER->##FIELD - ENEMY->##FIELD;  \
                }                                              \
            }                                                  \
        } else {                                               \
            PLAYER->##FIELD += (signed int)(VAL);              \
            ENEMY->##FIELD += (signed int)(VAL);               \
            if (PLAYER->##FIELD < 0) {                         \
                PLAYER->##FIELD = 0;                           \
            }                                                  \
            if (ENEMY->##FIELD < 0) {                          \
                ENEMY->##FIELD = 0;                            \
            }                                                  \
            RES_P = (signed int)(VAL);                         \
            RES_E = (signed int)(VAL);                         \
        }                                                      \
    }
    ArcomagePlayer *player;  // esi@1
    ArcomagePlayer *enemy;   // edi@1
                             //    int v23; // eax@26
                             //    signed int v24; // ebx@26
                             //    int v103;
                             //    int v104;

    Point v184;           // [sp+Ch] [bp-64h]@488
    int enemy_num;        // [sp+14h] [bp-5Ch]@1
    ArcomageCard *pCard;  // [sp+18h] [bp-58h]@1
    int buildings_e;      // [sp+1Ch] [bp-54h]@1
    int buildings_p;      // [sp+20h] [bp-50h]@1
    int quarry_p;         // [sp+28h] [bp-48h]@1
    int dmg_e;            // [sp+2Ch] [bp-44h]@1
    int dmg_p;            // [sp+30h] [bp-40h]@1
    int bricks_p;         // [sp+34h] [bp-3Ch]@1
    int tower_e;          // [sp+38h] [bp-38h]@1
    int tower_p;          // [sp+3Ch] [bp-34h]@1
    int wall_e;           // [sp+40h] [bp-30h]@1
    int wall_p;           // [sp+44h] [bp-2Ch]@1
    int beasts_e;         // [sp+48h] [bp-28h]@1
    int beasts_p;         // [sp+4Ch] [bp-24h]@1
    int gems_e;           // [sp+50h] [bp-20h]@1
    int gems_p;           // [sp+54h] [bp-1Ch]@1
    int bricks_e;         // [sp+58h] [bp-18h]@1
    int zoo_e;            // [sp+5Ch] [bp-14h]@1
    int zoo_p;            // [sp+60h] [bp-10h]@1
    int magic_e;          // [sp+64h] [bp-Ch]@1
    int magic_p;          // [sp+68h] [bp-8h]@1
    int quarry_e;         // [sp+6Ch] [bp-4h]@1

    quarry_p = 0;
    magic_p = 0;
    zoo_p = 0;
    bricks_p = 0;
    gems_p = 0;
    beasts_p = 0;
    wall_p = 0;
    tower_p = 0;
    buildings_p = 0;
    dmg_p = 0;
    quarry_e = 0;
    magic_e = 0;
    zoo_e = 0;
    bricks_e = 0;
    gems_e = 0;
    beasts_e = 0;
    wall_e = 0;
    tower_e = 0;
    buildings_e = 0;
    dmg_e = 0;

    player = &am_Players[player_num];
    pCard = &pCards[uCardID];
    enemy_num = (player_num + 1) % 2;
    enemy = &am_Players[enemy_num];
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
            dword_4FAA68 =
                pCard->draw_extra_card_count + (pCard->field_30 == 1);
            dword_4FAA64 = pCard->draw_extra_card_count;
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
            dword_4FAA68 = pCard->can_draw_extra_card2 + (pCard->field_4D == 1);
            dword_4FAA64 = pCard->can_draw_extra_card2;
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
    if (buildings_p || buildings_e || dmg_p || dmg_e)
        pArcomageGame->PlaySound(48);
    if (wall_p > 0 || wall_e > 0) pArcomageGame->PlaySound(49);
    if (wall_p < 0 || wall_e < 0) pArcomageGame->PlaySound(50);
    if (tower_p > 0 || tower_e > 0) pArcomageGame->PlaySound(52);
    if (tower_p < 0 || tower_e < 0) pArcomageGame->PlaySound(53);
    if (player_num) {
        if (quarry_p) {
            v184.x = 573;
            v184.y = 92;
            am_40D2B4(&v184, quarry_p);
        }
        if (quarry_e) {
            v184.x = 26;
            v184.y = 92;
            am_40D2B4(&v184, quarry_e);
        }
        if (magic_p) {
            v184.x = 573;
            v184.y = 164;
            am_40D2B4(&v184, magic_p);
        }
        if (magic_e) {
            v184.x = 26;
            v184.y = 164;
            am_40D2B4(&v184, magic_e);
        }
        if (zoo_p) {
            v184.x = 573;
            v184.y = 236;
            am_40D2B4(&v184, zoo_p);
        }
        if (zoo_e) {
            v184.x = 26;
            v184.y = 236;
            am_40D2B4(&v184, zoo_e);
        }
        if (bricks_p) {
            v184.x = 563;
            v184.y = 114;
            am_40D2B4(&v184, bricks_p);
        }
        if (bricks_e) {
            v184.x = 16;
            v184.y = 114;
            am_40D2B4(&v184, bricks_e);
        }
        if (gems_p) {
            v184.x = 563;
            v184.y = 186;
            am_40D2B4(&v184, gems_p);
        }
        if (gems_e) {
            v184.x = 16;
            v184.y = 186;
            am_40D2B4(&v184, gems_e);
        }
        if (beasts_p) {
            v184.x = 563;
            v184.y = 258;
            am_40D2B4(&v184, beasts_p);
        }
        if (beasts_e) {
            v184.x = 16;
            v184.y = 258;
            am_40D2B4(&v184, beasts_e);
        }
        if (wall_p) {
            v184.x = 442;
            v184.y = 296;
            am_40D2B4(&v184, wall_p);
        }
        if (wall_e) {
            v184.x = 180;
            v184.y = 296;
            am_40D2B4(&v184, wall_e);
        }
        if (tower_p) {
            v184.x = 514;
            v184.y = 296;
            am_40D2B4(&v184, tower_p);
        }
        if (tower_e) {
            v184.x = 122;
            v184.y = 296;
            am_40D2B4(&v184, tower_e);
        }
        if (dmg_p) {
            v184.x = 442;
            v184.y = 296;
            am_40D2B4(&v184, dmg_p);
        }
        if (buildings_p) {
            v184.x = 514;
            v184.y = 296;
            am_40D2B4(&v184, buildings_p);
        }
        if (dmg_e) {
            v184.x = 180;
            v184.y = 296;
            am_40D2B4(&v184, dmg_e);
        }
        if (buildings_e) {
            v184.x = 122;
            v184.y = 296;
            am_40D2B4(&v184, buildings_e);
        }
    } else {
        if (quarry_p) {
            v184.x = 26;
            v184.y = 92;
            am_40D2B4(&v184, quarry_p);
        }
        if (quarry_e) {
            v184.x = 573;
            v184.y = 92;
            am_40D2B4(&v184, quarry_e);
        }
        if (magic_p) {
            v184.x = 26;
            v184.y = 164;
            am_40D2B4(&v184, magic_p);
        }
        if (magic_e) {
            v184.x = 573;
            v184.y = 164;
            am_40D2B4(&v184, magic_e);
        }
        if (zoo_p) {
            v184.x = 26;
            v184.y = 236;
            am_40D2B4(&v184, zoo_p);
        }
        if (zoo_e) {
            v184.x = 573;
            v184.y = 236;
            am_40D2B4(&v184, zoo_e);
        }
        if (bricks_p) {
            v184.x = 16;
            v184.y = 114;
            am_40D2B4(&v184, bricks_p);
        }
        if (bricks_e) {
            v184.x = 563;
            v184.y = 114;
            am_40D2B4(&v184, bricks_e);
        }
        if (gems_p) {
            v184.x = 16;
            v184.y = 186;
            am_40D2B4(&v184, gems_p);
        }
        if (gems_e) {
            v184.x = 563;
            v184.y = 186;
            am_40D2B4(&v184, gems_e);
        }
        if (beasts_p) {
            v184.x = 16;
            v184.y = 258;
            am_40D2B4(&v184, beasts_p);
        }
        if (beasts_e) {
            v184.x = 563;
            v184.y = 258;
            am_40D2B4(&v184, beasts_e);
        }
        if (wall_p) {
            v184.x = 180;
            v184.y = 296;
            am_40D2B4(&v184, wall_p);
        }
        if (wall_e) {
            v184.x = 442;
            v184.y = 296;
            am_40D2B4(&v184, wall_e);
        }
        if (tower_p) {
            v184.x = 122;
            v184.y = 296;
            am_40D2B4(&v184, tower_p);
        }
        if (tower_e) {
            v184.x = 514;
            v184.y = 296;
            am_40D2B4(&v184, tower_e);
        }
        if (dmg_p) {
            v184.x = 180;
            v184.y = 296;
            am_40D2B4(&v184, dmg_p);
        }
        if (buildings_p) {
            v184.x = 122;
            v184.y = 296;
            am_40D2B4(&v184, buildings_p);
        }
        if (dmg_e) {
            v184.x = 442;
            v184.y = 296;
            am_40D2B4(&v184, dmg_e);
        }
        if (buildings_e) {
            v184.x = 514;
            v184.y = 296;
            am_40D2B4(&v184, buildings_e);
        }
    }
#undef APPLY_TO_BOTH
#undef APPLY_TO_ENEMY
#undef APPLY_TO_PLAYER
}

int am_40D2B4(Point *startXY, int effect_value) {
    int v2;      // ebp@1
    int result;  // eax@3
    int v6;
    stru272_stru0 *v8;  // ecx@12
    signed int v11;     // [sp+10h] [bp-8h]@1

    v11 = 0;
    v2 = effect_value;

    while (array_4FABD0[v11].have_effect) {  // Ritor1: needed refactoring
        result = array_4FABD0[v11].field_40->_40E2A7();
        if (!result) {
            array_4FABD0[v11].have_effect = 0;
            --v11;
        }
        ++v11;
        if (v11 >= 10) return result;
    }
    v6 = v11;
    array_4FABD0[v11].have_effect = 1;
    if (effect_value <= 0) {
        array_4FABD0[v6].effect_sign = 0;
        effect_value = -effect_value;
    } else {
        array_4FABD0[v6].effect_sign = 1;
    }
    array_4FABD0[v6].field_4.effect_area.x = startXY->x - 20;
    array_4FABD0[v6].field_4.effect_area.z = startXY->x + 20;
    array_4FABD0[v6].field_4.effect_area.y = startXY->y - 20;
    array_4FABD0[v6].field_4.effect_area.w = startXY->y + 20;
    array_4FABD0[v6].field_4.field_10 = -60;
    array_4FABD0[v6].field_4.field_14 = 60;
    array_4FABD0[v6].field_4.field_18 = 180;
    array_4FABD0[v6].field_4.field_1Cf = 0.5;
    array_4FABD0[v6].field_4.field_20 = 150;
    array_4FABD0[v6].field_4.field_24f = 50.0;
    array_4FABD0[v6].field_4.field_28f = 3.0;
    array_4FABD0[v6].field_4.field_2Cf = 8.0;
    array_4FABD0[v6].field_4.field_30 = 5;
    array_4FABD0[v6].field_4.field_34 = 15;
    array_4FABD0[v6].field_4.sparks_array = &array_4FABD0[v6].effect_sparks[0];
    v8 = array_4FABD0[v6].field_40;
    v8->StartFill(&array_4FABD0[v6].field_4);
    if (10 * effect_value > 150) effect_value = 15;

    if (v8->signature != SIG_trpg) return 2;
    if (!v8->field_59) return 3;
    v8->position_in_sparks_arr = 10 * effect_value;
    v8->field_30 = 0.0;
    v8->field_58 = 0;
    v8->field_44 = 0;
    v8->field_4C = 0;
    v8->field_48 = 0;
    v8->field_50 = 0;
    for (int i = 0; i < v8->field_4; ++i) v8->field_54[i].have_spark = 0;
    return 0;
}

int ApplyDamageToBuildings(int player_num, int damage) {
    int v3 = am_Players[player_num].wall_height;
    // if ( v3 <= 0 )
    int result = 0;
    // else
    //{
    if (v3 >= -damage) {
        result = damage;
        am_Players[player_num].wall_height += damage;
    } else {
        damage += v3;
        result = -v3;
        am_Players[player_num].wall_height = 0;
        am_Players[player_num].tower_height += damage;
    }
    //}
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
    int v2;      // esi@4
    int v3;      // esi@5
    // int v4;      // edi@5
    Rect pXYZW;  // [sp+8h] [bp-1Ch]@5
    Point pXY;   // [sp+18h] [bp-Ch]@5

    pAudioPlayer->StopChannels(-1, -1);
    strcpy_s(pArcomageGame->pPlayer1Name, Player1Name);
    strcpy_s(pArcomageGame->pPlayer2Name, Player2Name);
    am_byte_4FAA76 = 0;
    am_byte_4FAA75 = 0;

    for (int i = 0; i < 10; ++i) {
        v2 = (i + 1) % 4;
        v3 = (i + 1) / 4;
        shown_cards[i].uCardId = -1;
        shown_cards[i].field_4 = 0;
        shown_cards[i].field_8.x = 100 * v2 + 120;
        shown_cards[i].field_8.y = 138 * v3 + 18;
        shown_cards[i].field_10_xplus = -100 * v2 / 5;
        shown_cards[i].field_14_y_plus = -138 * v3 / 5;
        shown_cards[i].field_18_point.x = shown_cards[i].field_8.x;
        shown_cards[i].field_18_point.y = shown_cards[i].field_8.y;
    }

    pXY.x = 0;
    pXY.y = 0;
    ArcomageGame::LoadBackground();
    pXYZW.x = 0;
    pXYZW.z = window->GetWidth();
    pXYZW.y = 0;
    pXYZW.w = window->GetHeight();
    am_BeginScene(pArcomageGame->pBackgroundPixels, -1, 1);
    render->am_Blt_Copy(&pXYZW, &pXY, 2);
    am_EndScene();
    render->Present();
    ArcomageGame::LoadSprites();
    render->Present();

    //  v4 = 120;
    //  for (int i = 0; i < 12; ++i)
    //    am_sounds[i] = pSoundList->LoadSound(v4++, 0);

    for (int i = 0; i < 10; ++i)
        array_4FABD0[i].field_40 = stru272_stru0::New();

    current_card_slot_index = -1;
    amuint_4FAA4C = -1;
    byte_4FAA74 = 0;
    pArcomageGame->field_F4 = 0;
    am_gameover = false;
    byte_505880 = 0;
    dword_4FAA70 = 0;
    need_to_discard_card = 0;
    SetStartGameData();
    InitalHandsFill();
    // nullsub_1();
    pArcomageGame->GameOver = 0;
    pArcomageGame->pfntComic = pFontComic;
    pArcomageGame->pfntArrus = pFontArrus;
}

ArcomageGame::ArcomageGame() {
    field_4 = 0;
    bGameInProgress = 0;
    field_F9 = 0;
}

void SetStartConditions() {
    const ArcomageStartConditions *st_cond;  // eax@1

    st_cond = &start_conditions[window_SpeakInHouse->par1C - 108];
    start_tower_height = st_cond->tower_height;
    start_wall_height = st_cond->wall_height;
    start_quarry_level = st_cond->quarry_level - 1;
    start_magic_level = st_cond->magic_level - 1;
    start_zoo_level = st_cond->zoo_level - 1;
    minimum_cards_at_hand = 5;
    quarry_bonus = 1;
    magic_bonus = 1;
    zoo_bonus = 1;
    max_tower_height = st_cond->max_tower;
    max_resources_amount = st_cond->max_resources;

    opponent_mastery = st_cond->mastery_lvl;

    start_bricks_amount = st_cond->bricks_amount;
    start_gems_amount = st_cond->gems_amount;
    start_beasts_amount = st_cond->beasts_amount;
}

void am_DrawText(const String &str, Point *pXY) {
    pPrimaryWindow->DrawText(pFontComic, pXY->x,
                             pXY->y - ((pFontComic->GetHeight() - 3) / 2) + 3,
                             0, str, 0, 0, 0);
}

void DrawRect(Rect *pXYZW, unsigned __int16 uColor, char bSolidFill) {
    render->BeginScene();
    render->SetUIClipRect(0, 0, window->GetWidth() - 1,
                          window->GetHeight() - 1);
    if (bSolidFill) {
        for (int i = pXYZW->y; i <= pXYZW->w; ++i)
            render->RasterLine2D(pXYZW->x, i, pXYZW->z, i, uColor);
    } else {
        render->RasterLine2D(pXYZW->x, pXYZW->y, pXYZW->z, pXYZW->y, uColor);
        render->RasterLine2D(pXYZW->z, pXYZW->y, pXYZW->z, pXYZW->w, uColor);
        render->RasterLine2D(pXYZW->z, pXYZW->w, pXYZW->x, pXYZW->w, uColor);
        render->RasterLine2D(pXYZW->x, pXYZW->w, pXYZW->x, pXYZW->y, uColor);
    }
    render->EndScene();
}

void DrawSquare(Point *pTargetXY, unsigned __int16 uColor) {
    render->BeginScene();
    if (pTargetXY->x >= 0 && pTargetXY->x <= window->GetWidth() - 1 &&
        pTargetXY->y >= 0 && pTargetXY->y <= window->GetHeight() - 1) {
        render->WritePixel16(pTargetXY->x, pTargetXY->y, uColor);
        render->WritePixel16(pTargetXY->x + 1, pTargetXY->y, uColor);
        render->WritePixel16(pTargetXY->x, pTargetXY->y + 1, uColor);
        render->WritePixel16(pTargetXY->x + 1, pTargetXY->y + 1, uColor);
    }
    render->EndScene();
}

void DrawPixel(Point *pTargetXY, unsigned __int16 uColor) {
    render->BeginScene();
    if (pTargetXY->x >= 0 && pTargetXY->x <= window->GetWidth() - 1 &&
        pTargetXY->y >= 0 && pTargetXY->y <= window->GetHeight() - 1) {
        render->WritePixel16(pTargetXY->x, pTargetXY->y, uColor);
    }
    render->EndScene();
}

int rand_interval(int min, int max) { return min + rand() % (max - min + 1); }

void am_IntToString(int val, char *pOut) { sprintf(pOut, "%d", val); }

void set_stru1_field_8_InArcomage(int inValue) {
    switch (inValue) {
        case 91:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 123;
            break;
        case 92:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 124;
            break;
        case 93:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 125;
            break;
        case 96:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 126;
            break;
        case 61:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 43;
            break;
        case 55:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 38;
            break;
        case 56:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 42;
            break;
        case 57:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 40;
            break;
        case 59:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 58;
            break;
        case 54:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 94;
            break;
        case 50:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 64;
            break;
        case 51:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 35;
            break;
        case 52:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 36;
            break;
        case 53:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 37;
            break;
        case 49:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 33;
            break;
        case 39:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 34;
            break;
        case 44:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 60;
            break;
        case 46:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 62;
            break;
        case 47:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 63;
            break;
        case 48:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = 41;
            break;
        default:
            HEXRAYS_LOBYTE(pArcomageGame->stru1.field_8) = inValue;
            break;
    }
}
