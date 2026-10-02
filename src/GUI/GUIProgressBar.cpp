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

    releaseImages();

    if (type == TYPE_None)
        return true;
    assert(type == TYPE_Box || type == TYPE_Fullscreen);
    uType = type;

    turnHourIconId = pIconsFrameTable->animationId("turnhour");

    if (uType == TYPE_Fullscreen) {
        loading_bg = assets->getIcon(fmt::format("loading{}.pcx", vrng->random(5) + 1));

        uProgressCurrent = 0;
        uX = 122;
        uY = 151;
        uWidth = 449;
        uHeight = 56;
        uProgressMax = 26;

        progressbar_loading = assets->getIcon("loadprog");
        drawIfNotSingleFrame();
        return true;
    } else {
        progressbar_dungeon = assets->getIcon(ProgressBarResourceByAlignment[pParty->alignment]);
    }

    uProgressCurrent = 0;
    uProgressMax = 26;
    drawIfNotSingleFrame();
    return true;
}

void GUIProgressBar::Reset(uint8_t uMaxProgress) {
    uProgressCurrent = 0;
    uProgressMax = uMaxProgress;
}

void GUIProgressBar::Progress() {
    uProgressCurrent = std::min((uint8_t)(uProgressCurrent + 1), uProgressMax);
    drawIfNotSingleFrame();
}

void GUIProgressBar::Release() {
    if (uType != TYPE_None && engine->config->debug.SingleFrameLoadingScreen.value()) {
        uProgressCurrent = uProgressMax;
        Draw();
    }

    releaseImages();
}

void GUIProgressBar::releaseImages() {
    if (loading_bg != nullptr) {
        assets->releaseImage(loading_bg);
        loading_bg = nullptr;
    }

    if (progressbar_loading != nullptr) {
        assets->releaseImage(progressbar_loading);
        progressbar_loading = nullptr;
    }

    if (progressbar_dungeon != nullptr) {
        assets->releaseImage(progressbar_dungeon);
        progressbar_dungeon = nullptr;
    }

    uType = TYPE_None;
}

void GUIProgressBar::Draw() {
    // render->BeginScene3D();
    render->BeginScene2D();

    if (uType != TYPE_Fullscreen) {
        engine->DrawGUI();
        GUI_UpdateWindows();
        pParty->updateCharactersAndHirelingsEmotions();

        render->DrawQuad2D(progressbar_dungeon, {80, 122});
        render->DrawQuad2D(pIconsFrameTable->animationFrame(turnHourIconId, 0_ticks), {100, 146});
        render->FillRect(Recti(174, 164, floorf((double)(113 * uProgressCurrent) / (double)uProgressMax + 0.5f), 16), colorTable.Red);
    } else {
        if (loading_bg) {
            render->DrawQuad2D(loading_bg, {0, 0});
        }
        render->SetUIClipRect(Recti(172, 459, (double)(300 * uProgressCurrent) / (double)uProgressMax, 12));
        render->DrawQuad2D(progressbar_loading, {172, 459});
        render->ResetUIClipRect();
    }

    render->Present();
}

void GUIProgressBar::drawIfNotSingleFrame() {
    if (!engine->config->debug.SingleFrameLoadingScreen.value())
        Draw();
}

bool GUIProgressBar::IsActive() {
    return uType != TYPE_None;
}
