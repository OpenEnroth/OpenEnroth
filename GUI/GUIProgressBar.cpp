#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/Party.h"

#include "Engine/Graphics/IRender.h"

#include "Engine/Tables/IconFrameTable.h"

#include "GUI/GUIProgressBar.h"

struct GUIProgressBar *pGameLoadingUI_ProgressBar = new GUIProgressBar;

//----- (00Initialize) --------------------------------------------------------
bool GUIProgressBar::Initialize(Type type) {
    // GUIProgressBar *v2; // esi@1
    signed int v4;  // eax@7
    int v5;         // ecx@8
    // int v6; // edi@8
    int v7;  // edx@14

    switch (type) {
        case TYPE_None:
            return true;

        case TYPE_Box:
        case TYPE_Fullscreen:
            break;

        default:
            Error("Invalid GUIProgressBar type: %u", type);
    }

    // v2 = this;
    if (loading_bg) return false;

    uType = type;

    v4 = 1;
    if (uType == TYPE_Fullscreen) {
        v5 = 0;
        // v6 = (int)&field_10;
        do {
            if (field_10[v4] == 1) ++v5;
            ++v4;
        } while (v4 <= 5);
        if (v5 == 5) memset(field_10, 0, 8);
        v7 = rand() % 5 + 1;
        if (field_10[v7] == 1) {
            do
                v7 = rand() % 5 + 1;
            while (field_10[v7] == 1);
        }

        loading_bg =
            assets->GetImage_PCXFromIconsLOD(StringPrintf("loading%d.pcx", v7));
        // pLoadingBg.Load(Str1, 2);
        uProgressCurrent = 0;
        uX = 122;
        uY = 151;
        uWidth = 449;
        uHeight = 56;
        uProgressMax = 26;

        progressbar_loading = assets->GetImage_Alpha("loadprog");
        Draw();
        return true;
    }

    switch (pParty->alignment) {
        case PartyAlignment_Good:
            progressbar_dungeon = assets->GetImage_ColorKey("bardata-b", 0x7FF);
            break;
        case PartyAlignment_Neutral:
            progressbar_dungeon = assets->GetImage_ColorKey("bardata", 0x7FF);
            break;
        case PartyAlignment_Evil:
            progressbar_dungeon = assets->GetImage_ColorKey("bardata-c", 0x7FF);
            break;
        default:
            Error("Invalid alignment type: %u", pParty->alignment);
    }

    uProgressCurrent = 0;
    uProgressMax = 26;
    Draw();
    return true;
}

//----- (004435BB) --------------------------------------------------------
void GUIProgressBar::Reset(unsigned __int8 uMaxProgress) {
    uProgressCurrent = 0;
    uProgressMax = uMaxProgress;
}

//----- (004435CD) --------------------------------------------------------
void GUIProgressBar::Progress() {
    ++this->uProgressCurrent;
    if (this->uProgressCurrent > this->uProgressMax)
        this->uProgressCurrent = this->uProgressMax;
    this->Draw();
}

//----- (004435E2) --------------------------------------------------------
void GUIProgressBar::Release() {
    //    int v3; // edi@7

    if (loading_bg) {
        loading_bg->Release();
        loading_bg = nullptr;
    }

    if (this->uType == 1) {
        if (this->uProgressCurrent != this->uProgressMax) {
            this->uProgressCurrent = this->uProgressMax - 1;
            Progress();
        }

        if (progressbar_loading) {
            progressbar_loading->Release();
            progressbar_loading = nullptr;
        }
    } else {
        if (progressbar_dungeon) {
            progressbar_dungeon->Release();
            progressbar_dungeon = nullptr;
        }
    }
}

void GUIProgressBar::Draw() {
    render->BeginScene();
    if (uType != TYPE_Fullscreen) {
        render->DrawTextureAlphaNew(80 / 640.0f, 122 / 480.0f,
                                    progressbar_dungeon);
        render->DrawTextureAlphaNew(
            100 / 640.0f, 146 / 480.0f,
            pIconsFrameTable->GetFrame(uIconID_TurnHour, 0)->GetTexture());
        render->FillRectFast(
            174, 164,
            floorf(((double)(113 * uProgressCurrent) / (double)uProgressMax) +
                   0.5f),
            16, 0xF800);
    } else {
        if (loading_bg) {
            render->DrawTextureNew(0, 0, loading_bg);
            render->SetUIClipRect(
                172, 459,
                15 *
                        (int)(__int64)((double)(300 * uProgressCurrent) /
                                       (double)uProgressMax) /
                        15 +
                    172,
                471);
            render->DrawTextureAlphaNew(172 / 640.0f, 459 / 480.0f,
                                        progressbar_loading);
            render->ResetUIClipRect();
        }
    }

    render->EndScene();
    render->Present();
}
