#include "ItemGrid.h"

#include <cassert>

static int roundIntDown(int a, int b) {
    return a - a % b;
}

static int divIntDown(int a, int b) {
    return a >= 0 ? a / b : -1 - (-1 - a) / b;
}

int itemOffset(int dimension) {
    if (dimension < 14)
        dimension = 14;

    int offset = (roundIntDown(dimension - 14, 32) + 32 - dimension) / 2;

    assert(-9 <= offset && offset <= 9);
    return offset;
}

Pointi mapToInventoryGrid(Pointi mousePos, Pointi inventoryTopLeft, bool isHoldingItem) {
    if (isHoldingItem) {
        // If holding an item, we want to snap to the center of the grid cell rather than the edge
        mousePos += Pointi(16, 16);
    }
    Pointi relativePos = mousePos - inventoryTopLeft;
    // TODO(captainurist): divIntDown is >> 5
    int x = divIntDown(relativePos.x, 32);
    int y = divIntDown(relativePos.y, 32);
    return Pointi(x, y);
}

Pointi mapFromInventoryGrid(Pointi gridPos, Pointi inventoryTopLeft) {
    return gridPos * 32 + inventoryTopLeft;
}
