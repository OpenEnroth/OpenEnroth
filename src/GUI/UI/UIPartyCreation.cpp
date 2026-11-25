#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/GameType.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Random/Random.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "Engine/Spells/SpellEnumFunctions.h"
#include "Engine/Timer.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIPartyCreation.h"

#include "Io/Mouse.h"
#include "Io/KeyboardInputHandler.h"

#include "Media/Audio/AudioPlayer.h"

#include "Library/Logger/Logger.h"

#include "Utility/MapAccess.h"

// `paperdoll_armor_indexByType()` (UICharacter.cpp) maps a real MM8 common armour item's id to a dense
// 0..N-1 index. It validates that an equipped item has doll art at all (artifact/relic-tier items don't)
// before resolving its texture.
const std::unordered_map<ItemId, int> &paperdoll_armor_indexByType();


std::shared_ptr<GraphicsImage> ui_partycreation_top;
std::shared_ptr<GraphicsImage> ui_partycreation_sky_scroller;

std::shared_ptr<GraphicsImage> ui_partycreation_left;
std::shared_ptr<GraphicsImage> ui_partycreation_right;
std::shared_ptr<GraphicsImage> ui_partycreation_minus;
std::shared_ptr<GraphicsImage> ui_partycreation_plus;
std::shared_ptr<GraphicsImage> ui_partycreation_buttmake2;
std::shared_ptr<GraphicsImage> ui_partycreation_buttmake;

// MM8's real per-attribute-row spinner icons (`cminup`/`cplusup` in icons.lod) and the "Default" voice
// button, distinct assets from the MM7-shared buttminu/buttplus corner pair above.
// See `ConstructMm8Controls`/`UpdateMm8` for how they're used.
std::shared_ptr<GraphicsImage> ui_partycreation_attr_minus;
std::shared_ptr<GraphicsImage> ui_partycreation_attr_plus;
// `c_dft_up` (EnglishD.lod), not `bt_dfltu`. See `kMm8DefaultBtn`'s comment for why.
std::shared_ptr<GraphicsImage> ui_partycreation_default;
// The gold oval ring frame drawn over the Portrait thumbnail (`selring`, icons.lod). The real screen
// draws it over the plain portrait bitmap. See `kMm8PortraitRingBox`'s comment.
std::shared_ptr<GraphicsImage> ui_partycreation_portrait_ring;

// MM8's own Clear/Cancel/OK button art (`c_clr_*`/`c_cncl_*`/`c_ok_*` in EnglishD.lod), distinct from the
// MM7-only `ui_partycreation_buttmake`/`buttmake2` below, which aren't shipped in an MM8 install at all.
std::shared_ptr<GraphicsImage> ui_partycreation_clear_mm8;
std::shared_ptr<GraphicsImage> ui_partycreation_cancel_mm8;
std::shared_ptr<GraphicsImage> ui_partycreation_ok_mm8;

std::shared_ptr<GraphicsImage> ui_partycreation_character_frame;

std::array<std::shared_ptr<GraphicsImage>, 9> ui_partycreation_class_icons;
std::array<std::shared_ptr<GraphicsImage>, 22> ui_partycreation_portraits;

// MM8-only doll-preview panel parts - per-portrait paperdoll sprites (icons.lod ships
// `pcNNbod`/`pcNNlhd`/`pcNNrhd` for each of the 22 portraits, the same naming convention as MM7's
// existing per-face "wetsuit" paperdoll in UICharacter.cpp) plus the panel's static backdrop.
// See `DrawMm8Doll` for how they're composited.
std::shared_ptr<GraphicsImage> ui_partycreation_doll_backdrop;
std::array<std::shared_ptr<GraphicsImage>, 22> ui_partycreation_doll_body;
std::array<std::shared_ptr<GraphicsImage>, 22> ui_partycreation_doll_lhand;
std::array<std::shared_ptr<GraphicsImage>, 22> ui_partycreation_doll_rhand;

std::array<std::shared_ptr<GraphicsImage>, 19> ui_partycreation_arrow_r;
std::array<std::shared_ptr<GraphicsImage>, 19> ui_partycreation_arrow_l;


std::array<GUIButton*, 4> pCreationUI_BtnPressRight2;
std::array<GUIButton*, 4> pCreationUI_BtnPressLeft2;
std::array<GUIButton*, 4> pCreationUI_BtnPressLeft;
std::array<GUIButton*, 4> pCreationUI_BtnPressRight;

GUIButton* pPlayerCreationUI_BtnReset;
GUIButton* pPlayerCreationUI_BtnOK;
GUIButton* pPlayerCreationUI_BtnPlus;
GUIButton* pPlayerCreationUI_BtnMinus;

// MM8's per-attribute-row spinners + the Voice "Default" button - see `ConstructMm8Controls`.
std::array<GUIButton*, 7> pCreationUI_BtnAttrPlus;
std::array<GUIButton*, 7> pCreationUI_BtnAttrMinus;
GUIButton* pCreationUI_BtnVoiceDefault;
// MM8's 3rd bottom button - see the `kMm8ClearBtn` comment. Clear/OK reuse the shared
// MM7 pPlayerCreationUI_BtnReset/BtnOK below (same message ids, just MM8-specific art/position).
GUIButton* pCreationUI_BtnCancelMm8;



static Duration errorMessageExpireTime; // expiration time (anim timer) of error message

static const int ARROW_SPIN_PERIOD_MS = 475;

// Layout for MM8's own single-champion "Create Character" screen, as opposed to MM7's 4-portrait
// `UIPartyCreation`. Coordinates were measured directly off the real `makeme.pcx` (640x480,
// `EnglishD.lod` - the panel frames/headers are baked into this art, unlike MM7's screen which
// draws everything from code) and cross-checked against MANUAL.PDF's page 6 "Character Creation Screen"
// figure (both its panel-label list AND the annotated screenshot image embedded on that page), which
// clarifies several things the art alone doesn't:
//  - the "Portrait"/"Voice" panel is a single box whose baked text is a placeholder caption, not a
//    header to draw around - the portrait image is meant to be drawn right over it, same convention
//    as this codebase's other missing-asset placeholders. The box also holds a "Default" button
//    (reverts to the default voice for the current portrait, per the manual's Basic Operations text)
//    below the Portrait/Voice cycle-arrow rows.
//  - the tall panel to the right of Skills (solid `TealMask`-keyed in the art) is the "Doll preview"
//    panel - a 3D-*looking* render of the character, but it is actually a 2D layered sprite composite:
//    `icons.lod`'s `backdoll` background plus per-portrait `pcNNbod`/`pcNNlhd`/`pcNNrhd` sprites, the
//    exact same asset-naming convention (and drawing idiom) as MM7's existing per-face "wetsuit"
//    paperdoll in `UICharacter.cpp`. See `DrawMm8Doll` below.
//    MM8 has no visible race/class list anywhere on this screen. Per the manual the portrait determines
//    gender and class - i.e. cycling the Portrait arrows *is* the race/class picker.
//    There is no real portrait-to-race table yet (`icons.lod`'s MM8 portraits are still the generic
//    MM7-shared 25-portrait pool), so `CreateParty_EventLoop`'s FacePrev/FaceNext handlers
//    map `uCurrentFace % allMm8StartableRaces().count()` onto a startable race as a documented
//    placeholder. Real per-portrait race art would replace this with a proper table.
// x=79: the baked Name/Class input-box art (see kMm8PortraitThumbBox's comment for the same "box is baked,
// only content is live-drawn" idiom) spans roughly x=68-192, and the real screen draws the value text
// left-aligned with a small ~5px padding from the box's own left border.
constexpr Recti kMm8NameBox = {79, 75, 48, 21};
constexpr Recti kMm8ClassBox = {79, 120, 48, 21};
// The baked `makeme.pcx` background only draws a box (with placeholder "Portrait"/"Voice" captions,
// see kMm8FacePrevBtn et al below) from x=70 rightward. The oval portrait thumbnail is a live-drawn
// element in the plain background to its left, not an overlay on top of that box.
// Sized to the ring's see-through opening (52x71, inset (4,4) from the ring sprite's top-left), not its
// outer canvas: the ring's opaque gold band takes several px on every side, and DrawImageCoverFit fills
// the box edge-to-edge, so a larger box would overflow the ring.
constexpr Recti kMm8PortraitThumbBox = {8, 166, 52, 71};
// The gold oval ring frame (`selring`, icons.lod, natively 61x80) that the real screen overlays on top of
// the Portrait thumbnail. `selring`'s teal center is colorkeyed away so the portrait drawn underneath at
// `kMm8PortraitThumbBox` shows through.
constexpr Recti kMm8PortraitRingBox = {4, 162, 61, 80};
// The Portrait/Voice arrows are drawn with the 3-arg `DrawQuad2D(image, srcRect, dstRect)` overload, cropped
// to the masked triangle (`kMm8ArrowIconSrcRect`) and stretched to these boxes. The 2-arg overload ignores
// the Recti's w/h and would draw the ~8x7 triangle at native size. The real screen's arrows are wider than
// the asset's own aspect, and there is no vertical room to grow further without the Voice row colliding
// with `kMm8DefaultBtn` right below it.
constexpr Recti kMm8FacePrevBtn = {67, 169, 22, 14};
constexpr Recti kMm8FaceNextBtn = {160, 169, 22, 14};
constexpr Recti kMm8VoicePrevBtn = {67, 200, 22, 14};
constexpr Recti kMm8VoiceNextBtn = {160, 200, 22, 14};
// The masked triangle's own bounding box within `ar_lt_up`/`ar_rt_up`'s shared 17x17 canvas (`ar_rt_up` is a
// mirrored copy of the same shape). Excludes the stray 1px opaque bevel-corner column at x=0 (L) / x=16 (R)
// that isn't part of the triangle itself.
constexpr Recti kMm8ArrowIconSrcRect = {4, 5, 8, 7};
// `c_dft_up` (EnglishD.lod, natively 75x27), not `bt_dfltu`: `bt_dfltu` is a gold-on-black "DEFAULT" button
// with a different border and colors, while `c_dft_up` (plain white "Default" on a dark bevelled box, same
// family as `c_clr_up`/`c_cncl_up`/`c_ok_up` below) matches the real screen. Drawn at native size, no scaling.
constexpr Recti kMm8DefaultBtn = {93, 214, 75, 27};
constexpr Pointi kMm8BonusPoolValuePos = {160, 264};
constexpr Recti kMm8AttributesBox = {100, 290, 94, 144}; // Might..Luck, one row each.
constexpr int kMm8AttrSpinnerSize = 16; // cminup/cplusup/cmindn/cplusdn are all 16x17 in icons.lod.
// Neither spinner sits flush against its box edge on the real screen: Minus has a ~5px gap from the box's
// left edge, Plus a ~11px gap from its right edge, identical across every row.
constexpr int kMm8AttrMinusX = kMm8AttributesBox.x + 5;
constexpr int kMm8AttrPlusX = kMm8AttributesBox.x + kMm8AttributesBox.w - kMm8AttrSpinnerSize - 11;
// The number is CENTERED in the column between the two spinner icons on the real screen, not right-justified
// against the Plus icon. See `UpdateMm8`'s use of `AlignText_Center` on this column.
constexpr int kMm8AttributesNumberColX = kMm8AttrMinusX + kMm8AttrSpinnerSize;
constexpr int kMm8AttributesNumberColW = kMm8AttrPlusX - kMm8AttributesNumberColX;
constexpr int kMm8AttributesLabelX = 15; // Attribute labels sit to the LEFT of the box (real MM8 layout has icon-number-icon inside the box, label outside it).
// The real screen puts 18px between attribute row baselines. The number column's 7 rows land at an exact 18px cadence.
constexpr int kMm8AttributesRowGap = 18;
// The box's own top border (baked background art) lines up with `kMm8AttributesBox.y`, but row 0's content
// sits ~8px further down than that on the real screen.
constexpr int kMm8AttributesFirstRowY = kMm8AttributesBox.y + 8;
constexpr Recti kMm8SkillsBox = {225, 91, 207, 95}; // Chosen skills, 2x2.
constexpr Recti kMm8AvailableSkillsBox = {225, 217, 207, 199}; // Remaining skill choices, 2x5.
// Click hitbox columns only (see `ConstructMm8Controls`). Skill text is positioned by kMm8SkillColW.
constexpr std::array<int, 2> kMm8SkillColX = {233, 330};
// The real screen leaves ~34-37px between skill row baselines, more than a font-height step, so this is a
// dedicated gap distinct from the attribute row gap.
constexpr int kMm8SkillRowGap = 35;
// Skill names are CENTERED within each half of the box width, not left-aligned at kMm8SkillColX. Both boxes
// share the same x/width, so one column width covers both (see `UpdateMm8`'s use of `AlignText_Center` on this column).
constexpr int kMm8SkillColW = kMm8SkillsBox.w / 2;
// Row 0's content sits this far below the box's own top border.
constexpr int kMm8SkillFirstRowY = 20;
constexpr Recti kMm8DollPreviewBox = {454, 58, 171, 360}; // The real "Doll preview" panel - see comment above.
// The body sprite is drawn centered horizontally (plus a small fixed bias) and bottom-aligned to a "floor"
// point in the panel. That generalizes across portraits, since it is just centering and standing on the floor.
// No decompiled MM8 binary exists to pull the original compositing offsets from (unlike MM7's `pPaperdoll_BodyX/Y`),
// so `kMm8DollFloorBiasX`/`kMm8DollFloorMarginY` were measured on the `pc02bod` sprite (`Party::Reset()`'s MM8 default
// portrait) as the original game draws it.
constexpr int kMm8DollFloorBiasX = 16; // The body's horizontal center vs. the panel's own center.
constexpr int kMm8DollFloorMarginY = 20; // Gap between the body sprite's bottom edge and the panel's bottom edge.
//
// MM8's 22 portraits use 4 distinct rigged poses, reused across recolors, told apart by body-sprite size:
// "A" 83x297 with hands 47x59/17x66 (pc02/04/06/08/10/12/14/16/18/20), "B" ~131x320 with hands 42x85/27x52
// (pc01/03/05/07/09/11/13/15/17/19), "C" 98x310 with hands 49x55/31x76 (pc21, Minotaur), and "D" 97x317 with
// hands 44x38/31x76 (pc22, Minotaur). All 4 share one stance: weight on the forward leg, the other leg bent up
// behind. One arm's forearm and hand are baked into the body and cut off at the wrist (`pcNNlhd` continues it),
// and the other arm is a bare shoulder with no baked geometry (`pcNNrhd` fills it).
// The body sprite's bounding box is set by whichever limb sticks out furthest, not by the joints, so one
// body-relative fraction does not scale the attach points across classes. Each hand's attach point is therefore
// measured against its own body's silhouette at draw time, with no per-class table.
// ~0.286 of body height - coarse "which vertical band is the shoulder" guess, common to all 4 classes
// (see FindHandAttachApex for why the exact row does not need to be precise).
constexpr float kMm8DollRightHandOffsetFracY = 85.0f / 297;
constexpr int kMm8DollRightHandOverlap = 1; // How far rhand's measured attach point sinks into the body's measured edge at that same row.
constexpr int kMm8DollLeftHandOverlap = 3; // Same idea for lhand's stub-to-hand seam.
// Unlike the X axis, the Y axis needs overlap too: `lhandPos.y` landing the hand's plug row exactly on the body's
// measured stub row leaves no margin for the two independent silhouette scans (`FindBodyStubCutoff`,
// `FindHandTopRight`) to disagree by a px, which opens a gap. Sinking the hand a couple px up into the stub covers it.
constexpr int kMm8DollLeftHandOverlapY = 2;
constexpr float kMm8DollStubSearchYFracLo = 0.15f; // Search band (fraction of body height) for the baked arm stub's cutoff - wide enough to cover the shoulder-to-elbow region across all 4 classes.
constexpr float kMm8DollStubSearchYFracHi = 0.55f;
constexpr int kMm8DollStubDropPx = 12; // A row-to-row rightward-edge retreat bigger than this means the stub has ended (its cut cross-section), not just the natural taper of the arm.

// Finds the body sprite's own leftmost non-transparent column at a SPECIFIC row (yTop==yBottom-1 in
// every call site below) - i.e. the real torso/shoulder silhouette at that exact height, not the
// sprite's overall (pose-dependent) bounding box edge. Takes a [yTop,yBottom) band rather than a single row,
// for robustness against a transparent gap row, and returns the first hit, not a minimum. Falls back to the
// sprite's own width (no overlap) if the band is empty or out of range.
static int FindBodyLeftEdge(Ptr<GraphicsImage> body, int yTop, int yBottom) {
    RgbaImage &pixels = body->rgba();
    yTop = std::max(yTop, 0);
    yBottom = std::min(yBottom, body->height());
    for (int y = yTop; y < yBottom; y++) {
        std::span<Color> row = pixels[y];
        for (int x = 0; x < body->width(); x++)
            if (row[x].a != 0)
                return x;
    }
    return body->width();
}

// rhand (`pcNNrhd`) attaches to a fully bare shoulder with no baked body geometry to align against. Its contact
// edge is the hand sprite's widest point, which sits at row 0 for the near-vertical classes A/B but 6-18px further
// down the canvas for the diagonal Minotaur classes C/D. This scans the hand's top half for that apex row and
// column, so the caller can sample the body at the matching row.
static Pointi FindHandAttachApex(Ptr<GraphicsImage> hand) {
    RgbaImage &pixels = hand->rgba();
    int bestX = -1, bestY = 0;
    int searchRows = hand->height() / 2;
    for (int y = 0; y < searchRows; y++) {
        std::span<Color> row = pixels[y];
        for (int x = hand->width() - 1; x >= 0; x--) {
            if (row[x].a != 0) {
                if (x > bestX) {
                    bestX = x;
                    bestY = y;
                }
                break;
            }
        }
    }
    return Pointi(std::max(bestX, 0), bestY);
}

// lhand (`pcNNlhd`) continues a forearm that IS baked into the body sprite, cut off cleanly at the wrist. Unlike
// rhand's bare shoulder, this is a landmark on the body itself: scanning the body's right-edge profile downward,
// the arm is the widest contributor in the shoulder-to-elbow band, and its cutoff is a sharp retreat (not the
// arm's natural taper) where the hand piece takes over. Returns the apex just before that drop.
static Pointi FindBodyStubCutoff(Ptr<GraphicsImage> body, int yTop, int yBottom) {
    RgbaImage &pixels = body->rgba();
    yTop = std::max(yTop, 0);
    yBottom = std::min(yBottom, body->height());
    int bestX = -1, bestY = yTop;
    for (int y = yTop; y < yBottom; y++) {
        std::span<Color> row = pixels[y];
        int x = -1;
        for (int c = body->width() - 1; c >= 0; c--) {
            if (row[c].a != 0) {
                x = c;
                break;
            }
        }
        if (x < 0)
            continue;
        if (x > bestX) {
            bestX = x;
            bestY = y;
        } else if (bestX - x > kMm8DollStubDropPx) {
            break;
        }
    }
    return Pointi(std::max(bestX, 0), bestY);
}

// The lhand sprite's own "plug" that fits into the body's stub "socket" found above - its topmost
// rightward extent, i.e. the top of the wrist cuff that continues the body's cut arm.
static Pointi FindHandTopRight(Ptr<GraphicsImage> hand, int rows) {
    RgbaImage &pixels = hand->rgba();
    for (int y = 0; y < std::min(rows, hand->height()); y++) {
        std::span<Color> row = pixels[y];
        for (int x = hand->width() - 1; x >= 0; x--)
            if (row[x].a != 0)
                return Pointi(x, y);
    }
    return Pointi(0, 0);
}

// rhand's own "grip" landmark: the horizontal center of the palm/finger cluster near the sprite's low end (the
// opposite end from FindHandAttachApex's shoulder-attach point). Not the bottom-most non-transparent row: the
// open-hand pose's fingers splay to different lengths, so the lowest pixel is one fingertip's outlier. Samples
// `kMm8DollGripRowFrac`'s row instead, in the palm band where row width stays wide until the last ~15% of the
// sprite, and centers on that row's span.
constexpr float kMm8DollGripRowFrac = 0.80f;
Pointi FindHandGripPoint(Ptr<GraphicsImage> hand) {
    RgbaImage &pixels = hand->rgba();
    int startRow = std::clamp(static_cast<int>(kMm8DollGripRowFrac * hand->height()), 0, hand->height() - 1);
    for (int y = startRow; y >= 0; y--) {
        std::span<Color> row = pixels[y];
        int left = -1, right = -1;
        for (int x = 0; x < hand->width(); x++) {
            if (row[x].a != 0) {
                if (left == -1) left = x;
                right = x;
            }
        }
        if (left != -1)
            return Pointi((left + right) / 2, y);
    }
    return Pointi(hand->width() / 2, hand->height() - 1);
}

// Real MM8 has a 3rd "Cancel" button between Clear and OK. `c_clr_*`/`c_cncl_*`/`c_ok_*` (EnglishD.lod) are the
// screen's button art, each natively 83x30. The MM7 BUTTMAKE/BUTTMAKE2 assets do not exist in an MM8 install's LODs.
// The real screen leaves a non-uniform ~3px then ~4px gap between the 3 buttons, not a flush layout.
constexpr Recti kMm8ClearBtn = {373, 436, 83, 30};
constexpr Recti kMm8CancelBtn = {459, 436, 83, 30};
constexpr Recti kMm8OkBtn = {546, 436, 83, 30};

bool PartyCreationUI_LoopInternal();

// Several of this screen's icons.lod assets (aframe1, arrowl*/arrowr*, buttminu/buttplus,
// presleft/presrigh, MAKETOP/MAKESKY) simply aren't shipped in MM8 installs. getImage_Alpha/Solid/
// ColorKey all fall back to a visible "pending" placeholder texture on a missing entry - the right
// call for a genuinely-loading asset, but wrong for one that will just never exist. These wrappers
// check existence first so a missing MM8 asset is skipped (DrawQuad2D already no-ops on nullptr)
// instead of covering the screen in placeholder icons; behavior is unchanged for MM7, where all of
// these assets are real.
static std::shared_ptr<GraphicsImage> getImage_AlphaIfExists(std::string_view name) {
    return assets->hasIconsLodEntry(name) ? assets->getImage_Alpha(name) : nullptr;
}
static std::shared_ptr<GraphicsImage> getImage_SolidIfExists(std::string_view name) {
    return assets->hasIconsLodEntry(name) ? assets->getImage_Solid(name) : nullptr;
}
static std::shared_ptr<GraphicsImage> getImage_ColorKeyIfExists(std::string_view name) {
    return assets->hasIconsLodEntry(name) ? assets->getImage_ColorKey(name) : nullptr;
}
// Same as above, but for MM8's EnglishD.lod (see `getImage_ColorKeyFromEnglishDLOD`) - used for the
// "Default" voice button, which unlike the other assets above only ever ships in EnglishD.lod.
static std::shared_ptr<GraphicsImage> getImage_ColorKeyFromEnglishDLODIfExists(std::string_view name) {
    return assets->hasEnglishDLodEntry(name) ? assets->getImage_ColorKeyFromEnglishDLOD(name) : nullptr;
}

// `ar_lt_up`/`ar_rt_up` (MM8's own Portrait/Voice cycle arrows) are baked as fully-opaque "stone button" sprites.
// That is correct for the attribute [-]/[+] spinners (`cminup`/`cplusup`), but the real Portrait/Voice arrows are a
// plain dark triangle drawn directly over the panel background with no border or backing plate. No borderless
// triangle asset ships in any MM8 install LOD (icons.lod/icons.UI.lod both checked), so this masks the sprite in
// place down to its near-black triangle pixels (luma sums: triangle <=181, backing plate >=236, no AA pixels in
// between). Safe to mutate in place: `ar_lt_up`/`ar_rt_up` are loaded nowhere else in the codebase.
// A hard luma cutoff alone throws away the sprite's own mid-grey bevel shading, since that shares a luma range with
// the backing-plate bleed at the crop edges, and leaves a flat near-black cutout. The triangle's pixels are near-exact
// (0,0,0), so a smooth alpha ramp keyed off luma still leaves true-black texels at alpha 255. Capping the ramp's peak
// below full opacity (`kMm8ArrowTriangleMaxAlpha`) blends the darkest pixels toward the panel background, which gives
// the original game's subdued shaded-grey look (its darkest triangle pixel is (10,14,10), against a ~(50,60,57) background).
constexpr int kMm8ArrowTriangleLumaThreshold = 200;
constexpr int kMm8ArrowTriangleMaxAlpha = 170;
static void MaskToPlainTriangle(Ptr<GraphicsImage> image) {
    if (!image) return;
    RgbaImage &pixels = image->rgba();
    for (int y = 0; y < pixels.height(); y++) {
        std::span<Color> row = pixels[y];
        for (int x = 0; x < pixels.width(); x++) {
            int luma = row[x].r + row[x].g + row[x].b;
            row[x].a = luma >= kMm8ArrowTriangleLumaThreshold
                ? 0
                : static_cast<uint8_t>(kMm8ArrowTriangleMaxAlpha * (kMm8ArrowTriangleLumaThreshold - luma) / kMm8ArrowTriangleLumaThreshold);
        }
    }
}

// Cover-fit (fill `dst` completely, cropping whichever axis has overflow) rather than
// contain-fit (letterbox) - is used to fill the Doll-preview panel and the Portrait thumbnail box
// without leaving visible colorkey/background around a differently-proportioned source portrait.
static void DrawImageCoverFit(Ptr<GraphicsImage> image, const Recti &dst) {
    if (!image) return;
    float scale = std::max(static_cast<float>(dst.w) / image->width(), static_cast<float>(dst.h) / image->height());
    Sizei srcSize(static_cast<int>(dst.w / scale), static_cast<int>(dst.h / scale));
    Pointi srcOrigin((image->width() - srcSize.w) / 2, (image->height() - srcSize.h) / 2);
    render->DrawQuad2D(image, Recti(srcOrigin, srcSize), dst);
}

// Centers `text` in `[baseX, baseX+colW)` when it fits on one line. Otherwise splits at the last space (every
// MM8 skill name that overflows this column is exactly two words - "Body Building", "Identify Item", "Identify
// Monster") and centers each half on its own line, as the real screen wraps them.
// The real screen wraps text that fills the column exactly, not only text that overflows it: "Body Building"
// measures exactly kMm8SkillColW, so the test is a strict `<`.
static void DrawSkillName(GUIFont *font, Color color, std::string_view text, int baseX, int colW, int y, const Recti &frame) {
    if (font->GetLineWidth(text) < colW || text.find(' ') == std::string_view::npos) {
        GUIWindow_PartyCreation::DrawText(font, {baseX + font->AlignText_Center(colW, text), y}, color, text, frame);
        return;
    }
    std::string_view line1 = text.substr(0, text.rfind(' '));
    std::string_view line2 = text.substr(text.rfind(' ') + 1);
    GUIWindow_PartyCreation::DrawText(font, {baseX + font->AlignText_Center(colW, line1), y}, color, line1, frame);
    GUIWindow_PartyCreation::DrawText(font, {baseX + font->AlignText_Center(colW, line2), y + font->GetHeight()}, color, line2, frame);
}

// `backdoll` (175x378) is a few px larger than `kMm8DollPreviewBox` (171x360) in both axes. It is drawn unscaled,
// 4px above the box's top edge, as in the original game, which does not stretch it to fit.
// The 4px vertical and 0px horizontal excess overflows the box edges, and does not reach the Ok/Cancel row
// above y=436 or the adjacent Skills box.
constexpr int kMm8DollBackdropOffsetY = -4;
// `backdoll`'s canvas is transparent outside its rounded top corners (it's a round glow disc, not a rectangle).
// `makeme.pcx`'s pixels behind those corners are a teal colorkey (0,255,255), a deliberately reserved transparent
// hole, so nothing opaque sits under them and they show whatever was drawn before (plain black). The real screen
// shows a flat dark backdrop in those corners, (41,40,24), so this fill goes behind the backdrop composite and
// leaves backdoll's halo shape alone.
constexpr Color kMm8DollPanelBackfillColor = {41, 40, 24};

// Body complection of `CharacterUI_DrawPaperdoll` (UICharacter.cpp): MM7's race/sex-derived 4-way split. The same
// `v1`-`v4` equipment asset axis applies to MM8's doll sprites (`item084v2` matched complection 1 for a non-dwarf
// female), even though `DrawMm8Doll`'s own body/hand art is keyed by portrait (`uCurrentFace`), not this.
//
// Reads `character.uSex`, not `GetSexByVoice()`: for MM8's default new-game character (`uCurrentFace=1`, female
// `pc02bod` body art, 83px wide) GetSexByVoice() reads MALE, since `uVoiceID=1` falls in the MM7-legacy voice
// table's male bucket. `Party::Reset()` force-sets `uSex = SEX_FEMALE` for that character, and that corrected
// field is what complection must read. With the male complection, `GetItemTextureFilename` resolves the torso-only
// `item084v1` (119x144) instead of the full-length `item084v2` (100x252), which paints its own legs and boots,
// so the female body's default legs showed below the hem. No second `pcNNbod` series exists in the data to substitute.
static int Mm8DollBodyComplection(const Character &character) {
    bool dwarf = character.GetRace() == RACE_DWARF;
    bool male = character.uSex == SEX_MALE;
    if (dwarf)
        return male ? 2 : 3;
    return male ? 0 : 1;
}

// The armor layer's vertical/horizontal anchor. `kMm8DollArmorTopOffsetFracY` equals `kMm8DollStubSearchYFracLo`
// (where the shoulder region begins), but is its own constant because the two call sites serve different purposes.
// Both offsets were measured only for item084v2 on the class-A body (pc02, complection 1).
constexpr float kMm8DollArmorTopOffsetFracY = kMm8DollStubSearchYFracLo;
constexpr int kMm8DollArmorCenterOffsetX = -2;
constexpr int kMm8DollArmorTopOffsetCorrectionY = 1;

// Per-layer equipment compositing decisions of `DrawMm8Doll`, as a pure query. Rings do not render on the
// original's doll, so ITEM_SLOT_RING is not handled here. Helm/boots/cloak/belt/gauntlets are not handled yet.
std::vector<Mm8DollEquipmentLayer> ComputeMm8DollEquipmentLayers(const Character &character, Ptr<GraphicsImage> body, Pointi bodyPos, Ptr<GraphicsImage> rhand, Pointi rhandPos) {
    std::vector<Mm8DollEquipmentLayer> layers;

    // The armour anchors by its top edge at a fraction of the body's height, not by its top-left at the body's
    // top-left. A top-left-to-top-left anchor draws the garment from the head down rather than from the collar,
    // and leaves any width difference on one side. A floor-aligned anchor detaches the short jerkin (item084v1,
    // 119x144) from the shoulders, so the garment's own height sets how far down it reaches. In the
    // original the full outfit item084v2 (100x252) sits at (bodyPos - 10, bodyPos + 46) on the class-A body.
    if (InventoryConstEntry armor = character.inventory.entry(ITEM_SLOT_ARMOUR)) {
        // `typeIndex` only gates "does this item have real doll art at all" (artifact/relic-tier items don't
        // resolve), it does NOT feed positioning. MM7's per-type coordinate table (`paperdoll_Armor_Coord[complection]
        // [typeIndex]`, UICharacter.cpp) is authored against MM7's `paperdoll_dbods` canvas, not MM8's `pcNNbod`,
        // so the size-derived anchor below needs no per-type table.
        int typeIndex = valueOr(paperdoll_armor_indexByType(), armor->itemId, -1);
        if (typeIndex != -1) {
            std::string textureName = GetItemTextureFilename(armor->itemId, Mm8DollBodyComplection(character) + 1, 0);
            if (std::shared_ptr<GraphicsImage> armorTexture = getImage_AlphaIfExists(textureName)) {
                Pointi armorPos(
                    bodyPos.x + (body->width() - armorTexture->width()) / 2 + kMm8DollArmorCenterOffsetX,
                    bodyPos.y + std::lround(kMm8DollArmorTopOffsetFracY * body->height()) + kMm8DollArmorTopOffsetCorrectionY);
                layers.push_back({Mm8DollLayerKind::Armor, textureName, Recti(armorPos, Sizei(armorTexture->width(), armorTexture->height())), armor.index()});
            }
        }
    }

    // The equipped main-hand weapon, anchored to the rhand sprite's own grip landmark. Unlike armor, MM8 has no
    // doll-scale "item{:03}vN" sprite for weapons (a starting sword's texture is "item016", no "v" suffix). MM7 draws
    // weapons with the item's regular inventory icon (`Item::GetIconName()`), positioned by its authored
    // `paperdollAnchorOffset` (items.txt columns 14/15, gameType-agnostic). MM7 anchors that offset against a
    // per-complection `paperdoll_Weapon[][]` table with no MM8-measured equivalent, so the grip landmark
    // (`FindHandGripPoint`) replaces it.
    if (rhand) {
        if (InventoryConstEntry weapon = character.inventory.entry(ITEM_SLOT_MAIN_HAND)) {
            if (std::shared_ptr<GraphicsImage> weaponTexture = getImage_AlphaIfExists(weapon->GetIconName())) {
                Pointi grip = rhandPos + FindHandGripPoint(rhand);
                const Pointi &anchor = pItemTable->items[weapon->itemId].paperdollAnchorOffset;
                Pointi weaponPos(grip.x - anchor.x, grip.y - anchor.y);
                layers.push_back({Mm8DollLayerKind::Weapon, weapon->GetIconName(), Recti(weaponPos, Sizei(weaponTexture->width(), weaponTexture->height())), weapon.index()});
            }
        }
    }

    return layers;
}

// The body and rhand positioning math `DrawMm8Doll` needs, split out as a pure query like
// `ComputeMm8DollEquipmentLayers` above.
Mm8DollBodyLayout ComputeMm8DollBodyLayout(const Character &character, const Recti &box) {
    Mm8DollBodyLayout layout;
    layout.body = ui_partycreation_doll_body[character.uCurrentFace];
    if (!layout.body) return layout; // No doll art for this portrait.
    layout.rhand = ui_partycreation_doll_rhand[character.uCurrentFace];

    Pointi floorAnchor(box.x + box.w / 2 + kMm8DollFloorBiasX, box.y + box.h - kMm8DollFloorMarginY);
    layout.bodyPos = Pointi(floorAnchor.x - layout.body->width() / 2, floorAnchor.y - layout.body->height());

    if (layout.rhand) {
        int rhandRelY = std::lround(kMm8DollRightHandOffsetFracY * layout.body->height());
        Pointi apex = FindHandAttachApex(layout.rhand); // Where rhand's OWN silhouette actually touches the shoulder - not necessarily its row 0 (see comment above).
        int attachY = rhandRelY + apex.y;
        int torsoEdge = FindBodyLeftEdge(layout.body, attachY, attachY + 1);
        layout.rhandPos = Pointi(layout.bodyPos.x + torsoEdge - apex.x + kMm8DollRightHandOverlap, layout.bodyPos.y + rhandRelY);
    }
    return layout;
}

// Composites the Doll-preview panel - static `backdoll` background plus the current portrait's body/hand
// paperdoll sprites, drawn unscaled (native pixel size, no runtime scaling). The body is floor-anchored
// (centered + bottom-aligned in the panel). Hands are then anchored to the body sprite's own drawn position.
// See the `kMm8DollFloor*`/`kMm8Doll*Hand*` constants' comments.
void DrawMm8Doll(const Character &character, const Recti &box) {
    render->FillRect(box, kMm8DollPanelBackfillColor);

    // Clips every layer below, including `backdoll`. Unclipped, the backdrop (175x378, drawn at `box.y +
    // kMm8DollBackdropOffsetY`) overflows the bottom of a shorter box such as `kMm8CharacterRecordDollBox` (h=345)
    // and paints its floor art through the tab-button row beneath it.
    render->SetUIClipRect(box);
    if (ui_partycreation_doll_backdrop)
        render->DrawQuad2D(ui_partycreation_doll_backdrop, Pointi(box.x, box.y + kMm8DollBackdropOffsetY));

    std::shared_ptr<GraphicsImage> lhand = ui_partycreation_doll_lhand[character.uCurrentFace];

    Mm8DollBodyLayout layout = ComputeMm8DollBodyLayout(character, box);
    std::shared_ptr<GraphicsImage> body = layout.body;
    if (!body) {
        // No doll art for this portrait - leave just the backdrop rather than nothing.
        render->ResetUIClipRect();
        return;
    }
    std::shared_ptr<GraphicsImage> rhand = layout.rhand;
    Pointi bodyPos = layout.bodyPos;
    Pointi rhandPos = layout.rhandPos;

    // An equipped armour texture can be wider than the body sprite it is anchored against. The clip set above
    // keeps any overflowing equipment layer inside the panel.
    render->DrawQuad2D(body, bodyPos);

    std::vector<Mm8DollEquipmentLayer> equipmentLayers = ComputeMm8DollEquipmentLayers(character, body, bodyPos, rhand, rhandPos);
    auto drawEquipmentLayer = [&](Mm8DollLayerKind kind) {
        for (const Mm8DollEquipmentLayer &layer : equipmentLayers) {
            if (layer.kind == kind) {
                if (std::shared_ptr<GraphicsImage> texture = getImage_AlphaIfExists(layer.assetName))
                    render->DrawQuad2D(texture, layer.dstRect.topLeft());
            }
        }
    };

    // The armour is drawn before the hands. The armour sprite (`item084v2`, 100x252) is wider than the class-A body
    // (83x297), so drawn after the hands it would paint over the hand-attach seam. Drawn right after the body, it
    // keeps `FindBodyLeftEdge`/`FindBodyStubCutoff` measuring the base-body silhouette, and the hands stay on top.
    drawEquipmentLayer(Mm8DollLayerKind::Armor);

    if (rhand) {
        render->DrawQuad2D(rhand, rhandPos);

        // The weapon is composited over the grip, in front of the hand. The original draws armour and weapon
        // together with no z-order bug.
        drawEquipmentLayer(Mm8DollLayerKind::Weapon);
    }
    if (lhand) {
        int yLo = std::lround(kMm8DollStubSearchYFracLo * body->height());
        int yHi = std::lround(kMm8DollStubSearchYFracHi * body->height());
        Pointi stub = FindBodyStubCutoff(body, yLo, yHi); // Where the body's own baked arm stub is cut off.
        Pointi plug = FindHandTopRight(lhand, 3); // The matching point on lhand's own wrist cuff.
        Pointi lhandPos(bodyPos.x + stub.x - plug.x - kMm8DollLeftHandOverlap, bodyPos.y + stub.y - plug.y - kMm8DollLeftHandOverlapY);
        render->DrawQuad2D(lhand, lhandPos);
    }
    render->ResetUIClipRect();
}

// Real MM8 ties race/class to the chosen Portrait (see the `kMm8DollPreviewBox` comment above). There is no real
// portrait-to-race table yet, so this uses `uCurrentFace` modulo the 7 startable races as a documented placeholder.
static void ChangeClassForCurrentFaceMm8(Character &character) {
    Race race = allMm8StartableRaces()[character.uCurrentFace % allMm8StartableRaces().size()];
    character.ChangeClass(mm8BaseClassForRace(race));
    // `ChangeClass()` grants MM7-promotion-style catch-up XP, which does not apply to a starting race-class
    // picked with the Portrait arrows either (see `Party::Reset()`).
    character.experience = 0;
}

bool PlayerCreation_Choose4Skills() {
    // Bounded to a literal 4, not raw `pCharacters` (grown to 5). This gates the party-creation "OK" click for both
    // game types, before any MM8 recruiting has happened, so `activeCharacterCount()` is not the right bound either.
    // A full-array scan would require the never-recruited 5th MM8 slot to also have exactly 4 active skills, which
    // nothing sets, blocking new-game start.
    for (int i = 0; i < 4; i++) {
        const Character &character = pParty->pCharacters[i];
        int skills_count = 0;
        for (Skill i : allVisibleSkills())
            if (character.pActiveSkills[i])
                ++skills_count;

        if (skills_count != 4) return false;
    }

    return true;
}

void CreateParty_EventLoop() {
    auto pPlayer = pParty->pCharacters.data();
    while (engine->_messageQueue->haveMessages()) {
        UIMessageType msg;
        int param, param2;
        engine->_messageQueue->popMessage(&msg, &param, &param2);

        switch (msg) {
        case UIMSG_PlayerCreation_SelectAttribute:
        {
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem =
                (pGUIWindow_CurrentMenu->pCurrentPosActiveItem -
                    pGUIWindow_CurrentMenu->pStartingPosActiveItem) %
                7 +
                pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * param;
            uPlayerCreationUI_SelectedCharacter = param;
            pAudioPlayer->playUISound(SOUND_SelectingANewCharacter);
            break;
        }
        case UIMSG_PlayerCreation_VoicePrev:
        {
            Sex sex = pParty->pCharacters[param].GetSexByVoice();
            do {
                if (pParty->pCharacters[param].uVoiceID == 0) {
                    pParty->pCharacters[param].uVoiceID = 19;
                } else {
                    --pParty->pCharacters[param].uVoiceID;
                }
            } while (pParty->pCharacters[param].GetSexByVoice() != sex);
            auto pButton = pCreationUI_BtnPressLeft2[param];

            new OnButtonClick(pButton->rect.topLeft(), {0, 0}, pButton, std::string(), false);
            pAudioPlayer->playUISound(SOUND_SelectingANewCharacter);
            pAudioPlayer->stopVoiceSounds();
            pParty->pCharacters[param].playReaction(SPEECH_PICK_ME);
            break;
        }
        case UIMSG_PlayerCreation_VoiceNext:
        {
            Sex sex = pParty->pCharacters[param].GetSexByVoice();
            do {
                pParty->pCharacters[param].uVoiceID =
                    (pParty->pCharacters[param].uVoiceID + 1) % 20;
            } while (pParty->pCharacters[param].GetSexByVoice() != sex);
            auto pButton = pCreationUI_BtnPressRight2[param];
            new OnButtonClick(pButton->rect.topLeft(), {0, 0}, pButton, std::string(), false);
            pAudioPlayer->playUISound(SOUND_SelectingANewCharacter);
            pAudioPlayer->stopVoiceSounds();
            pParty->pCharacters[param].playReaction(SPEECH_PICK_ME);
            break;
        }
        case UIMSG_PlayerCreation_FacePrev:
            // pPlayer = &pParty->pCharacters[pParam];
            if (!pParty->pCharacters[param].uCurrentFace) {
                pParty->pCharacters[param].uCurrentFace = 19;
            } else {
                pParty->pCharacters[param].uCurrentFace -= 1;
            }
            pParty->pCharacters[param].uVoiceID =
                pParty->pCharacters[param].uCurrentFace;
            // Must run before SetInitialStats() below: it reads GetRace(), which for MM8 is derived from classType,
            // so the class must already reflect the new portrait.
            if (gameType() == GAME_TYPE_MM8)
                ChangeClassForCurrentFaceMm8(pParty->pCharacters[param]);
            pParty->pCharacters[param].SetInitialStats();
            pParty->pCharacters[param].SetSexByVoice();
            pParty->pCharacters[param].RandomizeName();
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem =
                (pGUIWindow_CurrentMenu->pCurrentPosActiveItem -
                    pGUIWindow_CurrentMenu->pStartingPosActiveItem) %
                7 +
                pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * param;
            uPlayerCreationUI_SelectedCharacter = param;
            new OnButtonClick(pCreationUI_BtnPressLeft[param]->rect.topLeft(), {0, 0},
                pCreationUI_BtnPressLeft[param], std::string(), false);
            pAudioPlayer->playUISound(SOUND_SelectingANewCharacter);
            pAudioPlayer->stopVoiceSounds();
            pParty->pCharacters[param].playReaction(SPEECH_PICK_ME);
            break;
        case UIMSG_PlayerCreation_FaceNext:
            // pPlayer = &pParty->pCharacters[pParam];
            int v20;
            v20 =
                (char)((int)pParty->pCharacters[param].uCurrentFace + 1) % 20;
            pParty->pCharacters[param].uCurrentFace = v20;
            pParty->pCharacters[param].uVoiceID = v20;
            // Must run before SetInitialStats() below, see UIMSG_PlayerCreation_FacePrev above.
            if (gameType() == GAME_TYPE_MM8)
                ChangeClassForCurrentFaceMm8(pParty->pCharacters[param]);
            pParty->pCharacters[param].SetInitialStats();
            pParty->pCharacters[param].SetSexByVoice();
            pParty->pCharacters[param].RandomizeName();
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem =
                (pGUIWindow_CurrentMenu->pCurrentPosActiveItem -
                    pGUIWindow_CurrentMenu->pStartingPosActiveItem) %
                7 +
                pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * param;
            uPlayerCreationUI_SelectedCharacter = param;
            new OnButtonClick(pCreationUI_BtnPressRight[param]->rect.topLeft(), {0, 0},
                pCreationUI_BtnPressRight[param], std::string(), false);
            pAudioPlayer->playUISound(SOUND_SelectingANewCharacter);
            pAudioPlayer->stopVoiceSounds();
            pParty->pCharacters[param].playReaction(SPEECH_PICK_ME);
            break;
        case UIMSG_PlayerCreationClickPlus:
            new OnButtonClick({613, 393}, {0, 0}, pPlayerCreationUI_BtnPlus, std::string(), false);
            pPlayer[uPlayerCreationUI_SelectedCharacter].IncreaseAttribute(
                static_cast<Attribute>((pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem) % 7));
            pAudioPlayer->playUISound(SOUND_ClickMinus);
            break;
        case UIMSG_PlayerCreationClickMinus:
            new OnButtonClick({523, 393}, {0, 0}, pPlayerCreationUI_BtnMinus, std::string(), false);
            pPlayer[uPlayerCreationUI_SelectedCharacter].DecreaseAttribute(
                static_cast<Attribute>((pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem) % 7));
            pAudioPlayer->playUISound(SOUND_ClickPlus);
            break;
        // MM8's per-row attribute spinners - `param` is the attribute index directly (the
        // row the click landed on), unlike UIMSG_PlayerCreationClickPlus/Minus above which act on
        // whichever row `pCurrentPosActiveItem` currently highlights.
        case UIMSG_PlayerCreationClickPlusMm8:
            new OnButtonClick(pCreationUI_BtnAttrPlus[param]->rect.topLeft(), {0, 0}, pCreationUI_BtnAttrPlus[param], std::string(), false);
            pParty->pCharacters[0].IncreaseAttribute(static_cast<Attribute>(param));
            pAudioPlayer->playUISound(SOUND_ClickMinus);
            break;
        case UIMSG_PlayerCreationClickMinusMm8:
            new OnButtonClick(pCreationUI_BtnAttrMinus[param]->rect.topLeft(), {0, 0}, pCreationUI_BtnAttrMinus[param], std::string(), false);
            pParty->pCharacters[0].DecreaseAttribute(static_cast<Attribute>(param));
            pAudioPlayer->playUISound(SOUND_ClickPlus);
            break;
        case UIMSG_PlayerCreationVoiceDefaultMm8:
            pParty->pCharacters[0].uVoiceID = pParty->pCharacters[0].uCurrentFace;
            new OnButtonClick(pCreationUI_BtnVoiceDefault->rect.topLeft(), {0, 0}, pCreationUI_BtnVoiceDefault, std::string(), false);
            pAudioPlayer->playUISound(SOUND_SelectingANewCharacter);
            pAudioPlayer->stopVoiceSounds();
            pParty->pCharacters[0].playReaction(SPEECH_PICK_ME);
            break;
        case UIMSG_PlayerCreationSelectActiveSkill:
            if (pPlayer[uPlayerCreationUI_SelectedCharacter].GetSkillIdxByOrder(3) == SKILL_INVALID)
                pParty->pCharacters[uPlayerCreationUI_SelectedCharacter].pActiveSkills[pPlayer[uPlayerCreationUI_SelectedCharacter]
                    .GetSkillIdxByOrder(param + 4)] = CombinedSkillValue::novice();
            pAudioPlayer->playUISound(SOUND_ClickSkill);
            break;
        case UIMSG_PlayerCreationSelectClass:
            pPlayer[uPlayerCreationUI_SelectedCharacter].ChangeClass((Class)param);
            pAudioPlayer->playUISound(SOUND_SelectingANewCharacter);
            break;
        case UIMSG_PlayerCreationClickOK:
            new OnButtonClick(pPlayerCreationUI_BtnOK->rect.topLeft(), {0, 0}, pPlayerCreationUI_BtnOK);
            if (CharacterCreation_GetUnspentAttributePointCount() || !PlayerCreation_Choose4Skills()) {
                errorMessageExpireTime = animTimer->time() + Duration::fromRealtimeSeconds(4); // show message for 4 seconds
            } else {
                uGameState = GAME_STATE_STARTING_NEW_GAME;
            }
            break;
        case UIMSG_PlayerCreationClickReset:
            new OnButtonClick(pPlayerCreationUI_BtnReset->rect.topLeft(), {0, 0}, pPlayerCreationUI_BtnReset);
            pParty->Reset();
            break;
        // MM8's 3rd bottom button behaves like Escape (abandon character creation), not like Reset/Clear
        // (which re-rolls the current character's stats).
        case UIMSG_PlayerCreationClickCancelMm8:
            new OnButtonClick(pCreationUI_BtnCancelMm8->rect.topLeft(), {0, 0}, pCreationUI_BtnCancelMm8);
            uGameState = GAME_FINISHED;
            break;
        case UIMSG_PlayerCreationRemoveUpSkill:
        {
            int v4;
            v4 = pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem;
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem = v4 % 7 + pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * param;
            if (pPlayer[param].GetSkillIdxByOrder(2) != SKILL_INVALID)
                pParty->pCharacters[param].pActiveSkills[pPlayer[param].GetSkillIdxByOrder(2)] = CombinedSkillValue::none();
            break;
        }
        case UIMSG_PlayerCreationRemoveDownSkill:
        {
            int v4;
            v4 = pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem;
            pGUIWindow_CurrentMenu->pCurrentPosActiveItem = v4 % 7 + pGUIWindow_CurrentMenu->pStartingPosActiveItem + 7 * param;
            if (pPlayer[param].GetSkillIdxByOrder(3) != SKILL_INVALID)
                pParty->pCharacters[param].pActiveSkills[pPlayer[param].GetSkillIdxByOrder(3)] = CombinedSkillValue::none();
        } break;
        case UIMSG_PlayerCreationChangeName:
            pAudioPlayer->playUISound(SOUND_ClickSkill);
            uPlayerCreationUI_NameEditCharacter = param;
            keyboardInputHandler->StartTextInput(TEXT_INPUT_TEXT, 15, pGUIWindow_CurrentMenu.get());
            break;
        case UIMSG_Escape:
            if (!(engineFlags & ENGINE_ESCAPE_ENABLED)) break;
            if (GetCurrentMenuID() == MENU_MAIN ||
                GetCurrentMenuID() == MENU_MMT_MAIN_MENU ||
                GetCurrentMenuID() == MENU_CREATEPARTY ||
                GetCurrentMenuID() == MENU_NAMEPANELESC) {
                // if ( current_screen_type == SCREEN_VIDEO )
                // pVideoPlayer->FastForwardToFrame(pVideoPlayer->pResetflag);
                engine->_messageQueue->addMessageCurrentFrame(UIMSG_ChangeGameState,
                    0, 0);
            }
            break;
        case UIMSG_ChangeGameState:
            uGameState = GAME_FINISHED;
            break;
        default:
            break;
        }
    }
}

bool PartyCreationUI_Loop() {
    pAudioPlayer->MusicStop();
    gameTimer->setPaused(true);

    // This call is here b/c otherwise Character::timeToRecovery will be overwritten in the main loop from the
    // turn-based queue if we're currently in turn-based combat.
    pTurnEngine->End(false);

    pParty->Reset();
    pParty->createDefaultParty();
    if (gameType() == GAME_TYPE_MM8)
        pParty->seatStartingInnRecruits();

    pNPCStats->pNPCData = pNPCStats->pOriginalNPCData;
    pNPCStats->pGroups = pNPCStats->pOriginalGroups;
    // MM6/7-only starting companion, auto-joined at character creation. MM8's npcdata.txt has an unrelated NPC
    // at this index, and a fresh MM8 party starts solo.
    if (gameType() != GAME_TYPE_MM8)
        pNPCStats->pNPCData[3].flags |= NPC_HIRED; // Lady Margaret.

    pGUIWindow_CurrentMenu = std::make_unique<GUIWindow_PartyCreation>();
    return !PartyCreationUI_LoopInternal();
}

//----- (00495B39) --------------------------------------------------------
// void PlayerCreationUI_Draw()
void GUIWindow_PartyCreation::Update() {
    int pTextCenter;                // eax@3
    int pX;                         // ecx@7
    GUIButton *uPosActiveItem;      // edi@12
    int v17;                        // eax@33
    Color pStatColor;        // eax@44
    Skill pSkillsType;  // eax@44
    Class uClassType;   // edi@53
    Color pColorText;                 // eax@53
    Skill pSkillId;     // edi@72
    size_t pLenText;                // eax@72
    signed int v104;                // ecx@72
    signed int pBonusNum;           // edi@82
    std::string pText;                // [sp+10h] [bp-160h]@14
    int v126;                       // [sp+148h] [bp-28h]@25
    int pIntervalY;                 // [sp+150h] [bp-20h]@14
    int pX_Numbers;                 // [sp+154h] [bp-1Ch]@18
    int uX;                         // [sp+160h] [bp-10h]@18
    int pIntervalX;
    int pCorrective;

    // move sky
    render->BeginScene2D();
    // 2D scenes are only ever painted over, never cleared. Without an opaque fill first, any pixel this screen's
    // art doesn't cover (missing background, or transparent-keyed regions such as MM8's makeme.pcx colorkeyed panel)
    // would show whatever the previous screen drew there.
    render->FillRect(frameRect, colorTable.Black);
    if (main_menu_background)
        render->DrawQuad2D(main_menu_background, {0, 0});
    int sky_slider_anim_timer = static_cast<int>(std::fmod(animTimer->time().realtimeMillisecondsFloat() * 640.0 / 20, 640.0));
    render->DrawQuad2D(ui_partycreation_sky_scroller, {sky_slider_anim_timer, 2});
    render->DrawQuad2D(ui_partycreation_sky_scroller, {sky_slider_anim_timer - 640, 2});
    render->DrawQuad2D(ui_partycreation_top, {0, 0});

    if (gameType() == GAME_TYPE_MM8) {
        UpdateMm8();
        return;
    }

    uPlayerCreationUI_SelectedCharacter = (pGUIWindow_CurrentMenu->pCurrentPosActiveItem - pGUIWindow_CurrentMenu->pStartingPosActiveItem) / 7;
    switch (uPlayerCreationUI_SelectedCharacter) {
        case 0:
            pX = 12;
            break;
        case 1:
            pX = 171;
            break;
        case 2:
            pX = 329;
            break;
        case 3:
            pX = 488;
            break;
        default:
            assert(false);
            pX = 0;
            break;
    }

    pTextCenter = ui_partycreation_font->AlignText_Center(
        640, localization->str(LSTR_C_R_E_A_T_E_P_A_R_T_Y));
    DrawText(ui_partycreation_font.get(), {pTextCenter + 1, 0}, colorTable.White,
        localization->str(LSTR_C_R_E_A_T_E_P_A_R_T_Y), pGUIWindow_CurrentMenu->frameRect);

    render->DrawQuad2D(ui_partycreation_portraits[pParty->pCharacters[0].uCurrentFace], {17, 35});
    render->DrawQuad2D(ui_partycreation_portraits[pParty->pCharacters[1].uCurrentFace], {176, 35});
    render->DrawQuad2D(ui_partycreation_portraits[pParty->pCharacters[2].uCurrentFace], {335, 35});
    render->DrawQuad2D(ui_partycreation_portraits[pParty->pCharacters[3].uCurrentFace], {494, 35});

    // arrows
    render->DrawQuad2D(ui_partycreation_character_frame, {pX, 29});
    uPosActiveItem = pGUIWindow_CurrentMenu->GetControl(pGUIWindow_CurrentMenu->pCurrentPosActiveItem);
    // cycle arrows backwards
    int arrowAnimTextureNum = ui_partycreation_arrow_l.size() - 1 - (animTimer->time().realtimeMilliseconds() % ARROW_SPIN_PERIOD_MS) / (ARROW_SPIN_PERIOD_MS / ui_partycreation_arrow_l.size());
    render->DrawQuad2D(ui_partycreation_arrow_l[arrowAnimTextureNum], {uPosActiveItem->rect.x + uPosActiveItem->rect.w - 4, uPosActiveItem->rect.y});
    render->DrawQuad2D(ui_partycreation_arrow_r[arrowAnimTextureNum], {uPosActiveItem->rect.x - 12, uPosActiveItem->rect.y});

    pText = localization->str(LSTR_SKILLS);
    for (int i = pText.size() - 1; i >= 0; i--)
        pText[i] = toupper(pText[i]); // TODO(captainurist): #unicode this won't work with a Russian localization.

    pIntervalX = 18;
    pIntervalY = assets->pFontCreate->GetHeight() - 2;
    uX = 32;
    pX_Numbers = 640 - 147;  // 493;

    for (int i = 0; i < 4; ++i) {
        DrawText(assets->pFontCreate.get(), {pIntervalX + 73, 100}, colorTable.White,
            localization->expand(localization->className(pParty->pCharacters[i].classType)), pGUIWindow_CurrentMenu->frameRect);
        // `ui_partycreation_class_icons` only has entries for MM7's 9 base classes - MM8 has no equivalent
        // icon art, so the icon is just skipped for MM8 classes.
        if (!isMm8Class(pParty->pCharacters[i].classType)) {
            render->DrawQuad2D(ui_partycreation_class_icons[std::to_underlying(pParty->pCharacters[i].classType) / 4], {pIntervalX + 77, 50});
        }

        if (pGUIWindow_CurrentMenu->keyboard_input_status != WINDOW_INPUT_NONE && uPlayerCreationUI_NameEditCharacter == i) {
            switch (pGUIWindow_CurrentMenu->keyboard_input_status) {
            case WINDOW_INPUT_IN_PROGRESS:  // press name panel
                v17 = pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontCreate.get(), {159 * uPlayerCreationUI_NameEditCharacter + 18, 124}, colorTable.White,
                    keyboardInputHandler->GetTextInput(), 120, 1);
                DrawFlashingInputCursor(159 * uPlayerCreationUI_NameEditCharacter + v17 + 20, 124, assets->pFontCreate.get(), pGUIWindow_CurrentMenu->frameRect);
                break;
            case WINDOW_INPUT_CONFIRMED:  // press enter
                pGUIWindow_CurrentMenu->keyboard_input_status = WINDOW_INPUT_NONE;
                v126 = 0;
                for (int j = 0; j < keyboardInputHandler->GetTextInput().size(); ++j) // edit name
                    if (keyboardInputHandler->GetTextInput()[j] == ' ')
                        ++v126;
                if (keyboardInputHandler->GetTextInput().size() > 0 && v126 != keyboardInputHandler->GetTextInput().size())
                    pParty->pCharacters[i].name = keyboardInputHandler->GetTextInput();
                pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontCreate.get(), {pIntervalX, 124}, colorTable.White, pParty->pCharacters[i].name, 130, 0);
                break;
            default:
                break;
            }
        } else {
            pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontCreate.get(), {pIntervalX, 124}, colorTable.White, pParty->pCharacters[i].name, 130, 0);
        }

        std::string pRaceName = pParty->pCharacters[i].GetRaceName();
        pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontCreate.get(), {pIntervalX + 72, pIntervalY + 12}, colorTable.White, pRaceName, 130, 0);

        pTextCenter = assets->pFontCreate->AlignText_Center(150, pText);
        DrawText(assets->pFontCreate.get(), {pTextCenter + uX - 24, 291}, colorTable.Tacha, pText, pGUIWindow_CurrentMenu->frameRect);  // Skills

        int posY = 169;

        auto str1 = fmt::format("{}\r{:03}{}", localization->str(LSTR_MIGHT), pX_Numbers, pParty->pCharacters[i].GetActualMight());
        pStatColor = pParty->pCharacters[i].GetStatColor(ATTRIBUTE_MIGHT);
        DrawText(assets->pFontCreate.get(), {uX, posY}, pStatColor, str1, pGUIWindow_CurrentMenu->frameRect);

        auto str2 = fmt::format("{}\r{:03}{}", localization->str(LSTR_INTELLECT), pX_Numbers, pParty->pCharacters[i].GetActualIntelligence());
        pStatColor = pParty->pCharacters[i].GetStatColor(ATTRIBUTE_INTELLIGENCE);
        DrawText(assets->pFontCreate.get(), {uX, pIntervalY + posY}, pStatColor, str2, pGUIWindow_CurrentMenu->frameRect);
        auto str3 = fmt::format("{}\r{:03}{}", localization->str(LSTR_PERSONALITY), pX_Numbers, pParty->pCharacters[i].GetActualPersonality());
        pStatColor = pParty->pCharacters[i].GetStatColor(ATTRIBUTE_PERSONALITY);
        DrawText(assets->pFontCreate.get(), {uX, 2 * pIntervalY + posY}, pStatColor, str3, pGUIWindow_CurrentMenu->frameRect);

        auto str4 = fmt::format("{}\r{:03}{}", localization->str(LSTR_ENDURANCE), pX_Numbers, pParty->pCharacters[i].GetActualEndurance());
        pStatColor = pParty->pCharacters[i].GetStatColor(ATTRIBUTE_ENDURANCE);
        DrawText(assets->pFontCreate.get(), {uX, 3 * pIntervalY + posY}, pStatColor, str4, pGUIWindow_CurrentMenu->frameRect);
        auto str5 = fmt::format("{}\r{:03}{}", localization->str(LSTR_ACCURACY), pX_Numbers, pParty->pCharacters[i].GetActualAccuracy());
        pStatColor = pParty->pCharacters[i].GetStatColor(ATTRIBUTE_ACCURACY);
        DrawText(assets->pFontCreate.get(), {uX, 4 * pIntervalY + posY}, pStatColor, str5, pGUIWindow_CurrentMenu->frameRect);

        auto str6 = fmt::format("{}\r{:03}{}", localization->str(LSTR_SPEED), pX_Numbers, pParty->pCharacters[i].GetActualSpeed());
        pStatColor = pParty->pCharacters[i].GetStatColor(ATTRIBUTE_SPEED);
        DrawText(assets->pFontCreate.get(), {uX, 5 * pIntervalY + posY}, pStatColor, str6, pGUIWindow_CurrentMenu->frameRect);
        auto str7 = fmt::format("{}\r{:03}{}", localization->str(LSTR_LUCK), pX_Numbers, pParty->pCharacters[i].GetActualLuck());
        pStatColor = pParty->pCharacters[i].GetStatColor(ATTRIBUTE_LUCK);
        DrawText(assets->pFontCreate.get(), {uX, 6 * pIntervalY + posY}, pStatColor, str7, pGUIWindow_CurrentMenu->frameRect);

        posY = 311;

        pSkillsType = pParty->pCharacters[i].GetSkillIdxByOrder(0);
        pTextCenter = assets->pFontCreate->AlignText_Center(150, localization->skillName(pSkillsType));
        auto str8 = fmt::format("\t{:03}{}", pTextCenter, localization->skillName(pSkillsType));
        DrawText(assets->pFontCreate.get(), {uX - 24, posY}, colorTable.White, str8, pGUIWindow_CurrentMenu->frameRect);

        pSkillsType = pParty->pCharacters[i].GetSkillIdxByOrder(1);
        pTextCenter = assets->pFontCreate->AlignText_Center(150, localization->skillName(pSkillsType));
        auto str9 = fmt::format("\t{:03}{}", pTextCenter, localization->skillName(pSkillsType));
        DrawText(assets->pFontCreate.get(), {uX - 24, pIntervalY + posY}, colorTable.White, str9, pGUIWindow_CurrentMenu->frameRect);

        pSkillsType = pParty->pCharacters[i].GetSkillIdxByOrder(2);
        pTextCenter = assets->pFontCreate->AlignText_Center(150, localization->skillName(pSkillsType));
        auto str10 = fmt::format("\t{:03}{}", pTextCenter, localization->skillName(pSkillsType));
        pColorText = colorTable.Green;
        if (pSkillsType == SKILL_INVALID)
            pColorText = colorTable.Aqua;
        DrawText(assets->pFontCreate.get(), {uX - 24, 2 * pIntervalY + posY}, pColorText, str10, pGUIWindow_CurrentMenu->frameRect);

        pSkillsType = pParty->pCharacters[i].GetSkillIdxByOrder(3);
        pTextCenter = assets->pFontCreate->AlignText_Center(150, localization->skillName(pSkillsType));
        auto str11 = fmt::format("\t{:03}{}", pTextCenter, localization->skillName(pSkillsType));
        pColorText = colorTable.Green;
        if (pSkillsType == SKILL_INVALID)
            pColorText = colorTable.Aqua;
        DrawText(assets->pFontCreate.get(), {uX - 24, 3 * pIntervalY + posY}, pColorText, str11, pGUIWindow_CurrentMenu->frameRect);

        pIntervalX += 159;
        pX_Numbers -= 158;
        uX += 158;
    }

    pText = localization->str(LSTR_CLASS);
    for (int i = pText.size() - 1; i >= 0; i--)
        pText[i] = toupper(pText[i]); // TODO(captainurist): #unicode this won't work for Russian localization.

    uClassType = pParty->pCharacters[uPlayerCreationUI_SelectedCharacter].classType;
    pTextCenter = assets->pFontCreate->AlignText_Center(193, pText);
    DrawText(assets->pFontCreate.get(), {pTextCenter + 324, 395}, colorTable.Tacha, pText, pGUIWindow_CurrentMenu->frameRect);  // Classes

    if (gameType() != GAME_TYPE_MM8) {
        pColorText = colorTable.Aqua;
        if (uClassType != CLASS_KNIGHT)
            pColorText = colorTable.White;
        pTextCenter = assets->pFontCreate->AlignText_Center(65, localization->className(CLASS_KNIGHT));
        DrawText(assets->pFontCreate.get(), {pTextCenter + 323, 417}, pColorText, localization->className(CLASS_KNIGHT), pGUIWindow_CurrentMenu->frameRect);

        pColorText = colorTable.Aqua;
        if (uClassType != CLASS_PALADIN)
            pColorText = colorTable.White;
        pTextCenter =
            assets->pFontCreate->AlignText_Center(65, localization->className(CLASS_PALADIN));
        DrawText(assets->pFontCreate.get(), {pTextCenter + 323, pIntervalY + 417}, pColorText, localization->className(CLASS_PALADIN), pGUIWindow_CurrentMenu->frameRect);

        pColorText = colorTable.Aqua;
        if (uClassType != CLASS_RANGER)
            pColorText = colorTable.White;
        pTextCenter =
            assets->pFontCreate->AlignText_Center(65, localization->className(CLASS_RANGER));
        DrawText(assets->pFontCreate.get(), {pTextCenter + 323, 2 * pIntervalY + 417}, pColorText, localization->className(CLASS_RANGER), pGUIWindow_CurrentMenu->frameRect);

        pColorText = colorTable.Aqua;
        if (uClassType != CLASS_CLERIC)
            pColorText = colorTable.White;
        pTextCenter =
            assets->pFontCreate->AlignText_Center(65, localization->className(CLASS_CLERIC));
        DrawText(assets->pFontCreate.get(), {pTextCenter + 388, 417}, pColorText, localization->className(CLASS_CLERIC), pGUIWindow_CurrentMenu->frameRect);

        pColorText = colorTable.Aqua;
        if (uClassType != CLASS_DRUID)
            pColorText = colorTable.White;
        pTextCenter =
            assets->pFontCreate->AlignText_Center(65, localization->className(CLASS_DRUID));
        DrawText(assets->pFontCreate.get(), {pTextCenter + 388, pIntervalY + 417}, pColorText, localization->className(CLASS_DRUID), pGUIWindow_CurrentMenu->frameRect);

        pColorText = colorTable.Aqua;
        if (uClassType != CLASS_SORCERER)
            pColorText = colorTable.White;
        pTextCenter =
            assets->pFontCreate->AlignText_Center(65, localization->className(CLASS_SORCERER));
        DrawText(assets->pFontCreate.get(), {pTextCenter + 388, 2 * pIntervalY + 417}, pColorText, localization->className(CLASS_SORCERER), pGUIWindow_CurrentMenu->frameRect);

        pColorText = colorTable.Aqua;
        if (uClassType != CLASS_ARCHER)
            pColorText = colorTable.White;
        pTextCenter =
            assets->pFontCreate->AlignText_Center(65, localization->className(CLASS_ARCHER));
        DrawText(assets->pFontCreate.get(), {pTextCenter + 453, 417}, pColorText, localization->className(CLASS_ARCHER), pGUIWindow_CurrentMenu->frameRect);

        pColorText = colorTable.Aqua;
        if (uClassType != CLASS_MONK)
            pColorText = colorTable.White;
        pTextCenter =
            assets->pFontCreate->AlignText_Center(65, localization->className(CLASS_MONK));
        DrawText(assets->pFontCreate.get(), {pTextCenter + 453, pIntervalY + 417}, pColorText, localization->className(CLASS_MONK), pGUIWindow_CurrentMenu->frameRect);

        pColorText = colorTable.Aqua;
        if (uClassType != CLASS_THIEF)
            pColorText = colorTable.White;
        pTextCenter = assets->pFontCreate->AlignText_Center(65, localization->className(CLASS_THIEF));
        DrawText(assets->pFontCreate.get(), {pTextCenter + 453, 2 * pIntervalY + 417}, pColorText, localization->className(CLASS_THIEF), pGUIWindow_CurrentMenu->frameRect);
    } else {
        // MM8 offers the 7 startable race-classes instead of MM7's 9 classes, reusing the same 3x3
        // button grid (2 of the 9 slots stay empty).
        std::array<Pointi, 9> classSlots = {{
            {323, 417}, {323, pIntervalY + 417}, {323, 2 * pIntervalY + 417},
            {388, 417}, {388, pIntervalY + 417}, {388, 2 * pIntervalY + 417},
            {453, 417}, {453, pIntervalY + 417}, {453, 2 * pIntervalY + 417},
        }};

        int slot = 0;
        for (Race race : allMm8StartableRaces()) {
            Class raceClass = mm8BaseClassForRace(race);
            pColorText = (uClassType == raceClass) ? colorTable.Aqua : colorTable.White;
            const std::string &name = localization->className(raceClass);
            pTextCenter = assets->pFontCreate->AlignText_Center(65, name);
            DrawText(assets->pFontCreate.get(), {pTextCenter + classSlots[slot].x, classSlots[slot].y}, pColorText, name, pGUIWindow_CurrentMenu->frameRect);
            ++slot;
        }
    }

    pTextCenter = assets->pFontCreate->AlignText_Center(
        236, localization->str(LSTR_AVAILABLE_SKILLS));
    DrawText(assets->pFontCreate.get(), {pTextCenter + 37, 395}, colorTable.Tacha, localization->str(LSTR_AVAILABLE_SKILLS), pGUIWindow_CurrentMenu->frameRect);
    for (int i = 0; i < 9; ++i) {
        pSkillId = pParty->pCharacters[uPlayerCreationUI_SelectedCharacter].GetSkillIdxByOrder(i + 4);
        pText = localization->skillName(pSkillId);
        // trim skills that are too long
        if (pText.size() > 13)
            pText.resize(12);
        if (pText == "Body Building")
            pText = "Body Build";
        if (pText.starts_with(' '))
            pText.clear();

        pCorrective = -10;             // -5
        //if ((signed int)pLenText < 8)  // if ( (signed int)v124 > 2 )
        //    pCorrective = 0;
        pColorText = colorTable.Aqua;
        if (!pParty->pCharacters[uPlayerCreationUI_SelectedCharacter].pActiveSkills[pSkillId])
            pColorText = colorTable.White;

        pTextCenter = assets->pFontCreate->AlignText_Center(100, pText);
        DrawText(assets->pFontCreate.get(), {100 * (i / 3) + pTextCenter + pCorrective + 17, pIntervalY * (i % 3) + 417}, pColorText, pText, pGUIWindow_CurrentMenu->frameRect);
    }

    pTextCenter = assets->pFontCreate->AlignText_Center(
                0x5C, localization->str(LSTR_BONUS_1));
        DrawText(assets->pFontCreate.get(), {pTextCenter + 533, 394}, colorTable.Tacha, localization->str(LSTR_BONUS_1), pGUIWindow_CurrentMenu->frameRect);

    // force draw so overlays dont get muddled
    render->DrawTwodVerts();
    render->EndTextNew();

    pBonusNum = CharacterCreation_GetUnspentAttributePointCount();

    auto unspent_attribute_bonus_label = fmt::format("{}", pBonusNum);
    pTextCenter = assets->pFontCreate->AlignText_Center(84, unspent_attribute_bonus_label);
    DrawText(assets->pFontCreate.get(), {pTextCenter + 530, 410}, colorTable.White, unspent_attribute_bonus_label, pGUIWindow_CurrentMenu->frameRect);

    if (errorMessageExpireTime > animTimer->time()) {
        auto& sHint = pBonusNum < 0 ? localization->str(LSTR_YOU_CANT_SPEND_MORE_THAN_50_POINTS) : localization->str(LSTR_CREATE_PARTY_CANNOT_BE_COMPLETED_UNLESS);
        Recti popupRect(170, 140, 300, 100);
        DrawMessageBox(popupRect, sHint);
    }

    // force draw so overlays dont get muddled
    render->DrawTwodVerts();
    render->EndTextNew();
}

// MM8's single-champion "Create Character" screen - see the `kMm8*` layout constants above for how the layout was
// derived. Unlike MM7's `Update()` above, there's only ever one character on screen (`pParty->pCharacters[0]`). The
// other 3 party slots are still populated (see `Party::Reset`'s MM8 defaults) but aren't shown here, since
// `PartyMM8Roster` isn't wired up to gameplay yet, so the party's real size is not changed.
void GUIWindow_PartyCreation::UpdateMm8() {
    Character &character = pParty->pCharacters[0];

    // Small thumbnail in its own spot to the left of the Portrait/Voice box below, so the box's baked
    // "Portrait"/"Voice" captions and cycle arrows stay visible (see `kMm8PortraitThumbBox` comment above).
    DrawImageCoverFit(ui_partycreation_portraits[character.uCurrentFace], kMm8PortraitThumbBox);
    // Gold oval ring overlay, drawn over the portrait thumbnail - see the constant's comment.
    if (ui_partycreation_portrait_ring)
        render->DrawQuad2D(ui_partycreation_portrait_ring, kMm8PortraitRingBox.topLeft());

    // Portrait/Voice cycle arrows + the "Default" (revert-to-default-voice) button, drawn here every frame. The
    // buttons' `vTextures` only show as a transient click-flash (see `OnButtonClick::Update`), with no persistent
    // idle-state renderer, so an icon never drawn here would be an invisible hitbox. The arrows use the explicit
    // src/dst overload, see `kMm8ArrowIconSrcRect`'s comment.
    render->DrawQuad2D(ui_partycreation_left, kMm8ArrowIconSrcRect, kMm8FacePrevBtn);
    render->DrawQuad2D(ui_partycreation_right, kMm8ArrowIconSrcRect, kMm8FaceNextBtn);
    render->DrawQuad2D(ui_partycreation_left, kMm8ArrowIconSrcRect, kMm8VoicePrevBtn);
    render->DrawQuad2D(ui_partycreation_right, kMm8ArrowIconSrcRect, kMm8VoiceNextBtn);
    // Native size, no scaling: `c_dft_up` is already the real screen's own button size.
    if (ui_partycreation_default)
        render->DrawQuad2D(ui_partycreation_default, kMm8DefaultBtn.topLeft());

    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontCreate.get(), {kMm8NameBox.x, kMm8NameBox.y}, colorTable.White, character.name, kMm8NameBox.w + 40, 0);
    pGUIWindow_CurrentMenu->DrawTextInRect(assets->pFontCreate.get(), {kMm8ClassBox.x, kMm8ClassBox.y}, colorTable.White, localization->className(character.classType), kMm8ClassBox.w + 40, 0);

    // Attributes: one row per attribute, each with its own [-]/[+] spinner icon pair acting directly
    // on that row - matching the real MM8 screen (MANUAL.PDF pg.6), unlike MM7's single shared
    // pPlayerCreationUI_BtnPlus/Minus pair that acts on whichever row is currently keyboard-selected.
    static constexpr std::array<std::pair<LstrId, Attribute>, 7> kAttributes = {{
        {LSTR_MIGHT, ATTRIBUTE_MIGHT}, {LSTR_INTELLECT, ATTRIBUTE_INTELLIGENCE},
        {LSTR_PERSONALITY, ATTRIBUTE_PERSONALITY}, {LSTR_ENDURANCE, ATTRIBUTE_ENDURANCE},
        {LSTR_ACCURACY, ATTRIBUTE_ACCURACY}, {LSTR_SPEED, ATTRIBUTE_SPEED}, {LSTR_LUCK, ATTRIBUTE_LUCK},
    }};
    static constexpr std::array<int (Character::*)() const, 7> kGetters = {
        &Character::GetActualMight, &Character::GetActualIntelligence, &Character::GetActualPersonality,
        &Character::GetActualEndurance, &Character::GetActualAccuracy, &Character::GetActualSpeed, &Character::GetActualLuck,
    };
    for (int i = 0; i < 7; ++i) {
        auto [labelId, attribute] = kAttributes[i];
        Color statColor = character.GetStatColor(attribute);
        int rowY = kMm8AttributesFirstRowY + i * kMm8AttributesRowGap;
        DrawText(assets->pFontCreate.get(), {kMm8AttributesLabelX, rowY}, colorTable.White, localization->str(labelId), pGUIWindow_CurrentMenu->frameRect);
        // Centered between the [-]/[+] spinners, see `kMm8AttributesNumberColW`'s comment.
        auto number = fmt::format("{}", (character.*kGetters[i])());
        int numberX = kMm8AttributesNumberColX + assets->pFontCreate->AlignText_Center(kMm8AttributesNumberColW, number);
        DrawText(assets->pFontCreate.get(), {numberX, rowY}, statColor, number, pGUIWindow_CurrentMenu->frameRect);
        render->DrawQuad2D(ui_partycreation_attr_minus, {kMm8AttrMinusX, rowY});
        render->DrawQuad2D(ui_partycreation_attr_plus, {kMm8AttrPlusX, rowY});
    }

    // Skills: chosen skills (2x2 - MM8's char-create always requires exactly 4, same as MM7) and the
    // remaining choices (2x5, `GetSkillIdxByOrder(4..12)`), both laid out column-major like the real
    // MM8 screen (see MANUAL.PDF pg.6) rather than MM7's 3-wide grid.
    // The real screen colors the 2 class-given (`PRIMARY`) skills Yellow and the 2 player-chosen (`AVAILABLE`) ones Aqua.
    for (int i = 0; i < 4; ++i) {
        Skill skill = character.GetSkillIdxByOrder(i);
        Color color = i >= 2 ? colorTable.Aqua : colorTable.Yellow;
        std::string_view skillText = localization->skillName(skill);
        // Centered within its half of the box, see `kMm8SkillColW`'s comment.
        int baseX = kMm8SkillsBox.x + (i % 2) * kMm8SkillColW;
        int rowY = kMm8SkillsBox.y + kMm8SkillFirstRowY + (i / 2) * kMm8SkillRowGap;
        DrawSkillName(assets->pFontCreate.get(), color, skillText, baseX, kMm8SkillColW, rowY, pGUIWindow_CurrentMenu->frameRect);
    }
    for (int i = 0; i < 9; ++i) {
        Skill skill = character.GetSkillIdxByOrder(i + 4);
        // Some MM8 race/classes offer fewer than 9 AVAILABLE-affinity skills, so this can be SKILL_INVALID, which
        // the `pActiveSkills[skill]` read below must not index with.
        if (skill == SKILL_INVALID)
            continue;
        std::string_view skillText = localization->skillName(skill);
        if (skillText.starts_with(' '))
            continue;
        Color color = character.pActiveSkills[skill] ? colorTable.Aqua : colorTable.White;
        // Index 8 is always alone on the last row (2 columns * 4 full rows = indices 0-7), and the real screen centers
        // a lone trailing skill across the full box width, not its left-column half.
        int baseX = i == 8 ? kMm8AvailableSkillsBox.x : kMm8AvailableSkillsBox.x + (i % 2) * kMm8SkillColW;
        int colW = i == 8 ? kMm8AvailableSkillsBox.w : kMm8SkillColW;
        int rowY = kMm8AvailableSkillsBox.y + kMm8SkillFirstRowY + (i / 2) * kMm8SkillRowGap;
        DrawSkillName(assets->pFontCreate.get(), color, skillText, baseX, colW, rowY, pGUIWindow_CurrentMenu->frameRect);
    }

    // force draw so overlays dont get muddled
    render->DrawTwodVerts();
    render->EndTextNew();

    // Doll preview panel: real MM8's character-model render, see `DrawMm8Doll`.
    DrawMm8Doll(character, kMm8DollPreviewBox);

    // Clear/Cancel/OK: real MM8 button art, drawn explicitly every frame like the Portrait/Voice/Default controls above.
    if (ui_partycreation_clear_mm8)
        render->DrawQuad2D(ui_partycreation_clear_mm8, Recti(0, 0, ui_partycreation_clear_mm8->width(), ui_partycreation_clear_mm8->height()), kMm8ClearBtn);
    if (ui_partycreation_cancel_mm8)
        render->DrawQuad2D(ui_partycreation_cancel_mm8, Recti(0, 0, ui_partycreation_cancel_mm8->width(), ui_partycreation_cancel_mm8->height()), kMm8CancelBtn);
    if (ui_partycreation_ok_mm8)
        render->DrawQuad2D(ui_partycreation_ok_mm8, Recti(0, 0, ui_partycreation_ok_mm8->width(), ui_partycreation_ok_mm8->height()), kMm8OkBtn);

    int pBonusNum = CharacterCreation_GetUnspentAttributePointCount();
    auto unspentLabel = fmt::format("{}", pBonusNum);
    DrawText(assets->pFontCreate.get(), kMm8BonusPoolValuePos, colorTable.White, unspentLabel, pGUIWindow_CurrentMenu->frameRect);

    if (errorMessageExpireTime > animTimer->time()) {
        auto &sHint = pBonusNum < 0 ? localization->str(LSTR_YOU_CANT_SPEND_MORE_THAN_50_POINTS) : localization->str(LSTR_CREATE_PARTY_CANNOT_BE_COMPLETED_UNLESS);
        Recti popupRect(170, 140, 300, 100);
        DrawMessageBox(popupRect, sHint);
    }

    // force draw so overlays dont get muddled
    render->DrawTwodVerts();
    render->EndTextNew();
}

//----- (0049695A) --------------------------------------------------------
GUIWindow_PartyCreation::GUIWindow_PartyCreation() :
    GUIWindow(WINDOW_PARTY_CREATION, {0, 0}, render->GetRenderDimensions()) {
    engine->_messageQueue->clear();
    errorMessageExpireTime = Duration(); // Clear any lingering error popup from previous session.

    // MM8 ships `makeme.pcx` under EnglishD.lod rather than icons.lod. Its colorkeyed (TealMask) panel on the right is
    // the "Doll preview" panel (see the layout comment near `ConstructMm8Controls`). The colorkey is stripped either way
    // so it reads as an empty panel instead of a solid teal block.
    main_menu_background = gameType() == GAME_TYPE_MM8
        ? assets->getImage_PCXFromEnglishDLOD("makeme.pcx", colorTable.TealMask)
        : assets->getImage_PCXFromIconsLOD("makeme.pcx");

    current_screen_type = SCREEN_PARTY_CREATION;
    uPlayerCreationUI_SelectedCharacter = 0;
    int v0 = assets->pFontCreate->GetHeight() - 2;

    ui_partycreation_class_icons[0] = assets->getIcon("IC_KNIGHT");
    ui_partycreation_class_icons[1] = assets->getIcon("IC_THIEF");
    ui_partycreation_class_icons[2] = assets->getIcon("IC_MONK");
    ui_partycreation_class_icons[3] = assets->getIcon("IC_PALAD");
    ui_partycreation_class_icons[4] = assets->getIcon("IC_ARCH");
    ui_partycreation_class_icons[5] = assets->getIcon("IC_RANGER");
    ui_partycreation_class_icons[6] = assets->getIcon("IC_CLER");
    ui_partycreation_class_icons[7] = assets->getIcon("IC_DRUID");
    ui_partycreation_class_icons[8] = assets->getIcon("IC_SORC");

    ui_partycreation_top = getImage_AlphaIfExists("MAKETOP");
    ui_partycreation_sky_scroller = getImage_SolidIfExists("MAKESKY");

    ui_partycreation_character_frame = getImage_SolidIfExists("aframe1");

    for (int uX = 0; uX < 22; ++uX) {
        // MM8's icons.lod names every portrait "pcNN-01" (a consistent dash before the frame number). The shared MM7
        // `pPlayerPortraitsNames` table omits the dash for about half its entries, which would miss for MM8.
        std::string portraitName = gameType() == GAME_TYPE_MM8
            ? fmt::format("pc{:02}-01", uX + 1)
            : fmt::format("{}01", pPlayerPortraitsNames[uX]);
        ui_partycreation_portraits[uX] = assets->getImage_ColorKey(portraitName);
    }

    // Doll-preview panel parts, MM8-only - see `DrawMm8Doll`. Loaded via getImage_Alpha (not getImage_ColorKey),
    // matching how MM7's per-face paperdoll sprites of the same `pcNNbod` naming are loaded in UICharacter.cpp's WetsuitOff.
    if (gameType() == GAME_TYPE_MM8) {
        ui_partycreation_doll_backdrop = getImage_SolidIfExists("backdoll");
        for (int uX = 0; uX < 22; ++uX) {
            ui_partycreation_doll_body[uX] = getImage_AlphaIfExists(fmt::format("pc{:02}bod", uX + 1));
            ui_partycreation_doll_lhand[uX] = getImage_AlphaIfExists(fmt::format("pc{:02}lhd", uX + 1));
            ui_partycreation_doll_rhand[uX] = getImage_AlphaIfExists(fmt::format("pc{:02}rhd", uX + 1));
        }
        // See `kMm8PortraitRingBox`'s comment.
        ui_partycreation_portrait_ring = getImage_ColorKeyIfExists("selring");
    }

    ui_partycreation_minus = getImage_ColorKeyIfExists("buttminu");
    ui_partycreation_plus = getImage_ColorKeyIfExists("buttplus");
    // MM7's `presleft`/`presrigh` cycle-arrow icons aren't shipped in an MM8 install's icons.lod. `ar_lt_up`/`ar_rt_up`
    // are MM8's own left/right arrow icons, used for both the Portrait/Voice cycle arrows and the Doll-preview's
    // race-cycling arrows.
    ui_partycreation_right = getImage_ColorKeyIfExists(gameType() == GAME_TYPE_MM8 ? "ar_rt_up" : "presrigh");
    ui_partycreation_left = getImage_ColorKeyIfExists(gameType() == GAME_TYPE_MM8 ? "ar_lt_up" : "presleft");
    // MM8-only, see `MaskToPlainTriangle`'s comment. MM7's `presleft`/`presrigh` are already borderless and stay untouched.
    if (gameType() == GAME_TYPE_MM8) {
        MaskToPlainTriangle(ui_partycreation_right);
        MaskToPlainTriangle(ui_partycreation_left);
    }

    // sprites number go from (1 to 19)
    assert(ui_partycreation_arrow_l.size() == 19);
    assert(ui_partycreation_arrow_r.size() == 19);
    for (int i = 0; i < ui_partycreation_arrow_l.size(); ++i) {
        ui_partycreation_arrow_l[i] = getImage_AlphaIfExists(fmt::format("arrowl{}", i + 1));
        ui_partycreation_arrow_r[i] = getImage_AlphaIfExists(fmt::format("arrowr{}", i + 1));
    }

    if (gameType() == GAME_TYPE_MM8) {
        ConstructMm8Controls();
        ui_partycreation_font = GUIFont::LoadFont("cchar.fnt");
        return;
    }

    int uX = 8;
    for (int characterIndex = 0; characterIndex < 4; characterIndex++) {
        CreateButton({uX, 120}, {145, 25}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationChangeName, characterIndex);
        uX += 158;
    }

    pCreationUI_BtnPressLeft[0] = CreateButton({10, 32}, {11, 13}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_FacePrev, 0, INPUT_ACTION_INVALID, "", {ui_partycreation_left});
    pCreationUI_BtnPressLeft[1] = CreateButton({169, 32}, {11, 13}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_FacePrev, 1, INPUT_ACTION_INVALID, "", {ui_partycreation_left});
    pCreationUI_BtnPressLeft[2] = CreateButton({327, 32}, {11, 13}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_FacePrev, 2, INPUT_ACTION_INVALID, "", {ui_partycreation_left});
    pCreationUI_BtnPressLeft[3] = CreateButton({486, 32}, {11, 13}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_FacePrev, 3, INPUT_ACTION_INVALID, "", {ui_partycreation_left});

    pCreationUI_BtnPressRight[0] = CreateButton({74, 32}, {11, 13}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_FaceNext, 0, INPUT_ACTION_INVALID, "", {ui_partycreation_right});
    pCreationUI_BtnPressRight[1] = CreateButton({233, 32}, {11, 13}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_FaceNext, 1, INPUT_ACTION_INVALID, "", {ui_partycreation_right});
    pCreationUI_BtnPressRight[2] = CreateButton({391, 32}, {11, 13}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_FaceNext, 2, INPUT_ACTION_INVALID, "", {ui_partycreation_right});
    pCreationUI_BtnPressRight[3] = CreateButton({549, 32}, {11, 13}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_FaceNext, 3, INPUT_ACTION_INVALID, "", {ui_partycreation_right});

    pCreationUI_BtnPressLeft2[0] = CreateButton({10, 103}, {11, 13}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_VoicePrev, 0, INPUT_ACTION_INVALID, "", {ui_partycreation_left});
    pCreationUI_BtnPressLeft2[1] = CreateButton({169, 103}, {11, 13}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_VoicePrev, 1, INPUT_ACTION_INVALID, "", {ui_partycreation_left});
    pCreationUI_BtnPressLeft2[2] = CreateButton({327, 103}, {11, 13}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_VoicePrev, 2, INPUT_ACTION_INVALID, "", {ui_partycreation_left});
    pCreationUI_BtnPressLeft2[3] = CreateButton({486, 103}, {11, 13}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_VoicePrev, 3, INPUT_ACTION_INVALID, "", {ui_partycreation_left});

    pCreationUI_BtnPressRight2[0] = CreateButton({74, 103}, {11, 13}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_VoiceNext, 0, INPUT_ACTION_INVALID, "", {ui_partycreation_right});
    pCreationUI_BtnPressRight2[1] = CreateButton({233, 103}, {11, 13}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_VoiceNext, 1, INPUT_ACTION_INVALID, "", {ui_partycreation_right});
    pCreationUI_BtnPressRight2[2] = CreateButton({391, 103}, {11, 13}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_VoiceNext, 2, INPUT_ACTION_INVALID, "", {ui_partycreation_right});
    pCreationUI_BtnPressRight2[3] = CreateButton({549, 103}, {11, 13}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_VoiceNext, 3, INPUT_ACTION_INVALID, "", {ui_partycreation_right});

    uX = 8;
    for (int characterIndex = 0 ; characterIndex < 4; characterIndex++) {
        CreateButton({uX, 308}, {150, v0}, BUTTON_TYPE_NORMAL, 0, UIMSG_48, characterIndex);
        CreateButton({uX, v0 + 308}, {150, v0}, BUTTON_TYPE_NORMAL, 0, UIMSG_49, characterIndex);
        CreateButton(fmt::format("PartyCreation_RemoveSkill3_{}", characterIndex), {uX, 2 * v0 + 308}, {150, v0}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationRemoveUpSkill, characterIndex);
        CreateButton(fmt::format("PartyCreation_RemoveSkill4_{}", characterIndex), {uX, 3 * v0 + 308}, {150, v0}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationRemoveDownSkill, characterIndex);
        uX += 158;
    }

    CreateButton({5, 21}, {153, 365}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_SelectAttribute, 0, INPUT_ACTION_SELECT_CHAR_1);
    CreateButton({163, 21}, {153, 365}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_SelectAttribute, 1, INPUT_ACTION_SELECT_CHAR_2);
    CreateButton({321, 21}, {153, 365}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_SelectAttribute, 2, INPUT_ACTION_SELECT_CHAR_3);
    CreateButton({479, 21}, {153, 365}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_SelectAttribute, 3, INPUT_ACTION_SELECT_CHAR_4);

    uX = 23;
    int uControlParam = 2;
    do {
        CreateButton({uX, 169}, {120, 20}, BUTTON_TYPE_NORMAL, 0, UIMSG_0, uControlParam - 2);
        CreateButton({uX, v0 + 169}, {120, 20}, BUTTON_TYPE_NORMAL, 0, UIMSG_0, uControlParam - 1);
        CreateButton({uX, 2 * v0 + 169}, {120, 20}, BUTTON_TYPE_NORMAL, 0, UIMSG_0, uControlParam);
        CreateButton({uX, 3 * v0 + 169}, {120, 20}, BUTTON_TYPE_NORMAL, 0, UIMSG_0, uControlParam + 1);
        CreateButton({uX, 4 * v0 + 169}, {120, 20}, BUTTON_TYPE_NORMAL, 0, UIMSG_0, uControlParam + 2);
        CreateButton({uX, 5 * v0 + 169}, {120, 20}, BUTTON_TYPE_NORMAL, 0, UIMSG_0, uControlParam + 3);
        CreateButton({uX, 6 * v0 + 169}, {120, 20}, BUTTON_TYPE_NORMAL, 0, UIMSG_0, uControlParam + 4);

        uControlParam += 7;
        uX += 158;
    } while ((signed int)uControlParam < 30);

    setKeyboardControlGroup(28, true, 7, 40);

    CreateButton({323, 417}, {65, v0}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationSelectClass, 0);
    CreateButton({323, v0 + 417}, {65, v0}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationSelectClass, 0xC);
    CreateButton({323, 2 * v0 + 417}, {65, v0}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationSelectClass, 0x14);
    CreateButton({388, 417}, {65, v0}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationSelectClass, 0x18);
    CreateButton({388, v0 + 417}, {65, v0}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationSelectClass, 0x1C);
    CreateButton({388, 2 * v0 + 417}, {65, v0}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationSelectClass, 0x20);
    CreateButton({453, 417}, {65, v0}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationSelectClass, 0x10);
    CreateButton({453, v0 + 417}, {65, v0}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationSelectClass, 8);
    CreateButton({453, 2 * v0 + 417}, {65, v0}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationSelectClass, 4);

    uControlParam = 0;
    do {
        uX = -5;
        if (uControlParam <= 3)
            uX = 0;
        CreateButton({100 * (uControlParam / 3) + uX + 17, v0 * (uControlParam % 3) + 417}, {100, v0}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationSelectActiveSkill, uControlParam);
        ++uControlParam;
    } while (uControlParam < 9);

    ui_partycreation_buttmake = getImage_SolidIfExists("BUTTMAKE");
    ui_partycreation_buttmake2 = getImage_SolidIfExists("BUTTMAKE2");

    pPlayerCreationUI_BtnOK = CreateButton("PartyCreation_OK", {580, 431}, {51, 39}, BUTTON_TYPE_NORMAL, 0,
                                           UIMSG_PlayerCreationClickOK, 0, INPUT_ACTION_PARTY_CREATION_DONE, "",
                                           {ui_partycreation_buttmake});
    pPlayerCreationUI_BtnReset = CreateButton("PartyCreation_Clear", {527, 431}, {51, 39}, BUTTON_TYPE_NORMAL, 0,
                                              UIMSG_PlayerCreationClickReset, 0, INPUT_ACTION_PARTY_CREATION_CLEAR, "",
                                              {ui_partycreation_buttmake2});
    pPlayerCreationUI_BtnMinus = CreateButton({523, 393}, {20, 35}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationClickMinus, 0, INPUT_ACTION_PARTY_CREATION_DEC, "", {ui_partycreation_minus});
    pPlayerCreationUI_BtnPlus = CreateButton({613, 393}, {20, 35}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationClickPlus, 1, INPUT_ACTION_PARTY_CREATION_INC, "", {ui_partycreation_plus});

    ui_partycreation_font = GUIFont::LoadFont("cchar.fnt");
}

GUIWindow_PartyCreation::~GUIWindow_PartyCreation() {
    assets->evict(std::move(main_menu_background));
}

// Controls for MM8's single-champion screen. Mirrors the constructor's MM7 button-creation code above, just for one
// character instead of 4, laid out per the `kMm8*` layout constants near the top of this file.
void GUIWindow_PartyCreation::ConstructMm8Controls() {
    CreateButton({kMm8NameBox.x, kMm8NameBox.y}, {145, 25}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationChangeName, 0);

    // Portrait cycle arrows double as MM8's race/class picker (see the `kMm8DollPreviewBox` comment
    // at the top of this file) - given ids so GAME_TESTs can drive them directly, same as the other
    // "PartyCreation_*" buttons below.
    pCreationUI_BtnPressLeft[0] = CreateButton("PartyCreation_PortraitPrev", kMm8FacePrevBtn.topLeft(), kMm8FacePrevBtn.size(), BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_FacePrev, 0,
        INPUT_ACTION_INVALID, "", {ui_partycreation_left});
    pCreationUI_BtnPressRight[0] = CreateButton("PartyCreation_PortraitNext", kMm8FaceNextBtn.topLeft(), kMm8FaceNextBtn.size(), BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_FaceNext, 0,
        INPUT_ACTION_INVALID, "", {ui_partycreation_right});
    pCreationUI_BtnPressLeft2[0] = CreateButton("PartyCreation_VoicePrev", kMm8VoicePrevBtn.topLeft(), kMm8VoicePrevBtn.size(), BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_VoicePrev, 0,
        INPUT_ACTION_INVALID, "", {ui_partycreation_left});
    pCreationUI_BtnPressRight2[0] = CreateButton("PartyCreation_VoiceNext", kMm8VoiceNextBtn.topLeft(), kMm8VoiceNextBtn.size(), BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_VoiceNext, 0,
        INPUT_ACTION_INVALID, "", {ui_partycreation_right});

    ui_partycreation_default = getImage_ColorKeyFromEnglishDLODIfExists("c_dft_up");
    pCreationUI_BtnVoiceDefault = CreateButton("PartyCreation_VoiceDefault", kMm8DefaultBtn.topLeft(), kMm8DefaultBtn.size(), BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationVoiceDefaultMm8, 0,
        INPUT_ACTION_INVALID, "", {ui_partycreation_default});

    // Chosen-skills 2x2 grid hit-boxes, matching `UpdateMm8()`'s drawing (`kMm8SkillRowGap`, the real screen's wider row spacing).
    CreateButton({kMm8SkillColX[0], kMm8SkillsBox.y}, {kMm8SkillColX[1] - kMm8SkillColX[0], kMm8SkillRowGap}, BUTTON_TYPE_NORMAL, 0, UIMSG_48, 0);
    CreateButton({kMm8SkillColX[1], kMm8SkillsBox.y}, {kMm8SkillColX[1] - kMm8SkillColX[0], kMm8SkillRowGap}, BUTTON_TYPE_NORMAL, 0, UIMSG_49, 0);
    CreateButton("PartyCreation_RemoveSkill3", {kMm8SkillColX[0], kMm8SkillsBox.y + kMm8SkillRowGap}, {kMm8SkillColX[1] - kMm8SkillColX[0], kMm8SkillRowGap},
        BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationRemoveUpSkill, 0);
    CreateButton("PartyCreation_RemoveSkill4", {kMm8SkillColX[1], kMm8SkillsBox.y + kMm8SkillRowGap}, {kMm8SkillColX[1] - kMm8SkillColX[0], kMm8SkillRowGap},
        BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationRemoveDownSkill, 0);

    // Per-row [-]/[+] spinner icons, matching `UpdateMm8()`'s drawing - each acts directly on its own
    // attribute (`param` = attribute index). Unlike MM7's single shared pPlayerCreationUI_BtnPlus/Minus
    // pair, there's no "currently selected row" concept here, so (unlike the MM7 code above) this
    // doesn't create a whole-row select hitbox/keyboard-group first - one would just sit underneath
    // and overlap these icons, which real MM8 doesn't have either (no separate row-select control).
    ui_partycreation_attr_minus = getImage_ColorKeyIfExists("cminup");
    ui_partycreation_attr_plus = getImage_ColorKeyIfExists("cplusup");
    for (int i = 0; i < 7; ++i) {
        int rowY = kMm8AttributesFirstRowY + i * kMm8AttributesRowGap;
        pCreationUI_BtnAttrMinus[i] = CreateButton(fmt::format("PartyCreation_AttrMinus_{}", i), {kMm8AttrMinusX, rowY}, {kMm8AttrSpinnerSize, kMm8AttrSpinnerSize},
            BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationClickMinusMm8, i, INPUT_ACTION_INVALID, "", {ui_partycreation_attr_minus});
        pCreationUI_BtnAttrPlus[i] = CreateButton(fmt::format("PartyCreation_AttrPlus_{}", i), {kMm8AttrPlusX, rowY}, {kMm8AttrSpinnerSize, kMm8AttrSpinnerSize},
            BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationClickPlusMm8, i, INPUT_ACTION_INVALID, "", {ui_partycreation_attr_plus});
    }

    for (int i = 0; i < 9; ++i) {
        Pointi pos = {kMm8SkillColX[i % 2], kMm8AvailableSkillsBox.y + (i / 2) * kMm8SkillRowGap};
        CreateButton(pos, {kMm8SkillColX[1] - kMm8SkillColX[0], kMm8SkillRowGap}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationSelectActiveSkill, i);
    }

    // MM8's own Clear/Cancel/OK button art. `BUTTMAKE`/`BUTTMAKE2` (MM7's) don't exist in an MM8 install's LODs.
    // `c_clr_*`/`c_cncl_*`/`c_ok_*` are this screen's real assets (EnglishD.lod).
    ui_partycreation_clear_mm8 = getImage_ColorKeyFromEnglishDLODIfExists("c_clr_up");
    ui_partycreation_cancel_mm8 = getImage_ColorKeyFromEnglishDLODIfExists("c_cncl_up");
    ui_partycreation_ok_mm8 = getImage_ColorKeyFromEnglishDLODIfExists("c_ok_up");

    pPlayerCreationUI_BtnOK = CreateButton("PartyCreation_OK", kMm8OkBtn.topLeft(), kMm8OkBtn.size(),
        BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationClickOK, 0, INPUT_ACTION_PARTY_CREATION_DONE, "", {ui_partycreation_ok_mm8});
    pPlayerCreationUI_BtnReset = CreateButton("PartyCreation_Clear", kMm8ClearBtn.topLeft(), kMm8ClearBtn.size(),
        BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationClickReset, 0, INPUT_ACTION_PARTY_CREATION_CLEAR, "", {ui_partycreation_clear_mm8});
    pCreationUI_BtnCancelMm8 = CreateButton("PartyCreation_Cancel", kMm8CancelBtn.topLeft(), kMm8CancelBtn.size(),
        BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationClickCancelMm8, 0, INPUT_ACTION_INVALID, "", {ui_partycreation_cancel_mm8});
    // No shared corner +/- pair here (unlike MM7's `pPlayerCreationUI_BtnPlus/Minus`, built below for
    // MM7 only) - the per-row spinners above are MM8's only stat-adjustment control, matching the real
    // screen, which has no such corner control either.
}

//----- (00497526) --------------------------------------------------------
bool PartyCreationUI_LoopInternal() {
    Item item;
    bool party_not_creation_flag;

    party_not_creation_flag = false;

    pGUIWindow_CurrentMenu->keyboard_input_status = WINDOW_INPUT_NONE;
    SetCurrentMenuID(MENU_CREATEPARTY);
    while (GetCurrentMenuID() == MENU_CREATEPARTY) {
        MessageLoopWithWait();

        animTimer->tick(); // This one is used for animations.

        // PlayerCreationUI_Draw();
        // MainMenu_EventLoop();
        CreateParty_EventLoop();
        render->BeginScene2D();
        GUI_UpdateWindows();
        render->Present();
        if (uGameState ==
            GAME_FINISHED) {  // if click Esc in PlayerCreation Window
            party_not_creation_flag = true;
            SetCurrentMenuID(MENU_MAIN);
            continue;
        }
        if (uGameState ==
            GAME_STATE_STARTING_NEW_GAME) {  // if click OK in PlayerCreation
                                             // Window
            uGameState = GAME_STATE_PLAYING;
            SetCurrentMenuID(MENU_NEWGAME);
            continue;
        }
    }

    pGUIWindow_CurrentMenu = nullptr;

    item.Reset();
    // Deliberately a literal 4, not activeCharacterCount(): each iteration consumes RNG (`generateItem()`), and
    // bounding the loop to the solo MM8 champion shifts every later grng draw for the session. Slots 1-3 are
    // unrecruited MM8 placeholders with no on-screen representation. `enforcePlaceholderInertness()` after the loop
    // undoes the health/mana/item writes this loop makes to those slots.
    for (unsigned i = 0; i < 4; ++i) {
        if (pParty->pCharacters[i].classType == CLASS_KNIGHT)
            pParty->pCharacters[i].sResMagicBase = 10;
        // TODO(pskelton): why just CHARACTER_BUFF_RESIST_WATER?
        pParty->pCharacters[i].pCharacterBuffs[CHARACTER_BUFF_RESIST_WATER].Reset();
        for (MagicSchool page : allMagicSchools()) {
            if (pParty->pCharacters[i].pActiveSkills[skillForMagicSchool(page)]) {
                pParty->pCharacters[i].lastOpenedSpellbookPage = page;
                break;
            }
        }
        pItemTable->generateItem(ITEM_TREASURE_LEVEL_2, RANDOM_ITEM_RING, &item);
        pParty->pCharacters[i].inventory.add(item);

        pParty->pCharacters[i].health = pParty->pCharacters[i].GetMaxHealth();
        pParty->pCharacters[i].mana = pParty->pCharacters[i].GetMaxMana();
        for (Skill j : allSkills()) {
            if (!pParty->pCharacters[i].pActiveSkills[j]) continue;

            switch (j) {
            // The `ItemId` enum's named weapon/armor constants are autogenerated from whichever game's data was loaded
            // (see `ItemEnums.h`), so a hardcoded id like `ITEM_DAGGER` can resolve to a different real item under
            // another game's `items.txt` (MM8's ITEM_DAGGER is an endgame "Archangel Broadsword"). Look the item up by
            // its own `skill` data field instead, which is correct for whichever game is loaded.
            case SKILL_STAFF:
            case SKILL_SWORD:
            case SKILL_DAGGER:
            case SKILL_AXE:
            case SKILL_SPEAR:
            case SKILL_BOW:
            case SKILL_MACE:
            case SKILL_SHIELD:
            case SKILL_LEATHER:
            case SKILL_CHAIN:
            case SKILL_PLATE:
                pParty->pCharacters[i].inventory.add(Item(pItemTable->startingItemForSkill(j)));
                break;
            case SKILL_BLASTER:
                MM_ERROR("No blasters at startup :p");
                break;
            case SKILL_FIRE:
                pParty->pCharacters[i].inventory.add(Item(ITEM_SPELLBOOK_FIRE_BOLT));
                pParty->pCharacters[i].bHaveSpell[SPELL_FIRE_TORCH_LIGHT] = true;
                break;
            case SKILL_AIR:
                pParty->pCharacters[i].inventory.add(Item(ITEM_SPELLBOOK_FEATHER_FALL));
                pParty->pCharacters[i].bHaveSpell[SPELL_AIR_WIZARD_EYE] = true;
                break;
            case SKILL_WATER:
                pParty->pCharacters[i].inventory.add(Item(ITEM_SPELLBOOK_POISON_SPRAY));
                pParty->pCharacters[i].bHaveSpell[SPELL_WATER_AWAKEN] = true;
                break;
            case SKILL_EARTH:
                pParty->pCharacters[i].inventory.add(Item(ITEM_SPELLBOOK_SLOW));
                pParty->pCharacters[i].bHaveSpell[SPELL_EARTH_STUN] = true;
                break;
            case SKILL_SPIRIT:
                pParty->pCharacters[i].inventory.add(Item(ITEM_SPELLBOOK_BLESS));
                pParty->pCharacters[i].bHaveSpell[SPELL_SPIRIT_DETECT_LIFE] = true;
                break;
            case SKILL_MIND:
                pParty->pCharacters[i].inventory.add(Item(ITEM_SPELLBOOK_MIND_BLAST));
                pParty->pCharacters[i].bHaveSpell[SPELL_MIND_REMOVE_FEAR] = true;
                break;
            case SKILL_BODY:
                pParty->pCharacters[i].inventory.add(Item(ITEM_SPELLBOOK_HEAL));
                pParty->pCharacters[i].bHaveSpell[SPELL_BODY_CURE_WEAKNESS] = true;
                break;
            case SKILL_LIGHT:
                pParty->pCharacters[i].inventory.add(Item(ITEM_SPELLBOOK_DESTROY_UNDEAD));
                pParty->pCharacters[i].bHaveSpell[SPELL_LIGHT_LIGHT_BOLT] = true;
                break;
            case SKILL_DARK:
                pParty->pCharacters[i].inventory.add(Item(ITEM_SPELLBOOK_TOXIC_CLOUD));
                pParty->pCharacters[i].bHaveSpell[SPELL_DARK_REANIMATE] = true;
                break;
            case SKILL_DIPLOMACY:
                MM_ERROR("No diplomacy in mm7 (yet)");
                break;
            case SKILL_ITEM_ID:
            case SKILL_REPAIR:
            case SKILL_MEDITATION:
            case SKILL_PERCEPTION:
            case SKILL_TRAP_DISARM:
            case SKILL_LEARNING:
                pParty->pCharacters[i].inventory.add(Item(ITEM_POTION_BOTTLE));
                pParty->pCharacters[i].inventory.add(Item(grng->randomSample(allLevel1Reagents())));
                break;
            case SKILL_DODGE:
            case SKILL_UNARMED:
                pParty->pCharacters[i].inventory.add(Item(pItemTable->startingItemForSkill(j)));
                break;
            case SKILL_CLUB:
                // pParty->pCharacters[i].inventory.add(Item(ITEM_CLUB));
                break;
            default:
                break;
            }

            for (InventoryEntry entry : pParty->pCharacters[i].inventory.entries())
                entry->SetIdentified();
        }
    }
    // Undoes the healing this loop just did to any MM8 placeholder slot above. Re-enforces the inert-by-construction
    // invariant `Reset()`/`createDefaultParty()` already established earlier in this character-creation flow.
    pParty->enforcePlaceholderInertness();

    pAudioPlayer->stopSounds();
    return party_not_creation_flag;
}
