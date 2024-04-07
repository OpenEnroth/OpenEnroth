#include "Engine/Graphics/Overlays.h"

#include "Engine/Party.h"
#include "Engine/Time/Timer.h"
#include "Engine/mm7_data.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/TurnEngine/TurnEngine.h"

#include "GUI/GUIWindow.h"

#include "Sprites.h"


ActiveOverlayList *pActiveOverlayList = new ActiveOverlayList;  // idb
OverlayList *pOverlayList = new OverlayList;

// inlined
//----- (mm6c::0045BD50) --------------------------------------------------
void ActiveOverlayList::Reset() {
    for (unsigned int i = 0; i < 50; ++i) pOverlays[i].Reset();
}

//----- (004418B6) --------------------------------------------------------
int ActiveOverlayList::_4418B6(int uOverlayID, Pid pid, Duration animLength, int fpDamageMod, int16_t projSize) {
    Duration v11;    // dx@11

    for (unsigned int i = 0; i < 50; ++i) {
        if (this->pOverlays[i].animLength <= 0) {
            this->pOverlays[i].screenSpaceY = 0;
            this->pOverlays[i].screenSpaceX = 0;
            this->pOverlays[i].pid = pid;
            int indexer = 0;
            for (; indexer < (signed int)pOverlayList->pOverlays.size(); ++indexer) {
                if (uOverlayID == pOverlayList->pOverlays[indexer].uOverlayID) break;
            }
            this->pOverlays[i].indexToOverlayList = indexer;
            this->pOverlays[i].spriteFrameTime = 0;
            if (animLength)
                v11 = animLength;
            else
                v11 = pSpriteFrameTable->pSpriteSFrames[pOverlayList->pOverlays[indexer].uSpriteFramesetID].uAnimLength;
            this->pOverlays[i].animLength = v11.ticks();
            this->pOverlays[i].fpDamageMod = fpDamageMod;
            this->pOverlays[i].projSize = projSize;
            return true;
        }
    }
    return 0;
}

//----- (00458D97) --------------------------------------------------------
void OverlayList::InitializeSprites() {
    for (size_t i = 0; i < pOverlays.size(); ++i)
        pSpriteFrameTable->InitializeSprite(pOverlays[i].uSpriteFramesetID);
}

//----- (0045855F) --------------------------------------------------------
void ActiveOverlay::Reset() {
    this->indexToOverlayList = 0;
    this->spriteFrameTime = 0;
    this->animLength = 0;
    this->screenSpaceX = 0;
    this->screenSpaceY = 0;
    this->pid = Pid();
    this->projSize = 0;
    this->fpDamageMod = 65536;
}

//----- (004584B8) --------------------------------------------------------
ActiveOverlay::ActiveOverlay() { this->Reset(); }
