#pragma once

#include <string>
#include <utility>

#include "Engine/Localization.h"

#include "Library/Color/Color.h"

void GameUI_StatusBar_Draw();
void GameUI_StatusBar_Set(const std::string &str);
const std::string &GameUI_StatusBar_Get();
void GameUI_StatusBar_Clear();

void GameUI_StatusBar_Update(bool force_hide = false);

void GameUI_SetStatusBar(const std::string &str);

template<class... Args>
void GameUI_SetStatusBar(int localization_string_id, Args &&... args) {
    // TODO(captainurist): what if fmt throws?
    GameUI_SetStatusBar(fmt::sprintf(localization->GetString(localization_string_id), std::forward<Args>(args)...));
    // TODO(captainurist): there was also a call to sprintfex_internal here.
}

void GameUI_SetStatusBarShortNotification(const std::string &str);
void GameUI_StatusBar_ClearEventString();

std::string GameUI_StatusBar_GetInput();
void GameUI_StatusBar_OnInput(const std::string &str);
void GameUI_StatusBar_ClearInputString();

void GameUI_StatusBar_NothingHere();

void GameUI_StatusBar_DrawImmediate(const std::string &str, Color color);
