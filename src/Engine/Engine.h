#pragma once

#include <float.h>

#include <memory>
#include <string>
#include <climits>

#include "Engine/AssetsManager.h"
#include "Engine/ErrorHandling.h"
#include "Library/Logger/Logger.h"
#include "Engine/MM7.h"
#include "Engine/MapInfo.h"
#include "Engine/EngineIocContainer.h"
#include "Engine/mm7_data.h"

#include "Engine/Graphics/Polygon.h"
#include "Engine/Graphics/BSPModel.h"

#include "Io/KeyboardActionMapping.h"
#include "Io/KeyboardInputHandler.h"
#include "Io/Mouse.h"

#include "Application/GameConfig.h"
#include "Utility/DataPath.h"
#include "Utility/String.h"

class Nuklear;
using Io::KeyboardActionMapping;
using Io::KeyboardInputHandler;
using Io::Mouse;

void Engine_DeinitializeAndTerminate(int exitCode);

/*  320 */
enum class GAME_STATE {
    GAME_STATE_PLAYING = 0,
    GAME_FINISHED = 1,
    GAME_STATE_CHANGE_LOCATION = 2,
    GAME_STATE_LOADING_GAME = 3,
    GAME_STATE_NEWGAME_OUT_GAMEMENU = 4,
    GAME_STATE_5 = 5,
    GAME_STATE_STARTING_NEW_GAME = 6,
    GAME_STATE_GAME_QUITTING_TO_MAIN_MENU = 7,
    GAME_STATE_PARTY_DIED = 8,
    GAME_STATE_FINAL_WINDOW = 9,
    GAME_STATE_A = 10
};
using enum GAME_STATE;

extern GAME_STATE uGameState;

/*  105 */
#pragma pack(push, 1)
struct Game__StationaryLight {
    Vec3f vPosition;
    Vec3f vRGBColor;
    float flt_18;
};
#pragma pack(pop)

/*  108 */
#pragma pack(push, 1)
struct Game_stru0 {
    int field_0;
    uint8_t *ptr_4;
    int field_8;
    int field_C;
};
#pragma pack(pop)

/*  279 */
#pragma pack(push, 1)
// Game_stru1
struct Game_Bloodsplat {
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
    float radius;
};
#pragma pack(pop)

class Vis;
class ParticleEngine;
class CShow;
class GammaController;
struct ClippingFunctions;
struct stru10;

/*  104 */
#pragma pack(push, 1)
class Engine {
 public:
    explicit Engine(std::shared_ptr<GameConfig> config);
    virtual ~Engine();

    static void LogEngineBuildInfo();

    // void _44E904_gamma_saturation_adjust();
    // bool InitializeGammaController();
    void Initialize();
    bool PickMouse(float fPickDepth, unsigned int uMouseX, unsigned int uMouseY,
                   bool bOutline, struct Vis_SelectionFilter *sprite_filter,
                   struct Vis_SelectionFilter *face_filter);
    bool PickKeyboard(float pick_depth, bool bOutline, struct Vis_SelectionFilter *sprite_filter,
                      struct Vis_SelectionFilter *face_filter);
    void OnGameViewportClick();
    void OutlineSelection();
    int _44EC23_saturate_face_odm(struct Polygon *a2, int *a3, signed int a4);
    int _44ED0A_saturate_face_blv(struct BLVFace *a2, int *a3, signed int a4);
    bool AlterGamma_BLV(struct BLVFace *pFace, unsigned int *pColor);
    bool AlterGamma_ODM(struct ODMFace *pFace, unsigned int *pColor);
    bool draw_debug_outlines();
    bool _44EEA7();
    void PushStationaryLights(int a2);
    void StackPartyTorchLight();
    // void PrepareBloodsplats();
    void Deinitialize();
    void DrawParticles();
    void Draw();
    void DrawGUI();
    void ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
    void SecondaryInitialization();
    void _461103_load_level_sub();
    void DropHeldItem();
    bool MM7_Initialize();

    bool is_underwater = false;
    bool is_targeting = false;
    bool is_saturate_faces = false;
    bool is_forceredraw = false;
    bool is_fog = false; // keeps track of whether fog enabled in d3d

    inline bool IsTargetingMode() const { return is_targeting; }
    inline void SetTargetingMode(bool is_targeting) { this->is_targeting = is_targeting; }
    inline bool IsUnderwater() const { return is_underwater; }
    inline void SetUnderwater(bool is_underwater) { this->is_underwater = is_underwater; }
    inline bool IsSaturateFaces() const { return is_saturate_faces; }
    inline void SetSaturateFaces(bool is_saturate_faces) { this->is_saturate_faces = is_saturate_faces; }
    inline bool IsForceRedraw() const { return is_forceredraw; }
    inline void SetForceRedraw(bool is_forceredraw) { this->is_forceredraw = is_forceredraw; }
    inline bool IsFog() const { return is_fog; }
    inline void SetFog(bool is_fog) { this->is_fog = is_fog; } // fog off rather than on??

    std::shared_ptr<KeyboardInputHandler> GetKeyboardInputHandler() const {
        return keyboardInputHandler;
    }


    std::shared_ptr<GameConfig> config;
    Game__StationaryLight pStationaryLights[25];
    char field_2C0[1092];
    unsigned int uNumStationaryLights;
    Game_Bloodsplat pBloodsplats[20];
    int field_938;
    int field_93C;
    int field_940;
    int field_944;
    int field_948;
    int field_94C;
    int field_950;
    int field_954;
    int field_958;
    int field_95C;
    int field_960;
    int field_964;
    int field_968;
    int field_96C;
    int field_970;
    // Mouse *pMouse;
    int field_978;
    Game_stru0 stru_97C;
    char field_98C[1148];
    int uNumStationaryLights_in_pStationaryLightsStack;
    // unsigned int __depricated1;  // unsigned int bGammaControlInitialized;
    float fSaturation;
    // uint64_t __depricated2;  // uint64_t uSomeGammaStartTime;
    // uint64_t __depricated3;  // int64_t uSomeGammaDeltaTime;
    // void ThreadWard *pThreadWardInstance;
    // ParticleEngine *pParticleEngine;
    // Mouse *pMouseInstance;
    // Vis *pVisInstance;
    // struct SpellFxRenderer *spellfx;
    // Camera3D *pCamera3D;
    stru10 *pStru10Instance;
    // void stru11 *pStru11Instance;
    // void stru12 *pStru12Instance;
    // void CShow *pCShow;
    // Keyboard *pKeyboardInstance;
    // GammaController *pGammaController;
    // int field_E74;

    Logger *log = nullptr;
    BloodsplatContainer *bloodsplat_container = nullptr;
    DecalBuilder *decal_builder = nullptr;
    SpellFxRenderer *spell_fx_renedrer = nullptr;
    std::shared_ptr<Mouse> mouse = nullptr;
    std::shared_ptr<Nuklear> nuklear = nullptr;
    std::shared_ptr<ParticleEngine> particle_engine = nullptr;
    Vis *vis = nullptr;
    std::shared_ptr<KeyboardInputHandler> keyboardInputHandler = nullptr;
    std::shared_ptr<KeyboardActionMapping> keyboardActionMapping = nullptr;
};
#pragma pack(pop)

extern std::shared_ptr<Engine> engine;

/**
 * @offset 0x42FC15
 */
void PlayButtonClickSound();
void back_to_game();

void UpdateUserInput_and_MapSpecificStuff();
void PrepareWorld(unsigned int _0_box_loading_1_fullscreen);
void DoPrepareWorld(bool bLoading, int _1_fullscreen_loading_2_box);

void FinalInitialization();
bool CheckMM7CD(char c);

void MM6_Initialize();
void MM7Initialization();

void PrepareToLoadODM(bool bLoading, struct ODMRenderParams *a2);
void ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
void InitializeTurnBasedAnimations(void *);
unsigned int GetGravityStrength();
void GameUI_StatusBar_Update(bool force_hide = false);

void sub_44861E_set_texture(unsigned int uFaceCog, const char *pFilename);
void sub_44892E_set_faces_bit(int sCogNumber, FaceAttribute bit, int on);
void SetDecorationSprite(uint16_t uCog, bool bHide,
                         const char *pFileName);  // idb
void _494035_timed_effects__water_walking_damage__etc();

/**
 * Modify party health or mana based on party or players conditions/buffs.
 *
 * @offset 0x00493938.
 */
void RegeneratePartyHealthMana();
std::string GetReputationString(int reputation);
unsigned int _494820_training_time(unsigned int a1);
void LoadLevel_InitializeLevelStr();
void OnMapLeave();
void OnMapLoad();
void Level_LoadEvtAndStr(const std::string &pLevelName);
void ReleaseBranchlessDialogue();
bool _44100D_should_alter_right_panel();
void Transition_StopSound_Autosave(const char *pMapName,
                                   MapStartPoint point);  // sub_44987B idb

void OnTimer(int);
void TeleportToNWCDungeon();

