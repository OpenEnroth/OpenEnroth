#pragma once
#include "Engine/Strings.h"

void GameUI_StatusBar_Draw();
void GameUI_StatusBar_DrawForced();

void GameUI_StatusBar_Set(const char *pStr);
void GameUI_StatusBar_Set(String &str);

void GameUI_StatusBar_OnEvent(const char *pString, unsigned int num_seconds = 2);
void GameUI_StatusBar_OnEvent(String &str, unsigned int num_seconds = 2);
void GameUI_StatusBar_OnEvent_128ms(const char *str);
void GameUI_StatusBar_OnEvent_128ms(String &str);
void GameUI_StatusBar_ClearEventString();

String GameUI_StatusBar_GetInput();
void GameUI_StatusBar_OnInput(const char *str);
void GameUI_StatusBar_ClearInputString();

void GameUI_StatusBar_NothingHere();

void GameUI_StatusBar_DrawImmediate(String &str, int color);
void GameUI_StatusBar_DrawImmediate(const char *Str, int color);