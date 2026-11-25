#pragma once

#include <array>
#include <memory>
#include <string_view>

class GraphicsImage;

// Every MM8 button family wired onto the shared up/hover/down mechanism.
// MM8's main-menu and ESC-menu buttons ship up, hover and down frames in EnglishD.lod. The
// original shows the hover frame.
//
// Add a screen to this mechanism by adding a value here AND a matching `case` in
// mm8ButtonFrameSpec() (GUIButtonFrames.cpp) - CMakeLists.txt's `-Werror=switch` turns a missing
// case into a build failure, not a silent gap.
enum class MM8ButtonFamily {
    MainMenuNew,
    MainMenuLoad,
    MainMenuCredits,
    MainMenuExit,
    GameMenuNew,
    GameMenuSave,
    GameMenuLoad,
    GameMenuControls,
    GameMenuQuit,
    GameMenuResume,
    // The in-game Controls (options) screen. Turn-rate/toggle families ship only "_u"/
    // "_d" (no hover), no `bt_16xh`/`bt_wksdh`/etc entry exists in
    // EnglishD.lod. CONFIGURE KEYBOARD/VIDEO OPTIONS/RETURN ship the full "_u"/"_h"/"_d" triple.
    GameOptionsTurnRate16x,
    GameOptionsTurnRate32x,
    GameOptionsTurnRateSmooth,
    GameOptionsWalkSound,
    GameOptionsShowHits,
    GameOptionsAlwaysRun,
    GameOptionsFlipOnExit,
    GameOptionsConfigureKeyboard,
    GameOptionsVideoOptions,
    GameOptionsReturn,
};

// Every family currently registered - for tests/tools that need to walk the whole table.
inline constexpr std::array<MM8ButtonFamily, 20> kMM8ButtonFrameFamilies = {
    MM8ButtonFamily::MainMenuNew, MM8ButtonFamily::MainMenuLoad, MM8ButtonFamily::MainMenuCredits, MM8ButtonFamily::MainMenuExit,
    MM8ButtonFamily::GameMenuNew, MM8ButtonFamily::GameMenuSave, MM8ButtonFamily::GameMenuLoad,
    MM8ButtonFamily::GameMenuControls, MM8ButtonFamily::GameMenuQuit, MM8ButtonFamily::GameMenuResume,
    MM8ButtonFamily::GameOptionsTurnRate16x, MM8ButtonFamily::GameOptionsTurnRate32x, MM8ButtonFamily::GameOptionsTurnRateSmooth,
    MM8ButtonFamily::GameOptionsWalkSound, MM8ButtonFamily::GameOptionsShowHits, MM8ButtonFamily::GameOptionsAlwaysRun,
    MM8ButtonFamily::GameOptionsFlipOnExit, MM8ButtonFamily::GameOptionsConfigureKeyboard,
    MM8ButtonFamily::GameOptionsVideoOptions, MM8ButtonFamily::GameOptionsReturn,
};

// One row per MM8ButtonFamily: the EnglishD.lod entry name for each of the "up"/"hover"/"down"
// frames, and whether this mechanism actually loads that frame. `hasUp` is false for the
// main-menu families even though e.g. "t_new_up" is a real EnglishD.lod entry. title.pcx
// already bakes the unpressed art into the background, so upName is left empty for those rows.
struct GUIButtonFrameSpec {
    std::string_view screen;   // diagnostic label, e.g. "main-menu"
    std::string_view button;   // diagnostic label, e.g. "new"
    std::string_view upName;
    std::string_view hoverName;
    std::string_view downName;
    bool hasUp = true;
    bool hasHover = true;
    bool hasDown = true;
};

// The registry lookup - see the enum's own comment above for the "must register" enforcement story.
const GUIButtonFrameSpec &mm8ButtonFrameSpec(MM8ButtonFamily family);

// The frame triple plus the draw-time state selection - the one place "which frame does an MM8
// button draw right now" is decided. A null slot means the family doesn't ship (or
// this mechanism doesn't load) that frame - see GUIButtonFrameSpec's hasUp/hasHover/hasDown.
struct GUIButtonFrameSet {
    std::shared_ptr<GraphicsImage> up;
    std::shared_ptr<GraphicsImage> hover;
    std::shared_ptr<GraphicsImage> down;

    GraphicsImage *select(bool isHovered, bool isPressed) const {
        if (isPressed && down)
            return down.get();
        if (isHovered && hover)
            return hover.get();
        return up.get();
    }

    // Any one non-null frame - for measuring a button's size at CreateButton() time, where every
    // family's 3 frames share one size regardless of which slots happen to be loaded.
    GraphicsImage *any() const {
        if (up)
            return up.get();
        if (hover)
            return hover.get();
        return down.get();
    }

    void release();
};

// Loads a family's up/hover/down triple from EnglishD.lod (via the global `assets`, same
// convention as every other MM8 asset loader in this codebase), honoring hasUp/hasHover/hasDown.
GUIButtonFrameSet loadMM8ButtonFrameSet(MM8ButtonFamily family);
