#include "UICredits.h"

#include <string>

#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/GameResourceManager.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"

constexpr Recti creditsRect(389, 19, 250, 440);

GUICredits::GUICredits() : GUIWindow(WINDOW_Credits, {0, 0}, render->GetRenderDimensions()) {
    _fontQuick = GUIFont::LoadFont("quick.fnt");
    _fontCChar = GUIFont::LoadFont("cchar.fnt");

    _mm6TitleTexture = assets->getImage_PCXFromIconsLOD("mm6title.pcx");

    std::string text{ engine->_gameResourceManager->getEventsFile("credits.txt").string_view() };

    int height = _fontQuick->GetStringHeight2(_fontCChar.get(), text, creditsRect.w, 0) + 2 * creditsRect.h;
    _creditsTexture = GraphicsImage::Create(creditsRect.w, height);

    _fontQuick->DrawCreditsEntry(_fontCChar.get(), 0, creditsRect.h, creditsRect.w, height, colorTable.CornFlowerBlue, colorTable.Primrose, colorTable.Black, text, _creditsTexture);

    render->Update_Texture(_creditsTexture);

    CreateButton({0, 0}, {0, 0}, 1, 0, UIMSG_Escape, 0, Io::InputAction::Escape);
}

GUICredits::~GUICredits() {
    _mm6TitleTexture->Release();
    _creditsTexture->Release();
}

void GUICredits::Update() {
    render->DrawTextureNew(0, 0, _mm6TitleTexture);
    render->SetUIClipRect(creditsRect);
    render->DrawTextureOffset(creditsRect.x, creditsRect.y, 0, _moveY, _creditsTexture);
    render->ResetUIClipRect();

    _moveY += 0.25; // TODO(captainurist): #time gotta be dt-based.

    if (_moveY >= _creditsTexture->height()) {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_CreditsFinished);
    }
}
