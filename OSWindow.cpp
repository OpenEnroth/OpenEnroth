#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <crtdbg.h>

#include "OSWindow.h"

#include "Engine/Engine.h"

#include "Arcomage\Arcomage.h"
#include "Media/Audio/AudioPlayer.h"
#include "IO/Mouse.h"
#include "Engine/Timer.h"
#include "GUI/GUIWindow.h"
#include "Engine/Party.h"
#include "Engine/Graphics/IndoorCameraD3D.h"
#include "IO/Keyboard.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Vis.h"
#include "Media/Audio/AIL.h"
#include "Engine/MMT.h"


bool wizard_eye = false;          // wizard eye always on / включить на постоянно око чародея
bool change_seasons = true;      // toggle seasons change / смена времён года
bool all_magic = false;           // toggle full spellbook / включить всю магию
bool debug_information = false;   // toggle debug info / информация fps, положение группы, уровень пола и т.п.
bool show_picked_face = false;    // highlight picked face / выделить активный фейс
bool draw_portals_loops = false;  // show portal frames / видны рамки порталов
bool new_speed = false;
bool bSnow = true;
bool draw_terrain_dist_mist = false;//новая дальность отрисовки тайлов
bool no_actors = false;           // remove all monsters / убрать всех монстров
bool lights_flag = true;
bool debug_lights = false;
bool debug_terrain_polygin = false;

bool OSWindow::OnMouseLeftClick(int x, int y)
{
    if (UIControl::OnMouseLeftClick(x, y))
        return true;

    //if (pMediaPlayer->bPlaying_Movie)
      //pMediaPlayer->bPlaying_Movie = false;

    pMouse->SetMouseClick(x, y);

    if (GetCurrentMenuID() == MENU_CREATEPARTY)
        Mouse::UI_OnKeyDown(VK_SELECT);

    if (pEngine)
        pEngine->PickMouse(512.0, x, y, false, &vis_sprite_filter_3, &vis_door_filter);

    Mouse::UI_OnMouseLeftClick(0);
    return true;
}

bool OSWindow::OnMouseRightClick(int x, int y)
{
    if (UIControl::OnMouseRightClick(x, y))
        return true;

    if (pMediaPlayer->bPlaying_Movie)
        pMediaPlayer->bPlaying_Movie = false;

    pMouse->SetMouseClick(x, y);

    if (pEngine)
        pEngine->PickMouse(pIndoorCameraD3D->GetPickDepth(), x, y, 0, &vis_sprite_filter_2, &vis_door_filter);

    UI_OnMouseRightClick(0);
    return true;
}


bool _507B98_ctrl_pressed = false;
bool OSWindow::WinApiMessageProc(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT *result)
{
    switch (msg)
    {
    case WM_KEYUP:
    {
        if (wparam == VK_CONTROL)
            _507B98_ctrl_pressed = false;
        if (wparam == VK_SNAPSHOT)
            pRenderer->SavePCXScreenshot();

        OnKey(wparam);
        return *result = 0, true;
    }

    case WM_SIZING: return *result = 1, true;
    case WM_WINDOWPOSCHANGED:
        //if (pVideoPlayer && pVideoPlayer->AnyMovieLoaded() && pVideoPlayer->pBinkBuffer)
          //BinkBufferSetOffset(pVideoPlayer->pBinkBuffer, 0, 0);
        return false;

    case WM_CHAR:
        if (!pKeyActionMap->ProcessTextInput(wparam) && !viewparams->field_4C)
            GUI_HandleHotkey(wparam);
        return false;

    case WM_DESTROY:
        ExitProcess(GetLastError());
        //  SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
        //  PostQuitMessage(0);
        //  return 0;

        //case WM_DEVICECHANGE:
        //{
        //  if (wParam == 0x8000)          // CD or some device has been inserted - notify InsertCD dialog
        //    PostMessageW(hInsertCDWindow, WM_USER + 1, 0, 0);
        //  return 0;
        //}

    case WM_COMMAND:
        if (OnOSMenu(wparam))
            return *result = 0, true;
        return false;

    case WM_LBUTTONDOWN:
    {
        if (pArcomageGame->bGameInProgress)
        {
            pArcomageGame->stru1.field_0 = 7;
            ArcomageGame::OnMouseClick(0, true);
            return false;
        }

        OnMouseLeftClick(LOWORD(lparam), HIWORD(lparam));
    }
    return false;


    case WM_RBUTTONDOWN:
    {
        if (pArcomageGame->bGameInProgress)
        {
            pArcomageGame->stru1.field_0 = 8;
            ArcomageGame::OnMouseClick(1, true);
            return false;
        }

        OnMouseRightClick(LOWORD(lparam), HIWORD(lparam));
    }
    return false;


    case WM_LBUTTONUP:
        if (!pArcomageGame->bGameInProgress)
        {
            back_to_game();
            return false;
        }
        pArcomageGame->stru1.field_0 = 3;
        ArcomageGame::OnMouseClick(0, 0);
        return false;

    case WM_RBUTTONUP:
        if (!pArcomageGame->bGameInProgress)
        {
            back_to_game();
            return false;
        }
        pArcomageGame->stru1.field_0 = 4;
        ArcomageGame::OnMouseClick(1, false);
        return false;

    case WM_LBUTTONDBLCLK:
    {
        if (pArcomageGame->bGameInProgress)
        {
            pArcomageGame->stru1.field_0 = 7;
            return false;
        }

        OnMouseLeftClick(LOWORD(lparam), HIWORD(lparam));
    }
    return false;

    case WM_RBUTTONDBLCLK:
    {
        if (pArcomageGame->bGameInProgress)
        {
            pArcomageGame->stru1.field_0 = 8;
            return false;
        }

        OnMouseRightClick(LOWORD(lparam), HIWORD(lparam));
    }
    return false;

    /*case WM_MBUTTONDOWN:
      if (pRenderer->pRenderD3D && pEngine)
      {
        pEngine->PickMouse(pIndoorCameraD3D->GetPickDepth(), LOWORD(lParam), HIWORD(lParam), 1, &vis_sprite_filter_3, &vis_face_filter);
      }
      return false;*/

    case WM_MOUSEMOVE:
        if (pArcomageGame->bGameInProgress)
        {
            ArcomageGame::OnMouseMove(LOWORD(lparam), HIWORD(lparam));
            ArcomageGame::OnMouseClick(0, wparam == MK_LBUTTON);
            ArcomageGame::OnMouseClick(1, wparam == MK_RBUTTON);
        }
        else if (pMouse)
            pMouse->SetMouseClick(LOWORD(lparam), HIWORD(lparam));
        return false;

    case WM_SYSCOMMAND:
        if (wparam == SC_SCREENSAVE || wparam == SC_MONITORPOWER)
            return *result = 0, true;
        return false;

    case WM_KEYDOWN:
        if (uGameMenuUI_CurentlySelectedKeyIdx != -1)
        {
            pKeyActionMap->ProcessTextInput(wparam);
            return false;
        }
        if (!pArcomageGame->bGameInProgress)
        {
            if (pMediaPlayer->bPlaying_Movie)
                pMediaPlayer->bPlaying_Movie = false;
            if (wparam == VK_RETURN)
            {
                if (!viewparams->field_4C)
                    Mouse::UI_OnKeyDown(wparam);
                return 0;
            }
            if (wparam == VK_CONTROL)
            {
                _507B98_ctrl_pressed = true;
                return false;
            }
            if (wparam == VK_ESCAPE)
            {
                pMessageQueue_50CBD0->AddGUIMessage(UIMSG_Escape, window_SpeakInHouse != 0, 0);
                return 0;
            }
            if (wparam <= VK_HOME)
                return 0;
            if (wparam > VK_DOWN)
            {
                if (wparam != VK_F4 || pMovie_Track)
                    return 0;

                // F4 - toggle fullscreen
                SendMessageW(api_handle, WM_COMMAND, 104, 0);
                return 0;
            }
            if (wparam >= VK_LEFT && wparam <= VK_DOWN)
            {
                if (current_screen_type != SCREEN_GAME && current_screen_type != SCREEN_MODAL_WINDOW)
                {
                    if (!viewparams->field_4C)
                        Mouse::UI_OnKeyDown(wparam);
                    return 0;
                }
            }
            if (current_screen_type != SCREEN_GAME && current_screen_type != SCREEN_MODAL_WINDOW)
                return 0;
        }

        pArcomageGame->stru1.field_0 = 1;

        set_stru1_field_8_InArcomage(MapVirtualKey(wparam, MAPVK_VK_TO_CHAR));
        if (wparam == 27)
        {
            pArcomageGame->GameOver = 1;
            pArcomageGame->field_F4 = 1;
            pArcomageGame->uGameWinner = 2;
            pArcomageGame->Victory_type = -2;
            return false;
        }
        if (wparam != 114)
        {
            if (wparam == 115 && !pMovie_Track)
                SendMessage(api_handle, WM_COMMAND, 104, 0);
            return false;
        }
        SendMessageW(api_handle, WM_COMMAND, 103, 0);
        return *result = 0, true;

    case WM_ACTIVATEAPP:
        if (wparam && (GetForegroundWindow() == api_handle || GetForegroundWindow() == hInsertCDWindow))
        {
            if (dword_6BE364_game_settings_1 & GAME_SETTINGS_APP_INACTIVE)
            {
                dword_4E98BC_bApplicationActive = 1;
                /*if ( pRenderer->bWindowMode )
                {
                  HDC hDC = GetDC(api_handle);
                  int bitsPerPixel = GetDeviceCaps(hDC, BITSPIXEL);
                  int planes = GetDeviceCaps(hDC, PLANES);
                  ReleaseDC(api_handle, hDC);
                  if (bitsPerPixel != 16 || planes != 1)
                    Error(L"%S", pGlobalTXT_LocalizationStrings[62]);
                }*/
                dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_APP_INACTIVE;

                if (pArcomageGame->bGameInProgress)
                {
                    pArcomageGame->field_F9 = 1;
                }
                else
                {
                    if (dword_6BE364_game_settings_1 & GAME_SETTINGS_0200_EVENT_TIMER)
                        dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_0200_EVENT_TIMER;
                    else
                        pEventTimer->Resume();
                    if (dword_6BE364_game_settings_1 & GAME_SETTINGS_0400_MISC_TIMER)
                        dword_6BE364_game_settings_1 &= ~GAME_SETTINGS_0400_MISC_TIMER;
                    else
                        pMiscTimer->Resume();

                    viewparams->bRedrawGameUI = true;
                    if (pMovie_Track)//pVideoPlayer->pSmackerMovie )
                    {
                        pRenderer->RestoreFrontBuffer();
                        pRenderer->RestoreBackBuffer();
                        //BackToHouseMenu();
                    }
                }
                if (!bGameoverLoop && !pMovie_Track)//continue an audio track
                {
                    if (use_music_folder)
                        alSourcePlay(mSourceID);
                    else if (pAudioPlayer->hAILRedbook)//!pVideoPlayer->pSmackerMovie )
                        AIL_redbook_resume(pAudioPlayer->hAILRedbook);
                }
            }
        }
        else
        {
            if (!(dword_6BE364_game_settings_1 & GAME_SETTINGS_APP_INACTIVE))
            {
                dword_4E98BC_bApplicationActive = 0;
                if (pMovie_Track)
                    pMediaPlayer->bPlaying_Movie = true;

                ClipCursor(0);
                dword_6BE364_game_settings_1 |= GAME_SETTINGS_APP_INACTIVE;
                if (pEventTimer->bPaused)
                    dword_6BE364_game_settings_1 |= GAME_SETTINGS_0200_EVENT_TIMER;
                else
                    pEventTimer->Pause();
                if (pMiscTimer->bPaused)
                    dword_6BE364_game_settings_1 |= GAME_SETTINGS_0400_MISC_TIMER;
                else
                    pMiscTimer->Pause();

                if (pAudioPlayer != nullptr)
                {
                    pAudioPlayer->StopChannels(-1, -1);//приостановка воспроизведения звуков при неактивном окне игры
                    if (use_music_folder)
                        alSourcePause(mSourceID);
                    else if (pAudioPlayer->hAILRedbook)
                        AIL_redbook_pause(pAudioPlayer->hAILRedbook);
                }
            }
        }
        return *result = 0, true;

    case WM_SETFOCUS:
        dword_4E98BC_bApplicationActive = 0;
        ClipCursor(0);
        return false;

    case WM_KILLFOCUS:
        dword_4E98BC_bApplicationActive = 1;
        return false;

    case WM_PAINT:
        if (!api_handle || !pRenderer)
            return false;

        if (!GetUpdateRect(api_handle, 0, 0))// || !dword_4E98BC_bApplicationActive && !pRenderer->bWindowMode )
            return *result = 0, true;

        PAINTSTRUCT Paint;
        BeginPaint(api_handle, &Paint);
        if (pArcomageGame->bGameInProgress)
        {
            pArcomageGame->field_F9 = 1;
        }
        if (pRenderer->AreRenderSurfacesOk())
        {
            pRenderer->Present();
            //EndPaint(api_handle, &Paint);
            //return *result = 0, true;
        }

        EndPaint(api_handle, &Paint);
        return *result = 0, true;

    }
    return *result = 0, false;
}



bool OSWindow::Initialize(const wchar_t *title, int window_width, int window_height)
{
  WNDCLASSEXW wcxw;
  wcxw.cbSize = sizeof(WNDCLASSEXW);
  wcxw.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  wcxw.lpfnWndProc = WinApiMsgRouter;
  wcxw.cbClsExtra = 0;
  wcxw.cbWndExtra = 0;
  wcxw.hInstance = GetModuleHandleW(nullptr);
  wcxw.hIcon = wcxw.hIconSm = LoadIcon(NULL, IDI_QUESTION);// nullptr;
  wcxw.hCursor = LoadCursor(NULL, IDC_WAIT);//nullptr;
  wcxw.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);//(COLOR_BACKGROUND + 1);
  wcxw.lpszMenuName = nullptr;
  wcxw.lpszClassName = L"M&MTrilogy";
  if (!RegisterClassExW(&wcxw))
    return false;

  api_handle = CreateWindowExW(NULL, wcxw.lpszClassName, title, 0,
                               ReadWindowsRegistryInt("window X", (GetSystemMetrics(SM_CXSCREEN) - window_width) / 2),
                               ReadWindowsRegistryInt("window Y", (GetSystemMetrics(SM_CYSCREEN) - window_height) / 2),
                               window_width, window_height, nullptr,
                               nullptr, wcxw.hInstance, this);
  if (!api_handle)
  {
    UnregisterClassW(wcxw.lpszClassName, wcxw.hInstance);
    return false;
  }

  SetWindowedMode(window_width, window_height);
  Show();

  SetActiveWindow(api_handle);

  MSG msg;
  while (PeekMessageW(&msg, api_handle, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }

  return true;
}




OSWindow *OSWindow::Create(const wchar_t *title, int window_width, int window_height)
{
  auto window = new OSWindow;
  if (window)
    if (!window->Initialize(title, window_width, window_height))
    {
      delete window;
      window = nullptr;
    }

  return window;
}



LRESULT CALLBACK OSWindow::WinApiMsgRouter(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (msg == WM_NCCREATE)
    {
        CREATESTRUCTA* cs = (CREATESTRUCTA *)(lparam);
        OSWindow* window = (OSWindow *)cs->lpCreateParams;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)window);
        return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    OSWindow* window = (OSWindow *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    if (window && window->api_handle == hwnd)	//Uninitialized memory access
    {
        LPARAM result;
        if (window->WinApiMessageProc(msg, wparam, lparam, &result))//Unhandled application exception
            return result;
    }
    return DefWindowProcW(hwnd, msg, (WPARAM)wparam, (LPARAM)lparam);
}

void OSWindow::Show()
{
    ShowWindow(api_handle, SW_SHOWNORMAL);
    UpdateWindow(api_handle);
}

void OSWindow::SetCursor(const char *cursor_name)
{
    POINT cursor_pos;
    GetCursorPos(&cursor_pos);

    if (!strcmp(cursor_name, "MICON1"))
        //SetClassLongPtrW(api_handle, GCLP_HCURSOR, (LONG)LoadCursorW(GetModuleHandleW(nullptr), IDC_ARROW));
        SetClassLongPtrW(api_handle, GCLP_HCURSOR, (LONG)LoadCursorW(NULL, IDC_ARROW));
    else if (!strcmp(cursor_name, "MICON2"))
    {
        //HCURSOR hCurs1;

        // Create target 

        //pMouse->uCursorTextureID = pIcons_LOD->LoadTexture(cursor_name, TEXTURE_16BIT_PALETTE);
        //hCurs1 = LoadCursor(NULL, L"Target");
        if (for_refactoring)
        {
            MessageBoxA(nullptr, "Ritor1: original cursor(Target) isn't loading", "", 0);
            __debugbreak();
        }
        SetClassLongPtrW(api_handle, GCLP_HCURSOR, (LONG)LoadCursorW(NULL, IDC_CROSS));

    }
    else if (!strcmp(cursor_name, "MICON3"))
        SetClassLongPtrW(api_handle, GCLP_HCURSOR, (LONG)LoadCursorW(NULL, IDC_WAIT));

    //ClientToScreen(api_handle, &cursor_pos); //???
    SetCursorPos(cursor_pos.x, cursor_pos.y);
}

void OSWindow::SetFullscreenMode()
{
    //Ritor1: Error. Window size change in this function(Ошибка. При переходе на полноэкранный режим размеры окна изменяются в этой функции)
    SetMenu(api_handle, nullptr);//640,480 - 640,500
    /*bool m = false;
    HMENU h_menu = GetMenu(api_handle);
    m = DestroyMenu(h_menu);//как вариант - удалить меню*/

    SetWindowLongW(api_handle, GWL_EXSTYLE, WS_EX_TOPMOST);
    SetWindowLongW(api_handle, GWL_STYLE, WS_VISIBLE | WS_POPUP);//648, 534

    SetWindowPos(api_handle, HWND_TOP, 0, 0, -1, -1, SWP_NOSIZE | SWP_SHOWWINDOW);
}

void OSWindow::SetWindowedMode(int new_window_width, int new_window_height)
{
    RECT rcWindowPos;
    GetWindowRect(api_handle, &rcWindowPos);

    SetWindowLongW(api_handle, GWL_EXSTYLE, 0);
    SetWindowLongW(api_handle, GWL_STYLE, WS_VISIBLE | WS_OVERLAPPEDWINDOW);
    SetWindowPos(api_handle, HWND_TOP, 0, 0, -1, -1, 0);

    RECT rcWindow;
    GetWindowRect(api_handle, &rcWindow);

    RECT rcClient;
    GetClientRect(api_handle, &rcClient);

    int window_borders_width = (rcWindow.right - rcWindow.left) - (rcClient.right - rcClient.left),
        window_borders_height = (rcWindow.bottom - rcWindow.top) - (rcClient.bottom - rcClient.top);
    int window_total_width = new_window_width + window_borders_width,
        window_total_height = new_window_height + window_borders_height;

    #ifdef _DEBUG
        if (!GetMenu(api_handle))
            window_total_height += GetSystemMetrics(SM_CYMENU);
    #endif

    MoveWindow(
        api_handle,
        rcWindowPos.left,
        rcWindowPos.top,
        window_total_width,
        window_total_height,
        0
    );

    #ifdef _DEBUG
        static HMENU debug_menu = CreateDebugMenuPanel();
        SetMenu(api_handle, debug_menu);

        GetWindowRect(api_handle, &rcWindow);
        GetClientRect(api_handle, &rcClient);
    #endif
}




HMENU OSWindow::CreateDebugMenuPanel()
{
    HMENU menu = CreateMenu();
    {
        HMENU file = CreatePopupMenu();
        AppendMenuW(menu, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)file, L"&File");
        {
            AppendMenuW(file, MF_ENABLED | MF_STRING, 40001, L"Exit");
        }

        HMENU debug = CreatePopupMenu();
        AppendMenuW(menu, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)debug, L"&Debug");
        {
            HMENU debug_party = CreatePopupMenu();
            AppendMenuW(debug, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)debug_party, L"&Party");
            {
                AppendMenuW(debug_party, MF_ENABLED | MF_STRING, 40007, L"Give Gold (10 000)");
                AppendMenuW(debug_party, MF_ENABLED | MF_STRING, 40008, L"Give Exp (20 000)");
                AppendMenuW(debug_party, MF_ENABLED | MF_STRING, 40059, L"Give Skills (50 each)");
                AppendMenuW(debug_party, MF_ENABLED | MF_STRING, 40013, L"Remove Gold");

                HMENU debug_party_setconditions = CreatePopupMenu();
                AppendMenuW(debug_party, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)debug_party_setconditions, L"Set Condition");
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
            AppendMenuW(debug, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)debug_items, L"&Items");
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
            AppendMenuW(debug, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)debug_graphics, L"&Graphics");
            {
                HMENU lights_off = CreatePopupMenu();
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)lights_off, L"Lights");//40104
                {
                    AppendMenuW(lights_off, MF_ENABLED | MF_STRING, 40123, L"Lights on");
                    AppendMenuW(lights_off, MF_ENABLED | MF_STRING, 40124, L"Lights off");
                }
                //AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING, 40025, L"Debug Lights");
                HMENU debug_lights = CreatePopupMenu();
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)debug_lights, L"Debug Lights");//40104
                {
                    AppendMenuW(debug_lights, MF_ENABLED | MF_STRING, 40125, L"Debug lights on");
                    AppendMenuW(debug_lights, MF_ENABLED | MF_STRING, 40126, L"Debug lights off");
                }
                HMENU debug_terrain = CreatePopupMenu();
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)debug_terrain, L"Debug Terrain");//40104
                {
                    AppendMenuW(debug_terrain, MF_ENABLED | MF_STRING, 40127, L"Debug Terrain on");
                    AppendMenuW(debug_terrain, MF_ENABLED | MF_STRING, 40128, L"Debug Terrain off");
                }
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_GRAYED, 40023, L"Lighting Mode");
                AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING | MF_GRAYED, 40024, L"Lighting Geometry");

                //AppendMenuW(debug_graphics, MF_ENABLED | MF_STRING/* | MF_GRAYED*/, 40123, L"Lights Off");
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
            AppendMenuW(debug, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)debug_misc, L"&Misc");
            {
                AppendMenuW(debug_misc, MF_ENABLED | MF_STRING | MF_GRAYED, 40066, L"Object Viewcone Culling");
                AppendMenuW(debug_misc, MF_ENABLED | MF_STRING | MF_GRAYED, 40068, L"Red Tint");
                AppendMenuW(debug_misc, MF_ENABLED | MF_STRING | MF_GRAYED, 40071, L"Display Secrets");
                AppendMenuW(debug_misc, MF_ENABLED | MF_STRING | MF_GRAYED, 40102, L"Massive Bloodsplat");
                AppendMenuW(debug_misc, MF_ENABLED | MF_STRING | MF_GRAYED, 40103, L"Underwater Gravity");
            }

            HMENU debug_eax = CreatePopupMenu();
            AppendMenuW(debug, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)debug_eax, L"EAX Environs");
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
        AppendMenuW(menu, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)other, L"&Other");
        {
            HMENU other_wizard_eye = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)other_wizard_eye, L"Wizard eye");
            {
                AppendMenuW(other_wizard_eye, MF_ENABLED | MF_STRING, 40101, L"Wizard eye on");
                AppendMenuW(other_wizard_eye, MF_ENABLED | MF_STRING, 40102, L"Wizard eye off");
            }
            HMENU other_new_draw_object_dist = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)other_new_draw_object_dist, L"New draw object distance");
            {
                AppendMenuW(other_new_draw_object_dist, MF_ENABLED | MF_STRING, 40103, L"New draw object dist on");
                AppendMenuW(other_new_draw_object_dist, MF_ENABLED | MF_STRING, 40104, L"New draw object dist off");
            }
            HMENU other_change_seasons = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)other_change_seasons, L"Change seasons");
            {
                AppendMenuW(other_change_seasons, MF_ENABLED | MF_STRING, 40105, L"Change seasons on");
                AppendMenuW(other_change_seasons, MF_ENABLED | MF_STRING, 40106, L"Change seasons off");
            }
            HMENU other_all_magic = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)other_all_magic, L"All magic");
            {
                AppendMenuW(other_all_magic, MF_ENABLED | MF_STRING, 40107, L"All magic on");
                AppendMenuW(other_all_magic, MF_ENABLED | MF_STRING, 40108, L"All magic off");
            }
            HMENU other_debug_information = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)other_debug_information, L"Debug information");
            {
                AppendMenuW(other_debug_information, MF_ENABLED | MF_STRING, 40109, L"Debug information on");
                AppendMenuW(other_debug_information, MF_ENABLED | MF_STRING, 40110, L"Debug information off");
            }
            HMENU other_show_picked_face = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)other_show_picked_face, L"Show picked face");
            {
                AppendMenuW(other_show_picked_face, MF_ENABLED | MF_STRING, 40111, L"Show picked face on");
                AppendMenuW(other_show_picked_face, MF_ENABLED | MF_STRING, 40112, L"Show picked face off");
            }
            HMENU other_draw_portals_loops = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)other_draw_portals_loops, L"Draw portals loops");
            {
                AppendMenuW(other_draw_portals_loops, MF_ENABLED | MF_STRING, 40113, L"Draw portals loops on");
                AppendMenuW(other_draw_portals_loops, MF_ENABLED | MF_STRING, 40114, L"Draw portals loops off");
            }
            HMENU other_new_speed = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)other_new_speed, L"New_speed");
            {
                AppendMenuW(other_new_speed, MF_ENABLED | MF_STRING, 40115, L"New_speed on");
                AppendMenuW(other_new_speed, MF_ENABLED | MF_STRING, 40116, L"New_speed off");
            }
            HMENU other_snow = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)other_snow, L"Snow");
            {
                AppendMenuW(other_snow, MF_ENABLED | MF_STRING, 40117, L"Snowfall on");
                AppendMenuW(other_snow, MF_ENABLED | MF_STRING, 40118, L"Snowfall off");
            }
            HMENU other_draw_terrain_dist_mist = CreatePopupMenu();
            AppendMenuW(other, MF_ENABLED | MF_STRING | MF_POPUP, (UINT_PTR)other_draw_terrain_dist_mist, L"New draw terrain distance");
            {
                AppendMenuW(other_draw_terrain_dist_mist, MF_ENABLED | MF_STRING, 40119, L"New draw terrain distance on");
                AppendMenuW(other_draw_terrain_dist_mist, MF_ENABLED | MF_STRING, 40120, L"New draw terrain distance off");
            }

        }
    }
    return menu;
}




bool OSWindow::OnOSMenu(int item_id)
{
    switch (item_id)
    {
        default: return false;

        //case 103:  pRenderer->SavePCXScreenshot(); break;
        case 101:  // Quit game
        case 40001: // Menu "File"-> "Exit"
            pEngine->Deinitialize();
            SendMessageW(api_handle, WM_DESTROY, 0, 0);
            break;

        /*case 104: //F4 button
          pRenderer->ChangeBetweenWinFullscreenModes();
          if (pArcomageGame->bGameInProgress)
            pArcomageGame->field_F6 = 1;
        break;*/

        //SubMenu "Party"
        case 40006:  pParty->SetFood(pParty->uNumFoodRations + 20); break;
        case 40007:  pParty->SetGold(pParty->uNumGold + 10000);     break;
        case 40008:  pParty->GivePartyExp(20000);  break;
        case 40013:  pParty->SetGold(0);   break;

        case 40059:
            for (uint i = 0; i < 4; ++i)
                pParty->pPlayers[i].uSkillPoints = 50;
            break;

        case 40029:  pPlayers[uActiveCharacter]->SetPertified(pParty->uTimePlayed);  break;
        case 40030:  pPlayers[uActiveCharacter]->SetWeak(pParty->uTimePlayed);       break;
        case 40031:  pPlayers[uActiveCharacter]->SetPoisonSevere(pParty->uTimePlayed);    break;
        case 40032:  pPlayers[uActiveCharacter]->SetPoisonMedium(pParty->uTimePlayed);    break;
        case 40033:  pPlayers[uActiveCharacter]->SetPoisonWeak(pParty->uTimePlayed);    break;
        case 40034:  pPlayers[uActiveCharacter]->SetDiseaseSevere(pParty->uTimePlayed);   break;
        case 40035:  pPlayers[uActiveCharacter]->SetDiseaseMedium(pParty->uTimePlayed);   break;
        case 40036:  pPlayers[uActiveCharacter]->SetDiseaseWeak(pParty->uTimePlayed);   break;
        case 40037:  pPlayers[uActiveCharacter]->SetCursed(pParty->uTimePlayed);     break;
        case 40038:  pPlayers[uActiveCharacter]->SetInsane(pParty->uTimePlayed);     break;
        case 40039:  pPlayers[uActiveCharacter]->SetDrunk(pParty->uTimePlayed);      break;
        case 40040:  pPlayers[uActiveCharacter]->SetUnconcious(pParty->uTimePlayed); break;
        case 40041:  pPlayers[uActiveCharacter]->SetDead(pParty->uTimePlayed);       break;
        case 40042:  pPlayers[uActiveCharacter]->SetEradicated(pParty->uTimePlayed); break;
        case 40043:  pPlayers[uActiveCharacter]->SetAsleep(pParty->uTimePlayed);     break;
        case 40044:  pPlayers[uActiveCharacter]->SetAfraid(pParty->uTimePlayed);     break;
        case 40045:  pPlayers[uActiveCharacter]->SetParalyzed(pParty->uTimePlayed);  break;
        case 40073:  pPlayers[uActiveCharacter]->SetZombie(pParty->uTimePlayed);     break;

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

        //SubMenu "Time"
        case 40009:  pParty->uTimePlayed += Timer::Day;   break;
        case 40010:  pParty->uTimePlayed += Timer::Week;  break;
        case 40011:  pParty->uTimePlayed += Timer::Month; break;
        case 40012:  pParty->uTimePlayed += Timer::Year;  break;

        //SubMenu "Items"
        case 40015: // uItemID_Rep_St == 1
        case 40016: // uItemID_Rep_St == 2
        case 40017: // uItemID_Rep_St == 3
        case 40018: // uItemID_Rep_St == 4
        case 40019: // uItemID_Rep_St == 5
        case 40020: // uItemID_Rep_St == 6
        {
            int pItemID = rand() % 500;
            for (uint i = 0; i < 500; ++i)
            {
                if (pItemID + i > 499)
                    pItemID = 0;
                if (pItemsTable->pItems[pItemID + i].uItemID_Rep_St == (item_id - 40015 + 1))
                {
                    pPlayers[uActiveCharacter]->AddItem(-1, pItemID + i);
                    break;
                }
            }
        }
        break;

        case 40061:
        {
            int pItemID = rand() % 500;
            for (uint i = 0; i < 500; ++i)
            {
                if (pItemID + i > 499)
                    pItemID = 0;
                if (pItemsTable->pItems[pItemID + i].uItemID_Rep_St > 6)
                {
                    pPlayers[uActiveCharacter]->AddItem(-1, pItemID + i);
                    break;
                }
            }
        }
        break;

        //SubMenu "Other"
        case 40101:  wizard_eye = true;  break;
        case 40102:  wizard_eye = false;  break;
        case 40103:  pODMRenderParams->shading_dist_mist = 0x6000;  break;
        case 40104:  pODMRenderParams->shading_dist_mist = 0x2000;  break;
        case 40105:  change_seasons = true;  break;
        case 40106:  change_seasons = false;  break;
        case 40107:  all_magic = true;  break;
        case 40108:  all_magic = false;  break;
        case 40109:  debug_information = true;  break;
        case 40110:  debug_information = false;  break;
        case 40111:  show_picked_face = true;  break;
        case 40112:  show_picked_face = false;  break;
        case 40113:  draw_portals_loops = true;  break;
        case 40114:  draw_portals_loops = false;  break;
        case 40115:  new_speed = true;  break;
        case 40116:  new_speed = false;  break;
        case 40117:  bSnow = true;  break;
        case 40118:  bSnow = false;  break;
        case 40119:  draw_terrain_dist_mist = true;  break;
        case 40120:  draw_terrain_dist_mist = false;  break;
        case 40122:  no_actors = true; break;
        case 40123:  lights_flag = true;  break;
        case 40124:  lights_flag = false;  break;
        case 40125:	debug_lights = true; break;
        case 40126: debug_lights = false; break;
        case 40127: debug_terrain_polygin = true; break;
        case 40128: debug_terrain_polygin = false; break;
    }

    return true;
}