#include "LoadStep2State.h"

#include "Engine/mm7_data.h"

LoadStep2State::LoadStep2State() {
}

void LoadStep2State::enter() {
    // This specific value GAME_SETTINGS_4000 is checked only in UIPartyCreation. 
    // So, this assignment might be removed after the Party Creation becomes part of the FSM
    dword_6BE364_game_settings_1 |= GAME_SETTINGS_4000;
    /*
    GraphicsImage *tex = assets->getImage_PCXFromIconsLOD("mm6title.pcx");

    render->ResetUIClipRect();
    render->BeginScene2D();
    {
        render->DrawTextureNew(0, 0, tex);
        drawMM7CopyrightWindow();
        render->Present();

        initFunc();

        tex->Release();
    }
    GUIWindow_MainMenu::drawCopyrightAndInit([&] {
        engine->SecondaryInitialization();
        FinalInitialization();
    });*/
}

void LoadStep2State::update() {
}

void LoadStep2State::exit() {
}
