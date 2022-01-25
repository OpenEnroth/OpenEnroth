#pragma once

#include "Engine/Strings.h"

void GameUI_StatusBar_Draw();
void GameUI_StatusBar_DrawForced();

void GameUI_StatusBar_Set(const std::string &str);
void GameUI_StatusBar_Clear();

void GameUI_SetStatusBar(const std::string &str);
void GameUI_SetStatusBar(int localization_string_id, ...);
void GameUI_SetStatusBarShortNotification(const std::string &str);
void GameUI_StatusBar_ClearEventString();

std::string GameUI_StatusBar_GetInput();
void GameUI_StatusBar_OnInput(const std::string &str);
void GameUI_StatusBar_ClearInputString();

void GameUI_StatusBar_NothingHere();

void GameUI_StatusBar_DrawImmediate(const std::string &str, int color);
