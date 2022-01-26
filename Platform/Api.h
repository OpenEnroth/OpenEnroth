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

/**
 * @return                              Current value of a monotonic clock in milliseconds (1/1000s of a second).
 */
unsigned int OS_GetTime();
uint64_t OS_GetPrecisionTime();

std::vector<std::string> OS_FindFiles(const std::string &folder, const std::string &mask);

char OS_GetDirSeparator(void);
std::string OS_casepath(std::string path);
bool OS_FileExists(const std::string &path);
