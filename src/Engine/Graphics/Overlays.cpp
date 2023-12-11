#include "Engine/Graphics/Overlays.h"

#include "Engine/Party.h"
#include "Engine/Time/Timer.h"
#include "Engine/mm7_data.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/TurnEngine/TurnEngine.h"

#include "GUI/GUIWindow.h"

#include "Sprites.h"


struct ActiveOverlayList *pActiveOverlayList = new ActiveOverlayList;  // idb
struct OverlayList *pOverlayList = new OverlayList;

// inlined
//----- (mm6c::0045BD50) --------------------------------------------------
void ActiveOverlayList::Reset() {
    for (unsigned int i = 0; i < 50; ++i) pOverlays[i].Reset();
}

//----- (004418B6) --------------------------------------------------------
int ActiveOverlayList::_4418B6(int uOverlayID, Pid pid, int animLength, int fpDamageMod, int16_t projSize) {
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
                v11 = Duration::fromTicks(animLength);
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

//----- (00441964) --------------------------------------------------------
void ActiveOverlayList::DrawTurnBasedIcon() {
    Icon *frame = nullptr;      // eax@12
    unsigned int v5;  // [sp-8h] [bp-Ch]@4

    if (current_screen_type != SCREEN_GAME || !pParty->bTurnBasedModeOn) return;

    if (pTurnEngine->turn_stage == TE_MOVEMENT) {  // все персы отстрелялись(сжатый кулак)
        frame = pIconsFrameTable->GetFrame(
            pIconIDs_Turn[5 - pTurnEngine->uActionPointsLeft / 26],
            Duration::fromTicks(pEventTimer->uStartTime));
    } else if (pTurnEngine->turn_stage == TE_WAIT) {
        if (dword_50C998_turnbased_icon_1A)
            v5 = uIconID_TurnStart;  //анимация руки(запуск пошагового режима)
        else
            v5 = uIconID_TurnHour;  //группа ожидает(часы)
        frame = pIconsFrameTable->GetFrame(v5, dword_50C994);
    } else if (pTurnEngine->turn_stage == TE_ATTACK) {  //группа атакует(ладонь)
        frame = pIconsFrameTable->GetFrame(uIconID_TurnStop,
            Duration::fromTicks(pEventTimer->uStartTime));
    } else {
        assert(false);
        return;
    }
    // if ( render->pRenderD3D )
    render->DrawTextureNew(394 / 640.0f, 288 / 480.0f,
                                frame->GetTexture());
    /*else
      render->DrawTextureIndexedAlpha(0x18Au, 0x120u, v7);*/
    if (dword_50C994 < dword_50C998_turnbased_icon_1A) {
        dword_50C994 += Duration::fromTicks(pEventTimer->uTimeElapsed);
        if (dword_50C994 >= dword_50C998_turnbased_icon_1A)
            dword_50C998_turnbased_icon_1A = Duration::zero();
    }
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
