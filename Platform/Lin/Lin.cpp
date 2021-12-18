#include "Platform/Lin/Lin.h"

#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <string>
#include <cstring>
#include <sstream>
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

char OS_GetDirSeparator() {
    return '/';
}

std::string OS_casepath(std::string path) {
    std::string r;
    std::string sep;
    std::vector<std::string> tokens;

    sep.push_back(OS_GetDirSeparator());
    size_t pos = 0;

    DIR *d = nullptr;
    if (!path.substr(0, 1).compare(sep)) {
        r = sep;
        d = opendir("/");
    } else {
        d = opendir(".");
    }

    std::stringstream ss (path);
    std::string s;

    while (std::getline(ss, s, OS_GetDirSeparator())) {
        tokens.push_back(s);
    }

    int i = 0;
    for (auto it = tokens.begin(); it < tokens.end(); it++, i++) {
        s = *it;

        if (s.empty())
            continue;

        if (!d) {
            if (!r.empty() && r.compare(sep))
                r += sep;
            r += s;

            continue;
        }

        bool found = false;
        struct dirent *e = readdir(d);
        while (e) {
            if (strcasecmp(s.c_str(), e->d_name) == 0) {
                found = true;

                if (!r.empty() && r.compare(sep))
                    r += sep;
                r += e->d_name;

                closedir(d);

                // some filesystems like reiserfs don't set entry type and we need additional step
                if (e->d_type == DT_UNKNOWN) {
                    struct stat st;
                    if (stat(r.c_str(), &st) != -1) {
                        if (S_ISDIR(st.st_mode))
                            e->d_type = DT_DIR;
                    }
                }

                if (e->d_type == DT_DIR) {
                    d = opendir(r.c_str());
                } else
                    d = nullptr;

                break;
            }

            e = readdir(d);
        }

        if (!found) {
            if (!r.empty() && r.compare(sep))
                r += sep;
            r += s;
        }
    }

    if (d)
        closedir(d);

    return r;
}
