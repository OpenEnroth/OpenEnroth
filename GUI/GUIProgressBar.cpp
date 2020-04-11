#include "GUI/GUIProgressBar.h"

#include <algorithm>

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/Party.h"

#include "Engine/Graphics/IRender.h"

#include "Engine/Tables/IconFrameTable.h"

GUIProgressBar *pGameLoadingUI_ProgressBar = new GUIProgressBar();

GUIProgressBar::GUIProgressBar() {
    progressbar_dungeon = nullptr;
    progressbar_loading = nullptr;
    loading_bg = nullptr;
}

bool GUIProgressBar::Initialize(Type type) {
    Release();

    switch (type) {
        case TYPE_None:
            return true;

        case TYPE_Box:
        case TYPE_Fullscreen:
            break;

        default:
            Error("Invalid GUIProgressBar type: %u", type);
    }

    if (loading_bg) {
        return false;
    }

    uType = type;

    if (uType == TYPE_Fullscreen) {
        loading_bg = assets->GetImage_PCXFromIconsLOD(StringPrintf("loading%d.pcx", rand() % 5 + 1));

        uProgressCurrent = 0;
        uX = 122;
        uY = 151;
        uWidth = 449;
        uHeight = 56;
        uProgressMax = 26;

        progressbar_loading = assets->GetImage_Alpha("loadprog");
        Draw();
        return true;
    } else {
        switch (pParty->alignment) {
            case PartyAlignment::PartyAlignment_Good:
                progressbar_dungeon = assets->GetImage_ColorKey("bardata-b", 0x7FF);
                break;
            case PartyAlignment::PartyAlignment_Neutral:
                progressbar_dungeon = assets->GetImage_ColorKey("bardata", 0x7FF);
                break;
            case PartyAlignment::PartyAlignment_Evil:
                progressbar_dungeon = assets->GetImage_ColorKey("bardata-c", 0x7FF);
                break;
            default:
                Error("Invalid alignment type: %u", pParty->alignment);
        }
    }

    uProgressCurrent = 0;
    uProgressMax = 26;
    Draw();
    return true;
}

void GUIProgressBar::Reset(uint8_t uMaxProgress) {
    uProgressCurrent = 0;
    uProgressMax = uMaxProgress;
}

void GUIProgressBar::Progress() {
    uProgressCurrent = std::min((uint8_t)(uProgressCurrent + 1), uProgressMax);
    Draw();
}

void GUIProgressBar::Release() {
    if (loading_bg != nullptr) {
        loading_bg->Release();
        loading_bg = nullptr;
    }

    if (progressbar_loading != nullptr) {
        progressbar_loading->Release();
        progressbar_loading = nullptr;
    }

    if (progressbar_dungeon != nullptr) {
        progressbar_dungeon->Release();
        progressbar_dungeon = nullptr;
    }
}

void GUIProgressBar::Draw() {
    render->BeginScene();

    if (uType != TYPE_Fullscreen) {
        render->DrawTextureAlphaNew(80 / 640.0f, 122 / 480.0f, progressbar_dungeon);
        render->DrawTextureAlphaNew(100 / 640.0f, 146 / 480.0f, pIconsFrameTable->GetFrame(uIconID_TurnHour, 0)->GetTexture());
        render->FillRectFast(174, 164, floorf(((double)(113 * uProgressCurrent) / (double)uProgressMax) + 0.5f), 16, 0xF800);
    } else {
        if (loading_bg) {
            render->DrawTextureNew(0, 0, loading_bg);
        }
        render->SetUIClipRect(172, 459, (int)((double)(300 * uProgressCurrent) / (double)uProgressMax) + 172, 471);
        render->DrawTextureAlphaNew(172 / 640.0f, 459 / 480.0f, progressbar_loading);
        render->ResetUIClipRect();
    }

    render->EndScene();
    render->Present();
}
