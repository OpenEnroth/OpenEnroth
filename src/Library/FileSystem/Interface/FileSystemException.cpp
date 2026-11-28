#include "FileSystemException.h"

#include <cassert>
#include <string>

#include "FileSystem.h"

FileSystemException::FileSystemException(FileSystemError error, std::string_view displayPath) :
    Exception("{}", formatMessage(error, displayPath))
{}

std::string FileSystemException::formatMessage(FileSystemError error, std::string_view displayPath) {
    switch (error) {
    default: assert(false); [[fallthrough]];

    case FS_LS_FAILED_PATH_DOESNT_EXIST:
        return fmt::format("Could not list '{}' because the provided path doesn't exist.", displayPath);
    case FS_LS_FAILED_PATH_IS_FILE:
        return fmt::format("Could not list '{}' because the provided path is not a directory.", displayPath);
    case FS_LS_FAILED_PATH_NOT_ACCESSIBLE:
        return fmt::format("Could not list '{}' because the provided path is not accessible.", displayPath);

    case FS_READ_FAILED_PATH_NOT_READABLE:
        return fmt::format("Could not read '{}' because the provided path is not readable.", displayPath);
    case FS_READ_FAILED_PATH_DOESNT_EXIST:
        return fmt::format("Could not read '{}' because the provided path doesn't exist.", displayPath);
    case FS_READ_FAILED_PATH_IS_DIR:
        return fmt::format("Could not read '{}' because the provided path is a directory.", displayPath);
    case FS_READ_FAILED_PATH_NOT_ACCESSIBLE:
        return fmt::format("Could not read '{}' because the provided path is not accessible.", displayPath);

    case FS_WRITE_FAILED_PATH_NOT_WRITEABLE:
        return fmt::format("Could not write to '{}' because the provided path is not writeable.", displayPath);
    case FS_WRITE_FAILED_FILE_IN_PATH:
        return fmt::format("Could not write to '{}' because the provided path goes through an existing file.", displayPath);
    case FS_WRITE_FAILED_PATH_IS_DIR:
        return fmt::format("Could not write to '{}' because the provided path is a directory.", displayPath);
    case FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE:
        return fmt::format("Could not write to '{}' because the provided path is not accessible.", displayPath);

    case FS_REMOVE_FAILED_PATH_NOT_WRITEABLE:
        return fmt::format("Could not remove '{}' because the provided path is not writeable.", displayPath);
    case FS_REMOVE_FAILED_PATH_NOT_ACCESSIBLE:
        return fmt::format("Could not remove '{}' because the provided path is not accessible.", displayPath);
    }
}

[[noreturn]] void FileSystemException::raise(const FileSystem *fs, FileSystemError error, NormalPathView path) {
    throw FileSystemException(error, fs->displayPath(path));
}
