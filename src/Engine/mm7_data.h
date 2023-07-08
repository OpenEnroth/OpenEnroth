#pragma once

#include <array>
#include <vector>
#include <string>
#include <utility>

#include "Engine/MM7.h"
#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/CharacterEnums.h"

#include "GUI/GUIEnums.h"
#include "GUI/GUIDialogues.h"

#include "Library/Color/Color.h"

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

extern float flt_4D86CC;
extern int dword_4D86D8;

extern float flt_4D84E8;

extern unsigned int uGammaPos;
extern std::array<int, 8> BtnTurnCoord;
extern std::array<int8_t, 88> monster_popup_y_offsets;

extern std::array<int, 6> dword_4E4560;
extern std::array<int, 6> dword_4E4578;
extern std::array<int, 6> dword_4E4590;
extern std::array<int, 6> dword_4E45A8;
extern std::array<float, 10> flt_4E4A80;

extern std::array<std::array<int, 6>, 6> pNPCPortraits_x;  // 004E5E50
extern std::array<std::array<int, 6>, 6> pNPCPortraits_y;  // 004E5EE0
extern std::array<const char *, 11> pHouse_ExitPictures;
extern char _4E94D0_light_type;
extern char _4E94D2_light_type;
extern char _4E94D3_light_type;
extern std::array<const char *, 25> pPlayerPortraitsNames;

extern std::array<std::array<unsigned char, 25>, 48> byte_4ECF08;
extern IndexedArray<std::array<unsigned char, 3>, SPEECH_FIRST, SPEECH_LAST> speechVariants;
extern IndexedArray<std::array<unsigned char, 5>, SPEECH_FIRST, SPEECH_LAST> expressionVariants;
extern std::array<int16_t, 4> pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing;
extern IndexedArray<int, CHARACTER_SKILL_FIRST, CHARACTER_SKILL_LAST> skills_max_level;
extern IndexedArray<uint, CHARACTER_SKILL_FIRST, CHARACTER_SKILL_LAST> base_recovery_times_per_weapon_type;
extern std::array<IndexedArray<ClassSkillAffinity, CHARACTER_SKILL_FIRST, CHARACTER_SKILL_LAST>, 9> pSkillAvailabilityPerClass;
extern IndexedArray<IndexedArray<CharacterSkillMastery, CHARACTER_SKILL_FIRST, CHARACTER_SKILL_LAST>, PLAYER_CLASS_FIRST, PLAYER_CLASS_LAST> skillMaxMasteryPerClass;

extern std::array<int, 500> ai_near_actors_targets_pid;
extern std::array<unsigned int, 500> ai_near_actors_ids;
extern int ai_arrays_size;

extern int uLastPointedObjectID;
extern int KeyboardPageNum;
extern Color uGameUIFontShadow;
extern Color uGameUIFontMain;
extern int dword_507B00_spell_info_to_draw_in_popup;
extern int dword_507CC0_activ_ch;
extern bool OpenedTelekinesis;
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

extern unsigned int uNumBlueFacesInBLVMinimap;
extern std::array<uint16_t, 50> pBlueFacesInBLVMinimapIDs;

extern std::array<class GraphicsImage *, 14> party_buff_icons;
extern unsigned int uIconIdx_FlySpell;
extern unsigned int uIconIdx_WaterWalk;

extern Actor *pDialogue_SpeakingActor;
extern DIALOGUE_TYPE uDialogueType;
extern signed int sDialogue_SpeakingActorNPC_ID;
extern int uCurrentHouse_Animation;

extern std::string branchless_dialogue_str;

extern std::string Party_Teleport_Map_Name;
extern int Party_Teleport_X_Pos;
extern int Party_Teleport_Y_Pos;
extern int Party_Teleport_Z_Pos;
extern int Party_Teleport_Cam_Yaw;
extern int Party_Teleport_Cam_Pitch;
extern int Party_Teleport_Z_Speed;
extern int Start_Party_Teleport_Flag;

extern int dword_5B65C4_cancelEventProcessing;
extern int npcIdToDismissAfterDialogue;
extern std::string game_ui_status_bar_event_string;
extern std::string game_ui_status_bar_string;
extern unsigned int game_ui_status_bar_event_string_time_left;
extern int bForceDrawFooter;
extern int _5C35C0_force_party_death;
extern int bDialogueUI_InitializeActor_NPC_ID;

extern std::array<char, 10000> pTmpBuf3;
extern int64_t qword_5C6DF0;
extern struct FactionTable *pFactionTable;

extern int uDefaultTravelTime_ByFoot;
extern int day_attrib;
extern int day_fogrange_1;
extern int day_fogrange_2;
extern struct TileTable *pTileTable;              // idb
extern int dword_6BE13C_uCurrentlyLoadedLocationID;
extern float fWalkSpeedMultiplier;
extern float fBackwardWalkSpeedMultiplier;
extern float fTurnSpeedMultiplier;
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
extern Vec3i blv_prev_party_pos;

extern std::vector<Vec3f> pTerrainNormals;
extern std::array<unsigned short, 128 * 128 * 2> pTerrainNormalIndices;
extern std::array<unsigned int, 128 * 128 * 2> pTerrainSomeOtherData;
extern unsigned int uPlayerCreationUI_SelectedCharacter;
extern int currentAddressingAwardBit;
extern int guild_membership_approved;
extern CharacterSkillMastery dword_F8B1B0_MasteryBeingTaught;
extern int gold_transaction_amount;  // F8B1B4
extern std::string current_npc_text;  // F8B1E8
extern char dialogue_show_profession_details;

extern int bGameoverLoop;
