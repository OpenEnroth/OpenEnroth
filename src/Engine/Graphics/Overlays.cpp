#include "Engine/Graphics/Overlays.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>

#include "Engine/Party.h"
#include "Engine/Time.h"
#include "Engine/mm7_data.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Tables/FrameTableInc.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/TurnEngine/TurnEngine.h"
#include "GUI/GUIWindow.h"
#include "Utility/String.h"
#include "Sprites.h"
#include "Engine/ErrorHandling.h"
#include "GUI/GUIEnums.h"


struct ActiveOverlayList *pActiveOverlayList = new ActiveOverlayList;  // idb
struct OverlayList *pOverlayList = new OverlayList;

// inlined
//----- (mm6c::0045BD50) --------------------------------------------------
void ActiveOverlayList::Reset() {
    for (unsigned int i = 0; i < 50; ++i) pOverlays[i].Reset();
}

//----- (004418B6) --------------------------------------------------------
int ActiveOverlayList::_4418B6(int uOverlayID, Pid pid, int animLength, int fpDamageMod, int16_t projSize) {
    int16_t v11;    // dx@11

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
                v11 = 8 * pSpriteFrameTable->pSpriteSFrames[pOverlayList->pOverlays[indexer].uSpriteFramesetID].uAnimLength;
            this->pOverlays[i].animLength = v11;
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
            pEventTimer->uStartTime);
    } else if (pTurnEngine->turn_stage == TE_WAIT) {
        if (dword_50C998_turnbased_icon_1A)
            v5 = uIconID_TurnStart;  //анимация руки(запуск пошагового режима)
        else
            v5 = uIconID_TurnHour;  //группа ожидает(часы)
        frame = pIconsFrameTable->GetFrame(v5, dword_50C994);
    } else if (pTurnEngine->turn_stage == TE_ATTACK) {  //группа атакует(ладонь)
        frame = pIconsFrameTable->GetFrame(uIconID_TurnStop,
            pEventTimer->uStartTime);
    } else {
        __debugbreak();
        return;
    }
    // if ( render->pRenderD3D )
    render->DrawTextureNew(394 / 640.0f, 288 / 480.0f,
                                frame->GetTexture());
    /*else
      render->DrawTextureIndexedAlpha(0x18Au, 0x120u, v7);*/
    if (dword_50C994 < dword_50C998_turnbased_icon_1A) {
        dword_50C994 += pEventTimer->uTimeElapsed;
        if ((signed int)dword_50C994 >= dword_50C998_turnbased_icon_1A)
            dword_50C998_turnbased_icon_1A = 0;
    }
}

//----- (00458D97) --------------------------------------------------------
void OverlayList::InitializeSprites() {
    for (size_t i = 0; i < pOverlays.size(); ++i)
        pSpriteFrameTable->InitializeSprite(pOverlays[i].uSpriteFramesetID);
}

//----- (00458E4F) --------------------------------------------------------
bool OverlayList::FromFileTxt(const char *Args) {
    FILE *v4;         // eax@1
    unsigned int v5;  // esi@3
    void *v7;         // eax@9
    // FILE *v8; // ST0C_4@11
    char *i;                // eax@11
    char Buf[490];               // [sp+10h] [bp-2F0h]@3
    FrameTableTxtLine v18;  // [sp+204h] [bp-FCh]@4
    FrameTableTxtLine v19;  // [sp+280h] [bp-80h]@4
    FILE *File;             // [sp+2FCh] [bp-4h]@1
    unsigned int Argsa;     // [sp+308h] [bp+8h]@3

    pOverlays.clear();
    v4 = fopen(Args, "r");
    File = v4;
    if (!v4) Error("ObjectDescriptionList::load - Unable to open file: %s.");

    v5 = 0;
    Argsa = 0;
    if (fgets(Buf, sizeof(Buf), v4)) {
        do {
            *strchr(Buf, 10) = 0;
            memcpy(&v19, txt_file_frametable_parser(Buf, &v18), sizeof(v19));
            if (v19.uPropCount && *v19.pProperties[0] != 47) ++Argsa;
        } while (fgets(Buf, sizeof(Buf), File));
        v5 = Argsa;
    }
    pOverlays.reserve(v5);

    fseek(File, 0, 0);
    for (i = fgets(Buf, sizeof(Buf), File); i; i = fgets(Buf, sizeof(Buf), File)) {
        *strchr(Buf, 10) = 0;
        memcpy(&v19, txt_file_frametable_parser(Buf, &v18), sizeof(v19));
        if (v19.uPropCount && *v19.pProperties[0] != 47) {
            OverlayDesc &overlay = pOverlays.emplace_back();

            overlay.uOverlayID = atoi(v19.pProperties[0]);
            if (!iequals(v19.pProperties[1], "center")) {
                if (iequals(v19.pProperties[1], "transparent"))
                    overlay.uOverlayType = 2;
                else
                    overlay.uOverlayType = 1;
            } else {
                overlay.uOverlayType = 0;
            }
            overlay.uSpriteFramesetID = pSpriteFrameTable->FastFindSprite(v19.pProperties[2]);
        }
    }
    fclose(File);
    return 1;
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
