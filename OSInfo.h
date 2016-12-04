#pragma once
#include "OSAPI.h"

class OSInfo
{
  public:
    inline static bool CanUseDirectDraw4()
    {
      if (Ready())
        return info.dwPlatformId != VER_PLATFORM_WIN32_NT || info.dwMajorVersion >= 4;
      return false;
    }

    inline static bool CanUseDirectDraw2()
    {
      return true;
    }


  private:
    static bool           initialized;
    static OSVERSIONINFOA info;

    static bool Ready()
    {
      if (!initialized)
      {
        info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
        if (GetVersionExA(&info))
          initialized = true;
      }
      return initialized;
    }
};