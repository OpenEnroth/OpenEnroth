#include "LoadStep2State.h"

#include <Engine/AssetsManager.h>
#include <Engine/Engine.h>
#include <Engine/Localization.h>
#include <Engine/mm7_data.h>
#include <Engine/Graphics/Renderer/Renderer.h>
#include <Engine/Graphics/Image.h>
#include <GUI/GUIWindow.h>

FsmAction LoadStep2State::enter() {
    // TODO(Gerark) This specific value GAME_SETTINGS_4000 is checked only in UIPartyCreation.
    // So, this assignment might be removed after the Party Creation becomes part of the FSM
    dword_6BE364_game_settings_1 |= GAME_SETTINGS_4000;

    _fullscreenTexture = assets->getImage_PCXFromIconsLOD("mm6title.pcx");
    _isFirstPass = true;
    return FsmAction::none();
}

FsmAction LoadStep2State::update() {
    // Not exactly the best approach but still better than calling Begin/Present directly here
    // We resolve this state in 2 frames. The first pass is drawing the main bkg texture and the copyrights
    // The second pass perform the loading part.
    if (_isFirstPass) {
        render->DrawTextureNew(0, 0, _fullscreenTexture);
        _drawMM7CopyrightWindow();
        _isFirstPass = false;
    } else {
        engine->SecondaryInitialization();
        FinalInitialization();
        //In case we want to test a slower loading
        //std::this_thread::sleep_for(std::chrono::seconds(1));
        return FsmAction::transition("done");
    }
    return FsmAction::none();
}

void LoadStep2State::exit() {
    _fullscreenTexture->Release();
    _fullscreenTexture = nullptr;
    _isFirstPass = false;
}

void LoadStep2State::_drawMM7CopyrightWindow() {
    GUIWindow window;
    window.uFrameWidth = 624;
    window.uFrameHeight = 256;
    window.uFrameX = 8;
    window.uFrameY = 30;
    window.uFrameHeight = assets->pFontSmallnum->CalcTextHeight(localization->GetString(LSTR_3DO_COPYRIGHT), window.uFrameWidth, 24, 0);
    window.uFrameHeight += 2 * assets->pFontSmallnum->GetHeight() + 24;
    window.uFrameY = 470 - window.uFrameHeight;
    window.uFrameZ = window.uFrameX + window.uFrameWidth - 1;
    window.uFrameW = 469;
    window.DrawMessageBox(0);

    window.uFrameWidth -= 28;
    window.uFrameX += 12;
    window.uFrameY += 12;
    window.uFrameHeight -= 12;
    window.uFrameZ = window.uFrameX + window.uFrameWidth - 1;
    window.uFrameW = window.uFrameY + window.uFrameHeight - 1;
    window.DrawTitleText(assets->pFontSmallnum.get(), 0, 12, ui_mainmenu_copyright_color, localization->GetString(LSTR_3DO_COPYRIGHT), 3);
}
