#pragma once

/**
* @brief itemOffset Calculate offsets for placing items on a grid.
* @param dimension width or height of the item
*
* Inventory grid have 32x32 cells.
* Items don't exactly fit into the grid, but are slightly smaller or larger.
* The function calculates offsets for the item to be centered.
* Calculations are the same for length and width.
*
* @return offset in pixels
*/
int itemOffset(int dimension);
