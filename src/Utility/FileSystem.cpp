#include "FileSystem.h"

#include <string>

#include "Utility/String/Ascii.h"

std::filesystem::path expandUserPath(std::string_view path, std::string_view home) {
    if (path.starts_with("~/"))
        return std::filesystem::path(home) / path.substr(2);
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
        std::string foundPart;
        std::error_code error;
        for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(result, error)) {
            std::string entryName = entry.path().filename().string();
            if (ascii::noCaseEquals(entryName, part.string())) {
                foundPart = entryName;
                break;
            }
        }

        // If nothing is found then we just give up and expect the file not found error to be handled by the caller.
        if (foundPart.empty()) {
            foundPart = part.string();
        }

        result /= foundPart;
    }

    return result;
}
