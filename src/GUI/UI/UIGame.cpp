#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <string>
#include <utility>
#include <memory>

#include "GUI/UI/UIGame.h"

#include "ItemGrid.h"
#include "Application/Game.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Evt/Processor.h"
#include "Engine/GameType.h"
#include "Engine/Graphics/BSPModel.h"
#include "Engine/Tables/DecorationTable.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/AtlasLayout.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/MapEnumFunctions.h"
#include "Engine/Tables/MapTable.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Tables/ObjectTable.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/NPC.h"
#include "Engine/Party.h"
#include "Engine/Spells/Spells.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Tables/PortraitFrameTable.h"
#include "Engine/Timer.h"
#include "Engine/TurnEngine/TurnEngine.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIButtonFrames.h"
#include "GUI/GUIWindow.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/Books/LloydsBook.h"
#include "GUI/UI/Books/TownPortalBook.h"
#include "GUI/UI/UICharacter.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIStatusBar.h"
#include "GUI/UI/UISpellbook.h"

#include "Io/InputEnumFunctions.h"
#include "Io/Mouse.h"

#include "Media/MediaPlayer.h"
#include "Core/Serialization/PlatformKeySerialization.h"

#include "Utility/Math/TrigLut.h"

#include "Library/Logger/Logger.h"

std::array<int, 4> pHealthBarPos = {{23, 138, 251, 366}}; // was 22, 137
std::array<int, 4> pManaBarPos = {{102, 217, 331, 447}};
extern const int pHealthManaBarYPos = 402;

// MM8's always-on HUD layout. MM8 only ever has 1 active character (Party::activeCharacterCount()), so unlike the
// MM7 tables above this is a single position, not a 4-slot array.
constexpr Pointi kMm8ScoreBgPos = {79, 2};   // "scorebg" (food+gold icon strip) top-left
constexpr int kMm8ScoreTextY = 6;
constexpr int kMm8FoodTextLeftMargin = 120;  // \t-escape left margin -> text left edge at x=132
constexpr int kMm8GoldTextLeftMargin = 223;  // \t-escape left margin -> text left edge at x=235
constexpr int kMm8PortraitX = 19;
constexpr int kMm8PortraitY = 389;
constexpr std::array<int, 4> kMm8BannerX = {{110, 208, 305, 403}};
constexpr int kMm8SlotPitch = 98;

int mm8PortraitSlotShift(int slot) {
    return slot == 0 ? 0 : kMm8BannerX[slot - 1] - kMm8BannerX[0] + kMm8SlotPitch;
}
constexpr int kMm8BannerY = 387;

// The decorative chrome scorebg/selring/ibshieldNN sit on top of. scorebg etc. are colorkeyed icon-only sprites with a
// transparent surround, so without these the raw 3D scene shows through behind them instead of MM8's bar background.
constexpr Pointi kMm8TopBarBgPos = {0, 0};      // "topbar" (icons.lod, patched)
constexpr Pointi kMm8BottomBarBgPos = {0, 366}; // "basebar" (icons.lod)

// Top-bar left bookend ("asil00", icons.lod, 64x64, opaque, flush against x=0). The right bookend ("asir00") is a real
// GUIButton (pBtn_Skull) instead of a static draw, so its position lives at its CreateButton call site (GUIWindow.cpp).
// "asir01" is a second, colorkeyed skull frame (same shape, desaturated) that exists in the archive but is unused,
// since the top-bar buttons use a single-texture click-flash rather than an up/down swap.
constexpr Pointi kMm8TopBarFigurinePos = {0, 0};

// The real MM8 bottom-left-of-portrait status gem (19x23, "statg"/"staty"/"statr", see game_ui_bar_*'s own declaration comment).
constexpr Pointi kMm8StatusGemPos = {11, 456};

std::array<unsigned int, 2> pHiredNPCsIconsOffsetsX = {{489, 559}};
std::array<unsigned int, 2> pHiredNPCsIconsOffsetsY = {{152, 152}};

std::array<Pointi, 14> pPartySpellbuffsUI_XYs = {{
    {477, 247},
    {497, 247},
    {522, 247},
    {542, 247},
    {564, 247},
    {581, 247},
    {614, 247},
    {477, 279},
    {497, 279},
    {522, 279},
    {542, 279},
    {564, 279},
    {589, 279},
    {612, 279}
}};

std::array<uint8_t, 14> pPartySpellbuffsUI_smthns = {
    {14, 1, 10, 4, 7, 2, 9, 3, 6, 15, 8, 3, 12, 0}};

std::array<PartyBuff, 14> spellBuffsAtRightPanel = {
    {PARTY_BUFF_FEATHER_FALL, PARTY_BUFF_RESIST_FIRE, PARTY_BUFF_RESIST_AIR,
     PARTY_BUFF_RESIST_WATER, PARTY_BUFF_RESIST_MIND, PARTY_BUFF_RESIST_EARTH,
     PARTY_BUFF_RESIST_BODY, PARTY_BUFF_HEROISM, PARTY_BUFF_HASTE,
     PARTY_BUFF_SHIELD, PARTY_BUFF_STONE_SKIN, PARTY_BUFF_PROTECTION_FROM_MAGIC,
     PARTY_BUFF_IMMOLATION, PARTY_BUFF_DAY_OF_GODS}};

std::array<int, 8> BtnTurnCoord = {
    {0xA4, 0x5D, 0x16, 0xB, 0x5, 0xD, 0x7, 0x3B}};

// MM8's real Controls (options) screen layout, measured against the decoded "controlbg" background (EnglishD.lod).
// controlbg is drawn at {8,8} (GUIWindow_GameOptions::Update()). Every constant below is already in SCREEN space
// (measured local asset coordinate + {8,8}) so it can be used directly in a CreateButton() call.
constexpr Pointi kMm8CfgKeyboardPos = {89, 173};    // local (81,165)
constexpr Pointi kMm8VideoOptionsPos = {89, 243};   // local (81,235)
constexpr Pointi kMm8ReturnPos = {349, 388};        // local (341,380)
constexpr Pointi kMm8TurnRate16xPos = {87, 349};    // local (79,341)
constexpr Pointi kMm8TurnRate32xPos = {152, 349};   // local (144,341)
constexpr Pointi kMm8TurnRateSmoothPos = {216, 349};  // local (208,341)
constexpr Pointi kMm8WalkSoundPos = {87, 384};      // local (79,376)
constexpr Pointi kMm8ShowHitsPos = {208, 384};      // local (200,376)
constexpr Pointi kMm8AlwaysRunPos = {87, 412};      // local (79,404)
constexpr Pointi kMm8FlipOnExitPos = {208, 412};    // local (200,404)

// The 4 persistent captions controlbg bakes an empty olive slot for: "bl_turn"/"bl_sound"/"bl_music"/"bl_voice"
// (EnglishD.lod), which read "TURN RATE"/"SOUND VOLUME"/"MUSIC VOLUME"/"VOICE VOLUME". All 4 are 227x28, centered in
// their own ~228x30 slot.
constexpr Pointi kMm8CaptionTurnRatePos = {90, 314};  // local (82,306)
constexpr Pointi kMm8CaptionSoundPos = {347, 173};    // local (339,165)
constexpr Pointi kMm8CaptionMusicPos = {347, 244};    // local (339,236)
constexpr Pointi kMm8CaptionVoicePos = {347, 314};    // local (339,306)

// Each of the 3 volume rows' left arrow / track / right arrow (screen space). The track is 10 cells between 11
// boundaries. All 3 rows share this x layout. Only y differs (row pitch ~71px, the arrow/track row sits 38px below
// its own caption's top edge).
constexpr int kMm8VolumeArrowLeftX = 346;    // local 338
// kMm8VolumeTrackX/kMm8VolumeTrackWidth (local 355, width 194) live in UIGame.h - GameMenu.cpp's
// volume-change handlers need them too, to resolve a track click to the right level.
constexpr int kMm8VolumeArrowRightX = 557;   // local 549
constexpr int kMm8VolumeRowHeight = 18;
constexpr int kMm8VolumeSoundY = 210;   // local 202
constexpr int kMm8VolumeMusicY = 281;   // local 273
constexpr int kMm8VolumeVoiceY = 351;   // local 343
// Marker (convolNN, 16x18) x position per level 0..9, centered within its own cell (screen
// space). Cell boundaries measured by the same scan: local 355,372,392,411,431,451,470,490,510,
// 530,549.
constexpr std::array<int, 10> kMm8VolumeMarkerX = {363, 382, 401, 421, 441, 460, 480, 500, 520, 539};

std::shared_ptr<GraphicsImage> game_ui_statusbar;
std::shared_ptr<GraphicsImage> game_ui_rightframe;
std::shared_ptr<GraphicsImage> game_ui_topframe;
std::shared_ptr<GraphicsImage> game_ui_leftframe;
std::shared_ptr<GraphicsImage> game_ui_bottomframe;

std::shared_ptr<GraphicsImage> game_ui_monster_hp_green;
std::shared_ptr<GraphicsImage> game_ui_monster_hp_yellow;
std::shared_ptr<GraphicsImage> game_ui_monster_hp_red;
std::shared_ptr<GraphicsImage> game_ui_monster_hp_background;
std::shared_ptr<GraphicsImage> game_ui_monster_hp_border_left;
std::shared_ptr<GraphicsImage> game_ui_monster_hp_border_right;

std::shared_ptr<GraphicsImage> game_ui_minimap_frame;              // 5079D8
std::shared_ptr<GraphicsImage> game_ui_minimap_compass;            // 5079B4
std::array<std::shared_ptr<GraphicsImage>, 8> game_ui_minimap_dirs;

std::shared_ptr<GraphicsImage> game_ui_compass_frame;
// Kept independent of game_ui_minimap_compass (the MM7 minimap-needle asset, above), which the legacy MM7 ghost-compass
// draw in GameUI_DrawMinimap also reads unconditionally. For MM8 that variable stays nullptr, so its DrawQuad2D(nullptr,
// ...) call safely no-ops (checked at the top of OpenGLRenderer::DrawQuad2D).
std::shared_ptr<GraphicsImage> game_ui_compass_strip;

std::shared_ptr<GraphicsImage> game_ui_mm8_bar_frame;
std::shared_ptr<GraphicsImage> game_ui_mm8_bar_green;
std::shared_ptr<GraphicsImage> game_ui_mm8_bar_yellow;
std::shared_ptr<GraphicsImage> game_ui_mm8_bar_red;
std::shared_ptr<GraphicsImage> game_ui_mm8_bar_blue;

std::shared_ptr<GraphicsImage> game_ui_scorebg;
std::shared_ptr<GraphicsImage> game_ui_portrait_ring;
std::array<std::shared_ptr<GraphicsImage>, 4> game_ui_party_banners;
std::shared_ptr<GraphicsImage> game_ui_top_bar_bg;
std::shared_ptr<GraphicsImage> game_ui_bottom_bar_bg;
std::shared_ptr<GraphicsImage> game_ui_topbar_figurine;
std::shared_ptr<GraphicsImage> game_ui_topbar_skull;

std::shared_ptr<GraphicsImage> game_ui_menu_options;

std::shared_ptr<GraphicsImage> game_ui_tome_storyline;
std::shared_ptr<GraphicsImage> game_ui_tome_calendar;
std::shared_ptr<GraphicsImage> game_ui_tome_maps;
std::shared_ptr<GraphicsImage> game_ui_tome_autonotes;
std::shared_ptr<GraphicsImage> game_ui_tome_quests;

std::shared_ptr<GraphicsImage> game_ui_btn_rest;
std::shared_ptr<GraphicsImage> game_ui_btn_cast;
std::shared_ptr<GraphicsImage> game_ui_btn_zoomin;
std::shared_ptr<GraphicsImage> game_ui_btn_zoomout;
std::shared_ptr<GraphicsImage> game_ui_btn_quickref;
std::shared_ptr<GraphicsImage> game_ui_btn_settings;

std::shared_ptr<GraphicsImage> game_ui_menu_options_video_background;
std::shared_ptr<GraphicsImage> game_ui_menu_options_video_bloodsplats;
std::shared_ptr<GraphicsImage> game_ui_menu_options_video_coloredlights;
std::shared_ptr<GraphicsImage> game_ui_menu_options_video_tinting;
std::array<std::shared_ptr<GraphicsImage>, 10> game_ui_menu_options_video_gamma_positions;
std::array<std::shared_ptr<GraphicsImage>, 5> game_ui_options_controls;

std::shared_ptr<GraphicsImage> game_ui_evtnpc;  // 50795C

std::array<std::array<std::shared_ptr<GraphicsImage>, 56>, 5> game_ui_player_faces;
std::shared_ptr<GraphicsImage> game_ui_player_face_eradicated;
std::shared_ptr<GraphicsImage> game_ui_player_face_dead;

std::shared_ptr<GraphicsImage> game_ui_player_selection_frame;            // 50C98C
std::shared_ptr<GraphicsImage> game_ui_player_alert_yellow;               // 5079C8
std::shared_ptr<GraphicsImage> game_ui_player_alert_red;                  // 5079CC
std::shared_ptr<GraphicsImage> game_ui_player_alert_green;                // 5079D0

std::shared_ptr<GraphicsImage> game_ui_bar_red;
std::shared_ptr<GraphicsImage> game_ui_bar_yellow;
std::shared_ptr<GraphicsImage> game_ui_bar_green;
std::shared_ptr<GraphicsImage> game_ui_bar_blue;

std::shared_ptr<GraphicsImage> game_ui_playerbuff_pain_reflection;
std::shared_ptr<GraphicsImage> game_ui_playerbuff_hammerhands;
std::shared_ptr<GraphicsImage> game_ui_playerbuff_preservation;
std::shared_ptr<GraphicsImage> game_ui_playerbuff_bless;

int game_ui_wizardEye = -1;
int game_ui_torchLight = -1;

bool bFlashHistoryBook;
bool bFlashAutonotesBook;
bool bFlashQuestBook;

static bool bookFlashState = false;
static Time bookFlashTimer;

// The Controls (options) screen's MM8-only widgets, drawn every frame via GUIButtonFrameSet::select()
// (GUIWindow_GameOptions::Update()), not by the generic vTextures[0] draw. Not exported: nothing outside this file
// needs the raw pointer, GAME_TESTs drive these by their string id (game.pressGuiButton()).
static GUIButton *pBtn_Mm8TurnRate16x = nullptr;
static GUIButton *pBtn_Mm8TurnRate32x = nullptr;
static GUIButton *pBtn_Mm8TurnRateSmooth = nullptr;
static GUIButton *pBtn_Mm8WalkSound = nullptr;
static GUIButton *pBtn_Mm8ShowHits = nullptr;
static GUIButton *pBtn_Mm8AlwaysRun = nullptr;
static GUIButton *pBtn_Mm8FlipOnExit = nullptr;
static GUIButton *pBtn_Mm8ConfigureKeyboard = nullptr;
static GUIButton *pBtn_Mm8VideoOptions = nullptr;
static GUIButton *pBtn_Mm8Return = nullptr;

// The 4 persistent captions (see kMm8Caption*Pos above), loaded once per screen construction. Like
// GUIWindow_GameVideoOptions' own caption/toggle textures, they have no explicit Release() call.
static std::shared_ptr<GraphicsImage> mm8_options_caption_turn_rate;
static std::shared_ptr<GraphicsImage> mm8_options_caption_sound;
static std::shared_ptr<GraphicsImage> mm8_options_caption_music;
static std::shared_ptr<GraphicsImage> mm8_options_caption_voice;

extern InputAction currently_selected_action_for_binding;  // 506E68
extern std::unordered_set<InputAction> key_map_conflicted;  // 506E6C
extern std::unordered_map<InputAction, PlatformKey> curr_key_map;


GUIWindow_GameMenu::GUIWindow_GameMenu()
    : GUIWindow(WINDOW_GAME_MENU, {0, 0}, render->GetRenderDimensions()) {
    // MM8 ships this screen's background + button art under different names in EnglishD.lod, not icons.lod. The
    // "options" background bakes 6 blank button slots with no text. Each button's own caption lives in a separate
    // texture. "load1"'s real MM8 asset is "bt_loagd"/"bt_loagu"/"bt_loagh", a shipped retail typo ("loag" for "load"),
    // confirmed by decoding it (reads "LOAD GAME"). "bt_loadd"/"bt_loadu" are a different, smaller asset already
    // claimed by UISaveLoad.cpp's Load Game screen.
    //
    // Each button ships all 3 states: "_u" (up/idle), "_h" (hover), "_d" (down/pressed). The persistent caption is
    // "_u", the vivid gold of the real resting screen ("_d" is a dark greyed-out pressed look). The hover frame is
    // wired too, since the original glows gold on hover (see GUIButtonFrames.h).
    GUIButtonFrameSet framesNew, framesSave, framesLoad, framesControls, framesQuit, framesResume;
    if (gameType() == GAME_TYPE_MM8) {
        game_ui_menu_options = assets->getImage_ColorKeyFromEnglishDLOD("options");
        framesNew = loadMM8ButtonFrameSet(MM8ButtonFamily::GameMenuNew);
        framesSave = loadMM8ButtonFrameSet(MM8ButtonFamily::GameMenuSave);
        framesLoad = loadMM8ButtonFrameSet(MM8ButtonFamily::GameMenuLoad);
        framesControls = loadMM8ButtonFrameSet(MM8ButtonFamily::GameMenuControls);
        framesQuit = loadMM8ButtonFrameSet(MM8ButtonFamily::GameMenuQuit);
        framesResume = loadMM8ButtonFrameSet(MM8ButtonFamily::GameMenuResume);
    } else {
        // MM7 ships one texture per button and no hover/press art - stays on the "up" slot only,
        // drawn unconditionally every frame (see this class's own Update(), unaffected by hover).
        game_ui_menu_options = assets->getImage_ColorKey("options");
        framesNew.up = assets->getImage_ColorKey("new1");
        framesLoad.up = assets->getImage_ColorKey("load1");
        framesSave.up = assets->getImage_ColorKey("save1");
        framesControls.up = assets->getImage_ColorKey("controls1");
        framesResume.up = assets->getImage_ColorKey("resume1");
        framesQuit.up = assets->getImage_ColorKey("quit1");
    }

    // MM8's real "options" background bakes its 6 slots at 233x78 with a 282px column pitch and ~93.5px row pitch.
    // MM7's literals above are unrelated decompiled coordinates, so the MM8 branch below uses its own positions.
    Pointi newGamePos{0x13u, 0x9Bu}, saveGamePos{0x13u, 0xD1u}, loadGamePos{19, 263},
        controlsPos{241, 155}, quitPos{241, 209}, resumePos{241, 263};
    Sizei buttonDims{0xD6u, 0x28u};  // all 6 slots share one size on both MM7 and MM8.
    if (gameType() == GAME_TYPE_MM8) {
        newGamePos = {64, 148};
        saveGamePos = {64, 242};
        loadGamePos = {64, 335};
        controlsPos = {346, 148};
        quitPos = {346, 242};
        resumePos = {346, 335};
        buttonDims = {232, 77};
    }

    // Stable ids on all 6 buttons, so a test or other control routine can drive each of them directly via
    // pressGuiButton() instead of reading UIMSG_ constants or guessing pixel coordinates.
    pBtn_NewGame = CreateButton("GameMenu_NewGame", newGamePos, buttonDims, BUTTON_TYPE_NORMAL, 0,
        UIMSG_StartNewGame, 0, INPUT_ACTION_NEW_GAME, localization->str(LSTR_NEW_GAME));
    pBtn_NewGame->frames = framesNew;
    pBtn_SaveGame = CreateButton("GameMenu_SaveGame", saveGamePos, buttonDims, BUTTON_TYPE_NORMAL, 0,
        UIMSG_Game_OpenSaveGameDialog, 0, INPUT_ACTION_SAVE_GAME, localization->str(LSTR_SAVE_GAME));
    pBtn_SaveGame->frames = framesSave;
    pBtn_LoadGame = CreateButton("GameMenu_LoadGame", loadGamePos, buttonDims, BUTTON_TYPE_NORMAL, 0,
        UIMSG_Game_OpenLoadGameDialog, 0, INPUT_ACTION_LOAD_GAME, localization->str(LSTR_LOAD_GAME));
    pBtn_LoadGame->frames = framesLoad;
    pBtn_GameControls = CreateButton("GameMenu_Controls", controlsPos, buttonDims, BUTTON_TYPE_NORMAL, 0,
        UIMSG_Game_OpenOptionsDialog, 0, INPUT_ACTION_OPEN_OPTIONS, localization->str(LSTR_SOUND_KEYBOARD_GAME_OPTIONS));
    pBtn_GameControls->frames = framesControls;
    pBtn_QuitGame = CreateButton("GameMenu_Quit", quitPos, buttonDims, BUTTON_TYPE_NORMAL, 0,
        UIMSG_Quit, 0, INPUT_ACTION_EXIT_GAME, localization->str(LSTR_QUIT));
    pBtn_QuitGame->frames = framesQuit;
    pBtn_Resume = CreateButton("GameMenu_Resume", resumePos, buttonDims, BUTTON_TYPE_NORMAL, 0,
        UIMSG_GameMenu_ReturnToGame, 0, INPUT_ACTION_BACK_TO_GAME, localization->str(LSTR_RETURN_TO_GAME));
    pBtn_Resume->frames = framesResume;

    setKeyboardControlGroup(6, false, 0, 0);
}

void GUIWindow_GameMenu::Update() {
    // -----------------------------------
    // 004156F0 GUI_UpdateWindows --- part
    render->DrawQuad2D(game_ui_menu_options, pViewport.topLeft());

    // Each button's caption lives in its own up/hover/down frame triple (GUIButtonFrames.h), and the options background
    // above bakes 6 blank slots with no text. None of these 6 buttons spawn an OnButtonClick click-flash, so this
    // per-frame redraw is the caption's only draw, and `select()` both makes it visible and switches it to the hover
    // frame while the mouse is over the button. The caption is centered within the button's rect, since the caption
    // texture (219x64, or 221x52 for Load Game) is smaller than the slot.
    Pointi pt = mouse->position();
    for (GUIButton *button : {pBtn_NewGame, pBtn_SaveGame, pBtn_LoadGame, pBtn_GameControls, pBtn_QuitGame, pBtn_Resume}) {
        if (!button)
            continue;
        GraphicsImage *caption = button->frames.select(button->Contains(pt.x, pt.y), false);
        if (!caption)
            continue;
        Pointi captionPos = button->rect.topLeft() +
            Pointi{(button->rect.w - caption->width()) / 2, (button->rect.h - caption->height()) / 2};
        render->DrawQuad2D(caption, captionPos);
    }
}

//----- (00491CB5) --------------------------------------------------------
void GameUI_LoadPlayerPortraitsAndVoices() {
    // Grown to 5 alongside `game_ui_player_faces`/`Party::pCharacters`, see that array's own comment.
    for (unsigned i = 0; i < 5; ++i) {
        for (unsigned j = 0; j < 56; ++j) {
            // MM8's icons.lod names every face frame "pcNN-FF" (a consistent dash), while the shared MM7
            // `pPlayerPortraitsNames` table omits the dash for about half its entries, which resolves to the missing-texture placeholder.
            std::string portraitName = gameType() == GAME_TYPE_MM8
                ? fmt::format("pc{:02}-{:02}", pParty->pCharacters[i].uCurrentFace + 1, j + 1)
                : fmt::format("{}{:02}", pPlayerPortraitsNames[pParty->pCharacters[i].uCurrentFace], j + 1);
            game_ui_player_faces[i][j] = assets->getImage_ColorKey(portraitName);
        }
    }

    game_ui_player_face_eradicated =
        assets->getIcon("ERADCATE");
    game_ui_player_face_dead = assets->getIcon("DEAD");
    /*
        if (SoundSetAction[24][0])
        {
            for (unsigned i = 0; i < 4; ++i)
            {
                pSoundList->LoadSound(2 * (SoundSetAction[24][0] + 50 *
       pParty->pCharacters[i].uVoiceID) + 4998, 0); pSoundList->LoadSound(2 *
       (SoundSetAction[24][0] + 50 * pParty->pCharacters[i].uVoiceID) + 4999, 0);
            }
        }
    */
}

//----- (00491DE7) --------------------------------------------------------
void GameUI_ReloadPlayerPortraits(int player_id, int face_id) {  // the transition from the zombies to the normal state
    for (unsigned i = 0; i <= 55; ++i) {
        // See GameUI_LoadPlayerPortraitsAndVoices for why MM8 needs its own dashed name.
        auto filename = gameType() == GAME_TYPE_MM8
            ? fmt::format("pc{:02}-{:02}", face_id + 1, i + 1)
            : fmt::format("{}{:02}", pPlayerPortraitsNames[face_id], i + 1);
        game_ui_player_faces[player_id][i] =
            assets->getIcon(filename);
    }
}

//----- (00414D24) --------------------------------------------------------
static Color GameMenuUI_GetKeyBindingColor(InputAction action) {
    if (currently_selected_action_for_binding == action) {
        // TODO(pskelton): #time check tickcount usage here
        if (platform->tickCount() % 1000 < 500)
            return ui_gamemenu_keys_key_selection_blink_color_1;
        else
            return ui_gamemenu_keys_key_selection_blink_color_2;
    } else if (key_map_conflicted.contains(action)) {
        int intensity;

        int time = platform->tickCount() % 800;
        if (time < 400)
            intensity = -70 + 70 * time / 400;
        else
            intensity = +70 - 70 * time / 800;

        return Color(185 + intensity, 40 + intensity / 4, 40 + intensity / 4);
    }

    return ui_gamemenu_keys_key_default_color;
}

GUIWindow_GameKeyBindings::GUIWindow_GameKeyBindings()
    : GUIWindow(WINDOW_KEY_MAPPING_OPTIONS, {0, 0}, render->GetPresentDimensions()) {
    if (gameType() == GAME_TYPE_MM8) {
        game_ui_options_controls[0] = assets->getImage_ColorKeyFromEnglishDLOD("optkb");
        // No real MM8 asset exists for a hover-state variant of the "optkb" background (checked icons.lod,
        // icons.UI.lod and EnglishD.lod), so it stays null. The MM7 path's use of this slot is never drawn by Update().
        game_ui_options_controls[1] = nullptr;
        game_ui_options_controls[2] = assets->getImage_ColorKeyFromEnglishDLOD("bu_rtrnd");
        // The highlighted ("h") frame reads as the real MM8 "currently active page" look (lit border). The down ("d")
        // and up ("u") frames both look like a plain idle button.
        game_ui_options_controls[3] = assets->getImage_ColorKeyFromEnglishDLOD("bt_p1h");
        game_ui_options_controls[4] = assets->getImage_ColorKeyFromEnglishDLOD("bt_p2h");
    } else {
        game_ui_options_controls[0] = assets->getImage_ColorKey("optkb");
        game_ui_options_controls[1] = assets->getImage_ColorKey("optkb_h");
        game_ui_options_controls[2] = assets->getImage_ColorKey("resume1");
        game_ui_options_controls[3] = assets->getImage_ColorKey("optkb_1");
        game_ui_options_controls[4] = assets->getImage_ColorKey("optkb_2");
    }

    CreateButton({241, 302}, {214, 40}, BUTTON_TYPE_NORMAL, 0, UIMSG_Escape, 0);

    CreateButton({19, 302}, {108, 20}, BUTTON_TYPE_NORMAL, 0, UIMSG_SelectKeyPage1, 0);
    CreateButton({127, 302}, {108, 20}, BUTTON_TYPE_NORMAL, 0, UIMSG_SelectKeyPage2, 0);
    CreateButton("KeyBinding_Default", {127, 324}, {108, 20}, BUTTON_TYPE_NORMAL, 0, UIMSG_ResetKeyMapping, 0);
    CreateButton({19, 324}, {108, 20}, BUTTON_TYPE_NORMAL, 0, UIMSG_Game_OpenOptionsDialog, 0);

    CreateButton({129, 148}, {70, 19}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeKeyButton, 0);
    CreateButton({129, 167}, {70, 19}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeKeyButton, 1);
    CreateButton({129, 186}, {70, 19}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeKeyButton, 2);
    CreateButton({129, 205}, {70, 19}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeKeyButton, 3);
    CreateButton({129, 224}, {70, 19}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeKeyButton, 4);
    CreateButton({129, 243}, {70, 19}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeKeyButton, 5);
    CreateButton({129, 262}, {70, 19}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeKeyButton, 6);

    CreateButton({350, 148}, {70, 19}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeKeyButton, 7);
    CreateButton({350, 167}, {70, 19}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeKeyButton, 8);
    CreateButton({350, 186}, {70, 19}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeKeyButton, 9);
    CreateButton({350, 205}, {70, 19}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeKeyButton, 10);
    CreateButton({350, 224}, {70, 19}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeKeyButton, 11);
    CreateButton({350, 243}, {70, 19}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeKeyButton, 12);
    CreateButton({350, 262}, {70, 19}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeKeyButton, 13);

    currently_selected_action_for_binding = INPUT_ACTION_INVALID;
    KeyboardPageNum = 1;

    key_map_conflicted.clear();
    curr_key_map = keyboardActionMapping->currentKeybindings(KEYBINDINGS_CONFIGURABLE);
}

//----- (004142D3) --------------------------------------------------------
void GUIWindow_GameKeyBindings::Update() {
    // int v4;  // ecx@7
    // int v5;  // eax@8

    if (pGUIWindow_CurrentMenu->keyboard_input_status == WINDOW_INPUT_CONFIRMED) {
        InputAction action = currently_selected_action_for_binding;
        PlatformKey newKey = keyboardInputHandler->LastPressedKey();
        curr_key_map[action] = newKey;

        engine->_statusBar->clearAll();

        key_map_conflicted.clear();

        bool anyConflicts = false;
        for (auto x : curr_key_map) {
            for (auto y : curr_key_map) {
                if (x.first != y.first && x.second == y.second) {
                    key_map_conflicted.insert(x.first);
                    key_map_conflicted.insert(y.first);
                    anyConflicts = true;
                }
            }
        }

        if (anyConflicts)
            engine->_statusBar->setEvent(LSTR_KEY_CONFLICT);
        else
            engine->_statusBar->clearAll();

        keyboardInputHandler->EndTextInput();
        currently_selected_action_for_binding = INPUT_ACTION_INVALID;
    }
    render->DrawQuad2D(game_ui_options_controls[0], {8, 8});  // draw base texture

    int base_controls_offset = 0;
    if (KeyboardPageNum == 1) {
        render->DrawQuad2D(game_ui_options_controls[3], {19, 302});
    } else {
        base_controls_offset = 14;
        render->DrawQuad2D(game_ui_options_controls[4], {127, 302});
    }

    for (int i = 0; i < 7; ++i) {
        InputAction action1 = (InputAction)(base_controls_offset + i);
        DrawText(assets->pFontLucida.get(), {23, 142 + i * 21}, ui_gamemenu_keys_action_name_color, GetDisplayName(action1), pGUIWindow_CurrentMenu->frameRect);
        DrawText(assets->pFontLucida.get(), {127, 142 + i * 21}, GameMenuUI_GetKeyBindingColor(action1), GetDisplayName(curr_key_map[action1]), pGUIWindow_CurrentMenu->frameRect);

        int j = i + 7;
        InputAction action2 = (InputAction)(base_controls_offset + j);
        DrawText(assets->pFontLucida.get(), {247, 142 + i * 21}, ui_gamemenu_keys_action_name_color, GetDisplayName(action2), pGUIWindow_CurrentMenu->frameRect);
        DrawText(assets->pFontLucida.get(), {350, 142 + i * 21}, GameMenuUI_GetKeyBindingColor(action2), GetDisplayName(curr_key_map[action2]), pGUIWindow_CurrentMenu->frameRect);
    }
}

GUIWindow_GameVideoOptions::GUIWindow_GameVideoOptions()
    : GUIWindow(WINDOW_VIDEO_OPTIONS, {0, 0}, render->GetRenderDimensions()) {
    // -------------------------------------
    // GameMenuUI_OptionsVideo_Load --- part
    // MM8's real assets live in EnglishD.lod under a "bl_*" family ("bl_blspd"/"bl_coltd"/"bl_tintd" read "BLOOD SPLATS"/
    // "COLORED LIGHTS"/"TINTING"), not the MM7 "opvdH-*" names.
    if (gameType() == GAME_TYPE_MM8) {
        game_ui_menu_options_video_background = assets->getImage_ColorKeyFromEnglishDLOD("optvid");
        game_ui_menu_options_video_bloodsplats = assets->getImage_ColorKeyFromEnglishDLOD("bl_blspd");
        game_ui_menu_options_video_coloredlights = assets->getImage_ColorKeyFromEnglishDLOD("bl_coltd");
        game_ui_menu_options_video_tinting = assets->getImage_ColorKeyFromEnglishDLOD("bl_tintd");
    } else {
        game_ui_menu_options_video_background = assets->getImage_ColorKey("optvid");
        game_ui_menu_options_video_bloodsplats = assets->getImage_ColorKey("opvdH-bs");
        game_ui_menu_options_video_coloredlights = assets->getImage_ColorKey("opvdH-cl");
        game_ui_menu_options_video_tinting = assets->getImage_ColorKey("opvdH-tn");
    }

    game_ui_menu_options_video_gamma_positions[0] = assets->getIcon("convol10");
    game_ui_menu_options_video_gamma_positions[1] = assets->getIcon("convol20");
    game_ui_menu_options_video_gamma_positions[2] = assets->getIcon("convol30");
    game_ui_menu_options_video_gamma_positions[3] = assets->getIcon("convol40");
    game_ui_menu_options_video_gamma_positions[4] = assets->getIcon("convol50");
    game_ui_menu_options_video_gamma_positions[5] = assets->getIcon("convol60");
    game_ui_menu_options_video_gamma_positions[6] = assets->getIcon("convol70");
    game_ui_menu_options_video_gamma_positions[7] = assets->getIcon("convol80");
    game_ui_menu_options_video_gamma_positions[8] = assets->getIcon("convol90");
    game_ui_menu_options_video_gamma_positions[9] = assets->getIcon("convol00");
    // not_available_bloodsplats_texture_id =
    // pIcons_LOD->LoadTexture("opvdG-bs", TEXTURE_16BIT_PALETTE);
    // not_available_us_colored_lights_texture_id =
    // pIcons_LOD->LoadTexture("opvdG-cl", TEXTURE_16BIT_PALETTE);
    // not_available_tinting_texture_id = pIcons_LOD->LoadTexture("opvdG-tn",
    // TEXTURE_16BIT_PALETTE);

    CreateButton({0xF1u, 0x12Eu}, {0xD6u, 0x28u}, BUTTON_TYPE_NORMAL, 0, UIMSG_Escape, 0);

    // gamma buttons
    pBtn_SliderLeft = CreateButton({21, 161}, {17, 17}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeGammaLevel, 4, INPUT_ACTION_INVALID, "", { options_menu_skin.uTextureID_ArrowLeft }); // -
    CreateButton({42, 160}, {170, 17}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeGammaLevel, 0);
    pBtn_SliderRight = CreateButton({213, 161}, {17, 17}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeGammaLevel, 5, INPUT_ACTION_INVALID, "", { options_menu_skin.uTextureID_ArrowRight }); // +

    // if ( render->pRenderD3D )
    {
        CreateButton({0x13u, 0x118u}, {0xD6u, 0x12u}, BUTTON_TYPE_NORMAL, 0, UIMSG_ToggleBloodsplats, 0);
        CreateButton({0x13u, 0x12Eu}, {0xD6u, 0x12u}, BUTTON_TYPE_NORMAL, 0, UIMSG_ToggleColoredLights, 0);
        CreateButton({0x13u, 0x144u}, {0xD6u, 0x12u}, BUTTON_TYPE_NORMAL, 0, UIMSG_ToggleTint, 0);
    }

    // update gamma preview
    gamma_preview_image = GraphicsImage::Create(render->MakeViewportScreenshot(155, 117));
}

//----- (00414D9A) --------------------------------------------------------
void GUIWindow_GameVideoOptions::Update() {
    // -----------------------------------
    // 004156F0 GUI_UpdateWindows --- part
    int gammalevel = engine->config->graphics.Gamma.value();

    render->DrawQuad2D(game_ui_menu_options_video_background, {8, 8});  // draw base texture
    // if ( !render->bWindowMode && render->IsGammaSupported() )
    {
        render->DrawQuad2D(game_ui_menu_options_video_gamma_positions[gammalevel], {17 * gammalevel + 42, 162});

        if (gamma_preview_image)
            render->DrawQuad2D(gamma_preview_image, {274, 169});

        Recti msg_window(22, 190, 211, 79);
        DrawTitleText(
            assets->pFontSmallnum.get(), 0, 0, ui_gamemenu_video_gamma_title_color,
            localization->str(LSTR_GAMMA_CONTROLS_THE_RELATIVE_BRIGHTNESS), 3, msg_window
        );
    }

    if (engine->config->graphics.BloodSplats.value())
        render->DrawQuad2D(game_ui_menu_options_video_bloodsplats, {20, 281});
    if (engine->config->graphics.ColoredLights.value())
        render->DrawQuad2D(game_ui_menu_options_video_coloredlights, {20, 303});
    if (engine->config->graphics.Tinting.value())
        render->DrawQuad2D(game_ui_menu_options_video_tinting, {20, 325});
}

OptionsMenuSkin options_menu_skin;  // 507C60

GUIWindow_GameOptions::GUIWindow_GameOptions()
    : GUIWindow(WINDOW_GAME_OPTIONS, {0, 0}, render->GetRenderDimensions()) {
    // MM8's real background/turn-speed-icon assets live in EnglishD.lod under different names ("bt_16xd"/"bt_32xd"/
    // "bt_smood" read "16x"/"32x"/"SMOOTH"). The slider arrows reuse "ar_lt_up"/"ar_rt_up", the same icons.lod arrow
    // glyphs as the create-character screen, so `getImage_Alpha` (icons.lod) is the loader.
    GUIButtonFrameSet framesTurn16x, framesTurn32x, framesTurnSmooth;
    GUIButtonFrameSet framesWalkSound, framesShowHits, framesAlwaysRun, framesFlipOnExit;
    GUIButtonFrameSet framesConfigureKeyboard, framesVideoOptions, framesReturn;
    if (gameType() == GAME_TYPE_MM8) {
        options_menu_skin.uTextureID_Background = assets->getImage_ColorKeyFromEnglishDLOD("controlbg");
        options_menu_skin.uTextureID_ArrowLeft = assets->getImage_Alpha("ar_lt_up");
        options_menu_skin.uTextureID_ArrowRight = assets->getImage_Alpha("ar_rt_up");

        // The turn-rate/toggle families each ship an "_u"/"_d" pair (no hover), and CONFIGURE KEYBOARD/VIDEO
        // OPTIONS/RETURN ship the full "_u"/"_h"/"_d" triple. Update() selects the resting ("_u") or engaged/hovered
        // frame instead of skipping the draw when a toggle is off.
        framesTurn16x = loadMM8ButtonFrameSet(MM8ButtonFamily::GameOptionsTurnRate16x);
        framesTurn32x = loadMM8ButtonFrameSet(MM8ButtonFamily::GameOptionsTurnRate32x);
        framesTurnSmooth = loadMM8ButtonFrameSet(MM8ButtonFamily::GameOptionsTurnRateSmooth);
        framesWalkSound = loadMM8ButtonFrameSet(MM8ButtonFamily::GameOptionsWalkSound);
        framesShowHits = loadMM8ButtonFrameSet(MM8ButtonFamily::GameOptionsShowHits);
        framesAlwaysRun = loadMM8ButtonFrameSet(MM8ButtonFamily::GameOptionsAlwaysRun);
        framesFlipOnExit = loadMM8ButtonFrameSet(MM8ButtonFamily::GameOptionsFlipOnExit);
        framesConfigureKeyboard = loadMM8ButtonFrameSet(MM8ButtonFamily::GameOptionsConfigureKeyboard);
        framesVideoOptions = loadMM8ButtonFrameSet(MM8ButtonFamily::GameOptionsVideoOptions);
        framesReturn = loadMM8ButtonFrameSet(MM8ButtonFamily::GameOptionsReturn);

        // The 4 persistent captions.
        mm8_options_caption_turn_rate = assets->getImage_ColorKeyFromEnglishDLOD("bl_turn");
        mm8_options_caption_sound = assets->getImage_ColorKeyFromEnglishDLOD("bl_sound");
        mm8_options_caption_music = assets->getImage_ColorKeyFromEnglishDLOD("bl_music");
        mm8_options_caption_voice = assets->getImage_ColorKeyFromEnglishDLOD("bl_voice");
    } else {
        options_menu_skin.uTextureID_Background = assets->getImage_ColorKey("ControlBG");
        options_menu_skin.uTextureID_TurnSpeed[2] = assets->getImage_ColorKey("con_16x");
        options_menu_skin.uTextureID_TurnSpeed[1] = assets->getImage_ColorKey("con_32x");
        options_menu_skin.uTextureID_TurnSpeed[0] = assets->getImage_ColorKey("con_Smoo");
        options_menu_skin.uTextureID_ArrowLeft = assets->getImage_Alpha("con_ArrL");
        options_menu_skin.uTextureID_ArrowRight = assets->getImage_Alpha("con_ArrR");
        options_menu_skin.uTextureID_FlipOnExit = assets->getImage_ColorKey("option04");
        options_menu_skin.uTextureID_AlwaysRun = assets->getImage_ColorKey("option03");
        options_menu_skin.uTextureID_ShowDamage = assets->getImage_ColorKey("option02");
        options_menu_skin.uTextureID_WalkSound = assets->getImage_ColorKey("option01");
    }
    options_menu_skin.uTextureID_SoundLevels[0] = assets->getImage_ColorKey("convol10");
    options_menu_skin.uTextureID_SoundLevels[1] = assets->getImage_ColorKey("convol20");
    options_menu_skin.uTextureID_SoundLevels[2] = assets->getImage_ColorKey("convol30");
    options_menu_skin.uTextureID_SoundLevels[3] = assets->getImage_ColorKey("convol40");
    options_menu_skin.uTextureID_SoundLevels[4] = assets->getImage_ColorKey("convol50");
    options_menu_skin.uTextureID_SoundLevels[5] = assets->getImage_ColorKey("convol60");
    options_menu_skin.uTextureID_SoundLevels[6] = assets->getImage_ColorKey("convol70");
    options_menu_skin.uTextureID_SoundLevels[7] = assets->getImage_ColorKey("convol80");
    options_menu_skin.uTextureID_SoundLevels[8] = assets->getImage_ColorKey("convol90");
    options_menu_skin.uTextureID_SoundLevels[9] = assets->getImage_ColorKey("convol00");

    if (gameType() == GAME_TYPE_MM8) {
        pBtn_Mm8TurnRate16x = CreateButton("GameOptions_TurnRate16x", kMm8TurnRate16xPos, framesTurn16x.any()->size(),
            BUTTON_TYPE_NORMAL, 0, UIMSG_SetTurnSpeed, 0x80);
        pBtn_Mm8TurnRate16x->frames = framesTurn16x;
        pBtn_Mm8TurnRate32x = CreateButton("GameOptions_TurnRate32x", kMm8TurnRate32xPos, framesTurn32x.any()->size(),
            BUTTON_TYPE_NORMAL, 0, UIMSG_SetTurnSpeed, 0x40u);
        pBtn_Mm8TurnRate32x->frames = framesTurn32x;
        pBtn_Mm8TurnRateSmooth = CreateButton("GameOptions_TurnRateSmooth", kMm8TurnRateSmoothPos, framesTurnSmooth.any()->size(),
            BUTTON_TYPE_NORMAL, 0, UIMSG_SetTurnSpeed, 0);
        pBtn_Mm8TurnRateSmooth->frames = framesTurnSmooth;

        pBtn_Mm8WalkSound = CreateButton("GameOptions_WalkSound", kMm8WalkSoundPos, framesWalkSound.any()->size(),
            BUTTON_TYPE_NORMAL, 0, UIMSG_ToggleWalkSound, 0);
        pBtn_Mm8WalkSound->frames = framesWalkSound;
        pBtn_Mm8ShowHits = CreateButton("GameOptions_ShowHits", kMm8ShowHitsPos, framesShowHits.any()->size(),
            BUTTON_TYPE_NORMAL, 0, UIMSG_ToggleShowDamage, 0);
        pBtn_Mm8ShowHits->frames = framesShowHits;
        pBtn_Mm8AlwaysRun = CreateButton("GameOptions_AlwaysRun", kMm8AlwaysRunPos, framesAlwaysRun.any()->size(),
            BUTTON_TYPE_NORMAL, 0, UIMSG_ToggleAlwaysRun, 0);
        pBtn_Mm8AlwaysRun->frames = framesAlwaysRun;
        pBtn_Mm8FlipOnExit = CreateButton("GameOptions_FlipOnExit", kMm8FlipOnExitPos, framesFlipOnExit.any()->size(),
            BUTTON_TYPE_NORMAL, 0, UIMSG_ToggleFlipOnExit, 0);
        pBtn_Mm8FlipOnExit->frames = framesFlipOnExit;
    } else {
        CreateButton({22, 270}, options_menu_skin.uTextureID_TurnSpeed[2]->size(), BUTTON_TYPE_NORMAL, 0,
                     UIMSG_SetTurnSpeed, 0x80);
        CreateButton({93, 270}, options_menu_skin.uTextureID_TurnSpeed[1]->size(), BUTTON_TYPE_NORMAL, 0,
                     UIMSG_SetTurnSpeed, 0x40u);
        CreateButton({164, 270}, options_menu_skin.uTextureID_TurnSpeed[0]->size(), BUTTON_TYPE_NORMAL, 0,
                     UIMSG_SetTurnSpeed, 0);

        CreateButton({20, 303}, options_menu_skin.uTextureID_WalkSound->size(), BUTTON_TYPE_NORMAL, 0,
                     UIMSG_ToggleWalkSound, 0);
        CreateButton({128, 303}, options_menu_skin.uTextureID_ShowDamage->size(), BUTTON_TYPE_NORMAL, 0,
                     UIMSG_ToggleShowDamage, 0);
        CreateButton({20, 325}, options_menu_skin.uTextureID_AlwaysRun->size(), BUTTON_TYPE_NORMAL, 0,
                     UIMSG_ToggleAlwaysRun, 0);
        CreateButton({128, 325}, options_menu_skin.uTextureID_FlipOnExit->size(), BUTTON_TYPE_NORMAL, 0,
                     UIMSG_ToggleFlipOnExit, 0);
    }

    if (gameType() == GAME_TYPE_MM8) {
        pBtn_SliderLeft = CreateButton("GameOptions_SoundVolumeArrowLeft", {kMm8VolumeArrowLeftX, kMm8VolumeSoundY},
            options_menu_skin.uTextureID_ArrowLeft->size(), BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeSoundVolume, 4,
            INPUT_ACTION_INVALID, "", {options_menu_skin.uTextureID_ArrowLeft});
        pBtn_SliderRight = CreateButton("GameOptions_SoundVolumeArrowRight", {kMm8VolumeArrowRightX, kMm8VolumeSoundY},
            options_menu_skin.uTextureID_ArrowRight->size(), BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeSoundVolume, 5,
            INPUT_ACTION_INVALID, "", {options_menu_skin.uTextureID_ArrowRight});
        CreateButton("GameOptions_SoundVolumeTrack", {kMm8VolumeTrackX, kMm8VolumeSoundY},
            {kMm8VolumeTrackWidth, kMm8VolumeRowHeight}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeSoundVolume, 0);

        pBtn_SliderLeft = CreateButton("GameOptions_MusicVolumeArrowLeft", {kMm8VolumeArrowLeftX, kMm8VolumeMusicY},
            options_menu_skin.uTextureID_ArrowLeft->size(), BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeMusicVolume, 4,
            INPUT_ACTION_INVALID, "", {options_menu_skin.uTextureID_ArrowLeft});
        pBtn_SliderRight = CreateButton("GameOptions_MusicVolumeArrowRight", {kMm8VolumeArrowRightX, kMm8VolumeMusicY},
            options_menu_skin.uTextureID_ArrowRight->size(), BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeMusicVolume, 5,
            INPUT_ACTION_INVALID, "", {options_menu_skin.uTextureID_ArrowRight});
        CreateButton("GameOptions_MusicVolumeTrack", {kMm8VolumeTrackX, kMm8VolumeMusicY},
            {kMm8VolumeTrackWidth, kMm8VolumeRowHeight}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeMusicVolume, 0);

        pBtn_SliderLeft = CreateButton("GameOptions_VoiceVolumeArrowLeft", {kMm8VolumeArrowLeftX, kMm8VolumeVoiceY},
            options_menu_skin.uTextureID_ArrowLeft->size(), BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeVoiceVolume, 4,
            INPUT_ACTION_INVALID, "", {options_menu_skin.uTextureID_ArrowLeft});
        pBtn_SliderRight = CreateButton("GameOptions_VoiceVolumeArrowRight", {kMm8VolumeArrowRightX, kMm8VolumeVoiceY},
            options_menu_skin.uTextureID_ArrowRight->size(), BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeVoiceVolume, 5,
            INPUT_ACTION_INVALID, "", {options_menu_skin.uTextureID_ArrowRight});
        CreateButton("GameOptions_VoiceVolumeTrack", {kMm8VolumeTrackX, kMm8VolumeVoiceY},
            {kMm8VolumeTrackWidth, kMm8VolumeRowHeight}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeVoiceVolume, 0);
    } else {
        pBtn_SliderLeft = CreateButton({243, 162}, {16, 16}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeSoundVolume, 4, INPUT_ACTION_INVALID, "",
            {options_menu_skin.uTextureID_ArrowLeft});
        pBtn_SliderRight = CreateButton({435, 162}, {16, 16}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeSoundVolume, 5, INPUT_ACTION_INVALID, "",
            {options_menu_skin.uTextureID_ArrowRight});
        CreateButton({263, 162}, {172, 17}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeSoundVolume, 0);

        pBtn_SliderLeft = CreateButton({243, 216}, {16, 16}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeMusicVolume, 4, INPUT_ACTION_INVALID, "",
            {options_menu_skin.uTextureID_ArrowLeft});
        pBtn_SliderRight = CreateButton({435, 216}, {16, 16}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeMusicVolume, 5, INPUT_ACTION_INVALID, "",
            {options_menu_skin.uTextureID_ArrowRight});
        CreateButton({263, 216}, {172, 17}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeMusicVolume, 0);

        pBtn_SliderLeft = CreateButton({243, 270}, {16, 16}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeVoiceVolume, 4, INPUT_ACTION_INVALID, "",
            {options_menu_skin.uTextureID_ArrowLeft});
        pBtn_SliderRight = CreateButton({435, 270}, {16, 16}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeVoiceVolume, 5, INPUT_ACTION_INVALID, "",
            {options_menu_skin.uTextureID_ArrowRight});
        CreateButton({263, 270}, {172, 17}, BUTTON_TYPE_NORMAL, 0, UIMSG_ChangeVoiceVolume, 0);
    }

    if (gameType() == GAME_TYPE_MM8) {
        pBtn_Mm8Return = CreateButton("GameOptions_Return", kMm8ReturnPos, framesReturn.any()->size(), BUTTON_TYPE_NORMAL, 0,
            UIMSG_Escape, 0, INPUT_ACTION_INVALID, localization->str(LSTR_RETURN_TO_GAME));
        pBtn_Mm8Return->frames = framesReturn;
        pBtn_Mm8ConfigureKeyboard = CreateButton("GameOptions_ConfigureKeyboard", kMm8CfgKeyboardPos, framesConfigureKeyboard.any()->size(),
            BUTTON_TYPE_NORMAL, 0, UIMSG_OpenKeyMappingOptions, 0, INPUT_ACTION_PASS);
        pBtn_Mm8ConfigureKeyboard->frames = framesConfigureKeyboard;
        pBtn_Mm8VideoOptions = CreateButton("GameOptions_VideoOptions", kMm8VideoOptionsPos, framesVideoOptions.any()->size(),
            BUTTON_TYPE_NORMAL, 0, UIMSG_OpenVideoOptions, 0, INPUT_ACTION_OPEN_OPTIONS);
        pBtn_Mm8VideoOptions->frames = framesVideoOptions;
    } else {
        CreateButton({241, 302}, {214, 40}, BUTTON_TYPE_NORMAL, 0, UIMSG_Escape, 0, INPUT_ACTION_INVALID, localization->str(LSTR_RETURN_TO_GAME));
        CreateButton({19, 140}, {214, 40}, BUTTON_TYPE_NORMAL, 0, UIMSG_OpenKeyMappingOptions, 0, INPUT_ACTION_PASS);
        CreateButton({19, 194}, {214, 40}, BUTTON_TYPE_NORMAL, 0, UIMSG_OpenVideoOptions, 0, INPUT_ACTION_OPEN_OPTIONS);
    }
}

void GUIWindow_GameOptions::Update() {
    // MM8's "controlbg" (drawn below) already bakes its own "Might and Magic VIII" logo/title area, so the MM7
    // "options" parchment is not drawn underneath it, which would repaint a second logo.
    if (gameType() == GAME_TYPE_MM8) {
        render->DrawQuad2D(options_menu_skin.uTextureID_Background, {8, 8});
    } else {
        render->DrawQuad2D(game_ui_menu_options, {8, 8});
        render->DrawQuad2D(options_menu_skin.uTextureID_Background, {8, 132});
    }

    if (gameType() != GAME_TYPE_MM8) {
        switch ((int) engine->config->settings.TurnSpeed.value()) {
            case 64:
                render->DrawQuad2D(options_menu_skin.uTextureID_TurnSpeed[1], {BtnTurnCoord[1], 270});
                break;
            case 128:
                render->DrawQuad2D(options_menu_skin.uTextureID_TurnSpeed[2], {BtnTurnCoord[2], 270});
                break;
            default:
                render->DrawQuad2D(options_menu_skin.uTextureID_TurnSpeed[0], {BtnTurnCoord[0], 270});
                break;
        }

        if (engine->config->settings.WalkSound.value()) {
            render->DrawQuad2D(options_menu_skin.uTextureID_WalkSound, {20, 303});
        }
        if (engine->config->settings.ShowHits.value()) {
            render->DrawQuad2D(options_menu_skin.uTextureID_ShowDamage, {128, 303});
        }
        if (engine->config->settings.FlipOnExit.value()) {
            render->DrawQuad2D(options_menu_skin.uTextureID_FlipOnExit, {128, 325});
        }
        if (engine->config->settings.AlwaysRun.value()) {
            render->DrawQuad2D(options_menu_skin.uTextureID_AlwaysRun, {20, 325});
        }

        render->DrawQuad2D(options_menu_skin.uTextureID_SoundLevels[engine->config->settings.SoundLevel.value()],
            {265 + 17 * engine->config->settings.SoundLevel.value(), 162});
        render->DrawQuad2D(options_menu_skin.uTextureID_SoundLevels[engine->config->settings.MusicLevel.value()],
            {265 + 17 * engine->config->settings.MusicLevel.value(), 216});
        render->DrawQuad2D(options_menu_skin.uTextureID_SoundLevels[engine->config->settings.VoiceLevel.value()],
            {265 + 17 * engine->config->settings.VoiceLevel.value(), 270});
        return;
    }

    // The 4 persistent captions: controlbg bakes an empty olive slot for each.
    render->DrawQuad2D(mm8_options_caption_turn_rate, kMm8CaptionTurnRatePos);
    render->DrawQuad2D(mm8_options_caption_sound, kMm8CaptionSoundPos);
    render->DrawQuad2D(mm8_options_caption_music, kMm8CaptionMusicPos);
    render->DrawQuad2D(mm8_options_caption_voice, kMm8CaptionVoicePos);

    // Every widget below is drawn every frame, not skipped when a toggle is off. `select()`'s `isPressed` slot doubles
    // as "is this the engaged/selected state": the muted "_d" frame is the visually pressed-in look, not a resting one.
    // The 3 families that ship an "_h" frame (CFKB/VDOP/RETURN) use the same idiom for a real mouse hover.
    Pointi pt = mouse->position();
    auto drawFrame = [&](GUIButton *button, bool isEngaged) {
        if (!button)
            return;
        GraphicsImage *img = button->frames.select(button->Contains(pt.x, pt.y), isEngaged);
        if (img)
            render->DrawQuad2D(img, button->rect.topLeft());
    };

    int turnSpeed = (int) engine->config->settings.TurnSpeed.value();
    drawFrame(pBtn_Mm8TurnRate16x, turnSpeed == 128);
    drawFrame(pBtn_Mm8TurnRate32x, turnSpeed == 64);
    drawFrame(pBtn_Mm8TurnRateSmooth, turnSpeed != 128 && turnSpeed != 64);

    drawFrame(pBtn_Mm8WalkSound, engine->config->settings.WalkSound.value());
    drawFrame(pBtn_Mm8ShowHits, engine->config->settings.ShowHits.value());
    drawFrame(pBtn_Mm8AlwaysRun, engine->config->settings.AlwaysRun.value());
    drawFrame(pBtn_Mm8FlipOnExit, engine->config->settings.FlipOnExit.value());

    drawFrame(pBtn_Mm8ConfigureKeyboard, false);
    drawFrame(pBtn_Mm8VideoOptions, false);
    drawFrame(pBtn_Mm8Return, false);

    // convolNN (icons.lod) is a real MM8 asset. Only its screen position differs from MM7, whose literal
    // {265 + 17*level, y} would land inside the logo area.
    render->DrawQuad2D(options_menu_skin.uTextureID_SoundLevels[engine->config->settings.SoundLevel.value()],
        {kMm8VolumeMarkerX[engine->config->settings.SoundLevel.value()], kMm8VolumeSoundY});
    render->DrawQuad2D(options_menu_skin.uTextureID_SoundLevels[engine->config->settings.MusicLevel.value()],
        {kMm8VolumeMarkerX[engine->config->settings.MusicLevel.value()], kMm8VolumeMusicY});
    render->DrawQuad2D(options_menu_skin.uTextureID_SoundLevels[engine->config->settings.VoiceLevel.value()],
        {kMm8VolumeMarkerX[engine->config->settings.VoiceLevel.value()], kMm8VolumeVoiceY});
}

void GameUI_OnPlayerPortraitLeftClick(int uPlayerID) {
    Character *player = &pParty->pCharacters[uPlayerID];
    if (pParty->pPickedItem.itemId != ITEM_NULL) {
        if (std::optional<Pointi> pos = player->inventory.findSpace(pParty->pPickedItem)) {
            player->inventory.add(*pos, pParty->takeHoldingItem());
            return;
        }

        if (!player->CanAct())
            player = &pParty->activeCharacter();
        if (player->CanAct() || !pParty->activeCharacter().CanAct())
            player->playReaction(SPEECH_NO_ROOM);
    }

    if (current_screen_type == SCREEN_GAME) {
        if (pParty->hasActiveCharacter()) {
            if (pParty->activeCharacterIndex() != uPlayerID) {
                if (player->timeToRecovery || !player->CanAct())
                    return;

                pParty->setActiveCharacterIndex(uPlayerID);
                return;
            }
            pGUIWindow_CurrentMenu = std::make_unique<GUIWindow_CharacterRecord>(SCREEN_CHARACTERS);
            return;
        }
        return;
    }

    if (current_screen_type == SCREEN_SPELL_BOOK)
        return;

    if (current_screen_type == SCREEN_CHEST) {
        if (pParty->activeCharacterIndex() == uPlayerID) {
            current_character_screen_window = WINDOW_CHARACTER_INVENTORY;
            current_screen_type = SCREEN_CHEST_INVENTORY;
            return;
        }
        if (player->timeToRecovery)
            return;
        pParty->setActiveCharacterIndex(uPlayerID);
        return;
    }
    if (current_screen_type != SCREEN_HOUSE) {
        if (current_screen_type == SCREEN_SHOP_INVENTORY || current_screen_type == SCREEN_CHEST_INVENTORY) {
            pParty->setActiveCharacterIndex(uPlayerID);
            return;
        }
        if (current_screen_type != SCREEN_CHEST_INVENTORY) {
            pParty->setActiveCharacterIndex(uPlayerID);
            return;
        }
        if (pParty->activeCharacterIndex() == uPlayerID) {
            current_character_screen_window = WINDOW_CHARACTER_INVENTORY;
            current_screen_type = SCREEN_CHEST_INVENTORY;
            pParty->setActiveCharacterIndex(uPlayerID);
            return;
        }
        if (player->timeToRecovery)
            return;
        pParty->setActiveCharacterIndex(uPlayerID);
        return;
    }
    if (window_SpeakInHouse->keyboard_input_status == WINDOW_INPUT_IN_PROGRESS)
        return;

    if (pParty->activeCharacterIndex() != uPlayerID) {
        pParty->setActiveCharacterIndex(uPlayerID);
        return;
    }

    if (window_SpeakInHouse->currentDialogue() == DIALOGUE_SHOP_BUY_STANDARD ||
        window_SpeakInHouse->currentDialogue() == DIALOGUE_SHOP_BUY_SPECIAL) {
        current_character_screen_window = WINDOW_CHARACTER_INVENTORY;
        pGUIWindow_CurrentMenu = std::make_unique<GUIWindow_CharacterRecord>(SCREEN_SHOP_INVENTORY);
        return;
    }
}

std::string GameUI_GetMinimapHintText() {
    double v3;            // st7@1
    int v7;               // eax@4
    int global_coord_X;   // [sp+10h] [bp-1Ch]@1
    int global_coord_Y;   // [sp+14h] [bp-18h]@1
    int pY;      // [sp+1Ch] [bp-10h]@1
    int pX;      // [sp+28h] [bp-4h]@1

    std::string result;
    Pointi mousePos = mouse->position();
    pX = mousePos.x;
    pY = mousePos.y;
    v3 = 1.0 / (float)((signed int)viewparams->uMinimapZoom * 0.000015258789);
    global_coord_X =
        (int64_t)((double)(pX - 557) * v3 + (double)pParty->pos.x);
    global_coord_Y =
        (int64_t)((double)pParty->pos.y - (double)(pY - 74) * v3);
    if (uCurrentlyLoadedLevelType != LEVEL_OUTDOOR ||
        pOutdoor->pBModels.empty()) {
        if (engine->_currentLoadedMapId == MAP_INVALID)
            result = "No Maze Info for this maze on file!";
        else
            result = pMapTable->pInfos[engine->_currentLoadedMapId].name;
    } else {
        for (BSPModel &model : pOutdoor->pBModels) {
            v7 = Vec2i((int)model.boundingCenter.x - global_coord_X,
                       (int)model.boundingCenter.y - global_coord_Y).octagonalLength();
            if (v7 < 2 * model.boundingRadius) {
                for (int faceId : model.faces) {
                    BLVFace &face = pOutdoor->faces[faceId];
                    if (face.eventId) {
                        if (!(face.attributes & FACE_EVENT_IS_HINT)) {
                            std::string hintString = getEventHintString(face.eventId);
                            if (!hintString.empty())
                                result = hintString;
                        }
                    }
                }
                if (!result.empty())
                    return result;
            }
        }
        if (engine->_currentLoadedMapId == MAP_INVALID)
            result = "No Maze Info for this maze on file!";
        else
            result = pMapTable->pInfos[engine->_currentLoadedMapId].name;
        return result;
    }
    return result;
}

//----- (0041AD6E) --------------------------------------------------------
void GameUI_DrawRightPanelItems() {
    // game_ui_tome_quests/autonotes/storyline/maps/calendar all load MM7-only names ("ib-td1-A" etc), and no matching
    // MM8 asset exists in any LOD searched (icons.lod, icons.UI.lod, both patch LODs, EnglishD.lod). The draw is skipped
    // rather than rendering a missing-texture load. The buttons themselves (UI_Create's pBtn_Quests etc) stay live.
    if (gameType() == GAME_TYPE_MM8) return;

    if (bookFlashTimer > pParty->GetPlayingTime())
        bookFlashTimer = {};

    if (pParty->GetPlayingTime() - bookFlashTimer > 128_ticks) {
        bookFlashTimer = pParty->GetPlayingTime();
        bookFlashState = !bookFlashState;
    }

    if (bookFlashState && current_screen_type != SCREEN_REST) {
        if (bFlashQuestBook)
            render->DrawQuad2D(game_ui_tome_quests, {493, 355});
        if (bFlashAutonotesBook)
            render->DrawQuad2D(game_ui_tome_autonotes, {527, 353});
        if (bFlashHistoryBook)
            render->DrawQuad2D(game_ui_tome_storyline, {600, 361});
    }

    if (current_screen_type ==  SCREEN_BOOKS) {
        if (pGUIWindow_CurrentMenu->eWindowType == WINDOW_QUEST_BOOK)
            render->DrawQuad2D(game_ui_tome_quests, {493, 355});
        if (pGUIWindow_CurrentMenu->eWindowType == WINDOW_AUTONOTES_BOOK)
            render->DrawQuad2D(game_ui_tome_autonotes, {527, 353});
        if (pGUIWindow_CurrentMenu->eWindowType == WINDOW_JOURNAL_BOOK)
            render->DrawQuad2D(game_ui_tome_storyline, {600, 361});
        if (pGUIWindow_CurrentMenu->eWindowType == WINDOW_MAPS_BOOK)
            render->DrawQuad2D(game_ui_tome_maps, {546, 353});
        if (pGUIWindow_CurrentMenu->eWindowType == WINDOW_CALENDAR_BOOK)
            render->DrawQuad2D(game_ui_tome_calendar, {570, 353});
    }
}

static std::string toCompactString(int value) {
    std::string result = value < 1000000 ? fmt::format("{}", value) : fmt::format("{:.4g}M", value * 1e-6);
    assert(result.size() <= 6);
    return result;
}

// The two HUD-visibility predicates (see GUIWindow.h for the screen sets). These are the only screen conditions any
// MM8 HUD element may consult.
bool mm8HudVisible() {
    return gameType() == GAME_TYPE_MM8 &&
           current_screen_type == SCREEN_GAME &&
           !pMovie_Track &&
           uGameState != GAME_STATE_CHANGE_LOCATION;
}

bool mm8ScoreStripVisible() {
    return mm8HudVisible() || (gameType() == GAME_TYPE_MM8 && _44100D_should_alter_right_panel());
}

//----- (0041AEBB) --------------------------------------------------------
void GameUI_DrawFoodAndGold() {
    int text_y;  // esi@2

    if (uGameState != GAME_STATE_FINAL_WINDOW) {
        // MM8 shows food/gold via a dedicated top-of-screen icon strip ("scorebg"), not MM7's bottom-of-screen
        // right-justified text. Every MM8 screen that shows this HUD (world, house interior, character-record) uses the
        // same position, so there's no right-panel toggle.
        if (gameType() == GAME_TYPE_MM8) {
            // The strip draws on exploration and right-panel screens only. Full-screen windows (game menu, rest,
            // spellbook, books, quick reference, ...) get nothing.
            if (!mm8ScoreStripVisible())
                return;
            // scorebg is a colorkeyed icon-only sprite (transparent surround), so the bar chrome is drawn underneath it
            // first. The original paints the bar behind the strip on exploration, NPC dialogue, house/shop and the
            // character-record screen, and only the full-screen book/menu windows lack it. So the background follows
            // the strip's visibility, not the exploration-only predicate.
            if (mm8ScoreStripVisible())
                render->DrawQuad2D(game_ui_top_bar_bg, kMm8TopBarBgPos);
            render->DrawQuad2D(game_ui_scorebg, kMm8ScoreBgPos);
            GUIWindow::DrawText(assets->pFontSmallnum.get(), {0, kMm8ScoreTextY}, uGameUIFontMain, fmt::format("\t{:03}{}", kMm8FoodTextLeftMargin, toCompactString(pParty->GetFood())),
                pPrimaryWindow->frameRect, 0, uGameUIFontShadow);
            GUIWindow::DrawText(assets->pFontSmallnum.get(), {0, kMm8ScoreTextY}, uGameUIFontMain, fmt::format("\t{:03}{}", kMm8GoldTextLeftMargin, toCompactString(pParty->GetGold())),
                pPrimaryWindow->frameRect, 0, uGameUIFontShadow);
            render->EndTextNew();
            return;
        }

        text_y = _44100D_should_alter_right_panel() != 0 ? 381 : 322;

        GUIWindow::DrawText(assets->pFontSmallnum.get(), {0, text_y}, uGameUIFontMain, fmt::format("\r087{}", toCompactString(pParty->GetFood())), pPrimaryWindow->frameRect, 0, uGameUIFontShadow);
        GUIWindow::DrawText(assets->pFontSmallnum.get(), {0, text_y}, uGameUIFontMain, fmt::format("\r028{}", toCompactString(pParty->GetGold())), pPrimaryWindow->frameRect, 0, uGameUIFontShadow);
        // force to render all queued text now so it wont be delayed and drawn over things it isn't supposed to, like item in hand or nuklear
        render->EndTextNew();
    }
}

// MM8's own top-bar direction cartouche: "ib-comp-a" (icons.lod) is the same alignment-invariant horizontally-scrolled
// direction-letter strip that GameUI_DrawMinimap (below) scrolls for the minimap needle, at the same 0.1171875 px/yaw-unit
// rate, relocated to its own top-bar slot and clip window. The additive offset is MM7's own (541 clip - 285 draw = 256),
// so the same yaw maps to the same letter: 306 (this clip's x) - 256 = 50. "compcovr" (icons.UI.lod) is the gold cover
// frame drawn on top, with a colorkeyed hole punched exactly over the clip window.
// Called from Engine::drawHUD() after GUI_UpdateWindows(). GameUI_DrawFoodAndGold() paints an opaque 640x29 "topbar"
// background at (0,0) over this cartouche's whole screen position, so drawing it earlier gets painted over every tick.
void GameUI_DrawCompass() {
    // Exploration-only: full-screen windows get no compass.
    if (!mm8HudVisible()) return;

    render->DrawQuad2D(game_ui_compass_frame, {295, 4});
    render->SetUIClipRect(Recti(306, 10, 26, 9));
    render->DrawQuad2D(game_ui_compass_strip, {static_cast<int>(floorf((pParty->_viewYaw * 0.1171875) + 0.5f) + 50), 10});
    render->ResetUIClipRect();
}

// The command-icon buttons (pBtn_CastSpell/Rest/QuickReference/GameSettings) are regular GUIButton children of
// pPrimaryWindow, drawn in GUI_UpdateWindows()'s own window-update loop. That runs before GameUI_DrawFoodAndGold()
// paints its opaque 640x29 "topbar" background over the top strip, so their vTextures[0] are drawn again here, after
// GUI_UpdateWindows(), and the icons end up on top.
//
// The top bar's left bookend ("figurine", purely decorative, no button under it) and pBtn_Skull have the same exposure.
// Both bookends are exploration-only in the original: on dialogue and shop screens only the narrow food/gold strip
// shows, with neither bookend, the compass, nor the 4 command buttons. So the whole function gates on mm8HudVisible().
// The loop covers MM8's actual four top-bar buttons. pBtn_MapBook is null on the MM7 path and pBtn_CastSpell is null on
// the MM8 path, both skipped by the null guard.
void GameUI_DrawCommandIcons() {
    if (!mm8HudVisible()) return;

    render->DrawQuad2D(game_ui_topbar_figurine, kMm8TopBarFigurinePos);

    for (GUIButton *button : {pBtn_MapBook, pBtn_Rest, pBtn_QuickReference, pBtn_GameSettings}) {
        if (button && !button->vTextures.empty())
            render->DrawQuad2D(button->vTextures[0], button->rect.topLeft());
    }

    if (pBtn_Skull && !pBtn_Skull->vTextures.empty())
        render->DrawQuad2D(pBtn_Skull->vTextures[0], pBtn_Skull->rect.topLeft());
}

//----- (0041B0C9) --------------------------------------------------------
void GameUI_DrawLifeManaBars() {
    // Real MM8 portrait-adjacent HP/SP bar. `game_ui_bar_*` above load MM7 names ("ib-statG" etc) that don't exist in
    // MM8's LODs. The real bar assets are icons.lod's "manafrm" (enclosing frame) and "manag"/"manay"/"manar"/"manab"
    // (3x49 fill columns), see their load site in GUIWindow.cpp's SetUserInterface. "statbl"/"statg"/"staty"/"statr"
    // are a small 19x23 status gem (bottom-left of the portrait ring), not a bar.
    // That gem is drawn below (kMm8StatusGemPos), reusing game_ui_bar_green/yellow/red at the same HP thresholds as the
    // bar fill just below. game_ui_bar_blue ("statbl") has no observed trigger and is left unused.
    if (gameType() == GAME_TYPE_MM8) {
        // "manafrm" is 10px wide with three 1px-wide opaque vertical bars at local x=0/4/8 (left border / center divider /
        // right border, each full-height) and transparent gaps between them. The HP fill's 3px-wide column x=83..85
        // (local 1-3, left of the divider) and the SP fill's x=87..89 (local 5-7, right of the divider) sit between them,
        // not on the border lines.
        constexpr Pointi kMm8BarFramePos = {82, 428};
        constexpr int kMm8HpBarX = 83;
        constexpr int kMm8SpBarX = 87;
        constexpr int kMm8BarTopY = 430;

        // A lone hero's bars are drawn only while the hero can act. In a party every member has bars beside the portrait.
        const bool alone = pParty->activeCharacterCount() == 1;
        if (alone && !pParty->hasActiveCharacter())
            return;
        for (int slot = 0; slot < pParty->activeCharacterCount(); slot++) {
            const int shift = mm8PortraitSlotShift(slot);
            const Character &character = alone ? pParty->activeCharacter() : pParty->pCharacters[slot];
            render->DrawQuad2D(game_ui_mm8_bar_frame, kMm8BarFramePos + Pointi(shift, 0));
            if (character.health > 0) {
                double hpFillRatio = (double)character.health / (double)character.GetMaxHealth();
                auto pTextureHealth = game_ui_mm8_bar_green;
                std::shared_ptr<GraphicsImage> gemTexture = game_ui_bar_green;
                if (hpFillRatio > 0.5) {
                    if (hpFillRatio > 1.0) hpFillRatio = 1.0;
                } else if (hpFillRatio > 0.25) {
                    pTextureHealth = game_ui_mm8_bar_yellow;
                    gemTexture = game_ui_bar_yellow;
                } else if (hpFillRatio > 0.0) {
                    pTextureHealth = game_ui_mm8_bar_red;
                    gemTexture = game_ui_bar_red;
                }
                render->DrawQuad2D(gemTexture, kMm8StatusGemPos + Pointi(shift, 0));
                int height = pTextureHealth->height() * hpFillRatio;
                render->SetUIClipRect(Recti(kMm8HpBarX + shift, kMm8BarTopY + pTextureHealth->height() - height, pTextureHealth->width(), height));
                render->DrawQuad2D(pTextureHealth, {kMm8HpBarX + shift, kMm8BarTopY});
                render->ResetUIClipRect();
            }
            if (character.mana > 0) {
                double mpFillRatio = std::min(1.0, (double)character.mana / (double)character.GetMaxMana());
                int height = game_ui_mm8_bar_blue->height() * mpFillRatio;
                render->SetUIClipRect(Recti(kMm8SpBarX + shift, kMm8BarTopY + game_ui_mm8_bar_blue->height() - height, game_ui_mm8_bar_blue->width(), height));
                render->DrawQuad2D(game_ui_mm8_bar_blue, {kMm8SpBarX + shift, kMm8BarTopY});
                render->ResetUIClipRect();
            }
        }
        return;
    }

    for (int i = 0; i < pParty->activeCharacterCount(); ++i) {
        if (pParty->pCharacters[i].health > 0) {
            int v17 = 0;
            if (i == 2 || i == 3) v17 = 2;
            double hpFillRatio = (double)pParty->pCharacters[i].health / (double)pParty->pCharacters[i].GetMaxHealth();

            std::shared_ptr<GraphicsImage> pTextureHealth = game_ui_bar_green;
            if (hpFillRatio > 0.5) {
                if (hpFillRatio > 1.0)
                    hpFillRatio = 1.0;
            } else if (hpFillRatio > 0.25) {
                pTextureHealth = game_ui_bar_yellow;
            } else if (hpFillRatio > 0.0) {
                pTextureHealth = game_ui_bar_red;
            }
            if (hpFillRatio > 0.0) {
                int height = pTextureHealth->height() * hpFillRatio;
                render->SetUIClipRect(Recti(
                    v17 + pHealthBarPos[i],
                    pTextureHealth->height() - height + pHealthManaBarYPos,
                    pTextureHealth->width(),
                    height));
                render->DrawQuad2D(pTextureHealth, {v17 + pHealthBarPos[i], pHealthManaBarYPos});
                render->ResetUIClipRect();
            }
        }
        if (pParty->pCharacters[i].mana > 0) {
            double mpFillRatio = (double)pParty->pCharacters[i].mana / (double)pParty->pCharacters[i].GetMaxMana();
            if (mpFillRatio > 1.0)
                mpFillRatio = 1.0;
            int v17 = 0;
            if (i == 2) v17 = 1;
            int height = mpFillRatio * game_ui_bar_blue->height();
            render->SetUIClipRect(Recti(
                v17 + pManaBarPos[i],
                game_ui_bar_blue->height() - height + pHealthManaBarYPos,
                game_ui_bar_blue->width(), height));
            render->DrawQuad2D(game_ui_bar_blue, {v17 + pManaBarPos[i], pHealthManaBarYPos});
            render->ResetUIClipRect();
        }
    }
}

//----- (0041B3B6) --------------------------------------------------------
void GameUI_DrawRightPanel() {
    render->DrawQuad2D(game_ui_right_panel_frame, {pViewport.x + pViewport.w - 1, 0});
}

//----- (0041B3E2) --------------------------------------------------------
void GameUI_DrawRightPanelFrames() {
    render->DrawQuad2D(game_ui_topframe, {0, 0});
    render->DrawQuad2D(game_ui_leftframe, {0, 8});
    render->DrawQuad2D(game_ui_rightframe, {468, 0});
    render->DrawQuad2D(game_ui_bottomframe, {0, 352});
    GameUI_DrawRightPanelItems();

    // render->EndScene();
    // render->Present();
}

void GameUI_WritePointedObjectStatusString() {
    // GUIWindow *pWindow;                // edi@7
    // GUIButton *pButton;                // ecx@11
    int requiredSkillpoints;           // ecx@19
    UIMessageType pMessageType1;  // esi@24
    int invmatrixindex;                // eax@41
    // int v16;                           // ecx@46
    Vis_PIDAndDepth pickedObject;        // eax@55
    signed int pickedObjectID = 0;     // ecx@63
    BLVFace *pFace;                    // eax@69
    UIMessageType pMessageType2;  // esi@110
    UIMessageType pMessageType3;  // edx@117
    int pX;                   // [sp+D4h] [bp-Ch]@1
    int pY;                   // [sp+D8h] [bp-8h]@1

    // int testing;

    Pointi mousePos = mouse->position();
    pX = mousePos.x;
    pY = mousePos.y;
    Sizei renDims = render->GetRenderDimensions();
    if (pX < 0 || pX > renDims.w - 1 || pY < 0 || pY > renDims.h - 1)
        return;

    auto shouldMirror = [&](GUIWindow *pWindow) -> bool {
        if (pWindow->eWindowType != WINDOW_DIALOGUE)
            return true;
        if (current_screen_type == SCREEN_NPC_DIALOGUE)
            return true;
        if (current_screen_type == SCREEN_HOUSE && currentHouseNpc != -1 && currentHouseNpc < (int)houseNpcs.size())
            return houseNpcs[currentHouseNpc].type == HOUSE_NPC;
        return false;
    };

    if (current_screen_type == SCREEN_GAME) {
        if (pX <= (renDims.w - 1) * 0.73125 &&
            pY <= (renDims.h - 1) * 0.73125) {
            if (!pViewport.contains(Pointi(pX, pY))) {
                if (uLastPointedObjectID)
                    engine->_statusBar->clearPermanent();
                uLastPointedObjectID = Pid();
                return;
            }

            Vis_PIDAndDepth pickedObject = engine->PickMouseForInfo();
            pickedObjectID = (signed)pickedObject.pid.id();
            if (pickedObject.pid.type() == OBJECT_Sprite) {
                if (pObjectTable->pObjects[pSpriteObjects[pickedObjectID].uObjectDescID].uFlags & OBJECT_DESC_UNPICKABLE) {
                    engine->_statusBar->clearPermanent();
                    uLastPointedObjectID = Pid();
                    return;
                }
                if (pickedObject.depth >= engine->config->gameplay.MouseInteractionDepth.value() || pParty->pPickedItem.itemId != ITEM_NULL) {
                    engine->_statusBar->setPermanent(pSpriteObjects[pickedObjectID].containing_item.GetDisplayName());
                } else {
                    engine->_statusBar->setPermanent(LSTR_GET_S, pSpriteObjects[pickedObjectID].containing_item.GetDisplayName());
                }  // intentional fallthrough
            } else if (pickedObject.pid.type() == OBJECT_Decoration) {
                if (!pLevelDecorations[pickedObjectID].uEventID) {
                    std::string pText;                 // ecx@79
                    if (pLevelDecorations[pickedObjectID].IsInteractive())
                        pText = pNPCTopics[engine->_persistentVariables.decorVars[pLevelDecorations[pickedObjectID].eventVarId] + 380].pTopic; // 380 is the MM7 dispatch base, see EVENT_ChangeEvent.
                    else
                        pText = pDecorationTable->decoration(pLevelDecorations[pickedObjectID].uDecorationDescID)->hint;
                    engine->_statusBar->setPermanent(pText);
                } else {
                    std::string hintString = getEventHintString(pLevelDecorations[pickedObjectID].uEventID);
                    if (!hintString.empty())
                        engine->_statusBar->setPermanent(hintString);
                }  // intentional fallthrough
            } else if (pickedObject.pid.type() == OBJECT_Face) {
                if (pickedObject.depth < engine->config->gameplay.MouseInteractionDepth.value()) {
                    std::string newString;
                    if (uCurrentlyLoadedLevelType != LEVEL_INDOOR) {
                        const BLVFace &face = pOutdoor->faces[pickedObject.pid.id()];
                        short triggeredId = face.eventId;
                        if (triggeredId != 0)
                            newString = getEventHintString(face.eventId);
                    } else {
                        pFace = &pIndoor->faces[pickedObjectID];
                        if (pFace->attributes & FACE_INDICATE) {
                            unsigned short eventId = pFace->eventId;
                            if (eventId != 0)
                                newString = getEventHintString(eventId);
                        }
                    }
                    if (!newString.empty()) {
                        engine->_statusBar->setPermanent(newString);
                        uLastPointedObjectID = pickedObject.pid;
                        return;
                    }
                }
                engine->_statusBar->clearPermanent();
                uLastPointedObjectID = Pid();
                return;
            } else if (pickedObject.pid.type() == OBJECT_Actor) {
                engine->_statusBar->setPermanent(pActors[pickedObjectID].GetDisplayName());
            }
            if (!pickedObject.pid && uLastPointedObjectID)
                engine->_statusBar->clearPermanent();
            uLastPointedObjectID = pickedObject.pid;
            return;
        }
    } else if (current_screen_type == SCREEN_CHEST) {
        if (pX <= (renDims.w - 1) * 0.73125 &&
            pY <= (renDims.h - 1) * 0.73125) {  // if in chest area
            if (Chest::ChestUI_WritePointedObjectStatusString()) {
                return;
            } else if (uLastPointedObjectID) {  // not found so reset
                engine->_statusBar->clearPermanent();
            }
            uLastPointedObjectID = Pid();
            return;
        }
    } else {
        // if (pX <= (window->GetWidth() - 1) * 0.73125 && pY <=
        // (window->GetHeight() - 1) * 0.73125) {
        if (current_screen_type == SCREEN_CHARACTERS) {
            if (current_character_screen_window ==
                WINDOW_CHARACTER_INVENTORY) {
                if (pParty->hasActiveCharacter() && pY > 0 && pY < 350 && pX >= 13 &&
                    pX <= 462) {  // inventory poitned
                    // inventoryYCoord = (pY - 17) / 32;
                    // inventoryXCoord = (pX - 14) / 32;
                    // invMatrixIndex = inventoryXCoord + (INVETORYSLOTSWIDTH *
                    // inventoryYCoord);
                    Pointi gridPos = mapToInventoryGrid(Pointi(pX, pY), Pointi(14, 17));

                    // if (mouse.x <= 13 || mouse.x >= 462)
                    // return;
                    // testing =
                    // pParty->activeCharacter().GetItemIDAtInventoryIndex(invmatrixindex);
                    InventoryEntry pItemGen =
                        pParty->activeCharacter().inventory.entry(gridPos);

                    // TODO(captainurist): get rid of this std::to_underlying cast.
                    if (pItemGen) pickedObjectID = std::to_underlying(pItemGen->itemId);
                    // if (!pItemID)
                    // return;
                    // item =
                    // &pParty->activeCharacter().pInventoryItemList[pItemID -
                    // 1];

                    // v14 = render->pActiveZBuffer[pX +
                    // pSRZBufferLineOffsets[pY]];
                    if (pickedObjectID == 0 || pickedObjectID == -65536 ||
                        pickedObjectID >= 5000) {
                        if (uLastPointedObjectID)
                            engine->_statusBar->clearPermanent();
                        uLastPointedObjectID = Pid();
                        // return;
                    } else {
                        engine->_statusBar->setPermanent(pItemGen->GetDisplayName());
                        uLastPointedObjectID = Pid::dummy();
                        return;
                    }
                }
            }
        }

        // else if inventory

        for (GUIWindow *pWindow : lWindowList) {
            if (pWindow->Contains(pX, pY)) {
                for (GUIButton *pButton : pWindow->vButtons) {
                    switch (pButton->uButtonType) {
                        case BUTTON_TYPE_NORMAL:  // for dialogue window
                            if (pButton->Contains(pX, pY)) {
                                if (shouldMirror(pWindow))
                                    engine->_statusBar->setPermanent(pButton->label);
                                pMessageType1 = (UIMessageType)pButton->uData;
                                if (pMessageType1)
                                    GameUI_handleHintMessage(pMessageType1, pButton->msg_param);

                                uLastPointedObjectID = Pid::dummy();
                                return;
                            }
                            break;
                        case BUTTON_TYPE_CHARACTER:  // hovering over portraits
                            if (pButton->containsOval(mousePos)) {
                                engine->_statusBar->setPermanent(pButton->label);  // for character name
                                pMessageType2 = (UIMessageType)pButton->uData;
                                if (pMessageType2 != 0)
                                    GameUI_handleHintMessage(pMessageType2, pButton->msg_param);

                                uLastPointedObjectID = Pid::dummy();
                                return;
                            }
                            break;
                        case BUTTON_TYPE_SKILLS:  // hovering over buttons
                            if (pButton->Contains(pX, pY)) {
                                Skill skill = static_cast<Skill>(pButton->msg_param);
                                int skillLevel = pParty->activeCharacter().getSkillValue(skill).level();
                                requiredSkillpoints = skillLevel + 1;

                                if (skills_max_level[skill] <= skillLevel)
                                    engine->_statusBar->setPermanent(LSTR_YOU_HAVE_ALREADY_MASTERED_THIS_SKILL);
                                else if (pParty->activeCharacter().uSkillPoints < requiredSkillpoints)
                                    engine->_statusBar->setPermanent(LSTR_YOU_NEED_D_MORE_SKILL_POINTS_TO_ADVANCE, requiredSkillpoints - pParty->activeCharacter().uSkillPoints);
                                else
                                    engine->_statusBar->setPermanent(LSTR_CLICKING_HERE_WILL_SPEND_D_SKILL_POINTS, requiredSkillpoints);

                                uLastPointedObjectID = Pid::dummy();
                                return;
                            }
                            break;
                    }
                }
            }

            // ?? if we get here nothing is curos over??
            if (uLastPointedObjectID)  // not found so reset
                engine->_statusBar->clearPermanent();
            uLastPointedObjectID = Pid();

            if (pWindow->frameRect.h == 480) {
                // DebugBreak(); //Why is this condition here (in the original
                // too)? Might check fullscreen windows. Let Silvo know if you
                // find out
                // this is to stop the no windows active code below runnning
                return;
            }
        }
        // The game never gets to this point even in the original. It's also
        // bugged(neither branch displays anything).
        // TODO(_) fix these and move them up before the window check loop.

        // gets here when glitched

        /* if (current_screen_type == SCREEN_HOUSE)  // this is required
        when displaying inventory in a house/shop??
        {
        if (window_SpeakInHouse->currentDialogue() != DIALOGUE_SHOP_BUY_STANDARD
        || (v16 = render->pActiveZBuffer[pX + pSRZBufferLineOffsets[pY]], v16 ==
        0)
        || v16 == -65536)
        {
        if (uLastPointedObjectID != 0)
        {
        engine->_statusBar->clearPermanent();
        }
        uLastPointedObjectID = 0;
        return;
        }
        pItemGen = (ItemGen *)((char *)&pParty->pPickedItem + 36 * (v16 + 12 *
        (unsigned int)window_SpeakInHouse->ptr_1C) + 4);
        engine->_statusBar->setPermanent(pItemGen->GetDisplayName());
        engine->_statusBar->clearPermanent();
        uLastPointedObjectID = 0;
        return;
        }
        */

        //}
    }

    // no windows active -outside of game screen area only
    if (!lWindowList.empty()) {
        GUIWindow *win = lWindowList.back();
        if (win->Contains(pX, pY)) {
            // TODO(pskelton): this looks duplicated extract
            for (GUIButton *pButton : win->vButtons) {
                switch (pButton->uButtonType) {
                    case BUTTON_TYPE_NORMAL:
                        if (pButton->Contains(mousePos)) {
                            pMessageType3 = (UIMessageType)pButton->uData;
                            if (pMessageType3 == 0) {  // For books
                                engine->_statusBar->setPermanent(pButton->label);
                            } else {
                                GameUI_handleHintMessage(pMessageType3, pButton->msg_param);
                            }
                            uLastPointedObjectID = Pid::dummy();
                            return;
                        }
                        break;
                    case BUTTON_TYPE_CHARACTER:  // hovering over portraits
                        if (pButton->containsOval(mousePos)) {
                            engine->_statusBar->setPermanent(pButton->label);  // for character name
                            pMessageType2 = (UIMessageType)pButton->uData;
                            if (pMessageType2 != 0)
                                GameUI_handleHintMessage(pMessageType2, pButton->msg_param);
                            uLastPointedObjectID = Pid::dummy();
                            return;
                        }
                        break;
                    case BUTTON_TYPE_SKILLS:
                        // is this one needed?
                        assert(false);  // how does this work?
                             /*                if (pX >= pButton->uX && pX <=
                             pButton->uZ
                             && pY >= pButton->uY && pY <= pButton->uW)
                             {
                             requiredSkillpoints =
                             (pParty->activeCharacter().pActiveSkills[pButton->msg_param]
                             & 0x3F) + 1;

                             std::string str;
                             if (pParty->activeCharacter().uSkillPoints <
                             requiredSkillpoints)      str =
                             localization->formatString(
                             LSTR_YOU_NEED_D_MORE_SKILL_POINTS_TO_ADVANCE, requiredSkillpoints -
                             pParty->activeCharacter().uSkillPoints);
                             else      str =
                             localization->formatString(
                             LSTR_CLICKING_HERE_WILL_SPEND_D_SKILL_POINTS, requiredSkillpoints);
                             engine->_statusBar->setPermanent(str);
                             uLastPointedObjectID = 1;
                             return;
                             }*/
                        break;
                }
            }
        }
    }

    if (uLastPointedObjectID)
        engine->_statusBar->clearPermanent();
    uLastPointedObjectID = Pid();
}

//----- (0044158F) --------------------------------------------------------
void GameUI_DrawCharacterSelectionFrame() {
    // MM8 always shows its portrait ring ("selring", drawn in GameUI_DrawPortraits)
    // instead of a separate flashing active-character indicator - there's only ever 1 active
    // character, so a distinct "selected" state doesn't apply.
    if (gameType() == GAME_TYPE_MM8) return;

    if (pParty->hasActiveCharacter())
        render->DrawQuad2D(game_ui_player_selection_frame,
            {pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[pParty->activeCharacterIndex()] - 9, 380});
}

//----- (0044162D) --------------------------------------------------------
void GameUI_DrawPartySpells() {
    // MM8 ships no "isn-NN" party-buff icon under any name searched (icons.lod, icons.UI.lod, EnglishD.lod).
    // `party_buff_icons[i]` is dereferenced directly below (`icon->width()`, not through the null-safe DrawQuad2D), so
    // the whole right-panel spell-buff strip is skipped for MM8. LoadPartyBuffIcons (GUIWindow.cpp) is gated to match.
    if (gameType() != GAME_TYPE_MM8) {
        for (int i = 0; i < spellBuffsAtRightPanel.size(); ++i) {
            if (pParty->pPartyBuffs[spellBuffsAtRightPanel[i]].Active()) {
                std::shared_ptr<GraphicsImage> icon = party_buff_icons[i];
                AtlasLayout layout({16, 8}, {icon->width() / 16, icon->height() / 8});
                int frame = (animTimer->time().realtimeMilliseconds() / 20 + 20 * pPartySpellbuffsUI_smthns[i]) % 126;
                render->DrawQuad2D(icon, layout[frame], pPartySpellbuffsUI_XYs[i]);
            }
        }
    }

    if (current_screen_type == SCREEN_GAME || current_screen_type == SCREEN_NPC_DIALOGUE) {
        // Flight / water walk animation is purposefully slowed down compared to what's in the data files.
        Duration frameTime = animTimer->time() * 50 / 128;

        std::shared_ptr<GraphicsImage> spell_texture;  // [sp-4h] [bp-1Ch]@12

        if (pParty->FlyActive()) {
            if (pParty->bFlying)
                spell_texture = pIconsFrameTable->animationFrame(uIconIdx_FlySpell, frameTime);
            else
                spell_texture = pIconsFrameTable->animationFrame(uIconIdx_FlySpell, 0_ticks);
            render->DrawQuad2D(spell_texture, {8, 8});
        }

        if (pParty->WaterWalkActive()) {
            if (pParty->uFlags & PARTY_FLAG_STANDING_ON_WATER)
                spell_texture = pIconsFrameTable->animationFrame(uIconIdx_WaterWalk, frameTime);
            else
                spell_texture = pIconsFrameTable->animationFrame(uIconIdx_WaterWalk, 0_ticks);
            render->DrawQuad2D(spell_texture, {396, 8});
        }
    }

    // MM8 ships no "isg-NN-a/b/c" character-buff icon under any alignment suffix (checked every LOD). The loads in
    // SetUserInterface (GUIWindow.cpp) are gated to match, so these globals stay null for MM8. DrawQuad2D is null-safe.
    if (gameType() != GAME_TYPE_MM8) {
        for (int i = 0; i < pParty->activeCharacterCount(); ++i) {
            if (pParty->pCharacters[i].pCharacterBuffs[CHARACTER_BUFF_HAMMERHANDS].Active())
                render->DrawQuad2D(game_ui_playerbuff_hammerhands, {pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72, 427});
            if (pParty->pCharacters[i].pCharacterBuffs[CHARACTER_BUFF_BLESS].Active())
                render->DrawQuad2D(game_ui_playerbuff_bless, {pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72, 393});
            if (pParty->pCharacters[i].pCharacterBuffs[CHARACTER_BUFF_PRESERVATION].Active())
                render->DrawQuad2D(game_ui_playerbuff_preservation, {pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72, 410});
            if (pParty->pCharacters[i].pCharacterBuffs[CHARACTER_BUFF_PAIN_REFLECTION].Active())
                render->DrawQuad2D(game_ui_playerbuff_pain_reflection, {pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] + 72, 444});
        }
    }
}

//----- (004921C1) --------------------------------------------------------
void GameUI_DrawPortraits() {
    std::shared_ptr<GraphicsImage> pPortrait;                 // [sp-4h] [bp-1Ch]@27

    pParty->updateDelayedReaction();

    bool isMm8 = gameType() == GAME_TYPE_MM8;

    // Bottom-bar chrome the portrait/ring/banners below are drawn on top of. selring is a colorkeyed icon-only sprite
    // (transparent surround), so this has to go first or the raw 3D scene shows through around the portrait/banners.
    if (isMm8)
        render->DrawQuad2D(game_ui_bottom_bar_bg, kMm8BottomBarBgPos);

    for (int i = 0; i < pParty->activeCharacterCount(); ++i) {
        Character *pPlayer = &pParty->pCharacters[i];
        // MM8's first portrait sits inside its own bottom-bar ring (see kMm8PortraitX/Y), the others where the banners stand.
        int portraitX = isMm8 ? kMm8PortraitX + mm8PortraitSlotShift(i) : pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i];
        int portraitY = isMm8 ? kMm8PortraitY : 388;

        if (pPlayer->IsEradicated()) {
            pPortrait = game_ui_player_face_eradicated;
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active())
                render->DrawQuad2D(pPortrait, {portraitX, isMm8 ? portraitY : 387}, colorTable.MediumGrey); // was 388
            else
                render->DrawQuad2D(pPortrait, {portraitX + 1, isMm8 ? portraitY : 387}); // was 388
            continue;
        }
        if (pPlayer->IsDead()) {
            pPortrait = game_ui_player_face_dead;
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active())
                render->DrawQuad2D(pPortrait, {portraitX, portraitY}, colorTable.MediumGrey);
            else
                render->DrawQuad2D(pPortrait, {portraitX + 1, portraitY});
            continue;
        }

        int faceTextureIndex = 1;
        if (pPlayer->portrait == PORTRAIT_TALK)
            faceTextureIndex = pPlayer->talkAnimation.currentFrameIndex();
        else
            faceTextureIndex = pPortraitFrameTable->animationFrameIndex(pPortraitFrameTable->animationId(pPlayer->portrait),
                                                                        pPlayer->portraitTimePassed);
        if (true /* || pPlayer->uExpressionImageIndex != pFrame->uTextureID - 1*/) {
            pPlayer->portraitImageIndex = faceTextureIndex - 1;
            pPortrait = game_ui_player_faces[i][pPlayer->portraitImageIndex];  // pFace = (Texture_MM7*)game_ui_player_faces[i][pFrame->uTextureID];
            if (pParty->pPartyBuffs[PARTY_BUFF_INVISIBILITY].Active())
                render->DrawQuad2D(pPortrait, {portraitX, portraitY}, colorTable.MediumGrey);
            else
                render->DrawQuad2D(pPortrait, {portraitX + 1, portraitY});
            continue;
        }
    }

    if (isMm8) {
        // The ring marks the member who acts. A banner stands in each slot no member fills.
        int ringSlot = pParty->hasActiveCharacter() ? pParty->activeCharacterIndex() : 0;
        render->DrawQuad2D(game_ui_portrait_ring, {kMm8PortraitX + mm8PortraitSlotShift(ringSlot), kMm8PortraitY});
        for (int i = std::max(0, pParty->activeCharacterCount() - 1); i < kMm8BannerX.size(); ++i)
            render->DrawQuad2D(game_ui_party_banners[i], {kMm8BannerX[i], kMm8BannerY});
    }
    // MM8 ships no "IB-Init*-a/b/c" alert-glow asset at all, and the original shows no such indicator over the portrait.
    // Skipped, rather than drawing whatever a missing-texture load resolves to over the portrait.
    if (isMm8) {
        // no-op: MM8's always-on HUD has no per-character "ready to act" alert indicator.
    } else if (pParty->bTurnBasedModeOn) {
        if (pTurnEngine->turn_stage != TE_WAIT) {
            if (pTurnEngine->pQueue[0].uPackedID.type() == OBJECT_Character) {
                for (unsigned i = 0; i < pTurnEngine->pQueue.size(); ++i) {
                    if (pTurnEngine->pQueue[i].uPackedID.type() != OBJECT_Character)
                        break;

                    std::shared_ptr<GraphicsImage> alert_texture = game_ui_player_alert_green;
                    if (pParty->GetRedAlert())
                        alert_texture = game_ui_player_alert_red;
                    else if (pParty->GetYellowAlert())
                        alert_texture = game_ui_player_alert_yellow;

                    render->DrawQuad2D(alert_texture,
                        {pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[pTurnEngine->pQueue[i].uPackedID.id()] - 4, 384}); // was 385
                }
            }
        }
    } else {
        for (int i = 0; i < pParty->activeCharacterCount(); ++i) {
            if (pParty->pCharacters[i].CanAct() && !pParty->pCharacters[i].timeToRecovery) {
                std::shared_ptr<GraphicsImage> alert_texture = game_ui_player_alert_green;
                if (pParty->GetRedAlert())
                    alert_texture = game_ui_player_alert_red;
                else if (pParty->GetYellowAlert())
                    alert_texture = game_ui_player_alert_yellow;

                render->DrawQuad2D(alert_texture, {pPlayerPortraitsXCoords_For_PlayerBuffAnimsDrawing[i] - 4, 384}); // was 385
            }
        }
    }
}

//----- (00441D38) --------------------------------------------------------
void GameUI_DrawMinimap(const Recti &rect, int zoom) {
    // signed int pW;   // ebx@23
    int LineGreyDim;         // eax@23
    //double startx;      // st7@30
    // signed int ypix;  // eax@37
    // uint16_t *v28; // ecx@37
    // signed int xpix;       // edi@40
    int pPoint_X;         // edi@72
    int pPoint_Y;         // ebx@72
    // unsigned int lPitch;  // [sp+20h] [bp-34h]@1
    // signed int pY;        // [sp+20h] [bp-34h]@23
    // signed int pX;        // [sp+24h] [bp-30h]@23
    signed int xpixoffset16;       // [sp+24h] [bp-30h]@37
    signed int ypixoffset16;    // [sp+28h] [bp-2Ch]@37
    // int map_scale;              // [sp+2Ch] [bp-28h]@30
    // signed int pZ;        // [sp+60h] [bp+Ch]@23
    //double starty;            // [sp+60h] [bp+Ch]@30
    Color pColor;

    Pointi center = rect.center();
    render->SetUIClipRect(rect);

    bool bWizardEyeActive = pParty->wizardEyeActive();
    Mastery uWizardEyeSkillLevel = pParty->wizardEyeSkillLevel();
    if (CheckHiredNPCSpeciality(NPC_PROFESSION_CARTOGRAPHER)) {
        bWizardEyeActive = true;
        uWizardEyeSkillLevel = uWizardEyeSkillLevel > MASTERY_EXPERT ? uWizardEyeSkillLevel : MASTERY_EXPERT;
    }

    if (engine->config->debug.WizardEye.value()) {
        bWizardEyeActive = true;
        uWizardEyeSkillLevel = MASTERY_MASTER;
    }

    if (uCurrentlyLoadedLevelType == LEVEL_OUTDOOR) {
        // TODO(captainurist): function-local static that owns a texture, make it a member.
        static std::unique_ptr<GraphicsImage> minimaptemp;

        bool partymoved = true;  // TODO(pskelton): actually check for party movement

        if (partymoved) {
            int imageWidth = viewparams->location_minimap->width(); // Assume a square image.

            // Party position in fixpoint image coordinates.
            // Map is 2^16 by 2^16 in in-game coords, but (0, 0) is in the center of the map.
            int partyx16 = static_cast<int>(pParty->pos.x + 32768) * imageWidth;
            int partyy16 = static_cast<int>(32768 - pParty->pos.y) * imageWidth;

            // Top-left corner position in fixpoint image coordinates.
            int startx16 = partyx16 - (rect.w << 16) / (2 * zoom) * imageWidth;
            int starty16 = partyy16 - (rect.h << 16) / (2 * zoom) * imageWidth;

            // TODO(pskelton): could stretch texture rather than rescale
            // MM8's real minimap frame ("mapframe", icons.UI.lod) has a smaller interior content area than MM7's.
            // See the call site in Engine.cpp.
            if (gameType() == GAME_TYPE_MM8) {
                assert(rect.w == 124 && rect.h == 93);
            } else {
                assert(rect.w == 137 && rect.h == 117);
            }

            RgbaImage minimapImage = RgbaImage::solid(Color(), rect.size());
            int step16 = (1 << 16) * imageWidth / zoom;
            for (int dstY = 0, srcY16 = starty16; dstY < rect.h; ++dstY, srcY16 += step16) {
                std::span<Color> dstLine = minimapImage[dstY];
                std::span<const Color> srcLine = viewparams->location_minimap->rgba()[srcY16 >> 16];
                for (int dstX = 0, srcX16 = startx16; dstX < rect.w; ++dstX, srcX16 += step16)
                    dstLine[dstX] = srcLine[srcX16 >> 16];
            }

            // draw image
            minimaptemp = GraphicsImage::Create(std::move(minimapImage));
            render->DrawQuad2D(minimaptemp, rect.topLeft());
        } else {
            // no need to update map - just redraw
            render->DrawQuad2D(minimaptemp, rect.topLeft());
        }
        render->BeginLines2D();
    } else if (uCurrentlyLoadedLevelType == LEVEL_INDOOR) {
        render->FillRect(rect, colorTable.NavyBlue);
        uNumBlueFacesInBLVMinimap = 0;
        render->BeginLines2D();
        for (unsigned i = 0; i < (unsigned)pIndoor->mapOutlines.size(); ++i) {
            BLVMapOutline *pOutline = &pIndoor->mapOutlines[i];

            if (pIndoor->faces[pOutline->uFace1ID].Visible() &&
                pIndoor->faces[pOutline->uFace2ID].Visible()) {
                if (pOutline->uFlags & 1 || pIndoor->faces[pOutline->uFace1ID].attributes & FACE_SeenByParty ||
                    pIndoor->faces[pOutline->uFace2ID].attributes & FACE_SeenByParty) {
                    pOutline->uFlags = pOutline->uFlags | 1;
                    pIndoor->_visible_outlines[i >> 3] |= 1 << (7 - i % 8);

                    // Outdoor map size is 65536 x 65536, so we're normalizing the coords the same way it's done for
                    // outdoor maps.
                    Vec2f Vert1 = (pIndoor->vertices[pIndoor->mapOutlines[i].uVertex1ID] - pParty->pos).xy() / 65536.0f;
                    Vec2f Vert2 = (pIndoor->vertices[pIndoor->mapOutlines[i].uVertex2ID] - pParty->pos).xy() / 65536.0f;

                    // In-game VS screen-space Y are flipped.
                    Vert1.y = -Vert1.y;
                    Vert2.y = -Vert2.y;

                    Vec2i linea = center + (zoom * Vert1).toInt();
                    Vec2i lineb = center + (zoom * Vert2).toInt();

                    if (bWizardEyeActive && uWizardEyeSkillLevel >= MASTERY_MASTER &&
                        (pIndoor->faces[pOutline->uFace1ID].Clickable() ||
                            pIndoor->faces[pOutline->uFace2ID].Clickable()) &&
                        (pIndoor->faces[pOutline->uFace1ID].eventId ||
                            pIndoor->faces[pOutline->uFace2ID].eventId)) {
                        if (uNumBlueFacesInBLVMinimap < 49) {
                            pBlueFacesInBLVMinimapIDs[uNumBlueFacesInBLVMinimap++] = i;
                            continue;
                        }
                    }

                    LineGreyDim = std::abs(pOutline->sZ - pParty->pos.z) / 8;
                    if (LineGreyDim > 100) LineGreyDim = 100;
                    render->RasterLine2D(linea, lineb, viewparams->pPalette[-LineGreyDim + 200]);
                }
            }
        }

        for (unsigned i = 0; i < uNumBlueFacesInBLVMinimap; ++i) {
            BLVMapOutline *pOutline = &pIndoor->mapOutlines[pBlueFacesInBLVMinimapIDs[i]];
            int pX = center.x + zoom * (pIndoor->vertices[pOutline->uVertex1ID].x - pParty->pos.x) / 65536.0f;
            int pY = center.y - zoom * (pIndoor->vertices[pOutline->uVertex1ID].y - pParty->pos.y) / 65536.0f;
            int pZ = center.x + zoom * (pIndoor->vertices[pOutline->uVertex2ID].x - pParty->pos.x) / 65536.0f;
            int pW = center.y - zoom * (pIndoor->vertices[pOutline->uVertex2ID].y - pParty->pos.y) / 65536.0f;
            render->RasterLine2D(Pointi(pX, pY), Pointi(pZ, pW), ui_game_minimap_outline_color);
        }
    }

    // opengl needs slightly modified lines to show up properly
    int lineadj = 1;

    // draw objects on the minimap
    if (bWizardEyeActive) {
        if (uWizardEyeSkillLevel >= MASTERY_EXPERT) {
            for (unsigned i = 0; i < pSpriteObjects.size(); ++i) {
                if (pSpriteObjects[i].spriteId == SPRITE_NULL || !pSpriteObjects[i].uObjectDescID)
                    continue;
                // if (uWizardEyeSkillLevel == 1
                pPoint_X = center.x + (pSpriteObjects[i].vPosition.x - pParty->pos.x) * zoom / 65536.0f;
                pPoint_Y = center.y - (pSpriteObjects[i].vPosition.y - pParty->pos.y) * zoom / 65536.0f;
                // if ( pPoint_X >= render->raster_clip_x && pPoint_X <=
                // render->raster_clip_z &&
                //     pPoint_Y >= render->raster_clip_y && pPoint_Y <=
                //     render->raster_clip_w)
                {
                    if (pObjectTable->pObjects[pSpriteObjects[i].uObjectDescID]
                            .uFlags &
                        OBJECT_DESC_UNPICKABLE) {
                        render->RasterLine2D(Pointi(pPoint_X, pPoint_Y), Pointi(pPoint_X + 1, pPoint_Y + 1),
                                             ui_game_minimap_projectile_color);
                    } else if (zoom > 512) {
                        render->RasterLine2D(Pointi(pPoint_X - 2, pPoint_Y), Pointi(pPoint_X - 2, pPoint_Y + 1 + lineadj),
                                             ui_game_minimap_treasure_color);
                        render->RasterLine2D(Pointi(pPoint_X - 1, pPoint_Y - 1), Pointi(pPoint_X - 1, pPoint_Y + 1 + lineadj),
                                             ui_game_minimap_treasure_color);
                        render->RasterLine2D(Pointi(pPoint_X, pPoint_Y - 2), Pointi(pPoint_X, pPoint_Y + 1 + lineadj),
                                             ui_game_minimap_treasure_color);
                        render->RasterLine2D(Pointi(pPoint_X + 1, pPoint_Y - 1), Pointi(pPoint_X + 1, pPoint_Y + 1 + lineadj),
                                             ui_game_minimap_treasure_color);
                        render->RasterLine2D(Pointi(pPoint_X + 2, pPoint_Y), Pointi(pPoint_X + 2, pPoint_Y + 1 + lineadj),
                                             ui_game_minimap_treasure_color);
                    } else {
                        render->RasterLine2D(Pointi(pPoint_X - 1, pPoint_Y - 1), Pointi(pPoint_X - 1, pPoint_Y + lineadj),
                                             ui_game_minimap_treasure_color);
                        render->RasterLine2D(Pointi(pPoint_X, pPoint_Y - 1), Pointi(pPoint_X, pPoint_Y + lineadj),
                                             ui_game_minimap_treasure_color);
                    }
                }
            }
        }
        for (const Actor &actor : pActors) {
            if (actor.aiState != Removed &&
                actor.aiState != Disabled &&
                (actor.aiState == Dead || actor.ActorNearby())) {
                pPoint_X = center.x + (actor.pos.x - pParty->pos.x) * zoom / 65536.0f;
                pPoint_Y = center.y - (actor.pos.y - pParty->pos.y) * zoom / 65536.0f;
                // if ( pPoint_X >= render->raster_clip_x && pPoint_X <=
                // render->raster_clip_z
                //  && pPoint_Y >= render->raster_clip_y && pPoint_Y <=
                //  render->raster_clip_w )
                {
                    pColor = ui_game_minimap_actor_friendly_color;
                    if (actor.attributes & ACTOR_HOSTILE)
                        pColor = ui_game_minimap_actor_hostile_color;
                    if (actor.aiState == Dead)
                        pColor = ui_game_minimap_actor_corpse_color;
                    if (zoom > 1024) {
                        render->RasterLine2D(Pointi(pPoint_X - 2, pPoint_Y - 1), Pointi(pPoint_X - 2, pPoint_Y + 1 + lineadj),
                                             pColor);
                        render->RasterLine2D(Pointi(pPoint_X - 1, pPoint_Y - 2), Pointi(pPoint_X - 1, pPoint_Y + 2 + lineadj),
                                             pColor);
                        render->RasterLine2D(Pointi(pPoint_X, pPoint_Y - 2), Pointi(pPoint_X, pPoint_Y + 2 + lineadj),
                                             pColor);
                        render->RasterLine2D(Pointi(pPoint_X + 1, pPoint_Y - 2), Pointi(pPoint_X + 1, pPoint_Y + 2 + lineadj),
                                             pColor);
                        render->RasterLine2D(Pointi(pPoint_X + 2, pPoint_Y - 1), Pointi(pPoint_X + 2, pPoint_Y + 1 + lineadj),
                                             pColor);
                    } else {
                        render->RasterLine2D(Pointi(pPoint_X - 1, pPoint_Y - 1), Pointi(pPoint_X - 1, pPoint_Y + lineadj),
                                             pColor);
                        render->RasterLine2D(Pointi(pPoint_X, pPoint_Y - 1), Pointi(pPoint_X, pPoint_Y + lineadj),
                                             pColor);
                    }
                }
            }
        }
        for (unsigned i = 0; i < (signed int)pLevelDecorations.size(); ++i) {
            if (pLevelDecorations[i].uFlags & LEVEL_DECORATION_VISIBLE_ON_MAP) {
                pPoint_X = center.x + (pLevelDecorations[i].vPosition.x - pParty->pos.x) * zoom / 65536.0f;
                pPoint_Y = center.y - (pLevelDecorations[i].vPosition.y - pParty->pos.y) * zoom / 65536.0f;

                // if ( pPoint_X >= render->raster_clip_x && pPoint_X <=
                // render->raster_clip_z
                //  && pPoint_Y >= render->raster_clip_y && pPoint_Y <=
                //  render->raster_clip_w )
                {
                    if ((signed int)zoom > 512) {
                        render->RasterLine2D(Pointi(pPoint_X - 1, pPoint_Y - 1), Pointi(pPoint_X - 1, pPoint_Y + 1), ui_game_minimap_decoration_color_1);
                        render->RasterLine2D(Pointi(pPoint_X, pPoint_Y - 1), Pointi(pPoint_X, pPoint_Y + 1), ui_game_minimap_decoration_color_1);
                        render->RasterLine2D(Pointi(pPoint_X + 1, pPoint_Y - 1), Pointi(pPoint_X + 1, pPoint_Y + 1), ui_game_minimap_decoration_color_1);
                    } else {
                        render->RasterLine2D(Pointi(pPoint_X, pPoint_Y), Pointi(pPoint_X, pPoint_Y), ui_game_minimap_decoration_color_1);
                    }
                }
            }
        }
    }

    render->EndLines2D();

    // draw arrow on the minimap(include. Ritor1)
    unsigned arrow_idx;
    int rotate = pParty->_viewYaw & TrigLUT.uDoublePiMask;
    if (rotate <= 1920) arrow_idx = 6;
    if (rotate < 1664) arrow_idx = 5;
    if (rotate <= 1408) arrow_idx = 4;
    if (rotate < 1152) arrow_idx = 3;
    if (rotate <= 896) arrow_idx = 2;
    if (rotate < 640) arrow_idx = 1;
    if (rotate <= 384) arrow_idx = 0;
    if (rotate < 128 || rotate > 1920) arrow_idx = 7;
    render->DrawQuad2D(game_ui_minimap_dirs[arrow_idx], {center.x - 3, center.y - 3});

    // game_ui_minimap_compass stays nullptr for MM8: the top-bar compass cartouche (GameUI_DrawCompass, above) uses its
    // own game_ui_compass_strip pointer, and DrawQuad2D is null-safe, so this MM7-only needle draw is a no-op for MM8.
    // The clip reset below must run for every game type. Without it the clip rect stays at the minimap's content rect
    // into the next tick's drawWorld(), and GameUI_DrawFoodAndGold() (called outside DrawGUI()) is clipped to nothing.
    render->SetUIClipRect(Recti(541, 0, 26, 480));
    render->DrawQuad2D(game_ui_minimap_compass, {static_cast<int>(floorf((pParty->_viewYaw * 0.1171875) + 0.5f) + 285), 136});
    render->ResetUIClipRect();
    // The real MM8 frame sits bottom-right, not MM7's top-right.
    render->DrawQuad2D(game_ui_minimap_frame, gameType() == GAME_TYPE_MM8 ? Pointi(481, 367) : Pointi(468, 0));
}

// pBtn_ZoomIn/pBtn_ZoomOut's own OnButtonClick flash (Game.cpp) only draws vTextures[0] for one frame on click, so
// they need a per-frame redraw. Not drawn from inside GameUI_DrawMinimap: that function's outdoor branch runs
// BeginLines2D()/EndLines2D() just above, which leaves the GL blend state dirty for the next DrawQuad2D, so the
// button's opaque pixels come out blended with the 3D world. Called from drawHUD() after GUI_UpdateWindows(), like
// GameUI_DrawCommandIcons().
void GameUI_DrawZoomButtons() {
    // The zoom buttons sit on the minimap frame, so they're exploration-only with it.
    if (!mm8HudVisible())
        return;

    if (pBtn_ZoomIn && !pBtn_ZoomIn->vTextures.empty())
        render->DrawQuad2D(pBtn_ZoomIn->vTextures[0], pBtn_ZoomIn->rect.topLeft());
    if (pBtn_ZoomOut && !pBtn_ZoomOut->vTextures.empty())
        render->DrawQuad2D(pBtn_ZoomOut->vTextures[0], pBtn_ZoomOut->rect.topLeft());
}

//----- (00441498) --------------------------------------------------------
void GameUI_DrawTorchlightAndWizardEye() {
    if (current_screen_type == SCREEN_GAME ||
        current_screen_type == SCREEN_MENU ||
        current_screen_type == SCREEN_OPTIONS ||
        current_screen_type == SCREEN_REST ||
        current_screen_type == SCREEN_SPELL_BOOK ||
        current_screen_type == SCREEN_CHEST ||
        current_screen_type == SCREEN_SAVEGAME ||
        current_screen_type == SCREEN_LOADGAME ||
        current_screen_type == SCREEN_CHEST_INVENTORY ||
        current_screen_type == SCREEN_BOOKS ||
        current_screen_type == SCREEN_BRANCHLESS_NPC_DIALOG ||
        current_screen_type == SCREEN_QUICK_REFERENCE) {
        if (pParty->TorchlightActive())
            render->DrawQuad2D(pIconsFrameTable->animationFrame(game_ui_torchLight, animTimer->time()), {468, 0});
        if (pParty->wizardEyeActive())
            render->DrawQuad2D(pIconsFrameTable->animationFrame(game_ui_wizardEye, animTimer->time()), {606, 0});
    }
}

//----- (00491F87) --------------------------------------------------------
void GameUI_DrawHiredNPCs() {
    signed int uFrameID;            // [sp+24h] [bp-18h]@19

    if (!isHirelingsBlockedOnMap(engine->_currentLoadedMapId)) {
        FlatHirelings buf;
        buf.Prepare();

        for (int i = pParty->hirelingScrollPosition, count = 0; i < buf.Size() && count < 2; i++, count++) {
            std::string pContainer = npcPortraitTextureName(buf.Get(i)->portraitId);
            int npcX = pHiredNPCsIconsOffsetsX[count];
            int npcY = pHiredNPCsIconsOffsetsY[count];
            render->DrawQuad2D(assets->getIcon(pContainer), {npcX, npcY});

            // Dark sacrifice animation.
            if (!buf.IsFollower(i) && buf.GetSacrificeStatus(i)->inProgress) {
                render->DrawQuad2D(
                    pIconsFrameTable->animationFrame(pIconsFrameTable->animationId("spell96"), buf.GetSacrificeStatus(i)->elapsedTime),
                    {npcX, npcY});
            }
        }
    }
}

//----- (004178FE) --------------------------------------------------------
Color UI_GetHealthManaAndOtherQualitiesStringColor(int actual_value,
                                                          int base_value) {
    uint16_t R, G, B;

    if (actual_value == base_value) {
        return colorTable.White; // Default - white.
    } else if (actual_value < base_value) {
        if (100 * actual_value / base_value >=
            25)  // Yellow( current_pos > 1/4 )
            R = 255, G = 255, B = 100;
        else  // Red( current_pos < 1/4 )
            R = 255, G = 0, B = 0;
    } else {  // Green
        R = 0, G = 255, B = 0;
    }

    return Color(R, G, B);
}

//----- (00417939) --------------------------------------------------------
Color GetConditionDrawColor(Condition uConditionIdx) {
    switch (uConditionIdx) {
        case CONDITION_ZOMBIE:
        case CONDITION_GOOD:
            return ui_character_condition_normal_color;

        case CONDITION_CURSED:
        case CONDITION_WEAK:
        case CONDITION_FEAR:
        case CONDITION_DRUNK:
        case CONDITION_INSANE:
        case CONDITION_POISON_WEAK:
        case CONDITION_DISEASE_WEAK:
            return ui_character_condition_light_color;

        case CONDITION_SLEEP:
        case CONDITION_POISON_MEDIUM:
        case CONDITION_DISEASE_MEDIUM:
        case CONDITION_PARALYZED:
        case CONDITION_UNCONSCIOUS:
            return ui_character_condition_moderate_color;

        case CONDITION_POISON_SEVERE:
        case CONDITION_DISEASE_SEVERE:
        case CONDITION_DEAD:
        case CONDITION_PETRIFIED:
        case CONDITION_ERADICATED:
            return ui_character_condition_severe_color;
        default:
            assert(false);
            return Color();
    }
}

//----- (00495430) --------------------------------------------------------
std::string GetReputationString(int reputation) {
    if (reputation >= 25)
        return localization->str(LSTR_REPUTATION_HATED);
    else if (reputation >= 6)
        return localization->str(LSTR_REPUTATION_UNFRIENDLY);
    else if (reputation >= -5)
        return localization->str(LSTR_REPUTATION_NEUTRAL);
    else if (reputation >= -24)
        return localization->str(LSTR_REPUTATION_FRIENDLY);
    else
        return localization->str(LSTR_REPUTATION_LIKED);
}

void GameUI_handleHintMessage(UIMessageType type, int param) {
    switch (type) {
        case UIMSG_HintSelectRemoveQuickSpellBtn: {
            if (spellbookSelectedSpell != SPELL_NONE && spellbookSelectedSpell != pParty->activeCharacter().uQuickSpell) {
                engine->_statusBar->setPermanent(LSTR_SET_S_AS_THE_READY_SPELL, pSpellStats->pInfos[spellbookSelectedSpell].name);
            } else {
                if (pParty->activeCharacter().uQuickSpell != SPELL_NONE)
                    engine->_statusBar->setPermanent(LSTR_CLICK_HERE_TO_REMOVE_YOUR_QUICK_SPELL);
                else
                    engine->_statusBar->setPermanent(LSTR_SELECT_A_SPELL_THEN_CLICK_HERE_TO_SET_A);
            }
            break;
        }

        case UIMSG_Spellbook_ShowHightlightedSpellInfo: {
            if (!pParty->hasActiveCharacter())
                break;
            SpellId selectedSpell = static_cast<SpellId>(param);
            if (isHoldingMouseRightButton())
                dword_507B00_spell_info_to_draw_in_popup = selectedSpell;
            if (spellbookSelectedSpell == selectedSpell) {
                engine->_statusBar->setPermanent(LSTR_CAST_S, pSpellStats->pInfos[selectedSpell].name);
            } else {
                engine->_statusBar->setPermanent(LSTR_SELECT_S, pSpellStats->pInfos[selectedSpell].name);
            }
            break;
        }

        case UIMSG_ShowStatus_DateTime: {
            CivilTime time = pParty->GetPlayingTime().toCivilTime();
            std::string status = fmt::format(
                "{}:{:02}{} {} {} {} {}",
                time.hourAmPm,
                time.minute,
                localization->amPm(time.isPm),
                localization->dayName(time.dayOfWeek - 1),
                time.day,
                localization->monthName(time.month - 1),
                time.year);
            engine->_statusBar->setPermanent(status);
            break;
        }

        case UIMSG_ShowStatus_ManaHP: {
            Character* character = &pParty->pCharacters[param - 1];
            engine->_statusBar->setPermanent(fmt::format("{} / {} {}    {} / {} {}",
                character->GetHealth(), character->GetMaxHealth(), localization->str(LSTR_HIT_POINTS),
                character->GetMana(), character->GetMaxMana(), localization->str(LSTR_SPELL_POINTS)));
            break;
        }

        case UIMSG_ShowStatus_Player: {
            Character* character = &pParty->pCharacters[param];
            engine->_statusBar->setPermanent(fmt::format("{}: {}", NameAndTitle(character->name, character->classType),
                localization->characterConditionName(character->GetMajorConditionIdx())));
            break;
        }

        case UIMSG_ShowStatus_Food: {
            engine->_statusBar->setPermanent(LSTR_YOU_HAVE_LU_FOOD, pParty->GetFood());
            break;
        }

        case UIMSG_ShowStatus_Funds: {
            engine->_statusBar->setPermanent(LSTR_YOU_HAVE_D_TOTAL_GOLD_D_IN_THE_BANK, pParty->GetGold() + pParty->uNumGoldInBank, pParty->uNumGoldInBank);
            break;
        }

        case UIMSG_HintBeaconSlot: {
            if (pGUIWindow_CurrentMenu)
                (dynamic_cast<GUIWindow_LloydsBook*>(pGUIWindow_CurrentMenu.get()))->hintBeaconSlot(param);
            break;
        }

        case UIMSG_HintTownPortal: {
            if (pGUIWindow_CurrentMenu)
                (dynamic_cast<GUIWindow_TownPortalBook*>(pGUIWindow_CurrentMenu.get()))->hintTown(param);
            break;
        }

        case UIMSG_7A: {
            // unknown
            // button msg UIMSG_InventoryLeftClick
            break;
        }

        default: {
            MM_WARNING("GameUI_handleHintMessage - Unhandled message type: {}", static_cast<int>(type));
            break;
        }
    }
}
