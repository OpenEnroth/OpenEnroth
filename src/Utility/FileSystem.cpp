#include "FileSystem.h"

#include "String.h"

static std::string homePath() {
    const char *result = getenv("HOME");
    if (result)
        return result;

    // TODO(captainurist): this will break with unicode usernames on Windows
    result = getenv("USERPROFILE");
    if (result)
        return result;

    return std::string();
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
#ifdef __ANDROID__
        // TODO: android have troubles with std::u8string, so use std::string for now instead
        std::string foundPart;
#else
        std::u8string foundPart;
#endif
        std::error_code error;
        for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(result, error)) {
#ifdef __ANDROID__
            // TODO: android have troubles with std::u8string, so use std::string for now instead
            std::string entryName = entry.path().filename();
            if (iequals(entryName, part.string())) {
#else
            std::u8string entryName = entry.path().filename().u8string();
            if (iequalsAscii(entryName, part.u8string())) {
#endif
                foundPart = entryName;
                break;
            }
        }

        // If nothing is found then we just give up and expect the file not found error to be handled by the caller.
        if (foundPart.empty())
            foundPart = part.u8string();

        result /= foundPart;
    }

    return result;
}
