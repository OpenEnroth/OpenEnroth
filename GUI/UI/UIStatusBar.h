#pragma once

#include "Engine/Strings.h"

void GameUI_StatusBar_Draw();
void GameUI_StatusBar_DrawForced();

void GameUI_StatusBar_Set(const String &str);
void GameUI_StatusBar_Clear();

void GameUI_SetStatusBar(const String &str);
void GameUI_SetStatusBar(int localization_string_id, ...);
void GameUI_SetStatusBarShortNotification(const String &str);
void GameUI_StatusBar_ClearEventString();

String GameUI_StatusBar_GetInput();
void GameUI_StatusBar_OnInput(const String &str);
void GameUI_StatusBar_ClearInputString();

void GameUI_StatusBar_NothingHere();

void GameUI_StatusBar_DrawImmediate(const String &str, int color);
