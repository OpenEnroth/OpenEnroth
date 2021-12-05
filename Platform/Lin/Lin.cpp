#include "Platform/Lin/Lin.h"

#include <dirent.h>
#include <fnmatch.h>
#include <sys/time.h>

#include <string>
#include <cstring>
#include <vector>

#include "Engine/Point.h"

void OS_MsgBox(const char *msg, const char *title) {
    // MessageBoxA(nullptr, msg, title, 0);
}

unsigned int OS_GetTime() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return unsigned((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void OS_ShowCursor(bool show) {
    // ShowCursor(show ? 1 : 0);
}

void OS_Sleep(int ms) {
    // Sleep(ms);
}

bool OS_OpenConsole() {
    return true;
}

std::vector<std::string> OS_FindFiles(const std::string& folder, const std::string& mask) {
    std::vector<std::string> result;

    struct dirent *entry;
    DIR *dp;
    int flags = FNM_PATHNAME | FNM_PERIOD | FNM_CASEFOLD;

    dp = opendir(folder.c_str());
    if (dp != NULL) {
        while ((entry = readdir(dp)))
            if (fnmatch(mask.c_str(), entry->d_name, flags) == 0)
                result.push_back(entry->d_name);
        closedir(dp);
    }

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

void OS_SetAppString(const char* pKey, const char* pString) {
}

void OS_GetAppString(const char* pKeyName, char* pOutString, int uBufLen,
    const char* pDefaultValue) {
}

void OS_SetAppInt(const char* pKey, int val) {
}


// r must have strlen(path) + 2 bytes
static int casepath(char const* path, char* r) {
    size_t l = strlen(path);
    char *p = (char*)alloca(l + 1);
    strcpy(p, path);
    size_t rl = 0;

    DIR *d;
    if (p[0] == '/') {
        d = opendir("/");
        p = p + 1;
    } else {
        d = opendir(".");
        r[0] = '.';
        r[1] = 0;
        rl = 1;
    }

    int last = 0;
    char* c = strsep(&p, "/");
    while (c) {
        if (!d) {
            return 0;
        }

        if (last) {
            closedir(d);
            return 0;
        }

        r[rl] = '/';
        rl += 1;
        r[rl] = 0;

        struct dirent *e = readdir(d);
        while (e) {
            if (strcasecmp(c, e->d_name) == 0) {
                strcpy(r + rl, e->d_name);
                rl += strlen(e->d_name);

                closedir(d);
                d = opendir(r);

                break;
            }

            e = readdir(d);
        }

        if (!e) {
            strcpy(r + rl, c);
            rl += strlen(c);
            last = 1;
        }

        c = strsep(&p, "/");
    }

    if (d) closedir(d);
    return 1;
}


FILE* fcaseopen(const std::string& path, char const* mode) {
    return fcaseopen(path.c_str(), mode);
}


FILE *fcaseopen(char const *path, char const *mode) {
    FILE *f = fopen(path, mode);
    if (!f) {
        char *r = (char*)alloca(strlen(path) + 2);
        if (casepath(path, r)) {
            f = fopen(r, mode);
        }
    }
    return f;
}

std::string OS_GetDirSeparator() {
    return "/";
}
