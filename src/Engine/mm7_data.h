#pragma once

#include <array>
#include <vector>
#include <string>
#include <utility>

#include "Engine/Time.h"
#include "Engine/MM7.h"
#include "Engine/Autonotes.h"
#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/NPCEnums.h"
#include "Engine/Objects/PlayerEnums.h"
#include "Engine/MapInfo.h"

#include "GUI/GUIEnums.h"
#include "GUI/GUIDialogues.h"

#include "Utility/IndexedArray.h"
#include "Utility/Geometry/Vec.h"

class GUIButton;
class Actor;

//-------------------------------------------------------------------------
// Data declarations

extern int game_viewport_width;
extern int game_viewport_height;
extern int game_viewport_x;
extern int game_viewport_y;
extern int game_viewport_z;
extern int game_viewport_w;

extern int pWindowList_at_506F50_minus1_indexing[1];
extern int dword_4C9890[10];
extern int dword_4C9920[16];
extern float flt_4D86CC;
extern int dword_4D86D8;

extern float flt_4D84E8;

extern unsigned int uGammaPos;
extern std::array<int, 8> BtnTurnCoord;
extern std::array<int16_t, 4> RightClickPortraitXmin;
extern std::array<int16_t, 4> RightClickPortraitXmax;
extern std::array<unsigned int, 4> pHealthBarPos;
extern std::array<unsigned int, 4> pManaBarPos;
// extern std::array<char, 80> _4E2B21_buff_spell_tooltip_colors;
extern std::array<int8_t, 88> monster_popup_y_offsets;

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
extern char _4E94D0_light_type;
extern char _4E94D2_light_type;
extern char _4E94D3_light_type;
extern std::array<unsigned int, 2> saveload_dlg_xs;
extern std::array<unsigned int, 2> saveload_dlg_ys;
extern std::array<unsigned int, 2> saveload_dlg_zs;
extern std::array<unsigned int, 2> saveload_dlg_ws;
extern std::array<const char *, 465> pTransitionStrings;  // 4EB080
extern std::array<int, 9> dword_4EC268;
extern std::array<int, 7> dword_4EC28C;
extern std::array<const char *, 25> pPlayerPortraitsNames;

extern std::array<std::array<unsigned char, 25>, 48> byte_4ECF08;
extern IndexedArray<std::array<unsigned char, 3>, SPEECH_FIRST, SPEECH_LAST> speechVariants;
extern IndexedArray<std::array<unsigned char, 5>, SPEECH_FIRST, SPEECH_LAST> expressionVariants;
extern std::array<int16_t, 4> pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing;
extern IndexedArray<PLAYER_SKILL_LEVEL, PLAYER_SKILL_FIRST, PLAYER_SKILL_LAST> skills_max_level;
extern IndexedArray<uint, PLAYER_SKILL_FIRST, PLAYER_SKILL_LAST> base_recovery_times_per_weapon_type;
extern std::array<IndexedArray<CLASS_SKILL, PLAYER_SKILL_FIRST, PLAYER_SKILL_LAST>, 9> pSkillAvailabilityPerClass;
extern IndexedArray<IndexedArray<PLAYER_SKILL_MASTERY, PLAYER_SKILL_FIRST, PLAYER_SKILL_LAST>, PLAYER_CLASS_FIRST, PLAYER_CLASS_LAST> skillMaxMasteryPerClass;
extern std::array<unsigned int, 2> pHiredNPCsIconsOffsetsX;
extern std::array<unsigned int, 2> pHiredNPCsIconsOffsetsY;
extern std::array<short, 28> word_4EE150;
extern int16_t word_4F0576[];

extern std::array<int, 32> guild_membership_flags;
extern std::array<int16_t, 49> word_4F0754;
extern std::array<std::pair<int16_t, ITEM_TYPE>, 27> _4F0882_evt_VAR_PlayerItemInHands_vals;
extern std::array<unsigned short, 6> pMaxLevelPerTrainingHallType;
extern std::array<int, 11> price_for_membership;
extern std::array<int16_t, 32> word_4F0F30;
extern std::array<int, 500> ai_array_detected_actor_dist;
extern std::array<int, 500> ai_array_detected_actor_ids;
extern std::array<int, 500> ai_near_actors_targets_pid;
extern int ai_arrays_size;
extern std::array<int, 500> ai_near_actors_distances;
extern std::array<unsigned int, 500> ai_near_actors_ids;
extern int dword_4FA9B0[];
extern int dword_4FA9B4[];

// extern char am_turn_not_finished;
// extern HWND dword_4FAA28; // idb

// extern std::array<unsigned int, 480> pSRZBufferLineOffsets;
// extern Texture_MM7 *dword_50640C[];
extern int
    books_page_number;  // number for page in books(номер страницы в книгах)
extern int
    books_primary_item_per_page;  // number primary item in book page(номер
                                  // начальной записи на странице)
extern int BtnDown_flag;                     // BtnDown_flag
extern int BtnUp_flag;                       // BtnUp_flag
extern int quick_spell_at_page;
extern char byte_506550;
extern int uLastPointedObjectID;
extern int dword_506980_uW;
extern int dword_506984_uZ;
extern int dword_506988_uY;
extern int dword_50698C_uX;
extern int KeyboardPageNum;
//extern int dword_506F1C;
extern GUIButton *pBtn_ZoomOut;  // idb
extern GUIButton *pBtn_ZoomIn;  // idb
extern unsigned int uGameUIFontShadow;
extern unsigned int uGameUIFontMain;
extern int dword_507B00_spell_info_to_draw_in_popup;
extern int dword_507BF0_is_there_popup_onscreen;
extern int awards_scroll_bar_created;
extern int dword_507CC0_activ_ch;
extern bool OpenedTelekinesis;
extern std::array<int, 50> dword_50B570;
extern std::array<int, 50> dword_50B638;
extern struct stru367 PortalFace;
extern std::array<int, 100> dword_50BC10;
extern std::array<int, 100> dword_50BDA0;
extern int enchantingActiveCharacter;
extern std::array<unsigned int, 5> pIconIDs_Turn;
extern unsigned int uIconID_TurnStop;
extern unsigned int uIconID_TurnHour;
extern int uIconID_CharacterFrame;  // idb
extern unsigned int uIconID_TurnStart;
extern int dword_50C994;
extern int dword_50C998_turnbased_icon_1A;
extern int uSpriteID_Spell11;  // idb
extern bool IsEnchantingInProgress; // 50C9A0 Indicates that inventory window is opened for enchant-like spell
extern int ItemEnchantmentTimer; // 50C9A8 Timer for enchanting animation for item in inventory
extern UIMessageType AfterEnchClickEventId; // 50C9D0 Event id that is pushed in queue after enchant item has been clicked
extern int AfterEnchClickEventSecondParam; // 50C9D4 Parameter passed for event "AfterEnchClickEventId"
extern int AfterEnchClickEventTimeout; // 50C9D8 Timer before event "AfterEnchClickEventId" is pushed in queue
// extern int dword_50C9E8; // idb
// extern int dword_50C9EC[]; // 50C9EC
extern int dword_50CDC8;
extern char pStartingMapName[32];  // idb
extern std::array<char, 54> party_has_equipment;

extern unsigned int uNumBlueFacesInBLVMinimap;
extern std::array<uint16_t, 50> pBlueFacesInBLVMinimapIDs;

extern std::array<class Image *, 14> party_buff_icons;
extern unsigned int uIconIdx_FlySpell;
extern unsigned int uIconIdx_WaterWalk;
extern GameTime _5773B8_event_timer;  // 5773B8

extern Actor *pDialogue_SpeakingActor;
extern DIALOGUE_TYPE uDialogueType;
extern signed int sDialogue_SpeakingActorNPC_ID;
extern int uCurrentHouse_Animation;


// extern int Party_Teleport_Z_Speed;
// extern int Party_Teleport_Cam_Pitch;
// extern int Party_Teleport_Cam_Yaw;
// extern int Party_Teleport_Z_Pos;
// extern int Party_Teleport_Y_Pos;
// extern int Party_Teleport_X_Pos;  // transition entry point

extern std::array<std::array<char, 100>, 6> byte_591180;  // idb
extern std::array<struct NPCData *, 7>
    HouseNPCData;  // 0this array size temporarily increased to 60 from 6 to
                   // work aroud house overflow
extern GUIButton *HouseNPCPortraitsButtonsList[6];
extern std::string branchless_dialogue_str;
extern int EvtTargetObj;
extern int _unused_5B5924_is_travel_ui_drawn;  // 005B5924

extern char *Party_Teleport_Map_Name;
extern int Party_Teleport_X_Pos;
extern int Party_Teleport_Y_Pos;
extern int Party_Teleport_Z_Pos;
extern int Party_Teleport_Cam_Yaw;
extern int Party_Teleport_Cam_Pitch;
extern int Party_Teleport_Z_Speed;
extern int Start_Party_Teleport_Flag;

extern int dword_5B65C4_cancelEventProcessing;
extern int MapsLongTimers_count;  // dword_5B65C8
extern int npcIdToDismissAfterDialogue;
// extern std::array<char, 777> byte_5C3427;
extern std::string game_ui_status_bar_event_string;
extern std::string game_ui_status_bar_string;
extern unsigned int game_ui_status_bar_event_string_time_left;
extern int bForceDrawFooter;
extern int _5C35C0_force_party_death;
extern int bDialogueUI_InitializeActor_NPC_ID;
extern std::string p2DEventsTXT_Raw;

extern int dword_5C35D4;
extern char byte_5C45AF[];
extern std::array<char, 10000> pTmpBuf3;
extern int ui_current_text_color;
extern int64_t qword_5C6DF0;
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

extern std::string pMonstersTXT_Raw;
extern std::string pMonsterPlacementTXT_Raw;
extern std::string pSpellsTXT_Raw;
extern char *pMapStatsTXT_Raw;
extern std::string pHostileTXT_Raw;
extern char *pPotionsTXT_Raw;
extern char *pPotionNotesTXT_Raw;

extern std::array<int, 777> _6807B8_level_decorations_ids;  // idb
extern int _6807E0_num_decorations_with_sounds_6807B8;
extern std::array<int, 3000> event_triggers;  // 6807E8
extern size_t num_event_triggers;             // 6836C8
extern std::array<int, 64> dword_69B010;
extern float flt_69B138_dist;
extern char byte_69BD41_unused;
extern char cMM7GameCDDriveLetter;  // idb
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
// extern char pMM7WindowClassName[]; // idb
// extern HINSTANCE hInstance; // idb
// extern char *pCmdLine;
// extern HWND hWnd; // idb
extern std::string pCurrentMapName;
extern unsigned int uLevelMapStatsID;

#define GAME_SETTINGS_0001 0x0001  // Skip AI next frame due to changing levels etc.
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
#define GAME_SETTINGS_LOADING_SAVEGAME_SKIP_RESPAWN 0x2000  // don't respawn the level we're loading because we're loading a saved game
#define GAME_SETTINGS_4000 0x4000  // initialisation state
extern int dword_6BE364_game_settings_1;  // GAME_SETTINGS_*

/** Recovery multiplier for non-combat actions, e.g. receiving fall damage, casting buffs,
 * and receiving damage from monsters. */
extern float debug_non_combat_recovery_mul;

/** Recovery multiplier for combat actions, e.g. hand-to-hand and ranged attacks and combat spells. */
extern float debug_combat_recovery_mul;

/** Speed multiplier for monsters in turn-based mode. It does affect actual move distance, but setting a high value
 * doesn't make monsters dart like crazy because monster speed is capped at 1000. Doh. */
extern float debug_turn_based_monster_movespeed_mul;

// 2.1333
extern float flt_debugrecmod3;

extern std::string s_SavedMapName;
extern char bNoNPCHiring;
extern std::array<int, 100> dword_720020_zvalues;
extern std::array<int, 299> dword_7201B0_zvalues;
extern std::array<int16_t, 104> odm_floor_face_vert_coord_Y;  // word_720F70
extern std::array<int16_t, 104> odm_floor_face_vert_coord_X;  // word_721040
extern Vec3i blv_prev_party_pos;
extern char *dword_722F10;  // idb
extern std::array<const char *, 513> pQuestTable;
extern char *dword_723718_autonote_related;  // idb
extern IndexedArray<const char *, ITEM_FIRST_MESSAGE_SCROLL, ITEM_LAST_MESSAGE_SCROLL> pScrolls;
extern int dword_7241C8;

extern std::string pAwardsTXT_Raw;
extern std::string pScrollsTXT_Raw;
extern std::string pMerchantsTXT_Raw;
extern IndexedArray<const char *, MERCHANT_PHRASE_COUNT> pMerchantsBuyPhrases;
extern IndexedArray<const char *, MERCHANT_PHRASE_COUNT> pMerchantsSellPhrases;
extern IndexedArray<const char *, MERCHANT_PHRASE_COUNT> pMerchantsRepairPhrases;
extern IndexedArray<const char *, MERCHANT_PHRASE_COUNT> pMerchantsIdentifyPhrases;
extern std::string pTransitionsTXT_Raw;
extern std::string pAutonoteTXT_Raw;
extern std::string pQuestsTXT_Raw;
extern std::vector<Vec3f> pTerrainNormals;
extern std::array<unsigned short, 128 * 128 * 2> pTerrainNormalIndices;
extern std::array<unsigned int, 128 * 128 * 2> pTerrainSomeOtherData;
extern int dword_A74C88;
extern int uPlayerCreationUI_ArrowAnim;
extern unsigned int uPlayerCreationUI_SelectedCharacter;
extern int dword_A74CDC;
extern int64_t _A750D8_player_speech_timer;  // qword_A750D8
extern int uSpeakingCharacter; // 0-based
extern char byte_AE5B91;
extern std::array<int, 32> dword_F1B430;
extern std::array<int, 4> player_levels;
extern std::array<int16_t, 6> weapons_Ypos;  // word_F8B158
extern int guild_membership_approved;
extern PLAYER_SKILL_MASTERY dword_F8B1B0_MasteryBeingTaught;
extern int gold_transaction_amount;  // F8B1B4
extern std::array<const char *, 4> pShopOptions;
extern int dword_F8B1E0;
extern int dword_F8B1E4;
extern std::string current_npc_text;  // F8B1E8
extern char dialogue_show_profession_details;
extern std::array<char, 777> byte_F8B1EF;

extern int bGameoverLoop;
