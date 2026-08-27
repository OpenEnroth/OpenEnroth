#include "FileSystemException.h"

#include <cassert>
#include <string>

#include "Utility/System/PathView.h"
#include "FileSystem.h"

FileSystemException::FileSystemException(FileSystemError error, std::string_view arg0) :
    Exception("{}", formatMessage(error, arg0))
{}

std::string FileSystemException::formatMessage(FileSystemError error, std::string_view arg0) {
    switch (error) {
    default: assert(false); [[fallthrough]];

    case FS_LS_FAILED_PATH_DOESNT_EXIST:
        return fmt::format("Could not list '{}' because the provided path doesn't exist.", arg0);
    case FS_LS_FAILED_PATH_IS_FILE:
        return fmt::format("Could not list '{}' because the provided path is not a directory.", arg0);
    case FS_LS_FAILED_PATH_NOT_ACCESSIBLE:
        return fmt::format("Could not list '{}' because the provided path is not accessible.", arg0);

    case FS_READ_FAILED_PATH_NOT_READABLE:
        return fmt::format("Could not read '{}' because the provided path is not readable.", arg0);
    case FS_READ_FAILED_PATH_DOESNT_EXIST:
        return fmt::format("Could not read '{}' because the provided path doesn't exist.", arg0);
    case FS_READ_FAILED_PATH_IS_DIR:
        return fmt::format("Could not read '{}' because the provided path is a directory.", arg0);
    case FS_READ_FAILED_PATH_NOT_ACCESSIBLE:
        return fmt::format("Could not read '{}' because the provided path is not accessible.", arg0);

    case FS_WRITE_FAILED_PATH_NOT_WRITEABLE:
        return fmt::format("Could not write to '{}' because the provided path is not writeable.", arg0);
    case FS_WRITE_FAILED_FILE_IN_PATH:
        return fmt::format("Could not write to '{}' because the provided path goes through an existing file.", arg0);
    case FS_WRITE_FAILED_PATH_IS_DIR:
        return fmt::format("Could not write to '{}' because the provided path is a directory.", arg0);
    case FS_WRITE_FAILED_PATH_NOT_ACCESSIBLE:
        return fmt::format("Could not write to '{}' because the provided path is not accessible.", arg0);

    case FS_REMOVE_FAILED_PATH_NOT_WRITEABLE:
        return fmt::format("Could not remove '{}' because the provided path is not writeable.", arg0);
    case FS_REMOVE_FAILED_PATH_NOT_ACCESSIBLE:
        return fmt::format("Could not remove '{}' because the provided path is not accessible.", arg0);
    }
}

[[noreturn]] void FileSystemException::raise(const FileSystem *fs, FileSystemError error, PathView arg0) {
    throw FileSystemException(error, fs->displayPath(arg0));
}
