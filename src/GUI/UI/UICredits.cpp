#include "UICredits.h"

#include <string>

#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/GameResourceManager.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"

GUICredits::GUICredits() : GUIWindow(WINDOW_Credits, {0, 0}, render->GetRenderDimensions()) {
    _fontQuick = GUIFont::LoadFont("quick.fnt", "FONTPAL");
    _fontCChar = GUIFont::LoadFont("cchar.fnt", "FONTPAL");

    _mm6TitleTexture = assets->getImage_PCXFromIconsLOD("mm6title.pcx");

    std::string text{ engine->_gameResourceManager->getEventsFile("credits.txt").string_view() };

    GUIWindow credit_window;
    credit_window.uFrameWidth = 250;
    credit_window.uFrameHeight = 440;
    credit_window.uFrameX = 389;
    credit_window.uFrameY = 19;

    int width = 250;
    int height = _fontQuick->GetStringHeight2(_fontCChar.get(), text, &credit_window, 0, 1) + 2 * credit_window.uFrameHeight;
    _creditsTexture = GraphicsImage::Create(width, height);

    _fontQuick->DrawCreditsEntry(_fontCChar.get(), 0, credit_window.uFrameHeight, width, height, colorTable.CornFlowerBlue, colorTable.Primrose, text, _creditsTexture);

    render->Update_Texture(_creditsTexture);

    CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_Escape, 0, Io::InputAction::Escape);
}

GUICredits::~GUICredits() {
    _mm6TitleTexture->Release();
    _creditsTexture->Release();
}

void GUICredits::Update() {
    GUIWindow credit_window;
    credit_window.uFrameWidth = 250;
    credit_window.uFrameHeight = 440;
    credit_window.uFrameX = 389;
    credit_window.uFrameY = 19;

    render->DrawTextureNew(0, 0, _mm6TitleTexture);
    render->SetUIClipRect(Recti(credit_window.uFrameX, credit_window.uFrameY, credit_window.uFrameWidth, credit_window.uFrameHeight));
    render->DrawTextureOffset(credit_window.uFrameX, credit_window.uFrameY, 0, _moveY, _creditsTexture);
    render->ResetUIClipRect();

    _moveY += 0.25; // TODO(captainurist): #time gotta be dt-based.

    if (_moveY >= _creditsTexture->height()) {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_CreditsFinished);
    }
}
