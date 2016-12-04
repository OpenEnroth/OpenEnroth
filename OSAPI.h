#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <MMSystem.h>
#pragma comment(lib, "winmm.lib")


int ReadWindowsRegistryInt(const char *pKey, int uDefValue); // idb
void WriteWindowsRegistryString(const char *pKey, const char *pString);
void ReadWindowsRegistryString(const char *pKeyName, char *pOutString, int uBufLen, const char *pDefaultValue);
void WriteWindowsRegistryInt(const char *pKey, int val);

#undef DrawText
#undef PlaySound
#undef Polygon