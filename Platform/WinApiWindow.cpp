#include "Platform/WinApiWindow.h"

#include "Engine/Engine.h"
#include "Engine/Party.h"

#include "Engine/Graphics/IRender.h"

#include "GUI/GUIWindow.h"


void *WinApiWindow::GetWinApiHandle() {
    return (void *)hwnd;
}

void WinApiWindow::Activate() {
    SetActiveWindow(hwnd);
    SetForegroundWindow(hwnd);
    SendMessageW(hwnd, WM_ACTIVATEAPP, 1, 0);

    PeekMessageLoop();
}

void WinApiWindow::PeekMessageLoop() {
    MSG msg;
    while (PeekMessageW(&msg, hwnd, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            extern void Engine_DeinitializeAndTerminate(int exit_code);
            Engine_DeinitializeAndTerminate(0);
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void WinApiWindow::PeekSingleMessage() {
    MSG msg;
    if (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            extern void Engine_DeinitializeAndTerminate(int exit_code);
            Engine_DeinitializeAndTerminate(0);
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}



int WinApiWindow::GetX() const {
    RECT rc;
    GetWindowRect(hwnd, &rc);
    return rc.left;
}
int WinApiWindow::GetY() const {
    RECT rc;
    GetWindowRect(hwnd, &rc);
    return rc.top;
}
unsigned int WinApiWindow::GetWidth() const {
    RECT rc;
    GetClientRect(hwnd, &rc);
    return rc.right - rc.left;
}
unsigned int WinApiWindow::GetHeight() const {
    RECT rc;
    GetClientRect(hwnd, &rc);
    return rc.bottom - rc.top;
}
bool WinApiWindow::Focused() { return GetFocus() == hwnd; }

Point WinApiWindow::TransformCursorPos(Point &pt) const {
    Point transformed = pt;
    ScreenToClient(hwnd, (POINT *)&transformed);

    return transformed;
}

bool WinApiWindow::WinApiMessageProc(int msg, int wparam, void *lparam, void **result) {
    switch (msg) {
    case WM_KEYUP: {
        if (wparam == VK_CONTROL) {
            extern bool _507B98_ctrl_pressed;
            _507B98_ctrl_pressed = false;
        }
        if (wparam == VK_SNAPSHOT) {
            gameCallback->OnScreenshot();
        }

        return *result = 0, true;
    }

    case WM_SIZING:
        return *result = (void *)1, true;
    case WM_WINDOWPOSCHANGED:
        // if (pVideoPlayer && pVideoPlayer->AnyMovieLoaded() &&
        // pVideoPlayer->pBinkBuffer)
        // BinkBufferSetOffset(pVideoPlayer->pBinkBuffer, 0, 0);
        return false;

    case WM_CHAR:
        gameCallback->OnChar(wparam);
        return false;

    case WM_DESTROY:
        ExitProcess(GetLastError());
        //  SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
        //  PostQuitMessage(0);
        //  return 0;

    case WM_COMMAND:
        if (OnOSMenu(wparam)) {
            return *result = 0, true;
        }
        return false;

    case WM_LBUTTONDOWN: {
        gameCallback->OnMouseLeftClick(LOWORD(lparam), HIWORD(lparam));
    }
    return false;

    case WM_RBUTTONDOWN: {
        gameCallback->OnMouseRightClick(LOWORD(lparam), HIWORD(lparam));
    }
    return false;

    case WM_LBUTTONUP:
        gameCallback->OnMouseLeftUp();
        return false;

    case WM_RBUTTONUP:
        gameCallback->OnMouseRightUp();
        return false;

    case WM_LBUTTONDBLCLK: {
        gameCallback->OnMouseLeftDoubleClick(LOWORD(lparam), HIWORD(lparam));
    }
    return false;

    case WM_RBUTTONDBLCLK: {
        gameCallback->OnMouseRightDoubleClick(LOWORD(lparam), HIWORD(lparam));
    }
    return false;

    case WM_MOUSEMOVE:
        gameCallback->OnMouseMove(LOWORD(lparam), HIWORD(lparam), wparam & MK_LBUTTON, wparam & MK_RBUTTON);
        return false;

    case WM_SYSCOMMAND:
        if (wparam == SC_SCREENSAVE || wparam == SC_MONITORPOWER)
            return *result = 0, true;
        return false;

    case WM_KEYDOWN:
        gameCallback->OnVkDown(wparam, MapVirtualKeyW(wparam, MAPVK_VK_TO_CHAR));
        return *result = 0, true;

    case WM_ACTIVATEAPP:
        if (wparam && (GetForegroundWindow() == hwnd)) {
            gameCallback->OnActivated();
        } else {
            gameCallback->OnDeactivated();
        }
        return *result = 0, true;

    case WM_SETFOCUS:
        gameCallback->OnFocus();
        return false;

    case WM_KILLFOCUS:
        gameCallback->OnFocusLost();
        return false;

    case WM_PAINT:
        if (!hwnd) return false;

        if (!GetUpdateRect(hwnd, 0, 0)) {
            // || !dword_4E98BC_bApplicationActive &&
                 // !render->bWindowMode )
            return *result = 0, true;
        }

        PAINTSTRUCT Paint;
        BeginPaint(hwnd, &Paint);

        gameCallback->OnPaint();

        EndPaint(hwnd, &Paint);
        return *result = 0, true;
    }
    return *result = 0, false;
}

void *WinApiMsgRouter(HWND hwnd, int msg, int wparam, void *lparam) {
    if (msg == WM_NCCREATE) {
        CREATESTRUCTA *cs = (CREATESTRUCTA *)(lparam);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
        return (void *)DefWindowProcW(hwnd, msg, wparam, (LPARAM)lparam);
    }

    auto window = (WinApiWindow *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    if (window) {
        void *result;
        if (window->WinApiMessageProc(msg, wparam, lparam, &result))
            return result;
    }
    return (void *)DefWindowProcW(hwnd, msg, (WPARAM)wparam, (LPARAM)lparam);
}

void WinApiWindow::Show() {
    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);
}

void WinApiWindow::SetCursor(const char *cursor_name) {
    POINT cursor_pos;
    GetCursorPos(&cursor_pos);

    if (!strcmp(cursor_name, "MICON1")) {
        // SetClassLongPtrW(api_handle, GCLP_HCURSOR,
        // (LONG)LoadCursorW(GetModuleHandleW(nullptr), IDC_ARROW));
        SetClassLongPtrW(hwnd, GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_ARROW));
    } else if (!strcmp(cursor_name, "MICON2")) {
        // HCURSOR hCurs1;

        // Create target

        if (false) {
            MessageBoxA(nullptr,
                "Ritor1: original cursor(Target) isn't loading", "", 0);
            __debugbreak();
        }
        SetClassLongPtrW(hwnd, GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_CROSS));
    } else if (!strcmp(cursor_name, "MICON3")) {
        SetClassLongPtrW(hwnd, GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_WAIT));
    }

    // ClientToScreen(api_handle, &cursor_pos); //???
    SetCursorPos(cursor_pos.x, cursor_pos.y);
}

void WinApiWindow::SetFullscreenMode() {
    // Ritor1: Error. Window size changed in this
    // function when switching to fullscrean mode
    SetMenu(hwnd, nullptr);  // 640,480 - 640,500
                             /*bool m = false;
                             HMENU h_menu = GetMenu(api_handle);
                             m = DestroyMenu(h_menu);//как вариант - удалить меню*/

    SetWindowLongW(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST);
    SetWindowLongW(hwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);  // 648, 534

    SetWindowPos(hwnd, HWND_TOP, 0, 0, -1, -1, SWP_NOSIZE | SWP_SHOWWINDOW);
}

void WinApiWindow::SetWindowedMode(int new_window_width, int new_window_height) {
    RECT rcWindowPos;
    GetWindowRect(hwnd, &rcWindowPos);

    SetWindowLongW(hwnd, GWL_EXSTYLE, 0);
    SetWindowLongW(hwnd, GWL_STYLE, WS_VISIBLE | WS_OVERLAPPEDWINDOW);
    SetWindowPos(hwnd, HWND_TOP, 0, 0, -1, -1, 0);

    RECT rcWindow;
    GetWindowRect(hwnd, &rcWindow);

    RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    int window_borders_width = (rcWindow.right - rcWindow.left) - (rcClient.right - rcClient.left);
    int window_borders_height = (rcWindow.bottom - rcWindow.top) - (rcClient.bottom - rcClient.top);
    int window_total_width = new_window_width + window_borders_width;
    int window_total_height = new_window_height + window_borders_height;

#ifdef _DEBUG
    if (!GetMenu(hwnd)) {
        window_total_height += GetSystemMetrics(SM_CYMENU);
    }
#endif

    MoveWindow(hwnd, rcWindowPos.left, rcWindowPos.top, window_total_width, window_total_height, 0);

#ifdef _DEBUG
    static HMENU debug_menu = (HMENU)CreateDebugMenuPanel();
    SetMenu(hwnd, debug_menu);

    GetWindowRect(hwnd, &rcWindow);
    GetClientRect(hwnd, &rcClient);
#endif
}

void *WinApiWindow::CreateDebugMenuPanel() {
    HMENU menu = CreateMenu();
    {
        HMENU file = CreatePopupMenu();
        AppendMenuW(menu, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)file,
            L"&File");
        {
            AppendMenuW(file, MF_ENABLED | MF_STRING, 40001, L"Exit");
            AppendMenuW(file, MF_ENABLED | MF_STRING, 40002,
                L"Take Screenshot");
        }

        HMENU debug = CreatePopupMenu();
        AppendMenuW(menu, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)debug,
            L"&Debug");
        {
            HMENU debug_party = CreatePopupMenu();
            AppendMenuW(debug, MF_ENABLED | MF_STRING | MF_POPUP,
                (UINT_PTR)debug_party, L"&Party");
            {
                AppendMenuW(debug_party, MF_ENABLED | MF_STRING, 40007,
                    L"Give Gold (10 000)");
                AppendMenuW(debug_party, MF_ENABLED | MF_STRING, 40008,
                    L"Give Exp (20 000)");
                AppendMenuW(debug_party, MF_ENABLED | MF_STRING, 40059,
                    L"Give Skills (50 each)");
                AppendMenuW(debug_party, MF_ENABLED | MF_STRING, 40060,
                    L"Learn Skills");
                AppendMenuW(debug_party, MF_ENABLED | MF_STRING, 40013,
                    L"Remove Gold");

                HMENU debug_party_setconditions = CreatePopupMenu();
                AppendMenuW(debug_party, MF_ENABLED | MF_STRING | MF_POPUP,
                    (UINT_PTR)debug_party_setconditions,
                    L"Set Condition");
                {
                    AppendMenuW(debug_party_setconditions, MF_ENABLED | MF_STRING, 40044, L"Afraid");
                    AppendMenuW(debug_party_setconditions, MF_ENABLED | MF_STRING, 40043, L"Asleep");
                    AppendMenuW(debug_party_setconditions, MF_ENABLED | MF_STRING, 40037, L"Curse");
                    AppendMenuW(debug_party_setconditions, MF_ENABLED | MF_STRING, 40036, L"Disease Weak");
                    AppendMenuW(debug_party_setconditions, MF_ENABLED | MF_STRING, 40035, L"Disease Medium");
                    AppendMenuW(debug_party_setconditions, MF_ENABLED | MF_STRING, 40034, L"Disease Severe");
                    AppendMenuW(debug_party_setconditions, MF_ENABLED | MF_STRING, 40041, L"Dead");
                    AppendMenuW(debug_party_setconditions, MF_ENABLED | MF_STRING, 40039, L"Drunk");
                    AppendMenuW(debug_party_setconditions, MF_ENABLED | MF_STRING, 40042, L"Eradicated");
                    AppendMenuW(debug_party_setconditions, MF_ENABLED | MF_STRING, 40038, L"Insane");
                    AppendMenuW(debug_party_setconditions, MF_ENABLED | MF_STRING, 40045, L"Paralyzed");
                    AppendMenuW(debug_party_setconditions, MF_ENABLED | MF_STRING, 40033, L"Poison Weak");
                    AppendMenuW(debug_party_setconditions, MF_ENABLED | MF_STRING, 40032, L"Poison Medium");
                    AppendMenuW(debug_party_setconditions, MF_ENABLED | MF_STRING, 40031, L"Poison Severe");
                    AppendMenuW(debug_party_setconditions, MF_ENABLED | MF_STRING, 40029, L"&Stone");
                    AppendMenuW(debug_party_setconditions, MF_ENABLED | MF_STRING, 40040, L"Unconscious");
                    AppendMenuW(debug_party_setconditions, MF_ENABLED | MF_STRING, 40030, L"Weak");
                    AppendMenuW(debug_party_setconditions, MF_ENABLED | MF_STRING, 40073, L"Zombie");
                    AppendMenuW(debug_party_setconditions, MF_ENABLED | MF_STRING, 40046, L"Good");
                }

                AppendMenuW(debug_party, MF_ENABLED | MF_STRING, 40006, L"Set Food (20)");

                HMENU debug_party_alignment = CreatePopupMenu();
                AppendMenuW(debug_party, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)debug_party_alignment, L"Alignment");
                {
                    AppendMenuW(debug_party_alignment, MF_ENABLED | MF_STRING, 40062, L"Good");
                    AppendMenuW(debug_party_alignment, MF_ENABLED | MF_STRING | MF_CHECKED, 40063, L"Neutral");
                    AppendMenuW(debug_party_alignment, MF_ENABLED | MF_STRING, 40064, L"Evil");
                }
            }
            AppendMenuW(debug, MF_ENABLED | MF_STRING, 40122, L"Actors off");
            HMENU debug_time = CreatePopupMenu();
            AppendMenuW(debug, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)debug_time, L"&Time");
            {
                AppendMenuW(debug_time, MF_ENABLED | MF_STRING, 40009, L"Add 1 Day");
                AppendMenuW(debug_time, MF_ENABLED | MF_STRING, 40010, L"Add 1 Week");
                AppendMenuW(debug_time, MF_ENABLED | MF_STRING, 40011, L"Add 1 Month");
                AppendMenuW(debug_time, MF_ENABLED | MF_STRING, 40012, L"Add 1 Year");
            }

            HMENU debug_items = CreatePopupMenu();
            AppendMenuW(debug, MF_ENABLED | MF_STRING | MF_POPUP,
                (UINT_PTR)debug_items, L"&Items");
            {
                AppendMenuW(debug_items, MF_ENABLED | MF_STRING, 40015, L"Generate level &1 item");
                AppendMenuW(debug_items, MF_ENABLED | MF_STRING, 40016, L"Generate level &2 item");
                AppendMenuW(debug_items, MF_ENABLED | MF_STRING, 40017, L"Generate level &3 item");
                AppendMenuW(debug_items, MF_ENABLED | MF_STRING, 40018, L"Generate level &4 item");
                AppendMenuW(debug_items, MF_ENABLED | MF_STRING, 40019, L"Generate level &5 item");
                AppendMenuW(debug_items, MF_ENABLED | MF_STRING, 40020, L"Generate level &6 item");
                AppendMenuW(debug_items, MF_ENABLED | MF_STRING, 40061, L"Generate special item");
            }

            HMENU debug_graphics = CreatePopupMenu();
            AppendMenuW(debug, MF_ENABLED | MF_STRING | MF_POPUP,
                (UINT_PTR)debug_graphics, L"&Graphics");
            {
                HMENU lights_off = CreatePopupMenu();
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_POPUP,
                    (UINT_PTR)lights_off, L"Lights");  // 40104
                {
                    AppendMenuW(lights_off, MF_ENABLED | MF_STRING, 40123, L"Lights on");
                    AppendMenuW(lights_off, MF_ENABLED | MF_STRING, 40124, L"Lights off");
                }

                HMENU debug_lights = CreatePopupMenu();
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_POPUP,
                    (UINT_PTR)debug_lights, L"Debug Lights");  // 40104
                {
                    AppendMenuW(debug_lights, MF_ENABLED | MF_STRING, 40125, L"Debug lights on");
                    AppendMenuW(debug_lights, MF_ENABLED | MF_STRING, 40126, L"Debug lights off");
                }

                HMENU debug_terrain = CreatePopupMenu();
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_POPUP,
                    (UINT_PTR)debug_terrain,
                    L"Debug Terrain");  // 40104
                {
                    AppendMenuW(debug_terrain, MF_ENABLED | MF_STRING, 40127, L"Debug Terrain on");
                    AppendMenuW(debug_terrain, MF_ENABLED | MF_STRING, 40128, L"Debug Terrain off");
                }
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_GRAYED, 40023, L"Lighting Mode");
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_GRAYED, 40024, L"Lighting Geometry");

                // AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING/* |
                // MF_GRAYED*/, 40123, L"Lights Off");
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_GRAYED, 40105, L"Colored Lights");
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_GRAYED, 40101, L"Debug Decals");
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_GRAYED, 40027, L"HWID Portals");
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_GRAYED, 40047, L"SWID Portals");
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_GRAYED, 40051, L"OD Frustum");
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_GRAYED, 40054, L"SWOD Constant Redraw");
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_GRAYED, 40055, L"SWOD Lit Rasterizer");
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_GRAYED, 40056, L"Party Light off");
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_GRAYED, 40060, L"SWOD Nice Lighting off");
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_GRAYED, 40067, L"HWOD Additive Fog Lights");
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_GRAYED, 40072, L"HWID Nice Lighting");
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_GRAYED, 40048, L"Wireframe");
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_GRAYED, 40049, L"Fog");
            }

            HMENU debug_misc = CreatePopupMenu();
            AppendMenuW(debug, MF_ENABLED | MF_STRING | MF_POPUP,
                (UINT_PTR)debug_misc, L"&Misc");
            {
                AppendMenuW(debug_misc, MF_ENABLED | MF_STRING | MF_GRAYED, 40066, L"Object Viewcone Culling");
                AppendMenuW(debug_misc, MF_ENABLED | MF_STRING | MF_GRAYED, 40068, L"Red Tint");
                AppendMenuW(debug_misc, MF_ENABLED | MF_STRING | MF_GRAYED, 40071, L"Display Secrets");
                AppendMenuW(debug_misc, MF_ENABLED | MF_STRING | MF_GRAYED, 40102, L"Massive Bloodsplat");
                AppendMenuW(debug_misc, MF_ENABLED | MF_STRING | MF_GRAYED, 40103, L"Underwater Gravity");
            }

            HMENU debug_eax = CreatePopupMenu();
            AppendMenuW(debug, MF_ENABLED | MF_STRING | MF_POPUP,
                (UINT_PTR)debug_eax, L"EAX Environs");
            {
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40074, L"NONE");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40075, L"GENERIC");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40076, L"PADDEDCELL");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40077, L"ROOM");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40078, L"BATHROOM");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40079, L"LIVINGROOM");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40080, L"STONEROOM");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40081, L"AUDITORIUM");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40082, L"CONCERTHALL");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40083, L"CAVE");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40084, L"ARENA");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40085, L"HANGAR");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40086, L"CARPETEDHALLWAY");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40087, L"HALLWAY");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40088, L"STONECORRIDOR");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40089, L"ALLEY");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40090, L"FOREST");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40091, L"CITY");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40092, L"MOUNTAINS");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40093, L"QUARRY");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40094, L"PLAIN");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40095, L"PARKINGLOT");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40096, L"SEWERPIPE");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40097, L"UNDERWATER");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40098, L"DRUGGED");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40099, L"DIZZY");
                AppendMenuW(debug_eax, MF_ENABLED | MF_STRING | MF_GRAYED, 40100, L"PSICHOTIC");
            }
        }
        HMENU other = CreatePopupMenu();
        AppendMenuW(menu, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)other,
            L"&Other");
        {
            HMENU other_wizard_eye = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP,
                (UINT_PTR)other_wizard_eye, L"Wizard eye");
            {
                AppendMenuW(other_wizard_eye, MF_ENABLED | MF_STRING, 40101, L"Wizard eye on");
                AppendMenuW(other_wizard_eye, MF_ENABLED | MF_STRING, 40102, L"Wizard eye off");
            }

            HMENU other_new_draw_object_dist = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP,
                (UINT_PTR)other_new_draw_object_dist,
                L"New draw object distance");
            {
                AppendMenuW(other_new_draw_object_dist, MF_ENABLED | MF_STRING, 40103, L"New draw object dist on");
                AppendMenuW(other_new_draw_object_dist, MF_ENABLED | MF_STRING, 40104, L"New draw object dist off");
            }

            HMENU other_change_seasons = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP,
                (UINT_PTR)other_change_seasons, L"Change seasons");
            {
                AppendMenuW(other_change_seasons, MF_ENABLED | MF_STRING, 40105, L"Change seasons on");
                AppendMenuW(other_change_seasons, MF_ENABLED | MF_STRING, 40106, L"Change seasons off");
            }

            HMENU other_all_magic = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP,
                (UINT_PTR)other_all_magic, L"All magic");
            {
                AppendMenuW(other_all_magic, MF_ENABLED | MF_STRING, 40107, L"All magic on");
                AppendMenuW(other_all_magic, MF_ENABLED | MF_STRING, 40108, L"All magic off");
            }

            HMENU other_debug_information = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP,
                (UINT_PTR)other_debug_information,
                L"Debug information");
            {
                AppendMenuW(other_debug_information, MF_ENABLED | MF_STRING, 40109, L"Debug information on");
                AppendMenuW(other_debug_information, MF_ENABLED | MF_STRING, 40110, L"Debug information off");
            }

            HMENU other_show_picked_face = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP,
                (UINT_PTR)other_show_picked_face, L"Show picked face");
            {
                AppendMenuW(other_show_picked_face, MF_ENABLED | MF_STRING, 40111, L"Show picked face on");
                AppendMenuW(other_show_picked_face, MF_ENABLED | MF_STRING, 40112, L"Show picked face off");
            }

            HMENU other_draw_portals_loops = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP,
                (UINT_PTR)other_draw_portals_loops,
                L"Draw portals loops");
            {
                AppendMenuW(other_draw_portals_loops, MF_ENABLED | MF_STRING, 40113, L"Draw portals loops on");
                AppendMenuW(other_draw_portals_loops, MF_ENABLED | MF_STRING, 40114, L"Draw portals loops off");
            }

            HMENU other_new_speed = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP,
                (UINT_PTR)other_new_speed, L"New_speed");
            {
                AppendMenuW(other_new_speed, MF_ENABLED | MF_STRING, 40115, L"New_speed on");
                AppendMenuW(other_new_speed, MF_ENABLED | MF_STRING, 40116, L"New_speed off");
            }

            HMENU other_snow = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP,
                (UINT_PTR)other_snow, L"Snow");
            {
                AppendMenuW(other_snow, MF_ENABLED | MF_STRING, 40117, L"Snowfall on");
                AppendMenuW(other_snow, MF_ENABLED | MF_STRING, 40118, L"Snowfall off");
            }

            HMENU other_draw_terrain_dist_mist = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP,
                (UINT_PTR)other_draw_terrain_dist_mist,
                L"New draw terrain distance");
            {
                AppendMenuW(other_draw_terrain_dist_mist, MF_ENABLED | MF_STRING, 40119, L"New draw terrain distance on");
                AppendMenuW(other_draw_terrain_dist_mist, MF_ENABLED | MF_STRING, 40120, L"New draw terrain distance off");
            }
        }
    }
    return menu;
}

bool WinApiWindow::OnOSMenu(int item_id) {
    switch (item_id) {
    default:
        return false;

    case 103:  __debugbreak();  // render->SavePCXScreenshot(); break;
    case 104:  __debugbreak();  // render->ToggleFullscreen(); break;
    case 101:    // Quit game
    case 40001:  // Menu "File"-> "Exit"
        engine->Deinitialize();
        SendMessageW(hwnd, WM_DESTROY, 0, 0);
        break;

    case 40002:  // Menu "File"-> "Take Screenshot"
        render->SavePCXScreenshot();
        break;

        /*case 104: //F4 button
        render->ChangeBetweenWinFullscreenModes();
        if (pArcomageGame->bGameInProgress)
        pArcomageGame->field_F6 = 1;
        break;*/

        // SubMenu "Party"
    case 40006:
        pParty->SetFood(pParty->uNumFoodRations + 20);
        break;
    case 40007:
        pParty->SetGold(pParty->uNumGold + 10000);
        break;
    case 40008:
        pParty->GivePartyExp(20000);
        break;
    case 40013:
        pParty->SetGold(0);
        break;

    case 40059:
        for (uint i = 0; i < 4; ++i) pParty->pPlayers[i].uSkillPoints += 50;
        break;

        // learn all skills for class
    case 40060:
        for (uint i = 1; i < 5; ++i) {            // loop over players
            for (int ski = 0; ski < 37; ++ski) {  // loop over skills
                if (byte_4ED970_skill_learn_ability_by_class_table[pPlayers[i]->classType][ski] > 0) {            // if class can learn this skill
                    switch (ski) {  // give skils
                    case 0:     // PLAYER_SKILL_STAFF = 0,
                        if (pPlayers[i]->skillStaff == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillStaff, 1);
                        break;
                    case 1:  // PLAYER_SKILL_SWORD = 1,
                        if (pPlayers[i]->skillSword == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillSword, 1);
                        break;
                    case 2:  // PLAYER_SKILL_DAGGER = 2,
                        if (pPlayers[i]->skillDagger == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillDagger, 1);
                        break;
                    case 3:  // PLAYER_SKILL_AXE = 3,
                        if (pPlayers[i]->skillAxe == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillAxe, 1);
                        break;
                    case 4:  // PLAYER_SKILL_SPEAR = 4,
                        if (pPlayers[i]->skillSpear == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillSpear, 1);
                        break;
                    case 5:  // PLAYER_SKILL_BOW = 5,
                        if (pPlayers[i]->skillBow == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillBow, 1);
                        break;
                    case 6:  // PLAYER_SKILL_MACE = 6,
                        if (pPlayers[i]->skillMace == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillMace, 1);
                        break;
                    case 7:  // PLAYER_SKILL_BLASTER = 7,
                        if (pPlayers[i]->skillBlaster == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillBlaster, 1);
                        break;
                    case 8:  // PLAYER_SKILL_SHIELD = 8,
                        if (pPlayers[i]->skillShield == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillShield, 1);
                        break;
                    case 9:  // PLAYER_SKILL_LEATHER = 9,
                        if (pPlayers[i]->skillLeather == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillLeather, 1);
                        break;
                    case 10:  // PLAYER_SKILL_CHAIN = 10,
                        if (pPlayers[i]->skillChain == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillChain, 1);
                        break;
                    case 11:  // PLAYER_SKILL_PLATE = 11,
                        if (pPlayers[i]->skillPlate == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillPlate, 1);
                        break;
                    case 12:  // PLAYER_SKILL_FIRE = 12,
                        if (pPlayers[i]->skillFire == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillFire, 1);
                        break;
                    case 13:  // PLAYER_SKILL_AIR = 13,
                        if (pPlayers[i]->skillAir == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillAir, 1);
                        break;
                    case 14:  // PLAYER_SKILL_WATER = 14,
                        if (pPlayers[i]->skillWater == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillWater, 1);
                        break;
                    case 15:  // PLAYER_SKILL_EARTH = 15,
                        if (pPlayers[i]->skillEarth == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillEarth, 1);
                        break;
                    case 16:  // PLAYER_SKILL_SPIRIT = 16,
                        if (pPlayers[i]->skillSpirit == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillSpirit, 1);
                        break;
                    case 17:  // PLAYER_SKILL_MIND = 17,
                        if (pPlayers[i]->skillMind == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillMind, 1);
                        break;
                    case 18:  // PLAYER_SKILL_BODY = 18,
                        if (pPlayers[i]->skillBody == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillBody, 1);
                        break;
                    case 19:  // PLAYER_SKILL_LIGHT = 19,
                        if (pPlayers[i]->skillLight == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillLight, 1);
                        break;
                    case 20:  // PLAYER_SKILL_DARK = 20,
                        if (pPlayers[i]->skillDark == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillDark, 1);
                        break;
                    case 21:  // PLAYER_SKILL_ITEM_ID = 21,
                        if (pPlayers[i]->skillItemId == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillItemId, 1);
                        break;
                    case 22:  // PLAYER_SKILL_MERCHANT = 22,
                        if (pPlayers[i]->skillMerchant == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillMerchant, 1);
                        break;
                    case 23:  // PLAYER_SKILL_REPAIR = 23,
                        if (pPlayers[i]->skillRepair == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillRepair, 1);
                        break;
                    case 24:  // PLAYER_SKILL_BODYBUILDING = 24,
                        if (pPlayers[i]->skillBodybuilding == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillBodybuilding, 1);
                        break;
                    case 25:  // PLAYER_SKILL_MEDITATION = 25,
                        if (pPlayers[i]->skillMeditation == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillMeditation, 1);
                        break;
                    case 26:  // PLAYER_SKILL_PERCEPTION = 26,
                        if (pPlayers[i]->skillPerception == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillPerception, 1);
                        break;
                    case 27:  // PLAYER_SKILL_DIPLOMACY = 27,
                        break;
                    case 28:  // PLAYER_SKILL_TIEVERY = 28,
                        break;
                    case 29:  // PLAYER_SKILL_TRAP_DISARM = 29,
                        if (pPlayers[i]->skillDisarmTrap == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillDisarmTrap, 1);
                        break;
                    case 30:  // PLAYER_SKILL_DODGE = 30,
                        if (pPlayers[i]->skillDodge == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillDodge, 1);
                        break;
                    case 31:  // PLAYER_SKILL_UNARMED = 31,
                        if (pPlayers[i]->skillUnarmed == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillUnarmed, 1);
                        break;
                    case 32:  // PLAYER_SKILL_MONSTER_ID = 32,
                        if (pPlayers[i]->skillMonsterId == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillMonsterId, 1);
                        break;
                    case 33:  // PLAYER_SKILL_ARMSMASTER = 33,
                        if (pPlayers[i]->skillArmsmaster == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillArmsmaster, 1);
                        break;
                    case 34:  // PLAYER_SKILL_STEALING = 34,
                        if (pPlayers[i]->skillStealing == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillStealing, 1);
                        break;
                    case 35:  // PLAYER_SKILL_ALCHEMY = 35,
                        if (pPlayers[i]->skillAlchemy == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillAlchemy, 1);
                        break;
                    case 36:  // PLAYER_SKILL_LEARNING = 36,
                        if (pPlayers[i]->skillLearning == 0)
                            pPlayers[i]->AddSkillByEvent(
                                &Player::skillLearning, 1);
                        break;

                        // PLAYER_SKILL_CLUB = 37,
                        // PLAYER_SKILL_MISC = 38,
                        // PLAYER_SKILL_INVALID = -1
                    }
                }
            }
        }
        break;

    case 40029:
        pPlayers[uActiveCharacter]->SetPertified(pParty->GetPlayingTime());
        break;
    case 40030:
        pPlayers[uActiveCharacter]->SetWeak(pParty->GetPlayingTime());
        break;
    case 40031:
        pPlayers[uActiveCharacter]->SetPoisonSevere(
            pParty->GetPlayingTime());
        break;
    case 40032:
        pPlayers[uActiveCharacter]->SetPoisonMedium(
            pParty->GetPlayingTime());
        break;
    case 40033:
        pPlayers[uActiveCharacter]->SetPoisonWeak(
            pParty->GetPlayingTime());
        break;
    case 40034:
        pPlayers[uActiveCharacter]->SetDiseaseSevere(
            pParty->GetPlayingTime());
        break;
    case 40035:
        pPlayers[uActiveCharacter]->SetDiseaseMedium(
            pParty->GetPlayingTime());
        break;
    case 40036:
        pPlayers[uActiveCharacter]->SetDiseaseWeak(
            pParty->GetPlayingTime());
        break;
    case 40037:
        pPlayers[uActiveCharacter]->SetCursed(pParty->GetPlayingTime());
        break;
    case 40038:
        pPlayers[uActiveCharacter]->SetInsane(pParty->GetPlayingTime());
        break;
    case 40039:
        pPlayers[uActiveCharacter]->SetDrunk(pParty->GetPlayingTime());
        break;
    case 40040:
        pPlayers[uActiveCharacter]->SetUnconcious(pParty->GetPlayingTime());
        break;
    case 40041:
        pPlayers[uActiveCharacter]->SetDead(pParty->GetPlayingTime());
        break;
    case 40042:
        pPlayers[uActiveCharacter]->SetEradicated(pParty->GetPlayingTime());
        break;
    case 40043:
        pPlayers[uActiveCharacter]->SetAsleep(pParty->GetPlayingTime());
        break;
    case 40044:
        pPlayers[uActiveCharacter]->SetAfraid(pParty->GetPlayingTime());
        break;
    case 40045:
        pPlayers[uActiveCharacter]->SetParalyzed(pParty->GetPlayingTime());
        break;
    case 40073:
        pPlayers[uActiveCharacter]->SetZombie(pParty->GetPlayingTime());
        break;
    case 40046:
        pPlayers[uActiveCharacter]->conditions_times.fill(GameTime(0));
        pPlayers[uActiveCharacter]->sHealth =
            pPlayers[uActiveCharacter]->GetMaxHealth();
        pPlayers[uActiveCharacter]->sMana =
            pPlayers[uActiveCharacter]->GetMaxMana();
        break;
        // 40046

    case 40062:
        pParty->alignment = PartyAlignment_Good;
        SetUserInterface(pParty->alignment, true);
        break;
    case 40063:
        pParty->alignment = PartyAlignment_Neutral;
        SetUserInterface(pParty->alignment, true);
        break;
    case 40064:
        pParty->alignment = PartyAlignment_Evil;
        SetUserInterface(pParty->alignment, true);
        break;

        // SubMenu "Time"
    case 40009:
        pParty->GetPlayingTime() += GameTime::FromDays(1);
        break;
    case 40010:
        pParty->GetPlayingTime().value += Timer::Week;
        break;
    case 40011:
        pParty->GetPlayingTime().value += Timer::Month;
        break;
    case 40012:
        pParty->GetPlayingTime() += GameTime::FromYears(1);
        break;

        // SubMenu "Items"
    case 40015:  // uItemID_Rep_St == 1
    case 40016:  // uItemID_Rep_St == 2
    case 40017:  // uItemID_Rep_St == 3
    case 40018:  // uItemID_Rep_St == 4
    case 40019:  // uItemID_Rep_St == 5
    case 40020:  // uItemID_Rep_St == 6
    {
        int pItemID = rand() % 500;
        for (uint i = 0; i < 500; ++i) {
            if (pItemID + i > 499) pItemID = 0;
            if (pItemsTable->pItems[pItemID + i].uItemID_Rep_St ==
                (item_id - 40015 + 1)) {
                pPlayers[uActiveCharacter]->AddItem(-1, pItemID + i);
                break;
            }
        }
    } break;

    case 40061: {
        int pItemID = rand() % 500;
        for (uint i = 0; i < 500; ++i) {
            if (pItemID + i > 499) pItemID = 0;
            if (pItemsTable->pItems[pItemID + i].uItemID_Rep_St > 6) {
                pPlayers[uActiveCharacter]->AddItem(-1, pItemID + i);
                break;
            }
        }
    } break;

        // SubMenu "Other"
    case 40101:
        engine->SetDebugWizardEye(true);
        break;
    case 40102:
        engine->SetDebugWizardEye(false);
        break;
    case 40103:
        pODMRenderParams->far_clip = 0x6000;
        break;
    case 40104:
        pODMRenderParams->far_clip = 0x2000;
        break;
    case 40105:
        engine->SetSeasonsChange(true);
        break;
    case 40106:
        engine->SetSeasonsChange(false);
        break;
    case 40107:
        engine->SetDebugAllMagic(true);
        break;  // may need to close and reopen spellbook when changed??
    case 40108:
        engine->SetDebugAllMagic(false);
        break;  // may need to close and reopen spellbook when changed??
    case 40109:
        engine->SetDebugShowFps(true);
        break;
    case 40110:
        engine->SetDebugShowFps(false);
        break;
    case 40111:
        engine->SetDebugShowPickedFace(true);
        break;
    case 40112:
        engine->SetDebugShowPickedFace(false);
        break;
    case 40113:
        engine->SetDebugPortalOutlines(true);
        break;
    case 40114:
        engine->SetDebugPortalOutlines(false);
        break;
    case 40115:
        engine->SetDebugTurboSpeed(true);
        break;
    case 40116:
        engine->SetDebugTurboSpeed(false);
        break;
    case 40117:
        engine->SetAllowSnow(true);
        break;
    case 40118:
        engine->SetAllowSnow(false);
        break;
    case 40119:
        engine->SetExtendedDrawDistance(true);
        break;
    case 40120:
        engine->SetExtendedDrawDistance(false);
        break;
    case 40122:
        engine->SetNoActors(true);
        break;
    case 40123:
        engine->SetAllowLightmaps(true);
        break;
    case 40124:
        engine->SetAllowLightmaps(false);
        break;
    case 40125:
        engine->SetDebugLightmapsDecals(true);
        break;
    case 40126:
        engine->SetDebugLightmapsDecals(false);
        break;
    case 40127:
        engine->SetDebugTerrain(true);
        break;
    case 40128:
        engine->SetDebugTerrain(false);
        break;
    }

    return true;
}
