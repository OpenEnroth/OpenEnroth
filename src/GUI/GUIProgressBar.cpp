#include "GUI/GUIProgressBar.h"

#include <algorithm>

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Party.h"

#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Tables/IconFrameTable.h"
#include "Engine/Random/Random.h"

#include "GUI/GUIWindow.h"

#include "Utility/IndexedArray.h"

static constexpr IndexedArray<const char *, PartyAlignment_Good, PartyAlignment_Evil> ProgressBarResourceByAlignment = {
    {PartyAlignment_Good, "bardata-b"},
    {PartyAlignment_Neutral, "bardata"},
    {PartyAlignment_Evil, "bardata-c"}
};

GUIProgressBar *pGameLoadingUI_ProgressBar = new GUIProgressBar();

bool GUIProgressBar::Initialize(Type type) {
    if (loading_bg) {
        return false;
    }

    Release();

    if (type == TYPE_None)
        return true;
    assert(type == TYPE_Box || type == TYPE_Fullscreen);
    uType = type;

    turnHourIconId = pIconsFrameTable->animationId("turnhour");

    if (uType == TYPE_Fullscreen) {
        loading_bg = assets->getImage_PCXFromIconsLOD(fmt::format("loading{}.pcx", vrng->random(5) + 1));

        uProgressCurrent = 0;
        uX = 122;
        uY = 151;
        uWidth = 449;
        uHeight = 56;
        uProgressMax = 26;

        progressbar_loading = assets->getImage_Alpha("loadprog");
        Draw();
        return true;
    } else {
        progressbar_dungeon = assets->getImage_ColorKey(ProgressBarResourceByAlignment[pParty->alignment]);
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

    uType = TYPE_None;
}

void GUIProgressBar::Draw() {
    // render->BeginScene3D();
    render->BeginScene2D();
    //render->ClearBlack();

    if (uType != TYPE_Fullscreen) {
        engine->DrawGUI();
        GUI_UpdateWindows();
        pParty->updateCharactersAndHirelingsEmotions();

        render->DrawTextureNew(80 / 640.0f, 122 / 480.0f, progressbar_dungeon);
        render->DrawTextureNew(100 / 640.0f, 146 / 480.0f, pIconsFrameTable->animationFrame(turnHourIconId, 0_ticks));
        render->FillRectFast(174, 164, floorf(((double)(113 * uProgressCurrent) / (double)uProgressMax) + 0.5f), 16, colorTable.Red);
    } else {
        if (loading_bg) {
            render->DrawTextureNew(0, 0, loading_bg);
        }
        render->SetUIClipRect(Recti(172, 459, (int)((double)(300 * uProgressCurrent) / (double)uProgressMax), 12));
        render->DrawTextureNew(172 / 640.0f, 459 / 480.0f, progressbar_loading);
        render->ResetUIClipRect();
    }

    render->Present();
}

bool GUIProgressBar::IsActive() {
    return uType != TYPE_None;
}
