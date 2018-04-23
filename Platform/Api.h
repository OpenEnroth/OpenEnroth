#pragma once
#include "Engine/Point.h"
#include "Engine/Strings.h"

#include "Platform/OSWindow.h"

int OS_GetAppInt(const char *pKey, int uDefValue);
void OS_SetAppInt(const char *pKey, int val);

bool OS_GetAppString(const char *path, char *out_string, int out_string_size);
void OS_GetAppString(const char *pKeyName, char *pOutString, int uBufLen,
                     const char *pDefaultValue);
void OS_SetAppString(const char *pKey, const char *pString);

void OS_MsgBox(const wchar_t *msg, const wchar_t *title);
void OS_PeekMessage();
void OS_PeekMessageLoop();
void OS_WaitMessage();

void OS_Sleep(int ms);
void OS_ShowCursor(bool show);

bool OS_OpenConsole();

Point OS_GetMouseCursorPos();

unsigned int OS_GetTime();
uint64_t OS_GetPrecisionTime();

bool OS_IfShiftPressed();
bool OS_IfCtrlPressed();
