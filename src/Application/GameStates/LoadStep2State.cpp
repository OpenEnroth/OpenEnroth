#include "LoadStep2State.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/mm7_data.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "GUI/GUIWindow.h"

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
        render->DrawQuad2D(_fullscreenTexture, {0, 0});
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
    _fullscreenTexture->release();
    _fullscreenTexture = nullptr;
    _isFirstPass = false;
}

void LoadStep2State::_drawMM7CopyrightWindow() {
    GUIWindow window;
    window.frameRect.w = 624;
    window.frameRect.h = 256;
    window.frameRect.x = 8;
    window.frameRect.y = 30;
    window.frameRect.h = assets->pFontSmallnum->CalcTextHeight(localization->str(LSTR_1999_THE_3DO_COMPANY_ALL_RIGHTS_RESERVED), window.frameRect.w, 24);
    window.frameRect.h += 2 * assets->pFontSmallnum->GetHeight() + 24;
    window.frameRect.y = 470 - window.frameRect.h;
    window.DrawMessageBox(0);

    window.frameRect.w -= 28;
    window.frameRect.x += 12;
    window.frameRect.y += 12;
    window.frameRect.h -= 12;
    window.DrawTitleText(assets->pFontSmallnum.get(), 0, 12, ui_mainmenu_copyright_color, localization->str(LSTR_1999_THE_3DO_COMPANY_ALL_RIGHTS_RESERVED), 3);
}
