#include "FileSystem.h"

#include "String.h"

static std::string HomePath() {
    const char *result = getenv("HOME");
    if (result)
        return result;

    // TODO(captainurist): this will break with unicode usernames on Windows
    result = getenv("USERPROFILE");
    if (result)
        return result;

    return std::string();
}

std::filesystem::path ExpandUserPath(std::string path) {
    if (path.starts_with("~/"))
        return HomePath() + std::filesystem::path::preferred_separator + path.substr(2);
    return path;
}

std::filesystem::path MakeCaseInsensitivePath(std::filesystem::path path) {
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
