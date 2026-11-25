#pragma once

#include <memory>
#include <string>
#include <vector>

#include "GUI/GUIWindow.h"
#include "Library/Geometry/Rect.h"

#include "Utility/Ptr.h"

class GraphicsImage;
class Character;

bool PlayerCreation_Choose4Skills();
bool PartyCreationUI_Loop();

// Composites MM8's "Doll preview" panel (static `backdoll` background plus the
// character's body/hand paperdoll sprites). Shared with the character-record screen.
void DrawMm8Doll(const Character &character, const Recti &box);

// Which equipment layer this is - lets a caller (the draw path, or a test) pick out the
// armor vs. weapon layer without depending on vector order.
enum class Mm8DollLayerKind { Armor, Weapon };

// One equipment layer `DrawMm8Doll` composites on top of the base body/hand art -
// `assetName` is the resolved LOD entry it draws (an `item{:03}vN` doll-scale sprite for armor, or
// the equipped item's own inventory icon (`GetIconName()`) for a weapon, MM8
// weapons have no doll-scale sprites), `dstRect` is where it lands on screen.
// `itemIndex` is the SAME `InventoryConstEntry::index()` `OnPaperdollLeftClick`/
// `UIPopup.cpp`'s doll tooltip read back out of `equipmentHitMap`.
struct Mm8DollEquipmentLayer {
    Mm8DollLayerKind kind;
    std::string assetName;
    Recti dstRect;
    int itemIndex = -1;
};

// `DrawMm8Doll`'s per-layer equipment-compositing decisions (which asset, at what
// rect), as a pure query so a headless test can assert the composition structurally
// - does the right asset resolve, does it land inside the doll box, is the weapon anchored to the
// hand's grip point - without a GL framebuffer. `body`/`bodyPos`
// are the base body sprite and its screen position. The armor layer's anchor is derived from the
// body sprite's size. `rhand`/`rhandPos` are the right-hand sprite and its
// already-computed screen position, or `nullptr`/default if there is no rhand art for this
// character's portrait. Resolves asset names against the currently loaded LODs but draws nothing
// itself.
std::vector<Mm8DollEquipmentLayer> ComputeMm8DollEquipmentLayers(const Character &character, Ptr<GraphicsImage> body, Pointi bodyPos, Ptr<GraphicsImage> rhand, Pointi rhandPos);

// The base body/rhand screen positions `ComputeMm8DollEquipmentLayers` above needs as
// input, for the same portrait/box `DrawMm8Doll` would use.
// `body`/`rhand` are `nullptr` if this portrait has no doll art at all.
struct Mm8DollBodyLayout {
    std::shared_ptr<GraphicsImage> body;
    Pointi bodyPos;
    std::shared_ptr<GraphicsImage> rhand;
    Pointi rhandPos;
};
Mm8DollBodyLayout ComputeMm8DollBodyLayout(const Character &character, const Recti &box);

// rhand's "grip" landmark that the weapon layer above anchors to. Exposed for tests.
Pointi FindHandGripPoint(Ptr<GraphicsImage> hand);

class GUIWindow_PartyCreation : public GUIWindow {
 public:
    GUIWindow_PartyCreation();
    virtual ~GUIWindow_PartyCreation();

    virtual void Update() override;

 protected:
    std::shared_ptr<GraphicsImage> main_menu_background;
    std::unique_ptr<GUIFont> ui_partycreation_font;

 private:
    // MM8's native single-champion "Create Character" screen - see UIPartyCreation.cpp
    // for why this needs its own layout instead of reusing MM7's 4-portrait one.
    void ConstructMm8Controls();
    void UpdateMm8();
};
