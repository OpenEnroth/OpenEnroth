#include "Engine/Tables/OverlayTable.h"

#include <memory>

#include "Engine/Graphics/Sprites.h"

std::unique_ptr<OverlayTable> pOverlayTable = std::make_unique<OverlayTable>();

//----- (00458D97) --------------------------------------------------------
void OverlayTable::initializeSprites() {
    for (size_t i = 0; i < overlays.size(); ++i)
        pSpriteFrameTable->InitializeSprite(overlays[i].uSpriteFramesetID);
}
