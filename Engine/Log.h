#pragma once


#define LOG_DECOMPILATION_WARNING()  Log::Warning(L"%S [%S:%u]", __FUNCTION__, __FILE__, __LINE__);

class Log
{
  public:
    static void Initialize();
    static void Warning(const wchar_t *pFormat, ...);
};