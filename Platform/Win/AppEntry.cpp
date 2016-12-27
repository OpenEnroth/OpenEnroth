#include "Engine/Log.h"

#include "Platform/Api.h"
#include "Platform/Win/Win.h"


//----- (00462C94) --------------------------------------------------------
int __stdcall wWinMain(HINSTANCE hInstance, HINSTANCE hprevinstance, wchar_t *lpCmdLine, int nShowCmd)
{
    #ifndef NDEBUG
    {
        //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);
    }
    #endif

    Log::Initialize();

    bool mm7_installation_found = false;
    char mm7_path[2048];

    // standard 1.0 installation
    if (!mm7_installation_found)
    {
        mm7_installation_found = OS_GetAppString(
            "HKEY_LOCAL_MACHINE/SOFTWARE/New World Computing/Might and Magic VII/1.0/AppPath",
            mm7_path, sizeof(mm7_path)
        );

        if (mm7_installation_found)
        {
            Log::Warning(L"Standard MM7 installation found");
        }
    }

    // GoG version
    if (!mm7_installation_found)
    {
        mm7_installation_found = OS_GetAppString(
            "HKEY_LOCAL_MACHINE/SOFTWARE/GOG.com/GOGMM7/PATH",
            mm7_path, sizeof(mm7_path)
        );

        if (mm7_installation_found)
        {
            Log::Warning(L"GoG MM7 installation found");
        }
    }

	// Hack path fix - pskelton
	if (!mm7_installation_found) {
		mm7_installation_found = 1;
		strcpy(mm7_path, "E:/Programs/GOG Galaxy/Games/Might and Magic 7");
		Log::Warning(L"Hack Path MM7 installation found");
	}


    if (HWND hMM7Window = FindWindowW(L"M&MTrilogy", 0))//check whether the window is open
    {
        if (IsIconic(hMM7Window))
            ShowWindow(hMM7Window, SW_RESTORE);
        SetForegroundWindow(GetLastActivePopup(hMM7Window));
        return 0;
    }

    HWND hPrevWindow = GetActiveWindow();
    if (!hPrevWindow)
    {
        Log::Warning(L"OS init: ok");
        extern bool MM_Main(const wchar_t *pCmdLine, const char *mm7_path);
        MM_Main(lpCmdLine, mm7_path);
    }
    if (hPrevWindow)
    SetActiveWindow(hPrevWindow);

    return GetLastError();
}