#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRT_SECURE_NO_WARNINGS
#include "Log.h"

#include <stdio.h>
#include <windows.h>

HANDLE hStdOut = nullptr;


void Log::Initialize()
{
  if (AllocConsole())
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
}


void Log::Warning(const wchar_t *pFormat, ...)
{
  if (!hStdOut)
    return;

  va_list args;
  wchar_t pMsg[8192];

  va_start(args, pFormat);
  vswprintf_s(pMsg, 8192, pFormat, args);
  va_end(args);

  DWORD w;
  WriteConsoleW(hStdOut, pMsg, lstrlenW(pMsg), &w, nullptr);
  WriteConsoleW(hStdOut, L"\r\n", 2, &w, nullptr);
}
//----- (004BE386) --------------------------------------------------------
void __fastcall log_error(const char *pMessage)
{
  const char *v1; // edi@1
  FILE *f; // eax@1
  FILE *v3; // esi@1

  v1 = pMessage;
  f = fopen("errorlog.txt", "a");
  v3 = f;
  if ( f )
  {
    fprintf(f, "%s\n", v1);
    fclose(v3);
    fflush(v3);
  }
}