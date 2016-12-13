#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"

#include "Engine/Graphics/Render.h"

#include "GUI/UI/UiStatusBar.h"
#include "GUI/UI/UiGame.h"
#include "GUI/GUIFont.h"

//----- (0041C0B8) --------------------------------------------------------
void GameUI_StatusBar_Set(String &str)
{
    if (str.length() > 0)
    {
        if (!game_ui_status_bar_event_string_time_left)
        {
            game_ui_status_bar_string = str;
        }
    }
}

void GameUI_StatusBar_Set(const char *pStr)
{
    GameUI_StatusBar_Set(String(pStr));
}


void GameUI_StatusBar_OnEvent_Internal(String &str, unsigned int ms)
{
    game_ui_status_bar_event_string = str;
    game_ui_status_bar_event_string_time_left = GetTickCount() + ms;
}

//----- (0044C175) --------------------------------------------------------
void GameUI_StatusBar_OnEvent(String &str, unsigned int num_seconds)
{
    GameUI_StatusBar_OnEvent_Internal(str, 1000 * num_seconds);
}

void GameUI_StatusBar_OnEvent(const char *str, unsigned int num_seconds)
{
    GameUI_StatusBar_OnEvent(std::string(str), num_seconds);
}

void GameUI_StatusBar_OnEvent_128ms(String &str)
{
    GameUI_StatusBar_OnEvent_Internal(str, 128);
}

void GameUI_StatusBar_OnEvent_128ms(const char *str)
{
    GameUI_StatusBar_OnEvent_128ms(String(str));
}

void GameUI_StatusBar_ClearEventString()
{
    game_ui_status_bar_event_string.clear();
    game_ui_status_bar_event_string_time_left = 0;
}



void GameUI_StatusBar_OnInput(const char *str)
{
    game_ui_status_bar_event_string = String(str);
}

String GameUI_StatusBar_GetInput()
{
    return game_ui_status_bar_event_string;
}

void GameUI_StatusBar_ClearInputString()
{
    game_ui_status_bar_event_string.clear();
    game_ui_status_bar_event_string_time_left = 0;
}


//----- (0044C1D0) --------------------------------------------------------
void GameUI_StatusBar_NothingHere()
{
    if (!game_ui_status_bar_event_string_time_left)
    {
        GameUI_StatusBar_OnEvent(localization->GetString(521)); // Nothing here
    }
}




//----- (0041C179) --------------------------------------------------------
void GameUI_StatusBar_DrawForced()
{
    if (game_ui_status_bar_string.length() > 0 || game_ui_status_bar_event_string_time_left || bForceDrawFooter)
    {
        bForceDrawFooter = false;
        GameUI_StatusBar_Draw();
    }
}


//----- (0041C047) --------------------------------------------------------
void GameUI_StatusBar_Draw()
{
    pRenderer->DrawTextureNew(0, 352 / 480.0f, game_ui_statusbar);

    String status;
    if (game_ui_status_bar_event_string_time_left)
    {
        status = game_ui_status_bar_event_string;
    }
    else
    {
        status = game_ui_status_bar_string;
    }

    if (status.length() > 0)
    {
        pPrimaryWindow->DrawText(
            pFontLucida,
            pFontLucida->AlignText_Center(450, status) + 11,
            357,
            uGameUIFontMain,
            status,
            0,
            0,
            uGameUIFontShadow
        );
    }
}


//----- (004B46A5) --------------------------------------------------------
void GameUI_StatusBar_DrawImmediate(String &str, int color)
{
    pRenderer->DrawTextureNew(0, 352 / 480.0f, game_ui_statusbar);
    pPrimaryWindow->DrawText(
        pFontLucida,
        pFontLucida->AlignText_Center(450, str) + 11,
        357,
        color,
        str
    );
}


void GameUI_StatusBar_DrawImmediate(const char *Str, int color)
{
    GameUI_StatusBar_DrawImmediate(String(Str), color);
}