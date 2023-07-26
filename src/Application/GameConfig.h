#pragma once

#include <string>
#include <algorithm>

#include "Library/Config/Config.h"
#include "Engine/Graphics/RenderEnums.h"
#include "Io/Key.h"
#include "Platform/PlatformEnums.h"

#ifdef __ANDROID__
#define ConfigRenderer RENDERER_OPENGL_ES
#define ConfigWindowMode WINDOW_MODE_FULLSCREEN
#else
#define ConfigRenderer RENDERER_OPENGL
#define ConfigWindowMode WINDOW_MODE_WINDOWED
#endif

MM_DECLARE_SERIALIZATION_FUNCTIONS(PlatformWindowMode)
MM_DECLARE_SERIALIZATION_FUNCTIONS(PlatformLogLevel)

class GameConfig : public Config {
 public:
    GameConfig();
    ~GameConfig();

    void resetForTest();

    using Bool = ConfigEntry<bool>;
    using Int = ConfigEntry<int>;
    using Float = ConfigEntry<float>;
    using String = ConfigEntry<std::string>;
    using Key = ConfigEntry<PlatformKey>;

    class Debug : public ConfigSection {
     public:
        explicit Debug(GameConfig *config) : ConfigSection(config, "debug") {}

        Bool AllMagic = {this, "all_magic", false,
                         "Enable all available spells for each character in spellbook bypassing all class restrictions. "
                         "Currently also all skills will behave like they are on GM level."};

        Bool InfiniteFood = {this, "infinite_food", false, "Enable unlimited food, using food won't spend it."};
        Bool InfiniteGold = {this, "infinite_gold", false, "Enable unlimited gold, paying in shops won't spend it."};

        Bool LightmapDecals = {this, "lightmap_decals", false, "Draw lightmap and decals outlines."};

        Bool PortalOutlines = {this, "portal_outlines", false, "Draw BLV portal outlines."};
        Bool Terrain = {this, "terrain", false, "Draw terrain as wireframe."};

        Bool TownPortal = {this, "town_portal", false,
                               "Make all game locations reachable via town portal spell without requiring to visit them first."};

        Bool TurboSpeed = {this, "turbo_speed", false,
                           "Increase party movement speed by 12x. Most likely you want to use that option with "
                           "no_damage option enabled as collision physics often will shoot you in the air."};

        Bool WizardEye = {this, "wizard_eye", false, "Activate wizard eye spell that never expires."};

        Bool ShowFPS = {this, "show_fps", false, "Show debug HUD with FPS and other debug information."};

        Bool ShowPickedFace = {this, "show_picked_face", false,
                               "Face pointed with mouse will flash with red for buildings or green for dungeons."};

        Bool NoIntro = {this, "no_intro", false, "Skip intro movie on startup."};

        Bool NoLogo = {this, "no_logo", false, "Skip 3do logo on startup."};

        Bool NoSound = {this, "no_sound", false, "Don't play any sounds. Currently in-house movies are not affected."};

        Bool NoVideo = {this, "no_video", false, "Don't play any movies."};

        Bool NoActors = {this, "no_actors", false, "Disable all actors."};

        Bool NoDamage = {this, "no_damage", false, "Disable all incoming damage to party."};

        Bool NoDecorations = {this, "no_decorations", false, "Disable all decorations."};

        Bool NoMargaret = {this, "no_margareth", false, "Disable Margaret's tour messages on Emerald Island."};

        ConfigEntry<PlatformLogLevel> LogLevel = {this, "log_level", LOG_ERROR,
                                                  "Default log level. One of 'verbose', 'debug', 'info', 'warning', 'error' and 'critical'."};

        Int TraceFrameTimeMs = {this, "trace_frame_time_ms", 50, &ValidateFrameTime,
                                "Number of milliseconds per frame when recording game traces."};

        Bool FullMonsterID = { this, "full_monster_id", false, "Full monster info on popup." };

     private:
        static int ValidateFrameTime(int frameTime) {
            return std::clamp(frameTime, 1, 1000);
        }
    };

    Debug debug{ this };

    class Gameplay : public ConfigSection {
     public:
        explicit Gameplay(GameConfig *config): ConfigSection(config, "gameplay") {}

        Bool AlternativeConditionPriorities = {this, "alternative_condition_priorities", true,
                                               "Use condition priorities from Grayface patches (e.g. Zombie has the lowest priority)."};

        Int ArtifactLimit = {this, "artifact_limit", 13, &ValidateArtifactLimit,
                             "Max number of artifacts that the game can generate as loot in any one playthrough. Use 0 for unlimited."};

        Int ChestTryPlaceItems = {this, "chest_try_place_items", 2,
                                  "Fix problems with item loss in high-level chests. "
                                  "Use 0 for vanilla behaviour, items that don't fit will be lost. "
                                  "Use 1 to try to place items that didn't fit every time the chest is opened again. "
                                  "Use 2 to try to place items that didn't fit every time an item is picked up from the chest."};

        Int FloorChecksEps = {this, "floor_checks_eps", 3, &ValidateFloorChecksEps,
                              "Maximum allowed slack for point-inside-a-polygon checks when calculating floor z level. "
                              "This is needed because there are actual holes in level geometry sometimes, up to several units wide."};

        Int Gravity = {this, "gravity", 5, "Gravity strength, the higher the more gravity, 0 disables gravity completely."};

        Float KeyboardInteractionDepth = {this, "keyboard_interaction_depth", 512.0f, &ValidateInteractionDepth,
                                          "Maximum range for item pickup / opening chests / activating levers / etc "
                                          "with a keyboard (by pressing the interaction key, see keybindings.event_trigger)."};

        Float MouseInteractionDepth = {this, "mouse_interaction_depth", 512.0f, &ValidateInteractionDepth,
                                       "Maximum range for item pickup / opening chests / activating levers / etc with a mouse."};

        Int MinRecoveryMelee = {this, "minimum_recovery_melee", 30, &ValidateRecovery,
                                "Minimum recovery time for melee weapons. Was 30 in vanilla."};

        Int MinRecoveryRanged = {this, "minimum_recovery_ranged", 5, &ValidateRecovery,
                                 "Minimum recovery time for ranged weapons. Was 0 in vanilla, 5 in GrayFace patches."};

        Int MinRecoveryBlasters = {this, "minimum_recovery_blasters", 5, &ValidateRecovery,
                                   "Minimum recovery time for blasters. Was 0 in vanilla, 5 in Grayface patches"};

        Int MaxFlightHeight = {this, "max_flight_height", 4000, &ValidateMaxFlightHeight,
                               "Maximum height for the fly spell."};

        Float MouseInfoDepthIndoor = {this, "mouse_info_depth_indoor", 16192.0f, &ValidateInteractionDepth,
                                      "Maximum range at which right clicking on a monster produces a popup indoors. "
                                      "Also this is the max range for the souldrinker spell indoors."};
        Float MouseInfoDepthOutdoor = {this, "mouse_info_depth_outdoor", 12800.0f, &ValidateInteractionDepth,
                                       "Maximum range at which right clicking on a monster produces a popup outdoors. "
                                       "Default value is 12800 = 25 * 512, 25 map cells. "
                                       "Also this is the max range for the souldrinker spell outdoors."};

        Int NewGameFood = {this, "new_game_food", 7, "Starting food."};
        Int NewGameGold = {this, "new_game_gold", 200, "Starting gold."};
        String StartingMap = String(this, "starting_map", "out01.odm", "New Game starting map.");

        Int PartyEyeLevel = {this, "party_eye_level", 160, "Party eye level."};
        Int PartyHeight = {this, "party_height", 192, "Party height."};
        Int PartyWalkSpeed = {this, "party_walk_speed", 384, "Party walk speed."};

        Float RangedAttackDepth = {this, "ranged_attack_depth", 5120.0f, &ValidateRangedAttackDepth,
                                   "Max depth for ranged attacks and ranged spells. "
                                   "It's impossible to target monsters that are further away than this value. "
                                   "This is also the depth at which status bar tips are displayed on mouse over."};

        Float AoeDamageDistance = {this, "aoe_damage_distance", 512.0f, &ValidateAoeDistance,
                                   "Distance from point of impact of harmful AOE spell. "
                                   "Characters and monsters will suffer damage if they are close to point of impact by this value."};

        Float ShrinkRayAoeDistance = {this, "shrink_ray_aoe_distance", 256.0f, &ValidateAoeDistance,
                                      "Distance from point of impact of Shrinking Ray cast at GM mastery. "
                                      "Monsters will be affected by this spell if they are close to point of impact by this value."};

        Bool ShowUndentifiedItem = {this, "show_unidentified_item", true,
                                    "Show unidentified items with a green tint in inventory. "
                                    "If not set, vanilla behavior will be used with green tint applied in shops only."};

        Bool TreatClubAsMace = {this, "treat_club_as_mace", false,
                                "Treat clubs as maces. "
                                "In vanilla clubs are using a separate hidden skill and can be equipped without learning the mace skill."};

        Bool FixWaterWalkManaDrain = {this, "fix_water_walk_mana_drain", true,
                                      "Change water walk mana drain interval to 20 minutes. "
                                      "Spell description says that mana is drained every 20 minutes, but in vanilla, it was every 5 minutes."};

        Float SpellFailureRecoveryMod = {this, "spell_failure_recovery_mod", 0.5f, &ValidateSpellFailureRecoveryMod,
                                         "Recovery time modifier when spell casting ended in failure for the reason where spell cannot be cast at all in current context. "
                                         "Context include situation where outdoor spell is casted indoor or targeted spell is casted with no characters on screen."};

     private:
        static int ValidateMaxFlightHeight(int max_flight_height) {
            if (max_flight_height <= 0 || max_flight_height > 16192)
                return 4000;

            return max_flight_height;
        }
        static int ValidateArtifactLimit(int artifact_limit) {
            if (artifact_limit < 0)
                return 0;

            return artifact_limit;
        }
        static float ValidateInteractionDepth(float depth) {
            return std::clamp(depth, 64.0f, 16192.0f);
        }
        static float ValidateRangedAttackDepth(float depth) {
            return std::clamp(depth, 64.0f, 16192.0f);
        }
        static float ValidateAoeDistance(float depth) {
            return std::clamp(depth, 64.0f, 16192.0f);
        }
        static float ValidateSpellFailureRecoveryMod(float mod) {
            return std::clamp(mod, 0.0f, 1.0f);
        }
        static int ValidateFloorChecksEps(int eps) {
            return std::clamp(eps, 0, 10);
        }
        static int ValidateRecovery(int recovery) {
            if (recovery < 0)
                return 0;

            return recovery;
        }
    };

    class Gamepad : public ConfigSection {
     public:
        explicit Gamepad(GameConfig *config) : ConfigSection(config, "gamepad") {}

        Key AlwaysRun = {this, "always_run", PlatformKey::KEY_GAMEPAD_L3, "Always run toggle key."};
        Key Attack = {this, "attack", PlatformKey::KEY_GAMEPAD_L1, "Attack key."};
        Key AutoNotes = {this, "auto_notes", PlatformKey::KEY_GAMEPAD_UP, "Open autonotes key."};
        Key Backward = {this, "backward", PlatformKey::KEY_GAMEPAD_LEFTSTICK_DOWN, "Walk backwards key."};
        Key Cast = {this, "cast", PlatformKey::KEY_GAMEPAD_DOWN, "Cast a spell from spellbook key."};
        Key CastReady = {this, "cast_ready", PlatformKey::KEY_GAMEPAD_R1, "Cast a quick spell key."};
        Key CenterView = {this, "center_view", PlatformKey::KEY_NONE, "Center view key."};
        Key CharCycle = {this, "char_cycle", PlatformKey::KEY_NONE, "Switch between characters key."};
        Key Combat = {this, "combat", PlatformKey::KEY_GAMEPAD_START, "Switch between realtime and turn-based modes key."};
        Key EventTrigger = {this, "event_trigger", PlatformKey::KEY_GAMEPAD_A, "Interaction key."};
        Key FlyDown = {this, "fly_down", PlatformKey::KEY_GAMEPAD_L2, "Fly down key."};
        Key FlyUp = {this, "fly_up", PlatformKey::KEY_GAMEPAD_R2, "Fly up key."};
        Key Forward = {this, "forward", PlatformKey::KEY_GAMEPAD_LEFTSTICK_UP, "Move forward key."};
        Key Jump = {this, "jump", PlatformKey::KEY_GAMEPAD_Y, "Jump key."};
        Key Land = {this, "land", PlatformKey::KEY_GAMEPAD_R3, "Land key."};
        Key Left = {this, "left", PlatformKey::KEY_GAMEPAD_RIGHTSTICK_LEFT, "Turn left key."};
        Key LookDown = {this, "look_down", PlatformKey::KEY_GAMEPAD_RIGHTSTICK_DOWN, "Look down key."};
        Key LookUp = {this, "look_up", PlatformKey::KEY_GAMEPAD_RIGHTSTICK_UP, "Look up key."};
        Key MapBook = {this, "map_book", PlatformKey::KEY_GAMEPAD_LEFT, "Open map key."};
        Key Pass = {this, "pass", PlatformKey::KEY_GAMEPAD_GUIDE, "Pass turn key"};
        Key Quest = {this, "quest", PlatformKey::KEY_GAMEPAD_RIGHT, "Open quest book key"};
        Key QuickReference = {this, "quick_reference", PlatformKey::KEY_NONE, "Open quick reference menu"};
        Key Rest = {this, "rest", PlatformKey::KEY_GAMEPAD_BACK, "Rest key."};
        Key Right = {this, "right", PlatformKey::KEY_GAMEPAD_RIGHTSTICK_RIGHT, "Turn right key."};
        Key StepLeft = {this, "step_left", PlatformKey::KEY_GAMEPAD_LEFTSTICK_LEFT, "Strafe left key."};
        Key StepRight = {this, "step_right", PlatformKey::KEY_GAMEPAD_LEFTSTICK_RIGHT, "Strafe right key."};
        Key TimeCalendar = {this, "time_calendar", PlatformKey::KEY_NONE, "Open calendar key."};
        Key Yell = {this, "yell", PlatformKey::KEY_GAMEPAD_X, "Yell key."};
        Key ZoomIn = {this, "zoom_in", PlatformKey::KEY_NONE, "Zoom in automap key."};
        Key ZoomOut = {this, "zoom_out", PlatformKey::KEY_NONE, "Zoom out automap key."};
        Key QuickSave = {this, "quick_save", PlatformKey::KEY_NONE, "Quick save key"};
        Key QuickLoad = {this, "quick_load", PlatformKey::KEY_NONE, "Quick load key"};
        Key History = {this, "history", PlatformKey::KEY_NONE, "History book key"};
        Key Stats = {this, "stats", PlatformKey::KEY_GAMEPAD_A, "Stats tab key"};
        Key Skills = {this, "skills", PlatformKey::KEY_GAMEPAD_X, "Skills tab key"};
        Key Inventory = {this, "inventory", PlatformKey::KEY_GAMEPAD_Y, "Inventory tab key"};
        Key Awards = {this, "awards", PlatformKey::KEY_GAMEPAD_L1, "Stats tab key"};
        Key NewGame = {this, "new_game", PlatformKey::KEY_GAMEPAD_A, "New Game menu key"};
        Key SaveGame = {this, "save_game", PlatformKey::KEY_GAMEPAD_Y, "Save Game menu key"};
        Key LoadGame = {this, "load_game", PlatformKey::KEY_GAMEPAD_X, "Load Game menu key"};
        Key ExitGame = {this, "exit_game", PlatformKey::KEY_GAMEPAD_B, "Exit Game key"};
        Key ReturnToGame = {this, "return_to_game", PlatformKey::KEY_GAMEPAD_B, "Return to Game mode key"};
        Key Controls = {this, "controls", PlatformKey::KEY_GAMEPAD_L1, "Controls change menu key"};
        Key Options = {this, "options", PlatformKey::KEY_GAMEPAD_R1, "Options menu key"};
        Key Credits = {this, "credits", PlatformKey::KEY_GAMEPAD_Y, "Credits menu key"};
        Key Clear = {this, "clear", PlatformKey::KEY_GAMEPAD_Y, "Clear button in New Party Creation menu"};
        Key Return = {this, "return", PlatformKey::KEY_GAMEPAD_A, "Ok button in New Party Creation menu"};
        Key Minus = {this, "minus", PlatformKey::KEY_GAMEPAD_L1, "Minus button in New Party Creation menu"};
        Key Plus = {this, "plus", PlatformKey::KEY_GAMEPAD_R1, "Plus button in New Party Creation menu"};
        Key Yes = {this, "yes", PlatformKey::KEY_GAMEPAD_A, "Yes answer key"};
        Key No = {this, "no", PlatformKey::KEY_GAMEPAD_B, "No answer key"};
        Key Rest8Hours = {this, "rest_8_hours", PlatformKey::KEY_GAMEPAD_BACK, "Rest for 8 hours key in Rest menu"};
        Key WaitTillDawn = {this, "wait_till_dawn", PlatformKey::KEY_GAMEPAD_Y, "Wait till dawn key in Rest menu"};
        Key WaitHour = {this, "wait_hour", PlatformKey::KEY_GAMEPAD_X, "Wait hour in Rest menu"};
        Key Wait5Minutes = {this, "wait_5_minutes", PlatformKey::KEY_GAMEPAD_A, "Wait 5 minutes in Rest menu"};
        Key Screenshot = {this, "screenshot", PlatformKey::KEY_NONE, "Make screenshot key"};
        Key Console = {this, "console", PlatformKey::KEY_NONE, "Open console key (currently it is opening debug menu)"};
        Key ToggleMouseGrab = {this, "toggle_mouse_grab", PlatformKey::KEY_NONE, "Toggle mouse grab key"};
        Key ToggleBorderless = {this, "toggle_borderless", PlatformKey::KEY_NONE, "Toggle window borderless key"};
        Key ToggleFullscreen = {this, "toggle_fullscreen", PlatformKey::KEY_NONE, "Toggle window fullscreen key"};
        Key ToggleResizable = {this, "toggle_resizable", PlatformKey::KEY_NONE, "Toggle window resizable key"};
        Key CycleFilter = {this, "cycle_filter", PlatformKey::KEY_NONE, "Cycle rescale filter modes key"};
        Key ReloadShaders = {this, "reload_shaders", PlatformKey::KEY_NONE, "Reload shaders key"};
        Key SelectChar1 = {this, "select_char_1", PlatformKey::KEY_NONE, "Select 1 character key"};
        Key SelectChar2 = {this, "select_char_2", PlatformKey::KEY_NONE, "Select 2 character key"};
        Key SelectChar3 = {this, "select_char_3", PlatformKey::KEY_NONE, "Select 3 character key"};
        Key SelectChar4 = {this, "select_char_4", PlatformKey::KEY_NONE, "Select 4 character key"};
        Key SelectNPC1 = {this, "select_npc_1", PlatformKey::KEY_NONE, "Select 1 hireling key"};
        Key SelectNPC2 = {this, "select_npc_2", PlatformKey::KEY_NONE, "Select 2 hireling key"};
        Key DialogUp = {this, "dialog_up", PlatformKey::KEY_GAMEPAD_UP, "Dialog up key"};
        Key DialogDown = {this, "dialog_down", PlatformKey::KEY_GAMEPAD_DOWN, "Dialog down key"};
        Key DialogLeft = {this, "dialog_left", PlatformKey::KEY_GAMEPAD_LEFT, "Dialog left key"};
        Key DialogRight = {this, "dialog_right", PlatformKey::KEY_GAMEPAD_RIGHT, "Dialog right key"};
        Key DialogSelect = {this, "dialog_select", PlatformKey::KEY_GAMEPAD_A, "Dialog select key"};
        Key Escape = {this, "escape", PlatformKey::KEY_GAMEPAD_B, "Escape key"};
    };

    Gamepad gamepad{ this };

    Gameplay gameplay{ this };

    class Graphics : public ConfigSection {
     public:
        explicit Graphics(GameConfig *config): ConfigSection(config, "graphics") {}

        ConfigEntry<RendererType> Renderer = {this, "renderer", ConfigRenderer, "Renderer to use, 'OpenGL' or 'OpenGLES'."};

        Bool BloodSplats = {this, "bloodsplats", true, "Enable bloodsplats under corpses."};

        Float BloodSplatsMultiplier = {this, "bloodsplats_multiplier", 1.0f, "Bloodsplats radius multiplier."};

        Bool BloodSplatsFade = {this, "bloodsplats_fade", true, "Enable bloodsplats fading."};

        Float ClipFarDistance = {this, "clip_far_distance", 16192.0f, "Far clip distance."};
        Float ClipNearDistance = {this, "clip_near_distance", 32.0f, "Near clip distance."};

        Bool ColoredLights = {this, "colored_lights", true, "Enable colored lights."};

        Bool Fog = {this, "fog", true, "Enable fog effect. Used at far clip and in foggy weather."};

        Int FogHorizon = {this, "fog_horizon", 39, "Fog height for bottom sky horizon."};

        Float FogDepthRatio = {this, "fog_ratio", 0.75f, "Starting depth ratio of distance fog."};

        Int FPSLimit = {this, "fps_limit", 60, "FPS limit. Use 0 for unlimited."};

        Int Gamma = {this, "gamma", 4, &ValidateGamma, "Gamma level, can be used to adjust brightness."};

        Int HouseMovieX1 = {this, "house_movie_x1", 8, "Viewport top-left offset for in-house movies."};
        Int HouseMovieY1 = {this, "house_movie_y1", 8, "Viewport top-left offset for in-house movies."};

        Int HouseMovieX2 = {this, "house_movie_x2", 172, "Viewport bottom-right offset for in-house movies."};
        Int HouseMovieY2 = {this, "house_movie_y2", 128, "Viewport bottom-right offset for in-house movies."};

        Int MaxVisibleSectors = {this, "maxvisiblesectors", 10, &ValidateMaxSectors, "Max number of BSP sectors to display."};

        Bool SeasonsChange = {this, "seasons_change", true,
                              "Allow changing trees/ground depending on current season (originally was only used in MM6)."};

        Bool Snow = {this, "snow", false,
                     "Snow effect from MM6 (where it was activated by events). Currently it shows every third day in winter."};

        Bool Tinting = {this, "tinting", false,
                        "Enable vanilla's monster coloring method from hardware mode. "
                        "Where monsters look as if a bucket of paint was thrown at them."};

        Int TorchlightDistance = {this, "torchlight_distance", 800, &ValidateTorchlight,
                                  "Torchlight distance per power level. Use 0 to disable torchlight."};

        Int TorchlightFlicker = {this, "torchlight_flicker", 0, &ValidateTorchlight,
                                 "Torchlight lighting flicker effect distance. Use 0 to disable flicker."};

        Bool VSync = {this, "vsync", false, "Enable synchronization of framerate with monitor vertical refresh rate."};

        Int ViewPortX1 = {this, "viewport_x1", 8, "Viewport top-left offset."};
        Int ViewPortY1 = {this, "viewport_y1", 8, "Viewport top-left offset."};

        Int ViewPortX2 = {this, "viewport_x2", 172, "Viewport bottom-right offset."};
        Int ViewPortY2 = {this, "viewport_y2", 128, "Viewport bottom-right offset."};

        Int RenderWidth = {this, "render_width", 640, &ValidateRenderWidth, "Internal rendered resolution width"};
        Int RenderHeight = {this, "render_height", 480, &ValidateRenderHeight, "Internal rendered resolution height"};

        Int RenderFilter = {this, "render_filter", 2, &ValidateRenderFilter,
                            "Filtering method when scaling rendered framebuffer to window dimensions if they differ."
                            " 0 - disabled (render dimensions will always match window dimensions), 1 - linear filter, 2 - nearest filter"};

     private:
        static int ValidateGamma(int level) {
            return std::clamp(level, 0, 9);
        }
        static int ValidateMaxSectors(int sectors) {
            return std::clamp(sectors, 1, 150);
        }
        static int ValidateTorchlight(int distance) {
            if (distance < 0)
                return 0;

            return distance;
        }
        static int ValidateRenderWidth(int dimension) {
            if (dimension < 640)
                return 640;

            return dimension;
        }
        static int ValidateRenderHeight(int dimension) {
            if (dimension < 480)
                return 480;

            return dimension;
        }
        static int ValidateRenderFilter(int filter) {
            return std::clamp(filter, 0, 2);
        }
    };

    Graphics graphics{ this };

    class Keybindings : public ConfigSection {
     public:
        explicit Keybindings(GameConfig *config) : ConfigSection(config, "keybindings") {}

        Key AlwaysRun = {this, "always_run", PlatformKey::KEY_U, "Always run toggle key."};
        Key Attack = {this, "attack", PlatformKey::KEY_A, "Attack key."};
        Key AutoNotes = {this, "auto_notes", PlatformKey::KEY_N, "Open autonotes key."};
        Key Backward = {this, "backward", PlatformKey::KEY_DOWN, "Walk backwards key."};
        Key Cast = {this, "cast", PlatformKey::KEY_C, "Cast a spell from spellbook key."};
        Key CastReady = {this, "cast_ready", PlatformKey::KEY_S, "Cast a quick spell key."};
        Key CenterView = {this, "center_view", PlatformKey::KEY_END, "Center view key."};
        Key CharCycle = {this, "char_cycle", PlatformKey::KEY_TAB, "Switch between characters key."};
        Key Combat = {this, "combat", PlatformKey::KEY_RETURN, "Switch between realtime and turn-based modes key."};
        Key EventTrigger = {this, "event_trigger", PlatformKey::KEY_SPACE, "Interaction key."};
        Key FlyDown = {this, "fly_down", PlatformKey::KEY_INSERT, "Fly down key."};
        Key FlyUp = {this, "fly_up", PlatformKey::KEY_PAGEUP, "Fly up key."};
        Key Forward = {this, "forward", PlatformKey::KEY_UP, "Move forward key."};
        Key Jump = {this, "jump", PlatformKey::KEY_X, "Jump key."};
        Key Land = {this, "land", PlatformKey::KEY_HOME, "Land key."};
        Key Left = {this, "left", PlatformKey::KEY_LEFT, "Turn left key."};
        Key LookDown = {this, "look_down", PlatformKey::KEY_DELETE, "Look down key."};
        Key LookUp = {this, "look_up", PlatformKey::KEY_PAGEDOWN, "Look up key."};
        Key MapBook = {this, "map_book", PlatformKey::KEY_M, "Open map key."};
        Key Pass = {this, "pass", PlatformKey::KEY_B, "Pass turn key"};
        Key Quest = {this, "quest", PlatformKey::KEY_Q, "Open quest book key"};
        Key QuickReference = {this, "quick_reference", PlatformKey::KEY_Z, "Open quick reference menu"};
        Key Rest = {this, "rest", PlatformKey::KEY_R, "Rest key."};
        Key Right = {this, "right", PlatformKey::KEY_RIGHT, "Turn right key."};
        Key StepLeft = {this, "step_left", PlatformKey::KEY_LEFTBRACKET, "Strafe left key."};
        Key StepRight = {this, "step_right", PlatformKey::KEY_RIGHTBRACKET, "Strafe right key."};
        Key TimeCalendar = {this, "time_calendar", PlatformKey::KEY_T, "Open calendar key."};
        Key Yell = {this, "yell", PlatformKey::KEY_Y, "Yell key."};
        Key ZoomIn = {this, "zoom_in", PlatformKey::KEY_ADD, "Zoom in automap key."};
        Key ZoomOut = {this, "zoom_out", PlatformKey::KEY_SUBTRACT, "Zoom out automap key."};
        Key QuickSave = {this, "quick_save", PlatformKey::KEY_F5, "Quick save key"};
        Key QuickLoad = {this, "quick_load", PlatformKey::KEY_F9, "Quick load key"};
        Key History = {this, "history", PlatformKey::KEY_H, "History book key"};
        Key Stats = {this, "stats", PlatformKey::KEY_C, "Stats tab key"};
        Key Skills = {this, "skills", PlatformKey::KEY_S, "Skills tab key"};
        Key Inventory = {this, "inventory", PlatformKey::KEY_I, "Inventory tab key"};
        Key Awards = {this, "awards", PlatformKey::KEY_A, "Stats tab key"};
        Key NewGame = {this, "new_game", PlatformKey::KEY_N, "New Game menu key"};
        Key SaveGame = {this, "save_game", PlatformKey::KEY_S, "Save Game menu key"};
        Key LoadGame = {this, "load_game", PlatformKey::KEY_L, "Load Game menu key"};
        Key ExitGame = {this, "exit_game", PlatformKey::KEY_Q, "Exit Game key"};
        Key ReturnToGame = {this, "return_to_game", PlatformKey::KEY_R, "Return to Game mode key"};
        Key Controls = {this, "controls", PlatformKey::KEY_C, "Controls change menu key"};
        Key Options = {this, "options", PlatformKey::KEY_O, "Options menu key"};
        Key Credits = {this, "credits", PlatformKey::KEY_C, "Credits menu key"};
        Key Clear = {this, "clear", PlatformKey::KEY_C, "Clear button in New Party Creation menu"};
        Key Return = {this, "return", PlatformKey::KEY_RETURN, "Ok button in New Party Creation menu"};
        Key Minus = {this, "minus", PlatformKey::KEY_SUBTRACT, "Minus button in New Party Creation menu"};
        Key Plus = {this, "plus", PlatformKey::KEY_ADD, "Plus button in New Party Creation menu"};
        Key Yes = {this, "yes", PlatformKey::KEY_Y, "Yes answer key"};
        Key No = {this, "no", PlatformKey::KEY_N, "No answer key"};
        Key Rest8Hours = {this, "rest_8_hours", PlatformKey::KEY_R, "Rest for 8 hours key in Rest menu"};
        Key WaitTillDawn = {this, "wait_till_dawn", PlatformKey::KEY_D, "Wait till dawn key in Rest menu"};
        Key WaitHour = {this, "wait_hour", PlatformKey::KEY_H, "Wait hour in Rest menu"};
        Key Wait5Minutes = {this, "wait_5_minutes", PlatformKey::KEY_M, "Wait 5 minutes in Rest menu"};
        Key Screenshot = {this, "screenshot", PlatformKey::KEY_F2, "Make screenshot key"};
        Key Console = {this, "console", PlatformKey::KEY_TILDE, "Open console key (currently it is opening debug menu)"};
        Key ToggleMouseGrab = {this, "toggle_mouse_grab", PlatformKey::KEY_F1, "Toggle mouse grab key"};
        Key ToggleBorderless = {this, "toggle_borderless", PlatformKey::KEY_F3, "Toggle window borderless key"};
        Key ToggleFullscreen = {this, "toggle_fullscreen", PlatformKey::KEY_F4, "Toggle window fullscreen key"};
        Key ToggleResizable = {this, "toggle_resizable", PlatformKey::KEY_F6, "Toggle window resizable key"};
        Key CycleFilter = {this, "cycle_filter", PlatformKey::KEY_F7, "Cycle rescale filter modes key"};
        Key ReloadShaders = {this, "reload_shaders", PlatformKey::KEY_BACKSPACE, "Reload shaders key"};
        Key SelectChar1 = {this, "select_char_1", PlatformKey::KEY_DIGIT_1, "Select 1 character key"};
        Key SelectChar2 = {this, "select_char_2", PlatformKey::KEY_DIGIT_2, "Select 2 character key"};
        Key SelectChar3 = {this, "select_char_3", PlatformKey::KEY_DIGIT_3, "Select 3 character key"};
        Key SelectChar4 = {this, "select_char_4", PlatformKey::KEY_DIGIT_4, "Select 4 character key"};
        Key SelectNPC1 = {this, "select_npc_1", PlatformKey::KEY_DIGIT_5, "Select 1 hireling key"};
        Key SelectNPC2 = {this, "select_npc_2", PlatformKey::KEY_DIGIT_6, "Select 2 hireling key"};
        Key DialogUp = {this, "dialog_up", PlatformKey::KEY_UP, "Dialog up key"};
        Key DialogDown = {this, "dialog_down", PlatformKey::KEY_DOWN, "Dialog down key"};
        Key DialogLeft = {this, "dialog_left", PlatformKey::KEY_LEFT, "Dialog left key"};
        Key DialogRight = {this, "dialog_right", PlatformKey::KEY_RIGHT, "Dialog right key"};
        Key DialogSelect = {this, "dialog_select", PlatformKey::KEY_RETURN, "Dialog select key"};
        Key Escape = {this, "escape", PlatformKey::KEY_ESCAPE, "Escape key"};
    };

    Keybindings keybindings{ this };

    class Settings : public ConfigSection {
     public:
        explicit Settings(GameConfig *config) : ConfigSection(config, "settings") {}

        Bool AlwaysRun = {this, "always_run", true, "Enable always run."};

        Bool FlipOnExit = {this, "flip_on_exit", false, "Flip 180 degrees when leaving a building."};

        Bool ShowHits = {this, "show_hits", true, "Show HP status in status bar."};

        Int MusicLevel = {this, "music_level", 3, &ValidateLevel, "Music volume level."};

        Int SoundLevel = {this, "sound_level", 4, &ValidateLevel, "Sound volume level."};

        Int VoiceLevel = {this, "voice_level", 5, &ValidateLevel, "Voice volume level."};

        Int ScreenshotNumber = {this, "screenshot_number", 0, "Last saved screenshot number."};

        Float TurnSpeed = {this, "turn_speed", 0.0f, &ValidateTurnSpeed,
                           "Discrete turn speed, use 0 for smooth, 16 for 64 degrees turn, 32 for 128 degrees turn, etc. "
                           "Only smooth is usable on modern machines."};

        Int VerticalTurnSpeed = {this, "vertical_turn_speed", 25, &ValidateVerticalTurnSpeed, "Discrete vertical turn speed."};

        Bool WalkSound = {this, "walk_sound", true, "Enable footsteps sound when walking."};

     private:
        static int ValidateLevel(int level) {
            return std::clamp(level, 0, 9);
        }
        static int ValidateVerticalTurnSpeed(int speed) {
            return std::clamp(speed, 1, 128);
        }
        static float ValidateTurnSpeed(float speed) {
            return std::clamp(speed, 0.0f, 1024.0f);
        }
    };

    Settings settings{ this };

    class Window : public ConfigSection {
     public:
        explicit Window(GameConfig *config): ConfigSection(config, "window") {}

        String Title = String(this, "title", "OpenEnroth", &ValidateTitle, "Game window title.");

        Int Display = {this, "display", 0,
                       "Display number as exposed by SDL. "
                       "Order is platform-specific, e.g. on windows 0 is main display"};

        ConfigEntry<PlatformWindowMode> Mode = {this, "mode", ConfigWindowMode,
                                                "Window mode, one of 'windowed', 'borderless', 'fullscreen' or 'fullscreen_borderless'."};

        Int PositionX = {this, "position_x", -1, &ValidatePosition,
                         "Game window x position in display coordinates. Use -1 for centered."};
        Int PositionY = {this, "position_y", -1, &ValidatePosition,
                         "Game window y position in display coordinates. Use -1 for centered."};

        Int Width = {this, "width", 640, &ValidateWidth, "Window width."};
        Int Height = {this, "height", 480, &ValidateHeight, "Window height."};

        Bool MouseGrab = {this, "mouse_grab", false, "Restrict mouse movement to game window when it's in focus."};

        Bool Resizable = {this, "resizable", true, "Make window resizable by user or not."};

        Bool ReloadTex = {this, "reload_tex", true, "Reload texture assets if window is reinitialised."};

     private:
         static std::string ValidateTitle(std::string title) {
             if (title.empty())
                 title = "OpenEnroth";

             return title;
         }
         static int ValidatePosition(int position) {
            if (position < -1)
                return -1;

            return position;
        }
        static int ValidateWidth(int dimension) {
            if (dimension <= 0)
                return 640;

            return dimension;
        }
        static int ValidateHeight(int dimension) {
            if (dimension <= 0)
                return 480;

            return dimension;
        }
    };

    Window window{ this };
};

