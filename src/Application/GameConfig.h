#pragma once
#include <memory>
#include <string>

#include "Engine/CommandLine.h"

// TODO [config]: Clean up these defines
#define GAME_FLAGS_1_01                         0x01     // do not render terrain / bmodels(odm), lightmaps render bmodels in white(blv)
#define GAME_FLAGS_1_02                         0x02     // only 0th lod, no lightning
#define GAME_FLAGS_1_04                         0x04     // draw lightmaps / decals debug outlines
#define GAME_FLAGS_1_DRAW_BLV_DEBUGS            0x08     // draw portal debug outlines
#define GAME_FLAGS_1_40                         0x40     //
#define GAME_FLAGS_1_FORCE_LEGACY_PROJECTION    0x80     // force camera / culling / projection mode : BLV
#define GAME_FLAGS_1_400                        0x400    //
#define GAME_FLAGS_1_800                        0x800    //
#define GAME_FLAGS_1_TORCHLIGHT                 0x1000   // do Torchlight lightning
#define GAME_FLAGS_1_40000_sw_alter_palettes    0x40000  // software render alter palettes
#define GAME_FLAGS_1_HUGE_BLOODSPLATS           0x80000  // huge bloodsplats on bosses

#define GAME_FLAGS_2_FORCE_REDRAW               0x01     // force redraw this frame
#define GAME_FLAGS_2_SATURATE_LIGHTMAPS         0x02     // alter lightmaps saturation
#define GAME_FLAGS_2_ALLOW_DYNAMIC_BRIGHTNESS   0x04     // allows dynamic brightness calculation on lightmaps
#define GAME_FLAGS_2_ALTER_GRAVITY              0x08     // underwater effect, no gravity (for out15.odm - Shoals)
#define GAME_FLAGS_2_TARGETING_MODE             0x10     // spell targeting, current cursor is "MICON2" (crosshair)
#define GAME_FLAGS_2_DRAW_BLOODSPLATS           0x20

using Engine_::CommandLine;

namespace Application {
    class GameConfig {
     public:
        explicit GameConfig(const std::string &command_line);
        void Startup();
        void DefaultConfiguration();
        void LoadConfiguration();
        void SaveConfiguration();

        template <class T>
        class ConfigValue {
         public:
            ConfigValue(std::string n, T d, T(*v)(T) = nullptr) : name(n), defValue(d), value(d), validator(v) {}

            inline std::string Name() {
                return name;
            }

            inline T Default() {
                return defValue;
            }

            inline T Get() {
                return value;
            }

            inline void Set(T val) {
                if (validator)
                    value = validator(val);
                else
                    value = val;
            }

            inline void Reset() {
                value = defValue;
            }

            inline void Toggle() {
                value = !value;
            }

         private:
             std::string name;
             T value;
             T defValue;
             T(*validator)(T);
        };

        const std::string config_file = "womm.ini";
        std::shared_ptr<CommandLine> command_line;

        class Debug {
         public:
            const std::string section_name = "debug";

            void Default();
            void Load();
            void Save();

            /** Enable all available spells for each character in spellbook bypassing all class restrictions. Currently also all skills will behave like they are on GM level. */
            ConfigValue<bool> AllMagic = ConfigValue<bool>("all_magic", false);
            ConfigValue<bool> InfiniteFood = ConfigValue<bool>("infinite_food", false);
            ConfigValue<bool> InfiniteGold = ConfigValue<bool>("infinite_gold", false);

            /** Debug lightmap and decals outlines. */
            ConfigValue<bool> LightmapDecals = ConfigValue<bool>("lightmap_decals", false);

            /** Draw BLV portal frames. */
            ConfigValue<bool> PortalOutlines = ConfigValue<bool>("portal_outlines", false);
            ConfigValue<bool> Terrain = ConfigValue<bool>("terrain", false);
            ConfigValue<bool> TownPortal = ConfigValue<bool>("town_portal", false);

            /** Increase party movement speed by 12x. Most likely you want to use that option with no_damage option enabled as collision physics often will shoot you in the air. */
            ConfigValue<bool> TurboSpeed = ConfigValue<bool>("turbo_speed", false);

            /** Game will behave like spell wizard eye is casted and it will never expire. */
            ConfigValue<bool> WizardEye = ConfigValue<bool>("wizard_eye", false);
            ConfigValue<bool> ShowFPS = ConfigValue<bool>("show_fps", false);

            /** Face pointed by mouse will flash with red for buildings or green for indoor. */
            ConfigValue<bool> ShowPickedFace = ConfigValue<bool>("show_picked_face", false);

            /** Skip intro movie on startup */
            ConfigValue<bool> NoIntro = ConfigValue<bool>("no_intro", false);

            /** Skip 3do logo on startup */
            ConfigValue<bool> NoLogo = ConfigValue<bool>("no_logo", false);

            /** Don't play any sounds. Currently it doesn't affect in-house movies. */
            ConfigValue<bool> NoSound = ConfigValue<bool>("no_sound", false);

            /** Don't play any movies. */
            ConfigValue<bool> NoVideo = ConfigValue<bool>("no_video", false);

            /** Disable all actors */
            ConfigValue<bool> NoActors = ConfigValue<bool>("no_actors", false);

            /** Disable all incoming damage to party. */
            ConfigValue<bool> NoDamage = ConfigValue<bool>("no_damage", false);

            /** Disable all decorations */
            ConfigValue<bool> NoDecorations = ConfigValue<bool>("no_decorations", false);

            /** Disable Margareth's tour messages on Emerald Island. */
            ConfigValue<bool> NoMargareth = ConfigValue<bool>("no_margareth", false);
            ConfigValue<bool> VerboseLogging = ConfigValue<bool>("verbose_logging", false);

         private:
        };

        Debug debug;

        class Gameplay {
         public:
            const std::string section_name = "gameplay";

            void Default();
            void Load();
            void Save();

            ConfigValue<int> MaxFlightHeight = ConfigValue<int>("max_flight_height", 4000, &ValidateMaxFlightHeight);

            /** Artifact limit after which artifacts are no longer generated in loot. 0 - disable limit. */
            ConfigValue<int> ArtifactLimit = ConfigValue<int>("artifact_limit", 13, &ValidateArtifactLimit);

            /** Maximum depth for item pickup / opening chests / activating levers / etc with a mouse. */
            ConfigValue<float> MouseInteractionDepth = ConfigValue<float>("mouse_interaction_depth", 512.0f, &ValidateInteractionDepth);

            /** Maximum depth for item pickup / opening chests / activating levers / etc with a keyboard (by pressing space). */
            ConfigValue<float> KeyboardInteractionDepth = ConfigValue<float>("keyboard_interaction_depth", 512.0f, &ValidateInteractionDepth);

            /** Maximum depth at which right clicking on a monster produces a popup.
              * Also somehow this is the max depth for the souldrinker spell. */
            ConfigValue<float> MouseInfoDepthIndoor = ConfigValue<float>("mouse_info_depth_indoor", 16192.0f, &ValidateInteractionDepth);
            ConfigValue<float> MouseInfoDepthOutdoor = ConfigValue<float>("mouse_info_depth_outdoor", 12800.0f, &ValidateInteractionDepth); // That's 25 * 512, so 25 cells.

            /** Max depth for ranged attacks and ranged spells. It's impossible to target monsters that are father away
              * than this value. Incidentally this is also the depth at which status bar tips are displayed on mouse over. */
            ConfigValue<float> RangedAttackDepth = ConfigValue<float>("ranged_attack_depth", 5120.0f, &ValidateRangedAttackDepth);

            /** Maximum allowed slack for point-inside-a-polygon checks when calculating floor z level.
              * This is needed because there are actual holes in level geometry sometimes, up to several units wide. */
            ConfigValue<int> FloorChecksEps = ConfigValue<int>("floor_checks_eps", 3, &ValidateFloorChecksEps);

            /** Show unidentified items in green mask in inventory, otherwise vanilla behaviour when green mask applied in shops only. */
            ConfigValue<bool> ShowUndentifiedItem = ConfigValue<bool>("show_unidentified_item", false);

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
                if (depth <= 64.0f)
                    return 64.0f;
                else if (depth > 16192.0f)
                    return 16192.0f;

                return depth;
            }
            static float ValidateRangedAttackDepth(float depth) {
                if (depth <= 64.0f)
                    return 64.0f;
                else if (depth > 16192.0f)
                    return 16192.0f;

                return depth;
            }
            static int ValidateFloorChecksEps(int eps) {
                if (eps < 0)
                    return 0;
                else if (eps > 10)
                    return 10;

                return eps;
            }
        };

        Gameplay gameplay;

        class Graphics {
         public:
            const std::string section_name = "graphics";

            void Default();
            void Load();
            void Save();

            ConfigValue<std::string> Renderer = ConfigValue<std::string>("renderer", "OpenGL", &ValidateRenderer);
            ConfigValue<bool> BloodSplats = ConfigValue<bool>("bloodsplats", true);
            ConfigValue<bool> ColoredLights = ConfigValue<bool>("colored_lights", true);
            ConfigValue<bool> Tinting = ConfigValue<bool>("tinting", false);

            /** use low-resolution bitmaps from HWL file instead of hi-resolution ones from LOD */
            ConfigValue<bool> HWLBitmaps = ConfigValue<bool>("hwl_bitmaps", false);

            /** use low-resolution sprites from HWL file instead of hi-resolution ones from LOD */
            ConfigValue<bool> HWLSprites = ConfigValue<bool>("hwl_sprites", false);

            /** not in use? */
            ConfigValue<int> Gamma = ConfigValue<int>("gamma", 4, &ValidateGamma);
            ConfigValue<bool> Snow = ConfigValue<bool>("snow", false);
            ConfigValue<bool> SeasonsChange = ConfigValue<bool>("seasons_change", true);
            ConfigValue<bool> Specular = ConfigValue<bool>("specular", false);
            ConfigValue<bool> Fog = ConfigValue<bool>("fog", false);

            inline void ToggleExtendedDrawDistance() { extended_draw_distance = !extended_draw_distance; }

            // TODO [config]: remove this and move distance limits to config
            bool extended_draw_distance;    // 2.5x draw distance

         private:
            static std::string ValidateRenderer(std::string renderer) {
                if (renderer != "OpenGL" && renderer != "DirectDraw")
                    renderer = "OpenGL";

                return renderer;
            }
            static int ValidateGamma(int level) {
                if (level < 0)
                    return 0;
                else if (level > 9)
                    return 9;

                return level;
            }
        };

        Graphics graphics;

        class Keybindings {
         public:
            const std::string section_name = "keybindings";

            void Default();
            void Load();
            void Save();

            ConfigValue<std::string> Forward = ConfigValue<std::string>("forward", "UP", &ValidateKey);
            ConfigValue<std::string> Backward = ConfigValue<std::string>("backward", "DOWN", &ValidateKey);
            ConfigValue<std::string> Left = ConfigValue<std::string>("left", "LEFT", &ValidateKey);
            ConfigValue<std::string> Right = ConfigValue<std::string>("right", "RIGHT", &ValidateKey);
            ConfigValue<std::string> Attack = ConfigValue<std::string>("attack", "A", &ValidateKey);
            ConfigValue<std::string> CastReady = ConfigValue<std::string>("cast_ready", "S", &ValidateKey);
            ConfigValue<std::string> Yell = ConfigValue<std::string>("yell", "Y", &ValidateKey);
            ConfigValue<std::string> Jump = ConfigValue<std::string>("jump", "X", &ValidateKey);
            ConfigValue<std::string> Combat = ConfigValue<std::string>("combat", "RETURN", &ValidateKey);
            ConfigValue<std::string> EventTrigger = ConfigValue<std::string>("event_trigger", "SPACE", &ValidateKey);
            ConfigValue<std::string> Cast = ConfigValue<std::string>("cast", "C", &ValidateKey);
            ConfigValue<std::string> Pass = ConfigValue<std::string>("pass", "B", &ValidateKey);
            ConfigValue<std::string> Quest = ConfigValue<std::string>("pass", "Q", &ValidateKey);
            ConfigValue<std::string> CharCycle = ConfigValue<std::string>("char_cycle", "TAB", &ValidateKey);
            ConfigValue<std::string> QuickReference = ConfigValue<std::string>("quick_reference", "Q", &ValidateKey);
            ConfigValue<std::string> Rest = ConfigValue<std::string>("rest", "R", &ValidateKey);
            ConfigValue<std::string> TimeCalendar = ConfigValue<std::string>("time_calendar", "T", &ValidateKey);
            ConfigValue<std::string> AutoNotes = ConfigValue<std::string>("auto_notes", "N", &ValidateKey);
            ConfigValue<std::string> MapBook = ConfigValue<std::string>("map_book", "M", &ValidateKey);
            ConfigValue<std::string> LookUp = ConfigValue<std::string>("look_up", "PAGE DOWN", &ValidateKey);
            ConfigValue<std::string> LookDown = ConfigValue<std::string>("look_down", "DELETE", &ValidateKey);
            ConfigValue<std::string> CenterView = ConfigValue<std::string>("center_view", "END", &ValidateKey);
            ConfigValue<std::string> ZoomIn = ConfigValue<std::string>("zoom_in", "ADD", &ValidateKey);
            ConfigValue<std::string> ZoomOut = ConfigValue<std::string>("zoom_out", "SUBTRACT", &ValidateKey);
            ConfigValue<std::string> FlyUp = ConfigValue<std::string>("fly_up", "PAGE UP", &ValidateKey);
            ConfigValue<std::string> FlyDown = ConfigValue<std::string>("fly_down", "INSERT", &ValidateKey);
            ConfigValue<std::string> Land = ConfigValue<std::string>("land", "HOME", &ValidateKey);
            ConfigValue<std::string> AlwaysRun = ConfigValue<std::string>("always_run", "U", &ValidateKey);
            ConfigValue<std::string> StepLeft = ConfigValue<std::string>("step_left", "L BRACKET", &ValidateKey);
            ConfigValue<std::string> StepRight = ConfigValue<std::string>("step_right", "R BRACKET", &ValidateKey);

         private:
            static std::string ValidateKey(std::string key) {
                // TODO [config]: validation, maybe via GameKey::displayNames
                return key;
            }
        };

        Keybindings keybindings;

        class Settings {
         public:
            const std::string section_name = "settings";

            void Default();
            void Load();
            void Save();

            /** true - run, false - walk */
            ConfigValue<bool> AlwaysRun = ConfigValue<bool>("always_run", true);

            /** Horizontal view flip when exiting buildings */
            ConfigValue<bool> FlipOnExit = ConfigValue<bool>("flip_on_exit", false);

            /** Show hits status in status bar */
            ConfigValue<bool> ShowHits = ConfigValue<bool>("show_hits", true);

            /** Music volume level */
            ConfigValue<int> MusicLevel = ConfigValue<int>("music_level", 3, &ValidateLevel);

            /** Sound volume level */
            ConfigValue<int> SoundLevel = ConfigValue<int>("sound_level", 4, &ValidateLevel);

            /** Voice volume level */
            ConfigValue<int> VoiceLevel = ConfigValue<int>("voice_level", 5, &ValidateLevel);

            /** Discrete turn speed, 0 - smooth, x16 - 64 degress turn, x32 - 128 degress turn.
              * Only smooth is usable on modern machines. */
            ConfigValue<float> TurnSpeed = ConfigValue<float>("turn_speed", 0.0f, &ValidateTurnSpeed);

            /** Discrete vertical turn speed (mm7 default is 25). */
            ConfigValue<int> VerticalTurnSpeed = ConfigValue<int>("vertical_turn_speed", 25, &ValidateVerticalTurnSpeed);

            /** Party footstep's sound while moving. */
            ConfigValue<bool> WalkSound = ConfigValue<bool>("walk_sound", true);

         private:
            static int ValidateLevel(int level) {
                if (level < 0)
                    return 0;
                else if (level > 9)
                    return 9;

                return level;
            }
            static int ValidateVerticalTurnSpeed(int speed) {
                if (speed <= 0)
                    return 1;
                else if (speed > 128)
                    return 128;

                return speed;
            }
            static float ValidateTurnSpeed(float speed) {
                if (speed < 0.0f)
                    return 0.0f;
                else if (speed > 1024.0f)
                    return 1024.0f;

                return speed;
            }
        };

        Settings settings;

        class Window {
         public:
            const std::string section_name = "window";

            void Default();
            void Load();
            void Save();

            ConfigValue<std::string> Title = ConfigValue<std::string>("title", "World of Might and Magic", &ValidateTitle);

            /**
             * Borderless mode.
             *
             * It removes window borders. For fullscreen mode instead of going into traditional exclusive fullscreen mode it goes into fake fullscreen mode.
             * Fake fullscreen mode reposition window top-left corner to 0,0 ignoring window position coordinates.
             * And also resize window to display size.
             */
            ConfigValue<bool> Borderless = ConfigValue<bool>("borderless", false);

            /** Window mode. 1 - fullscreen, 0 - window */
            ConfigValue<bool> Fullscreen = ConfigValue<bool>("fullscreen", false);

            /** Display number as exposed by SDL. Order is platform-specific, e.g. on windows 0 is main display */
            ConfigValue<int> Display = ConfigValue<int>("display", 0);

            /** Coordinates in pixels for position of left-top window corner. -1 is window centered on this axis. */
            ConfigValue<int> PositionX = ConfigValue<int>("position_x", -1, &ValidatePosition);
            ConfigValue<int> PositionY = ConfigValue<int>("position_y", -1, &ValidatePosition);

            /** Window size in pixels. */
            ConfigValue<int> Width = ConfigValue<int>("width", 640, &ValidateWidth);
            ConfigValue<int> Height = ConfigValue<int>("height", 480, &ValidateHeight);

            /** Don't grab mouse. When false, you cannot move the mouse outside the game window while it is in focus. */
            ConfigValue<bool> MouseGrab = ConfigValue<bool>("mouse_grab", true);

         private:
             static std::string ValidateTitle(std::string title) {
                 if (title.empty())
                     title = "World of Might and Magic";

                 return title;
             }
             static int ValidatePosition(int position) {
                if (position < -1)
                    return -1;

                return position;
            }
            static int ValidateWidth(int dimension) {
                if (dimension < 640)
                    return 640;

                return dimension;
            }
            static int ValidateHeight(int dimension) {
                if (dimension < 480)
                    return 480;

                return dimension;
            }
        };

        Window window;

        // TODO [config]: Clean up this block
        inline bool Flag1_1() const { return flags1 & GAME_FLAGS_1_01; }
        inline bool Flag1_2() const { return flags1 & GAME_FLAGS_1_02; }
        inline bool ForceLegacyProjection() const { return flags1 & GAME_FLAGS_1_FORCE_LEGACY_PROJECTION; }
        inline bool NoHugeBloodsplats() const { return flags1 & GAME_FLAGS_1_HUGE_BLOODSPLATS; }
        inline bool AlterPalettes() const { return flags1 & GAME_FLAGS_1_40000_sw_alter_palettes; }
        inline bool DrawBlvDebugs() const { return flags1 & GAME_FLAGS_1_DRAW_BLV_DEBUGS; }
        inline bool TorchlightEffect() const { return flags1 & GAME_FLAGS_1_TORCHLIGHT; }
        inline bool CanSaturateFaces() const { return flags2 & GAME_FLAGS_2_SATURATE_LIGHTMAPS; }
        void SetSaturateFaces(bool saturate) {
            if (saturate)
                flags2 |= GAME_FLAGS_2_SATURATE_LIGHTMAPS;
            else
                flags2 &= ~GAME_FLAGS_2_SATURATE_LIGHTMAPS;
        }
        void SetForceRedraw(bool redraw) {
            if (redraw)
                flags2 |= GAME_FLAGS_2_FORCE_REDRAW;
            else
                flags2 &= ~GAME_FLAGS_2_FORCE_REDRAW;
        }
        inline bool ForceRedraw() const { return flags2 & GAME_FLAGS_2_FORCE_REDRAW; }
        inline bool AllowDynamicBrigtness() const { return flags2 & GAME_FLAGS_2_ALLOW_DYNAMIC_BRIGHTNESS; }

        // GAME_FLAGS_1_*
        int flags1 = GAME_FLAGS_1_40 | GAME_FLAGS_1_800;

        // GAME_FLAGS_2_*
        int flags2 = GAME_FLAGS_2_ALLOW_DYNAMIC_BRIGHTNESS | GAME_FLAGS_2_DRAW_BLOODSPLATS | GAME_FLAGS_2_SATURATE_LIGHTMAPS;

     private:
    };
}  // namespace Application
