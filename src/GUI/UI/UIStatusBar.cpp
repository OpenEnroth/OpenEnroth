#include "UIStatusBar.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Localization.h"

#include "Engine/Graphics/IRender.h"

#include "GUI/GUIFont.h"

#include "GUI/UI/UIGame.h"

void GameUI_StatusBar_Set(const std::string &str) {
    if (str.length() > 0) {
        if (!game_ui_status_bar_event_string_time_left) {
            game_ui_status_bar_string = str;
        }
    }
}

const std::string &GameUI_StatusBar_Get() {
    if (game_ui_status_bar_event_string_time_left) {
        return game_ui_status_bar_event_string;
    } else {
        return game_ui_status_bar_string;
    }
}

void GameUI_StatusBar_Clear() {
    game_ui_status_bar_string.clear();
    GameUI_StatusBar_ClearEventString();
}

//----- (00448B45) --------------------------------------------------------
void GameUI_StatusBar_Update(bool force_hide) {
    if (force_hide ||
        game_ui_status_bar_event_string_time_left &&
        platform->tickCount() >= game_ui_status_bar_event_string_time_left && !pEventTimer->bPaused) {
        game_ui_status_bar_event_string_time_left = 0;
    }
}

void GameUI_StatusBar_OnEvent_Internal(const std::string &str, unsigned int ms) {
    game_ui_status_bar_event_string = str;
    game_ui_status_bar_event_string_time_left = platform->tickCount() + ms;
}

void GameUI_SetStatusBar(const std::string &str) {
    GameUI_StatusBar_OnEvent_Internal(str, 2 * 1000);
}

void GameUI_SetStatusBarShortNotification(const std::string &str) {
    GameUI_StatusBar_OnEvent_Internal(str, 128);
}

void GameUI_StatusBar_ClearEventString() {
    game_ui_status_bar_event_string.clear();
    game_ui_status_bar_event_string_time_left = 0;
}

void GameUI_StatusBar_OnInput(const std::string &str) {
    game_ui_status_bar_event_string = std::string(str);
}

std::string GameUI_StatusBar_GetInput() { return game_ui_status_bar_event_string; }

void GameUI_StatusBar_ClearInputString() {
    game_ui_status_bar_event_string.clear();
    game_ui_status_bar_event_string_time_left = 0;
}

void GameUI_StatusBar_NothingHere() {
    if (game_ui_status_bar_event_string_time_left == 0) {
        GameUI_SetStatusBar(LSTR_NOTHING_HERE);
    }
}

void GameUI_StatusBar_Draw() {
    render->DrawTextureNew(0, 352 / 480.0f, game_ui_statusbar);

    const std::string &status = GameUI_StatusBar_Get();
    if (status.length() > 0) {
        pPrimaryWindow->DrawText(pFontLucida, {pFontLucida->AlignText_Center(450, status) + 11, 357}, uGameUIFontMain, status, 0, uGameUIFontShadow);
    }
}

void GameUI_StatusBar_DrawImmediate(const std::string &str, Color color) {
    render->DrawTextureNew(0, 352 / 480.0f, game_ui_statusbar);
    pPrimaryWindow->DrawText(pFontLucida, {pFontLucida->AlignText_Center(450, str) + 11, 357}, color, str);
}
