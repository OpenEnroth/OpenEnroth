#include "Engine/Tables/OverlayTable.h"

#include "Engine/Graphics/Sprites.h"

OverlayTable *pOverlayTable = new OverlayTable;

//----- (00458D97) --------------------------------------------------------
void OverlayTable::initializeSprites() {
    for (size_t i = 0; i < overlays.size(); ++i)
        pSpriteFrameTable->InitializeSprite(overlays[i].uSpriteFramesetID);
}
