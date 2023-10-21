#include "FileSystem.h"

#include "String.h"
#include "System.h"

static std::string homePath() {
#ifdef _WINDOWS
    return u8getenv("USERPROFILE");
#else
    return u8getenv("HOME");
#endif
}

std::filesystem::path expandUserPath(std::string path) {
    if (path.starts_with("~/"))
        return std::filesystem::path(homePath()) / path.substr(2);
    return path;
}

std::filesystem::path makeCaseInsensitivePath(std::filesystem::path path) {
    if (path.empty())
        return path;

    std::filesystem::path result;
    if (path.is_relative()) {
        result = std::filesystem::path(".");
    } else {
        result = path.root_path();
        path = path.relative_path();
    }

    for (const std::filesystem::path &part : path) {
        std::u8string foundPart;
        std::error_code error;
        for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(result, error)) {
            std::u8string entryName = entry.path().filename().u8string();
            if (iequalsAscii(entryName, part.u8string())) {
                foundPart = entryName;
                break;
            }
        }

        // If nothing is found then we just give up and expect the file not found error to be handled by the caller.
        if (foundPart.empty()) {
            foundPart = part.u8string();
        }

        result /= foundPart;
    }

    return result;
}
