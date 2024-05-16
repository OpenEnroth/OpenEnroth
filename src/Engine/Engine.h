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
#include "Engine/Time/Time.h"

#include "Utility/Memory/Blob.h"

namespace Io {
class Mouse;
class KeyboardInputHandler;
class KeyboardActionMapping;
} // namespace Io

struct BLVFace;
struct Vis_PIDAndDepth;
struct Vis_SelectionFilter;
struct Polygon;
struct DecalBuilder;
struct BloodsplatContainer;
struct SpellFxRenderer;
class Nuklear;
class Vis;
class ParticleEngine;
struct ClippingFunctions;
struct stru10;
class GUIMessageQueue;
class GameResourceManager;
class StatusBar;
class EngineCallObserver;
struct IndoorLocation;
struct OutdoorLocation;
struct LightsStack_StationaryLight_;
struct LightsStack_MobileLight_;
class OverlaySystem;

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

struct PersistentVariables {
    std::array<unsigned char, 75> mapVars;
    std::array<unsigned char, 125> decorVars;
};

class Engine {
 public:
    explicit Engine(std::shared_ptr<GameConfig> config, OverlaySystem &overlaySystem);
    virtual ~Engine();

    static void LogEngineBuildInfo();

    void Initialize();
    Vis_PIDAndDepth PickMouse(float fPickDepth, unsigned int uMouseX, unsigned int uMouseY,
                              Vis_SelectionFilter *sprite_filter, Vis_SelectionFilter *face_filter);
    Vis_PIDAndDepth PickKeyboard(float pick_depth, Vis_SelectionFilter *sprite_filter, Vis_SelectionFilter *face_filter);

    Vis_PIDAndDepth PickMouseInfoPopup();
    Vis_PIDAndDepth PickMouseTarget();
    Vis_PIDAndDepth PickMouseNormal();

    /**
     * @offset 0x42213C
     */
    void onGameViewportClick();
    int _44ED0A_saturate_face_blv(BLVFace *a2, int *a3, signed int a4);
    bool draw_debug_outlines();
    void StackPartyTorchLight();
    void DrawParticles();
    void Draw();
    void drawWorld();
    void drawHUD();
    void drawOverlay();
    void DrawGUI();
    void ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows();
    void SecondaryInitialization();
    void _461103_load_level_sub();
    void MM7_Initialize();

    inline bool IsUnderwater() const { return is_underwater; }
    inline void SetUnderwater(bool is_underwater) { this->is_underwater = is_underwater; }
    inline bool IsSaturateFaces() const { return is_saturate_faces; }
    inline void SetSaturateFaces(bool is_saturate_faces) { this->is_saturate_faces = is_saturate_faces; }
    inline bool IsFog() const { return is_fog; }
    inline void SetFog(bool is_fog) { this->is_fog = is_fog; } // fog off rather than on??

    void toggleOverlays();

    bool is_underwater = false;
    bool is_saturate_faces = false;
    bool is_fog = false; // keeps track of whether fog enabled in d3d

    std::shared_ptr<GameConfig> config;
    int uNumStationaryLights_in_pStationaryLightsStack;
    float fSaturation;
    stru10 *pStru10Instance;
    BloodsplatContainer *bloodsplat_container = nullptr;
    DecalBuilder *decal_builder = nullptr;
    SpellFxRenderer *spell_fx_renedrer = nullptr;
    EngineCallObserver *callObserver = nullptr;
    std::shared_ptr<Io::Mouse> mouse;
    std::shared_ptr<ParticleEngine> particle_engine;
    Vis *vis = nullptr;
    std::shared_ptr<Io::KeyboardInputHandler> keyboardInputHandler;
    std::shared_ptr<Io::KeyboardActionMapping> keyboardActionMapping;
    EventMap _globalEventMap;
    EventMap _localEventMap;
    std::vector<std::string> _levelStrings;
    PersistentVariables _persistentVariables;
    MapId _currentLoadedMapId = MAP_INVALID;
    MapId _transitionMapId = MAP_INVALID;
    TeleportPoint _teleportPoint;
    OverlaySystem &_overlaySystem;

    std::unique_ptr<GUIMessageQueue> _messageQueue;
    std::unique_ptr<GameResourceManager> _gameResourceManager;
    std::unique_ptr<StatusBar> _statusBar;
    std::unique_ptr<IndoorLocation> _indoor;
    std::unique_ptr<OutdoorLocation> _outdoor;
    std::unique_ptr<LightsStack_StationaryLight_> _stationaryLights;
    std::unique_ptr<LightsStack_MobileLight_> _mobileLights;
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

void InitializeTurnBasedAnimations(void *);
unsigned int GetGravityStrength();

/**
 * @offset 0x44861E
 */
void setTexture(unsigned int uFaceCog, std::string_view pFilename);

/**
 * @offset 0x44892E
 */
void setFacesBit(int sCogNumber, FaceAttribute bit, int on);

/**
 * @offset 0x44882F
 */
void setDecorationSprite(uint16_t uCog, bool bHide, std::string_view pFileName);  // idb
void _494035_timed_effects__water_walking_damage__etc(Duration dt);
void maybeWakeSoloSurvivor();
void updatePartyDeathState();

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
Duration timeUntilDawn();

/**
 * @offset 0x443E31
 */
void initLevelStrings(const Blob &blob);
void loadMapEventsAndStrings(MapId mapid);
bool _44100D_should_alter_right_panel();
void Transition_StopSound_Autosave(std::string_view pMapName, MapStartPoint point);  // sub_44987B idb

void TeleportToNWCDungeon();

