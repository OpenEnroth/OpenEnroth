#pragma once

#include <string>

#include "Library/Geometry/Point.h"

void DrawPopupWindow(int uX, int uY, int uWidth, int uHeight);  // idb

/**
 * If `mousePos` is over a character portrait, uses the picked item on that character and return true. Note that using
 * an item can either consume that item, or display an error string (E.g. "Crossbow can not be used that way").
 *
 * @param mousePos                  Mouse position in 640x480 UI coordinates.
 * @return                          Whether the picked item was consumed.
 */
bool tryUseItemOnPortrait(Pointi mousePos);

/**
 * Renders the right-click popup for the current screen (spell description, monster info, item
 * info, character stats hint, etc.) once per frame. Must be called late in the GUI pass so the
 * popup ends up on top of every window's `Update()` output.
 *
 * Does only rendering, logic on how right-click is handled lives in the press-event handler
 * (`GameWindowHandler::OnMouseRightClick`).
 *
 * @param mousePos                  Mouse position in 640x480 UI coordinates.
 * @offset 0x00416D62
 */
void UI_OnMouseRightClick(Pointi mousePos);

class GraphicsImage;

extern GraphicsImage *parchment;
extern GraphicsImage *messagebox_corner_x;       // 5076AC
extern GraphicsImage *messagebox_corner_y;       // 5076B4
extern GraphicsImage *messagebox_corner_z;       // 5076A8
extern GraphicsImage *messagebox_corner_w;       // 5076B0
extern GraphicsImage *messagebox_border_top;     // 507698
extern GraphicsImage *messagebox_border_bottom;  // 5076A4
extern GraphicsImage *messagebox_border_left;    // 50769C
extern GraphicsImage *messagebox_border_right;   // 5076A0

extern bool holdingMouseRightButton;
extern bool rightClickItemActionPerformed;
extern bool identifyOrRepairReactionPlayed;
extern bool monsterIdReactionPlayed;
