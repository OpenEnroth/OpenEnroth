#pragma once

#include <memory>
#include <string>
#include <vector>
#include <array>

#include "Application/GameConfig.h"

#include "Engine/Events/EventMap.h"
#include "Engine/MapEnums.h"
#include "Engine/TeleportPoint.h"
#include "Engine/mm7_data.h"
#include "Engine/Time.h"

#include "Utility/Memory/Blob.h"

#include "GUI/GUIMessageQueue.h"

namespace Io {
class Mouse;
class KeyboardInputHandler;
class KeyboardActionMapping;
} // namespace Io

struct Vis_PIDAndDepth;
struct Polygon;
class DecalBuilder;
class BloodsplatContainer;
class SpellFxRenderer;
class Nuklear;
class Vis;
class ParticleEngine;
struct ClippingFunctions;
struct stru10;
class Logger;
class GUIMessageQueue;
class GameResourceManager;
class StatusBar;

/*  320 */
enum class GameState {
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
using enum GameState;

extern GameState uGameState;

struct Game__StationaryLight {
    Vec3f vPosition;
    Vec3f vRGBColor;
    float flt_18;
};

struct Game_stru0 {
    int field_0;
    uint8_t *ptr_4;
    int field_8;
    int field_C;
};

struct Game_Bloodsplat {
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
    float radius;
};

struct PersistentVariables {
    std::array<unsigned char, 75> mapVars;
    std::array<unsigned char, 125> decorVars;
};

class Engine {
 public:
    explicit Engine(std::shared_ptr<GameConfig> config);
    virtual ~Engine();

    static void LogEngineBuildInfo();

    void Initialize();
    void PickMouse(float fPickDepth, unsigned int uMouseX, unsigned int uMouseY,
                   bool bOutline, struct Vis_SelectionFilter *sprite_filter,
                   struct Vis_SelectionFilter *face_filter);
    bool PickKeyboard(float pick_depth, bool bOutline, struct Vis_SelectionFilter *sprite_filter,
                      struct Vis_SelectionFilter *face_filter);

    /**
     * @offset 0x42213C
     */
    void onGameViewportClick();
    void OutlineSelection(const Vis_PIDAndDepth &selection);
    int _44EC23_saturate_face_odm(struct Polygon *a2, int *a3, signed int a4); // TODO(captainurist): drop?
    int _44ED0A_saturate_face_blv(struct BLVFace *a2, int *a3, signed int a4);
    bool draw_debug_outlines();
    void filterPickMouse();
    void StackPartyTorchLight();
    void Deinitialize();
    void DrawParticles();
    void Draw();
    void drawWorld();
    void drawHUD();
    void DrawGUI();
    void ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
    void SecondaryInitialization();
    void _461103_load_level_sub();
    void MM7_Initialize();

    inline bool IsTargetingMode() const { return is_targeting; }
    inline void SetTargetingMode(bool is_targeting) { this->is_targeting = is_targeting; }
    inline bool IsUnderwater() const { return is_underwater; }
    inline void SetUnderwater(bool is_underwater) { this->is_underwater = is_underwater; }
    inline bool IsSaturateFaces() const { return is_saturate_faces; }
    inline void SetSaturateFaces(bool is_saturate_faces) { this->is_saturate_faces = is_saturate_faces; }
    inline bool IsFog() const { return is_fog; }
    inline void SetFog(bool is_fog) { this->is_fog = is_fog; } // fog off rather than on??

    bool is_underwater = false;
    bool is_targeting = false;
    bool is_saturate_faces = false;
    bool is_fog = false; // keeps track of whether fog enabled in d3d

    std::shared_ptr<GameConfig> config;
    int uNumStationaryLights_in_pStationaryLightsStack;
    float fSaturation;
    stru10 *pStru10Instance;
    Logger *log = nullptr;
    BloodsplatContainer *bloodsplat_container = nullptr;
    DecalBuilder *decal_builder = nullptr;
    SpellFxRenderer *spell_fx_renedrer = nullptr;
    std::shared_ptr<Io::Mouse> mouse;
    std::shared_ptr<Nuklear> nuklear;
    std::shared_ptr<ParticleEngine> particle_engine;
    Vis *vis = nullptr;
    std::shared_ptr<Io::KeyboardInputHandler> keyboardInputHandler;
    std::shared_ptr<Io::KeyboardActionMapping> keyboardActionMapping;
    EventMap _globalEventMap;
    EventMap _localEventMap;
    std::vector<std::string> _levelStrings;
    PersistentVariables _persistentVariables;
    TeleportPoint _teleportPoint;

    std::unique_ptr<GUIMessageQueue> _messageQueue;
    std::unique_ptr<GameResourceManager> _gameResourceManager;
    std::unique_ptr<StatusBar> _statusBar;
};

extern Engine *engine;

/**
 * @offset 0x42FC15
 */
void PlayButtonClickSound();
void back_to_game();

void UpdateUserInput_and_MapSpecificStuff();
void PrepareWorld(unsigned int _0_box_loading_1_fullscreen);
void DoPrepareWorld(bool bLoading, int _1_fullscreen_loading_2_box);

void FinalInitialization();

void MM6_Initialize();
void MM7Initialization();

void PrepareToLoadODM(bool bLoading, struct ODMRenderParams *a2);
void InitializeTurnBasedAnimations(void *);
unsigned int GetGravityStrength();

/**
 * @offset 0x44861E
 */
void setTexture(unsigned int uFaceCog, const std::string &pFilename);

/**
 * @offset 0x44892E
 */
void setFacesBit(int sCogNumber, FaceAttribute bit, int on);

/**
 * @offset 0x44882F
 */
void setDecorationSprite(uint16_t uCog, bool bHide, const std::string &pFileName);  // idb
void _494035_timed_effects__water_walking_damage__etc();

/**
 * Modify party health or mana based on party or players conditions/buffs.
 *
 * @offset 0x493938.
 */
void RegeneratePartyHealthMana();
std::string GetReputationString(int reputation);

/**
 * @offset 0x494820
 */
GameTime timeUntilDawn();

/**
 * @offset 0x443E31
 */
void initLevelStrings(Blob &blob);
void Level_LoadEvtAndStr(const std::string &pLevelName);
bool _44100D_should_alter_right_panel();
void Transition_StopSound_Autosave(const std::string &pMapName, MapStartPoint point);  // sub_44987B idb

void TeleportToNWCDungeon();

