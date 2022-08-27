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
        void DefaultConfiguration();
        void LoadConfiguration();
        void SaveConfiguration();

        bool SetCustomBool(std::string section, std::string key, bool value);
        bool SetCustomInteger(std::string section, std::string key, int value);
        bool SetCustomFloat(std::string section, std::string key, float value);
        bool SetCustomString(std::string section, std::string key, std::string value);

        bool GetCustomBool(std::string section, std::string key, bool def_value);
        int GetCustomInteger(std::string section, std::string key, int def_value);
        float GetCustomFloat(std::string section, std::string key, float def_value);
        std::string GetCustomString(std::string section, std::string key, std::string def_value);

        const std::string config_file = "womm.ini";
        std::shared_ptr<CommandLine> command_line;

        class Debug {
         public:
            const std::string section_name = "debug";

            void Default();
            void Load();
            void Save();

            inline bool DefaultAllMagic() {
                return false;
            }
            inline void SetAllMagic(bool is_enabled) {
                this->all_magic = is_enabled;
            }
            inline bool GetAllMagic() {
                return this->all_magic;
            }
            inline void ToggleAllMagic() {
                this->all_magic = !this->all_magic;
            }

            inline bool DefaultInfiniteFood() {
                return false;
            }
            inline void SetInfiniteFood(bool is_enabled) {
                this->infinite_food = is_enabled;
            }
            inline bool GetInfiniteFood() {
                return this->infinite_food;
            }
            inline void ToggleInfiniteFood() {
                this->infinite_food = !this->infinite_food;
            }

            inline bool DefaultInfiniteGold() {
                return false;
            }
            inline void SetInfiniteGold(bool is_enabled) {
                this->infinite_gold = is_enabled;
            }
            inline bool GetInfiniteGold() {
                return this->infinite_gold;
            }
            inline void ToggleInfiniteGold() {
                this->infinite_gold = !this->infinite_gold;
            }

            inline bool DefaultLightmapDecals() {
                return false;
            }
            inline void SetLightmapDecals(bool is_enabled) {
                this->lightmaps_decals = is_enabled;
            }
            inline bool GetLightmapDecals() {
                return this->lightmaps_decals;
            }
            inline void ToggleLightmapDecals() {
                this->lightmaps_decals = !this->lightmaps_decals;
            }

            inline bool DefaultPortalOutlines() {
                return false;
            }
            inline void SetPortalOutlines(bool is_enabled) {
                this->portal_outlines = is_enabled;
            }
            inline bool GetPortalOutlines() {
                return this->portal_outlines;
            }
            inline void TogglePortalOutlines() {
                this->portal_outlines = !this->portal_outlines;
            }

            inline bool DefaultTerrain() {
                return false;
            }
            inline void SetTerrain(bool is_enabled) {
                this->terrain = is_enabled;
            }
            inline bool GetTerrain() {
                return this->terrain;
            }
            inline void ToggleTerrain() {
                this->terrain = !this->terrain;
            }

            inline bool DefaultTownPortal() {
                return false;
            }
            inline void SetTownPortal(bool is_enabled) {
                this->town_portal = is_enabled;
            }
            inline bool GetTownPortal() {
                return this->town_portal;
            }
            inline void ToggleTownPortal() {
                this->town_portal = !this->town_portal;
            }

            inline bool DefaultTurboSpeed() {
                return false;
            }
            inline void SetTurboSpeed(bool is_enabled) {
                this->turbo_speed = is_enabled;
            }
            inline bool GetTurboSpeed() {
                return this->turbo_speed;
            }
            inline void ToggleTurboSpeed() {
                this->turbo_speed = !this->turbo_speed;
            }

            inline bool DefaultWizardEye() {
                return false;
            }
            inline void SetWizardEye(bool is_enabled) {
                this->wizard_eye = is_enabled;
            }
            inline bool GetWizardEye() {
                return this->wizard_eye;
            }
            inline void ToggleWizardEye() {
                this->wizard_eye = !this->wizard_eye;
            }

            inline bool DefaultShowFPS() {
                return false;
            }
            inline void SetShowFPS(bool is_enabled) {
                this->show_fps = is_enabled;
            }
            inline bool GetShowFPS() {
                return this->show_fps;
            }
            inline void ToggleShowFPS() {
                this->show_fps = !this->show_fps;
            }

            inline bool DefaultShowPickedFace() {
                return false;
            }
            inline void SetShowPickedFace(bool is_enabled) {
                this->show_picked_face = is_enabled;
            }
            inline bool GetShowPickedFace() {
                return this->show_picked_face;
            }
            inline void ToggleShowPickedFace() {
                this->show_picked_face = !this->show_picked_face;
            }

            inline bool DefaultNoIntro() {
                return false;
            }
            inline void SetNoIntro(bool is_enabled) {
                this->no_intro = is_enabled;
            }
            inline bool GetNoIntro() {
                return this->no_intro;
            }
            inline void ToggleNoIntro() {
                this->no_intro = !this->show_fps;
            }

            inline bool DefaultNoLogo() {
                return false;
            }
            inline void SetNoLogo(bool is_enabled) {
                this->no_logo = is_enabled;
            }
            inline bool GetNoLogo() {
                return this->no_logo;
            }
            inline void ToggleNoLogo() {
                this->no_logo = !this->no_logo;
            }

            inline bool DefaultNoSound() {
                return false;
            }
            inline void SetNoSound(bool is_enabled) {
                this->no_sound = is_enabled;
            }
            inline bool GetNoSound() {
                return this->no_sound;
            }
            inline void ToggleNoSound() {
                this->no_sound = !this->no_sound;
            }

            inline bool DefaultNoVideo() {
                return false;
            }
            inline void SetNoVideo(bool is_enabled) {
                this->no_video = is_enabled;
            }
            inline bool GetNoVideo() {
                return this->no_video;
            }
            inline void ToggleNoVideo() {
                this->no_video = !this->no_video;
            }

            inline bool DefaultNoActors() {
                return false;
            }
            inline void SetNoActors(bool is_enabled) {
                this->no_actors = is_enabled;
            }
            inline bool GetNoActors() {
                return this->no_actors;
            }
            inline void ToggleNoActors() {
                this->no_actors = !this->no_actors;
            }

            inline bool DefaultNoDecorations() {
                return false;
            }
            inline void SetNoDecorations(bool is_enabled) {
                this->no_decorations = is_enabled;
            }
            inline bool GetNoDecorations() {
                return this->no_decorations;
            }
            inline void ToggleNoDecorations() {
                this->no_decorations = !this->no_decorations;
            }

            inline bool DefaultNoDamage() {
                return false;
            }
            inline void SetNoDamage(bool is_enabled) {
                this->no_damage = is_enabled;
            }
            inline bool GetNoDamage() {
                return this->no_damage;
            }
            inline void ToggleNoDamage() {
                this->no_damage = !this->no_damage;
            }

            inline bool DefaultNoMargareth() {
                return false;
            }
            inline void SetNoMargareth(bool is_enabled) {
                this->no_margareth = is_enabled;
            }
            inline bool GetNoMargareth() {
                return this->no_margareth;
            }
            inline void ToggleNoMargareth() {
                this->no_margareth = !this->no_margareth;
            }

            inline bool DefaultVerboseLogging() {
                return false;
            }
            inline void SetVerboseLogging(bool is_enabled) {
                this->verbose_logging = is_enabled;
            }
            inline bool GetVerboseLogging() {
                return this->verbose_logging;
            }
            inline void ToggleVerboseLogging() {
                this->verbose_logging = !this->verbose_logging;
            }

         private:
            /** Enable all available spells for each character in spellbook bypassing all class restrictions. Currently also all skills will behave like they are on GM level. */
            bool all_magic;

            bool infinite_food;
            bool infinite_gold;

            /** Debug lightmap and decals outlines. */
            bool lightmaps_decals;

            /** Draw BLV portal frames. */
            bool portal_outlines;

            bool terrain;
            bool town_portal;

            /** Increase party movement speed by 12x. Most likely you want to use that option with no_damage option enabled as collision physics often will shoot you in the air. */
            bool turbo_speed;

            /** Game will behave like spell wizard eye is casted and it will never expire. */
            bool wizard_eye;

            bool show_fps;

            /** Face pointed by mouse will flash with red for buildings or green for indoor. */
            bool show_picked_face;

            /** Disable all actors */
            bool no_actors;

            /** Disable all incoming damage to party. */
            bool no_damage;

            /** Disable all decorations */
            bool no_decorations;

            /** Skip intro movie on startup */
            bool no_intro;

            /** Skip 3do logo on startup */
            bool no_logo;

            /** Disable Margareth's tour messages on Emerald Island. */
            bool no_margareth;

            /** Don't play any sounds. Currently it doesn't affect in-house movies. */
            bool no_sound;

            /** Don't play any movies. */
            bool no_video;

            bool verbose_logging;
        };

        Debug debug;

        class Gameplay {
         public:
            const std::string section_name = "gameplay";

            void Default();
            void Load();
            void Save();

            inline int DefaultMaxFlightHeight() {
                return 4000;
            }
            inline void SetMaxFlightHeight(int max_flight_height) {
                this->max_flight_height = ValidateMaxFlightHeight(max_flight_height);
            }
            inline int GetMaxFlightHeight() {
                return this->max_flight_height;
            }

            inline int DefaultArtifactLimit() {
                return 13;
            }
            inline void SetArtifactLimit(int artifact_limit) {
                this->artifact_limit = ValidateArtifactLimit(artifact_limit);
            }
            inline int GetArtifactLimit() {
                return this->artifact_limit;
            }

            inline float DefaultMouseInteractionDepth() {
                return 512.0f;
            }
            inline void SetMouseInteractionDepth(float mouse_interaction_depth) {
                this->mouse_interaction_depth = ValidateMouseInteractionDepth(mouse_interaction_depth);
            }
            inline float GetMouseInteractionDepth() {
                return this->mouse_interaction_depth;
            }

            inline float DefaultKeyboardInteractionDepth() {
                return 512.0f;
            }
            inline void SetKeyboardInteractionDepth(float keyboard_interaction_depth) {
                this->keyboard_interaction_depth = ValidateKeyboardInteractionDepth(keyboard_interaction_depth);
            }
            inline float GetKeyboardInteractionDepth() {
                return this->keyboard_interaction_depth;
            }

            inline float DefaultMouseInfoDepthIndoor() {
                return 16192.0f;
            }
            inline void SetMouseInfoDepthIndoor(float mouse_info_depth_indoor) {
                this->mouse_info_depth_indoor = ValidateMouseInfoDepthIndoor(mouse_info_depth_indoor);
            }
            inline float GetMouseInfoDepthIndoor() {
                return this->mouse_info_depth_indoor;
            }

            inline float DefaultMouseInfoDepthOutdoor() {
                return 12800.0f;
            }
            inline void SetMouseInfoDepthOutdoor(float mouse_info_depth_outdoor) {
                this->mouse_info_depth_outdoor = ValidateMouseInfoDepthOutdoor(mouse_info_depth_outdoor);
            }
            inline float GetMouseInfoDepthOutdoor() {
                return this->mouse_info_depth_outdoor;
            }

            inline float DefaultRangedAttackDepth() {
                return 5120.0f;
            }
            inline void SetRangedAttackDepth(float ranged_attack_depth) {
                this->ranged_attack_depth = ValidateRangedAttackDepth(ranged_attack_depth);
            }
            inline float GetRangedAttackDepth() {
                return this->ranged_attack_depth;
            }

            inline int DefaultFloorChecksEps() {
                return 3;
            }
            inline void SetFloorChecksEps(int floor_checks_eps) {
                this->floor_checks_eps = ValidateFloorChecksEps(floor_checks_eps);
            }
            inline int GetFloorChecksEps() {
                return this->floor_checks_eps;
            }

            inline bool DefaultShowUndentifiedItem() {
                return false;
            }
            inline void SetShowUndentifiedItem(bool is_enabled) {
                this->show_unidentified_item = is_enabled;
            }
            inline bool GetShowUndentifiedItem() const {
                return this->show_unidentified_item;
            }
            inline void ToggleShowUndentifiedItem() {
                this->show_unidentified_item = !this->show_unidentified_item;
            }

         private:
            inline int ValidateMaxFlightHeight(int max_flight_height) {
                if (max_flight_height <= 0 || max_flight_height > 16192)
                    return DefaultMaxFlightHeight();

                return max_flight_height;
            }
            inline int ValidateArtifactLimit(int artifact_limit) {
                if (artifact_limit < 0)
                    return DefaultArtifactLimit();

                return artifact_limit;
            }

            inline float ValidateMouseInteractionDepth(float depth) {
                if (depth <= 0.0f || depth > 16192.0f)
                    return DefaultMouseInteractionDepth();

                return depth;
            }
            inline float ValidateKeyboardInteractionDepth(float depth) {
                if (depth <= 0.0f || depth > 16192.0f)
                    return DefaultKeyboardInteractionDepth();

                return depth;
            }
            inline float ValidateMouseInfoDepthIndoor(float depth) {
                if (depth <= 0.0f || depth > 16192.0f)
                    return DefaultMouseInfoDepthIndoor();

                return depth;
            }
            inline float ValidateMouseInfoDepthOutdoor(float depth) {
                if (depth <= 0.0f || depth > 16192.0f)
                    return DefaultMouseInfoDepthOutdoor();

                return depth;
            }
            inline float ValidateRangedAttackDepth(float depth) {
                if (depth <= 0.0f || depth > 16192.0f)
                    return DefaultRangedAttackDepth();

                return depth;
            }
            inline int ValidateFloorChecksEps(int eps) {
                if (eps <= 0)
                    return DefaultFloorChecksEps();

                return eps;
            }

            int max_flight_height;
            int artifact_limit;

            /** Maximum depth for item pickup / opening chests / activating levers / etc with a mouse. */
            float mouse_interaction_depth;

            /** Maximum depth for item pickup / opening chests / activating levers / etc with a keyboard (by pressing space). */
            float keyboard_interaction_depth;

            /** Maximum depth at which right clicking on a monster produces a popup.
                * Also somehow this is the max depth for the souldrinker spell. */
            float mouse_info_depth_indoor;
            float mouse_info_depth_outdoor; // That's 25 * 512, so 25 cells.

            /** Max depth for ranged attacks and ranged spells. It's impossible to target monsters that are father away
                * than this value. Incidentally this is also the depth at which status bar tips are displayed on mouse over. */
            float ranged_attack_depth;

            /** Maximum allowed slack for point-inside-a-polygon checks when calculating floor z level. This is needed because
                * there are actual holes in level geometry sometimes, up to several units wide. */
            int floor_checks_eps;

            bool show_unidentified_item;    // show unidentified items in green mask in inventory
        };

        Gameplay gameplay;

        class Graphics {
         public:
            const std::string section_name = "graphics";

            void Default();
            void Load();
            void Save();

            inline std::string DefaultRenderer() { return "OpenGL"; }
            inline void SetRenderer(std::string renderer) {
                this->renderer = ValidateRenderer(renderer);
            }
            inline std::string GetRenderer() {
                return this->renderer;
            }

            inline bool DefaultBloodSplats() {
                return true;
            }
            inline void SetBloodSplats(bool is_enabled) {
                this->bloodsplats = is_enabled;
            }
            inline bool GetBloodSplats() {
                return this->bloodsplats;
            }
            inline void ToggleBloodSplats() {
                this->bloodsplats = !this->bloodsplats;
            }

            inline bool DefaultColoredLight() {
                return true;
            }
            inline void SetColoredLights(bool is_enabled) {
                this->colored_lights = is_enabled;
            }
            inline bool GetColoredLights() {
                return this->colored_lights;
            }
            inline void ToggleColoredLights() {
                this->colored_lights = !this->colored_lights;
            }

            inline bool DefaultTinting() {
                return true;
            }
            inline void SetTinting(bool is_enabled) {
                this->tinting = is_enabled;
            }
            inline bool GetTinting() {
                return this->tinting;
            }
            inline void ToggleTinting() {
                this->tinting = !this->tinting;
            }

            inline bool DefaultHWLBitmaps() {
                return false;
            }
            inline void SetHWLBitmaps(bool is_enabled) {
                this->hwl_bitmaps = is_enabled;
            }
            inline bool GetHWLBitmaps() {
                return this->hwl_bitmaps;
            }
            inline void ToggleHWLBitmaps() {
                this->hwl_bitmaps = !this->hwl_bitmaps;
            }

            inline bool DefaultHWLSprites() {
                return false;
            }
            inline void SetHWLSprites(bool is_enabled) {
                this->hwl_sprites = is_enabled;
            }
            inline bool GetHWLSprites() {
                return this->hwl_sprites;
            }
            inline void ToggleHWLSprites() {
                this->hwl_sprites = !this->hwl_sprites;
            }

            inline int DefaultGamma() {
                return 4;
            }
            inline void SetGamma(int level) {
                this->gamma = ValidateGamma(level);
            }
            inline int GetGamma() {
                return this->gamma;
            }

            inline bool DefaultSnow() {
                return false;
            }
            inline void SetSnow(bool is_enabled) {
                this->snow = is_enabled;
            }
            inline bool GetSnow() const {
                return this->snow;
            }
            inline void ToggleSnow() {
                this->snow = !this->snow;
            }

            inline bool DefaultSeasonsChange() {
                return true;
            }
            inline void SetSeasonsChange(bool is_enabled) {
                this->seasons_change = is_enabled;
            }
            inline bool GetSeasonsChange() const {
                return this->seasons_change;
            }
            inline void ToggleSeasonsChange() {
                this->seasons_change = !this->seasons_change;
            }

            inline bool DefaultSpecular() {
                return false;
            }
            inline void SetSpecular(bool is_enabled) {
                this->specular = is_enabled;
            }
            inline bool GetSpecular() const {
                return this->specular;
            }
            inline void ToggleSpecular() {
                this->specular = !this->specular;
            }

            inline bool DefaultFog() {
                return false;
            }
            inline void SetFog(bool is_enabled) {
                this->fog = is_enabled;
            }
            inline bool GetFog() const {
                return this->fog;
            }
            inline void ToggleFog() {
                this->fog = !this->fog;
            }

            inline void ToggleExtendedDrawDistance() { extended_draw_distance = !extended_draw_distance; }

            // TODO [config]: remove this and move distance limits to config
            bool extended_draw_distance;    // 2.5x draw distance

         private:
            inline std::string ValidateRenderer(std::string renderer) {
                if (renderer != "OpenGL" && renderer != "DirectDraw")
                    renderer = DefaultRenderer();

                return renderer;
            }
            inline int ValidateGamma(int level) {
                if (level < 0)
                    return 0;
                else if (level > 9)
                    return 9;

                return level;
            }

            std::string renderer;

            bool bloodsplats;
            bool colored_lights;
            bool tinting;
            /** use low-resolution bitmaps from HWL file instead of hi-resolution ones from LOD */
            bool hwl_bitmaps;
            /** use low-resolution sprites from HWL file instead of hi-resolution ones from LOD */
            bool hwl_sprites;

            /** not in use? */
            int gamma;

            bool seasons_change;
            bool snow;

            bool specular;
            bool fog;
        };

        Graphics graphics;

        class Keybindings {
         public:
            const std::string section_name = "keybindings";

            void Default();
            void Load();
            void Save();

            inline std::string DefaultForward() {
                return "UP";
            }
            inline void SetForward(std::string key_name) {
                this->forward = validateKey(key_name);
            }
            inline std::string GetForward() {
                return this->forward;
            }

            inline std::string DefaultBackward() {
                return "DOWN";
            }
            inline void SetBackward(std::string key_name) {
                this->backward = validateKey(key_name);
            }
            inline std::string GetBackward() {
                return this->backward;
            }

            inline std::string DefaultLeft() {
                return "LEFT";
            }
            inline void SetLeft(std::string key_name) {
                this->left = validateKey(key_name);
            }
            inline std::string GetLeft() {
                return this->left;
            }

            inline std::string DefaultRight() {
                return "RIGHT";
            }
            inline void SetRight(std::string key_name) {
                this->right = validateKey(key_name);
            }
            inline std::string GetRight() {
                return this->right;
            }

            inline std::string DefaultAttack() {
                return "A";
            }
            inline void SetAttack(std::string key_name) {
                this->attack = validateKey(key_name);
            }
            inline std::string GetAttack() {
                return this->attack;
            }

            inline std::string DefaultCastReady() {
                return "S";
            }
            inline void SetCastReady(std::string key_name) {
                this->cast_ready = validateKey(key_name);
            }
            inline std::string GetCastReady() {
                return this->cast_ready;
            }

            inline std::string DefaultYell() {
                return "Y";
            }
            inline void SetYell(std::string key_name) {
                this->yell = validateKey(key_name);
            }
            inline std::string GetYell() {
                return this->yell;
            }

            inline std::string DefaultJump() {
                return "X";
            }
            inline void SetJump(std::string key_name) {
                this->jump = validateKey(key_name);
            }
            inline std::string GetJump() {
                return this->jump;
            }

            inline std::string DefaultCombat() {
                return "RETURN";
            }
            inline void SetCombat(std::string key_name) {
                this->combat = validateKey(key_name);
            }
            inline std::string GetCombat() {
                return this->combat;
            }

            inline std::string DefaultEventTrigger() {
                return "SPACE";
            }
            inline void SetEventTrigger(std::string key_name) {
                this->event_trigger = validateKey(key_name);
            }
            inline std::string GetEventTrigger() {
                return this->event_trigger;
            }

            inline std::string DefaultCast() {
                return "C";
            }
            inline void SetCast(std::string key_name) {
                this->cast = validateKey(key_name);
            }
            inline std::string GetCast() {
                return this->cast;
            }

            inline std::string DefaultPass() {
                return "B";
            }
            inline void SetPass(std::string key_name) {
                this->pass = validateKey(key_name);
            }
            inline std::string GetPass() {
                return this->pass;
            }

            inline std::string DefaultCharCycle() {
                return "TAB";
            }
            inline void SetCharCycle(std::string key_name) {
                this->char_cycle = validateKey(key_name);
            }
            inline std::string GetCharCycle() {
                return this->char_cycle;
            }

            inline std::string DefaultQuest() {
                return "Q";
            }
            inline void SetQuest(std::string key_name) {
                this->quest = validateKey(key_name);
            }
            inline std::string GetQuest() {
                return this->quest;
            }

            inline std::string DefaultQuickReference() {
                return "Z";
            }
            inline void SetQuickReference(std::string key_name) {
                this->quick_reference = validateKey(key_name);
            }
            inline std::string GetQuickReference() {
                return this->quick_reference;
            }

            inline std::string DefaultRest() {
                return "R";
            }
            inline void SetRest(std::string key_name) {
                this->rest = validateKey(key_name);
            }
            inline std::string GetRest() {
                return this->rest;
            }

            inline std::string DefaultTimeCalendar() {
                return "T";
            }
            inline void SetTimeCalendar(std::string key_name) {
                this->time_calendar = validateKey(key_name);
            }
            inline std::string GetTimeCalendar() {
                return this->time_calendar;
            }

            inline std::string DefaultAutoNotes() {
                return "N";
            }
            inline void SetAutoNotes(std::string key_name) {
                this->auto_notes = validateKey(key_name);
            }
            inline std::string GetAutoNotes() {
                return this->auto_notes;
            }

            inline std::string DefaultMapBook() {
                return "M";
            }
            inline void SetMapBook(std::string key_name) {
                this->map_book = validateKey(key_name);
            }
            inline std::string GetMapBook() {
                return this->map_book;
            }

            inline std::string DefaultLookUp() {
                return "PAGE DOWN";
            }
            inline void SetLookUp(std::string key_name) {
                this->look_up = validateKey(key_name);
            }
            inline std::string GetLookUp() {
                return this->look_up;
            }

            inline std::string DefaultLookDown() {
                return "DELETE";
            }
            inline void SetLookDown(std::string key_name) {
                this->look_down = validateKey(key_name);
            }
            inline std::string GetLookDown() {
                return this->look_down;
            }

            inline std::string DefaultCenterView() {
                return "END";
            }
            inline void SetCenterView(std::string key_name) {
                this->center_view = validateKey(key_name);
            }
            inline std::string GetCenterView() {
                return this->center_view;
            }

            inline std::string DefaultZoomIn() {
                return "ADD";
            }
            inline void SetZoomIn(std::string key_name) {
                this->zoom_in = validateKey(key_name);
            }
            inline std::string GetZoomIn() {
                return this->zoom_in;
            }

            inline std::string DefaultZoomOut() {
                return "SUBTRACT";
            }
            inline void SetZoomOut(std::string key_name) {
                this->zoom_out = validateKey(key_name);
            }
            inline std::string GetZoomOut() {
                return this->zoom_out;
            }

            inline std::string DefaultFlyUp() {
                return "PAGE UP";
            }
            inline void SetFlyUp(std::string key_name) {
                this->fly_up = validateKey(key_name);
            }
            inline std::string GetFlyUp() {
                return this->fly_up;
            }

            inline std::string DefaultFlyDown() {
                return "INSERT";
            }
            inline void SetFlyDown(std::string key_name) {
                this->fly_down = validateKey(key_name);
            }
            inline std::string GetFlyDown() {
                return this->fly_down;
            }

            inline std::string DefaultLand() {
                return "HOME";
            }
            inline void SetLand(std::string key_name) {
                this->land = validateKey(key_name);
            }
            inline std::string GetLand() {
                return this->land;
            }

            inline std::string DefaultAlwaysRun() {
                return "U";
            }
            inline void SetAlwaysRun(std::string key_name) {
                this->always_run = validateKey(key_name);
            }
            inline std::string GetAlwaysRun() {
                return this->always_run;
            }

            inline std::string DefaultStepLeft() {
                return "L BRACKET";
            }
            inline void SetStepLeft(std::string key_name) {
                this->step_left = validateKey(key_name);
            }
            inline std::string GetStepLeft() {
                return this->step_left;
            }

            inline std::string DefaultStepRight() {
                return "R BRACKET";
            }
            inline void SetStepRight(std::string key_name) {
                this->step_right = validateKey(key_name);
            }
            inline std::string GetStepRight() {
                return this->step_right;
            }

         private:
            inline std::string validateKey(std::string key) {
                // TODO [config]: validation, maybe via GameKey::displayNames
                return key;
            }

            std::string forward;
            std::string backward;
            std::string left;
            std::string right;
            std::string attack;
            std::string cast_ready;
            std::string yell;
            std::string jump;
            std::string combat;
            std::string event_trigger;
            std::string cast;
            std::string pass;
            std::string char_cycle;
            std::string quest;
            std::string quick_reference;
            std::string rest;
            std::string time_calendar;
            std::string auto_notes;
            std::string map_book;
            std::string look_up;
            std::string look_down;
            std::string center_view;
            std::string zoom_in;
            std::string zoom_out;
            std::string fly_up;
            std::string fly_down;
            std::string land;
            std::string always_run;
            std::string step_left;
            std::string step_right;
        };

        Keybindings keybindings;

        class Settings {
         public:
            const std::string section_name = "settings";

            void Default();
            void Load();
            void Save();

            inline bool DefaultAlwaysRun() {
                return true;
            }
            inline void SetAlwaysRun(bool is_enabled) {
                this->always_run = is_enabled;
            }
            inline bool GetAlwaysRun() const {
                return this->always_run;
            }
            inline void ToggleAlwaysRun() {
                this->always_run = !this->always_run;
            }

            inline bool DefaultFlipOnExit() {
                return false;
            }
            inline void SetFlipOnExit(bool is_enabled) {
                this->flip_on_exit = is_enabled;
            }
            inline bool GetFlipOnExit() const {
                return this->flip_on_exit;
            }
            inline void ToggleFlipOnExit() {
                this->flip_on_exit = !this->flip_on_exit;
            }

            inline bool DefaultShowHits() {
                return true;
            }
            inline void SetShowHits(bool is_enabled) {
                this->show_hits = is_enabled;
            }
            inline bool GetShowHits() const {
                return this->show_hits;
            }
            inline void ToggleShowHits() {
                this->show_hits = !this->show_hits;
            }

            inline int DefaultMusicLevel() {
                return 3;
            }
            inline void SetMusicLevel(int level) {
                this->music_level = ValidateMusicLevel(level);
            }
            inline int GetMusicLevel() {
                return this->music_level;
            }

            inline int DefaultSoundLevel() {
                return 4;
            }
            inline void SetSoundLevel(int level) {
                this->sound_level = ValidateSoundLevel(level);
            }
            inline int GetSoundLevel() {
                return this->sound_level;
            }

            inline int DefaultVoiceLevel() {
                return 5;
            }
            inline void SetVoiceLevel(int level) {
                this->voice_level = ValidateVoiceLevel(level);
            }
            inline int GetVoiceLevel() {
                return this->voice_level;
            }

            inline float DefaultTurnSpeed() {
                return 0.0f;
            }
            inline void SetTurnSpeed(float speed) {
                this->turn_speed = ValidateTurnSpeed(speed);
            }
            inline float GetTurnSpeed() {
                return this->turn_speed;
            }

            inline int DefaultVerticalSpeed() {
                return 25;
            }
            inline void SetVerticalTurnSpeed(int speed) {
                this->vertical_turn_speed = ValidateVerticalTurnSpeed(speed);
            }
            inline int GetVerticalTurnSpeed() {
                return this->vertical_turn_speed;
            }

            inline bool DefaultWalkSound() {
                return true;
            }
            inline void SetWalkSound(bool is_enabled) {
                this->walk_sound = is_enabled;
            }
            inline bool GetWalkSound() {
                return this->walk_sound;
            }
            inline void ToggleWalkSound() {
                this->walk_sound = !this->walk_sound;
            }

         private:
            inline int ValidateMusicLevel(int level) {
                if (level < 0)
                    return 0;
                else if (level > 9)
                    return 9;

                return level;
            }
            inline int ValidateSoundLevel(int level) {
                if (level < 0)
                    return 0;
                else if (level > 9)
                    return 9;

                return level;
            }
            inline int ValidateVoiceLevel(int level) {
                if (level < 0)
                    return 0;
                else if (level > 9)
                    return 9;

                return level;
            }
            inline int ValidateVerticalTurnSpeed(int speed) {
                if (speed <= 0)
                    return DefaultVerticalSpeed();

                return speed;
            }
            inline float ValidateTurnSpeed(float speed) {
                if (speed <= 0.0f || speed > 1024.0f)
                    return DefaultTurnSpeed();

                return speed;
            }

            /** true - run, false - walk */
            bool always_run = true;

            /** Horizontal view flip when exiting buildings */
            bool flip_on_exit = false;

            /** Discrete turn speed, 0 - smooth, x16 - 64 degress turn, x32 - 128 degress turn. Only smooth is usable on modern machines. */
            int turn_speed;

            /** Discrete vertical turn speed (mm7 default is 25). */
            int vertical_turn_speed;

            /** not in use */
            float turn_speed_smooth;

            /** Sound volume level */
            int sound_level;

            /** Voice volume level */
            int voice_level;

            /** Music volume level */
            int music_level;

            /** Show hits status in status bar */
            bool show_hits;

            /** Party footstep's sound while moving. */
            bool walk_sound;
        };

        Settings settings;

        class Window {
         public:
            const std::string section_name = "window";

            void Default();
            void Load();
            void Save();

            inline bool DefaultBorderless() {
                return false;
            }
            inline void SetBorderless(bool is_borderless) {
                this->borderless = is_borderless;
            }
            inline bool GetBorderless() {
                return this->borderless;
            }
            inline void ToggleBorderless() {
                this->borderless = !this->borderless;
            }

            inline int DefaultDisplay() {
                return 0;
            }
            inline int GetDisplay() {
                return this->display;
            }
            inline void SetDisplay(int display) {
                this->display = display;
            }

            inline bool DefaultFullscreen() {
                return false;
            }
            inline void SetFullscreen(bool is_fullscreen) {
                this->fullscreen = is_fullscreen;
            }
            inline bool GetFullscreen() {
                return this->fullscreen;
            }
            inline void ToggleFullscreen() {
                this->fullscreen = !this->fullscreen;
            }

            inline int DefaultPositionX() {
                return -1;
            }
            inline int DefaultPositionY() {
                return -1;
            }
            inline void SetPositionX(int position_x) {
                this->position_x = ValidatePositionX(position_x);
            }
            inline void SetPositionY(int position_y) {
                this->position_y = ValidatePositionY(position_y);
            }
            inline void SetPosition(int position_x, int position_y) {
                SetPositionX(position_x);
                SetPositionY(position_y);
            }
            inline int GetPositionX() {
                return this->position_x;
            }
            inline int GetPositionY() {
                return this->position_y;
            }

            inline int DefaultWidth() {
                return 640;
            }
            inline int DefaultHeight() {
                return 480;
            }
            inline void SetWidth(int window_width) {
                this->width = ValidateWidth(window_width);
            }
            inline void SetHeight(int window_height) {
                this->height = ValidateHeight(window_height);
            }
            inline void SetDimensions(int window_width, int window_height) {
                SetWidth(window_height);
                SetHeight(window_width);
            }
            inline int GetWidth() {
                return this->width;
            }
            inline int GetHeight() {
                return this->height;
            }

            inline bool DefaultMouseGrab() {
                return true;
            }
            inline void SetMouseGrab(bool is_mouse_grab) {
                this->mouse_grab = is_mouse_grab;
            }
            inline bool GetMouseGrab() {
                return this->mouse_grab;
            }
            inline void ToggleMouseGrab() {
                this->mouse_grab = this->mouse_grab;
            }

            inline std::string DefaultTitle() { return "World of Might and Magic"; }
            inline void SetTitle(std::string title) {
                this->title = ValidateTitle(title);
            }
            inline std::string GetTitle() {
                return this->title;
            }

         private:
            inline int ValidatePositionX(int position) {
                if (position < -1)
                    return DefaultPositionX();

                return position;
            }
            inline int ValidatePositionY(int position) {
                if (position < -1)
                    return DefaultPositionY();

                return position;
            }

            inline int ValidateHeight(int height) {
                if (height <= 0)
                    return DefaultHeight();

                return height;
            }
            inline int ValidateWidth(int width) {
                if (width <= 0)
                    return DefaultWidth();

                return width;
            }

            inline std::string ValidateTitle(std::string title) {
                if (title.empty())
                    title = DefaultTitle();

                return title;
            }

            /** Coordinates in pixels for position of left-top window corner. -1 is window centered on this axis. */
            int position_x;
            int position_y;

            /** Window size in pixels. */
            int width;
            int height;

            /** Display number as exposed by SDL. Order is platform-specific, e.g. on windows 0 is main display */
            int display;

            /** Window mode. 1 - fullscreen, 0 - window */
            bool fullscreen;

            /**
             * Borderless mode.
             *
             * It removes window borders. For fullscreen mode instead of going into traditional exclusive fullscreen mode it goes into fake fullscreen mode.
             * Fake fullscreen mode reposition window top-left corner to 0,0 ignoring window position coordinates.
             * And also resize window to display size.
             */
            int borderless;

            /** Don't grab mouse. When false, you cannot move the mouse outside the game window while it is in focus. */
            bool mouse_grab;

            std::string title;
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
        bool SectionCheck(std::string section);
    };
}  // namespace Application
