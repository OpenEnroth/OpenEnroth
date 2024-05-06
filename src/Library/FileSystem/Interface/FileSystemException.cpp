#include "FileSystemException.h"

#include <cassert>
#include <string>

#include "FileSystemPath.h"

FileSystemException::FileSystemException(Code code, const FileSystemPath &arg0) : Exception("{}", formatMessage(code, arg0, FileSystemPath())) {}

FileSystemException::FileSystemException(Code code, const FileSystemPath &arg0, const FileSystemPath &arg1) : Exception("{}", formatMessage(code, arg0, arg1)) {}

std::string FileSystemException::formatMessage(Code code, const FileSystemPath &arg0, const FileSystemPath &arg1) {
    switch (code) {
    default: assert(false); [[fallthrough]];

    case LS_FAILED_PATH_DOESNT_EXIST:
        return fmt::format("Could not list '{}' because the provided path doesn't exist.", arg0);
    case LS_FAILED_PATH_IS_FILE:
        return fmt::format("Could not list '{}' because the provided path is not a directory.", arg0);

    case READ_FAILED_PATH_NOT_READABLE:
        return fmt::format("Could not read '{}' because the provided path is not readable.", arg0);
    case READ_FAILED_PATH_DOESNT_EXIST:
        return fmt::format("Could not read '{}' because the provided path doesn't exist.", arg0);
    case READ_FAILED_PATH_IS_DIR:
        return fmt::format("Could not read '{}' because the provided path is a directory.", arg0);

    case WRITE_FAILED_PATH_NOT_WRITEABLE:
        return fmt::format("Could not write to '{}' because the provided path is not writeable.", arg0);
    case WRITE_FAILED_PATH_IS_DIR:
        return fmt::format("Could not write to '{}' because the provided path is a directory.", arg0);

    case RENAME_FAILED_DST_NOT_WRITEABLE:
        return fmt::format("Could not rename '{}' to '{}' because destination path is not writeable.", arg0, arg1);
    case RENAME_FAILED_SRC_NOT_WRITEABLE:
        return fmt::format("Could not rename '{}' to '{}' because source path is not writeable.", arg0, arg1);
    case RENAME_FAILED_SRC_DOESNT_EXIST:
        return fmt::format("Could not rename '{}' to '{}' because source path doesn't exist.", arg0, arg1);
    case RENAME_FAILED_DST_IS_DIR:
        return fmt::format("Could not rename '{}' to '{}' because destination path is a directory.", arg0, arg1);
    case RENAME_FAILED_SRC_IS_DIR_DST_IS_FILE:
        return fmt::format("Could not rename '{}' to '{}' because source path is a directory and destination path is an exiting file.", arg0, arg1);
    case RENAME_FAILED_SRC_IS_PARENT_OF_DST:
        return fmt::format("Could not rename '{}' to '{}' because moving a directory into one of its subdirectories is not supported.", arg0, arg1);

    case REMOVE_FAILED_PATH_NOT_WRITEABLE:
        return fmt::format("Could not remove '{}' because the provided path is not writeable.", arg0);
    }
}
