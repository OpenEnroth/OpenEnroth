#include "UIStatusBar.h"

#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/EngineGlobals.h"
#include "Engine/GameType.h"
#include "Engine/Localization.h"
#include "Engine/mm7_data.h"

#include "Engine/Graphics/Renderer/Renderer.h"

#include "GUI/GUIFont.h"

#include "GUI/UI/UIGame.h"

const std::string &StatusBar::get() {
    if (_eventStatusExpireTime) {
        return _eventStatusString;
    } else {
        return _statusString;
    }
}

// MM7's status strip sits right under its own narrower viewport (y=352/357, matching
// BaseRenderer.cpp's MM7-only ViewPortY2 default) - MM8's sits lower, under the
// character-record tab-button row (text spans y=372-384). `game_ui_statusbar`'s background quad is null for MM8
// (GUIWindow.cpp's SetUserInterface), so only the text position differs here.
constexpr int kStatusTextY = 357;
constexpr int kMm8StatusTextY = 368;

void StatusBar::draw() {
    render->DrawQuad2D(game_ui_statusbar, {0, 352});

    const std::string &status = get();
    if (status.length() > 0) {
        int y = gameType() == GAME_TYPE_MM8 ? kMm8StatusTextY : kStatusTextY;
        GUIWindow::DrawText(assets->pFontLucida.get(), { assets->pFontLucida->AlignText_Center(450, status) + 11, y}, uGameUIFontMain, status, pPrimaryWindow->frameRect, 0, uGameUIFontShadow);
    }
}

void StatusBar::drawForced(std::string_view str, Color color) {
    render->DrawQuad2D(game_ui_statusbar, {0, 352});
    int y = gameType() == GAME_TYPE_MM8 ? kMm8StatusTextY : kStatusTextY;
    GUIWindow::DrawText(assets->pFontLucida.get(), { assets->pFontLucida->AlignText_Center(450, str) + 11, y}, color, str, pPrimaryWindow->frameRect);
}

void StatusBar::update() {
    // Was also checking that event timer is not stopped
    if (_eventStatusExpireTime && platform->tickCount() >= _eventStatusExpireTime)
        _eventStatusExpireTime = 0;
}

void StatusBar::setPermanent(std::string_view str) {
    if (str.length() > 0 && _eventStatusExpireTime == 0)
        _statusString = str;
}

void StatusBar::clearPermanent() {
    _statusString.clear();
}

void StatusBar::clearAll() {
    _statusString.clear();
    clearEvent();
}

void StatusBar::setEvent(std::string_view str) {
    _eventStatusString = str;
    _lastEvent = str;
    _lastEventSeq++;
    _eventStatusExpireTime = platform->tickCount() + EVENT_DURATION;
}

void StatusBar::setEventShort(std::string_view str) {
    _eventStatusString = str;
    _lastEvent = str;
    _lastEventSeq++;
    _eventStatusExpireTime = platform->tickCount() + EVENT_DURATION_SHORT;
}

void StatusBar::clearEvent() {
    _eventStatusExpireTime = 0;
}

void StatusBar::nothingHere() {
    if (_eventStatusExpireTime == 0)
        setEvent(LSTR_NOTHING_HERE);
}
