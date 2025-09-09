#include "ItemGrid.h"

#include <cassert>
#include "Engine/Objects/Item.h"
#include "Engine/AssetsManager.h"
#include "Engine/Graphics/Image.h" // Add this include at the top of the file

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

Pointi mapToInventoryGrid(Pointi mousePos, Pointi inventoryTopLeft, Item* heldItem) {
    // check if heldItem is null

    if (heldItem && heldItem->itemId != ITEM_NULL) {
        // If holding an item, we want to snap to the center of the grid cell rather than the edge
        mousePos += Pointi(16, 16);
        // We need to calulate the image width and height and then define the item offset from that
        GraphicsImage *pTexture = assets->getImage_Alpha(heldItem->GetIconName());
        signed int X_offset = itemOffset(pTexture->width());
        signed int Y_offset = itemOffset(pTexture->height());
        mousePos -= Pointi(X_offset, Y_offset);
    }
    Pointi relativePos = mousePos - inventoryTopLeft;
    return Pointi(divIntDown(relativePos.x, 32), divIntDown(relativePos.y, 32));
}

Pointi mapFromInventoryGrid(Pointi gridPos, Pointi inventoryTopLeft) {
    return gridPos * 32 + inventoryTopLeft;
}
