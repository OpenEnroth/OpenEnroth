#pragma once

#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/Configuration.h"
#include "Engine/ErrorHandling.h"
#include "Engine/Log.h"
#include "Engine/MM7.h"
#include "Engine/MapInfo.h"
#include "Engine/IocContainer.h"
#include "Engine/Strings.h"
#include "Engine/VectorTypes.h"
#include "Engine/mm7_data.h"

#include "Engine/Graphics/Configuration.h"
#include "Engine/Graphics/Polygon.h"

void Engine_DeinitializeAndTerminate(int exitCode);

bool FileExists(const char *fname);


/*  320 */
enum GAME_STATE {
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

/*  105 */
#pragma pack(push, 1)
struct Game__StationaryLight {
    Vec3_float_ vPosition;
    Vec3_float_ vRGBColor;
    float flt_18;
};
#pragma pack(pop)

/*  108 */
#pragma pack(push, 1)
struct Game_stru0 {
    int field_0;
    unsigned __int8 *ptr_4;
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
class LightmapBuilder;
class ParticleEngine;
class Mouse;
class Keyboard;
class ThreadWard;
class CShow;
class GammaController;
struct stru9;
struct stru10;

/*  104 */
#pragma pack(push, 1)
struct Engine {
    static Engine *Create(Engine_::Configuration *config);
    static void Destroy();

 protected:
    explicit Engine(Engine_::Configuration *config);
    virtual ~Engine();

 public:
    // void _44E904_gamma_saturation_adjust();
    // bool InitializeGammaController();
        void Initialize();
    bool PickMouse(float fPickDepth, unsigned int uMouseX, unsigned int uMouseY,
                   bool bOutline, struct Vis_SelectionFilter *sprite_filter,
                   struct Vis_SelectionFilter *face_filter);
    bool PickKeyboard(bool bOutline, struct Vis_SelectionFilter *sprite_filter,
                      struct Vis_SelectionFilter *face_filter);
    void OnGameViewportClick();
    void OutlineSelection();
    int _44EC23_saturate_face_odm(struct Polygon *a2, int *a3, signed int a4);
    int _44ED0A_saturate_face_blv(struct BLVFace *a2, int *a3, signed int a4);
    bool AlterGamma_BLV(struct BLVFace *pFace, signed int *pColor);
    bool AlterGamma_ODM(struct ODMFace *pFace, signed int *pColor);
    bool draw_debug_outlines();
    bool _44EEA7();
    bool _44F07B();
    void PushStationaryLights(int a2);
    void PrepareBloodsplats();
    void Deinitialize();
    void DrawParticles();
    void Draw();
    void ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
    void SecondaryInitialization();
    void _461103_load_level_sub();
    void DropHeldItem();

    Graphics::Configuration *ConfigureRender();

    inline bool IsUnderwater() const { return config->IsUnderwater(); }
    inline void SetUnderwater(bool is_underwater) { config->SetUnderwater(is_underwater); }

    //----- (0042EB6A) --------------------------------------------------------
    // struct SpellFxRenderer *GetSpellFxRenderer() {
    //    return this->spellfx;
    // }
    //----- (0042EB71) --------------------------------------------------------
    // struct IndoorCameraD3D *GetIndoorCamera() {
    //    return this->pIndoorCameraD3D;
    // }

    Engine_::Configuration *config = nullptr;
    // void ( ***vdestructor_ptr)(Game *, bool);
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
    int uNumBloodsplats;  // not used anymore??
    int field_E0C;  // to do with bloodsplats - not used anymore??
    __int64 field_E10;
    int uNumStationaryLights_in_pStationaryLightsStack;
    // unsigned int __depricated1;  // unsigned int bGammaControlInitialized;
    float fSaturation;
    // unsigned __int64 __depricated2;  // unsigned __int64 uSomeGammaStartTime;
    // unsigned __int64 __depricated3;  // __int64 uSomeGammaDeltaTime;
    // void ThreadWard *pThreadWardInstance;
    // ParticleEngine *pParticleEngine;
    // Mouse *pMouseInstance;
    // LightmapBuilder *pLightmapBuilder;
    // Vis *pVisInstance;
    // struct SpellFxRenderer *spellfx;
    // void *__depricated5;  // IndoorCameraD3D *pIndoorCameraD3D;
    stru9 *pStru9Instance;
    stru10 *pStru10Instance;
    // void stru11 *pStru11Instance;
    // void stru12 *pStru12Instance;
    // void CShow *pCShow;
    // Keyboard *pKeyboardInstance;
    // void *__depricated_4;  // GammaController *pGammaController;
    int field_E74;

    Log *log = nullptr;
    BloodsplatContainer *bloodsplat_container = nullptr;
    DecalBuilder *decal_builder = nullptr;
    SpellFxRenderer *spell_fx_renedrer = nullptr;
    LightmapBuilder *lightmap_builder = nullptr;
    Mouse *mouse = nullptr;
    Keyboard *keyboard = nullptr;
    ParticleEngine *particle_engine = nullptr;
    Vis *vis = nullptr;
};
#pragma pack(pop)

extern Engine *pEngine;

void sub_42FBDD();
void CloseWindowBackground();
void back_to_game();

void UpdateUserInput_and_MapSpecificStuff();
void PrepareWorld(unsigned int _0_box_loading_1_fullscreen);
void DoPrepareWorld(unsigned int bLoading, int _1_fullscreen_loading_2_box);

void FinalInitialization();
bool CheckMM7CD(char c);

void MM6_Initialize();
void MM7Initialization();

void PrepareToLoadODM(unsigned int bLoading, struct ODMRenderParams *a2);
void ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
void InitializeTurnBasedAnimations(void *);
unsigned int GetGravityStrength();
void GameUI_StatusBar_Update(bool force_hide = false);

void sub_44861E_set_texture(unsigned int uFaceCog, const char *pFilename);
void sub_44892E_set_faces_bit(int sCogNumber, int bit, int on);
void SetDecorationSprite(uint16_t uCog, bool bHide,
                         const char *pFileName);  // idb
void _494035_timed_effects__water_walking_damage__etc();
void _493938_regenerate();
void sub_491E3A();
String GetReputationString(int reputation);
unsigned int _494820_training_time(unsigned int a1);
void LoadLevel_InitializeLevelStr();
void OnMapLeave();
void OnMapLoad();
void Level_LoadEvtAndStr(const char *pLevelName);
void sub_4452BB();
bool _44100D_should_alter_right_panel();
void Transition_StopSound_Autosave(const char *pMapName,
                                   MapStartPoint point);  // sub_44987B idb

void OnTimer(int);
bool TeleportToNWCDungeon();

void SetDataPath(const char *data_path);
std::string MakeDataPath(const char *file_rel_path);
