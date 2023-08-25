#include "UIStatusBar.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Localization.h"

#include "Engine/Graphics/IRender.h"

#include "GUI/GUIFont.h"

#include "GUI/UI/UIGame.h"

const std::string &StatusBar::get() {
    if (_eventStatusExpireTime) {
        return _eventStatusString;
    } else {
        return _statusString;
    }
}

void StatusBar::draw() {
    render->DrawTextureNew(0, 352 / 480.0f, game_ui_statusbar);

    const std::string &status = get();
    if (status.length() > 0) {
        pPrimaryWindow->DrawText(assets->pFontLucida.get(), { assets->pFontLucida->AlignText_Center(450, status) + 11, 357}, uGameUIFontMain, status, 0, uGameUIFontShadow);
    }
}

void StatusBar::drawForced(const std::string &str, Color color) {
    render->DrawTextureNew(0, 352 / 480.0f, game_ui_statusbar);
    pPrimaryWindow->DrawText(assets->pFontLucida.get(), { assets->pFontLucida->AlignText_Center(450, str) + 11, 357}, color, str);
}

void StatusBar::update() {
    // Was also checking that event timer is not stopped
    if (_eventStatusExpireTime && platform->tickCount() >= _eventStatusExpireTime) {
        _eventStatusExpireTime = 0;
    }
}

void StatusBar::setPermanent(const std::string &str) {
    if (str.length() > 0) {
        if (_eventStatusExpireTime == 0) {
            _statusString = str;
        }
    }
}

void StatusBar::clearPermanent() {
    _statusString.clear();
}

void StatusBar::clearAll() {
    _statusString.clear();
    clearEvent();
}

void StatusBar::setEvent(const std::string &str) {
    _eventStatusString = str;
    _eventStatusExpireTime = platform->tickCount() + EVENT_DURATION;
}

void StatusBar::setEventShort(const std::string &str) {
    _eventStatusString = str;
    _eventStatusExpireTime = platform->tickCount() + EVENT_DURATION_SHORT;
}

void StatusBar::clearEvent() {
    _eventStatusExpireTime = 0;
}

void StatusBar::nothingHere() {
    if (_eventStatusExpireTime == 0) {
        setEvent(LSTR_NOTHING_HERE);
    }
}
