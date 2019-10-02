#pragma once

#include <memory>
#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/EngineConfig.h"
#include "Engine/EngineConfigFactory.h"
#include "Engine/ErrorHandling.h"
#include "Engine/Log.h"
#include "Engine/MM7.h"
#include "Engine/MapInfo.h"
#include "Engine/IocContainer.h"
#include "Engine/Strings.h"
#include "Engine/VectorTypes.h"
#include "Engine/mm7_data.h"

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

using Engine_::EngineConfig;
using Engine_::EngineConfigFactory;

/*  104 */
#pragma pack(push, 1)
struct Engine {
 public:
    Engine();
    virtual ~Engine();

    // void _44E904_gamma_saturation_adjust();
    // bool InitializeGammaController();
    inline bool Configure(std::shared_ptr<const EngineConfig> config) {
        this->config = config;
        return true;
    }

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
    bool AlterGamma_BLV(struct BLVFace *pFace, unsigned int *pColor);
    bool AlterGamma_ODM(struct ODMFace *pFace, unsigned int *pColor);
    bool draw_debug_outlines();
    bool _44EEA7();
    bool _44F07B();
    void PushStationaryLights(int a2);
    // void PrepareBloodsplats();
    void Deinitialize();
    void DrawParticles();
    void Draw();
    void DrawGUI();
    void ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
    void SecondaryInitialization();
    void _461103_load_level_sub();
    void DropHeldItem();
    bool MM7_Initialize(const std::string &mm7_path);

    inline bool IsUnderwater() const { return config->IsUnderwater(); }
    inline bool CanSaturateFaces() const { return config->CanSaturateFaces(); }  // this is for perception - move to party
    inline bool AllowSnow() const { return config->allow_snow; }
    inline void SetUnderwater(bool is_underwater) {
        EngineConfigFactory engineConfigFactory;
        auto new_config = engineConfigFactory.Clone(config);
        new_config->SetUnderwater(is_underwater);

        this->config = new_config;
    }
    inline void SetSaturateFaces(bool saturate) {
        EngineConfigFactory engineConfigFactory;
        auto new_config = engineConfigFactory.Clone(config);
        new_config->SetSaturateFaces(saturate);

        this->config = new_config;
    }
    inline void SetForceRedraw(bool redraw) {
        EngineConfigFactory engineConfigFactory;
        auto new_config = engineConfigFactory.Clone(config);
        new_config->SetForceRedraw(redraw);

        this->config = new_config;
    }
    inline void SetTargetingMode(bool is_targeting) {
        MutateConfig(
            [is_targeting](std::shared_ptr<EngineConfig> &cfg) {
                cfg->SetTargetingMode(is_targeting);
            });
    }
    inline void SetDebugWizardEye(bool wizard_eye) {
        MutateConfig(
            [wizard_eye](std::shared_ptr<EngineConfig> &cfg) {
                cfg->debug_wizard_eye = wizard_eye;
            });
    }
    inline void SetDebugAllMagic(bool all_magic) {
        MutateConfig(
            [all_magic](std::shared_ptr<EngineConfig> &cfg) {
                cfg->debug_all_magic = all_magic;
            });
    }
    inline void SetDebugShowFps(bool show_fps) {
        MutateConfig(
            [show_fps](std::shared_ptr<EngineConfig> &cfg) {
                cfg->show_fps = show_fps;
            });
    }
    inline void SetDebugShowPickedFace(bool show_picked_face) {
        MutateConfig(
            [show_picked_face](std::shared_ptr<EngineConfig> &cfg) {
                cfg->show_picked_face = show_picked_face;
            });
    }
    inline void SetDebugPortalOutlines(bool portal_outlines) {
        MutateConfig(
            [portal_outlines](std::shared_ptr<EngineConfig> &cfg) {
                cfg->debug_portal_outlines = portal_outlines;
            });
    }
    inline void SetDebugTurboSpeed(bool turbo_speed) {
        MutateConfig(
            [turbo_speed](std::shared_ptr<EngineConfig> &cfg) {
                cfg->debug_turbo_speed = turbo_speed;
            });
    }
    inline void SetSeasonsChange(bool seasons_change) {
        MutateConfig(
            [seasons_change](std::shared_ptr<EngineConfig> &cfg) {
                cfg->seasons_change = seasons_change;
            });
    }
    inline void SetAllowSnow(bool allow_snow) {
        MutateConfig(
            [allow_snow](std::shared_ptr<EngineConfig> &cfg) {
                cfg->allow_snow = allow_snow;
            });
    }
    inline void SetExtendedDrawDistance(bool extended_draw_distance) {
        MutateConfig(
            [extended_draw_distance](std::shared_ptr<EngineConfig> &cfg) {
                cfg->extended_draw_distance = extended_draw_distance;
            });
    }
    inline void SetNoActors(bool no_actors) {
        MutateConfig(
            [no_actors](std::shared_ptr<EngineConfig> &cfg) {
                cfg->no_actors = no_actors;
            });
    }
    inline void SetAllowLightmaps(bool allow_lightmaps) {
        MutateConfig(
            [allow_lightmaps](std::shared_ptr<EngineConfig> &cfg) {
                cfg->allow_lightmaps = allow_lightmaps;
            });
    }
    inline void SetDebugLightmapsDecals(bool debug_lightmaps_decals) {
        MutateConfig(
            [debug_lightmaps_decals](std::shared_ptr<EngineConfig> &cfg) {
                cfg->debug_lightmaps_decals = debug_lightmaps_decals;
            });
    }
    inline void SetDebugTerrain(bool debug_terrain) {
        MutateConfig(
            [debug_terrain](std::shared_ptr<EngineConfig> &cfg) {
                cfg->debug_terrain = debug_terrain;
            });
    }
    inline void ToggleAlwaysRun() {
        MutateConfig(
            [](std::shared_ptr<EngineConfig> &cfg) {
                cfg->ToggleAlwaysRun();
            });
    }
    inline void ToggleFlipOnExit() {
        MutateConfig(
            [](std::shared_ptr<EngineConfig> &cfg) {
                cfg->ToggleFlipOnExit();
            });
    }
    inline void ToggleWalkSound() {
        MutateConfig(
            [](std::shared_ptr<EngineConfig> &cfg) {
                cfg->ToggleWalkSound();
            });
    }
    inline void ToggleShowDamage() {
        MutateConfig(
            [](std::shared_ptr<EngineConfig> &cfg) {
                cfg->ToggleShowDamage();
            });
    }
    inline void ToggleBloodsplats() {
        MutateConfig(
            [](std::shared_ptr<EngineConfig> &cfg) {
                cfg->ToggleBloodsplats();
            });
    }

    inline void ToggleDebugTownPortal() {
        MutateConfig(
            [](std::shared_ptr<EngineConfig> &cfg) {
            cfg->ToggleDebugTownPortal();
        });
    }

    inline void ToggleDebugWizardEye() {
        MutateConfig(
            [](std::shared_ptr<EngineConfig> &cfg) {
            cfg->ToggleDebugWizardEye();
        });
    }

    inline void ToggleDebugAllMagic() {
        MutateConfig(
            [](std::shared_ptr<EngineConfig> &cfg) {
            cfg->ToggleDebugAllMagic();
        });
    }

    inline void ToggleDebugTerrain() {
        MutateConfig(
            [](std::shared_ptr<EngineConfig> &cfg) {
            cfg->ToggleDebugTerrain();
        });
    }

    inline void ToggleDebugLightmap() {
        MutateConfig(
            [](std::shared_ptr<EngineConfig> &cfg) {
            cfg->ToggleDebugLightmap();
        });
    }

    inline void ToggleDebugTurboSpeed() {
        MutateConfig(
            [](std::shared_ptr<EngineConfig> &cfg) {
            cfg->ToggleDebugTurboSpeed();
        });
    }

    inline void ToggleDebugNoActors() {
        MutateConfig(
            [](std::shared_ptr<EngineConfig> &cfg) {
            cfg->ToggleDebugNoActors();
        });
    }

    inline void ToggleDebugDrawDist() {
        MutateConfig(
            [](std::shared_ptr<EngineConfig> &cfg) {
            cfg->ToggleDebugDrawDist();
        });
    }

    inline void ToggleDebugSnow() {
        MutateConfig(
            [](std::shared_ptr<EngineConfig> &cfg) {
            cfg->ToggleDebugSnow();
        });
    }

    inline void ToggleDebugNoDamage() {
        MutateConfig(
            [](std::shared_ptr<EngineConfig> &cfg) {
            cfg->ToggleDebugNoDamage();
        });
    }

    inline void ToggleDebugPortalLines() {
        MutateConfig(
            [](std::shared_ptr<EngineConfig> &cfg) {
            cfg->ToggleDebugPortalLines();
        });
    }

    inline void ToggleDebugPickedFace() {
        MutateConfig(
            [](std::shared_ptr<EngineConfig> &cfg) {
            cfg->ToggleDebugPickedFace();
        });
    }

    inline void ToggleDebugShowFPS() {
        MutateConfig(
            [](std::shared_ptr<EngineConfig> &cfg) {
            cfg->ToggleDebugShowFPS();
        });
    }

    inline void ToggleDebugSeasonsChange() {
        MutateConfig(
            [](std::shared_ptr<EngineConfig> &cfg) {
            cfg->ToggleDebugSeasonsChange();
        });
    }

    inline void SetMusicLevel(int level) {
        MutateConfig(
            [level](std::shared_ptr<EngineConfig> &cfg) {
                int lvl = level;
                if (lvl < 0)
                    lvl = 0;
                if (lvl > 9)
                    lvl = 9;

                cfg->music_level = lvl;
            });
    }
    inline void SetSoundLevel(int level) {
        MutateConfig(
            [level](std::shared_ptr<EngineConfig> &cfg) {
                int lvl = level;
                if (lvl < 0)
                    lvl = 0;
                if (lvl > 9)
                    lvl = 9;

                cfg->sound_level = lvl;
            });
    }
    inline void SetVoiceLevel(int level) {
        MutateConfig(
            [level](std::shared_ptr<EngineConfig> &cfg) {
                int lvl = level;
                if (lvl < 0)
                    lvl = 0;
                if (lvl > 9)
                    lvl = 9;

                cfg->voice_level = lvl;
            });
    }
    inline void SetTurnSpeed(int turn_speed) {
        MutateConfig(
            [turn_speed](std::shared_ptr<EngineConfig> &cfg) {
                cfg->turn_speed = turn_speed;
            });
    }
    inline void MutateConfig(std::function<void(std::shared_ptr<EngineConfig> &)> mutator) {
        EngineConfigFactory engineConfigFactory;
        this->Configure(
            engineConfigFactory.Mutate(config, mutator));
    }


    std::shared_ptr<const EngineConfig> config;
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
    // unsigned __int64 __depricated2;  // unsigned __int64 uSomeGammaStartTime;
    // unsigned __int64 __depricated3;  // __int64 uSomeGammaDeltaTime;
    // void ThreadWard *pThreadWardInstance;
    // ParticleEngine *pParticleEngine;
    // Mouse *pMouseInstance;
    // LightmapBuilder *pLightmapBuilder;
    // Vis *pVisInstance;
    // struct SpellFxRenderer *spellfx;
    // IndoorCameraD3D *pIndoorCameraD3D;
    stru9 *pStru9Instance;
    stru10 *pStru10Instance;
    // void stru11 *pStru11Instance;
    // void stru12 *pStru12Instance;
    // void CShow *pCShow;
    // Keyboard *pKeyboardInstance;
    // GammaController *pGammaController;
    // int field_E74;

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

extern std::shared_ptr<Engine> engine;

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

enum CHARACTER_ATTRIBUTE_TYPE {
    CHARACTER_ATTRIBUTE_STRENGTH = 0,
    CHARACTER_ATTRIBUTE_INTELLIGENCE = 1,
    CHARACTER_ATTRIBUTE_WILLPOWER = 2,
    CHARACTER_ATTRIBUTE_ENDURANCE = 3,
    CHARACTER_ATTRIBUTE_ACCURACY = 4,
    CHARACTER_ATTRIBUTE_SPEED = 5,
    CHARACTER_ATTRIBUTE_LUCK = 6,
    CHARACTER_ATTRIBUTE_HEALTH = 7,
    CHARACTER_ATTRIBUTE_MANA = 8,
    CHARACTER_ATTRIBUTE_AC_BONUS = 9,

    CHARACTER_ATTRIBUTE_RESIST_FIRE = 10,
    CHARACTER_ATTRIBUTE_RESIST_AIR = 11,
    CHARACTER_ATTRIBUTE_RESIST_WATER = 12,
    CHARACTER_ATTRIBUTE_RESIST_EARTH = 13,
    CHARACTER_ATTRIBUTE_RESIST_MIND = 14,
    CHARACTER_ATTRIBUTE_RESIST_BODY = 15,

    CHARACTER_ATTRIBUTE_SKILL_ALCHEMY = 16,
    CHARACTER_ATTRIBUTE_SKILL_STEALING = 17,
    CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM = 18,
    CHARACTER_ATTRIBUTE_SKILL_ITEM_ID = 19,
    CHARACTER_ATTRIBUTE_SKILL_MONSTER_ID = 20,
    CHARACTER_ATTRIBUTE_SKILL_ARMSMASTER = 21,
    CHARACTER_ATTRIBUTE_SKILL_DODGE = 22,
    CHARACTER_ATTRIBUTE_SKILL_UNARMED = 23,

    CHARACTER_ATTRIBUTE_LEVEL = 24,
    CHARACTER_ATTRIBUTE_ATTACK = 25,
    CHARACTER_ATTRIBUTE_MELEE_DMG_BONUS = 26,
    CHARACTER_ATTRIBUTE_MELEE_DMG_MIN = 27,
    CHARACTER_ATTRIBUTE_MELEE_DMG_MAX = 28,
    CHARACTER_ATTRIBUTE_RANGED_ATTACK = 29,
    CHARACTER_ATTRIBUTE_RANGED_DMG_BONUS = 30,
    CHARACTER_ATTRIBUTE_RANGED_DMG_MIN = 31,
    CHARACTER_ATTRIBUTE_RANGED_DMG_MAX = 32,
    CHARACTER_ATTRIBUTE_RESIST_SPIRIT = 33,

    CHARACTER_ATTRIBUTE_SKILL_FIRE = 34,
    CHARACTER_ATTRIBUTE_SKILL_AIR = 35,
    CHARACTER_ATTRIBUTE_SKILL_WATER = 36,
    CHARACTER_ATTRIBUTE_SKILL_EARTH = 37,
    CHARACTER_ATTRIBUTE_SKILL_SPIRIT = 38,
    CHARACTER_ATTRIBUTE_SKILL_MIND = 39,
    CHARACTER_ATTRIBUTE_SKILL_BODY = 40,
    CHARACTER_ATTRIBUTE_SKILL_LIGHT = 41,
    CHARACTER_ATTRIBUTE_SKILL_DARK = 42,
    CHARACTER_ATTRIBUTE_SKILL_MEDITATION = 43,
    CHARACTER_ATTRIBUTE_SKILL_BOW = 44,
    CHARACTER_ATTRIBUTE_SKILL_SHIELD = 45,
    CHARACTER_ATTRIBUTE_SKILL_LEARNING = 46
};
