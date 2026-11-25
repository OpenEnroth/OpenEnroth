#include "UICredits.h"

#include <string>
#include <utility>

#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/GameType.h"
#include "Engine/Resources/ResourceManager.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"

constexpr Recti creditsRect(389, 19, 250, 440);

GUICredits::GUICredits() : GUIWindow(WINDOW_CREDITS, {0, 0}, render->GetRenderDimensions()) {
    _fontQuick = GUIFont::LoadFont("quick.fnt");
    _fontCChar = GUIFont::LoadFont("cchar.fnt");

    // MM8 installs don't ship an `mm6title.pcx` background image in icons.lod - set to
    // nullptr rather than show a placeholder glyph. MM7 reuses an MM6 title, so keep the load
    // active for MM7/MM6.
    _mm6TitleTexture = nullptr;
    if (gameType() != GAME_TYPE_MM8) {
        _mm6TitleTexture = assets->getImage_PCXFromIconsLOD("mm6title.pcx");
    }

    std::string text{ engine->resources()->eventsText("credits.txt").str() };

    int height = _fontQuick->GetStringHeightWithSecondFont(_fontCChar.get(), text, creditsRect.w, 0) + 2 * creditsRect.h;

    RgbaImage credits = RgbaImage::solid(Color(), creditsRect.w, height);
    _fontQuick->DrawCreditsEntry(_fontCChar.get(), 0, creditsRect.h, creditsRect.w, height, colorTable.CornFlowerBlue, colorTable.Primrose, colorTable.Black, text, &credits);
    _creditsTexture = GraphicsImage::Create(std::move(credits));
}

GUICredits::~GUICredits() {
    assets->evict(std::move(_mm6TitleTexture));
}

void GUICredits::Update() {
    if (_mm6TitleTexture)
        render->DrawQuad2D(_mm6TitleTexture, {0, 0});
    render->SetUIClipRect(creditsRect);
    render->DrawQuad2D(_creditsTexture, {creditsRect.x, static_cast<int>(creditsRect.y - _moveY)});
    render->ResetUIClipRect();

    _moveY += 0.25; // TODO(captainurist): #time gotta be dt-based.

    if (_moveY >= _creditsTexture->height())
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_CreditsFinished);
}
