#include "Application.h"

#include <Shlwapi.h>
#include <Windows.h>
#pragma comment(lib, "Shlwapi.lib")
#include <Shlobj.h>
#pragma comment(lib, "Shell32.lib")

#include <string>
#include <vector>

#include "Launcher/Launcher.Windows/resource.h"

int __stdcall BrowseFolderCallback(HWND hwnd, UINT msg, LPARAM lparam, LPARAM data) {
    if (msg == BFFM_INITIALIZED) {
        SendMessageA(hwnd, BFFM_SETSELECTION, true, data);
    }
    return 0;
}

INT_PTR __stdcall DialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    Application *app = (Application *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch (msg) {
        case WM_INITDIALOG: {
            app = (Application *)lparam;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)app);

            CheckRadioButton(
                hwnd,
                IDC_RADIO_RENDERER_DIRECTDRAW,
                IDC_RADIO_RENDERER_OPENGL,
                IDC_RADIO_RENDERER_DIRECTDRAW);
            break;
        }

        case WM_COMMAND: {
            if (LOWORD(wparam) == IDC_BUTTON_BROWSE_INSTALL) {
                char choice[2000];
                choice[0] = 0;
                strcpy(choice, app->GetMm7InstallPath().c_str());

                BROWSEINFO bi;
                ZeroMemory(&bi, sizeof(bi));
                bi.lParam = (LPARAM)choice;
                bi.lpfn = BrowseFolderCallback;
                bi.ulFlags = BIF_USENEWUI;
                PIDLIST_ABSOLUTE list = SHBrowseForFolder(&bi);

                SHGetPathFromIDListA(list, choice);
                app->SetMm7InstallPath(std::string(choice));
                SendDlgItemMessageA(hwnd, IDC_EDIT_MM7_INSTALL_DIR, WM_SETTEXT,
                                    0, (LPARAM)choice);

                return TRUE;
            } else if (LOWORD(wparam) == IDC_BUTTON_LAUNCH) {
                char mm7_install_dir[2000];
                GetWindowTextA(GetDlgItem(hwnd, IDC_EDIT_MM7_INSTALL_DIR),
                               mm7_install_dir, 2000);

                app->SetMm7InstallPath(std::string(mm7_install_dir));

                if (IsDlgButtonChecked(hwnd, IDC_RADIO_RENDERER_DIRECTDRAW) == BST_CHECKED) {
                    app->SetRenderer(std::string("DirectDraw"));
                } else if (IsDlgButtonChecked(hwnd, IDC_RADIO_RENDERER_OPENGL) == BST_CHECKED) {
                    app->SetRenderer(std::string("OpenGL"));
                }

                std::string config_errors;
                if (!app->ValidateConfig(config_errors)) {
                    MessageBoxA(hwnd, config_errors.c_str(),
                                "Configuration error", MB_OK);
                } else {
                    PostMessage(hwnd, WM_QUIT, 0, 0);
                }

                return TRUE;
            }
            break;
        }
    }
    return FALSE;
}

Application *Application::Configure(ApplicationConfig &config) {
    this->config = config;
    return this;
}

void Application::SetMm7InstallPath(const std::string &path) {
    this->config.mm7_install_path = path;
}
const std::string &Application::GetMm7InstallPath() const {
    return this->config.mm7_install_path;
}

static std::string GetExeFilename() {
    char buf[2000];
    GetModuleFileNameA(GetModuleHandle(0), buf, 2000);
    return std::string(buf);
}

static std::string GetExePath() {
    auto filename = GetExeFilename();
    return filename.substr(0, filename.find_last_of("\\/"));
}

bool Application::ValidateConfig(std::string &out_errors) {
    out_errors = "";

    std::string mm7_exe_path = config.mm7_install_path + "/MM7.exe";
    if (!PathFileExistsA(mm7_exe_path.c_str())) {
        out_errors =
            "Might and Magic VII exe not found in: " + config.mm7_install_path;
        return false;
    }

    return true;
}

void Application::Run() {
    int init = CoInitializeEx(0, COINIT_APARTMENTTHREADED);  // SHBrowseForFolder

    auto module = GetModuleHandleA(nullptr);
    HWND dialog = CreateDialogParamA(module, MAKEINTRESOURCEA(IDD_FORMVIEW),
                                     nullptr, DialogProc, (LPARAM)this);
    HICON icon =
        (HICON)LoadImageA(module, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0,
                         LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
    SendMessageA(dialog, WM_SETICON, ICON_BIG, (LPARAM)icon);
    SendDlgItemMessageA(dialog, IDC_EDIT_MM7_INSTALL_DIR, WM_SETTEXT, 0,
                        (LPARAM)config.mm7_install_path.c_str());

    MSG msg;
    while (GetMessageA(&msg, dialog, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    std::string openenroth_filename = GetExePath() + "/" + "OpenEnroth.exe";
    std::string command_line = openenroth_filename + " -window -nointro -nologo -novideo -nomarg -render=" + GetRenderer();

    std::vector<char> cmd(command_line.begin(), command_line.end());
    cmd.push_back(0);
    CreateProcessA(
        openenroth_filename.c_str(), cmd.data(), nullptr, nullptr, FALSE,
        NORMAL_PRIORITY_CLASS, nullptr,
        config.mm7_install_path.c_str(), &si, &pi);

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

}
