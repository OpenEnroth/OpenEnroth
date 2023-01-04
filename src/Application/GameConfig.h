#pragma once

#include <string>
#include <algorithm>

#include "Library/Config/Config.h"

#ifdef __ANDROID__
#define ConfigRenderer "OpenGLES"
#define ConfigWindowMode 2
#else
#define ConfigRenderer "OpenGL"
#define ConfigWindowMode 0
#endif

class Logger;

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

            String AlwaysRun = String(this, "always_run", "L3", &ValidateKey, "Always run toggle key.");
            String Attack = String(this, "attack", "L1", &ValidateKey, "Attack key.");
            String AutoNotes = String(this, "auto_notes", "DPAD UP", &ValidateKey, "Open autonotes key.");
            String Backward = String(this, "backward", "LSTICK DOWN", &ValidateKey, "Walk backwards key.");
            String Cast = String(this, "cast", "DPAD DOWN", &ValidateKey, "Cast a spell from spellbook key.");
            String CastReady = String(this, "cast_ready", "R1", &ValidateKey, "Cast a quick spell key.");
            String CenterView = String(this, "center_view", "", &ValidateKey, "Center view key.");
            String CharCycle = String(this, "char_cycle", "", &ValidateKey, "Switch between characters key.");
            String Combat = String(this, "combat", "START", &ValidateKey, "Switch between realtime and turn-based modes key.");
            String EventTrigger = String(this, "event_trigger", "A (CROSS)", &ValidateKey, "Interaction key.");
            String FlyDown = String(this, "fly_down", "L2", &ValidateKey, "Fly down key.");
            String FlyUp = String(this, "fly_up", "R2", &ValidateKey, "Fly up key.");
            String Forward = String(this, "forward", "LSTICK UP", &ValidateKey, "Move forward key.");
            String Jump = String(this, "jump", "Y (TRIANGLE)", &ValidateKey, "Jump key.");
            String Land = String(this, "land", "R3", &ValidateKey, "Land key.");
            String Left = String(this, "left", "RSTICK LEFT", &ValidateKey, "Turn left key.");
            String LookDown = String(this, "look_down", "RSTICK DOWN", &ValidateKey, "Look down key.");
            String LookUp = String(this, "look_up", "RSTICK UP", &ValidateKey, "Look up key.");
            String MapBook = String(this, "map_book", "DPAD LEFT", &ValidateKey, "Open map key.");
            String Pass = String(this, "pass", "GUIDE", &ValidateKey, "Pass turn key");
            String Quest = String(this, "quest", "DPAD RIGHT", &ValidateKey, "Open quest book key");
            String QuickReference = String(this, "quick_reference", "", &ValidateKey, "Open quick reference menu");
            String Rest = String(this, "rest", "BACK", &ValidateKey, "Rest key.");
            String Right = String(this, "right", "RSTICK RIGHT", &ValidateKey, "Turn right key.");
            String StepLeft = String(this, "step_left", "LSTICK LEFT", &ValidateKey, "Strafe left key.");
            String StepRight = String(this, "step_right", "LSTICK RIGHT", &ValidateKey, "Strafe right key.");
            String TimeCalendar = String(this, "time_calendar", "", &ValidateKey, "Open calendar key.");
            String Yell = String(this, "yell", "X (SQUARE)", &ValidateKey, "Yell key.");
            String ZoomIn = String(this, "zoom_in", "", &ValidateKey, "Zoom in automap key.");
            String ZoomOut = String(this, "zoom_out", "", &ValidateKey, "Zoom out automap key.");

            String QuickSave = String(this, "quick_save", "", &ValidateKey, "Quick save key");
            String QuickLoad = String(this, "quick_load", "", &ValidateKey, "Quick load key");
            String History = String(this, "history", "", &ValidateKey, "History book key");
            String Stats = String(this, "stats", "A (CROSS)", &ValidateKey, "Stats tab key");
            String Skills = String(this, "skills", "X (SQUARE)", &ValidateKey, "Skills tab key");
            String Inventory = String(this, "inventory", "Y (TRIANGLE)", &ValidateKey, "Inventory tab key");
            String Awards = String(this, "awards", "L1", &ValidateKey, "Stats tab key");
            String NewGame = String(this, "new_game", "A (CROSS)", &ValidateKey, "New Game menu key");
            String SaveGame = String(this, "save_game", "Y (TRIANGLE)", &ValidateKey, "Save Game menu key");
            String LoadGame = String(this, "load_game", "X (SQUARE)", &ValidateKey, "Load Game menu key");
            String ExitGame = String(this, "exit_game", "B (CIRCLE)", &ValidateKey, "Exit Game key");
            String ReturnToGame = String(this, "return_to_game", "B (CIRCLE)", &ValidateKey, "Return to Game mode key");
            String Controls = String(this, "controls", "L1", &ValidateKey, "Controls change menu key");
            String Options = String(this, "options", "R1", &ValidateKey, "Options menu key");
            String Credits = String(this, "credits", "Y (TRIANGLE)", &ValidateKey, "Credits menu key");
            String Clear = String(this, "clear", "Y (TRIANGLE)", &ValidateKey, "Clear button in New Party Creation menu");
            String Return = String(this, "return", "A (CROSS)", &ValidateKey, "Ok button in New Party Creation menu");
            String Minus = String(this, "minus", "L1", &ValidateKey, "Minus button in New Party Creation menu");
            String Plus = String(this, "plus", "R1", &ValidateKey, "Plus button in New Party Creation menu");
            String Yes = String(this, "yes", "A (CROSS)", &ValidateKey, "Yes answer key");
            String No = String(this, "no", "B (CIRCLE)", &ValidateKey, "No answer key");
            String Rest8Hours = String(this, "rest_8_hours", "BACK", &ValidateKey, "Rest for 8 hours key in Rest menu");
            String WaitTillDawn = String(this, "wait_till_dawn", "Y (TRIANGLE)", &ValidateKey, "Wait till dawn key in Rest menu");
            String WaitHour = String(this, "wait_hour", "X (SQUARE)", &ValidateKey, "Wait hour in Rest menu");
            String Wait5Minutes = String(this, "wait_5_minutes", "A (CROSS)", &ValidateKey, "Wait 5 minutes in Rest menu");
            String Screenshot = String(this, "screenshot", "", &ValidateKey, "Make screenshot key");
            String Console = String(this, "console", "", &ValidateKey, "Open console key (currently it is opening debug menu)");
            String ToggleMouseGrab = String(this, "toggle_mouse_grab", "", &ValidateKey, "Toggle mouse grab key");
            String ToggleBorderless = String(this, "toggle_borderless", "", &ValidateKey, "Toggle window borderless key");
            String ToggleFullscreen = String(this, "toggle_fullscreen", "", &ValidateKey, "Toggle window fullscreen key");
            String ToggleResizable = String(this, "toggle_resizable", "", &ValidateKey, "Toggle window resizable key");
            String CycleFilter = String(this, "cycle_filter", "", &ValidateKey, "Cycle rescale filter modes key");
            String ReloadShaders = String(this, "reload_shaders", "", &ValidateKey, "Reload shaders key");
            String SelectChar1 = String(this, "select_char_1", "", &ValidateKey, "Select 1 character key");
            String SelectChar2 = String(this, "select_char_2", "", &ValidateKey, "Select 2 character key");
            String SelectChar3 = String(this, "select_char_3", "", &ValidateKey, "Select 3 character key");
            String SelectChar4 = String(this, "select_char_4", "", &ValidateKey, "Select 4 character key");
            String SelectNPC1 = String(this, "select_npc_1", "", &ValidateKey, "Select 1 hireling key");
            String SelectNPC2 = String(this, "select_npc_2", "", &ValidateKey, "Select 2 hireling key");
            String DialogUp = String(this, "dialog_up", "DPAD UP", &ValidateKey, "Dialog up key");
            String DialogDown = String(this, "dialog_down", "DPAD DOWN", &ValidateKey, "Dialog down key");
            String DialogLeft = String(this, "dialog_left", "DPAD LEFT", &ValidateKey, "Dialog left key");
            String DialogRight = String(this, "dialog_right", "DPAD RIGHT", &ValidateKey, "Dialog right key");
            String DialogSelect = String(this, "dialog_select", "A (CROSS)", &ValidateKey, "Dialog select key");
            String Escape = String(this, "escape", "B (CIRCLE)", &ValidateKey, "Escape key");

         private:
            static std::string ValidateKey(std::string key) {
                // TODO [config]: validation, maybe via GameKey::displayNames
                return key;
            }
        };

        Gamepad gamepad{ this };

        Gameplay gameplay{ this };

        class Graphics : public ConfigSection {
         public:
            explicit Graphics(GameConfig *config): ConfigSection(config, "graphics") {}

            String Renderer = String(this, "renderer", ConfigRenderer, &ValidateRenderer, "Renderer to use. OpenGL or OpenGLES");

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
            static std::string ValidateRenderer(std::string renderer) {
                if (renderer != "OpenGL" && renderer != "OpenGLES"
#ifdef DDRAW_ENABLED
                    && renderer != "DirectDraw"
#endif
                )
                    renderer = "OpenGL";

                return renderer;
            }
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

            String AlwaysRun = String(this, "always_run", "U", &ValidateKey, "Always run toggle key.");
            String Attack = String(this, "attack", "A", &ValidateKey, "Attack key.");
            String AutoNotes = String(this, "auto_notes", "N", &ValidateKey, "Open autonotes key.");
            String Backward = String(this, "backward", "DOWN", &ValidateKey, "Walk backwards key.");
            String Cast = String(this, "cast", "C", &ValidateKey, "Cast a spell from spellbook key.");
            String CastReady = String(this, "cast_ready", "S", &ValidateKey, "Cast a quick spell key.");
            String CenterView = String(this, "center_view", "END", &ValidateKey, "Center view key.");
            String CharCycle = String(this, "char_cycle", "TAB", &ValidateKey, "Switch between characters key.");
            String Combat = String(this, "combat", "RETURN", &ValidateKey, "Switch between realtime and turn-based modes key.");
            String EventTrigger = String(this, "event_trigger", "SPACE", &ValidateKey, "Interaction key.");
            String FlyDown = String(this, "fly_down", "INSERT", &ValidateKey, "Fly down key.");
            String FlyUp = String(this, "fly_up", "PAGE UP", &ValidateKey, "Fly up key.");
            String Forward = String(this, "forward", "UP", &ValidateKey, "Move forward key.");
            String Jump = String(this, "jump", "X", &ValidateKey, "Jump key.");
            String Land = String(this, "land", "HOME", &ValidateKey, "Land key.");
            String Left = String(this, "left", "LEFT", &ValidateKey, "Turn left key.");
            String LookDown = String(this, "look_down", "DELETE", &ValidateKey, "Look down key.");
            String LookUp = String(this, "look_up", "PAGE DOWN", &ValidateKey, "Look up key.");
            String MapBook = String(this, "map_book", "M", &ValidateKey, "Open map key.");
            String Pass = String(this, "pass", "B", &ValidateKey, "Pass turn key");
            String Quest = String(this, "quest", "Q", &ValidateKey, "Open quest book key");
            String QuickReference = String(this, "quick_reference", "Z", &ValidateKey, "Open quick reference menu");
            String Rest = String(this, "rest", "R", &ValidateKey, "Rest key.");
            String Right = String(this, "right", "RIGHT", &ValidateKey, "Turn right key.");
            String StepLeft = String(this, "step_left", "L BRACKET", &ValidateKey, "Strafe left key.");
            String StepRight = String(this, "step_right", "R BRACKET", &ValidateKey, "Strafe right key.");
            String TimeCalendar = String(this, "time_calendar", "T", &ValidateKey, "Open calendar key.");
            String Yell = String(this, "yell", "Y", &ValidateKey, "Yell key.");
            String ZoomIn = String(this, "zoom_in", "ADD", &ValidateKey, "Zoom in automap key.");
            String ZoomOut = String(this, "zoom_out", "SUBTRACT", &ValidateKey, "Zoom out automap key.");

            String QuickSave = String(this, "quick_save", "F5", &ValidateKey, "Quick save key");
            String QuickLoad = String(this, "quick_load", "F9", &ValidateKey, "Quick load key");
            String History = String(this, "history", "H", &ValidateKey, "History book key");
            String Stats = String(this, "stats", "C", &ValidateKey, "Stats tab key");
            String Skills = String(this, "skills", "S", &ValidateKey, "Skills tab key");
            String Inventory = String(this, "inventory", "I", &ValidateKey, "Inventory tab key");
            String Awards = String(this, "awards", "A", &ValidateKey, "Stats tab key");
            String NewGame = String(this, "new_game", "N", &ValidateKey, "New Game menu key");
            String SaveGame = String(this, "save_game", "S", &ValidateKey, "Save Game menu key");
            String LoadGame = String(this, "load_game", "L", &ValidateKey, "Load Game menu key");
            String ExitGame = String(this, "exit_game", "Q", &ValidateKey, "Exit Game key");
            String ReturnToGame = String(this, "return_to_game", "R", &ValidateKey, "Return to Game mode key");
            String Controls = String(this, "controls", "C", &ValidateKey, "Controls change menu key");
            String Options = String(this, "options", "O", &ValidateKey, "Options menu key");
            String Credits = String(this, "credits", "C", &ValidateKey, "Credits menu key");
            String Clear = String(this, "clear", "C", &ValidateKey, "Clear button in New Party Creation menu");
            String Return = String(this, "return", "RETURN", &ValidateKey, "Ok button in New Party Creation menu");
            String Minus = String(this, "minus", "SUBTRACT", &ValidateKey, "Minus button in New Party Creation menu");
            String Plus = String(this, "plus", "ADD", &ValidateKey, "Plus button in New Party Creation menu");
            String Yes = String(this, "yes", "Y", &ValidateKey, "Yes answer key");
            String No = String(this, "no", "N", &ValidateKey, "No answer key");
            String Rest8Hours = String(this, "rest_8_hours", "R", &ValidateKey, "Rest for 8 hours key in Rest menu");
            String WaitTillDawn = String(this, "wait_till_dawn", "D", &ValidateKey, "Wait till dawn key in Rest menu");
            String WaitHour = String(this, "wait_hour", "H", &ValidateKey, "Wait hour in Rest menu");
            String Wait5Minutes = String(this, "wait_5_minutes", "M", &ValidateKey, "Wait 5 minutes in Rest menu");
            String Screenshot = String(this, "screenshot", "F2", &ValidateKey, "Make screenshot key");
            String Console = String(this, "console", "~", &ValidateKey, "Open console key (currently it is opening debug menu)");
            String ToggleMouseGrab = String(this, "toggle_mouse_grab", "F1", &ValidateKey, "Toggle mouse grab key");
            String ToggleBorderless = String(this, "toggle_borderless", "F3", &ValidateKey, "Toggle window borderless key");
            String ToggleFullscreen = String(this, "toggle_fullscreen", "F4", &ValidateKey, "Toggle window fullscreen key");
            String ToggleResizable = String(this, "toggle_resizable", "F6", &ValidateKey, "Toggle window resizable key");
            String CycleFilter = String(this, "cycle_filter", "F7", &ValidateKey, "Cycle rescale filter modes key");
            String ReloadShaders = String(this, "reload_shaders", "BACKSPACE", &ValidateKey, "Reload shaders key");
            String SelectChar1 = String(this, "select_char_1", "1", &ValidateKey, "Select 1 character key");
            String SelectChar2 = String(this, "select_char_2", "2", &ValidateKey, "Select 2 character key");
            String SelectChar3 = String(this, "select_char_3", "3", &ValidateKey, "Select 3 character key");
            String SelectChar4 = String(this, "select_char_4", "4", &ValidateKey, "Select 4 character key");
            String SelectNPC1 = String(this, "select_npc_1", "5", &ValidateKey, "Select 1 hireling key");
            String SelectNPC2 = String(this, "select_npc_2", "6", &ValidateKey, "Select 2 hireling key");
            String DialogUp = String(this, "dialog_up", "UP", &ValidateKey, "Dialog up key");
            String DialogDown = String(this, "dialog_down", "DOWN", &ValidateKey, "Dialog down key");
            String DialogLeft = String(this, "dialog_left", "LEFT", &ValidateKey, "Dialog left key");
            String DialogRight = String(this, "dialog_right", "RIGHT", &ValidateKey, "Dialog right key");
            String DialogSelect = String(this, "dialog_select", "RETURN", &ValidateKey, "Dialog select key");

            String Escape = String(this, "escape", "Escape", &ValidateKey, "Escape key");

         private:
            static std::string ValidateKey(std::string key) {
                // TODO [config]: validation, maybe via GameKey::displayNames
                return key;
            }
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

            String Title = String(this, "title", "World of Might and Magic", &ValidateTitle, "Game window title.");

            Int Display = Int(this, "display", 0,
                              "Display number as exposed by SDL. "
                              "Order is platform-specific, e.g. on windows 0 is main display");

            Int Mode = Int(this, "mode", ConfigWindowMode, &ValidateMode,
                           "Window mode. Use 0 for windowed, 1 for borderless windowed, 2 for fullscreen, 3 for borderless (fake) fullscreen.");

            Int PositionX = Int(this, "position_x", -1, &ValidatePosition,
                                "Game window x position in display coordinates. Use -1 for centered.");
            Int PositionY = Int(this, "position_y", -1, &ValidatePosition,
                                "Game window y position in display coordinates. Use -1 for centered.");

            Int Width = Int(this, "width", 640, &ValidateWidth, "Window width.");
            Int Height = Int(this, "height", 480, &ValidateHeight, "Window height.");

            Bool MouseGrab = Bool(this, "mouse_grab", true, "Restrict mouse movement to game window when it's in focus.");

            Bool Resizable = ConfigValue<bool>(this, "resizable", true, "Make window resizable by user or not.");

         private:
             static std::string ValidateTitle(std::string title) {
                 if (title.empty())
                     title = "World of Might and Magic";

                 return title;
             }
             static int ValidateMode(int mode) {
                 if (mode < 0 || mode > 3)
                     mode = 0;

                 return mode;
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
        const std::string config_file = "womm.ini";
        Logger *logger = nullptr;
    };

}  // namespace Application
