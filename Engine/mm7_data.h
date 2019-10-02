#pragma once

#include <array>
#include <string>

#include "Engine/Strings.h"
#include "Engine/Time.h"

#include "NZIArray.h"

#include "VectorTypes.h"

class GUIButton;

//-------------------------------------------------------------------------
// Data declarations

extern int game_starting_year;
extern unsigned int game_viewport_width;
extern unsigned int game_viewport_height;
extern int game_viewport_x;
extern int game_viewport_y;
extern int game_viewport_z;
extern int game_viewport_w;

extern int pWindowList_at_506F50_minus1_indexing[1];
extern int dword_4C9890[10];
extern int dword_4C9920[16];
extern char _4D864C_force_sw_render_rules;
extern float flt_4D86CC;
extern int dword_4D86D8;

extern float flt_4D84E8;

extern unsigned int uGammaPos;
extern std::array<int, 8> BtnTurnCoord;
extern std::array<int16_t, 4> RightClickPortraitXmin;
extern std::array<int16_t, 4> RightClickPortraitXmax;
// extern int pArmorSkills[5];
// extern int pWeaponSkills[9];
// extern int pMiscSkills[12];
// extern int pMagicSkills[9];
extern std::array<unsigned int, 4> pHealthBarPos;
extern std::array<unsigned int, 4> pManaBarPos;
// extern std::array<char, 80> _4E2B21_buff_spell_tooltip_colors;
extern std::array<char, 88> monster_popup_y_offsets;
extern unsigned char hourglass_icon_idx;

extern const char *format_4E2D80;
extern char format_4E2D90[8];
extern const char *format_4E2DE8;  // idb
extern const char *Stat_string_format_2_column_text;
extern const char
    *Stat_string_format_2_column_less_100;  // format text of resistance in
                                            // Stats screen
extern const char *Stat_string_format_2_column_over_100;
extern int dword_4E455C;
extern std::array<int, 6> dword_4E4560;
extern std::array<int, 6> dword_4E4578;
extern std::array<int, 6> dword_4E4590;
extern std::array<int, 6> dword_4E45A8;
extern std::array<float, 10> flt_4E4A80;

extern std::array<std::array<int, 2>, 14> pPartySpellbuffsUI_XYs;
extern std::array<unsigned char, 14> byte_4E5DD8;
extern std::array<uint8_t, 14> pPartySpellbuffsUI_smthns;
extern std::array<std::array<int, 6>, 6> pNPCPortraits_x;  // 004E5E50
extern std::array<std::array<int, 6>, 6> pNPCPortraits_y;  // 004E5EE0
extern std::array<const char *, 11> pHouse_ExitPictures;
extern std::array<const char *, 11> _4E6BDC_loc_names;
extern std::array<int16_t, 11> word_4E8152;
extern std::array<std::array<char, 14>, 7> byte_4E8168;
extern char _4E94D0_light_type;
extern char _4E94D2_light_type;
extern char _4E94D3_light_type;
extern std::array<unsigned int, 2> saveload_dlg_xs;
extern std::array<unsigned int, 2> saveload_dlg_ys;
extern std::array<unsigned int, 2> saveload_dlg_zs;
extern std::array<unsigned int, 2> saveload_dlg_ws;
extern std::array<char *, 465> pTransitionStrings;  // 4EB080
extern std::array<int, 9> dword_4EC268;
extern std::array<int, 7> dword_4EC28C;
extern std::array<const char *, 25> pPlayerPortraitsNames;

extern std::array<std::array<unsigned char, 25>, 48> byte_4ECF08;
extern std::array<std::array<unsigned char, 8>, 110> SoundSetAction;
extern std::array<int16_t, 4>
    pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing;
extern std::array<std::array<char, 37>, 36>
    byte_4ED970_skill_learn_ability_by_class_table;
extern std::array<unsigned int, 2> pHiredNPCsIconsOffsetsX;
extern std::array<unsigned int, 2> pHiredNPCsIconsOffsetsY;
extern std::array<int, 2> Party_Spec_Motion_status_ids;  // dword_4EE07C
extern std::array<short, 28> word_4EE150;
extern int ScreenshotFileNumber;  // dword_4EFA80
extern int16_t word_4F0576[];

extern std::array<int, 32> guild_mambership_flags;
extern std::array<int16_t, 49> word_4F0754;
extern std::array<int16_t, 54> _4F0882_evt_VAR_PlayerItemInHands_vals;
extern std::array<unsigned short, 6> pMaxLevelPerTrainingHallType;
extern std::array<int, 11> price_for_membership;
extern std::array<int16_t, 32> word_4F0F30;
extern std::array<int, 500> ai_array_4F5E68;
extern std::array<int, 500> ai_array_4F6638_actor_ids;
extern std::array<int, 500> ai_near_actors_targets_pid;
extern int ai_arrays_size;
extern std::array<int, 500> ai_near_actors_distances;
extern std::array<unsigned int, 500> ai_near_actors_ids;
extern std::array<int, 182> dword_4F8580;
extern int dword_4FA9B0[];
extern int dword_4FA9B4[];

extern char byte_4FAA24;
// extern HWND dword_4FAA28; // idb

extern std::array<unsigned int, 480> pSRZBufferLineOffsets;
extern std::array<char, 777> books_num_items_per_page;
extern int lloyds_beacon_spell_id;  // dword_506338
extern int lloyds_beacon_sound_id;  // dword_50633C
extern signed int sRecoveryTime;    // idb
extern unsigned int uRequiredMana;  // idb
extern int _506348_current_lloyd_playerid;
extern int64_t lloyds_beacon_spell_level;  // qword_506350
extern int MapBookOpen;
// extern Texture_MM7 *dword_50640C[];
extern int
    books_page_number;  // number for page in books(номер страницы в книгах)
extern int
    books_primary_item_per_page;  // number primary item in book page(номер
                                  // начальной записи на странице)
extern int Autonotes_Instructors_page_flag;  // dword_50652C
extern int Autonotes_Misc_page_flag;         // dword_506530
extern int Book_PageBtn6_flag;               // dword_506534
extern int Book_PageBtn5_flag;               // dword_506538
extern int Book_PageBtn4_flag;               // dword_50653C
extern int Book_PageBtn3_flag;               // dword_506540
extern int BtnDown_flag;                     // BtnDown_flag
extern int BtnUp_flag;                       // BtnUp_flag
extern int quick_spell_at_page;
extern char byte_506550;
extern int _506568_autonote_type;  // 506568
extern bool bRecallingBeacon;
extern int uLastPointedObjectID;
extern int dword_506980_uW;
extern int dword_506984_uZ;
extern int dword_506988_uY;
extern int dword_50698C_uX;
extern int uGameMenuUI_CurentlySelectedKeyIdx;                   // 506E68
extern std::array<bool, 28> GameMenuUI_InvaligKeyBindingsFlags;  // 506E6C
extern std::array<unsigned int, 28> pPrevVirtualCidesMapping;
extern int KeyboardPageNum;
extern int uRestUI_FoodRequiredToRest;
extern int _506F14_resting_stage;
extern int _506F18_num_minutes_to_sleep;
extern int dword_506F1C;
extern char bFlashHistoryBook;
extern char bFlashAutonotesBook;
extern char bFlashQuestBook;
extern GUIButton *pBtn_ZoomOut;  // idb
extern GUIButton *pBtn_ZoomIn;  // idb
extern unsigned int uGameUIFontShadow;
extern unsigned int uGameUIFontMain;
extern int dword_507B00_spell_info_to_draw_in_popup;
extern unsigned int uActiveCharacter;
extern int dword_507BF0_is_there_popup_onscreen;
extern int awards_scroll_bar_created;
extern int dword_507CC0_activ_ch;
extern GameTime GameUI_RightPanel_BookFlashTimer;
extern int _507CD4_RestUI_hourglass_anim_controller;
extern bool OpenedTelekinesis;
extern std::array<int, 50> dword_50B570;
extern std::array<int, 50> dword_50B638;
extern struct stru367 PortalFace;
extern std::array<int, 100> dword_50BC10;
extern std::array<int, 100> dword_50BDA0;
extern std::array<int, 100> _50BF30_actors_in_viewport_ids;
extern char town_portal_caster_id;
extern int some_active_character;
extern std::array<unsigned int, 5> pIconIDs_Turn;
extern unsigned int uIconID_TurnStop;
extern unsigned int uIconID_TurnHour;
extern int uIconID_CharacterFrame;  // idb
extern unsigned int uIconID_TurnStart;
extern int dword_50C994;
extern int dword_50C998_turnbased_icon_1A;
extern int uSpriteID_Spell11;  // idb
extern bool _50C9A0_IsEnchantingInProgress;
extern int _50C9A8_item_enchantment_timer;
extern int _50C9D0_AfterEnchClickEventId;
extern int _50C9D4_AfterEnchClickEventSecondParam;
extern int _50C9D8_AfterEnchClickEventTimeout;
extern int dword_50C9DC;
extern struct NPCData *ptr_50C9E0;
// extern int dword_50C9E8; // idb
// extern int dword_50C9EC[]; // 50C9EC
extern int dword_50CDC8;
extern Vec3_int_ layingitem_vel_50FDFC;
extern char pStartingMapName[32];  // idb
extern std::array<uint8_t, 5> IsPlayerWearingWatersuit;
extern std::array<char, 54> party_has_equipment;
extern std::array<char, 17> byte_5111F6_OwnedArtifacts;

extern unsigned int uNumBlueFacesInBLVMinimap;  // could remove
extern std::array<uint16_t, 50> pBlueFacesInBLVMinimapIDs;
extern std::array<class Image *, 14> party_buff_icons;
extern unsigned int uIconIdx_FlySpell;
extern unsigned int uIconIdx_WaterWalk;
extern int dword_576E28;
extern int _576E2C_current_minimap_zoom;
extern GameTime _5773B8_event_timer;  // 5773B8

extern struct Actor *pDialogue_SpeakingActor;
extern unsigned int uDialogueType;
extern signed int sDialogue_SpeakingActorNPC_ID;
extern struct LevelDecoration *_591094_decoration;
extern int uCurrentHouse_Animation;
extern char *dword_591164_teleport_map_name;  // idb
extern int dword_591168_teleport_speedz;
extern int dword_59116C_teleport_directionx;
extern int dword_591170_teleport_directiony;
extern int dword_591174_teleportz;
extern int dword_591178_teleporty;
extern int dword_59117C_teleportx;
extern std::array<std::array<char, 100>, 6> byte_591180;  // idb
extern std::array<struct NPCData *, 7>
    HouseNPCData;  // 0this array size temporarily increased to 60 from 6 to
                   // work aroud house overflow
extern GUIButton *HouseNPCPortraitsButtonsList[6];
extern String branchless_dialogue_str;
extern int EvtTargetObj;
extern int _unused_5B5924_is_travel_ui_drawn;  // 005B5924
extern int _5B65A8_npcdata_uflags_or_other;
extern int _5B65AC_npcdata_fame_or_other;
extern int _5B65B0_npcdata_rep_or_other;
extern int _5B65B4_npcdata_loword_house_or_other;
extern int _5B65B8_npcdata_hiword_house_or_other;
extern int dword_5B65BC;
extern int dword_5B65C0;
extern int dword_5B65C4_cancelEventProcessing;
extern int MapsLongTimers_count;  // dword_5B65C8
extern int npcIdToDismissAfterDialogue;
extern int dword_5B65D0_dialogue_actor_npc_id;
extern int dword_5C3418;
extern int dword_5C341C;
// extern std::array<char, 777> byte_5C3427;
extern String game_ui_status_bar_event_string;
extern String game_ui_status_bar_string;
extern unsigned int game_ui_status_bar_event_string_time_left;
extern int bForceDrawFooter;
extern int _5C35C0_force_party_death;
extern int bDialogueUI_InitializeActor_NPC_ID;
extern char *p2DEventsTXT_Raw;

extern int dword_5C35D4;
extern char byte_5C45AF[];
extern std::array<char, 10000> pTmpBuf3;
extern std::array<char, 100> Journal_limitation_factor;
extern int ui_current_text_color;
extern int64_t qword_5C6DF0;
extern int dword_5C6DF8;
extern char *pHealthPointsAttributeDescription;
extern char *pSpellPointsAttributeDescription;
extern char *pArmourClassAttributeDescription;
extern char *pPlayerConditionAttributeDescription;  // idb
extern char *pFastSpellAttributeDescription;
extern char *pPlayerAgeAttributeDescription;
extern char *pPlayerLevelAttributeDescription;
extern char *pPlayerExperienceAttributeDescription;
extern char *pAttackBonusAttributeDescription;
extern char *pAttackDamageAttributeDescription;
extern char *pMissleBonusAttributeDescription;
extern char *pMissleDamageAttributeDescription;
extern char *pFireResistanceAttributeDescription;
extern char *pAirResistanceAttributeDescription;
extern char *pWaterResistanceAttributeDescription;
extern char *pEarthResistanceAttributeDescription;
extern char *pMindResistanceAttributeDescription;
extern char *pBodyResistanceAttributeDescription;
extern char *pSkillPointsAttributeDescription;
extern struct FactionTable *pFactionTable;

extern char *pMonstersTXT_Raw;
extern char *pMonsterPlacementTXT_Raw;
extern char *pSpellsTXT_Raw;
extern char *pMapStatsTXT_Raw;
extern char *pHostileTXT_Raw;
extern char *pPotionsTXT_Raw;
extern char *pPotionNotesTXT_Raw;

extern std::array<int, 777> _6807B8_level_decorations_ids;  // idb
extern int _6807E0_num_decorations_with_sounds_6807B8;
extern std::array<int, 3000> event_triggers;  // 6807E8
extern size_t num_event_triggers;             // 6836C8
extern std::array<int, 64> dword_69B010;
extern float flt_69B138_dist;
extern char byte_69BD41_unused;
extern int pSaveListPosition;
extern unsigned int uLoadGameUI_SelectedSlot;
extern char cMM7GameCDDriveLetter;  // idb
extern unsigned int uGameState;
extern int uDefaultTravelTime_ByFoot;
extern int day_attrib;
extern int day_fogrange_1;
extern int day_fogrange_2;
extern struct TileTable *pTileTable;              // idb
extern std::array<char, 777> pDefaultSkyTexture;  // idb
extern std::array<char, 16>
    byte_6BE124_cfg_textures_DefaultGroundTexture;  // idb
extern int _6BE134_odm_main_tile_group;
extern int dword_6BE138;
extern int dword_6BE13C_uCurrentlyLoadedLocationID;
extern float fWalkSpeedMultiplier;
extern float fBackwardWalkSpeedMultiplier;
extern float fTurnSpeedMultiplier;
extern float flt_6BE150_look_up_down_dangle;
// extern char pMM7WindowClassName[]; // idb
// extern HINSTANCE hInstance; // idb
// extern char *pCmdLine;
// extern HWND hWnd; // idb
extern String pCurrentMapName;
extern unsigned int uLevelMapStatsID;

#define GAME_SETTINGS_0001 0x0001
#define GAME_SETTINGS_INVALID_RESOLUTION 0x0002
#define GAME_SETTINGS_NO_INTRO 0x0004
#define GAME_SETTINGS_NO_LOGO 0x0008
#define GAME_SETTINGS_NO_SOUND 0x0010
#define GAME_SETTINGS_NO_WALK_SOUND 0x0020
#define GAME_SETTINGS_NO_HOUSE_ANIM 0x0040
#define GAME_SETTINGS_0080_SKIP_USER_INPUT_THIS_FRAME 0x0080
#define GAME_SETTINGS_APP_INACTIVE 0x0100
#define GAME_SETTINGS_0200_EVENT_TIMER 0x0200
#define GAME_SETTINGS_0400_MISC_TIMER 0x0400
#define GAME_SETTINGS_0800 0x0800
#define GAME_SETTINGS_1000 0x1000
#define GAME_SETTINGS_2000 0x2000  // initialisation state?
#define GAME_SETTINGS_4000 0x4000
extern int dword_6BE364_game_settings_1;  // GAME_SETTINGS_*

extern float _6BE3A0_fov;
extern float flt_6BE3A4_debug_recmod1;
extern float flt_6BE3A8_debug_recmod2;
extern float flt_6BE3AC_debug_recmod1_x_1_6;
extern std::string s_SavedMapName;
extern char bNoNPCHiring;
extern std::array<int, 100> dword_720020_zvalues;
extern std::array<int, 299> dword_7201B0_zvalues;
extern std::array<int16_t, 104> word_7209A0_intercepts_ys_plus_ys;
extern std::array<int16_t, 104> word_720A70_intercepts_xs_plus_xs;
extern std::array<int16_t, 104> word_720B40_intercepts_zs;
extern std::array<int16_t, 102> word_720C10_intercepts_xs;
extern std::array<int16_t, 777> word_720CE0_ys;  // idb
extern std::array<int16_t, 777> word_720DB0_xs;  // idb
extern std::array<int, 20> dword_720E80;
extern std::array<int, 20> dword_720ED0;
extern std::array<int, 20> ceiling_height_level;
extern std::array<int16_t, 104> odm_floor_face_vert_coord_Y;  // word_720F70
extern std::array<int16_t, 104> odm_floor_face_vert_coord_X;  // word_721040
extern std::array<int, 20> current_Face_id;                   // dword_721110
extern std::array<int, 20> current_BModel_id;                 // dword_721160
extern std::array<int, 20> odm_floor_level;  // idb dword_7211B0
extern int blv_prev_party_x;
extern int blv_prev_party_z;
extern int blv_prev_party_y;
extern char *dword_722F10;  // idb
extern std::array<const char *, 513> pQuestTable;
extern char *dword_723718_autonote_related;  // idb
extern std::array<const char *, 82> pScrolls;
extern int dword_7241C8;

extern char *pAwardsTXT_Raw;
extern char *pScrollsTXT_Raw;
extern char *pMerchantsTXT_Raw;
extern std::array<const char *, 7> pMerchantsBuyPhrases;
extern std::array<const char *, 7> pMerchantsSellPhrases;
extern std::array<const char *, 7> pMerchantsRepairPhrases;
extern std::array<const char *, 7> pMerchantsIdentifyPhrases;
extern char *pTransitionsTXT_Raw;
extern char *pAutonoteTXT_Raw;
extern char *pQuestsTXT_Raw;
extern unsigned int uNumTerrainNormals;
extern struct Vec3_float_ *pTerrainNormals;
extern std::array<unsigned short, 128 * 128 * 2> pTerrainNormalIndices;
extern std::array<unsigned int, 128 * 128 * 2> pTerrainSomeOtherData;
extern int dword_A74C88;
extern int uPlayerCreationUI_ArrowAnim;
extern unsigned int uPlayerCreationUI_SelectedCharacter;
extern int dword_A74CDC;
extern int64_t _A750D8_player_speech_timer;  // qword_A750D8
extern int uSpeakingCharacter;
extern char byte_AE5B91;
extern std::array<int, 32> dword_F1B430;
extern std::array<int, 4> player_levels;
extern std::array<int16_t, 6> weapons_Ypos;  // word_F8B158
extern int16_t bountyHunting_monster_id_for_hunting;
extern const char *bountyHunting_text;  // idb
extern int contract_approved;
extern int dword_F8B1AC_award_bit_number;  // idb
extern int dword_F8B1B0_MasteryBeingTaught;
extern int gold_transaction_amount;  // F8B1B4
extern std::array<const char *, 4> pShopOptions;
extern int dword_F8B1D8;
extern int dword_F8B1E0;
extern int dword_F8B1E4;
extern String current_npc_text;  // F8B1E8
extern char dialogue_show_profession_details;
extern std::array<char, 777> byte_F8B1EF;
extern std::array<char, 4> byte_F8B1F0;

extern int bGameoverLoop;
extern std::array<int16_t, 104> intersect_face_vertex_coords_list_a;  // word_F8BC48
extern std::array<int16_t, 104> intersect_face_vertex_coords_list_b;  // word_F8BD18
