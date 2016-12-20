#pragma once
#include "OSAPI.h"

#include "Engine/Graphics/Image.h"




/*  272 */
#pragma pack(push, 1)
struct ArcomageCard
{
  char pCardName[32];
  int slot;
  char field_24;
  char needed_quarry_level;
  char needed_magic_level;
  char needed_zoo_level;
  char needed_bricks;
  char needed_gems;
  char needed_beasts;
  bool can_be_discarded;
  int  compare_param;
  char field_30;
  char draw_extra_card_count;
  char to_player_quarry_lvl;
  char to_player_magic_lvl;
  char to_player_zoo_lvl;
  char to_player_bricks;
  char to_player_gems;
  char to_player_beasts;
  char to_player_buildings;
  char to_player_wall;
  char to_player_tower;
  char to_enemy_quarry_lvl;
  char to_enemy_magic_lvl;
  char to_enemy_zoo_lvl;
  char to_enemy_bricks;
  char to_enemy_gems;
  char to_enemy_beasts;
  char to_enemy_buildings;
  char to_enemy_wall;
  char to_enemy_tower;
  char to_pl_enm_quarry_lvl;
  char to_pl_enm_magic_lvl;
  char to_pl_enm_zoo_lvl;
  char to_pl_enm_bricks;
  char to_pl_enm_gems;
  char to_pl_enm_beasts;
  char to_pl_enm_buildings;
  char to_pl_enm_wall;
  char to_pl_enm_tower;
  char field_4D;
  char can_draw_extra_card2;
  char to_player_quarry_lvl2;
  char to_player_magic_lvl2;
  char to_player_zoo_lvl2;
  char to_player_bricks2;
  char to_player_gems2;
  char to_player_beasts2;
  char to_player_buildings2;
  char to_player_wall2;
  char to_player_tower2;
  char to_enemy_quarry_lvl2;
  char to_enemy_magic_lvl2;
  char to_enemy_zoo_lvl2;
  char to_enemy_bricks2;
  char to_enemy_gems2;
  char to_enemy_beasts2;
  char to_enemy_buildings2;
  char to_enemy_wall2;
  char to_enemy_tower2;
  char to_pl_enm_quarry_lvl2;
  char to_pl_enm_magic_lvl2;
  char to_pl_enm_zoo_lvl2;
  char to_pl_enm_bricks2;
  char to_pl_enm_gems2;
  char to_pl_enm_beasts2;
  char to_pl_enm_buildings2;
  char to_pl_enm_wall2;
  char to_pl_enm_tower2;         
  char field_6A;
  char field_6B;
};
#pragma pack(pop)


/*  401 */
#pragma pack(push, 1)
struct AcromageCardOnTable
{
  int uCardId;
  int field_4;
  POINT field_8;
  int field_10_xplus;
  int field_14_y_plus;
  POINT field_18_point;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct ArcomagePlayer
{
  char pPlayerName[32];
  int IsHisTurn;
  int tower_height;
  int wall_height;
  int quarry_level;
  int magic_level;
  int zoo_level;
  int resource_bricks;
  int resource_gems;
  int resource_beasts;
  int cards_at_hand[10];
  POINT card_shift[10];
};
#pragma pack(pop)



#pragma pack(push, 1)
struct ArcomageGame_stru1
{
  int field_0;
  int field_4;
  int field_8;
};
#pragma pack(pop)




#pragma pack(push, 1)
struct ArcomageGame
{
    ArcomageGame();

    static bool LoadBackground();
    static bool LoadSprites();
    static bool MsgLoop(int a1, ArcomageGame_stru1 *a2);
    static void PlaySound(unsigned int event_id);
    static void OnMouseClick(char right_left, bool bDown);
    static void OnMouseMove(int x, int y);
    static void GetCardRect(unsigned int uCardID, RECT *pCardRect);
    static void PrepareArcomage();
    static void DoBlt_Copy(unsigned __int16 *pPixels); // idb

    static void Loop();

    char field_0;
    char field_1;
    char field_2;
    char field_3;
    int field_4;
    ArcomageGame_stru1 stru1;
    tagMSG msg;
    int mouse_x;
    int mouse_y;
    char mouse_left;
    char field_39;
    char mouse_right;
    char field_3B;
    unsigned __int16 *pBackgroundPixels;
    unsigned __int16 *pSpritesPixels;
    unsigned __int16 *pBlit_Copy_pixels;
    struct GUIFont *pfntComic;//ptr_48;
    struct GUIFont *pfntArrus;//ptr_4C;
    int field_50;
    int field_54;
    Image *pGameBackground;
    Image *pSprites;
    int event_timer_time;
    int uGameWinner;
    int Victory_type;
    char pPlayer1Name[32];
    char pPlayer2Name[32];
    char field_F4;
    char GameOver;
    char field_F6;
    char prev_mouse_left;
    char prev_mouse_right;
    char field_F9;
    char bGameInProgress;
};
#pragma pack(pop)




extern ArcomageGame *pArcomageGame;
extern ArcomageCard pCards[87];
extern void set_stru1_field_8_InArcomage(int inValue);



#pragma pack(push, 1)
struct stru272_stru1
{
  int have_spark;
  POINT spark_position;
  int field_C;
  int field_10;
  int field_14;
  int field_18;
};
#pragma pack(pop)

/*  404 */
#pragma pack(push, 1)
struct stru272_stru2
{
/*int field_0;
int field_4;
int field_8;
int field_C;*/
  RECT effect_area;
  int field_10;
  int field_14;
  int field_18;
  float field_1Cf;
  int field_20;
  float field_24f;
  float field_28f;
  float field_2Cf;
  int field_30;
  int field_34;
  stru272_stru1* sparks_array;
};
#pragma pack(pop)


/*  270 */
#pragma pack(push, 1)
struct stru272_stru0         // ARCOMAGE stuff
{
  static stru272_stru0 *New();

  int Free();
  int StartFill(stru272_stru2* a2);
  int Clear(char a2, char a3);
  int DrawEffect();
  int _40E2A7();

  int signature;
  int field_4;
  int position_in_sparks_arr;
  int field_C;
  int field_10;
  int field_14;
  int field_18;
  int field_1C;
  int field_20;
  int field_24;
  float field_28;
  float field_2C;
  float field_30;
  int field_34;
  int field_38;
  int field_3C;
  int field_40;
  int field_44;
  int field_48;
  int field_4C;
  int field_50;
  stru272_stru1 * field_54;
  char field_58;
  char field_59;
  char field_5A;
  char field_5B;
};
#pragma pack(pop)

#define DECK_SIZE 108

#pragma pack(push, 1)
struct ArcomageDeck
{
  char name[32];
  char cardsInUse[DECK_SIZE];
  int cards_IDs[DECK_SIZE];
};

#pragma pack(pop)

#pragma pack(push, 1)
struct stru272
{
  char have_effect;
  char effect_sign;
  char _pad_2;
  char _pad_3;
  stru272_stru2 field_4;
  stru272_stru0 *field_40;
  stru272_stru1 effect_sparks[150];
};
#pragma pack(pop)

