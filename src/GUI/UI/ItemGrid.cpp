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

    assert(-6 <= offset && offset <= 9);
    return offset;
}

Pointi mapToInventoryGrid(Pointi mousePos, Pointi inventoryTopLeft) {
    Pointi relativePos = mousePos - inventoryTopLeft;
    return Pointi(divIntDown(relativePos.x, 32), divIntDown(relativePos.y, 32));
}
