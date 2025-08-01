#include "ItemGrid.h"

#include <cassert>

#include "Utility/Math/IntDiv.h"

int itemOffset(int dimension) {
    if (dimension < 14)
        dimension = 14;

    int offset = (roundToNegInf(dimension - 14, 32) + 32 - dimension) / 2;

    assert(-6 <= offset && offset <= 9);
    return offset;
}

Pointi mapToInventoryGrid(Pointi mousePos, Pointi inventoryTopLeft) {
    Pointi relativePos = mousePos - inventoryTopLeft;
    return Pointi(divToNegInf(relativePos.x, 32), divToNegInf(relativePos.y, 32));
}

Pointi mapFromInventoryGrid(Pointi gridPos, Pointi inventoryTopLeft) {
    return gridPos * 32 + inventoryTopLeft;
}
