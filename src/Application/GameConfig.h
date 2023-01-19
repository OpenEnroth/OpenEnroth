#pragma once

#include <string>
#include <algorithm>

#include "Library/Config/Config.h"
#include "Engine/Graphics/RendererType.h"
#include "Io/Key.h"
#include "Platform/PlatformEnums.h"

#ifdef __ANDROID__
#define ConfigRenderer RendererType::OpenGLES
#define ConfigWindowMode WINDOW_MODE_FULLSCREEN
#else
#define ConfigRenderer RendererType::OpenGL
#define ConfigWindowMode WINDOW_MODE_WINDOWED
#endif

class Logger;

MM_DECLARE_SERIALIZATION_FUNCTIONS(RendererType)
MM_DECLARE_SERIALIZATION_FUNCTIONS(PlatformWindowMode)

namespace Application {
    class GameConfig : public Config {
     public:
        GameConfig();
        ~GameConfig();

        void LoadConfiguration();
        void SaveConfiguration();

        using Bool = ConfigValue<bool>;
        using Int = ConfigValue<int>;
        using Float = ConfigValue<float>;
        using String = ConfigValue<std::string>;
        using Key = ConfigValue<PlatformKey>;

        class Debug : public ConfigSection {
         public:
            explicit Debug(GameConfig *config) : ConfigSection(config, "debug") {}

            Bool AllMagic = Bool(this, "all_magic", false,
                                 "Enable all available spells for each character in spellbook bypassing all class restrictions. "
                                 "Currently also all skills will behave like they are on GM level.");

            Bool InfiniteFood = Bool(this, "infinite_food", false, "Enable unlimited food, using food won't spend it.");
            Bool InfiniteGold = Bool(this, "infinite_gold", false, "Enable unlimited gold, paying in shops won't spend it.");

            Bool LightmapDecals = Bool(this, "lightmap_decals", false, "Draw lightmap and decals outlines.");

            Bool PortalOutlines = Bool(this, "portal_outlines", false, "Draw BLV portal outlines.");
            Bool Terrain = Bool(this, "terrain", false, "Draw terrain as wireframe.");

            Bool TownPortal = Bool(this, "town_portal", false,
                                   "Make all game locations reachable via town portal spell without requiring to visit them first.");

            Bool TurboSpeed = Bool(this, "turbo_speed", false,
                                   "Increase party movement speed by 12x. Most likely you want to use that option with "
                                   "no_damage option enabled as collision physics often will shoot you in the air.");

            Bool WizardEye = Bool(this, "wizard_eye", false, "Activate wizard eye spell that never expires.");

            Bool ShowFPS = Bool(this, "show_fps", false, "Show debug HUD with FPS and other debug information.");

            Bool ShowPickedFace = Bool(this, "show_picked_face", false,
                                       "Face pointed with mouse will flash with red for buildings or green for dungeons.");

            Bool NoIntro = Bool(this, "no_intro", false, "Skip intro movie on startup.");

            Bool NoLogo = Bool(this, "no_logo", false, "Skip 3do logo on startup.");

            Bool NoSound = Bool(this, "no_sound", false, "Don't play any sounds. Currently in-house movies are not affected.");

            Bool NoVideo = Bool(this, "no_video", false, "Don't play any movies.");

            Bool NoActors = Bool(this, "no_actors", false, "Disable all actors.");

            Bool NoDamage = Bool(this, "no_damage", false, "Disable all incoming damage to party.");

            Bool NoDecorations = Bool(this, "no_decorations", false, "Disable all decorations.");

            Bool NoMargaret = Bool(this, "no_margareth", false, "Disable Margaret's tour messages on Emerald Island.");

            Bool VerboseLogging = Bool(this, "verbose_logging", false, "Verbose logging to debug console. Can be extremely spammy.");
        };

        Debug debug{ this };

        class Gameplay : public ConfigSection {
         public:
            explicit Gameplay(GameConfig *config): ConfigSection(config, "gameplay") {}

            Bool AlternativeConditionPriorities = Bool(this, "alternative_condition_priorities", true,
                                                       "Use condition priorities from Grayface patches (e.g. Zombie has the lowest priority).");

            Int ArtifactLimit = Int(this, "artifact_limit", 13, &ValidateArtifactLimit,
                                    "Max number of artifacts that the game can generate as loot in any one playthrough. Use 0 for unlimited.");

            Int ChestTryPlaceItems = Int(this, "chest_try_place_items", 2,
                                         "Fix problems with item loss in high-level chests. "
                                         "Use 0 for vanilla behaviour, items that don't fit will be lost. "
                                         "Use 1 to try to place items that didn't fit every time the chest is opened again. "
                                         "Use 2 to try to place items that didn't fit every time an item is picked up from the chest.");

            Int FloorChecksEps = Int(this, "floor_checks_eps", 3, &ValidateFloorChecksEps,
                                     "Maximum allowed slack for point-inside-a-polygon checks when calculating floor z level. "
                                     "This is needed because there are actual holes in level geometry sometimes, up to several units wide.");

            Int Gravity = Int(this, "gravity", 5, "Gravity strength, the higher the more gravity, 0 disables gravity completely.");

            Float KeyboardInteractionDepth = Float(this, "keyboard_interaction_depth", 512.0f, &ValidateInteractionDepth,
                                                   "Maximum range for item pickup / opening chests / activating levers / etc "
                                                   "with a keyboard (by pressing the interaction key, see keybindings.event_trigger).");

            Float MouseInteractionDepth = Float(this, "mouse_interaction_depth", 512.0f, &ValidateInteractionDepth,
                                                "Maximum range for item pickup / opening chests / activating levers / etc with a mouse.");

            Int MinRecoveryMelee = Int(this, "minimum_recovery_melee", 30, &ValidateRecovery,
                                       "Minimum recovery time for melee weapons. Was 30 in vanilla.");

            Int MinRecoveryRanged = Int(this, "minimum_recovery_ranged", 5, &ValidateRecovery,
                                        "Minimum recovery time for ranged weapons. Was 0 in vanilla, 5 in GrayFace patches.");

            Int MinRecoveryBlasters = Int(this, "minimum_recovery_blasters", 5, &ValidateRecovery,
                                          "Minimum recovery time for blasters. Was 0 in vanilla, 5 in Grayface patches");

            Int MaxFlightHeight = Int(this, "max_flight_height", 4000, &ValidateMaxFlightHeight,
                                      "Maximum height for the fly spell.");

            Float MouseInfoDepthIndoor = Float(this, "mouse_info_depth_indoor", 16192.0f, &ValidateInteractionDepth,
                                               "Maximum range at which right clicking on a monster produces a popup indoors. "
                                               "Also this is the max range for the souldrinker spell indoors.");
            Float MouseInfoDepthOutdoor = Float(this, "mouse_info_depth_outdoor", 12800.0f, &ValidateInteractionDepth,
                                                "Maximum range at which right clicking on a monster produces a popup outdoors. "
                                                "Default value is 12800 = 25 * 512, 25 map cells. "
                                                "Also this is the max range for the souldrinker spell outdoors.");

            Int NewGameFood = Int(this, "new_game_food", 7, "Starting food.");
            Int NewGameGold = Int(this, "new_game_gold", 200, "Starting gold.");
            String StartingMap = String(this, "starting_map", "out01.odm", "New Game starting map.");

            Int PartyEyeLevel = Int(this, "party_eye_level", 160, "Party eye level.");
            Int PartyHeight = Int(this, "party_height", 192, "Party height.");
            Int PartyWalkSpeed = Int(this, "party_walk_speed", 384, "Party walk speed.");

            Float RangedAttackDepth = Float(this, "ranged_attack_depth", 5120.0f, &ValidateRangedAttackDepth,
                                            "Max depth for ranged attacks and ranged spells. "
                                            "It's impossible to target monsters that are further away than this value. "
                                            "This is also the depth at which status bar tips are displayed on mouse over.");

            Bool ShowUndentifiedItem = Bool(this, "show_unidentified_item", true,
                                            "Show unidentified items with a green tint in inventory. "
                                            "If not set, vanilla behavior will be used with green tint applied in shops only.");

            Bool TreatClubAsMace = Bool(this, "treat_club_as_mace", false,
                                        "Treat clubs as maces. "
                                        "In vanilla clubs are using a separate hidden skill and can be equipped without learning the mace skill.");

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

            Key AlwaysRun = Key(this, "always_run", PlatformKey::Gamepad_L3, "Always run toggle key.");
            Key Attack = Key(this, "attack", PlatformKey::Gamepad_L1, "Attack key.");
            Key AutoNotes = Key(this, "auto_notes", PlatformKey::Gamepad_Up, "Open autonotes key.");
            Key Backward = Key(this, "backward", PlatformKey::Gamepad_LeftStick_Down, "Walk backwards key.");
            Key Cast = Key(this, "cast", PlatformKey::Gamepad_Down, "Cast a spell from spellbook key.");
            Key CastReady = Key(this, "cast_ready", PlatformKey::Gamepad_R1, "Cast a quick spell key.");
            Key CenterView = Key(this, "center_view", PlatformKey::None, "Center view key.");
            Key CharCycle = Key(this, "char_cycle", PlatformKey::None, "Switch between characters key.");
            Key Combat = Key(this, "combat", PlatformKey::Gamepad_Start, "Switch between realtime and turn-based modes key.");
            Key EventTrigger = Key(this, "event_trigger", PlatformKey::Gamepad_A, "Interaction key.");
            Key FlyDown = Key(this, "fly_down", PlatformKey::Gamepad_L2, "Fly down key.");
            Key FlyUp = Key(this, "fly_up", PlatformKey::Gamepad_R2, "Fly up key.");
            Key Forward = Key(this, "forward", PlatformKey::Gamepad_LeftStick_Up, "Move forward key.");
            Key Jump = Key(this, "jump", PlatformKey::Gamepad_Y, "Jump key.");
            Key Land = Key(this, "land", PlatformKey::Gamepad_R3, "Land key.");
            Key Left = Key(this, "left", PlatformKey::Gamepad_RightStick_Left, "Turn left key.");
            Key LookDown = Key(this, "look_down", PlatformKey::Gamepad_RightStick_Down, "Look down key.");
            Key LookUp = Key(this, "look_up", PlatformKey::Gamepad_RightStick_Up, "Look up key.");
            Key MapBook = Key(this, "map_book", PlatformKey::Gamepad_Left, "Open map key.");
            Key Pass = Key(this, "pass", PlatformKey::Gamepad_Guide, "Pass turn key");
            Key Quest = Key(this, "quest", PlatformKey::Gamepad_Right, "Open quest book key");
            Key QuickReference = Key(this, "quick_reference", PlatformKey::None, "Open quick reference menu");
            Key Rest = Key(this, "rest", PlatformKey::Gamepad_Back, "Rest key.");
            Key Right = Key(this, "right", PlatformKey::Gamepad_RightStick_Right, "Turn right key.");
            Key StepLeft = Key(this, "step_left", PlatformKey::Gamepad_LeftStick_Left, "Strafe left key.");
            Key StepRight = Key(this, "step_right", PlatformKey::Gamepad_LeftStick_Right, "Strafe right key.");
            Key TimeCalendar = Key(this, "time_calendar", PlatformKey::None, "Open calendar key.");
            Key Yell = Key(this, "yell", PlatformKey::Gamepad_X, "Yell key.");
            Key ZoomIn = Key(this, "zoom_in", PlatformKey::None, "Zoom in automap key.");
            Key ZoomOut = Key(this, "zoom_out", PlatformKey::None, "Zoom out automap key.");
            Key QuickSave = Key(this, "quick_save", PlatformKey::None, "Quick save key");
            Key QuickLoad = Key(this, "quick_load", PlatformKey::None, "Quick load key");
            Key History = Key(this, "history", PlatformKey::None, "History book key");
            Key Stats = Key(this, "stats", PlatformKey::Gamepad_A, "Stats tab key");
            Key Skills = Key(this, "skills", PlatformKey::Gamepad_X, "Skills tab key");
            Key Inventory = Key(this, "inventory", PlatformKey::Gamepad_Y, "Inventory tab key");
            Key Awards = Key(this, "awards", PlatformKey::Gamepad_L1, "Stats tab key");
            Key NewGame = Key(this, "new_game", PlatformKey::Gamepad_A, "New Game menu key");
            Key SaveGame = Key(this, "save_game", PlatformKey::Gamepad_Y, "Save Game menu key");
            Key LoadGame = Key(this, "load_game", PlatformKey::Gamepad_X, "Load Game menu key");
            Key ExitGame = Key(this, "exit_game", PlatformKey::Gamepad_B, "Exit Game key");
            Key ReturnToGame = Key(this, "return_to_game", PlatformKey::Gamepad_B, "Return to Game mode key");
            Key Controls = Key(this, "controls", PlatformKey::Gamepad_L1, "Controls change menu key");
            Key Options = Key(this, "options", PlatformKey::Gamepad_R1, "Options menu key");
            Key Credits = Key(this, "credits", PlatformKey::Gamepad_Y, "Credits menu key");
            Key Clear = Key(this, "clear", PlatformKey::Gamepad_Y, "Clear button in New Party Creation menu");
            Key Return = Key(this, "return", PlatformKey::Gamepad_A, "Ok button in New Party Creation menu");
            Key Minus = Key(this, "minus", PlatformKey::Gamepad_L1, "Minus button in New Party Creation menu");
            Key Plus = Key(this, "plus", PlatformKey::Gamepad_R1, "Plus button in New Party Creation menu");
            Key Yes = Key(this, "yes", PlatformKey::Gamepad_A, "Yes answer key");
            Key No = Key(this, "no", PlatformKey::Gamepad_B, "No answer key");
            Key Rest8Hours = Key(this, "rest_8_hours", PlatformKey::Gamepad_Back, "Rest for 8 hours key in Rest menu");
            Key WaitTillDawn = Key(this, "wait_till_dawn", PlatformKey::Gamepad_Y, "Wait till dawn key in Rest menu");
            Key WaitHour = Key(this, "wait_hour", PlatformKey::Gamepad_X, "Wait hour in Rest menu");
            Key Wait5Minutes = Key(this, "wait_5_minutes", PlatformKey::Gamepad_A, "Wait 5 minutes in Rest menu");
            Key Screenshot = Key(this, "screenshot", PlatformKey::None, "Make screenshot key");
            Key Console = Key(this, "console", PlatformKey::None, "Open console key (currently it is opening debug menu)");
            Key ToggleMouseGrab = Key(this, "toggle_mouse_grab", PlatformKey::None, "Toggle mouse grab key");
            Key ToggleBorderless = Key(this, "toggle_borderless", PlatformKey::None, "Toggle window borderless key");
            Key ToggleFullscreen = Key(this, "toggle_fullscreen", PlatformKey::None, "Toggle window fullscreen key");
            Key ToggleResizable = Key(this, "toggle_resizable", PlatformKey::None, "Toggle window resizable key");
            Key CycleFilter = Key(this, "cycle_filter", PlatformKey::None, "Cycle rescale filter modes key");
            Key ReloadShaders = Key(this, "reload_shaders", PlatformKey::None, "Reload shaders key");
            Key SelectChar1 = Key(this, "select_char_1", PlatformKey::None, "Select 1 character key");
            Key SelectChar2 = Key(this, "select_char_2", PlatformKey::None, "Select 2 character key");
            Key SelectChar3 = Key(this, "select_char_3", PlatformKey::None, "Select 3 character key");
            Key SelectChar4 = Key(this, "select_char_4", PlatformKey::None, "Select 4 character key");
            Key SelectNPC1 = Key(this, "select_npc_1", PlatformKey::None, "Select 1 hireling key");
            Key SelectNPC2 = Key(this, "select_npc_2", PlatformKey::None, "Select 2 hireling key");
            Key DialogUp = Key(this, "dialog_up", PlatformKey::Gamepad_Up, "Dialog up key");
            Key DialogDown = Key(this, "dialog_down", PlatformKey::Gamepad_Down, "Dialog down key");
            Key DialogLeft = Key(this, "dialog_left", PlatformKey::Gamepad_Left, "Dialog left key");
            Key DialogRight = Key(this, "dialog_right", PlatformKey::Gamepad_Right, "Dialog right key");
            Key DialogSelect = Key(this, "dialog_select", PlatformKey::Gamepad_A, "Dialog select key");
            Key Escape = Key(this, "escape", PlatformKey::Gamepad_B, "Escape key");
        };

        Gamepad gamepad{ this };

        Gameplay gameplay{ this };

        class Graphics : public ConfigSection {
         public:
            explicit Graphics(GameConfig *config): ConfigSection(config, "graphics") {}

            ConfigValue<RendererType> Renderer = ConfigValue<RendererType>(this, "renderer", ConfigRenderer, "Renderer to use, 'OpenGL' or 'OpenGLES'.");

            Bool BloodSplats = Bool(this, "bloodsplats", true, "Enable bloodsplats under corpses.");

            Float BloodSplatsMultiplier = Float(this, "bloodsplats_multiplier", 1.0f, "Bloodsplats radius multiplier.");

            Bool BloodSplatsFade = Bool(this, "bloodsplats_fade", true, "Enable bloodsplats fading.");

            Float ClipFarDistance = Float(this, "clip_far_distance", 16192.0f, "Far clip distance.");
            Float ClipNearDistance = Float(this, "clip_near_distance", 32.0f, "Near clip distance.");

            Bool ColoredLights = Bool(this, "colored_lights", true, "Enable colored lights.");

            // TODO(captainurist): drop
            Int D3DDevice = Int(this, "d3d_device", 0, "D3D device number which was set by setup program in vanilla for hardware mode.");

            // TODO(captainurist): drop? lightmap builder option for old drawing system, need to be eventually deleted and replaced with gamma?
            Bool DynamicBrightness = Bool(this, "dynamic_brightness", true, "");

            Bool Fog = Bool(this, "fog", true, "Enable fog effect. Used at far clip and in foggy weather.");

            Int FogHorizon = Int(this, "fog_horizon", 39, "Fog height for bottom sky horizon.");

            Float FogDepthRatio = Float(this, "fog_ratio", 0.75f, "Starting depth ratio of distance fog.");

            Int FPSLimit = Int(this, "fps_limit", 60, "FPS limit. Use 0 for unlimited.");

            Int Gamma = Int(this, "gamma", 4, &ValidateGamma, "Gamma level, can be used to adjust brightness.");

            Int HouseMovieX1 = Int(this, "house_movie_x1", 8, "Viewport top-left offset for in-house movies.");
            Int HouseMovieY1 = Int(this, "house_movie_y1", 8, "Viewport top-left offset for in-house movies.");

            Int HouseMovieX2 = Int(this, "house_movie_x2", 172, "Viewport bottom-right offset for in-house movies.");
            Int HouseMovieY2 = Int(this, "house_movie_y2", 128, "Viewport bottom-right offset for in-house movies.");

            Bool HWLBitmaps = Bool(this, "hwl_bitmaps", false,
                                   "Use low-resolution bitmaps from HWL file instead of hi-resolution ones from LOD.");

            Bool HWLSprites = Bool(this, "hwl_sprites", false,
                                   "Use low-resolution sprites from HWL file instead of hi-resolution ones from LOD.");

            Int MaxVisibleSectors = Int(this, "maxvisiblesectors", 10, &ValidateMaxSectors, "Max number of BSP sectors to display.");

            Bool SeasonsChange = Bool(this, "seasons_change", true,
                                      "Allow changing trees/ground depending on current season (originally was only used in MM6).");

            Bool Snow = Bool(this, "snow", false,
                             "Snow effect from MM6 (where it was activated by events). Currently it shows every third day in winter.");

            Bool Tinting = Bool(this, "tinting", false,
                                "Enable vanilla's monster coloring method from hardware mode. "
                                "Where monsters look as if a bucket of paint was thrown at them.");

            Int TorchlightDistance = Int(this, "torchlight_distance", 800, &ValidateTorchlight,
                                         "Torchlight distance per power level. Use 0 to disable torchlight.");

            Int TorchlightFlicker = Int(this, "torchlight_flicker", 200, &ValidateTorchlight,
                                        "Torchlight lighting flicker effect distance. Use 0 to disable flicker.");

            Bool VSync = Bool(this, "vsync", false, "Enable synchronization of framerate with monitor vertical refresh rate.");

            Int ViewPortX1 = Int(this, "viewport_x1", 8, "Viewport top-left offset.");
            Int ViewPortY1 = Int(this, "viewport_y1", 8, "Viewport top-left offset.");

            Int ViewPortX2 = Int(this, "viewport_x2", 172, "Viewport bottom-right offset.");
            Int ViewPortY2 = Int(this, "viewport_y2", 128, "Viewport bottom-right offset.");

            Int RenderWidth = Int(this, "render_width", 640, &ValidateRenderWidth, "Internal rendered resolution width");
            Int RenderHeight = Int(this, "render_height", 480, &ValidateRenderHeight, "Internal rendered resolution height");

            Int RenderFilter = Int(this, "render_filter", 2, &ValidateRenderFilter,
                                    "Filtering method when scaling rendered framebuffer to window dimensions if they differ."
                                    " 0 - disabled (render dimensions will always match window dimensions), 1 - linear filter, 2 - nearest filter");

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

            Key AlwaysRun = Key(this, "always_run", PlatformKey::U, "Always run toggle key.");
            Key Attack = Key(this, "attack", PlatformKey::A, "Attack key.");
            Key AutoNotes = Key(this, "auto_notes", PlatformKey::N, "Open autonotes key.");
            Key Backward = Key(this, "backward", PlatformKey::Down, "Walk backwards key.");
            Key Cast = Key(this, "cast", PlatformKey::C, "Cast a spell from spellbook key.");
            Key CastReady = Key(this, "cast_ready", PlatformKey::S, "Cast a quick spell key.");
            Key CenterView = Key(this, "center_view", PlatformKey::End, "Center view key.");
            Key CharCycle = Key(this, "char_cycle", PlatformKey::Tab, "Switch between characters key.");
            Key Combat = Key(this, "combat", PlatformKey::Return, "Switch between realtime and turn-based modes key.");
            Key EventTrigger = Key(this, "event_trigger", PlatformKey::Space, "Interaction key.");
            Key FlyDown = Key(this, "fly_down", PlatformKey::Insert, "Fly down key.");
            Key FlyUp = Key(this, "fly_up", PlatformKey::PageUp, "Fly up key.");
            Key Forward = Key(this, "forward", PlatformKey::Up, "Move forward key.");
            Key Jump = Key(this, "jump", PlatformKey::X, "Jump key.");
            Key Land = Key(this, "land", PlatformKey::Home, "Land key.");
            Key Left = Key(this, "left", PlatformKey::Left, "Turn left key.");
            Key LookDown = Key(this, "look_down", PlatformKey::Delete, "Look down key.");
            Key LookUp = Key(this, "look_up", PlatformKey::PageDown, "Look up key.");
            Key MapBook = Key(this, "map_book", PlatformKey::M, "Open map key.");
            Key Pass = Key(this, "pass", PlatformKey::B, "Pass turn key");
            Key Quest = Key(this, "quest", PlatformKey::Q, "Open quest book key");
            Key QuickReference = Key(this, "quick_reference", PlatformKey::Z, "Open quick reference menu");
            Key Rest = Key(this, "rest", PlatformKey::R, "Rest key.");
            Key Right = Key(this, "right", PlatformKey::Right, "Turn right key.");
            Key StepLeft = Key(this, "step_left", PlatformKey::LeftBracket, "Strafe left key.");
            Key StepRight = Key(this, "step_right", PlatformKey::RightBracket, "Strafe right key.");
            Key TimeCalendar = Key(this, "time_calendar", PlatformKey::T, "Open calendar key.");
            Key Yell = Key(this, "yell", PlatformKey::Y, "Yell key.");
            Key ZoomIn = Key(this, "zoom_in", PlatformKey::Add, "Zoom in automap key.");
            Key ZoomOut = Key(this, "zoom_out", PlatformKey::Subtract, "Zoom out automap key.");
            Key QuickSave = Key(this, "quick_save", PlatformKey::F5, "Quick save key");
            Key QuickLoad = Key(this, "quick_load", PlatformKey::F9, "Quick load key");
            Key History = Key(this, "history", PlatformKey::H, "History book key");
            Key Stats = Key(this, "stats", PlatformKey::C, "Stats tab key");
            Key Skills = Key(this, "skills", PlatformKey::S, "Skills tab key");
            Key Inventory = Key(this, "inventory", PlatformKey::I, "Inventory tab key");
            Key Awards = Key(this, "awards", PlatformKey::A, "Stats tab key");
            Key NewGame = Key(this, "new_game", PlatformKey::N, "New Game menu key");
            Key SaveGame = Key(this, "save_game", PlatformKey::S, "Save Game menu key");
            Key LoadGame = Key(this, "load_game", PlatformKey::L, "Load Game menu key");
            Key ExitGame = Key(this, "exit_game", PlatformKey::Q, "Exit Game key");
            Key ReturnToGame = Key(this, "return_to_game", PlatformKey::R, "Return to Game mode key");
            Key Controls = Key(this, "controls", PlatformKey::C, "Controls change menu key");
            Key Options = Key(this, "options", PlatformKey::O, "Options menu key");
            Key Credits = Key(this, "credits", PlatformKey::C, "Credits menu key");
            Key Clear = Key(this, "clear", PlatformKey::C, "Clear button in New Party Creation menu");
            Key Return = Key(this, "return", PlatformKey::Return, "Ok button in New Party Creation menu");
            Key Minus = Key(this, "minus", PlatformKey::Subtract, "Minus button in New Party Creation menu");
            Key Plus = Key(this, "plus", PlatformKey::Add, "Plus button in New Party Creation menu");
            Key Yes = Key(this, "yes", PlatformKey::Y, "Yes answer key");
            Key No = Key(this, "no", PlatformKey::N, "No answer key");
            Key Rest8Hours = Key(this, "rest_8_hours", PlatformKey::R, "Rest for 8 hours key in Rest menu");
            Key WaitTillDawn = Key(this, "wait_till_dawn", PlatformKey::D, "Wait till dawn key in Rest menu");
            Key WaitHour = Key(this, "wait_hour", PlatformKey::H, "Wait hour in Rest menu");
            Key Wait5Minutes = Key(this, "wait_5_minutes", PlatformKey::M, "Wait 5 minutes in Rest menu");
            Key Screenshot = Key(this, "screenshot", PlatformKey::F2, "Make screenshot key");
            Key Console = Key(this, "console", PlatformKey::Tilde, "Open console key (currently it is opening debug menu)");
            Key ToggleMouseGrab = Key(this, "toggle_mouse_grab", PlatformKey::F1, "Toggle mouse grab key");
            Key ToggleBorderless = Key(this, "toggle_borderless", PlatformKey::F3, "Toggle window borderless key");
            Key ToggleFullscreen = Key(this, "toggle_fullscreen", PlatformKey::F4, "Toggle window fullscreen key");
            Key ToggleResizable = Key(this, "toggle_resizable", PlatformKey::F6, "Toggle window resizable key");
            Key CycleFilter = Key(this, "cycle_filter", PlatformKey::F7, "Cycle rescale filter modes key");
            Key ReloadShaders = Key(this, "reload_shaders", PlatformKey::Backspace, "Reload shaders key");
            Key SelectChar1 = Key(this, "select_char_1", PlatformKey::Digit1, "Select 1 character key");
            Key SelectChar2 = Key(this, "select_char_2", PlatformKey::Digit2, "Select 2 character key");
            Key SelectChar3 = Key(this, "select_char_3", PlatformKey::Digit3, "Select 3 character key");
            Key SelectChar4 = Key(this, "select_char_4", PlatformKey::Digit4, "Select 4 character key");
            Key SelectNPC1 = Key(this, "select_npc_1", PlatformKey::Digit5, "Select 1 hireling key");
            Key SelectNPC2 = Key(this, "select_npc_2", PlatformKey::Digit6, "Select 2 hireling key");
            Key DialogUp = Key(this, "dialog_up", PlatformKey::Up, "Dialog up key");
            Key DialogDown = Key(this, "dialog_down", PlatformKey::Down, "Dialog down key");
            Key DialogLeft = Key(this, "dialog_left", PlatformKey::Left, "Dialog left key");
            Key DialogRight = Key(this, "dialog_right", PlatformKey::Right, "Dialog right key");
            Key DialogSelect = Key(this, "dialog_select", PlatformKey::Return, "Dialog select key");
            Key Escape = Key(this, "escape", PlatformKey::Escape, "Escape key");
        };

        Keybindings keybindings{ this };

        class Settings : public ConfigSection {
         public:
            explicit Settings(GameConfig *config) : ConfigSection(config, "settings") {}

            Bool AlwaysRun = Bool(this, "always_run", true, "Enable always run.");

            Bool FlipOnExit = Bool(this, "flip_on_exit", false, "Flip 180 degrees when leaving a building.");

            Bool ShowHits = Bool(this, "show_hits", true, "Show HP status in status bar.");

            Int MusicLevel = Int(this, "music_level", 3, &ValidateLevel, "Music volume level.");

            Int SoundLevel = Int(this, "sound_level", 4, &ValidateLevel, "Sound volume level.");

            Int VoiceLevel = Int(this, "voice_level", 5, &ValidateLevel, "Voice volume level.");

            Int ScreenshotNumber = Int(this, "screenshot_number", 0, "Last saved screenshot number.");

            Float TurnSpeed = Float(this, "turn_speed", 0.0f, &ValidateTurnSpeed,
                                    "Discrete turn speed, use 0 for smooth, 16 for 64 degrees turn, 32 for 128 degrees turn, etc. "
                                    "Only smooth is usable on modern machines.");

            Int VerticalTurnSpeed = Int(this, "vertical_turn_speed", 25, &ValidateVerticalTurnSpeed, "Discrete vertical turn speed.");

            Bool WalkSound = Bool(this, "walk_sound", true, "Enable footsteps sound when walking.");

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

            Int Display = Int(this, "display", 0,
                              "Display number as exposed by SDL. "
                              "Order is platform-specific, e.g. on windows 0 is main display");

            ConfigValue<PlatformWindowMode> Mode = ConfigValue<PlatformWindowMode>(this, "mode", ConfigWindowMode,
                           "Window mode, one of 'windowed', 'borderless', 'fullscreen' or 'fake_fullscreen'.");

            Int PositionX = Int(this, "position_x", -1, &ValidatePosition,
                                "Game window x position in display coordinates. Use -1 for centered.");
            Int PositionY = Int(this, "position_y", -1, &ValidatePosition,
                                "Game window y position in display coordinates. Use -1 for centered.");

            Int Width = Int(this, "width", 640, &ValidateWidth, "Window width.");
            Int Height = Int(this, "height", 480, &ValidateHeight, "Window height.");

            Bool MouseGrab = Bool(this, "mouse_grab", true, "Restrict mouse movement to game window when it's in focus.");

            Bool Resizable = ConfigValue<bool>(this, "resizable", true, "Make window resizable by user or not.");

            Bool ReloadTex = ConfigValue<bool>(this, "reload_tex", true, "Reload texture assets if window is reinitialised.");

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

     private:
        const std::string config_file = "openenroth.ini";
        Logger *logger = nullptr;
    };

}  // namespace Application
