#pragma once

#include <string>
#include <vector>

#include "Engine/Point.h"
#include "Engine/Strings.h"

int OS_GetAppInt(const char *pKey, int uDefValue);
void OS_SetAppInt(const char *pKey, int val);

bool OS_GetAppString(const char *path, char *out_string, int out_string_size);
void OS_GetAppString(const char *pKeyName, char *pOutString, int uBufLen,
                     const char *pDefaultValue);
void OS_SetAppString(const char *pKey, const char *pString);

void OS_MsgBox(const char *msg, const char *title);

void OS_Sleep(int ms);
void OS_ShowCursor(bool show);

bool OS_OpenConsole();

unsigned int OS_GetTime();
uint64_t OS_GetPrecisionTime();

std::vector<std::string> OS_FindFiles(const std::string &folder, const std::string &mask);

FILE* fcaseopen(char const* path, char const* mode);

std::string OS_GetDirSeparator(void);
