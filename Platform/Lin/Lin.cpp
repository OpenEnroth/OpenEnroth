#include "Engine/Point.h"

#include <string>
#include <vector>

#include <sys/time.h>

void OS_MsgBox(const char *msg, const char *title) {
    //MessageBoxA(nullptr, msg, title, 0);
}

unsigned int OS_GetTime() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return unsigned((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

bool OS_IfShiftPressed() {
    //return GetAsyncKeyState(VK_SHIFT);
    return false;
}

bool OS_IfCtrlPressed() {
    //return GetAsyncKeyState(VK_CONTROL);
    return false;
}

void OS_ShowCursor(bool show) {
    //ShowCursor(show ? 1 : 0);
}

void OS_WaitMessage() {
    //WaitMessage();
}

void OS_Sleep(int ms) {
    //Sleep(ms);
}

Point OS_GetMouseCursorPos() {
    //POINT pt;
    //GetCursorPos(&pt);

    //SDL or XQueryPointer
    return Point(0, 0);
}

bool OS_OpenConsole() {
    /*if (AllocConsole()) {
        freopen("conin$", "r", stdin);
        freopen("conout$", "w", stdout);
        freopen("conout$", "w", stderr);

        return true;
    }*/

    return false;
}

std::vector<std::string> OS_FindFiles(const std::string& folder, const std::string& mask) {
    std::vector<std::string> result;

    /*std::string path = folder + "\\" + mask;

    WIN32_FIND_DATAA ffd = { 0 };
    HANDLE hFind = FindFirstFileA(path.c_str(), &ffd);
    if (INVALID_HANDLE_VALUE == hFind) {
        return result;
    }

    do {
        if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            result.push_back(ffd.cFileName);
        }
    } while (FindNextFileA(hFind, &ffd) != 0);

    FindClose(hFind);*/

    return result;
}


//////////////////// There is no Windows Registry ////////////////////

bool OS_GetAppString(const char* path, char* out_string, int out_string_size) {
    return false;
}

int OS_GetAppInt(const char* pKey, int uDefValue) {
    unsigned int cbData;         // [sp+8h] [bp-20h]@1
    const char *lpValueName;     // [sp+Ch] [bp-1Ch]@1
    unsigned int dwDisposition;  // [sp+10h] [bp-18h]@2
    unsigned char Data[4];       // [sp+14h] [bp-14h]@5

    lpValueName = pKey;
    cbData = 4;
    *(int *)Data = uDefValue;

    return *(int *)Data;
}

void OS_SetAppString(const char* pKey, const char* pString) {}

void OS_GetAppString(const char* pKeyName, char* pOutString, int uBufLen,
    const char* pDefaultValue) {}

void OS_SetAppInt(const char* pKey, int val) {}


// r must have strlen(path) + 2 bytes
static int casepath(char const* path, char* r)
{
    size_t l = strlen(path);
    char* p = alloca(l + 1);
    strcpy(p, path);
    size_t rl = 0;

    DIR *d;
    if (p[0] == '/')
    {
        d = opendir("/");
        p = p + 1;
    }
    else
    {
        d = opendir(".");
        r[0] = '.';
        r[1] = 0;
        rl = 1;
    }

    int last = 0;
    char* c = strsep(&p, "/");
    while (c)
    {
        if (!d)
        {
            return 0;
        }

        if (last)
        {
            closedir(d);
            return 0;
        }

        r[rl] = '/';
        rl += 1;
        r[rl] = 0;

        struct dirent *e = readdir(d);
        while (e)
        {
            if (strcasecmp(c, e->d_name) == 0)
            {
                strcpy(r + rl, e->d_name);
                rl += strlen(e->d_name);

                closedir(d);
                d = opendir(r);

                break;
            }

            e = readdir(d);
        }

        if (!e)
        {
            strcpy(r + rl, c);
            rl += strlen(c);
            last = 1;
        }

        c = strsep(&p, "/");
    }

    if (d) closedir(d);
    return 1;
}

FILE *fcaseopen(char const *path, char const *mode)
{
    FILE *f = fopen(path, mode);
    if (!f)
    {
        char *r = alloca(strlen(path) + 2);
        if (casepath(path, r))
        {
            f = fopen(r, mode);
        }
    }
    return f;
}