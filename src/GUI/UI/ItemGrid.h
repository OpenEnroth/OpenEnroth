#pragma once

#include "Library/Geometry/Point.h"

/**
 * @brief Calculate offsets for placing items in inventory grid.
 *
 * Inventory grid have 32x32 pixels cells. Items don't exactly fit into the grid, but are slightly smaller or larger.
 * The function calculates offsets for the item to be centered. Calculations are the same for height and width.
 *
 * @param dimension                     Width or height of the item.
 * @return                              Offset in pixels.
 */
int itemOffset(int dimension);

/**
 * @param mousePos                      Mouse position.
 * @param inventoryTopLeft              Top left position of the inventory grid.
 * @return                              Inventory grid position, in grid coordinates. Might be out of grid if the mouse
 *                                      position is out of grid.
 */
Pointi mapToInventoryGrid(Pointi mousePos, Pointi inventoryTopLeft);
