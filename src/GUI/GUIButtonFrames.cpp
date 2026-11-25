#include "GUI/GUIButtonFrames.h"

#include <utility>

#include "Engine/AssetsManager.h"
#include "Engine/Graphics/Image.h"

const GUIButtonFrameSpec &mm8ButtonFrameSpec(MM8ButtonFamily family) {
    // All names are distinct EnglishD.lod entries. No default case, see the enum's comment in
    // GUIButtonFrames.h.
    switch (family) {
        case MM8ButtonFamily::MainMenuNew: {
            static constexpr GUIButtonFrameSpec spec{"main-menu", "new", "", "t_new_ht", "t_new_dn", false, true, true};
            return spec;
        }
        case MM8ButtonFamily::MainMenuLoad: {
            static constexpr GUIButtonFrameSpec spec{"main-menu", "load", "", "t_load_ht", "t_load_dn", false, true, true};
            return spec;
        }
        case MM8ButtonFamily::MainMenuCredits: {
            static constexpr GUIButtonFrameSpec spec{"main-menu", "credits", "", "t_cred_ht", "t_cred_dn", false, true, true};
            return spec;
        }
        case MM8ButtonFamily::MainMenuExit: {
            static constexpr GUIButtonFrameSpec spec{"main-menu", "exit", "", "t_quit_ht", "t_quit_dn", false, true, true};
            return spec;
        }
        case MM8ButtonFamily::GameMenuNew: {
            static constexpr GUIButtonFrameSpec spec{"game-menu", "new", "bt_newu", "bt_newh", "bt_newd", true, true, true};
            return spec;
        }
        case MM8ButtonFamily::GameMenuSave: {
            static constexpr GUIButtonFrameSpec spec{"game-menu", "save", "bu_saveu", "bu_saveh", "bu_saved", true, true, true};
            return spec;
        }
        case MM8ButtonFamily::GameMenuLoad: {
            // "loag" is a typo for "load" in the game's own asset names.
            static constexpr GUIButtonFrameSpec spec{"game-menu", "load", "bt_loagu", "bt_loagh", "bt_loagd", true, true, true};
            return spec;
        }
        case MM8ButtonFamily::GameMenuControls: {
            static constexpr GUIButtonFrameSpec spec{"game-menu", "controls", "bt_contu", "bt_conth", "bt_contd", true, true, true};
            return spec;
        }
        case MM8ButtonFamily::GameMenuQuit: {
            static constexpr GUIButtonFrameSpec spec{"game-menu", "quit", "bt_quitu", "bt_quith", "bt_quitd", true, true, true};
            return spec;
        }
        case MM8ButtonFamily::GameMenuResume: {
            static constexpr GUIButtonFrameSpec spec{"game-menu", "resume", "bu_rtrnu", "bu_rtrnh", "bu_rtrnd", true, true, true};
            return spec;
        }
        // Turn-rate/toggle families - EnglishD.lod has only
        // "_u"/"_d" entries (e.g. "bt_16xu"/"bt_16xd") for these 7 families, no "_h" of any name.
        case MM8ButtonFamily::GameOptionsTurnRate16x: {
            static constexpr GUIButtonFrameSpec spec{"game-options", "turn-rate-16x", "bt_16xu", "", "bt_16xd", true, false, true};
            return spec;
        }
        case MM8ButtonFamily::GameOptionsTurnRate32x: {
            static constexpr GUIButtonFrameSpec spec{"game-options", "turn-rate-32x", "bt_32xu", "", "bt_32xd", true, false, true};
            return spec;
        }
        case MM8ButtonFamily::GameOptionsTurnRateSmooth: {
            static constexpr GUIButtonFrameSpec spec{"game-options", "turn-rate-smooth", "bt_smoou", "", "bt_smood", true, false, true};
            return spec;
        }
        case MM8ButtonFamily::GameOptionsWalkSound: {
            static constexpr GUIButtonFrameSpec spec{"game-options", "walk-sound", "bt_wksdu", "", "bt_wksdd", true, false, true};
            return spec;
        }
        case MM8ButtonFamily::GameOptionsShowHits: {
            static constexpr GUIButtonFrameSpec spec{"game-options", "show-hits", "bt_hitsu", "", "bt_hitsd", true, false, true};
            return spec;
        }
        case MM8ButtonFamily::GameOptionsAlwaysRun: {
            static constexpr GUIButtonFrameSpec spec{"game-options", "always-run", "bt_runu", "", "bt_rund", true, false, true};
            return spec;
        }
        case MM8ButtonFamily::GameOptionsFlipOnExit: {
            static constexpr GUIButtonFrameSpec spec{"game-options", "flip-on-exit", "bt_flipu", "", "bt_flipd", true, false, true};
            return spec;
        }
        // These 3 ship the full "_u"/"_h"/"_d" triple - real hover, same as the ESC-menu families.
        case MM8ButtonFamily::GameOptionsConfigureKeyboard: {
            static constexpr GUIButtonFrameSpec spec{"game-options", "configure-keyboard", "bt_cfkbu", "bt_cfkbh", "bt_cfkbd", true, true, true};
            return spec;
        }
        case MM8ButtonFamily::GameOptionsVideoOptions: {
            static constexpr GUIButtonFrameSpec spec{"game-options", "video-options", "bt_vdopu", "bt_vdoph", "bt_vdopd", true, true, true};
            return spec;
        }
        case MM8ButtonFamily::GameOptionsReturn: {
            static constexpr GUIButtonFrameSpec spec{"game-options", "return", "bt_rtrnu", "bt_rtrnh", "bt_rtrnd", true, true, true};
            return spec;
        }
    }
    std::unreachable();
}

void GUIButtonFrameSet::release() {
    up.reset();
    hover.reset();
    down.reset();
}

GUIButtonFrameSet loadMM8ButtonFrameSet(MM8ButtonFamily family) {
    const GUIButtonFrameSpec &spec = mm8ButtonFrameSpec(family);
    GUIButtonFrameSet result;
    if (spec.hasUp)
        result.up = assets->getImage_ColorKeyFromEnglishDLOD(spec.upName);
    if (spec.hasHover)
        result.hover = assets->getImage_ColorKeyFromEnglishDLOD(spec.hoverName);
    if (spec.hasDown)
        result.down = assets->getImage_ColorKeyFromEnglishDLOD(spec.downName);
    return result;
}
