#include <stdarg.h>

#include "GUI/UI/UIStatusBar.h"

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Localization.h"

#include "Engine/Graphics/IRender.h"

#include "GUI/GUIFont.h"

#include "GUI/UI/UIGame.h"

#include "Platform/Api.h"

void GameUI_StatusBar_Set(const String &str) {
    if (str.length() > 0) {
        if (!game_ui_status_bar_event_string_time_left) {
            game_ui_status_bar_string = str;
        }
    }
}

void GameUI_StatusBar_Clear() {
    game_ui_status_bar_string.clear();
    GameUI_StatusBar_ClearEventString();
}

void GameUI_StatusBar_OnEvent_Internal(const String &str, unsigned int ms) {
    game_ui_status_bar_event_string = str;
    game_ui_status_bar_event_string_time_left = OS_GetTime() + ms;
}

void GameUI_SetStatusBar(const String &str) {
    GameUI_StatusBar_OnEvent_Internal(str, 2 * 1000);
}


void GameUI_SetStatusBar(int localization_string_id, ...) {
    va_list args_ptr;

    const char* format = localization->GetString(localization_string_id);
    char buf[4096];

    va_start(args_ptr, localization_string_id);
    vsprintf(buf, format, args_ptr);
    va_end(args_ptr);

    extern int sprintfex_internal(char* str);
    sprintfex_internal(buf);
    GameUI_SetStatusBar(buf);
}


void GameUI_SetStatusBarShortNotification(const String &str) {
    GameUI_StatusBar_OnEvent_Internal(str, 128);
}

void GameUI_StatusBar_ClearEventString() {
    game_ui_status_bar_event_string.clear();
    game_ui_status_bar_event_string_time_left = 0;
}

void GameUI_StatusBar_OnInput(const String &str) {
    game_ui_status_bar_event_string = String(str);
}

String GameUI_StatusBar_GetInput() { return game_ui_status_bar_event_string; }

void GameUI_StatusBar_ClearInputString() {
    game_ui_status_bar_event_string.clear();
    game_ui_status_bar_event_string_time_left = 0;
}

void GameUI_StatusBar_NothingHere() {
    if (!game_ui_status_bar_event_string_time_left) {
        GameUI_SetStatusBar(localization->GetString(LSTR_NOTHING_HERE));
    }
}

void GameUI_StatusBar_DrawForced() {
    if (game_ui_status_bar_string.length() > 0 ||
        game_ui_status_bar_event_string_time_left || bForceDrawFooter) {
        bForceDrawFooter = false;
        GameUI_StatusBar_Draw();
    }
}

void GameUI_StatusBar_Draw() {
    render->DrawTextureNew(0, 352 / 480.0f, game_ui_statusbar);

    String status;
    if (game_ui_status_bar_event_string_time_left) {
        status = game_ui_status_bar_event_string;
    } else {
        status = game_ui_status_bar_string;
    }

    if (status.length() > 0) {
        pPrimaryWindow->DrawText(
            pFontLucida, pFontLucida->AlignText_Center(450, status) + 11, 357,
            uGameUIFontMain, status, 0, 0, uGameUIFontShadow);
    }
}

void GameUI_StatusBar_DrawImmediate(const String &str, int color) {
    render->DrawTextureNew(0, 352 / 480.0f, game_ui_statusbar);
    pPrimaryWindow->DrawText(pFontLucida,
                             pFontLucida->AlignText_Center(450, str) + 11, 357,
                             color, str);
}
