#include "Platform/Posix/Posix.h"

#include <SDL.h>
#include <dirent.h>
#include <glob.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <filesystem>

#include "Utility/String.h"

bool OS_OpenConsole() {
    return true;
}

//////////////////// There is no Windows Registry ////////////////////

bool OS_GetAppString(const char* path, char* out_string, int out_string_size) {
    return false;
}

std::filesystem::path OS_makepath(std::string path) {
    if (path.starts_with("~/"))
        return std::getenv("HOME") + std::filesystem::path::preferred_separator + path.substr(2);
    return path;
}

std::filesystem::path OS_casepath(std::filesystem::path path) {
    if (path.empty())
        return path;

    std::filesystem::path result;
    if (path.is_relative())
        result = std::filesystem::path(".");

    for (const std::string &part : path) {
        if (part == "/") {
            result = part;
            continue;
        }

        std::string foundPart;
        std::error_code error;
        for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(result, error)) {
            std::string entryName = entry.path().filename().string();

            if (iequals(entryName, part)) {
                foundPart = entryName;
                break;
            }
        }

        // If nothing is found then we just give up and expect the file not found error to be handled by the caller.
        if (foundPart.empty())
            foundPart = part;

        result /= foundPart;
    }

    return result;
}

bool OS_FileExists(const std::string& path) {
    return _access(path.c_str(), 0) != -1;
}
