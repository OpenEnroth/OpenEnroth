#pragma once

enum class FileType {
    FILE_INVALID, // Returned by `AbstractFileSystem::stat` if file doesn't exist.
    FILE_REGULAR,
    FILE_DIRECTORY,
};
using enum FileType;
